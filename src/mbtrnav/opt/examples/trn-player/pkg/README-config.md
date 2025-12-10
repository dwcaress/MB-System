# Config File Reference

A summary of trn-player configuration file options is included below.
See the TRN Documentation Quick Start Guide for additional details.

## Syntax

* key = value pairs
* leading/trailing whitespace OK 
* supports terrainAid.cfg and/or trn-player options
* semicolon terminations (terrainAid.cfg) optional
* comments may start with '#' or '//''
* environment variables using $[A-Za-z0-9_-.] (no braces) will be expanded

## trn-player Options

```
  Option  : Description
 ----------------------------------------------------------------
 cdir     : config directory
 config   : config file path
 ddir     : data directory
 dperiod  : decimation period (ms) alias: samplePeriod
            >0  : Decimates records to match specified 
                  input period as nearly as possible
            <=0 : Disabled (use all records)

 eofile   : estimate output file name (w/o path)
 ftype    : TRN filter type
              0: TRN_FT_NONE
              1: TRN_FT_POINTMASS
              2: TRN_FT_PARTICLE (default)
              3: TRN_FT_BANK
              values other then 2 are experimental

 fstat    : force beam status true (TerrainNav.log)
 oflags   : output flags (may include multiple)
              p: pretty
              m: measurement CSV
              e: estimate CSV
              q: quiet
              S: output MMSE
              L: output MLE
              B: output both MLE, MMSE

 interp  : map interpolation method (DEM maps only)
              0: nearest-neighbor (no interpolation)
              1: bilinear
              2: bicubic

 iformat : input format
              0: MbTrn.log
              1: TerrainNav.log
              2: TerrainAid.log
              3: DVL CSV [1,2]
              4: IDT CSV [1,2]
              5: MB/Generic CSV [1,2]

              [1] implies moformat = iformat; use -Z --moformat to override
              [2] no default name; use -O, --mofile to set/enable

 ifile    : log/input name (override lookup based on type)
 mdir     : map directory
 mfile    : map file name (w/o path)
 nfile    : navigation log file name (w/o path)
 odir     : TRN output directory
 mofile   : measurement output file name
 pfomode  : particles file logging mode
              0: HISTOGRAM (distribution summary)
              1: PARTICLES (all particles; large data volume)

              log full particle states or a histogram
              Note: generates a large volume of data
              writes to odir/filterDistrib.txt

 pfile    : particles file name (w/o path)
 reinits  : allow TRN particle filter reinits
 stype    : Bathymetry data format passed to 
            measurement update; may differ from sensor of origin
            Corresponds to terrainAid.cfg sensor_type
              0: UNDEFINED
              1: TRN_SENSOR_DVL    DVL 
              2: TRN_SENSOR_MB     Multibeam 
              3: TRN_SENSOR_PENCIL Single Beam
              4: TRN_SENSOR_HOMER  Homer Relative Measurement
              5: TRN_SENSOR_DELTAT Imagenex DeltaT

 vfile    : vehicle spec file name (w/o path)<
 mweight  : set modified weighting scheme
              0: TRN_WT_NONE No modification
              1: TRN_WT_NORM  Shandor's original alpha modification
              2: TRN_WT_XBEAM Crossbeam with original
              3: TRN_WT_SUBCL Subcloud with original
              4: TRN_FORCE_SUBCL Force Subcloud every measurement
              5: TRN_WT_INVAL Force invalid

 mtype    : map file format
              0: UNDEFINED
              1: TRN_MAP_DEM Digital Elevation Map (.GRD)
              2: TRN_MAP_BO  Binary Octree Map (.BO)

 moformat : measurement output CSV file format (input file format enum)
              Use -Z, --moformat to override default (== iformat)
              i.e. to convert DVL, IDT to MB/Generic format

              No default name

              Use -Z, --mofile to enable measurement output, set file name
```

Defaults: 
 - sensor   : TRN\_SENSOR\_MB
 - mfile    : PortTiles
 - vfile    : mappingAUV_specs.cfg
 - ifile    : Mbtrn.log
 - nfile    : navigation.log
 - mdir     : ./maps
 - cdir     : ./data
 - ddir     : ./data
 - iformat  : IOFMT_MBTRN
 - oflags   : pS
 - odir     : trnplayer[-TRN.n]
 - moformat : same as input format for CSV input; IOFMT\_MB otherwise
 - eofile   : none
 - mofile   : none
 
