///
/// @file mbtnav_cli
/// @authors k. Headley
/// @date 01 jan 2018

/// TRN client - subscribes to MB-System TRN updates

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
#include "msocket.h"
#include "mxdebug.h"
#include "mxd_app.h"
#include "mutils.h"
#include "mtime.h"
#include "trn_msg.h"

/////////////////////////
// Macros
/////////////////////////
#define MBTNAV_NAME "mbtnav-cli"
#ifndef MBTNAV_VER
/// @def MBTNAV_VER
/// @brief module build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DMBTNAV_VER=<version>
#define MBTNAV_VER (dev)
#endif
#ifndef MBTNAV_BUILD
/// @def MBTNAV_BUILD
/// @brief module build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DMBTNAV_BUILD=`date`
#define MBTNAV_BUILD VERSION_STRING(MBTNAV_VER)" "LIBMFRAME_BUILD
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

/// @def MBTNAV_VERBOSE_DFL
/// @brief default debug level
#define MBTNAV_VERBOSE_DFL 1
/// @def MBTNAV_HOST_DFL
/// @brief default server host
#define MBTNAV_HOST_DFL "localhost"
/// @def MBTNAV_PORT_DFL
/// @brief default UDP socket port
#define MBTNAV_PORT_DFL 8000
/// @def MBTNAV_BLOCK_DFL
/// @brief default socket blocking
#define MBTNAV_BLOCK_DFL 1
/// @def MBTNAV_CYCLES_DFL
/// @brief default cycles
#define MBTNAV_CYCLES_DFL (-1)
/// @def MBTNAV_HBEAT_DFL
/// @brief default heartbeat interval
#define MBTNAV_HBEAT_DFL 20
/// @def MBTNAV_BUF_LEN
/// @brief default buffer length
#define MBTNAV_BUF_LEN   2048

/// @def ID_APP
/// @brief debug module ID
#define ID_APP  1
/// @def ID_APP2
/// @brief debug module ID
#define ID_APP2 2
/// @def ID_APP3
/// @brief debug module ID
#define ID_APP3 3

/// @def MBTRN_MSGTYPE_ACK
/// @brief TRN message type ACK
#define MBTRN_MSGTYPE_ACK (0x004B4341)
/// @def MBTRN_MSGTYPE_MB1
/// @brief TRN message type MB1 record
#define MBTRN_MSGTYPE_MB1 (0x0031424D)

/////////////////////////
// Declarations
/////////////////////////

typedef enum{
    OF_ASCII=0x1,
    OF_CSV=0x2,
    OF_HEX=0x4
}ofmt_flag_t;

/// @typedef struct app_cfg_s app_cfg_t
/// @brief application configuration parameter structure
typedef struct app_cfg_s{
    /// @var app_cfg_s::verbose
    /// @brief TBD
    int verbose;
    /// @var app_cfg_s::host
    /// @brief TBD
    char *host;
    /// @var app_cfg_s::port
    /// @brief TBD
    int port;
    /// @var app_cfg_s::blocking
    /// @brief use blocking IO
    int blocking;
    /// @var app_cfg_s::cycles
    /// @brief number of cycles (<=0 : unlimited)
    int cycles;
    /// @var app_cfg_s::hbeat
    /// @brief hbeat interval (packets)
    int hbeat;
    /// @var app_cfg_s::bsize
    /// @brief buffer size
    uint32_t bsize;
    /// @var app_cfg_s::ofmt
    /// @brief output format flags
    ofmt_flag_t ofmt;
}app_cfg_t;

/// @typedef enum trnc_action_t trnc_action_t
/// @brief state machine actions
typedef enum{TRNAT_NOP=0,TRNAT_CONNECT,TRNAT_WR_REQ,TRNAT_RD_MSG,TRNAT_SHOW_MSG,TRNAT_QUIT}trnc_action_t;

/// @typedef enum trnc_state_t trnc_state_t
/// @brief state machine states
typedef enum{TRNSM_INIT=0,TRNSM_CONNECTED,TRNSM_REQ_PENDING,TRNSM_SUBSCRIBED, TRNSM_HBEAT_EXPIRED,TRNSM_DONE}trnc_state_t;

