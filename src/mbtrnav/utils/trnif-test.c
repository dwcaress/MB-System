///
/// @file trnif-test.c
/// @authors k. Headley
/// @date 12 jun 2019

/// Unit test wrapper for trnif

/// Compile test using
/// gcc -DWITH_NETIF_TEST -o netif-test netif-test.c netif.c -L../bin -lmframe
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

#include "netif.h"
#include "trn_msg.h"
#include "trnw.h"
#include "trnif_proto.h"

#include "mframe.h"
#include "medebug.h"
#include "mmdebug.h"

/////////////////////////
// Macros
/////////////////////////

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

/////////////////////////
// Declarations
/////////////////////////
typedef struct app_cfg_s{
    int verbose;
    netif_t *netif;
    trn_config_t *trn_cfg;
    wtnav_t *trn;
    msock_socket_t *cli;
    char *host;
    int port;
    char *map;
    char *cfg;
    char *particles;
    char *logdir;
}app_cfg_t;

bool g_interrupt=false;

/////////////////////////
// Imports
/////////////////////////

/////////////////////////
// Module Global Variables
/////////////////////////

/////////////////////////
// Function Definitions
/////////////////////////

/// @fn void s_show_help()
/// @brief output user help message to stdout.
/// @return none
static void s_show_help()
{
    char help_message[] = "\ntrnif unit test\n";
    char usage_message[] = "\ntrnc [options]\n"
    "--verbose=n    : verbose output, n>0\n"
    "--help         : output help message\n"
    "--version      : output version info\n"
    "--host=ip:n    : TRN server host:port\n"
    "--map=s        : map file/directory [*]\n"
    "--cfg=s        : config file        [*]\n"
    "--particles=s  : particles file     [*]\n"
    "--logdir=s     : log directory      [*]\n"
    "[*] - required\n"
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
        {"map", required_argument, NULL, 0},
        {"cfg", required_argument, NULL, 0},
        {"particles", required_argument, NULL, 0},
        {"logdir", required_argument, NULL, 0},
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
                        cfg->host="localhost";
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
                // map
                else if (strcmp("map", options[option_index].name) == 0) {
                    if(NULL!=cfg->map)free(cfg->map);
                    cfg->map=(NULL==optarg?NULL:strdup(optarg));
                }
                // cfg
                else if (strcmp("cfg", options[option_index].name) == 0) {
                    if(NULL!=cfg->cfg)free(cfg->cfg);
                    cfg->cfg=(NULL==optarg?NULL:strdup(optarg));
                }
                // particles
                else if (strcmp("particles", options[option_index].name) == 0) {
                    if(NULL!=cfg->particles)free(cfg->particles);
                    cfg->particles=(NULL==optarg?NULL:strdup(optarg));
                }
                // logdir
                else if (strcmp("logdir", options[option_index].name) == 0) {
                    if(NULL!=cfg->logdir)free(cfg->logdir);
                   cfg->logdir=(NULL==optarg?NULL:strdup(optarg));

                }
                break;
            default:
                help=true;
                break;
        }
        if (version) {
            fprintf(stderr,"no version\n");
            exit(0);
        }
        if (help) {
            s_show_help();
            exit(0);
        }
    }// while
   

    fprintf(stderr,"verbose   [%d]\n",cfg->verbose);
    fprintf(stderr,"host      [%s]\n",cfg->host);
    fprintf(stderr,"port      [%d]\n",cfg->port);
    fprintf(stderr,"map       [%s]\n",cfg->map);
    fprintf(stderr,"cfg       [%s]\n",cfg->cfg);
    fprintf(stderr,"particles [%s]\n",cfg->map);
    fprintf(stderr,"logdir    [%s]\n",cfg->map);

