///
/// @file r7kc.c
/// @authors k. Headley
/// @date 01 jan 2018

/// Reson 7k Center data structures and protocol API

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

#include "r7kc.h"
#include "mxdebug.h"
#include "mxd_app.h"
#include "merror.h"

/////////////////////////
// Macros
/////////////////////////

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

// string buffer expansion increment
#define R7K_STR_INC 256
#define TRACKING_BYTES 16
/////////////////////////
// Declarations
/////////////////////////
uint32_t g_ticket=0;
byte g_tracking_number[TRACKING_BYTES]={0};

void s_get_ticket(uint32_t *dest){
    *dest = g_ticket++;
}

void s_get_tracking_number(byte *dest){
    memcpy(dest,g_tracking_number,TRACKING_BYTES);
    int i=TRACKING_BYTES;
    bool stop=false;
    for(i=0;i<TRACKING_BYTES && !stop;i++){
        if(g_tracking_number[i]<0xFF)
            stop=true;
        g_tracking_number[i]++;
    }
}

// define module IDs in mconfig.h

/// @enum r7k_module_ids
/// @brief application module IDs
/// [note : starting above reserved mframe module IDs]
//typedef enum{
//    MOD_R7K=MM_MODULE_COUNT,
//    APP_MODULE_COUNT
//}r7k_module_ids;

///// @enum r7k_channel_id
///// @brief test module channel IDs
///// [note : starting above reserved mframe channel IDs]
//typedef enum{
//    ID_R7K_V1=MM_CHANNEL_COUNT,
//    ID_R7K_V2,
//    ID_R7K_PARSER,
//    ID_R7K_DRFCON,
//    R7K_CH_COUNT
//}r7k_channel_id;
//
///// @enum r7k_channel_mask
///// @brief test module channel masks
//typedef enum{
//    R7K_V1= (1<<ID_R7K_V1),
//    R7K_V2= (1<<ID_R7K_V2),
//    R7K_PARSER= (1<<ID_R7K_PARSER),
//    R7K_DRFCON= (1<<ID_R7K_DRFCON)
//}r7k_channel_mask;
//
///// @var char *mmd_test_m1_ch_names[R7K_CH_COUNT]
///// @brief test module channel names
//char *r7k_ch_names[R7K_CH_COUNT]={
//    "trace.r7k",
//    "debug.r7k",
//    "warn.r7k",
//    "err.r7k",
//    "r7k.1",
//    "r7k.verbose",
//    "r7k.parser"
//};
//
//static mmd_module_config_t mmd_config_default= {MOD_R7K,"MOD_R7K",R7K_CH_COUNT,((MM_ERR|MM_WARN)|R7K_V1),r7k_ch_names};

/////////////////////////
// Imports
/////////////////////////

/////////////////////////
// Module Global Variables
/////////////////////////

/////////////////////////
// Function Definitions
/////////////////////////

/// @fn int r7k_subscribe(msock_socket_t * s, uint32_t * records, uint32_t record_count)
/// @brief subscribe to reson 7k center messages.
/// @param[in] s socket reference
/// @param[in] records record subscription list
/// @param[in] record_count sub list length
/// @return 0 on success, -1 otherwise
int r7k_req_config(msock_socket_t *s)
{
    int retval=-1;

    if (NULL != s ) {

        size_t rth_len = sizeof(r7k_rth_7500_rc_t);
        size_t rd_len = sizeof(r7k_reqrec_rd_t);

        r7k_msg_t *msg = r7k_msg_new(rth_len+rd_len);

        if (msg) {

            // set NF fields
            msg->nf->tx_id            = r7k_txid();
            msg->nf->protocol_version = R7K_NF_PROTO_VER;
            msg->nf->seq_number      = 0;
            msg->nf->offset          = sizeof(r7k_nf_t);
            msg->nf->packet_size     = R7K_MSG_NF_PACKET_SIZE(msg);
            msg->nf->total_size      = R7K_MSG_NF_TOTAL_SIZE(msg);
            msg->nf->dest_dev_id     = 0;
            msg->nf->dest_enumerator = 0;
            msg->nf->src_enumerator  = 0;
            msg->nf->src_dev_id      = 0;

            // set DRF fields
            msg->drf->size           = R7K_MSG_DRF_SIZE(msg);
            msg->drf->record_type_id = R7K_RT_REMCON;
            msg->drf->device_id      = R7K_DEVID_7KCENTER;
            msg->drf->sys_enumerator = R7K_DRF_SYS_ENUM_DFL;


            // set record type header info
            r7k_rth_7500_rc_t *prth =(r7k_rth_7500_rc_t *)(msg->data);
            prth->remcon_id = R7K_RTID_REQ_REC;
            s_get_ticket(&prth->ticket);
            s_get_tracking_number(prth->tracking_number);
            // set record data (record type only)
            r7k_reqrec_rd_t *prdata = (r7k_reqrec_rd_t *)(msg->data+rth_len);
            prdata->record_type = R7K_RT_CONFIG_DATA;

            // set checksum [do last]
            r7k_msg_set_checksum(msg);

            // serialize, send
            MX_MMSG(R7KR_DEBUG, "sending CONFIG_DATA request:\n");
            if(mxd_testModule(R7KC_ERROR, 1) || mxd_testModule(R7KC, 2)){
                r7k_msg_show(msg,true,3);
            }
            r7k_msg_send(s, msg);

            // get ACK/NAK
            r7k_msg_t *reply = NULL;
            r7k_msg_receive(s, &reply, R7K_SUBSCRIBE_TIMEOUT_MS);

            if(NULL!=reply){

                // show ACK/NAK
                char *rep_str="?";
                if(reply->drf->record_type_id==R7K_RT_REMCON_ACK)
                    rep_str="ACK";
                if(reply->drf->record_type_id==R7K_RT_REMCON_NACK)
                    rep_str="NACK";

                MX_MPRINT(R7KR_DEBUG, "CONFIG_DATA reply received %s [%p]:\n", rep_str, reply);
                if(mxd_testModule(R7KC_ERROR, 1) || mxd_testModule(R7KC, 2)){
                    r7k_msg_show(reply,true,3);
                }

                if(reply->drf->record_type_id==R7K_RT_REMCON_ACK){
                    // if ACK, read/show config data message
                    MX_MMSG(R7KR_DEBUG, "CONFIG_DATA reading config data\n");

                    // release reply memory and reuse for config data
                    r7k_msg_destroy(&reply);
                    reply=NULL;
                    int status = -1;

                    if( (status=r7k_msg_receive(s, &reply, R7K_SUBSCRIBE_TIMEOUT_MS)) > 0 && NULL!=reply){
                        MX_MPRINT(R7KR_DEBUG, "CONFIG_DATA message received [%p/%d]:\n", reply, status);

                        if(mxd_testModule(R7KC_ERROR, 1) || mxd_testModule(R7KC, 2)){
                            r7k_msg_show(reply,true,3);

                            if(reply->drf->record_type_id==R7K_RT_CONFIG_DATA){

                                r7k_rth_7001_rd_t *rdata = (r7k_rth_7001_rd_t *)reply->data;
                                r7k_7001_dev_info_t *dev_inf = (r7k_7001_dev_info_t *)((byte *)reply->data+sizeof(r7k_rth_7001_rd_t));
                                int indent=3;
                                int wkey=15;
                                int wval=8;

                                MX_MPRINT(R7KR_DEBUG, "%*s%*s %*"PRIu64"\n", indent," ", wkey, "sonar_sn", wval, rdata->sonar_sn);
                                MX_MPRINT(R7KR_DEBUG, "%*s%*s %*"PRIu32"\n\n",indent," ",wkey,"device_count", wval, rdata->device_count);

                                unsigned int i=0;
                                for(i=0;i<rdata->device_count;i++){
                                    MX_MPRINT(R7KR_DEBUG, "%*s%*s [%*d/%d] ***\n",indent," ",wkey,"*** Device", 3, (i+1), rdata->device_count);
                                    // fixed length device info header
                                    MX_MPRINT(R7KR_DEBUG, "%*s%*s %*"PRIu32"\n",indent," ",wkey,"unique_id", wval, dev_inf->unique_id);
                                    MX_MPRINT(R7KR_DEBUG, "%*s%*s %*s\n",indent," ", wkey, "desc", wval, dev_inf->desc);
                                    MX_MPRINT(R7KR_DEBUG, "%*s%*s %*"PRIu32"\n", indent, " ", wkey, "alph_data", wval, dev_inf->alph_data_type);
                                    MX_MPRINT(R7KR_DEBUG, "%*s%*s %*"PRIu64"\n", indent, " ", wkey, "serial_number", wval, dev_inf->serial_number);
                                    MX_MPRINT(R7KR_DEBUG, "%*s%*s %*"PRIu32"\n", indent, " ", wkey, "info_bytes", wval, dev_inf->info_bytes);
                                    // variable length device data XML
                                    byte *pinfo = (byte *)(&dev_inf->info_bytes) + sizeof(uint32_t);
                                    if(dev_inf->info_bytes){
                                        MX_MPRINT(R7KR_DEBUG, "%*s%*s\n%s\n", indent, " ", wkey, "device XML:", (char *)pinfo);
                                    }
                                    // point to next device info
                                    dev_inf = (r7k_7001_dev_info_t *)(pinfo + dev_inf->info_bytes);
                                }
                                MX_MMSG(R7KR_DEBUG, "\n");
                            }
                        }
                    }
                }
            }

            // validate reply
            // release resources
            r7k_msg_destroy(&msg);
            r7k_msg_destroy(&reply);
            retval=0;
        }
    }else{
        MX_ERROR_MSG("ERR - invalid argument\n");
    }
    return retval;
}

