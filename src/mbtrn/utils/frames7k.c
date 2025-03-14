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

#include <getopt.h>
#include "r7kc.h"
#include "msocket.h"
#include "mfile.h"
#include "mlist.h"
#include "mxdebug.h"
#include "mxd_app.h"
#include "r7k-reader.h"
#include "merror.h"

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
#define RESON_HOST_DFL "localhost"

#define FRAMES7K_NAME "frames7k"
#ifndef FRAMES7K_VER
/// @def FRAMES7K_VER
/// @brief module build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DFRAMES7K_VER=<version>
#define FRAMES7K_VER (dev)
#endif
#ifndef FRAMES7K_BUILD
/// @def FRAMES7K_BUILD
/// @brief module build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DMFRAME_BUILD=`date`
#define FRAMES7K_BUILD VERSION_STRING(FRAMES7K_VER)" "LIBMFRAME_BUILD
#endif

/////////////////////////
// Declarations
/////////////////////////
typedef enum {
    IMODE_SOCKET = 0,
    IMODE_FILE
}InputMode;

/// @typedef struct app_cfg_s app_cfg_t
/// @brief application configuration parameter structure
typedef struct app_cfg_s{
    /// @var app_cfg_s::verbose
    /// @brief verbose output flag
    int verbose;
    /// @var app_cfg_s::host
    /// @brief hostname
    char *host;
    /// @var app_cfg_s::port
    /// @brief S7K IP port
    int port;
    /// @var app_cfg_s::cycles
    /// @brief number of cycles (<=0 : unlimited)
    int cycles;
    /// @var app_cfg_s::size
    /// @brief frame buffer size
    uint32_t size;
    /// @var app_cfg_s::id
    /// @brief reader id
    r7k_device_t dev;
    /// @var app_cfg_s::input_mode
    /// @brief input mode
    InputMode mode;
    /// @var app_cfg_s::net_frame
    /// @brief input net_frame
    bool net_frames;
    /// @var app_cfg_s::file_list
    /// @brief data source file list
    mlist_t *file_paths;
    /// @var app_cfg_s::nsubs
    /// @brief number of subscriptions
    uint32_t nsubs;
    /// @var app_cfg_s::subs
    /// @brief  subscription list
    uint32_t *subs;
    /// @var app_cfg_s::filter
    /// @brief filter using subcription list
    bool filter;
    /// @var app_cfg_s::show_data
    /// @brief show frame data
    bool show_data;

}app_cfg_t;

static void s_show_help();
static bool g_stop_flag=false;

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
    char usage_message[] = "\n frames7k [options] [file,...]\n"
    " Options :\n"
    "  --verbose=<n>       : verbose debug output\n"
    "  --host=<s>[:port]   : reson host name or IP address and port\n"
    "  --data              : show data payload (formatted hex)\n"
    "  --filter            : filter using subscription list for file input\n"
    "  --subs=[o,]<d,...>  : append/overwrite record type list\n"
    "                          For socket input, sets subscription (and output) list.\n"
    "                          For file input, sets output list when --filter specified.\n"
    "                          Use 'o' to overwrite default list (default: append).\n"
    "                          Default record types:\n"
    "                            1003 1006 1008 1010\n"
    "                            1012 1013 1015 1016\n"
    "                            7000 7004 7027\n"
    "  --file=<s>          : S7K file name (or list after options)\n"
    "  --net               : input includes net frames\n"
    "  --cycles=<n>        : number of cycles (default 0: until CTRL-C or end of input)\n"
    "  --dev=<s>           : device [e.g. T50, 7125_400]; options:\n"
    "                          7125_400 : Reson 7125 400 kHz (default)\n"
    "                          7125_200 : Reson 7125 200 kHz\n"
    "                               T50 : Reson T50\n"
    "  --size=<n>          : reader capacity (bytes)\n"
    "\n"
    " Examples\n"
    "\n"
    "   # Subscribe/read from socket, using default 7kCenter port, show message data\n"
    "    frames7k --host=192.168.1.101 --data\n"
    "\n"
    "   # Show headers of S7K data files\n"
    "    frames7k $(ls /path/to/data/*.s7k)\n"
    "\n"
    "   # Read from files, display headers from record types 7000,1008\n"
    "    frames7k --subs=o,7000,1008 --filter $(ls /path/to/data/*.s7k)\n"
    "\n"
    "   # Read from socket (non-standard port), display headers from record types 7000,1008\n"
    "    frames7k --subs=o,7000,1008 --filter --host:192.168.1.101:7001\n"
    "\n";
    printf("%s",help_message);
    printf("%s",usage_message);
}
// End function s_show_help

