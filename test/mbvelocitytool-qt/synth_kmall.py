#--------------------------------------------------------------------
#    The MB-system:  synth_kmall.py  1/11/2026
#
#    Copyright (c) 2026 by
#    Christian dos Santos Ferreira
#      MARUM
#      University of Bremen
#      Bremen Germany
#    David W. Caress (caress@mbari.org)
#      Monterey Bay Aquarium Research Institute
#      Moss Landing, California, USA
#    Dale N. Chayes
#      Center for Coastal and Ocean Mapping
#      University of New Hampshire
#      Durham, New Hampshire, USA
#
#    See README.md file for copying and redistribution conditions.
#--------------------------------------------------------------------
"""Write a small synthetic Kongsberg survey line (*.kmall + *.kmwcd) for the tests.

Datagram layouts from the Kongsberg EM datagram format (Rev J): #MRZ version 3,
#MWC version 2, #SPO, #SVP. The scene is chosen so that every value read back
through MBIO can be computed exactly:

- dual swath: 2 receive fans per ping (swath along positions 0 and 1), NBEAMS
  beams each, beam angles from +ANGLE_MAX (port, Kongsberg sign) to -ANGLE_MAX
- flat seafloor BOTTOM_BELOW_TX m below the transducer, transducer DRAFT m deep,
  constant sound speed SOUNDSPEED from the surface down (straight rays exact)
- the .kmall has pings PING0 .. PING0+NPINGS-1; the .kmwcd starts one ping later,
  its name is 1 s later, and its last ping has only fan 0 (file cut)
- water column: sample rate FS, start sample START_SAMPLE in odd beams (0 in even
  beams), NSAMPLES samples, the first NFILL samples the fill value -128, the other
  raw values ampl_raw(), one single raw value RARE_RAW (the lowest non-fill value)
- soundings: method amplitude in even beams, phase in odd beams; beam REJECTED of
  fan 0 rejected (detectionType 2), beam EXTRA of fan 0 an extra detection (1)
- position records every 0.5 s in both files, moving north-east
"""
import math
import os
import struct

NBEAMS = 16
ANGLE_MAX = 60.0          # deg
DRAFT = 5.0               # m, txTransducerDepth_m
BOTTOM_BELOW_TX = 500.0   # m
SOUNDSPEED = 1500.0       # m/s
FS = 20.0                 # Hz, water column sample rate
NSAMPLES = 60
START_SAMPLE = 3
NFILL = 2
RARE_RAW = -50
TVG_OFFSET = 30           # dB
LINE = "0001"
T0 = 1735732800.0         # 2025-01-01 12:00:00 UTC
PING0 = 100
NPINGS = 5
PING_INTERVAL = 2.0       # s
FAN_DELAY = 0.03          # s, fan 1 after fan 0
HEADING = 45.0
LAT0, LON0 = 54.0, 8.0
REJECTED, EXTRA = 4, 3
KMALL_NAME = LINE + "_20250101_120000.kmall"
KMWCD_NAME = LINE + "_20250101_120001.kmwcd"


def beam_angle(b):
    """Kongsberg beam pointing angle (deg, positive port) of beam b."""
    return ANGLE_MAX - 2.0 * ANGLE_MAX * b / (NBEAMS - 1)


def detect_samples(b):
    """Two-way range in samples of the bottom detection of beam b."""
    r = BOTTOM_BELOW_TX / math.cos(math.radians(beam_angle(b)))
    return 2.0 * r / SOUNDSPEED * FS


def ampl_raw(ping, fan, b, k):
    """Raw amplitude (0.5 dB steps) of sample k of a beam; -128 = fill."""
    if k < NFILL:
        return -128
    if ping == PING0 + 1 and fan == 0 and b == 0 and k == NFILL:
        return RARE_RAW
    return (b * 7 + k * 3 + fan * 11 + ping) % 81 - 40


def ping_time(ping, fan=0):
    return T0 + (ping - PING0) * PING_INTERVAL + fan * FAN_DELAY


def position(t):
    """(lat, lon) of the vessel at time t."""
    return LAT0 + 1.0e-5 * (t - T0), LON0 + 1.0e-5 * (t - T0)


def _datagram(kind, version, t, body):
    sec = int(t)
    nanosec = int(round((t - sec) * 1.0e9))
    size = 20 + len(body) + 4
    return struct.pack("<I4sBBHII", size, kind, version, 0, 124, sec, nanosec) + body + struct.pack("<I", size)


def _cmn_part(ping, fan):
    return struct.pack("<HHBBBBBBBB", 12, ping % 65536, 2, fan, 2, fan, 0, 0, 1, 0)


def svp(t, points=((0.0, SOUNDSPEED), (5000.0, SOUNDSPEED))):
    lat, lon = position(t)
    body = struct.pack("<HH4sIdd", 28, len(points), b"S00 ", int(t), lat, lon)
    for d, c in points:
        body += struct.pack("<ffIff", d, c, 0, 10.0, 35.0)
    return _datagram(b"#SVP", 1, t, body)


