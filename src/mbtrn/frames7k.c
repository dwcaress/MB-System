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

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <getopt.h>
#include <stdarg.h>
#include <signal.h>
#include "r7kc.h"
#include "iowrap.h"
#include "mdebug.h"
#include "mbtrn.h"
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

/// @def RESON_HOST_DFL
/// @brief default reson hostname
#define RESON_HOST_DFL "134.89.13.49"

#define FRAMES7K_NAME "frames7k"
#ifndef FRAMES7K_BUILD
/// @def FRAMES7K_BUILD
/// @brief module build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DMBTRN_BUILD=`date`
#define FRAMES7K_BUILD ""VERSION_STRING(MBTRN_BUILD)
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
}app_cfg_t;

static void s_show_help();

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
        {NULL, 0, NULL, 0}};

    /* process argument list */
    while ((c = getopt_long(argc, argv, "", options, &option_index)) != -1){
        switch (c) {
                /* long options all return c=0 */
            case 0:
                /* verbose */
                if (strcmp("verbose", options[option_index].name) == 0) {
                    sscanf(optarg,"%d",&cfg->verbose);
                }
                // version
                if (strcmp("version", options[option_index].name) == 0) {
                    version=true;
                }
                
                /* help */
                else if (strcmp("help", options[option_index].name) == 0) {
                    help = true;
                }
                
                /* host */
                else if (strcmp("host", options[option_index].name) == 0) {
                    cfg->host=strdup(optarg);
                }
                /* cycles */
                else if (strcmp("cycles", options[option_index].name) == 0) {
                    sscanf(optarg,"%d",&cfg->cycles);
                }
                /* size */
                else if (strcmp("size", options[option_index].name) == 0) {
                    sscanf(optarg,"%u",&cfg->size);
                }
                break;
            default:
                help=true;
                break;
        }
        if (version) {
            mbtrn_show_app_version(FRAMES7K_NAME,FRAMES7K_BUILD);
            exit(0);
       }
        if (help) {
            mbtrn_show_app_version(FRAMES7K_NAME,FRAMES7K_BUILD);
            s_show_help();
            exit(0);
        }
    }// while
}
// End function parse_args

