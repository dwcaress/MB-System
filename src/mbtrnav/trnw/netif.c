///
/// @file netif.c
/// @authors k. headley
/// @date 2019-06-21

/// TRN net interface API

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

#include "netif.h"
#include "mtime.h"
#include "mutils.h"
#include "medebug.h"
#include "mconfig.h"

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
#define COPYRIGHT "Copyright 2002-YYYY MBARI Monterey Bay Aquarium Research Institute, all rights reserved."
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

/////////////////////////
// Imports
/////////////////////////

/////////////////////////
// Module Global Variables
/////////////////////////

#ifdef NETIF_MMDEBUG
// definitions are used when not
// defined in mconfig.c (i.e. by application)

/// @var char *netif_ch_names[NETIF_CH_COUNT]
/// @brief module channel names
char *netif_ch_names[NETIF_CHAN_COUNT]={
    "trace.netif",
    "debug.netif",
    "warn.netif",
    "err.netif",
    "netif.v1",
    "netif.v2",
    "netif.v3",
    "netif.v4"
};
static mmd_module_config_t mmd_config_defaults[]={
    {MOD_NETIF,"MOD_NETIF",NETIF_CHAN_COUNT,((MM_ERR|MM_WARN)|NETIF_V1|NETIF_V2|NETIF_V3|NETIF_V4),netif_ch_names}
};
#endif // NETIF_MMDEBUG

const char *prof_event_labels[]={ \
    "e_src_socket",
    "e_src_con",
    "e_cli_rx_z",
    "e_cli_rx_e",
    "e_eagain",
    "e_cli_tx_z",
    "e_cli_tx_e",
    "e_pub_tx",
    "e_proto_rd",
    "e_proto_hnd",
    "cli_con",
    "cli_dis",
    "cli_rx",
    "cli_tx",
    "cli_rr",
    "cli_pub"
};

const char *prof_status_labels[]={ \
    "cli_list_len",
    "cli_rx_bytes",
    "cli_tx_bytes",
    "cli_res_bytes",
    "cli_pub_bytes"
};

const char *prof_chan_labels[]={ \
    "udcon_xt",
    "chkhb_xt",
    "read_xt",
    "handle_xt",
    "reqres_xt",
    "pub_xt"
};
#define PROF_LABEL_COUNT 3
const char **prof_stats_labels[PROF_LABEL_COUNT]={
    prof_event_labels,
    prof_status_labels,
    prof_chan_labels
};
mlog_config_t mlog_conf = {
    ML_NOLIMIT, ML_NOLIMIT, ML_NOLIMIT,
    ML_MONO,
    ML_FILE,ML_TFMT_ISO1806
};
mfile_flags_t log_flags = MFILE_RDWR|MFILE_APPEND|MFILE_CREATE;
mfile_mode_t log_mode = MFILE_RU|MFILE_WU|MFILE_RG|MFILE_WG;

double netif_profile_interval_sec=20.0;

/////////////////////////
// Function Definitions
/////////////////////////

/// @fn _Bool s_peer_idval_cmp(void * item, void * value)
/// @brief compare msock_connection_t ID to specified value. Used by mlist.
/// @param[in] item void pointer to msock_connection_t
/// @param[in] value void pointer to integer (id value)
/// @return returns true if the specified peer has the specified ID. false otherwise
static bool s_peer_idval_cmp(void *item, void *value)
{
    bool retval=false;
    if (NULL!=item && NULL!=value) {
        msock_connection_t *peer = (msock_connection_t *)item;
        int svc = *((int *)value);
        PMPRINT(MOD_NETIF,MM_DEBUG,(stderr,"peer[%p] id[%d] svc[%d]\n",peer,peer->id,svc));

//        fprintf(stderr,"%s - peer[%p] id[%d] svc[%d]\n",__FUNCTION__,peer,peer->id,svc);
        retval = (peer->id == svc);
    }else{
        PTRACE();
    }
    return retval;
}
// End function s_peer_vcmp


 int netif_udp_update_connections(netif_t *self)
{
    int retval=-1;

    msock_socket_t *socket =self->socket;
    msock_connection_t *peer =self->peer;
    mlist_t *list = self->list;
    int errsave=0;
    double connect_time=0.0;
    byte buf[NETIF_UDP_BUF_LEN]={0};

    int64_t iobytes=0;

    // clear buffer
    memset(buf,0,NETIF_UDP_BUF_LEN);
    //    PMPRINT(MOD_NETIF,NETIF_V4,(stderr,"[UDPCON.%s]:RX n[%d]\n",self->port_name,netif_connections(self)));

    // read client socket
    iobytes = msock_recvfrom(socket, peer->addr, buf, NETIF_UDP_BUF_LEN,0);
    errsave=errno;

    switch (iobytes) {
        case 0:
            PMPRINT(MOD_NETIF,NETIF_V3,(stderr,"[UDPCON.%s]:ERR - recvfrom ret[0] (no input)\n",self->port_name));
            retval=-1;
            MST_COUNTER_INC(self->profile->stats->events[NETIF_EV_ECLI_RXZ]);

            break;// fall through - OK(?)
        case -1:
            if(errsave!=EAGAIN){
                PMPRINT(MOD_NETIF,NETIF_V2,(stderr,"[UDPCON.%s]:ERR - recvfrom ret[-1] err[%d/%s]\n",self->port_name,errsave,strerror(errsave)));
            }else{
                MST_COUNTER_INC(self->profile->stats->events[NETIF_EV_EAGAIN]);
            }
            MST_COUNTER_INC(self->profile->stats->events[NETIF_EV_ECLI_RXE]);
            break;

        default:

            // record arrival time
            connect_time= mtime_dtime();

            // get host name info from connection
            int svc = msock_connection_addr2str(peer);

            PMPRINT(MOD_NETIF,NETIF_V4,(stderr,"[UDPCON.%s]:RX - ret[%"PRId64"] bytes id[%s:%s]\n",
                                        self->port_name,iobytes, peer->chost, peer->service));

            // update client list
            msock_connection_t *pcon=NULL;


            if( (pcon=mlist_vlookup(list,(void *)&svc,s_peer_idval_cmp))!=NULL){
                //                        fprintf(stderr,"%s - [UDPCON] found sub id[%p/%s:%s]\n",__FUNCTION__,peer,peer->chost, peer->service);
                // update heartbeat if client on list
                pcon->hbtime=connect_time;
            }else{
                PMPRINT(MOD_NETIF,NETIF_V1,(stderr,"[UDPCON.%s]:ADD_SUB - id[%p/%s:%s] idx[%zd]\n",self->port_name,peer,peer->chost, peer->service,mlist_size(list)-1));
                // client doesn't exist
                // initialize and add to list
                peer->id = svc;
                peer->heartbeat = 0;
                peer->hbtime = connect_time;
                peer->next=NULL;
                mlist_add(list, (void *)self->peer);
                // save pointer to finish up (send ACK, update hbeat)
                pcon=self->peer;
                // create a new peer for next read
                self->peer = msock_connection_new();
                mlog_tprintf(self->mlog_id,"[UDPCON.%s]:ADD_SUB - id[%p/%s:%s] n[%zd]\n",self->port_name,peer,peer->chost, peer->service,mlist_size(list));
                MST_COUNTER_INC(self->profile->stats->events[NETIF_EV_CLI_CONN]);
                MST_COUNTER_SET(self->profile->stats->status[NETIF_STA_CLI_LIST_LEN],mlist_size(self->list));
            }
            if ( (NULL!=pcon) && ( iobytes > 0) ) {
                PMPRINT(MOD_NETIF,NETIF_V1,(stderr,"%s - [UDPCON] handle SUB connect message (if any)\n",__FUNCTION__));
                int errout=0;
                // invoke handler (if client sent connect message)
                MST_METRIC_START(self->profile->stats->metrics[NETIF_CH_HANDLE_XT], mtime_dtime());

                int hret=self->handle_fn(buf,self,pcon,&errout);
                if( hret > 0){
                    MST_COUNTER_ADD(self->profile->stats->status[NETIF_STA_CLI_TX_BYTES],hret);
                }

                MST_METRIC_LAP(self->profile->stats->metrics[NETIF_CH_HANDLE_XT], mtime_dtime());
            }

            break;
    }

    return retval;
}

