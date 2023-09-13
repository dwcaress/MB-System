///
/// @file trncli_test.c
/// @authors k. Headley
/// @date 12 jun 2019

/// Unit test wrapper for trn_cli

/// Compile test using
/// gcc -o trncli-test trncli-test.c trn_cli.c -L../bin -lmframe
/// @sa doxygen-examples.c for more examples of Doxygen markup

/////////////////////////
// Terms of use
/////////////////////////
/*
 Copyright Information
 
 Copyright 2002-2019 MBARI
 Monterey Bay Aquarium Research Institute, all rights reserved.
 
 Terms of Use
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version. You can access the GPLv3 license at
 http://www.gnu.org/licenses/gpl-3.0.html
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details
 (http://www.gnu.org/licenses/gpl-3.0.html)
 
 MBARI provides the documentation and software code "as is", with no warranty,
 express or implied, as to the software, title, non-infringement of third party
 rights, merchantability, or fitness for any particular purpose, the accuracy of
 the code, or the performance or results which you may obtain from its use. You
 assume the entire risk associated with use of the code, and you agree to be
 responsible for the entire cost of repair or servicing of the program with
 which you are using the code.
 
 In no event shall MBARI be liable for any damages, whether general, special,
 incidental or consequential damages, arising out of your use of the software,
 including, but not limited to, the loss or corruption of your data or damages
 of any kind resulting from use of the software, any prohibited use, or your
 inability to use the software. You agree to defend, indemnify and hold harmless
 MBARI and its officers, directors, and employees against any claim, loss,
 liability or expense, including attorneys' fees, resulting from loss of or
 damage to property or the injury to or death of any person arising out of the
 use of the software.
 
 The MBARI software is provided without obligation on the part of the
 Monterey Bay Aquarium Research Institute to assist in its use, correction,
 modification, or enhancement.
 
 MBARI assumes no responsibility or liability for any third party and/or
 commercial software required for the database or applications. Licensee agrees
 to obtain and maintain valid licenses for any additional third party software
 required.
 */

/////////////////////////
// Headers
/////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "trn_cli.h"
#include "mframe.h"
#include "mfile.h"
#include "mtime.h"
#include "msocket.h"
#include "mlog.h"
#include "mb1_msg.h"

/////////////////////////
// Macros
/////////////////////////
#define TRNCLI_TEST_NAME "trncli-test"
#ifndef TRNCLI_TEST_BUILD
/// @def TRNCLI_TEST_BUILD
/// @brief module build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DMBTRN_BUILD=`date`
#define TRNCLI_TEST_BUILD ""VERSION_STRING(APP_BUILD)
#endif

// These macros should only be defined for
// application main files rather than general C files
/*
 /// @def PRODUCT
 /// @brief header software product name
 #define PRODUCT "TBD_PRODUCT"
 
 /// @def COPYRIGHT
 /// @brief header software copyright info
 #define COPYRIGHT "Copyright 2002-2013 MBARI Monterey Bay Aquarium Research Institute, all rights reserved."
 /// @def NOWARRANTY
 /// @brief header software terms of use
 #define NOWARRANTY  \
 "This program is distributed in the hope that it will be useful,\n"\
 "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"\
 "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"\
 "GNU General Public License for more details (http://www.gnu.org/licenses/gpl-3.0.html)\n"
 */


#define TRNCLI_TEST_TRNSVR_HOST "127.0.0.1"
#define TRNCLI_TEST_TRNSVR_PORT 28000 //27027
#define TRNCLI_TEST_MBTRN_HOST "localhost"
#define TRNCLI_TEST_MBTRN_PORT 27000
#define TRNCLI_TEST_MBTRN_HBEAT 25
#define TRNCLI_CSV_LINE_BYTES 1024*20
#define TRNCLI_TEST_UPDATE_N 10
#define TRNCLI_TEST_LOG_NAME "trncli"
#define TRNCLI_TEST_LOG_DESC "trn client log"
#define TRNCLI_TEST_LOG_DIR  "."
#define TRNCLI_TEST_LOG_EXT  ".log"
#define TRNCLI_TEST_IFILE "./test.mb1"
#define TRNCLI_TEST_TRNCFG_MAP "PortTiles"
#define TRNCLI_TEST_TRNCFG_CFG "mappingAUV_specs.cfg"
#define TRNCLI_TEST_TRNCFG_PARTICLES "particles.cfg"
#define TRNCLI_TEST_TRNCFG_LOGDIR "logs"
#define TRN_CMD_LINE_BYTES 2048
#define TRNCLI_TEST_CONNECT_DELAY_SEC 2
#define TRNCLI_CONNECT_RETRIES 5
#define MB1_READ_RETRIES 50
#define MB1_ETO_MSEC 250
#define LOG_PATH_BYTES 512

/////////////////////////
// Declarations
/////////////////////////

typedef enum {
    SRC_CSV=0,
    SRC_MSVR,
    SRC_MBIN
}trncli_src_type;

typedef enum {
    MODE_MONITOR='m',
    MODE_UPDATE='u'
}app_mode_id;

/// @typedef struct app_cfg_s app_cfg_t
/// @brief application configuration parameter structure
typedef struct app_cfg_s{
    /// @var app_cfg_s::verbose
    /// @brief TBD
    bool verbose;
    /// @var app_cfg_s::debug
    /// @brief TBD
    bool debug;
    /// @var app_cfg_s::mode
    /// @brief TBD
    app_mode_id mode;
    /// @var app_cfg_s::no_init
    /// @brief TBD
    bool no_init;
    /// @var app_cfg_s::log_en
    /// @brief TBD
    bool log_en;
    /// @var app_cfg_s::mb1_file
    /// @brief TBD
    char *mb1_file;
    /// @var app_cfg_s::mb1_src
    /// @brief TBD
    trncli_src_type mb1_src;
    /// @var app_cfg_s::trn_cfg
    /// @brief TBD
    trn_config_t *trn_cfg;
    /// @var app_cfg_s::mb_host
    /// @brief TBD
    char *mb1_host;
    /// @var app_cfg_s::mb_port
    /// @brief TBD
    int mb1_port;
    /// @var app_cfg_s::mb_hbeat
    /// @brief TBD
    int trnc_hbn;
    /// @var app_cfg_s::est_n
    /// @brief TBD
    int est_n;
    /// @var app_cfg_s::utm
    /// @brief TBD
    long int utm;
    /// @var app_cfg_s::log_cfg
    /// @brief TBD
    mlog_config_t *log_cfg;
    /// @var app_cfg_s::log_id
    /// @brief TBD
    mlog_id_t log_id;
    /// @var app_cfg_s::log_name
    /// @brief TBD
    char *log_name;
    /// @var app_cfg_s::log_dir
    /// @brief TBD
    char *log_dir;
    /// @var app_cfg_s::log_path
    /// @brief TBD
    char *log_path;
    /// @var app_cfg_s::state_n
    /// @brief TBD
    int state_n;
    /// @var app_cfg_s::tcli_connect_retries
    /// @brief TBD
    int tcli_connect_retries;
    /// @var app_cfg_s::mb1_read_retries
    /// @brief TBD
    int mb1_read_retries;
    /// @var app_cfg_s::eto_msec
    /// @brief TBD
    int eto_msec;
}app_cfg_t;

static void s_show_help();
static void parse_args(int argc, char **argv, app_cfg_t *cfg);
static void s_termination_handler (int signum);
static app_cfg_t *app_cfg_new();
static void app_cfg_destroy(app_cfg_t **pself);
static int s_dbg_printf(FILE *fp, bool debug, const char *fmt, ...);
static int s_tokenize(char *src, char **dest, char *del, int ntok);

static int s_csv_to_mb1(mb1_t **pdest, mfile_file_t *src);

static int32_t s_read_mb1_csv(mfile_file_t *src, char *dest, uint32_t len);
static int32_t s_read_mb1_bin( mb1_t **pdest, mfile_file_t *src, app_cfg_t *cfg);
static int32_t s_trnc_read_mb1_rec( mb1_t **pdest, msock_socket_t *src, app_cfg_t *cfg);

static void s_test_cli_con(app_cfg_t *cfg, int err, bool *r_con, bool *r_int, uint32_t eto_msec);
static int s_trncli_show_trn_state(trncli_t *tcli, mb1_t *mb1, app_cfg_t *cfg);
static int s_trncli_show_trn_update(trncli_t *tcli, mb1_t *mb1, app_cfg_t *cfg);
static int s_show_mb1(mb1_t *mb1, app_cfg_t *cfg);
static void s_do_trn_updates(mb1_t *mb1, int mb1_count, trncli_t *tcli, app_cfg_t *cfg, bool *r_quit);
static int s_trncli_test_csv(trncli_t *tcli, app_cfg_t *cfg);
static int s_trncli_test_trnc(trncli_t *tcli, app_cfg_t *cfg);
static int s_trncli_test_mbin(trncli_t *tcli, app_cfg_t *cfg);

