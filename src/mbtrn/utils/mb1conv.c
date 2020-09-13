///
/// @file mb1conv.c
/// @authors k. headley
/// @date 08 aug 2019

/// MB1 to MB-System F71/FBT record conversion

/// @sa doxygen-examples.c for more examples of Doxygen markup


/////////////////////////
// Terms of use
/////////////////////////
/*
 Copyright Information
 
 Copyright 2002-2019 MBARI
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

#include "mb1conv.h"
#include "mb1_msg.h"
#include "mb71_msg.h"
#include "mframe.h"
#include "mfile.h"
#include "medebug.h"

/////////////////////////
// Macros
/////////////////////////
#define MB1CONV_NAME "mb1conv"
#ifndef MB1CONV_BUILD
/// @def MB1CONV_BUILD
/// @brief module build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DMBTRN_BUILD=`date`
#define MB1CONV_BUILD ""VERSION_STRING(APP_BUILD)
#endif

// These macros should only be defined for 
// application main files rather than general C files
/*
 /// @def PRODUCT
 /// @brief header software product name
 #define PRODUCT "TBD_PRODUCT"
 
 /// @def COPYRIGHT
 /// @brief header software copyright info
 #define COPYRIGHT "Copyright 2002-2019 MBARI Monterey Bay Aquarium Research Institute, all rights reserved."
 /// @def NOWARRANTY
 /// @brief header software terms of use
 #define NOWARRANTY  \
 "This program is distributed in the hope that it will be useful,\n"\
 "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"\
 "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"\
 "GNU General Public License for more details (http://www.gnu.org/licenses/gpl-3.0.html)\n"
 */

#ifndef MAX
#define MAX(a,b) ( (a>b) ? a : b)
#endif

/////////////////////////
// Declarations 
/////////////////////////

/// @typedef struct app_cfg_s app_cfg_t
/// @brief application configuration parameter structure
typedef struct app_cfg_s{
    /// @var app_cfg_s::verbose
    /// @brief TBD
    int verbose;
    /// @var app_cfg_s::bswap
    /// @brief TBD
    bool bswap;
    /// @var app_cfg_s::ifile
    /// @brief TBD
    char *ifile;
    /// @var app_cfg_s::ofile
    /// @brief TBD
    char *ofile;
}app_cfg_t;

static void s_show_help();

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
    char help_message[] = "\n Convert MB1 (tbin) records to F71 (fbt)\n";
    char usage_message[] = "\n mb1conv [options]\n"
    "  --verbose=n : verbose output level\n"
    "  --help      : output help message\n"
    "  --version   : output version info\n"
    "  --no-swap   : don't byteswap\n"
    "  --ifile     : input file\n"
    "  --ofile     : output file (default is <ifile>.mb71)\n"
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
        {"no-swap", no_argument, NULL, 0},
        {"ifile", required_argument, NULL, 0},
        {"ofile", required_argument, NULL, 0},
        {NULL, 0, NULL, 0}};
    
    // process argument list
    while ((c = getopt_long(argc, argv, "", options, &option_index)) != -1){

        switch (c) {
                // long options all return c=0
            case 0:
                // verbose
                if (strcmp("verbose", options[option_index].name) == 0) {
                    cfg->verbose=atoi(optarg);
                }
                // bswap
                if (strcmp("no-swap", options[option_index].name) == 0) {
                    cfg->bswap=false;
                }
                
                // help
                else if (strcmp("help", options[option_index].name) == 0) {
                    help = true;
                }
                // version
                else if (strcmp("version", options[option_index].name) == 0) {
                    version = true;
                }
                // ifile
                else if (strcmp("ifile", options[option_index].name) == 0) {
                    if(NULL!=cfg->ifile){
                        free(cfg->ifile);
                    }
                    cfg->ifile=strdup(optarg);
                }
                // ofile
                else if (strcmp("ofile", options[option_index].name) == 0) {
                    if(NULL!=cfg->ofile){
                        free(cfg->ofile);
                    }
                    cfg->ofile=strdup(optarg);
                }
                break;
            default:
                help=true;
                break;
        }
        if (version) {
            //            mbtrn_show_app_version(TCPC_NAME,TCPC_BUILD);
            fprintf(stderr,"%s build %s\n",MB1CONV_NAME,MB1CONV_BUILD);
            exit(0);
        }
        if (help) {
            //            mbtrn_show_app_version(TCPC_NAME,TCPC_BUILD);
            s_show_help();
            exit(0);
        }
    }// while
    
    if(NULL==cfg->ofile){
        cfg->ofile = (char *)malloc(strlen(cfg->ifile)+6);
        memset(cfg->ofile,0,strlen(cfg->ifile)+6);
        sprintf(cfg->ofile,"%s",cfg->ifile);
        char *cp=strrchr(cfg->ofile,'.');
        
        if(NULL==cp){
            cp=cfg->ofile+strlen(cfg->ofile);
        }
        sprintf(cp,".mb71");
    }
    if( cfg->verbose>0){
    PDPRINT((stderr,"verbose   [%d]\n",(cfg->verbose)));
    PDPRINT((stderr,"swap      [%s]\n",(cfg->bswap?"Y":"N")));
    PDPRINT((stderr,"ifile     [%s]\n",cfg->ifile));
    PDPRINT((stderr,"ofile     [%s]\n",cfg->ofile));
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
            PDPRINT((stderr,"sig received[%d]\n",signum));
            g_interrupt=true;
            g_signal=signum;
            break;
        default:
            fprintf(stderr,"s_termination_handler: sig not handled[%d]\n",signum);
            break;
    }
}
// End function termination_handler

