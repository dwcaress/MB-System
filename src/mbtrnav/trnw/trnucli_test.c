///
/// @file trnucli_test.c
/// @authors k. Headley
/// @date 12 jun 2019

/// Unit test wrapper for trn_cli

/// Compile test using
/// gcc -o trnu-cli trnucli-test.c trn_cli.c -L../bin -lmframe
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
#include "trnu_cli.h"
#include "mframe.h"
#include "mfile.h"
#include "msocket.h"
#include "mlog.h"
#include "mtime.h"
#include "medebug.h"
#include "trn_msg.h"

/////////////////////////
// Macros
/////////////////////////
#define TRNUCLI_TEST_NAME "trnucli-test"
#ifndef TRNUCLI_TEST_BUILD
/// @def TRNUCLI_TEST_BUILD
/// @brief module build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DMBTRN_BUILD=`date`
#define TRNUCLI_TEST_BUILD ""VERSION_STRING(APP_BUILD)
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


#define TRNUC_SRC_CSV_STR  "csv"
#define TRNUC_SRC_TRNU_STR "svr"
#define TRNUC_SRC_BIN_STR  "bin"

#define HOSTNAME_BUF_LEN 256

/////////////////////////
// Declarations
/////////////////////////

typedef enum {
    SRC_CSV=0,
    SRC_TRNU,
    SRC_BIN
}trnucli_src_type;

/// @typedef struct app_cfg_s app_cfg_t
/// @brief application configuration parameter structure
typedef struct app_cfg_s{
    /// @var app_cfg_s::verbose
    /// @brief TBD
    bool verbose;
    /// @var app_cfg_s::ifile
    /// @brief TBD
    char *ifile;
    /// @var app_cfg_s::input_src
    /// @brief TBD
    trnucli_src_type input_src;
    /// @var app_cfg_s::trnu_host
    /// @brief TBD
    char *trnu_host;
    /// @var app_cfg_s::trnu_port
    /// @brief TBD
    int trnu_port;
    /// @var app_cfg_s::trnu_hbeat
    /// @brief TBD
    int trnu_hbeat;
    /// @var app_cfg_s::hbeat_to_sec
    /// @brief TBD
    double hbeat_to_sec;
    /// @var app_cfg_s::flags
    /// @brief TBD
    trnuc_flags_t flags;
    /// @var app_cfg_s::update_n
    /// @brief TBD
    int update_n;
    /// @var app_cfg_s::ofmt
    /// @brief TBD
    trnuc_fmt_t ofmt;
    /// @var app_cfg_s::ofile
    /// @brief TBD
    FILE *ofile;
    /// @var app_cfg_s::demo
    /// @brief TBD
    int demo;
    /// @var app_cfg_s::test_reset_mod
    /// @brief TBD
    int test_reset_mod;
    /// @var app_cfg_s::async
    /// @brief TBD
    uint32_t async;
    /// @var app_cfg_s::session_timer
    /// @brief TBD
    double session_timer;
    /// @var app_cfg_s::recon_timer
    /// @brief TBD
    double recon_timer;
    /// @var app_cfg_s::recon
    /// @brief TBD
    double recon_to_sec;
    /// @var app_cfg_s::stats_log_period_sec
    /// @brief TBD
    double stats_log_period_sec;
    /// @var app_cfg_s::listen_to_msec
    /// @brief TBD
    uint32_t listen_to_ms;
    /// @var app_cfg_s::nddelms
    /// @brief TBD
    uint32_t enodata_delay_ms;
    /// @var app_cfg_s::rcdelms
    /// @brief TBD
    uint32_t erecon_delay_ms;
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
    /// @var app_cfg_s::log_en
    /// @brief TBD
    bool log_en;
}app_cfg_t;

#define TRNUCLI_TEST_TRNU_PORT 8000
#define TRNUCLI_TEST_TRNU_HBEAT 25
#define TRNUCLI_TEST_CSV_LINE_BYTES 1024*20
#define TRNUCLI_TEST_UPDATE_N 10
#define TRNUCLI_TEST_LOG_NAME "trnucli"
#define TRNUCLI_TEST_LOG_DESC "trnu client log"
#define TRNUCLI_TEST_LOG_DIR  "."
#define TRNUCLI_TEST_LOG_EXT  ".log"
#define TRNUCLI_TEST_CMD_LINE_BYTES 2048
#define TRNUCLI_TEST_CONNECT_WAIT_SEC 5
#define TRNUCLI_TEST_ELISTEN_RETRIES 5
#define TRNUCLI_TEST_ELISTEN_WAIT 3
#define TRNUCLI_TEST_ENODATA_DELAY_MSEC  50
#define TRNUCLI_TEST_ERECON_DELAY_MSEC  5000
#define TRNUCLI_TEST_RECON_TMOUT_SEC 10.0
#define TRNUCLI_TEST_HBEAT_TMOUT_SEC 0.0
#define TRNUCLI_TEST_LISTEN_TMOUT_MSEC 50
#define TRNUCLI_TEST_LOG_EN true
#define TRNUCLI_TEST_STATS_LOG_PERIOD_SEC 60.0
#define TRNUCLI_TEST_OFILE stdout
#define TRNUCLI_TEST_OFMT TRNUC_FMT_PRETTY
#define TRNUCLI_TEST_SRC SRC_TRNU

static void s_show_help();
static const char *s_app_ofmt2str(trnuc_fmt_t fmt);
static const char *s_app_ofile2str(FILE *file);
static const char *s_app_input2str(trnucli_src_type src);
static int s_app_cfg_show(app_cfg_t *self,bool verbose, int indent);

/////////////////////////
// Imports
/////////////////////////

/////////////////////////
// Module Global Variables
/////////////////////////
static bool g_interrupt=false;
static bool g_signal=0;
const char *ofmt_strings[]={
    TRNUC_FMT_PRETTY_STR,
    TRNUC_FMT_CSV_STR,
    TRNUC_FMT_HEX_STR,
    TRNUC_FMT_PRETTY_HEX_STR
};

const char *ofile_strings[]={
    TRNUC_OFILE_SOUT_STR,
    TRNUC_OFILE_SERR_STR
};

