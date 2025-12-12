///
/// @file TrnPlayerCtx.cpp
/// @authors k. headley
/// @date 2025-09-16
///
/// Copyright 2025 Monterey Bay Aquarium Research Institute
/// see LICENSE file for terms of use and license information.
///
/// @brief Configuration class for Minimal TRN example application (TrnPlayer)
/// @details Parses command line options and contains configuration
/// and instance state variables used by TrnPlayer. Supports config files that
/// support terrainAid.cfg files; terrainAid.cfg parameters are also accepted
/// on the command line (as long opts).
///
/// Use trn-player -h option for description of options
/// @ see parse_opts

#ifndef _TRN_PLAYER_CTX_HPP_
#define _TRN_PLAYER_CTX_HPP_

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <getopt.h>
#include <ostream>

// for basename
#include <libgen.h>
#include "DataLogReader.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>

// parser debug output macros
#define CTX_DPRINT(fmt,...) if(TrnPlayerCtx::gdebug(false) > 1)fprintf(stderr, fmt, __VA_ARGS__)
#define CTX_DMSG(fmt) if(TrnPlayerCtx::gdebug(false) > 1)fprintf(stderr, fmt)

#define WIN_DECLSPEC

// convert binary to character
#define BIN2CH(b) (b ? 'Y' : 'N')

// buffer size (paths, e.g.)
#define BUF_SZ ((int)256)
#define LBUF_SZ ((int)2048)

// default log output directory basename
// e.g. trn-player-logs-TRN.N
#define TRN_LOGDIR_DFL "trnplayer"

// quit if MAX_ERRORS occur
#define MAX_ERRS ((int)3)

// Log type values
typedef enum {
    IOFMT_MBTRN = 0,
    IOFMT_TRNNAV,
    IOFMT_TRNAID,
    IOFMT_CSV_DVL,
    IOFMT_CSV_IDT,
    IOFMT_CSV_MB,
    IOFMT_NAV,
} IOFormats;

// TRN Estimate type values
typedef enum {
    EST_BOTH = 0,
    EST_MLE = TRN_EST_MLE,
    EST_MMSE = TRN_EST_MMSE,
} EstTypes;

// Output flags
typedef enum {
    OUT_PRETTY = 0x1,
    OUT_EST_CSV = 0x2,
    OUT_MEAS_CSV = 0x4,
    OUT_MMSE = 0x100,
    OUT_MLE = 0x200,
    OUT_EST_FILE = 0x400,
    OUT_MEAS_FILE = 0x800
} OFlags;

// TRN particle filter output modes
typedef enum {
    PFO_HISTOGRAM = HISTOGRAMTOFILE,
    PFO_PARTICLES = PARTICLESTOFILE,
    PFO_NONE = 2
} PFOModes;

// TrnPlayer configuration
class TrnPlayerCtx {

public:
    // Default constructor
    TrnPlayerCtx()
    : filter_type(2)
    , map_type(2)
    , sensor_type(2)
    , mod_weight(TRN_WT_NONE)
    , reinit_en(false)
    , map_interp(0)
    , force_lgf(false)
    , dec_period_ms(0)
    , dec_prev_time(0.)
    , input_format(IOFMT_MBTRN)
    , meas_out_format(IOFMT_CSV_MB)
    , verbose(false)
    , debug(0)
    , is_config_set(false)
    , is_particles_set(false)
    , is_help_set(false)
    , force_status(false)
    , last_meas(false)
    , oflags(OUT_PRETTY | OUT_MMSE)
    , pf_omode(PFO_NONE)
    , trn_log(NULL)
    , nav_log(NULL)
    , csv_log(NULL)
    , meas_out(NULL)
    , est_out(NULL)
    , part_out(NULL)
    {
        memset(ddir, 0, BUF_SZ);
        memset(odir, 0, BUF_SZ);
        memset(mdir, 0, BUF_SZ);
        memset(cdir, 0, BUF_SZ);
        memset(eofile, 0, BUF_SZ);
        memset(mfile, 0, BUF_SZ);
        memset(nfile, 0, BUF_SZ);
        memset(pfile, 0, BUF_SZ);
        memset(vfile, 0, BUF_SZ);
        memset(ifile, 0, BUF_SZ);
        memset(mofile, 0, BUF_SZ);
        memset(pfile, 0, BUF_SZ);
        memset(pfofile, 0, BUF_SZ);
        memset(dpath, 0, BUF_SZ);
        memset(eopath, 0, BUF_SZ);
        memset(mpath, 0, BUF_SZ);
        memset(vpath, 0, BUF_SZ);
        memset(npath, 0, BUF_SZ);
        memset(mopath, 0, BUF_SZ);
        memset(ppath, 0, BUF_SZ);
        memset(pfopath, 0, BUF_SZ);
        geo_dr[0] = geo_dr[1] = geo_dr[2] = 0.;
        geo_dt[0] = geo_dt[1] = geo_dt[2] = 0.;

        // set default map, config, data directories
        const char *pwd = (getenv("PWD") == NULL ? "." : getenv("PWD"));

        snprintf(mdir, BUF_SZ, "%s/maps", pwd);
        snprintf(cdir, BUF_SZ, "%s/data", pwd);
        snprintf(ddir, BUF_SZ, "%s/data", pwd);

        // set default map, vehicle spec (for example)
        snprintf(mfile, BUF_SZ, "PortTiles");
        snprintf(vfile, BUF_SZ, "mappingAUV_specs.cfg");
        snprintf(pfofile, BUF_SZ, "filterDistrib.txt");

        // init ranges : 0: range MIN 1: range MAX
        ping_range[0] = std::numeric_limits<int>::max();
        ping_range[1] = std::numeric_limits<int>::min();
        time_range[0] = std::numeric_limits<double>::max();;
        time_range[1] = std::numeric_limits<double>::min();
    }

    // Destructor
    ~TrnPlayerCtx()
    {
        if(csv_log)
            fclose(csv_log);
        if(meas_out)
            fclose(meas_out);
        if(est_out)
            fclose(est_out);
        if(part_out)
            delete part_out;
    }

    // hack to enable CTX_NDEBUG macros to use static variable
    // instead of compile time option
    static int gdebug(bool set, int val=0) {
        static int dval = false;
        if(set)
            dval = val;
        return dval;
    }

