///
/// @file trnif_proto.c
/// @authors k. headley
/// @date 18 oct 2019

/// TRN netif protocol
/// trnif read, handle, pub

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
#include "trnif_proto.h"
#include "mutils.h"
#include "medebug.h"
#include "mmdebug.h"
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
#define COPYRIGHT "Copyright 2002-2019 MBARI Monterey Bay Aquarium Research Institute, all rights reserved."
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

/////////////////////////
// Function Definitions
/////////////////////////

static uint32_t s_trnif_dfl_send_tcp(msock_connection_t *peer, char *msg, int32_t send_len, int *errout)
{
    uint32_t retval=0;
    if(NULL!=peer && NULL!=msg && send_len>0){
        int64_t send_bytes=0;
        int flags=0;
#if !defined(__APPLE__)
        flags=MSG_NOSIGNAL;
#endif
        if( (send_bytes=msock_sendto(peer->sock,  peer->addr, (byte *)msg, send_len,flags))==send_len){
            retval=send_bytes;
            PDPRINT((stderr,"Reply OK len[%d] peer[%s:%s]\n",send_len, peer->chost,peer->service));
        }else{
            if(NULL!=errout){
                *errout=errno;
            }
            PDPRINT((stderr,"Reply ERR peer[%s:%s] sock[%p/%p] len[%d] err[%d/%s]\n",peer->chost,peer->service,peer->sock,  peer->addr, send_len,errno,strerror(errno)));
        }
    }
    return retval;
}

static int s_trnif_msg_read_dfl(byte *dest, uint32_t readlen, msock_socket_t *socket, msock_connection_t *peer, int *errout)
{
    int retval = 0;

    if(NULL!=dest && NULL!=socket && NULL!=peer){

        int64_t msg_bytes=0;
        PDPRINT((stderr,"%s: READ - readlen[%d]\n",__FUNCTION__,readlen));
        if( (msg_bytes=msock_recvfrom(socket, peer->addr,dest,readlen,0)) >0 ){
            retval = msg_bytes;
            PDPRINT((stderr,"%s: READ - OK read[%"PRId64"]\n",__FUNCTION__,msg_bytes));
        }else{
            if(errno!=EAGAIN)
            PDPRINT((stderr,"%s: READ - ERR read[%"PRId64"] [%d/%s]\n",__FUNCTION__,msg_bytes,errno,strerror(errno)));
        }
    }
    return retval;
}

static uint32_t s_trnif_dfl_send_udp(netif_t *self,msock_connection_t *peer, char *msg, int32_t send_len, int *errout)
{
    uint32_t retval=0;
    if(NULL!=peer && NULL!=msg && send_len>0){
        int64_t send_bytes=0;
        int flags=0;
#if !defined(__APPLE__)
        flags=MSG_NOSIGNAL;
#endif
        if( (send_bytes=msock_sendto(self->socket,  peer->addr, (byte *)msg, send_len,flags))==send_len){
            retval=send_bytes;
            PDPRINT((stderr,"Reply OK len[%d] peer[%s:%s]\n",send_len, peer->chost,peer->service));
        }else{
            if(NULL!=errout){
                *errout=errno;
            }
            PDPRINT((stderr,"Reply ERR peer[%s:%s] sock[%p/%p] len[%d] err[%d/%s]\n",peer->chost,peer->service,peer->sock,  peer->addr, send_len,errno,strerror(errno)));
        }
    }
    return retval;
}

/// @fn int trnif_msg_read_trnmsg(byte **dest, uint32_t len, netif_t *self, msock_connection_t *peer, int *errout)
/// @brief read a message from a peer
/// @return message len on success, 0 otherwise

