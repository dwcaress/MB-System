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
    SRC_TRNC,
    SRC_MBIN
}trncli_src_type;

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
    trncli_src_type input_src;
    /// @var app_cfg_s::trn_cfg
    /// @brief TBD
    trn_config_t *trn_cfg;
    /// @var app_cfg_s::mb_host
    /// @brief TBD
    char *trnc_host;
    /// @var app_cfg_s::mb_port
    /// @brief TBD
    int trnc_port;
    /// @var app_cfg_s::mb_hbeat
    /// @brief TBD
    int trnc_hbn;
    /// @var app_cfg_s::update_n
    /// @brief TBD
    int update_n;
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
    /// @var app_cfg_s::test_api
    /// @brief TBD
    int test_api;
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
    char help_message[] = "\nTRN client (trn_cli) test\n";
    char usage_message[] = "\ntrncli-test [options]\n"
    "--verbose   : verbose output\n"
    "--help      : output help message\n"
    "--version   : output version info\n"
    "--thost     : TRN server host\n"
    "--mhost     : MB server host\n"
    "--input     : input type (M:mb1 C:csv T:trnc)\n"
    "--ifile     : input file\n"
    "--map       : TRN server map file (dir for tiles)\n"
    "--cfg       : TRN server config file\n"
    "--particles : TRN server particle file\n"
    "--logdir    : TRN server log directory\n"
    "--ftype     : TRN server filter type\n"
    "--mtype     : TRN server map type D:DEM B:BO\n"
    "--utm       : UTM zone\n"
    "--update    : TRN update N\n"
    "--hbeat     : trn server heartbeat (modulus)\n"
    "--test-api  : query status (modulus)\n"
    "--log       : enable logging\n"
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
        {"hbeat", required_argument, NULL, 0},
        {"thost", required_argument, NULL, 0},
        {"mhost", required_argument, NULL, 0},
        {"input", required_argument, NULL, 0},
        {"ifile", required_argument, NULL, 0},
        {"map", required_argument, NULL, 0},
        {"cfg", required_argument, NULL, 0},
        {"particles", required_argument, NULL, 0},
        {"logdir", required_argument, NULL, 0},
        {"ftype", required_argument, NULL, 0},
        {"mtype", required_argument, NULL, 0},
        {"utm", required_argument, NULL, 0},
        {"update", required_argument, NULL, 0},
        {"test-api", required_argument, NULL, 0},
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
                
                // thost
                else if (strcmp("thost", options[option_index].name) == 0) {
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
//                // tport
//                else if (strcmp("tport", options[option_index].name) == 0) {
//                    sscanf(optarg,"%d",&cfg->trn_cfg->trn_port);
//                }
                // mhost
                else if (strcmp("mhost", options[option_index].name) == 0) {
                    scpy = strdup(optarg);
                    shost = strtok(scpy,":");
                    sport = strtok(NULL,":");
                    if(NULL!=shost){
                        if(NULL!=cfg->trnc_host){
                            free(cfg->trnc_host);
                        }
                        cfg->trnc_host=strdup(shost);
                    }
                    if(NULL!=sport)
                        cfg->trnc_port=atoi(sport);
                    free(scpy);
                }
//                // mport
//                else if (strcmp("mport", options[option_index].name) == 0) {
//                    sscanf(optarg,"%d",&cfg->trnc_port);
//                }
                // input
                else if (strcmp("input", options[option_index].name) == 0) {
                    sscanf(optarg,"%c",&cmnem);
                    switch (cmnem) {
                        case 'c':
                        case 'C':
                            cfg->input_src=SRC_CSV;
                            break;
                        case 'm':
                        case 'M':
                            cfg->input_src=SRC_MBIN;
                            break;
                        case 't':
                        case 'T':
                            cfg->input_src=SRC_TRNC;
                            break;
                       default:
                            fprintf(stderr,"invalid input_src[%c]\n",cmnem);
                            break;
                    }
                }
                // ifile
                else if (strcmp("ifile", options[option_index].name) == 0) {
                    if(NULL!=cfg->ifile){
                        free(cfg->ifile);
                    }
                   cfg->ifile=optarg;
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
                            fprintf(stderr,"invalid ftype[%c]\n",cmnem);
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
                            fprintf(stderr,"invalid mtype[%c]\n",cmnem);
                            break;
                    }
                }
                // utm
                else if (strcmp("utm", options[option_index].name) == 0) {
                    sscanf(optarg,"%ld",&cfg->utm);
                }
                // update
                else if (strcmp("update", options[option_index].name) == 0) {
                    sscanf(optarg,"%d",&cfg->update_n);
                }
                // hbeat
                else if (strcmp("hbeat", options[option_index].name) == 0) {
                    sscanf(optarg,"%d",&cfg->trnc_hbn);
                }
                // test-api
                else if (strcmp("test-api", options[option_index].name) == 0) {
                    sscanf(optarg,"%d",&cfg->test_api);
                }

                break;
            default:
                help=true;
                break;
        }
        if (version) {
            //            mbtrn_show_app_version(TCPC_NAME,TCPC_BUILD);
            fprintf(stderr,"%s build %s\n",TRNCLI_TEST_NAME,TRNCLI_TEST_BUILD);
            exit(0);
        }
        if (help) {
            //            mbtrn_show_app_version(TCPC_NAME,TCPC_BUILD);
            s_show_help();
            exit(0);
        }
    }// while
    PDPRINT((stderr,"verbose   [%s]\n",(cfg->verbose?"Y":"N")));
    PDPRINT((stderr,"ifile     [%s]\n",cfg->ifile));
    PDPRINT((stderr,"input_src [%d]\n",cfg->input_src));
    PDPRINT((stderr,"thost     [%s]\n",cfg->trn_cfg->trn_host));
    PDPRINT((stderr,"tport     [%d]\n",cfg->trn_cfg->trn_port));
    PDPRINT((stderr,"mhost     [%s]\n",cfg->trnc_host));
    PDPRINT((stderr,"mport     [%d]\n",cfg->trnc_port));
    PDPRINT((stderr,"utm       [%ld]\n",cfg->utm));
    PDPRINT((stderr,"hbeat     [%d]\n",cfg->trnc_hbn));
    PDPRINT((stderr,"update_n  [%u]\n",cfg->update_n));
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
        instance->ifile=strdup(TRNCLI_TEST_IFILE);
        instance->input_src=SRC_MBIN;
        instance->trn_cfg=trncfg_dnew();
        
        if(NULL!=instance->trn_cfg->map_file)free(instance->trn_cfg->map_file);
        instance->trn_cfg->map_file=strdup(TRNCLI_TEST_TRNCFG_MAP);
        if(NULL!=instance->trn_cfg->cfg_file)free(instance->trn_cfg->cfg_file);
        instance->trn_cfg->cfg_file=strdup(TRNCLI_TEST_TRNCFG_CFG);
        if(NULL!=instance->trn_cfg->particles_file)free(instance->trn_cfg->particles_file);
        instance->trn_cfg->particles_file=strdup(TRNCLI_TEST_TRNCFG_PARTICLES);
        if(NULL!=instance->trn_cfg->log_dir)free(instance->trn_cfg->log_dir);
        instance->trn_cfg->log_dir=strdup(TRNCLI_TEST_TRNCFG_LOGDIR);

        instance->trnc_host=strdup(TRNCLI_TEST_MBTRN_HOST);
        instance->trnc_port=TRNCLI_TEST_MBTRN_PORT;
        instance->trnc_hbn=TRNCLI_TEST_MBTRN_HBEAT;
        instance->update_n=TRNCLI_TEST_UPDATE_N;
        instance->log_cfg=mlog_config_new(ML_TFMT_ISO1806,ML_DFL_DEL,ML_MONO|ML_NOLIMIT,ML_FILE,0,0,0);
        instance->log_id=MLOG_ID_INVALID;
        instance->log_name=strdup(TRNCLI_TEST_LOG_NAME);
        instance->log_dir=strdup(TRNCLI_TEST_LOG_DIR);
        instance->log_path=(char *)malloc(512);
        instance->utm=TRNCLI_UTM_DFL;
        instance->test_api=0;
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
            if(NULL!=self->trnc_host)
            free(self->trnc_host);
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

