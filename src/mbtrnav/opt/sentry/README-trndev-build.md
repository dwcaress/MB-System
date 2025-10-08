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

A copy of the cmake build output is automatically staged in

```
trndev-<version>/mframe/build/pkg
trndev-<version>/libtrnav/build/pkg 
```

After building, run again using the -i option to install applications, libraries and headers in /usr/local (default).  
Optionally, use -d and/or -p to install in $DESTDIR/$PREFIX:

```
sudo ./trndev-build-cmake.sh -i [-d <path> -p <path>]
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
  -m <s> : libmframe cmake options
  -t <s> : libtrnav cmake options
  -h     : help message
  -v     : verbose output

Use Notes:
 This script is typically run twice:
   build (optionally using -t, -m)
   then
   install (using -i, optionally -p, -d)
   or
   uninstall (using -u)

   When uninstalling, use the same -p, -d options
   used to install

Examples:

 # build
   trndev-build-cmake.sh

 # build using cmake TRN options
   trndev-build-cmake.sh -t "-DbuildUseProj=ON"

 # install to default directory /usr/local
   sudo trndev-build-cmake.sh -i

 # uninstall from default directory /usr/local
   sudo trndev-build-cmake.sh -u

 # install to staging directory w/ default PREFIX (as non-root)
   trndev-build-cmake.sh -id $PWD/stage

 # uninstall from staging directory w/ custom PREFIX (as non-root)
   trndev-build-cmake.sh -ud $PWD/stage -p /foo/bar
```

### trndev-build-gnu.sh

The trndev-build-gnu.sh script may be used to build/install the packages using gnu make.  
Applications and libraries are staged in libtrnav/bin; the gnu build includes some additional  
utility applications.  

After building, run again using the -i option to install applications, libraries and headers in /usr/local (default).  

When using the -d option and -p together, add a leading slash to the -p path.

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


__cmake__

```
# build using cmake
trndev-build-cmake.sh

# install to default directory /usr/local
sudo trndev-build-cmake.sh -i

# uninstall from default directory /usr/local
sudo trndev-build-cmake.sh -u

# install to staging directory w/ default PREFIX (as non-root)
trndev-build-cmake.sh -id $PWD/stage

# uninstall from staging directory w/ custom PREFIX (as non-root)
trndev-build-cmake.sh -ud $PWD/stage -p /foo/bar
```

__gnu make__

```
# build using gnu
trndev-build-gnu.sh

# install to staging directory w/ default PREFIX (as non-root)
trndev-build-gnu.sh -id $PWD/stage

# uninstall from staging directory w/ custom PREFIX (as non-root)
# note leading slash with -p
trndev-build-gnu.sh -ud $PWD/stage -p /foo/bar
```
---

## Run the test apps

Run TRN update client/server applications.  
Use CTRL-C to stop either application.  

It is recommended to run in separate terminals; otherwise run trnusver-test in the background.

Instructions in this doc reference TRNDEV as the path to the directory containing libtrnav and mframe.  
Detailed use notes are included below.  

```
 export TRNDEV=/path/to/trn-dev
```

### run trnu client/server test apps


__from cmake build staging directory__

```
 cd ${TRNDEV}
 ./libtrnav/build/pkg/bin/libtrnav-0.1.0/trnusvr-test --host=localhost
 ./libtrnav/build/pkg/bin/libtrnav-0.1.0/trnucli-test --host=localhost --input=S --ofmt=p --async=3000 
```

__from gnu build staging directory__

```
 cd ${TRNDEV}
 ./libtrnav/bin/trnusvr-test --host=localhost
 ./libtrnav/bin/trnucli-test --host=localhost --input=S --ofmt=p --async=3000
```

### Test application output

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

# Appendices

## Troubleshooting

### missing libpthread or libm

The location and packaging of the pthreads and math libraries (libpthread, libm) varies by platform.  
On some platforms, they may exist as static or dynamic libraries, and may not have generic symbolic links (e.g. libpthread.so).

The simple CMakeLists.txt files used here originated for a specific application, using older versions of linux and cmake.  
It uses cmake find\_library, which provides possible names and path hints to enable cmake to find the libraries.

If cmake is unable to find a library, it may be necessary to locate it and add a new path or name to the appropriate CmakeLists.txt.
To find libpthread on linux, try

```
sudo find /usr -name libpthread*
```

If it is not found, it may be necessary to install it.  
For linux, package names may vary by distribution.  
On MacOS, they may be available via Macports or Homebrew, or by installing XCode command line tools.

## Test app use notes 

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

---

## Build distribution

Build libtrnav and mframe source distribution tar.gz with build scripts.  

Repo(s) are in an MBARI workspace
and require permission to access

```
git clone git@bitbucket.org:mbari/libtrnav.git
cd libtrnav
mkdir tmp
cd tmp
../opt/sentry/trndev-mkpkg.sh
```

Generates mktrndev/trndev-<version>.tar.gz

---

## Build manually

### Using cmake

The cmake command may vary by platform (cmake, cmake3, etc.) 

__build mframe__

```
 cd ${TRNDEV}/mframe
 mkdir build
 cd build/
 cmake ..
 cmake --build .
 cmake --install . --prefix `pwd`/pkg
```

__build libtrnav__

```
 cd ${TRNDEV}/libtrnav
 mkdir build
 cd build/
 cmake ..
 cmake --build .
 cmake --install . --prefix `pwd`/pkg
```

Output is in libtrnav/build/pkg

---

### Using gnu make

__build mframe__

```
 cd ${TRNDEV}/mframe
 mkdir build
 make 
```

__build libtrnav__

```
 cd ${TRNDEV}/libtrnav
 mkdir build
 make all trnc
```

Output is in libtrnav/bin  
Objects are libtrnav/build

