///
/// @file trnu_cli.c
/// @authors k. headley
/// @date 10 jul 2019

/// TRN update UDP client

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
#include "trnu_cli.h"
#include "trnif_proto.h"

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

#define TRNUCLI_ACK_BYTES       4
#define TRNUCLI_ACK_RETRIES    10
#define TRNUCLI_ACK_WAIT_MSEC 150
#define TRNUCLI_SHOW_WKEY 16
#define TRNUCLI_SHOW_WVAL 16

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

// send message, get ack/nack
// return 0 if ACK/NACK, -1 otherwise
static int s_get_acknak(trnucli_t *self, uint32_t retries, uint32_t delay)
{
    int retval = -1;

    if(NULL!=self && retries){
        PDPRINT((stderr,"%s - retries[%"PRIu32"] del[%"PRIu32"]\n",__func__,retries,delay));
        byte ack[TRNUCLI_ACK_BYTES]={0};
        uint32_t rx_retries=retries;
        msock_set_blocking(self->trnu->sock,false);

        while(retval!=0 && rx_retries>0){
            int64_t test=-1;
            bool is_acknak=false;
            bool is_update=false;
            memset(ack,0,TRNUCLI_ACK_BYTES);
            PDPRINT((stderr,"%s - rx_retries[%"PRIu32"]\n",__func__,rx_retries));

            // check message type
            if( (test=msock_recvfrom(self->trnu->sock,self->trnu->sock->addr,ack,TRNUCLI_ACK_BYTES,MSG_PEEK))>0){
                const char *cp=(char *)ack;
                uint32_t *ip=(uint32_t *)ack;
                if(strcmp(cp,PROTO_TRNU_ACK)==0 || strcmp(cp,PROTO_TRNU_NACK)==0 ){
                    is_acknak=true;
                }else if(*ip == TRNU_PUB_SYNC){
                    is_update=true;
                }
            }

            if(is_acknak){
                // this is the ACK/NAK we're looking for
                if((test=msock_recvfrom(self->trnu->sock,self->trnu->sock->addr,ack,TRNUCLI_ACK_BYTES,0))>0){
                    PDPRINT((stderr,"%s - ret/ret/ack[%"PRIu32"/%"PRId64"/%s]\n",__func__,rx_retries,test,ack));
#ifdef WITH_PDEBUG
                    int64_t i=0;
                    for(i=0;i<test && i<TRNUCLI_ACK_BYTES;i++)
                        fprintf(stderr,"%02X ",ack[i]);
                    fprintf(stderr,"\n");
#endif
                    const char *cp=(char *)ack;
                    if(strcmp(cp,PROTO_TRNU_ACK)==0 || strcmp(cp,PROTO_TRNU_NACK)==0){
                        retval=0;
                        break;
                    }
                }
            }else if(is_update){
                // it's an update - go get it, then resume ACK/NAK search
                trnucli_listen(self,false);
            }else{
                PDPRINT((stderr,"ACK/NACK failed [%"PRIu32"/%"PRId64"/%s]\n",rx_retries,test,ack));
#ifdef WITH_PDEBUG
                    fprintf(stderr,"ack bytes:\n");

                    int64_t i=0;
                    for(i=0;i<TRNUCLI_ACK_BYTES;i++)
                        fprintf(stderr,"%02X ",ack[i]);
                    fprintf(stderr,"\n");
#endif
            }
	    if(delay>0){
              mtime_delay_ms(delay);
            }
            rx_retries--;
        }//while
    }//else invalid args

    return retval;
}//s_get_acknak

// send message, get ack/nack
// return 0 if ACK/NACK, -1 otherwise
static int s_send_recv(trnucli_t *self, byte *msg, int32_t len)
{
    int retval = -1;

    if(NULL!=self && NULL!=msg && len>0){
        int64_t test=msock_sendto(self->trnu->sock,NULL,msg,len,0);
        if(test>0){
            PDPRINT((stderr,"send msg OK [%s/%"PRId64"]\n",msg,test));
            retval=s_get_acknak(self,TRNUCLI_ACK_RETRIES,TRNUCLI_ACK_WAIT_MSEC);
        }else{PTRACE();}
    }

    return retval;
}//s_send_recv

trnucli_t *trnucli_new(update_callback_fn update_fn, trnuc_flags_t flags, double hbeat_to_sec)
{
    trnucli_t *instance=(trnucli_t *)malloc(sizeof(trnucli_t));
    if(NULL!=instance){
        memset(instance,0,sizeof(trnucli_t));
        instance->trnu = msock_connection_new();
        instance->update = NULL;
        instance->update_fn = update_fn;
        instance->hbeat_to_sec = hbeat_to_sec;
        instance->flags = flags;
    }
    
    return instance;
}// end function trnucli_new

void trnucli_destroy(trnucli_t **pself)
{
    if(NULL!=pself){
        trnucli_t *self=(trnucli_t *)(*pself);
        if(NULL!=self->trnu){
            msock_connection_destroy(&self->trnu);
        }
        if(NULL!=self->update){
            free(self->update);
        }
        free(self);
        *pself=NULL;
    }
}// end function trnucli_destroy

int trnucli_connect(trnucli_t *self, char *host, int port)
{
    int retval=-1;

    if(NULL!=self->trnu->sock){
        msock_socket_destroy(&self->trnu->sock);
    }

    if( (self->trnu->sock=msock_socket_new(host,port,ST_UDP))!=NULL ){

        msock_set_blocking(self->trnu->sock,false);
        int test=-1;

        if ( (test=msock_connect(self->trnu->sock))==0) {
            int32_t slen=(strlen(PROTO_TRNU_CON)+1);
            retval = s_send_recv(self,(byte *)PROTO_TRNU_CON,slen);
        }else{
            PTRACE();
            PDPRINT((stderr,"CON failed [%d]\n",test));
        }
    }else{
        PTRACE();
    }

    return retval;
}// end function trnucli_connect

int trnucli_disconnect(trnucli_t *self)
{
    int retval=-1;
    if(NULL!=self && NULL!=self->trnu && NULL!=self->trnu->sock){
    msock_set_blocking(self->trnu->sock,false);
    char msg[8]={0};
    sprintf(msg,PROTO_TRNU_DIS);
    int32_t sret=msock_sendto(self->trnu->sock,NULL,(byte *)msg,4,0);
    if(sret>0){
        retval=0;
    }
    }
    return retval;
}// end function trnucli_disconnect

int trnucli_set_callback(trnucli_t *self, update_callback_fn func)
{
    int retval=-1;
    if(NULL!=self){
        self->update_fn = func;
        retval=0;
    }
    return retval;
}// end function trnucli_set_callback


