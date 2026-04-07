#!/usr/bin/env python3
"""
fetch.py
Fetches state vectors and basic physical data for solar system bodies from JPL Horizons.
- Uses CENTER = "500@0" (same reference as original C code)
- Fetches VECTORS (first line) and OBJ_DATA, parses position (m), velocity (m/s), mass (kg), and size (diameter in m)
- Prints JSON per body including start/stop times

Requires: requests
Install: pip install requests
"""

import requests
import re
import json
import sys
from typing import Optional

BASE = "https://ssd.jpl.nasa.gov/api/horizons.api"
USER_AGENT = "py-horizons-client/1.0"

CENTER = "500@0"
UTC_START_TIME = "2026-02-24 00:00:00"
UTC_STOP_TIME  = "2026-02-25 00:00:00"

BODIES = [
    ("Sun",     "10"),
    ("Mercury", "199"),
    ("Venus",   "299"),
    ("Earth",   "399"),
    ("Moon",    "301"),
    ("Mars",    "499"),
    ("Jupiter", "599"),
    ("Saturn",  "699"),
    ("Uranus",  "799"),
    ("Neptune", "899"),
    ("Pluto",   "999"),
]

sess = requests.Session()
headers = {"User-Agent": USER_AGENT}


def fetch_text(params: dict, timeout: float = 20.0) -> Optional[str]:
    try:
        r = sess.get(BASE, params=params, headers=headers, timeout=timeout)
        r.raise_for_status()
        return r.text
    except requests.exceptions.RequestException as e:
        # Try to give more useful debugging info
        msg = f"HTTP error: {e}"
        try:
            if e.response is not None:
                msg += f"\nStatus: {e.response.status_code}\nURL: {e.response.url}\nResponse snippet: {e.response.text[:2000]}"
        except Exception:
            pass
        print(msg, file=sys.stderr)
        return None


def parse_first_statevec(resp: str):
    # find $$SOE / $$EOE and take first CSV line after SOE
    soe = resp.find("$$SOE")
    eoe = resp.find("$$EOE")
    if soe == -1 or eoe == -1 or eoe <= soe:
        return None
    p = resp.find('\n', soe)
    if p == -1:
        return None
    p += 1
    # skip blank lines
    while p < eoe and resp[p] in ('\r', '\n'):
        p += 1
    if p >= eoe:
        return None
    line_end = resp.find('\n', p)
    if line_end == -1 or line_end > eoe:
        line_end = eoe
    line = resp[p:line_end].strip()
    if not line:
        return None

    parts = [t.strip() for t in line.split(',')]
    # collect numeric values (handle FORTRAN 'D' exponents and quoted fields)
    nums = []
    for tok in parts:
        if not tok:
            continue
        # strip wrapping quotes
        if (tok[0] == '"' and tok[-1] == '"') or (tok[0] == "'" and tok[-1] == "'"):
            tok = tok[1:-1]
        # replace D exponent with E
        tok = tok.replace('D', 'E').replace('d', 'E')
        # remove surrounding parentheses
        tok = tok.strip('()')
        try:
            v = float(tok)
            nums.append(v)
        except Exception:
            # skip non-numeric tokens
            continue
        if len(nums) >= 7:
            break
    if len(nums) < 7:
        # print the raw CSV line for debugging
        print(f"Failed to parse numeric fields from line: {line}", file=sys.stderr)
        return None
    vals = nums[:7]
    # convert: positions are in km -> meters, velocities in km/s -> m/s
    x_km, y_km, z_km = vals[1], vals[2], vals[3]
    vx_kms, vy_kms, vz_kms = vals[4], vals[5], vals[6]
    return {
        'r_m': [x_km * 1000.0, y_km * 1000.0, z_km * 1000.0],
        'v_ms': [vx_kms * 1000.0, vy_kms * 1000.0, vz_kms * 1000.0],
    }


