# mbtrnpp Configuration and Operation

## Contents
* [Synopsis](#synopsis)
* [File Contents](#file-contents)
  * [Environment File](#environment-file)
  * [Config File](#mbtrnpp-config-file)
* [Operation Overview](#operation-overview)  
  * [Example](#example)
  * [Tips](#tips)
  * [Output](#output)
* [Troubleshooting](#troubleshooting)
* [Appendices](#appendices)
  * [Building MB System](#building-mb-system)
  * [Code Signing gdb](#code-signing-ggdb)
  * [Formatting Markdown](#formatting-markdown)

<div style="page-break-after: always">  </div>

## Synopsis

This reference describes configuration and operation of mbtrnpp.

There are two configuration files for mbtrnpp, both are optional but strongly recommended
* environment file
  * sets variables used by TRN and (potentially) mbtrnpp
* mbtrnpp config file
  * sets MB-System and TRN processing parameters
  * supports special variables that use the environment or are dynamically determined at runtime

Running mbtrnpp is typically done by 
* sourcing the environment file
* running mbtrnpp with the --config option, along with config file override options


See [Appendix: Formatting Markdown](#formatting-markdown) for info on reformatting as HTML, PDF


<div style="page-break-after: always">  </div>

## File Contents
The environment and mbtrnpp configuration are managed by an environment file and configuration file, detailed below.


### Environment File 

The environment file (e.g. mbtrnpp-sentry.env) may be used to set environment variables
used by the TRN server and mbtrnpp.

The environment file is typically used before running mbtrnpp, and defines these variables:

|   variable     |               description            |              example           |  
| -------------: | :----------------------------------- | :----------------------------- |  
| TRN_RESON_HOST | mbtrnpp socket input IP address      | "192.168.100.1" |
| TRN_HOST       | TRN server host                      | "192.168.100.100" |
| TRN_GROUP      | TRN multicast group                  | "239.255.0.16" |
| TRN_MBTRNDIR   | mbtrnpp executable directory         | "/usr/local/bin"|
| TRN_MAPFILES   | TRN server map file directory path   | "/home/mappingauv/maps"|
| TRN_DATAFILES  | TRN server data file directory path  | "/home/mappingauv/config"|
| TRN_LOGFILES   | TRN server log output directory path | "/home/mappingauv/logs/mbtrnpp"|


It may be convenient to have multiple env files for different test environments, e.g. to switch between networks, or between an installed version and a build directory. It may be useful to have an environment file that unsets all of the relevant variables.


These environment variables interact with: mbtrnpp.cfg:

TRN_RESON_HOST

    Description:
     IP address of mbtrnpp socket input e.g. reson, 7k center, kongsberg

    Source:
     * environment variable if set
     * config file definition
     * command line definition
     * defaults to "localhost" if errors occur


TRN_HOST

    Description:
     IP address of mbtrnpp host

    Source:
     * environment variable if set
     * defaults to "localhost" if errors occur

TRN_GROUP

    Description:
     Multicast group IP address
     Applies to trnum output interface

    Source:
     * environment variable if set
     * defaults to TRNUM_GROUP_DFL in mbtrnpp.c (239.255.0.16) 


TRN_LOGFILES

    Description:
     TRN (server) log output directory path (generally inside log-directory)

    Source:
     * environment variable if set
     * defaults to "." if unset or errors occur

TRN_MAPFILES

    Description:
     TRN map files directory path

    Source:
     * environment variable if set
     * defaults to "." if unset or errors occur

TRN_DATAFILES

    Description:
     TRN data files directory path

    Source:
     * environment variable if set
     * defaults to "." if unset or errors occur

TRN_CFGFILES

    Description:
     TRN configuration files directory path

    Source:
     * environment variable if set
     * defaults to "." if unset or errors occur

SESSION

    Description:
     Session ID string (YYYYmmdd-hhmmss, used in mbtrnpp log file names)

    Source:
     * internally generated (no environment override)


TRN_SESSION

    Description:
     TRN session ID string (YYYY.jjj, used in TRN log file names)

    Source:
     * internally generated (no environment override)


<div style="page-break-after: always">  </div>

### mbtrnpp Config File

The mbtrnpp configuration file (e.g. mbtrnpp.cfg) contains configuration parameters for mbtrnpp utility in MB-System; it may be provided using the command line option 
``` 
--config=<path> 

```  

The file supports all of the command line options for mbtrnpp. The order of precedence for options is:  

* command line options
* configuration file options
* compile time defaults

The annotated example configuration file in MB-System ```src/mbtrn/tools/mbtrnpp-cfg.example``` fully documents the options and syntax.


| option | description | example | notes |
| :----- | :---------- | ------: | ----: |
| *MB-System Options* ||||
| log-directory=\<path\>       | path to MB-System and TRN server log output                   | /path/to/log/dir | |
| input=\<input spec\>         | specify input source (socket, file)                           | socket:TRN_RESON_HOST:7000:0| |
| output=\<output spec\>       | MB-System output                                              | file:mbtrnpp_SESSION.mb1 | |
| swath-width=\<sonar swath\>  | Sonar swath width (decimal deg)                               | 90.0 | |
| soundings=\<n\>              | number of sonar soundings to output (uint)                    | 11 | |
| median-filter=\<filter spec\>| median filter parameters \<threshold\>/\<n_across\>/\<n_along\>| 0.10/9/3 | |
| format=\<n\>                 | MB-System input format number                                 | 88 | |
| *TRN Options* ||||
| mbhbt=\<s\>                  | MB1 server heartbeat timeout (s)                              | 15 | |
| trnhbt=\<s\>                 | TRN server heartbeat timeout (s)                              | 15 | |
| trnuhbt=\<s\>                | TRNU (udp update) server heartbeat timeout (s)                | 15 | |
| delay=\<s\>                  | MB-1 processing loop delay (s)                                |  0 | |
| statsec=\<s\>                | TRN profiling logging interval (s)                            | 30 | |
| trn-en=\<bool\>              | enable/disable TRN processing                                 |  Y | use Y/1: enable N/0: disable |
| trn-dev=\<char\*\>              | specify sonar (reson only)                                 |  see Note [6]   |
| trn-utm=\<n\>                | UTM zone for TRN processing (int, 1-60)                       |  9 | 9:axial 10:monterey bay      |
| trn-map=\<path\>             | TRN server map file path                                      | /home/mappingauv/maps/AxialTiles  | required for TRN processing; may be a directory path for tiled map |
| trn-cfg=\<path\>             | TRN configuration file                                        | TRN_DATAFILES/mappingAUV_specs.cfg| required for TRN processing|
| trn-par=\<path\>             | TRN particles file                                            | TRN_DATAFILES/particles.cfg       | optional for TRN processing|
| trn-mid=\id\>                | TRN mission ID (for TRN server logs naming)                   | mb-TRN_SESSION | |
| trn-mtype=\<n\>              | TRN map type (int)                                            |  1 | see Note [1] |
| trn-ftype=\<n\>              | TRN filter type (int)                                         |  2 | see Note [2] |
| trn-fgrade=\<n\>             | TRN filter grade (int)                                        |  1 | 0:low grade 1:high grade |
| trn-freinit=\<n\>             | enable filter reinit (bool)                                   |  1 | 0:disable filter reinit 1:enable filter reinit |
| trn-mweight=\<n\>            | use modified weighting                                        |  4 | see Note [3] |
| trn-ncov=\<n\>               | TRN convergence criteria - northing covariance limit (double) | 49 | |
| trn-ecov=\<n\>               | TRN convergence criteria - easting covariance limit (double)  | 49 | |
| trn-nerr=\<n\>               | TRN convergence criteria - northing error limit (double)      | 200 | |
| trn-eerr=\<n\>               | TRN convergence criteria - easting error limit (double)       | 200 | |
| mb-out=\<spec\>              | MB1 output specifier (comma delimited list)                   | mb1svr:TRN_HOST:27000,mb1 | see Note [4] |
| trn-out=\<spec\>             | TRN output specifier (comma delimited list)                   | trnsvr:TRN_HOST:28000,trnu,trnusvr:TRN_HOST:8000 | see Note [5] |
| trn-decn=\<n\>               | TRN updpate decimation modulus (uint)                         |  9 | update TRN every nth MB1 sample |
| trn-decs=\<s\>               | TRN updpate decimation interval (decimal seconds)             | typically unused  | update TRN every s decimal seconds |
| trn-nombgain=\<bool\>        | enable/disable gating TRN resets using sonar transmit gain    | typically unused |use Y/1: enable N/0: disable|
| reinit-file                  | reinitialize TRN every time a new datalist file is read       | - | |
| reinit-gain                  | enable/disable gating TRN resets using sonar transmit gain    | - | |
| reinit-xyoffset=\<max\>      | reinitialize TRN whenever the magnitude of the xy converged offset > max (double) | 150.0 | |
| reinit-zoffset=\<min\>/\<max\>| reinitialize TRN whenever the magnitude of the xy converged z offset z<min, z>max (double) | 2.0/2.0 | |
| ping-decimate=\<n\>          | ping decimation modulus (int)                                 | 1 | |

[1] map type options
``` 
1: TRN_MAP_DEM 
2: TRN_MAP_BO
```
[2] filter type options
```
 0: TRN_FILT_NONE 
 1: TRN_FILT_POINTMASS
 2: TRN_FILT_PARTICLE 
 3: TRN_FILT_BANK 
```

[3] modified weighting options
```
0: No weighting modifications 
1: Shandor's original alpha modification 
2: Crossbeam with Shandor's weighting 
3: Subcloud with Shandor's original 
4: Subcloud with modified NIS always on
```

[4] mb-out spec list elements
```
 mb1svr:<ip>:<port>  - enable MB1 server (use default port if unspecified)
 mb1                 - enable MB1 binary record logging
 file:<path>         - enable MB1 binary record logging
 file                - enable MB1 binary record logging (use default name)
 reson               - enable reson S7K frame logging
 nomb1               - disable MB1 binary record logging
 noreson             - disable reson S7K frame logging
 nomb1               - disable MB1 server
 nombtrnpp           - disable mbtrnpp message log (not recommended)
```

[5] trn-out spec list elements
```
 trnsvr:<ip>:<port>  - enable trn server interface (use default port if unspecified)
 trnusvr:<ip>:<port> - enable TRN update server
 trnumsvr:<group>    - enable TRN UDP multicast server
 trnu                - enable TRN update ascii logging
 trnub               - enable TRN update binary logging
 sout                - enable TRN output to stdout
 serr                - enable TRN output to stderr
 debug               - enable TRN debug output
 notrnsvr            - disable trn server interface
 notrnusvr           - disable TRN update server

 Note that mnemonics may be used in the config files to load selected parameters from the environment:
   <ip> may be specified using mnemonic TRN_HOST
   <group> may be specified using mnemonic TRN_GROUP
e.g.
  trn-out=trnsvr:TRN_HOST:28000,trnu,trnusvr:TRN_HOST:8000,trnumsvr:TRN_GROUP
```

[6] trn-dev sonar selection values
```
7125_200 - Seabat 7125 200 kHz
7125_400 - Seabat 7125 400 kHz
T50      - T50-S, T50-R

The mnemonic is translated to a numeric ID that is passed into the reson reader, where it is 
mapped to the correct device ID and system enumerator required for the subscription message.

The mnemonic values and associated IDs are defined in r7kc.h
The mbtrnpp default is set using mbtrnpp.c:OPT_TRN_DEV_DFL

The option may be set for mbtrnpp and mbtrnpp.sh using --trn-dev
The option may be set for frames7k and stream7k using --dev
```
<div style="page-break-after: always">  </div>

## Operation Overview

mbtrnpp is typically run as follows:

* open a console session on the mbtrnpp host

* source the environment file
```
. /path/to/env/file
```

* start mbtrnpp
```
mbtrnpp --config=/path/to/mbtrnpp.cfg # ...options
```

### Example
```
cd /home/mappingauv
. env/mbtrnpp-Sentry.env
env |grep TRN
mbtrnpp --config=TRNcfg/mbtrnpp-Sentry.cfg

```

### Tips

* Use the --help option to inspect the configuration (mbtrnpp outputs defaults, config file and final configuration and exits)


## Output

The logged mbtrnpp outputs are described below.
Locations are determined by the log-directory option and/or TRN_LOGDIRECTORY environment variable as indicated.


| log | contents | location | notes |
| --- | -------- | -------- | ----- |
| YYYYMMDD_HHMMSSNNNNN_mbtrnpp_log.txt| MB-System processing log   | log-directory | |
| mbtnpp-YYYYMMDD_HHMMSSnnnn.log      | mbtrnpp session log        | log-directory | |
| mb1svr-YYYYMMDD_HHMMSSnnnn.log      | mbserver session log       | log-directory | |
| mbtrnpp_YYYYMMDD_HHMMSSnnnn.mb1     | mb1 binary record log      | log-directory | |
| trnsvr-YYYYMMDD_HHMMSSnnnn.log      | TRN TCP server session log | log-directory | |
| trnusvr-YYYYMMDD_HHMMSSnnnn.log     | TRN UDP server session log | log-directory | |
| trnu-YYYYMMDD_HHMMSSnnnn.log        | TRN UDP update record log  | log-directory | |
| \<mid\>-YYYY.JJJ-TRN.NN             | TRN instance log directory | TRN_LOGDIRECTORY | mid (mission ID) determined by trn-mid option|
 
<div style="page-break-after: always">  </div>

----

## Troubleshooting 

----

### General

* Read error/warning message output; it may help to redirect stdout and stderr to a file, then tail and grep
```
mbtrnpp [your_command_line_options...] &> mbtrnpp-console.out
# [in a separate console]
tail -f mbtrnpp-console.out |grep -e <expression>
```
* Read the log files

* Check the configuration options to make sure they are set as intended
```
mbtrnpp [your_command_line_options...] --help
```
* Check the environment variables
```
env|grep TRN
```

* Check UTC zone (--trn-utc); when improperly configured, data will flow, but TRN will reject input and fail to converge and may report errors.

### Connections

* Are any IP addresses in the configuration using "localhost"? If so, any clients running on another host can't connect to them.
* Do the IP ports being used for clients and their servers (TRN network interfaces) match?
* Tools like netstat and wireshark are potentially helpful

### Segfaults

segfaults are uncommon in the production code. In the event that one occurs, 
gdb (or ggdb on OSX) are useful for debugging; OSX and linux are slightly different, and may be need to run under libtool.

```
# on linux
gdb --args your_cmdline 

# on OSX
ggdb -- your_cmdline 
```

If debugging in a build directory (i.e. uninstalled libtool binaries), use libtool (or glibtool on OSX) to run gdb

```
# on linux
libtool --mode=execute gdb --args  your_cmdline

# on OSX
glibtool --mode=execute ggdb -- your_cmdline
```

Installing ggdb on OSX requires that it be codesigned; see [Appendix: Code Signing ggdb](#code-signing-ggdb) for procedure.


<div style="page-break-after: always">  </div>

----

# Appendices

----

## Building MB System

Configuration lines for MB-System on different platforms

### Linux, OSX (homebrew)

```
./configure --enable-mbtrn --enable-mbtnav --disable-mbtools
make clean
make

```

### OSX (w/ macports)
If dependencies are located on macports paths, set compilation environment and include appropriate configure options:

```
CFLAGS="-I/opt/local/include -Wall" LDFLAGS=-L/opt/local/lib CPPFLAGS="-I/opt/local/include -Wall" \
./configure \
--with-proj-lib=/opt/local/lib/proj5/lib \
--with-proj-include=/opt/local/lib/proj5/include \
--with-fftw-lib=/opt/local/lib \
--with-fftw-include=/opt/local/include \
--with-motif-lib=/opt/local/lib \
--with-motif-include=/opt/local/include \
--enable-mbtrn --enable-mbtnav --disable-mbtools

```

### Cygwin


Cygwin dependencies:

Use the cygwin setup utility to install the following packages:

```
gcc-g++
rpc-devel, gambas3-devel, libproj-devel, libproj12, libnetcdf-devel, libnetcdf19
libgdal-devl, libgdal19, libfftw3-devel, libfftw3, libpcre, glib, ghostscript, cmake, curl
```
The generic mapping tools package (libgmt) is also required, but must be built and installed from source.
The source package may be downloaded from the project website:
```
https://www.generic-mapping-tools.org/download
support data must also be downloaded from
https://github.com/GenericMappingTools/gshhg-gmt/releases/download/2.3.7/gshhg-gmt-2.3.7.tar.gz
```
There is a windows installer on the gmt project website, but it does not install the required libraries and headers.
Full instructions for building from source are available on the project website

```
https://github.com/GenericMappingTools/gmt/blob/master/BUILDING.md
```

The basic steps include:

	download and uncompress the source tar.gz
	cd gmt-<version>
	optionally copy and modify 
	   cmake/ConfigUserTemplate.cmake -> cmake/ConfigUser.cmake 
	   cmake/ConfigUserAdvancedTemplate.cmake -. cmake/ConfigUserAdvanced.cmake
	mkdir build
	cd build
	cmake ..
	make
	make install

Building mbtrnpp for cygwin requires the --disable-dependency-tracking configure option, and make must be invoked with LDFLAGS=-no-undefined:

```
./configure  —-disable-dependency-tracking  --enable-mbtrn --enable-mbtnav --disable-mbtools
make clean
make LDFLAGS=-no-undefined

```

*Note --disable-dependency-tracking configure option may have an ordering sensitivity:*  

```
./configure —-disable-dependency-tracking --enable-mbtrn --enable-mbtnav --disable-mbtools  
```

succeeds but

```
./configure --enable-mbtrn --enable-mbtnav --disable-mbtools —-disable-dependency-tracking 
```

fails. The issue may only appear initially and clear up on subsequent builds.

*Note: undefined reference to PSL_... errors may occur during src/gmt build.*
This appears to happen because gmt dependencies are generated configure using

```
gmt-config --libs 
```
which returns 

```
-L/usr/local/lib -lgmt
```
but doesn't include a dependency on libpostscriptlight
A workaround is to include the necessary args in LDFLAGS passed to make (AFTER mbtrn has built)

```
make LDFLAGS="-no-undefined -L/usrlocal/lib -lpostscriptlight"
```
This warning may be ignored
```
*** Warning: linker path does not have real file for library -lpostscriptlight.dll.a.
*** I have the capability to make that library automatically link in when
*** you link to this library.  But I can only do this if you have a
*** shared version of the library, which you do not appear to have
*** because I did check the linker path looking for a file starting
*** with libpostscriptlight.dll.a but no candidates were found. (...for file magic test)
```
----

----

## Code Signing ggdb

*Note - I found that if I first followed these instructions to create the certificate BEFORE attempting to complete the gdb signing instructions, I was able to get it to work. The only exception is that I had to keep the certificate in the System Keychain instead of moving it back into Login.*

Here's a consolidated set of steps:

### Creating the Certificate with the right permissions

* Launch /Applications/Utilities/Keychain Access.app
* In Keychain Access select the "login" keychain in the "Keychains" list in the upper left hand corner of the window.
* Select the following menu item:
  * Keychain Access->Certificate Assistant->Create a Certificate...
* Set the following settings:
  * Name = "gdb-cert"
  * Identity Type = Self Signed Root
  * Certificate Type = Code Signing
  * Click Create
  * Can customize the expiration date (3650 days = 10yrs)
  * Click Continue
  * Click Done
* Click on "My Certificates"
* Double click on your new "gdb-cert" certificate
* Turn down the "Trust" disclosure triangle, scroll to the "Code Signing" trust pulldown menu and select "Always Trust" and authenticate as needed using your username and password.
* Drag the new "gdb-cert" code signing certificate (not the public or private keys of the same name) from the "login" keychain to the "System" keychain in the Keychains pane on the left hand side of the main Keychain Access window. This will move this certificate to the "System" keychain. You'll have to authorize a few more times, set it to be "Always trusted" when asked.
* In the Keychain Access GUI, click and drag "gdb-cert" in the "System" keychain onto the desktop. The drag will create a "~/Desktop/gdb-cert.cer" file used in the next step.
* Switch to Terminal, and run the following:
  * sudo security add-trust -d -r trustRoot -p basic -p codeSign -k /Library/Keychains/System.keychain ~/Desktop/gdb-cert.cer
  * rm -f ~/Desktop/gdb-cert.cer
* Drag the "gdb-cert" certificate from the "System" keychain back into the "login" keychain (and maybe back again...?) EDIT: apparently not necessary, per comments
* Quit Keychain Access
* Reboot

### Checking the Certificate:

* security find-certificate -c gdb-cert -> should show some details about the cert, if it can be found
* security find-certificate -p -c gdb-cert | openssl x509 -checkend 0 -> should say the cert won't expire
* security dump-trust-settings -d -> should show that this cert has code signing trust setting enabled (may show other certs/permissions)

### Creating the "entitlements.xml" File:

Copy the text below and save it in an "entitlements.xml" file in your current directory:

```
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
   
<key>com.apple.security.cs.debugger</key>

<true/>
</dict>
</plist>
```
### Signing the debugger binaries

Run the following commands in terminal:
```
# codesign with entitlements
codesign --entitlements entitlements.xml -fs gdb-cert $(which gdb) 

# verify codesigning
codesign -vv $(which gdb) 

# display details of code signature
codesign -d --entitlements - $(which gdb) 
```
### Refresh System Certificates

Reboot the machine

----

----

## Formatting Markdown

This file is best viewed as a PDF, HTML, in a text editor or markdown viewer, e.g. this [extension for Google Chrome](https://chrome.google.com/webstore/detail/markdown-viewer/ckkdlimhmcjmikdlpkmbgfkaikojcbjk?hl=en)
The Chrome extension offers a number of CSS formatting styles, and it's possible to export it as PDF or HTML. 


Use pandoc to generate HTML (and other formats)
```
pandoc -s [-c <css_style>] <input_path> --metadata title="doc title" -o <output_path>

```
where

|    option   |          description       |
| ----------: | :------------------------- |
|          -s | standalone output          |
|          -c | specify a CSS style        | 
|  input_path | markdown source            |
|  --metadata | specify the HTML page title|
| output_path | path to output HTML file   |

The default HTML styles are plain, but functional.
Once in HTML format, use a browser PDF export (print or save as PDF) to generate a PDF file (typically preserves formatting and links).

Tip: get CSS styles used by the google markdown extension or other viewer, then pass them to pandoc with the -c option.
To get styles from the chrome markdown extension, use the page inspection tools to locate the css resource, then copy/paste into a text (.css, e.g.) file.

Example
```
pandoc -s /doc/markdown/mbtrnpp-op-Sentry.md --metadata title="mntrnpp Config Cuide" -o /doc/html/mbntrnpp-cfg.html
```

----

----

