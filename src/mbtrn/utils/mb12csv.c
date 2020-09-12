///
/// @file mb12csv.c
/// @authors k. headley
/// @date 08 aug 2019

/// MB1 to CSV record conversion

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
//#include <math.h>

#include "mb12csv.h"
#include "mb1_msg.h"
#include "mframe.h"
#include "mfile.h"
#include "medebug.h"

/////////////////////////
// Macros
/////////////////////////
#define MB12CSV_NAME "mb12csv"
#ifndef MB12CSV_BUILD
/// @def MB12CSV_BUILD
/// @brief module build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DMBTRN_BUILD=`date`
#define MB12CSV_BUILD ""VERSION_STRING(APP_BUILD)
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

#define CFG_CSV_EN(f)  ( ((f&CF_OUT_CSV)!=0) ? true : false)
#define CFG_FILE_EN(f) ( ((f&CF_OUT_FILE)!=0) ? true : false)
#define CFG_HDR_EN(f)  ( ((f&CF_OUT_HDR)!=0) ? true : false)
#define CFG_DEG_EN(f)  ( ((f&CF_UNITS_DEG)!=0) ? true : false)
#define CFG_RAD_EN(f)  ( ((f&CF_UNITS_RAD)!=0) ? true : false)
#define CFG_OFLAGS(f)  (f&CF_OFLAGS)
#define CFG_UFLAGS(f)  (f&CF_UFLAGS)
#define CFG_HDR_CH_DFL '#'
typedef enum{
    CF_OUT_CSV  =0x01,
    CF_OUT_FILE =0x02,
    CF_OUT_HDR  =0x04,
    CF_UNITS_DEG=0x10,
    CF_UNITS_RAD=0x20,
    CF_OFLAGS   =0x07,
    CF_UFLAGS   =0x30,
}cfg_flags_t;

