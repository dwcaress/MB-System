#--------------------------------------------------------------------
#    The MB-system:  synth_s7k.py  19/11/2026
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
"""Write a small synthetic Reson 7k survey line (*.s7k, format 89) for the tests.

Record layouts from the Teledyne 7k Data Format Definition 3.12: data record frame
protocol version 5, 7000 Sonar Settings, 7004 Beam Geometry, 7027 Raw Detection, 7042
Compressed Water Column, 1003 Position, 1008 Depth, 1012 Roll Pitch Heave, 1013 Heading.
The scene is chosen so that every value read back through MBIO can be computed exactly:

- NBEAMS beams from -ANGLE_MAX (port) to +ANGLE_MAX (starboard; 7k angles are positive
  to starboard) relative to the sonar, which is rolled by ROLL degrees, with the roll
  compensation flag set as a T50 logs it
- flat seafloor BOTTOM_BELOW_TX m below the transducer, the transducer SENSORDEPTH m deep,
  constant sound speed SOUNDSPEED, no pitch or heave
- one detection per beam in 7027, amplitude detections in even beams and phase detections
  in odd beams; ping 0 has multi-detection on and a second, lower priority detection of
  beam MULTI_BEAM logged before the primary one
- each ping has its own kind of 7042 record (PINGS): 8 bit dB values with the default
  factor 40 and the downsampling divisor bits set without a downsampling type (as a T50
  logs them), 8 bit dB values with a compression factor and phase, 16 bit magnitudes
  and phase, 32 bit magnitudes limited at the bottom with segment numbers and a first
  sample > 0, downsampled 8 bit values without the record's sample rate, no 7042, a
  7042 whose beam claims more samples than the record holds, a 7042 with too many
  beams, and transmit steering with the roll compensation flag off
- position, attitude, heading and depth records every 0.5 s, moving north-east
- no file header (7200) and no file catalog (7300): MBIO reads the records in order

write_cases() writes a second line of the same scene with the reader cases (CASES): pings
without a 7000 record, pings without a 7027 record (dropped by the reader), broken 7042
records after such a ping or after a good 7042 record of the same ping, a ping whose
detections are a 7047 Segmented Raw Detection record (one segment, no optional data), and
a 7000 record without a sound speed after a 7610 Sound Velocity record.
"""
import datetime
import math
import os
import struct

NBEAMS = 32
ANGLE_MAX = 64.0          # deg
SENSORDEPTH = 500.0       # m, depth record (1008, depth to sensor): a sonar on a vehicle
BOTTOM_BELOW_TX = 100.0   # m
SOUNDSPEED = 1500.0       # m/s
FS = 1000.0               # Hz, sonar sample rate (7000, 7027)
NSAMPLES = 320            # samples per beam (nominal range 240 m)
T0 = 1735732800.0         # 2025-01-01 12:00:00 UTC
PING0 = 2000
PING_INTERVAL = 1.0       # s
PING_OFFSET = 0.25        # s, ping times between the sensor records
HEADING = 45.0            # deg
LAT0, LON0 = 54.0, 8.0
DEVICE = 50               # T50
MULTI_BEAM = 1            # beam with a second detection in ping 0 (early in the fan: paired with
                          # the beams by index, most soundings would be a neighbour's)
ROLL = 2.0                # deg, roll of the sonar (1012 records)
TX_STEER = 0.02           # rad, transmit steering of the steered ping
SEG_STEER = 0.03          # rad, along track transmit steering of the 7047 segment
S7K_NAME = "20250101_120000.s7k"

# the 7042 record of each ping: flags, sample rate written in the record (0: none), compression
# factor, first sample
PINGS = [
    dict(name="dB8", flags=0x0026, rate=FS, factor=0.0, first=0),
    dict(name="dB8 factor phase", flags=0x2004, rate=FS, factor=30.0, first=0),
    dict(name="mag16 phase", flags=0x0000, rate=FS, factor=0.0, first=0),
    dict(name="mag32 bottom segments", flags=0x5003, rate=FS, factor=0.0, first=10),
    dict(name="downsampled", flags=0x0346, rate=0.0, factor=0.0, first=5),
    dict(name="no 7042"),
    dict(name="samples beyond record", flags=0x0026, rate=FS, factor=0.0, first=0),
    dict(name="too many beams", flags=0x0026, rate=FS, factor=0.0, first=0),
    dict(name="steered", flags=0x0026, rate=FS, factor=0.0, first=0),
]
NPINGS = len(PINGS)
NO_WC = (5, 6, 7)           # pings without a usable water column
BAD_RECORDS = (6, 7)        # pings with a 7042 record MBIO rejects
DOWNSAMPLING = 4            # divisor of the downsampled ping (flags bits 4-7 = 4)
STEER_PING = 8

