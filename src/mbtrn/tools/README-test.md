# Testing TRN 

2021-07-14
k headley

In the operational context, the mbtrnpp application receives bathymetry and navigation data 
sonar or file. Filtering is applied, generating an MB1 record, consisting of a set of timestamped beam 
ranges and location. The MB1 data is passed to a TRN instance, which produces location offsets 
in the form of TRN update messages (trm_msg) that may be used by applications.

mbtrnpp is supports real-time input from multibeam sonars, as well as reading data from files
(i.e. data playback).

mbtrnpp presents a number of network interfaces for receiving MB1 data, TRN updates, and
for interacting with the TRN instance (e.g. status and control).

In operation, the MVC typically uses the TRN UDP update interface (trnusvr).
The mb1svr is typically used for simulation, and interacts with a separate, stand-alone trn-server instance.

The interfaces are summarized in the tables below.

__mbtrnpp Inputs__

|Input        |Protocol|Description                                  |
|-------------|--------|---------------------------------------------|
|7KCenter     |TCP/UDP |Reson multibeam                              |
|Data list    |file    |Read multibeam formats supported by MB-System|
|emu7k        |file    |emulate 7KCenter (S7K only)                  |

__mbtrnpp Outputs__

|Output          |Protocol     |message   |Description          |
|----------------|-------------|----------|---------------------|
|mb1svr          |UDP          | mb1\_msg |MB1 record           |
|trnusvr         |UDP          | trn\_msg |TRN UDP update [1]   |
|trnumsvr        |UDP multicast| trn\_msg |TRN UDP update [2]   |
|trnsvr          |TCP/IP       | TRN      |TRN client     [3]   |

[1] bi-directional protocol w/ TRN reset support, heartbeat
[2] uni-directional, no protocol/heartbeat
[3] full TRN server API

__MVC Inputs__

|mapper MVC Input|Protocol|Description             |
|----------------|--------|------------------------|
|MbTrnUpd8       |UDP     |TRN UDP update (trnusvr)|
|MbTrnRcv        |UDP     |MB1 records   (mb1svr)  |


## Testing MB-System TRN Interfaces

The MB-System TRN code is may be exercised independently using multibeam data files and a client 
for the desired output interface.   
The basic procedure consists of these steps:

* Generate a data list for the test data files
  * on the command line e.g.
  * use the MB-System utility
  * edit by hand
* Generate an mbtrnpp environment and config file (or copy and modify an existing one)
  * Copy and edit an existing file (be sure it has all the latest options)
  * Start with the templates included in 
    ```
    MBTRN/src/mbtrn/tools/env
    MBTRN/src/mbtrn/tools/cfg
    ```
  * Use the mbtrncfg web-based configuration tool
    ```
    MBTRN/src/mbtrn/tools/mbtrncfg
    ```
* Source the environment file
* Start mbtrnpp
* Run an appropriate mbtrn test client
  * trnc (MB1/mb1svr)
  * mbtnav-cli (UDP/trnusvr)
  * trnu-cli  (UDP/trnusvr, UDP mcast/trnumsvr)
  * trn-cli (MB1/mb1svr)
  check the output:
  * valid pings
  * current timestamps
  * mbtrnpp covariance magnitude, convergence counts (Ncs, Nct, Nus, Nut, Nr), USE flag 
* Optionally run the appropriate MVC utility to exercise the MVC TRN input
	 * MbTrnUpd8Test (UDP/trnusvr)
	 * MbTrnRecvTest (MB1/mb1svr)
* Optionally, plot TRN output (mbtrnpp logs) using using MBTRN/src/tools/mbtrnpp-plot
* See Resources and Examples below for additional documentation

## Testing MVC Interfaces
A pair of utilities exist may be used to test the MVC 

## Resources

### Doc

__READMEs__

```
Tip: most TRN doc is written in markdown. Viewing options include
- Use the Chrome markdown browser extension
   https://chrome.google.com/webstore/detail/markdown-viewer/ckkdlimhmcjmikdlpkmbgfkaikojcbjk?hl=en
   
- convert to PDF using pandoc or other conversion package
- open in a markdown editor/reader (e.g. bbedit)
```

|File                                                     |Contents                                    |
|---------------------------------------------------------|--------------------------------------------|
|MB-SYSTEM/src/mbtrn/tools/README-tools.md                |list of content in the mbtrn/tools directory|
|MB-SYSTEM/src/mbtrn/mbtrnpp-plot/README-mbtrn-plot-use.md|how to plot TRN results using mbtrn-plot    |
|MB-SYSTEM/src/mbtrn/mbtrnpp-plot/README-mbtrn-plot-dev.md|extending mbtrn-plot                        |
|MB-SYSTEM/src/mbtrn/mbtrncfg/README.md                   |using the mbtnpp configuration tool         |
|MB-SYSTEM/src/mbtrn/cfg/README-mbtrnpp.md                |running mbtrnpp using the wrapper script    |
|MB-SYSTEM/src/mbtrn/bin/README.md                        |mbtrnpp wrapper script use notes            |

### Utilities

__Application__

|name      |description                                             |module                |
|----------|--------------------------------------------------------|----------------------|
|mbtrnpp   |MB-System TRN application                               |mbtrnutils            |
|mbtrnpp.sh|mbtrnpp wrapper script w/ auto-restart, console logging |mbtrn/tools/bin       |
|mbtrncfg  |Web based mbtrnpp config file and command line generator|mbtrn/tools/mbtrncfg  |


__Test__

