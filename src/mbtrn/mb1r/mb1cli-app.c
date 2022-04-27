///
/// @file mb1cli-app.c
/// @authors k. Headley
/// @date 26 mar 2022

/// MB1 TCP reader
/// contains mb1r_reader_t, a component that
/// reads MB1 records from a TCP socket (like mbtrnpp MB1 input, rather then it's UDP MB1 output)

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
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>
#include "mb1-reader.h"
#include "mb1_msg.h"
#include "mframe.h"
#include "merror.h"

/////////////////////////
// Macros
/////////////////////////
#define MB1CLI_NAME "mb1-cli"
#ifndef MB1CLI_BUILD
/// @def MB1CLI_BUILD
/// @brief module build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DMB1CLI_BUILD=`date`
#define MB1CLI_BUILD ""VERSION_STRING(APP_BUILD)
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
#define MB1CLI_HOST_DFL "localhost"
#define MB1CLI_CYCLES_DFL -1
#define MB1CLI_RETRIES_DFL -1
#define MB1CLI_VERBOSE_DFL 0
#define MB1CLI_OFORMAT_DFL MB1C_OF_HEADER
#define HOSTNAME_BUF_LEN 256
//#define TRACE() fprintf(stderr,"%s:%d"\n, __func__, __LINE__)

/////////////////////////
// Declarations
/////////////////////////
typedef enum{
    MB1C_OF_HEADER=0x1,
    MB1C_OF_BEAMS=0x2,
    MB1C_OF_HEX=0x4,
    MB1C_OF_ALL=0x7,
}mb1c_fmt_t;

typedef struct app_cfg_s{
    int verbose;
    char *host;
    mb1c_fmt_t oformat;
    int port;
    int cycles;
    int retries;
}app_cfg_t;


static void s_show_help();
static int s_app_cfg_show(app_cfg_t *self,bool verbose, int indent);

/////////////////////////
// Imports
/////////////////////////

/////////////////////////
// Module Global Variables
/////////////////////////
static int g_signal=-1;
static bool g_interrupt=false;

/////////////////////////
// Function Definitions
/////////////////////////