int netif_tcp_update_connections(netif_t *self)
{
    int retval=-1;

    PMPRINT(MOD_NETIF,NETIF_V4,(stderr,"[TCPCON.%s]:ACC\n",self->port_name));

    msock_socket_t *sock_inst =self->socket;
    msock_connection_t *peer =self->peer;
    mlist_t *list = self->list;
    int new_fd=-1;
    int errsave=0;
    double connect_time=0.0;

    msock_set_blocking(sock_inst,false);
    new_fd = msock_accept(sock_inst,peer->addr);
    errsave=errno;
    msock_set_blocking(sock_inst,true);

    switch(new_fd){
        case -1:
            if(errsave!=EAGAIN){
                PMPRINT(MOD_NETIF,NETIF_V4,(stderr,"[TCPCON.%s]:ERR - accept ret[-1] sfd[%d] nfd[%d] err[%d/%s]\n",self->port_name,sock_inst->fd,new_fd,errsave,strerror(errsave)));
            }else{
                MST_COUNTER_INC(self->profile->stats->events[NETIF_EV_EAGAIN]);
            }
            MST_COUNTER_INC(self->profile->stats->events[NETIF_EV_ECLI_RXE]);
               break;
        case 0:
            PMPRINT(MOD_NETIF,MM_ALL,(stderr,"[TCPCON.%s]:ERR - ret[0] (no input) err[%d/%s]\n",self->port_name,errsave,strerror(errsave)));
            MST_COUNTER_INC(self->profile->stats->events[NETIF_EV_ECLI_RXZ]);
           break;

        default:
            connect_time = mtime_dtime();
            PMPRINT(MOD_NETIF,NETIF_V4,(stderr,"[TCPCON.%s]:CONNECTED -  sfd[%d] nfd[%d]\n",self->port_name,sock_inst->fd,new_fd));
            // generate a socket (wrapper) for the client connection
            peer->sock = msock_wrap_fd(new_fd);
            // record connect time

            msock_connection_addr2str(peer);
            peer->hbtime=connect_time;

            if(mmd_channel_isset(MOD_NETIF,(MM_DEBUG))){
                int sndbuf=0;
                int rcvbuf=0;
                socklen_t bsz=sizeof(int);
//                int newsz=16*1024;
                // read TCP buffer sizes
                if(msock_get_opt(peer->sock,SO_SNDBUF,&sndbuf,&bsz)<0){
                    PMPRINT(MOD_NETIF,NETIF_V4,(stderr,"[TCPCON.%s]: ERR getopt SNDBUF failed [%d/%s]\n",self->port_name,errno,strerror(errno)));
                }

                if(msock_get_opt(peer->sock,SO_RCVBUF,&rcvbuf,&bsz)<0){
                    PMPRINT(MOD_NETIF,NETIF_V4,(stderr,"[TCPCON.%s]: ERR getopt RCVBUF failed [%d/%s]\n",self->port_name,errno,strerror(errno)));
                }

                PMPRINT(MOD_NETIF,NETIF_V1,(stderr,"[TCPCON.%s]:ADD_CLI - SNDBUF[%d] RCVBUF[%d]\n",self->port_name,sndbuf,rcvbuf));

//                // resize if too small
//                if(newsz>sndbuf)
//                if(msock_set_opt(peer->sock,SO_SNDBUF,&newsz,bsz)<0){
//                    PMPRINT(MOD_NETIF,NETIF_V4,(stderr,"[TCPCON.%s]: ERR setopt SNDBUF failed [%d/%s]\n",self->port_name,errno,strerror(errno)));
//                }
//
//                if(newsz>sndbuf)
//                if(msock_set_opt(peer->sock,SO_RCVBUF,&newsz,bsz)<0){
//                    PMPRINT(MOD_NETIF,NETIF_V4,(stderr,"[TCPCON.%s]: ERR setopt RCVBUF failed [%d/%s]\n",self->port_name,errno,strerror(errno)));
//                }
//
//                // confirm new size
//                if(msock_get_opt(peer->sock,SO_SNDBUF,&sndbuf,&bsz)<0){
//                    PMPRINT(MOD_NETIF,NETIF_V4,(stderr,"[TCPCON.%s]: ERR getopt SNDBUF failed [%d/%s]\n",self->port_name,errno,strerror(errno)));
//                }
//
//                if(msock_get_opt(peer->sock,SO_RCVBUF,&rcvbuf,&bsz)<0){
//                    PMPRINT(MOD_NETIF,NETIF_V4,(stderr,"[TCPCON.%s]: ERR getopt RCVBUF failed [%d/%s]\n",self->port_name,errno,strerror(errno)));
//                }
//
//                PMPRINT(MOD_NETIF,NETIF_V1,(stderr,"[TCPCON.%s]:ADD_CLI - SNDBUF[%d] RCVBUF[%d]\n",self->port_name,sndbuf,rcvbuf));

            }

            //            PMPRINT(MOD_NETIF,NETIF_V4,(stderr,"[TCPCON.%s]:ADD_CLI - id[%p/%s:%s] fd[%d] sfd[%d] nfd[%d] \n",self->port_name,peer,peer->chost, peer->service,peer->s->fd,sock_inst->fd,new_fd));

            mlist_add(list,(void *)peer);
            self->peer = msock_connection_new();
            PMPRINT(MOD_NETIF,NETIF_V1,(stderr,"[TCPCON.%s]:ADD_CLI - id[%p/%s:%s] fd[%d] idx[%zd]\n",self->port_name,self->peer,peer->chost, peer->service,peer->sock->fd,mlist_size(list)-1));
            mlog_tprintf(self->mlog_id,"[TCPCON.%s]:ADD_CLI - id[%p/%s:%s] n[%zd]\n",self->port_name,self->peer,peer->chost, peer->service,mlist_size(list));
            retval=0;

            MST_COUNTER_INC(self->profile->stats->events[NETIF_EV_CLI_CONN]);
            MST_COUNTER_SET(self->profile->stats->status[NETIF_STA_CLI_LIST_LEN],mlist_size(self->list));
         break;
    }

    return retval;
}

