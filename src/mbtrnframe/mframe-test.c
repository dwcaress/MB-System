///
/// @file mframe-test.c
/// @authors k. Headley
/// @date 11 aug 2017

/// mframe unit test(s)

/// @sa doxygen-examples.c for more examples of Doxygen markup


/////////////////////////
// Terms of use
/////////////////////////
/*
 Copyright Information
 
 mframe - simple framework for building C libraries and utilities
 
 Copyright 2002-2017 MBARI
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
#if !defined(__QNX__)
#include <getopt.h>
#endif

#include "mframe.h"
#include "mmdebug.h"
#include "msocket.h"
#include "mserial.h"
#include "mbbuf.h"
#include "mlog.h"
#include "mswap.h"
#include "mutils.h"

/////////////////////////
// Macros
/////////////////////////

// These macros should only be defined for 
// application main files rather than general C files
/*
/// @def PRODUCT
/// @brief header software product name
#define PRODUCT "mframe-test"

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

#define APP_NAME "mframe-test"

#ifndef APP_VERSION
/// @def APP_VERSION
/// @brief application version.
#define APP_VERSION 1.0.0
#endif

#ifndef APP_BUILD
/// @def APP_BUILD
/// @brief library build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DAPP_BUILD=`date`
#define APP_BUILD "0000/00/00T00:00:00-0000"
#endif

/// @def APP_VERSION_STR
/// @brief library version string macro.
#define APP_VERSION_STR ""VERSION_STRING(APP_VERSION)
/// @def APP_BUILD_STR
/// @brief library version build date string macro.
#define APP_BUILD_STR ""VERSION_STRING(APP_BUILD)

/// @def app_name
/// @brief app name string stub
#define app_name() (APP_NAME)
/// @def app_version
/// @brief app version string stub
#define app_version() (APP_VERSION_STR)
/// @def app_build
/// @brief app build string stub
#define app_build()   (APP_BUILD_STR)

/// @def APP_VERBOSE_DFL
/// @brief default debug level
#define APP_VERBOSE_DFL 0


/////////////////////////
// Declarations 
/////////////////////////

/// @typedef struct app_cfg_s app_cfg_t
/// @brief application configuration parameter structure
typedef struct app_cfg_s{
    /// @var app_cfg_s::verbose
    /// @brief TBD
    int verbose;
}app_cfg_t;

/// @enum app_module_ids
/// @brief application module IDs
/// [note : starting above reserved mframe module IDs]
typedef enum{
    MOD_MFTEST=MM_MODULE_COUNT,
    APP_MODULE_COUNT
}app_module_ids;

/// @enum mftest_channel_id
/// @brief test module channel IDs
/// [note : starting above reserved mframe channel IDs]
typedef enum{
    ID_MFTEST_1=MM_CHANNEL_COUNT,
    APP_CH_COUNT
}mftest_channel_id;

/// @enum mftest_channel_mask
/// @brief test module channel masks
typedef enum{
    MFTEST_1= (1<<ID_MFTEST_1)
}mftest_channel_mask;

/// @var char *mmd_test_m1_ch_names[APP_CH_COUNT]
/// @brief test module channel names
char *app_ch_names[APP_CH_COUNT]={
    "trace.mftest",
    "debug.mftest",
    "warn.mftest",
    "err.mftest",
    "mftest.1",
};

mmd_module_config_t mmd_app_defaults[]={
    {MOD_MFTEST,"MOD_MFTEST",APP_CH_COUNT,((MM_ERR|MM_WARN)|MFTEST_1),app_ch_names},
};

static void s_show_help();
static void s_show_app_version();
static void s_show_help();

/////////////////////////
// Imports
/////////////////////////

/////////////////////////
// Module Global Variables
/////////////////////////
static bool g_interrupt=false;

/////////////////////////
// Function Definitions
/////////////////////////

/// @fn void s_show_help()
/// @brief output user help message to stdout.
/// @return none
static void s_show_help()
{
    char help_message[] = "\nApp template\n";
    char usage_message[] = "\n"APP_NAME" [options]\n"
    "--verbose=n    : verbose output, n>0\n"
    "--help         : output help message\n"
    "--version      : output version info\n"
    "\n";
    printf("%s",help_message);
    printf("%s",usage_message);
}
// End function s_show_help

/// @fn void s_show_app_version(const char *app_version)
/// @brief get version string.
/// @return version string
void s_show_app_version()
{
    printf("\n %s : ver[%s] build[%s] lib[ ver %s build %s ]\n\n",app_name(), app_version(), app_build(), mframe_version(), mframe_build());
}
// End function s_show_app_version

/// @fn void parse_args(int argc, char ** argv, app_cfg_t * cfg)
/// @brief parse command line args, set application configuration.
/// @param[in] argc number of arguments
/// @param[in] argv array of command line arguments (strings)
/// @param[in] cfg application config structure
/// @return none
#if !defined(__QNX__)

void parse_args(int argc, char **argv, app_cfg_t *cfg)
{
    extern char MF_EXPORT *optarg;
    int option_index=0;
    int c=0;
    bool help=false;
    bool version=false;

    static struct option options[] = {
        {"verbose", required_argument, NULL, 0},
        {"help", no_argument, NULL, 0},
        {"version", no_argument, NULL, 0},
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

                // help
                else if (strcmp("help", options[option_index].name) == 0) {
                    help = true;
                }

                // version
                else if (strcmp("version", options[option_index].name) == 0) {
                    version = true;
                }
                break;
            default:
                help=true;
                break;
        }
        if (version) {
            s_show_app_version();
            exit(0);
        }
        if (help) {
            s_show_app_version();
            s_show_help();
            exit(0);
        }
    }// while
    // TODO: configure debug output...

    PMPRINT(MOD_MFTEST,MFTEST_1,(stderr,"verbose [%s]\n",(cfg->verbose?"Y":"N")));
//    PMPRINT(MOD_MFTEST,MFTEST_1,(stderr,"c [%d]\n",c));
    PMPRINT(MOD_MFTEST,MFTEST_1,(stderr,"version [%d]\n",version));
//    PMPRINT(MOD_MFTEST,MFTEST_1,(stderr,"option_index [%d]\n",option_index));
    PMPRINT(MOD_MFTEST,MFTEST_1,(stderr,"help [%s]\n",(help?"Y":"N")));
}
// End function parse_args
#else
void parse_args(int argc, char **argv, app_cfg_t *cfg)
{
}

#endif

/// @fn void s_termination_handler (int signum)
/// @brief termination signal handler.
/// @param[in] signum signal number
/// @return none
static void s_termination_handler (int signum)
{
    switch (signum) {
        case SIGINT:
        case SIGHUP:
        case SIGTERM:
            PMPRINT(MOD_MFTEST,MFTEST_1,(stderr,"\nsig received[%d]\n",signum));
            g_interrupt=true;
            break;
        default:
            fprintf(stderr,"\ns_termination_handler: sig not handled[%d]\n",signum);
            break;
    }
}
// End function termination_handler

/// @fn int s_app_main ()
/// @brief application main function.
/// @param[in]
/// @return none
static int s_app_main(app_cfg_t *cfg)
{
    int retval=-1;
    if (NULL!=cfg) {
#if defined(WITH_MBBUF_TEST) || defined(WITH_MLOG_TEST)
        char *av[]={"false"};
#endif

#ifdef WITH_MSOCKET_TEST
        retval=msock_test();
        PMPRINT(MOD_MFTEST,MFTEST_1,(stderr,"msock_test [%d]\n",retval));
#endif
#ifdef WITH_MSERIAL_TEST
        retval=mser_test();
        PMPRINT(MOD_MFTEST,MFTEST_1,(stderr,"mser_test [%d]\n",retval));
#endif
#ifdef WITH_MBBUF_TEST
        av[0]="false";
        retval=mbbuf_test(1,&av[0]);
        PMPRINT(MOD_MFTEST,MFTEST_1,(stderr,"mbbuf_test [%d]\n",retval));
#endif
#ifdef WITH_MLOG_TEST
        av[0]="false";
        retval=mlog_test(1, &av[0]);
        PMPRINT(MOD_MFTEST,MFTEST_1,(stderr,"mlog_test [%d]\n",retval));
#endif
#ifdef WITH_MSWAP_TEST
        retval=mswap_test((cfg->verbose!=0));
        PMPRINT(MOD_MFTEST,MFTEST_1,(stderr,"mswap_test [%d]\n",retval));
#endif
#ifdef WITH_MUTILS_TEST
        retval=mfu_test(cfg->verbose);
        PMPRINT(MOD_MFTEST,MFTEST_1,(stderr,"mutils_test [%d]\n",retval));
#endif
//        int count=0;
//        while (!g_interrupt) {
//            MDEBUG("app main cycle [%d]\n",++count);
//            sleep(2);
//        }
    }
    return retval;
}
// End function s_app_main

/// @fn int main(int argc, char ** argv)
/// @brief TRN test client main entry point.
/// may specify arguments on command line:
/// host   UDP server host (MB System host)
/// port   UDP server port (MB System TRN output port)
/// block  use blocking IO
/// cycles number of cycles (<=0 to run indefinitely)
/// bsize  buffer size
/// @param[in] argc number of command line arguments
/// @param[in] argv array of command line arguments (strings)
/// @return 0 on success, -1 otherwise
int main(int argc, char **argv)
{
    int retval=0;
    // set default app configuration
    struct app_cfg_s cfg_s= {
        APP_VERBOSE_DFL
    };
	app_cfg_t *cfg=NULL;
    struct sigaction saStruct;

    // library init
    mmd_initialize();
    
    // configure module channel from static defaults
    // [data is copied, no change to original]
    mmd_module_configure(&mmd_app_defaults[0]);

    cfg=&cfg_s;
    
    // configure signal handling
    // for main thread
    sigemptyset(&saStruct.sa_mask);
    saStruct.sa_flags = 0;
    saStruct.sa_handler = s_termination_handler;
    sigaction(SIGINT, &saStruct, NULL);
    
    // parse command line args (update config)
    parse_args(argc, argv, cfg);
    
    PMPRINT(MOD_MFTEST,MFTEST_1,(stderr,"starting app - press CTRL-C to exit\n"));
    // run the app
    retval = s_app_main(cfg);
    
    return retval;
}
// End function main