static void s_dev2drfid(r7k_device_t device_id, uint32_t *dev_id, uint16_t *sys_enum){

    if(NULL!=dev_id && NULL!=sys_enum){
        if(device_id==R7KC_DEV_7125_200KHZ){
            // for 7125, use 7KCenter ID
            *dev_id = R7K_DEVID_7KCENTER;
            *sys_enum = R7K_DRF_SYS_ENUM_200KHZ;
        }else if(device_id==R7KC_DEV_7125_400KHZ){
            // for 7125, use 7KCenter ID
            *dev_id = R7K_DEVID_7KCENTER;
            *sys_enum = R7K_DRF_SYS_ENUM_400KHZ;
        }else if(device_id==R7KC_DEV_T50){
            *dev_id = R7K_DEVID_T50;
            *sys_enum = R7K_DRF_SYS_ENUM_DFL;
        }
    }
}

r7k_device_t r7k_parse_devid(const char *key)
{

    r7k_device_t retval = R7KC_DEV_INVALID;

    if(NULL!=key){
        if(strcasecmp(key,R7K_MNEM_7125_200KHZ)==0){
            retval=R7KC_DEV_7125_200KHZ;
        }else if(strcasecmp(key,R7K_MNEM_7125_400KHZ)==0){
            retval=R7KC_DEV_7125_400KHZ;
        }else if(strcasecmp(key,R7K_MNEM_T50)==0){
            retval=R7KC_DEV_T50;
        }
    }
    return retval;
}

const char *r7k_devidstr(int dev_id)
{

    const char *retval = R7K_MNEM_INVALID;
    switch (dev_id) {
        case R7KC_DEV_7125_200KHZ:
            retval=R7K_MNEM_7125_200KHZ;
            break;
        case R7KC_DEV_7125_400KHZ:
            retval=R7K_MNEM_7125_400KHZ;
            break;
        case R7KC_DEV_T50:
            retval=R7K_MNEM_T50;
            break;

        default:
            break;
    }
    return retval;
}


/// @fn int r7k_subscribe(msock_socket_t * s, uint32_t * records, uint32_t record_count)
/// @brief subscribe to reson 7k center messages.
/// @param[in] s socket reference
/// @param[in] device_id device ID
/// @param[in] enum_id system enumerator
/// @param[in] records record subscription list
/// @param[in] record_count sub list length
/// @return 0 on success, -1 otherwise
int  r7k_subscribe(msock_socket_t *s, r7k_device_t device_id, uint32_t *records, uint32_t record_count)
{
    int retval=-1;

    if (NULL != s && NULL != records && record_count>0) {

        size_t rth_len = sizeof(r7k_rth_7500_rc_t);
        size_t rd_len = sizeof(r7k_sub_rd_t)+record_count*sizeof(uint32_t);

        r7k_msg_t *msg = r7k_msg_new(rth_len+rd_len);

        if (msg) {

            // set NF fields
            msg->nf->tx_id            = r7k_txid();
            msg->nf->protocol_version = R7K_NF_PROTO_VER;
            msg->nf->seq_number      = 0;
            msg->nf->offset          = sizeof(r7k_nf_t);
            msg->nf->packet_size     = R7K_MSG_NF_PACKET_SIZE(msg);
            msg->nf->total_size      = R7K_MSG_NF_TOTAL_SIZE(msg);
            msg->nf->dest_dev_id     = 0;
            msg->nf->dest_enumerator = 0;
            msg->nf->src_enumerator  = 0;
            msg->nf->src_dev_id      = 0;

            // set DRF fields
            msg->drf->size           = R7K_MSG_DRF_SIZE(msg);
            msg->drf->record_type_id = R7K_RT_REMCON;

            // map generic device ID to correct 7K Center DRF
            // device ID and system enumerator
            s_dev2drfid(device_id, &msg->drf->device_id, &msg->drf->sys_enumerator);

            // set record type header info
            r7k_rth_7500_rc_t *prth =(r7k_rth_7500_rc_t *)(msg->data);
            prth->remcon_id = R7K_RTID_SUB;
            s_get_ticket(&prth->ticket);
            s_get_tracking_number(prth->tracking_number);

            r7k_sub_rd_t *prdata = (r7k_sub_rd_t *)(msg->data+rth_len);
            prdata->record_count = record_count;

            // set record data
            memcpy((msg->data+rth_len+sizeof(r7k_sub_rd_t)),records, (record_count*sizeof(uint32_t)));

            // set checksum [do last]
            // TODO: optionally do in send()?
            r7k_msg_set_checksum(msg);

            // serialize, send
            MX_MMSG(R7KR_DEBUG, "sending SUB request:\n");
            if(mxd_testModule(R7KC_ERROR, 1) || mxd_testModule(R7KC, 2)){
                r7k_msg_show(msg,true,3);
            }
            r7k_msg_send(s, msg);

            // get ACK/NAK
            r7k_msg_t *reply = NULL;
            r7k_msg_receive(s, &reply, R7K_SUBSCRIBE_TIMEOUT_MS);

            char *rt_str="?";
            if(NULL!=reply){
                if(reply->drf->record_type_id==R7K_RT_REMCON_ACK){
                    rt_str="ACK";
                    retval=0;
                }
                if(reply->drf->record_type_id==R7K_RT_REMCON_NACK){
                    rt_str="NACK";
                    MX_MPRINT(R7KR_DEBUG, "SUB request returned NAK - possibly invalid device (%d/%s)\n", device_id, r7k_devidstr(device_id));
                }
            }
            MX_MPRINT(R7KR_DEBUG, "SUB reply received [%p/%s]:\n", reply, rt_str);
            if(mxd_testModule(R7KC_ERROR, 1) || mxd_testModule(R7KC, 2)){
                r7k_msg_show(reply,true,3);
            }
            // validate reply
            // release resources
            r7k_msg_destroy(&msg);
            r7k_msg_destroy(&reply);
        }
    }else{
    	MX_ERROR_MSG("ERR - invalid argument\n");
    }
    return retval;
}
// End function r7k_subscribe

/// @fn int r7k_unsubscribe(msock_socket_t * s)
/// @brief unsubscribe from reson 7k records (not implemented).
/// @param[in] s socket reference
/// @return 0 on success, -1 otherwise
int r7k_unsubscribe(msock_socket_t *s)
{
    int retval=-1;
    MX_ERROR_MSG("ERR - not implemented\n");
    return retval;
}
// End function r7k_unsubscribe

/// @fn uint16_t r7k_txid()
/// @brief transmission ID (for messages sent to r7kc).
/// @return transmission ID 0-65535
uint16_t r7k_txid()
{
    static uint16_t txid=0;
    return ++txid;
}
// End function r7k_txid

/// @fn uint32_t r7k_checksum(byte * pdata, uint32_t len)
/// @brief return r7k checksum for data.
/// @param[in] pdata data pointer
/// @param[in] len length of data.
/// @return r7k checksum value (sum of bytes).
uint32_t r7k_checksum(byte *pdata, uint32_t len)
{
    uint32_t checksum=0;
    if (NULL!=pdata) {
        byte *bp = pdata;
//        fprintf(stderr,"\n");
        for (uint32_t i=0; i<len; i++) {
            checksum += (byte)(*(bp+i));
//            fprintf(stderr,"%x ",(*(bp+i)));
        }
    }
//    fprintf(stderr,"\nret[%08X]\n",checksum);
    return checksum;
}
// End function r7k_checksum

/// @fn void r7k_update_time(r7k_time_t * t7k)
/// @brief set the time in r7k time format.
/// @param[in] t7k r7k time structure reference
/// @return none
void r7k_update_time(r7k_time_t *t7k)
{
    if ( (NULL!=t7k) ) {
        // ANSI time (sec resolution)
        // supported on OSX, Windows, *NIX
        struct tm tms={0};
        time_t tt = time(NULL);
        gmtime_r(&tt, &tms);
        t7k->year    = tms.tm_year%100;
        t7k->day     = tms.tm_yday;
        t7k->hours   = tms.tm_hour;
        t7k->minutes = tms.tm_min;
        t7k->seconds = (tms.tm_sec == 60. ? 0. : (float)tms.tm_sec);
    }
}
// End function r7k_update_time

/// @fn void r7k_hex_show(byte * data, uint32_t len, uint16_t cols, _Bool show_offsets, uint16_t indent)
/// @brief output data buffer bytes in hex to stderr.
/// @param[in] data buffer pointer
/// @param[in] len number of bytes to display
/// @param[in] cols number of columns to display
/// @param[in] show_offsets show starting offset for each row
/// @param[in] indent output indent spaces
/// @return none
void r7k_hex_show(byte *data, uint32_t len, uint16_t cols, bool show_offsets, uint16_t indent)
{
    if (NULL!=data && len>0 && cols>0) {
        int rows = len/cols;
        int rem = len%cols;

        byte *p=data;
        fprintf(stderr,"%*s:\n",indent,__func__);
        for (int i=0; i<rows; i++) {
            if (show_offsets) {
                fprintf(stderr,"%*s%04ld [",indent,(indent>0?" ":""),(long int)(p-data));
            }else{
                fprintf(stderr,"%*s[",indent,(indent>0?" ":""));
            }
            for (int j=0; j<cols; j++) {
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
                fprintf(stderr,"%*s%04ld [",indent,(indent>0?" ":""),(long int)(p-data));
            }else{
                fprintf(stderr,"%*s[",indent,(indent>0?" ":""));
            }
            for (int j=0; j<rem; j++) {
                fprintf(stderr," %02x",*p++);
            }
            fprintf(stderr,"%*s ]\n",3*(cols-rem)," ");
        }

    }
}
// End function r7k_hex_show