static app_cfg_t *app_cfg_new()
{
    app_cfg_t *instance = (app_cfg_t *)malloc(sizeof(app_cfg_t));
    
    if(NULL!=instance){
        memset(instance,0,sizeof(app_cfg_t));
        instance->verbose=MB1CONV_VERBOSE_DFL;
        instance->bswap=MB1CONV_BSWAP_DFL;
        instance->ifile=strdup(MB1CONV_IFILE_DFL);
        instance->ofile=NULL;
    }
    return instance;
}

static void app_cfg_destroy(app_cfg_t **pself)
{
    if(NULL!=pself){
        app_cfg_t *self = (app_cfg_t *)(*pself);
        if(NULL!=self){
            if(NULL!=self->ifile){
                free(self->ifile);
            }
            if(NULL!=self->ofile){
                free(self->ofile);
            }
            free(self);
            *pself=NULL;
        }
    }
}

static int32_t s_read_mb1_rec( mb1_frame_t **pdest, mfile_file_t *src, app_cfg_t *cfg)
{
    int32_t retval=-1;

    if(NULL!=src && NULL!=pdest){
        byte *bp = NULL;
        uint32_t readlen = 1;
        uint32_t record_bytes=0;
        int64_t read_bytes=0;
        mb1_frame_t *dest = *pdest;

        // sync to start of record
        bp = (byte *)dest->sounding;

        while( (read_bytes=mfile_read(src,(byte *)bp,readlen))==readlen){
            if(*bp=='M'){
                // found sync start
                record_bytes+=read_bytes;
                bp++;
                readlen=MB1_HEADER_BYTES-1;

                if(NULL!=cfg && cfg->verbose>2){
                    fprintf(stderr,"%d: read_bytes[%lld] bp[%p] err[%d/%s] readlen[%u]\n",__LINE__,read_bytes,bp,errno,strerror(errno),readlen);
                }
                break;
            }
        }

        if(NULL!=cfg && cfg->verbose>2){
            fprintf(stderr,"%d: read sync err[%d/%s]\n",__LINE__,errno,strerror(errno));
            fprintf(stderr,"%d: frame[%p]\n",__LINE__,dest);
            fprintf(stderr,"%d: sounding[%p]\n",__LINE__,dest->sounding);
            fprintf(stderr,"%d: chksum[%p]\n",__LINE__,dest->checksum);
            fprintf(stderr,"%d: readlen[%u]\n",__LINE__,readlen);
            fprintf(stderr,"%d: sizeof double[%zd]\n",__LINE__,sizeof(double));
            fprintf(stderr,"%d: sizeof int[%zd]\n",__LINE__,sizeof(int));
            fprintf(stderr,"%d: sizeof mb1_sounding_t[%zd]\n",__LINE__,sizeof(mb1_sounding_t));
            fprintf(stderr,"%d: sizeof mb1_beam_t[%zd]\n",__LINE__,sizeof(mb1_beam_t));
            fprintf(stderr,"%d: sizeof mb1_frame_t[%zd]\n",__LINE__,sizeof(mb1_frame_t));
        }

        // if start of sync found, read header (fixed-length sounding bytes)
        if(record_bytes>0 && (read_bytes=mfile_read(src,(byte *)bp,readlen))==readlen){

            if(NULL!=cfg && cfg->verbose>2){
            fprintf(stderr,"%d: type[x%08X]\n",__LINE__,dest->sounding->type);
            fprintf(stderr,"%d: nbeams[%u]\n",__LINE__,dest->sounding->nbeams);
            }
            record_bytes+=read_bytes;

            bp=NULL;
            readlen=0;
            retval=record_bytes;

            if(NULL!=dest && NULL!=dest->sounding ){
                if(dest->sounding->type==0x0031424D){
                    if(dest->sounding->nbeams>0 && dest->sounding->nbeams<=MB1_MAX_BEAMS){
                        if(mb1_frame_resize(&dest, dest->sounding->nbeams, MB1_RS_BEAMS)!=NULL){
                            bp=(byte *)&dest->sounding->beams[0];
                            readlen = dest->sounding->size-(MB1_HEADER_BYTES+MB1_CHECKSUM_BYTES);
                            *pdest=dest;
                        }else{fprintf(stderr,"%s:%d - ERR frame_resize\n",__func__,__LINE__);}
                    }else{
                        // don't resize, but update checksum pointer
                        dest->checksum=MB1_PCHECKSUM_U32(dest);
                    }
                }else{
                    if(NULL!=cfg && cfg->verbose>=2)
                    fprintf(stderr,"%s:%d - ERR invalid type[%u]\n",__func__,__LINE__,dest->sounding->type);
                    retval=-1;
                }
            }else{
                fprintf(stderr,"%s:%d - ERR dest[%p] sounding[%p]\n",__func__,__LINE__,dest,dest->sounding);
            }

            if(NULL!=cfg && cfg->verbose>2){
                fprintf(stderr,"%d: sounding->sz[%"PRIu32"] read[%"PRId64"/%"PRIu32"] err[%d/%s]\n",__LINE__,dest->sounding->size,read_bytes,readlen,errno,strerror(errno));
                fprintf(stderr,"%d: sounding->type[%08X]\n",__LINE__,dest->sounding->type);
                fprintf(stderr,"%d: sounding->checksum[%p]\n",__LINE__,dest->checksum);
                if(readlen>0)
                fprintf(stderr,"%d: sounding->checksum[%"PRIu32"]\n",__LINE__,*dest->checksum);
            }

            // if header OK, read sounding data (variable length)
            read_bytes=0;
            if(readlen>0 && (read_bytes=mfile_read(src,(byte *)bp,readlen))==readlen){
                record_bytes+=read_bytes;

                bp=(byte *)dest->checksum;
                readlen=MB1_CHECKSUM_BYTES;

                // read checksum
                if( (read_bytes=mfile_read(src,(byte *)bp,readlen))==readlen){
                    record_bytes+=read_bytes;
                    retval=record_bytes;

                    unsigned int checksum = mb1_frame_calc_checksum(dest);
                    if(checksum!=*(dest->checksum)){
                        fprintf(stderr,"checksum err (calc/read)[%08X/%08X] failed fp/fsz[%"PRId64"/%"PRId64"]\n",checksum,*(dest->checksum),mfile_seek(src,0,MFILE_CUR),mfile_fsize(src));
                    }
                }else{
                    fprintf(stderr,"%d: read failed err[%d/%s] fp/fsz[%"PRId64"/%"PRId64"]\n",__LINE__,errno,strerror(errno),mfile_seek(src,0,MFILE_CUR),mfile_fsize(src));
                }
            }else{
                if(readlen>0)
                fprintf(stderr,"%d: read failed err[%d/%s] readlen[%"PRId32"] read_bytes[%"PRId64"] fp/fsz[%"PRId64"/%"PRId64"]\n",__LINE__,errno,strerror(errno),readlen,read_bytes,mfile_seek(src,0,MFILE_CUR),mfile_fsize(src));
            }
        }else{
            if(mfile_seek(src,0,MFILE_CUR)==mfile_fsize(src)){
                if(NULL!=cfg && cfg->verbose>0)
                fprintf(stderr,"%d: read failed end of file reached fp/fsz[%"PRId64"/%"PRId64"] err[%d/%s]\n",__LINE__,mfile_seek(src,0,MFILE_CUR),mfile_fsize(src),errno,strerror(errno));
            }else{
            	fprintf(stderr,"%d: read failed err[%d/%s]\n",__LINE__,errno,strerror(errno));
            }
        }
        if(NULL!=cfg && cfg->verbose>2){
            fprintf(stderr,"%d: record_bytes[%"PRIu32"] retval[%d] err[%d/%s]\n",__LINE__,record_bytes,retval,errno,strerror(errno));
        }
    }
    return retval;
}