static msock_socket_t **s_get_mb1_instance(app_cfg_t *cfg);
static trncli_t **s_get_trncli_instance(app_cfg_t *cfg, bool force_new);
static void s_init_log(int argc, char **argv, app_cfg_t *cfg);
static int s_app_main(app_cfg_t *cfg);

/////////////////////////
// Imports
/////////////////////////

/////////////////////////
// Module Global Variables
/////////////////////////
static bool g_interrupt=false;
static bool g_signal=0;
static bool tcli_connected = false;
static bool tcli_initialized = false;
static bool mb1_connected = false;

/////////////////////////
// Function Definitions
/////////////////////////

/// @fn void s_show_help()
/// @brief output user help message to stdout.
/// @return none
static void s_show_help()
{
    // monitor mode:
    char help_message[] = "\n TRN client (trn_cli) test\n";
    char usage_message[] = "\n use: trn-cli [options]\n"
    "\n"
    " Options\n"
    " --verbose     : verbose output\n"
    " --debug       : debug output\n"
    " --log-en      : enable app logging\n"
    " --help        : output help message\n"
    " --version     : output version info\n"
    " --mode        : mode\n"
    "                  m: monitor - show mb1, TRN output (polled via trn_cli)\n"
    "                  u: update  - send mb1 updates to TRN host\n"
    " --no-init     : disable TRN init message in UPDATE mode\n"
    " --mb1-src     : mb1 input source:\n"
    "                  m: <mb1 file>\n"
    "                  c: <csv file>\n"
    "                  s: <mb1svr host>[:<mb1svr port>]\n"
    " --est-n       : TRN estimate output (modulus, every nth MB1)\n"
    " --state-n     : TRN state output (modulus, every nth MB1)\n"
    " --hbeat       : MB1 source heartbeat (modulus, every nth MB1)\n"
    " --host        : TRN host\n"
    " --map         : TRN map file (dir for tiles)\n"
    " --cfg         : TRN devices config file\n"
    " --par         : TRN particle file\n"
    " --logdir      : TRN log directory suffix (TRN-<logdir>.nnn)\n"
    " --trn-ftype   : TRN filter type:\n"
    "                  0: TRN_FILT_NONE\n"
    "                  1: TRN_FILT_POINTMASS\n"
    "                  2: TRN_FILT_PARTICLE\n"
    "                  3: TRN_FILT_BANK\n"
    " --trn-mtype   : TRN map type:\n"
    "                  D: Digital Elevation Map (DEM, GRD)\n"
    "                  B: Binary Octree (BO)\n"
    " --trn-freinit : TRN filter reinit Y: enable N: disable\n"
    " --trn-fgrade  : TRN filter grade L: low H: high\n"
    " --trn-mw      : TRN modified weighting:\n"
    "                  0: TRN_MWEIGHT_NONE\n"
    "                  1: TRN_MWEIGHT_SHANDOR\n"
    "                  2: TRN_MWEIGHT_CROSSBEAM\n"
    "                  3: TRN_MWEIGHT_SUBCLOUD_SHANDOR\n"
    "                  4: TRN_MWEIGHT_SUBCLOUD_NISON\n"
    " --trn-utm      : TRN UTM zone\n"
    " --trn-ncov     : TRN max northing covariance\n"
    " --trn-nerr     : TRN max northing error\n"
    " --trn-ecov     : TRN max easing covariance\n"
    " --trn-err      : TRN max easting error\n"
    "\n"
    " Notes:\n"
    "  Tests trn_cli API in one of two modes: MONITOR or UPDATE; uses MONITOR mode by default.\n"
    "  In either mode:\n"
    "    - receives and displays MB1 records from source specied by --mb1-src option.\n"
    "    - polls for TRN host (--host) for estimates and state using trn_cli instance and displays output.\n"
    "\n"
    "  In UPDATE mode:\n"
    "    - also pushes MB1 updates to TRN host before requesting state.\n"
    "    - the MB1 source and TRN host should be not be the same. For example, if using an mbtrnpp\n"
    "      instance as the MB1 source, the TRN host should be a separate TRN instance (e.g. trn-server).\n"
    "    - the TRN initialization parameters (map, cfg, particles, etc.) must be provided unless\n"
    "      the TRN instance is otherwise intialized.\n"
    "    - good for re-playing MB1 data via trn-server"
    "\n"
    "  In monitor mode, it may be used as an MB1 file reader by specifying --mb1_src=m:<path>, \n"
    "  --verbose, and leaving --est-n and --state-n unset (zero)\n"
    "\n"
    " Example:\n"
    "  ## Monitor mode (mbtrnpp MB1 output and TRN state)\n"
    "  trn-cli --mb1-src=s:192.168.1.101:27000 --host=192.168.1.101:28000 --est-n=3 --state-n=3 --hbeat=10\n"
    "\n"
    "  ## Monitor mode (MB1 file reader)\n"
    "  trn-cli --mb1-src=m:foo.mb1 --verbose\n"
    "\n"
    "  ## Update mode (replay MB1 via trn-server)\n"
    "  # define TRN environment\n"
    "  cat 20180713m0.env\n"
    "   #!/bin/bash\n"
    "   export TRN_LOGFILES=$PWD/logs\n"
    "   export TRN_DATAFILES=/Volumes/linux-share/config\n"
    "   export TRN_MAPFILES=/Volumes/linux-share/maps\n"
    "\n"
    "  # source environment before running TRN server\n"
    "  . 20180713m0.env\n"
    "  # start TRN server"
    "  $ trn-server  -p 27001\n"
    "\n"
    "  # run trn-cli"
    "  trn-cli ./src/mbtrnav/trn-cli  ./src/mbtrnav/trn-cli --mb1-src=m:/path/to/data.mb1 \\\n"
    "  --host=$TRN_HOST:27001 --map=PortTiles --cfg=mappingAUV_specs.cfg --par=particles.cfg \\\n"
    "  --logdir=foo --mode=u --est-n=3 --state-n=3 --hbeat=10 \n"
    "\n";
    printf("%s", help_message);
    printf("%s", usage_message);
}

