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
#include <getopt.h>
#include <stdarg.h>
#include "r7kc.h"
#include "iowrap.h"
#include "mdebug.h"
#include "mbtrn.h"

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

/////////////////////////
// Declarations
/////////////////////////

/// @typedef struct app_cfg_s app_cfg_t
/// @brief application configuration parameter structure
typedef struct app_cfg_s{
    /// @var app_cfg_s::verbose
    /// @brief verbose output flag
    bool verbose;
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
    char help_message[] = "\nStream raw reson bytes to console\n";
    char usage_message[] = "\nstream7k [options]\n"
    "--verbose : verbose output\n"
    "--host    : reson host name or IP address\n"
    "--cycles  : number of cycles (dfl 0 - until CTRL-C)\n"
    "--size    : reader capacity (bytes)\n"
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
    
    static struct option options[] = {
        {"verbose", no_argument, NULL, 0},
        {"help", no_argument, NULL, 0},
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
                    cfg->verbose=true;
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
        if (help) {
            s_show_help();
            exit(0);
        }
    }// while
}
// End function parse_args


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
    const char *host=RESON_HOST_DFL;
    
    uint32_t nsubs=11;
    uint32_t subs[]={1003, 1006, 1008, 1010, 1012, 1013, 1015,
        1016, 7000, 7004, 7027};
    
    app_cfg_t cfg = {false,strdup(RESON_HOST_DFL),0,102400};
    
    parse_args(argc, argv, &cfg);
    
    iow_socket_t *s = iow_socket_new(cfg.host, R7K_7KCENTER_PORT, ST_TCP);
    int count = 0;
    bool forever = false;
    if(cfg.cycles<=0){
        forever=true;
    }
    if (NULL != s) {
        // initialize reader
        // create and open socket connection
        
        mbtrn_reader_t *reader = mbtrn_reader_create(cfg.host,R7K_7KCENTER_PORT,cfg.size, subs, nsubs);
        
        
        // show reader config
        mbtrn_reader_show(reader,true, 5);
        
        uint32_t pstat=0;
        uint32_t read_bytes=0;
        byte buf[MBTRN_TRN_PING_BYTES]={0};
        int istat=0;
        int rstat=0;
       uint32_t tmout = 350;

        // flush socket
        int32_t flush_retries=0;//MBTRN_FLUSH_RETRIES
        MDEBUG("flushing [optional retries[%d]]\n",flush_retries);
        mbtrn_reader_flush(reader,60000,flush_retries,500);
        usleep(MBTRN_PING_INTERVAL_USEC);
        
        while (forever || count++<cfg.cycles) {
            
            // get a set of data record frames
            // here, separate poll, parse, enumeration and raw buffer reads
            MDEBUG("polling cycle[%d/%d]\n",count,cfg.cycles);
            MDEBUG("calling xread\n");
            if( (istat = mbtrn_reader_xread(reader,buf,cfg.size,tmout,MBR_ALLOW_PARTIAL)) > 0){
                MDEBUG("xread %d/%d OK  [%d] - returned [%d/%d]\n",count,cfg.cycles,rstat,istat,cfg.size);
                MDEBUG("enumerating frames\n");
                // enumerate over the frames, show them
                r7k_drf_t *drfe = mbtrn_reader_enumerate(reader);
                int i=0;
                while (drfe!=NULL) {
                    MDEBUG("\nframe [%d/%d]\n",++i,mbtrn_reader_frames(reader));
                    r7k_drf_show(drfe,cfg.verbose,5);
                    drfe = mbtrn_reader_next(reader);
                }
            }else{
                MERROR("xread %d/%d ERR [%d] - returned [%d/%d]\n",count+1,cfg.cycles,rstat,istat,cfg.size);
            }
        }
    }
    return 0;
}
// End function main

