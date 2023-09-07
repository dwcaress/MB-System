

/// @file mb1rs-app.c
/// @authors k. Headley
/// @date 01 jan 2018

/// MB1 record server application

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

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <inttypes.h>
#include <getopt.h>
#include <mthread.h>
#include "mb1rs.h"
#include "mb1_msg.h"
#include "msocket.h"
#include "mxdebug.h"
#include "merror.h"
#include "mtime.h"

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

/////////////////////////
// Declarations
/////////////////////////

static void s_show_help();
static void s_parse_args(int argc, char **argv, mb1rs_cfg_t *cfg);
static void s_termination_handler (int signum);

/////////////////////////
// Imports
/////////////////////////

/////////////////////////
// Module Global Variables
/////////////////////////
static bool g_interrupt=false;
static bool g_signal=0;

/////////////////////////
// Function Definitions
/////////////////////////

/// @fn void s_show_help()
/// @brief output user help message to stdout.
/// @return none
static void s_show_help()
{
    char help_message[] = "\nMB1 record test server\n";
    char usage_message[] = "\nmb1rs [options]\n"
    "--help      : output help message\n"
    "--version   : output version info\n"
    "--verbose=n : verbose output\n"
    "--host=s:n  : server host (addr[:port])\n"
    "--src=s     : input source (file:<path>, auto:<nbeams>)\n"
    "--rto-ms=n  : read timeout msec\n"
    "--del-ms=n  : loop delay msec\n"
    "--lim-cyc=n : quit after n cycles\n"
    "--lim-ret=n : quit after n retries\n"
    "--lim-sec=d : quit after d seconds\n"
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
static void s_parse_args(int argc, char **argv, mb1rs_cfg_t *cfg)
{
    extern char WIN_DECLSPEC *optarg;
    int option_index;
    int c;
    bool help=false;
    bool version=false;
    static struct option options[] = {
        {"help", no_argument, NULL, 0},
        {"version", no_argument, NULL, 0},
        {"verbose", required_argument, NULL, 0},
        {"host", required_argument, NULL, 0},
        {"src", required_argument, NULL, 0},
        {"rto-ms", required_argument, NULL, 0},
        {"del-ms", required_argument, NULL, 0},
        {"lim-cyc", required_argument, NULL, 0},
        {"lim-ret", required_argument, NULL, 0},
        {"lim-sec", required_argument, NULL, 0},
        {NULL, 0, NULL, 0}};

    // process argument list
    while ((c = getopt_long(argc, argv, "", options, &option_index)) != -1){
        char *scpy = NULL;
        char *shost = NULL;
        char *sport = NULL;
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
                // host
                else if (strcmp("host", options[option_index].name) == 0) {
                    scpy = strdup(optarg);
                    shost = strtok(scpy,":");
                    sport = strtok(NULL,":");
                    if(NULL!=shost){
                        if(NULL!=cfg->host){
                            free(cfg->host);
                        }
                        cfg->host=strdup(shost);
                    }
                    if(NULL!=sport)
                        cfg->port=atoi(sport);
                    free(scpy);
                }
                // src
                else if (strcmp("src", options[option_index].name) == 0) {
                    char *cp = NULL;
                    if( (cp=strstr(optarg,"auto:"))!=NULL){
                        free(cfg->ifile);
                        sscanf(cp+5,"%"PRIu32"",&cfg->auto_nbeams);
                        MB1RS_SET_MSK(&cfg->flags, MB1RS_MODE_AUTO);
                    }else if( (cp=strstr(optarg,"file:"))!=NULL){
                        free(cfg->ifile);
                        cfg->ifile = strdup(cp+5);
                        MB1RS_CLR_MSK(&cfg->flags, MB1RS_MODE_AUTO);
                    }
                }
                // rto-ms
                else if (strcmp("rto-ms", options[option_index].name) == 0) {
                    sscanf(optarg,"%"PRIu32"",&cfg->rto_ms);
                }
                // del-ms
                else if (strcmp("del-ms", options[option_index].name) == 0) {
                    sscanf(optarg,"%"PRIu32"",&cfg->del_ms);
                }
                // cycles
                else if (strcmp("lim-cyc", options[option_index].name) == 0) {
                    sscanf(optarg,"%"PRIu32"",&cfg->lim_cyc);
                }
                // retries
                else if (strcmp("lim-ret", options[option_index].name) == 0) {
                    sscanf(optarg,"%"PRIu32"",&cfg->lim_ret);
                }
                // seconds
                else if (strcmp("lim-sec", options[option_index].name) == 0) {
                    sscanf(optarg,"%lf",&cfg->lim_sec);
                }
                break;
            default:
                help=true;
                break;
        }
        if (version) {
            mb1rs_show_app_version(MB1RS_NAME,MB1RS_BUILD_STR);
            exit(0);
        }
        if (help) {
            mb1rs_show_app_version(MB1RS_NAME,MB1RS_BUILD_STR);
            s_show_help();
            exit(0);
        }
    }// while

    if(cfg->verbose > 0){
        mxd_setModule(MXDEBUG, 1, false, NULL);
    }
    MX_DEBUG("verbose   [%d]\n", cfg->verbose);
    MX_DEBUG("host      [%s:%d]\n", cfg->host,cfg->port);
    if(MB1RS_GET_MSK(&cfg->flags, MB1RS_MODE_AUTO)) {
        MX_DEBUG("src       [a:%"PRIu32"]\n", cfg->auto_nbeams);
    } else {
        MX_DEBUG("src       [f:%s]\n", cfg->ifile);
    }
    MX_DEBUG("rto_ms    [%"PRIu32"]\n", cfg->rto_ms);
    MX_DEBUG("del_ms    [%"PRIu32"]\n", cfg->del_ms);
    MX_DEBUG("lim_cyc   [%"PRIu32"]\n", cfg->lim_cyc);
    MX_DEBUG("lim_ret   [%"PRIu32"]\n", cfg->lim_ret);
    MX_DEBUG("lim_sec   [%lf]\n", cfg->lim_sec);
}
// End function parse_args

/// @fn void termination_handler (int signum)
/// @brief termination signal handler.
/// @param[in] signum signal number
/// @return none
static void s_termination_handler (int signum)
{
    MX_DEBUG("sig received[%d]\n", signum);
    switch (signum) {
        case SIGINT:
        case SIGHUP:
        case SIGTERM:
            g_interrupt=true;
            break;
        default:
            fprintf(stderr,"s_termination_handler: sig not handled[%d]\n",signum);
            break;
    }
    g_signal=signum;
}
// End function termination_handler

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

    mb1rs_cfg_t *cfg = mb1rs_cfg_new();

    s_parse_args(argc, argv, cfg);

    mb1rs_ctx_t *svr = mb1rs_new(cfg);

    mb1rs_start(svr);
    double run_start=mtime_dtime();
    while(!g_interrupt){
        sleep(5);
        if(cfg->lim_sec>0 && ((mtime_dtime()-run_start) > cfg->lim_sec)){
            fprintf(stderr,"run time limit exceeded");
            break;
        }
    }

    if(svr->err_count==0)
        retval=0;

    if(cfg->verbose>0)
        fprintf(stderr,"cyc[%"PRIu32"/%"PRIu32"]  ret[%"PRIu32"/%"PRIu32"] tx[%"PRIu32"] err[%d]\n",
                svr->cyc_count, svr->cfg->lim_cyc,
                svr->ret_count, svr->cfg->lim_ret,
                svr->tx_count, svr->err_count);

    mb1rs_stop(svr);
    mb1rs_destroy(&svr);
    return retval;
}
