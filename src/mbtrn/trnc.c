///
/// @file trnc.c
/// @authors k. Headley
/// @date 01 jan 2018

/// TRN test client
/// subscribes to mbtrnpreprocess data

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
#include "r7kc.h"
#include "r7k-reader.h"
#include "mmdebug.h"
#include "medebug.h"
#include "mutils.h"
#include "mb1_msg.h"

/////////////////////////
// Macros
/////////////////////////
#define TRNC_NAME "trnc"
#ifndef TRNC_VER
/// @def TRNC_VER
/// @brief module build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DTRNC_VER=<version>
#define TRNC_VER (dev)
#endif
#ifndef TRNC_BUILD
/// @def LIBTRNC_BUILD
/// @brief module build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DMFRAME_BUILD=`date`
#define TRNC_BUILD VERSION_STRING(TRNC_VER)" "LIBMFRAME_BUILD
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

/// @def TRNC_VERBOSE_DFL
/// @brief default debug level
#define TRNC_VERBOSE_DFL 1
/// @def TRNC_HOST_DFL
/// @brief default server host
#define TRNC_HOST_DFL "localhost"
/// @def TRNC_PORT_DFL
/// @brief default UDP socket port
#define TRNC_PORT_DFL 27000
/// @def TRNC_BLOCK_DFL
/// @brief default socket blocking
#define TRNC_BLOCK_DFL 1
/// @def TRNC_CYCLES_DFL
/// @brief default cycles
#define TRNC_CYCLES_DFL (-1)
/// @def TRNC_HBEAT_DFL
/// @brief default heartbeat interval
#define TRNC_HBEAT_DFL 20
/// @def TRNC_BUF_LEN
/// @brief default buffer length
#define TRNC_BUF_LEN   2048

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

///// @enum trnc_channel_id
///// @brief test module channel IDs
///// [note : starting above reserved mframe channel IDs]
//typedef enum{
//    ID_TRNC_V1=MM_CHANNEL_COUNT,
//    ID_TRNC_V2,
//    TRNC_CH_COUNT
//}trnc_channel_id;
//
///// @enum trnc_channel_mask
///// @brief test module channel masks
//typedef enum{
//    TRNC_V1= (1<<ID_TRNC_V1),
//    TRNC_V2= (1<<ID_TRNC_V2)
//}trnc_channel_mask;
//
//
///// @var char *trnc_ch_names[TRNC_CH_COUNT]
///// @brief module channel names
//char *trnc_ch_names[TRNC_CH_COUNT]={
//    "trace.trnc",
//    "debug.trnc",
//    "warn.trnc",
//    "err.trnc",
//    "trnc.v1",
//    "trnc.v2"
//};
//static mmd_module_config_t mmd_config_default= {MOD_TRNC,"MOD_TRNC",TRNC_CH_COUNT,((MM_ERR|MM_WARN)|TRNC_1),trnc_ch_names};

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
}app_cfg_t;

/// @typedef enum trnc_action_t trnc_action_t
/// @brief state machine actions
typedef enum{AT_NOP=0,AT_CONNECT,AT_WR_REQ,AT_RD_MSG,AT_SHOW_MSG,AT_QUIT}trnc_action_t;

