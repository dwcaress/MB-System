///
/// @file upds.c
/// @authors k. Headley
/// @date 01 jan 2018

/// UDP test server

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
#include "mtime.h"

/////////////////////////
// Macros
/////////////////////////
#define UDPS_NAME "udps"
#ifndef UDPS_VER
/// @def UDPS_VER
/// @brief module build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DUDPS_VER=<version>
#define UDPS_VER (dev)
#endif
#ifndef UDPS_BUILD
/// @def UDPS_BUILD
/// @brief module build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DMFRAME_BUILD=`date`
#define UDPS_BUILD VERSION_STRING(UDPS_VER)" "LIBMFRAME_BUILD
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

/// @def UDPS_HOST_DFL
/// @brief default server host
#define UDPS_HOST_DFL "localhost"
/// @def UDPS_PORT_DFL
/// @brief default UDP socket port
#define UDPS_PORT_DFL 27000
/// @def UDPS_BUF_LEN
/// @brief default buffer length
#define UDPS_BUF_LEN   128
/// @def UDPS_RCVERR_DELAY_SEC
/// @brief delay default when no data available
#define UDPS_RCVERR_DELAY_SEC  1
/// @def UDPS_MAX_CONN_LIM
/// @brief maximum number client connections
#define UDPS_MAX_CONN_LIM      128

/// @def UDPS_BLOCK_DFL
/// @brief socket blocking default
#define UDPS_BLOCK_DFL   0

/// @def UDPS_CYCLES_DFL
/// @brief cycles default
#define UDPS_CYCLES_DFL   -1

/// @def UDPS_DELAY_MSEC_DFL
/// @brief delay default
#define UDPS_DELAY_MSEC_DFL 0

/// @def UDPS_VERBOSE_DFL
/// @brief verbose output default
#define UDPS_VERBOSE_DFL false

