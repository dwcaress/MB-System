///
/// @file tbinx.c
/// @authors k. Headley
/// @date 01 jan 2018

/// Re-transmit MB1 records
/// reads MB-System MB1 records from a file
/// Transmits to socket, csv file, and/or stdout

/////////////////////////
// Terms of use
/////////////////////////
/*
 Copyright Information
 
 Copyright 2000-2018 MBARI
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

#include <getopt.h>
#include <signal.h>
#include "mframe.h"
#include "mlist.h"
#include "mfile.h"
#include "msocket.h"
#include "mxdebug.h"
#include "mxd_app.h"
#include "r7k-reader.h"
#include "mb1_msg.h"

/////////////////////////
// Macros
/////////////////////////
#define TBINX_NAME "tbinx"
#ifndef TBINX_VER
/// @def TBINX_VER
/// @brief module build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DTBINX_VER=<version>
#define TBINX_VER (dev)
#endif
#ifndef TBINX_BUILD
/// @def TBINX_BUILD
/// @brief module build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DTBINX_BUILD=`date`
#define TBINX_BUILD VERSION_STRING(TBINX_VER)" "LIBMFRAME_BUILD
#endif

// These macros should only be defined for
// application main files rather than general C files
/*
 /// @def PRODUCT
 /// @brief header software product name
 #define PRODUCT "MBRT"
 
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


/////////////////////////
// Declarations
/////////////////////////

/// @typedef enum oflags_t oflags_t
/// @brief flags specifying output types
typedef enum{OF_NONE=0,OF_SOUT=0x1,OF_CSV=0x2, OF_SOCKET=0x4, OF_SERR=0x10}oflags_t;

/// @def TBX_MAX_VERBOSE
/// @brief max verbose output level
#define TBX_MAX_VERBOSE 3
/// @def TBX_MSG_CON_LEN
/// @brief TRN connect message length
#define TBX_MSG_CON_LEN 4
/// @def TBX_HBTOK_DFL
/// @brief default heartbeat interval (messages)
#define TBX_HBTOK_DFL 50
/// @def TBX_MAX_DELAY_SEC
/// @brief max message output delay
#define TBX_MAX_DELAY_SEC 3
/// @def TBX_MIN_DELAY_SEC
/// @brief minimum message output delay (s)
#define TBX_MIN_DELAY_SEC 0
/// @def TBX_MIN_DELAY_NSEC
/// @brief minimum message output delay (ns)
#define TBX_MIN_DELAY_NSEC 8000000
/// @def TBX_SOCKET_DELAY_SEC
/// @brief time to wait before socket retry
/// if no clients connected
#define TBX_SOCKET_DELAY_SEC 3
/// @def TBX_VERBOSE_DFL
/// @brief verbose output default
#define TBX_VERBOSE_DFL 0
/// @def TBX_NFILES_DFL
/// @brief default number of files
#define TBX_NFILES_DFL 0
/// @def TBX_OFLAGS_DFL
/// @brief default output flags
#define TBX_OFLAGS_DFL OF_SOUT
/// @def TBX_HOST_DFL
/// @brief default host
#define TBX_HOST_DFL "localhost"
/// @def TBX_PORT_DFL
/// @brief default output port
#define TBX_PORT_DFL 27000
/// @def TBX_DELAY_DFL
/// @brief default output delay
#define TBX_DELAY_DFL 0
/// @def TBX_DELAY_DFL
/// @brief default output delay
#define TBX_RCDMS_DFL 500

/// @def TBX_SNDBUF_SIZE
/// @brief osocket sendbuf size
#define TBX_SNDBUF_BYTES 1048576

/// @typedef struct app_cfg_s app_cfg_t
/// @brief application configuration parameter structure
typedef struct app_cfg_s{
    /// @var app_cfg_s::verbose
    /// @brief enable verbose output
    int verbose;
    /// @var app_cfg_s::nfiles
    /// @brief file list
     int nfiles;
    /// @var app_cfg_s::files
    /// @brief file list
    char **files;
    /// @var app_cfg_s::oflags
    /// @brief output type flags
    oflags_t oflags;
    /// @var app_cfg_s::csv_path
    /// @brief csv file name
    char *csv_path;
    /// @var app_cfg_s::host
    /// @brief host
    char *host;
    /// @var app_cfg_s::port
    /// @brief port
    int port;
    /// @var app_cfg_s::delay_msec
    /// @brief packet delay.
    /// -1: no delay
    ///  0: use timestamps (default)
    /// >0: use specified value
    int delay_msec;
    /// @var app_cfg_s::rcdms
    /// @brief delay if error/reconnect (msec)
    long rcdms;
 
}app_cfg_t;

static void s_show_help();

/////////////////////////
// Imports
/////////////////////////

/////////////////////////
// Module Global Variables
/////////////////////////

static msock_socket_t *trn_osocket = NULL;
static msock_connection_t *trn_peer = NULL;
static mlist_t *trn_plist = NULL;
static int trn_hbtok = TBX_HBTOK_DFL;
static int trn_tx_count=0;
static int trn_rx_count=0;
static int trn_tx_bytes=0;
static int trn_rx_bytes=0;
static int trn_msg_count=0;
static int trn_msg_bytes=0;
static int trn_cli_con=0;
static int trn_cli_dis=0;
static bool g_interrupt=false;
static int g_sig_count=0;
static int g_alt_count=0;

/////////////////////////
// Function Definitions
/////////////////////////

/// @fn void s_show_help()
/// @brief output user help message to stdout.
/// @return none
static void s_show_help()
{
    char help_message[] = "\nmbtrnpreprocess binary log emitter\n";
    char usage_message[] = "\ntbinx [options]\n"
    "--verbose=n      : verbose output, n>0\n"
    "--help           : output help message\n"
    "--version        : output version info\n"
    "--socket=host:port : export to socket\n"
    "--sout           : export to stdout\n"
    "--serr           : export to stderr\n"
    "--csv=file       : export to csv file\n"
    "--delay=msec     : minimum packet delay [0:use timestamps (default), -1:no delay]\n"
    "--rcdms=msec     : delay on reconnect/socket error (msec)\n"
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
static void parse_args(int argc, char **argv, app_cfg_t *cfg)
{
    extern char WIN_DECLSPEC *optarg;
    int option_index;
    int c;
    bool help=false;
    bool version=false;
    
    static struct option options[] = {
        {"verbose", required_argument, NULL, 0},
        {"help", no_argument, NULL, 0},
        {"version", no_argument, NULL, 0},
        {"sout", no_argument, NULL, 0},
        {"serr", no_argument, NULL, 0},
        {"socket", required_argument, NULL, 0},
        {"csv", required_argument, NULL, 0},
        {"delay", required_argument, NULL, 0},
        {"rcdms", required_argument, NULL, 0},
       {NULL, 0, NULL, 0}};

    /* process argument list */
    while ((c = getopt_long(argc, argv, "", options, &option_index)) != -1){
        switch (c) {
                // long options all return c=0
            case 0:
                // verbose
                if (strcmp("verbose", options[option_index].name) == 0) {
                    sscanf(optarg,"%d",&cfg->verbose);
                }
                // help
                else if (strcmp("help", options[option_index].name) == 0) {
                    help = true;
                }
                // version
                else if (strcmp("version", options[option_index].name) == 0) {
                    version = true;
                }
                // sout
                else if (strcmp("sout", options[option_index].name) == 0) {
                    cfg->oflags|=OF_SOUT;
                }
                // serr
                else if (strcmp("serr", options[option_index].name) == 0) {
                    cfg->oflags |= OF_SERR;
                    cfg->oflags &= ~OF_SOUT;
                }
                // socket
                else if (strcmp("socket", options[option_index].name) == 0) {
                    cfg->oflags|=OF_SOCKET;
                    cfg->oflags &= ~OF_SOUT;
                    
                    char *ocopy=strdup(optarg);
                    cfg->host=strtok(ocopy,":");
                    if (cfg->host==NULL) {
                        cfg->host=TBX_HOST_DFL;
                    }
                    char *ip = strtok(NULL,":");
                    if (ip!=NULL) {
                        sscanf(ip,"%d",&cfg->port);
                    }
                }
                // csv
                else if (strcmp("csv", options[option_index].name) == 0) {
                    if (cfg->csv_path) {
                        free(cfg->csv_path);
                    }
                    cfg->oflags|=OF_CSV;
                    cfg->csv_path=strdup(optarg);
                }
                // delay
                else if (strcmp("delay", options[option_index].name) == 0) {
                    sscanf(optarg,"%d",&cfg->delay_msec);
                }
                // delay
                else if (strcmp("rcdms", options[option_index].name) == 0) {
                    sscanf(optarg,"%ld",&cfg->rcdms);
                }
               break;
            default:
                help=true;
                break;
        }
        if (version) {
            MFRAME_SHOW_VERSION(TBINX_NAME, TBINX_BUILD);
            //           r7kr_show_app_version(TBINX_NAME,TBINX_BUILD);
            exit(0);
        }
        if (help) {
            MFRAME_SHOW_VERSION(TBINX_NAME, TBINX_BUILD);
            //            r7kr_show_app_version(TBINX_NAME,TBINX_BUILD);
            s_show_help();
            exit(0);
        }
    }// while
    
    if (cfg->verbose>TBX_MAX_VERBOSE) {
        cfg->verbose=TBX_MAX_VERBOSE;
    }
    if (cfg->verbose<0) {
        cfg->verbose=0;
    }


    cfg->files=&argv[optind];
    cfg->nfiles=argc-optind;
    
    mxd_setModule(MXDEBUG, 0, true, NULL);
    mxd_setModule(MXERROR, 5, false, NULL);
    mxd_setModule(TBINX, 0, false, "tbinx.error");
    mxd_setModule(TBINX_ERROR, 0, true, "tbinx.error");
    mxd_setModule(TBINX_DEBUG, 0, true, "tbinx.debug");
    mxd_setModule(MXMSOCK, 0, true, "msock");
    mxd_setModule(R7KC, 0, true, "r7kc");
    mxd_setModule(R7KC_DEBUG, 0, true, "r7kc.debug");
    mxd_setModule(R7KC_ERROR, 0, true, "r7kc.error");
    mxd_setModule(R7KR, 0, true, "r7kr");
    mxd_setModule(R7KR_ERROR, 0, true, "r7kr.error");
    mxd_setModule(R7KR_DEBUG, 0, true, "r7kr.debug");

    switch (cfg->verbose) {
        case 0:
            break;
        case 1:
            mxd_setModule(TBINX, 1, false, "tbinx.error");
            break;
        case 2:
            mxd_setModule(MXDEBUG, 5, false, NULL);
            mxd_setModule(TBINX, 5, false, "tbinx.error");
            break;
        case 3:
        case 4:
        case 5:
            mxd_setModule(MXDEBUG, 5, false, NULL);
            mxd_setModule(TBINX_ERROR, 5, false, "tbinx.error");
            mxd_setModule(TBINX_DEBUG, 5, false, "tbinx.debug");
            mxd_setModule(MXMSOCK, 5, false, "msock");
            mxd_setModule(R7KC, 5, false, "r7kc");
            mxd_setModule(R7KC_DEBUG, 5, false, "r7kc.debug");
            mxd_setModule(R7KC_ERROR, 5, false, "r7kc.error");
            mxd_setModule(R7KR, 5, false, "r7kr");
            mxd_setModule(R7KR_ERROR, 5, false, "r7kr.error");
            mxd_setModule(R7KR_DEBUG, 5, false, "r7kr.debug");
            break;
        default:
            break;
    }

    if(cfg->verbose != 0)
        mxd_show();

    if (cfg->verbose!=0) {
        fprintf(stderr, "verbose   [%s]\n", (cfg->verbose?"Y":"N"));
        fprintf(stderr, "nfiles    [%d]\n", cfg->nfiles);
        if (cfg->nfiles>0) {
            for (int i=0; i<cfg->nfiles; i++) {
                fprintf(stderr, "files[%2d] [%s]\n", i,cfg->files[i]);
            }
        }
        fprintf(stderr, "sout      [%c]\n", (((cfg->oflags&OF_SOUT)   !=0 ) ? 'Y' : 'N'));
        fprintf(stderr, "csv       [%c]\n", (((cfg->oflags&OF_CSV)    !=0 ) ? 'Y' : 'N'));
        fprintf(stderr, "socket    [%c]\n", (((cfg->oflags&OF_SOCKET) !=0 ) ? 'Y' : 'N'));
        if (cfg->oflags&OF_SOCKET) {
        fprintf(stderr, "host:port [%s:%d]\n",cfg->host,cfg->port);
        }
        fprintf(stderr,"delay     [%d]\n",(cfg->delay_msec));
        fprintf(stderr,"rcdms     [%ld]\n",(cfg->rcdms));
    }
}
// End function parse_args

