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

trnucli_t *trnucli_new(update_callback_fn update_fn, trnuc_flags_t flags, double hbto)
{
    trnucli_t *instance=(trnucli_t *)malloc(sizeof(trnucli_t));
    if(NULL!=instance){
        memset(instance,0,sizeof(trnucli_t));
        instance->trnu = msock_connection_new();
        instance->update = NULL;
        instance->update_fn = update_fn;
        instance->hbto = hbto;
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
            int retries=3;
            while(retries>0 && retval!=0){
                if( (test=msock_sendto(self->trnu->sock,NULL,(byte *)PROTO_TRNU_CON,(strlen(PROTO_TRNU_CON)+1),0))>0){
                    PDPRINT((stderr,"CON msg send OK [%d]\n",test));
                    byte ack[8]={0};
                    int rx_retries=5;
                    while(rx_retries>0){
                        mtime_delay_ms(250);
    //                    msock_set_blocking(self->trnu->sock,true);
                        if( (test=msock_recv(self->trnu->sock,ack,8,0))>0){
                            PDPRINT((stderr,"ACK OK [%d/%s]\n",test,ack));
                            retval=0;
                            break;
                        }
                        rx_retries--;
                    }
                }
                retries--;
            }
        }else{
            PTRACE();
            PDPRINT((stderr,"CON failed [%d]\n",test));
        }
    }else{PTRACE();}

    return retval;
}// end function trnucli_connect

int trnucli_disconnect(trnucli_t *self)
{
    int retval=-1;

    msock_set_blocking(self->trnu->sock,false);
    char msg[8]={0};
    sprintf(msg,PROTO_TRNU_DIS);
    int32_t sret=msock_sendto(self->trnu->sock,NULL,(byte *)msg,4,0);
    if(sret>0){
        retval=0;
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

int trnucli_listen(trnucli_t *self)
{
    int retval=-1;

    if(NULL!=self){
        if(NULL==self->update){
            self->update=(trnu_pub_t *)malloc(TRNU_PUB_BYTES);
        }
        if(NULL!=self->update){
            memset(self->update,0,TRNU_PUB_BYTES);
            msock_set_blocking(self->trnu->sock,TRNUC_BLK_LISTEN(self->flags));
            int32_t rret=msock_recv(self->trnu->sock,(byte *)self->update,TRNU_PUB_BYTES,0);
            if(rret>0){
                retval=0;
                if(NULL!=self->update_fn){
                    retval= self->update_fn(self->update);
                }else{
                    // no handler specified
                }
            }
        }else{
            // could not alloc
        }
    }

    return retval;
}

int trnucli_reset_trn(trnucli_t *self)
{
    int retval=-1;

    int test=-1;
    if( (test=msock_sendto(self->trnu->sock,NULL,(byte *)PROTO_TRNU_RST,(strlen(PROTO_TRNU_RST)+1),0))>0){
        PDPRINT((stderr,"reset msg OK [%d]\n",test));
        byte ack[8]={0};
        retval=1;
        if( (test=msock_recv(self->trnu->sock,ack,8,0))>0){
            PDPRINT((stderr,"ACK OK [%d/%s]\n",test,ack));
            retval=0;
        }
    }else{PTRACE();}

    return retval;
}// end trnucli_reset_trn trnucli_connect

int trnucli_hbeat(trnucli_t *self)
{
    int retval=-1;

    int test=-1;
    if( (test=msock_sendto(self->trnu->sock,NULL,(byte *)PROTO_TRNU_HBT,(strlen(PROTO_TRNU_HBT)+1),0))>0){
        PDPRINT((stderr,"hbeat msg OK [%d]\n",test));
        byte ack[8]={0};
        retval=1;
        if( (test=msock_recv(self->trnu->sock,ack,8,0))>0){
            PDPRINT((stderr,"ACK OK [%d/%s]\n",test,ack));
            retval=0;
        }
    }else{PTRACE();}

    return retval;
}// end trnucli_reset_trn trnucli_connect


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

static int s_update_hex(trnu_pub_t *update, char *dest, int len, bool pretty)
{
    int retval=0;
    if(NULL!=update && NULL!=dest && len>0){
        int rem=len;
        int wbytes=0;
        byte *bp=(byte *)update;
        char *dp=dest;
        int i=0;
        bool hdr=true;
        for(i=0;i<TRNU_PUB_BYTES;i++){
            if(pretty){
                if(hdr){
                    unsigned long ofs = ( bp>(byte *)update ? (bp-(byte *)update-1) : 0);
                    wbytes=snprintf(dp,rem,"%08lx: ",ofs);
                    rem-=(wbytes-1);
                    dp+=wbytes;
                    hdr=false;
                }
                wbytes=snprintf(dp,rem,"%02x%c",*bp,( (i>0) && (((i+1)%16)==0 ) ? '\n' : ' ') );
                rem-=(wbytes-1);
                dp+=wbytes;

                if(( (i>0) && ((i+1)%16)==0 ))hdr=true;

            }else{
            	wbytes=snprintf(dp,rem,"%02x",*bp);
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
            olen=TRNUC_STR_LEN;
        }
        obuf=*dest;

        if(NULL!=obuf && olen>0){
            memset(obuf,0,olen);

            switch (fmt) {
                case TRNUC_FMT_PRETTY:
                    retval=s_update_pretty(self,obuf,olen,0);
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
