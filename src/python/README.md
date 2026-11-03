# mbio_py - Python access to MB-System's MBIO library

`mbio_py` reads swath sonar files through MB-System's MBIO library (`libmbio`)
with Python's `ctypes` and `numpy`. Nothing is compiled: the package calls the
`libmbio` of the MB-System installation it belongs to. It reads every format
MBIO reads, and gives access to water column data (`mb_extract_wc()`, at present
for Kongsberg kmall data: *.kmall and *.kmwcd files, format 261).

It is meant for prototypes and small tools in Python. Two examples come with it:
the scripts in `examples/` and the water column viewer `mbpy_wcdview`
(`wcd_viewer/wcd_viewer.py`). The C++/Qt6 program `mbwcdview` is the fast
viewer for the same task.

## Installation

`mbio_py` is installed with MB-System (CMake `make install`):

| installed | |
|---|---|
| `<prefix>/share/mbsystem/python/mbio_py/` | the package |
| `<prefix>/share/mbsystem/python/examples/` | example scripts |
| `<prefix>/share/mbsystem/python/wcd_viewer/` | the water column viewer |
| `<prefix>/bin/mbpy_wcdview` | starts the viewer |

`libmbio` must be built as a shared library (`BUILD_SHARED_LIBS=ON`, the
default). The package finds it in this order:
1. the environment variable `MBSYSTEM_LIBMBIO` (full path of `libmbio.dylib`,
   `libmbio.so` or `libmbio.dll`),
2. the library of the installation, written into `mbio_py/_config.py` at build
   time,
3. the system library search path.

### Requirements

| | needs |
|---|---|
| mbio_py | Python >= 3.11 (tested with 3.14), numpy |
| examples | matplotlib |
| mbpy_wcdview | Python >= 3.11, numpy, matplotlib, PySide6 (Qt6 for Python) |

MacPorts (macOS), for example with Python 3.13:

    sudo port install python313 py313-numpy py313-matplotlib py313-pyside6
    sudo port select --set python3 python313

Any platform, in a virtual environment (Linux distributions often ship numpy and
matplotlib, but not always PySide6):

    python3 -m venv ~/mbpy
    ~/mbpy/bin/pip install numpy matplotlib PySide6
    ~/mbpy/bin/python <prefix>/bin/mbpy_wcdview

Windows: install Python from python.org and use the same `pip install`;
MB-System's `libmbio.dll` must be found (set `MBSYSTEM_LIBMBIO`).

### Using the package in your own scripts

    export PYTHONPATH=<prefix>/share/mbsystem/python      # sh, bash, zsh

or in the script:

    import sys
    sys.path.insert(0, "<prefix>/share/mbsystem/python")
    import mbio_py as mb

## The API

### Reader: any format, record by record

    import mbio_py as mb
    from mbio_py import constants as K

    with mb.Reader("TN136HS.309.snipped.mb21") as r:   # format from the name, or Reader(path, 21)
        for rec in r:
            if rec.kind == K.MB_DATA_DATA:
                print(rec.ping_number, rec.time_d, rec.navlon, rec.navlat,
                      rec.heading, rec.sensordepth, rec.bath.size)

`Record` fields: `kind` (MB_DATA_*), `error` (non-fatal MB_ERROR_*, 0 if none),
`time_d`, `time_i`, `navlon`, `navlat`, `speed`, `heading`, `distance`,
`altitude`, `sensordepth`, and numpy arrays `beamflag`, `bath`,
`bathacrosstrack`, `bathalongtrack`, `amp`; for survey and water column records
`ping_number`; for survey records `detects` (MB_DETECT_*); `water_column`
(a `WaterColumnPing`, or None); for sound velocity profile records
`svp_depth`, `svp_velocity`; for comments `comment`.

