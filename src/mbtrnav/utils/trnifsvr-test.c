///
/// @file trnifsvr-test.c
/// @authors k. Headley
/// @date 12 jun 2019

/// Test server for trnif/trn_server clients

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
    netif_t *netif;
    trn_config_t *trn_cfg;
    wtnav_t *trn;
    msock_socket_t *cli;
}app_cfg_t;

/////////////////////////
// Imports
/////////////////////////

/////////////////////////
// Module Global Variables
/////////////////////////

/////////////////////////
// Function Definitions
/////////////////////////

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
        if( (test=msock_recv(cli,(byte *)reply,TRN_MSG_SIZE,0))>0){
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


static int s_run(app_cfg_t *cfg)
{
    int retval=-1;
    if(NULL!=cfg){

        cfg->netif->read_fn   = trnif_msg_read_ct;
        cfg->netif->handle_fn = trnif_msg_handle_ct;

        while(1){
        // server: connect to client
        int uc = netif_update_connections(cfg->netif);

        // server: get TRN_MSG_PING, return TRN_MSG_ACK
        int sc = netif_reqres(cfg->netif);
            sleep(1);
        }
        retval = 0;
    }
    return retval;
}


static int s_app_main(app_cfg_t *cfg)
{
    int retval=-1;
    double start_time=mtime_dtime();
    netif_t *netif = netif_new("trnsvr","localhost",
                               27027,
                               ST_TCP,
                               IFM_REQRES,
                               0.0,
                               NULL,
                               NULL,
                               NULL);
    assert(netif!=NULL);

    trn_config_t *trn_cfg = trncfg_new(NULL,
                                      -1,
                                      10L,
                                      TRN_MAP_BO,
                                      TRN_FILT_PARTICLE,
                                      "/home/headley/tmp/maps/PortTiles",
                                      "/home/headley/tmp/config/mappingAUV_specs.cfg",
                                      "/home/headley/tmp/config/particles.cfg",
                                      "logs",
                                      0,
                                       TRN_MAX_NCOV_DFL,
                                       TRN_MAX_NERR_DFL,
                                       TRN_MAX_ECOV_DFL,
                                       TRN_MAX_EERR_DFL
                                       );

    wtnav_t *trn = wtnav_new(trn_cfg);

    netif_set_reqres_res(netif,trn);

    netif_init_mmd();
    netif_show(netif,true,5);

    // initialize message log
    int il = netif_init_log(netif, NETIF_MLOG_NAME, NULL);
    mlog_tprintf(netif->mlog_id,"*** netif session start (TEST) ***\n");
    mlog_tprintf(netif->mlog_id,"libnetif v[%s] build[%s]\n",netif_get_version(),netif_get_build());

    // server: open socket, listen
    int nc = netif_connect(netif);

    // fill in config
    cfg->netif = netif;
    cfg->trn_cfg = trn_cfg;
    cfg->trn = trn;
    cfg->cli = NULL;

    // test trn_server/commsT protocol
    s_run(cfg);

    mlog_tprintf(netif->mlog_id,"*** netif session end (TEST) uptime[%.3lf] ***\n",(mtime_dtime()-start_time));

    // server: close, release netif
    netif_destroy(&netif);
    // release trn config
    trncfg_destroy(&trn_cfg);
    // release trn
    wtnav_destroy(trn);
    // debug: release resources
    mmd_release();

    retval=0;
    return retval;
}

int main(int argc, char **argv)
{
    int retval=-1;

    app_cfg_t cfg_s={
        NULL,
        NULL,
        NULL,
        NULL
    }, *cfg=&cfg_s;

    s_app_main(cfg);
//#ifdef WITH_NETIF_TEST
//    netif_test();
//#else
//    fprintf(stderr,"netif_test not implemented - compile using -DNETIF_WITH_TEST (WITH_NETIF_TEST=1 make...)\r\n");
//#endif
    return retval;
}