int netif_update_connections(netif_t *self)
{
    int retval = -1;

    if(NULL!=self){
        MST_METRIC_START(self->profile->stats->metrics[NETIF_CH_UDCON_XT], mtime_dtime());
        if(self->ctype==ST_UDP)
            netif_udp_update_connections(self);
        if(self->ctype==ST_TCP)
            netif_tcp_update_connections(self);
        MST_METRIC_LAP(self->profile->stats->metrics[NETIF_CH_UDCON_XT], mtime_dtime());
    }
   return retval;
}

int netif_connections(netif_t *self)
{
    int retval=-1;
    if(NULL!=self && NULL!=self->list){
        retval = mlist_size(self->list);
    }
    return retval;
}

int netif_check_hbeat(netif_t *self, msock_connection_t **ppsub, int idx)
{
    int retval = -1;

    if(NULL!=self && NULL!=ppsub){
        MST_METRIC_START(self->profile->stats->metrics[NETIF_CH_CHKHB_XT], mtime_dtime());
        msock_connection_t *psub = *ppsub;
        double now = mtime_dtime();
        double tmout = self->hbto;

        if (NULL!=psub && tmout>0.0 && ((now-psub->hbtime)>tmout)) {

            PMPRINT(MOD_NETIF,NETIF_V1,(stderr,"[CHKHB.%s]:DEL_CLI - expired id[%d/%s:%s] - removed\n",self->port_name,idx,psub->chost, psub->service));
            mlog_tprintf(self->mlog_id,"[CHKHB.%s]:DEL_CLI - expired id[%d/%s:%s] - removed\n",self->port_name,idx,psub->chost, psub->service);

            mlist_remove(self->list,psub);
            *ppsub=NULL;

            MST_COUNTER_INC(self->profile->stats->events[NETIF_EV_CLI_DISN]);
            MST_COUNTER_SET(self->profile->stats->status[NETIF_STA_CLI_LIST_LEN],mlist_size(self->list));

        }else{
            if(NULL!=psub)
            PMPRINT(MOD_NETIF,NETIF_V1,(stderr,"[CHKHB.%s]:OK id[%d/%s:%s] - %.3lf/%.3lf/%.3lf %lf\n",self->port_name,idx,psub->chost, psub->service, now,psub->hbtime,(now-psub->hbtime),tmout));
        }
        retval=0;
        MST_METRIC_LAP(self->profile->stats->metrics[NETIF_CH_CHKHB_XT], mtime_dtime());
    }

    return retval;
}