static int s_csv_to_mb1(mb1_t *dest, mfile_file_t *src)
{
    int retval=-1;
    if(NULL!=dest && NULL!=src){
        char line[TRNCLI_CSV_LINE_BYTES]={0};
        char **fields=NULL;
        int32_t test = 0;
        if( (test=s_read_csv_rec(src,line,TRNCLI_CSV_LINE_BYTES))>0){
            if( (test=s_tokenize(line, fields, "\n", MB1_FIELDS))>=MB1_FIXED_FIELDS){
   
                //            sscanf(fields[0],"%u",&dest->header.type);
                //            sscanf(fields[1],"%u",&dest->header.size);
                sscanf(fields[1],"%lf",&dest->sounding.ts);
                sscanf(fields[2],"%lf",&dest->sounding.lat);
                sscanf(fields[3],"%lf",&dest->sounding.lon);
                sscanf(fields[4],"%lf",&dest->sounding.depth);
                sscanf(fields[5],"%lf",&dest->sounding.hdg);
                sscanf(fields[6],"%d",&dest->sounding.ping_number);
                sscanf(fields[7],"%u",&dest->sounding.nbeams);
                unsigned int i=0;
                for(i=0;i<dest->sounding.nbeams;i++){
                    int x=8+(i*4);
                    sscanf(fields[x+0],"%u",&dest->sounding.beams[i].beam_num);
                    sscanf(fields[x+1],"%lf",&dest->sounding.beams[i].rhox);
                    sscanf(fields[x+2],"%lf",&dest->sounding.beams[i].rhoy);
                    sscanf(fields[x+3],"%lf",&dest->sounding.beams[i].rhoz);
                }
                retval=0;
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

static int32_t s_read_mb1_rec( mb1_t *dest, mfile_file_t *src)
{
    int32_t retval=-1;

    if(NULL!=src && NULL!=dest){

        uint32_t readlen = 1;
        uint32_t record_bytes=0;
       // sync to start of record
        byte *bp = (byte *)&dest->header;
 
        while(mfile_read(src,(byte *)bp,readlen)==readlen){
            if(*bp=='M'){
                record_bytes+=readlen;
                bp++;
                readlen=MB1_BIN_HEADER_BYTES-1;
                break;
            }
        }

       if(record_bytes>0 && mfile_read(src,(byte *)bp,readlen)==readlen){
           record_bytes+=readlen;
            bp=(byte *)dest->sounding.beams;
            readlen = dest->header.size-(MB1_BIN_HEADER_BYTES+MB1_BIN_CHECKSUM_BYTES);
            
            if(mfile_read(src,(byte *)bp,readlen)==readlen){
               record_bytes+=readlen;
                byte chksum[MB1_BIN_CHECKSUM_BYTES]={0};
                bp =(byte *)chksum;
                readlen=MB1_BIN_CHECKSUM_BYTES;
            
                if(mfile_read(src,(byte *)bp,readlen)==readlen){
                    record_bytes+=readlen;
                    retval=record_bytes;
                }
            }
        }
    }
    return retval;
}

static int32_t s_trnc_read_mb1_rec( mb1_t *dest, msock_socket_t *src, app_cfg_t *cfg)
{
    int32_t retval=-1;

    if(NULL!=src && NULL!=dest){
        uint32_t readlen = MB1_BIN_MAX_BYTES;
        int32_t test=0;
        // sync to start of record
        byte *bp = (byte *)&dest->header;
        //        msock_set_blocking(src,true);

        if( (test=msock_recvfrom(src,NULL,(byte *)bp,readlen,0))>MB1_BIN_HEADER_BYTES){
            uint32_t record_bytes=readlen;
            retval=record_bytes;
            
            fprintf(stderr,"%s - read [%d/%u] \n",__FUNCTION__,test,readlen);
            fprintf(stderr,"ts[%.3lf] beams[%u] ping[%d] \n",dest->sounding.ts, dest->sounding.nbeams, dest->sounding.ping_number);
            fprintf(stderr,"lat[%.5lf] lon[%.5lf] hdg[%.2lf] sd[%.1lf]\n\n",dest->sounding.lat, dest->sounding.lon, dest->sounding.hdg, dest->sounding.depth);

        }else{
            if(test>0 && (strncmp((char *)bp,"ACK",3)==0 || strncmp((char *)bp,"NACK",4)==0)){
                // received ACK, ignore
            }else if(errno!=EAGAIN){
                fprintf(stderr,"ERR: read failed (%s) ret[%d/%u] [%d/%s]\n",__FUNCTION__,
                        test,readlen, errno,strerror(errno));
                mlog_tprintf(cfg->log_id,"ERR: read failed (%s) ret[%d/%u] [%d/%s]\n",__FUNCTION__,
                             test,readlen, errno,strerror(errno));
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
    
    if(NULL!=dcli && NULL!=mb1){
        int test=-1;

        if( (test=trncli_send_update(dcli, mb1, &pt, &mt))==0){
            if( (test=trncli_get_bias_estimates(dcli, pt, &pt_dat, &mle_dat, &mse_dat))==0){
                if(NULL!=pt_dat &&  NULL!= mle_dat && NULL!=mse_dat ){
                    fprintf(stderr,"\n\tBias Estimates:\n");
                    fprintf(stderr,"\tMLE: %.2lf,%.4lf,%.4lf,%.4lf\n",mle_dat->time,
                            (mle_dat->x-pt_dat->x),(mle_dat->y-pt_dat->y),(mle_dat->z-pt_dat->z));
                    fprintf(stderr,"\tMSE: %.2lf,%.4lf,%.4lf,%.4lf\n",mse_dat->time,
                            (mse_dat->x-pt_dat->x),(mse_dat->y-pt_dat->y),(mse_dat->z-pt_dat->z));
                    fprintf(stderr,"\tCOV:[%.2lf,%.2lf,%.2lf\n\n",sqrt(mse_dat->covariance[0]),sqrt(mse_dat->covariance[2]),sqrt(mse_dat->covariance[5]));

//                    tprintf(cfg->log_id,"\n\tBias Estimates:\n");
                    mlog_tprintf(cfg->log_id,"MLE,%.2lf,%.4lf,%.4lf,%.4lf\n",mle_dat->time,
                            (mle_dat->x-pt_dat->x),(mle_dat->y-pt_dat->y),(mle_dat->z-pt_dat->z));
                    mlog_tprintf(cfg->log_id,"MSE,%.2lf,%.4lf,%.4lf,%.4lf\n",mse_dat->time,
                            (mse_dat->x-pt_dat->x),(mse_dat->y-pt_dat->y),(mse_dat->z-pt_dat->z));
                    mlog_tprintf(cfg->log_id,"COV,%.2lf,%.2lf,%.2lf\n",sqrt(mse_dat->covariance[0]),sqrt(mse_dat->covariance[2]),sqrt(mse_dat->covariance[5]));

                    retval=0;

                }else{
                    fprintf(stderr,"ERR: pt[%p] pt_dat[%p] mle_dat[%p] mse_dat[%p]\n",pt,pt_dat,mle_dat,mse_dat);
                    mlog_tprintf(cfg->log_id,"ERR: pt[%p] pt_dat[%p] mle_dat[%p] mse_dat[%p]\n",pt,pt_dat,mle_dat,mse_dat);
                    mlog_tprintf(cfg->log_id,"ERR: ts[%.3lf] beams[%u] ping[%d] \n",mb1->sounding.ts, mb1->sounding.nbeams, mb1->sounding.ping_number);
                    mlog_tprintf(cfg->log_id,"ERR: lat[%.5lf] lon[%.5lf] hdg[%.2lf] sd[%.1lf]\n\n",mb1->sounding.lat, mb1->sounding.lon, mb1->sounding.hdg, mb1->sounding.depth);

                }
            }else{
                fprintf(stderr,"ERR: trncli_get_bias_estimates failed [%d]\n",test);
                mlog_tprintf(cfg->log_id,"ERR: trncli_get_bias_estimates failed [%d]\n",test);
            }
        }else{
            fprintf(stderr,"ERR: trncli_send_update failed [%d]\n",test);
            mlog_tprintf(cfg->log_id,"ERR: trncli_send_update failed [%d]\n",test);
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
    mfile_file_t *ifile = mfile_file_new(cfg->ifile);
    int test=-1;
    
    if( (test=mfile_open(ifile, MFILE_RONLY)) >0){
        bool quit=false;
        mb1_t mb1;
        while( !g_interrupt && !quit && (test=s_csv_to_mb1(&mb1, ifile))>0){
            if( (test=s_trncli_process_mb1(dcli,&mb1,cfg))!=0){
                if(test==EPIPE){
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

static int s_trncli_test_trnc(trncli_t *dcli, app_cfg_t *cfg)
{
    int retval=0;
    msock_socket_t *isock = NULL;//msock_socket_new(cfg->trnc_host,cfg->trnc_port,ST_UDP);
    int err_count=0;
    bool quit=false;
    int hbeat=cfg->trnc_hbn;
    byte req[4]="REQ", ack[4]={0};
//    msock_set_blocking(isock,true);
    int count=0;
    bool connected=false;
    mb1_t mb1;

//    if( (test=msock_connect(isock))==0){
//         if(msock_sendto(isock,NULL,req,4,0)==4){
//            if(msock_recv(isock,ack,4,0)==4){
//                hbeat=cfg->trnc_hbn;
//            }
//        }

        while( !g_interrupt && !quit ){
            int test=-1;
            memset(&mb1,0,sizeof(mb1_t));
           if(!connected){
                fprintf(stderr,"%s:%d connecting\n",__FUNCTION__,__LINE__);
                msock_socket_destroy(&isock);
                isock = msock_socket_new(cfg->trnc_host,cfg->trnc_port,ST_UDP);
                if(NULL!=isock){
                    msock_set_blocking(isock,true);
            		if( ( test=msock_connect(isock))==0){
                        if(msock_sendto(isock,NULL,req,4,0)==4){
                            memset(ack,0,4);
                            if(msock_recv(isock,ack,4,0)==4){
                                hbeat=cfg->trnc_hbn;
                                connected=true;
                                msock_set_blocking(isock,false);
                                err_count=0;
                                mlog_tprintf(cfg->log_id,"input connected [%s:%d]\n",cfg->trnc_host,cfg->trnc_port);
                            }
                        }
                    }else{
                        fprintf(stderr,"%s - msock_connect [%s:%d] failed [%d][%d/%s]\n",__FUNCTION__,cfg->trnc_host,cfg->trnc_port,test,errno,strerror(errno));
                        err_count++;
                    }
                }

                if(!connected) sleep(TRNCLI_TEST_CONNECT_DELAY_SEC);
                
            }else if( (test=s_trnc_read_mb1_rec(&mb1, isock, cfg))>0){
//                fprintf(stderr,"%s:%d reading mb1\n",__FUNCTION__,__LINE__);

                if(cfg->update_n>0){
                    if(++count%cfg->update_n==0){
                        if( (test=s_trncli_process_mb1(dcli,&mb1,cfg))==0){
                            // reset error count
                            err_count=0;
                        }else{
                            err_count++;
                            // these are the TRN server socket
                            switch (test) {
                                case EPIPE:
                                    //                                connected=false;
                                    mlog_tprintf(cfg->log_id,"ERR:EPIPE output disconnected [%s:%d]\n",cfg->trn_cfg->trn_host,cfg->trn_cfg->trn_port);
                                    break;
                                case EINTR:
                                    mlog_tprintf(cfg->log_id,"ERR:EINTR s_trncli_process_mb1 [%s:%d]\n",cfg->trnc_host,cfg->trnc_port);
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

                if( cfg->test_api>0  && (count%cfg->test_api) ==0 ){
                    int pval=-1;
                    bool bval=false;
                    int err[8]={0};
                    int i=0;
                    bval=trncli_is_intialized(dcli);
                    fprintf(stderr,"  is initialized [%c]\n",(bval?'Y':'N'));
                    err[i++]=errno;

                    bval=trncli_is_converged(dcli);
                    fprintf(stderr,"    is converged [%c]\n",(bval?'Y':'N'));
                    err[i++]=errno;

                    bval=trncli_last_meas_succesful(dcli);
                    fprintf(stderr,"   last meas val [%c]\n",(bval?'Y':'N'));
                    err[i++]=errno;

                    pval=trncli_reinit_count(dcli);
                    fprintf(stderr,"    reinit count [%d]\n",pval);
                    err[i++]=errno;

                    pval=trncli_get_filter_type(dcli);
                    fprintf(stderr,"     filter type [%d]\n",pval);
                    err[i++]=errno;

                    pval=trncli_get_filter_state(dcli);
                    fprintf(stderr,"    filter state [%d]\n",pval);
                    err[i++]=errno;

                    bval=trncli_outstanding_meas(dcli);
                    fprintf(stderr,"outstanding meas [%c]\n",(bval?'Y':'N'));
                    err[i++]=errno;

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
                                    fprintf(stderr,"ERR[%d] [%d/%s]\n",j,err[j],strerror(err[j]));
                                    break;
                            }
                        }
                    }
                    if(recon_trn){
                        fprintf(stderr,"WARN: reconnecting to TRN\n");
                        if( (test=trncli_connect(dcli, cfg->trn_cfg->trn_host, cfg->trn_cfg->trn_port))==0){
                            fprintf(stderr,"connect OK\n");
                            if( trncli_init_trn(dcli, cfg->trn_cfg)>0){
                                fprintf(stderr,"init OK\n");
                            }else{
                                fprintf(stderr,"trncli_init_server failed [%d]\n",test);
                            }
                        }else{
                            fprintf(stderr,"trncli_connect failed [%d]\n",test);
                        }

                    }
                }
            }else{
                // MB1 read error
                test=errno;
                err_count++;
                // these are the MBTRN server socket

//                fprintf(stderr,"%s:%d  ERR read mb1 failed errno[%d:%s] ecount[%d]\n",__FUNCTION__,__LINE__,errno,strerror(errno),err_count);
                switch (test) {
                    case EPIPE:
                        connected=false;
                        mlog_tprintf(cfg->log_id,"ERR: EPIPE input disconnected [%s:%d] ecount[%d]\n",cfg->trnc_host,cfg->trnc_port,err_count);
                        break;
                    case EINTR:
                        mlog_tprintf(cfg->log_id,"ERR: EINTR s_trnc_read_mb1_rec [%s:%d] ecount[%d]\n",cfg->trnc_host,cfg->trnc_port,err_count);
                        quit=true;
                        break;
                    case EAGAIN:
                        sleep(1);
                        if(err_count>10){
                        connected=false;
                        mlog_tprintf(cfg->log_id,"ERR: EAGAIN input disconnected [%s:%d] ecount[%d]\n",cfg->trnc_host,cfg->trnc_port,err_count);
                        }
                        break;

                    default:
                        fprintf(stderr,"ERR: s_trnc_read_mb1_rec failed  [%d/%s]\n",errno,strerror(errno));
                        mlog_tprintf(cfg->log_id,"s_trnc_read_mb1_rec failed  [%d/%s] ecount[%d]\n",errno,strerror(errno),err_count);
                        break;
                }

            }// else MB1 read error
            
        }// while

//    }else{
//        fprintf(stderr,"%s - msock_connect [%s:%d] failed [%d][%d/%s]\n",__FUNCTION__,cfg->trnc_host,cfg->trnc_port,test,errno,strerror(errno));
//        err_count++;
//    }
    
    if(g_interrupt){
        mlog_tprintf(cfg->log_id,"INTERRUPTED sig[%d] - exiting\n",g_signal);
    }
    
    msock_socket_destroy(&isock);
    return retval;
}

static int s_trncli_test_mbin(trncli_t *dcli, app_cfg_t *cfg)
{
    int retval=0;
    mfile_file_t *ifile = NULL;
    ifile = mfile_file_new(cfg->ifile);
    int test=-1;
    
    if( (test=mfile_open(ifile, MFILE_RONLY))>0){
        bool quit=false;
        mb1_t mb1;
        while( !g_interrupt && !quit && (test=s_read_mb1_rec(&mb1, ifile))>0){
            if( (test=s_trncli_process_mb1(dcli,&mb1,cfg))!=0){
                if(test==EPIPE){
                    quit=true;
                }
            }
        }// while
    }else{
        fprintf(stderr,"%s - mfile_open [%s] failed [%d][%d/%s]\n",__FUNCTION__,cfg->ifile,test,errno,strerror(errno));
    }
    mfile_file_destroy(&ifile);
    return retval;
}

static int s_app_main(app_cfg_t *cfg)
{
    int retval=-1;
    trncli_t *dcli = trncli_new(cfg->utm);
    int test=-1;
  
    if( (test=trncli_connect(dcli, cfg->trn_cfg->trn_host, cfg->trn_cfg->trn_port))==0){
        fprintf(stderr,"connect OK\n");
        if(trncli_init_trn(dcli, cfg->trn_cfg)>0){
            fprintf(stderr,"init OK\n");
        }else{
            fprintf(stderr,"trncli_init_trn failed [%d]\n",test);
        }
        switch (cfg->input_src) {
            case SRC_CSV:
                retval=s_trncli_test_csv(dcli, cfg);
                break;
            case SRC_MBIN:
                retval=s_trncli_test_mbin(dcli, cfg);
                break;
            case SRC_TRNC:
                retval=s_trncli_test_trnc(dcli, cfg);
                break;
            default:
                fprintf(stderr,"invalid input type [%d]\n",cfg->input_src);
                break;
        }
    }else{
        fprintf(stderr,"trncli_connect failed [%d]\n",test);
    }
    
    if( (test=trncli_disconnect(dcli))!=0){
        fprintf(stderr,"trncli_disconnect failed [%d]\n",test);
    }
    
     trncli_destroy(&dcli);
    
    mlog_tprintf(cfg->log_id,"*** trncli-test session end ***\n");

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
    mlog_tprintf(cfg->log_id,"*** trncli-test session start ***\n");
    mlog_tprintf(cfg->log_id,"cmdline [%s]\n",g_cmd_line);
    mlog_tprintf(cfg->log_id,"build [%s]\n",TRNCLI_TEST_BUILD);

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