/// @fn void parse_args(int argc, char ** argv, app_cfg_t * cfg)
/// @brief parse command line args, set application configuration.
/// @param[in] argc number of arguments
/// @param[in] argv array of command line arguments (strings)
/// @param[in] cfg application config structure
/// @return none
static void parse_args(int argc, char **argv, app_cfg_t *cfg)
{
    extern char WIN_DECLSPEC *optarg;
    int option_index;
    int c;
    bool help=false;
    bool version=false;
    char cmnem=0;
    static struct option options[] = {
        {"verbose", no_argument, NULL, 0},
        {"debug", no_argument, NULL, 0},
        {"help", no_argument, NULL, 0},
        {"version", no_argument, NULL, 0},
        {"log-en", no_argument, NULL, 0},
        {"mode", required_argument, NULL, 0},
        {"mb1-src", required_argument, NULL, 0},
        {"est-n", required_argument, NULL, 0},
        {"state-n", required_argument, NULL, 0},
        {"host", required_argument, NULL, 0},
        {"map", required_argument, NULL, 0},
        {"hbeat", required_argument, NULL, 0},
        {"cfg", required_argument, NULL, 0},
        {"par", required_argument, NULL, 0},
        {"logdir", required_argument, NULL, 0},
        {"trn-ftype", required_argument, NULL, 0},
        {"trn-mtype", required_argument, NULL, 0},
        {"trn-utm", required_argument, NULL, 0},
        {"trn-freinit", required_argument, NULL, 0},
        {"trn-fgrade", required_argument, NULL, 0},
        {"trn-mw", required_argument, NULL, 0},
        {"trn-ncov", required_argument, NULL, 0},
        {"trn-nerr", required_argument, NULL, 0},
        {"trn-ecov", required_argument, NULL, 0},
        {"trn-eerr", required_argument, NULL, 0},
        {NULL, 0, NULL, 0}};

    // process argument list
    while ((c = getopt_long(argc, argv, "", options, &option_index)) != -1){
        char *scpy = NULL;
        char *ssrc = NULL;
        char *sfile = NULL;
        char *shost = NULL;
        char *sport = NULL;
        switch (c) {
                // long options all return c=0
            case 0:
                // verbose
                if (strcmp("verbose", options[option_index].name) == 0) {
                    cfg->verbose=true;
                }
                // debug
                if (strcmp("debug", options[option_index].name) == 0) {
                    cfg->debug=true;
                }
                // log-en
                if (strcmp("log-en", options[option_index].name) == 0) {
                    cfg->log_en=true;
                }
                // help
                else if (strcmp("help", options[option_index].name) == 0) {
                    help = true;
                }
                // version
                else if (strcmp("version", options[option_index].name) == 0) {
                    version = true;
                }
                // mode
                else if (strcmp("mode", options[option_index].name) == 0) {
                    if(tolower(optarg[0]) == 'm'){
                        cfg->mode = MODE_MONITOR;
                    } else if(tolower(optarg[0]) == 'u'){
                        cfg->mode = MODE_UPDATE;
                    }
                }
                // host
                else if (strcmp("host", options[option_index].name) == 0) {
                    scpy = strdup(optarg);
                    shost = strtok(scpy,":");
                    sport = strtok(NULL,":");
                    if(NULL!=shost){
                        if(NULL!=cfg->trn_cfg->trn_host){
                            free(cfg->trn_cfg->trn_host);
                        }
                        cfg->trn_cfg->trn_host=strdup(shost);
                    }
                    if(NULL!=sport)
                        cfg->trn_cfg->trn_port=atoi(sport);
                    free(scpy);
                }
                // input
                else if (strcmp("mb1-src", options[option_index].name) == 0) {
                    scpy = strdup(optarg);
                    ssrc = strtok(scpy,":");
                    if(NULL!=ssrc){
                        if(ssrc[0] == 'c' || ssrc[0] == 'c'){
                            cfg->mb1_src=SRC_CSV;
                            sfile = strtok(NULL,":");
                            if(NULL != sfile){
                                if(NULL!=cfg->mb1_file){
                                    free(cfg->mb1_file);
                                }
                                cfg->mb1_file=strdup(sfile);
                            }
                        } else if(ssrc[0] == 'm' || ssrc[0] == 'M'){
                            cfg->mb1_src=SRC_MBIN;
                            sfile = strtok(NULL,":");
                            if(NULL != sfile){
                                if(NULL!=cfg->mb1_file){
                                    free(cfg->mb1_file);
                                }
                                cfg->mb1_file=strdup(sfile);
                            }
                        } else if(ssrc[0] == 's' || ssrc[0] == 'S'){
                            cfg->mb1_src=SRC_MSVR;
                            shost = strtok(NULL,":");
                            sport = strtok(NULL,":");
                            if(NULL!=shost){
                                if(NULL!=cfg->mb1_host){
                                    free(cfg->mb1_host);
                                }
                                cfg->mb1_host=strdup(shost);
                            }
                            if(NULL!=sport)
                                cfg->mb1_port=atoi(sport);
                        }
                    }
                }
                // map
                else if (strcmp("map", options[option_index].name) == 0) {
                    if(NULL!=cfg->trn_cfg->map_file){
                        free(cfg->trn_cfg->map_file);
                    }
                    cfg->trn_cfg->map_file=strdup(optarg);
                }
                // cfg
                else if (strcmp("cfg", options[option_index].name) == 0) {
                    if(NULL!=cfg->trn_cfg->cfg_file){
                        free(cfg->trn_cfg->cfg_file);
                    }
                    cfg->trn_cfg->cfg_file=strdup(optarg);
                }
                // particles
                else if (strcmp("par", options[option_index].name) == 0) {
                    if(NULL!=cfg->trn_cfg->particles_file){
                        free(cfg->trn_cfg->particles_file);
                    }
                    cfg->trn_cfg->particles_file=strdup(optarg);
                }
                // log_dir
                else if (strcmp("logdir", options[option_index].name) == 0) {
                    if(NULL!=cfg->trn_cfg->log_dir){
                        free(cfg->trn_cfg->log_dir);
                    }
                    cfg->trn_cfg->log_dir=strdup(optarg);
                }
                // ftype
                else if (strcmp("trn-ftype", options[option_index].name) == 0) {
                    sscanf(optarg,"%c",&cmnem);
                    switch (cmnem) {
                        case 'n':
                        case 'N':
                        case '0':
                            cfg->trn_cfg->filter_type=TRN_FILT_NONE;
                            break;
                        case 'm':
                        case 'M':
                        case '1':
                            cfg->trn_cfg->filter_type=TRN_FILT_POINTMASS;
                            break;
                        case 'p':
                        case 'P':
                        case '2':
                            cfg->trn_cfg->filter_type=TRN_FILT_PARTICLE;
                            break;
                        case 'b':
                        case 'B':
                        case '3':
                            cfg->trn_cfg->filter_type=TRN_FILT_BANK;
                            break;
                        default:
                            fprintf(stderr, "ERR - invalid trn-ftype[%c]\n", cmnem);
                            break;
                    }
                }
                // mtype
                else if (strcmp("trn-mtype", options[option_index].name) == 0) {
                    
                    sscanf(optarg,"%c",&cmnem);
                    switch (cmnem) {
                        case 'd':
                        case 'D':
                            cfg->trn_cfg->map_type=TRN_MAP_DEM;
                            break;

                        case 'b':
                        case 'B':
                            cfg->trn_cfg->map_type=TRN_MAP_BO;
                            break;
                        default:
                            fprintf(stderr, "ERR - invalid trn-mtype[%c]\n", cmnem);
                            break;
                    }
                }
                // utm
                else if (strcmp("trn-utm", options[option_index].name) == 0) {
                    if(sscanf(optarg,"%ld",&cfg->utm) == 1)
                        cfg->trn_cfg->utm_zone=cfg->utm;
                }
                // freinit
                else if (strcmp("trn-freinit", options[option_index].name) == 0) {
                    sscanf(optarg,"%c",&cmnem);
                    switch (cmnem) {
                        case 'y':
                        case 'Y':
                        case '1':
                            cfg->trn_cfg->filter_reinit=1;
                            break;

                        case 'n':
                        case 'N':
                        case '0':
                            cfg->trn_cfg->filter_reinit=0;
                            break;
                        default:
                            fprintf(stderr, "ERR - invalid trn-freinit[%c]\n", cmnem);
                            break;
                    }
                }
                // fgrade
                else if (strcmp("trn-fgrade", options[option_index].name) == 0) {
                    sscanf(optarg,"%c",&cmnem);
                    switch (cmnem) {
                        case 'h':
                        case 'H':
                        case '1':
                            cfg->trn_cfg->filter_grade=1;
                            break;

                        case 'l':
                        case 'L':
                        case '0':
                            cfg->trn_cfg->filter_grade=0;
                            break;
                        default:
                            fprintf(stderr, "ERR - invalid trn-fgrade[%c]\n", cmnem);
                            break;
                    }
                }
                // mw
                else if (strcmp("trn-mw", options[option_index].name) == 0) {
                    uint16_t mw = 0xFFFF;
                    if((sscanf(optarg,"%hu",&mw)==1) &&
                       (mw>=0 && mw<=4)){
                        cfg->trn_cfg->mod_weight = mw;
                    } else {
                        fprintf(stderr, "ERR - invalid trn-mw[%"PRIu16"]\n", mw);
                    }
                }
                // ncov
                else if (strcmp("trn-ncov", options[option_index].name) == 0) {
                    double val = -1.0;
                    if((sscanf(optarg,"%lf",&val)==1) &&
                       (val>0)){
                        cfg->trn_cfg->max_northing_cov = val;
                    } else {
                        fprintf(stderr, "ERR - invalid trn-ncov[%lf]\n", val);
                    }
                }
                // nerr
                else if (strcmp("trn-nerr", options[option_index].name) == 0) {
                    double val = -1.0;
                    if((sscanf(optarg,"%lf",&val)==1) &&
                       (val>0)){
                        cfg->trn_cfg->max_northing_err = val;
                    } else {
                        fprintf(stderr, "ERR - invalid trn-nerr[%lf]\n", val);
                    }
                }
                // ecov
                else if (strcmp("trn-ecov", options[option_index].name) == 0) {
                    double val = -1.0;
                    if((sscanf(optarg,"%lf",&val)==1) &&
                       (val>0)){
                        cfg->trn_cfg->max_easting_cov = val;
                    } else {
                        fprintf(stderr, "ERR - invalid trn-ecov[%lf]\n", val);
                    }
                }
                // eerr
                else if (strcmp("trn-eerr", options[option_index].name) == 0) {
                    double val = -1.0;
                    if((sscanf(optarg,"%lf",&val)==1) &&
                       (val>0)){
                        cfg->trn_cfg->max_easting_err = val;
                    } else {
                        fprintf(stderr, "ERR - invalid trn-eerr[%lf]\n", val);
                    }
                }
                // update
                else if (strcmp("est-n", options[option_index].name) == 0) {
                    sscanf(optarg,"%d",&cfg->est_n);
                }
                // hbeat
                else if (strcmp("hbeat", options[option_index].name) == 0) {
                    sscanf(optarg,"%d",&cfg->trnc_hbn);
                }
                // test-api
                else if (strcmp("state-n", options[option_index].name) == 0) {
                    sscanf(optarg,"%d",&cfg->state_n);
                }

                break;
            default:
                help=true;
                break;
        }
        if (version) {
            //            mbtrn_show_app_version(TCPC_NAME,TCPC_BUILD);
            fprintf(stderr, "%s: build %s\n", TRNCLI_TEST_NAME, TRNCLI_TEST_BUILD);
            exit(0);
        }
        if (help) {
            //            mbtrn_show_app_version(TCPC_NAME,TCPC_BUILD);
            s_show_help();
            exit(0);
        }
    }// while

    s_dbg_printf(stderr, cfg->verbose, "verbose   [%s]\n", (cfg->verbose?"Y":"N"));
    s_dbg_printf(stderr, cfg->verbose, "debug     [%s]\n", (cfg->debug?"Y":"N"));
    s_dbg_printf(stderr, cfg->verbose, "log_en    [%s]\n", (cfg->log_en?"Y":"N"));
    s_dbg_printf(stderr, cfg->verbose, "mode      [%c]\n", (char)cfg->mode);
    s_dbg_printf(stderr, cfg->verbose, "host      [%s]\n", cfg->trn_cfg->trn_host);
    s_dbg_printf(stderr, cfg->verbose, "port      [%d]\n", cfg->trn_cfg->trn_port);
    s_dbg_printf(stderr, cfg->verbose, "map       [%s]\n", cfg->trn_cfg->map_file);
    s_dbg_printf(stderr, cfg->verbose, "cfg       [%s]\n", cfg->trn_cfg->cfg_file);
    s_dbg_printf(stderr, cfg->verbose, "particles [%s]\n", cfg->trn_cfg->particles_file);
    s_dbg_printf(stderr, cfg->verbose, "logdir    [%s]\n", cfg->trn_cfg->log_dir);
    s_dbg_printf(stderr, cfg->verbose, "ftype     [%d]\n", cfg->trn_cfg->filter_type);
    s_dbg_printf(stderr, cfg->verbose, "mtype     [%d]\n", cfg->trn_cfg->map_type);
    s_dbg_printf(stderr, cfg->verbose, "freinit   [%d]\n", cfg->trn_cfg->filter_reinit);
    s_dbg_printf(stderr, cfg->verbose, "fgrade    [%d]\n", cfg->trn_cfg->filter_grade);
    s_dbg_printf(stderr, cfg->verbose, "mw        [%d]\n", cfg->trn_cfg->mod_weight);
    s_dbg_printf(stderr, cfg->verbose, "utm       [%ld]\n", cfg->trn_cfg->utm_zone);
    s_dbg_printf(stderr, cfg->verbose, "ncov      [%.3lf]\n", cfg->trn_cfg->max_northing_cov);
    s_dbg_printf(stderr, cfg->verbose, "nerr      [%.3lf]\n", cfg->trn_cfg->max_northing_err);
    s_dbg_printf(stderr, cfg->verbose, "ecov      [%.3lf]\n", cfg->trn_cfg->max_easting_cov);
    s_dbg_printf(stderr, cfg->verbose, "eerr      [%.3lf]\n", cfg->trn_cfg->max_easting_err);
    s_dbg_printf(stderr, cfg->verbose, "mb1_src   [%d]\n", cfg->mb1_src);
    s_dbg_printf(stderr, cfg->verbose, "mb1_file  [%s]\n", cfg->mb1_file);
    s_dbg_printf(stderr, cfg->verbose, "mb1_host  [%s]\n", cfg->mb1_host);
    s_dbg_printf(stderr, cfg->verbose, "mb1_port  [%d]\n", cfg->mb1_port);
    s_dbg_printf(stderr, cfg->verbose, "hbeat     [%d]\n", cfg->trnc_hbn);
    s_dbg_printf(stderr, cfg->verbose, "est_n     [%u]\n", cfg->est_n);
    s_dbg_printf(stderr, cfg->verbose, "state_n   [%u]\n", cfg->state_n);
}