# the reader cases (write_cases): sample rate of the 7000 record (None: no 7000), 7027 record
# written (or s7047: a 7047 record instead), 7042 records, and the sample rate of the water
# column MBIO returns (None: no water column; "dropped": the reader drops the ping, which has
# no detections and keeps its 7000 record in the store); sv7000: the sound speed of the 7000
# record, sv7610: a 7610 record before the ping
CASE0 = NPINGS + 1          # ping index of the first case, after the line's pings
BROKEN32 = dict(name="samples beyond record", flags=0x1000, rate=FS, factor=0.0, first=0)
CASES = [
    dict(name="no 7000 yet", s7000=None, s7027=True, wc=[PINGS[4]], rate=FS / DOWNSAMPLING),
    dict(name="complete", s7000=FS, s7027=True, wc=[PINGS[0]], rate=FS),
    dict(name="no 7027", s7000=FS, s7027=False, wc=[PINGS[0]], rate="dropped"),
    dict(name="broken 7042 after a dropped ping", s7000=FS, s7027=True, wc=[BROKEN32], rate=None),
    dict(name="good 7042 then a broken one", s7000=FS, s7027=True, wc=[PINGS[0], BROKEN32], rate=None),
    dict(name="no 7027 again", s7000=FS, s7027=False, wc=[PINGS[0]], rate="dropped"),
    dict(name="too many beams after a dropped ping", s7000=2.0 * FS, s7027=True, wc=[PINGS[7]], rate=None),
    dict(name="no 7000, the last one of another rate", s7000=None, s7027=True, wc=[PINGS[4]],
         rate=FS / DOWNSAMPLING),
    dict(name="no 7027, a 7000 of another rate", s7000=2.0 * FS, sv7000=1520.0, s7027=False, wc=[PINGS[0]],
         rate="dropped"),
    dict(name="no 7000 after a dropped ping with one", s7000=None, s7027=True, wc=[PINGS[4]],
         rate=FS / DOWNSAMPLING),
    dict(name="segmented detections, no 7000", s7000=None, s7027=False, s7047=True, wc=[PINGS[4]],
         rate=FS / DOWNSAMPLING),
    dict(name="no sound speed in 7000, a 7610 before it", s7000=FS, sv7000=0.0, sv7610=1480.0, s7027=True,
         wc=[PINGS[0]], rate=FS),
    dict(name="complete again", s7000=FS, s7027=True, wc=[PINGS[0]], rate=FS),
]
CASES_NAME = "20250101_121000.s7k"


def beam_angle(b):
    """7k beam steering angle (deg, positive starboard) of beam b, relative to the sonar."""
    return -ANGLE_MAX + 2.0 * ANGLE_MAX * b / (NBEAMS - 1)


def vertical_angle(b):
    """Angle of beam b from the vertical (deg, positive starboard), as MBIO turns it with the roll."""
    return beam_angle(b) - ROLL


def detect_samples(b):
    """Two-way range in sonar samples (FS) of the detection of beam b."""
    r = BOTTOM_BELOW_TX / math.cos(math.radians(vertical_angle(b)))
    return 2.0 * r / SOUNDSPEED * FS


def ping_time(p):
    return T0 + p * PING_INTERVAL + PING_OFFSET


def position(t):
    """(lat, lon) of the vessel at time t, degrees."""
    return LAT0 + 1.0e-5 * (t - T0), LON0 + 1.0e-5 * (t - T0)


def spec_of(p):
    """The (first) 7042 record of ping p: PINGS for the line, CASES for the reader cases."""
    return CASES[p - CASE0]["wc"][0] if p >= CASE0 else PINGS[p]


def nsamples(p, b, spec=None):
    """Number of samples of beam b in the 7042 record of ping p."""
    spec = spec or spec_of(p)
    if spec["flags"] & 0x0001:  # limited at the bottom detection + 10 %
        return min(NSAMPLES, int(1.1 * detect_samples(b))) - spec["first"]
    if (spec["flags"] >> 8) & 0x0F:  # downsampled
        return NSAMPLES // DOWNSAMPLING
    return NSAMPLES


