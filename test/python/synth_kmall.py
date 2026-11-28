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
  raw values ampl_raw(), one single raw value RARE_RAW (the lowest non-fill value);
  in the first water column ping, sample INNER_FILL of beam 1 of both fans is -128 too
  (a weak echo clipped to the lowest level)
- soundings: method amplitude in even beams, phase in odd beams; beam REJECTED of
  fan 0 rejected (detectionType 2), beam EXTRA of fan 0 an extra detection (1)
- position records every 0.5 s in both files, moving north-east

Options of write_line(): the transducer depth (draft) and its change from ping to ping
(draft_step, m per ping), pings whose #MRZ datagrams are left out of the .kmall (skip_mrz),
an extra detection (numExtraDetections 1) after the soundings of fan extra_fan of every ping,
half way to the seafloor in the direction of beam EXTRA_BEAM (extra), a sound speed falling
by svp_gradient m/s in the 150 m below the transducer in the #SVP datagrams (the soundings
stay at their straight ray positions), and soundings placed at sounding_scale times their
straight ray across track distance on the seafloor, as a sonar's ray tracing moves them
toward nadir (the travel times and the water column stay those of straight rays), water
column detections in whole samples (whole_samples), as *.all and *.wcd files give them, and
an offset added to the soundings' beam angles re the receive array (rx_angle_offset, deg), so
that the beam angles MBIO computes for the soundings differ from those of the water column.
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
INNER_FILL = 12
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
EXTRA_BEAM = 8            # beam of the extra detection of write_line(extra=True)
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
    if ping == PING0 + 1 and b == 1 and k == INNER_FILL:
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


def svp(t, draft=DRAFT, gradient=0.0):
    points = [(0.0, SOUNDSPEED), (5000.0, SOUNDSPEED)]
    if gradient:
        points = [(0.0, SOUNDSPEED), (draft, SOUNDSPEED), (draft + 150.0, SOUNDSPEED - gradient),
                  (draft + 5000.0, SOUNDSPEED - gradient)]
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


def ping_draft(ping, draft=DRAFT, draft_step=0.0):
    """Transducer depth of a ping."""
    return draft + draft_step * (ping - PING0)


def mrz(ping, fan, draft=DRAFT, extra=False, extra_fan=1, sounding_scale=1.0, rx_angle_offset=0.0):
    t = ping_time(ping, fan)
    lat, lon = position(t)
    ping_info = struct.pack(
        "<HHfBBBBBBBBfffffffffffhhBBHIfffHHfHHffffffBBBBddffBBH",
        0, 0, 1.0 / PING_INTERVAL, 0, 3, 0, 0, 0, 0, 0, 0,
        12000.0, 11000.0, 13000.0, 0.01, 0.01, 1000.0, 1.0,
        ANGLE_MAX, ANGLE_MAX, ANGLE_MAX, ANGLE_MAX, 800, 800, 0, 0, 0, 0,
        1.0, 1.0, 0.0, 0, 0, 0.0, 1, 48, HEADING, SOUNDSPEED, draft, 0.0, 0.0, 0.0,
        0, 0, 0, 0, lat, lon, 0.0, 0.0, 0, 0, 0)
    ping_info = struct.pack("<H", len(ping_info)) + ping_info[2:]
    tx_sector = struct.pack("<BBBBfffffffBBHfff", 0, 0, 0, 0, 0.0, 1.0 + fan, 220.0, 0.0, 12000.0, 2000.0,
                            0.01, 0, 0, 0, 0.0, 0.0, 0.01)
    sounding_format = "<HBBBBBBBBHffffffHHffffffffffffffffffHHHH"
    nextra = 1 if (extra and fan == extra_fan) else 0
    rx_info = struct.pack("<HHHHffffHHHH", 32, NBEAMS, NBEAMS, struct.calcsize(sounding_format), FS, FS,
                          -20.0, -25.0, 0, nextra, 0, 4)
    soundings = b""
    # (beam, fraction of the range to the seafloor, detectionType of an extra detection)
    entries = [(b, 1.0, None) for b in range(NBEAMS)] + [(EXTRA_BEAM, 0.5, 1)] * nextra
    for b, f, extra_type in entries:
        a = beam_angle(b)
        r = f * BOTTOM_BELOW_TX / math.cos(math.radians(a))
        dtype = 2 if (fan == 0 and b == REJECTED) else (1 if (fan == 0 and b == EXTRA) else 0)
        if extra_type is not None:
            dtype = extra_type
        soundings += struct.pack(
            sounding_format, b, 0, dtype, 1 + b % 2, 0, 0, 0, 0, 0, 0, 1.0, 0.0, 0.1, 0.1, 0.001, 0.001,
            b, int(f * detect_samples(b)), a, 1.0, -20.0, -20.0, 0.0, 220.0, 0.0, 0.0, a + rx_angle_offset, 0.0,
            2.0 * r / SOUNDSPEED, 0.0, 0.0, 0.0, draft + f * BOTTOM_BELOW_TX,
            -sounding_scale * f * BOTTOM_BELOW_TX * math.tan(math.radians(a)), 0.0, 0.0, 0, 0, 0, 0)
    body = struct.pack("<HH", 1, 1) + _cmn_part(ping, fan) + ping_info + tx_sector + rx_info + soundings
    return _datagram(b"#MRZ", 3, t, body)


def mwc(ping, fan, whole_samples=False):
    t = ping_time(ping, fan)
    body = struct.pack("<HH", 1, 1) + _cmn_part(ping, fan)
    body += struct.pack("<HHHhf", 12, 1, 16, 0, 0.1)
    body += struct.pack("<fffHh", 1.0 + fan, 12000.0, 1.0, 0, 0)
    body += struct.pack("<HHBBBbff", 16, NBEAMS, 16, 0, 30, TVG_OFFSET, FS, SOUNDSPEED)
    for b in range(NBEAMS):
        det = detect_samples(b)
        start = START_SAMPLE if b % 2 else 0
        body += struct.pack("<fHHHHf", beam_angle(b), start, int(det), 0, NSAMPLES,
                            float(int(det)) if whole_samples else det)
        body += bytes((ampl_raw(ping, fan, b, k) & 0xFF) for k in range(NSAMPLES))
    return _datagram(b"#MWC", 2, t, body)


def write_line(directory, draft=DRAFT, skip_mrz=(), extra=False, svp_gradient=0.0, extra_fan=1, sounding_scale=1.0,
               draft_step=0.0, whole_samples=False, rx_angle_offset=0.0):
    """Write the .kmall and .kmwcd into directory; return their paths (options: see above)."""
    t_end = ping_time(PING0 + NPINGS)
    kmall, kmwcd = [svp(T0 - 2.0, draft, svp_gradient)], [svp(T0 - 2.0, draft, svp_gradient)]
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
        if p not in skip_mrz:
            kmall.append(mrz(p, fan, ping_draft(p, draft, draft_step), extra, extra_fan, sounding_scale,
                             rx_angle_offset))
        last = PING0 + NPINGS - 1
        if p > PING0 and not (p == last and fan == 1):
            kmwcd.append(mwc(p, fan, whole_samples))
    paths = (os.path.join(directory, KMALL_NAME), os.path.join(directory, KMWCD_NAME))
    for path, dgms in zip(paths, (kmall, kmwcd)):
        with open(path, "wb") as f:
            f.write(b"".join(dgms))
    return paths