int trnif_msg_read_trnmsg(byte **pdest, uint32_t *len, netif_t *self, msock_connection_t *peer, int *errout)
{
    int retval = 0;

    enum State {ST_START=0,ST_SYNC_OK,ST_HDR_OK,ST_DATA_OK, ST_CHK_OK, ST_SYNC, ST_QUIT };
    enum State state=ST_SYNC;
    enum Action {AC_NOP=0,AC_SYNC, AC_HDR, AC_DATA, AC_CHK, AC_ERR };
    enum Action action = AC_NOP;

    if(NULL!=pdest && NULL!=self && NULL!=peer){
        byte *buf=*pdest;
        if(NULL==buf){
            buf=(byte *)malloc(TRNIF_MAX_SIZE);
        }
        if(NULL!=buf){
            memset(buf,0,TRNIF_MAX_SIZE);
            uint32_t msg_bytes=0;
            uint32_t test=0;
            uint32_t readlen=0;
#if defined(WITH_PDEBUG)
            char *stateNames[]={"ST_START","ST_SYNC_OK","ST_HDR_OK","ST_DATA_OK", "ST_CHK_OK", "ST_SYNC", "ST_QUIT"};
#endif
         byte *cur = buf;
            int err=0;
            trnmsg_header_t *pheader=(trnmsg_header_t *)buf;
            if(NULL!=errout){
                *errout=MSG_EOK;
            }

            while(msg_bytes<*len && state!=ST_QUIT && !self->stop){
                switch (state) {
                    case ST_SYNC:
                        // read the sync pattern
                        msg_bytes=0;
                        cur=buf;
                        action = AC_SYNC;
                        break;
                    case ST_SYNC_OK:
                        // read the header
                        // (cur points to correct location)
                        action=AC_HDR;
                        break;
                    case ST_HDR_OK:
                        // read the header
                        // (cur points to correct location)
                        readlen=pheader->data_len;
                        action=AC_DATA;
                        break;
                    case ST_DATA_OK:
                        // compare checksum
                        action=AC_CHK;
                        break;
                    default:
                        // illegal state
                        MST_COUNTER_INC(self->profile->stats->events[NETIF_EV_EPROTO_RD]);
                        break;
                }
                PDPRINT((stderr,"state[%s] readlen[%u]\n",stateNames[state],readlen));

                if(action==AC_SYNC){
                    // read byte by byte until sync pattern matched
                    while( (cur-buf)<TRNIF_SYNC_LEN && action!=AC_ERR && !self->stop){
                        if( (test=msock_recvfrom(peer->sock, peer->addr,cur,1,MSG_DONTWAIT))==1 && TRNIF_SYNC_CMP(*cur,(cur-buf))){
                            PDPRINT((stderr,"SYNC - OK test[%d] cur[%p/x%02X] cur-buf[%ld] cmp[%d]\n",test,cur,*cur,(cur-buf),(TRNIF_SYNC_CMP(*cur,(cur-buf))?1:0)));
                            cur+=test;
                            msg_bytes+=test;
                            state=ST_SYNC_OK;
                        }else{
                            err=errno;
                            PDPRINT((stderr,"SYNC - ERR cur-buf[%ld] *cur[%02X] c(%02X) test[%d] err[%d/%s]\n",(cur-buf),*cur,((g_trn_sync>>(cur-buf))),test,err,strerror(err)));
                            action=AC_ERR;
                        }
                    }
                }// AC_SYNC

                if(action==AC_HDR){
                    if( (test=msock_recvfrom(peer->sock, peer->addr,cur,TRNIF_HDR_LEN-TRNIF_SYNC_LEN,0)) >0  ){
                        PDPRINT((stderr,"HDR - OK test[%d] cur[%p/x%02X]\n",test,cur,*cur));
                        cur+=test;
                        msg_bytes+=test;
                        state=ST_HDR_OK;
                    }else{
                        PTRACE();
                        err=errno;
                        action=AC_ERR;
                    }
                }// AC_HDR

                if(action==AC_DATA){
                    PTRACE();
                    if(readlen==0){
                        state=ST_DATA_OK;
                    }else if( (test=msock_recvfrom(peer->sock, peer->addr,cur,readlen,0)) == readlen ){
                        PDPRINT((stderr,"DATA - OK test[%d] cur[%p/x%02X]\n",test,cur,*cur));
                        cur+=test;
                        msg_bytes+=test;
                        state=ST_DATA_OK;
                    }else{
                        PTRACE();
                        err=errno;
                        action=AC_ERR;
                    }
                }// AC_DATA

                if(action==AC_CHK){
                    PTRACE();
                    trn_checksum_t chk=0;
                    byte *cp = buf+TRNIF_HDR_LEN;
                    chk = mfu_checksum(cp, pheader->data_len);
                    if(chk == pheader->checksum){
                        retval=msg_bytes;
                    }else{
                        if(NULL!=errout){
                            *errout=MSG_ECHK;
                        }
                        MST_COUNTER_INC(self->profile->stats->events[NETIF_EV_EPROTO_RD]);
                    }
                    state=ST_QUIT;
                }// AC_CHK

                if(action==AC_ERR){
                    MST_COUNTER_INC(self->profile->stats->events[NETIF_EV_EPROTO_RD]);
                    // check for errors
                    switch (err) {
                            // EWOULDBLOCK == EAGAIN on many systems
                        case EAGAIN:
                            PTRACE();
                            // nothing to read: quit
                            if(NULL!=errout){
                                *errout=MSG_ENODATA;
                            }
                            state=ST_QUIT;
                            break;
                        default:
                            PTRACE();
                            // start over
                            cur=buf;
                            msg_bytes=0;
                            state=ST_SYNC;
                            break;
                    }
                }// AC_ERR
            }
        }else{PTRACE();}

    }// else invalid arg
    PDPRINT((stderr,"errout[%d] msg_len/ret[%u]\n",(NULL==errout?-1:*errout),retval));
    //    sleep(1);

    return retval;
}
// End function