def get_state(command_id: str, start: str, stop: str, center: str):
    params = {
        'format': 'text',
        'MAKE_EPHEM': 'YES',
        'EPHEM_TYPE': 'VECTORS',
        'VEC_TABLE': '2',
        'VEC_CORR': 'NONE',
        'REF_SYSTEM': 'ICRF',
        'OUT_UNITS': 'KM-S',
        'CSV_FORMAT': 'YES',
        'OBJ_DATA': 'NO',
        'CENTER': center,
        'COMMAND': command_id,
        # Horizons requires quoted date/time strings if they contain spaces
        'START_TIME': f"'{start}'",
        'STOP_TIME': f"'{stop}'",
        'STEP_SIZE': '1m',
    }
    txt = fetch_text(params)
    if not txt:
        return None
    parsed = parse_first_statevec(txt)
    if not parsed:
        # print a snippet of the response to stderr for debugging
        snippet = txt[:10000]
        print(f"Parse failed for {command_id}. Response snippet:\n{snippet}", file=sys.stderr)
        return None
    return parsed


# heuristics to extract mass (kg) and diameter (km) / radius (km) from OBJ_DATA text
re_kg = re.compile(r'([+-]?\d+(?:\.\d+)?(?:[eE][+-]?\d+)?)\s*kg', re.IGNORECASE)
re_km = re.compile(r'([+-]?\d+(?:\.\d+)?(?:[eE][+-]?\d+)?)\s*km', re.IGNORECASE)
re_pow10 = re.compile(r'10\^\s*([+-]?\d+)', re.IGNORECASE)
re_value_after = re.compile(r'[:=]\s*([+-]?\d+(?:\.\d+)?(?:[eE][+-]?\d+)?)')