/// @fn void r7k_parser_show(r7k_parse_stat_t * self, _Bool verbose, uint16_t indent)
/// @brief output r7k parser statistics to stderr.
/// @param[in] self parser stats structure reference
/// @param[in] verbose use verbose output
/// @param[in] indent output indent spaces
/// @return none
void r7k_parser_show(r7k_parse_stat_t *self, bool verbose, uint16_t indent)
{
    if (NULL!=self) {
        fprintf(stderr,"%*s[self           %10p]\n",indent,(indent>0?" ":""), self);
        fprintf(stderr,"%*s[src_bytes      %10u]\n",indent,(indent>0?" ":""), self->src_bytes);
        fprintf(stderr,"%*s[sync_bytes     %10u]\n",indent,(indent>0?" ":""), self->sync_bytes);
        fprintf(stderr,"%*s[unread_bytes   %10u]\n",indent,(indent>0?" ":""), self->unread_bytes);
        fprintf(stderr,"%*s[parsed_records %10u]\n",indent,(indent>0?" ":""), self->parsed_records);
        fprintf(stderr,"%*s[parsed_bytes   %10u]\n",indent,(indent>0?" ":""), self->parsed_bytes);
        fprintf(stderr,"%*s[resync_count   %10u]\n",indent,(indent>0?" ":""), self->resync_count);
        fprintf(stderr,"%*s[status         %10d]\n",indent,(indent>0?" ":""), self->status);
    }
}
// End function r7k_parser_show

/// @fn char * r7k_parser_str(r7k_parse_stat_t * self, char * dest, uint32_t len, _Bool verbose, uint16_t indent)
/// @brief output parser statistics to a string. Caller must free.
/// @param[in] self parser stat struct reference
/// @param[in] dest string output buffer, use NULL to allocate new string
/// @param[in] len buffer length
/// @param[in] verbose use verbose output
/// @param[in] indent output indent spaces
/// @return new string (if dest==NULL) or dest on success, NULL otherwise. May truncate if buffer length is insufficient.
char *r7k_parser_str(r7k_parse_stat_t *self, char *dest, uint32_t len, bool verbose, uint16_t indent)
{
    char *retval=NULL;

//    uint32_t inc=256;

    if (NULL!=self) {

        char *wbuf=(char *)malloc(R7K_STR_INC*sizeof(char));

        // TODO: check length/realloc
        if (wbuf!=NULL) {
            memset(wbuf,0,len);
            char *dp=wbuf;

            uint32_t n=1;
            char *fmt="%*s[self           %10p]\n";
            uint32_t wlen=len;
            uint32_t wb = snprintf(NULL,0,fmt,indent,(indent>0?" ":""), self);
            wlen+=wb;
            if (wlen>R7K_STR_INC) {
                n++;
                wbuf=(char *)realloc(wbuf,n*R7K_STR_INC*sizeof(char));
                dp = wbuf+strlen(wbuf);
            }
            sprintf(dp,fmt,indent,(indent>0?" ":""), self);
            dp = wbuf+strlen(wbuf);

            fmt="%*s[src_bytes      %10u]\n";
            wb = snprintf(NULL,0,fmt,indent,(indent>0?" ":""), self->src_bytes);
            wlen+=wb;
            if (wlen>R7K_STR_INC) {
                n++;
                wbuf=(char *)realloc(wbuf,n*R7K_STR_INC*sizeof(char));
                dp = wbuf+strlen(wbuf);
            }
            sprintf(dp,fmt,indent,(indent>0?" ":""), self->src_bytes);
            dp = wbuf+strlen(wbuf);

            fmt="%*s[sync_bytes     %10u]\n";
            wb = snprintf(NULL,0,fmt,indent,(indent>0?" ":""), self->sync_bytes);
            wlen+=wb;
            if (wlen>R7K_STR_INC) {
                n++;
                wbuf=(char *)realloc(wbuf,n*R7K_STR_INC*sizeof(char));
                dp = wbuf+strlen(wbuf);
            }
            sprintf(dp,fmt,indent,(indent>0?" ":""), self->sync_bytes);
            dp = wbuf+strlen(wbuf);

            fmt="%*s[unread_bytes   %10u]\n";
            wb = snprintf(NULL,0,fmt,indent,(indent>0?" ":""), self->unread_bytes);
            wlen+=wb;
            if (wlen>R7K_STR_INC) {
                n++;
                wbuf=(char *)realloc(wbuf,n*R7K_STR_INC*sizeof(char));
                dp = wbuf+strlen(wbuf);
            }
            sprintf(dp,fmt,indent,(indent>0?" ":""), self->unread_bytes);
            dp = wbuf+strlen(wbuf);


            fmt="%*s[parsed_records %10u]\n";
            wb = snprintf(NULL,0,fmt,indent,(indent>0?" ":""), self->parsed_records);
            wlen+=wb;
            if (wlen>R7K_STR_INC) {
                n++;
                wbuf=(char *)realloc(wbuf,n*R7K_STR_INC*sizeof(char));
                dp = wbuf+strlen(wbuf);
            }
            sprintf(dp,fmt,indent,(indent>0?" ":""), self->parsed_records);
            dp = wbuf+strlen(wbuf);

            fmt="%*s[parsed_bytes   %10u]\n";
            wb = snprintf(NULL,0,fmt,indent,(indent>0?" ":""), self->parsed_bytes);
            wlen+=wb;
            if (wlen>R7K_STR_INC) {
                n++;
                wbuf=(char *)realloc(wbuf,n*R7K_STR_INC*sizeof(char));
                dp = wbuf+strlen(wbuf);
            }
            sprintf(dp,fmt,indent,(indent>0?" ":""), self->parsed_bytes);
            dp = wbuf+strlen(wbuf);

            fmt="%*s[resync_count   %10u]\n";
            wb = snprintf(NULL,0,fmt,indent,(indent>0?" ":""), self->resync_count);
            wlen+=wb;
            if (wlen>R7K_STR_INC) {
                n++;
                wbuf=(char *)realloc(wbuf,n*R7K_STR_INC*sizeof(char));
                dp = wbuf+strlen(wbuf);
            }
            sprintf(dp,fmt,indent,(indent>0?" ":""), self->resync_count);
            dp = wbuf+strlen(wbuf);

            fmt="%*s[status         %10d]\n";
            wb = snprintf(NULL,0,fmt,indent,(indent>0?" ":""), self->status);
            wlen+=wb;
            if (wlen>R7K_STR_INC) {
                n++;
                wbuf=(char *)realloc(wbuf,n*R7K_STR_INC*sizeof(char));
                dp = wbuf+strlen(wbuf);
            }
            sprintf(dp,fmt,indent,(indent>0?" ":""), self->status);

            if (NULL!=dest) {
                snprintf(dest,(len<wlen?len:wlen),"%s",wbuf);
                free(wbuf);
                retval=dest;
            }else{
                retval=wbuf;
            }
        }
    }
    return retval;
}
// End function r7k_parser_str

