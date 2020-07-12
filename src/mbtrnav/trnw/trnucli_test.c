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


#define TRNUCLI_TEST_TRNU_HOST "127.0.0.1"
#define TRNUCLI_TEST_TRNU_PORT 8000
#define TRNUCLI_TEST_TRNU_HBEAT 25
#define TRNUCLI_CSV_LINE_BYTES 1024*20
#define TRNUCLI_TEST_UPDATE_N 10
#define TRNUCLI_TEST_LOG_NAME "trnucli"
#define TRNUCLI_TEST_LOG_DESC "trnu client log"
#define TRNUCLI_TEST_LOG_DIR  "."
#define TRNUCLI_TEST_LOG_EXT  ".log"
#define TRN_CMD_LINE_BYTES 2048
#define TRNUCLI_TEST_CONNECT_DELAY_SEC 5

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
    /// @var app_cfg_s::flags
    /// @brief TBD
    trnuc_flags_t flags;
    /// @var app_cfg_s::update_n
    /// @brief TBD
    int update_n;
    /// @var app_cfg_s::ofmt
    /// @brief TBD
    trnuc_fmt_t ofmt;
    /// @var app_cfg_s::demo
    /// @brief TBD
    bool demo;
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
}app_cfg_t;


static void s_show_help();

/////////////////////////
// Imports
/////////////////////////