int netif_reqres(netif_t *self)
{
    int retval=-1;


    if(NULL!=self && NULL!=self->read_fn && NULL!=self->handle_fn){
        MST_COUNTER_INC(self->profile->stats->events[NETIF_EV_CLI_REQRESN]);
        MST_METRIC_START(self->profile->stats->metrics[NETIF_CH_REQRES_XT], mtime_dtime());

        // iterate over data providers...
        msock_connection_t *psub = (msock_connection_t *)mlist_first(self->list);
        int cli=0;
        byte *pmsg=NULL;
        uint32_t msg_len=0;
        int merr=0;

        while (psub != NULL) {
           cli++;

            // read message
            // update hbeat
            // handle message
            // send reply

            msock_set_blocking(psub->sock, false);

            MST_METRIC_START(self->profile->stats->metrics[NETIF_CH_READ_XT], mtime_dtime());
            int iobytes=self->read_fn(&pmsg,&msg_len,self,psub, &merr);
            MST_METRIC_LAP(self->profile->stats->metrics[NETIF_CH_READ_XT], mtime_dtime());


            if (  (iobytes > 0) && (self->hbto > 0.0) ) {
                PMPRINT(MOD_NETIF,NETIF_V2,(stderr,"[SVCCLI.%s]:RX - bytes[%d] id[%d/%s:%s] hb[%.2lf]\n", self->port_name,iobytes, cli, psub->chost, psub->service, psub->hbtime));

                // update connection hbeat time
                psub->hbtime=mtime_dtime();

                MST_COUNTER_ADD(self->profile->stats->status[NETIF_STA_CLI_RX_BYTES],iobytes);
                MST_COUNTER_INC(self->profile->stats->events[NETIF_EV_CLI_TXN]);

            }else{
                if(errno!=EAGAIN)
                PMPRINT(MOD_NETIF,NETIF_V4,(stderr,"[SVCCLI.%s]:ERR - recvfrom ret[%d] id[%d/%s:%s] err[%d/%s]\n",self->port_name,iobytes,cli,psub->chost, psub->service,errno,strerror(errno)));
           }

            // check hbeat, remove expired connections
            // (ignored if hbto <= 0)
            netif_check_hbeat(self, &psub, cli);

            // handle message (if received message and not expired)
            if(NULL!=psub && iobytes>0){
                int errout=0;
                MST_METRIC_START(self->profile->stats->metrics[NETIF_CH_HANDLE_XT], mtime_dtime());

                if( (iobytes=self->handle_fn(pmsg,self,psub,&errout))<=0){
                    switch (errout) {
                        case EPIPE:
                            PMPRINT(MOD_NETIF,NETIF_V1,(stderr,"[SVCCLI.%s]:DEL_CLI - send err (EPIPE) id[%d/%s:%s] err[%d/%s]\n",self->port_name,cli,psub->chost, psub->service,errno,strerror(errno)));
                            mlog_tprintf(self->mlog_id,"[SVCCLI.%s]:DEL_CLI - send err (EPIPE) id[%d/%s:%s]\n",self->port_name,cli,psub->chost, psub->service);

                            mlist_remove(self->list,psub);
                            MST_COUNTER_INC(self->profile->stats->events[NETIF_EV_CLI_DISN]);
                            MST_COUNTER_SET(self->profile->stats->status[NETIF_STA_CLI_LIST_LEN],mlist_size(self->list));
                            psub=NULL;
                            break;
                        default:
                            PMPRINT(MOD_NETIF,NETIF_V4,(stderr,"[SVCCLI.%s]:ERR - send id[%d/%s:%s] err[%d/%s]\n",self->port_name,cli,psub->chost, psub->service,errno,strerror(errno)));
                            break;
                    }
                }else{
                    MST_COUNTER_ADD(self->profile->stats->status[NETIF_STA_CLI_TX_BYTES],iobytes);
                    MST_COUNTER_ADD(self->profile->stats->status[NETIF_STA_CLI_RES_BYTES],iobytes);
                    MST_COUNTER_INC(self->profile->stats->events[NETIF_EV_CLI_RXN]);
              }// else handle msg OK

                MST_METRIC_LAP(self->profile->stats->metrics[NETIF_CH_HANDLE_XT], mtime_dtime());

            }

            if(NULL!=pmsg){
                free(pmsg);
                pmsg=NULL;
            }
            psub=(msock_connection_t *)mlist_next(self->list);

        }// while psub


        retval=0;
        MST_METRIC_LAP(self->profile->stats->metrics[NETIF_CH_REQRES_XT], mtime_dtime());
    }
   return retval;
}
// End function

int netif_pub(netif_t *self, char *output_buffer, size_t len)
{
    int retval=-1;

    if(NULL!=self && NULL!=self->pub_fn && NULL!=output_buffer && len>0){
        MST_METRIC_START(self->profile->stats->metrics[NETIF_CH_PUB_XT], mtime_dtime());

        // iterate over subscribers...
        msock_connection_t *psub = (msock_connection_t *)mlist_first(self->list);
        int idx=0;

        while (psub != NULL) {
            int iobytes = self->pub_fn(self,psub,output_buffer,len);

            if (  iobytes > 0) {
                // update stats if publish successful
                MST_COUNTER_INC(self->profile->stats->events[NETIF_EV_CLI_TXN]);
                MST_COUNTER_ADD(self->profile->stats->status[NETIF_STA_CLI_TX_BYTES],iobytes);
                MST_COUNTER_INC(self->profile->stats->events[NETIF_EV_CLI_PUBN]);
                MST_COUNTER_ADD(self->profile->stats->status[NETIF_STA_CLI_PUB_BYTES],iobytes);

                PMPRINT(MOD_NETIF,NETIF_V2,(stderr,"[SVCPUB.%s]:TX - ret[%5d] bytes id[%d/%s:%s] hbtime[%.2lf]\n", self->port_name,iobytes, idx, psub->chost, psub->service, psub->hbtime));

            }else{
                PMPRINT(MOD_NETIF,NETIF_V4,(stderr,"\n[SVCPUB.%s]:ERR - sendto ret[%d] id[%d/%s:%s] [%d/%s]\n",self->port_name,iobytes,idx,psub->chost, psub->service,errno,strerror(errno)));
//                mlog_tprintf(self->mlog_id,"[PUB.%s]:ERR - sendto ret[%d] id[%d/%s:%s] [%d/%s]\n",self->port_name,iobytes,idx,psub->chost, psub->service,errno,strerror(errno));
                MST_COUNTER_INC(self->profile->stats->events[NETIF_EV_EPUB_TX]);
            }

            // check hbeat, remove expired connections
            // (ignored if hbto <= 0)
            netif_check_hbeat(self, &psub, idx);

            psub=(msock_connection_t *)mlist_next(self->list);
            idx++;
        }// while psub
        retval=0;
        MST_METRIC_LAP(self->profile->stats->metrics[NETIF_CH_PUB_XT], mtime_dtime());
    }

    return retval;
}
// End function