int trnucli_listen(trnucli_t *self, bool callback_en)
{
    int retval=-1;

    if(NULL!=self){
        if(NULL==self->update){
            self->update=(trnu_pub_t *)malloc(TRNU_PUB_BYTES);
        }
        if(NULL!=self->update){
            memset(self->update,0,TRNU_PUB_BYTES);
            msock_set_blocking(self->trnu->sock,TRNUC_BLK_LISTEN(self->flags));
            int32_t read_len=TRNU_PUB_BYTES;
            int64_t rret=msock_recvfrom(self->trnu->sock,self->trnu->sock->addr,(byte *)self->update,read_len,0);
            if(rret==read_len){
                retval=0;
                PDPRINT((stderr,"%s - recv OK rret/mb1cyc[%"PRId64",%d]\n",__func__,rret,self->update->mb1_cycle));
                if(callback_en && NULL!=self->update_fn){
                    retval= self->update_fn(self->update);
                }else{
                    // no handler specified
                }
            }else{
                PDPRINT((stderr,"%s - recv ERR rret[%"PRId64"]\n",__func__,rret));
#ifdef WITH_PDEBUG
                if(rret>=0){
                    int64_t i=0;
                    byte *bp=(byte *)self->update;
                    for(i=0;i<rret && i<TRNU_PUB_BYTES;i++)
                        fprintf(stderr,"%02X ",bp[i]);
                    fprintf(stderr,"\n");
                }
#endif
            }// else reject non-updates (e.g. ACK/NACK
        }else{
            // could not alloc
        }
    }

    return retval;
}//trnucli_listen

int trnucli_reset_trn(trnucli_t *self)
{
    int retval=-1;

    if(NULL!=self){
        int32_t len=(strlen(PROTO_TRNU_RST)+1);
        retval=s_send_recv(self,(byte *)PROTO_TRNU_RST,len);
    }
    return retval;
}//trnucli_reset_trn

int trnucli_hbeat(trnucli_t *self)
{
    int retval=-1;

    if(NULL!=self){
        int32_t len=(strlen(PROTO_TRNU_HBT)+1);
        retval=s_send_recv(self,(byte *)PROTO_TRNU_HBT,len);
    }

    return retval;
}// end trnucli_hbeat

double trnucli_update_mb1time(trnu_pub_t *update)
{
    double retval=-1.0;
    if(NULL!=update){
        retval = update->mb1_time;
    }
    return retval;
}//trnucli_ctx_update_mb1time

double trnucli_update_mb1age(trnu_pub_t *update)
{
    double retval=-1.0;
    if(NULL!=update){
        retval = mtime_etime()-update->mb1_time;
    }
    return retval;
}//trnucli_update_mb1time

double trnucli_update_hosttime(trnu_pub_t *update)
{
    double retval=-1.0;
    if(NULL!=update){
        retval = update->update_time;
    }
    return retval;
}//trnucli_update_hosttime

double trnucli_update_hostage(trnu_pub_t *update)
{
    double retval=-1.0;
    if(NULL!=update){
        retval = mtime_etime()-update->update_time;
    }
    return retval;
}// trnucli_update_hostage


static int s_update_pretty_org(trnu_pub_t *update, char *dest, int len, int indent)
{
    int retval=0;
    if(NULL!=update && NULL!=dest && len>0){
        int wkey=TRNUCLI_SHOW_WKEY;
        int wval=TRNUCLI_SHOW_WVAL;
        int rem=len;
        char *dp=dest;
        int wbytes=snprintf(dp,rem,"%*s %*s  %*p\n",indent,(indent>0?" ":""), wkey,"addr",wval,update);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*.3lf\n",indent,(indent>0?" ":""), wkey,"mb1_time",wval,update->mb1_time);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*.3lf\n",indent,(indent>0?" ":""), wkey,"update_time",wval,update->update_time);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*s%08X\n",indent,(indent>0?" ":""), wkey,"sync",(wval-8)," ",update->sync);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*d\n",indent,(indent>0?" ":""), wkey,"reinit_count",wval,update->reinit_count);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*.3lf\n",indent,(indent>0?" ":""), wkey,"reinit_t_update",wval,update->reinit_tlast);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*d\n",indent,(indent>0?" ":""), wkey,"filter_state",wval,update->filter_state);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*d\n",indent,(indent>0?" ":""), wkey,"success",wval,update->success);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*hd\n",indent,(indent>0?" ":""), wkey,"is_converged",wval,update->is_converged);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*hd\n",indent,(indent>0?" ":""), wkey,"is_valid",wval,update->is_valid);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*d\n",indent,(indent>0?" ":""), wkey,"mb1_cycle",wval,update->mb1_cycle);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*d\n",indent,(indent>0?" ":""), wkey,"ping_number",wval,update->ping_number);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s\n",indent,(indent>0?" ":""), wkey,"estimates:");
        rem-=(wbytes-1);
        dp+=wbytes;
        int i=0;
        for(i=0;i<3;i++){
            trnu_estimate_t *est = &update->est[i];
            const char *est_labels[3]={"pt", "mle", "mmse"};
            //            const char *cov_labels[4]={"pt.covx", "pt.covy","pt.covz","pt.covxy"};
            wbytes=snprintf(dp,rem,"%*s %*s[%d]   %.3lf,%s,%.3lf,%.3lf,%.3lf,%.3lf,%.3lf,%.3lf,%.3lf\n",indent,(indent>0?" ":""),wkey-3," ",i,
                    est->time,est_labels[i],
                    est->x,est->y,est->z,
                    est->cov[0],est->cov[1],est->cov[2],est->cov[3]);

            rem-=(wbytes-1);
            dp+=wbytes;
        }

        wbytes=snprintf(dp,rem,"%*s %*s\n",indent,(indent>0?" ":""), wkey,"Bias Estimates:");
        rem-=(wbytes-1);
        dp+=wbytes;
        trnu_estimate_t *ept = &update->est[TRNU_EST_PT];
        trnu_estimate_t *emle = &update->est[TRNU_EST_MLE];
        trnu_estimate_t *emmse = &update->est[TRNU_EST_MMSE];
        wbytes=snprintf(dp,rem,"%*s %*s %.3lf,%.3lf,%.3lf\n",indent,(indent>0?" ":""), wkey," MLE:",(emle->x-ept->x),(emle->y-ept->y),(emle->z-ept->z));
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s %.3lf,%.3lf,%.3lf\n",indent,(indent>0?" ":""), wkey,"MMSE:",(emmse->x-ept->x),(emmse->y-ept->y),(emmse->z-ept->z));
        rem-=(wbytes-1);
        dp+=wbytes;
        snprintf(dp,rem,"%*s %*s %.3lf,%.3lf,%.3lf\n",indent,(indent>0?" ":""), wkey," COV:",sqrt(emmse->cov[0]),sqrt(emmse->cov[1]),sqrt(emmse->cov[2]));

        retval=strlen(dest)+1;
    }
    return retval;
}