//    mconf_init(NULL,NULL);
//    mmd_channel_set(MOD_MBTNAV,MM_ERR);
//    mmd_channel_set(MOD_R7K,MM_ERR);
//    mmd_channel_set(MOD_R7KR,MM_ERR);
//    mmd_channel_set(MOD_MSOCK,MM_ERR);
//
//    switch (cfg->verbose) {
//        case 0:
//            mmd_channel_set(MOD_MBTNAV,0);
//            mmd_channel_set(MOD_R7K,0);
//            mmd_channel_set(MOD_R7KR,0);
//            mmd_channel_set(MOD_MSOCK,0);
//            break;
//        case 1:
//            mmd_channel_en(MOD_MBTNAV,MBTNAV_V1);
//            mmd_channel_en(MOD_S7K,MM_DEBUG);
//            break;
//        case 2:
//            mmd_channel_en(MOD_MBTNAV,MBTNAV_V1);
//            mmd_channel_en(MOD_MBTNAV,MBTNAV_V2);
//            mmd_channel_en(MOD_MBTNAV,MM_DEBUG);
//            mmd_channel_en(MOD_MSOCK,MM_DEBUG);
//            mmd_channel_en(MOD_R7K,MM_DEBUG);
//            mmd_channel_en(MOD_R7KR,MM_DEBUG);
//            break;
//        default:
//            mmd_channel_en(MOD_MBTNAV,MM_DEBUG);
//            break;
//    }
//
//    PMPRINT(MOD_MBTNAV,MM_DEBUG,(stderr,"verbose [%s]\n",(cfg->verbose?"Y":"N")));
//    PMPRINT(MOD_MBTNAV,MM_DEBUG,(stderr,"host    [%s]\n",cfg->host));
//    PMPRINT(MOD_MBTNAV,MM_DEBUG,(stderr,"port    [%d]\n",cfg->port));
//    PMPRINT(MOD_MBTNAV,MM_DEBUG,(stderr,"hbeat   [%d]\n",cfg->hbeat));
//    PMPRINT(MOD_MBTNAV,MM_DEBUG,(stderr,"block   [%s]\n",(cfg->blocking==0?"N":"Y")));
//    PMPRINT(MOD_MBTNAV,MM_DEBUG,(stderr,"cycles  [%d]\n",cfg->cycles));
//    PMPRINT(MOD_MBTNAV,MM_DEBUG,(stderr,"bsize   [%d]\n",cfg->bsize));
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
            fprintf(stderr,"\nsig received[%d]\n",signum);
            g_interrupt=true;
            break;
        default:
            fprintf(stderr,"\ns_termination_handler: sig not handled[%d]\n",signum);
            break;
    }
}
// End function termination_handler

static int s_test_ct_xsend(msock_socket_t *cli, char *msg, int32_t len)
{
    
    int retval=-1;
    if(NULL!=cli){
        
        if( len>0 && NULL!=msg && msock_send(cli,(byte *)msg,len)==len){
            retval=len;
            fprintf(stderr,"client CT xsend OK [%d]\n",len);
        }else{
            fprintf(stderr,"client CT xsend failed\n");
        }
    }
    return retval;
}

static int s_test_ct_send(msock_socket_t *cli)
{
    int retval=-1;
    char *msg_out=NULL;
    
    if(NULL!=cli){
        
        int32_t len = trnw_type_msg(&msg_out, TRN_MSG_PING);
        
        if( len>0 && NULL!=msg_out && msock_send(cli,(byte *)msg_out,len)==len){
            fprintf(stderr,"client CT send OK [%d]\n",len);
        }else{
            fprintf(stderr,"client CT send failed\n");
        }
        free(msg_out);
    }
    return retval;
}

static int s_test_ct_recv(msock_socket_t *cli)
{
    int retval=-1;
    
    if(NULL!=cli){
        int64_t test=0;
        char reply[TRN_MSG_SIZE]={0};
        
        msock_set_blocking(cli,false);
        if( (test=msock_recv(cli,reply,TRN_MSG_SIZE,0))>0){
            wcommst_t *ct = NULL;
            wcommst_unserialize(&ct, reply, TRN_MSG_SIZE);
            char mtype = wcommst_get_msg_type(ct);
            fprintf(stderr,"client CT recv OK len[%lld] msg_type[%c/%02X]:\n",test,mtype,mtype);
            wcommst_show(ct, true, 5);
            wcommst_destroy(ct);
            retval=test;
        }else{
            fprintf(stderr,"client CT recv failed len[%lld][%d/%s]\n",test,errno,strerror(errno));
        }
    }
    return retval;
}

