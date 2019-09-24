///
/// @file trn_cli.c
/// @authors k. headley
/// @date 10 jul 2019

/// TRN client

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
#include "trn_cli.h"

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
static int32_t s_trncli_send_recv(trncli_t *self, byte *msg, int32_t len, bool block);

/////////////////////////
// Imports
/////////////////////////

/////////////////////////
// Module Global Variables
/////////////////////////

/////////////////////////
// Function Definitions
/////////////////////////

static int32_t s_trncli_send_recv(trncli_t *self, byte *msg, int32_t len, bool block)
{
    int32_t retval=-1;
    int32_t sret=-1;
    int32_t rret=-1;
    
    msock_set_blocking(self->trn->sock,false);
    if((sret=msock_send(self->trn->sock,msg,len))>0){
        msock_set_blocking(self->trn->sock,block);
        if((rret=msock_recv(self->trn->sock,msg,TRNW_MSG_SIZE,0))>0){
            retval=rret;
        }
    }

    PDPRINT((stderr,"%s - send ret[%"PRId32"]\n",__FUNCTION__,sret));
    PDPRINT((stderr,"%s - recv ret[%"PRId32"]\n",__FUNCTION__,rret));

    return retval;
}// end function s_trncli_send_recv

trncli_t *trncli_new(trncli_type_id type, long int utm_zone)
{
    trncli_t *instance=(trncli_t *)malloc(sizeof(trncli_t));
    if(NULL!=instance){
        memset(instance,0,sizeof(trncli_t));
        instance->type = type;
        instance->measurement = NULL;
        instance->utm_zone=utm_zone;
        instance->trn = msock_connection_new();
    }
    
    return instance;
}// end function trncli_new

void trncli_destroy(trncli_t **pself)
{
    if(NULL!=pself){
        trncli_t *self=(trncli_t *)(*pself);
        if(NULL!=self->trn){
            msock_connection_destroy(&self->trn);
        }
        if(NULL!=self->measurement){
            wmeast_destroy(self->measurement);
        }
        free(self);
        *pself=NULL;
    }
}// end function trncli_destroy

int trncli_connect(trncli_t *self, char *host, int port)
{
    int retval=-1;
    if( (self->trn->sock=msock_socket_new(host,port,ST_TCP))!=NULL ){
        msock_set_blocking(self->trn->sock,true);

        if ( msock_connect(self->trn->sock)==0) {
            retval=0;
        }
    }
    return retval;
}// end function trncli_connect

int trncli_disconnect(trncli_t *self)
{
    int retval=-1;
    byte *msg=NULL;
    int32_t mlen=0;
    if( (mlen=trnw_type_msg((char **)&msg,TRN_MSG_BYE))>0){
        
        retval=(s_trncli_send_recv(self, msg, mlen, true)>0?0:-1);

        free(msg);
        
    }
   return retval;
}// end function trncli_disconnect


int32_t trncli_init_server(trncli_t *self, trn_config_t *cfg)
{
    int32_t retval=-1;
    
    byte *msg=NULL;
    int32_t mlen=0;
    
    if( (mlen=trnw_init_msg((char **)&msg, cfg))>0){
#if defined(WITH_PDEBUG)
        fprintf(stderr,"TX - INIT:\n");
        trnw_msg_show((char *)msg,true,5);
#endif

        retval=s_trncli_send_recv(self, msg, mlen, true);

#if defined(WITH_PDEBUG)
        fprintf(stderr,"RX - INIT ret[%"PRId32"]\n",retval);
        trnw_msg_show((char *)msg,true,5);
#endif
        free(msg);
    }
    return retval;
}// end function trncli_init_server

int32_t trncli_update_measurement(trncli_t *self, wmeast_t *meas)
{
    int32_t retval=-1;
  
    byte *msg=NULL;
    int32_t mlen=0;
    if( (mlen=trnw_meas_msg((char **)&msg,meas,TRN_MSG_MEAS,TRN_SENSOR_MB))>0){
#if defined(WITH_PDEBUG)
        fprintf(stderr,"TX MEAS UPDATE MEAST:\n");
        wmeast_show(meas, true, 5);
        fprintf(stderr,"TX MEAS UPDATE MSG:\n");
        trnw_msg_show((char *)msg,true,5);
#endif
        retval=s_trncli_send_recv(self, msg, mlen, true);
        
#if defined(WITH_PDEBUG)
        fprintf(stderr,"RX MEAS UPDATE ret[%"PRId32"]\n",retval);
        trnw_msg_show((char *)msg,true,5);
#endif
        free(msg);
    }
    return retval;
}// end function trncli_update_measurement