/// @fn void termination_handler (int signum)
/// @brief termination signal handler.
/// @param[in] signum signal number
/// @return none
static void s_termination_handler (int signum)
{
    switch (signum) {
        case SIGINT:
        case SIGHUP:
        case SIGTERM:
            s_dbg_printf(stderr, true, "sig received[%d]\n", signum);
            g_interrupt=true;
            g_signal=signum;
            break;
        default:
            fprintf(stderr, "WARN - s_termination_handler: sig not handled[%d]\n", signum);
            break;
    }
}

static app_cfg_t *app_cfg_new()
{
    app_cfg_t *instance = (app_cfg_t *)malloc(sizeof(app_cfg_t));
    
    if(NULL!=instance){
        memset(instance,0,sizeof(app_cfg_t));
        instance->verbose=false;
        instance->debug=false;
        instance->mb1_file=strdup(TRNCLI_TEST_IFILE);
        instance->mb1_src=SRC_MBIN;
        instance->log_en=false;
        instance->no_init=false;
        instance->mode=MODE_MONITOR;
        instance->mb1_host=strdup(TRNCLI_TEST_MBTRN_HOST);
        instance->mb1_port=TRNCLI_TEST_MBTRN_PORT;
        instance->trnc_hbn=TRNCLI_TEST_MBTRN_HBEAT;
        instance->est_n=TRNCLI_TEST_UPDATE_N;
        instance->log_cfg=mlog_config_new(ML_TFMT_ISO1806,ML_DFL_DEL,ML_MONO|ML_NOLIMIT,ML_FILE,0,0,0);
        instance->log_id=MLOG_ID_INVALID;
        instance->log_name=strdup(TRNCLI_TEST_LOG_NAME);
        instance->log_dir=strdup(TRNCLI_TEST_LOG_DIR);
        instance->log_path=(char *)malloc(LOG_PATH_BYTES);
        instance->utm=TRNCLI_UTM_DFL;
        instance->state_n=0;
        instance->tcli_connect_retries = TRNCLI_CONNECT_RETRIES;
        instance->mb1_read_retries = MB1_READ_RETRIES;
        instance->eto_msec = MB1_ETO_MSEC;

        instance->trn_cfg=trncfg_dnew();
        
        if(NULL!=instance->trn_cfg->map_file)free(instance->trn_cfg->map_file);
        instance->trn_cfg->map_file=strdup(TRNCLI_TEST_TRNCFG_MAP);
        if(NULL!=instance->trn_cfg->cfg_file)free(instance->trn_cfg->cfg_file);
        instance->trn_cfg->cfg_file=strdup(TRNCLI_TEST_TRNCFG_CFG);
        if(NULL!=instance->trn_cfg->particles_file)free(instance->trn_cfg->particles_file);
        instance->trn_cfg->particles_file=strdup(TRNCLI_TEST_TRNCFG_PARTICLES);
        if(NULL!=instance->trn_cfg->log_dir)free(instance->trn_cfg->log_dir);
        instance->trn_cfg->log_dir=strdup(TRNCLI_TEST_TRNCFG_LOGDIR);
        instance->trn_cfg->map_type = TRN_MAP_BO;
        instance->trn_cfg->filter_type = TRN_FILT_PARTICLE;
        instance->trn_cfg->utm_zone = instance->utm;
        instance->trn_cfg->mod_weight = TRN_MWEIGHT_NONE;
        instance->trn_cfg->filter_reinit = TRN_FILT_REINIT_EN;
        instance->trn_cfg->filter_grade = TRN_GRD_HIGH;
        instance->trn_cfg->oflags = 0x0;
        instance->trn_cfg->max_northing_cov = TRN_MAX_NCOV_DFL;
        instance->trn_cfg->max_northing_err = TRN_MAX_NERR_DFL;
        instance->trn_cfg->max_easting_cov = TRN_MAX_ECOV_DFL;
        instance->trn_cfg->max_easting_err = TRN_MAX_EERR_DFL;
    }
    return instance;
}

static void app_cfg_destroy(app_cfg_t **pself)
{
    if(NULL!=pself){
        app_cfg_t *self = (app_cfg_t *)(*pself);
        if(NULL!=self){
            if(NULL!=self->mb1_file)
                free(self->mb1_file);
            if(NULL!=self->mb1_host)
                free(self->mb1_host);
            if(NULL!=self->log_name)
                free(self->log_name);
            if(NULL!=self->log_dir)
                free(self->log_dir);
            if(NULL!=self->log_path)
                free(self->log_path);

            trncfg_destroy(&self->trn_cfg);
            mlog_delete_instance(self->log_id);
            mlog_config_destroy(&self->log_cfg);

            free(self);
            *pself=NULL;
        }
    }
}