const char *src_strings[]={
    TRNUC_SRC_CSV_STR,
    TRNUC_SRC_TRNU_STR,
    TRNUC_SRC_BIN_STR
};

/////////////////////////
// Function Definitions
/////////////////////////

/// @fn void s_show_help()
/// @brief output user help message to stdout.
/// @return none
static void s_show_help()
{
    char help_message[] = "\nTRNU client (trnu_cli) test\n";
    char usage_message[] = "\ntrnucli-test [options]\n"
    " --verbose     : verbose output\n"
    " --help        : output help message\n"
    " --version     : output version info\n"
    " --host[:port] : TRNU server host:port\n"
    " --input=[bcs] : input type (B:bin C:csv S:socket)\n"
    " --ofmt=[pcx]  : output format (P:pretty X:hex PX:pretty_hex C:csv\n"
    " --serr        : send updates to stderr (default is stdout)\n"
    " --ifile       : input file\n"
    " --hbtos       : heartbeat period (sec, 0.0 to disable)\n"
    "\ntrncli API options:\n"
    " --block=[lc]  : block on connect/listen (L:listen C:connect)\n"
    " --update=n    : TRN update N\n"
    " --demo=n      : use trn_cli handler mechanism, w/ periodic TRN resets (mod n)\n"
    " --test-reset=n : enable periodic TRN resets (mod n)\n"
    "\ntrncli_ctx API options:\n"
    " --rctos=n     : reconnect timeout sec (reconnect if no message received for n sec)\n"
    " --nddelms=n   : delay n ms on listen error\n"
    " --ltoms=n     : listen timeuot msec\n"
    " --rcdelms=n   : delay n ms on reconnect error\n"
    " --no-log      : disable client logging\n"
    " --logstats=f  : async client stats log period (sec, <=0.0 to disable)\n"
    " --async=n     : use asynchronous implementation, show status every n msec\n"
    "\n"
    " Example:\n"
    " # async client\n"
    " trnucli-test --host=<trnsvr IP>[:<port>] --input=S --ofmt=p --async=3000\n"
    "\n";
    printf("%s",help_message);
    printf("%s",usage_message);
    int wkey=10;
    int wval=10;
    printf(" Defaults:\n");
    printf("%*s  %*d\n",wkey,"port",wval,TRNUCLI_TEST_TRNU_PORT);
    printf("%*s  %*s\n",wkey,"input",wval,s_app_input2str(TRNUCLI_TEST_SRC));
    printf("%*s  %*s\n",wkey,"ofmt",wval,s_app_ofmt2str(TRNUCLI_TEST_OFMT));
    printf("%*s  %*.1lf\n",wkey,"hbtos",wval,TRNUCLI_TEST_HBEAT_TMOUT_SEC);
    printf("%*s  %*d\n",wkey,"update",wval,TRNUCLI_TEST_UPDATE_N);
    printf("%*s  %*.3lf\n", wkey,"rctos",wval,TRNUCLI_TEST_RECON_TMOUT_SEC);
    printf("%*s  %*"PRIu32"\n",wkey,"ltoms",wval,TRNUCLI_TEST_LISTEN_TMOUT_MSEC);
    printf("%*s  %*"PRIu32"\n",wkey,"nddelms",wval,TRNUCLI_TEST_ENODATA_DELAY_MSEC);
    printf("%*s  %*"PRIu32"\n",wkey,"rcdelms",wval,TRNUCLI_TEST_ERECON_DELAY_MSEC);
    printf("%*s  %*.3lf\n",wkey,"logstats",wval,TRNUCLI_TEST_STATS_LOG_PERIOD_SEC);
    printf("%*s  %*s\n",wkey,"log_en",wval,(TRNUCLI_TEST_LOG_EN?"Y":"N"));
    printf("%*s  %*s\n",wkey,"ofile",wval,s_app_ofile2str(TRNUCLI_TEST_OFILE));
    printf("\n");
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
        {"help", no_argument, NULL, 0},
        {"verbose", no_argument, NULL, 0},
        {"version", no_argument, NULL, 0},
        {"no-log", no_argument, NULL, 0},
        {"serr", no_argument, NULL, 0},
        {"host", required_argument, NULL, 0},
        {"input", required_argument, NULL, 0},
        {"ofmt", required_argument, NULL, 0},
        {"block", required_argument, NULL, 0},
        {"ifile", required_argument, NULL, 0},
        {"update", required_argument, NULL, 0},
        {"hbtos", required_argument, NULL, 0},
        {"rctos", required_argument, NULL, 0},
        {"ltoms", required_argument, NULL, 0},
        {"nddelms", required_argument, NULL, 0},
        {"rcdelms", required_argument, NULL, 0},
        {"demo", required_argument, NULL, 0},
        {"test-reset", required_argument, NULL, 0},
        {"async", required_argument, NULL, 0},
        {"logstats", required_argument, NULL, 0},
        {NULL, 0, NULL, 0}};
 
    // process argument list
    while ((c = getopt_long(argc, argv, "", options, &option_index)) != -1){
        char *scpy = NULL;
        char *shost = NULL;
        char *sport = NULL;
        switch (c) {
                // long options all return c=0
            case 0:
                // verbose
                if (strcmp("verbose", options[option_index].name) == 0) {
                    cfg->verbose=true;
                }
                
                // help
                else if (strcmp("help", options[option_index].name) == 0) {
                    help = true;
                }
                // version
                else if (strcmp("version", options[option_index].name) == 0) {
                    version = true;
                }
                // no-log
                else if (strcmp("no-log", options[option_index].name) == 0) {
                    cfg->log_en = false;
                }
                // host
                else if (strcmp("host", options[option_index].name) == 0) {
                    scpy = strdup(optarg);
                    char *cp=scpy;
                    while(isspace(*cp) && *cp!='\0' )cp++;

                    if(*cp==':'){
                        shost=NULL;
                        sport = strtok(cp,":");
                    }else{
                    	shost = strtok(cp,":");
                    	sport = strtok(NULL,":");
                    }
                    if(NULL!=shost){
                        if(NULL!=cfg->trnu_host){
                            free(cfg->trnu_host);
                        }
                        cfg->trnu_host=strdup(shost);
                    }
                    if(NULL!=sport)
                        cfg->trnu_port=atoi(sport);
                    free(scpy);
                }

                // input
                else if (strcmp("input", options[option_index].name) == 0) {
                    sscanf(optarg,"%c",&cmnem);
                    switch (cmnem) {
                        case 'c':
                        case 'C':
                            cfg->input_src=SRC_CSV;
                            break;
                        case 'b':
                        case 'B':
                            cfg->input_src=SRC_BIN;
                            break;
                        case 's':
                        case 'S':
                            cfg->input_src=SRC_TRNU;
                            break;
                       default:
                            fprintf(stderr,"WARN - invalid input_src[%c]\n",cmnem);
                            break;
                    }
                }
                // ifile
                else if (strcmp("ifile", options[option_index].name) == 0) {
                    if(NULL!=cfg->ifile){
                        free(cfg->ifile);
                    }
                    cfg->ifile=(NULL!=optarg?strdup(optarg):NULL);
                }
                // serr
                else if (strcmp("serr", options[option_index].name) == 0) {
                    cfg->ofile=stderr;
                }
                // update
                else if (strcmp("update", options[option_index].name) == 0) {
                    sscanf(optarg,"%d",&cfg->update_n);
                }
                // ofmt
                else if (strcmp("ofmt", options[option_index].name) == 0) {
                    if(strcmp(optarg,"P")==0 || strcmp(optarg,"p")==0){
                        cfg->ofmt=TRNUC_FMT_PRETTY;
                    }
                    if(strcmp(optarg,"X")==0 || strcmp(optarg,"x")==0){
                        cfg->ofmt=TRNUC_FMT_HEX;
                    }
                    if(strcmp(optarg,"C")==0 || strcmp(optarg,"c")==0){
                        cfg->ofmt=TRNUC_FMT_CSV;
                   }
                    if(strcmp(optarg,"PX")==0 || strcmp(optarg,"px")==0){
                        cfg->ofmt=TRNUC_FMT_PRETTY_HEX;
                    }
                }
                // block
                else if (strcmp("block", options[option_index].name) == 0) {
                    if(strstr(optarg,"c")!=NULL || strstr(optarg,"C")!=NULL )
                        cfg->flags|=TRNUC_BLK_CON;
                    if(strstr(optarg,"l")!=NULL || strstr(optarg,"L")!=NULL )
                        cfg->flags|=TRNUC_BLK_LISTEN;
                }
                // hbtos
                else if (strcmp("hbtos", options[option_index].name) == 0) {
                    sscanf(optarg,"%lf",&cfg->hbeat_to_sec);
                }
                // rctos
                else if (strcmp("rctos", options[option_index].name) == 0) {
                    sscanf(optarg,"%lf",&cfg->recon_to_sec);
                }
                // ltoms
                else if (strcmp("ltoms", options[option_index].name) == 0) {
                    sscanf(optarg,"%"PRIu32"",&cfg->listen_to_ms);
                }
                // nddelms
                else if (strcmp("nddelms", options[option_index].name) == 0) {
                    sscanf(optarg,"%"PRIu32"",&cfg->enodata_delay_ms);
                }
                // rcdelms
                else if (strcmp("rcdelms", options[option_index].name) == 0) {
                    sscanf(optarg,"%"PRIu32"",&cfg->erecon_delay_ms);
                }
                // demo
                else if (strcmp("demo", options[option_index].name) == 0) {
                    sscanf(optarg,"%d",&cfg->demo);
                }
                // test-reset
                else if (strcmp("test-reset", options[option_index].name) == 0) {
                    sscanf(optarg,"%d",&cfg->test_reset_mod);
                }
                // async
                else if (strcmp("async", options[option_index].name) == 0) {
                    sscanf(optarg,"%"PRIu32"",&cfg->async);
                }
                // logstats
                else if (strcmp("logstats", options[option_index].name) == 0) {
                    sscanf(optarg,"%lf",&cfg->stats_log_period_sec);
                }

                break;
            default:
                help=true;
                break;
        }
        if (version) {
            //            mbtrn_show_app_version(TCPC_NAME,TCPC_BUILD);
            fprintf(stderr,"%s build %s\n",TRNUCLI_TEST_NAME,TRNUCLI_TEST_BUILD);
            exit(0);
        }
        if (help) {
            //            mbtrn_show_app_version(TCPC_NAME,TCPC_BUILD);
            s_show_help();
            exit(0);
        }
    }// while

    // use this host if unset
    if(NULL==cfg->trnu_host){
        // if unset, use local IP
        char host[HOSTNAME_BUF_LEN]={0};
        if(gethostname(host, HOSTNAME_BUF_LEN)==0 && strlen(host)>0){
            struct hostent *host_entry;

            if( (host_entry = gethostbyname(host))!=NULL){
                //Convert into IP string
                char *s =inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));
                cfg->trnu_host = strdup(s);
            } //find host information
        }

        if(NULL==cfg->trnu_host){
            cfg->trnu_host=strdup("localhost");
        }
    }

    if(cfg->verbose){
        fprintf(stderr," Configuration:\n");
        s_app_cfg_show(cfg,true,5);
        fprintf(stderr,"\n");
    }

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
            PDPRINT((stderr,"INFO - sig received[%d]\n",signum));
            g_interrupt=true;
            g_signal=signum;
            break;
        default:
            fprintf(stderr,"ERR - s_termination_handler: sig not handled[%d]\n",signum);
            break;
    }
}
// End function termination_handler


