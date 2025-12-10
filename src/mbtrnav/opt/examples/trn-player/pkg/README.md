# README

trn-player distribution contains binaries and example data for libtrnav trn-player example.

Contents:

- README         : this file
- README-config  : config file reference
- trnplayer.cfg  : configuration file
- bin/trn-player : trn-player executable
- share          : README files
- libtrnav-data  : example data and maps
    * libtrnav-data/maps       : PortTiles map
    * libtrnav-data/data/mbtrn : MbTrn.log and config
    * libtrnav-data/data/tnav  : TerrainNav.log and config
    * libtrnav-data/data/csv   : CSV logs and config

# Install

Unzip the distribution tar.gz
(creates trn-player directory)

```
tar xzvf trnplayer-<version>.tar.gz
cd trn-player
```

# Run example data

By default, the configuration file included runs an example in libtrnav-data/trn-player/data/mbtrn.
From the trn-player top directory:

```
./bin/trn-player --config=trnplayer.cfg -v
```

The run other data set configurations in the config file, comment all but the selected configuration.

# Run your data

Create a configuration file or copy trnplayer.cfg.
Comment out any existing configurations.
Set the following options:

// MB data mbtrn/MbTrn.log
// data (input) directory
ddir

// config directory
cdir

// map directory
mdir

// map file name (w/o path)
mfile

// vehicle spec file name (w/o path)
vfile

// input file name (w/o path)
ifile

// input file format
iformat

// bathymtry data format passed to TRN
// (may differ from sensor of origin)
// Same as terrainAid.cfg sensor_type option.
stype

// estimate output CSV file name (not output if unset)
eofile

// measurement output CSV file name (not output if unset)
mofile

// measurement output CSV file format (input file format enum)
// Implies moformat = iformat; use -Z --moformat to override
// No default name; use -O, --mofile to set/enable
moformat

// decimate data to use specified input period (msec)
// samplePeriod is also valid.
dperiod

Example (DVL data 300_4800short300.csv)

```
ddir = ./libtrnav-data/trn-player/data/tnav
cdir = ./libtrnav-data/trn-player/data/tnav
mdir = ./libtrnav-data/trn-player/maps
mfile = PortTiles
vfile = RDIequippedAUV_specs.cfg
ifile = 300_4800short300.csv
iformat = 3
stype = 1
eofile = est.csv
dperiod = 3000
fstat
```

# Config File Reference

See the TRN documentation Quick Start Guide and
README-config.md for additional details