/// @def UDPS_CONNECTIONS_DFL
/// @brief maximum number client connections
#define UDPS_CONNECTIONS_DFL      16

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
    /// @var app_cfg_s::port
    /// @brief IP port
    int port;
    /// @var app_cfg_s::blocking
    /// @brief use blocking IO
    int blocking;
    /// @var app_cfg_s::cycles
    /// @brief number of cycles (<=0 : unlimited)
    int cycles;
    /// @var app_cfg_s::delay_msec
    /// @brief delay between tx
    uint32_t delay_msec;
    /// @var app_cfg_s::connections
    /// @brief max connections
    uint32_t connections;
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
    char help_message[] = "\nUDP server\n";
    char usage_message[] = "\nudps [options]\n"
    "--verbose  : verbose output\n"
    "--help     : output help message\n"
    "--version  : output version info\n"
    "--host     : UDP server host\n"
    "--port     : UDP server port\n"
    "--blocking : blocking receive [0:1]\n"
    "--delay    : transmit delay (msec)\n"
    "--conn=n   : max connections\n"
    //    "--host    : number of cycles (dfl 0 - until CTRL-C)\n"
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
        {"connections", required_argument, NULL, 0},
        {"cycles", required_argument, NULL, 0},
        {"delay", required_argument, NULL, 0},
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
                    if (cfg->host) {
                        free(cfg->host);
                    }
                    cfg->host=strdup(optarg);
                }
                // host
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
                // delay
                else if (strcmp("delay", options[option_index].name) == 0) {
                    sscanf(optarg,"%"PRId32"",&cfg->delay_msec);
                }
                // connections
                else if (strcmp("connections", options[option_index].name) == 0) {
                    sscanf(optarg,"%"PRIu32"",&cfg->connections);
                    if (cfg->connections==0||cfg->connections>UDPS_MAX_CONN_LIM) {
                        cfg->connections=UDPS_CONNECTIONS_DFL;
                    }
                }
                break;
            default:
                help=true;
                break;
        }
        if (version) {
            MFRAME_SHOW_VERSION(UDPS_NAME, UDPS_BUILD);
            exit(0);
        }
        if (help) {
            MFRAME_SHOW_VERSION(UDPS_NAME, UDPS_BUILD);
            s_show_help();
            exit(0);
        }
    }// while
    fprintf(stderr,"verbose [%s]\n",(cfg->verbose?"Y":"N"));
    fprintf(stderr,"host    [%s]\n",cfg->host);
    fprintf(stderr,"port    [%d]\n",cfg->port);
    fprintf(stderr,"block   [%s]\n",(cfg->blocking==0?"N":"Y"));
    fprintf(stderr,"cycles  [%d]\n",cfg->cycles);
    fprintf(stderr,"conn    [%"PRIu32"]\n",cfg->connections);
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
/// @brief upd server main entry point.
/// @param[in] s socket reference (ready to bind)
/// @param[in] cfg app config reference
/// @return 0 on success, -1 otherwise
static int s_app_main(msock_socket_t *s, app_cfg_t *cfg)
{
    int retval=-1;
    
    if (NULL!=cfg && NULL!=s) {
        msock_connection_t **connections=(msock_connection_t **)malloc(cfg->connections*sizeof(msock_connection_t));
        byte buf[UDPS_BUF_LEN]={0};
        
        int i=0;
        static struct timespec delay={0};
        static struct timespec rem={0};
        // init connection address info
        for (i=0; i<(int)cfg->connections; i++) {
            connections[i]=msock_connection_new();
            fprintf(stderr,"connections[%p] peer[%02d] peer@[%p]  ainfo[%p] ai_addr[%p]\n",connections,i,connections[i],connections[i]->addr->ainfo,connections[i]->addr->ainfo->ai_addr);
        }
        
        // bind to port
        fprintf(stderr,"binding [%s] fd[%d]\n",cfg->host,s->fd);
        int test=0;
        const int optionval = 1;
#if !defined(__CYGWIN__)
        msock_set_opt(s, SO_REUSEPORT, &optionval, sizeof(optionval));
#endif
        msock_set_opt(s, SO_REUSEADDR, &optionval, sizeof(optionval));
        if ( (test=msock_bind(s))==0) {
            uint32_t con_idx=0;
            retval=0;
            do{
                // clear buffers
                fprintf(stderr,"waiting to receive (%s)...\n",(cfg->blocking==0?"non-blocking":"blocking"));
                
                // clear buffer
                memset(buf,0,UDPS_BUF_LEN);
                
                // read client socket
                int iobytes = msock_recvfrom(s, connections[con_idx]->addr, buf, UDPS_BUF_LEN,0);
                // record arrival time
                double tarrival = mtime_dtime();
                
                const char *ctest=NULL;
                struct sockaddr_in *psin = NULL;
                msock_connection_t *trn_peer=connections[con_idx];
                
                switch (iobytes) {
                    case 0:
                        fprintf(stderr,"msock_recvfrom peer[%d] returned 0; peer socket closed\n",con_idx);
                        retval=-1;
                        break;
                    case -1:
                        if (cfg->verbose) {
                            fprintf(stderr,"msock_recvfrom peer[%d] returned -1 [%d/%s]\n",con_idx,errno,strerror(errno));
                        }
                        sleep(UDPS_RCVERR_DELAY_SEC);
                        break;
                        
                    default:
                        // get host name info from connection
                        if (NULL != trn_peer->addr &&
                            NULL != trn_peer->addr->ainfo &&
                            NULL != trn_peer->addr->ainfo->ai_addr) {
                            
                            psin = (struct sockaddr_in *)trn_peer->addr->ainfo->ai_addr;
                            ctest = inet_ntop(AF_INET, &psin->sin_addr, trn_peer->chost, MSOCK_ADDR_LEN);
                            if (NULL!=ctest) {
                                
                                uint16_t port = ntohs(psin->sin_port);
                                
                                int svc = port;
                                snprintf(trn_peer->service,NI_MAXSERV,"%d",svc);
                                
                                fprintf(stderr,"%11.3f Received %d bytes from peer[%d] %s:%s\n",
                                       tarrival,iobytes, con_idx, trn_peer->chost, trn_peer->service);
                                
                                // send reply data to requesting peer
                                if ( (iobytes = msock_sendto(s, trn_peer->addr, buf, UDPS_BUF_LEN, 0 )) > 0) {
                                    
                                    fprintf(stderr,"%11.3f Sent %d bytes to peer[%d] %s:%s\n",
                                           mtime_dtime(),iobytes, con_idx, trn_peer->chost, trn_peer->service);
                                    
                                }else{
                                    fprintf(stderr,"send peer[%d] failed [%d]\n",i,iobytes);
                                }
                                
                            }else{
                                fprintf(stderr,"inet_ntop (recv) failed peer[%d] [%d %s]\n",con_idx,errno,strerror(errno));
                                fprintf(stderr,"peer[%d] received %d bytes\n",con_idx,iobytes);
                            }
                        }
                        
                        // increment peer index for next recv
                        // rollover at max peer
                        if (con_idx<(cfg->connections-1)) {
                            con_idx++;
                        }else{
                            con_idx=0;
                        }
                        break;
                }
                
                // option: send messages to all connections
                // instead of req/res
                //                for (i=0; i<=(con_idx-1); i++) {
                //                    if ( (iobytes = msock_sendto(s, connections[i]->addr, buf, UDPS_BUF_LEN, 0 )) > 0) {
                //                        fprintf(stderr,"%11.3f Sent %zd bytes to connection[%d] %s:%s\n",
                //                               mtime_dtime(),iobytes, i, connections[i]->chost, connections[i]->service));
                //                    }else{
                //                        fprintf(stderr,"send connection[%d] failed [%d]\n",i,iobytes));
                //                    }
                //                }
                
                // delay specified period
                if (cfg->delay_msec>0) {
                    delay.tv_sec = cfg->delay_msec/1000;
                    delay.tv_nsec = (cfg->delay_msec%1000)*1000000L;
                    while (nanosleep(&delay,&rem)<0) {
                        fprintf(stderr,"sleep interrupted\n");
                        delay.tv_sec=rem.tv_sec;
                        delay.tv_nsec=rem.tv_nsec;
                    }
                }
            }while ( (--cfg->cycles != 0) && !g_interrupt);
            
        }else{
            fprintf(stderr,"bind failed [%d]\n",test);
        }
        
        // free connections
        for (i=0; i<(int)cfg->connections; i++) {
            msock_connection_destroy(&connections[i]);
        }
        // free connections array
        free(connections);
        
    }else{
        fprintf(stderr,"ERR - invalid argument\n");
    }
    return retval;
}
// End function s_app_main


/// @fn int main(int argc, char ** argv)
/// @brief upd server main entry point.
/// may specify arguments on command line:
/// port   UDP socket port
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
        UDPS_VERBOSE_DFL,
        strdup(UDPS_HOST_DFL),
        UDPS_PORT_DFL,
        UDPS_BLOCK_DFL,
        UDPS_CYCLES_DFL,
        UDPS_DELAY_MSEC_DFL,
        UDPS_CONNECTIONS_DFL
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
    //    int so_reuseaddr=0;
    //    setsockopt(s->fd,SOL_SOCKET,SO_REUSEADDR,&so_reuseaddr,sizeof(int));
    //    int so_reuseport=0;
//#if !defined(__CYGWIN__)
    //    setsockopt(s->fd,SOL_SOCKET,SO_REUSEPORT,&so_reuseport,sizeof(int));
//#endif
    //    struct linger so_linger={0};
    //    setsockopt(s->fd,SOL_SOCKET,SO_LINGER,&so_linger,sizeof(so_linger));
    
    // run app
    retval=s_app_main(s, cfg);
    
    // release resources
    msock_socket_destroy(&s);
    free(cfg->host);
    
    fprintf(stderr,"done\n\n");
    return retval;
}
// End function main