def spo(t):
    lat, lon = position(t)
    body = struct.pack("<HHHH", 8, 0, 0, 0)
    body += struct.pack("<IIfddfff", int(t), int(round((t - int(t)) * 1e9)), 0.1, lat, lon,
                        1.0e-5 * 111320.0 * math.sqrt(1 + math.cos(math.radians(lat)) ** 2), 45.0, 0.0)
    body += b"$GPGGA synthetic\r\n\x00\x00"
    return _datagram(b"#SPO", 0, t, body)


def mrz(ping, fan):
    t = ping_time(ping, fan)
    lat, lon = position(t)
    ping_info = struct.pack(
        "<HHfBBBBBBBBfffffffffffhhBBHIfffHHfHHffffffBBBBddffBBH",
        0, 0, 1.0 / PING_INTERVAL, 0, 3, 0, 0, 0, 0, 0, 0,
        12000.0, 11000.0, 13000.0, 0.01, 0.01, 1000.0, 1.0,
        ANGLE_MAX, ANGLE_MAX, ANGLE_MAX, ANGLE_MAX, 800, 800, 0, 0, 0, 0,
        1.0, 1.0, 0.0, 0, 0, 0.0, 1, 48, HEADING, SOUNDSPEED, DRAFT, 0.0, 0.0, 0.0,
        0, 0, 0, 0, lat, lon, 0.0, 0.0, 0, 0, 0)
    ping_info = struct.pack("<H", len(ping_info)) + ping_info[2:]
    tx_sector = struct.pack("<BBBBfffffffBBHfff", 0, 0, 0, 0, 0.0, 1.0 + fan, 220.0, 0.0, 12000.0, 2000.0,
                            0.01, 0, 0, 0, 0.0, 0.0, 0.01)
    sounding_format = "<HBBBBBBBBHffffffHHffffffffffffffffffHHHH"
    rx_info = struct.pack("<HHHHffffHHHH", 32, NBEAMS, NBEAMS, struct.calcsize(sounding_format), FS, FS,
                          -20.0, -25.0, 0, 0, 0, 4)
    soundings = b""
    for b in range(NBEAMS):
        a = beam_angle(b)
        r = BOTTOM_BELOW_TX / math.cos(math.radians(a))
        dtype = 2 if (fan == 0 and b == REJECTED) else (1 if (fan == 0 and b == EXTRA) else 0)
        soundings += struct.pack(
            sounding_format, b, 0, dtype, 1 + b % 2, 0, 0, 0, 0, 0, 0, 1.0, 0.0, 0.1, 0.1, 0.001, 0.001,
            b, int(detect_samples(b)), a, 1.0, -20.0, -20.0, 0.0, 220.0, 0.0, 0.0, a, 0.0,
            2.0 * r / SOUNDSPEED, 0.0, 0.0, 0.0, DRAFT + BOTTOM_BELOW_TX,
            -BOTTOM_BELOW_TX * math.tan(math.radians(a)), 0.0, 0.0, 0, 0, 0, 0)
    body = struct.pack("<HH", 1, 1) + _cmn_part(ping, fan) + ping_info + tx_sector + rx_info + soundings
    return _datagram(b"#MRZ", 3, t, body)


def mwc(ping, fan):
    t = ping_time(ping, fan)
    body = struct.pack("<HH", 1, 1) + _cmn_part(ping, fan)
    body += struct.pack("<HHHhf", 12, 1, 16, 0, 0.1)
    body += struct.pack("<fffHh", 1.0 + fan, 12000.0, 1.0, 0, 0)
    body += struct.pack("<HHBBBbff", 16, NBEAMS, 16, 0, 30, TVG_OFFSET, FS, SOUNDSPEED)
    for b in range(NBEAMS):
        det = detect_samples(b)
        start = START_SAMPLE if b % 2 else 0
        body += struct.pack("<fHHHHf", beam_angle(b), start, int(det), 0, NSAMPLES, det)
        body += bytes((ampl_raw(ping, fan, b, k) & 0xFF) for k in range(NSAMPLES))
    return _datagram(b"#MWC", 2, t, body)


def write_line(directory):
    """Write the .kmall and .kmwcd into directory; return their paths."""
    t_end = ping_time(PING0 + NPINGS)
    kmall, kmwcd = [svp(T0 - 2.0)], [svp(T0 - 2.0)]
    t = T0 - 1.0
    events = []
    while t < t_end:
        events.append((t, "spo"))
        t += 0.5
    for p in range(PING0, PING0 + NPINGS):
        for fan in (0, 1):
            events.append((ping_time(p, fan), ("ping", p, fan)))
    for t, ev in sorted(events, key=lambda e: e[0]):
        if ev == "spo":
            kmall.append(spo(t))
            kmwcd.append(spo(t))
            continue
        _, p, fan = ev
        kmall.append(mrz(p, fan))
        last = PING0 + NPINGS - 1
        if p > PING0 and not (p == last and fan == 1):
            kmwcd.append(mwc(p, fan))
    paths = (os.path.join(directory, KMALL_NAME), os.path.join(directory, KMWCD_NAME))
    for path, dgms in zip(paths, (kmall, kmwcd)):
        with open(path, "wb") as f:
            f.write(b"".join(dgms))
    return paths