static int s_dbg_printf(FILE *fp, bool debug, const char *fmt, ...)
{
    int retval = 0;
    if(debug && NULL!=fp){
        va_list va1;
        va_start(va1,fmt);
        retval = vfprintf(fp, fmt, va1);
        va_end(va1);
    }
    return retval;
}

static int s_tokenize(char *src, char **dest, char *del, int ntok)
{
    int i=-1;
    if(NULL!=src && NULL!=del){
        if(dest==NULL){
            dest = (char **)malloc(ntok*sizeof(char *));
        }
        i=0;
        while(i<ntok){
            dest[i]=strtok((i==0?src:NULL),del);
            if(NULL==dest[i]){
                break;
            }
            i++;
        }
    }
    return i;
}

static int s_csv_to_mb1(mb1_t **pdest, mfile_file_t *src)
{
    int retval=-1;
    if(NULL!=pdest && NULL!=src){
        mb1_t *dest = *pdest;
        char line[TRNCLI_CSV_LINE_BYTES]={0};
        char **fields=NULL;
        int32_t test = 0;
        if( (test=s_read_mb1_csv(src,line,TRNCLI_CSV_LINE_BYTES))>0){
            // tokenize assigns value to fields and returns number of entries
            test=s_tokenize(line, fields, "\n", MB1_CSV_MAX_FIELDS);
            if( fields!=NULL && test>=MB1_CSV_HEADER_FIELDS ){

                //            sscanf(fields[0],"%u",&dest.type);
                //            sscanf(fields[1],"%u",&dest.size);
                sscanf(fields[1],"%lf",&dest->ts);
                sscanf(fields[2],"%lf",&dest->lat);
                sscanf(fields[3],"%lf",&dest->lon);
                sscanf(fields[4],"%lf",&dest->depth);
                sscanf(fields[5],"%lf",&dest->hdg);
                sscanf(fields[6],"%d",&dest->ping_number);
                sscanf(fields[7],"%u",&dest->nbeams);
                mb1_zero(dest,MB1_RS_BEAMS);
                unsigned int i=0;
                for(i=0;i<dest->nbeams;i++){
                    int x=8+(i*4);
                    sscanf(fields[x+0],"%u",&dest->beams[i].beam_num);
                    sscanf(fields[x+1],"%lf",&dest->beams[i].rhox);
                    sscanf(fields[x+2],"%lf",&dest->beams[i].rhoy);
                    sscanf(fields[x+3],"%lf",&dest->beams[i].rhoz);
                }
                mb1_set_checksum(dest);
                retval=0;
                free(fields);
            }else{
                fprintf(stderr, "ERR - tokenize failed [%d]\n", test);
            }
        }else{
            fprintf(stderr, "ERR - read_csv_rec failed [%d]\n", test);
        }
    }else{
        fprintf(stderr, "ERR - invalid argument d[%p] s[%p]\n", pdest, src);
    }
    return retval;
}

static int32_t s_read_mb1_csv(mfile_file_t *src, char *dest, uint32_t len)
{
    int32_t retval=-1;

    if(NULL!=src && NULL!=dest && len>0){
        char *cp=dest;
        while(mfile_read(src,(byte *)cp,1)==1){
            if(cp>=(dest+len) || *cp=='\n'){
                if(*cp=='\n'){
                    retval++;
                    *cp='\0';
                }
                break;
            }
            retval++;
            cp++;
        }

    }
    return retval;
}

static int32_t s_read_mb1_bin( mb1_t **pdest, mfile_file_t *src, app_cfg_t *cfg)
{
    int32_t retval=-1;

    if(NULL!=src && NULL!=pdest){
        mb1_t *dest = *pdest;
        uint32_t readlen = 1;
        uint32_t record_bytes=0;
        // sync to start of record
        byte *bp = (byte *)dest;
        size_t test=0;

        // find header type sequence start byte
        while(mfile_read(src,(byte *)bp,readlen)==(int64_t)readlen){
            if(*bp=='M'){
                record_bytes+=readlen;
                bp++;
                readlen = (uint32_t)MB1_HEADER_BYTES-1;
                break;
            }
        }

        // read header remainder
        int64_t br = mfile_read(src,(byte *)bp,readlen);
        if((record_bytes > 0) && (br == (int64_t)readlen)){
             record_bytes+=readlen;
            bp=(byte *)MB1_PBEAMS(dest);
            readlen = MB1_BEAM_ARRAY_BYTES(dest->nbeams) + MB1_CHECKSUM_BYTES;

            // read data and checksum
            if((br=mfile_read(src,(byte *)bp,readlen)) == (int64_t)readlen){
                record_bytes += readlen;
                retval = record_bytes;
            }else{
                int64_t cur_save = mfile_seek(src,0,MFILE_CUR);
                int64_t cur_end = mfile_seek(src,0,MFILE_END);
                if(cur_save == cur_end){
                    s_dbg_printf(stderr, cfg->debug, "end of file [%"PRId64"]\n",cur_end);
                    retval = 0;
                }else{
                    fprintf(stderr, "%s:%d ERR - data read [%zu/%"PRIu32"]:\n", __func__, __LINE__, test, readlen);
                    mb1_show(dest, true, 5);
                }
                mfile_seek(src,cur_save,MFILE_SET);
            }
        }else{
            int64_t cur_save = mfile_seek(src,0,MFILE_CUR);
            int64_t cur_end = mfile_seek(src,0,MFILE_END);
            if(cur_save == cur_end){
                s_dbg_printf(stderr, cfg->debug, "end of file [%"PRId64"]\n",cur_end);
                retval = 0;
            }else{
                fprintf(stderr, "%s:%d ERR - header read readlen[%"PRIu32"] br[%"PRId64"] [%zu]\n", __func__, __LINE__, readlen, br, test);
            }
        }
    }
    return retval;
}

static int32_t s_trnc_read_mb1_rec( mb1_t **pdest, msock_socket_t *src, app_cfg_t *cfg)
{
    int32_t retval=-1;

    if(NULL!=src && NULL!=pdest){
        mb1_t *dest = *pdest;
        uint32_t readlen = MB1_MAX_SOUNDING_BYTES;
        // sync to start of record
        byte *bp = (byte *)dest;
        //        msock_set_blocking(src,true);

        int64_t test = msock_recvfrom(src, NULL, (byte *)bp, readlen, 0);

        if(test > (int64_t)MB1_HEADER_BYTES){

            uint32_t record_bytes=readlen;
            retval=record_bytes;
            
            s_dbg_printf(stderr, cfg->debug, "%s:%d - read [%"PRId64"/%"PRIu32"]\n", __func__, __LINE__, test, readlen);

            mlog_oset_t log_dest = mlog_get_dest(cfg->log_id);
            mlog_set_dest(cfg->log_id, (log_dest | ML_SERR));

            mlog_tprintf(cfg->log_id, "ts[%.3lf] beams[%u] ping[%d]\n", dest->ts, dest->nbeams, dest->ping_number);
            mlog_tprintf(cfg->log_id, "lat[%.5lf] lon[%.5lf] hdg[%.2lf] sd[%.1lf]\n", dest->lat, dest->lon, dest->hdg, dest->depth);

            if(cfg->verbose && dest->nbeams>0){
                // add beam data for verbose output
                fprintf(stderr, "%5s %8s %8s %8s\n","beam", "rhox", "rhoy", "rhoz");
                for(int i=0; i<dest->nbeams; i++){
                    fprintf(stderr, "[%03"PRIu32"] %8.2lf %8.2lf %8.2lf\n", dest->beams[i].beam_num, dest->beams[i].rhox, dest->beams[i].rhoy, dest->beams[i].rhoz );
                }
            }

            mlog_set_dest(cfg->log_id, log_dest);

        }else{
            if(test>0 && (strncmp((char *)bp,"ACK",3)==0 || strncmp((char *)bp,"NACK",4)==0)){
                // received ACK, ignore
                s_dbg_printf(stderr, cfg->debug, "read ACK\n");
            }else if(errno!=EAGAIN){
                fprintf(stderr, "ERR - read failed (%s) ret[%"PRId64"/%"PRIu32"] [%d/%s]\n", __func__, test,readlen, errno, strerror(errno));
                mlog_tprintf(cfg->log_id, "ERR - read failed (%s) ret[%"PRId64"/%"PRIu32"] [%d/%s]\n", __func__, test, readlen, errno, strerror(errno));
            }
        }
    }
    return retval;
}