def raw_value(p, b, k, spec=None):
    """Stored magnitude (8 bit dB value, or 16 or 32 bit magnitude) of sample k of beam b."""
    spec = spec or spec_of(p)
    if spec["flags"] & 0x1000:
        return (b * 100003 + k * 7919) % 2000000
    if spec["flags"] & 0x0004:
        return (b * 7 + k * 3 + p) % 140
    return (b * 131 + k * 17) % 3000


def amplitude_db(p, b, k):
    """Amplitude (dB) MBIO returns for sample k of beam b of ping p."""
    spec = spec_of(p)
    v = raw_value(p, b, k)
    if spec["flags"] & 0x0004:
        factor = spec["factor"] if (spec["flags"] & 0x2000) else 40.0
        return 20.0 / factor * v
    return 20.0 * math.log10(max(v, 1))


def _record(rtype, t, body, device=DEVICE):
    """One 7k record: data record frame (protocol version 5), body, checksum."""
    dt = datetime.datetime.fromtimestamp(t, datetime.timezone.utc)
    seconds = dt.second + 1.0e-6 * dt.microsecond
    size = 64 + len(body) + 4
    frame = struct.pack("<HHIIII", 5, 60, 0x0000FFFF, size, 0, 0)
    frame += struct.pack("<HHfBB", dt.year, dt.timetuple().tm_yday, seconds, dt.hour, dt.minute)
    frame += struct.pack("<HIIHHIHHIII", 1, rtype, device, 0, 0, 0, 1, 0, 0, 0, 0)
    data = frame + body
    return data + struct.pack("<I", sum(data) & 0xFFFFFFFF)


def sonar_settings(p, rate=FS, sv=SOUNDSPEED):
    rx_flags = 0 if p == STEER_PING else 1  # bit 0: roll compensation
    steer = TX_STEER if p == STEER_PING else 0.0
    body = struct.pack("<QIHffffIIfHHfffffI", 0, PING0 + p, 0, 400000.0, rate, 0.0, 0.001, 1, 0, 0.0, 1, 0, 3.0,
                       PING_INTERVAL, 240.0, 218.0, 34.0, 0)
    body += struct.pack("<IfffffIfIIIfIffffffff", 0, steer, 0.0, 0.0175, 2.0, 0.0, 0, 0.0, 0, 0, 0, 0.0, rx_flags,
                        0.0096, 0.0, 0.0, 0.0, 0.0, 87.0, sv, 30.0)
    body += struct.pack("<H", 0)
    return _record(7000, ping_time(p), body)


def beam_geometry(p):
    angles = [math.radians(beam_angle(b)) for b in range(NBEAMS)]
    body = struct.pack("<QI", 0, NBEAMS)
    body += struct.pack("<%df" % NBEAMS, *([0.0] * NBEAMS))
    body += struct.pack("<%df" % NBEAMS, *angles)
    body += struct.pack("<%df" % NBEAMS, *([0.4712] * NBEAMS))
    body += struct.pack("<%df" % NBEAMS, *([0.0096] * NBEAMS))
    return _record(7004, ping_time(p), body)


def raw_detection(p):
    multi = p == 0
    steer = TX_STEER if p == STEER_PING else 0.0
    detections = []
    for b in range(NBEAMS):
        dflags = (1 if b % 2 == 0 else 2) | (1 << 2)  # amplitude or phase detection, quality type 1
        if multi and b == MULTI_BEAM:  # a second detection with priority 1 first
            detections.append(struct.pack("<HffIIffff", b, 0.8 * detect_samples(b), math.radians(beam_angle(b)),
                                          dflags | (1 << 9), 0, 0.01, 100.0, 0.0, NSAMPLES))
        detections.append(struct.pack("<HffIIffff", b, detect_samples(b), math.radians(beam_angle(b)), dflags, 3,
                                      0.01, 100.0, 0.0, NSAMPLES))
    body = struct.pack("<QIHIIBIfff", 0, PING0 + p, 0, len(detections), 34, 3, 0x10 if multi else 0, FS, steer, 0.0)
    body += bytes(60)
    return _record(7027, ping_time(p), body + b"".join(detections))


def segmented_raw_detection(p, rate=FS):
    """7047 with one segment of sample rate rate and along track steering SEG_STEER and one
    detection per beam, no optional data."""
    header = struct.pack("<HIHIHQIHff", 36, 1, 68, NBEAMS, 32, 0, PING0 + p, 0, SOUNDSPEED, 0.0)
    segment = struct.pack("<HffffIffffIfffBIfBf", 1, SEG_STEER, 0.0, 0.0, 400000.0, 0, 0.0, 0.001, 0.0175, 0.0175, 0,
                          0.0, 0.0, 0.0096, 0, 0, rate, 0, 0.0)
    detections = b"".join(struct.pack("<HHffIIfff", b, 1, detect_samples(b) * rate / FS,
                                      math.radians(beam_angle(b)), 0, 3, 0.01, 100.0, 0.0) for b in range(NBEAMS))
    return _record(7047, ping_time(p), header + segment + detections)