static int s_test_trnmsg_send(msock_socket_t *cli)
{
    int retval=-1;
    
    if(NULL!=cli){
        
        trnmsg_t *msg_out = trnmsg_new_type_msg(TRNIF_PING, 0x1234);
        int32_t len = trnmsg_len(msg_out);
        
        if( len>0 && NULL!=msg_out && msock_send(cli,(byte *)msg_out,len)==len){
            fprintf(stderr,"client TRNMSG send OK [%d]\n",len);
            trnmsg_show(msg_out,true,5);
            retval=len;
        }else{
            fprintf(stderr,"client TRNMSG send failed\n");
        }
        free(msg_out);
    }
    return retval;
}

static int s_test_trnmsg_recv(msock_socket_t *cli)
{
    int retval=-1;
    
    if(NULL!=cli){
        int64_t test=0;
        char reply[TRNIF_MAX_SIZE]={0};
        
        msock_set_blocking(cli,false);
        if( (test=msock_recv(cli,reply,TRNIF_MAX_SIZE,0))>0){
            trnmsg_t *msg_in = NULL;
            int len = trnmsg_deserialize(&msg_in,(byte *)reply,TRNIF_MAX_SIZE);
            if(NULL!=msg_in){
                trnmsg_id_t mtype = msg_in->hdr.msg_id;
                fprintf(stderr,"client TRNMSG recv OK len[%lld] msg_type[%hu/%s]:\n",test,mtype,TRNIF_IDSTR(mtype));
                trnmsg_show(msg_in, true, 5);
                trnmsg_destroy(&msg_in);
                retval=len;
                
            }
        }else{
            fprintf(stderr,"client TRNMSG recv failed len[%lld][%d/%s]\n",test,errno,strerror(errno));
        }
    }
    return retval;
}


static int s_test_ct(app_cfg_t *cfg)
{
    int retval=-1;
    if(NULL!=cfg){
        
        cfg->netif->read_fn   = trnif_msg_read_ct;
        cfg->netif->handle_fn = trnif_msg_handle_ct;
        
        // client: send TRN_MSG_PING
        s_test_ct_send(cfg->cli);
        
        // server: connect to client
        int uc = netif_update_connections(cfg->netif);
        
        // server: get TRN_MSG_PING, return TRN_MSG_ACK
        int sc = netif_reqres(cfg->netif);
        
        // client: get TRN_MSG_ACK
        s_test_ct_recv(cfg->cli);
        
        fprintf(stderr,"%s : BEFORE INIT trn[%p] trn->obj[%p]\n",__FUNCTION__,cfg->trn,wtnav_obj_addr(cfg->trn));
        
        // client: send TRN_MSG_INIT
        char *init_msg=NULL;
        int32_t len = trnw_init_msg(&init_msg, cfg->trn_cfg);

        if(NULL!=init_msg && len>0){
            s_test_ct_xsend(cfg->cli,init_msg,len);
            free(init_msg);
        }
        
        // server: connect to client
        uc = netif_update_connections(cfg->netif);
        
        // server: get TRN_MSG_INIT, return TRN_MSG_ACK
        sc = netif_reqres(cfg->netif);
        
        // client: get TRN_MSG_ACK
        s_test_ct_recv(cfg->cli);
        fprintf(stderr,"%s : AFTER INIT trn[%p] trn->obj[%p]\n",__FUNCTION__,cfg->trn,wtnav_obj_addr(cfg->trn));

        retval = 0;
    }
    return retval;
}

static int s_test_trnmsg(app_cfg_t *cfg)
{
    int retval=-1;
    if(NULL!=cfg){
        // change message handler
        cfg->netif->read_fn   = trnif_msg_read_trnmsg;
        cfg->netif->handle_fn = trnif_msg_handle_trnmsg;
        
        // client: send TRNMSG PING
        s_test_trnmsg_send(cfg->cli);
        
        // server: get MSG_PING, return TRNMSG_ACK
        int sc = netif_reqres(cfg->netif);
        
        // client: get TRNMSG_ACK
        s_test_trnmsg_recv(cfg->cli);
        
        retval=0;
    }
    return retval;
}


