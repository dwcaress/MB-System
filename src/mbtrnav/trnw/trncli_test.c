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
#include "msocket.h"
#include "mlog.h"
#include "medebug.h"
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
#define TRNCLI_TEST_CONNECT_DELAY_SEC 5

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
}app_cfg_t;


static void s_show_help();
static trncli_t *s_get_trncli_instance(app_cfg_t *cfg, bool force_new);

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
    " --verbose   : verbose output\n"
    " --debug     : debug output\n"
    " --log-en    : enable app logging\n"
    " --help      : output help message\n"
    " --version   : output version info\n"
    " --mode      : mode\n"
    "                m: monitor - show mb1, TRN output (polled via trn_cli)\n"
    "                u: update  - send mb1 updates to TRN host\n"
    " --no-init   : disable TRN init message in UPDATE mode\n"
    " --mb1-src   : mb1 input source:\n"
    "                m:<mb1 file>\n"
    "                c:<csv file>\n"
    "                s:<mb1svr host>[:<mb1svr port>]\n"
    " --est-n     : TRN estimate output (modulus, every nth MB1)\n"
    " --state-n   : TRN state output (modulus, every nth MB1)\n"
    " --hbeat     : MB1 source heartbeat (modulus, every nth MB1)\n"
    " --host      : TRN host\n"
    " --map       : TRN map file (dir for tiles)\n"
    " --cfg       : TRN config file\n"
    " --particles : TRN particle file\n"
    " --logdir    : TRN log directory\n"
    " --ftype     : TRN filter type\n"
    " --mtype     : TRN map type D:DEM B:BO\n"
    " --utm       : UTM zone\n"
    "\n"
    " Notes:\n"
    "  Tests trn_cli API in one of two modes: MONITOR or UPDATE; uses MONITOR mode by default.\n"
    "  In either mode:\n"
    "    - receives and displays MB1 records from source specied by --mb1-src option.\n"
    "    - polls for TRN host (--host) for estimates and state using trn_cli instance and displays output.\n"
    "  In UPDATE mode:\n"
    "    - also pushes MB1 updates to TRN host before requesting state.\n"
    "    - the MB1 source and TRN host should be not be the same. For example, if using an mbtrnpp\n"
    "      instance as the MB1 source, the TRN host should be a separate TRN instance (e.g. trn-server).\n"
    "    - the TRN initialization parameters (map, cfg, particles, etc.) must be provided unless\n"
    "      the TRN instance is otherwise intialized.\n"
    "\n"
    " Example:\n"
    "  # monitor mode\n"
    "  trn-cli --mb1-src=s:192.168.1.101:27000 --host=192.168.1.101:28000 --est-n=3 --state-n=3 --hbeat=10\n"
    "\n"
    "  # update mode\n"
    "  trn-cli --mb1-src=s:192.168.1.101:27000 --host=192.168.1.101:28000 --est-n=3 --state-n=3 --hbeat=10 \\\n"
    "   --map=$TRN_MAPFILES/PortTiles --cfg=$TRN_DATAFILES/trn.cfg --particles=$TRN_DATAFILES/particles.cfg \\\n"
    "   --logdir=$TRN_LOGDIR\n"
    "\n";
    printf("%s", help_message);
    printf("%s", usage_message);
}
// End function s_show_help