static const char *s_app_ofmt2str(trnuc_fmt_t fmt)
{
    const char *retval=NULL;
    switch (fmt) {
        case TRNUC_FMT_PRETTY:
            retval=ofmt_strings[0];
            break;
        case TRNUC_FMT_CSV:
            retval=ofmt_strings[1];
            break;
        case TRNUC_FMT_HEX:
            retval=ofmt_strings[2];
            break;
        case TRNUC_FMT_PRETTY_HEX:
            retval=ofmt_strings[3];
            break;
        default:
            break;
    }
    return retval;
}

static const char *s_app_ofile2str(FILE *file)
{
    const char *retval=NULL;
    if(file==stdout)
        retval=ofile_strings[0];
    else if(file==stderr)
        retval=ofile_strings[1];
    return retval;
}

static const char *s_app_input2str(trnucli_src_type src)
{
    const char *retval=NULL;
    switch (src) {
        case SRC_CSV:
            retval=src_strings[0];
            break;
        case SRC_TRNU:
            retval=src_strings[1];
            break;
        case SRC_BIN:
            retval=src_strings[2];
            break;

        default:
            break;
    }
    return retval;
}

static app_cfg_t *app_cfg_new()
{
    app_cfg_t *instance = (app_cfg_t *)malloc(sizeof(app_cfg_t));
    
    if(NULL!=instance){
        memset(instance,0,sizeof(app_cfg_t));
        instance->verbose=false;
        instance->ifile=NULL;
        instance->input_src=TRNUCLI_TEST_SRC;

        instance->trnu_host=NULL;
        instance->trnu_port=TRNUCLI_TEST_TRNU_PORT;
        instance->ofmt=TRNUCLI_TEST_OFMT;
        instance->trnu_hbeat=TRNUCLI_TEST_TRNU_HBEAT;
        instance->hbeat_to_sec=TRNUCLI_TEST_HBEAT_TMOUT_SEC;
        instance->update_n=TRNUCLI_TEST_UPDATE_N;
        instance->log_cfg=mlog_config_new(ML_TFMT_ISO1806,ML_DFL_DEL,ML_MONO|ML_NOLIMIT,ML_FILE,0,0,0);
        instance->log_id=MLOG_ID_INVALID;
        instance->log_name=strdup(TRNUCLI_TEST_LOG_NAME);
        instance->log_dir=strdup(TRNUCLI_TEST_LOG_DIR);
        instance->log_path=(char *)malloc(512);
        instance->recon_to_sec=TRNUCLI_TEST_RECON_TMOUT_SEC;
        instance->recon_timer=0.0;
        instance->listen_to_ms=TRNUCLI_TEST_LISTEN_TMOUT_MSEC;
        instance->enodata_delay_ms=TRNUCLI_TEST_ENODATA_DELAY_MSEC;
        instance->erecon_delay_ms=TRNUCLI_TEST_ERECON_DELAY_MSEC;
        instance->stats_log_period_sec=TRNUCLI_TEST_STATS_LOG_PERIOD_SEC;
        instance->log_en=TRNUCLI_TEST_LOG_EN;
        instance->ofile=TRNUCLI_TEST_OFILE;
    }
    return instance;
}