// extract valid DRFs from raw network frames
// stops when src parsed or dest full
/// @fn uint32_t r7k_parse(byte * src, uint32_t len, r7k_drf_container_t * dest, r7k_parse_stat_t * status)
/// @brief parse r7k network frames, return buffer with parsed data record frames.
/// does some validation, e.g. length, checksum, rejects invalid frames
/// @param[in] src buffer containing raw r7k data (i.e. network frames)
/// @param[in] len size of src/dest buffer
/// @param[in] dest destination buffer for parsed data (i.e. data record frames
/// @param[in] status parser status structure
/// @return number of bytes returned in destination buffer
uint32_t r7k_parse(byte *src, uint32_t len, r7k_drf_container_t *dest, r7k_parse_stat_t *status)
{
    int retval=-1;

    if (NULL != src && NULL!=dest) {
        me_errno=ME_OK;
        r7k_nf_t *pnf=NULL;
        r7k_drf_t *pdrf=NULL;
        r7k_checksum_t *pchk=NULL;
        byte *psrc = NULL;
        uint32_t record_count = 0;
        uint32_t sync_bytes = 0;
        psrc=src;
        memset(status,0,sizeof(r7k_parse_stat_t));
        status->src_bytes = len;

        bool resync=false;

        // move src pointer along, and mark
        // add records to the drf container (expands dynamically)
        // stop when we've found the end of the source buffer

        while (psrc<(src+len)) {

            pnf = (r7k_nf_t *)psrc;
//            MX_MMSG(R7KR_DEBUG, "psrc[%p]\n",psrc));
            // pnf is legit?...
            if ( (pnf->protocol_version == (uint16_t)R7K_NF_PROTO_VER) &&
                (pnf->total_packets > (uint32_t)0) &&
                (pnf->total_size >= (uint32_t)R7K_DRF_BYTES)
                ) {

                pdrf=(r7k_drf_t *)(psrc + R7K_NF_BYTES);

                // check DRF
                if( (pdrf->protocol_version == (uint16_t)R7K_DRF_PROTO_VER) &&
                   (pdrf->sync_pattern == (uint32_t)R7K_DRF_SYNC_PATTERN) &&
                   (pdrf->size > (uint32_t)R7K_DRF_BYTES) ){

                    byte *pw = (byte *)pdrf;
                    // this fixes an uninitialized variable (vchk)
                    // warning (in valgrind? compiler?)
                    pw[0]=pw[0];
                    pchk = (r7k_checksum_t *)(pw+pdrf->size-R7K_CHECKSUM_BYTES);

                    if ( ((byte *)pchk) < (src+len) ) {

                        r7k_checksum_t vchk = 0;
                        vchk = r7k_checksum(pw,(pdrf->size-R7K_CHECKSUM_BYTES));

                        // checksum is valid or unused (flags:0 unset)
                        if ( ((pdrf->flags&0x1) == 0) || vchk == *pchk ) { // vchk UNINIT
                            // found a valid record...

                            // add it to the frame container
                            // also adds frame offset info
                            if(r7k_drfcon_add(dest,(byte *)pdrf,pdrf->size)==0){
                                //                            byte *prev=psrc;
                                // update src pointer
                                psrc = ((byte *)pchk + R7K_CHECKSUM_BYTES+R7K_CHECKSUM_BYTES);

                                // update record count
                                record_count++;

                                //                            MX_MPRINT(R7KR_DEBUG, "adding record prv[%p] nxt[%p]\n", prev, psrc);
                                // set retval to parsed bytes
                                retval = r7k_drfcon_length(dest);

                                resync=false;
                                status->parsed_records++;
                                status->status=ME_OK;

                            }else{
                                MX_MMSG(R7KR_DEBUG, "DRF container full\n");
                                status->status = ME_ENOSPACE;
                                me_errno = ME_ENOSPACE;
                                break;
                            }

                        }else{
                            MX_MPRINT(R7KR_DEBUG, "CHKSUM err: checksum mismatch p/c[%u/%u]\n", *pchk, vchk);
                            // skip to checksum, start resync there
                            sync_bytes+=((byte *)pchk-psrc);
                            psrc=(byte *)pchk;
                            resync=true;
                        }
                     }else{
                         MX_MMSG(R7KR_DEBUG, "CHKSUM err: pointer out of bounds\n");
                         break;
                     }
                }else{
                    MX_MMSG(R7KR_DEBUG, "DRF err\n");
                    //                    r7k_drf_show(pdrf,true,3);
                    //                    r7k_hex_show(psrc,0x50,16,true,5);
                    resync=true;
                }
            }else{
                MX_MPRINT(R7KR_DEBUG, "NRF err: psrc[%p] ofs[%zd] protov[%hu] totpkt[%u] totsz[%u]\n",
                        psrc,
                        (psrc-src),
                        pnf->protocol_version,
                        pnf->total_packets,
                        pnf->total_size);

                resync=true;
            }

            if (resync) {
                // search for the next valid network frame
                int64_t x=0;
                // test NF, DRF proto versions and DRF sync pattern
                // to indicate possibly valid frame
                size_t hdr_len =(R7K_NF_BYTES+R7K_DRF_BYTES);
                 size_t oofs=(psrc-src);

                // start looking at next byte
                psrc++;
                bool sync_found=false;
                bool capacity_ok= false;
                size_t space_rem=(src+len-psrc);
                // search until sync found or end of buffer
                while ( (NULL != psrc) && (space_rem>0) && (sync_found==false) ){

                    space_rem = (src+len-psrc);
                    // set net frame and data record pointers
                    pnf = (r7k_nf_t *)psrc;
                    pdrf = (r7k_drf_t *)(psrc+R7K_NF_BYTES);

                    capacity_ok= (space_rem >= hdr_len);
                    bool hdr_valid = (( pnf->protocol_version == (uint16_t)R7K_NF_PROTO_VER ) &&
                                      ( pnf->total_packets > (uint32_t)0 ) &&
                                      ( pnf->total_size >= (uint32_t)R7K_DRF_BYTES) &&
                                      ( pdrf->protocol_version  == (uint16_t)R7K_DRF_PROTO_VER ) &&
                                      ( pdrf->sync_pattern  == (uint32_t)R7K_DRF_SYNC_PATTERN) );

                    // stop if we find a valid record
                    // or there isn't remaining space for one
                    if(  capacity_ok && hdr_valid ){
                        sync_found=true;
                        break;
                    }
                    if (!capacity_ok) {
                        break;
                    }
                    // advance source pointer
                    psrc++;
                    x++;
                    sync_bytes++;
                }


                if (sync_found){
                    MX_MPRINT(R7KR_DEBUG, "skipped %"PRId64" bytes oofs[%zd] new_ofs[%zd]\n", x, oofs, (psrc-src));
//                    MX_MPRINT(R7KR_DEBUG, "nrf ofs[%zd] protov[%hu] totpkt[%u] totsz[%u]\n",
//                            (psrc-src),
//                            pnf->protocol_version,
//                            pnf->total_packets,
//                            pnf->total_size));
                }else{
                    MX_MPRINT(R7KR_DEBUG, "ERR - resync failed: spc[%zd] hdr_len[%zd] skipped [%"PRId64"]\n", space_rem, hdr_len, x);
                    if (!capacity_ok) {
                        MX_MMSG(R7KR_DEBUG, "DRF container full\n");
                        status->status = ME_ENOSPACE;
                        me_errno = ME_ENOSPACE;
                        break;
                    }
                }
                status->resync_count++;
                resync=false;
            }
        }
        status->unread_bytes = ((src+len)-psrc);
        status->parsed_bytes = r7k_drfcon_length(dest);
        status->sync_bytes   = sync_bytes;
       if (mxd_testModule(R7KC_PARSER, 1)) {
            r7k_parser_show(status,true,5);
        }
        MX_MPRINT(R7KC_PARSER, "valid[%d] resyn[%d] sync[%d] rv[%d]\n",status->parsed_records,status->resync_count,status->sync_bytes,retval);
    }
    return retval;
}
// End function r7k_parse

/// @fn int r7k_stream_show(msock_socket_t * s, int sz, uint32_t tmout_ms, int cycles)
/// @brief output raw r7k stream to stderr as formatted ASCII hex.
/// @param[in] s r7k host socket
/// @param[in] sz read buffer size (read sz bytes at a time)
/// @param[in] tmout_ms read timeout
/// @param[in] cycles number of cycles to read (<=0 read forever)
/// @return 0 on success, -1 otherwise
int r7k_stream_show(msock_socket_t *s, int sz, uint32_t tmout_ms, int cycles, bool *interrupt)
{
    int retval=-1;
    int x=(sz<=0?16:sz);
    byte *buf=(byte *)malloc(x);

    if(NULL!=buf){
        int good=0,err=0,zero=0,tmout=0;
        int status = 0;
        bool forever=true;
        int count=0;

        if (cycles>0) {
            forever=false;
        }
        //    MX_ERROR("cycles[%d] forever[%s] c||f[%s]\n",cycles,(forever?"Y":"N"),(forever || (cycles>0) ? "Y" :"N")));

        // read cycles or forever (cycles<=0)
        while ( (forever || (count++ < cycles)) &&
               (NULL!=interrupt && !(*interrupt)) ) {
            memset(buf,0,x);
            int64_t test = msock_read_tmout(s, buf, x, tmout_ms);
            if(test>0){
                good++;
                r7k_hex_show(buf, test, 16, true, 3);
                fprintf(stderr,"c[%d/%d] ret[%"PRId64"/%u] stat[%d] good/zero/tmout/err [%d/%d/%d/%d]\n",count,cycles,test,sz,status,good,zero,tmout,err);
                retval=0;
            }else if(test<0){
                MX_MPRINT(R7KR_DEBUG, "ERR [%d/%s]\n", me_errno, me_strerror(me_errno));
                err++;
                tmout = (me_errno==ME_ETMOUT ? tmout+1 : tmout );
                if (me_errno==ME_ETMOUT || me_errno==ME_EOF || me_errno==ME_ESOCK) {
                    break;
                }
            }else{
                MX_MMSG(R7KR_DEBUG, "read returned 0\n");
                zero++;
                if (me_errno==ME_ESOCK || me_errno==ME_EOF) {
                    break;
                }
            }
        }
        free(buf);
    }
    return retval;
}
// End function r7k_stream_show

/// @fn r7k_7ktime2d(r7k_time_t *r7kt)
/// @brief convert 7ktime to decimal time
/// @param[in] r7kt 7ktime reference
/// @return decimal time on success, 0.0 otherwise
double r7k_7ktime2d(r7k_time_t *r7kt)
{
    double retval=0.0;

    if (NULL != r7kt) {

        double pti=0.0;
        double ptf=modf(r7kt->seconds, &pti);
        struct tm tms = {0};
        char tstr[64]={0};
        sprintf(tstr,"%u %u %02d:%02d:%02.0f",r7kt->year,r7kt->day,(int)r7kt->hours,(int)r7kt->minutes,r7kt->seconds);

        strptime(tstr,"%Y %j %H:%M:%S",&tms);
        tms.tm_isdst=-1;
        time_t tt = mktime(&tms);
        retval=(double)tt+ptf;
//        fprintf(stderr,"tms[%s] ret[%.3lf]\n",tms,retval);

    }// else invalid argument

    return retval;
}
// End function r7k_7ktime2d

/// @fn r7k_nf_t * r7k_nf_new()
/// @brief create new r7k network frame structure. used mostly by components.
/// @return reference to new instance on success, NULL otherwise
r7k_nf_t *r7k_nf_new()
{
    r7k_nf_t *self = (r7k_nf_t *)malloc( sizeof(r7k_nf_t) );
    if (NULL!=self) {
        r7k_nf_init(&self,true);
    }
    return self;
}
// End function r7k_nf_new

/// @fn void r7k_nf_destroy(r7k_nf_t ** pself)
/// @brief release network frame structure resources.
/// @param[in] pself pointer to instance reference
/// @return none
void r7k_nf_destroy(r7k_nf_t **pself)
{
    if (pself) {
        r7k_nf_t *self = *pself;
        if (self) {
            free(self);
        }
        *pself=NULL;
    }

}
// End function r7k_nf_destroy