static void s_show_help();

/////////////////////////
// Imports
/////////////////////////

/////////////////////////
// Module Global Variables
/////////////////////////
static bool g_interrupt=false;

/////////////////////////
// Function Definitions
/////////////////////////

/// @fn void s_show_help()
/// @brief output user help message to stdout.
/// @return none
static void s_show_help()
{
    char help_message[] = "\nTRN test client\n";
    char usage_message[] = "\ntrnc [options]\n"
    "--verbose=n    : verbose output, n>0\n"
    "--help         : output help message\n"
    "--version      : output version info\n"
    "--host=ip:n    : TRN server host\n"
    "--hbeat=n      : hbeat interval (packets)\n"
    "--blocking=0|1 : blocking receive [0:1]\n"
    "--bsize=n      : buffer size\n"
    "--ofmt=a|c|h   : output formats (one or more of a:ascii c:csv h:hex)\n"
//    "--port=n       : UDP server port\n"
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

    static struct option options[] = {
        {"verbose", required_argument, NULL, 0},
        {"help", no_argument, NULL, 0},
        {"version", no_argument, NULL, 0},
        {"host", required_argument, NULL, 0},
        {"hbeat", required_argument, NULL, 0},
        {"blocking", required_argument, NULL, 0},
        {"cycles", required_argument, NULL, 0},
        {"bsize", required_argument, NULL, 0},
        {"ofmt", required_argument, NULL, 0},
       {NULL, 0, NULL, 0}};

    // process argument list
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

                // host
                else if (strcmp("host", options[option_index].name) == 0) {

                    char *hsave=cfg->host;
                    char *ocopy=strdup(optarg);
                    cfg->host=strtok(ocopy,":");
                    if (cfg->host==NULL) {
                        cfg->host=MBTNAV_HOST_DFL;
                    }
                    char *ip = strtok(NULL,":");
                    if (ip!=NULL) {
                        sscanf(ip,"%d",&cfg->port);
                    }
                    // don't free ocopy here
                    if(hsave!=NULL){
                        free(hsave);
                    }
                }
                // blocking
                else if (strcmp("blocking", options[option_index].name) == 0) {
                    sscanf(optarg,"%d",&cfg->blocking);
                }
                // hbeat
                else if (strcmp("hbeat", options[option_index].name) == 0) {
                    sscanf(optarg,"%d",&cfg->hbeat);
                }
                // cycles
                else if (strcmp("cycles", options[option_index].name) == 0) {
                    sscanf(optarg,"%d",&cfg->cycles);
                }
                // buffer size
                else if (strcmp("bsize", options[option_index].name) == 0) {
                    sscanf(optarg,"%u",&cfg->bsize);
                    cfg->bsize = (cfg->bsize>0 ? cfg->bsize : MBTNAV_BUF_LEN);

                }
                // output format
                else if (strcmp("ofmt", options[option_index].name) == 0) {
                	char *cp=optarg;
                 	while(*cp!='\0'){
                        switch(toupper(*cp)){
                            case 'A':
                                cfg->ofmt |= OF_ASCII;
                                break;
                            case 'H':
                                cfg->ofmt |= OF_HEX;
                                break;
                            case 'C':
                                cfg->ofmt |= OF_CSV;
                                break;
                            default:
                                fprintf(stderr,"WARN: unknown output format[%c]\n",*cp);
                                break;
                        }
                        cp++;
                    }
                }
                break;
            default:
                help=true;
                break;
        }
        if (version) {
            MFRAME_SHOW_VERSION(MBTNAV_NAME, MBTNAV_BUILD);
            //             r7kr_show_app_version(MBTNAV_NAME,MBTNAV_BUILD);
            exit(0);
        }
        if (help) {
            MFRAME_SHOW_VERSION(MBTNAV_NAME, MBTNAV_BUILD);
            //             r7kr_show_app_version(MBTNAV_NAME,MBTNAV_BUILD);
            s_show_help();
            exit(0);
        }
    }// while

    mxd_setModule(MXDEBUG, 0, true, NULL);
    mxd_setModule(MXERROR, 5, false, NULL);
    mxd_setModule(MBTNAVC, 0, true, "mbtnavc");
    mxd_setModule(MBTNAVC_ERROR, 0, true, "mbtnavc.error");
    mxd_setModule(MBTNAVC_DEBUG, 0, true, "mbtnavc.debug");
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
            mxd_setModule(MBTNAVC, 1, false, "mbtnavc.error");
            break;
        case 2:
            mxd_setModule(MXDEBUG, 5, false, NULL);
            mxd_setModule(MBTNAVC_ERROR, 5, false, "mbtnavc.error");
            break;
        case 3:
        case 4:
        case 5:
            mxd_setModule(MXDEBUG, 5, false, NULL);
            mxd_setModule(MBTNAVC_ERROR, 5, false, "mbtnavc.error");
            mxd_setModule(MBTNAVC_DEBUG, 5, false, "mbtnavc.debug");
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

    MX_MPRINT(MBTNAVC, "verbose [%s]\n", (cfg->verbose?"Y":"N"));
    MX_MPRINT(MBTNAVC, "host    [%s]\n", cfg->host);
    MX_MPRINT(MBTNAVC, "port    [%d]\n", cfg->port);
    MX_MPRINT(MBTNAVC, "hbeat   [%d]\n", cfg->hbeat);
    MX_MPRINT(MBTNAVC, "block   [%s]\n", (cfg->blocking==0?"N":"Y"));
    MX_MPRINT(MBTNAVC, "cycles  [%d]\n", cfg->cycles);
    MX_MPRINT(MBTNAVC, "bsize   [%d]\n", cfg->bsize);
    MX_MPRINT(MBTNAVC, "ofmt    [%x]\n", cfg->ofmt);
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
            MX_MPRINT(MBTNAVC, "\nsig received[%d]\n",signum);
            g_interrupt=true;
            break;
        default:
            fprintf(stderr,"\ns_termination_handler: sig not handled[%d]\n",signum);
            break;
    }
}
// End function termination_handler

