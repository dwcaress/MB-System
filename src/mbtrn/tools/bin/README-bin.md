# mbtrn Scripts

mbtrn/tools/bin contains scripts related to running mbtrnpp

## mbtrnpp.sh

mbtrnpp.sh is a (bash) shell wrapper script for the mbtrnpp executable.

While not strictly required, using the wrapper script ensures that mbtrnpp is restarted if it exits prematurely, providing an opportunity for error recovery.

### Features

* Auto-restart when mbtrnpp exits
* Environment file and overrides on command line
* Test option for evaluating and generating command lines
* Optional restart delay and cycle limit prevents run-away log generation
* Optional console output log provides useful diagnostic data source if failures occur

### Use Notes

```
 Description: Run mbtrnpp; restart on exit

 use: mbtrnpp.sh [options] [-- --option=value...]
 Options:
  -a cmd  : app command            [/Volumes/linux-share/git/mbsys-trn/MB-System/src/mbtrnutils/mbtrnpp]
  -c n    : cycles (<=0 forever)   [-1]
  -d path : enable console log, set directory
  -e path : environment file
  -h      : print use message
  -G addr : TRN UDP multicast group [239.255.0.16]
  -D path : TRN config directory    [/Volumes/wdcs/20210617m2/config]
  -L path : TRN log directory       [/Volumes/wdcs/20210617m2/logs/mbtrnpp]
  -M path : TRN maps directory      [/Volumes/wdcs/20210617m2/maps]
  -m path : override mbtrnpp dir    [/Volumes/linux-share/git/mbsys-trn/MB-System/src/mbtrnutils]
  -o addr : TRN host                [134.89.13.166]
             affects : [--trn-out, --mb-out]
  -r addr : TRN reson host          [134.89.32.107]
             affects: [--input]
  -t      : test [print cmdline]
  -v      : verbose output         [N]
  -w n    : loop delay (s)         [5]

 Example:
 # typical use [Note '--' marking start of mbtrnpp options]
  mbtrnpp.sh -e /path/to/environment/file -- --config=/path/to/config/file

 # log console output [-d <dir> creates <dir>/mbtrnpp-console-<session>.log]
  mbtrnpp.sh -e /path/to/environment/file -d /path/to/console/output/dir -- --config=/path/to/config/file

 # test: show environment and mbtrnpp command line exit
  mbtrnpp.sh -e /path/to/environment/file -t -- --config=/path/to/config/file [options...]

 # test: show all mbtrnpp parsed command line options and exit
  mbtrnpp.sh -e /path/to/environment/file -- --config=/path/to/config/file [options...] --help

 Environment variables:
  TRN_MAPFILES   - TRN map directory       [--trn-maps]
  TRN_DATAFILES  - TRN config directory    [--trn-cfg, --trn-par]
  TRN_LOGFILES   - TRN log directory       [--trn-log]
  TRN_RESON_HOST - TRN reson (or emu7k) IP [--input]
  TRN_HOST       - TRN server IP           [--trn-out, --mb-out]
  TRN_GROUP      - TRN multicast group     [--trn-out]
  TRN_MBTRNDIR   - mbtrnpp directory       [mbtrnpp path]

```