static void s_test_cli_con(app_cfg_t *cfg, int err, bool *r_con, bool *r_int, uint32_t eto_msec)
{
    if(NULL != cfg){
        const char *host = ((r_con != NULL) && (*r_con == tcli_connected) ? cfg->trn_cfg->trn_host : cfg->mb1_host);
        int port = ((r_con != NULL) && (*r_con == tcli_connected) ? cfg->trn_cfg->trn_port : cfg->mb1_port);
        switch (err) {
            case 0:
                // success
                break;
            case EPIPE:
                mlog_tprintf(cfg->log_id, "ERR: EPIPE client disconnected [%s:%d]\n", host, port);
                if(NULL != r_con)
                    *r_con = false;
                break;
            case EAGAIN:
            case ETIMEDOUT:
                // ignore
                err=0;
                if(eto_msec > 0)
                    mtime_delay_ms(eto_msec);
                break;
            case ECONNRESET:
                mlog_tprintf(cfg->log_id, "ERR: ECONNRESET client disconnected [%s:%d]\n", host, port);
                if(NULL != r_con)
                    *r_con = false;
                break;
            case ECONNREFUSED:
                mlog_tprintf(cfg->log_id, "ERR: ECONNREFUSED client disconnected [%s:%d]\n", host, port);
                if(NULL != r_con)
                    *r_con = false;
                break;
            case ENODATA:
                mlog_tprintf(cfg->log_id, "ERR: ENODATA client disconnected? [%s:%d]\n", host, port);
                if(NULL != r_con)
                    *r_con = false;
                break;
            case EINTR:
                mlog_tprintf(cfg->log_id, "ERR: EINTR user interrupt [%s:%d]\n", host, port);
                if(NULL != r_int)
                    *r_int = true;
                break;
            default:
                s_dbg_printf(stderr, cfg->debug, "ERR - [%d/%s]\n", err, strerror(err));
                break;
        }
    }
    return;
}

static int s_trncli_show_trn_state(trncli_t *tcli, mb1_t *mb1, app_cfg_t *cfg)
{
    int retval=-1;

    if(NULL != tcli && NULL != mb1 && NULL != cfg){
        int pval=-1;
        bool bval=false;
        int err=0;
        bool intr=false;

        mlog_oset_t log_dest = mlog_get_dest(cfg->log_id);
        mlog_set_dest(cfg->log_id, (log_dest | ML_SERR));

        fprintf(stderr,"\n");

        for(int i=0; i<7; i++){
            err = 0;
            switch(i){
                case 0:
                    bval=trncli_is_intialized(tcli);
                    err = errno;
                    mlog_tprintf(cfg->log_id, "is initialized [%c]\n", (bval?'Y':'N'));
                    break;
                case 1:
                    bval=trncli_is_converged(tcli);
                    err = errno;
                    mlog_tprintf(cfg->log_id, "is converged [%c]\n", (bval?'Y':'N'));
                    break;
                case 2:
                    bval=trncli_last_meas_succesful(tcli);
                    err = errno;
                    mlog_tprintf(cfg->log_id, "last meas val [%c]\n", (bval?'Y':'N'));
                    break;
                case 3:
                    pval=trncli_reinit_count(tcli);
                    err = errno;
                    mlog_tprintf(cfg->log_id, "reinit count [%d]\n", pval);
                    break;
                case 4:
                    pval=trncli_get_filter_type(tcli);
                    err = errno;
                    mlog_tprintf(cfg->log_id, "filter type [%d]\n", pval);
                    break;
                case 5:
                    pval=trncli_get_filter_state(tcli);
                    err = errno;
                    mlog_tprintf(cfg->log_id, "filter state [%d]\n", pval);
                    break;
                case 6:
                    bval=trncli_outstanding_meas(tcli);
                    err = errno;
                    mlog_tprintf(cfg->log_id, "outstanding meas [%c]\n", (bval?'Y':'N'));
                    break;
                default:
                    break;
            }

            s_test_cli_con(cfg, err, &tcli_connected, &intr, 0);

            if(!tcli_connected || intr){
                break;
            }
        }

        fprintf(stderr,"\n");
        mlog_set_dest(cfg->log_id, log_dest);

        if(tcli_connected && !intr)
            retval = 0;
    }

    return retval;
}

static int s_trncli_show_trn_update(trncli_t *tcli, mb1_t *mb1, app_cfg_t *cfg)
{
    int retval=-1;

    wmeast_t *mt = NULL;
    wposet_t *pt = NULL;
    pt_cdata_t *pt_dat=NULL;
    pt_cdata_t *mle_dat=NULL;
    pt_cdata_t *mse_dat=NULL;
    
    if(NULL!=tcli && NULL!=mb1 && tcli_connected){
        int test=-1;

        if(cfg->mode == MODE_UPDATE){

            test = trncli_send_update(tcli, mb1, &pt, &mt);

            if(test != 0){
                fprintf(stderr, "ERR - trncli_send_update failed [%d]\n", test);
                mlog_tprintf(cfg->log_id, "ERR - trncli_send_update failed [%d]\n", test);
            }

        } else {
            wposet_mb1_to_pose(&pt, mb1, cfg->utm);
        }

        test = trncli_get_bias_estimates(tcli, pt, &pt_dat, &mle_dat, &mse_dat);

        if( test == 0){

            if(NULL!=pt_dat &&  NULL!= mle_dat && NULL!=mse_dat ){

                mlog_oset_t log_dest = mlog_get_dest(cfg->log_id);
                mlog_set_dest(cfg->log_id, (log_dest | ML_SERR));

                fprintf(stderr,"\n");
                mlog_tprintf(cfg->log_id, "MLE,%.2lf,%.4lf,%.4lf,%.4lf\n", mle_dat->time, (mle_dat->x-pt_dat->x), (mle_dat->y-pt_dat->y), (mle_dat->z-pt_dat->z));
                mlog_tprintf(cfg->log_id, "MSE,%.2lf,%.4lf,%.4lf,%.4lf\n", mse_dat->time, (mse_dat->x-pt_dat->x), (mse_dat->y-pt_dat->y), (mse_dat->z-pt_dat->z));
                mlog_tprintf(cfg->log_id, "COV,%.2lf,%.2lf,%.2lf\n", sqrt(mse_dat->covariance[0]), sqrt(mse_dat->covariance[2]), sqrt(mse_dat->covariance[5]));

                mlog_set_dest(cfg->log_id, log_dest);

                retval=0;

            } else {
                fprintf(stderr,"\n");
                fprintf(stderr, "ERR - pt[%p] pt_dat[%p] mle_dat[%p] mse_dat[%p]\n", pt, pt_dat, mle_dat, mse_dat);
                mlog_tprintf(cfg->log_id, "ERR - pt[%p] pt_dat[%p] mle_dat[%p] mse_dat[%p]\n", pt, pt_dat, mle_dat, mse_dat);
                mlog_tprintf(cfg->log_id, "ERR - ts[%.3lf] beams[%u] ping[%d] \n", mb1->ts, mb1->nbeams, mb1->ping_number);
                mlog_tprintf(cfg->log_id, "ERR - lat[%.5lf] lon[%.5lf] hdg[%.2lf] sd[%.1lf]\n", mb1->lat, mb1->lon, mb1->hdg, mb1->depth);
            }
        } else {
            // check error, update trn cli connection status
            retval = errno;
            s_test_cli_con(cfg, retval, &tcli_connected, NULL, 0);
            s_dbg_printf(stderr, cfg->debug, "ERR - trncli_get_bias_estimates failed [%d]\n", test);
            mlog_tprintf(cfg->log_id, "ERR - trncli_get_bias_estimates failed [%d]\n", test);
        }

        wmeast_destroy(mt);
        wposet_destroy(pt);
        if(NULL!=pt_dat)
            free(pt_dat);
        if(NULL!=mse_dat)
            free(mse_dat);
        if(NULL!=mle_dat)
            free(mle_dat);
    }

    return retval;
}

static int s_show_mb1(mb1_t *mb1, app_cfg_t *cfg)
{
    int retval = -1;
    if(NULL != mb1 && NULL != cfg){
        mlog_oset_t log_dest = mlog_get_dest(cfg->log_id);
        mlog_set_dest(cfg->log_id, (log_dest | ML_SERR));

        mlog_tprintf(cfg->log_id, "ts[%.3lf] beams[%u] ping[%d]\n", mb1->ts, mb1->nbeams, mb1->ping_number);
        mlog_tprintf(cfg->log_id, "lat[%.5lf] lon[%.5lf] hdg[%.2lf] sd[%.1lf]\n", mb1->lat, mb1->lon, mb1->hdg, mb1->depth);

        if(cfg->verbose && mb1->nbeams>0){
            // add beam data for verbose output
            fprintf(stderr, "%5s %8s %8s %8s\n","beam", "rhox", "rhoy", "rhoz");
            for(int i=0; i<mb1->nbeams; i++){
                fprintf(stderr, "[%03"PRIu32"] %8.2lf %8.2lf %8.2lf\n", mb1->beams[i].beam_num, mb1->beams[i].rhox, mb1->beams[i].rhoy, mb1->beams[i].rhoz );
            }
        }

        mlog_set_dest(cfg->log_id, log_dest);
        retval = 0;
    }

    return retval;
}

