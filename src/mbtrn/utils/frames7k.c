///
/// @file frames7k.c
/// @authors k. Headley
/// @date 01 jan 2018

/// test application: subscribe to reson and stream parsed frames to console

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
#include "r7kc.h"
#include "msocket.h"
#include "mxdebug.h"
#include "mxd_app.h"
#include "r7k-reader.h"
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

/// @def RESON_HOST_DFL
/// @brief default reson hostname
#define RESON_HOST_DFL "localhost"

#define FRAMES7K_NAME "frames7k"
#ifndef FRAMES7K_VER
/// @def FRAMES7K_VER
/// @brief module build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DFRAMES7K_VER=<version>
#define FRAMES7K_VER (dev)
#endif
#ifndef FRAMES7K_BUILD
/// @def FRAMES7K_BUILD
/// @brief module build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DMFRAME_BUILD=`date`
#define FRAMES7K_BUILD VERSION_STRING(FRAMES7K_VER)" "LIBMFRAME_BUILD
#endif

/////////////////////////
// Declarations
/////////////////////////

/// @typedef struct app_cfg_s app_cfg_t
/// @brief application configuration parameter structure
typedef struct app_cfg_s{
    /// @var app_cfg_s::verbose
    /// @brief verbose output flag
    int verbose;
    /// @var app_cfg_s::host
    /// @brief hostname
    char *host;
    /// @var app_cfg_s::cycles
    /// @brief number of cycles (<=0 : unlimited)
    int cycles;
    /// @var app_cfg_s::size
    /// @brief frame buffer size
    uint32_t size;
    /// @var app_cfg_s::id
    /// @brief reader id
    r7k_device_t dev;
}app_cfg_t;

static void s_show_help();
static bool g_stop_flag=false;

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
    char help_message[] = "\n Stream reson data frames to console\n";
    char usage_message[] = "\n frames7k [options]\n"
    " Options :\n"
    "  --verbose=n : verbose output\n"
    "  --host      : reson host name or IP address\n"
    "  --cycles    : number of cycles (dfl 0 - until CTRL-C)\n"
    "  --size      : reader capacity (bytes)\n"
    "  --dev=s     : device  [e.g. T50, 7125_400]\n"
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
        {"cycles", required_argument, NULL, 0},
        {"size", required_argument, NULL, 0},
        {"dev", required_argument, NULL, 0},
        {NULL, 0, NULL, 0}};

    /* process argument list */
    while ((c = getopt_long(argc, argv, "", options, &option_index)) != -1){
        switch (c) {
                // long options all return c=0
            case 0:
                // verbose
                if (strcmp("verbose", options[option_index].name) == 0) {
                    sscanf(optarg,"%d",&cfg->verbose);
                }
                // version
                if (strcmp("version", options[option_index].name) == 0) {
                    version=true;
                }
                
                // help
                else if (strcmp("help", options[option_index].name) == 0) {
                    help = true;
                }
                
                // host
                else if (strcmp("host", options[option_index].name) == 0) {
                    cfg->host=strdup(optarg);
                }
                // cycles
                else if (strcmp("cycles", options[option_index].name) == 0) {
                    sscanf(optarg,"%d",&cfg->cycles);
                }
                // size
                else if (strcmp("size", options[option_index].name) == 0) {
                    sscanf(optarg,"%u",&cfg->size);
                }
                // dev
                else if (strcmp("dev", options[option_index].name) == 0) {
                    r7k_device_t test = R7KC_DEV_INVALID;
                    if( (test=r7k_parse_devid(optarg)) != R7KC_DEV_INVALID){
                        cfg->dev = test;
                    }
                }
                break;
            default:
                help=true;
                break;
        }
        if (version) {
            MFRAME_SHOW_VERSION(FRAMES7K_NAME, FRAMES7K_BUILD);

//            r7kr_show_app_version(FRAMES7K_NAME,FRAMES7K_BUILD);
            exit(0);
       }
        if (help) {
            MFRAME_SHOW_VERSION(FRAMES7K_NAME, FRAMES7K_BUILD);
//           r7kr_show_app_version(FRAMES7K_NAME,FRAMES7K_BUILD);
            s_show_help();
            exit(0);
        }
    }// while

    mxd_setModule(MXDEBUG, 0, true, NULL);
    mxd_setModule(MXERROR, 5, false, NULL);
    mxd_setModule(FRAMES7K, 1, false, "trnc.error");
    mxd_setModule(FRAMES7K_ERROR, 1, true, "trnc.error");
    mxd_setModule(FRAMES7K_DEBUG, 1, true, "trnc.debug");
    mxd_setModule(MXMSOCK, 1, true, "msock");
    mxd_setModule(R7KC, 1, true, "r7kc");
    mxd_setModule(R7KC_DEBUG, 1, true, "r7kc.debug");
    mxd_setModule(R7KC_ERROR, 1, true, "r7kc.error");
    mxd_setModule(R7KR, 1, true, "r7kr");
    mxd_setModule(R7KR_ERROR, 1, true, "r7kr.error");
    mxd_setModule(R7KR_DEBUG, 1, true, "r7kr.debug");

    switch (cfg->verbose) {
        case 0:
            break;
        case 1:
            mxd_setModule(MXDEBUG, 0, true, NULL);
            mxd_setModule(MXERROR, 5, false, NULL);
            mxd_setModule(FRAMES7K, 1, false, "trnc.error");
            break;
        case 2:
            mxd_setModule(MXDEBUG, 5, true, NULL);
            mxd_setModule(MXERROR, 5, false, NULL);
            mxd_setModule(FRAMES7K, 5, false, "trnc.error");
            break;
        case 3:
        case 4:
        case 5:
            mxd_setModule(MXDEBUG, 5, false, NULL);
            mxd_setModule(MXERROR, 5, false, NULL);
            mxd_setModule(FRAMES7K_ERROR, 5, false, "trnc.error");
            mxd_setModule(FRAMES7K_DEBUG, 5, false, "trnc.debug");
            mxd_setModule(MXMSOCK, 5, false, "msock");
            mxd_setModule(R7KC, 5, false, "r7kc");
            mxd_setModule(R7KC_DEBUG, 5, false, "r7kc.debug");
            mxd_setModule(R7KC_ERROR, 5, false, "r7kc.error");
            mxd_setModule(R7KR, 5, false, "r7kr");
            mxd_setModule(R7KR_ERROR, 5, false, "r7kr.error");
            mxd_setModule(R7KR_DEBUG, 5, false, "r7kr.debug");
            break;
        default:
            break;
    }

    if(cfg->verbose != 0)
        mxd_show();

