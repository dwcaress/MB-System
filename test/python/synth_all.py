#!/usr/bin/env python3
#--------------------------------------------------------------------
#    The MB-system:  synth_all.py  1/11/2026
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
"""A synthetic Kongsberg *.wcd file (water column datagram k 0x6B, position datagram P 0x50)
with a known content, read by the MBIO formats 58 and 59 tests.

Datagram layouts from the Kongsberg EM datagram format (850-160692/W). The pings of the file:

- PINGS[0], PINGS[1]: the ping numbers and times of the two survey pings of the repository
  sample SURVEY_SAMPLE, so that the two files pair. Ping 0 has one datagram; ping 1 has three,
  the second 1 ms later, with a position datagram between the first two
- ping 2: three datagrams, the middle one lost (gap in the swath)
- ping 3: three datagrams, the last one lost: ends when the k of ping 4 arrives
- ping 4: start sample START_SAMPLE in the beams, two datagrams
- ping 5: two datagrams, the file ends inside the second one, in a beam
Each datagram holds BEAMS_PER_DATAGRAM beams and each ping has the same three transmit sectors.
"""
import struct

SURVEY_SAMPLE = "mb58/0008_20111219_210356_METEOR_EM122.snipped.all"
WCD_NAME = "0008_20111219_210356_METEOR_EM122.snipped.wcd"
ALL_NAME = "0008_20111219_210356_METEOR_EM122.snipped.all"

MODEL = 122
SERIAL = 101
DATE = 20111219
MSEC0 = 21 * 3600000 + 3 * 60000 + 56960   # time of the first survey ping of the sample
PINGS = [18440, 18441, 18442, 18443, 18444, 18445]
MSECS = [MSEC0, MSEC0 + 209, MSEC0 + 400, MSEC0 + 600, MSEC0 + 800, MSEC0 + 1000]
DATAGRAMS = {0: 1, 1: 3, 2: 3, 3: 3, 4: 2, 5: 2}   # number of datagrams of each ping
LOST = {(2, 2), (3, 3)}                           # (ping index, datagram number) not in the file
CUT_PING = 5                                      # the file ends inside datagram 2 of this ping
BEAMS_PER_DATAGRAM = 4
NSAMPLES = 9                                      # odd, so that the datagrams need the spare byte
START_SAMPLE = 40000                              # ping 4; above the range of a signed short
SOUNDSPEED = 15012                                # 0.1 m/s
SAMPLE_FREQ = 1234500                             # 0.01 Hz
TVG_OFFSET = 20                                   # dB
SECTORS = ((-150, 0), (0, 1), (250, 2))           # tilt (0.01 deg), sector
LAT, LON = 54.5, 8.25


def nbeams_read(i):
    """Beams of ping index i in the file, whole beams only."""
    n = sum(BEAMS_PER_DATAGRAM for d in range(1, DATAGRAMS[i] + 1) if (i, d) not in LOST)
    return n - 1 if i == CUT_PING else n


def beam_angle(d, b):
    """Kongsberg beam pointing angle (0.01 deg, positive port) of beam b of datagram d."""
    return 6000 - 1000 * (d - 1) - 200 * b


def sector(d, b):
    return (d + b) % 3


def raw(i, d, b, k):
    """Raw amplitude (0.5 dB) of sample k."""
    return -128 if k == 0 else ((i * 7 + d * 5 + b * 3 + k) % 60) - 60


def detected(d, b):
    return 0 if b == 1 else 3 + b