static int s_update_csv_org(trnu_pub_t *update, char *dest, int len)
{
    int retval=0;
    if(NULL!=update && NULL!=dest && len>0){
        int rem=len;
        char *dp=dest;

        int wbytes=snprintf(dp,rem,"%.3lf,",update->mb1_time);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%.3lf,",update->update_time);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%04X,",update->sync);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%d,",update->reinit_count);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%.3lf,",update->reinit_tlast);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%d,",update->filter_state);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%d,",update->success);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%hd,",update->is_converged);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%hd,",update->is_valid);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%d,",update->mb1_cycle);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%d,",update->ping_number);
        rem-=(wbytes-1);
        dp+=wbytes;
        int i=0;
        for(i=0;i<3;i++){
            trnu_estimate_t *est = &update->est[i];
            wbytes=snprintf(dp,rem,"%.3lf,%.3lf,%.3lf,%.3lf,%.3lf,%.3lf,%.3lf,",
                            est->x,est->y,est->z,
                            est->cov[0],est->cov[1],est->cov[2],est->cov[3]);

            rem-=(wbytes-1);
            dp+=wbytes;
        }

        trnu_estimate_t *ept = &update->est[TRNU_EST_PT];
        trnu_estimate_t *emle = &update->est[TRNU_EST_MLE];
        trnu_estimate_t *emmse = &update->est[TRNU_EST_MMSE];
        wbytes=snprintf(dp,rem,"%.3lf,%.3lf,%.3lf,",(emle->x-ept->x),(emle->y-ept->y),(emle->z-ept->z));
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%.3lf,%.3lf,%.3lf,",(emmse->x-ept->x),(emmse->y-ept->y),(emmse->z-ept->z));
        rem-=(wbytes-1);
        dp+=wbytes;
        snprintf(dp,rem,"%.3lf,%.3lf,%.3lf",sqrt(emmse->cov[0]),sqrt(emmse->cov[1]),sqrt(emmse->cov[2]));

        retval=strlen(dest)+1;
    }
    return retval;
}

static int s_update_pretty(trnu_pub_t *update, char *dest, int len, int indent)
{
    int retval=0;
    if(NULL!=update && NULL!=dest && len>0){
        int wkey=15;
        int wval=15;
        int rem=len;
        char *dp=dest;
        int wbytes=snprintf(dp,rem,"%*s %*s  %*p\n",indent,(indent>0?" ":""), wkey,"addr",wval,update);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*.3lf\n",indent,(indent>0?" ":""), wkey,"mb1_time",wval,update->mb1_time);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*.3lf\n",indent,(indent>0?" ":""), wkey,"update_time",wval,update->update_time);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*.3lf\n",indent,(indent>0?" ":""), wkey,"reinit_time",wval,update->reinit_time);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*s%08X\n",indent,(indent>0?" ":""), wkey,"sync",(wval-8)," ",update->sync);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*d\n",indent,(indent>0?" ":""), wkey,"reinit_count",wval,update->reinit_count);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*.3lf\n",indent,(indent>0?" ":""), wkey,"reinit_tlast",wval,update->reinit_tlast);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*d\n",indent,(indent>0?" ":""), wkey,"filter_state",wval,update->filter_state);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*d\n",indent,(indent>0?" ":""), wkey,"success",wval,update->success);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*hd\n",indent,(indent>0?" ":""), wkey,"is_converged",wval,update->is_converged);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*hd\n",indent,(indent>0?" ":""), wkey,"is_valid",wval,update->is_valid);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*d\n",indent,(indent>0?" ":""), wkey,"mb1_cycle",wval,update->mb1_cycle);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*d\n",indent,(indent>0?" ":""), wkey,"ping_number",wval,update->ping_number);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*d\n",indent,(indent>0?" ":""), wkey,"n_con_seq",wval,update->n_con_seq);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*d\n",indent,(indent>0?" ":""), wkey,"n_con_tot",wval,update->n_con_tot);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*d\n",indent,(indent>0?" ":""), wkey,"n_uncon_seq",wval,update->n_uncon_seq);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*d\n",indent,(indent>0?" ":""), wkey,"n_uncon_tot",wval,update->n_uncon_tot);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s  %*d\n",indent,(indent>0?" ":""), wkey,"n_con_seq",wval,update->n_con_seq);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s\n",indent,(indent>0?" ":""), wkey,"estimates:");
        rem-=(wbytes-1);
        dp+=wbytes;
        int i=0;
        for(i=0;i<5;i++){
            trnu_estimate_t *est = &update->est[i];
            const char *est_labels[5]={"pt", "mle", "mmse", "offset", "last_good"};
            //            const char *cov_labels[4]={"pt.covx", "pt.covy","pt.covz","pt.covxy"};
            wbytes=snprintf(dp,rem,"%*s %*s[%d]   %.3lf,%s,%.3lf,%.3lf,%.3lf,%.3lf,%.3lf,%.3lf,%.3lf\n",indent,(indent>0?" ":""),wkey-3," ",i,
                            est->time,est_labels[i],
                            est->x,est->y,est->z,
                            est->cov[0],est->cov[1],est->cov[2],est->cov[3]);

            rem-=(wbytes-1);
            dp+=wbytes;
        }

        wbytes=snprintf(dp,rem,"%*s %*s\n",indent,(indent>0?" ":""), wkey,"Bias Estimates:");
        rem-=(wbytes-1);
        dp+=wbytes;
        trnu_estimate_t *ept = &update->est[TRNU_EST_PT];
        trnu_estimate_t *emle = &update->est[TRNU_EST_MLE];
        trnu_estimate_t *emmse = &update->est[TRNU_EST_MMSE];
        trnu_estimate_t *offset = &update->est[TRNU_EST_OFFSET];
        trnu_estimate_t *last_good = &update->est[TRNU_EST_LAST_GOOD];
        wbytes=snprintf(dp,rem,"%*s %*s %.3lf,%.3lf,%.3lf\n",indent,(indent>0?" ":""), wkey," OFFSET:",offset->x,offset->y,offset->z);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s %.3lf,%.3lf,%.3lf\n",indent,(indent>0?" ":""), wkey," LAST:",last_good->x,last_good->y,last_good->z);
        rem-=(wbytes-1);
        dp+=wbytes;
//       wbytes=snprintf(dp,rem,"%*s %*s %.3lf,%.3lf,%.3lf\n",indent,(indent>0?" ":""), wkey," MLE:",(emle->x-ept->x),(emle->y-ept->y),(emle->z-ept->z));
//        rem-=(wbytes-1);
//        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%*s %*s %.3lf,%.3lf,%.3lf\n",indent,(indent>0?" ":""), wkey,"MMSE:",(emmse->x-ept->x),(emmse->y-ept->y),(emmse->z-ept->z));
        rem-=(wbytes-1);
        dp+=wbytes;
        snprintf(dp,rem,"%*s %*s %.3lf,%.3lf,%.3lf\n",indent,(indent>0?" ":""), wkey," COV:",sqrt(emmse->cov[0]),sqrt(emmse->cov[1]),sqrt(emmse->cov[2]));

        retval=strlen(dest)+1;
    }
    return retval;
}