|     name     |description                                      |module |notes|
|--------------|-------------------------------------------------|-------|-----|
|trnc          |TRN MB1 client                                   |mbtrn  |0|
|mbtnav-cli    |TRN UDP update client w/ csv, ascii, hex output  |mbtrn  |0|
|udps/udpc     |General purpose UDP client/server                |mbtrn  |0,1|
|frames7k      |7KCenter client (formatted frame console output  |mbtrn  |0|
|stream7k      |7KCenter client (formatted stream console output |mbtrn  |0|
|emu7k         |7KCenter emulator                                |mbtrn  |2|
|r7kr_test     |7KCenter reader test                             |mbtrn  ||
|tbinx         |format, stream mbtrnpp MB1 log to socket, console|mbtrn  ||
|mmcpub/mmcsub |UDP multicast client/server                      |mbtrnav|0|
|trn-server    |Stand-alone TRN server application               |mbtrnav||
|trnu-cli      |TRN client (incl async, mcast) test              |mbtrnav|0|
|trn-cli       |trn\_cli/trnw API test                           |mbtrnav||
|trnclient-test|TRNClient API unit test                          |mbtrnav||
|netif-test    |netif component unit test                        |mbtrnav||
|trnif-test    |trnif component unit test                        |mbtrnav||
|trnusvr-test  |trnusvr component unit test                      |mbtrnav||
|trnifsvr-test |                                                 |mbtrnav||

[0] frequently used diagnostic   
[1] udpc may be used to check MB1 server (mb1svr) function  
[2] For testing, it is generally simpler to use mbtrnpp, which can source S7K files (and other formats) directly   

__Plotting__

|name        |description               |module                |
|------------|--------------------------|----------------------|
|mbtrnpp-plot| plog mbtrnpp TRN log data|mbrn/tools/mbtrpp-plot|

__Data__

|name   |description                            |module|
|-------|---------------------------------------|------|
|mb1conv|Convert MB1 (tbin) records to F71 (fbt)|mbtrn |
|mb12csv|Convert MB1 records to CSV             |mbtrn |
|

### Utility Examples

***

__Running mbtrnpp__

```
call mbtrnpp directly (OK for testing)

/path/to/mbtrnpp --config=/path/to/config_file [-- <option>...] 

or use the wrapper script (recommended for mission)

/path/to/mbtrnpp.sh [<mbtrnpp.sh options>...] [-- <mbtrnpp_options...>]      
```

Use mbtrnpp --verbose=n option to control debug output:

|   |   | 
|---|---|
|0|suppress diagnostic output|
|<0|TRN diagnostic output (-1 or -2 recommended)|
|>0|MB-System diagnostic output (1 or 2 is VERY verbose)|

Use the mbtrnpp.sh -t option to preview the mbtrnpp command line:

```
$ ./src/mbtrn/tools/bin/mbtrnpp.sh -e env/mbtrn-cygwin.env -t -- --config=env/mbtrnpp.cfg --trn-utm=11

env:
TRN_LOGFILES=/cygdrive/c/cygwin64/home/reson/test/logs
TRN_DATAFILES=/cygdrive/z/win_share/test/config
TRN_HOST=134.89.13.79
TRN_RESON_HOST=134.89.13.75
TRN_GROUP=239.255.0.16
TRN_MAPFILES=/cygdrive/z/win_share/test/maps
TRN_MBTRNDIR=./src/mbtrn

cmdline:
./src/mbtrn/mbtrnpp --config=env/mbtrnpp.cfg --trn-utm=11
```
***

__Generate an mbtrnpp data list for testing (bash)__ 

```
find /path/to/data -name <pattern>|sed -e "s/.s7k/.s7k 89/" \> my_datalist.mb89
e.g.
find /Volumes/linux-share -name 2018*s7k |sed -e "s/.s7k/.s7k 89/g" > test.mb89
cat test.mb89 
/Volumes/linux-share/20180712_212336.s7k 89
/Volumes/linux-share/20180713_224134.s7k 89
/Volumes/linux-share/20180713_224844.s7k 89
/Volumes/linux-share/20180713_225546.s7k 89
```

Note that the format number should reflect the data format


***

__Run mbtrnpp__ 

- run directly in this example (for test)
- recommended to run via mbtrnpp.sh wrapper script for deployment

```
# source env file
. /Volumes/wdcs/20210609m1/mbtrn-210609m1.env

# start mbtrnpp
/Volumes/linux-share/git/mbsys-trn/MB-System/src/mbtrnutils/mbtrnpp --config=/Volumes/wdcs/20210609m1/mbtrn-210609m1.cfg --verbose=0  --format=-1 --input=/Volumes/wdcs/20210609m1/20210609m1.mb89

command line:
[/Volumes/linux-share/git/mbsys-trn/MB-System/src/mbtrnutils/.libs/mbtrnpp --config=/Volumes/wdcs/20210609m1/mbtrn-210609m1.cfg --verbose=0 --format=-1 --input=/Volumes/wdcs/20210609m1/20210609m1.mb89]

configuration - default:
                           self                     0x10b6c3880
                        verbose                               0
                     input_mode                               2
                          input                   datalist.mb-1
              socket_definition    socket:TRN_RESON_HOST:7000:0
                    output_file                          stdout
                         format                               0
                  platform-file                                
              use_platform_file                               N
         platform-target-sensor                              -1
                     tide-model                                
                 use_tide_model                               N
                  log-directory                               .
                    trn_log_dir                               .
                      make_logs                               N
                  platform-file                               N
                    swath-width                          150.00
             n_output_soundings                             101
        median_filter_threshold                            0.50
         median_filter_n_across                               1
          median_filter_n_along                               1
               median_filter_en                               N
                   n_buffer_max                               1
                    mb1svr_host                       localhost
                    mb1svr_port                           27000
                    trnsvr_host                       localhost
                    trnsvr_port                           28000
                   trnusvr_host                       localhost
                   trnusvr_port                            8000
                 trnumsvr_group                    239.255.0.16
                  trnumsvr_port                           29000
                   trnumsvr_ttl                              64
                   output_flags                             800
                    mbsvr_hbtok                              50
                     mbsvr_hbto                            0.00
                    trnsvr_hbto                            0.00
                   trnusvr_hbto                            0.00
        mbtrnpp_loop_delay_msec                               0
        trn_status_interval_sec                           20.00
             mbtrnpp_stat_flags                               F
                     trn_enable                               N
                   trn_utm_zone                              10
                      trn_mtype                               2
                      trn_ftype                               2
                     trn_fgrade                               1
                    trn_freinit                               1
                    trn_mweight                               4
                   trn_max_ncov                           49.00
                   trn_max_nerr                           50.00
                   trn_max_ecov                           49.00
                   trn_max_eerr                           50.00
                   trn_map_file                          (null)
                   trn_cfg_file                          (null)
             trn_particles_file                          (null)
                trn_mission_dir                          (null)
                       trn_decn                               0
                       trn_decs                            0.00
       covariance_magnitude_max                            5.00
         convergence_repeat_min                             200
             reinit_gain_enable                               N
             reinit_file_enable                               N
         reinit_xyoffset_enable                               N
            reinit_xyoffset_max                            0.00
          reinit_zoffset_enable                               N
             reinit_zoffset_min                            0.00
             reinit_zoffset_max                            0.00
loading config file [/Volumes/wdcs/20210609m1/mbtrn-210609m1.cfg]
WARN - unsupported key/val [trn-reinit/1]
ERR - invalid key/value [trn-reinit/1]
options - post-config:
                           self                     0x10b6c51e0
                        verbose                              -2
                          input     socket:134.89.32.107:7000:0
                         format                              89
                  platform-file                          (null)
         platform-target-sensor                               0
                  log-directory  /Volumes/wdcs/20210609m1/logs/mbtrnpp
                     tide-model                          (null)
                         output  file:mbtrnpp_20210716-232025.mb1
                     projection                               0
                    swath-width                           90.00
                      soundings                              11
                  median-filter                        0.10/9/3
                          mbhbn                              50
                          mbhbt                           15.00
                         trnhbt                           15.00
                        trnuhbt                           15.00
                       trnumttl                              64
                          delay                               0
                        statsec                           30.00
                      statflags                               F/MSF_STATUS|MSF_EVENT|MSF_ASTAT|MSF_PSTAT
                         trn-en                               Y
                        trn-utm                              10
                        trn-map  /Volumes/wdcs/maps/UpperMontereyCanyon5mUTM.grd
                        trn-cfg  /Volumes/wdcs/20210609m1/config/mappingAUV_specs.cfg
                        trn-par  /Volumes/wdcs/20210609m1/config/particles.cfg
                        trn-mid                     mb-2021.197
                      trn-mtype                               1
                      trn-ftype                               2
                     trn-fgrade                               0
                    trn-freinit                               1
                    trn-mweight                               4
                       trn-ncov                           49.00
                       trn-nerr                          200.00
                       trn-ecov                           49.00
                       trn-eerr                          200.00
                         mb-out      mb1svr:134.89.13.166:27000
                        trn-out  trnsvr:134.89.13.166:28000,trnu,trnusvr:134.89.13.166:8000
                       trn-decn                               9
                       trn-decs                            0.00
       covariance-magnitude-max                           50.00
         convergence-repeat-min                              20
             reinit_gain_enable                               Y
             reinit_file_enable                               N
         reinit_xyoffset_enable                               Y
            reinit_xyoffset_max                          150.00
          reinit_zoffset_enable                               Y
             reinit_zoffset_min                           -2.00
             reinit_zoffset_max                            2.00
                           help                               N
options - post-cmdline:
                           self                     0x10b6c51e0
                        verbose                               0
                          input  /Volumes/wdcs/20210609m1/20210609m1.mb89
                         format                              -1
                  platform-file                          (null)
         platform-target-sensor                               0
                  log-directory  /Volumes/wdcs/20210609m1/logs/mbtrnpp
                     tide-model                          (null)
                         output  file:mbtrnpp_20210716-232025.mb1
                     projection                               0
                    swath-width                           90.00
                      soundings                              11
                  median-filter                        0.10/9/3
                          mbhbn                              50
                          mbhbt                           15.00
                         trnhbt                           15.00
                        trnuhbt                           15.00
                       trnumttl                              64
                          delay                               0
                        statsec                           30.00
                      statflags                               F/MSF_STATUS|MSF_EVENT|MSF_ASTAT|MSF_PSTAT
                         trn-en                               Y
                        trn-utm                              10
                        trn-map  /Volumes/wdcs/maps/UpperMontereyCanyon5mUTM.grd
                        trn-cfg  /Volumes/wdcs/20210609m1/config/mappingAUV_specs.cfg
                        trn-par  /Volumes/wdcs/20210609m1/config/particles.cfg
                        trn-mid                     mb-2021.197
                      trn-mtype                               1
                      trn-ftype                               2
                     trn-fgrade                               0
                    trn-freinit                               1
                    trn-mweight                               4
                       trn-ncov                           49.00
                       trn-nerr                          200.00
                       trn-ecov                           49.00
                       trn-eerr                          200.00
                         mb-out      mb1svr:134.89.13.166:27000
                        trn-out  trnsvr:134.89.13.166:28000,trnu,trnusvr:134.89.13.166:8000
                       trn-decn                               9
                       trn-decs                            0.00
       covariance-magnitude-max                           50.00
         convergence-repeat-min                              20
             reinit_gain_enable                               Y
             reinit_file_enable                               N
         reinit_xyoffset_enable                               Y
            reinit_xyoffset_max                          150.00
          reinit_zoffset_enable                               Y
             reinit_zoffset_min                           -2.00
             reinit_zoffset_max                            2.00
                           help                               N

using log directory /Volumes/wdcs/20210609m1/logs/mbtrnpp...

configuration - final:
                           self                     0x10b6c3880
                        verbose                               0
                     input_mode                               2
                          input  /Volumes/wdcs/20210609m1/20210609m1.mb89
              socket_definition    socket:TRN_RESON_HOST:7000:0
                    output_file     mbtrnpp_20210716-232025.mb1
                         format                              -1
                  platform-file                                
              use_platform_file                               N
         platform-target-sensor                               0
                     tide-model                                
                 use_tide_model                               N
                  log-directory  /Volumes/wdcs/20210609m1/logs/mbtrnpp
                    trn_log_dir  /Volumes/wdcs/20210609m1/logs/mbtrnpp
                      make_logs                               Y
                  platform-file                               Y
                    swath-width                           90.00
             n_output_soundings                              11
        median_filter_threshold                            0.10
         median_filter_n_across                               9
          median_filter_n_along                               3
               median_filter_en                               Y
                   n_buffer_max                               3
                    mb1svr_host                   134.89.13.166
                    mb1svr_port                           27000
                    trnsvr_host                   134.89.13.166
                    trnsvr_port                           28000
                   trnusvr_host                   134.89.13.166
                   trnusvr_port                            8000
                 trnumsvr_group                    239.255.0.16
                  trnumsvr_port                           29000
                   trnumsvr_ttl                              64
                   output_flags                             84F
                    mbsvr_hbtok                              50
                     mbsvr_hbto                           15.00
                    trnsvr_hbto                           15.00
                   trnusvr_hbto                           15.00
        mbtrnpp_loop_delay_msec                               0
        trn_status_interval_sec                           30.00
             mbtrnpp_stat_flags                               F
                     trn_enable                               Y
                   trn_utm_zone                              10
                      trn_mtype                               1
                      trn_ftype                               2
                     trn_fgrade                               0
                    trn_freinit                               1
                    trn_mweight                               4
                   trn_max_ncov                           49.00
                   trn_max_nerr                          200.00
                   trn_max_ecov                           49.00
                   trn_max_eerr                          200.00
                   trn_map_file  /Volumes/wdcs/maps/UpperMontereyCanyon5mUTM.grd
                   trn_cfg_file  /Volumes/wdcs/20210609m1/config/mappingAUV_specs.cfg
             trn_particles_file  /Volumes/wdcs/20210609m1/config/particles.cfg
                trn_mission_dir                     mb-2021.197
                       trn_decn                               9
                       trn_decs                            0.00
       covariance_magnitude_max                           50.00
         convergence_repeat_min                              20
             reinit_gain_enable                               Y
             reinit_file_enable                               N
         reinit_xyoffset_enable                               Y
            reinit_xyoffset_max                          150.00
          reinit_zoffset_enable                               Y
             reinit_zoffset_min                           -2.00
             reinit_zoffset_max                            2.00
mconf_init:251 >>> initializing module[id=13] -   MOD_R7KR/0000001C [0]
mconf_init:251 >>> initializing module[id=14] -    MOD_R7K/0000001C [0]
mconf_init:251 >>> initializing module[id=15] -    MOD_S7K/0000000C [0]
mconf_init:251 >>> initializing module[id=16] -    MOD_F7K/0000000C [0]
mconf_init:251 >>> initializing module[id=17] -   MOD_TRNC/0000000C [0]
mconf_init:251 >>> initializing module[id=18] -  MOD_EMU7K/0000000C [0]
mconf_init:251 >>> initializing module[id=19] -  MOD_TBINX/0000001C [0]
mconf_init:251 >>> initializing module[id=20] - MOD_MBTRNPP/0000001C [0]
mconf_init:251 >>> initializing module[id=21] - MOD_MBTNAV/0000001C [0]
mconf_init:251 >>> initializing module[id=22] -  MOD_NETIF/0000001C [0]
mconf_init:253 >>> MM_WARN  00000004
mconf_init:254 >>> MM_DEBUG 00000002
mconf_init:255 >>> MM_ERR   00000008
mconf_init:256 >>> MM_NONE  00000000
mconf_init:257 >>> MM_ALL   FFFFFFFF
mbtrnpp_init_debug:4567 >>> MOD_MBTRNPP[id=20]  en[0000001C] verbose[0]
mbtrnpp_init_debug:4623 >>> MOD_MBTRNPP  en[00000000]
mbtrnpp message log [/Volumes/wdcs/20210609m1/logs/mbtrnpp//mbtrnpp-20210716-232025.log]
     [self     0x7fe6a1d05d40]
     [file     0x7fe6a1d05e30]
        [self     0x7fe6a1d05e30]
        [path     /Volumes/wdcs/20210609m1/logs/mbtrnpp/mbtrnpp-20210716-2320250000.log]
        [fd               -1]
        [flags    0000000000]
        [mode     0000000000]
     [path     /Volumes/wdcs/20210609m1/logs/mbtrnpp/]
     [name     mbtrnpp-20210716-232025]
     [ext             log]
     [cfg      0x7fe6a1d05ea0]
        [self     0x7fe6a1d05ea0]
        [lim_b            64]
        [lim_s            64]
        [lim_t            64]
        [flags            20]
        [dest              2]
        [tfmt     %FT%H:%M:%SZ]
        [del               ,]
     [stime      Wed Dec 31 16:00:00 1969]
     [slen              0]
     [scount            0]
     [scur              0]
trn update log [/Volumes/wdcs/20210609m1/logs/mbtrnpp//trnu-20210716-232025.log]
     [self     0x7fe6a1d061d0]
     [file     0x7fe6a1d05f40]
        [self     0x7fe6a1d05f40]
        [path     /Volumes/wdcs/20210609m1/logs/mbtrnpp/trnu-20210716-2320250000.log]
        [fd               -1]
        [flags    0000000000]
        [mode     0000000000]
     [path     /Volumes/wdcs/20210609m1/logs/mbtrnpp/]
     [name     trnu-20210716-232025]
     [ext             log]
     [cfg      0x7fe6a1d05f60]
        [self     0x7fe6a1d05f60]
        [lim_b            64]
        [lim_s            64]
        [lim_t            64]
        [flags            20]
        [dest              2]
        [tfmt     %FT%H:%M:%SZ]
        [del               ,]
     [stime      Wed Dec 31 16:00:00 1969]
     [slen              0]
     [scount            0]
     [scur              0]
MAPIO::check_error No such file or directory
mapbounds {
	ncid = 0
	xmin = 4057223.341242
	xmax = 4082532.141780
	dx = 9.999526
	ymin = 568365.910144
	ymax = 608617.504799
	dy = 10.000396
}
TRN log directory is /Volumes/wdcs/20210609m1/logs/mbtrnpp/mb-2021.197-TRN
symlink /Volumes/wdcs/20210609m1/logs/mbtrnpp/latestTRN to /Volumes/wdcs/20210609m1/logs/mbtrnpp/mb-2021.197-TRN OK
parseSensorSpecs parsing sensor of type 1.
Sensor 0 is of type 1.
parseSensorSpecs parsing sensor of type 2.
Sensor 1 is of type 2.
parseSensorSpecs parsing sensor of type 5.
Sensor 2 is of type 5.
Opening log /Volumes/wdcs/20210609m1/logs/mbtrnpp/latestTRN/trn.log.0000
File created[/Volumes/wdcs/20210609m1/logs/mbtrnpp/latestTRN/trn.log.0000] returned[0x7fff9a005bf8]
mbtrnpp_init_trn : TRN initialize - OK
trnsvr netif:
                 self    0x7fe6a1e2b9b0
            port_name            trnsvr
                 host     134.89.13.166
                 port             28000
                  ttl                 0
               socket               0x0
                 peer    0x7fe6a3014a00
                list@    0x7fe6a1e2baf0
             list len                 0
              profile    0x7fe6a1e2bb50
              mlog_id                -1
            mlog_path            (null)
              log_dir                 .
                 hbto            15.000
              cmdline            (null)
                 stop                 0
TRN server netif OK [134.89.13.166:28000]
trnusvr netif:
                 self    0x7fe6a1e2c320
            port_name           trnusvr
                 host     134.89.13.166
                 port              8000
                  ttl                 0
               socket               0x0
                 peer    0x7fe6a3015000
                list@    0x7fe6a1e2c420
             list len                 0
              profile    0x7fe6a1e15db0
              mlog_id                -1
            mlog_path            (null)
              log_dir                 .
                 hbto            15.000
              cmdline            (null)
                 stop                 0
TRNU server netif OK [134.89.13.166:8000]
trnumsvr netif:
                 self    0x7fe6a1e2cbd0
            port_name          trnumsvr
                 host      239.255.0.16
                 port             29000
                  ttl                64
               socket               0x0
                 peer    0x7fe6a3025a00
                list@    0x7fe6a1e2ccd0
             list len                 0
              profile    0x7fe6a1e14e30
              mlog_id                -1
            mlog_path            (null)
              log_dir                 .
                 hbto             0.000
              cmdline            (null)
                 stop                 0
TRNUM server netif OK [239.255.0.16:29000]
     [self      0x7fe6a1d06d80]
     [host          (null)]
     [port              -1]
     [utm               10]
     [mtype              1]
     [ftype              2]
     [map_file  /Volumes/wdcs/maps/UpperMontereyCanyon5mUTM.grd]
     [cfg_file  /Volumes/wdcs/20210609m1/config/mappingAUV_specs.cfg]
     [part_file /Volumes/wdcs/20210609m1/config/particles.cfg]
     [log_dir   mb-2021.197]
     [maxNcov             49.000]
     [maxNerr             200.000]
     [maxEcov             49.000]
     [maxEerr             200.000]
configuring MB1 server socket using 134.89.13.166:27000 hbto[15.000000]
mb1svr netif:
                 self    0x7fe6a1c4f710
            port_name            mb1svr
                 host     134.89.13.166
                 port             27000
                  ttl                 0
               socket               0x0
                 peer    0x7fe6a2011e00
                list@    0x7fe6a1c520b0
             list len                 0
              profile    0x7fe6a1c08580
              mlog_id                -1
            mlog_path            (null)
              log_dir                 .
                 hbto            15.000
              cmdline            (null)
                 stop                 0
MB1 server netif OK [134.89.13.166:27000]

Sonar File </Volumes/wdcs/20210609m1/20210609_164631.s7k> of format <89> initialized for reading
2021/06/09-16:46:30.296875 1623257190.296875 | Read 25 non-survey data records...
          1 MB_DATA_HEADER (ID=3): general header
          3 MB_DATA_PARAMETER (ID=9): Parameter record
          6 MB_DATA_HEADING (ID=17): Heading record
          6 MB_DATA_NAV1 (ID=29): Auxiliary nav system 1
          5 MB_DATA_ATTITUDE1 (56): ancillary attitude system 1
          4 MB_DATA_SONARDEPTH (59): HYSWEEP dynamic draft
2021/06/09-16:46:30.777343 1623257190.777343 | Read 25 non-survey data records...
          6 MB_DATA_HEADING (ID=17): Heading record
          6 MB_DATA_NAV1 (ID=29): Auxiliary nav system 1
          7 MB_DATA_ATTITUDE1 (56): ancillary attitude system 1
          6 MB_DATA_SONARDEPTH (59): HYSWEEP dynamic draft
2021/06/09-16:46:31.335937 1623257191.335937 | Read 25 non-survey data records...
          1 MB_DATA_PARAMETER (ID=9): Parameter record
          6 MB_DATA_HEADING (ID=17): Heading record
          7 MB_DATA_NAV1 (ID=29): Auxiliary nav system 1
          7 MB_DATA_ATTITUDE1 (56): ancillary attitude system 1
          4 MB_DATA_SONARDEPTH (59): HYSWEEP dynamic draft
2021/06/09-16:46:31.816406 1623257191.816406 | Read 25 non-survey data records...
          7 MB_DATA_HEADING (ID=17): Heading record
          6 MB_DATA_NAV1 (ID=29): Auxiliary nav system 1
          6 MB_DATA_ATTITUDE1 (56): ancillary attitude system 1
          6 MB_DATA_SONARDEPTH (59): HYSWEEP dynamic draft
--reinit time_d:1623257192.230743 centered on offset: 0.000000 0.000000 0.000000
2021/06/09-16:46:34.896498 1623257194.896499 | -121.878200   36.779408   61.286 | 0 filtered beams - Ping not used - failed bias estimate
2021/06/09-16:46:37.895652 1623257197.895652 | -121.878241   36.779422   61.308 | 0 filtered beams - Ping not used - failed bias estimate
2021/06/09-16:46:40.894656 1623257200.894657 | -121.878281   36.779436   61.337 | 0 filtered beams - Ping not used - failed bias estimate
2021/06/09-16:46:43.893507 1623257203.893508 | -121.878322   36.779450   61.407 | 11 filtered beams - Ping not used - failed bias estimate
---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
YYYY/MM/DD-HH:MM:SS.SSSSSS TTTTTTTTTT.TTTTTT | Nav: Easting  Northing     Z     | TRN: Easting  Northing     Z     | Off: East   North     Z   | Cov: East     North       Z   :     Mag   | Best Off: T      E      N      Z    |   Ncs   Nct   Nus   Nut  Nr | Use 
---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
2021/06/09-16:46:46.892256 1623257206.892257 |  600088.262 4070994.808   61.499 |  600208.320 4070966.138   61.124 |  120.058  -28.670  -0.375 | 26643.843 46851.839 34314.001 : 63893.974 |     0.000000   0.000   0.000  0.000 |     0     0     1     1   1 |    
2021/06/09-16:46:49.891128 1623257209.891128 |  600084.550 4070996.215   61.552 |  600194.377 4070971.539   61.084 |  109.827  -24.676  -0.468 | 24097.092 41372.757 30268.089 : 56643.906 |     2.998871   0.000   0.000  0.000 |     0     0     2     2   1 |    
2021/06/09-16:46:52.889941 1623257212.889942 |  600080.849 4070997.297   61.540 |  600184.827 4070975.749   61.019 |  103.978  -21.548  -0.521 | 21936.744 37419.507 27616.436 : 51420.889 |     5.997685   0.000   0.000  0.000 |     0     0     3     3   1 |    
2021/06/09-16:46:55.888530 1623257215.888530 |  600077.084 4070998.155   61.473 |  600176.886 4070977.496   60.870 |   99.803  -20.659  -0.603 | 20165.495 34235.989 25649.870 : 47293.403 |     8.996273   0.000   0.000  0.000 |     0     0     4     4   1 |    
2021/06/09-16:46:58.887064 1623257218.887065 |  600073.148 4070998.890   61.393 |  600170.495 4070978.572   60.711 |   97.347  -20.318  -0.682 | 18749.536 31669.661 24294.388 : 44099.091 |    11.994808   0.000   0.000  0.000 |     0     0     5     5   1 |    
2021/06/09-16:47:01.885647 1623257221.885647 |  600069.271 4070999.617   61.317 |  600164.855 4070979.749   60.560 |   95.583  -19.867  -0.757 | 17457.747 29421.237 23131.529 : 41297.092 |    14.993390   0.000   0.000  0.000 |     0     0     6     6   1 |    
2021/06/09-16:47:04.883769 1623257224.883770 |  600065.363 4071000.510   61.262 |  600158.254 4070980.290   60.446 |   92.890  -20.220  -0.816 | 16335.043 27658.378 22002.739 : 38935.075 |    17.991513   0.000   0.000  0.000 |     0     0     7     7   1 |    
2021/06/09-16:47:07.882150 1623257227.882151 |  600061.693 4071001.555   61.252 |  600151.927 4070980.513   60.397 |   90.234  -21.042  -0.855 | 15342.768 26078.599 21023.111 : 36843.793 |    20.989894   0.000   0.000  0.000 |     0     0     8     8   1 |    
2021/06/09-16:47:10.880646 1623257230.880647 |  600057.893 4071002.812   61.321 |  600145.320 4070980.155   60.466 |   87.427  -22.657  -0.855 | 14292.820 24593.679 20046.798 : 34799.538 |    23.988390   0.000   0.000  0.000 |     0     0     9     9   1 |    
2021/06/09-16:47:13.879518 1623257233.879518 |  600054.167 4071004.164   61.491 |  600139.810 4070980.439   60.617 |   85.644  -23.725  -0.874 | 13290.933 23204.223 19180.873 : 32908.824 |    26.987261   0.000   0.000  0.000 |     0     0    10    10   1 |    
```

***

__trnc : TRN MB1 client__

```
./src/mbtrn/trnc --host=192.168.1.101

TRN test client

use:
trnc [options]
--verbose=n    : verbose output, n>0
--help         : output help message
--version      : output version info
--host=ip:n    : TRN server host
--hbeat=n      : hbeat interval (packets)
--blocking=0|1 : blocking receive [0:1]
--bsize=n      : buffer size

output:

ts[1623257198.229] ping[014749] lat[36.7794] lon[-121.8782]
sd[  61.31] hdg[  5.12] nb[000]
     0000 [ 4d 42 31 00 3c 00 00 00 df f9 a3 1b 3c 30 d8 41 ]
     0010 [ 02 00 0c 32 c4 63 42 40 02 00 24 3d 35 78 5e c0 ]
     0020 [ 00 00 00 c0 ce a7 4e 40 01 00 00 80 51 7b 14 40 ]
     0030 [ 9e 39 00 00 00 00 00 00 69 0e 00 00             ]

...

ts[1623257201.894] ping[014760] lat[36.7794] lon[-121.8783]
sd[  61.36] hdg[  5.14] nb[000]
     0000 [ 4d 42 31 00 3c 00 00 00 3f 90 8e 1c 3c 30 d8 41 ]
     0010 [ 03 00 98 c3 c4 63 42 40 04 00 d6 11 36 78 5e c0 ]
     0020 [ 00 00 00 80 71 af 4e 40 00 00 00 40 5a 94 14 40 ]
     0030 [ a9 39 00 00 00 00 00 00 4a 0e 00 00             ]

ts[1623257202.228] ping[014761] lat[36.7794] lon[-121.8783]
sd[  61.37] hdg[  5.14] nb[000]
     0000 [ 4d 42 31 00 54 01 00 00 7d e8 a3 1c 3c 30 d8 41 ]
     0010 [ 02 00 40 d0 c4 63 42 40 03 00 1e 24 36 78 5e c0 ]
     0020 [ 00 00 00 00 02 b0 4e 40 00 00 00 20 f1 94 14 40 ]
     0030 [ aa 39 00 00 0a 00 00 00 62 00 00 00 00 00 00 40 ]
     0040 [ 18 ff 0b c0 00 00 00 80 ae b2 41 c0 00 00 00 00 ]
     0050 [ c6 f8 47 40 7b 00 00 00 00 00 00 40 68 06 0c c0 ]
     0060 [ 00 00 00 60 74 a2 39 c0 00 00 00 00 09 ff 47 40 ]
     0070 [ 94 00 00 00 00 00 00 e0 3e 00 0c c0 00 00 00 40 ]
     0080 [ 7c 3f 31 c0 00 00 00 40 c2 f9 47 40 ad 00 00 00 ]
     0090 [ 00 00 00 c0 3b fc 0b c0 00 00 00 a0 84 67 23 c0 ]
     00a0 [ 00 00 00 00 53 f6 47 40 c6 00 00 00 00 00 00 40 ]
     00b0 [ c4 11 0c c0 00 00 00 60 58 d9 04 c0 00 00 00 00 ]
     00c0 [ c3 08 48 40 df 00 00 00 00 00 00 e0 ca 08 0c c0 ]
     00d0 [ 00 00 00 20 c1 92 11 40 00 00 00 c0 13 01 48 40 ]
     00e0 [ f8 00 00 00 00 00 00 80 ca 08 0c c0 00 00 00 80 ]
     00f0 [ c4 2c 27 40 00 00 00 80 13 01 48 40 11 01 00 00 ]
     0100 [ 00 00 00 c0 a9 20 0c c0 00 00 00 20 eb 5e 33 40 ]
     0110 [ 00 00 00 40 84 15 48 40 2a 01 00 00 00 00 00 80 ]
     0120 [ 8f 37 0c c0 00 00 00 60 a0 28 3c 40 00 00 00 40 ]
     0130 [ 1f 29 48 40 43 01 00 00 00 00 00 c0 de 48 0c c0 ]
     0140 [ 00 00 00 60 18 4e 43 40 00 00 00 80 f1 37 48 40 ]
     0150 [ bf 4d 00 00                                     ]

```

***

__mbtnav-cli : TRN test client__

```
 ./src/mbtrn/mbtnav-cli --host=134.89.13.166:8000 --ofmt=c
 
use:
TRN test client

trnc [options]
--verbose=n    : verbose output, n>0
--help         : output help message
--version      : output version info
--host=ip:n    : TRN server host
--hbeat=n      : hbeat interval (packets)
--blocking=0|1 : blocking receive [0:1]
--bsize=n      : buffer size
--ofmt=a|c|h   : output formats (one or more of a:ascii c:csv h:hex)

output:
[csv]
1626383880.477,1623257373.177,4071152.9645,599771.2570,72.5562,1623257373.177,4071061.8326,599903.1688,66.7895,4071060.1554,599880.8251,66.5907,79.027,82.587,3.632,1,0,1,540,15274,0,0,1623257373.177,1.6772,22.3437,0.1987,1623257206.892,0.0000,0.0000,0.0000,0,0,56,56
1626383880.956,1623257376.176,4070854.6929,599961.1321,61.8045,1623257376.176,4071065.2909,599897.5215,67.0524,4071061.4418,599877.1146,66.7812,77.297,82.161,3.639,1,0,1,549,15283,0,0,1623257376.176,3.8491,20.4069,0.2712,1623257206.892,0.0000,0.0000,0.0000,0,0,57,57
1626383881.382,1623257379.175,4071156.7029,599777.5722,72.3252,1623257379.175,4071068.5018,599892.2533,67.2665,4071062.6919,599873.4718,66.9454,75
[hex]
     0000 [ 00 54 44 53 29 75 f8 22 3c 30 d8 41 88 50 0c c7 ]
     0010 [ 2c 0f 4f 41 11 8f e2 62 fb 4f 22 41 00 00 00 00 ]
     0020 [ 43 a0 4e 40 00 00 00 00 00 00 00 00 00 00 00 00 ]
     0030 [ 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ]
     0040 [ 00 00 00 00 29 75 f8 22 3c 30 d8 41 8d 03 43 02 ]
     0050 [ 11 0f 4f 41 9e f6 b4 1a 45 50 22 41 95 e5 88 b1 ]
     0060 [ ce 06 4e 40 00 00 00 00 00 00 00 00 00 00 00 00 ]
     0070 [ 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ]
     0080 [ 00 00 00 00 29 75 f8 22 3c 30 d8 41 ea ff 2d fa ]
     0090 [ 1a 0f 4f 41 d6 08 ac 86 a2 50 22 41 1d b2 b0 6e ]
     00a0 [ ef 1d 4e 40 81 0b d6 53 cb 5c db 40 6c 04 4e d9 ]
     00b0 [ 11 8e d4 40 87 09 72 65 06 01 1e 40 d5 cc 47 71 ]
     00c0 [ c2 86 d0 40 29 75 f8 22 3c 30 d8 41 00 00 9e 50 ]
     00d0 [ de cc 41 c0 00 a0 38 2f 79 e4 54 40 60 bc e9 29 ]
     00e0 [ 72 4a f0 bf 81 0b d6 53 cb 5c db 40 6c 04 4e d9 ]
     00f0 [ 11 8e d4 40 87 09 72 65 06 01 1e 40 d5 cc 47 71 ]
     0100 [ c2 86 d0 40 bd 1a b9 1d 3c 30 d8 41 00 00 00 00 ]
     0110 [ 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ]
     0120 [ 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ]
     0130 [ 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ]
     0140 [ 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 00 ]
     0150 [ 00 00 00 00 01 00 00 00 00 00 00 00 6c 00 00 00 ]
     0160 [ f6 39 00 00 00 00 00 00 00 00 00 00 08 00 00 00 ]
     0170 [ 08 00 00 00 29 75 f8 22 3c 30 d8 41 00 00 00 00 ]
     0180 [ 00 00 00 00 61 11 04 8d 29 3c d8 41             ]
[ascii]
     [self         0x7ffee4105e10]
     [ pt ]
      [self         0x7ffee4105e14]
      [time         1623257254.871]
      [x               4071013.756]
      [y                600028.343]
      [z                    61.862]
      [cov[0]               0.000]
      [cov[1]               0.000]
      [cov[2]               0.000]
      [cov[3]               0.000]
     [ mle ]
      [self         0x7ffee4105e54]
      [time         1623257254.871]
      [x               4070959.964]
      [y                600236.555]
      [z                    56.116]
      [cov[0]               0.000]
      [cov[1]               0.000]
      [cov[2]               0.000]
      [cov[3]               0.000]
     [ mse ]
      [self         0x7ffee4105e94]
      [time         1623257254.871]
      [x               4070971.843]
      [y                600102.673]
      [z                    60.560]
      [cov[0]           18244.019]
      [cov[1]           15945.380]
      [cov[2]               8.502]
      [cov[3]            9706.703]
     [ offset ]
      [self         0x7ffee4105ed4]
      [time         1623257254.871]
      [x                   -41.913]
      [y                    74.330]
      [z                    -1.302]
      [cov[0]           18244.019]
      [cov[1]           15945.380]
      [cov[2]               8.502]
      [cov[3]            9706.703]
     [ last useful ]
      [self         0x7ffee4105f14]
      [time         1623257206.892]
      [x                     0.000]
      [y                     0.000]
      [z                     0.000]
      [cov[0]               0.000]
      [cov[1]               0.000]
      [cov[2]               0.000]
      [cov[3]               0.000]
     [reinit                     1]
     [reinit_t               0.000]
     [filt_state                 0]
     [success                    1]
     [is_converged               0]
     [is_valid                   0]
     [mb1_cycle                189]
     [ping_number            14919]
     [mb1_time      1623257254.871]
     [update_time   1626383991.520]
     [n_con_seq                  0]
     [n_con_tot                  0]
     [n_uncon_seq               17]
     [n_uncon_tot               17]
     [reinit_time            0.000]
     
```

***

__trnu-cli : demo/test trnu_cli API__

- Supports sync or async UDP clients

```
# sync client
./src/mbtrnav/trnu-cli --host=192.168.1.101:8000 

# async client
trnucli-test --host=<trnsvr IP>[:<port>] --input=S --ofmt=p --async=3000

use:
TRNU client (trnu_cli) test

trnucli-test [options]
 --verbose     : verbose output
 --help        : output help message
 --version     : output version info
 --host=[ip:port] : TRNU server host:port
 --mcast=[ip:port:ttl] : TRNU mcast server host:port, ttl
 --input=[bcs] : input type (B:bin C:csv S:socket)
 --ofmt=[pcx]  : output format (P:pretty X:hex PX:pretty_hex C:csv
 --serr        : send updates to stderr (default is stdout)
 --ifile       : input file
 --hbtos       : heartbeat period (sec, 0.0 to disable)

trncli API options:
 --block=[lc]  : block on connect/listen (L:listen C:connect)
 --update=n    : TRN update N
 --demo=n      : use trn_cli handler mechanism, w/ periodic TRN resets (mod n)
 --test-reset=n : enable periodic TRN resets (mod n)

trncli_ctx API options:
 --rctos=n     : reconnect timeout sec (reconnect if no message received for n sec)
 --nddelms=n   : delay n ms on listen error
 --ltoms=n     : listen timeuot msec
 --rcdelms=n   : delay n ms on reconnect error
 --no-log      : disable client logging
 --logstats=f  : async client stats log period (sec, <=0.0 to disable)
 --async=n     : use asynchronous implementation, show status every n msec

 Example:
 # async client
 trnucli-test --host=<trnsvr IP>[:<port>] --input=S --ofmt=p --async=3000

 Defaults:
mcast group  239.255.0.16
mcast port       29000
       ttl          32
 trnu host   127.0.0.1
 trnu port        8000
 cli_flags            00000000
 ctx_flags            00000001
     input         svr
      ofmt      pretty
     hbtos         0.0
    update          10
     rctos      10.000
     ltoms          50
   nddelms          50
   rcdelms        5000
  logstats      60.000
     ofile      stdout

output:
trnucli_connect OK [0]
                 addr   0x7fe8d0404310
             mb1_time   1623257291.528
          update_time   1626384315.173
          reinit_time            0.000
                 sync         53445400
         reinit_count                1
         reinit_tlast            0.000
         filter_state                0
              success                1
         is_converged                0
             is_valid                0
            mb1_cycle              297
          ping_number            15029
            n_con_seq                0
            n_con_tot                0
          n_uncon_seq               29
          n_uncon_tot               29
           estimates:
                  [0]   1623257291.528,pt,4071028.237,599982.428,63.106,0.000,0.000,0.000,0.000
                  [1]   1623257291.528,mle,4070870.736,599982.780,61.499,0.000,0.000,0.000,0.000
                  [2]   1623257291.528,mmse,4070967.593,600035.574,61.700,13598.216,10063.259,9.981,3145.697
                  [3]   1623257291.528,offset,-60.645,53.146,-1.407,13598.216,10063.259,9.981,3145.697
                  [4]   1623257206.892,last_good,0.000,0.000,0.000,0.000,0.000,0.000,0.000
      Bias Estimates:
              OFFSET: -60.645,53.146,-1.407
                LAST: 0.000,0.000,0.000
                MMSE: -60.645,53.146,-1.407
                 COV: 116.611,100.316,3.159

                 addr   0x7fe8d0404310
             mb1_time   1623257294.528
          update_time   1626384315.604
          reinit_time            0.000
                 sync         53445400
         reinit_count                1
         reinit_tlast            0.000
         filter_state                0
              success                1
         is_converged                0
             is_valid                0
            mb1_cycle              306
          ping_number            15038
            n_con_seq                0
            n_con_tot                0
          n_uncon_seq               30
          n_uncon_tot               30
           estimates:
                  [0]   1623257294.528,pt,4071029.299,599978.533,63.140,0.000,0.000,0.000,0.000
                  [1]   1623257294.528,mle,4070909.782,599967.995,63.223,0.000,0.000,0.000,0.000
                  [2]   1623257294.528,mmse,4070970.174,600031.790,61.776,13263.800,9795.689,9.935,2794.570
                  [3]   1623257294.528,offset,-59.125,53.256,-1.364,13263.800,9795.689,9.935,2794.570
                  [4]   1623257206.892,last_good,0.000,0.000,0.000,0.000,0.000,0.000,0.000
```

***

__trn-cli : demo/test trn_cli/trnw API__

- mb1 client
- requests updates from TRN server
- optionally initializes TRN server
- input from mb1 file, csv file or mb1 server


```
./src/mbtrnav/trn-cli --thost=192.168.1.101:28000 --input=T  --mhost=192.168.1.101:27000

use:
TRN client (trn_cli) test

trncli-test [options]
--verbose   : verbose output
--help      : output help message
--version   : output version info
--thost     : TRN server host
--mhost     : MB server host
--input     : input type (M:mb1 C:csv T:trnc)
--ifile     : input file
--map       : TRN server map file (dir for tiles)
--cfg       : TRN server config file
--particles : TRN server particle file
--logdir    : TRN server log directory
--ftype     : TRN server filter type
--mtype     : TRN server map type D:DEM B:BO
--utm       : UTM zone
--update    : TRN update N
--hbeat     : trn server heartbeat (modulus)
--test-api  : query status (modulus)
--log       : enable logging


output:
connect OK
init OK
s_trncli_test_trnc:831 connecting
s_trnc_read_mb1_rec - read [340/14396] 
ts[1623257274.198] beams[10] ping[14977] 
lat[36.77971] lon[-121.87930] hdg[5.08] sd[62.5]

s_trnc_read_mb1_rec - read [368/14396] 
ts[1623257274.531] beams[11] ping[14978] 
lat[36.77971] lon[-121.87931] hdg[5.08] sd[62.5]

...
s_trnc_read_mb1_rec - read [368/14396] 
ts[1623257277.196] beams[11] ping[14986] 
lat[36.77973] lon[-121.87934] hdg[5.07] sd[62.7]


	Bias Estimates:
	MLE: 1623257277.20,79.9550,-53.2928,3.2956
	MSE: 1623257277.20,-46.3682,50.1529,-1.0403
	COV:[94.87,91.43,3.17
```

***

__udpc : generic UDP client/demo__

- useful as TRN UDP (trnusvr) diagnostic

```
./src/mbtrn/udpc --verbose --host=192.168.1.101

use:

UDP client

udpc [options]
--verbose  : verbose output
--help     : output help message
--version  : output version info
--port     : UDP server port
--blocking : blocking receive [0:1]
--host     : UDP server host

output:

verbose [Y]
host    [192.168.1.101]
port    [27000]
block   [N]
cycles  [-1]
connect [192.168.1.101:27000]
sendto OK [4]
fd[3] waiting for server (non-blocking)...
msock_recvfrom returned -1 [35/Resource temporarily unavailable]
msock_recvfrom error [35/Resource temporarily unavailable]
sendto OK [4]
fd[3] waiting for server (non-blocking)...
fd[3] received 4 bytes
sendto OK [4]
fd[3] waiting for server (non-blocking)...
fd[3] received 128 bytes
sendto OK [4]
```

***

__MbTrnUpd8Test : MVC TRN UDP update client__

- Test TRN ingest on MVC 

```
$AUV/bin/mbTrnUpd8Test -m 134.89.32.111 -t 0

************************************************************
MbTrnUpd8Test - Log mbtrn trn update data every 3 seconds

mbTrnUpd8Server -m 134.89.13.166 -t 0
: mbTrnUpd8Server MbTrnUpd8IFServer -- running

: mbTrnUpd8 - ip:134.89.13.166  port:8000  to:0 ms
logfile already exists!
I will try to append a 1 to your file
logfile already exists!
I will try to append a 2 to your file
logfile already exists!
I will try to append a 3 to your file
logfile already exists!
I will try to append a 4 to your file
logfile already exists!
I will try to append a 5 to your file
logfile already exists!
I will try to append a 6 to your file
logfile already exists!
I will try to append a 7 to your file
: recv timeout ms is 0
: MbTrnUpd8::init() SO_RCVTIMEO to : 2 secs
: MbTrnUpd8::init() SO_RCVTIMEO set to : 0 ms
: MbTrnUpd8::initialize() Jul 10 2021 01:34:58 - Ready to connect to server 134.89.13.166/8000
: MbTrnUpd8::sendmsg(REQ)
: MbTrnUpd8::sendmsg() - sent 4 bytes
: MbTrnUpd8::recvmsg() - read 4 bytes, 1st 41
: MbTrnUpd8::connect() - got ACK from server - connected!
: MbTrnUpd8::init() SO_RCVTIMEO to : 0 0
: MbTrnUpd8::init() SO_RCVTIMEO set to : 0 ms
: mbTrnUpd8 - connected with mbtrn
: MbTrnUpd8::recvmsg() - read 396 bytes, 1st 0
: MbTrnUpd8: Structure size is 396

: MbTrnUpd8: Record length is 396

: MbTrnUpd8: Record is 396

: MbTrnUpd8::process() - mb1 time: 1623257264.53
: 
MbTrnUpd8 record: ping  : 14948  ping time: 1623257264.53	  update time: 1626477637.08
                  reinit: 1  filter OK: 0  	  measure  OK: 1
                  cycle : 216  is valid : 0

: MbTrnUpd8 estimates:        PT    	MLE	MMSE
                     times: 1623257264.53	1623257264.53	1623257264.53
                         X: 4071017.23	4070938.44	4070973.84
                         Y: 600015.96	600093.21	600081.39
                         Z: 61.81	58.93	60.61
                    covarX: 0.00	0.00	14661.48
                    covarY: 0.00	0.00	14062.88
                    covarZ: 0.00	0.00	8.60
                   covarXY: 0.00	0.00	7222.65

: MbTrnUpd8::recvmsg() - read 396 bytes, 1st 0
: MbTrnUpd8: Structure size is 396

: MbTrnUpd8: Record length is 396

: MbTrnUpd8: Record is 396

: MbTrnUpd8::process() - mb1 time: 1623257267.53
: 
MbTrnUpd8 record: ping  : 14957  ping time: 1623257267.53	  update time: 1626477637.53
                  reinit: 1  filter OK: 0  	  measure  OK: 1
                  cycle : 225  is valid : 0

: MbTrnUpd8 estimates:        PT    	MLE	MMSE
                     times: 1623257267.53	1623257267.53	1623257267.53
                         X: 4071018.41	4070755.39	4070973.14
                         Y: 600012.25	599968.44	600076.16
                         Z: 61.99	59.08	60.76
                    covarX: 0.00	0.00	14190.61
                    covarY: 0.00	0.00	13682.01
                    covarZ: 0.00	0.00	8.63
                   covarXY: 0.00	0.00	6839.30

: MbTrnUpd8::recvmsg() - read 396 bytes, 1st 0
: MbTrnUpd8: Structure size is 396

: MbTrnUpd8: Record length is 396

: MbTrnUpd8: Record is 396

: MbTrnUpd8::process() - mb1 time: 1623257270.53

```

***

__MbTrnUpdRecv : MVC TRN MB1 update client__

- Typically for simulation (use MbTrnUpd8 for operations)
- Requires separate TRN server instance

```
$AUV/bin/mbTrnRecvTest -m 134.89.32.111

Output
: MbTrnRecv::sendmsg(REQ)
: MbTrnRecv::sendmsg() - sent 4 bytes
: MbTrnRecv::recvmsg() failure: 11
MbTrnRecv::recvmsg(): Resource temporarily unavailable
: MbTrnRecv::connect() - failed
: MbTrnRecv::sendmsg(REQ)
: MbTrnRecv::sendmsg() - sent 4 bytes
: MbTrnRecv::recvmsg() - read 4 bytes, 1st 41
: MbTrnRecv::connect() - got ACK from server - connected!
: MbTrnRecv::init() SO_RCVTIMEO to : 0 500000
: MbTrnRecv::init() SO_RCVTIMEO set to : 500 ms
: MbTrnRecv::run() - connected!
: MbTrnRecv::recvmsg() - read 368 bytes, 1st 4d
: MbTrnRecv: Packet 180 is ts : 1623257246.209 , ping # 14893, with 11 beams at 62.058 depth

: MbTrnRecv::recvmsg() failure: 11
MbTrnRecv::recvmsg(): Resource temporarily unavailable
: MbTrnRecv::recvmsg() - read 4 bytes, 1st 41
: MbTrnRecv::process() - packet size 4 - non-MB1 msg!
: MbTrnRecv::recvmsg() - read 340 bytes, 1st 4d
: MbTrnRecv: Packet 183 is ts : 1623257246.542 , ping # 14894, with 10 beams at 62.057 depth

: MbTrnRecv::recvmsg() failure: 11
MbTrnRecv::recvmsg(): Resource temporarily unavailable
: MbTrnRecv::recvmsg() - read 368 bytes, 1st 4d
: MbTrnRecv: Packet 185 is ts : 1623257246.875 , ping # 14895, with 11 beams at 62.060 depth

```