/// @fn r7k_nf_t * r7k_nf_init(r7k_nf_t ** pnf, _Bool erase)
/// @brief initialize network frame with common defaults. create new
/// network frame if pnf argument is NULL.
/// @param[in] pnf network frame to initialize (or NULL to create new)
/// @param[in] erase zero network frame before initializing
/// @return network frame reference on success, NULL otherwise
r7k_nf_t * r7k_nf_init(r7k_nf_t **pnf,bool erase)
{
    r7k_nf_t *nf=NULL;
    if(NULL!=pnf){
        if (NULL == *pnf) {
            nf = (r7k_nf_t *)malloc(sizeof(r7k_nf_t));
            memset(nf,0,sizeof(r7k_nf_t));
            *pnf = nf;
        }else{
            nf = *pnf;
        }
        if (NULL!=nf) {
            if (erase) {
                memset(nf,0,sizeof(r7k_nf_t));
            }
            // caller must set:
            // total_size
            // packet_size

            // caller may optionally set
            // total packets
            // trans_id
            // seq_number

            nf->protocol_version = R7K_NF_PROTO_VER;
            nf->offset           = R7K_NF_BYTES;
            nf->total_packets    = 1;
            nf->total_records    = 1;
            nf->tx_id            = 0;

            nf->seq_number       = 0;
            nf->dest_dev_id      = R7K_DEVID_7KCENTER;
            nf->dest_enumerator  = 0;
            nf->src_enumerator   = 0;
            nf->src_dev_id       = R7K_NF_DEVID_UNUSED;
        }

    }


    return nf;
}
// End function r7k_nf_init

/// @fn void r7k_nf_show(r7k_nf_t * self, _Bool verbose, uint16_t indent)
/// @brief output network frame structure parameter summary to stderr.
/// @param[in] self network fram struct reference
/// @param[in] verbose use verbose output
/// @param[in] indent output indent spaces
/// @return none
void r7k_nf_show(r7k_nf_t *self, bool verbose, uint16_t indent)
{
    if (self) {
        fprintf(stderr,"%*s[self             %10p]\n",indent,(indent>0?" ":""), self);
        fprintf(stderr,"%*s[protocol_version %10hu]\n",indent,(indent>0?" ":""), self->protocol_version);
        fprintf(stderr,"%*s[offset           %10hu]\n",indent,(indent>0?" ":""), self->offset);
        fprintf(stderr,"%*s[total_packets    %10u]\n",indent,(indent>0?" ":""), self->total_packets);
        fprintf(stderr,"%*s[total_records    %10hu]\n",indent,(indent>0?" ":""), self->total_records);
        fprintf(stderr,"%*s[tx_id            %10hu]\n",indent,(indent>0?" ":""), self->tx_id);
        fprintf(stderr,"%*s[packet_size      %10u]\n",indent,(indent>0?" ":""), self->packet_size);
        fprintf(stderr,"%*s[total_size       %10u]\n",indent,(indent>0?" ":""), self->total_size);
        fprintf(stderr,"%*s[seq_number       %10u]\n",indent,(indent>0?" ":""), self->seq_number);
        fprintf(stderr,"%*s[dest_dev_id      %10u]\n",indent,(indent>0?" ":""), self->dest_dev_id);
        fprintf(stderr,"%*s[dest_enumerator  %10hu]\n",indent,(indent>0?" ":""), self->dest_enumerator);
        fprintf(stderr,"%*s[src_enumerator   %10hu]\n",indent,(indent>0?" ":""), self->src_enumerator);
        fprintf(stderr,"%*s[src_dev_id       %10u]\n",indent,(indent>0?" ":""), self->src_dev_id);
    }

}
// End function r7k_nf_show

/// @fn r7k_drf_t * r7k_drf_new()
/// @brief TBD.
/// @return TBD
r7k_drf_t *r7k_drf_new()
{
    r7k_drf_t *self = (r7k_drf_t *)malloc( sizeof(r7k_drf_t) );
    if (NULL!=self) {
        r7k_drf_init(self,true);
    }
    return self;
}
// End function r7k_drf_new

/// @fn void r7k_drf_destroy(r7k_drf_t ** pself)
/// @brief TBD.
/// @param[in] pself TBD
/// @return TBD
void r7k_drf_destroy(r7k_drf_t **pself)
{
    if (pself) {
        r7k_drf_t *self = *pself;
        if (self) {
            free(self);
        }
        *pself=NULL;
    }
}
// End function r7k_drf_destroy

/// @fn void r7k_drf_show(r7k_drf_t * self, _Bool verbose, uint16_t indent)
/// @brief TBD.
/// @param[in] self TBD
/// @param[in] verbose TBD
/// @param[in] indent TBD
/// @return TBD
void r7k_drf_show(r7k_drf_t *self, bool verbose, uint16_t indent)
{
    if (self) {
        fprintf(stderr,"%*s[self            %15p]\n",indent,(indent>0?" ":""), self);
        fprintf(stderr,"%*s[protocol_ver    %15hu]\n",indent,(indent>0?" ":""), self->protocol_version);
        fprintf(stderr,"%*s[offset          %15hu]\n",indent,(indent>0?" ":""), self->offset);
        fprintf(stderr,"%*s[sync_pattern    %*c0x%04x]\n",indent,(indent>0?" ":""),9,' ',self->sync_pattern);
        fprintf(stderr,"%*s[size            %15u]\n",indent,(indent>0?" ":""), self->size);
        fprintf(stderr,"%*s[opt_data_offset %15u]\n",indent,(indent>0?" ":""), self->opt_data_offset);
        fprintf(stderr,"%*s[opt_data_id     %15u]\n",indent,(indent>0?" ":""), self->opt_data_id);
        fprintf(stderr,"%*s[_7ktime   %02hu %03hu %02hhu:%02hhu:%06.3f]\n", \
                indent,(indent>0?" ":""), self->_7ktime.year, self->_7ktime.day, \
                self->_7ktime.hours, self->_7ktime.minutes, self->_7ktime.seconds);
        fprintf(stderr,"%*s[record_version  %15hu]\n",indent,(indent>0?" ":""), self->record_version);
        fprintf(stderr,"%*s[record_type_id  %15u]\n",indent,(indent>0?" ":""), self->record_type_id);
        fprintf(stderr,"%*s[device_id       %15u]\n",indent,(indent>0?" ":""), self->device_id);
        fprintf(stderr,"%*s[reserved0       %15hu]\n",indent,(indent>0?" ":""), self->reserved0);
        fprintf(stderr,"%*s[sys_enumerator  %15hu]\n",indent,(indent>0?" ":""), self->sys_enumerator);
        fprintf(stderr,"%*s[reserved1       %15u]\n",indent,(indent>0?" ":""), self->reserved1);
        fprintf(stderr,"%*s[flags           %15hu]\n",indent,(indent>0?" ":""), self->flags);
        fprintf(stderr,"%*s[reserved2       %15hu]\n",indent,(indent>0?" ":""), self->reserved2);
        fprintf(stderr,"%*s[reserved3       %15u]\n",indent,(indent>0?" ":""), self->reserved3);
        fprintf(stderr,"%*s[total_frag_recs %15u]\n",indent,(indent>0?" ":""), self->total_frag_recs);
        fprintf(stderr,"%*s[frag_number     %15u]\n",indent,(indent>0?" ":""), self->frag_number);
        // this won't work here, since drf has no data, but could be used elsewhere
        // for parsed records
//        if (verbose) {
//            byte *pdata = ((byte *)&self->sync_pattern)+self->offset;
//            uint32_t data_sz = self->size-R7K_DRF_BYTES-R7K_CHECKSUM_BYTES;
//            fprintf(stderr,"%*s[data:            %10p]\n",indent,(indent>0?" ":""),pdata);
//            fprintf(stderr,"%*s[data bytes:      %10u]\n",indent,(indent>0?" ":""),data_sz);
//
//            r7k_hex_show(pdata,data_sz,16,true,indent+3);
//
//            fprintf(stderr,"%*s[checksum          x%08x]\n",indent,(indent>0?" ":""), r7k_drf_get_checksum(self));
//        }
    }
}
// End function r7k_drf_show

/// @fn r7k_checksum_t r7k_drf_get_checksum(r7k_drf_t * self)
/// @brief return checksum of parsed data record frame (DRF).
/// @param[in] self drf reference
/// @return checksum on success, 0 otherwise
r7k_checksum_t r7k_drf_get_checksum(r7k_drf_t *self)
{
    r7k_checksum_t retval=0;

    if (self) {
        byte *bp = (byte *)self;
        r7k_checksum_t *pchk = (r7k_checksum_t *)(bp+self->size-R7K_CHECKSUM_BYTES);
        retval = *pchk;
    }
    return retval;
}
// End function r7k_drf_get_checksum

/// @fn void r7k_drf_init(r7k_drf_t * drf, _Bool erase)
/// @brief initialize a data record frame structure.
/// @param[in] drf data record frame reference
/// @param[in] erase zero DRF structure before initialization
/// @return none
void r7k_drf_init(r7k_drf_t *drf, bool erase)
{
    if (NULL == drf) {
        drf = (r7k_drf_t *)malloc(sizeof(r7k_drf_t));
        memset(drf,0,sizeof(r7k_drf_t));
    }
    if (NULL!=drf) {
        if (erase) {
            memset(drf,0,sizeof(r7k_drf_t));
        }
        // caller must set:
        // size
        // _7ktime
        // record_type_id

        // and optionally set
        // device_id
        // opt_data_offset
        // opt_data_id

        drf->protocol_version = R7K_DRF_PROTO_VER;
        drf->offset           = R7K_DRF_BYTES;
        drf->sync_pattern     = R7K_DRF_SYNC_PATTERN;
        //drf->size;
        drf->opt_data_offset  = 0;
        drf->opt_data_id      = 0;
        //drf->_7ktime;
        drf->record_version   = R7K_DRF_RECORD_VER;
        //drf->record_type_id;
        drf->device_id        = R7K_DEVID_7KCENTER;
        drf->reserved0        = 0;
        drf->sys_enumerator   = R7K_DRF_SYS_ENUM_400KHZ;
        drf->reserved1        = 0;
        drf->flags            = 0x1;
        drf->reserved2        = 0;
        drf->reserved3        = 0;
        drf->total_frag_recs  = 0;
        drf->frag_number      = 0;
    }
}
// End function r7k_drf_init