    // Convert output flags to string
    static char *oflag_str(char *dest, size_t len, unsigned int oflags)
    {
        if(dest == NULL || len <= 60)
            return NULL;

        memset(dest, 0, len);
        int rem = len;
        char *op = dest;
        int bp = 0;

        if((oflags & OUT_PRETTY) != 0) {
            if((bp = snprintf(op, rem, "PRETTY")) > 0) {
                op += bp;
                rem -= bp;
            }
        }
        if((oflags & OUT_EST_CSV) != 0) {
            const char *c = (op > dest ? " |" : "");
            if((bp = snprintf(op, rem, "%s EST_CSV", c)) > 0) {
                op += bp;
                rem -= bp;
            }
        }
        if((oflags & OUT_MEAS_CSV) != 0) {
            const char *c = (op > dest ? " |" : "");
            if((bp = snprintf(op, rem, "%s MEAS_CSV", c)) > 0) {
                op += bp;
                rem -= bp;
            }
        }
        if((oflags & OUT_MMSE) != 0) {
            const char *c = (op > dest ? " |" : "");
            if((bp = snprintf(op, rem, "%s MMSE", c)) > 0) {
                op += bp;
                rem -= bp;
            }
        }
        if((oflags & OUT_MLE) != 0) {
            const char *c = (op > dest ? " |" : "");
            if((bp = snprintf(op, rem, "%s MLE", c)) > 0) {
                op += bp;
                rem -= bp;
            }
        }
        if((oflags & OUT_MEAS_FILE) != 0) {
            const char *c = (op > dest ? " |" : "");
            if((bp = snprintf(op, rem, "%s MEAS_FILE", c)) > 0) {
                op += bp;
                rem -= bp;
            }
        }
        if((oflags & OUT_EST_FILE) != 0) {
            const char *c = (op > dest ? " |" : "");
            if((bp = snprintf(op, rem, "%s EST_FILE", c)) > 0) {
                op += bp;
                rem -= bp;
            }
        }
        return dest;
    }

    // Return log name for specified type
    static const char *log_name(int input_format) {
        static const char *log_names[] = {"MbTrn.log", "TerrainNav.log", "TerrainAid.log", "dvl.csv", "idt.csv", "mb.csv", "navigation.log"};

        return log_names[abs(input_format) % 7];
    }

    // Output TrnPlayerCtx summary to console
    static void show(TrnPlayerCtx *ctx)
    {
        if(ctx == NULL)
            return;

        const char *iofmts[] = {"IOFMT_MBTRN", "IOFMT_TRNNAV", "IOFMT_TRNAID", "IOFMT_CSV_DVL", "IOFMT_CSV_IDT", "IOFMT_CSV_MB"};
        const char *stypes[] = {"UNDEFINED", "TRN_SENSOR_DVL", "TRN_SENSOR_MB", "TRN_SENSOR_PENCIL", "TRN_SENSOR_HOMER", "TRN_SENSOR_DELTAT"};
        const char *mtypes[] = {"UNDEFINED", "TRN_MAP_DEM", "TRN_MAP_BO"};
        const char *ftypes[] = {"TRN_FT_NONE", "TRN_FT_POINTMASS", "TRN_FT_PARTICLE", "TRN_FT_BANK"};
        const char *wtypes[] = {"TRN_WT_NONE", "TRN_WT_NORM", "TRN_WT_XBEAM", "TRN_WT_SUBCL", "TRN_FORCE_SUBCL", "TRN_WT_INVAL"};
        const char *Itypes[] = {"NONE/NEAREST", "BILINEAR", "BUCUBIC", "SPILNE"};
        const char *pfmodes[] = {"HISTOGRAM", "PARTICLES","NONE"};

        char obuf[80] = {0};
        char dbuf[BUF_SZ] = {0};
        char cbuf[BUF_SZ] = {0};
        char mbuf[BUF_SZ] = {0};
        ssize_t len[3] = {0};

        len[0] = readlink(ctx->ddir, dbuf, sizeof(dbuf) - 1);
        len[1] = readlink(ctx->cdir, cbuf, sizeof(cbuf) - 1);
        len[2] = readlink(ctx->mdir, mbuf, sizeof(mbuf) - 1);

        int wkey = 10, wval=0;

        fprintf(stderr, "\n");
        fprintf(stderr, " --- Config Summary ---\n");
        fprintf(stderr, " %*s : %*s\n", wkey, "mpath", wval, ctx->mpath);
        fprintf(stderr, " %*s : %*s\n", wkey, "vpath", wval, ctx->vpath);
        fprintf(stderr, " %*s : %*s\n", wkey, "dpath", wval, ctx->dpath);
        fprintf(stderr, " %*s : %*s\n", wkey, "npath", wval, ctx->npath);
        fprintf(stderr, " %*s : %*s\n", wkey, "mopath", wval, ctx->mopath);
        fprintf(stderr, " %*s : %*s\n", wkey, "eopath", wval, ctx->eopath);
        fprintf(stderr, " %*s : %*s\n", wkey, "ppath", wval, ctx->ppath);
        fprintf(stderr, " %*s : %*s\n", wkey, "pfopath", wval, ctx->pfopath);
        fprintf(stderr, " %*s : %*s\n", wkey, "ddir", wval, ctx->ddir);
        fprintf(stderr, " %*s : %*s\n", wkey, "mdir", wval, ctx->mdir);
        fprintf(stderr, " %*s : %*s\n", wkey, "cdir", wval, ctx->cdir);
        fprintf(stderr, " %*s : %s -> %s\n", wkey, "dlink", "data", (len[0] > 0 ? dbuf : ctx->ddir));
        fprintf(stderr, " %*s : %s -> %s\n", wkey, "clink", "config", (len[1] > 0 ? cbuf : ctx->cdir));
        fprintf(stderr, " %*s : %s -> %s\n", wkey, "mlink", "maps", (len[2] > 0 ? mbuf : ctx->mdir));
        fprintf(stderr, " %*s : %*s\n", wkey, "odir", wval, ctx->odir);
        fprintf(stderr, " %*s : %d (%s)\n", wkey, "iformat", ctx->input_format, iofmts[(ctx->input_format % 6)]);
        fprintf(stderr, " %*s : %d (%s)\n", wkey, "stype", ctx->sensor_type, stypes[(ctx->sensor_type % 6)]);
        fprintf(stderr, " %*s : %d (%s)\n", wkey, "ftype", ctx->filter_type, ftypes[(ctx->filter_type % 4)]);
        fprintf(stderr, " %*s : %d (%s)\n", wkey, "mtype", ctx->map_type, mtypes[(ctx->map_type % 3)]);
        fprintf(stderr, " %*s : %d (%s)\n", wkey, "moformat", ctx->meas_out_format, iofmts[(ctx->meas_out_format%6)]);
        fprintf(stderr, " %*s : [%.3lf, %.3lf, %.3lf]\n", wkey, "geo_dr", ctx->geo_dr[0], ctx->geo_dr[1], ctx->geo_dr[2]);
        fprintf(stderr, " %*s : [%.3lf, %.3lf, %.3lf]\n", wkey, "geo_dt", ctx->geo_dt[0], ctx->geo_dt[1], ctx->geo_dt[2]);
        fprintf(stderr, " %*s : %c\n", wkey, "reinit", BIN2CH(ctx->reinit_en));
        fprintf(stderr, " %*s : %d (%s)\n", wkey, "mod_wt", ctx->mod_weight, wtypes[(ctx->mod_weight % 6)]);
        fprintf(stderr, " %*s : %d (%s)\n", wkey, "interp", ctx->map_interp, Itypes[(ctx->map_interp % 4)]);
        fprintf(stderr, " %*s : %c\n", wkey, "fstat", BIN2CH(ctx->force_status));
        fprintf(stderr, " %*s : %c\n", wkey, "flgf", BIN2CH(ctx->force_lgf));
        fprintf(stderr, " %*s : %ld\n", wkey, "dperiod", ctx->dec_period_ms);
        fprintf(stderr, " %*s : x%04x (%s)\n", wkey, "oflags", ctx->oflags, oflag_str(obuf, 80, ctx->oflags));
        fprintf(stderr, " %*s : %d (%s)\n", wkey, "pf_omode", ctx->pf_omode, pfmodes[ctx->pf_omode % 3]);
        fprintf(stderr, " %*s : %d\n", wkey, "debug", ctx->debug);
        fprintf(stderr, "\n");
    }