def parse_obj_data(resp: str):
    """
    Parse OBJ_DATA text from Horizons.
    Returns (mass_kg_or_None, radius_m_or_None).
    Handles:
      - explicit mass "kg"
      - GM in "km^3 / s^2" or "km^3 s^-2"
      - diameter or radius in km (or m)
      - formats like "Mass (10^24 kg) = 5.972" and "Diameter = 12756 km"
    """
    mass_kg = None
    radius_m = None

    G = 6.67430e-11  # m^3 kg^-1 s^-2

    # helper regexes
    re_kg = re.compile(r'([+-]?\d+(?:\.\d+)?(?:[eEdD][+-]?\d+)?)\s*(?:kg)\b', re.IGNORECASE)
    re_km = re.compile(r'([+-]?\d+(?:\.\d+)?(?:[eEdD][+-]?\d+)?)\s*(?:km)\b', re.IGNORECASE)
    re_m = re.compile(r'([+-]?\d+(?:\.\d+)?(?:[eEdD][+-]?\d+)?)\s*(?:m)\b', re.IGNORECASE)
    re_gm = re.compile(r'([+-]?\d+(?:\.\d+)?(?:[eEdD][+-]?\d+)?)\s*(?:km\^?3(?:/s\^?2| s\^-?2)|km\^3\s*s?-?2)', re.IGNORECASE)
    re_pow10_unit = re.compile(r'\(10\^?([+-]?\d+)\s*([a-zA-Z/ ]+)\)', re.IGNORECASE)
    re_named = re.compile(r'(diameter|radius|size|mass|gm)\s*[:=]\s*([^\n\r;]+)', re.IGNORECASE)

    # scan lines for likely entries
    for line in resp.splitlines():
        ln = line.strip()
        if not ln:
            continue
        low = ln.lower()

        # Prefer GM (standard gravitational parameter) if present; common format:
        # "GM (km^3 / s^2) = 1.32712440018E11" or "GM = 1.32712440018E11 km^3/s^2"
        if mass_kg is None and ('gm' in low or 'mu=' in low):
            m_gm = re.search(r'([+-]?\d+(?:\.\d+)?(?:[eEdD][+-]?\d+)?)', ln)
            if m_gm:
                try:
                    gm_val = float(m_gm.group(1).replace('D', 'E').replace('d', 'E'))
                    # if the line mentions km^3, assume gm_val is in km^3/s^2; convert to m^3/s^2
                    if 'km' in low:
                        gm_m3_s2 = gm_val * 1e9
                    else:
                        gm_m3_s2 = gm_val
                    # mass = GM / G
                    mass_kg = gm_m3_s2 / G
                    continue
                except Exception:
                    pass

        # explicit mass patterns: either a value with 'kg' or a multiplier like 'Mass (10^30 kg) = 1.98847'
        if mass_kg is None and 'mass' in low:
            # case: 'Mass (10^30 kg) = 1.98847'
            m_pow = re_pow10_unit.search(ln)
            if m_pow:
                try:
                    exp = int(m_pow.group(1))
                    mval = re_value_after.search(ln)
                    if mval:
                        val = float(mval.group(1).replace('D', 'E').replace('d', 'E'))
                        mass_kg = val * (10.0 ** exp)
                        continue
                except Exception:
                    pass

            # case: direct '1234.56 kg'
            m_kg = re_kg.search(ln)
            if m_kg:
                try:
                    mass_kg = float(m_kg.group(1).replace('D', 'E').replace('d', 'E'))
                    continue
                except Exception:
                    pass

            # fallback: look for a bare number after ':' or '=' and interpret as kg if plausibly large
            mnum = re_value_after.search(ln)
            if mnum:
                try:
                    val = float(mnum.group(1).replace('D', 'E').replace('d', 'E'))
                    # heuristics: if val > 1e10 treat as kg; if val < 1e6 then probably not mass
                    if val > 1e6:
                        mass_kg = val
                        continue
                except Exception:
                    pass

        # diameter / radius in km or m
        if any(k in low for k in ('diameter', 'radius', 'size')):
            # check for km first
            mkm = re_km.search(ln)
            if mkm:
                try:
                    val_km = float(mkm.group(1).replace('D', 'E').replace('d', 'E'))
                    if 'diameter' in low:
                        radius_m = (val_km * 1000.0) / 2.0
                    else:
                        radius_m = val_km * 1000.0
                except Exception:
                    pass
            else:
                mm = re_m.search(ln)
                if mm:
                    try:
                        val_m = float(mm.group(1).replace('D', 'E').replace('d', 'E'))
                        if 'diameter' in low:
                            radius_m = val_m / 2.0
                        else:
                            radius_m = val_m
                    except Exception:
                        pass
                else:
                    # try to extract numeric after '=' or ':'
                    mval = re_value_after.search(ln)
                    if mval:
                        try:
                            v = float(mval.group(1).replace('D', 'E').replace('d', 'E'))
                            # guess unit km if value is order of thousands
                            if v > 1000:
                                # likely km diameter
                                if 'diameter' in low:
                                    radius_m = (v * 1000.0) / 2.0
                                else:
                                    radius_m = v * 1000.0
                            else:
                                # likely km or m ambiguous — prefer km -> m
                                if 'diameter' in low:
                                    radius_m = (v * 1000.0) / 2.0
                                else:
                                    radius_m = v * 1000.0
                        except Exception:
                            pass

    # final sanity checks: ensure positive values
    if mass_kg is not None:
        try:
            if mass_kg <= 0:
                mass_kg = None
        except Exception:
            mass_kg = None
    if radius_m is not None:
        try:
            if radius_m <= 0:
                radius_m = None
        except Exception:
            radius_m = None

    return mass_kg, radius_m


def get_obj_info(command_id: str):
    params = {
        'format': 'text',
        'MAKE_EPHEM': 'NO',
        'OBJ_DATA': 'YES',
        'COMMAND': command_id,
    }
    txt = fetch_text(params)
    if not txt:
        return None, None, None
    mass, diam = parse_obj_data(txt)
    return mass, diam, txt


