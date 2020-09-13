///
/// @file trnif_msg.c
/// @authors k. headley
/// @date 03 jul 2019

/// TRN netif message API

/// @sa doxygen-examples.c for more examples of Doxygen markup


/////////////////////////
// Terms of use
/////////////////////////
/*
 Copyright Information
 
 Copyright 2002-YYYY MBARI
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
#include "trnif_msg.h"
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
trn_sync_t g_trn_sync=0xCAFEBEEF;
const char *trnmsg_id_names[TRNIF_MSG_ID_COUNT]={
"TRNIF_INIT",
"TRNIF_MEAS",
"TRNIF_MOTN",
"TRNIF_MLE",
"TRNIF_MMSE",
"TRNIF_SET_MW",
"TRNIF_SET_FR",
"TRNIF_SET_IMA",
"TRNIF_SET_VDR",
"TRNIF_SET_MIM",
"TRNIF_FILT_GRD",
"TRNIF_ACK",
"TRNIF_NACK",
"TRNIF_BYE",
"TRNIF_OUT_MEAS",
"TRNIF_LAST_MEAS",
"TRNIF_IS_CONV",
"TRNIF_FILT_TYPE",
"TRNIF_FILT_STATE",
"TRNIF_FILT_REINITS",
"TRNIF_FILT_REINIT",
"TRNIF_PING"
};

/////////////////////////
// Function Definitions
/////////////////////////

trnmsg_t *trnmsg_dnew(trnmsg_id_t id, uint32_t data_len)
{
    size_t msg_size = sizeof(trnmsg_header_t)+data_len;
    trnmsg_t *instance = (trnmsg_t *)calloc(1,msg_size);
    if(NULL!=instance){
        trnmsg_header_t *hdr = (trnmsg_header_t *)instance;
        hdr->sync = g_trn_sync;
        hdr->msg_id=id;
        hdr->data_len=data_len;
    }
    return instance;
}
// End function trnmsg_dnew

trnmsg_t *trnmsg_new(trnmsg_id_t id, byte *data, uint32_t data_len)
{
    trnmsg_t *instance = trnmsg_dnew(id,data_len);
    if(NULL!=instance){
        trnmsg_header_t *hdr = (trnmsg_header_t *)instance;

        if(NULL!=data && data_len>0){
            byte *pdata = TRNIF_PDATA(instance);
            memcpy(pdata,data,data_len);
            hdr->checksum = trnmsg_checksum(pdata,data_len);
        }else{
            hdr->checksum=0;
        }
        //trnmsg_show(instance,true,5);
    }
    return instance;
}
// End function trnmsg_new

trnmsg_t *trnmsg_new_type_msg(trnmsg_id_t id, int parameter)
{

    trnmsg_t *instance = trnmsg_dnew(id,sizeof(trn_type_t));
    if(NULL!=instance){
        trnmsg_header_t *hdr = (trnmsg_header_t *)instance;

        trn_type_t *data = TRNIF_TPDATA(instance,trn_type_t);
        if(NULL!=data){
            data->parameter=parameter;

            hdr->checksum = trnmsg_checksum((byte *)data,hdr->data_len);
        }
    }
    return instance;
}
// End function

trnmsg_t *trnmsg_new_vdr_msg(trnmsg_id_t id, int parameter, float vdr)
{
    trnmsg_t *instance = trnmsg_dnew(id,sizeof(trn_float_t));
    if(NULL!=instance){
        trn_float_t *data = TRNIF_TPDATA(instance,trn_float_t);
        data->parameter=parameter;
        data->data=vdr;
    }
    return instance;
}
// End function

trnmsg_t *trnmsg_new_pose_msg(trnmsg_id_t id, wposet_t *pt)
{
    trnmsg_t *instance = trnmsg_dnew(id,sizeof(pt_cdata_t));
    if(NULL!=instance){
        trnmsg_header_t *hdr = (trnmsg_header_t *)instance;
        char *data = TRNIF_TPDATA(instance,char);
        hdr->data_len = wposet_serialize(&data,pt,sizeof(pt_cdata_t));
    }
    return instance;
}
// End function

trnmsg_t *trnmsg_new_meas_msg(trnmsg_id_t id, int parameter, wmeast_t *mt)
{
    size_t data_len = sizeof(trn_meas_t)+TRNW_WMEAST_SERIAL_LEN(wmeast_get_nmeas(mt));
    trnmsg_t *instance = trnmsg_dnew(id,data_len);
    if(NULL!=instance){
        trnmsg_header_t *hdr = (trnmsg_header_t *)instance;
        char *data = TRNIF_TPDATA(instance,char);
        wmeast_serialize(&data,mt,data_len-sizeof(trn_meas_t));
        hdr->data_len = data_len;
    }
    return instance;
}
// End function

void trnmsg_destroy(trnmsg_t **pself)
{
    if(NULL!=pself){
        trnmsg_t *self = *pself;
        if(NULL!=self){
            free(self);
            *pself=NULL;
        }
    }
}
// End function trnmsg_destroy

void trnmsg_show(trnmsg_t *self, bool verbose, int indent)
{
    if (NULL != self) {
        int wkey=15;
        int wval=15;
        fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"self",wval,self);
        fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey,"hdr",wval,&self->hdr);
        fprintf(stderr,"%*s%*s %*s\n",indent,(indent>0?" ":""),wkey,"type",wval,trnmsg_idstr(self->hdr.msg_id));
        fprintf(stderr,"%*s%*s %*X\n",indent,(indent>0?" ":""),wkey,"sync",wval,self->hdr.sync);
        fprintf(stderr,"%*s%*s %*"PRIu32"]\n",indent,(indent>0?" ":""),wkey,"data_len",wval,self->hdr.data_len);
        fprintf(stderr,"%*s%*s %*s%08X]\n",indent,(indent>0?" ":""),wkey,"checksum",wval-8," ",self->hdr.checksum);
        if(verbose && self->hdr.data_len>0){
            fprintf(stderr,"%*s[%*s %*s]\n",indent,(indent>0?" ":""),wkey,"data",wval,"");
            trnmsg_hex_show(TRNIF_PDATA(self),self->hdr.data_len,16,true,indent);
        }
    }
}
// End function trnif_show

int32_t trnmsg_len(trnmsg_t *self)
{
    uint32_t retval=0;
    if(NULL!=self){
        retval=self->hdr.data_len+TRNIF_HDR_LEN;
    }
    return retval;
}
// End function trnmsg_len

trnmsg_t *trnmsg_realloc(trnmsg_t **dest, trnmsg_id_t type, byte *data, uint32_t data_len)
{
    trnmsg_t *retval=NULL;
    return retval;

}
// End function trnmsg_realloc

int32_t trnmsg_deserialize(trnmsg_t **pdest, byte *src, uint32_t len)
{
    int32_t retval=-1;
    if(NULL!=pdest && NULL!=src){
        
        trnmsg_t *msg = (trnmsg_t *)src;
        if(msg->hdr.sync == g_trn_sync &&
           msg->hdr.msg_id<TRNIF_MSG_ID_COUNT &&
           msg->hdr.data_len>0 && msg->hdr.data_len<=TRNIF_MAX_SIZE){
            
            trnmsg_t *dest = trnmsg_new(msg->hdr.msg_id,TRNIF_PDATA(src),msg->hdr.data_len);
            *pdest = dest;
            retval=0;
        }
    }
    return retval;
}
// End function trnmsg_deserialize

int32_t trnmsg_serialize(byte **dest, uint32_t len)
{
    int retval=-1;
    return retval;
}
// End function trnmsg_serialize

const char *trnmsg_idstr(int id)
{
    const char *retval=(id>0 && id<TRNIF_MSG_ID_COUNT ? trnmsg_id_names[id] : NULL);
    return retval;
}// trnmsg_idstr

/// @fn void trnmsg_hex_show(byte * data, uint32_t len, uint16_t cols, _Bool show_offsets, uint16_t indent)
/// @brief output data buffer bytes in hex to stderr.
/// @param[in] data buffer pointer
/// @param[in] len number of bytes to display
/// @param[in] cols number of columns to display
/// @param[in] show_offsets show starting offset for each row
/// @param[in] indent output indent spaces
/// @return none
void trnmsg_hex_show(byte *data, uint32_t len, uint16_t cols, bool show_offsets, uint16_t indent)
{
    if (NULL!=data && len>0 && cols>0) {
        int rows = len/cols;
        int rem = len%cols;
        int i=0;
        byte *p=data;
        for (i=0; i<rows; i++) {
            if (show_offsets) {
                //                fprintf(stderr,"%*s%04zd [",indent,(indent>0?" ":""),(p-data));
                fprintf(stderr,"%*s%04lx [",indent,(indent>0?" ":""),(unsigned long)(p-data));
            }else{
                fprintf(stderr,"%*s[",indent,(indent>0?" ":""));
            }
            int j=0;
            for (j=0; j<cols; j++) {
                if (p>=data && p<(data+len)) {
                    byte b = (*p);
                    fprintf(stderr," %02x",b);
                    p++;
                }else{
                    fprintf(stderr,"   ");
                }
            }
            fprintf(stderr," ]\n");
        }
        if (rem>0) {
            if (show_offsets) {
                //                fprintf(stderr,"%*s%04zd [",indent,(indent>0?" ":""),(p-data));
                fprintf(stderr,"%*s%04lx [",indent,(indent>0?" ":""),(unsigned long)(p-data));
            }else{
                fprintf(stderr,"%*s[",indent,(indent>0?" ":""));
            }
            int j=0;
            for (j=0; j<rem; j++) {
                fprintf(stderr," %02x",*p++);
            }
            fprintf(stderr,"%*s ]\n",3*(cols-rem)," ");
        }
        
    }
}
// End function trnmsg_hex_show

/// @fn uint32_t trnmsg_checksum(byte * pdata, uint32_t len)
/// @brief return uint32_t checksum for data.
/// @param[in] pdata data pointer
/// @param[in] len length of data.
/// @return uint32 checksum value (sum of bytes).
uint32_t trnmsg_checksum(byte *pdata, uint32_t len)
{
    uint32_t checksum=0;
    if (NULL!=pdata) {
        byte *bp = pdata;
        //        fprintf(stderr,"\n");
        uint32_t i=0;
        for (i=0; i<len; i++) {
            checksum += (byte)(*(bp+i));
            //            fprintf(stderr,"%x ",(*(bp+i)));
        }
    }
    //    fprintf(stderr,"\nret[%08X]\n",checksum);
    return checksum;
}
// End function trnmsg_checksum


byte *TRNIF_PDATA(void *msg)
{
    byte *retval = NULL;
    if(NULL!=msg)retval=(byte *)msg+TRNIF_HDR_LEN;
    return retval;
}