Fatal MBIO errors raise `mb.MBIOError` (with MB-System's message); non-fatal
ones are in `rec.error` and reading continues. `mb.get_format(path)` gives the
format of a file name (*.kmwcd: 261).

### WaterColumnPing: one ping of water column

    for rec in mb.Reader("0080_20250618_223434.kmwcd"):
        wc = rec.water_column
        if wc is None:
            continue
        across, down, along = wc.xyz()     # m from the transducer, straight rays
        swaths = wc.split_swaths()         # one WaterColumnPing per swath (dual swath)

Per beam: `swath` (0 = aftmost swath), `angle_across` (degrees from vertical,
positive starboard), `angle_along` (degrees, positive forward),
`sample_interval` (s), `soundspeed` (m/s at the transducer), `start_sample`,
`nsamples`, `detect_sample` (two-way bottom range in samples, 0 = none).
`amplitude` is beams x samples, in dB, NaN beyond a beam's samples; column `k`
of beam `i` is sample number `start_sample[i] + k`. For kmall data the
amplitude is the logged value (0.5 dB steps) minus the gain offset
(TVGoffset_dB), the display value defined by Kongsberg; the lowest logged
value (-128, i.e. -94 dB with a 30 dB offset) is a fill value.
`range_m()`, `detect_range_m()`, `xyz()` and `detect_xyz()` give positions
along straight rays with the sound speed at the transducer.

### WaterColumnFile: all pings of a file in memory

    wcf = mb.WaterColumnFile("0080_20250618_223434.kmwcd", progress=lambda n, f: print(n, f))
    wc = wcf.ping(10)                      # any ping, in any order
    print(len(wcf), wcf.fill_value, wcf.amp_min, wcf.amp_max)

Amplitudes are kept as float16 (exact for 0.5 dB steps), about 2 bytes per
sample. Also: the file's own position records (`nav_time_d`, `nav_lon`,
`nav_lat`) and sound velocity profiles (`svps`).

### KongsbergLine: a *.kmall and its *.kmwcd

    line = mb.KongsbergLine("0080_20250618_223434.kmwcd")   # or the .kmall
    wc = line.wc.ping(10)
    j = line.survey_index(10)                 # matching .kmall ping, or None
    soundings = line.survey.bath[j], line.survey.bathacrosstrack[j]
    lon, lat, sog_kn, cog = line.nav_at(wc.time_d)
    heading = line.heading_at(wc.time_d)
    draft = line.draft(10)                    # transducer depth from the .kmall ping
    depth, velocity = line.svp(wc.time_d)

SIS names each file after its first datagram's time, so the two files of a line
may differ by seconds: the companion is found by line number and nearest file
name time (`mb.find_companion`), then pings are matched by ping number and time.
Either file may be missing; `nav_at` falls back to the .kmall ping positions and
then to the .kmwcd position records.

### raytrace: samples through a sound velocity profile

    (across, depth, along), (det_across, det_depth, det_along) = mb.raytrace(wc, depth_svp, velocity_svp, draft)

Each beam is traced from the transducer depth with Snell's law through thin
layers of the profile (ray parameter from the logged sound speed at the
transducer), as the soundings are; depths are below the waterline. Without it,
in deep water with a fast surface layer the seafloor echo appears about 1 %
deeper than the soundings and the outer beams tens of metres off.

### Conventions
As in MBIO: `time_d` seconds since 1970-01-01 UTC; positions in degrees;
`speed` km/h; `heading` degrees; depths positive down; `bath` below the
waterline; `bathacrosstrack` from the sonar, positive starboard;
`sensordepth` transducer depth below the waterline. Beam flags with
`constants.beam_ok`, `beam_null`, `beam_flagged`, `beam_flagged_sonar`. For
kmall data rejected detections are null beams (no position) and extra
detections are flagged by the sonar.

## Examples (`examples/`)

| script | |
|---|---|
| `print_records.py file [format]` | record kinds and errors, first record of each kind |
| `print_wc_ping.py file.kmwcd [i]` | one water column ping in numbers |
| `plot_wc_ping.py file.kmwcd [i] [out.png]` | water column image of each swath with its bottom detections |
| `plot_nav.py file.kmall [out.png]` | track, speed and heading |
| `plot_bottom.py file.kmall [i] [out.png]` | soundings of one ping by detection method and flag |
| `plot_paired.py file [i] [out.png]` | water column with the soundings of the matching .kmall ping |

Run them with the installed copy, for example
`python3 <prefix>/share/mbsystem/python/examples/plot_paired.py 0030_20250208_100905.kmall 100 ping100.png`.

## The water column viewer: mbpy_wcdview

    mbpy_wcdview 0080_20250618_223434.kmwcd

Opens a *.kmwcd or *.kmall (or use the Open button) and finds the other file of
the line. Controls: depth and amplitude ranges, fixed across track scale,
`Bottom` (soundings of the matching .kmall ping: red amplitude, dark blue phase,
filled good, hollow flagged), `WC bottom` (the water column's own bottom
detections), `Ray trace (SVP)` (on when a profile is found), a slider stepping
one swath at a time (also mouse wheel and arrow keys), replay, `Pick` (positions
into a table saved as text), `Save Image` (twice the screen resolution, with the
file name and ping information). Without a .kmall: no soundings, depth below the
transducer, navigation from the .kmwcd position records, picks use the course
over ground. See also `man mbpy_wcdview`.

## Tests
`test/python` in the source tree (`ctest -R "mbio_py|wcdview"`), run with the
library of the build; they write a small synthetic .kmall/.kmwcd pair and need
python3 with numpy (PySide6 and matplotlib for the viewer test).