/// @fn void app_cfg_t *app_cfg_new()
/// @brief allocate new app_cfg_t instance
/// @param[in] none
/// @return new app_cfg_t on success, NULL otherwise
static app_cfg_t *app_cfg_new()
{
    app_cfg_t *instance = (app_cfg_t *)malloc(sizeof(app_cfg_t));
    if(instance != NULL) {
        instance->verbose = 0;
        instance->host = strdup(RESON_HOST_DFL);
        instance->port = R7K_7KCENTER_PORT;
        instance->cycles = 0;
        instance->size = R7K_MAX_FRAME_BYTES;
        instance->dev = R7KC_DEV_7125_400KHZ;
        instance->mode = IMODE_SOCKET;
        instance->net_frames = false;
        instance->file_paths = NULL;
        instance->nsubs = 11;
        size_t len = instance->nsubs * sizeof(uint32_t);
        instance->subs = (uint32_t *)malloc(len);
        memset(instance->subs, 0, len);
        instance->subs[0] = 1003;
        instance->subs[1] = 1006;
        instance->subs[2] = 1008;
        instance->subs[3] = 1010;
        instance->subs[4] = 1012;
        instance->subs[5] = 1013;
        instance->subs[6] = 1015;
        instance->subs[7] = 1016;
        instance->subs[8] = 7000;
        instance->subs[9] = 7004;
        instance->subs[10] = 7027;
        instance->filter = false;
        instance->show_data = false;
    }
    return instance;
}

/// @fn void app_cfg_destroy( *app_cfg_new()
/// @brief release  app_cfg_t resources
/// @param[in] pself pointer to app_cfg_t reference
/// @return none
static void app_cfg_destroy(app_cfg_t **pself)
{
    if(pself != NULL) {
        app_cfg_t *self = *pself;
        free(self->host);
        mlist_destroy(&self->file_paths);
        free(self->subs);
        free(self);
        *pself = NULL;
    }
}