# Authoritative fallbacks (mass in kg, diameter in meters)
FALLBACK_PHYS = {
    "Sun":     {"mass_kg": 1.98847e30, "diameter_m": 1.3927e9},
    "Mercury": {"mass_kg": 3.3011e23,  "diameter_m": 4879400.0},
    "Venus":   {"mass_kg": 4.8675e24,  "diameter_m": 12103600.0},
    "Earth":   {"mass_kg": 5.97237e24, "diameter_m": 12742000.0},
    "Mars":    {"mass_kg": 6.4171e23,  "diameter_m": 6779000.0},
    "Jupiter": {"mass_kg": 1.89813e27, "diameter_m": 139820000.0},
    "Saturn":  {"mass_kg": 5.6834e26,  "diameter_m": 116460000.0},
    "Uranus":  {"mass_kg": 8.6810e25,  "diameter_m": 50724000.0},
    "Neptune": {"mass_kg": 1.02413e26, "diameter_m": 49244000.0},
    "Pluto":   {"mass_kg": 1.303e22,   "diameter_m": 2376600.0},
}

DISTANCE_UNIT = 1.0  # multiply positions by this in generated code (keeps units flexible)

def format_num(n, fmt='{:f}'):
    try:
        return fmt.format(n)
    except Exception:
        return '0.0'


def print_create_planet(out):
    name = out['name']
    pos = out.get('position_m')
    vel = out.get('velocity_m_s')
    mass = out.get('mass_kg')
    diameter = out.get('diameter_m')
    obj_raw = out.get('obj_raw')

    # compute radius from diameter if available
    radius_m = None
    if diameter is not None:
        radius_m = diameter / 2.0

    # apply fallbacks when parsed data is missing or invalid
    fb = FALLBACK_PHYS.get(name)
    if fb is not None:
        if mass is None or not isinstance(mass, (int, float)) or mass <= 0.0:
            mass = fb.get('mass_kg')
            # if we used fallback, log raw OBJ_DATA snippet for debugging
            if obj_raw:
                print(f"[WARN] Using fallback mass for {name}; raw OBJ_DATA snippet:\n{obj_raw[:2000]}", file=sys.stderr)
        if radius_m is None or not isinstance(radius_m, (int, float)) or radius_m <= 0.0:
            diam_fb = fb.get('diameter_m')
            if diam_fb is not None:
                radius_m = diam_fb / 2.0

    # final fallbacks to zeros
    if pos is None:
        px = py = pz = 0.0
    else:
        px, py, pz = pos
    if vel is None:
        vx = vy = vz = 0.0
    else:
        vx, vy, vz = vel
    mass_val = mass if mass is not None else 0.0
    radius_val = radius_m if radius_m is not None else 0.0

    # Print C-like create_planet block
    print('        create_planet(          // ' + name)
    # position: multiply by DISTANCE_UNIT in output as requested
    print('            v3_set({:.12f} * DISTANCE_UNIT, {:.12f} * DISTANCE_UNIT, {:.12f} * DISTANCE_UNIT),'.format(px, py, pz))
    print('            v3_set({:.12f} * VELOCITY_UNIT, {:.12f} * VELOCITY_UNIT, {:.12f} * VELOCITY_UNIT),'.format(vx, vy, vz))
    # mass in kg (use scientific notation)
    print('            {:.12e} * MASS_UNIT,'.format(mass_val))
    # radius in meters
    print('            {:.12f} * SIZE_UNIT'.format(radius_val))
    print('        ),\n')


def main():
    for name, cid in BODIES:
        out = {
            'name': name,
            'command_id': cid,
            'start_time': UTC_START_TIME,
            'stop_time': UTC_STOP_TIME,
            'position_m': None,
            'velocity_m_s': None,
            'mass_kg': None,
            'diameter_m': None,
            'obj_raw': None,
            'error': None,
        }

        state = get_state(cid, UTC_START_TIME, UTC_STOP_TIME, CENTER)
        if not state:
            out['error'] = 'failed to fetch or parse state vector'
            # still print a placeholder create_planet with zeros and comment
            print('/* Failed to fetch state for {} */'.format(name))
            print_create_planet(out)
            continue
        out['position_m'] = state['r_m']
        out['velocity_m_s'] = state['v_ms']

        mass, diam, txt = get_obj_info(cid)
        out['mass_kg'] = mass
        out['diameter_m'] = diam
        out['obj_raw'] = txt

        print_create_planet(out)


if __name__ == '__main__':
    main()
