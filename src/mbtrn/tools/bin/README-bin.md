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
  -D path : TRN config directory    [/Volumes/linux-share/config]
  -L path : TRN log directory       [/Volumes/linux-share/git/mbsys-trn/MB-System/test/feature.initvars/logs]
  -M path : TRN maps directory      [/Volumes/linux-share/maps]
  -m path : override mbtrnpp dir    [/Volumes/linux-share/git/mbsys-trn/MB-System/src/mbtrnutils]
  -o addr : TRN host                [192.168.1.101]
             affects : [--trn-out, --mb-out]
  -s addr : TRN source host         [localhost]
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
  TRN_MAPFILES    - TRN map directory       [--trn-maps]
  TRN_DATAFILES   - TRN config directory    [--trn-cfg, --trn-par]
  TRN_LOGFILES    - TRN log directory       [--trn-log]
  TRN_SOURCE_HOST - TRN source IP           [--input]
  TRN_HOST        - TRN server IP           [--trn-out, --mb-out]
  TRN_GROUP       - TRN multicast group     [--trn-out]
  TRN_MBTRNDIR    - mbtrnpp directory       [mbtrnpp path]

```

## mdoc-release.sh, mkdoc.sh

mkdoc-release.sh is a shell script that generates a PDF and HTML document package from the mbtrn/tools README files.
mkdoc-release.sh creates a (timestamped) output directory for each session.

mkdoc-release.sh uses the mkdoc.sh script, which converts markdown (and text/PDF assets) to html and PDF using pandoc, ghostscript.  

Generally, users just call mkdoc-release.sh; it is not necessary to call mkdoc.sh directly.

### Features

* Generates CONTENTS doc 
* markdown may include PDF and text file assets
* Multiple styles available

### Dependencies
* wkhtmltopdf
* ghostscript
* pandoc

__mkdoc-release.sh__
```
Description: Generate mbtrn tools doc package

usage: mkdoc-release.sh [options]
Options:
-h    : print use message
-v    : verbose output  [N]

Examples:
src/mbtrn/tools/bin/mkdoc-release.sh -v

```
__mkdoc.sh__
```
Description: Convert markdown to HTML, PDF using pandoc

usage: mkdoc.sh [options]
Options:
-h    : print use message
-a p  : asset (file or directory)
may include more than once
-i f  : input file              []
-o f  : output file             []
-s s  : style                   [radar]
options                [ foghorn  ghostwriter  github  markdown  new-modern  radar  vostok ]
-S p  : style path              [/Volumes/linux-share/git/mbsys-trn/MB-System/src/mbtrn/tools/bin/../doc/styles]
-t s  : title                   []
-L m  : left margin   e.g. 20mm []
-R m  : right margin  e.g. 20mm []
-T m  : top margin    e.g. 20mm [20mm]
-B m  : bottom margin e.g. 20mm [20mm]
-N    : test                    [N]

Examples:
 src/mbtrn/tools/bin/mkdoc.sh -vi README-mbtrncfg.md
```