/// @fn void s_sig_handler(int sig)
/// @brief signal handler
/// @param[in] sig signal received

static void s_sig_handler(int sig)
{
    switch(sig){
        case SIGINT:
        case SIGKILL:
        case SIGQUIT:
        case SIGSTOP:
            g_interrupt = true;
            break;
        default:
            g_alt_count++;
            break;
    }
    g_sig_count++;
    return;
}

/// @fn int s_delay_message(trn_data_t *message)
/// @brief delay packet per packet timestamp
/// @param[in] message message reference
/// @param[in] prev_time previous message timestamp
/// @param[in] cfg app configuration reference
/// @return 0 on success, -1 otherwise
static int s_delay_message(mb1_t *sounding, double prev_time, app_cfg_t *cfg)
{
    int retval=-1;
    
    if (NULL!=sounding && NULL!=cfg) {
        
        struct timespec delay={0};
        struct timespec rem={0};

        if (cfg->delay_msec==0) {
            // use timestamps
           double tsdiff = (sounding->ts-prev_time);

            MX_LPRINT(TBINX, 4, "prev_time[%.3lf] ts[%.3lf] tsdiff[%.3lf]\n", prev_time, sounding->ts, tsdiff);
            if (tsdiff>TBX_MAX_DELAY_SEC) {
                // if delay too large, use min delay
                delay.tv_sec=TBX_MIN_DELAY_SEC;
                delay.tv_nsec=TBX_MIN_DELAY_NSEC;//0;//800000;
                MX_LPRINT(TBINX, 4, "case >max - using min delay[%"PRIu32":%"PRIu32"]\n", (uint32_t)delay.tv_sec, (uint32_t)delay.tv_nsec);
            }else{
                if (  prev_time>0 && tsdiff > 0) {
                    time_t lsec = (time_t)tsdiff;
                    long lnsec = (10000000L*(tsdiff-lsec));
                    rem.tv_sec=0;
                    rem.tv_nsec=0;
                    delay.tv_sec=lsec;
                    delay.tv_nsec=lnsec;
                    MX_LPRINT(TBINX, 4, "case ts - using delay[%"PRIu32":%"PRIu32"]\n", (uint32_t)delay.tv_sec, (uint32_t)delay.tv_nsec);
                    
                    while (nanosleep(&delay,&rem)<0) {
                        MX_LMSG(TBINX, 4, "sleep interrupted\n");
                        delay.tv_sec=rem.tv_sec;
                        delay.tv_nsec=rem.tv_nsec;
                    }
                }else{
                    // if delay <0, use min delay
                    delay.tv_sec=TBX_MIN_DELAY_SEC;
                    delay.tv_nsec=TBX_MIN_DELAY_NSEC;//0;//800000;
                    MX_LPRINT(TBINX, 4, "case ts<0 - using min delay[%"PRIu32":%"PRIu32"]\n", (uint32_t)delay.tv_sec, (uint32_t)delay.tv_nsec);
                }
            }
        }else if (cfg->delay_msec>0){
            // use specified delay
            uint64_t dsec=cfg->delay_msec/1000;
            uint64_t dmsec=cfg->delay_msec%1000;
            delay.tv_sec=dsec;
            delay.tv_nsec=dmsec*1000000L;
            MX_LPRINT(TBINX, 4, "case specified - using delay[%"PRIu32":%"PRIu32"]\n", (uint32_t)delay.tv_sec, (uint32_t)delay.tv_nsec);
        }else{
            // else no delay/min delay
            // with zero delay, client REQ missed/arrive late - why?
           delay.tv_sec=TBX_MIN_DELAY_SEC;
            delay.tv_nsec=TBX_MIN_DELAY_NSEC;//0;//800000;
            MX_LPRINT(TBINX, 4, "case <0 - using min delay[%"PRIu32":%"PRIu32"]\n", (uint32_t)delay.tv_sec, (uint32_t)delay.tv_nsec);
        }

        if (delay.tv_sec>0 || delay.tv_nsec>0) {
            
            while(nanosleep(&delay,&rem)<0){
                delay.tv_sec=rem.tv_sec;
                delay.tv_nsec=rem.tv_nsec;
            }
        }
        
        retval=0;
    }
    return retval;
}
// End function s_delay_message

