///
/// @file updc.c
/// @authors k. Headley
/// @date 01 jan 2018

/// UDP test client

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
#include <string.h>
#include <errno.h>
#include "iowrap.h"
#include "mdebug.h"

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

/// @def UDPS_HOST_DFL
/// @brief default server host
#define UDPS_HOST_DFL "localhost"
/// @def UDPS_PORT_DFL
/// @brief default UDP socket port
#define UDPS_PORT_DFL 9999
/// @def UDPS_BUF_LEN
/// @brief default buffer length
#define UDPS_BUF_LEN   128

/////////////////////////
// Declarations
/////////////////////////

/// @typedef struct app_cfg_s app_cfg_t
/// @brief application configuration parameter structure
typedef struct app_cfg_s{
    /// @var app_cfg_s::verbose
    /// @brief TBD
    bool verbose;
    /// @var app_cfg_s::host
    /// @brief TBD
    char *host;
    /// @var app_cfg_s::port
    /// @brief TBD
    int port;
    /// @var app_cfg_s::blocking
    /// @brief use blocking IO
    int blocking;
    /// @var app_cfg_s::cycles
    /// @brief number of cycles (<=0 : unlimited)
    int cycles;
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
    char help_message[] = "\nUDP server\n";
    char usage_message[] = "\nudps [options]\n"
    "--verbose  : verbose output\n"
    "--port     : UDP server port\n"
    "--blocking : blocking receive [0:1]\n"
    "--host     : UDP server host\n"
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
        {"port", required_argument, NULL, 0},
        {"blocking", required_argument, NULL, 0},
        {"cycles", required_argument, NULL, 0},
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
                    if(cfg->host!=NULL){
                        free(cfg->host);
                    }
                    cfg->host=strdup(optarg);
                }
                /* host */
                else if (strcmp("blocking", options[option_index].name) == 0) {
                    sscanf(optarg,"%d",&cfg->blocking);
                }
                /* port */
                else if (strcmp("port", options[option_index].name) == 0) {
                    sscanf(optarg,"%d",&cfg->port);
                }
                /* cycles */
                else if (strcmp("cycles", options[option_index].name) == 0) {
                    sscanf(optarg,"%d",&cfg->cycles);
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
    MDEBUG("verbose [%s]\n",(cfg->verbose?"Y":"N"));
    MDEBUG("host    [%s]\n",cfg->host);
    MDEBUG("port    [%d]\n",cfg->port);
    MDEBUG("block   [%s]\n",(cfg->blocking==0?"N":"Y"));
    MDEBUG("cycles  [%d]\n",cfg->cycles);
}
// End function parse_args


/// @fn int main(int argc, char ** argv)
/// @brief upd client main entry point.
/// may specify arguments on command line:
/// host   UDP server host
/// port   UDP server port
/// block  use blocking IO
/// cycles number of cycles (<=0 to run indefinitely)
/// @param[in] argc number of command line arguments
/// @param[in] argv array of command line arguments (strings)
/// @return 0 on success, -1 otherwise
int main(int argc, char **argv)
{
    
    int test=0;
    iow_peer_t *peer=iow_peer_new();

    byte buf[UDPS_BUF_LEN]={0};
    
    // set default app configuration
    app_cfg_t cfg = {true,strdup(UDPS_HOST_DFL),UDPS_PORT_DFL,1,-1};
    
    // parse command line args (update config)
    parse_args(argc, argv, &cfg);
    
    // create socket
    iow_socket_t *s = iow_socket_new(cfg.host, cfg.port, ST_UDP);
    iow_set_blocking(s,(cfg.blocking==0?false:true));
    
    int cycles=cfg.cycles;
   if (NULL != s) {
        MDEBUG("connect [%s:%d]\n",cfg.host,cfg.port);
        if ( (test=iow_connect(s))==0) {
            do{
                if( (test=iow_sendto(s,NULL,"REQ",4))>0){
                    MDEBUG("sendto OK [%d]\n",test);
                    do{
                        memset(buf,0,128);
                        
                        MDEBUG("fd[%d] waiting for server (%s)...\n",s->fd,(cfg.blocking==0?"non-blocking":"blocking"));
                        test = iow_recvfrom(s, NULL, buf, UDPS_BUF_LEN);
                        switch (test) {
                            case 0:
                                MDEBUG("iow_recvfrom returned 0; peer socket closed\n");
                                break;
                            case -1:
                                MDEBUG("iow_recvfrom returned -1 [%d/%s]\n",errno,strerror(errno));
                                break;
                                
                            default:
                                MDEBUG("fd[%d] received %d bytes\n",s->fd,test);
                                break;
                        }
                        sleep(1);
                    }while (--cycles != 0);
                    // set this to exit
                    cycles=1;
                }
                sleep(1);
            }while(--cycles != 0);
        }else{
            MERROR("connect failed [%d]\n",test);
        }
        free(cfg.host);
        iow_socket_destroy(&s);
    }
    return 0;
}
// End function main