static int s_update_csv(trnu_pub_t *update, char *dest, int len)
{
    int retval=0;
    if(NULL!=update && NULL!=dest && len>0){
        int rem=len;
        char *dp=dest;

        int wbytes=snprintf(dp,rem,"%.3lf,",update->mb1_time);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%.3lf,",update->update_time);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%.3lf,",update->reinit_time);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%04X,",update->sync);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%d,",update->reinit_count);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%.3lf,",update->reinit_tlast);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%d,",update->filter_state);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%d,",update->success);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%hd,",update->is_converged);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%hd,",update->is_valid);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%d,",update->mb1_cycle);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%d,",update->ping_number);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%d,",update->n_con_seq);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%d,",update->n_con_tot);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%d,",update->n_uncon_seq);
        rem-=(wbytes-1);
        dp+=wbytes;
        wbytes=snprintf(dp,rem,"%d,",update->n_uncon_tot);
        rem-=(wbytes-1);
        dp+=wbytes;
        int i=0;
        for(i=0;i<5;i++){
            trnu_estimate_t *est = &update->est[i];
            wbytes=snprintf(dp,rem,"%.3lf,%.3lf,%.3lf,%.3lf,%.3lf,%.3lf,%.3lf,",
                            est->x,est->y,est->z,
                            est->cov[0],est->cov[1],est->cov[2],est->cov[3]);

            rem-=(wbytes-1);
            dp+=wbytes;
        }

//        trnu_estimate_t *ept = &update->est[TRNU_EST_PT];
//        trnu_estimate_t *emle = &update->est[TRNU_EST_MLE];
//        trnu_estimate_t *emmse = &update->est[TRNU_EST_MMSE];
//        trnu_estimate_t *offset = &update->est[TRNU_EST_OFFSET];
//        trnu_estimate_t *last_good = &update->est[TRNU_EST_LAST_GOOD];
//        wbytes=snprintf(dp,rem,"%.3lf,%.3lf,%.3lf,",(emle->x-ept->x),(emle->y-ept->y),(emle->z-ept->z));
//        rem-=(wbytes-1);
//        dp+=wbytes;
//        wbytes=snprintf(dp,rem,"%.3lf,%.3lf,%.3lf,",(emmse->x-ept->x),(emmse->y-ept->y),(emmse->z-ept->z));
//        rem-=(wbytes-1);
//        dp+=wbytes;
//        wbytes=snprintf(dp,rem,"%.3lf,%.3lf,%.3lf,",offset->x,offset->y,offset->z);
//        rem-=(wbytes-1);
//        dp+=wbytes;
//        wbytes=snprintf(dp,rem,"%.3lf,%.3lf,%.3lf,",last_good->x,last_good->y,last_good->z);
//        rem-=(wbytes-1);
//        dp+=wbytes;
//      snprintf(dp,rem,"%.3lf,%.3lf,%.3lf",sqrt(emmse->cov[0]),sqrt(emmse->cov[1]),sqrt(emmse->cov[2]));

        retval=strlen(dest)+1;
    }
    return retval;
}


static int s_update_hex(trnu_pub_t *update, char *dest, int len, bool pretty)
{
    int retval=0;
    if(NULL!=update && NULL!=dest && len>0){
        int rem=len;
        byte *bp=(byte *)update;
        char *dp=dest;
        int i=0;
        bool hdr=true;
        for(i=0;i<TRNU_PUB_BYTES;i++){
            if(pretty){
                if(hdr){
                    unsigned long ofs = ( bp>(byte *)update ? (bp-(byte *)update-1) : 0);
                    int wbytes=snprintf(dp,rem,"%08lx: ",ofs);
                    rem-=(wbytes-1);
                    dp+=wbytes;
                    hdr=false;
                }
                int wbytes=snprintf(dp,rem,"%02x%c",*bp,( (i>0) && (((i+1)%16)==0 ) ? '\n' : ' ') );
                rem-=(wbytes-1);
                dp+=wbytes;

                if(( (i>0) && ((i+1)%16)==0 ))hdr=true;

            }else{
            	int wbytes=snprintf(dp,rem,"%02x",*bp);
                rem-=(wbytes-1);
                dp+=wbytes;
            }
            bp++;

        }
        retval=strlen(dest)+1;

    }
    return retval;
}

int trnucli_update_str(trnu_pub_t *self, char **dest, int len, trnuc_fmt_t fmt)
{
    int retval=-1;

    if(NULL!=self){
        int olen=len;
        char *obuf=NULL;
        if(NULL==*dest){
            *dest=(char *)malloc(TRNUC_STR_LEN);
            memset(*dest,0,TRNUC_STR_LEN);
            olen=TRNUC_STR_LEN;
        }
        obuf=*dest;

        if(NULL!=obuf && olen>0){
            memset(obuf,0,olen);

            switch (fmt) {
                case TRNUC_FMT_PRETTY:
                    retval=s_update_pretty(self,obuf,olen,5);
                    break;
                case TRNUC_FMT_CSV:
                    retval=s_update_csv(self,obuf,olen);
                    break;
                case TRNUC_FMT_HEX:
                    retval=s_update_hex(self,obuf,olen,false);
                    break;
                case TRNUC_FMT_PRETTY_HEX:
                    retval=s_update_hex(self,obuf,olen,true);
                    break;

                default:
                    // unsupported format
                    break;
            }
        }
    }
    return retval;
}


#ifdef WITH_ASYNC_TRNU

#define TRNUCLI_TEST_LOG_NAME "trnuctx"
#define TRNUCLI_TEST_LOG_DESC "trnu ctx log"
#define TRNUCLI_TEST_LOG_DIR  "."
#define TRNUCLI_TEST_LOG_EXT  ".log"
const char *g_ctx_state_strings[CTX_STATES]={
    "STOPPED","DISCONNECTED", "CONNECTING", "LISTENING","INVALID"
};
const char *g_ctx_action_strings[]={
    "NOP","CONNECT", "LISTEN","DISCONNECT"
};


static void s_init_log(trnucli_ctx_t *ctx)
{

    // if enabled and not initialized...
    if( (ctx->log_opts&TRNU_LOG_EN)!=0 && ctx->log_id==MLOG_ID_INVALID ){
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


        sprintf(ctx->log_path,"%s//%s-%s-%0lx-%s",ctx->log_dir,ctx->log_name,session_date,((unsigned long )ctx),TRNUCLI_TEST_LOG_EXT);

        ctx->log_id = mlog_get_instance(ctx->log_path, ctx->log_cfg, ctx->log_name);


        mfile_flags_t flags = MFILE_RDWR|MFILE_APPEND|MFILE_CREATE;
        mfile_mode_t mode = MFILE_RU|MFILE_WU|MFILE_RG|MFILE_WG;

        mlog_open(ctx->log_id, flags, mode);
        mlog_tprintf(ctx->log_id,"*** trnuctx session start ***\n");
        mlog_tprintf(ctx->log_id,"log_id=[%d]\n",ctx->log_id);
    }
    return;
}