/// @fn int s_out_stdx(FILE *dest, mb1_t *sounding)
/// @brief export message to dest (FILE *)
/// @param[in] dest destination FILE pointer
/// @param[in] message message reference
/// @return 0 on success, -1 otherwise
static int s_out_stdx(FILE *dest, mb1_t *sounding)
{
    int retval=0;
    
    if (NULL!=sounding && NULL!=dest) {
        
        fprintf(dest,"\nts[%.3lf] ping[%06"PRIu32"] beams[%03d]\nlat[%.4f] lon[%.4lf] hdg[%6.2lf] sd[%7.2lf]\n",\
                sounding->ts,
                sounding->ping_number,
                sounding->nbeams,
                sounding->lat,
                sounding->lon,
                sounding->hdg,
                sounding->depth
                );
        if (sounding->nbeams<=512) {
            for (int j = 0; j < (int)sounding->nbeams; j++) {
                fprintf(dest,"n[%03"PRId32"] atrk/X[%+10.3lf] ctrk/Y[%+10.3lf] dpth/Z[%+10.3lf]\n",
                        sounding->beams[j].beam_num,
                        sounding->beams[j].rhox,
                        sounding->beams[j].rhoy,
                        sounding->beams[j].rhoz);
            }
        }
    }else{
        MX_LMSG(TBINX, 2, "invalid argument\n");
        retval=-1;
    }
    return retval;
}
// End function s_out_stdx