def compressed_water_column(p, spec=None):
    spec = spec or spec_of(p)
    flags = spec["flags"]
    if flags & 0x1000:
        magsize, phasesize = 4, (0 if flags & 0x0002 else 1)
    else:
        magsize = 1 if flags & 0x0004 else 2
        phasesize = 0 if flags & 0x0002 else magsize
    mag_format = {1: "<B", 2: "<H", 4: "<I"}[magsize]
    nbeams_written = NBEAMS
    nbeams_header = 65535 if spec["name"] == "too many beams" else NBEAMS
    if spec["name"] == "too many beams":
        nbeams_written = 2
    beams = b""
    for b in range(nbeams_written):
        n = nsamples(p, b, spec)
        beams += struct.pack("<H", b)
        if flags & 0x4000:
            beams += struct.pack("<B", b % 3)
        n_header = 1000000 if (spec["name"] == "samples beyond record" and b == 1) else n
        beams += struct.pack("<I", n_header)
        for k in range(n):
            beams += struct.pack(mag_format, raw_value(p, b, k, spec)) + bytes(phasesize)
    nmax = max(nsamples(p, b, spec) for b in range(NBEAMS))
    header = struct.pack("<QIHHIIIIffI", 0, PING0 + p, 0, nbeams_header, NSAMPLES, nmax, flags, spec["first"],
                         spec["rate"], spec["factor"], 0)
    return _record(7042, ping_time(p), header + beams)


def sound_velocity(t, sv):
    """7610 Sound Velocity record."""
    return _record(7610, t, struct.pack("<f", sv), device=0)


def sensor_records(t):
    lat, lon = position(t)
    out = _record(1003, t, struct.pack("<Ifddd5B", 0, 0.0, math.radians(lat), math.radians(lon), 0.0, 0, 0, 0, 0, 12),
                  device=0)
    out += _record(1012, t, struct.pack("<fff", math.radians(ROLL), 0.0, 0.0), device=0)
    out += _record(1013, t, struct.pack("<f", math.radians(HEADING)), device=0)
    out += _record(1008, t, struct.pack("<BBHf", 0, 0, 0, SENSORDEPTH), device=0)
    return out


def write(directory):
    """Write the *.s7k into directory; return its path."""
    events = []
    t = T0 - 1.0
    while t < ping_time(NPINGS) + 1.0:
        events.append((t, "sensors", None))
        t += 0.5
    for p in range(NPINGS):
        events.append((ping_time(p), "ping", p))
    out = b""
    for t, kind, p in sorted(events, key=lambda e: e[0]):
        if kind == "sensors":
            out += sensor_records(t)
            continue
        out += sonar_settings(p) + beam_geometry(p) + raw_detection(p)
        if PINGS[p]["name"] != "no 7042":
            out += compressed_water_column(p)
    path = os.path.join(directory, S7K_NAME)
    with open(path, "wb") as f:
        f.write(out)
    return path


def write_cases(directory):
    """Write the reader cases (CASES) as a *.s7k into directory; return its path."""
    events = []
    t = T0 + CASE0 - 1.0
    while t < ping_time(CASE0 + len(CASES)) + 1.0:
        events.append((t, "sensors", None))
        t += 0.5
    for i in range(len(CASES)):
        events.append((ping_time(CASE0 + i), "ping", i))
    out = b""
    for t, kind, i in sorted(events, key=lambda e: e[0]):
        if kind == "sensors":
            out += sensor_records(t)
            continue
        case, p = CASES[i], CASE0 + i
        if "sv7610" in case:
            out += sound_velocity(ping_time(p), case["sv7610"])
        if case["s7000"] is not None:
            out += sonar_settings(p, case["s7000"], case.get("sv7000", SOUNDSPEED))
        out += beam_geometry(p)
        if case["s7027"]:
            out += raw_detection(p)
        if case.get("s7047"):
            out += segmented_raw_detection(p)
        for spec in case["wc"]:
            out += compressed_water_column(p, spec)
    path = os.path.join(directory, CASES_NAME)
    with open(path, "wb") as f:
        f.write(out)
    return path