Notes:
 - config files support terrainAid.cfg and long opts above
 - CLI supports terrainAid.cfg options e.g. --mapFileName
 - sensor spec and vehicle spec files must be in same directory
 - use -h -v to view configuration summary (w/ help) and exit

## terrainAid.cfg Options

```
 Option               : Description (trn-player alias)
 ----------------------------------------------------------------
 mapFileName          : map file name (mfile)
 particlesName        : particles file name (pfile)
 vehicleCfgName       : vehicle spec file name (vfile)
 dvlCfgName           : DVL spec file name
 resonCfgName         : Multibeam spec file name
 lrauvDvlFilename     : LRAUV DVL spec file name
 map_type             : TRN map format (mtype)
 filterType           : TRN filter type (ftype)
 allowFilterReinits   : allow TRN filter reinitializations if true (reinits)
 useModifiedWeighting : use modified weighting scheme (mweight) [1]
 mapInterpMethod      : map interpolation method (interp)
 forceLowGradeFilter  : Use low grade filter (flgf) [2]
 terrainNavServer     : trn-server host IP address
 terrainNavPort       : trn-server host IP port
 samplePeriod         : decimation period (msec) (dperiod)
 maxNorthingCov       : TRN convergence criteria
 maxNorthingError     : TRN convergence criteria
 maxEastingCov        : TRN convergence criteria
 maxEastingError      : TRN convergence criteria
 RollOffset           : roll angle (phi) bias (deg)
 useIDTData           : process as IDT
 useDVLSide           : use dvlSide.log instead of navigation.log
 useMbTrnData         : use MbTrn log
 useMbTrnServer       : use MbTrn server


 Notes:
 [1] Modified Weighting:
     pmf grid resolution is specified in the ones digit as follows (0 to 5)
      0 == default
      1 == 1
      2 == 2
      3 == 3
      4 == 4
      5 = 0.5
     Examples:
      use == 125, grid res == 0.5
      use == 123, grid res == 3
     Number of filters is calculated using the 10's digit
     as follows (0 to 5)
      0 == default
      1 == 1
      2 == 2
      3 == 4
      4 == 10
      5 == 11
     pmf grid size is specified using the 100's digit
     as follows (0 to 2):
      0 == 100
      1 == 200
      2 == 300
     Examples:
      useModifiedWeighting = 15;  1  Numfilt=1, Pmf grid .5m
      useModifiedWeighting = 11;  2  Numfilt=1, Pmf grid 1 m
      useModifiedWeighting = 21;  3  Numfilt=2, Pmf grid 1 m
      useModifiedWeighting = 51;  4  Numfilt=11,Pmf grid 1 m
      useModifiedWeighting = 41;  5  Numfilt=10,Pmf grid 1 m

 [2] forceLowGradeFilter:
      if true,filter performs angular rate
      integration and estimates attitude and position.
      if false, filter will choose its settings based on
      the kearfott system is available.

```

### CSV estimate output format

When a TRN estimate output file is named (-E, --eofile), TRN estimates are written 
to the specifed file in the output directory. 
The CSV field format is as follows:

```
 field indices
 [  1] mse time (epoch sec, double)
 [  2] mse N (m)
 [  3] mse E (m)
 [  4] mse D (m)
 [  5] mse vx (m/s)
 [  6] mse vy (m/s)
 [  7] mse vz (m/s)
 [  8] mse roll (phi) (rad)
 [  9] mse pitch (theta) (rad)
 [ 10] mse rheading (psi) (rad)
 [ 11] nav (pt) time (epoch sec, double)
 [ 12] nav N (m)
 [ 13] nav E (m)
 [ 14] nav D (m)
 [ 15] nav roll (phi) (rad)
 [ 16] nav pitch (theta) (rad)
 [ 17] nav rheading (psi) (rad)
 [ 18] offset N (mse - pt) (m)
 [ 19] offset E (mse - pt) (m)
 [ 20] offset D (mse - pt) (m)
 [ 21] covariance N (m)
 [ 22] covariance E (m)
 [ 23] covariance D (m)
 [ 24] covariance magnitude (m)
```