/// @fn void sig_debug (int signum)
/// @brief debugging signal handler.
/// @param[in] signum signal number
/// @return none
void sig_debug (int signum)
{
    fprintf(stderr,"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    fprintf(stderr,"!!!!!!!!!!!!!!!!!!  SIGNAL CAUGHT[%d]\n",signum);
    fprintf(stderr,"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
}
// End function sig_debug


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
    
    //    1) SIGHUP	 2) SIGINT	 3) SIGQUIT	 4) SIGILL	 5) SIGTRAP
    //    6) SIGABRT	 7) SIGBUS	 8) SIGFPE	 9) SIGKILL	10) SIGUSR1
    //    11) SIGSEGV	12) SIGUSR2	13) SIGPIPE	14) SIGALRM	15) SIGTERM
    //    16) SIGSTKFLT	17) SIGCHLD	18) SIGCONT	19) SIGSTOP	20) SIGTSTP
    //    21) SIGTTIN	22) SIGTTOU	23) SIGURG	24) SIGXCPU	25) SIGXFSZ
    //    26) SIGVTALRM	27) SIGPROF	28) SIGWINCH	29) SIGIO	30) SIGPWR
    //    31) SIGSYS	34) SIGRTMIN	35) SIGRTMIN+1	36) SIGRTMIN+2	37) SIGRTMIN+3
    //    38) SIGRTMIN+4	39) SIGRTMIN+5	40) SIGRTMIN+6	41) SIGRTMIN+7	42) SIGRTMIN+8
    //    43) SIGRTMIN+9	44) SIGRTMIN+10	45) SIGRTMIN+11	46) SIGRTMIN+12	47) SIGRTMIN+13
    //    48) SIGRTMIN+14	49) SIGRTMIN+15	50) SIGRTMAX-14	51) SIGRTMAX-13	52) SIGRTMAX-12
    //    53) SIGRTMAX-11	54) SIGRTMAX-10	55) SIGRTMAX-9	56) SIGRTMAX-8	57) SIGRTMAX-7
    //    58) SIGRTMAX-6	59) SIGRTMAX-5	60) SIGRTMAX-4	61) SIGRTMAX-3	62) SIGRTMAX-2
    //    63) SIGRTMAX-1	64) SIGRTMAX
    
    //    struct sigaction saDebug;
    //    sigemptyset(&saDebug.sa_mask);
    //    saDebug.sa_flags = 0;
    //    saDebug.sa_handler = sig_debug;
    //    sigaction(SIGHUP, &saDebug, NULL);
    //    //    sigaction(SIGINT, &saDebug, NULL);
    //    sigaction(SIGQUIT, &saDebug, NULL);
    //    sigaction(SIGILL, &saDebug, NULL);
    //    sigaction(SIGTRAP, &saDebug, NULL);
    //    sigaction(SIGABRT, &saDebug, NULL);
    //    sigaction(SIGBUS, &saDebug, NULL);
    //    sigaction(SIGFPE, &saDebug, NULL);
    //    sigaction(SIGKILL, &saDebug, NULL);
    //    sigaction(SIGUSR1, &saDebug, NULL);
    //    sigaction(SIGSEGV, &saDebug, NULL);
    //    sigaction(SIGUSR2, &saDebug, NULL);
    //    sigaction(SIGPIPE, &saDebug, NULL);
    //    sigaction(SIGALRM, &saDebug, NULL);
    //    sigaction(SIGTERM, &saDebug, NULL);
    //    sigaction(SIGSTKFLT, &saDebug, NULL);
    //    sigaction(SIGCHLD, &saDebug, NULL);
    //    sigaction(SIGCONT, &saDebug, NULL);
    //    sigaction(SIGSTOP, &saDebug, NULL);
    //    sigaction(SIGTSTP, &saDebug, NULL);
    //    sigaction(SIGTTIN, &saDebug, NULL);
    //    sigaction(SIGTTOU, &saDebug, NULL);
    //    sigaction(SIGURG, &saDebug, NULL);
    //    sigaction(SIGXCPU, &saDebug, NULL);
    //    sigaction(SIGXFSZ, &saDebug, NULL);
    //    sigaction(SIGVTALRM, &saDebug, NULL);
    //    sigaction(SIGPROF, &saDebug, NULL);
    //    sigaction(SIGWINCH, &saDebug, NULL);
    //    sigaction(SIGIO, &saDebug, NULL);
    //    sigaction(SIGPWR, &saDebug, NULL);
    //    sigaction(SIGSYS, &saDebug, NULL);
    //    sigaction(SIGRTMIN, &saDebug, NULL);
    
    uint32_t nsubs=11;
    uint32_t subs[]={1003, 1006, 1008, 1010, 1012, 1013, 1015,
        1016, 7000, 7004, 7027};
    
    app_cfg_t cfg = {1,strdup(RESON_HOST_DFL),0,102400};
    
    parse_args(argc, argv, &cfg);
    
    int count = 0;
    bool forever = false;
    if(cfg.cycles<=0){
        forever=true;
    }
    
    
    mcfg_configure(NULL,0);
    if (cfg.verbose>0) {
        mdb_set(APP1,MDL_DEBUG);
        mdb_set(MBTRN,MDL_DEBUG);
        mdb_set(MBTRNV,MDL_INFO);
    }else if (cfg.verbose>=2) {
        mdb_set(APP1,MDL_DEBUG);
        mdb_set(MBTRNV,MDL_DEBUG);
    }else{
        mdb_set(APP1,MDL_ERROR);
        mdb_set(MBTRN,MDL_ERROR);
        mdb_set(MBTRNV,MDL_ERROR);
    }
    
    
    
    // initialize reader
    // create and open socket connection
    
    mbtrn_reader_t *reader = mbtrn_reader_new(cfg.host,R7K_7KCENTER_PORT,cfg.size, subs, nsubs);
    
    // show reader config
    if (cfg.verbose>1) {
        mbtrn_reader_show(reader,true, 5);
    }
    
    uint32_t lost_bytes=0;
    int istat=0;

    MMDEBUG(APP2,"connecting reader [%s/%d]\n",cfg.host,R7K_7KCENTER_PORT);
    
    
    while (forever || (count<cfg.cycles) ) {
        count++;
        // test mbtrn_read_frame
        byte fbuf[MAX_FRAME_BYTES_7K]={0};
        //            if( (istat = mbtrn_read_frame(reader, fbuf,MAX_FRAME_BYTES_7K, &finfo, MBR_NET_STREAM,0.0, MBTRN_READ_TMOUT_MSEC )) > 0){
        if( (istat = mbtrn_read_frame(reader, fbuf, MAX_FRAME_BYTES_7K, MBR_NET_STREAM, 0.0, MBTRN_READ_TMOUT_MSEC,&lost_bytes )) > 0){
            
            
            MMDEBUG(APP1,"mbtrn_read_frame cycle[%d/%d] ret[%d] lost[%"PRIu32"]\n",count,cfg.cycles,istat,lost_bytes);
            
            if (cfg.verbose>=1) {
                MMDEBUG(APP1,"DRF:\n");
                r7k_nf_t *nf = (r7k_nf_t *)(fbuf);
                r7k_drf_t *drf = (r7k_drf_t *)(fbuf+R7K_NF_BYTES);
                r7k_nf_show(nf,false,5);
                r7k_drf_show(drf,false,5);
            }
            memset(fbuf,0,MAX_FRAME_BYTES_7K);
        }else{
            MERROR("ERR - mbtrn_read_frame - cycle[%d/%d] ret[%d] lost[%d]\n",count+1,cfg.cycles,istat,lost_bytes);
            if (me_errno==ME_ESOCK || me_errno==ME_EOF || me_errno==ME_ERCV) {
                MERROR("socket closed - reconnecting in 5 sec\n");
                sleep(5);
                mbtrn_reader_connect(reader,true);
            }
        }
        
        // original - read multiple frames
        // get a set of data record frames
        // here, separate poll, parse, enumeration and raw buffer reads
        //            MDEBUG("polling cycle[%d/%d]\n",count,cfg.cycles);
        //            MDEBUG("calling xread\n");
        //            fprintf(stderr,"WARN - using alt xread\n");
        
        //            if( (istat = mbtrn_reader_xread(reader,buf,cfg.size,MBTRN_READ_TMOUT_MSEC,MBR_BLOCK,0)) > 0){
        //                MDEBUG("xread %d/%d OK  [%d] - returned [%d/%d]\n",count,cfg.cycles,rstat,istat,cfg.size);
        //                MDEBUG("enumerating frames\n");
        //                // enumerate over the frames, show them
        //                r7k_drf_t *drfe = mbtrn_reader_enumerate(reader);
        //                int i=0;
        //                while (drfe!=NULL) {
        //                    MDEBUG("\nframe [%d/%d]\n",++i,mbtrn_reader_frames(reader));
        //                    r7k_drf_show(drfe,cfg.verbose,5);
        //                    drfe = mbtrn_reader_next(reader);
        //                }
        //            }else{
        //                MERROR("ERR - xread - cycle[%d/%d] rstat[%d] - returned istat/sz[%d/%d]\n",count+1,cfg.cycles,rstat,istat,cfg.size);
        //                if (me_errno==ME_ESOCK || me_errno==ME_ERCV) {
        //                    MERROR("socket closed - reconnecting in 5 sec\n");
        //                    sleep(5);
        //                    mbtrn_reader_connect(reader);
        //                }
        //            }
    }
    
    
    return 0;
}
// End function main