/// @fn void show_cfg(app_cfg_t *cfg, int indent)
/// @brief  show formatted configuration summary
/// @param[in] cfg app_cfg_t instance
/// @param[in] indent indent width (spaces)
/// @return none
static void show_cfg(app_cfg_t *cfg, int indent)
{
    int wkey = 16;
    int wval = 16;

    fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey, "App Config", wval, cfg);
    fprintf(stderr,"%*s%*s %*d\n",indent,(indent>0?" ":""),wkey,"verbose", wval, cfg->verbose);
    fprintf(stderr,"%*s%*s %*s\n",indent,(indent>0?" ":""),wkey,"host", wval, cfg->host);
    fprintf(stderr,"%*s%*s %*d\n",indent,(indent>0?" ":""),wkey,"port", wval, cfg->port);
    fprintf(stderr,"%*s%*s %*d\n",indent,(indent>0?" ":""),wkey,"cycles", wval, cfg->cycles);
    fprintf(stderr,"%*s%*s %*u\n",indent,(indent>0?" ":""),wkey,"size", wval, cfg->size);
    fprintf(stderr,"%*s%*s %*d\n",indent,(indent>0?" ":""),wkey,"dev", wval, cfg->dev);
    fprintf(stderr,"%*s%*s %*s\n",indent,(indent>0?" ":""),wkey,"mode",wval, (cfg->mode == IMODE_SOCKET ? "socket" : "file"));
    fprintf(stderr,"%*s%*s %*c\n",indent,(indent>0?" ":""),wkey,"data", wval, (cfg->show_data? 'Y' : 'N'));
    fprintf(stderr,"%*s%*s %*c\n",indent,(indent>0?" ":""),wkey,"net", wval, (cfg->net_frames ? 'Y' : 'N'));
    fprintf(stderr,"%*s%*s %*c\n",indent,(indent>0?" ":""),wkey,"filter", wval, (cfg->filter ? 'Y' : 'N'));
    fprintf(stderr,"%*s%*s %*u\n",indent,(indent>0?" ":""),wkey,"nsubs", wval, cfg->nsubs);

    if(cfg->nsubs > 0 && cfg->subs != NULL) {
        fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey, "Record Types", wval, cfg->subs);
        fprintf(stderr,"%*s%*s %*d\n",indent,(indent>0?" ":""),wkey, "N", wval, cfg->nsubs);

        int cols = 4;
        int rows = cfg->nsubs/cols;
        int rem = cfg->nsubs%cols;
        int i=0;

        for(i=0; i< cfg->nsubs; i++) {
            if(i%cols==0) {
                // indent row
                fprintf(stderr,"%*s%*d:",indent,(indent>0?" ":""), wkey, i);
            }
            // print record type
            fprintf(stderr," %u", cfg->subs[i]);

            if(i>0 && (i+1)%cols==0) {
                // add new line after column
                fprintf(stderr,"\n");
            }
        }
        if(i%cols != 0) {
            // add newline after partial row
            fprintf(stderr,"\n");
        }
    }

    if(cfg->file_paths != NULL) {
        fprintf(stderr,"%*s%*s %*p\n",indent,(indent>0?" ":""),wkey, "Files", wval, cfg->file_paths);
        fprintf(stderr,"%*s%*s %*zu\n",indent,(indent>0?" ":""),wkey, "N", wval, mlist_size(cfg->file_paths));
        char *path=(char *)mlist_first(cfg->file_paths);
        int i=0;
        while (NULL!=path) {
            fprintf(stderr,"%*s%*d: %*s\n",indent,(indent>0?" ":""), wkey, i++, wval, path);
            path = (char *)mlist_next(cfg->file_paths);
        }
    }
    fprintf(stderr,"\n");
    fprintf(stderr,"%*s%*s\n",indent,(indent>0?" ":""), wkey, "Debug Config");
    mxd_show();
    fprintf(stderr,"\n");
}

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
        {"data", no_argument, NULL, 0},
        {"help", no_argument, NULL, 0},
        {"version", no_argument, NULL, 0},
        {"file", required_argument, NULL, 0},
        {"net", no_argument, NULL, 0},
        {"host", required_argument, NULL, 0},
        {"cycles", required_argument, NULL, 0},
        {"size", required_argument, NULL, 0},
        {"dev", required_argument, NULL, 0},
        {"subs", required_argument, NULL, 0},
        {"filter", no_argument, NULL, 0},
        {NULL, 0, NULL, 0}};

    /* process argument list */
    while ((c = getopt_long(argc, argv, "", options, &option_index)) != -1){
        switch (c) {
                // long options all return c=0
            case 0:
                // verbose
                if (strcmp("verbose", options[option_index].name) == 0) {
                    sscanf(optarg,"%d",&cfg->verbose);
                }
                // version
                if (strcmp("version", options[option_index].name) == 0) {
                    version=true;
                }
                
                // help
                else if (strcmp("help", options[option_index].name) == 0) {
                    help = true;
                }

                // data
                if (strcmp("data", options[option_index].name) == 0) {
                    cfg->show_data = true;
                }

                // net
                else if (strcmp("net", options[option_index].name) == 0) {
                    cfg->net_frames = true;
                }

                // file
                else if (strcmp("file", options[option_index].name) == 0) {
                    if(cfg->file_paths == NULL) {
                        cfg->file_paths = mlist_new();
                        mlist_autofree(cfg->file_paths, free);
                    }
                    mlist_add(cfg->file_paths, optarg);
                    cfg->mode = IMODE_FILE;
                }

                // host
                else if (strcmp("host", options[option_index].name) == 0) {
                    char *ocopy = strdup(optarg);
                    char *host_tok = strtok(ocopy,":");
                    char *port_tok = strtok(NULL,":");
                    if(host_tok != NULL)
                        cfg->host=strdup(host_tok);
                    if(port_tok != NULL)
                        sscanf(port_tok, "%d", &cfg->port);
                    free(ocopy);
                    cfg->mode = IMODE_SOCKET;
                }
                // cycles
                else if (strcmp("cycles", options[option_index].name) == 0) {
                    sscanf(optarg,"%d",&cfg->cycles);
                }
                // size
                else if (strcmp("size", options[option_index].name) == 0) {
                    sscanf(optarg,"%u",&cfg->size);
                }
                // dev
                else if (strcmp("dev", options[option_index].name) == 0) {
                    r7k_device_t test = R7KC_DEV_INVALID;
                    if( (test=r7k_parse_devid(optarg)) != R7KC_DEV_INVALID){
                        cfg->dev = test;
                    }
                }
                // subs
                else if (strcmp("subs", options[option_index].name) == 0) {
                    char *ocopy = strdup(optarg);
                    char *next_tok = strtok(ocopy,",");

                    if(next_tok[0] == 'o' || next_tok[0] == 'O') {
                        cfg->nsubs = 0;
                        next_tok = strtok(NULL, ",");
                    }

                    while(next_tok != NULL) {
                        uint32_t sub=0;
                        if(sscanf(next_tok,"%u", &sub) == 1) {
                            size_t len = (cfg->nsubs + 1) * sizeof(uint32_t);
                            uint32_t *tmp = realloc(cfg->subs, len);
                            cfg->subs = tmp;
                            cfg->subs[cfg->nsubs] = sub;
                            cfg->nsubs++;
                        }
                        next_tok = strtok(NULL, ",");
                    }

                    free(ocopy);
                }

                // filter
                else if (strcmp("filter", options[option_index].name) == 0) {
                    cfg->filter = true;
                }

                break;
            default:
                help=true;
                break;
        }
        if (version) {
            MFRAME_SHOW_VERSION(FRAMES7K_NAME, FRAMES7K_BUILD);

//            r7kr_show_app_version(FRAMES7K_NAME,FRAMES7K_BUILD);
            exit(0);
       }
        if (help) {
            MFRAME_SHOW_VERSION(FRAMES7K_NAME, FRAMES7K_BUILD);
//           r7kr_show_app_version(FRAMES7K_NAME,FRAMES7K_BUILD);
            s_show_help();
            exit(0);
        }
    }// while

    mxd_setModule(MXDEBUG, 0, true, NULL);
    mxd_setModule(MXERROR, 5, false, NULL);
    mxd_setModule(FRAMES7K, 1, true, "frames7k");
    mxd_setModule(FRAMES7K_ERROR, 1, false, "frames7k.error");
    mxd_setModule(FRAMES7K_DEBUG, 1, true, "frames7k.debug");
    mxd_setModule(MXMSOCK, 1, true, "msock");
    mxd_setModule(R7KC, 1, true, "r7kc");
    mxd_setModule(R7KC_DEBUG, 1, true, "r7kc.debug");
    mxd_setModule(R7KC_ERROR, 1, true, "r7kc.error");
    mxd_setModule(R7KR, 1, true, "r7kr");
    mxd_setModule(R7KR_ERROR, 1, true, "r7kr.error");
    mxd_setModule(R7KR_DEBUG, 1, true, "r7kr.debug");

    switch (cfg->verbose) {
        case 0:
            break;
        case 1:
            mxd_setModule(MXDEBUG, 0, true, NULL);
            mxd_setModule(MXERROR, 5, false, NULL);
            mxd_setModule(FRAMES7K, 1, false, "frames7k");
            break;
        case 2:
            mxd_setModule(MXDEBUG, 5, true, NULL);
            mxd_setModule(MXERROR, 5, false, NULL);
            mxd_setModule(FRAMES7K, 5, false, "frames7k");
            break;
        case 3:
        case 4:
        case 5:
            mxd_setModule(MXDEBUG, 5, false, NULL);
            mxd_setModule(MXERROR, 5, false, NULL);
            mxd_setModule(FRAMES7K_ERROR, 5, false, "frames7k.error");
            mxd_setModule(FRAMES7K_DEBUG, 5, false, "frames7k.debug");
            mxd_setModule(MXMSOCK, 5, false, "msock");
            mxd_setModule(R7KC, 5, false, "r7kc");
            mxd_setModule(R7KC_DEBUG, 5, false, "r7kc.debug");
            mxd_setModule(R7KC_ERROR, 5, false, "r7kc.error");
            mxd_setModule(R7KR, 5, false, "r7kr");
            mxd_setModule(R7KR_ERROR, 5, false, "r7kr.error");
            mxd_setModule(R7KR_DEBUG, 5, false, "r7kr.debug");
            break;
        default:
            break;
    }

    if(optind > 0) {

        if(cfg->file_paths == NULL) {
            cfg->file_paths = mlist_new();
            mlist_autofree(cfg->file_paths, free);
        }
        cfg->mode = IMODE_FILE;
        for (int i=optind; i<argc; i++) {
            mlist_add(cfg->file_paths,strdup(argv[i]));
        }
    }

    if(cfg->verbose != 0) {
        show_cfg(cfg, 5);
    }
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
           // MX_LPRINT(FRAMES7K, 2, "received sig[%d]\n", signum);
            g_stop_flag=true;
            break;
        default:
            MX_ERROR_MSG("unhandled signal[%d]\n", signum);
            break;
    }
}
// End function termination_handler

