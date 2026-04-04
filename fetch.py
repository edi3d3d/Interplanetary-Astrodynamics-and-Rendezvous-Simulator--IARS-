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
    mass_kg = None
    diameter_m = None
    GM_km3_s2 = None

    # split into lines and inspect lines that mention mass or diameter/radius/size
    for line in resp.splitlines():
        low = line.lower()
        if 'mass' in low:
            # check for direct kg match on the line
            m = re_kg.search(line)
            if m:
                try:
                    mass_kg = float(m.group(1))
                    # keep searching for better match
                    continue
                except Exception:
                    pass
            # check for format like 'Mass (10^24 kg) = 1989.0'
            m2 = re_pow10.search(line)
            if m2:
                exp = int(m2.group(1))
                v = re_value_after.search(line)
                if v:
                    try:
                        val = float(v.group(1))
                        mass_kg = val * (10 ** exp)
                        continue
                    except Exception:
                        pass
        if any(k in low for k in ('diameter', 'size', 'radius')):
            # look for km
            m = re_km.search(line)
            if m:
                try:
                    val_km = float(m.group(1))
                    # if radius keyword, convert to diameter
                    if 'radius' in low and not 'diameter' in low:
                        diameter_km = val_km * 2.0
                    else:
                        diameter_km = val_km
                    diameter_m = diameter_km * 1000.0
                    # we can continue to look for mass
                    continue
                except Exception:
                    pass

        # try to capture GM (standard gravitational parameter) if present
        if 'gm' in low and GM_km3_s2 is None:
            # look for a numeric token on the line
            m = re.search(r'([+-]?\d+(?:\.\d+)?(?:[eEdD][+-]?\d+)?)', line)
            if m:
                tok = m.group(1).replace('D', 'E').replace('d', 'E')
                try:
                    GM_km3_s2 = float(tok)
                except Exception:
                    GM_km3_s2 = None
                continue

    # fallback: sometimes mass and diameter appear elsewhere without keywords; try to find first kg and first km in document
    if mass_kg is None:
        m = re_kg.search(resp)
        if m:
            try:
                mass_kg = float(m.group(1))
            except Exception:
                mass_kg = None
    # if we found GM but not mass, convert using G in km^3/kg/s^2
    if mass_kg is None and GM_km3_s2 is not None:
        G_km3_per_kg_s2 = 6.67430e-20
        try:
            mass_kg = GM_km3_s2 / G_km3_per_kg_s2
        except Exception:
            mass_kg = None
    if diameter_m is None:
        m = re_km.search(resp)
        if m:
            try:
                diameter_m = float(m.group(1)) * 1000.0
            except Exception:
                diameter_m = None

    return mass_kg, diameter_m


def get_obj_info(command_id: str):
    params = {
        'format': 'text',
        'MAKE_EPHEM': 'NO',
        'OBJ_DATA': 'YES',
        'COMMAND': command_id,
    }
    txt = fetch_text(params)
    if not txt:
        return None, None
    return parse_obj_data(txt)


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

    # compute radius from diameter if available
    radius_m = None
    if diameter is not None:
        radius_m = diameter / 2.0

    # fallbacks
    if pos is None:
        px = py = pz = 0.0
    else:
        px, py, pz = pos
    if vel is None:
        vx = vy = vz = 0.0
    else:
        vx, vy, vz = vel
    if mass is None:
        mass_val = 0.0
    else:
        mass_val = mass
    if radius_m is None:
        radius_val = 0.0
    else:
        radius_val = radius_m

    # Print C-like create_planet block
    print('create_planet(              // ' + name)
    # position: multiply by DISTANCE_UNIT in output as requested
    print('            v3_set({:.12f} * DISTANCE_UNIT, {:.12f} * DISTANCE_UNIT, {:.12f} * DISTANCE_UNIT),'.format(px, py, pz))
    print('            v3_set({:.12f}, {:.12f}, {:.12f}),'.format(vx, vy, vz))
    # mass in kg (use scientific notation)
    print('            {:.12e},'.format(mass_val))
    # radius in meters
    print('            {:.12f} * DISTANCE_UNIT'.format(radius_val))
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

        mass, diam = get_obj_info(cid)
        out['mass_kg'] = mass
        out['diameter_m'] = diam

        print_create_planet(out)


if __name__ == '__main__':
    main()