static void s_trnw_estimate_show(trnu_estimate_t *self, bool verbose, uint16_t indent)
{
    if (NULL != self) {
        fprintf(stderr,"%*s[self        %15p]\n",indent,(indent>0?" ":""), self);
        fprintf(stderr,"%*s[time        %15.3lf]\n",indent,(indent>0?" ":""),self->time);
        fprintf(stderr,"%*s[x           %15.3lf]\n",indent,(indent>0?" ":""),self->x);
        fprintf(stderr,"%*s[y           %15.3lf]\n",indent,(indent>0?" ":""),self->y);
        fprintf(stderr,"%*s[z           %15.3lf]\n",indent,(indent>0?" ":""),self->z);
        for(int i=0;i<4;i++)
        fprintf(stderr,"%*s[cov[%d]     %15.3lf]\n",indent,(indent>0?" ":""),i,self->cov[i]);
    }
}

static void s_trnw_offset_show_org(trnu_pub_t *self, bool verbose, uint16_t indent)
{
    if (NULL != self) {
        fprintf(stderr,"%*s[self        %15p]\n",indent,(indent>0?" ":""), self);
        fprintf(stderr,"%*s[ pt ]\n",indent,(indent>0?" ":""));
        s_trnw_estimate_show(&self->est[0],verbose,indent+1);
        fprintf(stderr,"%*s[ mle ]\n",indent,(indent>0?" ":""));
        s_trnw_estimate_show(&self->est[1],verbose,indent+1);
        fprintf(stderr,"%*s[ mse ]\n",indent,(indent>0?" ":""));
        s_trnw_estimate_show(&self->est[2],verbose,indent+1);
        fprintf(stderr,"%*s[reinit       %15d]\n",indent,(indent>0?" ":""), self->reinit_count);
        fprintf(stderr,"%*s[reinit_t     %15.3lf]\n",indent,(indent>0?" ":""),self->reinit_tlast);
        fprintf(stderr,"%*s[filt_state   %15d]\n",indent,(indent>0?" ":""), self->filter_state);
        fprintf(stderr,"%*s[success      %15d]\n",indent,(indent>0?" ":""), self->success);
        fprintf(stderr,"%*s[is_converged %15hd]\n",indent,(indent>0?" ":""), self->is_converged);
        fprintf(stderr,"%*s[is_valid     %15hd]\n",indent,(indent>0?" ":""), self->is_valid);
        fprintf(stderr,"%*s[mb1_cycle    %15d]\n",indent,(indent>0?" ":""), self->mb1_cycle);
        fprintf(stderr,"%*s[ping_number  %15d]\n",indent,(indent>0?" ":""), self->ping_number);
        fprintf(stderr,"%*s[mb1_time     %15.3lf]\n",indent,(indent>0?" ":""),self->mb1_time);
        fprintf(stderr,"%*s[update_time  %15.3lf]\n",indent,(indent>0?" ":""),self->update_time);
    }
}