//    mconf_init(NULL,NULL);
//    mmd_channel_set(MOD_F7K,MM_ERR);
//    mmd_channel_set(MOD_R7K,MM_ERR);
//    mmd_channel_set(MOD_R7KR,MM_ERR);
//    mmd_channel_set(MOD_MSOCK,MM_ERR);
//
//    switch (cfg->verbose) {
//        case 0:
//            mmd_channel_set(MOD_F7K,0);
//            mmd_channel_set(MOD_R7K,0);
//            mmd_channel_set(MOD_R7KR,0);
//            mmd_channel_set(MOD_MSOCK,0);
//            break;
//        case 1:
//            mmd_channel_en(MOD_F7K,S7K_V1);
//            mmd_channel_en(MOD_F7K,MM_DEBUG);
//            break;
//        case 2:
//            mmd_channel_en(MOD_F7K,S7K_V1);
//            mmd_channel_en(MOD_F7K,S7K_V2);
//            mmd_channel_en(MOD_F7K,MM_DEBUG);
//            mmd_channel_en(MOD_R7KR,MM_DEBUG);
//            break;
//        default:
//            if(cfg->verbose>2){
//                mmd_channel_en(MOD_F7K,S7K_V1);
//                mmd_channel_en(MOD_F7K,S7K_V2);
//                mmd_channel_en(MOD_F7K,MM_DEBUG);
//                mmd_channel_en(MOD_MSOCK,MM_DEBUG);
//                mmd_channel_en(MOD_R7K,MM_DEBUG|R7K_V2);
//                mmd_channel_en(MOD_R7KR,MM_DEBUG);
//            }
//            break;
//    }
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
            MX_LPRINT(FRAMES7K, 2, "received sig[%d]\n", signum);
            g_stop_flag=true;
            break;
        default:
            MX_ERROR_MSG("unhandled signal[%d]\n", signum);
            break;
    }
}
// End function termination_handler

