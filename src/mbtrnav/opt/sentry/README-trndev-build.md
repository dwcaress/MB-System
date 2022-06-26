# trndev README

Notes on building and running TRN update client software from the libtrnav repository.
     
     
Supported Platforms:
- linux
- OSX
- Cygwin

Requires:

- cmake or gnu make
- libtrnav https://bitbucket.org/mbari/libtrnav
- mframe https://bitbucket.org/mbari/mframe
- libnetcdf
- libpthread
- some netcdf versions require libhdf5 and/or libcurl

## Quick Start

### from trndev-<version>.tar.gz

Build the mframe and libtrnav libraries and applications using cmake:

```
tar xzvf trndev-<version>.tar.gz
cd trndev-<version>
./trndev-build-cmake.sh
```

Applications, libraries and headers are installed in /usr/local by default.
Optionally, set DESTDIR and/or PREFIX to install in $DESTDIR/$PREFIX:

```
sudo ./trndev-build-cmake.sh -i [-d <path> -p <path>]
```

A copy of the cmake build output is automatically staged in

```
trndev-<version>/mframe/build/pkg
trndev-<version>/libtrnav/build/pkg 
```

---

## Build script options

### trndev-build-cmake.sh

```
Description: build/install trndev using cmake

usage: trndev-build-cmake.sh [options]
Options:
-d <s> : set DESTDIR for install/uninstall []
-p <s> : set PREFIX for install/uninstall [/usr/local]
-i     : install
-u     : uninstall
-h     : help message
-v     : verbose output
```

### trndev-build-gnu.sh

```
Description: build/install trndev using gnu make

usage: trndev-build-gnu.sh [options]
Options:
-d <s> : set DESTDIR for install/uninstall []
-p <s> : set PREFIX for install/uninstall [/usr/local]
-i     : install
-u     : uninstall
-h     : help message
-v     : verbose output

```

### Examples

```

# build using cmake
trndev-build-cmake.sh

# build using gnu
trndev-build-gnu.sh

# install to default directory /usr/local
sudo trndev-build-cmake.sh -i

# uninstall from default directory /usr/local
sudo trndev-build-cmake.sh -u

# install to staging directory w/ default PREFIX (as non-root)
trndev-build-cmake.sh -id $PWD/stage

# uninstall from staging directory w/ custom PREFIX (as non-root)
trndev-build-cmake.sh -ud $PWD/stage -p /foo/bar
```
---

## Get the code 

### from developmenent snapshot (tar.gz)

```
ftp://ftp.mbari.org/pub/trn
tar xzvf trndev-<version>.tar.gz 
```

### from git

These repos are in an MBARI workspace
and require permission to access

```
 mkdir trn-dev
 cd trn-dev
 git clone git@bitbucket.org:mbari/mframe.git
 git clone git@bitbucket.org:mbari/libtrnav.git
```

---

## Build/run the test apps

Instructions in this doc reference TRNDEV
as the path to the directory containing libtrnav and mframe

```
 export TRNDEV=/path/to/trn-dev
```

## Manual build using cmake [1]

### build mframe

```
 cd ${TRNDEV}/mframe
 mkdir build
 cd build/
 cmake ..
 cmake --build .
 cmake --install . --prefix `pwd`/pkg
```

### build libtrnav 

```
 cd ${TRNDEV}/libtrnav
 mkdir build
 cd build/
 cmake ..
 cmake --build .
 cmake --install . --prefix `pwd`/pkg
```
### build test apps

Note: These build as part of the libtrnav package
The binaries are in ${TRNDEV}/build

```
 (deprecated)
 cd ${TRNDEV}/trnapp
 mkdir build
 cd build/
 cmake .. ls
 cmake --build .
```

### run apps [2]

```
 cd ${TRNDEV}
 ./libtrnav/build/trnusvr-test --host=localhost
 ./libtrnav/build/trnucli-test --host=localhost --input=S --ofmt=p --async=3000 
```

[1] cmake command may vary (cmake, cmake3, etc.)
[2] Recommended to run in separate terminals; otherwise run trnusver-test in the background.

---

## Manual build using gnu make

### build mframe

```
 cd ${TRNDEV}/mframe
 mkdir build
 make 
```