static void s_trnw_offset_show(trnu_pub_t *self, bool verbose, uint16_t indent)
{
    if (NULL != self) {
        fprintf(stderr,"%*s[self        %15p]\n",indent,(indent>0?" ":""), self);
        fprintf(stderr,"%*s[ pt ]\n",indent,(indent>0?" ":""));
        s_trnw_estimate_show(&self->est[0],verbose,indent+1);
        fprintf(stderr,"%*s[ mle ]\n",indent,(indent>0?" ":""));
        s_trnw_estimate_show(&self->est[1],verbose,indent+1);
        fprintf(stderr,"%*s[ mse ]\n",indent,(indent>0?" ":""));
        s_trnw_estimate_show(&self->est[2],verbose,indent+1);
        fprintf(stderr,"%*s[ offset ]\n",indent,(indent>0?" ":""));
        s_trnw_estimate_show(&self->est[3],verbose,indent+1);
        fprintf(stderr,"%*s[ last useful ]\n",indent,(indent>0?" ":""));
        s_trnw_estimate_show(&self->est[4],verbose,indent+1);
        fprintf(stderr,"%*s[reinit       %15d]\n",indent,(indent>0?" ":""), self->reinit_count);
        fprintf(stderr,"%*s[reinit_t     %15.3lf]\n",indent,(indent>0?" ":""),self->reinit_tlast);
        fprintf(stderr,"%*s[filt_state   %15d]\n",indent,(indent>0?" ":""), self->filter_state);
        fprintf(stderr,"%*s[success      %15d]\n",indent,(indent>0?" ":""), self->success);
        fprintf(stderr,"%*s[is_converged %15hd]\n",indent,(indent>0?" ":""), self->is_converged);
        fprintf(stderr,"%*s[is_valid     %15hd]\n",indent,(indent>0?" ":""), self->is_valid);
        fprintf(stderr,"%*s[mb1_cycle    %15d]\n",indent,(indent>0?" ":""), self->mb1_cycle);
        fprintf(stderr,"%*s[ping_number  %15d]\n",indent,(indent>0?" ":""), self->ping_number);
        fprintf(stderr,"%*s[mb1_time     %15.3lf]\n",indent,(indent>0?" ":""),self->mb1_time);
        fprintf(stderr,"%*s[update_time  %15.3lf]\n",indent,(indent>0?" ":""),self->update_time);
        fprintf(stderr,"%*s[n_con_seq    %15d]\n",indent,(indent>0?" ":""),self->n_con_seq);
        fprintf(stderr,"%*s[n_con_tot    %15d]\n",indent,(indent>0?" ":""),self->n_con_tot);
        fprintf(stderr,"%*s[n_uncon_seq  %15d]\n",indent,(indent>0?" ":""),self->n_uncon_seq);
        fprintf(stderr,"%*s[n_uncon_tot  %15d]\n",indent,(indent>0?" ":""),self->n_uncon_tot);
        fprintf(stderr,"%*s[reinit_time  %15.3lf]\n",indent,(indent>0?" ":""),self->reinit_time);

    }
}