static int s_netif_run(netif_t *self, uint32_t delay_msec)
{
    int retval =-1;

    if(NULL!=self){
        while(!self->stop){

            netif_update_connections(self);
            netif_reqres(self);
            mtime_delay_ms(delay_msec);
        }
    }// else invalid arg
    return retval;
}
// End function

int netif_connect(netif_t *self)
{
    int retval =-1;
    int test=-1;

    switch(self->ctype){
    case ST_UDP:
        self->socket = msock_socket_new(self->host, self->port, ST_UDP);
            if(NULL!=self->socket){
                const int optionval = 1;
#if !defined(__CYGWIN__)
                msock_set_opt(self->socket, SO_REUSEPORT, &optionval, sizeof(optionval));
#endif
                msock_set_opt(self->socket, SO_REUSEADDR, &optionval, sizeof(optionval));
                msock_set_blocking(self->socket,false);

                if ( (test=msock_bind(self->socket))==0) {
                    PMPRINT(MOD_NETIF,MM_DEBUG,(stderr,"TRN udp socket bind OK [%s:%d]\n",self->host,self->port));
                    retval=0;
                }else{
                    fprintf(stderr, "\nTRN udp socket bind failed [%d] [%d %s]\n",test,errno,strerror(errno));
                    MST_COUNTER_INC(self->profile->stats->events[NETIF_EV_ESRC_CON]);
                }
            }else{
                MST_COUNTER_INC(self->profile->stats->events[NETIF_EV_ESRC_SOCKET]);
            }
        break;
    case ST_TCP:
        self->socket = msock_socket_new(self->host, self->port, ST_TCP);
            if(NULL!=self->socket){
                msock_set_blocking(self->socket,false);
                const int optionval = 1;
#if !defined(__CYGWIN__)
                msock_set_opt(self->socket, SO_REUSEPORT, &optionval, sizeof(optionval));
#endif
                msock_set_opt(self->socket, SO_REUSEADDR, &optionval, sizeof(optionval));
                if ( (test=msock_bind(self->socket))==0) {
                    PMPRINT(MOD_NETIF,MM_DEBUG,(stderr,"TRN tcp socket bind OK [%s:%d]\n",self->host,self->port));
                    if ( (test=msock_listen(self->socket,NETIF_QUEUE_DFL))==0) {
                        PMPRINT(MOD_NETIF,MM_DEBUG,(stderr,"TRN tcp socket listen OK [%s:%d]\n",self->host,self->port));
                        retval=0;
                    }
                }else{
                    fprintf(stderr, "\nTRN tcp socket bind failed [%d] [%d %s]\n",test,errno,strerror(errno));
                    MST_COUNTER_INC(self->profile->stats->events[NETIF_EV_ESRC_CON]);
                }
            }else{
                MST_COUNTER_INC(self->profile->stats->events[NETIF_EV_ESRC_SOCKET]);
            }
        break;
    default:
        break;
    }

    return retval;
}
// End function


/// @fn const char *netif_get_version()
/// @brief get version string.
/// @return version string
const char *netif_get_version()
{
    return LIBNETIF_VERSION;
}
// End function mbtrn_get_version

/// @fn const char *netif_get_build()
/// @brief get build string.
/// @return version string
const char *netif_get_build()
{
    return LIBNETIF_BUILD;
}
// End function mbtrn_get_build

netif_t *netif_new(char *name, char *host, int port,
                   msock_socket_ctype ctype,
                   netif_mode_t mode,
                   double hbto,
                   netif_msg_read_fn read_fn,
                   netif_msg_handle_fn handle_fn,
                   netif_msg_pub_fn pub_fn)
{
    netif_t *instance =  (netif_t *)malloc(sizeof(netif_t));
	if(NULL!=instance){
        memset(instance,0,sizeof(netif_t));

        instance->host=(NULL==host?NULL:strdup(host));
        instance->port=port;
        instance->socket = NULL;
        instance->list = mlist_new();
        instance->peer = msock_connection_new();
        instance->hbto=hbto;
        instance->mode=mode;
        instance->port_name=strdup((NULL!=name?name:"?"));

        mlist_autofree(instance->list,msock_connection_free);

        instance->profile = mstats_profile_new(NETIF_EV_COUNT, NETIF_STA_COUNT, NETIF_CH_COUNT, prof_stats_labels, mtime_dtime(), netif_profile_interval_sec);
  		instance->mlog_id = MLOG_ID_INVALID;
    	instance->log_dir=strdup(NETIF_LOG_DIR_DFL);
     	instance->cmdline=NULL;
        instance->ctype = ctype;
        instance->stop=false;
        instance->read_fn=read_fn;
        instance->handle_fn=handle_fn;
        instance->pub_fn=pub_fn;
    }
	return instance;
}
// End function