def time_i(i):
    m = MSECS[i]
    return (2011, 12, 19, m // 3600000, m % 3600000 // 60000, m % 60000 // 1000, m % 1000 * 1000)


def _datagram(e, stx_type, body):
    data = struct.pack(e + "BH", stx_type, MODEL) + body
    check = sum(data) & 0xFFFF
    payload = b"\x02" + data + b"\x03" + struct.pack(e + "H", check)
    return struct.pack(e + "I", len(payload)) + payload


def water_column(e, i, d, msec_add=0, nbeam=None):
    """Datagram d (from 1) of ping index i."""
    start = START_SAMPLE if i == 4 else 0
    head = struct.pack(e + "IIHHHHHHHHIhBbB3s", DATE, MSECS[i] + msec_add, PINGS[i], SERIAL, DATAGRAMS[i], d, len(SECTORS),
                       DATAGRAMS[i] * BEAMS_PER_DATAGRAM, BEAMS_PER_DATAGRAM if nbeam is None else nbeam, SOUNDSPEED,
                       SAMPLE_FREQ, -12, 30, TVG_OFFSET, 0, b"\0\0\0")
    body = head
    for tilt, sec in SECTORS:
        body += struct.pack(e + "hHBB", tilt, 30000 + sec, sec, 0)   # centre frequency above 327 kHz (10 Hz)
    for b in range(BEAMS_PER_DATAGRAM):
        body += struct.pack(e + "hHHHBB", beam_angle(d, b), start, NSAMPLES, detected(d, b), sector(d, b), 255)
        body += struct.pack("%db" % NSAMPLES, *[raw(i, d, b, k) for k in range(NSAMPLES)])
    if (len(body) + 7) % 2:
        body += b"\0"
    return _datagram(e, 0x6B, body)


def position(e, msec):
    fix = b"$INGGA,fix"
    body = struct.pack(e + "IIHHiiHHHHBB", DATE, msec, 1, SERIAL, int(LAT * 20000000), int(LON * 10000000), 100, 500, 4500, 4500,
                       0x81, len(fix)) + fix
    if (len(body) + 7) % 2:
        body += b"\0"
    return _datagram(e, 0x50, body)


def contents(big_endian=False, bad_datagram=False):
    """The bytes of the file; bad_datagram adds an unintelligible k before ping 2."""
    e = ">" if big_endian else "<"
    out = b""
    for i in range(len(PINGS)):
        for d in range(1, DATAGRAMS[i] + 1):
            if (i, d) in LOST:
                continue
            if i == 2 and d == 1 and bad_datagram:
                out += water_column(e, i, d, nbeam=2000)
            dg = water_column(e, i, d, msec_add=1 if i == 1 and d == 2 else 0)
            if i == CUT_PING and d == DATAGRAMS[i]:
                # end the file in the last beam of the datagram
                dg = dg[:-(NSAMPLES // 2 + 4)]
            out += dg
            if i == 1 and d == 1:
                out += position(e, MSECS[1] + 5)
    return out


def write(path, big_endian=False, bad_datagram=False):
    with open(path, "wb") as f:
        f.write(contents(big_endian, bad_datagram))
    return path


def sample_interval():
    return 1.0 / (0.01 * SAMPLE_FREQ)


def tilt(sec):
    return 0.01 * dict((s, t) for t, s in SECTORS)[sec]


def amplitude(i, d, b):
    return [0.5 * raw(i, d, b, k) - TVG_OFFSET for k in range(NSAMPLES)]


def angle_across(d, b):
    return -0.01 * beam_angle(d, b)



def walk(path):
    """Strict walk of a file by the datagram size fields: list of (type, bytes after the size field)."""
    with open(path, "rb") as f:
        data = f.read()
    records, pos = [], 0
    while pos < len(data):
        size = struct.unpack_from("<I", data, pos)[0]
        record = data[pos + 4:pos + 4 + size]
        if len(record) != size or size % 2 or record[0] != 2 or record[-3] != 3:
            raise ValueError("bad datagram at byte %d, size %d" % (pos, size))
        if sum(record[1:-3]) & 0xFFFF != struct.unpack("<H", record[-2:])[0]:
            raise ValueError("bad checksum at byte %d" % pos)
        records.append((record[1], record))
        pos += 4 + size
    return records


def walk_until_cut(path):
    """As walk(), stopping at the datagram cut by the end of the file."""
    with open(path, "rb") as f:
        data = f.read()
    records, pos = [], 0
    while pos + 4 < len(data):
        size = struct.unpack_from("<I", data, pos)[0]
        if pos + 4 + size > len(data):
            break
        records.append((data[pos + 5], data[pos + 4:pos + 4 + size]))
        pos += 4 + size
    return records