void *s_trnucli_thread_fn(void *arg)
{

    trnucli_ctx_t *ctx = (trnucli_ctx_t *)arg;
    ctx->stop=false;
    trnucli_disconnect(ctx->cli);
    ctx->state=CTX_CONNECTING;
    ctx->action=ACT_NOP;
    ctx->listening_timer=0.0;
    ctx->connecting_timer=0.0;
    ctx->stats_log_timer=mtime_etime();

    mlog_tprintf(ctx->log_id,"host             %s\n",ctx->host);
    mlog_tprintf(ctx->log_id,"port             %d\n",ctx->port);
    mlog_tprintf(ctx->log_id,"update_fn        %p\n",ctx->cli->update_fn);
    mlog_tprintf(ctx->log_id,"hbeat_to_sec     %.3lf\n",ctx->hbeat_to_sec);
    mlog_tprintf(ctx->log_id,"listen_to_ms     %.3lf\n",ctx->listen_to_ms);
    mlog_tprintf(ctx->log_id,"enodata_delay_ms %"PRIu32"\n",ctx->enodata_delay_ms);
    mlog_tprintf(ctx->log_id,"erecon_delay_ms  %"PRIu32"\n",ctx->erecon_delay_ms);
    mlog_tprintf(ctx->log_id,"recon_to_sec     %.3lf\n",ctx->recon_to_sec);
    mlog_tprintf(ctx->log_id,"stats_log_sec    %.3lf\n",ctx->stats_log_sec);

    while(!ctx->stop){

        ctx->stats.n_cycle++;
        ctx->stats.t_session=mtime_etime()-ctx->session_timer;

        switch (ctx->state) {
            case CTX_CONNECTING:
                if(ctx->listening_timer>0.0)
                ctx->stats.t_listening+=mtime_etime()-ctx->listening_timer;
                ctx->listening_timer=0.0;

                if(ctx->connecting_timer==0.0)
                ctx->connecting_timer=mtime_etime();
                ctx->stats.t_connecting+=mtime_etime()-ctx->connecting_timer;
                ctx->connecting_timer=mtime_etime();

                ctx->action=ACT_CONNECT;
                break;
            case CTX_LISTENING:
                if(ctx->connecting_timer>0.0)
                ctx->stats.t_connecting+=mtime_etime()-ctx->connecting_timer;
                ctx->connecting_timer=0.0;

                if(ctx->listening_timer==0.0)
                ctx->listening_timer=mtime_etime();
                ctx->stats.t_listening+=mtime_etime()-ctx->listening_timer;
                ctx->listening_timer=mtime_etime();


                ctx->action=ACT_LISTEN;
                break;
            case CTX_STOPPED:
                ctx->action=ACT_NOP;
                break;
            default:
                fprintf(stderr,"ERR - illegal state[%d]\n",ctx->state);
                    mlog_tprintf(ctx->log_id,"ERR - illegal state[%d]\n",ctx->state);
                ctx->action=ACT_NOP;
                ctx->stop=true;
                break;
        }

        if( (ctx->stop==false) && (ctx->action==ACT_CONNECT) ){
            ctx->state=CTX_CONNECTING;
            int test = trnucli_connect(ctx->cli,ctx->host,ctx->port);
            if(test==0){
                double enow =mtime_etime();
                ctx->rc_timer=enow;
                ctx->hb_timer=enow;
                ctx->state=CTX_LISTENING;
                ctx->action=ACT_NOP;
                ctx->stats.n_connect++;
                ctx->reconnect=false;
                mlog_tprintf(ctx->log_id,"connected\n");
            }else{
                mlog_tprintf(ctx->log_id,"connect failed [%d/%s]\n",errno,strerror(errno));
                ctx->stats.n_econnect++;
                if(ctx->erecon_delay_ms>0)
                mtime_delay_ms(ctx->erecon_delay_ms);
            }
        } // CONNECT

        if((ctx->stop==false) && (ctx->action==ACT_LISTEN) ){

            // begin critical section
            mthread_mutex_lock(ctx->mtx);

            // set socket timeout/blocking if enabled
            if(ctx->listen_to_ms>0){
                TRNUC_MSET(&ctx->cli->flags,TRNUC_BLK_LISTEN);
                struct timeval tv;
                tv.tv_sec = ctx->listen_to_ms/1000;
                tv.tv_usec = (ctx->listen_to_ms%1000)*1000;
                if(msock_set_opt(ctx->cli->trnu->sock,SO_RCVTIMEO,&tv,sizeof(tv))!=0){
                    fprintf(stderr,"setopt ERR [%d/%s]\n",errno,strerror(errno));
                }
            }

             int test=trnucli_listen(ctx->cli,false);

            if(test==0 && NULL!=ctx->cli->update_fn){
            mthread_mutex_unlock(ctx->mtx);
                test=ctx->cli->update_fn(ctx->cli->update);
            mthread_mutex_lock(ctx->mtx);
            }

            // restore socket timeout if enabled
            if(ctx->listen_to_ms>0){
                struct timeval tv;
                tv.tv_sec = 0;
                tv.tv_usec = 0;
                if(msock_set_opt(ctx->cli->trnu->sock,SO_RCVTIMEO,&tv,sizeof(tv))!=0){
                    fprintf(stderr,"setopt ERR [%d/%s]\n",errno,strerror(errno));
                }
                TRNUC_MCLR(&ctx->cli->flags,TRNUC_BLK_LISTEN);
            }

            if(test==0){

                // update time of last update
                ctx->update_trx=mtime_etime();
                // copy update to output buffer
                memcpy(ctx->update,ctx->cli->update,sizeof(trnu_pub_t));

                // update reconnect timer
                ctx->rc_timer=mtime_etime();
                ctx->stats.n_update++;
                ctx->new_count++;
//                mlog_tprintf(ctx->log_id,"updates[%8d] cyc[%8d] elist[%8d] htime[%.3lf]\n",ctx->stats.n_update,ctx->stats.n_cycle,ctx->stats.n_elisten,ctx->update->update_time);
                mlog_tprintf(ctx->log_id,"upd/cyc/elist/tsvr,%d,%d,%d,%.3lf\n",ctx->stats.n_update,ctx->stats.n_cycle,ctx->stats.n_elisten,ctx->update->update_time);

                // end critical section
                mthread_mutex_unlock(ctx->mtx);

            }else{
                // end critical section (before delay)
                mthread_mutex_unlock(ctx->mtx);
                mtime_delay_ms(ctx->enodata_delay_ms);
                ctx->stats.n_elisten++;
           }

            if(ctx->hbeat_to_sec>0.0 && (mtime_etime()-ctx->hb_timer)>=ctx->hbeat_to_sec){
                mlog_tprintf(ctx->log_id,"hb_timer expired,%d\n",ctx->stats.n_hbeat);
                // heartbeat timer expired
                trnucli_hbeat(ctx->cli);
                ctx->hb_timer=mtime_etime();
                ctx->stats.n_hbeat++;
            }

            if(ctx->recon_to_sec>0.0 && (mtime_etime()-ctx->rc_timer)>=ctx->recon_to_sec){
                mlog_tprintf(ctx->log_id,"rc_timer expired,%d\n",ctx->stats.n_rcto);
                // reconnect timer expired
                ctx->state=CTX_CONNECTING;
                ctx->action=ACT_NOP;
                ctx->rc_timer=mtime_etime();
                ctx->stats.n_rcto++;
                ctx->stats.n_disconnect++;
            }

            if(ctx->reconnect){
                mlog_tprintf(ctx->log_id,"rc_req,%d\n",ctx->stats.n_rcto);
                // reset flag set via API
                ctx->state=CTX_CONNECTING;
                ctx->action=ACT_NOP;
                ctx->rc_timer=mtime_etime();
                ctx->stats.n_disconnect++;
            }
        } // LISTEN

        if(ctx->stats_log_sec>0.0 && (mtime_etime()-ctx->stats_log_timer)>ctx->stats_log_sec){
            // stats log interval timer expired
            // log stats and reset interval timer
            trnucli_ctx_stat_log(ctx);
            ctx->stats_log_timer=mtime_etime();
        }
    }// while !stop

    ctx->state=CTX_STOPPED;

    mlog_tprintf(ctx->log_id,"worker stop requested\n");
    // log stats
    trnucli_ctx_stat_log(ctx);

    mlog_tprintf(ctx->log_id,"disconnecting from host\n");
    trnucli_disconnect(ctx->cli);

    ctx->status=0;

    return (void *)(&ctx->status);
}