netif_t *netif_tcp_new(char *name, char *host, int port,
                       double hbto,
                       netif_mode_t mode,
                       netif_msg_read_fn reader,
                       netif_msg_handle_fn handler)
{
    netif_t *instance = netif_new(name,host,port,ST_TCP,hbto,mode,reader,handler,NULL);
    return instance;
}
// End function

netif_t *netif_udp_new(char *name, char *host, int port,
                       double hbto,
                       netif_mode_t mode,
                       netif_msg_read_fn reader,
                       netif_msg_handle_fn handler)
{
    netif_t *instance = netif_new(name, host,port,ST_UDP,hbto,mode,reader,handler,NULL);
    return instance;
}
// End function


void netif_destroy(netif_t **pself)
{
	if(NULL!=pself){
		netif_t *self = (netif_t *) *pself;
  		if(NULL!=self){
            if(NULL!=self->socket){
                msock_socket_destroy(&self->socket);
            }
            if(NULL!=self->peer){
            msock_connection_destroy(&self->peer);
            }
            if(NULL!=self->list){
                mlist_destroy(&self->list);
            }
            if(NULL!=self->profile){
            	mstats_profile_destroy(&self->profile);
            }
            if(self->mlog_id!=MLOG_ID_INVALID){
                mlog_close(self->mlog_id);
                 mlog_delete_instance(self->mlog_id);
            }
            if(NULL!=self->mlog_path){
            	free(self->mlog_path);
            }
            if(NULL!=self->log_dir){
                free(self->log_dir);
            }
            if(NULL!=self->cmdline){
                free(self->cmdline);
            }
            if(NULL!=self->host){
                free(self->host);
            }
            if(NULL!=self->port_name){
                free(self->port_name);
            }
            free(self);
            *pself=NULL;
        }
    }
	return;
}
// End function

void netif_show(netif_t *self, bool verbose, int indent)
{
    if (NULL != self) {
        int wkey=16;
        int wval=16;
        fprintf(stderr,"%*s%*s  %*p\n",indent,(indent>0?" ":""),wkey,"self",wval,self);
        fprintf(stderr,"%*s%*s  %*s\n",indent,(indent>0?" ":""),wkey,"port_name",wval,self->port_name);
        fprintf(stderr,"%*s%*s  %*s\n",indent,(indent>0?" ":""),wkey,"host",wval,self->host);
        fprintf(stderr,"%*s%*s  %*d\n",indent,(indent>0?" ":""),wkey,"port",wval,self->port);
        fprintf(stderr,"%*s%*s  %*p\n",indent,(indent>0?" ":""),wkey,"socket",wval,self->socket);
        fprintf(stderr,"%*s%*s  %*p\n",indent,(indent>0?" ":""),wkey,"peer",wval,self->peer);
        fprintf(stderr,"%*s%*s  %*p\n",indent,(indent>0?" ":""), wkey,"list@",wval,self->list);
        fprintf(stderr,"%*s%*s  %*lu\n",indent,(indent>0?" ":""),wkey,"list len",wval,mlist_size(self->list));
        fprintf(stderr,"%*s%*s  %*p\n",indent,(indent>0?" ":""),wkey,"profile",wval,self->profile);
        fprintf(stderr,"%*s%*s  %*d\n",indent,(indent>0?" ":""),wkey,"mlog_id",wval,self->mlog_id);
        fprintf(stderr,"%*s%*s  %*s\n",indent,(indent>0?" ":""),wkey,"mlog_path",wval,self->mlog_path);
        fprintf(stderr,"%*s%*s  %*s\n",indent,(indent>0?" ":""),wkey,"log_dir",wval,self->log_dir);
        fprintf(stderr,"%*s%*s  %*.3lf\n",indent,(indent>0?" ":""),wkey,"hbto",wval,self->hbto);
        fprintf(stderr,"%*s%*s  %*s\n",indent,(indent>0?" ":""),wkey,"cmdline",wval,self->cmdline);
        fprintf(stderr,"%*s%*s  %*d\n",indent,(indent>0?" ":""),wkey,"stop",wval,self->stop?1:0);
    }
}
// End function netif_show


int netif_init_log(netif_t *self, char *log_name, char *log_dir, char *session_str)
{
    int retval=-1;
    if(NULL!=self && NULL!=log_name){
        char session_date[32]={0};

        time_t rawtime;

        // remove existing log configuration
        if(self->mlog_id!=MLOG_ID_INVALID){
            mlog_close(self->mlog_id);
            mlog_delete_instance(self->mlog_id);
            if(NULL!=self->mlog_path){
                free(self->mlog_path);
                self->mlog_path=NULL;
            }
        }

        if(NULL!=log_dir){
            if(NULL!=self->log_dir){
                free(self->log_dir);
                self->log_dir=NULL;
            }
            self->log_dir=strdup(log_dir);
        }

        if(NULL!=session_str){
            sprintf(session_date,"%s",session_str);
        }else{
        // make session time string to use
        // in log file names
        time(&rawtime);
        // Get GMT time
        struct tm *gmt = gmtime(&rawtime);
        // format YYYYMMDD-HHMMSS
        sprintf(session_date, "%04d%02d%02d-%02d%02d%02d",
                (gmt->tm_year+1900),gmt->tm_mon+1,gmt->tm_mday,
                gmt->tm_hour,gmt->tm_min,gmt->tm_sec);

        }
        self->mlog_path = (char *)malloc(NETIF_LOG_PATH_BYTES);

        sprintf(self->mlog_path,"%s//%s-%s%s",self->log_dir,log_name,session_date,NETIF_LOG_EXT);

        self->mlog_id = mlog_get_instance(self->mlog_path,&mlog_conf, log_name);
        if(self->mlog_id!=MLOG_ID_INVALID){
        	retval=mlog_open(self->mlog_id, log_flags, log_mode);
        }
    }
    return retval;
}
// End function