int32_t trncli_update_motion(trncli_t *self, wposet_t *pose)
{
    int32_t retval=-1;
    
    byte *msg=NULL;
    int32_t mlen=0;
    if( (mlen=trnw_pose_msg((char **)&msg,pose,TRN_MSG_MOTN))>0){

#if defined(WITH_PDEBUG)
        fprintf(stderr,"TX MOTN UPDATE:\n");
        wposet_show(pose, true, 5);
        fprintf(stderr,"TX MOTN MSG:\n");
        trnw_msg_show((char *)msg, true, 5);
#endif
        retval=s_trncli_send_recv(self, msg, mlen, true);

#if defined(WITH_PDEBUG)
        fprintf(stderr,"RX MOTN UPDATE ret[%"PRId32"]\n",retval);
        trnw_msg_show((char *)msg,true,5);
#endif
        free(msg);
    }
    return retval;
}// end function trncli_update_motion

int32_t trncli_estimate_pose(trncli_t *self, wposet_t **pose, char msg_type)
{
    int32_t retval=-1;
    
    byte *msg=NULL;
    int32_t mlen=0;
    if( (mlen=trnw_type_msg((char **)&msg,msg_type))>0){
        
#if defined(WITH_PDEBUG)
        fprintf(stderr,"TX EST_POSE MSG [%c]:\n",msg_type);
        trnw_msg_show((char *)msg, true, 5);
#endif
        retval=s_trncli_send_recv(self, msg, mlen, true);
        wposet_msg_to_pose(pose, (char *)msg);

#if defined(WITH_PDEBUG)
        fprintf(stderr,"RX EST_POSE MSG [%c] ret[%"PRId32"]\n",msg_type,retval);
        trnw_msg_show((char *)msg,true,5);
        fprintf(stderr,"RX EST_POSE [%c]:\n",msg_type);
        wposet_show(*pose, true, 5);
#endif
        free(msg);
    }
    return retval;
}// end function trncli_update_motion

bool trncli_last_meas_valid(trncli_t *self)
{
    bool retval=false;
    
    byte *msg=NULL;
    int32_t mlen=0;
    ct_cdata_t *lmv_dat=NULL;

    if( (mlen=trnw_type_msg((char **)&msg,TRN_MSG_LAST_MEAS))>0){
        
#if defined(WITH_PDEBUG)
        fprintf(stderr,"TX LMV MSG:\n");
        trnw_msg_show((char *)msg, true, 5);
#endif

        retval=s_trncli_send_recv(self, msg, mlen, true);
        if( wcommst_msg_to_cdata(&lmv_dat,(char *)msg)==0 && NULL!=lmv_dat){
            retval = (lmv_dat->parameter==0?false:true);
        }
        
        wcommst_cdata_destroy(&lmv_dat);
        
#if defined(WITH_PDEBUG)
        fprintf(stderr,"RX LMV MSG ret[%"PRId32"]\n",retval);
        trnw_msg_show((char *)msg,true,5);
#endif
        
        free(msg);
    }
    return retval;
}

int32_t trncli_reinit_filter(trncli_t *self)
{
    int32_t retval=-1;
    
    byte *msg=NULL;
    int32_t mlen=0;
    if( (mlen=trnw_type_msg((char **)&msg,TRN_MSG_SET_FR))>0){
#if defined(WITH_PDEBUG)
        fprintf(stderr,"TX REINIT FILT:\n");
        trnw_msg_show((char *)msg,true,5);
#endif
        retval=s_trncli_send_recv(self, msg, mlen, true);

#if defined(WITH_PDEBUG)
        fprintf(stderr,"RX REINIT FILT ret[%"PRId32"]\n",retval);
        trnw_msg_show((char *)msg,true,5);
#endif
        free(msg);
    }
    return retval;
}// end function trncli_reinit_filter