/// @fn int s_out_csv(trn_data_t *message)
/// @brief export message to csv file
/// @param[in] dest CSV file reference
/// @param[in] message message reference
/// @return 0 on success, -1 otherwise
static int s_out_csv(mfile_file_t *dest, mb1_t *sounding)
{
    int retval=0;
    
    if (NULL!=sounding && NULL!=dest) {

         mfile_fprintf(dest,"%.3lf,%"PRIu32",%d,%lf,%lf,%lf,%lf",\
                    sounding->ts,
                    sounding->ping_number,
               sounding->nbeams,
               sounding->lat,
               sounding->lon,
               sounding->hdg,
               sounding->depth
               );
        for (int j = 0; j < (int)sounding->nbeams; j++) {
            mfile_fprintf(dest,",%d,%+lf,%+lf,%+lf",
                   sounding->beams[j].beam_num,
                   sounding->beams[j].rhox,
                   sounding->beams[j].rhoy,
                   sounding->beams[j].rhoz);
        }
        mfile_fprintf(dest,"\n");
    }else{
        MX_LMSG(TBINX, 2, "invalid argument\n");
        retval=-1;
    }
    return retval;
}
// End function s_out_csv

/// @fn int s_out_socket(trn_data_t *message)
/// @brief export message to socket
/// @param[in] s socket reference
/// @param[in] message message reference
/// @return 0 on success, -1 otherwise
static int s_out_socket(msock_socket_t *s, mb1_t *sounding, app_cfg_t *cfg)
{
    int retval=-1;
    
    if (NULL!=sounding && NULL!=s) {
        
        byte cmsg[TBX_MSG_CON_LEN];
        int iobytes = 0;
        int idx=-1;
        int svc=0;
        msock_connection_t *psub=mlist_head(trn_plist);
        bool data_available=false;
        
        // when socket output is enabled,
        // wait until a client connects
        // Otherwise, the data will just fall on the floor
        
        do{
            // check the socket for client activity
            MX_LMSG(TBINX, 1, "checking TRN host socket\n");
            iobytes = msock_recvfrom(s, trn_peer->addr, cmsg, TBX_MSG_CON_LEN,0);
            
            switch (iobytes) {
                case 0:
                    MX_LPRINT(TBINX, 2, "err - recvfrom ret 0 (socket closed) removing cli[%d]\n", trn_peer->id);
                    // socket closed, remove client from list
                    if(sscanf(trn_peer->service,"%d",&svc)==1){
                        msock_connection_t *peer = (msock_connection_t *)mlist_vlookup(trn_plist, &svc, r7kr_peer_vcmp);
                        if (peer!=NULL) {
                            mlist_remove(trn_plist,peer);
                        }
                    }
                    data_available=false;
                    break;
                case -1:
                    // nothing to read - bail out
                    if(errno != EWOULDBLOCK){
                        MX_LPRINT(TBINX, 1, "err - recvfrom cli[%d] ret -1 [%d/%s]\n", trn_peer->id, errno, strerror(errno));
                        struct timespec ts = {0, cfg->rcdms * 1000000L};
                        nanosleep(&ts, NULL);
                    }
                    data_available=false;
                    break;
                    
                default:
                    
                    // client sent something
                    // update stats
                    trn_rx_count++;
                    trn_rx_bytes+=iobytes;
                    data_available=true;
                    
                    const char *ctest=NULL;

                    struct sockaddr_in *psin = NULL;
                    
                    if (NULL != trn_peer->addr &&
                        NULL != trn_peer->addr->ainfo &&
                        NULL != trn_peer->addr->ainfo->ai_addr) {
                        // get client endpoint info
                        psin = (struct sockaddr_in *)trn_peer->addr->ainfo->ai_addr;
                        ctest = inet_ntop(AF_INET, &psin->sin_addr, trn_peer->chost, MSOCK_ADDR_LEN);
                        if (NULL!=ctest) {
                            
                            uint16_t port = ntohs(psin->sin_port);
                            
                            svc = port;
                            snprintf(trn_peer->service,NI_MAXSERV,"%d",svc);
                            
                            msock_connection_t *pclient=NULL;
                            
                            // check client list
                            // to see whether already connected
                            pclient = (msock_connection_t *)mlist_vlookup(trn_plist,&svc,r7kr_peer_vcmp);
                            if (pclient!=NULL) {
                                // client exists, update heartbeat tokens
                                // [could make additive, i.e. +=]
                                MX_LPRINT(TBINX, 1, "updating client hbeat id[%d] addr[%p]\n", svc,trn_peer);
                                pclient->heartbeat = trn_hbtok;
                            }else{
                                MX_LPRINT(TBINX, 1, "adding to client list id[%d] addr[%p]\n", svc, trn_peer);
                                // client doesn't exist
                                // initialize and add to list
                                trn_peer->id = svc;
                                trn_peer->heartbeat = trn_hbtok;
                                trn_peer->next=NULL;
                                mlist_add(trn_plist, (void *)trn_peer);
                                // save pointer to finish up (send ACK, update hbeat)
                                pclient=trn_peer;
                                // create a new peer for next connection
                                trn_peer = msock_connection_new();
                                
                                trn_cli_con++;
                            }
                            
                            MX_LPRINT(TBINX, 1, "rx [%d]b cli[%d/%s:%s]\n", iobytes, svc, trn_peer->chost, trn_peer->service);

                            if ( (NULL!=pclient) &&  (iobytes> 0) ) {
                                // send ACK
                                iobytes = msock_sendto(s, pclient->addr, (byte *)"ACK", 4, 0 );

                                MX_LPRINT(TBINX, 1, "tx ACK [%d]b cli[%d/%s:%s]\n", iobytes, svc, pclient->chost, pclient->service);
                                trn_tx_count++;
                                trn_tx_bytes+=iobytes;
                            }else{
                                fprintf(stderr,"tx cli[%d] failed pclient[%p] iobytes[%d] [%d/%s]\n",svc,pclient,iobytes,errno,strerror(errno));
                            }
                        }else{
                            MX_ERROR("err - inet_ntop failed [%d/%s]\n", errno, strerror(errno));
                        }
                    }else{
                        // invalid sockaddr
                        fprintf(stderr,"err - NULL cliaddr(rx) cli[%d]\n",trn_peer->id);
                    }
                    
                    break;
            }
            // keep reading while data is available
        } while (data_available && !g_interrupt);
        
        
        // send output to clients
        psub = (msock_connection_t *)mlist_first(trn_plist);
        idx=0;
        while (psub != NULL && !g_interrupt) {
            
            psub->heartbeat--;
            
            uint32_t message_len=MB1_SOUNDING_BYTES(sounding->nbeams);
            
            if ( (iobytes = msock_sendto(trn_osocket, psub->addr, (byte *)sounding, message_len, 0 )) > 0) {
                trn_tx_count++;
                trn_tx_bytes+=iobytes;
                 retval=0;

                MX_LPRINT(TBINX, 1, "tx TRN [%5d]b cli[%d/%s:%s] hb[%d]\n",
                        iobytes, idx, psub->chost, psub->service, psub->heartbeat);
                
            }else{
                MX_ERROR("err - sendto ret[%d] cli[%d] [%d/%s]\n", iobytes, idx, errno, strerror(errno));
            }
            // check heartbeat, remove expired peers
            if (psub->heartbeat==0) {
                MX_LPRINT(TBINX, 1, "hbeat=0 cli[%d/%d] - removed\n", idx, psub->id);
                mlist_remove(trn_plist,psub);
                trn_cli_dis++;
            }
            psub=(msock_connection_t *)mlist_next(trn_plist);
            idx++;
        }// while psub
        
        
    }else{
        MX_LMSG(TBINX, 2, "invalid argument\n");
        retval=-1;
    }
    
    // return -1 if message unsent
    return retval;
}
// End function s_out_socket

