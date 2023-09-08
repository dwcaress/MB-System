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


#include <getopt.h>
#include "msocket.h"

/////////////////////////
// Macros
/////////////////////////
#define UDPC_NAME "udpc"
#ifndef UDPC_VER
/// @def UDPC_VER
/// @brief module build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DUDPC_VER=<version>
#define UDPC_VER (dev)
#endif
#ifndef UDPC_BUILD
/// @def UDPC_BUILD
/// @brief module build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DMFRAME_BUILD=`date`
#define UDPC_BUILD VERSION_STRING(UDPC_VER)" "LIBMFRAME_BUILD
#endif

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

/// @def UDPC_HOST_DFL
/// @brief default server host
#define UDPC_HOST_DFL "localhost"
/// @def UDPC_PORT_DFL
/// @brief default UDP socket port
#define UDPC_PORT_DFL 27000
/// @def UDPC_BUF_LEN
/// @brief default buffer length
#define UDPC_BUF_LEN   128
/// @def UDPC_LOOP_DELAY_SEC
/// @brief loop delay
#define UDPC_LOOP_DELAY_SEC 1

/// @def UDPC_BLOCK_DFL
/// @brief socket blocking default
#define UDPC_BLOCK_DFL   0

/// @def UDPC_CYCLES_DFL
/// @brief cycles default
#define UDPC_CYCLES_DFL   -1

/// @def UDPC_DELAY_MSEC_DFL
/// @brief delay default
#define UDPC_DELAY_MSEC_DFL 0

/// @def UDPC_VERBOSE_DFL
/// @brief verbose output default
#define UDPC_VERBOSE_DFL false

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
static bool g_interrupt=false;

/////////////////////////
// Function Definitions
/////////////////////////

/// @fn void s_show_help()
/// @brief output user help message to stdout.
/// @return none
static void s_show_help()
{
    char help_message[] = "\nUDP client\n";
    char usage_message[] = "\nudpc [options]\n"
    "--verbose  : verbose output\n"
    "--help     : output help message\n"
    "--version  : output version info\n"
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
    bool version=false;
    
    static struct option options[] = {
        {"verbose", no_argument, NULL, 0},
        {"help", no_argument, NULL, 0},
        {"version", no_argument, NULL, 0},
        {"host", required_argument, NULL, 0},
        {"port", required_argument, NULL, 0},
        {"blocking", required_argument, NULL, 0},
        {"cycles", required_argument, NULL, 0},
        {NULL, 0, NULL, 0}};
    
    // process argument list
    while ((c = getopt_long(argc, argv, "", options, &option_index)) != -1){
        switch (c) {
                // long options all return c=0
            case 0:
                // verbose
                if (strcmp("verbose", options[option_index].name) == 0) {
                    cfg->verbose=true;
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
                    if(cfg->host!=NULL){
                        free(cfg->host);
                    }
                    cfg->host=strdup(optarg);
                }
                // blocking
                else if (strcmp("blocking", options[option_index].name) == 0) {
                    sscanf(optarg,"%d",&cfg->blocking);
                }
                // port
                else if (strcmp("port", options[option_index].name) == 0) {
                    sscanf(optarg,"%d",&cfg->port);
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
            MFRAME_SHOW_VERSION(UDPC_NAME, UDPC_BUILD);
            exit(0);
        }
        if (help) {
            MFRAME_SHOW_VERSION(UDPC_NAME, UDPC_BUILD);
            s_show_help();
            exit(0);
        }
    }// while
    fprintf(stderr,"verbose [%s]\n",(cfg->verbose?"Y":"N"));
    fprintf(stderr,"host    [%s]\n",cfg->host);
    fprintf(stderr,"port    [%d]\n",cfg->port);
    fprintf(stderr,"block   [%s]\n",(cfg->blocking==0?"N":"Y"));
    fprintf(stderr,"cycles  [%d]\n",cfg->cycles);
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
            fprintf(stderr,"sig received[%d]\n",signum);
            g_interrupt=true;
            break;
        default:
            fprintf(stderr,"s_termination_handler: sig not handled[%d]\n",signum);
            break;
    }
}
// End function termination_handler