// DRF container API
/// @fn r7k_drf_container_t * r7k_drfcon_new(uint32_t size)
/// @brief create new data record frame (DRF) container. DRF container
/// may contain multiple frames, and has an API for enumeration, as well as
/// for reading like a file.
/// @param[in] size buffer size
/// @return new drf container reference on success, NULL otherwise
r7k_drf_container_t *r7k_drfcon_new(uint32_t size)
{
    r7k_drf_container_t *self = (r7k_drf_container_t *)malloc(sizeof(r7k_drf_container_t));
    if (self) {
        memset(self,0,sizeof(r7k_drf_container_t));
        self->size=size;
        self->record_count=0;
        self->drf_enum=0;
        self->ofs_sz=R7K_DRFC_RECORD_INC;
        self->ofs_count=0;
        self->ofs_list = (uint32_t *)malloc(R7K_DRFC_RECORD_INC*sizeof(uint32_t));
        memset( self->ofs_list,0,R7K_DRFC_RECORD_INC);

        self->data = (byte *)malloc(size*sizeof(byte));
        memset(self->data,0,size);
        if (NULL != self->data) {
            self->p_write = self->data;
            self->p_read = self->data;
        }else{
            free(self);
            self=NULL;
        }
    }
    return self;
}
// End function r7k_drfcon_new

/// @fn void r7k_drfcon_destroy(r7k_drf_container_t ** pself)
/// @brief release data record frame container resources.
/// @param[in] pself pointer to instance reference
/// @return none
void r7k_drfcon_destroy(r7k_drf_container_t **pself)
{
    if (pself) {
        r7k_drf_container_t *self = *pself;
        if(self){
            if(NULL != self->data){
                free(self->data);
            }
            if (NULL != self->ofs_list) {
                free(self->ofs_list);
            }
            free(self);
            *pself=NULL;
        }
    }
}
// End function r7k_drfcon_destroy

/// @fn void r7k_drfcon_show(r7k_drf_container_t * self, _Bool verbose, uint16_t indent)
/// @brief output data record frame (DRF)  container parameter summary to stderr.
/// @param[in] self DRF container reference
/// @param[in] verbose use verbose output
/// @param[in] indent output indent spaces
/// @return none
void r7k_drfcon_show(r7k_drf_container_t *self, bool verbose, uint16_t indent)
{
    if (NULL != self) {
        fprintf(stderr,"%*s[self         %10p]\n",indent,(indent>0?" ":""), self);
        fprintf(stderr,"%*s[size         %10u]\n",indent,(indent>0?" ":""), self->size);
        fprintf(stderr,"%*s[record_count %10u]\n",indent,(indent>0?" ":""), self->record_count);
        fprintf(stderr,"%*s[data         %10p]\n",indent,(indent>0?" ":""), self->data);
        fprintf(stderr,"%*s[p_read       %10p]\n",indent,(indent>0?" ":""), self->p_read);
        fprintf(stderr,"%*s[p_write      %10p]\n",indent,(indent>0?" ":""), self->p_write);
        fprintf(stderr,"%*s[ofs_list     %10p]\n",indent,(indent>0?" ":""), self->ofs_list);
        fprintf(stderr,"%*s[ofs_sz       %10u]\n",indent,(indent>0?" ":""), self->ofs_sz);
        fprintf(stderr,"%*s[ofs_count    %10u]\n",indent,(indent>0?" ":""), self->ofs_count);
        fprintf(stderr,"%*s[drf_enum     %10u]\n",indent,(indent>0?" ":""), self->drf_enum);
        if (verbose) {
            for (unsigned int i=0; i<self->ofs_count; i++) {
                fprintf(stderr,"%*s[ofs[%02u]  %10u]\n",indent+3,(indent+3>0?" ":""),i, self->ofs_list[i]);
            }
        }
    }
}
// End function r7k_drfcon_show

/// @fn int r7k_drfcon_resize(r7k_drf_container_t * self, uint32_t new_size)
/// @brief resize a data record frame (DRF) container buffer.
/// @param[in] self DRF container reference
/// @param[in] new_size new size of buffer (bytes)
/// @return 0 on success, -1 otherwise
int r7k_drfcon_resize(r7k_drf_container_t *self, uint32_t new_size)
{
    int retval=-1;
    if (NULL!=self && new_size>0) {

        if (new_size > self->size) {
            // save read/write pointer offsets
            off_t rofs  = self->p_read-self->data;
            off_t wofs  = self->p_write-self->data;
            off_t owe   = self->data + self->size - self->p_write;
//            byte *odata    = self->data;
//            uint32_t osize = self->size;
            // increase size
            self->size    += (uint32_t)R7K_DRFC_SIZE_INC;
            self->data     = (byte *)realloc(self->data,(new_size*sizeof(byte)));

            // update read/write pointers
            self->p_read    = self->data+rofs;
            self->p_write   = self->data+wofs;

            // clear new memory
            memset(self->p_write+owe,0,R7K_DRFC_SIZE_INC);
            retval=0;
        }else{
            MX_ERROR_MSG("shrink not implemented\n");
        }

    }else{
        MX_ERROR_MSG("invalid argument\n");
    }
    return retval;
}
// End function r7k_drfcon_resize

/// @fn int r7k_drfcon_add(r7k_drf_container_t * self, byte * src, uint32_t len)
/// @brief add a new data record frame (DRF) to a DRF container.
/// @param[in] self DRF container reference
/// @param[in] src source data (containing parsed DRF)
/// @param[in] len size of new record to add
/// @return ME_OK on success, -1 otherwise and me_errno is set
int r7k_drfcon_add(r7k_drf_container_t *self, byte *src, uint32_t len)
{
    me_errno=ME_OK;

    int retval=-1;
    if (NULL!=self && NULL!=src) {

        if ( len <= r7k_drfcon_space(self) ) {

            // save record offset
            uint32_t record_ofs = self->p_write-self->data;

            // add to offset table if needed
            if ( (self->ofs_count!=0) && (self->ofs_count%self->ofs_sz) == 0) {
                uint32_t *mp = NULL;
                if( (mp = (uint32_t *)realloc(self->ofs_list, (size_t)(R7K_DRFC_RECORD_INC+self->ofs_sz)*sizeof(uint32_t) ))!=NULL){
                    self->ofs_list = mp;
                    self->ofs_sz+=R7K_DRFC_RECORD_INC;
                    memset( &self->ofs_list[self->ofs_count],0,R7K_DRFC_RECORD_INC*sizeof(uint32_t));
                }else{
                    MX_ERROR_MSG("record offset realloc failed\n");
                    me_errno = ME_ENOMEM;
                }
            }

            if (me_errno==ME_OK) {
                // add record
                memcpy(self->p_write,src,len);

                // update count, pointers
                self->p_write += len;

                // add record offset entry
                self->ofs_list[self->ofs_count] = record_ofs;
                self->ofs_count++;
                self->record_count++;

                retval=0;
            }
        }else{
            MX_MPRINT(R7KR_DEBUG, "no space in container cap/spc/req[%u/%u/%u]\n", self->size, r7k_drfcon_space(self), len);
            me_errno = ME_ENOSPACE;
        }
    }else{
        MX_ERROR_MSG("invalid argument\n");
    }
    return retval;
}
// End function r7k_drfcon_add

/// @fn int r7k_drfcon_flush(r7k_drf_container_t * self)
/// @brief clear data record frame (DRF) container buffer.
/// @param[in] self DRF reference
/// @return 0 on success, -1 otherwise
int r7k_drfcon_flush(r7k_drf_container_t *self)
{
    int retval=-1;
    if (NULL!=self && NULL!=self->data ) {
        if (r7k_drfcon_length(self)>0) {
            memset(self->data,0,self->size);
            self->p_read=self->data;
            self->p_write=self->data;
            if (NULL!=self->ofs_list && self->ofs_count>0) {
                memset(self->ofs_list,0,self->ofs_sz*sizeof(uint32_t));
                self->ofs_count=0;
            }
            self->record_count=0;
        }
        retval=0;
    }else{
        MX_ERROR_MSG("invalid argument]\n");
    }
    return retval;
}
// End function r7k_drfcon_flush

/// @fn int r7k_drfcon_seek(r7k_drf_container_t * self, uint32_t ofs)
/// @brief set data record frame (DRF) container output (read) pointer offset.
/// @param[in] self DRF container reference
/// @param[in] ofs new offset
/// @return new offset on success (>=0), -1 otherwise
int r7k_drfcon_seek(r7k_drf_container_t *self, uint32_t ofs)
{
    int retval=-1;
    if (NULL!=self && NULL!=self->data && ofs<self->size && (self->data+ofs)<=self->p_write) {
       MX_MPRINT(R7KR_DEBUG, "sz[%u] ofs[%u]\n", self->size, ofs);
        self->p_read = self->data+ofs;
        retval = 0;
    }
    return retval;
}
// End function r7k_drfcon_seek