/// @fn int s_process_file(char *file)
/// @brief process TRN message file, send to specified outputs.
/// @param[in] cfg app configuration reference
/// @return 0 on success, -1 otherwise
int s_process_file(app_cfg_t *cfg)
{
    int retval=0;
    
   if (NULL!=cfg && cfg->nfiles>0) {
        for (int i=0; i<cfg->nfiles; i++) {

            if(g_interrupt)
                break;

            MX_LPRINT(TBINX, 2, "processing %s\n", cfg->files[i]);

            mfile_file_t *ifile = mfile_file_new(cfg->files[i]);
            int test=0;
            mfile_file_t *csv_file = NULL;
            
            if(cfg->csv_path!=NULL){
                csv_file=mfile_file_new(cfg->csv_path);
                if ( (test=mfile_mopen(csv_file,MFILE_RDWR|MFILE_CREATE,MFILE_RU|MFILE_WU|MFILE_RG|MFILE_WG)) <= 0) {
                    MX_ERROR_MSG("could not open CSV file\n");
                    csv_file=NULL;
                }else{
                    MX_LPRINT(TBINX, 1, "opened CSV file [%s]\n", cfg->csv_path);
                }
            }
            
            if ((cfg->oflags&OF_SOCKET) ) {
                trn_peer=msock_connection_new();
                trn_plist = mlist_new();
                mlist_autofree(trn_plist,msock_connection_free);

                trn_osocket = msock_socket_new(cfg->host, cfg->port, ST_UDP);
                msock_set_blocking(trn_osocket,false);
                int optionval = 1;
#if !defined(__CYGWIN__)
                msock_set_opt(trn_osocket, SO_REUSEPORT, &optionval, sizeof(optionval));
#endif
                msock_set_opt(trn_osocket, SO_REUSEADDR, &optionval, sizeof(optionval));

                optionval = TBX_SNDBUF_BYTES;
                msock_set_opt(trn_osocket, SO_SNDBUF, &optionval, sizeof(optionval));

                if ( (test=msock_bind(trn_osocket))==0) {
                    fprintf(stderr,"TRN host socket bind OK [%s:%d]\n",cfg->host, cfg->port);
                }else{
                    fprintf(stderr, "\nTRN host socket bind failed [%d] [%d %s]\n",test,errno,strerror(errno));
                }
            }
            
            if ( (test=mfile_open(ifile,MFILE_RONLY)) > 0) {
                MX_LPRINT(TBINX, 2, "open OK [%s]\n", cfg->files[i]);
                
                byte msg_buf[MB1_MAX_SOUNDING_BYTES+sizeof(mb1_t)]={0};
                mb1_t *mb1= (mb1_t *) &msg_buf[0];
                byte *ptype = (byte *)(&(mb1->type));

                double prev_time =0.0;
                
                bool ferror=false;
                int64_t rbytes=0;
                bool sync_valid=false;
                while (!ferror) {
                    byte *sp = (byte *)mb1;
                    bool header_valid=false;
                    bool rec_valid=false;
                    while (!sync_valid) {
                        if( ((rbytes=mfile_read(ifile,(byte *)sp,1))==1) && *sp=='M'){
                            sp++;
                            if( ((rbytes=mfile_read(ifile,(byte *)sp,1))==1) && *sp=='B'){
                                sp++;
                                if( ((rbytes=mfile_read(ifile,(byte *)sp,1))==1) && *sp=='1'){
                                    sp++;
                                    if( ((rbytes=mfile_read(ifile,(byte *)sp,1))==1) && *sp=='\0'){
                                        sync_valid=true;
                                        MX_LPRINT(TBINX, 2, "sync read slen[%d]\n", MB1_TYPE_BYTES);
                                       MX_LPRINT(TBINX, 2, "  sync     ['%c''%c''%c''%c']/[%02X %02X %02X %02X]\n",
                                                ptype[0],
                                                ptype[1],
                                                ptype[2],
                                                ptype[3],
                                                ptype[0],
                                                ptype[1],
                                                ptype[2],
                                                ptype[3]);
                                        break;
                                    }else{
                                        sp=ptype;
                                    }
                                }else{
                                    sp=ptype;
                                }
                            }else{
                                sp=ptype;
                            }
                        }
                        if(rbytes<=0){
                            MX_LMSG(TBINX, 1, "reached EOF looking for sync\n");
                            ferror=true;
                            break;
                        }
                    }

                    if(g_interrupt)
                        ferror = true;

//                    if( (sync_valid && !ferror && (rbytes=mfile_read(ifile,(byte *)psize,4))==4)){
//                        // read size
//                        header_valid=true;
//                    }

                    if (sync_valid && !ferror) {
                        // read the rest of the sounding header
                        byte *psnd = (byte *)&mb1->size;
                        uint32_t readlen =(MB1_HEADER_BYTES-MB1_TYPE_BYTES);
                        if( ((rbytes=mfile_read(ifile,psnd,readlen))==readlen)){
                            
                            int32_t cmplen = MB1_SOUNDING_BYTES(mb1->nbeams);
                            
                            if ((int32_t)mb1->size == cmplen ) {
                                header_valid=true;
                                MX_LPRINT(TBINX, 2, "sounding header read len[%"PRIu32"/%"PRId64"]\n", (uint32_t)readlen, rbytes);
                                MX_LPRINT(TBINX, 3, "  size   [%d]\n", mb1->size);
                                MX_LPRINT(TBINX, 3, "  time   [%.3f]\n", mb1->ts);
                                MX_LPRINT(TBINX, 3, "  lat    [%.3f]\n", mb1->lat);
                                MX_LPRINT(TBINX, 3, "  lon    [%.3f]\n", mb1->lon);
                                MX_LPRINT(TBINX, 3, "  depth  [%.3f]\n", mb1->depth);
                                MX_LPRINT(TBINX, 3, "  hdg    [%.3f]\n", mb1->hdg);
                                MX_LPRINT(TBINX, 3, "  ping   [%06d]\n", mb1->ping_number);
                                MX_LPRINT(TBINX, 3, "  nbeams [%d]\n", mb1->nbeams);
                            }else{
                                MX_MPRINT(TBINX_DEBUG, "message len invalid l[%d] l*[%d]\n", mb1->size, cmplen);
                            }
                        }else{
                            MX_ERROR("could not read header bytes [%"PRId64"]\n", rbytes);
                            ferror=true;
                        }
                    }
                    
                    if(g_interrupt)
                        ferror = true;

                    if (header_valid && ferror==false ) {
                        
                        if(mb1->nbeams>0){
                            // read beam data
                            byte *bp = (byte *)mb1->beams;
                            uint32_t readlen = MB1_BEAM_ARRAY_BYTES(mb1->nbeams);
                            if( ((rbytes=mfile_read(ifile,bp,readlen))==readlen)){
                                
                                //                                MX_LPRINT(TBINX, 2, "beams read blen[%d/%"PRId64"]\n", beamsz, rbytes);
                                
                            }else{
                                MX_LPRINT(TBINX, 2, "beam read failed pb[%p] read[%"PRId64"]\n", bp, rbytes);
                            }

                        }

                        byte *cp = (byte *)MB1_PCHECKSUM(mb1);

                        if( ((rbytes=mfile_read(ifile,cp,MB1_CHECKSUM_BYTES))==MB1_CHECKSUM_BYTES)){
                            //                                    MX_LPRINT(TBINX, 2, "chksum read clen[%"PRId64"]\n", rbytes);
                            //                                    MX_LPRINT(TBINX, 3, "  chksum [%0X]\n", pmessage->chksum);
                            
                            rec_valid=true;
                        }else{
                            MX_MPRINT(TBINX_DEBUG, "chksum read failed [%"PRId64"]\n", rbytes);
                        }

                    }else{
                        MX_MPRINT(TBINX_DEBUG, "header read failed [%"PRId64"]\n", rbytes);
                    }

                    if(g_interrupt)
                        ferror = true;

                    if (rec_valid && ferror==false) {

                        trn_msg_bytes+=mb1->size;
                        trn_msg_count++;

                        s_delay_message(mb1, prev_time, cfg);
                        prev_time=mb1->ts;
                        
                        if (cfg->oflags&OF_SOUT) {
                            s_out_stdx(stdout,mb1) ;
                        }
                        if (cfg->oflags&OF_SERR) {
                            s_out_stdx(stderr,mb1) ;
                        }
                        if ( (cfg->oflags&OF_CSV) && (NULL!=csv_file) ) {
                            s_out_csv(csv_file,mb1);
                        }
                        if ( (cfg->oflags&OF_SOCKET) && (NULL!=trn_osocket) ) {
                            int test_con=0;
                            do{
                                // send message to socket
                                // or wait until clients connected
                                if( (test_con=s_out_socket(trn_osocket, mb1, cfg)) != 0 ){
//                                    sleep(TBX_SOCKET_DELAY_SEC);
                                }
                            }while (test_con!=0 && !g_interrupt);
                        }
                    }
                }
                mfile_close(ifile);
                mfile_file_destroy(&ifile);
                mfile_file_destroy(&csv_file);
            }else{
                MX_ERROR("file open failed[%s] [%d/%s]\n", cfg->files[i], errno, strerror(errno));
            }
        }
    }
    
    MX_LPRINT(TBINX, 1, "tx count/bytes[%d/%d]\n", trn_tx_count, trn_tx_bytes);
    MX_LPRINT(TBINX, 1, "rx count/bytes[%d/%d]\n", trn_rx_count, trn_rx_bytes);
    MX_LPRINT(TBINX, 1, "trn count/bytes[%d/%d]\n", trn_msg_count, trn_msg_bytes);
    MX_LPRINT(TBINX, 1, "g_interrupt[%d]\n", (g_interrupt ? 1 : 0));
    MX_LPRINT(TBINX, 1, "g_sig_count[%d]\n", g_sig_count);
    MX_LPRINT(TBINX, 1, "g_alt_count[%d]\n", g_alt_count);

    return retval;
}
// End function s_process_file