static void s_do_trn_updates(mb1_t *mb1, int mb1_count, trncli_t *i_tcli, app_cfg_t *cfg, bool *r_quit)
{
    if(NULL != mb1 && NULL != cfg){

        bool update_est = (cfg->est_n>0) && ((mb1_count % cfg->est_n) == 0);
        bool update_state = (cfg->state_n>0) && ((mb1_count % cfg->state_n) == 0);
        trncli_t *tcli = i_tcli;

        if( update_est || update_state){

            // connect TRN client if TRN estimate/state output enabled
            // and not connected
            if(!tcli_connected){
                trncli_t **ptcli = s_get_trncli_instance(cfg, false);
                tcli = *ptcli;
                if(tcli_connected){
                    // reset connect retries
                    // [tcli_connect_retries is deprecated]
                    cfg->tcli_connect_retries = TRNCLI_CONNECT_RETRIES;
                }
            }

            if(tcli_connected &&
               update_est){

                // get TRN estimates (MMSE, MLE)
                int test = s_trncli_show_trn_update(tcli, mb1, cfg);

                if(test != 0){
                    s_test_cli_con(cfg, test, &tcli_connected, r_quit, 0);
                }
            }

            if(tcli_connected &&
               update_state){

                int test = s_trncli_show_trn_state(tcli, mb1, cfg);

                if(test != 0){
                    s_test_cli_con(cfg, test, &tcli_connected, r_quit, 0);
                }
            }
        }
    }

    return;
}

static int s_trncli_test_csv(trncli_t *tcli, app_cfg_t *cfg)
{
    int retval=0;
    mfile_file_t *mb1_file = mfile_file_new(cfg->mb1_file);
    int test=-1;
    
    if( (test=mfile_open(mb1_file, MFILE_RONLY)) >0){
        bool quit=false;
        mb1_t *mb1 = mb1_new(0);
        int mb1_count = 0;

        while( !g_interrupt && !quit ){
            bool mb1_read_OK = false;

            test = s_csv_to_mb1(&mb1, mb1_file);

            if(test > 0){
                mb1_read_OK = true;
                mb1_count++;

                s_show_mb1(mb1, cfg);

            } else {
                // error or end of file
                break;
            }
            if(mb1_read_OK){
                s_do_trn_updates(mb1, mb1_count, tcli, cfg, &quit);
            }

            mb1_zero_len(mb1, MB1_MAX_SOUNDING_BYTES);
        }// while
        mb1_destroy(&mb1);
    }else{
        fprintf(stderr, "ERR - mfile_open failed [%d]\n", test);
        mlog_tprintf(cfg->log_id, "ERR - mfile_open failed [%d]\n", test);
    }
    mfile_file_destroy(&mb1_file);
    return retval;
}

static int s_trncli_test_trnc(trncli_t *tcli, app_cfg_t *cfg)
{
    int retval=0;
    msock_socket_t **pmb1 = NULL;
    msock_socket_t *mb1_sock = NULL;
    bool quit=false;
    int hbeat=cfg->trnc_hbn;
    int mb1_count=0;
    mb1_t *mb1 = NULL;

    while( !g_interrupt && !quit ){

        if(!mb1_connected){
            // connect MB1 input as needed
            pmb1 = s_get_mb1_instance(cfg);
            mb1_sock = *pmb1;
            hbeat=cfg->trnc_hbn;
            cfg->mb1_read_retries = MB1_READ_RETRIES;
        }

        bool mb1_read_OK = false;

        if(mb1_connected){

            mb1_resize(&mb1, MB1_MAX_BEAMS, MB1_RS_ALL);

            // read next MB1 record
            int test = s_trnc_read_mb1_rec(&mb1, mb1_sock, cfg);

            if(test > 0){
                // read successful
                mb1_read_OK = true;
                mb1_count++;
                cfg->mb1_read_retries = MB1_READ_RETRIES;

                if( hbeat-- <= 0){
                    // send hbeat as needed
                    hbeat=0;
                    const char *req="REQ";
                    if(msock_sendto(mb1_sock,NULL,(byte *)req,4,0)==4){
                        // mbtrnpp doesn't ACK hbeat update
                        hbeat=cfg->trnc_hbn;
                    }
                }
            }else{
                // MB1 read error
                test=errno;
                cfg->mb1_read_retries--;

                s_test_cli_con(cfg, test, &mb1_connected, &quit, cfg->eto_msec);
            }
        }
        
        if(mb1_read_OK){
            s_do_trn_updates(mb1, mb1_count, tcli, cfg, &quit);
        }
    }// while

    if(quit){
        s_dbg_printf(stderr, cfg->debug, "quit flag set - exiting\n");
        mlog_tprintf(cfg->log_id, "quit flag set - exiting\n");
    }
    if(g_interrupt){
        s_dbg_printf(stderr, cfg->debug, "INTERRUPTED sig[%d] - exiting\n", g_signal);
        mlog_tprintf(cfg->log_id, "INTERRUPTED sig[%d] - exiting\n", g_signal);
    }

    msock_socket_destroy(pmb1);
    return retval;
}

static int s_trncli_test_mbin(trncli_t *tcli, app_cfg_t *cfg)
{
    int retval=0;
    mfile_file_t *mb1_file = NULL;
    mb1_file = mfile_file_new(cfg->mb1_file);
    int test=-1;
    
    if( (test=mfile_open(mb1_file, MFILE_RONLY))>0){
        bool quit=false;
        mb1_t *mb1=mb1_new(MB1_MAX_BEAMS);
        int mb1_count = 0;
        while( !g_interrupt && !quit){

            bool mb1_read_OK = false;

            test = s_read_mb1_bin(&mb1, mb1_file,cfg);

            if(test > 0){
                mb1_read_OK = true;
                mb1_count++;
                
                s_show_mb1(mb1, cfg);

            } else {
                // error or end of file
                break;
            }

            if(mb1_read_OK){
                s_do_trn_updates(mb1, mb1_count, tcli, cfg, &quit);
            }

            mb1_zero_len(mb1, MB1_MAX_SOUNDING_BYTES);
        }// while
        mb1_destroy(&mb1);
    }else{
        fprintf(stderr, "%s:%d ERR - mfile_open [%s] failed [%d][%d/%s]\n", __func__, __LINE__, cfg->mb1_file, test, errno, strerror(errno));
    }
    mfile_file_destroy(&mb1_file);
    return retval;
}

static msock_socket_t **s_get_mb1_instance(app_cfg_t *cfg)
{
    static msock_socket_t *mb1_sock = NULL;

    if(NULL != mb1_sock){
        s_dbg_printf(stderr,cfg->debug, "%s:%d tearing down mb1svr socket\n", __func__, __LINE__);
        msock_socket_destroy(&mb1_sock);
        mb1_sock = NULL;
    }

    s_dbg_printf(stderr,cfg->debug, "%s:%d creating mb1svr socket %s:%d\n", __func__, __LINE__, cfg->mb1_host, cfg->mb1_port);
    mb1_sock = msock_socket_new(cfg->mb1_host,cfg->mb1_port,ST_UDP);


    if(NULL!=mb1_sock){
        s_dbg_printf(stderr, cfg->debug, "%s:%d connecting mb1svr  socket %s:%d\n", __func__, __LINE__, cfg->mb1_host, cfg->mb1_port);

        int test = msock_connect(mb1_sock);
        mtime_delay_ms(250);
        if(test==0){
            s_dbg_printf(stderr,cfg->debug, "%s:%d mb1svr sending REQ %s:%d\n", __func__, __LINE__, cfg->mb1_host, cfg->mb1_port);
            int REQ_BYTES=4;
            byte req[REQ_BYTES];
            memset(req, 0, REQ_BYTES);
            snprintf((char *)req, REQ_BYTES, "REQ");
            int64_t st = msock_sendto(mb1_sock,NULL,req, REQ_BYTES, 0);
            if(st == (int64_t)4){
                s_dbg_printf(stderr,cfg->debug, "%s:%d sendto ret[%"PRId64"]\n", __func__, __LINE__, st);
                mtime_delay_ms(250);

                int ACK_BYTES=4;
                byte ack[ACK_BYTES];
                memset(ack,0,ACK_BYTES);

                msock_set_blocking(mb1_sock,true);
                s_dbg_printf(stderr,cfg->debug, "%s:%d mb1svr reading ACK %s:%d\n", __func__, __LINE__, cfg->mb1_host, cfg->mb1_port);

                int64_t rf = msock_recvfrom(mb1_sock, NULL, (byte *)ack, ACK_BYTES, 0);
                s_dbg_printf(stderr,cfg->debug, "%s:%d recvfrom ret[%"PRId64"]\n", __func__, __LINE__, rf);

                if(rf==4){
                    mb1_connected=true;
                    msock_set_blocking(mb1_sock,false);
                    mlog_tprintf(cfg->log_id, "mb1svr input mb1_connected [%s:%d]\n", cfg->mb1_host, cfg->mb1_port);
                }
            }
        }else{
            fprintf(stderr,"%s:%d ERR - msock_connect [%s:%d] failed [%d][%d/%s]\n", __func__, __LINE__, cfg->mb1_host, cfg->mb1_port, test, errno, strerror(errno));
        }
    }else{
        fprintf(stderr,"%s:%d ERR - mb1 socket create [%s:%d] failed [%d/%s]\n", __func__, __LINE__, cfg->mb1_host, cfg->mb1_port, errno, strerror(errno));
    }

    return &mb1_sock;
}

