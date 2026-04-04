// horizons_fetch.c
// Build: gcc horizons_fetch.c -lcurl -o horizons_fetch
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <curl/curl.h>

typedef struct { float x,y,z; } Vec3;

typedef struct {
    Vec3 r_km;
    Vec3 v_kms;
    float mass_kg;
} StateVector;

typedef struct {
    char *data;
    size_t size;
} Buffer;

static size_t write_cb(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsz = size * nmemb;
    Buffer *buf = (Buffer*)userp;
    char *p = (char*)realloc(buf->data, buf->size + realsz + 1);
    if (!p) return 0;
    buf->data = p;
    memcpy(&(buf->data[buf->size]), contents, realsz);
    buf->size += realsz;
    buf->data[buf->size] = '\0';
    return realsz;
}

static char* url_escape(CURL *curl, const char *s) {
    return curl_easy_escape(curl, s, 0);
}

/*
Horizons VECTORS CSV_FORMAT line (VEC_TABLE=2) is typically:
  JD, X, Y, Z, VX, VY, VZ
So you must parse 7 doubles and SKIP vals[0] (JD).
*/
static int parse_first_statevec(const char *resp, StateVector *out) {
    const char *soe = strstr(resp, "$$SOE");
    const char *eoe = strstr(resp, "$$EOE");
    if (!soe || !eoe || eoe <= soe) return 0;

    // First line after $$SOE
    const char *p = strchr(soe, '\n');
    if (!p) return 0;
    p++;

    while (p < eoe && (*p == '\r' || *p == '\n')) p++;
    if (p >= eoe) return 0;

    const char *line_end = strchr(p, '\n');
    if (!line_end || line_end > eoe) line_end = eoe;

    char line[2048];
    size_t len = (size_t)(line_end - p);
    if (len >= sizeof(line)) len = sizeof(line) - 1;
    memcpy(line, p, len);
    line[len] = '\0';

    // Collect 7 doubles: JD, X, Y, Z, VX, VY, VZ
    float vals[7];
    int found = 0;

    char *tmp = strdup(line);
    if (!tmp) return 0;

    char *save = NULL;
    for (char *tok = strtok_r(tmp, ",", &save);
         tok != NULL && found < 7;
         tok = strtok_r(NULL, ",", &save)) {

        while (*tok == ' ' || *tok == '\t') tok++;

        char *endp = NULL;
        float v = strtod(tok, &endp);
        if (endp != tok) {
            vals[found++] = v;
        }
    }

    free(tmp);

    if (found < 7) return 0;

    // vals[0] = JD
    out->r_km  = (Vec3){ vals[1], vals[2], vals[3] };
    out->v_kms = (Vec3){ vals[4], vals[5], vals[6] };
    return 1;
}

static int add_one_minute_utc(const char *start, char *stop_out, size_t out_size)
{
    struct tm tm = {0};

    if (sscanf(start, "%d-%d-%d %d:%d:%d",
               &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
               &tm.tm_hour, &tm.tm_min, &tm.tm_sec) != 6)
    {
        stop_out[0] = '\0';
        return 0;
    }

    tm.tm_year -= 1900;
    tm.tm_mon  -= 1;
    tm.tm_isdst = 0;

    time_t t = timegm(&tm);
    if (t == (time_t)-1) {
        stop_out[0] = '\0';
        return 0;
    }

    t += 60;

    struct tm *tm_out = gmtime(&t);
    if (!tm_out) {
        stop_out[0] = '\0';
        return 0;
    }

    strftime(stop_out, out_size, "%Y-%m-%d %H:%M:%S", tm_out);
    return 1;
}

// Fetch state vector for COMMAND id at UTC_TIME (kept as-is).
// We set STOP_TIME = START_TIME + 1 minute, STEP_SIZE=1m, and parse the first row.
static int horizons_get_state(const char *command_id,
                              const char *utc_start_time,
                              const char *utc_stop_time,
                              const char *center,
                              StateVector *out)
{
    int ok = 0;
    CURL *curl = curl_easy_init();
    if (!curl) return 0;

    Buffer buf = {0};

    char *esc_center = url_escape(curl, center);
    char *esc_cmd    = url_escape(curl, command_id);
    char *esc_start  = url_escape(curl, utc_start_time);
    char *esc_stop   = url_escape(curl, utc_stop_time);

    char url[2048];
    snprintf(url, sizeof(url),
        "https://ssd.jpl.nasa.gov/api/horizons.api"
        "?format=text"
        "&MAKE_EPHEM=YES"
        "&EPHEM_TYPE=VECTORS"
        "&VEC_TABLE=2"
        "&VEC_CORR=NONE"
        "&REF_SYSTEM=ICRF"
        "&OUT_UNITS=KM-S"
        "&CSV_FORMAT=YES"
        "&OBJ_DATA=NO"
        "&CENTER=%s"
        "&COMMAND=%s"
        "&START_TIME='%s'"
        "&STOP_TIME='%s'"
        "&STEP_SIZE=1m",
        esc_center, esc_cmd, esc_start, esc_stop
    );

    curl_free(esc_center);
    curl_free(esc_cmd);
    curl_free(esc_start);
    curl_free(esc_stop);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "c-horizons-client/1.0");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);

    CURLcode res = curl_easy_perform(curl);
    if (res == CURLE_OK && buf.data) {
        ok = parse_first_statevec(buf.data, out);
        if (!ok) {
            fprintf(stderr, "Parse failed for %s\nResponse:\n%s\n", command_id, buf.data);
        }
    } else {
        fprintf(stderr, "HTTP fetch failed for %s: %s\n", command_id, curl_easy_strerror(res));
    }

    free(buf.data);
    curl_easy_cleanup(curl);
    return ok;
}

static void print_state(const char *name, const StateVector *s) {
    printf("%s:\n", name);
    printf("  r_km  = (%.6f, %.6f, %.6f)\n", s->r_km.x, s->r_km.y, s->r_km.z);
    printf("  v_kms = (%.9f, %.9f, %.9f)\n", s->v_kms.x, s->v_kms.y, s->v_kms.z);
    printf("  mass = () ")
}

int main(void) {
    const char *CENTER   = "500@0";
    const char *UTC_START_TIME = "2026-02-24";
    const char *UTC_STOP_TIME = "2026-02-25";

    struct { const char *name; const char *id; } bodies[] = {
        {"Sun",     "10"},
        {"Mercury", "199"},
        {"Venus",   "299"},
        {"Earth",   "399"},
        {"Mars",    "499"},
        {"Jupiter", "599"},
        {"Saturn",  "699"},
        {"Uranus",  "799"},
        {"Neptune", "899"},
        {"Pluto",   "999"},
    };

    for (size_t i = 0; i < sizeof(bodies)/sizeof(bodies[0]); i++) {
        StateVector s;
        if (!horizons_get_state(bodies[i].id, UTC_START_TIME, UTC_STOP_TIME, CENTER, &s)) {
            fprintf(stderr, "Failed: %s\n", bodies[i].name);
            continue;
        }
        print_state(bodies[i].name, &s);
    }

    return 0;
}