static void s_out_csv_org(trnu_pub_t *self)
{
    double time=mtime_etime();//s_etime();
        trnu_estimate_t *pt=&self->est[0];
        trnu_estimate_t *mle=&self->est[1];
        trnu_estimate_t *mse=&self->est[2];
    // system time,
    // mle_time, mle.x, mle.y, mle.z
    // mmse_time, mmse.x, mmse.y, mmse.z
    // pt.x, pt.y, pt.z
    // cov[0],cov[2],cov[5]
    // reinit_count, filter_state, lm_successful, mb1_cycle, mb1_ping_number, isconverged
    fprintf(stderr,"%.3lf,%.3lf,%.4lf,%.4lf,%.4lf,",time,mle->time,mle->x,mle->y,mle->z);
    fprintf(stderr,"%.3lf,%.4lf,%.4lf,%.4lf,",mse->time,mse->x,mse->y,mse->z);
    fprintf(stderr,"%.4lf,%.4lf,%.4lf,",pt->x, pt->y, pt->z);
    fprintf(stderr,"%.3lf,%.3lf,%.3lf,",sqrt(mse->cov[0]),sqrt(mse->cov[1]),sqrt(mse->cov[2]));
    fprintf(stderr,"%d,%d,%d,%d,%d,%hd,%hd\n",self->reinit_count,self->filter_state,self->success,self->mb1_cycle,self->ping_number,self->is_converged,self->is_valid);
}

static void s_out_csv(trnu_pub_t *self)
{
    double time=mtime_etime();//s_etime();
    trnu_estimate_t *pt=&self->est[0];
    trnu_estimate_t *mle=&self->est[1];
    trnu_estimate_t *mse=&self->est[2];
    trnu_estimate_t *offset=&self->est[3];
    trnu_estimate_t *recent=&self->est[4];
    // system time,
    // mle_time, mle.x, mle.y, mle.z
    // mmse_time, mmse.x, mmse.y, mmse.z
    // pt.x, pt.y, pt.z
    // cov[0],cov[2],cov[5]
    // reinit_count, filter_state, lm_successful, mb1_cycle, mb1_ping_number, isconverged
    fprintf(stderr,"%.3lf,%.3lf,%.4lf,%.4lf,%.4lf,",time,mle->time,mle->x,mle->y,mle->z);
    fprintf(stderr,"%.3lf,%.4lf,%.4lf,%.4lf,",mse->time,mse->x,mse->y,mse->z);
    fprintf(stderr,"%.4lf,%.4lf,%.4lf,",pt->x, pt->y, pt->z);
    fprintf(stderr,"%.3lf,%.3lf,%.3lf,",sqrt(mse->cov[0]),sqrt(mse->cov[1]),sqrt(mse->cov[2]));
    fprintf(stderr,"%d,%d,%d,%d,%d,%hd,%hd,",self->reinit_count,self->filter_state,self->success,self->mb1_cycle,self->ping_number,self->is_converged,self->is_valid);
    fprintf(stderr,"%.3lf,%.4lf,%.4lf,%.4lf,",offset->time,offset->x,offset->y,offset->z);
    fprintf(stderr,"%.3lf,%.4lf,%.4lf,%.4lf,",recent->time,recent->x,recent->y,recent->z);
    fprintf(stderr,"%d,%d,%d,%d\n",self->n_con_seq,self->n_con_tot,self->n_uncon_seq,self->n_uncon_tot);
}