static int32_t s_mb1_to_mb71v5(byte **dest, int32_t size, mb1_frame_t *src, app_cfg_t *cfg)
{
    int32_t retval=-1;
    
    if(NULL!=dest && NULL!=src){
        
        int32_t mb71_size = (98+7*src->sounding->nbeams);
        byte *bp = *dest;
        int32_t msize = size;

        if(mb71_size>size || NULL==bp){
            bp=(byte *)realloc((void *)*dest,mb71_size);
            msize = mb71_size;
        }

        if(NULL!=bp){
            memset(bp,0,msize);
            *dest = bp;
            mb71v5_t *pmb71 = (mb71v5_t *)(*dest);
            retval = mb71_size;

            // 22069=0x5635:'V''5'
            pmb71->recordtype  = 0x5635;
            pmb71->time_d      = src->sounding->ts;
            pmb71->longitude   = src->sounding->lon;
            pmb71->latitude    = src->sounding->lat;
            pmb71->sonardepth  = src->sounding->depth;
            pmb71->altitude    = 0;
            pmb71->heading     = src->sounding->hdg;
            pmb71->speed       = 0.0;
            pmb71->roll        = 0.0;
            pmb71->pitch       = 0.0;
            pmb71->heave       = 0.0;
            pmb71->beam_xwidth = 1.0;
            pmb71->beam_lwidth = 1.0;
            pmb71->beams_bath  = src->sounding->nbeams;
            pmb71->beams_amp   = 0;
            pmb71->pixels_ss   = 0;
            pmb71->spare1      = 0;
            // pmb71->depth_scale;
            // pmb71->distance_scale;
            pmb71->ss_scalepower = 0x00;
            pmb71->ss_type       = 0x00;
            pmb71->imagery_type  = 0x02;
            pmb71->topo_type     = 0x02;
            
            int i=0;
            int nbeams = src->sounding->nbeams;
            // get scaling
            double depthmax = -1e6;
            double distmax = -1e6;

            for ( i = 0; i < nbeams; i++) {
                depthmax = MAX(depthmax, fabs(src->sounding->beams[i].rhoz));
                distmax  = MAX(distmax, fabs(src->sounding->beams[i].rhoy));
                distmax  = MAX(distmax, fabs(src->sounding->beams[i].rhox));
            }
            
            if (depthmax > 0.0)
                pmb71->depth_scale = 0.001 * (float)(MAX((depthmax / 30.0), 1.0));
            if (distmax > 0.0)
                pmb71->distance_scale = 0.001 * (float)(MAX((distmax / 30.0), 1.0));
            
            if(NULL!=cfg && cfg->verbose>0){
                //            fprintf(stderr,"size double[%d] float[%d] int[%d] uchar[%d] ushort[%d] short[%d] mbf71v5_hdr_t[%d]\r\n",sizeof(double),sizeof(float),sizeof(int),sizeof(unsigned char),sizeof(unsigned short),sizeof(short),sizeof(mbf71v5_hdr_t));
                fprintf(stderr,"nb[%2d] mb71_sz[%d] beams[%p/%ld]\r\n", nbeams,mb71_size,&pmb71->beam_bytes[0],((long)&pmb71->beam_bytes[0]-(long)pmb71));
                fprintf(stderr,"ts[%.3lf] lat[%.3lf] lon[%.3lf] sonar_depth[%.3lf]\r\n",src->sounding->ts,src->sounding->lat,src->sounding->lon,src->sounding->depth);
                fprintf(stderr,"max_depth[%.4lf] max_distance[%.4lf]\r\n",depthmax,distmax);
                fprintf(stderr,"depth_scale[%.4lf] distance_scale[%.4lf]\r\n\r\n",pmb71->depth_scale,pmb71->distance_scale);
            }

            unsigned char *bf = MB71_PBF(pmb71,nbeams);
            short *bz = MB71_PBZ(pmb71,nbeams);
            short *by = MB71_PBY(pmb71,nbeams);
            short *bx = MB71_PBX(pmb71,nbeams);

            for(i=0;i<nbeams;i++){

                // beam flag
                bf[i]= 0x00;
                // depth (mb1 rohz = bath-sonardepth)
                bz[i]= (src->sounding->beams[i].rhoz)/pmb71->depth_scale;
                // across-track
                by[i]= src->sounding->beams[i].rhoy/pmb71->distance_scale;
                // along-track
                bx[i]=src->sounding->beams[i].rhox/pmb71->distance_scale;
            }
            // no amp, sidescan
            //        pmb71->amp;
            //        pmb71->ss;
            //        pmb71->ss_acrosstrack;
            //        pmb71->ss_alongtrack;            }
        }
    }else{
        fprintf(stderr,"%s:%d Invalid arg\n",__func__,__LINE__);
    }// else invalid arg
    return retval;
}