/////////////////////////
// Module Global Variables
/////////////////////////
static bool g_interrupt=false;
static bool g_signal=0;

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
    "--verbose     : verbose output\n"
    "--help        : output help message\n"
    "--version     : output version info\n"
    "--host[:port] : TRNU server host:port\n"
    "--input       : input type (B:bin C:csv T:trnu)\n"
    "--ofmt=[pcx]  : output format (P:pretty X:hex PX:pretty_hex C:csv\n"
    "--block=[lc]  : block on connect/listen (L:listen C:connect)\n"
    "--ifile       : input file\n"
    "--update=n    : TRN update N\n"
    "--log         : enable logging\n"
    "--demo        : use trn_cli handler mechanism (demo handler, print formatted output)\n"
    "\n";
    printf("%s",help_message);
    printf("%s",usage_message);
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
        {"help", no_argument, NULL, 0},
        {"version", no_argument, NULL, 0},
        {"host", required_argument, NULL, 0},
        {"input", required_argument, NULL, 0},
        {"ofmt", required_argument, NULL, 0},
        {"block", required_argument, NULL, 0},
        {"ifile", required_argument, NULL, 0},
        {"update", required_argument, NULL, 0},
        {"demo", no_argument, NULL, 0},
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
                
                // host
                else if (strcmp("host", options[option_index].name) == 0) {
                    scpy = strdup(optarg);
                    shost = strtok(scpy,":");
                    sport = strtok(NULL,":");
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
                        case 't':
                        case 'T':
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
                // demo
                else if (strcmp("demo", options[option_index].name) == 0) {
                    cfg->demo=true;
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
    PDPRINT((stderr,"verbose    [%s]\n",(cfg->verbose?"Y":"N")));
    PDPRINT((stderr,"ifile      [%s]\n",cfg->ifile));
    PDPRINT((stderr,"input_src  [%d]\n",cfg->input_src));
    PDPRINT((stderr,"host       [%s]\n",cfg->trnu_host));
    PDPRINT((stderr,"port       [%d]\n",cfg->trnu_port));
    PDPRINT((stderr,"trnu_hbeat [%d]\n",cfg->trnu_hbeat));
    PDPRINT((stderr,"ofmt       [%02x]\n",cfg->ofmt));
    PDPRINT((stderr,"update_n   [%u]\n",cfg->update_n));
    PDPRINT((stderr,"demo       [%c]\n",(cfg->demo?'Y':'N')));
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
            PDPRINT((stderr,"sig received[%d]\n",signum));
            g_interrupt=true;
            g_signal=signum;
            break;
        default:
            fprintf(stderr,"s_termination_handler: sig not handled[%d]\n",signum);
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
        instance->ifile=NULL;
        instance->input_src=SRC_TRNU;

        instance->trnu_host=strdup(TRNUCLI_TEST_TRNU_HOST);
        instance->trnu_port=TRNUCLI_TEST_TRNU_PORT;
        instance->trnu_hbeat=TRNUCLI_TEST_TRNU_HBEAT;
        instance->update_n=TRNUCLI_TEST_UPDATE_N;
        instance->log_cfg=mlog_config_new(ML_TFMT_ISO1806,ML_DFL_DEL,ML_MONO|ML_NOLIMIT,ML_FILE,0,0,0);
        instance->log_id=MLOG_ID_INVALID;
        instance->log_name=strdup(TRNUCLI_TEST_LOG_NAME);
        instance->log_dir=strdup(TRNUCLI_TEST_LOG_DIR);
        instance->log_path=(char *)malloc(512);
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
    int retval=-1;

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

    char g_cmd_line[TRN_CMD_LINE_BYTES]={0};
    char *ip=g_cmd_line;
    int x=0;
    for (x=0;x<argc;x++){
        if ((ip+strlen(argv[x])-g_cmd_line) > TRN_CMD_LINE_BYTES) {
            fprintf(stderr,"warning - logged cmdline truncated\n");
            break;
        }
        int ilen=sprintf(ip," %s",argv[x]);
        ip+=ilen;
    }
    g_cmd_line[TRN_CMD_LINE_BYTES-1]='\0';

    mlog_open(cfg->log_id, flags, mode);
    mlog_tprintf(cfg->log_id,"*** trnucli-test session start ***\n");
    mlog_tprintf(cfg->log_id,"cmdline [%s]\n",g_cmd_line);
    mlog_tprintf(cfg->log_id,"build [%s]\n",TRNUCLI_TEST_BUILD);

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
                fprintf(stderr,"tokenize failed [%d]\n",test);
            }
        }else{
            fprintf(stderr,"read_csv_rec failed [%d]\n",test);
        }
    }else{
        fprintf(stderr,"invalid argument d[%p] s[%p]\n",dest,src);
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
    	fprintf(stdout,"%s\n",str);
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

static int s_trnucli_test_trnu(app_cfg_t *cfg)
{
    int retval=0;
    int test=-1;
    trnucli_t *dcli = trnucli_new(NULL,cfg->flags,0.0);
    if( (test=trnucli_connect(dcli, cfg->trnu_host, cfg->trnu_port))==0){
        fprintf(stderr,"trnucli_connect [%d]\n",test);
    }else{
        fprintf(stderr,"trnucli_connect failed [%d]\n",test);
    }
    fprintf(stderr,"%s - running\n",__func__);
    // could set handler here (called by listener)...
    if(cfg->demo){
    	trnucli_set_callback(dcli,s_update_callback);
    }
    while(!g_interrupt){
        if( (test=trnucli_listen(dcli))==0){

            // could call handler or handle here
            if(!cfg->demo){
	            s_trnucli_process_update(dcli->update,cfg);
            }
        }else{
            fprintf(stderr,"listen ret[%d]\n",test);
        }
//        sleep(1);
    }

    if( (test=trnucli_disconnect(dcli))!=0){
        fprintf(stderr,"trnucli_disconnect failed [%d]\n",test);
    }

    trnucli_destroy(&dcli);

    if(g_interrupt){
        mlog_tprintf(cfg->log_id,"INTERRUPTED sig[%d] - exiting\n",g_signal);
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
        fprintf(stderr,"%s - mfile_open [%s] failed [%d][%d/%s]\n",__func__,cfg->ifile,test,errno,strerror(errno));
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
                retval=s_trnucli_test_trnu(cfg);
                break;
            default:
                fprintf(stderr,"invalid input type [%d]\n",cfg->input_src);
                break;
        }
    }else{
        fprintf(stderr,"ERR - NULL config\n");
    }

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