/// @fn void s_show_help()
/// @brief output user help message to stdout.
/// @return none
static void s_show_help()
{
    char help_message[] = "\nMB1 TCP client\n";
    char usage_message[] = "\nmb1-cli [options]\n"
    " --verbose        : verbose output\n"
    " --help           : output help message\n"
    " --version        : output version info\n"
    " --host=[ip:port] : TRNU server host:port\n"
    " --ofmt=[HBX]     : output format, one or more of:\n"
    "                     A|* : all\n"
    "                       H : header\n"
    "                       B : beams (implies H)\n"
    "                       X : hex\n"
    " --cycles         : cycles to process\n"
    " --retries        : retries\n"
    "\n"
    " Example:\n"
    " # client\n"
    " mb1-cli --host=<trnsvr IP>[:<port>]\n"
    "\n";
    printf("%s",help_message);
    printf("%s",usage_message);
    int wkey=10;
    int wval=10;
    printf(" Defaults:\n");
    printf("%*s  %*d\n",wkey,"verbose",wval,MB1CLI_VERBOSE_DFL);
    printf("%*s  %*s\n",wkey,"host",wval,MB1CLI_HOST_DFL);
    printf("%*s  %*d\n",wkey,"port",wval,MB1_IP_PORT_DFL);
    printf("%*s  %*s%03X\n",wkey,"ofmt",(wval-3)," ",MB1CLI_OFORMAT_DFL);
    printf("%*s  %*d\n",wkey,"cycles",wval,MB1CLI_CYCLES_DFL);
    printf("%*s  %*d\n",wkey,"port",wval,MB1CLI_RETRIES_DFL);
    printf("\n");
}
/// @fn void parse_args(int argc, char ** argv, app_cfg_t * cfg)
/// @brief parse command line args, set application configuration.
/// @param[in] argc number of arguments
/// @param[in] argv array of command line arguments (strings)
/// @param[in] cfg application config structure
/// @return none
static void parse_args(int argc, char **argv, app_cfg_t *cfg)
{
    extern char WIN_DECLSPEC *optarg;
    int option_index;
    int c;
    bool help=false;
    bool version=false;
    static struct option options[] = {
        {"help", no_argument, NULL, 0},
        {"verbose", required_argument, NULL, 0},
        {"version", no_argument, NULL, 0},
        {"host", required_argument, NULL, 0},
        {"ofmt", required_argument, NULL, 0},
        {"cycles", required_argument, NULL, 0},
        {"retries", required_argument, NULL, 0},
        {NULL, 0, NULL, 0}};

    // process argument list
    while ((c = getopt_long(argc, argv, "", options, &option_index)) != -1){
        char *scpy = NULL;
        char *shost = NULL;
        char *sport = NULL;
        bool quit = false;
        int ival=0;
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
                    if(NULL!=scpy){
                        free(cfg->host);
                        cfg->host = strdup(shost);
                    }
                    if(NULL != sport){
                        sscanf(sport,"%d",&cfg->port);
                    }
                    free(scpy);
                }
                // cycles
                else if (strcmp("cycles", options[option_index].name) == 0) {
                    sscanf(optarg,"%d",&cfg->cycles);
                }
                // retries
                else if (strcmp("retries", options[option_index].name) == 0) {
                    sscanf(optarg,"%d",&cfg->retries);
                }
                // ofmt
                else if (strcmp("ofmt", options[option_index].name) == 0) {
                    scpy = optarg;
                    quit = false;
                    ival=0;
                    while(*scpy != '\0' && !quit){
                        switch (*scpy) {
                            case '*':
                            case 'a':
                            case 'A':
                                ival = MB1C_OF_ALL;
                                quit=true;
                                break;
                            case 'b':
                            case 'B':
                                ival |= (MB1C_OF_BEAMS | MB1C_OF_HEADER);
                                break;
                            case 'h':
                            case 'H':
                                ival |= MB1C_OF_HEADER;
                                break;
                            case 'x':
                            case 'X':
                                ival |= MB1C_OF_HEX;
                                break;
                            default:
                                break;
                        }
                        scpy++;
                    }
                    if( (ival&MB1C_OF_ALL) !=0)
                        cfg->oformat=ival;
                }
                break;
            default:
                help=true;
                break;
        }
        if (version) {
            //            mbtrn_show_app_version(TCPC_NAME,TCPC_BUILD);
            fprintf(stderr,"%s build %s\n",MB1CLI_NAME,MB1CLI_BUILD);
            exit(0);
        }
        if (help) {
            //            mbtrn_show_app_version(TCPC_NAME,TCPC_BUILD);
            s_show_help();
            exit(0);
        }
    }// while

    // use this host if unset
    if(NULL==cfg->host){
        // if unset, use local IP
        char host[HOSTNAME_BUF_LEN]={0};
        if(gethostname(host, HOSTNAME_BUF_LEN)==0 && strlen(host)>0){
            struct hostent *host_entry;

            if( (host_entry = gethostbyname(host))!=NULL){
                //Convert into IP string
                char *s =inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));
                cfg->host = strdup(s);
            } //find host information
        }

        if(NULL==cfg->host){
            cfg->host=strdup("localhost");
        }
    }

    if(cfg->verbose){
        fprintf(stderr," Configuration:\n");
        s_app_cfg_show(cfg,true,5);
        fprintf(stderr,"\n");
    }

}

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
            fprintf(stderr,"INFO - sig received[%d]\n",signum);
            g_interrupt=true;
            g_signal=signum;
            break;
        default:
            fprintf(stderr,"ERR - s_termination_handler: sig not handled[%d]\n",signum);
            break;
    }
}

static app_cfg_t *app_cfg_new()
{
    app_cfg_t *instance = (app_cfg_t *)malloc(sizeof(app_cfg_t));

    if(NULL!=instance){
        memset(instance,0,sizeof(app_cfg_t));
        instance->verbose=MB1CLI_VERBOSE_DFL;
        instance->host=NULL;
        instance->port=MB1_IP_PORT_DFL;
        instance->oformat=MB1CLI_OFORMAT_DFL;
        instance->retries=MB1CLI_RETRIES_DFL;
        instance->cycles=MB1CLI_CYCLES_DFL;
    }
    return instance;
}

static void app_cfg_destroy(app_cfg_t **pself)
{
    if(NULL!=pself){
        app_cfg_t *self = (app_cfg_t *)(*pself);
        if(NULL!=self){
            free(self->host);
            free(self);
            *pself=NULL;
        }
    }
    return;
}

static int s_app_cfg_show(app_cfg_t *self,bool verbose, int indent)
{
    int retval=0;
    int wkey=15;
    int wval=14;
    fprintf(stderr,"%*s%*s  %*s\n",indent,(indent>0?" ":""),wkey,"verbose",wval,(self->verbose?"Y":"N"));
    fprintf(stderr,"%*s%*s  %*s\n",indent,(indent>0?" ":""),wkey,"host",wval,self->host);
    fprintf(stderr,"%*s%*s  %*d\n",indent,(indent>0?" ":""),wkey,"port",wval,self->port);
    fprintf(stderr,"%*s%*s  %*s%03X\n",indent,(indent>0?" ":""),wkey,"oformat",wval-3,"",self->oformat);
    fprintf(stderr,"%*s%*s  %*d\n",indent,(indent>0?" ":""),wkey,"retries",wval,self->retries);
    fprintf(stderr,"%*s%*s  %*d\n",indent,(indent>0?" ":""),wkey,"cycles",wval,self->cycles);

    return retval;
}