/// @typedef struct app_cfg_s app_cfg_t
/// @brief application configuration parameter structure
typedef struct app_cfg_s{
    /// @var app_cfg_s::verbose
    /// @brief TBD
    int verbose;
    /// @var app_cfg_s::ifile
    /// @brief TBD
    char *ifile;
    /// @var app_cfg_s::ofile
    /// @brief TBD
    char *ofile;
    /// @var app_cfg_s::flags
    /// @brief TBD
    cfg_flags_t flags;
    /// @var app_cfg_s::hdel
    /// @brief TBD
    char *hdel;

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
static const char *s_unit_str(int unit_flag){
   static const char *g_unit_names[2]={"degrees","radians"};
    const char *retval;
    switch (unit_flag) {
        case CF_UNITS_DEG:
            retval=g_unit_names[0];
            break;
        case CF_UNITS_RAD:
            retval=g_unit_names[1];
            break;
        default:
            retval=NULL;
            break;
    }
    return retval;
}

/// @fn void s_show_help()
/// @brief output user help message to stdout.
/// @return none
static void s_show_help()
{
    char help_message[] = "\n Convert MB1 records to CSV\n";
    char usage_message[] = "\n mb12csv [options]\n"
    "  --verbose=n : verbose output level\n"
    "  --help      : output help message\n"
    "  --version   : output version info\n"
    "  --ifile     : input file\n"
    "  --ofile     : output file (default is stdout only)\n"
    "  --nocsv     : suppress stdout output\n"
    "  --rad       : use radians for heading, lat/lon\n"
    "  --deg       : use degrees for heading, lat/lon\n"
    "  --header=s  : output descriptive header (s - delimiter, e.g. #, //)\n"
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
        {"rad", no_argument, NULL, 0},
        {"deg", no_argument, NULL, 0},
        {"nocsv", no_argument, NULL, 0},
        {"ifile", required_argument, NULL, 0},
        {"ofile", required_argument, NULL, 0},
        {"header", required_argument, NULL, 0},
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
                    cfg->flags|=CF_OUT_FILE;
                }
                // quiet
                else if (strcmp("nocsv", options[option_index].name) == 0) {
                    cfg->flags &= ~(CF_OUT_CSV);
                }
                // deg
                else if (strcmp("deg", options[option_index].name) == 0) {
                    cfg->flags &= ~(CF_UFLAGS);
                    cfg->flags |= CF_UNITS_DEG;
                }
                // rad
                else if (strcmp("rad", options[option_index].name) == 0) {
                    cfg->flags &= ~(CF_UFLAGS);
                    cfg->flags |= CF_UNITS_RAD;
                }
                // header
                else if (strcmp("header", options[option_index].name) == 0) {
                    cfg->flags |= CF_OUT_HDR;
                    if(NULL!=cfg->hdel)free(cfg->hdel);
                    cfg->hdel=strdup(optarg);
                }
                break;
            default:
                help=true;
                break;
        }
        if (version) {
            //            mbtrn_show_app_version(TCPC_NAME,TCPC_BUILD);
            fprintf(stderr,"%s build %s\n",MB12CSV_NAME,MB12CSV_BUILD);
            exit(0);
        }
        if (help) {
            //            mbtrn_show_app_version(TCPC_NAME,TCPC_BUILD);
            s_show_help();
            exit(0);
        }
    }// while
    
    if( cfg->verbose>0){
        PDPRINT((stderr,"verbose   [%d]\n",(cfg->verbose)));
        PDPRINT((stderr,"ifile     [%s]\n",cfg->ifile));
        PDPRINT((stderr,"ofile     [%s]\n",cfg->ofile));
        PDPRINT((stderr,"file      [%s]\n",( (CFG_FILE_EN(cfg->flags)) ?"Y":"N")));
        PDPRINT((stderr,"csv       [%s]\n",( (CFG_CSV_EN(cfg->flags)) ?"Y":"N")));
        PDPRINT((stderr,"units     [%s]\n",s_unit_str(CFG_UFLAGS(cfg->flags))));
        PDPRINT((stderr,"header    [%s/%s]\n",( (CFG_HDR_EN(cfg->flags)) ?"Y":"N"),cfg->hdel));
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
        instance->verbose=MB12CSV_VERBOSE_DFL;
        instance->ifile=NULL;
        instance->ofile=NULL;
        instance->flags=(CF_UNITS_DEG|CF_OUT_CSV);
        instance->hdel=NULL;
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

static void s_out_header(app_cfg_t *cfg)
{
    if(NULL!=cfg)
    {
        fprintf(stdout,"%srecord type             (MB1)\n",(NULL==cfg->hdel?"":cfg->hdel) );
        fprintf(stdout,"%stimestamp               (decimal epoch sec)\n",(NULL==cfg->hdel?"":cfg->hdel) );
        fprintf(stdout,"%slongitude               (%s)\n",(NULL==cfg->hdel?"":cfg->hdel), s_unit_str(CFG_UFLAGS(cfg->flags)) );
        fprintf(stdout,"%slatitude                (%s)\n",(NULL==cfg->hdel?"":cfg->hdel), s_unit_str(CFG_UFLAGS(cfg->flags)) );
        fprintf(stdout,"%sdepth                   (m)\n",(NULL==cfg->hdel?"":cfg->hdel) );
        fprintf(stdout,"%sheading                 (%s)\n",(NULL==cfg->hdel?"":cfg->hdel), s_unit_str(CFG_UFLAGS(cfg->flags)) );
        fprintf(stdout,"%snbeams                  (beams)\n",(NULL==cfg->hdel?"":cfg->hdel) );
        fprintf(stdout,"%sbeams[3][nbeams] x,y,z  (m)\n",(NULL==cfg->hdel?"":cfg->hdel) );
    }
}

static int32_t s_mb1_to_csv(byte **dest, int32_t size, mb1_frame_t *src, app_cfg_t *cfg)
{
    int32_t retval=-1;
    
    if(NULL!=dest && NULL!=src){
        
        int32_t csv_size = 4*1024;//(98+7*src->sounding->nbeams);
        byte *bp = *dest;
        int32_t msize = size;

        if(csv_size>size){
            bp=(byte *)realloc((void *)*dest,csv_size);
            msize = csv_size;
        }

        char *ostr=(char *)bp;
        char *op=(char *)ostr;
       if(NULL!=bp && NULL!=cfg){
            memset(bp,0,msize);
            *dest = bp;

           // apply unit conversions, if any
           double lat=src->sounding->lat;
           double lon=src->sounding->lon;
           double hdg=src->sounding->hdg;
           if(CFG_RAD_EN(cfg->flags)){
               lat *= 180./M_PI;
               lon *= 180./M_PI;
               hdg *= 180./M_PI;
           }

            sprintf(op,"MB1,");
            op=ostr+strlen(ostr);
            sprintf(op,"%.3lf,",src->sounding->ts);
            op=ostr+strlen(ostr);
            sprintf(op,"%e,",lon);
            op=ostr+strlen(ostr);
            sprintf(op,"%e,",lat);
            op=ostr+strlen(ostr);
            sprintf(op,"%e,",src->sounding->depth);
            op=ostr+strlen(ostr);
            sprintf(op,"%e,",hdg);
            op=ostr+strlen(ostr);
            sprintf(op,"%u,",src->sounding->nbeams);
            op=ostr+strlen(ostr);

            int i=0;
            int nbeams = src->sounding->nbeams;

            for(i=0;i<nbeams;i++){
                sprintf(op,"%e,",src->sounding->beams[i].rhox);
                op=ostr+strlen(ostr);
                sprintf(op,"%e,",src->sounding->beams[i].rhoy);
                op=ostr+strlen(ostr);
                sprintf(op,"%e,",src->sounding->beams[i].rhoz);
                op=ostr+strlen(ostr);
            }
        }

        // remove trailing commas and space
        while(op>ostr && (*op=='\0' || *op==',' || isspace(*op)) ){
            *op='\0';
            op--;
        }
        retval=(NULL!=ostr?strlen(ostr):-1);

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
        mfile_file_t *ofile = (NULL==cfg->ofile?NULL:mfile_file_new(cfg->ofile));
        byte *csv_bytes=NULL;
        mb1_frame_t *mb1=NULL;

        // open input file
        if( (test[0]=mfile_open(ifile, MFILE_RONLY))>0){

            if(NULL!=ofile){
                // open outfile if specified
                if( (test[1]=mfile_mopen(ofile, MFILE_RDWR|MFILE_CREATE,MFILE_RU|MFILE_WU|MFILE_RG|MFILE_WG))<=0 ){
                    fprintf(stderr,"ERR - could not open ofile\n");
                }
            }
            
            int record_size=0;
            int64_t file_size=mfile_fsize(ifile);;
            int32_t csv_size=0;
            bool quit=false;


            if(CFG_HDR_EN(cfg->flags)){
                // output header to stdout
                s_out_header(cfg);
            }
            // iterate over input records until done or interrupted
            while( !g_interrupt && !quit && input_bytes<file_size){
                
                // reset frame (or create if NULL)
                mb1_frame_resize(&mb1,0,MB1_RS_ALL);

                // read an mb1 record
                if((test[0]=s_read_mb1_rec(&mb1, ifile, cfg))>0){
                    rec_count++;
                    input_bytes+=test[0];

                    // convert to CSV
                    if((csv_size=s_mb1_to_csv(&csv_bytes,csv_size,mb1, cfg))>0){

                        output_bytes+=csv_size;

                        if( cfg->verbose>2){
                            mb1_frame_show(mb1,5,true);
                        }

                        if(CFG_CSV_EN(cfg->flags)){
                            // output to stdout
                        	fprintf(stdout,"%s\n",csv_bytes);
                        }

                        if(CFG_FILE_EN(cfg->flags) && NULL!=ofile ){
                            // output to ofile
                            char eol='\n';
                            mfile_write(ofile,(byte *)csv_bytes,csv_size);
                            mfile_write(ofile,(byte *)&eol,1);
                        }
                    }else{
                        err_count++;
                        if(NULL!=cfg && cfg->verbose>0){
                            fprintf(stderr,"s_mb1_to_csv failed [%d] ecount[%u]\n",record_size,err_count);
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
//                    quit=true;
                }
                
            }// while
            
        }else{
            fprintf(stderr,"mfile_open failed i/o[%d/%d]\n",test[0],test[1]);
            err_count++;
        }
        
        mfile_file_destroy(&ifile);
        mfile_file_destroy(&ofile);
        if(NULL!=csv_bytes){
            free(csv_bytes);
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