trnucli_ctx_t *trnucli_ctx_new_dfl(char *host, int port, update_callback_fn update_fn, double hbeat_to_sec, double recon_to_sec)
{
    return trnucli_ctx_newl(host, port, update_fn, hbeat_to_sec,
                            TRNUC_LISTEN_TO_MSEC_DFL,
                            TRNUC_ENODATA_DEL_MSEC_DFL,
                            TRNUC_ERECON_DEL_MSEC_DFL,
                            recon_to_sec,
                            true);
}

trnucli_ctx_t *trnucli_ctx_new(char *host, int port, update_callback_fn update_fn, double hbeat_to_sec, uint32_t listen_to_ms, uint32_t enodata_delay_ms, uint32_t erecon_delay_ms, double recon_to_sec)
{
    return trnucli_ctx_newl(host, port, update_fn, hbeat_to_sec, listen_to_ms, enodata_delay_ms, erecon_delay_ms, recon_to_sec, true);
}

trnucli_ctx_t *trnucli_ctx_newl(char *host, int port, update_callback_fn update_fn, double hbeat_to_sec, uint32_t listen_to_ms, uint32_t enodata_delay_ms, uint32_t erecon_delay_ms, double recon_to_sec, trnucli_logopt_t log_opts)
{
    trnucli_ctx_t *instance  = (trnucli_ctx_t *)malloc(sizeof(trnucli_ctx_t));
    if(NULL!=instance){
        memset(instance,0,sizeof(trnucli_ctx_t));
        instance->host=(NULL!=host ? strdup(host) : "localhost");
        instance->port=port;
        instance->cli=trnucli_new(update_fn,0,hbeat_to_sec);
        instance->worker=mthread_thread_new();
        instance->mtx=mthread_mutex_new();
        instance->update=(trnu_pub_t *)malloc(sizeof(trnu_pub_t));
        memset(instance->update,0,sizeof(trnu_pub_t));

        instance->listen_to_ms=listen_to_ms;
        instance->enodata_delay_ms=enodata_delay_ms;
        instance->recon_to_sec=recon_to_sec;
        instance->erecon_delay_ms=erecon_delay_ms;
        instance->hbeat_to_sec=hbeat_to_sec;

        instance->log_opts=log_opts;
        instance->log_cfg=mlog_config_new(ML_TFMT_ISO1806,ML_DFL_DEL,ML_MONO|ML_NOLIMIT,ML_FILE,0,0,0);
        instance->log_id=MLOG_ID_INVALID;
        instance->log_name=strdup(TRNUCLI_TEST_LOG_NAME);
        instance->log_dir=strdup(TRNUCLI_TEST_LOG_DIR);
        instance->log_path=(char *)malloc(512);
        memset(instance->log_path,0,512);

        instance->rc_timer=0.0;
        instance->hb_timer=0.0;
        instance->update_trx=0.0;
        instance->new_count=0;
        instance->status=-1;
        instance->state=CTX_STOPPED;
        instance->stop=false;

        s_init_log(instance);
    }
    return instance;
}//trnucli_ctx_new

void trnucli_ctx_destroy(trnucli_ctx_t **pself)
{
    if(NULL!=pself){
        if(NULL!=*pself){
            trnucli_ctx_t *self=*pself;

            // stop thread
            if(!self->stop || self->state!=CTX_STOPPED){
            	trnucli_ctx_stop(self);
            }

            // release thread
            mthread_thread_destroy(&self->worker);
            // release mutex
            mthread_mutex_destroy(&self->mtx);

            // release trnu_cli intance
            trnucli_destroy(&self->cli);
            if(NULL!=self->host)
                free(self->host);
            if(NULL!=self->update)
                free(self->update);

            if(NULL!=self->log_name)
                free(self->log_name);
            if(NULL!=self->log_dir)
                free(self->log_dir);
            if(NULL!=self->log_path)
                free(self->log_path);

            // close log
            mlog_delete_instance(self->log_id);
            mlog_config_destroy(&self->log_cfg);

            // release instance
            free(self);
            *pself=NULL;
        }
    }
    return;
}// trnucli_ctx_destroy

int trnucli_ctx_start(trnucli_ctx_t *self)
{
    int retval=-1;
    if(NULL!=self){

        // stop if running
        if(self->state!=CTX_STOPPED){
        	trnucli_ctx_stop(self);
        }
        self->session_timer=mtime_etime();
        mlog_tprintf(self->log_id,"start_time,%.3lf\n",self->session_timer);

        retval = mthread_thread_start(self->worker,s_trnucli_thread_fn,(void *)self);

    }
    return retval;
}//trnucli_ctx_start

int trnucli_ctx_stop(trnucli_ctx_t *self)
{
    int retval=-1;
    if(NULL!=self){
        if(self->state!=CTX_STOPPED){
            if(!self->stop){
                // set stop flag
                self->stop=true;
                // wait for thread to exit
                mlog_tprintf(self->log_id,"stop flag set - pending thread exit\n");
                if(mthread_thread_join(self->worker)==0){
                    retval=0;
                    self->state=CTX_STOPPED;
                    mlog_tprintf(self->log_id,"thread stopped\n");
                    double now=mtime_etime();
                    self->stats.t_session=now-self->session_timer;
                    mlog_tprintf(self->log_id,"stop_time,%.3lf elapsed[%.3lf]\n",now,self->stats.t_session);
                    mlog_tprintf(self->log_id,"*** trnuctx session end ***\n");
                }else{
                    mlog_tprintf(self->log_id,"ERR - thread join failed\n");
                }
            }
        }// else already stopped
    }
    return retval;
}//trnucli_ctx_stop