static int s_app_main(app_cfg_t *cfg)
{
    int retval=-1;
    
    if(NULL!=cfg){
        int32_t test[2]={0};
        int64_t input_bytes=0;
        uint32_t output_bytes=0;
        uint32_t err_count=0;
        uint32_t rec_count=0;
        mfile_file_t *ifile = mfile_file_new(cfg->ifile);
        mfile_file_t *ofile = mfile_file_new(cfg->ofile);
        byte *mb71_bytes=NULL;
        mb1_frame_t *mb1=NULL;
        mb71v5_t *pmb71 = NULL;
        
        if( (test[0]=mfile_open(ifile, MFILE_RONLY))>0 &&
           (test[1]=mfile_mopen(ofile, MFILE_RDWR|MFILE_CREATE,MFILE_RU|MFILE_WU|MFILE_RG|MFILE_WG))>0){
            
            int record_size=0;
            int64_t file_size=mfile_fsize(ifile);;
            int32_t mb71_size=0;
            bool quit=false;
            
            while( !g_interrupt && !quit && input_bytes<file_size){
                
                // reset frame (or create if NULL)
                mb1_frame_resize(&mb1,0,MB1_RS_ALL);
                
                if((test[0]=s_read_mb1_rec(&mb1, ifile,cfg))>0){
                    rec_count++;
                    input_bytes+=test[0];
                    
                    if((mb71_size=s_mb1_to_mb71v5(&mb71_bytes,mb71_size,mb1, cfg))>0){
                        // cast bytes to mb71 record frame
                        pmb71 = (mb71v5_t *)mb71_bytes;
                        output_bytes+=mb71_size;
                        if( cfg->verbose>2){
                            mb1_frame_show(mb1,5,true);
                        }
                        if( cfg->verbose>1){
                            mb71v5_show(pmb71,5,true);
                        }
                        
                        // byte swap mb71 frame, per config
                        // (once swapped, don't use data members)
                        if(cfg->bswap){
                            byte *bp=(byte *)malloc(mb71_size);
                            if(NULL!=bp){
                                memset(bp,0,mb71_size);
                                if(mb71v5_bswap((mb71v5_t *)bp, pmb71)==0){
                                    mfile_write(ofile,(byte *)bp,mb71_size);
                                }else{
                                    if(cfg->verbose>2)fprintf(stderr,"%s:%d ERR mb71v5_bswap failed\n",__func__,__LINE__);
                                }
                                free(bp);
                            }else{
                                fprintf(stderr,"%s:%d ERR swap buffer malloc failed\n",__func__,__LINE__);
                            }
                        }else{
                            // write the bytes
                            mfile_write(ofile,(byte *)pmb71,mb71_size);
                        }
                    }else{
                        err_count++;
                        if(NULL!=cfg && cfg->verbose>0){
                            fprintf(stderr,"s_mb1_to_mb71v5 failed [%d] ecount[%u]\n",record_size,err_count);
                        }
                    }
                }else{
                    err_count++;
                    if(mfile_seek(ifile,0,MFILE_CUR)==mfile_fsize(ifile)){
                        if(cfg->verbose>0)
                        fprintf(stderr,"reached end of file\n");
                        quit=true;
                    }else{
                        if(NULL!=cfg && cfg->verbose>=2)
                        fprintf(stderr,"s_read_mb1_rec failed [%d] ecount[%u] fp/fsz[%"PRId64"/%"PRId64"]\n",test[0],err_count,mfile_seek(ifile,0,MFILE_CUR),mfile_fsize(ifile));
                    }
                }
            }// while
            
        }else{
            fprintf(stderr,"mfile_open failed i/o[%d/%d]\n",test[0],test[1]);
            err_count++;
        }
        
        mfile_file_destroy(&ifile);
        mfile_file_destroy(&ofile);
        if(NULL!=mb71_bytes){
            free(mb71_bytes);
        }
        mb1_frame_destroy(&mb1);
        retval=0;
        
        if(cfg->verbose>0){
            fprintf(stderr,"%s:%d rec/in/out/err[%"PRIu32"/%"PRId64"/%"PRIu32"/%"PRIu32"]\n",__FUNCTION__,__LINE__,rec_count,input_bytes,output_bytes,err_count);
        }
    }// else NULL cfg
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
    
    app_cfg_t *cfg = app_cfg_new();
    
    // configure signal handling
    // for main thread
    struct sigaction saStruct;
    sigemptyset(&saStruct.sa_mask);
    saStruct.sa_flags = 0;
    saStruct.sa_handler = s_termination_handler;
    sigaction(SIGINT, &saStruct, NULL);
    
    // parse command line args (update config)
    parse_args(argc, argv, cfg);
    
    retval=s_app_main(cfg);
    
    app_cfg_destroy(&cfg);
 
    return retval;
}
// End function main