/// @fn void parse_args(int argc, char ** argv, app_cfg_t * cfg)
/// @brief parse command line args, set application configuration.
/// @param[in] argc number of arguments
/// @param[in] argv array of command line arguments (strings)
/// @param[in] cfg application config structure
/// @return none
void parse_args(int argc, char **argv, app_cfg_t *cfg)
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
        {"particles", required_argument, NULL, 0},
        {"logdir", required_argument, NULL, 0},
        {"ftype", required_argument, NULL, 0},
        {"mtype", required_argument, NULL, 0},
        {"utm", required_argument, NULL, 0},
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
                else if (strcmp("particles", options[option_index].name) == 0) {
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
                else if (strcmp("ftype", options[option_index].name) == 0) {
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
                            fprintf(stderr, "ERR - invalid ftype[%c]\n", cmnem);
                            break;
                    }
                }
                // mtype
                else if (strcmp("mtype", options[option_index].name) == 0) {
                    
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
                            fprintf(stderr, "ERR - invalid mtype[%c]\n", cmnem);
                            break;
                    }
                }
                // utm
                else if (strcmp("utm", options[option_index].name) == 0) {
                    sscanf(optarg,"%ld",&cfg->utm);
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
    PDPRINT((stderr, "verbose   [%s]\n", (cfg->verbose?"Y":"N")));
    PDPRINT((stderr, "debug     [%s]\n", (cfg->debug?"Y":"N")));
    PDPRINT((stderr, "log_en    [%s]\n", (cfg->log_en?"Y":"N")));
    PDPRINT((stderr, "mode      [%c]\n", (char)cfg->trn_cfg->mode));
    PDPRINT((stderr, "host      [%s]\n", cfg->trn_cfg->trn_host));
    PDPRINT((stderr, "port      [%d]\n", cfg->trn_cfg->trn_port));
    PDPRINT((stderr, "mb1_src   [%d]\n", cfg->mb1_src));
    PDPRINT((stderr, "mb1_file  [%s]\n", cfg->mb1_file));
    PDPRINT((stderr, "mb1_host  [%s]\n", cfg->mb1_host));
    PDPRINT((stderr, "mb1_port  [%d]\n", cfg->mb1_port));
    PDPRINT((stderr, "utm       [%ld]\n", cfg->utm));
    PDPRINT((stderr, "hbeat     [%d]\n", cfg->trnc_hbn));
    PDPRINT((stderr, "est_n     [%u]\n", cfg->est_n));
    PDPRINT((stderr, "state_n   [%u]\n", cfg->state_n));
}
// End function parse_args

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
            PDPRINT((stderr, "sig received[%d]\n", signum));
            g_interrupt=true;
            g_signal=signum;
            break;
        default:
            fprintf(stderr, "WARN - s_termination_handler: sig not handled[%d]\n", signum);
            break;
    }
}
// End function termination_handler