int trnucli_ctx_set_callback(trnucli_ctx_t *self, update_callback_fn func)
{
    int retval=-1;
    if(NULL!=self){
        mthread_mutex_lock(self->mtx);
        retval=trnucli_set_callback(self->cli,func);
        mthread_mutex_unlock(self->mtx);
    }
    return retval;
}//trnucli_ctx_set_callback

int trnucli_ctx_set_stats_log_period(trnucli_ctx_t *self, double interval_sec)
{
    int retval=-1;
    if(NULL!=self){
        mthread_mutex_lock(self->mtx);
        self->stats_log_sec=interval_sec;
        mthread_mutex_unlock(self->mtx);
    }
    return retval;
}//trnucli_ctx_set_stats_log_period

int trnucli_ctx_last_update(trnucli_ctx_t *self, trnu_pub_t *dest, double *r_age)
{
    int retval=-1;
    if(NULL!=self && NULL!=dest){
        //TODO fix mutex deadlock
        mthread_mutex_lock(self->mtx);
        memcpy(dest,self->update,sizeof(trnu_pub_t));
        self->new_count=0;
        if(NULL!=r_age){
            // optionally set age
            *r_age=trnucli_ctx_update_arrage(self);
        }
        retval=0;
        mthread_mutex_unlock(self->mtx);
    }
    return retval;
}//trnucli_ctx_last_update

double trnucli_ctx_update_arrtime(trnucli_ctx_t *self)
{
    double retval=-1.0;
    if(NULL!=self){
        mthread_mutex_lock(self->mtx);
        retval = self->update_trx;
        mthread_mutex_unlock(self->mtx);
    }
    return retval;
}//trnucli_ctx_update_arrtime

double trnucli_ctx_update_arrage(trnucli_ctx_t *self)
{
    double retval=-1.0;
    if(NULL!=self && self->update_trx>0.0){
        mthread_mutex_lock(self->mtx);
        retval = mtime_etime()-self->update_trx;
        mthread_mutex_unlock(self->mtx);
    }
    return retval;
}//trnucli_ctx_update_arrage

double trnucli_ctx_update_mb1time(trnucli_ctx_t *self)
{
    double retval=-1.0;
    if(NULL!=self && NULL!=self->update){
        mthread_mutex_lock(self->mtx);
        retval = self->update->mb1_time;
        mthread_mutex_unlock(self->mtx);
    }
    return retval;
}//trnucli_ctx_update_mb1time

double trnucli_ctx_update_mb1age(trnucli_ctx_t *self)
{
    double retval=-1.0;
    if(NULL!=self && NULL!=self->update){
        mthread_mutex_lock(self->mtx);
        retval = mtime_etime()-self->update->mb1_time;
        mthread_mutex_unlock(self->mtx);
    }
    return retval;
}//trnucli_ctx_update_mb1age

double trnucli_ctx_update_hosttime(trnucli_ctx_t *self)
{
    double retval=-1.0;
    if(NULL!=self && NULL!=self->update){
        mthread_mutex_lock(self->mtx);
        retval = self->update->update_time;
        mthread_mutex_unlock(self->mtx);
    }
    return retval;
}//trnucli_ctx_update_hosttime

double trnucli_ctx_update_hostage(trnucli_ctx_t *self)
{
    double retval=-1.0;
    if(NULL!=self && NULL!=self->update){
        mthread_mutex_lock(self->mtx);
        retval = mtime_etime()-self->update->update_time;
        mthread_mutex_unlock(self->mtx);
    }

    return retval;
}//trnucli_ctx_update_hostage

uint32_t trnucli_ctx_new_count(trnucli_ctx_t *self)
{
    uint32_t retval=0;
    if(NULL!=self && NULL!=self->update){
        mthread_mutex_lock(self->mtx);
        retval = self->new_count;
        mthread_mutex_unlock(self->mtx);
    }

    return retval;
}//trnucli_ctx_new_count

int trnucli_ctx_reset_trn(trnucli_ctx_t *self)
{
    int retval=-1;
    if(NULL!=self){
        mthread_mutex_lock(self->mtx);
        retval=trnucli_reset_trn(self->cli);
        mthread_mutex_unlock(self->mtx);
    }
    return retval;
}//trnucli_ctx_reset_trn

int trnucli_ctx_reconnect(trnucli_ctx_t *self)
{
    int retval=-1;
    if(NULL!=self){
        mthread_mutex_lock(self->mtx);
       // set reconnect flag
        self->reconnect=true;
        mthread_mutex_unlock(self->mtx);
        retval=0;
    }
    return retval;
}//trnucli_ctx_reconnect

int trnucli_ctx_isconnected(trnucli_ctx_t *self)
{
    int retval=0;
    if(NULL!=self){
        mthread_mutex_lock(self->mtx);
        retval=(self->state==CTX_LISTENING ? -1 : 0);
        mthread_mutex_unlock(self->mtx);
    }
    return retval;
}//trnucli_ctx_isconnected

trnucli_state_t trnucli_ctx_state(trnucli_ctx_t *self)
{
    trnucli_state_t retval=CTX_INVALID;

    if(NULL!=self && self->state>=0 && self->state<CTX_STATES){
        mthread_mutex_lock(self->mtx);
        retval=self->state;
        mthread_mutex_unlock(self->mtx);
    }

    return retval;
}// trnucli_ctx_state_str

const char *trnucli_ctx_state_str(trnucli_ctx_t *self)
{
    const char *retval=g_ctx_state_strings[CTX_INVALID];
    if(NULL!=self && self->state>=0 && self->state<CTX_STATES){
        // TODO: fix mutex deadlock
        mthread_mutex_lock(self->mtx);
        retval=g_ctx_state_strings[self->state];
        mthread_mutex_unlock(self->mtx);
    }
    return retval;
}// trnucli_ctx_state_str

int trnucli_ctx_stats(trnucli_ctx_t *self, trnucli_stats_t **pdest)
{
    int retval=-1;

    if(NULL!=self && NULL!=pdest){
        trnucli_stats_t *dest=*pdest;

        if(NULL == *pdest){
            // allocate destination instance if one not provided
            trnucli_stats_t *instance = (trnucli_stats_t *)malloc(sizeof(trnucli_stats_t));
            if(NULL!=instance){
                memset(instance,0,sizeof(trnucli_stats_t));
                dest = instance;
                *pdest=instance;
            }
        }

        if(NULL!=dest){
            // if valid destination, copy stats
            mthread_mutex_lock(self->mtx);
            memcpy(dest,&self->stats,sizeof(trnucli_stats_t));
            mthread_mutex_unlock(self->mtx);
            retval=0;
        }
    }
    return retval;
}// trnucli_ctx_stats