int32_t trncli_ack_server(trncli_t *self)
{
    int32_t retval=-1;
    
    byte *msg=NULL;
    int32_t mlen=0;
    if( (mlen=trnw_ack_msg((char **)&msg))>0){
        retval=s_trncli_send_recv(self, msg, mlen, true);
        fprintf(stderr,"%s - ACK ret[%"PRId32"]\n",__FUNCTION__,retval);
        
        free(msg);
    }
    return retval;
}// end function trncli_ack_server

int trncli_send_update(trncli_t *self, mb1_t *src, wposet_t **pt_out, wmeast_t **mt_out)
{
    int retval=-1;
    
    if(NULL!=self && NULL!=src && NULL!=pt_out && NULL!=mt_out){
        int test=-1;
        if( (test=wmeast_mb1_to_meas(mt_out, src, self->utm_zone)) == 0){
            
            if( (test=wposet_mb1_to_pose(pt_out, src, self->utm_zone)) == 0){
                // must do motion update first if pt time <= mt time
   
                if(trncli_update_motion(self,*pt_out)>0){
                    if(trncli_update_measurement(self, *mt_out)>0){
                        retval=0;
                    }else{
                        fprintf(stderr,"trncli_update_measurement failed [%d]\n",test);
                        retval=errno;
                    }

                }else{
                    fprintf(stderr,"trncli_update_motion failed [%d]\n",test);
                    retval=errno;
                }
                
            }else{
                fprintf(stderr,"wposet_mb1_to_pose failed [%d]\n",test);
            }
        }else{
            fprintf(stderr,"wmeast_mb1_to_meas failed [%d]\n",test);
        }
    }
    
    return retval;
}

int trncli_get_bias_estimates(trncli_t *self, wposet_t *pt, pt_cdata_t **pt_out, pt_cdata_t **mle_out, pt_cdata_t **mse_out)
{
    int retval=-1;
    wposet_t *mle = NULL;
    wposet_t *mse = NULL;

    if(NULL!=self && NULL!=pt && NULL!=mle_out && NULL!=mse_out){
        uint32_t uret;
        if((  uret=trncli_estimate_pose(self, &mle, TRN_MSG_MLE))>0){
            if((uret=trncli_estimate_pose(self, &mse, TRN_MSG_MMSE))>0){
                retval=0;

            }else{
                fprintf(stderr,"trncli_estimate_pose failed [%u]\n",uret);
                retval=errno;
            }

        }else{
            fprintf(stderr,"trncli_estimate_pose failed [%u]\n",uret);
           retval=errno;
        }
     
        if( trncli_last_meas_valid(self)==true){
            
            wposet_pose_to_cdata(pt_out, pt);
            wposet_pose_to_cdata(mle_out, mle);
            wposet_pose_to_cdata(mse_out, mse);
            
//            fprintf(stderr,"Bias Estimates:\n");
//            fprintf(stderr,"MLE: [%.2lf,%.4lf,%.4lf,%.4lf]\n",mle_dat->time,
//                    (mle_dat->x-pt_dat->x),(mle_dat->y-pt_dat->y),(mle_dat->z-pt_dat->z));
//            fprintf(stderr,"MSE: [%.2lf,%.4lf,%.4lf,%.4lf]\n",mse_dat->time,
//                    (mse_dat->x-pt_dat->x),(mse_dat->y-pt_dat->y),(mse_dat->z-pt_dat->z));
//            fprintf(stderr,"COV: [%.2lf,%.2lf,%.2lf]\n",sqrt(mse_dat->covariance[0]),sqrt(mse_dat->covariance[2]),sqrt(mse_dat->covariance[5]));


        }else{
            fprintf(stderr,"Last Meas Invalid\n");
        }
        wposet_destroy(mle);
        wposet_destroy(mse);

    }
    
    return retval;
}

int trncli_mb1_to_meas(wmeast_t **dest, mb1_t *src, long int utmZone)
{
   return wmeast_mb1_to_meas(dest, src,utmZone);
}// end function trncli_mb1_to_meas

int trncli_cdata_to_pose(wposet_t **dest, pt_cdata_t *src)
{
    return wposet_cdata_to_pose(dest, src);
}// end function trncli_motion_to_pose

int trncli_cdata_to_meas(wmeast_t **dest, mt_cdata_t *src)
{
    return wmeast_cdata_to_meas(dest, src);
}// end function trncli_cdata_to_meas