static app_cfg_t *app_cfg_new()
{
    app_cfg_t *instance = (app_cfg_t *)malloc(sizeof(app_cfg_t));
    
    if(NULL!=instance){
        memset(instance,0,sizeof(app_cfg_t));
        instance->verbose=false;
        instance->debug=false;
        instance->mb1_file=strdup(TRNCLI_TEST_IFILE);
        instance->mb1_src=SRC_MBIN;
        instance->trn_cfg=trncfg_dnew();
        
        if(NULL!=instance->trn_cfg->map_file)free(instance->trn_cfg->map_file);
        instance->trn_cfg->map_file=strdup(TRNCLI_TEST_TRNCFG_MAP);
        if(NULL!=instance->trn_cfg->cfg_file)free(instance->trn_cfg->cfg_file);
        instance->trn_cfg->cfg_file=strdup(TRNCLI_TEST_TRNCFG_CFG);
        if(NULL!=instance->trn_cfg->particles_file)free(instance->trn_cfg->particles_file);
        instance->trn_cfg->particles_file=strdup(TRNCLI_TEST_TRNCFG_PARTICLES);
        if(NULL!=instance->trn_cfg->log_dir)free(instance->trn_cfg->log_dir);
        instance->trn_cfg->log_dir=strdup(TRNCLI_TEST_TRNCFG_LOGDIR);

        instance->log_en=false;
        instance->mode=MODE_MONITOR;
        instance->mb1_host=strdup(TRNCLI_TEST_MBTRN_HOST);
        instance->mb1_port=TRNCLI_TEST_MBTRN_PORT;
        instance->trnc_hbn=TRNCLI_TEST_MBTRN_HBEAT;
        instance->est_n=TRNCLI_TEST_UPDATE_N;
        instance->log_cfg=mlog_config_new(ML_TFMT_ISO1806,ML_DFL_DEL,ML_MONO|ML_NOLIMIT,ML_FILE,0,0,0);
        instance->log_id=MLOG_ID_INVALID;
        instance->log_name=strdup(TRNCLI_TEST_LOG_NAME);
        instance->log_dir=strdup(TRNCLI_TEST_LOG_DIR);
        instance->log_path=(char *)malloc(512);
        instance->utm=TRNCLI_UTM_DFL;
        instance->state_n=0;
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


static int32_t s_read_csv_rec(mfile_file_t *src, char *dest, uint32_t len)
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

static int s_csv_to_mb1(mb1_t **pdest, mfile_file_t *src)
{
    int retval=-1;
    if(NULL!=pdest && NULL!=src){
        mb1_t *dest = *pdest;
        char line[TRNCLI_CSV_LINE_BYTES]={0};
        char **fields=NULL;
        int32_t test = 0;
        if( (test=s_read_csv_rec(src,line,TRNCLI_CSV_LINE_BYTES))>0){
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

static int32_t s_read_mb1_rec( mb1_t **pdest, mfile_file_t *src, app_cfg_t *cfg)
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
        while(mfile_read(src,(byte *)bp,readlen)==readlen){
            if(*bp=='M'){
                record_bytes+=readlen;
                bp++;
                readlen=MB1_HEADER_BYTES-1;
                break;
            }
        }

        // read header remainder
        if(record_bytes>0 && (test=mfile_read(src,(byte *)bp,readlen))==readlen){
             record_bytes+=readlen;
            bp=(byte *)MB1_PBEAMS(dest);
            readlen = MB1_BEAM_ARRAY_BYTES(dest->nbeams) + MB1_CHECKSUM_BYTES;

            // read data and checksum
            if((test=mfile_read(src,(byte *)bp,readlen))==readlen){
                record_bytes += readlen;
                retval = record_bytes;
            }else{
                int64_t cur_save=mfile_seek(src,0,MFILE_CUR);
                if(cur_save==mfile_seek(src,0,MFILE_END)){
                    s_dbg_printf(stderr, cfg->debug, "end of file\n");
                    retval = 0;
                }else{
                    fprintf(stderr, "%s:%d ERR - data read [%zu/%"PRIu32"]:\n", __func__, __LINE__, test, readlen);
                    mb1_show(dest, true, 5);
                }
                mfile_seek(src,cur_save,MFILE_SET);
            }
        }else{
            fprintf(stderr, "%s:%d ERR - header read [%zu]\n", __func__, __LINE__, test);
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

static int s_trncli_process_mb1(trncli_t *dcli, mb1_t *mb1, app_cfg_t *cfg)
{
    int retval=0;

    wmeast_t *mt = NULL;
    wposet_t *pt = NULL;
    pt_cdata_t *pt_dat=NULL;
    pt_cdata_t *mle_dat=NULL;
    pt_cdata_t *mse_dat=NULL;
    
    if(NULL!=dcli && NULL!=mb1 ){
        int test=-1;

        if(cfg->mode == MODE_UPDATE){
            test=trncli_send_update(dcli, mb1, &pt, &mt);
            if(test != 0){
                fprintf(stderr, "ERR - trncli_send_update failed [%d]\n", test);
                mlog_tprintf(cfg->log_id, "ERR - trncli_send_update failed [%d]\n", test);
            }
        }else{
            wposet_mb1_to_pose(&pt, mb1, cfg->utm);
        }

        if( (test=trncli_get_bias_estimates(dcli, pt, &pt_dat, &mle_dat, &mse_dat)) == 0){
            if(NULL!=pt_dat &&  NULL!= mle_dat && NULL!=mse_dat ){

                mlog_oset_t log_dest = mlog_get_dest(cfg->log_id);
                mlog_set_dest(cfg->log_id, (log_dest | ML_SERR));

                fprintf(stderr,"\n");
                mlog_tprintf(cfg->log_id, "MLE,%.2lf,%.4lf,%.4lf,%.4lf\n", mle_dat->time, (mle_dat->x-pt_dat->x), (mle_dat->y-pt_dat->y), (mle_dat->z-pt_dat->z));
                mlog_tprintf(cfg->log_id, "MSE,%.2lf,%.4lf,%.4lf,%.4lf\n", mse_dat->time, (mse_dat->x-pt_dat->x), (mse_dat->y-pt_dat->y), (mse_dat->z-pt_dat->z));
                mlog_tprintf(cfg->log_id, "COV,%.2lf,%.2lf,%.2lf\n", sqrt(mse_dat->covariance[0]), sqrt(mse_dat->covariance[2]), sqrt(mse_dat->covariance[5]));

                mlog_set_dest(cfg->log_id, log_dest);

                retval=0;

            }else{
                fprintf(stderr,"\n");
                fprintf(stderr, "ERR - pt[%p] pt_dat[%p] mle_dat[%p] mse_dat[%p]\n", pt, pt_dat, mle_dat, mse_dat);
                mlog_tprintf(cfg->log_id, "ERR - pt[%p] pt_dat[%p] mle_dat[%p] mse_dat[%p]\n", pt, pt_dat, mle_dat, mse_dat);
                mlog_tprintf(cfg->log_id, "ERR - ts[%.3lf] beams[%u] ping[%d] \n", mb1->ts, mb1->nbeams, mb1->ping_number);
                mlog_tprintf(cfg->log_id, "ERR - lat[%.5lf] lon[%.5lf] hdg[%.2lf] sd[%.1lf]\n", mb1->lat, mb1->lon, mb1->hdg, mb1->depth);
            }
        }else{
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

static int s_trncli_test_csv(trncli_t *dcli, app_cfg_t *cfg)
{
    int retval=0;
    mfile_file_t *mb1_file = mfile_file_new(cfg->mb1_file);
    int test=-1;
    
    if( (test=mfile_open(mb1_file, MFILE_RONLY)) >0){
        bool quit=false;
        mb1_t *mb1 = mb1_new(0);
        while( !g_interrupt && !quit && (test=s_csv_to_mb1(&mb1, mb1_file))>0){
            if( (test=s_trncli_process_mb1(dcli,mb1,cfg))!=0){
                if(test==EPIPE){
                    quit=true;
                }
            }
        }// while
        mb1_destroy(&mb1);
    }else{
        fprintf(stderr, "ERR - mfile_open failed [%d]\n", test);
        mlog_tprintf(cfg->log_id, "ERR - mfile_open failed [%d]\n", test);
    }
    mfile_file_destroy(&mb1_file);
    return retval;
}

static int s_trncli_test_trnc(trncli_t *dcli, app_cfg_t *cfg)
{
    int retval=0;
    msock_socket_t *isock = NULL;
    int err_count=0;
    bool quit=false;
    int hbeat=cfg->trnc_hbn;
    byte req[4]={0}, ack[4]={0};
    sprintf((char *)req, "REQ");
    int count=0;
    bool mb1_connected=false;
    mb1_t *mb1 = NULL;

    while( !g_interrupt && !quit ){
        if(!tcli_connected){
            dcli = s_get_trncli_instance(cfg, false);
        }
        int test=-1;
        mb1_resize(&mb1, MB1_MAX_BEAMS, MB1_RS_ALL);
        if(!mb1_connected){
            if(NULL!=isock){
                s_dbg_printf(stderr,cfg->verbose, "%s:%d tearing down mb1svr socket\n", __func__, __LINE__);
                msock_socket_destroy(&isock);
            }
            s_dbg_printf(stderr,cfg->verbose, "%s:%d creating mb1svr socket %s:%d\n", __func__, __LINE__, cfg->mb1_host, cfg->mb1_port);
            isock = msock_socket_new(cfg->mb1_host,cfg->mb1_port,ST_UDP);

            if(NULL!=isock){
                s_dbg_printf(stderr, cfg->debug, "%s:%d connecting mb1svr  socket %s:%d\n", __func__, __LINE__, cfg->mb1_host, cfg->mb1_port);
                msock_set_blocking(isock,true);
                if( ( test=msock_connect(isock))==0){
                    if(msock_sendto(isock,NULL,req,4,0)==4){
                        memset(ack,0,4);
                        if(msock_recv(isock,ack,4,0)==4){
                            hbeat=cfg->trnc_hbn;
                            mb1_connected=true;
                            msock_set_blocking(isock,false);
                            err_count=0;
                            mlog_tprintf(cfg->log_id, "mb1svr input mb1_connected [%s:%d]\n", cfg->mb1_host, cfg->mb1_port);
                        }
                    }
                }else{
                    fprintf(stderr,"%s:%d ERR - msock_connect [%s:%d] failed [%d][%d/%s]\n", __func__, __LINE__, cfg->mb1_host, cfg->mb1_port, test, errno, strerror(errno));
                    err_count++;
                }
            }

            if(!mb1_connected) sleep(TRNCLI_TEST_CONNECT_DELAY_SEC);

        }else if( (test=s_trnc_read_mb1_rec(&mb1, isock, cfg))>0){

            if(cfg->est_n>0){
                if(++count % cfg->est_n == 0){
                    test = s_trncli_process_mb1(dcli,mb1,cfg);
                    if(test == 0){
                        // reset error count
                        err_count=0;
                    }else{
                        err_count++;
                        // these are the TRN server socket
                        switch (test) {
                            case EPIPE:
                                // mb1_connected=false;
                                mlog_tprintf(cfg->log_id, "ERR: EPIPE TRN client disconnected [%s:%d]\n", cfg->trn_cfg->trn_host, cfg->trn_cfg->trn_port);
                                tcli_connected = false;
                                break;
                            case EINTR:
                                mlog_tprintf(cfg->log_id, "ERR: EINTR s_trncli_process_mb1 [%s:%d]\n", cfg->mb1_host, cfg->mb1_port);
                                quit=true;
                                break;

                            default:
                                break;
                        }
                    }
                }// if update
            }else{
                // updates disabled, increment count
                count++;
                err_count=0;
            }

            if( hbeat-- <= 0){
                hbeat=0;
                // send hbeat
                //                    fprintf(stderr,"TX HBEAT req[%s]\n",req);
                if(msock_sendto(isock,NULL,req,4,0)==4){
                    // mbtrnpp doesn't ACK hbeat update
                    hbeat=cfg->trnc_hbn;
                }
            }

            if((cfg->state_n > 0)  && ((count % cfg->state_n) == 0) ){
                int pval=-1;
                bool bval=false;
                int err[8]={0};
                int i=0;

                mlog_oset_t log_dest = mlog_get_dest(cfg->log_id);
                mlog_set_dest(cfg->log_id, (log_dest | ML_SERR));

                fprintf(stderr,"\n");

                bval=trncli_is_intialized(dcli);
                mlog_tprintf(cfg->log_id, "is initialized [%c]\n", (bval?'Y':'N'));
                err[i++]=errno;

                bval=trncli_is_converged(dcli);
                mlog_tprintf(cfg->log_id, "is converged [%c]\n", (bval?'Y':'N'));
                err[i++]=errno;

                bval=trncli_last_meas_succesful(dcli);
                mlog_tprintf(cfg->log_id, "last meas val [%c]\n", (bval?'Y':'N'));
                err[i++]=errno;

                pval=trncli_reinit_count(dcli);
                mlog_tprintf(cfg->log_id, "reinit count [%d]\n", pval);
                err[i++]=errno;

                pval=trncli_get_filter_type(dcli);
                mlog_tprintf(cfg->log_id, "filter type [%d]\n", pval);
                err[i++]=errno;

                pval=trncli_get_filter_state(dcli);
                mlog_tprintf(cfg->log_id, "filter state [%d]\n", pval);
                err[i++]=errno;

                fprintf(stderr,"\n");

                bval=trncli_outstanding_meas(dcli);
                mlog_tprintf(cfg->log_id, "outstanding meas [%c]\n", (bval?'Y':'N'));
                err[i++]=errno;

                mlog_set_dest(cfg->log_id, log_dest);

                int j=0;
                bool recon_trn=false;
                for(j=0;j<i;j++){
                    if(err[j]!=0){
                        switch (err[j]) {
                            case EPIPE:
                                recon_trn=true;
                                break;
                            case EAGAIN:
                            case ETIMEDOUT:
                                // ignore
                                break;
                            default:
                                fprintf(stderr,"ERR[%d] - [%d/%s]\n",j,err[j],strerror(err[j]));
                                break;
                        }
                    }
                }
                if(recon_trn){
                    fprintf(stderr,"WARN - reconnecting to TRN\n");
                    tcli_connected = false;
//                    dcli = s_get_trncli_instance(cfg,true);
//                    if( (test=trncli_connect(dcli, cfg->trn_cfg->trn_host, cfg->trn_cfg->trn_port))==0){
//                        s_dbg_printf(stderr, cfg->debug, "connect OK\n");
//
//                        if(cfg->mode == MODE_UPDATE && !cfg->no_init){
//                            if(trncli_init_trn(dcli, cfg->trn_cfg) > 0){
//                                s_dbg_printf(stderr, cfg->debug, "init OK\n");
//                            } else {
//                                fprintf(stderr,"ERR - trncli_init_server failed [%d]\n",test);
//                            }
//                        } else {
//                            s_dbg_printf(stderr, cfg->debug, "skipping TRN init\n");
//                        }
//                    } else {
//                        fprintf(stderr,"ERR - trncli_connect failed [%d]\n",test);
//                        quit=true;
//                    }
                }
            }
        }else{
            // MB1 read error
            test=errno;
            err_count++;

            switch (test) {
                case EPIPE:
                    mb1_connected=false;
                    mlog_tprintf(cfg->log_id, "ERR - EPIPE input disconnected [%s:%d] ecount[%d]\n", cfg->mb1_host, cfg->mb1_port, err_count);
                    break;
                case EINTR:
                    mlog_tprintf(cfg->log_id, "ERR - EINTR s_trnc_read_mb1_rec [%s:%d] ecount[%d]\n", cfg->mb1_host, cfg->mb1_port, err_count);
                    quit=true;
                    break;
                case EAGAIN:
                    sleep(1);
                    if(err_count>10){
                        mb1_connected=false;
                        mlog_tprintf(cfg->log_id, "ERR  EAGAIN input disconnected [%s:%d] ecount[%d]\n", cfg->mb1_host, cfg->mb1_port, err_count);
                    }
                    break;

                default:
                    fprintf(stderr, "ERR - s_trnc_read_mb1_rec failed  [%d/%s]\n", errno, strerror(errno));
                    mlog_tprintf(cfg->log_id, "s_trnc_read_mb1_rec failed  [%d/%s] ecount[%d]\n", errno, strerror(errno), err_count);
                    break;
            }
        }// else MB1 read error
    }// while

    if(quit){
        s_dbg_printf(stderr, cfg->debug, "quit flag set - exiting\n");
        mlog_tprintf(cfg->log_id, "quit flag set - exiting\n");
    }
    if(g_interrupt){
        s_dbg_printf(stderr, cfg->debug, "INTERRUPTED sig[%d] - exiting\n", g_signal);
        mlog_tprintf(cfg->log_id, "INTERRUPTED sig[%d] - exiting\n", g_signal);
    }
    
    msock_socket_destroy(&isock);
    return retval;
}

static int s_trncli_test_mbin(trncli_t *dcli, app_cfg_t *cfg)
{
    int retval=0;
    mfile_file_t *mb1_file = NULL;
    mb1_file = mfile_file_new(cfg->mb1_file);
    int test=-1;
    
    if( (test=mfile_open(mb1_file, MFILE_RONLY))>0){
        bool quit=false;
        mb1_t *mb1=mb1_new(MB1_MAX_BEAMS);
        while( !g_interrupt && !quit && (test=s_read_mb1_rec(&mb1, mb1_file,cfg))>0){
            if( (test=s_trncli_process_mb1(dcli,mb1,cfg))!=0){
                if(test==EPIPE){
                    quit=true;
                }
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

static trncli_t *s_get_trncli_instance(app_cfg_t *cfg, bool force_new)
{

    static trncli_t *tcli_instance = NULL;

    if(NULL == tcli_instance){
        tcli_instance = trncli_new(cfg->utm);
        tcli_connected = false;
        tcli_initialized = false;
    } else if(force_new){
        trncli_disconnect(tcli_instance);
        trncli_destroy(&tcli_instance);
        tcli_instance = NULL;
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

    return tcli_instance;
}

static int s_app_main(app_cfg_t *cfg)
{
    int retval=-1;
    trncli_t *dcli = NULL;//trncli_new(cfg->utm);
    int test=-1;
    bool quit=false;

    s_dbg_printf(stderr, cfg->debug, "use CTRL-C to exit\n");

    while( !g_interrupt && !quit ){

        trncli_t *dcli = s_get_trncli_instance(cfg, true);

        switch (cfg->mb1_src) {
            case SRC_CSV:
                retval=s_trncli_test_csv(dcli, cfg);
                quit=true;
                break;
            case SRC_MBIN:
                retval=s_trncli_test_mbin(dcli, cfg);
                quit=true;
                break;
            case SRC_MSVR:
                retval=s_trncli_test_trnc(dcli, cfg);
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

    if( (test=trncli_disconnect(dcli)) !=0 ){
        fprintf(stderr, "ERR - trncli_disconnect failed [%d]\n", test);
    }

    trncli_destroy(&dcli);
    mlog_tprintf(cfg->log_id, "*** trncli-test session end ***\n");

    const char *log_path = mlog_path(cfg->log_id);

    mlog_close(cfg->log_id);
    mlog_delete_instance(cfg->log_id);
    if(!cfg->log_en && (NULL != log_path)){
        s_dbg_printf(stderr, cfg->debug, "removing %s\n",log_path);
        int test = remove(log_path);
        if(test !=0 ){
            fprintf(stderr, "ERR - remove [%s] failed [%d:%s]\n", log_path, errno, strerror(errno));
        }
    }
    return retval;
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
    sprintf(session_date, "%04d%02d%02d-%02d%02d%02d",
            (gmt->tm_year+1900),gmt->tm_mon+1,gmt->tm_mday,
            gmt->tm_hour,gmt->tm_min,gmt->tm_sec);

    sprintf(cfg->log_path,"%s//%s-%s-%s",cfg->log_dir,cfg->log_name,session_date,TRNCLI_TEST_LOG_EXT);
    cfg->log_id = mlog_get_instance(cfg->log_path, cfg->log_cfg, TRNCLI_TEST_LOG_NAME);

    if(!cfg->log_en){
        // log disabled, set output to none
        // creates empty log file
        mlog_set_dest(cfg->log_id, ML_NONE);
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
        int ilen=sprintf(ip," %s",argv[x]);
        ip+=ilen;
    }
    g_cmd_line[TRN_CMD_LINE_BYTES-1]='\0';
    
    mlog_open(cfg->log_id, flags, mode);
    mlog_tprintf(cfg->log_id, "*** trncli-test session start ***\n");
    mlog_tprintf(cfg->log_id, "cmdline [%s]\n", g_cmd_line);
    mlog_tprintf(cfg->log_id, "build [%s]\n", TRNCLI_TEST_BUILD);
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

    app_cfg_destroy(&cfg);

    return retval;
}
// End function main