static void app_cfg_destroy(app_cfg_t **pself)
{
    if(NULL!=pself){
        app_cfg_t *self = (app_cfg_t *)(*pself);
        if(NULL!=self){
            if(NULL!=self->ifile)
            free(self->ifile);
            if(NULL!=self->trnu_host)
            free(self->trnu_host);
            if(NULL!=self->log_name)
            free(self->log_name);
            if(NULL!=self->log_dir)
                free(self->log_dir);
            if(NULL!=self->log_path)
                free(self->log_path);

            mlog_delete_instance(self->log_id);
            mlog_config_destroy(&self->log_cfg);

            free(self);
            *pself=NULL;
        }
    }
    return;
}

static int s_app_cfg_show(app_cfg_t *self,bool verbose, int indent)
{
    int retval=0;
    int wkey=15;
    int wval=14;
    fprintf(stderr,"%*s%*s  %*s\n",indent,(indent>0?" ":""),wkey,"verbose",wval,(self->verbose?"Y":"N"));
    fprintf(stderr,"%*s%*s  %*s\n",indent,(indent>0?" ":""),wkey,"host",wval,self->trnu_host);
    fprintf(stderr,"%*s%*s  %*d\n",indent,(indent>0?" ":""),wkey,"port",wval,self->trnu_port);

    fprintf(stderr,"%*s%*s  %*s\n",indent,(indent>0?" ":""),wkey,"input_src",wval,s_app_input2str(self->input_src));
    fprintf(stderr,"%*s%*s  %*u\n",indent,(indent>0?" ":""),wkey,"async",wval,self->async);
    fprintf(stderr,"%*s%*s  %*d\n",indent,(indent>0?" ":""),wkey,"demo",wval,self->demo);
    fprintf(stderr,"%*s%*s  %*d\n",indent,(indent>0?" ":""),wkey,"test_reset_mod",wval,self->test_reset_mod);
    fprintf(stderr,"%*s%*s  %*s\n",indent,(indent>0?" ":""),wkey,"ifile",wval,self->ifile);
    fprintf(stderr,"%*s%*s  %*s\n",indent,(indent>0?" ":""),wkey,"ofmt",wval,s_app_ofmt2str(self->ofmt));
    fprintf(stderr,"%*s%*s  %*s\n",indent,(indent>0?" ":""),wkey,"out",wval,s_app_ofile2str(self->ofile));

    fprintf(stderr,"%*s%*s  %*.3lf\n",indent,(indent>0?" ":""),wkey,"hbtos",wval,self->hbeat_to_sec);
    fprintf(stderr,"%*s%*s  %*.3lf\n",indent,(indent>0?" ":""),wkey,"rctos",wval,self->recon_to_sec);
    fprintf(stderr,"%*s%*s  %*"PRIu32"\n",indent,(indent>0?" ":""),wkey,"ltoms",wval,self->listen_to_ms);
    fprintf(stderr,"%*s%*s  %*"PRIu32"\n",indent,(indent>0?" ":""),wkey,"nddelms",wval,self->enodata_delay_ms);
    fprintf(stderr,"%*s%*s  %*"PRIu32"\n",indent,(indent>0?" ":""),wkey,"rcdelms",wval,self->erecon_delay_ms);
    fprintf(stderr,"%*s%*s  %*.3lf\n",indent,(indent>0?" ":""),wkey,"logstats",wval,self->stats_log_period_sec);
    fprintf(stderr,"%*s%*s  %*s\n",indent,(indent>0?" ":""),wkey,"no-log",wval,(self->verbose?"Y":"N"));

    fprintf(stderr,"%*s%*s  %*u\n",indent,(indent>0?" ":""),wkey,"update_n",wval,self->update_n);
    fprintf(stderr,"%*s%*s  %*d\n",indent,(indent>0?" ":""),wkey,"trnu_hbeat",wval,self->trnu_hbeat);

    return retval;
}