/// @fn int trnif_msg_handle_trnmsg(void *msg, netif_t *self, msock_connection_t *peer, int *errout)
/// @brief handle a message and respond to peer
/// @return 0 on success, error code otherwise

int trnif_msg_handle_trnmsg(void *msg, netif_t *self, msock_connection_t *peer, int *errout)
{
    int retval=-1;
    if(NULL!=msg && NULL!=self && NULL!=peer){

        wtnav_t *trn = self->rr_res;
        trnmsg_t *msg_in = NULL;
        trnmsg_t *msg_out = NULL;
        byte *pdata = TRNIF_PDATA(msg_in);
        trn_meas_t *trn_meas = NULL;
        wmeast_t *mt = NULL;

        // deserialize message bytes
        trnmsg_deserialize(&msg_in, (byte *)msg, TRNIF_MAX_SIZE);
        PDPRINT((stderr,"%s - TRNMSG received:\n",__FUNCTION__));
        if(mmd_channel_isset(MOD_NETIF,(MM_DEBUG | NETIF_V3 | NETIF_V4))){
        	trnmsg_show(msg_in,true,5);
        }

        switch (msg_in->hdr.msg_id) {
            case TRNIF_PING:
                PDPRINT((stderr,"PING from peer[%s:%s]\n",peer->chost,peer->service));
                msg_out = trnmsg_new_type_msg(TRNIF_ACK, 0xabcd);
                break;

            case TRNIF_MEAS:
                // get pointer to (trn_meas_t) data payload
                trn_meas = TRNIF_TPDATA(msg_in, trn_meas_t);
                if(NULL!=trn_meas){
                    // deserialize meast
                    wmeast_unserialize(&mt, (char *) pdata+sizeof(trn_meas_t), msg_in->hdr.data_len);
                    // do measurement update
                    wtnav_meas_update(trn, mt, trn_meas->parameter);
                    // make return message
                    msg_out = trnmsg_new_meas_msg( TRNIF_MEAS, trn_meas->parameter, mt);

                    wmeast_destroy(mt);
                }else{
                    PDPRINT((stderr,"%s - ERR NULL message\n",__func__));
                }

                break;
//
//            case TRNIF_MOTN:
//                // do measurement update (using ct->mt)
//                commst_motion_update(trn, ct);
//
//                // return ACK
//                send_len=trnw_ack_msg(&msg_out);
//                break;
//
//            case TRNIF_MLE:
//                // get MLE estimate
//                commst_estimate_pose(trn, ct,TRN_POSE_MLE);
//                // serialize updated message
//                send_len=wcommst_serialize(&msg_out,ct,TRN_MSG_SIZE);
//                break;
//
//            case TRNIF_MMSE:
//                // get MSE estimate
//                commst_estimate_pose(trn, ct,TRN_POSE_MMSE);
//                // serialize updated message
//                send_len=wcommst_serialize(&msg_out,ct,TRN_MSG_SIZE);
//                break;
//
//            case TRN_MSG_LAST_MEAS:
//                // get status, return ACK
//                // (parameter set accordingly)
//                if( wtnav_last_meas_successful(trn)){
//                    send_len=trnw_ptype_msg(&msg_out, TRN_MSG_ACK, 1);
//                }else{
//                    send_len=trnw_ptype_msg(&msg_out, TRN_MSG_ACK, 0);
//                }
//                break;
//
//            case TRN_MSG_N_REINITS:
//                // get status, return ACK
//                // (parameter set accordingly)
//                send_len=trnw_ptype_msg(&msg_out, TRN_MSG_ACK, wtnav_get_num_reinits(trn));
//                break;
//
//            case TRN_MSG_FILT_TYPE:
//                // get status, return ACK
//                // (parameter set accordingly)
//                send_len=trnw_ptype_msg(&msg_out, TRN_MSG_ACK, wtnav_get_filter_type(trn));
//                break;
//
//            case TRN_MSG_FILT_STATE:
//                // get status, return ACK
//                // (parameter set accordingly)
//                send_len=trnw_ptype_msg(&msg_out, TRN_MSG_ACK, wtnav_get_filter_state(trn));
//                break;
//
//            case TRN_MSG_OUT_MEAS:
//                // get status, return ACK
//                // (parameter set accordingly)
//                if( wtnav_outstanding_meas(trn)){
//                    send_len=trnw_ptype_msg(&msg_out, TRN_MSG_ACK, 1);
//                }else{
//                    send_len=trnw_ptype_msg(&msg_out, TRN_MSG_ACK, 0);
//                }
//                break;
//
//            case TRN_MSG_INIT:
//                // get status, return ACK
//                // (parameter set accordingly)
//                if( wtnav_initialized(trn)){
//                    send_len=trnw_ptype_msg(&msg_out, TRN_MSG_ACK, 1);
//                }else{
//                    send_len=trnw_ptype_msg(&msg_out, TRN_MSG_ACK, 0);
//                }
//                break;
//
//            case TRN_MSG_IS_CONV:
//                // get status, return ACK
//                // (parameter set accordingly)
//                if( wtnav_is_converged(trn)){
//                    send_len=trnw_ptype_msg(&msg_out, TRN_MSG_ACK, 1);
//                }else{
//                    send_len=trnw_ptype_msg(&msg_out, TRN_MSG_ACK, 0);
//                }
//                break;
//
//            case TRN_MSG_FILT_REINIT:
//                // reinit, return ACK
//                wtnav_reinit_filter(trn,true);
//                send_len=trnw_ack_msg(&msg_out);
//                break;

            default:
                PDPRINT((stderr,"UNKNOWN msg msg_in[%p] type [%c/%02X] from peer[%s:%s]\n",msg_in,msg_in->hdr.msg_id,msg_in->hdr.msg_id,peer->chost,peer->service));
                break;
        }

        if(NULL!=msg_out){
            int32_t send_len = trnmsg_len(msg_out);
            retval=s_trnif_dfl_send_tcp(peer, (char *)msg_out, send_len, errout);
        }else{
            MST_COUNTER_INC(self->profile->stats->events[NETIF_EV_EPROTO_HND]);
        }
        trnmsg_destroy(&msg_in);
        trnmsg_destroy(&msg_out);
    }// else invalid arg
    return retval;
}
// End function