### build libtrnav

```
 cd ${TRNDEV}/libtrnav
 mkdir build
 make all trnc
```

### run apps [1]

```
 cd ${TRNDEV}
 ./libtrnav/bin/trnusvr-test --host=localhost
 ./libtrnav/bin/trnucli-test --host=localhost --input=S --ofmt=p --async=3000
```
    Use CTRL-C to stop either application

[1] Recommended to run in separate terminals; otherwise run trnusver-test in the background.

---

## Output

Log output is directed to working directory (where app was started).

|  trnusvr-test Output  |        Destination      |
| :--------------------:|:------------------------|
| status                | stderr                  |
| server instance data  | netif-\<session\>.log.  |
| trnusvr-test session  | trnusvr-\<session\>.log |

|  trnucli-test Output  |             Destination           |
| :--------------------:|:--------------------------------- |
| status                | stderr                            |
| trnucli-test session  | trnucli-\<session\>.log           |
| trnuli_ctx session    | trnuctx-<session\>-\<client\>.log |

---

## Use notes

### trnusvr-test

trnusvr-test is a stand-alone test server for TRN UDP update clients.

It serves synthesized data using the same interface presented by mbtrnpp,
the MB-System multibeam filtering component. 
While it uses the same binary libraries for the network interface, it does not include a TRN instance.

Applications integrating TRN UDP clients can use this server as a lightweight check, without installing MB-System.

The data are designed to exercise and validate the client APIs and resource loading.
The location fields are zero; time and state/status fields vary periodically. 

Users could potentially extend the server to provide additional application-specific test coverage.

```
    $ trnusvr-test --help

    trnif server unit test

    trnusvr-test [options]
     --verbose=n    : verbose output, n>0
     --help         : output help message
     --version      : output version info
     --host=ip:n    : TRN server host:port
     --update=f     : update period sec
     --hbto=f       : hbeat tiemout
     --delay=u      : delay msec
     --logdir=s     : logdir prefix

    Typical Use:
    trnusvr-test --host=<trnsvr IP>[:<port>]
```

### trnucli-test

trnucli-test provides a reference implementation and test coverage for TRN UDP update client components and APIs.

In general, trnucli-test creates a TRN update client that connects to an update server and outputs the updates received.

Input formats:
- binary (from server or file)
- CSV

Output formats
- binary
- CSV 
- hex (raw or formatted) 

There are two TRN client APIs: trncli (synchronous) and trncli_ctx (asynchronous).
The trncli demo can read and write updates in CVS or binary formats.
The trncli-ctx demo reads from a socket and 

```
    $ trnucli-test --help

    TRNU client (trnu_cli) test

    trnucli-test [options]
     --verbose     : verbose output
     --help        : output help message
     --version     : output version info
     --host[:port] : TRNU server host:port
     --input=[bcs] : input type (B:bin C:csv S:socket)
     --ofmt=[pcx]  : output format (P:pretty X:hex PX:pretty_hex C:csv
     --serr        : send updates to stderr (default is stdout)
     --ifile       : input file
     --hbtos       : heartbeat period (sec, 0.0 to disable)

    trncli API options:
     --block=[lc]  : block on connect/listen (L:listen C:connect)
     --update=n    : TRN update N
     --demo=n      : use trn_cli handler mechanism, w/ periodic TRN resets (mod n)

    trncli_ctx API options:
      --rctos=n     : reconnect timeout sec (reconnect if no message received for n sec)
      --nddelms=n   : delay n ms on listen error
      --ltoms=n     : listen timeuot msec
      --rcdelms=n   : delay n ms on reconnect error
      --no-log      : disable client logging
      --logstats=f  : async client stats log period (sec, <=0.0 to disable)
      --async=n     : use asynchronous implementation, show status every n msec

    Typical Use:

    # async client
    trnucli-test --host=<trnsvr IP>[:<port>] --input=S --ofmt=p --async=3000

    Defaults:
          port        8000
         input         svr
          ofmt      pretty
         hbtos         0.0
        update          10
         rctos      10.000
         ltoms          50
       nddelms          50
       rcdelms        5000
      logstats      60.000
        log_en           Y
         ofile      stdout
```