/// @fn int s_app_main (app_cfg_t *cfg)
/// @brief app main.
/// @param[in] cfg app_cfg_t reference
/// @return 0 on success, -1 otherwise
static int s_app_main (app_cfg_t *cfg)
{
    int retval=-1;
    
    if (NULL!=cfg) {
        uint32_t nsubs=11;
        uint32_t subs[]={1003, 1006, 1008, 1010, 1012, 1013, 1015,
            1016, 7000, 7004, 7027};
        
        int count = 0;
        bool forever = false;
        if(cfg->cycles<=0){
            forever=true;
        }
        
        // initialize reader
        // create and open socket connection
        r7kr_reader_t *reader = r7kr_reader_new(cfg->dev, cfg->host,R7K_7KCENTER_PORT,cfg->size, subs, nsubs);
        
        // show reader config
        if (cfg->verbose>1) {
            r7kr_reader_show(reader,true, 5);
        }

        uint32_t lost_bytes=0;
        // test r7kr_read_frame
        byte frame_buf[MAX_FRAME_BYTES_7K]={0};
        
        MX_LPRINT(FRAMES7K, 2, "reader connected [%s/%d] err(%s)\n", cfg->host,R7K_7KCENTER_PORT,  me_strerror(me_errno));
        
        retval=0;
        int read_retries=5;
        while ( (forever || (count<cfg->cycles)) && !g_stop_flag) {
            int istat=0;
            count++;
            // clear frame buffer
            memset(frame_buf,0,MAX_FRAME_BYTES_7K);
            // read frame
            if( (istat = r7kr_read_frame(reader, frame_buf, MAX_FRAME_BYTES_7K, R7KR_NET_STREAM, 0.0, R7KR_READ_TMOUT_MSEC,&lost_bytes )) > 0){
                read_retries=5;

                MX_LPRINT(FRAMES7K, 1, "r7kr_read_frame cycle[%d/%d] ret[%d] lost[%"PRIu32"]\n", count, cfg->cycles, istat, lost_bytes);
                // show contents
                if (cfg->verbose>=1) {
                    r7k_nf_t *nf = (r7k_nf_t *)(frame_buf);
                    r7k_drf_t *drf = (r7k_drf_t *)(frame_buf+R7K_NF_BYTES);
                    MX_LMSG(FRAMES7K, 1, "NF:\n");
                    r7k_nf_show(nf,false,5);
                    MX_LMSG(FRAMES7K, 1, "DRF:\n");
                    r7k_drf_show(drf,false,5);
                    if(cfg->verbose>3){
                        MX_LMSG(FRAMES7K, 1, "data:\n");
                        if (istat>0 && cfg->verbose>1) {
                            r7k_hex_show(frame_buf,istat,16,true,5);
                        }
                    }
                }
            }else{
                // read error
                MX_ERROR("ERR - r7kr_read_frame - cycle[%d/%d] ret[%d] me_err[%d] lost[%d]\n", count+1, cfg->cycles, istat, (me_errno-ME_ERRORNO_BASE), lost_bytes);
                if (me_errno==ME_ESOCK || me_errno==ME_EOF || me_errno==ME_ERECV || (read_retries-- <= 0)) {
                    MX_ERROR_MSG("socket closed - reconnecting in 5 sec\n");
                    sleep(5);
                    r7kr_reader_connect(reader,true);
                    read_retries=5;
                }
            }
        }
        
        if (g_stop_flag) {
            MX_LPRINT(FRAMES7K, 2, "interrupted - exiting cycles[%d/%d]\n", count, cfg->cycles);
        }else{
            MX_LPRINT(FRAMES7K, 2, "cycles[%d/%d]\n", count, cfg->cycles);
        }
    }// else invalid argument
    return retval;
}
// End function s_app_main

/// @fn int main(int argc, char ** argv)
/// @brief frames7k main entry point.
/// subscribe to reson 7k center data streams, and output
/// parsed data record frames to stderr.
/// Use argument --cycles=x, x<=0  to stream indefinitely
/// @param[in] argc number of command line arguments
/// @param[in] argv array of command line arguments (strings)
/// @return 0 on success, -1 otherwise
int main(int argc, char **argv)
{
    int retval=-1;
    
    // configure signal handling
    // for main thread
    struct sigaction saStruct;
    sigemptyset(&saStruct.sa_mask);
    saStruct.sa_flags = 0;
    saStruct.sa_handler = s_termination_handler;
    sigaction(SIGINT, &saStruct, NULL);
    
    app_cfg_t cfg_s = {1,strdup(RESON_HOST_DFL),0,MAX_FRAME_BYTES_7K,R7KC_DEV_7125_400KHZ};
    app_cfg_t *cfg = &cfg_s;

    // parse command line options
    parse_args(argc, argv, cfg);

    // run app
    retval=s_app_main(cfg);
    
    return retval;
}
// End function main

