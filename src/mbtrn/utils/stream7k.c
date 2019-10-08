///
/// @file stream7k.c
/// @authors k. Headley
/// @date 01 jan 2018

/// test application: subscribe to reson and stream bytes to console

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
#include "mframe.h"
#include "msocket.h"
#include "merror.h"
#include "r7k-reader.h"
#include "mmdebug.h"
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

#define STREAM7K_NAME "stream7k"
#ifndef STREAM7K_VER
/// @def STREAM7K_VER
/// @brief module build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DSTREAM7K_VER=<version>
#define STREAM7K_VER (dev)
#endif
#ifndef STREAM7K_BUILD
/// @def STREAM7K_BUILD
/// @brief module build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DMFRAME_BUILD=`date`
#define STREAM7K_BUILD VERSION_STRING(STREAM7K_VER)" "LIBMFRAME_BUILD
#endif

/// @def RESON_HOST_DFL
/// @brief default reson hostname
#define RESON_HOST_DFL "localhost"

/////////////////////////
// Declarations
/////////////////////////

/// @enum stream7k_module_ids
/// @brief application module IDs
/// [note : starting above reserved mframe module IDs]
//typedef enum{
//    MOD_S7K=MM_MODULE_COUNT,
//    APP_MODULE_COUNT
//}stream7k_module_ids;

///// @enum s7k_channel_id
///// @brief test module channel IDs
///// [note : starting above reserved mframe channel IDs]
//typedef enum{
//    ID_S7K_V1=MM_CHANNEL_COUNT,
//    ID_S7K_V2,
//    S7K_CH_COUNT
//}s7k_channel_id;
//
///// @enum s7k_channel_mask
///// @brief test module channel masks
//typedef enum{
//    S7K_V1= (1<<ID_S7K_V1),
//    S7K_V2= (1<<ID_S7K_V2)
//}s7k_channel_mask;
//
///// @var char *mmd_test_m1_ch_names[S7K_CH_COUNT]
///// @brief test module channel names
//char *s7k_ch_names[S7K_CH_COUNT]={
//    "trace.mbtrn",
//    "debug.mbtrn",
//    "warn.mbtrn",
//    "err.mbtrn",
//    "s7k.v1",
//    "s7k.v2"
//};
//
//static mmd_module_config_t mmd_config_default={MOD_S7K,"MOD_S7K",S7K_CH_COUNT,((MM_ERR|MM_WARN)),s7k_ch_names};

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
    /// @brief number of cycles
    int cycles;
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
    char help_message[] = "\n Stream raw reson bytes to console\n";
    char usage_message[] = "\n stream7k [options]\n"
    "\n Options:\n"
    "  --verbose=n : verbose output\n"
    "  --host      : reson host name or IP address\n"
    "  --cycles    : number of cycles (dfl 0 - until CTRL-C)\n"
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
                break;
            default:
                help=true;
                break;
        }
        if (version) {
            MFRAME_SHOW_VERSION(STREAM7K_NAME, STREAM7K_BUILD);
//            r7kr_show_app_version(STREAM7K_NAME, STREAM7K_BUILD);
            exit(0);
        }
        if (help) {
            MFRAME_SHOW_VERSION(STREAM7K_NAME, STREAM7K_BUILD);
//            r7kr_show_app_version(STREAM7K_NAME, STREAM7K_BUILD);
            s_show_help();
            exit(0);
        }
    }// while


	mconf_init(NULL,NULL);
    mmd_channel_set(MOD_S7K,MM_ERR);
    mmd_channel_set(MOD_R7K,MM_ERR);
    mmd_channel_set(MOD_R7KR,MM_ERR);
    mmd_channel_set(MOD_MSOCK,MM_ERR);

    switch (cfg->verbose) {
        case 0:
            mmd_channel_set(MOD_S7K,0);
            mmd_channel_set(MOD_R7K,0);
            mmd_channel_set(MOD_R7KR,0);
            mmd_channel_set(MOD_MSOCK,0);
            break;
        case 1:
            mmd_channel_en(MOD_S7K,S7K_V1);
            mmd_channel_en(MOD_S7K,MM_DEBUG);
            break;
        case 2:
            mmd_channel_en(MOD_S7K,S7K_V1);
            mmd_channel_en(MOD_S7K,S7K_V2);
            mmd_channel_en(MOD_S7K,MM_DEBUG);
            mmd_channel_en(MOD_MSOCK,MM_DEBUG);
            mmd_channel_en(MOD_R7K,MM_DEBUG);
            mmd_channel_en(MOD_R7KR,MM_DEBUG);
            break;
        default:
            break;
	}
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
            PMPRINT(MOD_S7K,S7K_V2,(stderr,"received sig[%d]\n",signum));
            g_stop_flag=true;
            break;
        default:
            PEPRINT((stderr,"not handled[%d]\n",signum));
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
        retval=0;
        
        uint32_t nsubs=11;
        uint32_t subs[]={1003, 1006, 1008, 1010, 1012, 1013, 1015,
            1016, 7000, 7004, 7027};
        msock_socket_t *s = NULL;
        int cycle_count=0;
        
        while (!g_stop_flag) {
            s = msock_socket_new(cfg->host, R7K_7KCENTER_PORT, ST_TCP);
            if (NULL != s) {
                PMPRINT(MOD_S7K,S7K_V1,(stderr,"connecting [%s]\n",cfg->host));
                if (msock_connect(s)==0) {
                    
                    if(r7k_subscribe(s, subs, nsubs)==0){
                        int test=msock_set_blocking(s,true);
                        PMPRINT(MOD_S7K,S7K_V1,(stderr,"set_blocking ret[%d]\n",test));
                        PMPRINT(MOD_S7K,S7K_V1,(stderr,"subscribing [%u]\n",nsubs));
                        PMPRINT(MOD_S7K,S7K_V1,(stderr,"streaming c[%d]\n",cfg->cycles));
                        r7k_stream_show(s,1024, 350, cfg->cycles,&g_stop_flag);
                        cycle_count++;
                    }else{
                        PMPRINT(MOD_S7K,S7K_V1,(stderr,"subscribe failed [%d/%s]\n",me_errno,strerror(me_errno)));
                    }
                }else{
                    PMPRINT(MOD_S7K,S7K_V1,(stderr,"connnect failed [%d/%s]\n",me_errno,strerror(me_errno)));
                }
            }else{
                PMPRINT(MOD_S7K,S7K_V1,(stderr,"msock_socket_new failed [%d/%s]\n",me_errno,strerror(me_errno)));
            }
            if (cfg->cycles>0 && (cycle_count>=cfg->cycles)) {
                g_stop_flag=true;
            }else{
                if (!g_stop_flag) {
                    PMPRINT(MOD_S7K,S7K_V1,(stderr,"retrying connection in 5 s\n"));
                    msock_socket_destroy(&s);
                    sleep(5);
                }
            }
        }
        if (g_stop_flag) {
            PMPRINT(MOD_S7K,S7K_V2,(stderr,"stop flag set\n"));
        }
    }// else invalid argument
    return retval;
}
// End function s_app_main

/// @fn int main(int argc, char ** argv)
/// @brief stream7k main entry point.
/// subscribe to reson 7k center data streams, and output
/// bytes as formatted ASCII hex to stderr.
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

    app_cfg_t cfg_s = {true,strdup(RESON_HOST_DFL),0};
    app_cfg_t *cfg = &cfg_s;
    
    // parse command line options
    parse_args(argc, argv, cfg);
    
    // run app
    retval=s_app_main(cfg);
    
    free(cfg->host);

    return retval;
}
// End function main