/// @fn int s_app_main(msock_socket_t *s, app_cfg_t *cfg)
/// @brief upd client main entry point.
/// @param[in] s socket reference (ready to connect)
/// @param[in] cfg app config reference
/// @return 0 on success, -1 otherwise
static int s_app_main(msock_socket_t *s, app_cfg_t *cfg)
{
    int retval=-1;
    
    if (NULL!=cfg && NULL!=s) {
        int test=0;
        byte buf[UDPC_BUF_LEN]={0};
        int cycles=cfg->cycles;
        
        fprintf(stderr,"connect [%s:%d]\n",cfg->host,cfg->port);
        if ( (test=msock_connect(s))==0) {
            bool done=false;
            do{
                if( (test=msock_sendto(s,NULL,(byte *)"REQ",4,0))>0){
                    fprintf(stderr,"sendto OK [%d]\n",test);
                    retval=0;
                    memset(buf,0,128);
                    
                    fprintf(stderr,"fd[%d] waiting for server (%s)...\n",s->fd,(cfg->blocking==0?"non-blocking":"blocking"));
                    test = msock_recvfrom(s, NULL, buf, UDPC_BUF_LEN,0);
                    switch (test) {
                        case 0:
                            fprintf(stderr,"msock_recvfrom returned 0; peer socket closed\n");
                            retval=-1;
                            break;
                        case -1:
                            fprintf(stderr,"msock_recvfrom returned -1 [%d/%s]\n",errno,strerror(errno));
                            
                            switch (errno) {
                                case ENOTCONN:
                                case ECONNREFUSED:
                                    // host not connected
                                    // wait and retry
                                    sleep(5);
                                    break;
                                    
                                default:
                                    fprintf(stderr,"msock_recvfrom error [%d/%s]\n",errno,strerror(errno));
                                    break;
                            }
                            break;
                            
                        default:
                            fprintf(stderr,"fd[%d] received %d bytes\n",s->fd,test);
                            break;
                    }
                    // check cycles or wait
                    if (--cycles==0) {
                        done=true;
                    }else{
                        sleep(UDPC_LOOP_DELAY_SEC);
                    }
                }
            }while( !done && !g_interrupt);
        }else{
            fprintf(stderr,"connect failed [%d]\n",test);
        }
        
    }else{
        fprintf(stderr,"ERR - invalid argument\n");
    }
    
    return retval;
}

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
    int retval=-1;
    
    // set default app configuration
    app_cfg_t cfg_s = {
        UDPC_VERBOSE_DFL,
        strdup(UDPC_HOST_DFL),
        UDPC_PORT_DFL,
        UDPC_BLOCK_DFL,
        UDPC_CYCLES_DFL
    };
    app_cfg_t *cfg=&cfg_s;
    
    // configure signal handling
    // for main thread
    struct sigaction saStruct;
    sigemptyset(&saStruct.sa_mask);
    saStruct.sa_flags = 0;
    saStruct.sa_handler = s_termination_handler;
    sigaction(SIGINT, &saStruct, NULL);
    
    // parse command line args (update config)
    parse_args(argc, argv, cfg);
    
    // create socket
    msock_socket_t *s = msock_socket_new(cfg->host, cfg->port, ST_UDP);
    msock_set_blocking(s,(cfg->blocking==0?false:true));
    //    struct linger so_linger={0};
    //    setsockopt(s->fd,SOL_SOCKET,SO_LINGER,&so_linger,sizeof(so_linger));
    
    // run the app
    retval=s_app_main(s, cfg);
    shutdown(s->fd,SHUT_RDWR);
    
    // release resources
    free(cfg->host);
    msock_socket_destroy(&s);
    fprintf(stderr,"done\n\n");
    return retval;
}
// End function main