int netif_start(netif_t *self, uint32_t delay_msec)
{
    int retval=-1;
    if(NULL!=self && NULL!=self->host){
        if(self->mlog_id==MLOG_ID_INVALID){
        	netif_init_log(self,NETIF_MLOG_NAME,NULL,NULL);
        }

        mlog_tprintf(self->mlog_id,"*** netif session start ***\n");
        //    mlog_tprintf(self->mlog_id,"cmdline [%s]\n",g_cmd_line);
        mlog_tprintf(self->mlog_id,"libnetif v[%s] build[%s]\n",netif_get_version(),netif_get_build());
        int test=-1;
        if( (test=netif_connect(self))==0){
            // enter main loop
            s_netif_run(self,delay_msec);
        }else{
            mlog_tprintf(self->mlog_id,"connect failed[%d]\n",test);
            PEPRINT((stderr,"connect failed[%d]\n",test));
        }
        mlog_tprintf(self->mlog_id,"*** netif session end ***\n");

    }

    return retval;
}
// End function

int netif_restart(netif_t *self)
{
    int retval=-1;
    mlog_tprintf(self->mlog_id,"session restart called\n");
    return retval;
}
// End function

int netif_stop(netif_t *self,int sig)
{
    int retval=-1;
    if(NULL!=self){
        mlog_tprintf(self->mlog_id,"session stop called sig[%d/%s]\n",sig,strsignal(sig));
       self->stop=true;
        retval=0;
    }
    return retval;
}
// End function

int netif_configure_debug(netif_t *self, int level)
{
    int retval=0;

    switch (level) {
        case 0:
            mmd_channel_dis(MOD_NETIF,MM_ALL);
            break;
        case 1:
            mmd_channel_en(MOD_NETIF,NETIF_V1);
            break;
        case 2:
            mmd_channel_en(MOD_NETIF,NETIF_V1|NETIF_V2);
            break;
        case 3:
            mmd_channel_en(MOD_NETIF,NETIF_V1|NETIF_V2|NETIF_V3);
            break;
        case 4:
            mmd_channel_en(MOD_NETIF,NETIF_V1|NETIF_V2|NETIF_V3|NETIF_V4);
            break;

        default:
            mmd_channel_en(MOD_NETIF,(MM_ERR|MM_WARN)|NETIF_V1);
            break;
    }

    return retval;
}
// End function

void netif_set_reqres_res(netif_t *self, void *res)
{
    self->rr_res=res;
}
void netif_set_pub_res(netif_t *self, void *res)
{
    self->pub_res=res;
}

void netif_init_mmd()
{
#ifdef NETIF_MMDEBUG
    mmd_module_configure(&mmd_config_defaults[0]);
#else
    mconf_init(NULL,NULL);
#endif
}

mstats_t *netif_stats(netif_t *self)
{
    mstats_t *retval=NULL;
    if(NULL!=self && NULL!=self->profile){
        retval = self->profile->stats;
    }
    return retval;
}
mlog_id_t netif_log(netif_t *self)
{
    mlog_id_t retval=MLOG_ID_INVALID;
    if(NULL!=self ){
        retval = self->mlog_id;
    }
    return retval;
}

#ifdef WITH_NETIF_TEST
//#include "trn_msg.h"
#include "trnw.h"
//#include "trnif_proto.h"

static int s_netif_pub_msg(netif_t *self, msock_connection_t *peer, char *data, size_t len)
{
    int retval=-1;

    if(NULL!=self && NULL!=peer && NULL!=data && len>0){
        if(self->ctype==ST_UDP){
            int flags=0;
#if !defined(__APPLE__)
            flags=MSG_NOSIGNAL;
#endif
            int64_t iobytes = msock_sendto(self->socket, peer->addr, (byte *) data, len, flags );
            if (  iobytes > 0) {
                fprintf(stderr,"client PUB UDP OK len[%"PRId64"]:\n",iobytes);
            }else{
                fprintf(stderr,"client PUB UDP ERR len[%"PRId64"][%d/%s]\n",iobytes,errno,strerror(errno));
            }
        }
        if(self->ctype==ST_TCP){
            int64_t iobytes = msock_send(peer->sock, (byte *)data, len );
            if ( iobytes > 0) {
                fprintf(stderr,"client PUB TCP OK len[%"PRId64"]:\n",iobytes);
            }else{
                fprintf(stderr,"client PUB TCP ERR len[%"PRId64"][%d/%s]\n",iobytes,errno,strerror(errno));
            }
        }
        retval=0;
    }else{
        fprintf(stderr,"%s - invalid args\n",__FUNCTION__);
    }
    return retval;

}
// End function

static int s_test_pub_recv(msock_socket_t *cli)
{
    int retval=-1;

    if(NULL!=cli){
        int64_t test=0;
        char reply[TRN_MSG_SIZE]={0};

        msock_set_blocking(cli,false);
        if( (test=msock_recv(cli,(byte *)reply,TRN_MSG_SIZE,0))>0){
            fprintf(stderr,"client PUB recv OK len[%"PRId64"]:\n",test);
            mfu_hex_show((byte *)reply,test,16,true,5);
            retval=test;
        }else{
            fprintf(stderr,"client PUB recv ERR len[%"PRId64"][%d/%s]\n",test,errno,strerror(errno));
        }
    }
    return retval;
}

#define NETIF_TEST_MSG_BYTES 32

int s_netif_test_read(byte **pdest, uint32_t *len, netif_t *self, msock_connection_t *peer, int *errout)
{
    int retval=-1;
    if(NULL!=pdest && NULL!=self && NULL!=peer){
        int64_t msg_bytes=0;
        uint32_t readlen=NETIF_TEST_MSG_BYTES;
        byte *buf=*pdest;
        if(NULL==buf){
            buf=(byte *)malloc(NETIF_TEST_MSG_BYTES);
            memset(buf,0,NETIF_TEST_MSG_BYTES);
            *pdest=buf;
        }
        if( (msg_bytes=msock_recvfrom(peer->sock, peer->addr,buf,readlen,0)) >0 ){
            PDPRINT((stderr,"%s: READ - msg_bytes[%"PRId64"]\n",__FUNCTION__,msg_bytes));
            retval=msg_bytes;
        }
    }
    return retval;
}