int trnif_msg_read_ct(byte **pdest, uint32_t *len, netif_t *self, msock_connection_t *peer, int *errout)
{
#define TRNIF_READ_RETRIES_CT 40
#define TRNIF_READ_DELAY_CT  10
    int retval = 0;

    int64_t msg_bytes=0;
    int retries = 0;
   if(NULL!=pdest && NULL!=self && NULL!=peer){
        uint32_t readlen=TRN_MSG_SIZE;
        byte *buf=*pdest;
        if(NULL==buf){
            buf=(byte *)malloc(TRN_MSG_SIZE);
            memset(buf,0,TRN_MSG_SIZE);
            *pdest=buf;
        }

        // default read (all at once)
//        retval=s_trnif_msg_read_dfl(buf, readlen, peer->sock, peer, errout);
//        PDPRINT((stderr,"%s: READ - readlen[%d]\n",__FUNCTION__,readlen));

        // alternative read
        // (writer chunks data, and there is no sync mechanism - scary)
        // - try to read entire message
        // - if no bytes received on first attempt, return
        // - if any bytes received, keep reading w/ brief delay until
        //   message complete or retries expire
        byte *pread = buf;
        int64_t read_sz = readlen;
       while(retries<TRNIF_READ_RETRIES_CT  && msg_bytes<read_sz ){
           int64_t read_bytes = 0;
//            fprintf(stderr,"%s:%d msg_bytes[%"PRId64"] retries[%d/%d] readlen[%u]\n",__FUNCTION__,__LINE__,msg_bytes,TRNIF_READ_RETRIES_CT-retries,TRNIF_READ_RETRIES_CT,readlen);
            if( (read_bytes=msock_recv(peer->sock, pread,readlen,0)) > 0 ){
                readlen   -= read_bytes;
                msg_bytes += read_bytes;
                pread     += read_bytes;
            }else{
                int errsave=errno;
                if(errsave!=EAGAIN){
	                fprintf(stderr,"%s:%d ERR recv msg_bytes[%"PRId64"] [%d/%s]\n",__FUNCTION__,__LINE__,msg_bytes,errsave,strerror(errsave));
                    MST_COUNTER_INC(self->profile->stats->events[NETIF_EV_EPROTO_RD]);
                }
                if(NULL!=errout){
               		*errout = errsave;
                }
            }
            if(msg_bytes==0 && retries==0)
                break;
            mtime_delay_ms(TRNIF_READ_DELAY_CT);
            retries++;
        }

//        fprintf(stderr,"%s:%d msg_bytes[%"PRId64"] retries[%d/%d] readlen[%u]\n",__FUNCTION__,__LINE__,msg_bytes,TRNIF_READ_RETRIES_CT-retries,TRNIF_READ_RETRIES_CT,readlen);
        *len = msg_bytes;
        retval = msg_bytes;

    }else{
        if(NULL!=self){
        MST_COUNTER_INC(self->profile->stats->events[NETIF_EV_EPROTO_RD]);
        }else{
            fprintf(stderr,"%s - ERR invalid arg\n",__func__);
        }
    }

    if(mmd_channel_isset(MOD_NETIF,(MM_DEBUG))){
   	 PDPRINT((stderr,"%s:%d RET msg_bytes[%"PRId64"] retries[%d]\n",__FUNCTION__,__LINE__,msg_bytes,retries));
    }
    return retval;
}
// End function