/// @fn int main(int argc, char ** argv)
/// @brief TRN test client main entry point.
/// may specify arguments on command line:
/// host   UDP server host (MB System host)
/// port   UDP server port (MB System TRN output port)
/// block  use blocking IO
/// cycles number of cycles (<=0 to run indefinitely)
/// bsize  buffer size
/// @param[in] argc number of command line arguments
/// @param[in] argv array of command line arguments (strings)
/// @return 0 on success, -1 otherwise
int main(int argc, char **argv)
{
    int retval=0;

    struct sigaction saStruct;
    sigemptyset(&saStruct.sa_mask);
    saStruct.sa_flags = 0;
    saStruct.sa_handler = s_sig_handler;
    sigaction(SIGINT, &saStruct, NULL);
    sigaction(SIGQUIT, &saStruct, NULL);

    // set default app configuration
    app_cfg_t cfg = {TBX_VERBOSE_DFL,TBX_NFILES_DFL,NULL,
        TBX_OFLAGS_DFL,NULL,
        TBX_HOST_DFL,TBX_PORT_DFL,
        TBX_DELAY_DFL,
        TBX_RCDMS_DFL
    };
    
    app_cfg_t *pcfg = &cfg;
    
    if (argc<2) {
        s_show_help();
    }else{
        // parse command line args (update config)
        parse_args(argc, argv, pcfg);

        s_process_file(pcfg);
    }

    free(pcfg->csv_path);
    return retval;
}
// End function main

