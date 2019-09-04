///
/// @file utility-template.c
/// @authors k. Headley
/// @date 11 aug 2017

/// General template for C utility
/// compile using:  gcc -DAPP_BUILD="`date -u`"  -DAPP_VERSION=1.0.1 utility-template.c  -o appname
/// @sa doxygen-examples.c for more examples of Doxygen markup


/////////////////////////
// Terms of use
/////////////////////////
/*
 Copyright Information
 
 application template - basic scaffolding for command line apps.
 
 Copyright 2002-2018 MBARI
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
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include <signal.h>

/////////////////////////
// Macros
/////////////////////////

// These macros should only be defined for 
// application main files rather than general C files
/*
/// @def PRODUCT
/// @brief header software product name
#define PRODUCT "app_name"

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

/// @def VERSION_HELPER
/// @brief version string helper.
#define VERSION_HELPER(s) #s
/// @def VERSION_STRING
/// @brief version string macro.
#define VERSION_STRING(s) VERSION_HELPER(s)

/// @def LIB_NAME
/// @brief library name string macro.
#define LIB_NAME "lib_name"

#ifndef LIB_VERSION
/// @def LIB_VERSION
/// @brief library application build version.
/// Sourced from CFLAGS in Makefile
/// w/ -DLIB_VERSION=<version>
#define LIB_VERSION 1.0.0
#endif

#ifndef LIB_BUILD
/// @def LIB_BUILD
/// @brief library build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DLIB_BUILD=`date`
#define LIB_BUILD "0000/00/00T00:00:00-0000"
#endif

/// @def LIB_VERSION_STR
/// @brief library version string macro.
#define LIB_VERSION_STR ""VERSION_STRING(LIB_VERSION)
/// @def LIB_BUILD_STR
/// @brief library version build date string macro.
#define LIB_BUILD_STR ""VERSION_STRING(LIB_BUILD)

/// @def LIB_NAME_STR
/// @brief library name string macro.
#define LIB_NAME_STR LIB_NAME
/// @def lib_name
/// @brief library name string stub
#define lib_name() (LIB_NAME_STR)
/// @def lib_version
/// @brief library version string stub
#define lib_version() (LIB_VERSION_STR)
/// @def lib_build
/// @brief library build string stub
#define lib_build()   (LIB_BUILD_STR)

/// @def APP_NAME
/// @brief application name string macro.
#define APP_NAME "app_name"

#ifndef APP_VERSION
/// @def APP_VERSION
/// @brief module build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DAPP_VERSION=`date -u`
#define APP_VERSION 1.0.0
#endif

#ifndef APP_BUILD
/// @def APP_BUILD
/// @brief module build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DAPP_BUILD=`date -u`
#define APP_BUILD "0000/00/00T00:00:00-0000"
#endif

/// @def APP_NAME_STR
/// @brief library name string macro.
#define APP_NAME_STR APP_NAME
/// @def APP_VERSION_STR
/// @brief library version string macro.
#define APP_VERSION_STR ""VERSION_STRING(APP_VERSION)
/// @def APP_BUILD_STR
/// @brief library version build date string macro.
#define APP_BUILD_STR ""VERSION_STRING(APP_BUILD)

/// @def app_name
/// @brief app name string stub
#define app_name() (APP_NAME_STR)
/// @def app_version
/// @brief app version string stub
#define app_version() (APP_VERSION_STR)
/// @def app_build
/// @brief app build string stub
#define app_build()   (APP_BUILD_STR)

/// @def APP_VERBOSE_DFL
/// @brief default debug level
#define APP_VERBOSE_DFL 0

/// @def WIN_DECLSPEC
/// @brief Windows calling convention
#define WIN_DECLSPEC

/// @def handle_error(msg)
/// @brief print error message and exit
/// @var[in] msg message
#define handle_error(msg) \
do { perror(msg); exit(EXIT_FAILURE); } while (0)

/// @def APP_VERBOSE_DFL
/// @brief default debug level
#define APP_VERBOSE_DFL 0

/// @def VERBOSE( ... )
/// @brief verbose output message
#define VERBOSE(n, ... ) if(n<=g_verbose)fprintf(stderr,__VA_ARGS__ )
/// @def DEBUG( ... )
/// @brief print debug message for specified channel
#define DEBUG(x, ... ) if(g_debug&(uint16_t)x)fprintf(stderr,__VA_ARGS__ )
/// @def TRACE( ... )
/// @brief print trace message
#define TRACE( ... ) if(g_trace)printf(__VA_ARGS__ )
/// @def BOOL2STR( b )
/// @brief boolean string value
#define BOOL2STR( b ) (b ? "Y" : "N")
/// @def BOOL2CHAR( b )
/// @brief boolean char value
#define BOOL2CHAR( b ) (b ? 'Y' : 'N')

/////////////////////////
// Declarations 
/////////////////////////
/// @typedef struct app_cfg_s app_cfg_t
/// @brief application configuration parameter structure
typedef struct app_cfg_s{
    /// @var app_cfg_s::verbose
    /// @brief TBD
    uint16_t verbose;
    /// @var app_cfg_s::file_count
    /// @brief number of input files
    int file_count;
    /// @var app_cfg_s::src_files
    /// @brief list of input files
    char **src_files;
}app_cfg_t;

static void s_show_help();
static void s_show_app_version();
static char *s_app_version_str(char *dest, size_t len);
static void s_show_lib_version();
static char *s_lib_version_str(char *dest, size_t len);
static void s_show_help();

/////////////////////////
// Imports
/////////////////////////

/////////////////////////
// Module Global Variables
/////////////////////////
/// @var bool g_interrupt
/// @brief user interrupt flag
static bool g_interrupt=false;
/// @var uint16_t g_debug
/// @brief interrupt channel enable bits (DEBUG)
static uint16_t g_debug=0x0000;
/// @var uint16_t g_verbose
/// @brief verbose output enable (VERBOSE)
static uint16_t g_verbose=0;
/// @var bool g_trace
/// @brief debug trace enable (TRACE)
static bool g_trace=false;


/////////////////////////
// Function Definitions
/////////////////////////

/// @fn void s_show_help()
/// @brief output user help message to stdout.
/// @return none
static void s_show_help()
{
    char help_message[] = "\n App template\n";
    char usage_message[] = "\n "APP_NAME" [options]\n"
    "  --verbose=n : verbose output, n>0\n"
    "  --debug=n   : debug output\n"
    "  --help      : output help message\n"
    "  --version   : output version info\n"
    "\n";
    printf("%s",help_message);
    printf("%s",usage_message);
}
// End function s_show_help

/// @fn void s_show_app_version()
//// @brief output application version string to stdout.
/// @return none
void s_show_app_version()
{
    printf("%s - ver [%s] build [%s]",app_name(), app_version(), app_build());
}
// End function s_show_app_version

/// @fn char *s_app_version_str(char *dest, size_t len)
//// @brief output application version string to buffer.
/// @return pointer to destination
static char *s_app_version_str(char *dest, size_t len)
{
    static char buf[64]={0};
    char *retval=NULL;
    if(NULL!=dest && len>0){
        snprintf(dest,len,"%s - ver [%s] build [%s]",app_name(), app_version(), app_build());
        retval=dest;
    }else{
        snprintf(buf,64,"%s - ver [%s] build [%s]",app_name(), app_version(), app_build());
        retval=buf;
    }
    return retval;
}
// End function s_app_version_str

/// @fn void s_show_lib_version()
/// @brief output library version string to stdout.
/// @return none
void s_show_lib_version()
{
    printf("%s - ver [%s] build [%s]",lib_name(), lib_version(), lib_build());
}
// End function s_show_app_version

/// @fn char *s_lib_version_str(char *dest, size_t len)
//// @brief output application version string to buffer.
/// @return pointer to destination
static char *s_lib_version_str(char *dest, size_t len)
{
    static char buf[64]={0};
    char *retval=NULL;
    if(NULL!=dest && len>0){
        snprintf(dest,len,"%s - ver [%s] build [%s]",lib_name(), lib_version(), lib_build());
        retval=dest;
    }else{
        snprintf(buf,64,"%s - ver [%s] build [%s]",lib_name(), lib_version(), lib_build());
        retval=buf;
    }
    return retval;
}
// End function s_lib_version_str

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
        {"debug", required_argument, NULL, 0},
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
                    if(sscanf(optarg,"%hu",&cfg->verbose)==1){
                        g_verbose=cfg->verbose;
                    }
                }
                // debug
                else if (strcmp("debug", options[option_index].name) == 0) {
                    if(sscanf(optarg,"0x%4hx",&g_debug)==1){
//                        fprintf(stderr,"%s %d\n",__FUNCTION__,__LINE__);
                    }else if(sscanf(optarg,"x%4hx",&g_debug)==1){
//                        fprintf(stderr,"%s %d\n",__FUNCTION__,__LINE__);
                    }else if(sscanf(optarg,"%hu",&g_debug)==1){
//                        fprintf(stderr,"%s %d\n",__FUNCTION__,__LINE__);
                    }else{
//                        fprintf(stderr,"%s %d\n",__FUNCTION__,__LINE__);
                        g_debug=0;
                    }
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
             printf("\n %s\n %s\n\n",s_app_version_str(NULL,0),s_lib_version_str(NULL,0));
            exit(0);
        }
        if (help) {
            printf("\n %s\n %s\n",s_app_version_str(NULL,0),s_lib_version_str(NULL,0));
            s_show_help();
            exit(0);
        }
    }// while
    
    VERBOSE(2,"optind[%d] argc[%d]\n",optind,argc);

    // process additional args, e.g. file list
//    int j=optind;
//    int i=0;
//    cfg->file_count=(argc-optind);
//    cfg->src_files = (char **)malloc(cfg->file_count*sizeof(char *));
//    while(j<argc){
//        cfg->src_files[i++]=strdup(argv[j++]);
//    }

    // configure debug output
    // if using mdebug.h
//    switch (cfg->verbose) {
//        case 0:
//            mdb_set(ID_APP,MDL_INFO);
//            break;
//        case 1:
//            mdb_set(ID_APP,MDL_DEBUG);
//            break;
//        case 2:
//            mdb_set(ID_APP,MDL_DEBUG);
//            mdb_set(ID_APP2,MDL_DEBUG);
//            break;
//        case 3:
//            mdb_set(ID_APP,MDL_DEBUG);
//            mdb_set(ID_APP2,MDL_DEBUG);
//            mdb_set(ID_APP3,MDL_DEBUG);
//            break;
//        default:
//            mdb_set(ID_APP,MDL_ERROR);
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
            DEBUG(1,"\nsig received[%d]\n",signum);
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
        retval=0;
        int count=0;
        while (!g_interrupt) {
            VERBOSE(1,"app main cycle [%d]\n",++count);
            sleep(2);
        }
    }
    return retval;
}
// End function s_app_main

/// @fn int main(int argc, char ** argv)
/// @brief Application main entry point.
/// Use --help command line option for use information.
/// @param[in] argc number of command line arguments
/// @param[in] argv array of command line arguments (strings)
/// @return 0 on success, -1 otherwise
int main(int argc, char **argv)
{
    int retval=0;
    
    // set default app configuration
    app_cfg_t cfg_inst = {
        APP_VERBOSE_DFL,
        0,
        NULL
    };
    app_cfg_t *cfg = &cfg_inst;
    
    // configure signal handling
    // for main thread
    struct sigaction saStruct;
    sigemptyset(&saStruct.sa_mask);
    saStruct.sa_flags = 0;
    saStruct.sa_handler = s_termination_handler;
    sigaction(SIGINT, &saStruct, NULL);
    
    // parse command line args (update config)
    parse_args(argc, argv, cfg);
    
    VERBOSE(1,"starting app - press CTRL-C to exit\n");
    // run the app
    retval = s_app_main(cfg);
    
    return retval;
}
// End function main