### CSV measurement output formats

#### DVL CSV fields

```
 field indices
 [  1] = time(i);
 [  2] = auvN(i);
 [  3] = auvE(i);
 [  4] = depth(i);
 [  5] = yaw(i);   heading/psi
 [  6] = pitch(i); pitch/theta
 [  7] = roll(i);  roll/phi
 [  8] = 0;        flag(0)
 [  9] = 0;        flag(0)
 [ 10] = 0;        flag(0)
 [ 11] = vx(i);
 [ 12] = vy(i);
 [ 13] = vz(i);
 [ 14] = dvlvalid;
 [ 15] = bottomlock;
 [ 16] = numbeams;
 [ 17] = beam_number[i]
 [ 18] = beamStatus[i]
 [ 19] = range[i];
 ...
 (fixed number of beams (4), 3 fields per beam)
```

### IDT CSV Fields

```
 field indices
 [  1] = time(i);
 [  2] = auvN(i);
 [  3] = auvE(i);
 [  4] = depth(i);
 [  5] = yaw(i);   heading/psi
 [  6] = pitch(i); pitch/theta
 [  7] = roll(i);  roll/phi
 [  8] = 0;        flag(0)
 [  9] = 0;        flag(0)
 [ 10] = 0;        flag(0)
 [ 11] = vx(i);
 [ 12] = vy(i);
 [ 13] = vz(i);
 [ 14] = dvlvalid;
 [ 15] = bottomlock;
 [ 16] = numbeams;
 [ 17] = beam_number[i]
 [ 18] = beamStatus[i]
 [ 19] = range[i];
 ...
 (fixed number of beams (120), 3 fields per beam)
```

### MB CSV Fields

```
 field indices
 [  1] = time(i);
 [  2] = auvN(i);
 [  3] = auvE(i);
 [  4] = depth(i);
 [  5] = yaw(i);   heading/psi
 [  6] = pitch(i); pitch/theta
 [  7] = roll(i);  roll/phi
 [  8] = 0;        flag(0)
 [  9] = 0;        flag(0)
 [ 10] = 0;        flag(0)
 [ 11] = vx(i);
 [ 12] = vy(i);
 [ 13] = vz(i);
 [ 14] = dvlvalid;
 [ 15] = bottomlock;
 [ 16] = numbeams;
 [ 17] = beam_number
 [ 18] = beamStatus[i]
 [ 19] = range[i];
 [ 20] = alongTrack[i];
 [ 21] = crossTrack[i];
 [ 22] = altitudes[i]
 (variable number of beams, 6 fields per beam)
```

### oflags Description

The oflags parameter is a character string to control
trn-output content and format. 

Values include: empSLB
Defaults are 'pS' (pretty output with MMSE)

p: pretty

Formatted console output:

```
MMSE [t,x,y,z,s] : 63898130419.000, 4061708.231, 594373.492, 67.741, 1
 OFS   [x,y,z] :  +39.921,  +40.644,  +25.505
 COV [x,y,z,m] :    1.646,    1.675,    1.622, 2.855

```

m: measurement CSV

Measurement (TRN inputs) in CSV format specified by moformat.
If moformat is unspecified, uses input format.
If input type is a CSV format, moformat defaults to input type
but may be overridden by specifying moformat explicitly.

Example: --moformat=1 (DVL) 

```
63898130270.000,4061675.360,594219.527,16.263,0.167,-0.192,0.396,0,0,0,0.100,0.000,0.000,1,1,4,0,1,115.917969,1,1,70.320312,2,1,77.679688,3,1,96.435547
```

e: estimate CSV

TRN Estimate in CSV format

```
 Format fields:
 mse time (epoch sec, double)
 mse N,E,D (m)
 mse vx, vy, vz (m/s)
 mse roll, pitch, heading (phi, theta, psi) (rad)
 nav (pt) time (epoch sec, double)
 nav N,E,D (m)
 nav roll, pitch, heading (phi, theta, psi) (rad)
 offset (mse - pt) x, y, z (m)
 covariance x,y,z,magnitude (m)
```

S: output MMSE in pretty output

L: output MLE in pretty output

B: output both MLE, MMSE in pretty output