/// @fn int s_app_main (app_cfg_t *cfg)
/// @brief app main.
/// @param[in] cfg app_cfg_t reference
/// @return 0 on success, -1 otherwise
static int s_app_main (app_cfg_t *cfg)
{
    int retval=-1;
    
    if (NULL!=cfg) {

        int count = 0;
        bool forever = false;

        if(cfg->cycles <= 0){
            forever = true;
        }
        
        // initialize reader
        // create and open socket connection

        r7kr_reader_t *reader = NULL;
        mfile_file_t *ifile = NULL;
        r7kr_flags_t r7k_flags = R7KR_NET_STREAM;
        char *s7k_path = (char *)mlist_first(cfg->file_paths);
        mfile_file_t *s7k_file = mfile_file_new(s7k_path);

        if(cfg->mode == IMODE_SOCKET){

            // configure socket reader
            MX_LPRINT(FRAMES7K, 1, "connecting host[%s:%d] dev[%d]\n", cfg->host, cfg->port, cfg->dev);
            reader = r7kr_reader_new(cfg->dev, cfg->host, cfg->port, cfg->size, cfg->subs, cfg->nsubs);

        } else if(cfg->mode == IMODE_FILE) {

            // configure file reader
            MX_LPRINT(FRAMES7K, 1, "processing file [%s]\n", s7k_path);
            reader = r7kr_freader_new(s7k_file, cfg->size, cfg->subs,  cfg->nsubs);
            r7k_flags =  (cfg->net_frames ? R7KR_NF_STREAM : R7KR_DRF_STREAM);
        }

        // show reader config
        if (cfg->verbose>1) {
            r7kr_reader_show(reader,true, 5);
        }

        uint32_t lost_bytes=0;
        // test r7kr_read_frame
        // must malloc; large stack variables cause SIGABRT
        byte *frame_buf = (byte *)malloc(R7K_MAX_FRAME_BYTES);
        memset(frame_buf, 0, R7K_MAX_FRAME_BYTES);

        if(cfg->mode == IMODE_SOCKET) {
            MX_LPRINT(FRAMES7K, 2, "reader connected [%s/%d] err(%s)\n", cfg->host, cfg->port,  me_strerror(me_errno));
        }

        retval=0;
        int read_retries = 5;
        long int seq_number = 0;

        while ( (forever || (count < cfg->cycles)) && !g_stop_flag) {
            int istat=0;
            count++;
            // clear frame buffer
            memset(frame_buf, 0, R7K_MAX_FRAME_BYTES);

            byte *pframe = frame_buf;
            if(cfg->mode == IMODE_FILE && !cfg->net_frames) {
                pframe = frame_buf + R7K_NF_BYTES;
            }

            // read frame
            if( (istat = r7kr_read_frame(reader, pframe, R7K_MAX_FRAME_BYTES, r7k_flags, 0.0, R7KR_READ_TMOUT_MSEC, &lost_bytes )) > 0){
                read_retries=5;

                r7k_nf_t *nf = (r7k_nf_t *)(frame_buf);
                r7k_drf_t *drf = (r7k_drf_t *)(frame_buf+R7K_NF_BYTES);

                if(cfg->mode == IMODE_FILE && !cfg->net_frames) {
                    memset((void *)frame_buf,0,R7K_NF_BYTES);

                    nf->protocol_version = R7K_NF_PROTO_VER;
                    nf->tx_id       = r7k_txid();
                    nf->seq_number  = seq_number++;
                    nf->offset      = R7K_NF_BYTES;
                    nf->packet_size = R7K_NF_BYTES + drf->size;
                    nf->total_size  = drf->size;
                    nf->total_records  = 1;
                }


                MX_LPRINT(FRAMES7K, 2, "r7kr_read_frame cycle[%d/%d] ret[%d] lost[%"PRIu32"]\n", count, cfg->cycles, istat, lost_bytes);

                // show all frames by default
                // in socket mode, only subscribed types received
                bool show_frame = true;

                if(cfg->mode == IMODE_FILE) {
                    // block if filter specified in file mode (using sub list)
                    if(cfg->filter && !r7kr_reader_issub(reader, drf->record_type_id))
                        show_frame = false;
                }

                // show contents
                if (show_frame) {

                    MX_MSG("NF:\n");
                    r7k_nf_show(nf,false,5);

                    MX_MSG("DRF:\n");
                    r7k_drf_show(drf,false,5);

                    if(cfg->show_data && istat > 0) {
                        // show raw data for verbose mode
                        MX_MSG("data:\n");
                        r7k_hex_show(frame_buf, istat, 16, true, 5);
                    }
                    MX_MSG("\n");
                }
            } else {
                // read error
                MX_LPRINT(FRAMES7K, 2, "ERR - r7kr_read_frame - cycle[%d/%d] ret[%d] me_err[%d] lost[%d]\n", count+1, cfg->cycles, istat, (me_errno-ME_ERRORNO_BASE), lost_bytes);

                if (me_errno==ME_ESOCK || me_errno==ME_EOF ||
                    me_errno==ME_ERECV || (read_retries-- <= 0)) {
                    if(cfg->mode == IMODE_SOCKET){
                        MX_ERROR_MSG("socket closed - reconnecting in 5 sec\n");
                        sleep(5);
                        r7kr_reader_connect(reader,true);
                    } else if(cfg->mode == IMODE_FILE) {

                        // read error, move on to next file
                        // TODO: check/handle specific errors

                        // close current file and reader
                        MX_LPRINT(FRAMES7K, 2, "closing file [%s]\n", s7k_path);
                        mfile_close(s7k_file);
                        mfile_file_destroy(&s7k_file);

                        MX_LMSG(FRAMES7K, 2, "closing reader\n");
                        r7kr_reader_destroy(&reader);

                        s7k_file = NULL;
                        reader = NULL;

                        // try the next path in the list
                        s7k_path = (char *)mlist_next(cfg->file_paths);

                        while(!g_stop_flag && s7k_path != NULL) {

                            MX_LPRINT(FRAMES7K, 1, "processing file [%s]\n", s7k_path);
                            // open next file
                            s7k_file = mfile_file_new(s7k_path);

                            reader = r7kr_freader_new(s7k_file, cfg->size, cfg->subs,  cfg->nsubs);

                            if(reader != NULL) {
                                // reader and file OK, process the file
                                MX_LPRINT(FRAMES7K, 2, "initialized reader using [%s] nsubs[%u]\n", s7k_path, cfg->nsubs);
                                lost_bytes = 0;
                                break;
                            }

                            // reader invalid (probably invalid file)
                            // release file, reader
                            if(s7k_file != NULL)
                                mfile_file_destroy(&s7k_file);
                            s7k_file = NULL;
                            reader = NULL;

                            // try next file in list
                            s7k_path = (char *)mlist_next(cfg->file_paths);
                        }

                        if(s7k_path == NULL) {
                            // no more files, done
                            MX_LMSG(FRAMES7K, 1, "no more files - quitting\n");
                            retval = 0;
                            break;
                        }

                        if(reader == NULL) {
                            // reader error
                            MX_LMSG(FRAMES7K, 1, "invalid reader - quitting\n");
                            break;
                        }
                    }
                    read_retries = 5;
                }
            }
        }
        
        if (g_stop_flag) {
            MX_LPRINT(FRAMES7K, 2, "interrupted - exiting cycles[%d/%d]\n", count, cfg->cycles);
        }else{
            MX_LPRINT(FRAMES7K, 2, "cycles[%d/%d]\n", count, cfg->cycles);
        }

        // release resources
        mfile_file_destroy(&s7k_file);
        r7kr_reader_destroy(&reader);
        free(frame_buf);
    }// else invalid argument

    return retval;
}
// End function s_app_main

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
    int retval=-1;
    
    // configure signal handling
    // for main thread
    struct sigaction saStruct;
    sigemptyset(&saStruct.sa_mask);
    saStruct.sa_flags = 0;
    saStruct.sa_handler = s_termination_handler;
    sigaction(SIGINT, &saStruct, NULL);

    app_cfg_t *cfg = app_cfg_new();

    // parse command line options
    parse_args(argc, argv, cfg);

    // run app
    retval=s_app_main(cfg);
    
    app_cfg_destroy(&cfg);

    return retval;
}
// End function main