static int s_tokenize(char *src, char ***dest, char *del, int ntok)
{
    int i=-1;
    if(NULL!=src && NULL!=del){
        if(*dest==NULL){
            *dest = (char **)malloc(ntok*sizeof(char *));
        }
        i=0;
        char **fields=*dest;
        while(i<ntok){
            fields[i]=strtok((i==0?src:NULL),del);
            if(NULL==fields[i]){
                break;
            }
            i++;
	    }
    }
    return i;
}

static int s_update_callback(trnu_pub_t *update)
{
    int retval=0;

    // demo callback : call the string formatter and output
    char *str=NULL;
    trnucli_update_str(update,&str,0,TRNUC_FMT_PRETTY);
    if(NULL!=str){
    	fprintf(stdout,"%s\n",str);
        free(str);
    }
    str=NULL;

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
    cfg->session_timer=mtime_etime();
    // Get GMT time
    gmt = gmtime(&rawtime);
    // format YYYYMMDD-HHMMSS
    sprintf(session_date, "%04d%02d%02d-%02d%02d%02d",
            (gmt->tm_year+1900),gmt->tm_mon+1,gmt->tm_mday,
            gmt->tm_hour,gmt->tm_min,gmt->tm_sec);


    sprintf(cfg->log_path,"%s//%s-%s-%s",cfg->log_dir,cfg->log_name,session_date,TRNUCLI_TEST_LOG_EXT);

    cfg->log_id = mlog_get_instance(cfg->log_path, cfg->log_cfg, TRNUCLI_TEST_LOG_NAME);


    mfile_flags_t flags = MFILE_RDWR|MFILE_APPEND|MFILE_CREATE;
    mfile_mode_t mode = MFILE_RU|MFILE_WU|MFILE_RG|MFILE_WG;

    char g_cmd_line[TRNUCLI_TEST_CMD_LINE_BYTES]={0};
    char *ip=g_cmd_line;
    int x=0;
    for (x=0;x<argc;x++){
        if ((ip+strlen(argv[x])-g_cmd_line) > TRNUCLI_TEST_CMD_LINE_BYTES) {
            fprintf(stderr,"WARN - logged cmdline truncated\n");
            break;
        }
        int ilen=sprintf(ip," %s",argv[x]);
        ip+=ilen;
    }
    g_cmd_line[TRNUCLI_TEST_CMD_LINE_BYTES-1]='\0';

    mlog_open(cfg->log_id, flags, mode);
    mlog_tprintf(cfg->log_id,"*** trnucli-test session start ***\n");
    mlog_tprintf(cfg->log_id,"start_time,%.3lf\n",cfg->session_timer);
    mlog_tprintf(cfg->log_id,"log_id=[%d]\n",cfg->log_id);
    mlog_tprintf(cfg->log_id,"cmdline [%s]\n",g_cmd_line);
    mlog_tprintf(cfg->log_id,"build [%s]\n",TRNUCLI_TEST_BUILD);

    return;
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

static int s_csv_to_update_org(trnu_pub_t *dest, mfile_file_t *src)
{
    int retval=-1;
//    fprintf(stderr,"WARN - %s: not implemented\n",__func__);

    if(NULL!=dest && NULL!=src){
        char line[TRNUC_CSV_LINE_BYTES]={0};
        char **fields=NULL;
        int32_t test = 0;
        if( (test=s_read_csv_rec(src,line,TRNUC_CSV_LINE_BYTES))>0){
            fprintf(stderr,"read csvline:\n%s\n",line);
            if( (test=s_tokenize(line, &fields, ",", TRNUC_CSV_FIELDS))==TRNUC_CSV_FIELDS){

                sscanf(fields[0],"%lf",&dest->mb1_time);
                sscanf(fields[1],"%lf",&dest->update_time);
                sscanf(fields[2],"%08X",&dest->sync);
                sscanf(fields[3],"%d",&dest->reinit_count);
                sscanf(fields[4],"%lf",&dest->reinit_tlast);
                sscanf(fields[5],"%d",&dest->filter_state);
                sscanf(fields[6],"%d",&dest->success);
                sscanf(fields[7],"%hd",&dest->is_converged);
                sscanf(fields[8],"%hd",&dest->is_valid);
                sscanf(fields[9],"%d",&dest->mb1_cycle);
                sscanf(fields[10],"%d",&dest->ping_number);
                unsigned int i=0;
                for(i=0;i<3;i++){
                    int j=11+(i*7);
                    sscanf(fields[j+0],"%lf",&dest->est[i].x);
                    sscanf(fields[j+1],"%lf",&dest->est[i].y);
                    sscanf(fields[j+2],"%lf",&dest->est[i].z);
                    sscanf(fields[j+3],"%lf",&dest->est[i].cov[0]);
                    sscanf(fields[j+4],"%lf",&dest->est[i].cov[1]);
                    sscanf(fields[j+5],"%lf",&dest->est[i].cov[2]);
                    sscanf(fields[j+6],"%lf",&dest->est[i].cov[3]);
                }
                retval=test;
                free(fields);
            }else{
                fprintf(stderr,"ERR - tokenize failed [%d]\n",test);
            }
        }else{
            fprintf(stderr,"ERR - read_csv_rec failed [%d]\n",test);
        }
    }else{
        fprintf(stderr,"ERR - invalid argument d[%p] s[%p]\n",dest,src);
    }
    return retval;
}

static int s_csv_to_update(trnu_pub_t *dest, mfile_file_t *src)
{
    int retval=-1;
    //    fprintf(stderr,"WARN - %s: not implemented\n",__func__);

    if(NULL!=dest && NULL!=src){
        char line[TRNUC_CSV_LINE_BYTES]={0};
        char **fields=NULL;
        int32_t test = 0;
        if( (test=s_read_csv_rec(src,line,TRNUC_CSV_LINE_BYTES))>0){
            fprintf(stderr,"read csvline:\n%s\n",line);
            if( (test=s_tokenize(line, &fields, ",", TRNUC_CSV_FIELDS))==TRNUC_CSV_FIELDS){

                sscanf(fields[0],"%lf",&dest->mb1_time);
                sscanf(fields[1],"%lf",&dest->update_time);
                sscanf(fields[1],"%lf",&dest->reinit_time);
                sscanf(fields[2],"%08X",&dest->sync);
                sscanf(fields[3],"%d",&dest->reinit_count);
                sscanf(fields[4],"%lf",&dest->reinit_tlast);
                sscanf(fields[5],"%d",&dest->filter_state);
                sscanf(fields[6],"%d",&dest->success);
                sscanf(fields[7],"%hd",&dest->is_converged);
                sscanf(fields[8],"%hd",&dest->is_valid);
                sscanf(fields[9],"%d",&dest->mb1_cycle);
                sscanf(fields[10],"%d",&dest->ping_number);
                sscanf(fields[10],"%d",&dest->n_con_seq);
                sscanf(fields[10],"%d",&dest->n_con_tot);
                sscanf(fields[10],"%d",&dest->n_uncon_seq);
                sscanf(fields[10],"%d",&dest->n_uncon_tot);
                unsigned int i=0;
                for(i=0;i<5;i++){
                    int j=11+(i*7);
                    sscanf(fields[j+0],"%lf",&dest->est[i].x);
                    sscanf(fields[j+1],"%lf",&dest->est[i].y);
                    sscanf(fields[j+2],"%lf",&dest->est[i].z);
                    sscanf(fields[j+3],"%lf",&dest->est[i].cov[0]);
                    sscanf(fields[j+4],"%lf",&dest->est[i].cov[1]);
                    sscanf(fields[j+5],"%lf",&dest->est[i].cov[2]);
                    sscanf(fields[j+6],"%lf",&dest->est[i].cov[3]);
                }
                retval=test;
                free(fields);
            }else{
                fprintf(stderr,"ERR - tokenize failed [%d]\n",test);
            }
        }else{
            fprintf(stderr,"ERR - read_csv_rec failed [%d]\n",test);
        }
    }else{
        fprintf(stderr,"ERR - invalid argument d[%p] s[%p]\n",dest,src);
    }
    return retval;
}

static int32_t s_fread_bin_update( trnu_pub_t *dest, mfile_file_t *src)
{
    int32_t retval=-1;

    if(NULL!=src && NULL!=dest){

        trnu_pub_t update;
        uint32_t readlen = 1;
        uint32_t record_bytes=0;
       // point to start of record
        byte *bp = (byte *)&update;
        int64_t fsize=mfile_fsize(src);

        // find sync pattern
        while(mfile_seek(src,0,MFILE_CUR) < fsize){
            if(mfile_read(src,(byte *)bp,readlen)==readlen && *bp=='\x00' ){
                bp++;
                if(mfile_read(src,(byte *)bp,readlen)==readlen && *bp=='\x54'){
                    bp++;
                    if(mfile_read(src,(byte *)bp,readlen)==readlen && *bp=='\x44'){
                        bp++;
                        if(mfile_read(src,(byte *)bp,readlen)==readlen && *bp=='\x53'){
                            record_bytes+=readlen;
                            bp++;
                            readlen=TRNU_PUB_BYTES-4;
                            break;
                        }else{
                            bp=(byte *)&dest;
                        }
                    }else{
                        bp=(byte *)&dest;
                    }
                }else{
                    bp=(byte *)&dest;
                }
            }
        }

        // if sync found, read the rest of the record
       if(record_bytes>0 && mfile_read(src,(byte *)bp,readlen)==readlen){
           record_bytes+=readlen;
           retval=record_bytes;
           // copy to destination (on success only)
           memcpy(dest,&update,TRNU_PUB_BYTES);
        }
    }
    return retval;
}

static int s_trnucli_process_update(trnu_pub_t *update, app_cfg_t *cfg)
{
    int retval=0;

    // call the string formatter and output
    char *str=NULL;
    trnucli_update_str(update,&str,0,cfg->ofmt);
    if(NULL!=str){
    	fprintf(cfg->ofile,"%s\n",str);
        free(str);
    }
    str=NULL;

    return retval;
}

static int s_trnucli_test_csv(app_cfg_t *cfg)
{
    int retval=0;
    mfile_file_t *ifile = mfile_file_new(cfg->ifile);
    int test=-1;
    
    if( (test=mfile_open(ifile, MFILE_RONLY)) >0){
        int64_t file_end=mfile_seek(ifile,0,MFILE_END);
        mfile_seek(ifile,0,MFILE_SET);
        bool quit=false;
        trnu_pub_t update_rec;
        while( !g_interrupt && !quit && (test=s_csv_to_update(&update_rec, ifile))>0){
            if( (test=s_trnucli_process_update(&update_rec,cfg))!=0){
                if(mfile_seek(ifile,0,MFILE_CUR)==file_end){
                    quit=true;
                }
            }
        }// while
    }else{
        fprintf(stderr,"mfile_open failed [%d]\n",test);
    }
    mfile_file_destroy(&ifile);
    return retval;
}
static int s_trnucli_test_trnu_async(app_cfg_t *cfg)
{
    int retval=-1;

    // configure a trnu async context instance
    // applications may handle updates, or assign an update callback function
    //    For this client:
    //    - updates handled by app (NULL handler assigned)
    //    - hbeat_to_sec     : heartbeat period
    //    - enodata_delay_ms : delay if data not available
    //    - erecon_delay_ms  : delay  if connect attempt fails
    //    - recon_to_sec     : reconnect if data unavailable
    trnucli_ctx_t *ctx = trnucli_ctx_newl(cfg->trnu_host,cfg->trnu_port,NULL,
                                          cfg->hbeat_to_sec,
                                          cfg->listen_to_ms,
                                          cfg->enodata_delay_ms,
                                          cfg->erecon_delay_ms,
                                          cfg->recon_to_sec,
                                          cfg->log_en?TRNU_LOG_EN:TRNU_LOG_DIS);

    if(NULL!=ctx){

        // configure stats logging
        trnucli_ctx_set_stats_log_period(ctx,cfg->stats_log_period_sec);

        // start the client - separate worker thread
        //  - manages connection (reconnects on timeout)
        //  - receives updates w/ optional update handler callback
        int test=trnucli_ctx_start(ctx);

        if(test==0){
            // success
            fprintf(stderr,"ctx start OK\n");
            mlog_tprintf(cfg->log_id,"host             %s\n",cfg->trnu_host);
            mlog_tprintf(cfg->log_id,"port             %d\n",cfg->trnu_port);
            mlog_tprintf(cfg->log_id,"hbeat_to_sec     %.3lf\n",cfg->hbeat_to_sec);
            mlog_tprintf(cfg->log_id,"listen_to_ms     %"PRIu32"\n",cfg->listen_to_ms);
            mlog_tprintf(cfg->log_id,"enodata_delay_ms %"PRIu32"\n",cfg->enodata_delay_ms);
            mlog_tprintf(cfg->log_id,"erecon_delay_ms  %"PRIu32"\n",cfg->erecon_delay_ms);
            mlog_tprintf(cfg->log_id,"recon_to_sec     %.3lf\n",cfg->recon_to_sec);
        }else{
            fprintf(stderr,"ERR - ctx start failed\n");
            mlog_tprintf(cfg->log_id,"ERR - ctx start failed\n");
        }

        // app does things here...
        int x=0;
        // this app prints status to demo API methods
        // until the use interrupts (CTRL-C)
        while(test==0 && !g_interrupt){

            // reinit per config
            if(NULL!=cfg && cfg->test_reset_mod>0 && x>0 && x%cfg->test_reset_mod==0 ){
                fprintf(stderr,"\nTest Reset mod/update[%d/%d]\n",cfg->test_reset_mod,x);
                int test=trnucli_ctx_reset_trn(ctx);
                fprintf(stderr,"\nReset returned[%d]\n",test);
            }else{
                fprintf(stderr,"\nSkipping Test Reset mod/update[%d/%d]\n",cfg->test_reset_mod,x);
            }
            x++;

            // show the context...
            fprintf(stderr,"\nUpdate Status\n");
            fprintf(stderr,"     updates since last read        [%"PRIu32"]\n",trnucli_ctx_new_count(ctx));
            fprintf(stderr,"     update arrival time (arrtime)  [%.3lf]\n",trnucli_ctx_update_arrtime(ctx));
            fprintf(stderr,"     update arrival age  (arrage)   [%.3lf]\n",trnucli_ctx_update_arrage(ctx));
            fprintf(stderr,"     update data time    (mb1time)  [%.3lf]\n",trnucli_ctx_update_mb1time(ctx));
            fprintf(stderr,"     update data age     (mb1age)   [%.3lf]\n",trnucli_ctx_update_mb1age(ctx));
            fprintf(stderr,"     update host time    (hosttime) [%.3lf]\n",trnucli_ctx_update_hosttime(ctx));
            fprintf(stderr,"     update host age     (hostage)  [%.3lf]\n",trnucli_ctx_update_hostage(ctx));

            // show stats...
            fprintf(stderr,"\nContext Stats\n");
            trnucli_stats_t *stats=NULL;
            trnucli_ctx_stats(ctx,&stats);
            trnucli_ctx_stat_show(stats,true,5);
            if(NULL!=stats)free(stats);

            fprintf(stderr,"\nTRN Client Context\n");
            trnucli_ctx_show(ctx,false,5);

            // show latest update...
            fprintf(stderr,"\nUpdate Data\n");
            trnu_pub_t latest={0};
            if(trnucli_ctx_last_update(ctx,&latest,NULL)==0){
                // format per config (pretty, hex, csv, etc.)
                s_trnucli_process_update(&latest,cfg);
            }

            // delay
            if(cfg->async>0)
                mtime_delay_ms(cfg->async);
        }// while !CTRL-C

        fprintf(stderr,"user interrupt - stopping\n");
        mlog_tprintf(cfg->log_id,"user interrupt - stopping\n");

        // release client resources
        fprintf(stderr,"destroying ctx\n");
        mlog_tprintf(cfg->log_id,"destroying ctx\n");
        trnucli_ctx_destroy(&ctx);
    }

    return retval;
} //  s_trnucli_test_trnu_async

static int s_trnucli_test_trnu(app_cfg_t *cfg)
{
    int retval=-1;
    int test=-1;
    static int call_count=0;
    trnucli_t *dcli = trnucli_new(NULL,cfg->flags,0.0);
    typedef enum{DISCONNECTED=0,LISTENING}trnuc_state_t;
    typedef enum{NOP=0,CONNECT,LISTEN}trnuc_action_t;
    const char *state_str[]={"DISCONNECTED","LISTENING"};
    const char *action_str[]={"NOP","CONNECT","LISTEN"};

    if(cfg->demo>0){
        // in demo mode, set a callback to
        // process updates (called by trn_cli::listen()
    	trnucli_set_callback(dcli,s_update_callback);
    }

    trnuc_state_t state=DISCONNECTED;
    trnuc_action_t action=NOP;
    int e_connect=0;
    int e_listen=0;
    int connect_count=0;
    int disconnect_count=0;
    int update_count=0;
    int reset_count=0;
    cfg->recon_timer=mtime_dtime();
    while(!g_interrupt){

        switch (state) {
            case DISCONNECTED:
                action=CONNECT;
                break;
            case LISTENING:
                action=LISTEN;
                break;
            default:
                fprintf(stderr,"ERR - invalid state [%d]\n",state);
                mlog_tprintf(cfg->log_id,"ERR - invalid state [%d]\n",state);
                action=NOP;
                break;
        }
        if(cfg->verbose)
        fprintf(stderr,"state [%d/%s] action [%d/%s]\n",state,state_str[state], action,action_str[action]);

        if(action==CONNECT){
            if( (test=trnucli_connect(dcli, cfg->trnu_host, cfg->trnu_port))==0){
                cfg->recon_timer=mtime_dtime();
                fprintf(stderr,"trnucli_connect OK [%d]\n",test);
                mlog_tprintf(cfg->log_id,"trnucli_connect OK [%d]\n",test);
                state=LISTENING;
                connect_count++;
            }else{
                fprintf(stderr,"trnucli_connect failed [%d]\n",test);
                mlog_tprintf(cfg->log_id,"trnucli_connect failed [%d]\n",test);
                e_connect++;
                sleep(TRNUCLI_TEST_CONNECT_WAIT_SEC);
            }
        }// action CONNECT

        if(action==LISTEN){
            if( (test=trnucli_listen(dcli,(cfg->demo>0)))==0){
                update_count++;

                if(cfg->demo<=0){
                    cfg->recon_timer=mtime_dtime();
                    // in normal mode, process the update here
                    s_trnucli_process_update(dcli->update,cfg);
                    if(cfg->verbose)
                    fprintf(stderr,"processed update (normal mode)\n");
                    mlog_tprintf(cfg->log_id,"processed update (normal mode)\n");
                }else{
                    cfg->recon_timer=mtime_dtime();
                    if(cfg->verbose)
                    fprintf(stderr,"processed update (demo mode)\n");
                    // in demo mode, reset TRN periodically and send heartbeat
                    if( (call_count>0) && (call_count%cfg->demo)==0){
                        int rst=trnucli_reset_trn(dcli);
                        int hbt=trnucli_hbeat(dcli);
                        reset_count++;
                        fprintf(stderr,"reset TRN [%d]\n",rst);
                        fprintf(stderr,"hbeat TRN [%d]\n",hbt);
                        mlog_tprintf(cfg->log_id,"reset TRN [%d]\n",rst);
                        mlog_tprintf(cfg->log_id,"hbeat TRN [%d]\n",hbt);
                    }
                    call_count++;
                }
            }else{
                if(cfg->verbose)
                fprintf(stderr,"ERR - listen ret[%d] rcto[%.3lf/%.3lf]\n",test,(cfg->recon_to_sec-(mtime_dtime()-cfg->recon_timer)),cfg->recon_to_sec);
                mlog_tprintf(cfg->log_id,"ERR - listen ret[%d]\n",test);
                e_listen++;
                mtime_delay_ms(cfg->enodata_delay_ms);
            }

            if(cfg->recon_to_sec>0.0 && (mtime_dtime()-cfg->recon_timer)>=cfg->recon_to_sec){
                // reconnect if timer expired
                fprintf(stderr,"ERR - recon timer expired [%.3lf] - restarting\n",cfg->recon_to_sec);
                mlog_tprintf(cfg->log_id,"ERR - recon timer expired [%.3lf] - restarting\n",cfg->recon_to_sec);
                disconnect_count++;
                trnucli_disconnect(dcli);
                state=DISCONNECTED;
                cfg->recon_timer=mtime_dtime();
            }
        }// action LISTEN

    }

    fprintf(stderr,"connect_count    [%d]\n",connect_count);
    fprintf(stderr,"disconnect_count [%d]\n",disconnect_count);
    fprintf(stderr,"reset_count      [%d]\n",reset_count);
    fprintf(stderr,"update_count     [%d]\n",update_count);
    fprintf(stderr,"e_connect        [%d]\n",e_connect);
    fprintf(stderr,"e_listen         [%d]\n",e_listen);

    mlog_tprintf(cfg->log_id,"connect_count    [%d]\n",connect_count);
    mlog_tprintf(cfg->log_id,"disconnect_count [%d]\n",disconnect_count);
    mlog_tprintf(cfg->log_id,"reset_count      [%d]\n",reset_count);
    mlog_tprintf(cfg->log_id,"update_count     [%d]\n",update_count);
    mlog_tprintf(cfg->log_id,"e_connect        [%d]\n",e_connect);
    mlog_tprintf(cfg->log_id,"e_listen         [%d]\n",e_listen);

    // disconnect from server
    if( (test=trnucli_disconnect(dcli))!=0){
        fprintf(stderr,"ERR - trnucli_disconnect failed [%d]\n",test);
    }

    // release instance
    trnucli_destroy(&dcli);

    if(g_interrupt){
        // interrupted by user
        mlog_tprintf(cfg->log_id,"INFO - Interrupted sig[%d] - exiting\n",g_signal);
        retval=0;
    }
    
    return retval;
}

static int s_trnucli_test_bin(app_cfg_t *cfg)
{
    int retval=0;
    mfile_file_t *ifile = NULL;
    ifile = mfile_file_new(cfg->ifile);
    int test=-1;
    
    if( (test=mfile_open(ifile, MFILE_RONLY))>0){
        bool quit=false;
        trnu_pub_t update_rec;
        while( !g_interrupt && !quit && (test=s_fread_bin_update(&update_rec, ifile))>0){
            if( (test=s_trnucli_process_update(&update_rec,cfg))!=0){
                if(test==EPIPE){
                    quit=true;
                }
            }
        }// while
    }else{
        fprintf(stderr,"ERR - mfile_open [%s] failed [%d][%d/%s]\n",cfg->ifile,test,errno,strerror(errno));
    }
    mfile_file_destroy(&ifile);
    return retval;
}

static int s_app_main(app_cfg_t *cfg)
{
    int retval=-1;

    if(NULL!=cfg){
        switch (cfg->input_src) {
            case SRC_CSV:
                retval=s_trnucli_test_csv(cfg);
                break;
            case SRC_BIN:
                retval=s_trnucli_test_bin(cfg);
                break;
            case SRC_TRNU:
                if(cfg->async>0){
                    retval=s_trnucli_test_trnu_async(cfg);
                }else{
                	retval=s_trnucli_test_trnu(cfg);
                }
                break;
            default:
                fprintf(stderr,"ERR - invalid input type [%d]\n",cfg->input_src);
                break;
        }
    }else{
        fprintf(stderr,"ERR - NULL config\n");
    }

    double now=mtime_etime();
    mlog_tprintf(cfg->log_id,"stop_time,%.3lf elapsed[%.3lf]\n",now,now-cfg->session_timer);
    mlog_tprintf(cfg->log_id,"*** trnucli-test session end ***\n");

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

    app_cfg_destroy(&cfg);

    return retval;
}
// End function main