/// @fn uint32_t r7k_drfcon_tell(r7k_drf_container_t * self)
/// @brief return current output (read) pointer offset.
/// @param[in] self DRF container reference
/// @return output pointer offset (>=0) on success, -1 otherwise
uint32_t r7k_drfcon_tell(r7k_drf_container_t *self)
{
    uint32_t retval=0;
    if (NULL!=self && NULL!=self->data) {
        retval = self->p_read - self->data;
    }
    return retval;
}
// End function r7k_drfcon_tell

/// @fn uint32_t r7k_drfcon_read(r7k_drf_container_t * self, byte * dest, uint32_t len)
/// @brief read bytes from the data record frame (DRF) container.
/// @param[in] self DRF container reference
/// @param[in] dest buffer to read data into
/// @param[in] len number of bytes to read
/// @return number of bytes read (>=0) on success, -1 otherwise
uint32_t r7k_drfcon_read(r7k_drf_container_t *self, byte *dest, uint32_t len)
{
    uint32_t retval=0;

    if (NULL!=self && NULL!=self->data && len!=0 ) {
        uint32_t read_len = 0;

        if (len<r7k_drfcon_pending(self)) {
            read_len=len;
        }else{
            read_len = r7k_drfcon_pending(self);
        }
        if (read_len>0) {
            memcpy(dest,self->p_read,read_len);
            self->p_read += read_len;
            if (self->p_read > self->p_write) {
                MX_MMSG(R7KC_DRFCON, "pread>pwrite\n");
                self->p_read = self->p_write;
            }else if (self->p_read > (self->data+self->size)) {
                MX_MMSG(R7KC_DRFCON, "pread>data+size\n");
                self->p_read = self->data+self->size;
            }
            retval = read_len;
        }else{
//           MX_MPRINT(R7KR_DEBUG, "read_len <= 0 [%u]\n",read_len));
//            r7k_drfcon_show(self,false,5);
        }
    }else{
        MX_ERROR_MSG("invalid argument\n");
    }
    return retval;
}
// End function r7k_drfcon_read

/// @fn uint32_t r7k_drfcon_size(r7k_drf_container_t * self)
/// @brief return total capacity (bytes) of data record frame (DRF) container.
/// @param[in] self DRF container reference
/// @return number of bytes (>=0) on success, -1 otherwise
uint32_t r7k_drfcon_size(r7k_drf_container_t *self)
{
    uint32_t retval=-1;
    if (NULL!=self){
        retval = self->size;
    }
    return retval;
}
// End function r7k_drfcon_size

/// @fn uint32_t r7k_drfcon_length(r7k_drf_container_t * self)
/// @brief total number of bytes currently in data record frame (DRF) container.
/// @param[in] self DRF container reference
/// @return number of bytes (>=0) on success, -1 otherwise
uint32_t r7k_drfcon_length(r7k_drf_container_t *self)
{
    uint32_t retval=-1;
    if (NULL!=self){
        retval = self->p_write - self->data;
    }
    return retval;
}
// End function r7k_drfcon_length

/// @fn uint32_t r7k_drfcon_pending(r7k_drf_container_t * self)
/// @brief return number of unread bytes in data record frame (DRF) container.
/// @param[in] self DRF container reference
/// @return number of bytes (>=0) on success, -1 otherwise
uint32_t r7k_drfcon_pending(r7k_drf_container_t *self)
{
    uint32_t retval=0;
    if (NULL!=self){
        retval = self->p_write - self->p_read;
    }
   return retval;
}
// End function r7k_drfcon_pending

/// @fn uint32_t r7k_drfcon_space(r7k_drf_container_t * self)
/// @brief return amount of space available (bytes) for writing in data record frame (DRF) container.
/// @param[in] self DRF container reference
/// @return number of bytes (>=0) on success, -1 otherwise
uint32_t r7k_drfcon_space(r7k_drf_container_t *self)
{
    uint32_t retval=-1;
    if (NULL!=self){
        retval = self->data + self->size - self->p_write;
    }
    return retval;
}
// End function r7k_drfcon_space

/// @fn uint32_t r7k_drfcon_frames(r7k_drf_container_t * self)
/// @brief number of data record frames (DRFs) in container.
/// @param[in] self DRF container reference
/// @return number of frames (>=0) on success, -1 otherwise
uint32_t r7k_drfcon_frames(r7k_drf_container_t *self)
{
    uint32_t retval=-1;
    if (NULL!=self){
        retval = self->record_count;
    }
    return retval;
}
// End function r7k_drfcon_frames

/// @fn int r7k_drfcon_bytes(r7k_drf_container_t * self, uint32_t ofs, byte * dest, uint32_t len)
/// @brief copy bytes from a data record frame (DRF) container to a specified buffer.
/// @param[in] self DRF container reference
/// @param[in] ofs starting DRF offset
/// @param[in] dest destination data buffer
/// @param[in] len number of bytes to copy
/// @return TBD
int r7k_drfcon_bytes(r7k_drf_container_t *self, uint32_t ofs, byte *dest, uint32_t len)
{
    int retval=-1;
    if (NULL!=self &&
        NULL!=self->data &&
        len < (self->size - ofs) &&
        ofs < self->size ){

        memcpy(dest,self->data+ofs,len);
        retval = 0;
    }
    return retval;
}
// End function r7k_drfcon_bytes

/// @fn r7k_drf_t * r7k_drfcon_enumerate(r7k_drf_container_t * self)
/// @brief return first data record frame (DRF) in container. subsequent calls
/// to r7k_drfcon_next() will return the next frame(s) in the container.
/// @param[in] self DRF container reference
/// @return first DRF reference on success, NULL when the last frame is reached
r7k_drf_t* r7k_drfcon_enumerate(r7k_drf_container_t *self)
{
    r7k_drf_t *retval = NULL;
    if (NULL!=self && self->record_count>0){
        self->drf_enum = 0;
        retval = (r7k_drf_t *)(self->data+self->ofs_list[self->drf_enum]);
        self->drf_enum++;
    }
    return retval;
}
// End function r7k_drfcon_enumerate

/// @fn r7k_drf_t * r7k_drfcon_next(r7k_drf_container_t * self)
/// @brief return next first data record frame (DRF) in container.
/// To begin enumeration, first call r7k_drfcon_enumerate();
/// @param[in] self DRF container reference
/// @return DRF reference on success, NULL when no frames remain
r7k_drf_t* r7k_drfcon_next(r7k_drf_container_t *self)
{
    r7k_drf_t *retval = NULL;
    if (NULL!=self){

        if (self->drf_enum < self->ofs_count) {
            if (self->ofs_list[self->drf_enum] < self->size) {
                retval = (r7k_drf_t *)(self->data+self->ofs_list[self->drf_enum]);
                self->drf_enum++;
            }
        }
    }
    return retval;
}
// End function r7k_drfcon_next

/// @fn r7k_msg_t * r7k_msg_new(uint32_t data_len)
/// @brief create new r7k protocol message structure.
/// r7k messages have DRF and NF elements, but must be explicitly
/// serialized before sending to 7k center.
/// currently only used in mbtrn-server.
/// @param[in] data_len number of message data bytes
/// @return new message reference on success, NULL otherwise.
r7k_msg_t *r7k_msg_new(uint32_t data_len)
{

    r7k_msg_t *self = (r7k_msg_t *)malloc(sizeof(r7k_msg_t));
    if (NULL != self) {
        memset(self,0,sizeof(r7k_msg_t));
        self->nf      = r7k_nf_new();
        self->drf     = r7k_drf_new();
        self->msg_len = R7K_NF_BYTES;
        self->msg_len += R7K_DRF_BYTES;
        self->msg_len += data_len;
        self->msg_len += R7K_CHECKSUM_BYTES;

        self->data_size = data_len;
        if (data_len>0) {
            self->data=(byte *)malloc(data_len*sizeof(byte));
            memset(self->data,0,data_len*sizeof(byte));
        }else{
            self->data=NULL;
        }
        r7k_update_time(&self->drf->_7ktime);
    }
    return self;
}
// End function r7k_msg_new

/// @fn void r7k_msg_destroy(r7k_msg_t ** pself)
/// @brief release message structure resources.
/// @param[in] pself pointer to message reference
/// @return none
void r7k_msg_destroy(r7k_msg_t **pself)
{
    if (pself) {
        r7k_msg_t *self = *pself;
        if (self) {
            if (NULL != self->nf) {
                free(self->nf);
            }
            if (NULL != self->drf) {
                free(self->drf);
            }
            if (NULL != self->data) {
                free(self->data);
            }
            free(self);
        }
        *pself=NULL;
    }
}
// End function r7k_msg_destroy

/// @fn void r7k_msg_show(r7k_msg_t * self, _Bool verbose, uint16_t indent)
/// @brief output r7k message parameter summary to stderr.
/// @param[in] self r7k message reference
/// @param[in] verbose use verbose output
/// @param[in] indent output indent spaces
/// @return none
void r7k_msg_show(r7k_msg_t *self, bool verbose, uint16_t indent)
{
    if (self) {
        fprintf(stderr,"%*s[self     %10p]\n",indent,(indent>0?" ":""), self);
        fprintf(stderr,"%*s[msg_len %10u]\n",indent,(indent>0?" ":""), self->msg_len);
        fprintf(stderr,"%*s[nf       %10p]\n",indent,(indent>0?" ":""), self->nf);
        if (verbose) {
            r7k_nf_show(self->nf,verbose,indent+3);
        }
        fprintf(stderr,"%*s[drf      %10p]\n",indent,(indent>0?" ":""), self->drf);
        if (verbose) {
            r7k_drf_show(self->drf,verbose,indent+3);
        }
        fprintf(stderr,"%*s[data_size %10u]\n",indent,(indent>0?" ":""), self->data_size);
        fprintf(stderr,"%*s[data       %10p]\n",indent,(indent>0?" ":""), self->data);
        if (verbose) {
            r7k_hex_show(self->data,self->data_size,16,true,indent+3);
        }

        fprintf(stderr,"%*s[checksum 0x%08X]\n",indent,(indent>0?" ":""), self->checksum);
    }else{
        fprintf(stderr,"%*s[self %10p (NULL message)]\n",indent,(indent>0?" ":""), self);

    }
}
// End function r7k_msg_show