int trnucli_ctx_show(trnucli_ctx_t *self,bool verbose, int indent)
{
    int retval=0;
    if(NULL!=self){
        int wkey=TRNUCLI_SHOW_WKEY;
        int wval=TRNUCLI_SHOW_WVAL;

        if(verbose)
            retval+=fprintf(stderr,"%*s %*s  %*p\n",indent,(indent>0?" ":""), wkey,"self",wval,self);
        retval+=fprintf(stderr,"%*s %*s  %*s\n",indent,(indent>0?" ":""), wkey,"host",wval,self->host);
        retval+=fprintf(stderr,"%*s %*s  %*d\n",indent,(indent>0?" ":""), wkey,"port",wval,self->port);
        retval+=fprintf(stderr,"%*s %*s  %*s [%d]\n",indent,(indent>0?" ":""),wkey,"state",wval,trnucli_ctx_state_str(self), self->state);
        retval+=fprintf(stderr,"%*s %*s  %*.3lf\n",indent,(indent>0?" ":""),wkey,"update_t",wval,self->update_trx);
        retval+=fprintf(stderr,"%*s %*s  %*.3lf\n",indent,(indent>0?" ":""),wkey,"recon_to_sec",wval,self->recon_to_sec);
        retval+=fprintf(stderr,"%*s %*s  %*"PRIu32"\n",indent,(indent>0?" ":""),wkey,"listen_to_ms",wval,self->listen_to_ms);
        retval+=fprintf(stderr,"%*s %*s  %*"PRIu32"\n",indent,(indent>0?" ":""),wkey,"enodata_delay_ms",wval,self->enodata_delay_ms);
        retval+=fprintf(stderr,"%*s %*s  %*"PRIu32"\n",indent,(indent>0?" ":""),wkey,"erecon_delay_ms",wval,self->erecon_delay_ms);
        retval+=fprintf(stderr,"%*s %*s  %*.3lf\n",indent,(indent>0?" ":""),wkey,"hbeat_to_sec",wval,self->hbeat_to_sec);
        retval+=fprintf(stderr,"%*s %*s  %*.3lf\n",indent,(indent>0?" ":""),wkey,"rc_timer",wval,self->rc_timer );
        retval+=fprintf(stderr,"%*s %*s  %*.3lf\n",indent,(indent>0?" ":""),wkey,"hb_timer",wval,self->hb_timer );
        retval+=fprintf(stderr,"%*s %*s  %*"PRIu32"\n",indent,(indent>0?" ":""),wkey,"new_count",wval,self->new_count );
        if(verbose){
            retval+=fprintf(stderr,"%*sstats:\n",indent,(indent>0?" ":"") );
            trnucli_ctx_stat_show(&self->stats, verbose, indent+1);
        }
    }
    return retval;
}//trnucli_ctx_show

int trnucli_ctx_stat_show(trnucli_stats_t *self,bool verbose, int indent)
{
    int retval=0;
    if(NULL!=self){
        int wkey=TRNUCLI_SHOW_WKEY;
        int wval=TRNUCLI_SHOW_WVAL;
        if(verbose)
            retval+=fprintf(stderr,"%*s %*s  %*p\n",indent,(indent>0?" ":""), wkey,"self",wval,self);
        retval+=fprintf(stderr,"%*s %*s  %*d\n",indent,(indent>0?" ":""), wkey,"n_cycle",wval,self->n_cycle);
        retval+=fprintf(stderr,"%*s %*s  %*d\n",indent,(indent>0?" ":""), wkey,"n_update",wval,self->n_update);
        retval+=fprintf(stderr,"%*s %*s  %*d\n",indent,(indent>0?" ":""), wkey,"n_connect",wval,self->n_connect);
        retval+=fprintf(stderr,"%*s %*s  %*d\n",indent,(indent>0?" ":""), wkey,"n_disconnect",wval,self->n_disconnect);
        retval+=fprintf(stderr,"%*s %*s  %*d\n",indent,(indent>0?" ":""), wkey,"n_reset",wval,self->n_reset);
        retval+=fprintf(stderr,"%*s %*s  %*d\n",indent,(indent>0?" ":""), wkey,"n_hbeat",wval,self->n_hbeat);
        retval+=fprintf(stderr,"%*s %*s  %*d\n",indent,(indent>0?" ":""), wkey,"n_rcto",wval,self->n_rcto);
        retval+=fprintf(stderr,"%*s %*s  %*d\n",indent,(indent>0?" ":""), wkey,"n_elisten",wval,self->n_elisten);
        retval+=fprintf(stderr,"%*s %*s  %*d\n",indent,(indent>0?" ":""), wkey,"n_econnect",wval,self->n_econnect);
        retval+=fprintf(stderr,"%*s %*s  %*.3lf\n",indent,(indent>0?" ":""), wkey,"t_session",wval,self->t_session);
        retval+=fprintf(stderr,"%*s %*s  %*.3lf\n",indent,(indent>0?" ":""), wkey,"t_connecting",wval,self->t_connecting);
        retval+=fprintf(stderr,"%*s %*s  %*.3lf\n",indent,(indent>0?" ":""), wkey,"t_listening",wval,self->t_listening);
    }
    return retval;
}//trnucli_ctx_stat_show

int trnucli_ctx_stat_log(trnucli_ctx_t *self)
{
    int retval=0;
    if(NULL!=self){
        retval+=mlog_tprintf(self->log_id,"e,%s,%d\n","n_cycle",self->stats.n_cycle);
        retval+=mlog_tprintf(self->log_id,"e,%s,%d\n","n_update",self->stats.n_update);
        retval+=mlog_tprintf(self->log_id,"e,%s,%d\n","n_connect",self->stats.n_connect);
        retval+=mlog_tprintf(self->log_id,"e,%s,%d\n","n_disconnect",self->stats.n_disconnect);
        retval+=mlog_tprintf(self->log_id,"e,%s,%d\n","n_reset",self->stats.n_reset);
        retval+=mlog_tprintf(self->log_id,"e,%s,%d\n","n_hbeat",self->stats.n_hbeat);
        retval+=mlog_tprintf(self->log_id,"e,%s,%d\n","n_rcto",self->stats.n_rcto);
        retval+=mlog_tprintf(self->log_id,"e,%s,%d\n","n_elisten",self->stats.n_elisten);
        retval+=mlog_tprintf(self->log_id,"e,%s,%d\n","n_econnect",self->stats.n_econnect);
        retval+=mlog_tprintf(self->log_id,"t,%s,%.3lf\n","t_session",self->stats.t_session);
        retval+=mlog_tprintf(self->log_id,"t,%s,%.3lf\n","t_connecting",self->stats.t_connecting);
        retval+=mlog_tprintf(self->log_id,"t,%s,%.3lf\n","t_listening",self->stats.t_listening);
    }
    return retval;
}//trnucli_ctx_stat_log


#endif