int trnif_msg_handle_ct(void *msg, netif_t *self, msock_connection_t *peer, int *errout)
{
    int retval=-1;

    if(NULL!=msg && NULL!=self && NULL!=peer){

        int32_t send_len=0;
        char *msg_out=NULL;
        wcommst_t *ct = NULL;
        // dereference resource bundle
        wtnav_t *trn = (wtnav_t *) self->rr_res;
        // deserialize message bytes
        wcommst_unserialize(&ct,(char *)msg,TRN_MSG_SIZE);
        char msg_type =wcommst_get_msg_type(ct);
        int param=0;
        double dparam=0.0;
        static int ensemble_count=0;

        if(mmd_channel_isset(MOD_NETIF,(MM_DEBUG))){
        	wcommst_show(ct,true,5);
        }
        double msg_time = mtime_etime();
        switch (msg_type) {

            case TRN_MSG_INIT:
                // TODO : initialize TRN
                commst_initialize(trn, ct);
                // return ACK/NACK
                if(wtnav_initialized(trn)){
                    send_len=trnw_ack_msg(&msg_out);
                    mlog_tprintf(self->mlog_id,"trn_init_ack,[%s:%s]\n", peer->chost, peer->service);
                }else{
                    send_len=trnw_nack_msg(&msg_out);
                    mlog_tprintf(self->mlog_id,"trn_init_nack,[%s:%s]\n", peer->chost, peer->service);
                }

                mlog_tprintf(self->mlog_id,"trn_init,%lf,[%s:%s]\n",msg_time,peer->chost, peer->service);
                break;

            case TRN_MSG_MEAS:
                // do measurement update (using ct->mt)
                commst_meas_update(trn, ct);
                // serialize updated message
                send_len=wcommst_serialize(&msg_out,ct,TRN_MSG_SIZE);
                mlog_tprintf(self->mlog_id,"trn_meas,%lf,[%s:%s]\n",msg_time,peer->chost, peer->service);
                break;

            case TRN_MSG_MOTN:
                // do measurement update (using ct->mt)
                commst_motion_update(trn, ct);
                mlog_tprintf(self->mlog_id,"trn_motn,%lf,[%s:%s]\n",msg_time,peer->chost, peer->service);

                // return ACK
                send_len=trnw_ack_msg(&msg_out);
                break;

            case TRN_MSG_MLE:
                // get MLE estimate
                commst_estimate_pose(trn, ct,TRN_POSE_MLE);
                // serialize updated message
                send_len=wcommst_serialize(&msg_out,ct,TRN_MSG_SIZE);
                mlog_tprintf(self->mlog_id,"trn_mle,%lf,[%s:%s]\n",msg_time,peer->chost, peer->service);

                if(mmd_channel_isset(MOD_NETIF,(MM_DEBUG))){
                    PDPRINT((stderr,"MLE ct[%p] msg_out[%p] send_len[%d]\n",ct,msg_out,send_len));
//                    wcommst_show(ct,true,5);
                }
                break;

            case TRN_MSG_MMSE:
                // get MSE estimate
                commst_estimate_pose(trn, ct,TRN_POSE_MMSE);
                // serialize updated message
                send_len=wcommst_serialize(&msg_out,ct,TRN_MSG_SIZE);
	            mlog_tprintf(self->mlog_id,"trn_mmse,%lf,%d,[%s:%s]\n",msg_time,++ensemble_count,peer->chost, peer->service);

                if(mmd_channel_isset(MOD_NETIF,(MM_DEBUG))){
                    PDPRINT((stderr,"MMSE ct[%p] msg_out[%p] send_len[%d]\n",ct,msg_out,send_len));
//                    wcommst_show(ct,true,5);
                }
               break;

            case TRN_MSG_LAST_MEAS:
                // get status, return ACK
                // (parameter set accordingly)
                if( wtnav_last_meas_successful(trn)){
                    param=1;
                }else{
                    param=0;
                }
                send_len=trnw_ptype_msg(&msg_out, TRN_MSG_ACK, param);
                mlog_tprintf(self->mlog_id,"trn_lms,%lf,%d,[%s:%s]\n",msg_time, param, peer->chost, peer->service);
                break;

            case TRN_MSG_N_REINITS:
                // get status, return ACK
                // (parameter set accordingly)
                send_len=trnw_ptype_msg(&msg_out, TRN_MSG_ACK, wtnav_get_num_reinits(trn));
                if(mmd_channel_isset(MOD_NETIF,(MM_DEBUG))){
                    PDPRINT((stderr,"N_REINITS ct[%p] msg_out[%p] send_len[%d]\n",ct,msg_out,send_len));
//                    mfu_hex_show(msg_out, 128, 16, true, 5);
                }
            mlog_tprintf(self->mlog_id,"trn_n_reinits,%lf,[%s:%s]\n",msg_time,peer->chost, peer->service);
               break;

            case TRN_MSG_FILT_TYPE:
                // get status, return ACK
                // (parameter set accordingly)
                send_len=trnw_ptype_msg(&msg_out, TRN_MSG_ACK, wtnav_get_filter_type(trn));
                mlog_tprintf(self->mlog_id,"trn_ftype,%lf,[%s:%s]\n",msg_time,peer->chost, peer->service);
                break;

            case TRN_MSG_FILT_STATE:
                // get status, return ACK
                // (parameter set accordingly)
                send_len=trnw_ptype_msg(&msg_out, TRN_MSG_ACK, wtnav_get_filter_state(trn));
            mlog_tprintf(self->mlog_id,"trn_fstate,%lf,[%s:%s]\n",msg_time, peer->chost, peer->service);
               break;

            case TRN_MSG_OUT_MEAS:
                // get status, return ACK
                // (parameter set accordingly)
                if( wtnav_outstanding_meas(trn)){
                    param=1;
                }else{
                    param=0;
                }
                send_len=trnw_ptype_msg(&msg_out, TRN_MSG_ACK, param);
                mlog_tprintf(self->mlog_id,"trn_out_meas,%lf,%d,[%s:%s]\n",msg_time, param, peer->chost, peer->service);
               break;

            case TRN_MSG_IS_CONV:
                // get status, return ACK
                // (parameter set accordingly)
                if( wtnav_is_converged(trn)){
                    param=1;
                }else{
                    param=0;
                }
                send_len=trnw_ptype_msg(&msg_out, TRN_MSG_ACK, param);
                mlog_tprintf(self->mlog_id,"trn_is_conv,%lf,%d,[%s:%s]\n",msg_time, param,peer->chost, peer->service);
               break;

            case TRN_MSG_FILT_REINIT:
                // reinit, return ACK
                wtnav_reinit_filter(trn,true);
                send_len=trnw_ack_msg(&msg_out);
                mlog_tprintf(self->mlog_id,"trn_filt_reinit,%lf,[%s:%s]\n", msg_time,peer->chost, peer->service);
                break;

            case TRN_MSG_SET_MW:
                // set modified weighting, return ACK
                param = wcommst_get_parameter(ct);
                wtnav_set_modified_weighting(trn, param);
                send_len=trnw_ack_msg(&msg_out);
                mlog_tprintf(self->mlog_id,"trn_set_mw,%lf,%d,[%s:%s]\n", msg_time,param, peer->chost, peer->service);
               break;

            case TRN_MSG_SET_FR:
                // set filter reinit, return ACK
                param = wcommst_get_parameter(ct);
                wtnav_set_filter_reinit(trn, (param==0?false:true));
                send_len=trnw_ack_msg(&msg_out);
                mlog_tprintf(self->mlog_id,"trn_set_fr,%lf,%d,[%s:%s]\n",msg_time, param, peer->chost, peer->service);
                break;

            case TRN_MSG_SET_IMA:
                // set filter reinit, return ACK
                param = wcommst_get_parameter(ct);
                wtnav_set_interp_meas_attitude(trn, (param==0?false:true));
                send_len=trnw_ack_msg(&msg_out);
                mlog_tprintf(self->mlog_id,"trn_set_ima,%lf,%d,[%s:%s]\n",msg_time, param, peer->chost, peer->service);
                break;

            case TRN_MSG_SET_MIM:
                // set modified weighting, return ACK
                param = wcommst_get_parameter(ct);
                wtnav_set_map_interp_method(trn, param);
                send_len=trnw_ack_msg(&msg_out);
                mlog_tprintf(self->mlog_id,"trn_set_mim,%lf,%d,[%s:%s]\n",msg_time, param, peer->chost, peer->service);
               break;

            case TRN_MSG_SET_VDR:
                // set vehicle drift rate, return ACK
                dparam =wcommst_get_vdr(ct);
                wtnav_set_vehicle_drift_rate(trn, dparam);
                send_len=trnw_ack_msg(&msg_out);
                mlog_tprintf(self->mlog_id,"trn_set_vdr,%lf,%lf,[%s:%s]\n", msg_time,dparam, peer->chost, peer->service);
                break;

            case TRN_MSG_FILT_GRD:
                // set filter gradient, return ACK
                param = wcommst_get_parameter(ct);
               if(param==0){
                	wtnav_use_highgrade_filter(trn);
                }else{
                    wtnav_use_lowgrade_filter(trn);
                }
                send_len=trnw_ack_msg(&msg_out);
                mlog_tprintf(self->mlog_id,"trn_set_filtgrd,%lf,%d,[%s:%s]\n", msg_time,param, peer->chost, peer->service);
               break;

            case TRN_MSG_PING:
                PDPRINT((stderr,"trn_ping_ack,%lf,[%s:%s]\n",msg_time,peer->chost,peer->service));
                send_len = trnw_ack_msg(&msg_out);
                mlog_tprintf(self->mlog_id,"trn_ping_ACK,%lf,[%s:%s]\n", msg_time, peer->chost, peer->service);
               break;

            case TRN_MSG_IS_INIT:
                // get status, return ACK
                // (parameter set accordingly)
                if( wtnav_initialized(trn)){
                    param=1;
                }else{
                    param=0;
                }
                send_len=trnw_ptype_msg(&msg_out, TRN_MSG_ACK, param);
                mlog_tprintf(self->mlog_id,"trn_is_init,%lf,%d,[%s:%s]\n", msg_time,param, peer->chost, peer->service);
               break;

            default:
                PDPRINT((stderr,"UNSUPPORTED msg ct[%p] type [%c/%02X] from peer[%s:%s] %lf\n",ct,msg_type,msg_type,peer->chost,peer->service,mtime_etime()));

                send_len=trnw_nack_msg(&msg_out);
                MST_COUNTER_INC(self->profile->stats->events[NETIF_EV_EPROTO_HND]);
                break;
        }

        if(send_len>0){
            retval=s_trnif_dfl_send_tcp(peer, msg_out, send_len, errout);
            if(mmd_channel_isset(MOD_NETIF,(MM_DEBUG))){
                PDPRINT((stderr,"SEND_LEN>0 msg_type[%c/%02X] peer[%s:%s] %lf\n",(msg_type>0x20?msg_type:'.'), msg_type, peer->chost, peer->service, mtime_etime()));
                mfu_hex_show((byte *)msg_out, 128, 16, true, 5);
            }
        }else{
            PDPRINT((stderr,"SEND_LEN<=0 type [%c/%02X] peer[%s:%s] %lf\n",(msg_type>0x20?msg_type:'.'),msg_type, peer->chost, peer->service, mtime_etime()));
            MST_COUNTER_INC(self->profile->stats->events[NETIF_EV_EPROTO_HND]);
		 }

        wcommst_destroy(ct);
        if(NULL!=msg_out){
            free(msg_out);
        }
    }// else invalid arg
    return retval;
}
// End function