/// @fn int s_trnc_state_machine(msock_socket_t *s, app_cfg_t *cfg)
/// @brief state machine implementation
/// @param[in] s msock_socket_t reference
/// @param[in] cfg app_cfg reference
/// @return none
static int s_trnc_state_machine(msock_socket_t *s, app_cfg_t *cfg)
{
    int retval=-1;
    int scycles=cfg->cycles;
    int trn_tx_count=0;
    int trn_rx_count=0;
    int trn_tx_bytes=0;
    int trn_rx_bytes=0;
    int trn_msg_count=0;
    int trn_msg_bytes=0;

    if (NULL!=s) {

        // initialize variables
//        TODO replace
//        trn_message_t message;
//        trn_message_t *pmessage = &message;
//        mbtrn_header_t *pheader = &pmessage->data.header;
//        mbtrn_sounding_t *psounding = &pmessage->data.sounding;
//        memset(pmessage,0,sizeof(message));
        byte msg_buf[TRNU_PUB_BYTES]={0};
        int hbeat_counter=0;
        int64_t test=0;
        byte *pread=NULL;
        uint32_t readlen=0;

        trnc_state_t state=TRNSM_INIT;
        trnc_action_t action=TRNAT_NOP;
        trnu_pub_t *frame = (trnu_pub_t *)msg_buf;

        // state machine entry point
        while (state != TRNSM_DONE && !g_interrupt) {

            // check states, assign actions
            switch (state) {
                case TRNSM_INIT:
                    memset(&msg_buf,0,TRNU_PUB_BYTES);
                    action = TRNAT_CONNECT;
                    break;
                case TRNSM_CONNECTED:
                    action=TRNAT_WR_REQ;
                    break;
                case TRNSM_REQ_PENDING:
                case TRNSM_SUBSCRIBED:
                    memset(&msg_buf,0,TRNU_PUB_BYTES);
                    action=TRNAT_RD_MSG;
                    break;
                case TRNSM_HBEAT_EXPIRED:
                    action=TRNAT_WR_REQ;
                    break;

                default:
                    break;
            }// switch

            // action: connect
            if (action == TRNAT_CONNECT) {
                MX_MPRINT(MBTNAVC, "connecting [%s:%d]\n", cfg->host, cfg->port);
                if( (test=msock_connect(s))==0 ) {
                    MX_MPRINT(MBTNAVC, "connect OK fd[%d]\n", s->fd);
                    state=TRNSM_CONNECTED;
                }else{
                    MX_ERROR("connect failed [%"PRId64"]\n", test);
                }
            }

            // action: write request
            if (action == TRNAT_WR_REQ) {
                const char *reqstr="REQ\0";

                test=msock_sendto(s,NULL,(byte *)reqstr,4,0);

                MX_MPRINT(MBTNAVC, "sendto REQ ret[%"PRId64"] [%d/%s]\n", test, errno, strerror(errno));

                if( test>0 ){
                    trn_tx_count++;
                    trn_tx_bytes+=test;
                    state=TRNSM_REQ_PENDING;
                }else{
                    MX_MPRINT(MBTNAVC, "sendto failed ret[%"PRId64"] [%d/%s]\n", test, errno, strerror(errno));
                }
            }

            // action: read response
            if (action == TRNAT_RD_MSG) {
                pread = msg_buf;
                readlen = TRNU_PUB_BYTES;

                // request message
                if ((test = msock_recvfrom(s, NULL, pread, readlen,0))>0) {

                    trn_rx_bytes+=test;
                    trn_rx_count++;


                    // check message type
                    if(frame->sync==MBTRN_MSGTYPE_ACK){
                        MX_MPRINT(MBTNAVC, "received ACK ret[%"PRId64"] [%08X]\n", test, frame->sync);
                        hbeat_counter=0;
                        state=TRNSM_SUBSCRIBED;
                    }else if(frame->sync==TRNU_PUB_SYNC && test==readlen){

                        MX_MPRINT(MBTNAVC, "received MSG ret[%"PRId64"] type[%08X] size[%"PRId64"] \n", test, frame->sync, test);
                        trn_msg_count++;
                        trn_msg_bytes+=test;
                        action=TRNAT_SHOW_MSG;
                        if (state!=TRNSM_REQ_PENDING){
                            state=TRNSM_SUBSCRIBED;
                        }
                        hbeat_counter++;
                        MX_MPRINT(MBTNAVC, "hbeat[%d/%d]\n", hbeat_counter, cfg->hbeat);
                        if ( (hbeat_counter!=0) &&  (cfg->hbeat>0) && (hbeat_counter%cfg->hbeat==0)) {
                            state=TRNSM_HBEAT_EXPIRED;
                        }
                    }else{
                        // response not recognized
                        MX_MPRINT(MBTNAVC, "invalid message sync[%u] len[%"PRId64"]\n", frame->sync, test);
                    }

                }else{
                    // read returned error
                    //  MX_MPRINT(MBTNAVC, "invalid message [%d]\n", test);
                    switch (errno) {
                        case EWOULDBLOCK:
                            // nothing to read
                            //   MX_MPRINT(MBTNAVC, "err - [%d/%s]\n", errno, strerror(errno));
                            break;
                        case ENOTCONN:
                        case ECONNREFUSED:
                            // host disconnected
                            MX_MPRINT(MBTNAVC, "err - server not connected [%d/%s]\n", errno, strerror(errno));
                            msock_socket_destroy(&s);
                            s = msock_socket_new(cfg->host, cfg->port, ST_UDP);
                            msock_set_blocking(s,(cfg->blocking==0?false:true));
                            sleep(5);
                            state=TRNSM_INIT;
                            retval=-1;
                            break;
                        default:
                            MX_MPRINT(MBTNAVC, "err ? [%d/%s]\n", errno, strerror(errno));
                            break;
                    }//switch
                }
            }

            // action: show message
            if (action == TRNAT_SHOW_MSG) {
                if( (cfg->ofmt&OF_HEX) != 0)
                mfu_hex_show(msg_buf, test, 16, true, 5);

                if( (cfg->ofmt&OF_ASCII) != 0)
                s_trnw_offset_show(frame,true,5);

                if( (cfg->ofmt&OF_CSV) != 0)
                    s_out_csv(frame);
            }

            // action: quit state machine
            if (action == TRNAT_QUIT) {
                break;
            }

            // check cycles and signals
            scycles--;
            if(scycles==0){
                MX_TRACE();
                retval=0;
                state=TRNSM_DONE;
            }

            if(g_interrupt){
                MX_TRACE();
                retval=-1;
                state=TRNSM_DONE;
            }

        }// while !TRNSM_DONE
    }//else invalid arg

    MX_LPRINT(MBTNAVC, 1, "tx count/bytes[%d/%d]\n", trn_tx_count, trn_tx_bytes);
    MX_LPRINT(MBTNAVC, 1, "rx count/bytes[%d/%d]\n", trn_rx_count, trn_rx_bytes);
    MX_LPRINT(MBTNAVC, 1, "trn count/bytes[%d/%d]\n", trn_msg_count, trn_msg_bytes);

    return retval;
}
// End function s_trnc_state_machine