int s_netif_test_handle(void *msg, netif_t *self, msock_connection_t *peer, int *errout)
{
    int retval=-1;
    if(NULL!=msg && NULL!=self && NULL!=peer){
        char *msg_in = (char *)msg;
        char *msg_out=NULL;
        if(strcmp(msg_in,"PING")==0){
            msg_out=strdup("ACK");
        }else{
            msg_out=strdup("NACK");
        }
        if(msock_send(peer->sock,(byte *)msg_out,strlen(msg_out)+1)){
	        PDPRINT((stderr,"%s: PING - ACK/NACK OK [%s]\n",__FUNCTION__,msg_out));
        }else{
            PDPRINT((stderr,"%s: PING - ACK/NACK ERR [%s] [%d/%s]\n",__FUNCTION__,msg_out,errno,strerror(errno)));
        }

        free(msg_out);
    }
    return retval;
}
static int s_netif_test_send(msock_socket_t *cli)
{
    int retval=-1;

    if(NULL!=cli){
        char *msg_out=strdup("PING");
        if(NULL!=msg_out){
            size_t len=strlen(msg_out)+1;
            if( len>0 && msock_send(cli,(byte *)msg_out,len)==len){
                fprintf(stderr,"client REQ send OK [%s/%zu]\n",msg_out,len);
                retval=0;
            }else{
                fprintf(stderr,"client REQ send failed\n");
            }
            free(msg_out);
        }
    }
    return retval;
}

static int s_netif_test_recv(msock_socket_t *cli)
{
    int retval=-1;

    if(NULL!=cli){
        int64_t test=0;
        char reply[16]={0};

        msock_set_blocking(cli,false);
        if( (test=msock_recv(cli,(byte *)reply,16,0))>0){
            if(test==4 && strcmp(reply,"ACK")==0){
                fprintf(stderr,"client ACK recv OK len[%s/%"PRId64"]\n",reply,test);
                retval=0;
            }else if(test==5 && strcmp(reply,"NACK")==0){
                fprintf(stderr,"client NACK recv OK len[%s/%"PRId64"]\n",reply,test);
            }else{
                fprintf(stderr,"client ACK/NACK recv INVALID len[%s/%"PRId64"]\n",reply,test);
            }
        }else{
            fprintf(stderr,"client ACK recv ERR len[%"PRId64"][%d/%s]\n",test,errno,strerror(errno));
        }
    }
    return retval;
}

int netif_test()
{
    int retval=-1;
    double start_time=mtime_dtime();
    netif_t *netif = netif_new("test",NETIF_HOST_DFL,
                               NETIF_PORT_DFL,
                               ST_TCP,
                               IFM_REQRES,
                               3.0,
                               NULL,
                               NULL,
                               NULL);
    assert(netif!=NULL);

    // create un-configured trn
    wtnav_t *trn = wtnav_dnew();

    assert(trn!=NULL);

    netif_init_mmd();
    netif_set_reqres_res(netif,trn);
    // initialize message log
    int il = netif_init_log(netif, NETIF_MLOG_NAME, NULL,NULL);
    if(il!=0){
        fprintf(stderr,"ERR - netif_init_log returned[%d]\n",il);
    }

    mlog_tprintf(netif->mlog_id,"*** netif session start (TEST) ***\n");
    mlog_tprintf(netif->mlog_id,"libnetif v[%s] build[%s]\n",netif_get_version(),netif_get_build());

    // manually cycle through operations

    // server: open socket, listen
    int nc = netif_connect(netif);

    if(nc!=0){
        fprintf(stderr,"ERR - netif_connect returned[%d]\n",nc);
    }

    netif_show(netif,true,5);

    // client: connect
    msock_socket_t *cli = msock_socket_new(NETIF_HOST_DFL,NETIF_PORT_DFL, ST_TCP);
    msock_connect(cli);

    // server: register new connection(s)
    int uc = netif_update_connections(netif);
    if(uc!=0){
        fprintf(stderr,"ERR - netif_update_connections returned[%d]\n",uc);
    }

    // change message handler
    netif->read_fn   = s_netif_test_read;
    netif->handle_fn = s_netif_test_handle;
    netif->pub_fn    = s_netif_pub_msg;

    // client: send PING
    s_netif_test_send(cli);

    // server: get PING, return ACK/NACK
    int sc = netif_reqres(netif);
    if(sc!=0){
        fprintf(stderr,"ERR - netif_reqres returned[%d]\n",sc);
    }

    // client: get ACK/NACK
    s_netif_test_recv(cli);

    // server: publish data
    char obuf[]="MB1";
    int sp = netif_pub(netif, obuf, 4);
    if(sp!=0){
        fprintf(stderr,"ERR - netif_pub returned[%d]\n",sp);
    }

    // client: get pub data
    s_test_pub_recv(cli);

    // client: force expire, check, prune
    sleep(3);

    uc = netif_reqres(netif);

    // client: release socket
    msock_socket_destroy(&cli);

    mlog_tprintf(netif->mlog_id,"*** netif session end (TEST) uptime[%.3lf] ***\n",(mtime_dtime()-start_time));
    // server: close, release netif
    netif_destroy(&netif);
    // release trn
    wtnav_destroy(trn);
    // debug: release resources
    mmd_release();

    retval=0;
    return retval;
}
// End function
#endif //WITH_NETIF_TEST