int trnif_msg_read_mb(byte **pdest, uint32_t *len, netif_t *self, msock_connection_t *peer, int *errout)
{
    int retval = 0;

    if(NULL!=pdest && NULL!=self && NULL!=peer){
        uint32_t readlen=MBIF_MSG_SIZE;
        byte *buf=*pdest;
        if(NULL==buf){
            buf=(byte *)malloc(MBIF_MSG_SIZE);
            memset(buf,0,MBIF_MSG_SIZE);
            *pdest=buf;
        }

        if( (retval=s_trnif_msg_read_dfl(buf, readlen, peer->sock, peer, errout))<=0){
            MST_COUNTER_INC(self->profile->stats->events[NETIF_EV_EPROTO_RD]);
        }
    }
    return retval;
}
// End function

int trnif_msg_handle_mb(void *msg, netif_t *self, msock_connection_t *peer, int *errout)
{
    int retval=-1;

    if(NULL!=msg && NULL!=self && NULL!=peer){

        int32_t send_len=0;
        char *msg_out=NULL;

        if(strcmp(msg,PROTO_MB_CON)==0){
            msg_out=strdup(PROTO_MB_ACK);
            send_len=strlen(PROTO_MB_ACK)+1;
        }else if(strcmp(msg,PROTO_MB_REQ)==0){
            msg_out=strdup(PROTO_MB_ACK);
            send_len=strlen(PROTO_MB_ACK)+1;
        }else{
            msg_out=strdup(PROTO_MB_NACK);
            send_len=strlen(PROTO_MB_NACK)+1;
        }

        if(send_len>0){
            // send response
            retval=s_trnif_dfl_send_udp(self,peer, msg_out, send_len, errout);
        }else{
            MST_COUNTER_INC(self->profile->stats->events[NETIF_EV_EPROTO_HND]);
        }

        if(NULL!=msg_out){
            free(msg_out);
        }
    }

    return retval;
}