/// @typedef enum trnc_state_t trnc_state_t
/// @brief state machine states
typedef enum{ST_INIT=0,ST_CONNECTED,ST_REQ_PENDING,ST_SUBSCRIBED, ST_HBEAT_EXPIRED,ST_DONE}trnc_state_t;

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
                        cfg->host=TRNC_HOST_DFL;
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
                    cfg->bsize = (cfg->bsize>0 ? cfg->bsize : TRNC_BUF_LEN);

                }
                break;
            default:
                help=true;
                break;
        }
        if (version) {
            MFRAME_SHOW_VERSION(TRNC_NAME, TRNC_BUILD);
            //             r7kr_show_app_version(TRNC_NAME,TRNC_BUILD);
            exit(0);
        }
        if (help) {
            MFRAME_SHOW_VERSION(TRNC_NAME, TRNC_BUILD);
            //             r7kr_show_app_version(TRNC_NAME,TRNC_BUILD);
            s_show_help();
            exit(0);
        }
    }// while
    
    mconf_init(NULL,NULL);
    mmd_channel_set(MOD_TRNC,MM_ERR);
    mmd_channel_set(MOD_R7K,MM_ERR);
    mmd_channel_set(MOD_R7KR,MM_ERR);
    mmd_channel_set(MOD_MSOCK,MM_ERR);

    switch (cfg->verbose) {
        case 0:
            mmd_channel_set(MOD_TRNC,0);
            mmd_channel_set(MOD_R7K,0);
            mmd_channel_set(MOD_R7KR,0);
            mmd_channel_set(MOD_MSOCK,0);
            break;
        case 1:
            mmd_channel_en(MOD_TRNC,TRNC_V1);
            mmd_channel_en(MOD_S7K,MM_DEBUG);
            break;
        case 2:
            mmd_channel_en(MOD_TRNC,TRNC_V1);
            mmd_channel_en(MOD_TRNC,TRNC_V2);
            mmd_channel_en(MOD_TRNC,MM_DEBUG);
            mmd_channel_en(MOD_MSOCK,MM_DEBUG);
            mmd_channel_en(MOD_R7K,MM_DEBUG);
            mmd_channel_en(MOD_R7KR,MM_DEBUG);
            break;
        default:
            mmd_channel_en(MOD_TRNC,MM_DEBUG);
            break;
    }
    
    PMPRINT(MOD_TRNC,MM_DEBUG,(stderr,"verbose [%s]\n",(cfg->verbose?"Y":"N")));
    PMPRINT(MOD_TRNC,MM_DEBUG,(stderr,"host    [%s]\n",cfg->host));
    PMPRINT(MOD_TRNC,MM_DEBUG,(stderr,"port    [%d]\n",cfg->port));
    PMPRINT(MOD_TRNC,MM_DEBUG,(stderr,"hbeat   [%d]\n",cfg->hbeat));
    PMPRINT(MOD_TRNC,MM_DEBUG,(stderr,"block   [%s]\n",(cfg->blocking==0?"N":"Y")));
    PMPRINT(MOD_TRNC,MM_DEBUG,(stderr,"cycles  [%d]\n",cfg->cycles));
    PMPRINT(MOD_TRNC,MM_DEBUG,(stderr,"bsize   [%d]\n",cfg->bsize));
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
            PMPRINT(MOD_TRNC,MM_DEBUG,(stderr,"\nsig received[%d]\n",signum));
            g_interrupt=true;
            break;
        default:
            fprintf(stderr,"\ns_termination_handler: sig not handled[%d]\n",signum);
            break;
    }
}
// End function termination_handler

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
        byte msg_buf[MB1_MAX_FRAME_BYTES]={0};
        int hbeat_counter=0;
        int64_t test=0;
        byte *pread=NULL;
        uint32_t readlen=0;
        
        trnc_state_t state=ST_INIT;
        trnc_action_t action=AT_NOP;
        
        // state machine entry point
        while (state != ST_DONE && !g_interrupt) {
            
            // check states, assign actions
            switch (state) {
                case ST_INIT:
//                    memset(&message,0,MBTRN_MAX_MSG_BYTES);
                    memset(&msg_buf,0,MB1_MAX_FRAME_BYTES);
                    action = AT_CONNECT;
                    break;
                case ST_CONNECTED:
                    action=AT_WR_REQ;
                    break;
                case ST_REQ_PENDING:
                case ST_SUBSCRIBED:
//                    memset(&message,0,MBTRN_MAX_MSG_BYTES);
                    memset(&msg_buf,0,MB1_MAX_FRAME_BYTES);
                    action=AT_RD_MSG;
                    break;
                case ST_HBEAT_EXPIRED:
                    action=AT_WR_REQ;
                    break;
                    
                default:
                    break;
            }// switch
            
            // action: connect
            if (action == AT_CONNECT) {
                PMPRINT(MOD_TRNC,MM_DEBUG,(stderr,"connecting [%s:%d]\n",cfg->host,cfg->port));
                if( (test=msock_connect(s))==0 ) {
                    PMPRINT(MOD_TRNC,MM_DEBUG,(stderr,"connect OK fd[%d]\n",s->fd));
                    state=ST_CONNECTED;
                }else{
                    PEPRINT((stderr,"connect failed [%"PRId64"]\n",test));
                }
            }
            
            // action: write request
            if (action == AT_WR_REQ) {
                const char *reqstr="REQ\0";

                test=msock_sendto(s,NULL,(byte *)reqstr,4,0);
                
                PMPRINT(MOD_TRNC,MM_DEBUG,(stderr,"sendto REQ ret[%"PRId64"] [%d/%s]\n",test,errno,strerror(errno)));
                
                if( test>0 ){
                    trn_tx_count++;
                    trn_tx_bytes+=test;
                    state=ST_REQ_PENDING;
                }else{
                    PMPRINT(MOD_TRNC,MM_DEBUG,(stderr,"sendto failed ret[%"PRId64"] [%d/%s]\n",test,errno,strerror(errno)));
                }
            }
            
            // action: read response
            if (action == AT_RD_MSG) {
//                pread = (byte *)&message;
//                readlen = MBTRN_MAX_MSG_BYTES;
                pread = msg_buf;
                readlen = MB1_MAX_FRAME_BYTES;
                
                // request message
                if ((test = msock_recvfrom(s, NULL, pread, readlen,0))>0) {
                    
                    trn_rx_bytes+=test;
                    trn_rx_count++;
                    
                    mfu_hex_show(msg_buf, test, 16, true, 5);
                    
                    // check message type
                    mb1_frame_t frame = {0},*mb1=&frame;
                    
                    mb1->sounding=(mb1_sounding_t *)(msg_buf);
                    mb1->checksum=MB1_PCHECKSUM_U32(mb1);
                    
//                    mb1->sounding = (mb1_sounding_t *)(msg_buf+sizeof(mb1_frame_t));
//                    mb1->checksum = MB1_PCHECKSUM_U32(mb1);
                    if (mb1->sounding->type==MBTRN_MSGTYPE_ACK) {
                        PMPRINT(MOD_TRNC,MM_DEBUG,(stderr,"received ACK ret[%"PRId64"] [%08X]\n",test,mb1->sounding->type));
                        hbeat_counter=0;
                        state=ST_SUBSCRIBED;
                    }else if (mb1->sounding->type==MBTRN_MSGTYPE_MB1) {
                        PMPRINT(MOD_TRNC,MM_DEBUG,(stderr,"received MSG ret[%"PRId64"] type[%08X] size[%d] ping[%06d]\n",test,mb1->sounding->type,mb1->sounding->size,mb1->sounding->ping_number));
                        trn_msg_count++;
                        trn_msg_bytes+=test;
                        
                        action=AT_SHOW_MSG;
                        
                        if (state!=ST_REQ_PENDING){
                            state=ST_SUBSCRIBED;
                        }
                        hbeat_counter++;
                        PMPRINT(MOD_TRNC,MM_DEBUG,(stderr,"hbeat[%d/%d]\n",hbeat_counter,cfg->hbeat));
                        if ( (hbeat_counter!=0) && (hbeat_counter%cfg->hbeat==0)) {
                            state=ST_HBEAT_EXPIRED;
                        }
                    }else{
                        // response not recognized
                        PMPRINT(MOD_TRNC,MM_DEBUG,(stderr,"invalid message [%08X]\n",mb1->sounding->type));
                    }
                }else{
                    // read returned error
                    //  PMPRINT(MOD_TRNC,MM_DEBUG,(stderr,"invalid message [%d]\n",test));
                    switch (errno) {
                        case EWOULDBLOCK:
                            // nothing to read
                            //   PMPRINT(MOD_TRNC,MM_DEBUG,(stderr,"err - [%d/%s]\n",errno, strerror(errno)));
                            break;
                        case ENOTCONN:
                        case ECONNREFUSED:
                            // host disconnected
                            PMPRINT(MOD_TRNC,MM_DEBUG,(stderr,"err - server not connected [%d/%s]\n",errno, strerror(errno)));
                            msock_socket_destroy(&s);
                            s = msock_socket_new(cfg->host, cfg->port, ST_UDP);
                            msock_set_blocking(s,(cfg->blocking==0?false:true));
                            sleep(5);
                            state=ST_INIT;
                            retval=-1;
                            break;
                        default:
                            PMPRINT(MOD_TRNC,MM_DEBUG,(stderr,"err ? [%d/%s]\n",errno, strerror(errno)));
                            break;
                    }//switch
                }
            }
            
            // action: show message
            if (action == AT_SHOW_MSG) {
//                PMPRINT(MOD_TRNC,MM_DEBUG|TRNC_V1|TRNC_V2,(stderr,"\nts[%.3f] ping[%06d] lat[%.4lf] lon[%.4lf]\nsd[%7.2lf] hdg[%6.2lf] nb[%03"PRIu32"]\n",
//                        psounding->ts,
//                        psounding->ping_number,
//                        psounding->lat,
//                        psounding->lon,
//                        psounding->depth,
//                        psounding->hdg,
//                        (uint32_t)psounding->nbeams));
//                uint32_t j=0;
//                struct mbtrn_beam_data *bd=psounding->beams;
//                for (j=0; j<psounding->nbeams; j++,bd++){
//                    PMPRINT(MOD_TRNC,MM_DEBUG|TRNC_V2,(stderr,"n[%03"PRIu32"] atrk/X[% 10.3lf] ctrk/Y[% 10.3lf] dpth/Z[% 10.3lf]\n",
//                            (uint32_t)bd->beam_num,
//                            bd->rhox,
//                            bd->rhoy,
//                            bd->rhoz));
//                }
                
                mb1_frame_t frame = {0},*mb1=&frame;
                
                mb1->sounding=(mb1_sounding_t *)(msg_buf);
                mb1->checksum=MB1_PCHECKSUM_U32(mb1);
                PMPRINT(MOD_TRNC,MM_DEBUG|TRNC_V1|TRNC_V2,(stderr,"\nts[%.3f] ping[%06d] lat[%.4lf] lon[%.4lf]\nsd[%7.2lf] hdg[%6.2lf] nb[%03"PRIu32"]\n",
                                                           mb1->sounding->ts,
                                                           mb1->sounding->ping_number,
                                                           mb1->sounding->lat,
                                                           mb1->sounding->lon,
                                                           mb1->sounding->depth,
                                                           mb1->sounding->hdg,
                                                           (uint32_t)mb1->sounding->nbeams));
                uint32_t j=0;
                mb1_beam_t *bd=mb1->sounding->beams;
                for (j=0; j<mb1->sounding->nbeams; j++,bd++){
                    PMPRINT(MOD_TRNC,MM_DEBUG|TRNC_V2,(stderr,"n[%03"PRIu32"] atrk/X[% 10.3lf] ctrk/Y[% 10.3lf] dpth/Z[% 10.3lf]\n",
                                                       (uint32_t)bd->beam_num,
                                                       bd->rhox,
                                                       bd->rhoy,
                                                       bd->rhoz));
                }


            }
            
            // action: quit state machine
            if (action == AT_QUIT) {
                break;
            }
            
            // check cycles and signals
            scycles--;
            if(scycles==0){
                PTRACE();
                retval=0;
                state=ST_DONE;
            }
            
            if(g_interrupt){
                PTRACE();
                retval=-1;
                state=ST_DONE;
            }
            
        }// while !ST_DONE
    }//else invalid arg
    
    PMPRINT(MOD_TRNC,TRNC_V1,(stderr,"tx count/bytes[%d/%d]\n",trn_tx_count,trn_tx_bytes));
    PMPRINT(MOD_TRNC,TRNC_V1,(stderr,"rx count/bytes[%d/%d]\n",trn_rx_count,trn_rx_bytes));
    PMPRINT(MOD_TRNC,TRNC_V1,(stderr,"trn count/bytes[%d/%d]\n",trn_msg_count,trn_msg_bytes));
    
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
        TRNC_VERBOSE_DFL,
        strdup(TRNC_HOST_DFL),
        TRNC_PORT_DFL,
        TRNC_BLOCK_DFL,
        TRNC_CYCLES_DFL,
        TRNC_HBEAT_DFL,
        TRNC_BUF_LEN
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