/// @fn int s_app_main ()
/// @brief application main function.
/// @param[in]
/// @return none
static int s_app_main(app_cfg_t *cfg)
{
    int retval=0;

    // init receive buffer
    byte buf[cfg->bsize];
    memset(buf,0,cfg->bsize);

    // create socket
    msock_socket_t *s = msock_socket_new(cfg->host, cfg->port, ST_UDP);
    msock_set_blocking(s,(cfg->blocking==0?false:true));

// these options are used by mbtrnrcv
// setting these options is only for debug
//    int so_reuseaddr=1;
//    int so_rcvlowat=8;
//    struct timeval so_rcvtimeo={10,0};
//    setsockopt(s->fd,SOL_SOCKET,SO_REUSEADDR,&so_reuseaddr,sizeof(so_reuseaddr));
//    setsockopt(s->fd,SOL_SOCKET,SO_RCVLOWAT,&so_rcvlowat,sizeof(so_rcvlowat));
//    setsockopt(s->fd,SOL_SOCKET,SO_RCVTIMEO,&so_rcvtimeo,sizeof(so_rcvtimeo));

    retval=s_trnc_state_machine(s, cfg);

    return retval;
}
// End function s_app_main

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

    // set default app configuration
    app_cfg_t cfg = {
        MBTNAV_VERBOSE_DFL,
        strdup(MBTNAV_HOST_DFL),
        MBTNAV_PORT_DFL,
        MBTNAV_BLOCK_DFL,
        MBTNAV_CYCLES_DFL,
        MBTNAV_HBEAT_DFL,
        MBTNAV_BUF_LEN,
        0x0
    };

    // configure signal handling
    // for main thread
    struct sigaction saStruct;
    sigemptyset(&saStruct.sa_mask);
    saStruct.sa_flags = 0;
    saStruct.sa_handler = s_termination_handler;
    sigaction(SIGINT, &saStruct, NULL);

    // parse command line args (update config)
    parse_args(argc, argv, &cfg);

    // run the app
    retval = s_app_main(&cfg);

    return retval;
}
// End function main