/// @fn int main(int argc, char ** argv)
/// @brief trn_server client test main entry point.
/// @param[in] argc number of command line arguments
/// @param[in] argv array of command line arguments (strings)
/// @return 0 on success, -1 otherwise
int main(int argc, char **argv)
{
    int retval=-1;

    struct sigaction saStruct;
    sigemptyset(&saStruct.sa_mask);
    saStruct.sa_flags = 0;
    saStruct.sa_handler = s_termination_handler;
    sigaction(SIGINT, &saStruct, NULL);

    app_cfg_t *cfg = app_cfg_new();
    parse_args(argc, argv, cfg);


    int errors=0;

    if(cfg->verbose>0){
        s_app_cfg_show(cfg, true, 5);
    }

    // initialize reader
    // create and open socket connection
    mb1r_reader_t *reader = mb1r_reader_new(cfg->host, cfg->port, MB1_MAX_SOUNDING_BYTES);

    // show reader config
    if(cfg->verbose>1)
    mb1r_reader_show(reader,true, 5);

    uint32_t lost_bytes=0;
    // test mb1r_read_frame
    byte frame_buf[MB1_MAX_SOUNDING_BYTES]={0};
    int frames_read=0;

    if(cfg->verbose>1)
        fprintf(stderr,"connecting reader [%s/%d]\n",cfg->host,cfg->port);

    int retries=cfg->retries>0 ? cfg->retries>0 : 1;
    int cycles=cfg->cycles>0 ? cfg->cycles : 1;
    while ( !g_interrupt ) {
         // clear frame buffer
        memset(frame_buf,0,MB1_MAX_SOUNDING_BYTES);
        int istat=0;

        if(cfg->verbose>1)
            fprintf(stderr,"reading MB1 frame ret[%d]\n",retries);

        // read frame
        if( (istat = mb1r_read_frame(reader, frame_buf, MB1_MAX_SOUNDING_BYTES, MB1R_NOFLAGS, 0.0, MB1R_READ_TMOUT_MSEC, &lost_bytes )) > 0){

            frames_read++;
            if(cfg->cycles>0)cycles--;

            if(cfg->verbose>0)
                fprintf(stderr,"mb1r_read_frame cycle[%d/%d] lost[%u] ret[%d]\n", frames_read,
                        cfg->cycles,lost_bytes,istat);
            // show contents
            mb1_t *psnd = (mb1_t *)(frame_buf);
            if( cfg->oformat == MB1C_OF_ALL){
                mb1_show(psnd,true,5);
                mb1_hex_show(frame_buf,istat,16,true,10);
                fprintf(stderr,"\n");
            }else{
                if((cfg->oformat & MB1C_OF_HEADER) != 0) {
                    mb1_show(psnd,((cfg->oformat & MB1C_OF_BEAMS)!=0),5);
                }
                if((cfg->oformat & MB1C_OF_HEX) != 0) {
                    mb1_hex_show(frame_buf,istat,16,true,10);
                }
                fprintf(stderr,"\n");
            }
        }else{
            if(cfg->retries>0)retries--;
            errors++;
            // read error
            fprintf(stderr,"ERR - mb1r_read_frame - cycle[%d/%d] ret[%d] lost[%u] err[%d/%s]\n",frames_read+1,cfg->cycles,istat,lost_bytes,errno,strerror(errno));
            if (errno==ECONNREFUSED || me_errno==ME_ESOCK || me_errno==ME_EOF || me_errno==ME_ERECV) {

                fprintf(stderr,"socket closed - reconnecting in 5 sec\n");
                sleep(5);
                mb1r_reader_connect(reader,true);
            }
        }
        if(cycles<1 || retries<1)
            break;

    }

    if(cfg->verbose>0)
        fprintf(stderr,"releasing reader\n");
    mb1r_reader_destroy(&reader);

    if(frames_read==cfg->cycles)
        retval=0;

    if(cfg->verbose>0)
        fprintf(stderr,"frames[%d/%d]  retries[%d] lost[%u] errors[%d]\n",
            frames_read, cfg->cycles, cfg->retries-retries, lost_bytes, errors);

    app_cfg_destroy(&cfg);

    return retval;
}
