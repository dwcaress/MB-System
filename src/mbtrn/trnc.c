///
/// @file trnc.c
/// @authors k. Headley
/// @date 01 jan 2018

/// TRN test client
/// subscribes to mbtrnpreprocess data

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
#include <inttypes.h>
#include "iowrap.h"
#include "r7kc.h"
#include "mdebug.h"
#include "mbtrn_types.h"

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
#define UDPS_BUF_LEN   2048

/// @def ID_APP
/// @brief debug module ID
#define ID_APP 1


/////////////////////////
// Declarations
/////////////////////////

/// @typedef struct app_cfg_s app_cfg_t
/// @brief application configuration parameter structure
typedef struct app_cfg_s{
    /// @var app_cfg_s::verbose
    /// @brief TBD
    int verbose;
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
    /// @var app_cfg_s::bsize
    /// @brief buffer size
    uint32_t bsize;
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
    char help_message[] = "\nTRN test client\n";
    char usage_message[] = "\ntrnc [options]\n"
    "--verbose=n    : verbose output, n>0\n"
    "--host=ip:n    : UDP server host\n"
    "--blocking=0|1 : blocking receive [0:1]\n"
    "--bsize=n      : buffer size\n"
//    "--port=n       : UDP server port\n"
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
        {"verbose", required_argument, NULL, 0},
        {"help", no_argument, NULL, 0},
        {"host", required_argument, NULL, 0},
//        {"port", required_argument, NULL, 0},
        {"blocking", required_argument, NULL, 0},
        {"cycles", required_argument, NULL, 0},
        {"bsize", required_argument, NULL, 0},
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
                
                /* help */
                else if (strcmp("help", options[option_index].name) == 0) {
                    help = true;
                }
                
                /* host */
                else if (strcmp("host", options[option_index].name) == 0) {

                    char *hsave=cfg->host;
                    char *ocopy=strdup(optarg);
                    cfg->host=strtok(ocopy,":");
                    if (cfg->host==NULL) {
                        cfg->host=UDPS_HOST_DFL;
                    }
                    char *ip = strtok(NULL,":");
                    if (ip!=NULL) {
                        sscanf(ip,"%d",&cfg->port);
                    }
                    // don't free ocopy here

                    if(hsave!=NULL){
                        free(hsave);
                    }
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
                /* buffer size */
                else if (strcmp("bsize", options[option_index].name) == 0) {
                    sscanf(optarg,"%u",&cfg->bsize);
                    cfg->bsize = (cfg->bsize>0 ? cfg->bsize : UDPS_BUF_LEN);

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
    
    switch (cfg->verbose) {
        case 0:
            mdb_set(ID_APP,MDL_INFO);
            break;
        case 1:
            mdb_set(ID_APP,MDL_DEBUG);
            break;
        default:
            mdb_set(ID_APP,MDL_ERROR);
            break;
    }
    
    
    MMDEBUG(ID_APP,"verbose [%s]\n",(cfg->verbose?"Y":"N"));
    MMDEBUG(ID_APP,"host    [%s]\n",cfg->host);
    MMDEBUG(ID_APP,"port    [%d]\n",cfg->port);
    MMDEBUG(ID_APP,"block   [%s]\n",(cfg->blocking==0?"N":"Y"));
    MMDEBUG(ID_APP,"cycles  [%d]\n",cfg->cycles);
    MMDEBUG(ID_APP,"bsize   [%d]\n",cfg->bsize);
}
// End function parse_args


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
    
    int test=0;
    iow_peer_t *peer=iow_peer_new();
    
    // set default app configuration
    app_cfg_t cfg = {0,strdup(UDPS_HOST_DFL),UDPS_PORT_DFL,1,-1,UDPS_BUF_LEN};
    
    // parse command line args (update config)
    parse_args(argc, argv, &cfg);
    // init receive buffer
    byte buf[cfg.bsize];
    memset(buf,0,cfg.bsize);
    
    // create socket
    iow_socket_t *s = iow_socket_new(cfg.host, cfg.port, ST_UDP);
    iow_set_blocking(s,(cfg.blocking==0?false:true));
    
    int rcycles=cfg.cycles;
    int scycles=cfg.cycles;
    int trn_tx_count=0;
    int trn_rx_count=0;
    int trn_tx_bytes=0;
    int trn_rx_bytes=0;
    bool quit=false;

    if (NULL != s) {
        MMDEBUG(ID_APP,"connect [%s:%d]\n",cfg.host,cfg.port);
        if ( (test=iow_connect(s))==0) {
            do{
                if( (test=iow_sendto(s,NULL,(byte *)"REQ",4))>0){
                    trn_tx_count++;
                    trn_tx_bytes+=test;
                    rcycles=5;
                    quit=false;
                    MMDEBUG(ID_APP,"sendto OK [%d]\n",test);
                    do{
                        memset(buf,0,cfg.bsize);
                        
                        MMDEBUG(ID_APP,"fd[%d] waiting for server (%s)...\n",s->fd,(cfg.blocking==0?"non-blocking":"blocking"));
                        test = iow_recvfrom(s, NULL, buf, cfg.bsize);
                        switch (test) {
                            case 0:
                                MMDEBUG(ID_APP,"iow_recvfrom returned 0; peer socket closed\n");
                                quit=true;
                                break;
                            case -1:
                                if (cfg.verbose>1) {
                                    MMDEBUG(ID_APP,"iow_recvfrom returned -1 [%d/%s]\n",errno,strerror(errno));
                                }
                                break;
                                
                            default:
                                trn_rx_count++;
                                trn_rx_bytes+=test;
                                MMDEBUG(ID_APP,"fd[%d] received %d/%u bytes\n",s->fd,test,cfg.bsize);
                                if (cfg.verbose) {
                                    r7k_hex_show(buf,test,16,true,5);
                                    if (test>4) {
                                        int i=0;
                                        unsigned int chksum=0;
                                        byte *bp=(byte *)buf;
                                        for (i=0; i<(test-4); i++) {
                                            chksum+=(unsigned int)(*bp++);
                                        }
                                        fprintf(stderr,"     checksum[%u/%#08X]\n",chksum,chksum);
                                        struct mbtrn_sounding *ms=(struct mbtrn_sounding  *)(buf+8);
                                        //MMDEBUG(ID_APP,"\nts[%lf] lat[%lf] lon[%lf]\nsd[%lf] hdg[%lf] nb[%d]\n",ms->ts,ms->lat,ms->lon,ms->depth,ms->hdg,ms->nbeams);
                                        MMDEBUG(ID_APP,"\nts[%f] lat[%f] lon[%f]\nsd[%f] hdg[%f] nb[%d]\n",
                                               ms->ts,
                                               ms->lat,
                                               ms->lon,
                                               ms->depth,
                                               ms->hdg,
                                               (uint32_t)ms->nbeams);
                                        uint32_t j=0;
                                        struct mbtrn_beam_data *bd=ms->beams;
                                        for (j=0; j<ms->nbeams; j++,bd++){
                                            MMDEBUG(ID_APP,"n[%03"PRId32"] rhox[% f] rhoy[% f] rhoz[% f]\n",
                                                   (uint32_t)bd->beam_num,
                                                   bd->rhox,
                                                   bd->rhoy,
                                                   bd->rhoz);
                                        }
                                    }
                                }
                                break;
                        }
                        MMINFO(ID_APP,"tx_count[%d] tx_bytes[%d]\n",trn_tx_count,trn_tx_bytes);
                        MMINFO(ID_APP,"rx_count[%d] rx_bytes[%d]\n",trn_rx_count,trn_rx_bytes);
                        if (--rcycles == 0) {
                            quit=true;
                        }
                        //sleep(1);
                    }while (!quit);

                }
                sleep(1);
            }while(--scycles != 0);
        }else{
            MERROR("connect failed [%d]\n",test);
        }
       MMDEBUG(ID_APP,"socket shutdown returned [%d]\n",shutdown(s->fd,SHUT_RDWR));
        free(cfg.host);
        iow_socket_destroy(&s);
       iow_peer_destroy(&peer);
    }
    return 0;
}
// End function main