static trncli_t **s_get_trncli_instance(app_cfg_t *cfg, bool force_new)
{

    static trncli_t *tcli_instance = NULL;

    if(NULL == tcli_instance){
        tcli_instance = trncli_new(cfg->utm);
        tcli_connected = false;
        tcli_initialized = false;
    } else if(force_new){
        trncli_disconnect(tcli_instance);
        trncli_destroy(&tcli_instance);
        tcli_instance = trncli_new(cfg->utm);
        tcli_connected = false;
        tcli_initialized = false;
    }

    if(NULL != tcli_instance) {

        s_dbg_printf(stderr, cfg->debug, "cfg trncli host:port %s:%d src[%d]\n", cfg->trn_cfg->trn_host, cfg->trn_cfg->trn_port, cfg->mb1_src);

        if(!tcli_connected){
            int test=trncli_connect(tcli_instance, cfg->trn_cfg->trn_host, cfg->trn_cfg->trn_port);
            if(  test == 0){
                s_dbg_printf(stderr, cfg->debug, "trncli_connect OK\n");
                tcli_connected = true;
            } else {
                fprintf(stderr,"ERR - trncli_connect failed\n");
                tcli_connected = false;
                tcli_initialized = false;
            }
        }

        if(tcli_connected){
            if(cfg->mode == MODE_UPDATE && !cfg->no_init){
                int test = trncli_init_trn(tcli_instance, cfg->trn_cfg);

                if( test > 0){
                    s_dbg_printf(stderr, cfg->debug, "trncli_init_trn OK\n");
                    tcli_initialized = true;
                } else {
                    fprintf(stderr, "ERR - trncli_init_trn failed [%d]\n", test);
                    tcli_initialized = false;
                }
            } else {
                s_dbg_printf(stderr, cfg->debug, "skipping TRN init\n");
                tcli_initialized = true;
            }
        }

    } else {
        fprintf(stderr, "ERR - trncli_new failed\n");
        tcli_connected = false;
        tcli_initialized = false;
    }

    return &tcli_instance;
}

static void s_init_log(int argc, char **argv, app_cfg_t *cfg)
{
    char session_date[32] = {0};

    // make session time string to use
    // in log file names
    time_t rawtime;
    struct tm *gmt;

    time(&rawtime);
    // Get GMT time
    gmt = gmtime(&rawtime);
    // format YYYYMMDD-HHMMSS
    snprintf(session_date, 32, "%04d%02d%02d-%02d%02d%02d",
            (gmt->tm_year+1900),gmt->tm_mon+1,gmt->tm_mday,
            gmt->tm_hour,gmt->tm_min,gmt->tm_sec);

    snprintf(cfg->log_path, LOG_PATH_BYTES, "%s//%s-%s-%s",cfg->log_dir,cfg->log_name,session_date,TRNCLI_TEST_LOG_EXT);
    cfg->log_id = mlog_get_instance(cfg->log_path, cfg->log_cfg, TRNCLI_TEST_LOG_NAME);

    if(!cfg->log_en){
        // log disabled, set output to none
        // creates empty log file
        mlog_set_dest(cfg->log_id, ML_SERR);
    }

    mfile_flags_t flags = MFILE_RDWR|MFILE_APPEND|MFILE_CREATE;
    mfile_mode_t mode = MFILE_RU|MFILE_WU|MFILE_RG|MFILE_WG;

    char g_cmd_line[TRN_CMD_LINE_BYTES]={0};
    char *ip=g_cmd_line;
    int x=0;
    for (x=0;x<argc;x++){
        if ((ip+strlen(argv[x])-g_cmd_line) > TRN_CMD_LINE_BYTES) {
            fprintf(stderr, "WARN - logged cmdline truncated\n");
            break;
        }
        int ilen=snprintf(ip, TRN_CMD_LINE_BYTES-(strlen(ip)+1), " %s",argv[x]);
        ip+=ilen;
    }
    g_cmd_line[TRN_CMD_LINE_BYTES-1]='\0';

    mlog_open(cfg->log_id, flags, mode);
    mlog_tprintf(cfg->log_id, "*** trncli-test session start ***\n");
    mlog_tprintf(cfg->log_id, "cmdline [%s]\n", g_cmd_line);
    mlog_tprintf(cfg->log_id, "build [%s]\n", TRNCLI_TEST_BUILD);
}

static int s_app_main(app_cfg_t *cfg)
{
    int retval=-1;
    int test=-1;
    bool quit=false;

    s_dbg_printf(stderr, cfg->debug, "use CTRL-C to exit\n");

    trncli_t **ptcli=NULL;
    trncli_t *tcli = NULL;

    while( !g_interrupt && !quit ){

        while(!g_interrupt &&
              !tcli_connected){

            ptcli = s_get_trncli_instance(cfg, true);

            if(cfg->mode == MODE_UPDATE)
                break;
            if(!tcli_connected)
                sleep(TRNCLI_TEST_CONNECT_DELAY_SEC);
        }

        if(NULL!=ptcli)
            tcli = *ptcli;

        if(!g_interrupt)
            switch (cfg->mb1_src) {
                case SRC_CSV:
                    retval=s_trncli_test_csv(tcli, cfg);
                    quit=true;
                    break;
                case SRC_MBIN:
                    retval=s_trncli_test_mbin(tcli, cfg);
                    quit=true;
                    break;
                case SRC_MSVR:
                    retval=s_trncli_test_trnc(tcli, cfg);
                    // retry, don't quit
                    break;
                default:
                    fprintf(stderr, "ERR - invalid input type [%d]\n", cfg->mb1_src);
                    quit=true;
                    break;
            }
        if(!(quit || g_interrupt))
            sleep(3);
    }

    if( (test=trncli_disconnect(tcli)) !=0 ){
        fprintf(stderr, "ERR - trncli_disconnect failed [%d]\n", test);
    }

    trncli_destroy(ptcli);
    mlog_tprintf(cfg->log_id, "*** trncli-test session end ***\n");

    return retval;
}

/// @fn int main(int argc, char ** argv)
/// @brief trn_server client test main entry point.
/// @param[in] argc number of command line arguments
/// @param[in] argv array of command line arguments (strings)
/// @return 0 on success, -1 otherwise

int main(int argc, char **argv)
{
    int retval=-1;
    
    app_cfg_t *cfg = app_cfg_new();
    
    // configure signal handling
    // for main thread
    struct sigaction saStruct;
    sigemptyset(&saStruct.sa_mask);
    saStruct.sa_flags = 0;
    saStruct.sa_handler = s_termination_handler;
    sigaction(SIGINT, &saStruct, NULL);
    
    // parse command line args (update config)
    parse_args(argc, argv, cfg);

    s_init_log(argc,argv,cfg);
    
    s_app_main(cfg);

    const char *log_path = mlog_path(cfg->log_id);

    mlog_close(cfg->log_id);
    mlog_delete_instance(cfg->log_id);

    if(!cfg->log_en && (NULL != log_path)){
        // delete (empty) log file if logging disabled
        s_dbg_printf(stderr, cfg->debug, "removing %s\n",log_path);
        int test = remove(log_path);
        if(test !=0 ){
            fprintf(stderr, "ERR - could not remove log [%s] [%d:%s]\n", log_path, errno, strerror(errno));
        }
    }

    app_cfg_destroy(&cfg);

    return retval;
}