    static void show_help(const char *bname="trn-player")
    {
        fprintf(stderr, "\n");
        fprintf(stderr, " %s : TRN Log Player\n", bname);
        fprintf(stderr, "\n");
        fprintf(stderr, " Description:\n");
        fprintf(stderr, "  Demonstrates libtrnav core classes and data structures.\n");
        fprintf(stderr, "  Process TRN logs and write TRN inputs and/or output in various formats.\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "  Input Formats:               Output Formats:\n");
        fprintf(stderr, "    MbTrn.log                    Measurement CSV (DVL, IDT, Multibeam)\n");
        fprintf(stderr, "    TerrainNav.log               Estimate CSV\n");
        fprintf(stderr, "    TerrainAid.log               Pretty (MMSE and/or MLE estimates, offset, covariance)\n");
        fprintf(stderr, "    CSV_DVL\n");
        fprintf(stderr, "    CSV_IDT\n");
        fprintf(stderr, "    CSV_MB (Multibeam)\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "  Use: %s [options...]\n", bname);
        fprintf(stderr, "\n");
        fprintf(stderr, "  options:\n");
        fprintf(stderr, "    -c <s>, --cdir     : config directory\n");
        fprintf(stderr, "    -C <s>, --config   : config file path\n");
        fprintf(stderr, "    -d <s>, --ddir     : data directory\n");
        fprintf(stderr, "    --dperiod          : decimation period (ms) alias: samplePeriod\n");
        fprintf(stderr, "                         >0  : Decimates records to match specified\n");
        fprintf(stderr, "                               input period as nearly as possible\n");
        fprintf(stderr, "                         <=0 : Disabled (use all records)\n");
        fprintf(stderr, "    -D <d>, --debug    : debug output\n");
        fprintf(stderr, "    -E <s>, --eofile   : estimate output file name (w/o path)\n");
        fprintf(stderr, "    -f <d>, --ftype    : TRN filter type\n");
        fprintf(stderr, "                          0: TRN_FT_NONE\n");
        fprintf(stderr, "                          1: TRN_FT_POINTMASS\n");
        fprintf(stderr, "                          2: TRN_FT_PARTICLE (default)\n");
        fprintf(stderr, "                          3: TRN_FT_BANK\n");
        fprintf(stderr, "                         values other then 2 are experimental\n");
        fprintf(stderr, "    -F, --fstat        : force beam status true (TerrainNav.log)\n");
        fprintf(stderr, "    -G <s>, --oflags   : output flags (may include multiple)\n");
        fprintf(stderr, "                          p: pretty\n");
        fprintf(stderr, "                          m: measurement CSV\n");
        fprintf(stderr, "                          e: estimate CSV\n");
        fprintf(stderr, "                          q: quiet\n");
        fprintf(stderr, "                          S: output MMSE\n");
        fprintf(stderr, "                          L: output MLE\n");
        fprintf(stderr, "                          B: output both MLE, MMSE\n");
        fprintf(stderr, "    -h, --help         : print help message\n");
        fprintf(stderr, "    --interp           : map interpolation method (DEM maps only)\n");
        fprintf(stderr, "                          0: nearest-neighbor (no interpolation)\n");
        fprintf(stderr, "                          1: bilinear\n");
        fprintf(stderr, "                          2: bicubic\n");
        fprintf(stderr, "                          3: spline\n");
        fprintf(stderr, "    -i <d>, --iformat  : input format\n");
        fprintf(stderr, "                          0: MbTrn.log\n");
        fprintf(stderr, "                          1: TerrainNav.log\n");
        fprintf(stderr, "                          2: TerrainAid.log\n");
        fprintf(stderr, "                          3: DVL CSV [1,2]\n");
        fprintf(stderr, "                          4: IDT CSV [1,2]\n");
        fprintf(stderr, "                          5: MB/Generic CSV [1,2]\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "                          [1] implies moformat = iformat; use -Z --moformat to override\n");
        fprintf(stderr, "                          [2] no default name; use -O, --mofile to set/enable\n");
        fprintf(stderr, "    -I <s>, --ifile    : log/input name (override lookup based on type)\n");
        fprintf(stderr, "    -m <s>, --mdir     : map directory\n");
        fprintf(stderr, "    -M <s>, --mfile    : map file name (w/o path)\n");
        fprintf(stderr, "    -N <s>, --nfile    : navigation log file name (w/o path)\n");
        fprintf(stderr, "    -o <s>, --odir     : TRN output directory\n");
        fprintf(stderr, "    -O <s>, --mofile   : measurement output file name\n");
        fprintf(stderr, "    --pfomode          : particles file logging mode\n");
        fprintf(stderr, "                          -1: NONE\n");
        fprintf(stderr, "                           0: HISTOGRAM (distribution summary)\n");
        fprintf(stderr, "                           1: PARTICLES (all particles; large data volume)\n");
        fprintf(stderr, "    -P <s>, --pfile    : particles file name (w/o path)\n");
        fprintf(stderr, "    -r <b>, --reinits  : allow TRN particle filter reinits\n");
        fprintf(stderr, "    -s <d>, --stype    : Bathymetry data format passed to \n");
        fprintf(stderr, "                         measurement update; may differ from sensor of origin\n");
        fprintf(stderr, "                         Corresponds to terrainAid.cfg sensor_type\n");
        fprintf(stderr, "                          0: UNDEFINED\n");
        fprintf(stderr, "                          1: TRN_SENSOR_DVL    DVL \n");
        fprintf(stderr, "                          2: TRN_SENSOR_MB     Multibeam \n");
        fprintf(stderr, "                          3: TRN_SENSOR_PENCIL Single Beam\n");
        fprintf(stderr, "                          4: TRN_SENSOR_HOMER  Homer Relative Measurement\n");
        fprintf(stderr, "                          5: TRN_SENSOR_DELTAT Imagenex DeltaT\n");
        fprintf(stderr, "    -v, --verbose      : verbose output\n");
        fprintf(stderr, "    -V <s>, --vfile    : vehicle spec file name (w/o path)<\n");
        fprintf(stderr, "    -w <d>, --mweight  : set modified weighting scheme\n");
        fprintf(stderr, "                          0: TRN_WT_NONE No modification\n");
        fprintf(stderr, "                          1: TRN_WT_NORM  Shandor's original alpha modification\n");
        fprintf(stderr, "                          2: TRN_WT_XBEAM Crossbeam with original\n");
        fprintf(stderr, "                          3: TRN_WT_SUBCL Subcloud with original\n");
        fprintf(stderr, "                          4: TRN_FORCE_SUBCL Force Subcloud every measurement\n");
        fprintf(stderr, "                          5: TRN_WT_INVAL Force invalid\n");
        fprintf(stderr, "    -x <d>, --mtype    : map file format\n");
        fprintf(stderr, "                          0: UNDEFINED\n");
        fprintf(stderr, "                          1: TRN_MAP_DEM Digital Elevation Map (.GRD)\n");
        fprintf(stderr, "                          2: TRN_MAP_BO  Binary Octree Map (.BO)\n");
        fprintf(stderr, "    -Z <d>, --moformat : measurement output CSV file format (input file format enum)\n");
        fprintf(stderr, "                         Implies moformat = iformat; use -Z --moformat to override\n");
        fprintf(stderr, "                         No default name; use -O, --mofile to set/enable\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "Defaults:\n");
        fprintf(stderr, " - sensor   : TRN_SENSOR_MB\n");
        fprintf(stderr, " - mfile    : PortTiles\n");
        fprintf(stderr, " - vfile    : mappingAUV_specs.cfg\n");
        fprintf(stderr, " - ifile    : Mbtrn.log\n");
        fprintf(stderr, " - mdir     : ./maps\n");
        fprintf(stderr, " - cdir     : ./data\n");
        fprintf(stderr, " - ddir     : ./data\n");
        fprintf(stderr, " - iformat  : IOFMT_MBTRN\n");
        fprintf(stderr, " - oflags   : pS\n");
        fprintf(stderr, " - pf_omode : SAVE_PARTICLES (compilation default)\n");
        fprintf(stderr, " - odir     : trnplayer[-TRN.n]\n");
        fprintf(stderr, " - moformat : same as input format for CSV input; IOFMT_MB otherwise\n");
        fprintf(stderr, " - eofile   : NONE\n");
        fprintf(stderr, " - mofile   : NONE\n");
        fprintf(stderr, " - pfile    : NONE\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "Notes:\n");
        fprintf(stderr, " - config files support terrainAid.cfg and long opts above\n");
        fprintf(stderr, " - CLI supports terrainAid.cfg options e.g. --mapFileName\n");
        fprintf(stderr, " - sensor spec and vehicle spec files must be in same directory\n");
        fprintf(stderr, " - use -h -v to view configuration summary (w/ help) and exit\n");
        fprintf(stderr, " - these terrainAid.cfg options are supported:\n");
        fprintf(stderr, "    mapFileName\n");
        fprintf(stderr, "    particlesName\n");
        fprintf(stderr, "    vehicleCfgName\n");
        fprintf(stderr, "    map_type\n");
        fprintf(stderr, "    filterType\n");
        fprintf(stderr, "    forceLowGradeFilter\n");
        fprintf(stderr, "    allowFilterReinits\n");
        fprintf(stderr, "    useModifiedWeighting\n");
        fprintf(stderr, "    samplePeriod\n");
        fprintf(stderr, "    useDVLSide\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "\n");
    }

    static int parse_cmdline(int argc, char **argv, TrnPlayerCtx *ctx)
    {
        extern char WIN_DECLSPEC *optarg;
        int option_index=0;
        int c;
        static int pass=0;
        if(ctx->debug>0)
            fprintf(stderr, "%s:%4d --- pass[%d]\n", __func__, __LINE__, pass);
        pass++;

        static struct option options[] =
        {
            {"cdir", required_argument, NULL, 0},
            {"config", required_argument, NULL, 0},
            {"ddir", required_argument, NULL, 0},
            {"debug", required_argument, NULL, 0},
            {"dperiod", required_argument, NULL, 0},
            {"eofile", required_argument, NULL, 0},
            {"ftype", required_argument, NULL, 0},
            {"fstat", no_argument, NULL, 0},
            {"help", no_argument, NULL, 0},
            {"interp", required_argument, NULL, 0},
            {"iformat", required_argument, NULL, 0},
            {"ifile", required_argument, NULL, 0},
            {"mdir", required_argument, NULL, 0},
            {"mfile", required_argument, NULL, 0},
            {"nfile", required_argument, NULL, 0},
            {"mtype", required_argument, NULL, 0},
            {"mweight", required_argument, NULL, 0},
            {"odir", required_argument, NULL, 0},
            {"mofile", required_argument, NULL, 0},
            {"moformat", required_argument, NULL, 0},
            {"oflags", required_argument, NULL, 0},
            {"pfile", required_argument, NULL, 0},
            {"pfomode", required_argument, NULL, 0},
            {"reinits", required_argument, NULL, 0},
            {"stype", required_argument, NULL, 0},
            {"verbose", no_argument, NULL, 0},
            {"vfile", required_argument, NULL, 0},
            // terrainAid.cfg support options
            {"mapFileName", required_argument, NULL, 0},
            {"particlesName", required_argument, NULL, 0},
            {"vehicleCfgName", required_argument, NULL, 0},
            {"dvlCfgName", required_argument, NULL, 0},
            {"resonCfgName", required_argument, NULL, 0},
            {"terrainNavServer", required_argument, NULL, 0},
            {"lrauvDvlFilename", required_argument, NULL, 0},
            {"map_type", required_argument, NULL, 0},
            {"filterType", required_argument, NULL, 0},
            {"terrainNavPort", required_argument, NULL, 0},
            {"forceLowGradeFilter", required_argument, NULL, 0},
            {"allowFilterReinits", required_argument, NULL, 0},
            {"useModifiedWeighting", required_argument, NULL, 0},
            {"samplePeriod", required_argument, NULL, 0},
            {"maxNorthingCov", required_argument, NULL, 0},
            {"maxNorthingError", required_argument, NULL, 0},
            {"maxEastingCov", required_argument, NULL, 0},
            {"maxEastingError", required_argument, NULL, 0},
            {"RollOffset", required_argument, NULL, 0},
            {"useIDTData", required_argument, NULL, 0},
            {"useDVLSide", no_argument, NULL, 0},
            {"useMbTrnData", required_argument, NULL, 0},
            {"useMbTrnServer", required_argument, NULL, 0},
            {"mapInterpMethod", required_argument, NULL, 0},
            {"allowFilterReinits", required_argument, NULL, 0},

            {NULL, 0, NULL, 0}
        };
        // reset optind
        optind=1;

        // process argument list
        while ((c = getopt_long(argc, argv, "c:C:d:D:E:f:FG:hi:I:m:M:N:o:O:P:r:s:vV:w:x:z:Z:", options, &option_index)) != -1) {

            switch (c) {
                case 0:
                    //                    fprintf(stderr, "%s:%d - optarg[%s]\n", __func__, __LINE__, optarg );

                    // process options that may be set during processing
                    // verbose
                    if (strcmp("verbose", options[option_index].name) == 0) {
                        ctx->verbose = true;
                    }
                    // debug
                    else if (strcmp("debug", options[option_index].name) == 0) {
                        sscanf(optarg, "%d", &ctx->debug);
                    }
                    // help
                    else if (strcmp("help", options[option_index].name) == 0) {
                        ctx->is_help_set = true;
                    }
                    if(!ctx->is_config_set){
                        // config not processced
                        // if specified:
                        // set flag, get config file path, stop processing
                        if (strcmp("config", options[option_index].name) == 0) {
                            snprintf(ctx->cpath, BUF_SZ,"%s",optarg);
                            ctx->is_config_set = true;
                        }
                        break;
                    } else {
                        // config file not set or already processed
                        // process long options
                        // (translate to short option)

                        // cdir
                        if (strcmp("cdir", options[option_index].name) == 0) {
                            c = 'c';
                        }
                        else if(strcmp("ddir", options[option_index].name) == 0) {
                            c = 'd';
                        }
                        else if(strcmp("dperiod", options[option_index].name) == 0) {
                            c = 1130;
                        }
                        else if(strcmp("debug", options[option_index].name) == 0) {
                            c = 'D';
                        }
                        else if(strcmp("eofile", options[option_index].name) == 0) {
                            c = 'E';
                        }
                        else if(strcmp("fstat", options[option_index].name) == 0) {
                            c = 'F';
                        }
                        else if(strcmp("ftype", options[option_index].name) == 0) {
                            c = 'f';
                        }
                        else if(strcmp("help", options[option_index].name) == 0) {
                            c = 'h';
                        }
                        else if(strcmp("interp", options[option_index].name) == 0) {
                            c = 1240;
                        }
                        else if(strcmp("iformat", options[option_index].name) == 0) {
                            c = 'i';
                        }
                        else if(strcmp("ifile", options[option_index].name) == 0) {
                            c = 'I';
                        }
                        else if(strcmp("mdir", options[option_index].name) == 0) {
                            c = 'm';
                        }
                        else if(strcmp("mweight", options[option_index].name) == 0) {
                            c = 'w';
                        }
                        else if(strcmp("mtype", options[option_index].name) == 0) {
                            c = 'x';
                        }
                        else if(strcmp("mfile", options[option_index].name) == 0) {
                            c = 'M';
                        }
                        else if(strcmp("mofile", options[option_index].name) == 0) {
                            c = 'O';
                        }
                        if (strcmp("moformat", options[option_index].name) == 0) {
                            c = 'Z';
                        }
                        else if(strcmp("nfile", options[option_index].name) == 0) {
                            c = 'N';
                        }
                        else if(strcmp("odir", options[option_index].name) == 0) {
                            c = 'o';
                        }
                        else if(strcmp("oflags", options[option_index].name) == 0) {
                            c = 'G';
                        }
                        else if(strcmp("pfile", options[option_index].name) == 0) {
                            c = 'P';
                        }
                        else if(strcmp("pfomode", options[option_index].name) == 0) {
                            c = 1250;
                        }
                        else if(strcmp("reinits", options[option_index].name) == 0) {
                            c = 'r';
                        }
                        else if(strcmp("stype", options[option_index].name) == 0) {
                            c = 's';
                        }
                        else if(strcmp("verbose", options[option_index].name) == 0) {
                            c = 'v';
                        }
                        else if(strcmp("vfile", options[option_index].name) == 0) {
                            c = 'V';
                        }
                        else if(strcmp("mapFileName", options[option_index].name) == 0) {
                            c = 'M';//1000;
                        }
                        else if(strcmp("particlesName", options[option_index].name) == 0) {
                            c = 'P';//1010;
                        }
                        else if(strcmp("vehicleCfgName", options[option_index].name) == 0) {
                            c = 'V';//1020;
                        }
                        else if(strcmp("dvlCfgName", options[option_index].name) == 0) {
                            c = 1030;
                        }
                        else if(strcmp("resonCfgName", options[option_index].name) == 0) {
                            c = 1040;
                        }
                        else if(strcmp("terrainNavServer", options[option_index].name) == 0) {
                            c = 1050;
                        }
                        else if(strcmp("lrauvDvlFilename", options[option_index].name) == 0) {
                            c = 1060;
                        }
                        else if(strcmp("map_type", options[option_index].name) == 0) {
                            c = 'x';//1070;
                        }
                        else if(strcmp("filterType", options[option_index].name) == 0) {
                            c = 'f';//1080;
                        }
                        else if(strcmp("terrainNavPort", options[option_index].name) == 0) {
                            c = 1090;
                        }
                        else if(strcmp("forceLowGradeFilter", options[option_index].name) == 0) {
                            c = 1100;
                        }
                        else if(strcmp("allowFilterReinits", options[option_index].name) == 0) {
                            c = 'r';//1110;
                        }
                        else if(strcmp("useModifiedWeighting", options[option_index].name) == 0) {
                            c = 'w'; //1120;
                        }
                        else if(strcmp("samplePeriod", options[option_index].name) == 0) {
                            c = 1130;
                        }
                        else if(strcmp("maxNorthingCov", options[option_index].name) == 0) {
                            c = 1140;
                        }
                        else if(strcmp("maxNorthingError", options[option_index].name) == 0) {
                            c = 1150;
                        }
                        else if(strcmp("maxEastingCov", options[option_index].name) == 0) {
                            c = 1160;
                        }
                        else if(strcmp("maxEastingError", options[option_index].name) == 0) {
                            c = 1170;
                        }
                        else if(strcmp("RollOffset", options[option_index].name) == 0) {
                            c = 1180;
                        }
                        else if(strcmp("useIDTData", options[option_index].name) == 0) {
                            c = 1190;
                        }
                        else if(strcmp("useDVLSide", options[option_index].name) == 0) {
                            c = 1200;
                        }
                        else if(strcmp("useMbTrnData", options[option_index].name) == 0) {
                            c = 1210;
                        }
                        else if(strcmp("useMbTrnServer", options[option_index].name) == 0) {
                            c = 1220;
                        }
                    }
                    break;
                default:
                    break;
            }

            const char *supp = "";

            switch (c) {
                    // fall through to short options
                    // short options
                case 'c':
                    snprintf(ctx->cdir, BUF_SZ, "%s", optarg);
                    break;
                case 'C':
                    snprintf(ctx->cpath, BUF_SZ, "%s", optarg);
                    break;
                case 'd':
                    snprintf(ctx->ddir, BUF_SZ, "%s", optarg);
                    break;
                case 'D':
                    sscanf(optarg, "%d", &ctx->debug);
                    TrnPlayerCtx::gdebug(true, ctx->debug);
                    break;
                case 'E':
                    snprintf(ctx->eofile, BUF_SZ, "%s", optarg);
                    ctx->oflags |= OUT_EST_FILE;
                    break;
                case 'f':
                    sscanf(optarg, "%d", &ctx->filter_type);
                    if(ctx->filter_type > 3 || ctx->filter_type < 0)
                        ctx->filter_type = 0;
                    else
                        ctx->filter_type = (ctx->filter_type % 4);
                    break;
                case 'F':
                    ctx->force_status = true;
                    break;
                case 'G':
                    // clear all flags
                    // must set content and format flag(s)
                {
                    // save file output flags (sticky)
                    int save_of = (ctx->oflags & (OUT_EST_FILE | OUT_MEAS_FILE));

                    // clear all
                    ctx->oflags = 0x0;

                    if(strstr(optarg,"p") != NULL)
                        ctx->oflags |= OUT_PRETTY;
                    if(strstr(optarg,"e") != NULL)
                        ctx->oflags |= OUT_EST_CSV;
                    if(strstr(optarg,"m") != NULL)
                        ctx->oflags |= OUT_MEAS_CSV;
                    if(strstr(optarg,"S") != NULL)
                        ctx->oflags |= OUT_MMSE;
                    if(strstr(optarg,"L") != NULL)
                        ctx->oflags |= OUT_MLE;
                    if(strstr(optarg,"B") != NULL)
                        ctx->oflags |= (OUT_MLE | OUT_MMSE);
                    if(strstr(optarg,"q") != NULL)
                        ctx->oflags = 0x0;

                    // restore saved file flags
                    ctx->oflags |= save_of;
                }
                    break;
                case 'h':
                    ctx->is_help_set = true;
                    break;
                case 'i':
                    sscanf(optarg, "%d", &ctx->input_format);
                    ctx->input_format = (ctx->input_format % 6);
                    if(ctx->input_format == IOFMT_CSV_DVL) {
                        ctx->sensor_type = TRN_SENSOR_DVL;
                        ctx->meas_out_format = IOFMT_CSV_DVL;
                    }
                    if(ctx->input_format == IOFMT_CSV_IDT) {
                        ctx->sensor_type = TRN_SENSOR_DELTAT;
                        ctx->meas_out_format = IOFMT_CSV_IDT;
                    }
                    if(ctx->input_format == IOFMT_CSV_MB) {
                        ctx->sensor_type = TRN_SENSOR_MB;
                        ctx->meas_out_format = IOFMT_CSV_MB;
                    }
                    break;
                case 'I':
                    snprintf(ctx->ifile, BUF_SZ, "%s", optarg);
                    break;
                case 'm':
                    snprintf(ctx->mdir, BUF_SZ, "%s", optarg);
                    break;
                case 'M':
                    snprintf(ctx->mfile, BUF_SZ, "%s", optarg);
                    break;
                case 'N':
                    snprintf(ctx->nfile, BUF_SZ, "%s", optarg);
                    break;
                case 'o':
                    snprintf(ctx->odir, BUF_SZ, "%s", optarg);
                    break;
                case 'O':
                    snprintf(ctx->mofile, BUF_SZ, "%s", optarg);
                    ctx->oflags |= OUT_MEAS_FILE;
                    break;
                case 'P':
                    snprintf(ctx->pfile, BUF_SZ, "%s", optarg);
                    ctx->is_particles_set = true;
                    break;
                case 'r':
                    parse_bool(&ctx->reinit_en, optarg);
                    break;
                case 's':
                    sscanf(optarg, "%d", &ctx->sensor_type);
                    if(ctx->sensor_type > 5 || ctx->sensor_type < 0)
                        ctx->sensor_type = 0;
                    else
                        ctx->sensor_type = (ctx->sensor_type % 6);
                    break;
                case 'v':
                    ctx->verbose = true;
                    break;
                case 'V':
                    snprintf(ctx->vfile, BUF_SZ, "%s", optarg);
                    break;
                case 'w':
                    sscanf(optarg, "%d", &ctx->mod_weight);
                    if(ctx->mod_weight > 5 || ctx->mod_weight < 0)
                        ctx->mod_weight = 5;
                    else
                        ctx->mod_weight = (ctx->mod_weight % 6);
                    break;
                case 'x':
                    sscanf(optarg, "%d", &ctx->map_type);
                    if(ctx->map_type > 2 || ctx->map_type < 0)
                        ctx->map_type = 0;
                    else
                        ctx->map_type = (ctx->map_type % 3);
                    break;
                case 'Z':
                    sscanf(optarg, "%d", &ctx->meas_out_format);
                    if(ctx->meas_out_format > IOFMT_CSV_MB || ctx->meas_out_format < IOFMT_CSV_DVL)
                        ctx->meas_out_format = IOFMT_CSV_DVL;
                    break;
                case 0:
                case 1000:
                case 1010:
                case 1020:
                case 1030:
                case 1040:
                case 1050:
                case 1060:
                case 1070:
                case 1080:
                case 1090:
                case 1110:
                case 1120:
                case 1140:
                case 1150:
                case 1160:
                case 1170:
                case 1180:
                case 1190:
                case 1210:
                case 1220:
                    // pass unsupported options w/ warning
                    // to support terrainAid.cfg
                    if(c!=0) {
                        supp = "(** unsupported **)";
                        fprintf(stderr, "%s:%4d - WARN: unsupported option c[%d] opt: %20s arg: %20s\n", __func__, __LINE__, c, options[option_index].name, optarg);
                    }
                    break;
                case 1100:
                    // forceLowGradeFilter force low grade filter
                    parse_bool(&ctx->force_lgf, optarg);
                    break;
                case 1130:
                    // samplePeriod decimation by time (sec)
                    sscanf(optarg, "%ld", &ctx->dec_period_ms);
                    break;
                case 1200:
                    // useDVLSide
                    snprintf(ctx->nfile, BUF_SZ, "dvlSide.log");
                    break;
                case 1240:
                    // map_interp interp mode
                    sscanf(optarg, "%d", &ctx->map_interp);
                    if(ctx->map_interp > 3 || ctx->map_interp < 0)
                        ctx->map_interp = 0;
                    else
                        ctx->map_interp = (ctx->map_interp % 4);
                    break;
                case 1250:
                    // particle filter output mode
                    sscanf(optarg, "%d", &ctx->pf_omode);
                    // use compilation default if outside of valid range
                    if(ctx->pf_omode > PFO_NONE || ctx->pf_omode < PFO_HISTOGRAM)
                        ctx->pf_omode = PFO_NONE;
                    break;
                default:
                    fprintf(stderr, "%s:%4d - ERR unrecognized option %c/%d\n", __func__, __LINE__, (char)c, c);
                    ctx->is_help_set = true;
                    break;
            }
            if(ctx->debug>0)
                fprintf(stderr, "%s:%4d - c[%c] opt: %20s arg: %20s %s\n", __func__, __LINE__, (c==0 ? ' ' : c), options[option_index].name, optarg, supp );
        }

        return (ctx->is_help_set ? -1 : 0);//0;
    }

    static int parse_bool(bool *dest, const char *src)
    {
        int retval=-1;
        if(nullptr != src)
            switch(src[0]) {
                case 'y':
                case 'Y':
                case 't':
                case 'T':
                case '1':
                    *dest = true;
                    retval = 0;
                    break;
                case 'n':
                case 'N':
                case 'f':
                case 'F':
                case '0':
                    *dest = false;
                    retval = 0;
                    break;
                default:
                    break;
            }
        return retval;
    }

    static char *comment(char *src)
    {
        CTX_DPRINT("%s:%d >>> comment[%s]\n",__func__,__LINE__,src);
        char *bp = src;
        char *retval = src;
        if(NULL != src){
            while(*bp != '\0'){
                if(isspace(*bp)){
                    retval = bp;
                } else if(*bp == '#'){
                    CTX_DPRINT("%s:%d\n",__func__,__LINE__);
                    *bp = '\0';
                    retval = bp;
                    break;
                } else if (*bp == '/' && bp[1]=='/'){
                    CTX_DPRINT("%s:%d\n",__func__,__LINE__);
                    *bp = '\0';
                    retval = bp;
                    break;
                }else{
                    CTX_DPRINT("%s:%d\n",__func__,__LINE__);
                    retval = bp;
                    break;
                }
                bp++;
            }
        }
        return retval;
    }

    static char *trim(char *src)
    {
        char *bp = NULL;
        if(NULL!=src){
            char *pterm = strstr(src,";");
            // terminate line at first semicolon, if any
            // (for terrainAid.cfg support)
            if(pterm != NULL)
                *pterm = '\0';
            bp = src;
            char *ep = src+strlen(src);
            while(isspace(*bp) && (*bp != '\0')){
                bp++;
            }
            while((isspace(*ep) || *ep == '\0') && (ep >= src)){
                *ep = '\0';
                ep--;
            }
        }
        return bp;
    }

    static void parse_key_val(char *src, const char *del, char **pkey, char **pval)
    {
        char *scpy = strdup(src);
        char *key = strtok(scpy, del);
        char *val = strtok(NULL, del);
        if(NULL!=key)
            *pkey = strdup(key);
        else
            *pkey = NULL;
        if(NULL!=val)
            *pval = strdup(val);
        else
            *pval = NULL;
        free(scpy);
    }

    static char *expand_env(char *src)
    {
        char *retval = NULL;

        if(NULL!=src && strlen(src)>0 ){
            char *obuf=NULL;
            size_t len = strlen(src)+1;
            char *wp = (char *)malloc(len);
            char *sp = wp;
            memset(wp, 0, len);
            snprintf(wp, len, "%s", src);
            char *pb;

            while( (pb = strstr(wp,"$")) != NULL)
            {
                CTX_DPRINT(">>> wp[%s]\n",wp);
                char *pe = pb+1;
                CTX_DMSG(">>> pe...");
                while( (isalnum(*pe) || *pe=='-' || *pe == '_') && *pe!='\0' ){
                    CTX_DPRINT("[%c] ",*pe);
                    pe++;
                }
                CTX_DMSG("\n");
                // pe points to char AFTER end of var name
                if(pe>pb){
                    size_t var_len = pe-pb;
                    char var_buf[var_len+1];
                    memset(var_buf,0,var_len+1);
                    for(uint32_t i=1;i<var_len;i++){
                        var_buf[i-1] = pb[i];
                    }
                    CTX_DPRINT(">>> var_buf[%s]\n",var_buf);
                    char *val = getenv(var_buf);
                    size_t val_len = (val!=NULL?strlen(val):0);
                    size_t new_len = strlen(wp) - var_len + val_len + 1;
                    *pb='\0';
                    *(pe-1)='\0';
                    char *pecpy = strdup(pe);
                    char *rebuf = (char *)malloc(new_len);
                    memset(rebuf, 0, new_len);
                    snprintf(rebuf, new_len, "%s%s%s", wp, val, pecpy);
                    free(pecpy);
                    free(obuf);
                    obuf = rebuf;
                    wp = obuf;
                    retval = obuf;
                }
            }
            free(sp);
        }
        return retval;
    }

    static void parse_file(const std::string &file_path, TrnPlayerCtx *ctx)
    {
        std::ifstream file(file_path.c_str(), std::ifstream::in);

        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line) && file.good()) {
                // using printf() in all tests for consistency
                CTX_DPRINT(">>> line : [%s]\n", line.c_str());
                if(line.length()>0){
                    char *lcp = strdup(line.c_str());
                    char *wp = trim(lcp);
                    CTX_DPRINT(">>> wp[%s]\n", wp);
                    if(wp==NULL || strlen(wp)<=0){
                        // empty/comment
                    } else {
                        char *cp = comment(wp);
                        CTX_DPRINT(">>> cp[%s]\n", cp);
                        if(strlen(cp) > 0){
                            char *key=NULL;
                            char *val=NULL;
                            parse_key_val(cp, "=", &key, &val);
                            char *tval = trim(val);
                            char *tkey = trim(key);
                            CTX_DPRINT(">>> key[%s] val[%s]\n",tkey,tval);

                            char *etval = expand_env(tval);
                            if(etval==NULL)
                                etval=tval==NULL?strdup(""):strdup(tval);

                            CTX_DPRINT(">>> key[%s] etval[%s]\n",tkey,etval);
                            size_t cmd_len = strlen(key) + strlen(etval) + 4;
                            char *cmd_buf = (char *)malloc(cmd_len);
                            memset(cmd_buf, 0, cmd_len);
                            snprintf(cmd_buf, cmd_len, "--%s%s%s", key,(strlen(etval)>0?"=":""),etval);
                            char dummy[]{'f','o','o','\0'};
                            char *cmdv[2]={dummy,cmd_buf};
                            CTX_DPRINT(">>> cmd_buf[%s] cmdv[%p]\n",cmd_buf,&cmdv[0]);

                            // parse this argument
                            parse_cmdline(2,&cmdv[0], ctx);

                            free(key);
                            free(val);
                            free(etval);
                            free(cmd_buf);
                            cmd_buf = NULL;
                            key=NULL;
                            val=NULL;
                            etval=NULL;
                        }else{
                            CTX_DMSG( ">>> [comment line]\n");
                        }
                    }
                    free(lcp);
                    lcp=NULL;
                }
            }
            file.close();
        } else {
            fprintf(stderr, "ERR - file open failed [%s] [%d/%s]\n", file_path.c_str(), errno, strerror(errno));
        }
    }

    static int parse(int argc, char **argv, TrnPlayerCtx *ctx) {
        ctx->is_config_set = false;

        // first pass : sets is_config_set flag if --config
        int test =  parse_cmdline(argc, argv, ctx);

        if(test < 0)
            return test;

        // check is_config_set flag and parse file if set
        if(ctx->is_config_set) {
            parse_file(std::string(ctx->cpath), ctx);
            // re-parse command line (ignoring config)
            // so CLI args override file
            parse_cmdline(argc, argv, ctx);
        }

        // parsing complete: set context variables

        // set map file path
        snprintf(ctx->mpath, BUF_SZ, "%s/%s", ctx->mdir, ctx->mfile);
        // set vehicle specs path
        snprintf(ctx->vpath, BUF_SZ, "%s/%s", ctx->cdir, ctx->vfile);
        // set log dir if unset
        if(strlen(ctx->odir) == 0)
            snprintf(ctx->odir, BUF_SZ, "%s", TRN_LOGDIR_DFL);
        // set particles file if configured
        if(strlen(ctx->pfile) > 0)
            snprintf(ctx->ppath, BUF_SZ, "%s/%s", ctx->cdir, ctx->pfile);

        // set input data log
        if(ctx->input_format == IOFMT_TRNNAV) {
            snprintf(ctx->dpath, BUF_SZ, "%s/%s", ctx->ddir, log_name(IOFMT_TRNNAV));
        } else if(ctx->input_format == IOFMT_TRNAID) {
            snprintf(ctx->dpath, BUF_SZ, "%s/%s", ctx->ddir, log_name(IOFMT_TRNAID));
            if(strlen(ctx->nfile) > 0) {
                snprintf(ctx->npath, BUF_SZ, "%s/%s", ctx->ddir, ctx->nfile);
            } else {
                snprintf(ctx->npath, BUF_SZ, "%s/%s", ctx->ddir, log_name(IOFMT_NAV));
            }
        }  else if(ctx->input_format == IOFMT_CSV_DVL) {
            snprintf(ctx->dpath, BUF_SZ, "%s/%s", ctx->ddir, log_name(IOFMT_CSV_DVL));
        }   else if(ctx->input_format == IOFMT_CSV_IDT) {
            snprintf(ctx->dpath, BUF_SZ, "%s/%s", ctx->ddir, log_name(IOFMT_CSV_IDT));
        }   else if(ctx->input_format == IOFMT_CSV_MB) {
            snprintf(ctx->dpath, BUF_SZ, "%s/%s", ctx->ddir, log_name(IOFMT_CSV_MB));
        } else {
            snprintf(ctx->dpath, BUF_SZ, "%s/%s", ctx->ddir, log_name(IOFMT_MBTRN));
        }

        if((ctx->oflags & OUT_MEAS_FILE) && strlen(ctx->mofile) > 0) {
            snprintf(ctx->mopath, BUF_SZ, "%s/%s", "latestTRN", ctx->mofile);
        }

        if((ctx->oflags & OUT_EST_FILE) && strlen(ctx->eofile) > 0) {
            snprintf(ctx->eopath, BUF_SZ, "%s/%s", "latestTRN", ctx->eofile);
        }

        if(strlen(ctx->ifile) > 0) {
            snprintf(ctx->dpath, BUF_SZ, "%s/%s", ctx->ddir, ctx->ifile);
        }
        if(ctx->pf_omode != PFO_NONE) {
            snprintf(ctx->pfopath, BUF_SZ, "%s/%s", "latestTRN", ctx->pfofile);
        }

        return 0;
    }


    // Filter Type
    // 0 : TRN_FT_NONE
    // 1 : TRN_FT_POINTMASS
    // 2 : TRN_FT_PARTICLE (always use 2)
    // 3 : TRN_FT_BANK
    int filter_type;

    // Map Type
    // 0 : UNDEFINED
    // 1 : TRN_MAP_DEM Digital Elevation Map (.GRD)
    // 2 : TRN_MAP_BO  Binary Octree Map (.BO)
    int map_type;

    // Sensor Type
    // 0 : UNDEFINED
    // 1 : TRN_SENSOR_DVL    DVL
    // 2 : TRN_SENSOR_MB     Multibeam
    // 3 : TRN_SENSOR_PENCIL Single Beam
    // 4 : TRN_SENSOR_HOMER  Homer Relative Measurement
    // 5 : TRN_SENSOR_DELTAT Imagenex multibeamconst int sensor_type=2;
    int sensor_type;

    // Use modified weigting
    // 0 : TRN_WT_NONE No modification
    // 1 : TRN_WT_NORM  Shandor's original alpha modification
    // 2 : TRN_WT_XBEAM Crossbeam with original
    // 3 : TRN_WT_SUBCL Subcloud with original
    // 4 : TRN_FORCE_SUBCL Force Subcloud every measurement
    // 5 : TRN_WT_INVAL invalid
    int mod_weight;

    // allow filter reinitialization
    bool reinit_en;

    // Map interpolation method
    // 0 : nearest-neighbor (no interpolation, default)
    // 1 : bilinear
    // 2 : bicubic
    // 3 : spline
    int map_interp;

    // force low grade filter if true (use high grade otherwise)
    bool force_lgf;

    // decimation period, msec
    long int dec_period_ms;

    // previous record timestamp
    double dec_prev_time;

    // Input Format
    // 0 : MbTrn.log
    // 1 : TerrainNav.log
    // 2 : TerrainAid.log
    // 3 : DVL CSV
    // 4 : IDT CSV
    // 5 : MB/Generic
    int input_format;

    // CSV output format
    // (see input format CSV types only)
    int meas_out_format;

    // Enable verbose output
    bool verbose;
    // enable debug output
    int debug;
    bool is_config_set;
    bool is_particles_set;
    bool is_help_set;

    // Hack to force beam status valid
    // for TerrainNav.log on LRAUV w/ RDI
    bool force_status;

    // last measurement successful
    bool last_meas;

    double time_range[2];
    int ping_range[2];

    // mounting geometry parameters
    // dr: euler angles phi,theta,psi (pitch,roll,yaw) (deg)
    // dt: translation (x,y,z) (m)
    double geo_dr[3];
    double geo_dt[3];

    // Output control flags (bitfield)
    // -- Format Flags --
    // OUT_PRETTY   0x1
    // OUT_EST_CSV  0x2
    // OUT_MEAS_CSV 0x4
    // -- Content Flags --
    // OUT_MMSE     0x100 (OUT_PRETTY)
    // OUT_MLE      0x200 (OUT_PRETTY)
    unsigned int oflags;

    // particle filter output mode
    // 0 : HISTOGRAMTOFILE
    // 1 : PARTICLESTOFILE
    unsigned int pf_omode;

    // Bathymetry log reader
    DataLogReader *trn_log;

    // Navigation log reader
    DataLogReader *nav_log;

    // CSV log reader
    FILE *csv_log;
    // CSV meas output
    FILE *meas_out;
    // CSV estimate output
    FILE *est_out;
    // particles file output
    ofstream *part_out;

    // TRN config directory
    char cdir[BUF_SZ];
    // Paths and files
    // TRN Input data directory
    char ddir[BUF_SZ];
    // Map directory
    char mdir[BUF_SZ];
    // TRN Output log directory
    char odir[BUF_SZ];

    // TRN estimate output file name
    char eofile[BUF_SZ];
    // TRN log file name (override name based on type)
    char ifile[BUF_SZ];
    // TRN navigation log file name (override name based on type)
    char nfile[BUF_SZ];
    // TRN map file name
    char mfile[BUF_SZ];
    // CSV output file name
    char mofile[BUF_SZ];
    // TRN particles file name (override name based on type)
    char pfile[BUF_SZ];
    // TRN particles log file name
    char pfofile[BUF_SZ];
    // TRN vehicle spec file name
    char vfile[BUF_SZ];

    // App config file path
    char cpath[BUF_SZ];
    // TRN input log file path
    char dpath[BUF_SZ];
    // TRN estimate output file path
    char eopath[BUF_SZ];
    // TRN output file path
    char mopath[BUF_SZ];
    // TRN map file path
    char mpath[BUF_SZ];
    // TRN nav file path
    char npath[BUF_SZ];
    // TRN particles file output path
    char pfopath[BUF_SZ];
    // TRN particles file path
    char ppath[BUF_SZ];
    // TRN vehicle spec file path
    char vpath[BUF_SZ];
};
#endif