static int s_app_main(app_cfg_t *cfg)
{
    int retval=-1;
    double start_time=mtime_dtime();
    netif_t *netif = netif_new(cfg->host,
                               cfg->port,
                               ST_TCP,
                               IFM_REQRES,
                               3.0,
                               NULL,
                               NULL,
                               NULL);
    assert(netif!=NULL);
    
    trn_config_t *trn_cfg = trncfg_new(cfg->host,
                                      cfg->port,
                                      10L,
                                      TRN_MAP_BO,
                                      TRN_FILT_PARTICLE,
                                       cfg->map,
                                       cfg->cfg,
                                       cfg->particles,
                                       cfg->logdir,
//                                      "/home/headley/tmp/maps/PortTiles",
//                                      "/home/headley/tmp/config/mappingAUV_specs.cfg",
//                                      "/home/headley/tmp/config/particles.cfg",
//                                      "logs",
                                      0);
    
    wtnav_t *trn = wtnav_new(trn_cfg);
    
    netif_set_reqres_res(netif,trn);
    
//    mmd_module_configure(&mmd_config_defaults[0]);
    netif_init_mmd();
    netif_show(netif,true,5);

    // initialize message log
    int il = netif_init_log(netif, NETIF_MLOG_NAME, ".");
    mlog_tprintf(netif->mlog_id,"*** netif session start (TEST) ***\n");
    mlog_tprintf(netif->mlog_id,"libnetif v[%s] build[%s]\n",netif_get_version(),netif_get_build());

    // server: open socket, listen
    int nc = netif_connect(netif);
    
    // client: connect
    msock_socket_t *cli = msock_socket_new(NETIF_HOST_DFL,NETIF_PORT_DFL, ST_TCP);
    msock_connect(cli);
 
    // fill in config
    cfg->netif = netif;
    cfg->trn_cfg = trn_cfg;
    cfg->trn = trn;
    cfg->cli = cli;
    
    // test trn_server/commsT protocol
    s_test_ct(cfg);
    // test trnmsg protocol
    s_test_trnmsg(cfg);
        
    // client: force expire, check, prune
    sleep(3);
    int uc = netif_reqres(netif);
    
    mlog_tprintf(netif->mlog_id,"*** netif session end (TEST) uptime[%.3lf] ***\n",(mtime_dtime()-start_time));


    retval=0;
    return retval;
}

int main(int argc, char **argv)
{
    int retval=-1;
    
//    int verbose;
//    netif_t *netif;
//    trn_config_t *trn_cfg;
//    wtnav_t *trn;
//    msock_socket_t *cli;
//    char *host;
//    int port;
//    char *map;
//    char *cfg;
//    char *particles;
//    char *logdir;

    app_cfg_t cfg_s={
        0,
        NULL,
        NULL,
        NULL,
        NULL,
        strdup(NETIF_HOST_DFL),
        NETIF_PORT_DFL,
        NULL,
        NULL,
        NULL,
        strdup("logs")
    }, *cfg=&cfg_s;

    parse_args(argc,argv,cfg);
    s_app_main(cfg);
    
    if(NULL!=cfg){
        if(NULL!=cfg->host)free(cfg->host);
        if(NULL!=cfg->map)free(cfg->map);
        if(NULL!=cfg->cfg)free(cfg->cfg);
        if(NULL!=cfg->particles)free(cfg->particles);
        if(NULL!=cfg->logdir)free(cfg->logdir);
        // client: release socket
        msock_socket_destroy(&cfg->cli);
        // server: close, release netif
        netif_destroy(&cfg->netif);
        // release trn config
        trncfg_destroy(&cfg->trn_cfg);
        // release trn
        wtnav_destroy(cfg->trn);
        // debug: release resources
        mmd_release();

    }
//#ifdef WITH_NETIF_TEST
//    netif_test();
//#else
//    fprintf(stderr,"netif_test not implemented - compile using -DNETIF_WITH_TEST (WITH_NETIF_TEST=1 make...)\r\n");
//#endif
    return retval;
}
