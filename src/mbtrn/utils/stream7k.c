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
#include "mxdebug.h"
#include "mxd_app.h"

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
    char help_message[] = "\n Stream raw reson bytes to console\n";
    char usage_message[] = "\n stream7k [options]\n"
    "\n Options:\n"
    "  --verbose=n : verbose output\n"
    "  --host      : reson host name or IP address\n"
    "  --cycles    : number of cycles (dfl 0 - until CTRL-C)\n"
    "  --dev=s     : device [e.g. T50, 7125_400]\n"
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
        {"dev", required_argument, NULL, 0},
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

    mxd_setModule(MXDEBUG, 0, true, NULL);
    mxd_setModule(MXERROR, 5, false, NULL);
    mxd_setModule(STREAM7K, 0, false, "stream7k.error");
    mxd_setModule(STREAM7K_ERROR, 0, true, "stream7k.error");
    mxd_setModule(STREAM7K_DEBUG, 0, true, "stream7k.debug");
    mxd_setModule(MXMSOCK, 0, true, "msock");
    mxd_setModule(R7KC, 0, true, "r7kc");
    mxd_setModule(R7KC_DEBUG, 0, true, "r7kc.debug");
    mxd_setModule(R7KC_ERROR, 0, true, "r7kc.error");
    mxd_setModule(R7KR, 0, true, "r7kr");
    mxd_setModule(R7KR_ERROR, 0, true, "r7kr.error");
    mxd_setModule(R7KR_DEBUG, 0, true, "r7kr.debug");

    switch (cfg->verbose) {
        case 0:
            break;
        case 1:
            mxd_setModule(STREAM7K, 1, false, "stream7k.error");
            break;
        case 2:
            mxd_setModule(MXDEBUG, 5, false, NULL);
            mxd_setModule(STREAM7K, 5, false, "stream7k.error");
            break;
        case 3:
        case 4:
        case 5:
            mxd_setModule(MXDEBUG, 5, false, NULL);
            mxd_setModule(STREAM7K_ERROR, 5, false, "stream7k.error");
            mxd_setModule(STREAM7K_DEBUG, 5, false, "stream7k.debug");
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
            MX_LPRINT(STREAM7K, 2, "received sig[%d]\n", signum);
            g_stop_flag=true;
            break;
        default:
           MX_ERROR("not handled[%d]\n", signum);
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
                MX_LPRINT(STREAM7K, 1, "connecting host[%s] dev[%d]\n", cfg->host, cfg->dev);
                if (msock_connect(s)==0) {
                    s->status=SS_CONNECTED;

                    if(mxd_testModule(R7KR_DEBUG, 1)){
                        fprintf(stderr, "requesting 7k device config data");
                        r7k_req_config(s);
                    }

                    if(r7k_subscribe(s, cfg->dev, subs, nsubs)==0){
                        int test=msock_set_blocking(s,true);
                        MX_LPRINT(STREAM7K, 1, "set_blocking ret[%d]\n", test);
                        MX_LPRINT(STREAM7K, 1, "subscribing [%u]\n", nsubs);
                        MX_LPRINT(STREAM7K, 1, "streaming c[%d]\n", cfg->cycles);
                        r7k_stream_show(s,1024, 350, cfg->cycles, &g_stop_flag);
                        cycle_count++;
                    }else{
                        MX_LPRINT(STREAM7K, 1, "subscribe failed [%d/%s]\n", me_errno, strerror(me_errno));
                    }
                }else{
                    MX_LPRINT(STREAM7K, 1, "connect failed [%d/%s]\n", me_errno, strerror(me_errno));
                }
            }else{
                MX_LPRINT(STREAM7K, 1, "msock_socket_new failed [%d/%s]\n", me_errno, strerror(me_errno));
            }
            if (cfg->cycles>0 && (cycle_count>=cfg->cycles)) {
                g_stop_flag=true;
            }else{
                if (!g_stop_flag) {
                    MX_LMSG(STREAM7K, 1, "retrying connection in 5 s\n");
                    msock_socket_destroy(&s);
                    sleep(5);
                }
            }
        }
        if (g_stop_flag) {
            MX_LMSG(STREAM7K, 2, "stop flag set\n");
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

    app_cfg_t cfg_s = {true,strdup(RESON_HOST_DFL),0,R7KC_DEV_7125_400KHZ};
    app_cfg_t *cfg = &cfg_s;
    
    // parse command line options
    parse_args(argc, argv, cfg);
    
    // run app
    retval=s_app_main(cfg);
    
    free(cfg->host);

    return retval;
}
// End function main