int trnif_msg_pub_mb(netif_t *self, msock_connection_t *peer, char *data, size_t len)
{
    // use default
    return trnif_msg_pub(self,peer,data,len);
}

int trnif_msg_read_trnu(byte **pdest, uint32_t *len, netif_t *self, msock_connection_t *peer, int *errout)
{
    int retval = 0;

    if(NULL!=pdest && NULL!=self && NULL!=peer){
        uint32_t readlen=TRNX_MSG_SIZE;
        byte *buf=*pdest;
        if(NULL==buf){
            buf=(byte *)malloc(TRNX_MSG_SIZE);
            memset(buf,0,TRNX_MSG_SIZE);
            *pdest=buf;
        }

        if( (retval=s_trnif_msg_read_dfl(buf, readlen, peer->sock, peer, errout))<=0){
            MST_COUNTER_INC(self->profile->stats->events[NETIF_EV_EPROTO_RD]);
        }
    }
    return retval;
}

int trnif_msg_handle_trnu(void *msg, netif_t *self, msock_connection_t *peer, int *errout)
{
    int retval=-1;

    if(NULL!=msg && NULL!=self && NULL!=peer){

        // get resource bundle
        trnuif_res_t *resources=self->rr_res;

       int32_t send_len=0;
        char *msg_out=NULL;

        double msg_time = mtime_etime();

        if(strcmp(msg,PROTO_TRNU_REQ)==0 ||
           strcmp(msg,PROTO_TRNU_CON)==0||
           strcmp(msg,PROTO_TRNU_HBT)==0||
           strcmp(msg,PROTO_TRNU_DIS)==0||
            strcmp(msg,PROTO_TRNU_PING)==0){
            msg_out=strdup(PROTO_TRNU_ACK);
            send_len=strlen(PROTO_TRNU_ACK)+1;
        }else{
            msg_out=strdup(PROTO_TRNU_NACK);
            send_len=strlen(PROTO_TRNU_NACK)+1;
        }

        if(strcmp(msg,PROTO_TRNU_RST)==0){
            // reinit, return ACK/NACK
            if(NULL!=resources && NULL!=resources->reset_callback){
                int test=resources->reset_callback();
                const char *ret_msg=(test==0 ? PROTO_TRNU_ACK : PROTO_TRNU_NACK);
                msg_out=strdup(ret_msg);
                send_len=strlen(ret_msg)+1;
                mlog_tprintf(self->mlog_id,"trn_filt_reinit,%lf,[%s:%s],%d\n", msg_time,peer->chost, peer->service,test);
            }else{
                mlog_tprintf(self->mlog_id,"trn_filt_reinit[NULL resuorce],%lf,[%s,%s],-1\n", msg_time,peer->chost, peer->service);
                msg_out=strdup(PROTO_TRNU_NACK);
                send_len=strlen(PROTO_TRNU_NACK)+1;
            }
        }

        if(send_len>0){
            // send response
            retval=s_trnif_dfl_send_udp(self,peer, msg_out, send_len, errout);
        }else{
            MST_COUNTER_INC(self->profile->stats->events[NETIF_EV_EPROTO_HND]);
        }

        if(NULL!=msg_out){
            free(msg_out);
        }
    }

    return retval;
}

int trnif_msg_pub_trnu(netif_t *self, msock_connection_t *peer, char *data, size_t len)
{
    // use default
    return trnif_msg_pub(self,peer,data,len);
}

int trnif_msg_pub(netif_t *self, msock_connection_t *peer, char *data, size_t len)
{
    int retval=-1;

    if(NULL!=self && NULL!=peer && NULL!=data && len>0){
        if(self->ctype==ST_UDP){
            int flags=0;
#if !defined(__APPLE__)
            flags=MSG_NOSIGNAL;
#endif
            int64_t iobytes = msock_sendto(self->socket, peer->addr, (byte *)data, len, flags );
            if ( iobytes > 0) {
                retval=iobytes;
            }
        }
        if(self->ctype==ST_TCP){
            int64_t iobytes = msock_send(peer->sock, (byte *)data, len );
            if ( iobytes > 0) {
                retval=iobytes;
            }
        }
    }
    return retval;
}
// End function