/// @fn uint32_t r7k_msg_set_checksum(r7k_msg_t * self)
/// @brief set the checksum for an r7k message structure.
/// @param[in] self r7k message reference
/// @return previous checksum value.
uint32_t r7k_msg_set_checksum(r7k_msg_t *self)
{
    uint32_t cs_save=0;
    if (NULL!=self) {
        cs_save = self->checksum;
        // compute checksum over
        // DRF, RTH, record data, optional data
        byte *bp = (byte *)self->drf;
//        uint32_t cs_len = self->drf->size-R7K_CHECKSUM_BYTES;
//        self->checksum=r7k_checksum(bp,cs_len);

        for (uint32_t i=0; i<(R7K_DRF_BYTES); i++) {
            self->checksum += *(bp+i);
        }
        if (self->data_size>0) {
            bp = (byte *)self->data;
            for (uint32_t i=0; i<self->data_size; i++) {
                self->checksum += *(bp+i);
            }
        }
    }
    return cs_save;
}
// End function r7k_msg_set_checksum

/// @fn byte * r7k_msg_serialize(r7k_msg_t * self)
/// @brief serialize r7k message into new network frame buffer.
/// caller must release frame buffer resources.
/// @param[in] self r7k message reference
/// @return new network frame buffer on success, NULL otherwise
byte *r7k_msg_serialize(r7k_msg_t *self)
{
    byte *retval = NULL;
    if ( (NULL!=self)
        &&
        (NULL != self->nf)
        &&
        (NULL != self->drf)
        &&
        (NULL != self->data)
        &&
        (self->data_size>0)
        &&
        (self->msg_len==(self->data_size+R7K_NF_BYTES+R7K_DRF_BYTES+R7K_CHECKSUM_BYTES))) {

        size_t bufsz = (self->msg_len)*sizeof(byte);
//        fprintf(stderr,"r7k_msg_serialize - bufsz[%"PRIu32"] msg->data_size[%"PRIu32"]\n",bufsz,self->data_size);
        retval = (byte *)malloc( bufsz );

        if (retval) {
            byte *pnf  = retval;
            byte *pdrf = pnf+R7K_NF_BYTES;
            byte *pdata = pdrf+R7K_DRF_BYTES;
            byte *pchk = pdrf+self->drf->size-R7K_CHECKSUM_BYTES;//pdata+self->data_size;

            memcpy(pnf,self->nf,R7K_NF_BYTES);
            memcpy(pdrf,self->drf,R7K_DRF_BYTES);
            memcpy(pdata,self->data,self->data_size);
            memcpy(pchk,&self->checksum,R7K_CHECKSUM_BYTES);
        }
    }else{
        MX_ERROR_MSG("invalid argument\n");
    }

    return retval;
}
// End function r7k_msg_serialize

/// @fn int r7k_msg_receive(msock_socket_t * s, r7k_msg_t ** dest, uint32_t timeout_msec)
/// @brief receive network frame from 7k center into r7k message structure.
/// @param[in] s socket reference
/// @param[in] dest pointer to r7k message reference to hold message
/// @param[in] timeout_msec read timeout
/// @return number of bytes received on success, -1 otherwise.
int r7k_msg_receive(msock_socket_t *s, r7k_msg_t **dest, uint32_t timeout_msec)
{
    int retval=-1;
    if (NULL != s && s->status==SS_CONNECTED) {

       int64_t nbytes=0;
        // read nf, drf headers
        int64_t header_len = sizeof(r7k_nf_headers_t);

        //byte headers[header_len];	// Invalid C.	JL
        byte *headers;
        headers = (byte *)malloc(header_len+1);

        memset(headers,0,header_len);
        if ( (nbytes=msock_read_tmout(s,headers,header_len,timeout_msec)) == header_len) {
            int64_t total_len=nbytes;
            MX_MPRINT(R7KR_DEBUG, "read headers [%"PRId64"/%"PRId64"]\n", nbytes, header_len);
            // get frame content
            r7k_nf_t *nf = (r7k_nf_t *)(headers);
            r7k_drf_t *drf = (r7k_drf_t *)(headers+sizeof(r7k_nf_t));
			// get size of data (RTH, RD, OD, checksum)
            uint32_t read_len = drf->size - sizeof(r7k_drf_t);
            uint32_t data_len = read_len-sizeof(r7k_checksum_t);
//           MX_MMSG(R7KR_DEBUG, "ACK nf:\n");
//            r7k_nf_show(nf,true,5);
//           MX_MMSG(R7KR_DEBUG, "ACK drf:\n");
//            r7k_drf_show(drf,true,5);
            MX_MPRINT(R7KR_DEBUG, "data_len[%u] read_len[%u]\n", data_len, read_len);
            // read rth/rd/od [if any], checksum
            if (read_len>0) {
                //byte data[read_len];		// INVALID C. JL
                byte *data;
                data = (byte *)malloc(read_len);
                memset(data,0,read_len);
                if ( (nbytes=msock_read_tmout(s,data,read_len,timeout_msec)) == read_len) {
                    total_len+=nbytes;
                   MX_MPRINT(R7KR_DEBUG, "read data [%"PRId64"/%d] -> %p\n", nbytes, read_len, dest);
                    // TODO: validate content
                    // create message
                    r7k_msg_t *m = r7k_msg_new(data_len);
                    if (NULL!=m) {
                        memcpy(m->nf,nf,sizeof(r7k_nf_t));
                        memcpy(m->drf,drf,sizeof(r7k_drf_t));
                        memcpy(m->data,data,data_len);
                        memcpy(&m->checksum,(data+data_len),sizeof(r7k_checksum_t));
//                       MX_MMSG(R7KR_DEBUG, "dest msg:\n");
//                        r7k_msg_show(m,true,6);
                        *dest=m;
                        retval=total_len;
                    }else{
                        MX_ERROR_MSG("recv - msg_new failed\n");
                    }
                }else{
                   MX_MPRINT(R7KR_DEBUG, "recv - incomplete data read nbytes[%"PRId64"] data_len[%u]\n", nbytes, data_len);
                }
                free(data);
            }else{
               MX_MPRINT(R7KR_DEBUG, "recv - read_len <= 0 nbytes[%"PRId64"] read_len[%u]\n", nbytes, read_len);
            }
        }else{
           MX_MPRINT(R7KR_DEBUG, "recv - incomplete header read? nbytes[%"PRId64"] header_len[%"PRId64"]\n", nbytes, header_len);
        }
        free(headers);
    }else{
        MX_MPRINT(R7KR_DEBUG, "recv - invalid socket or status s[%p, %d/%d]\n", s, (s!=NULL?s->status:0), SS_CONNECTED);
    }

    return retval;
}
// End function r7k_msg_receive

/// @fn int r7k_msg_send(msock_socket_t * s, r7k_msg_t * self)
/// @brief serialize and send an r7k message to 7k center.
/// @param[in] s socket reference
/// @param[in] self r7k message structure
/// @return number of bytes sent (>=0) on success, -1 otherwise
int r7k_msg_send(msock_socket_t *s, r7k_msg_t *self)
{
    int retval=-1;
    if ( (NULL != self) && (NULL!=s)) {

        byte *buf = r7k_msg_serialize(self);
//       MX_MMSG(R7KR_DEBUG, "SEND nf:\n");
//        r7k_nf_show((r7k_nf_t *)buf,true,4);
        int64_t status=0;

        if( (status=msock_send(s,buf,self->msg_len))>0){
            retval=0;
            MX_MPRINT(R7KR_DEBUG, "send OK s[%p, %d]\n", s, s->status);
        }else{
            MX_ERROR("send failed [%"PRId64"] [%d/%s]\n", status, errno, strerror(errno));
        }

        free(buf);
    }else{

                MX_MMSG(R7KR_DEBUG, "invalid socket or message\n");
   }
    return retval;
}
// End function r7k_msg_send

/// @fn int r7k_test()
/// @brief r7k unit test(s).
/// currently subscribes to test server (exercising most of the r7k API).
/// @return 0 on success, -1 otherwise
int r7k_test()
{
    int retval=-1;
    MX_MMSG(R7KR_DEBUG, "entering...\n");
    uint32_t sub_recs[2]={1000,2000};
    MX_MMSG(R7KR_DEBUG, "create/connect socket...\n");
    msock_socket_t *s = msock_socket_new("localhost",R7K_7KCENTER_PORT,ST_TCP);
    msock_connect(s);
    MX_MMSG(R7KR_DEBUG, "subscribing...\n");
    retval = r7k_subscribe(s,R7KC_DEV_7125_400KHZ,sub_recs,2);
    MX_MMSG(R7KR_DEBUG, "releasing resources...\n");
    msock_socket_destroy(&s);

//    byte random[50];
//    for (int i=0; i<50; i++) {
//        random[i]=i%256+20;
//    }
//   MX_MMSG(R7KR_DEBUG, "hex_show 30/9/f/5\n");
//    r7k_hex_show(random,30,9,false,5);
//   MX_MMSG(R7KR_DEBUG, "hex_show 30/7/t/5\n");
//    r7k_hex_show(random,30,7,true,5);
//   MX_MMSG(R7KR_DEBUG, "hex_show 30/10/t,5\n");
//    r7k_hex_show(random,30,10,true,5);
    return retval;
}
// End function r7k_test
