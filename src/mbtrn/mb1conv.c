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

/// @fn int mb_get_date()
/// @brief translate epoch time to yy/mm/dd/hh/mm/ss.ssssss
/// @return int status

/* 	function mb_get_date returns yy/mm/dd/hr/mi/sc calculated
 * 	from the number of seconds after 1/1/70 00:00:0 */

/* time conversions */
#define MB_SECINYEAR 31536000.0
#define MB_SECINDAY 86400.0
#define MB_SECINHOUR 3600.0
#define MB_SECINMINUTE 60.0
#define MB_ISECINYEAR 31536000
#define MB_ISECINDAY 86400
#define MB_ISECINHOUR 3600
#define MB_ISECINMINUTE 60
#define MB_IMININHOUR 60
#define MB_SECONDS_01JAN2000 946684800.0

int mb_get_date(int verbose, double time_d, int time_i[7]) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose: %d\n", verbose);
		fprintf(stderr, "dbg2       time_d:  %f\n", time_d);
	}
/* year-day conversion */
static const int yday[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

	/* get the date */
	const int daytotal = (int)(time_d / MB_SECINDAY);
	time_i[3] = (int)((time_d - daytotal * MB_SECINDAY) / MB_SECINHOUR);
	time_i[4] = (int)((time_d - daytotal * MB_SECINDAY - time_i[3] * MB_SECINHOUR) / MB_SECINMINUTE);
	time_i[5] = (int)(time_d - daytotal * MB_SECINDAY - time_i[3] * MB_SECINHOUR - time_i[4] * MB_SECINMINUTE);
	time_i[6] =
	    (int)1000000 * (time_d - daytotal * MB_SECINDAY - time_i[3] * MB_SECINHOUR - time_i[4] * MB_SECINMINUTE - time_i[5]);
	time_i[0] = (int)(time_d / MB_SECINYEAR) + 1970;
	int leapday = (time_i[0] - 1969) / 4;
	int yearday = daytotal - 365 * (time_i[0] - 1970) - leapday + 1;
	if (yearday <= 0) {
		time_i[0]--;
		leapday = (time_i[0] - 1969) / 4;
		yearday = daytotal - 365 * (time_i[0] - 1970) - leapday + 1;
	}
	leapday = 0;
	if (((time_i[0] % 4 == 0 && time_i[0] % 100 != 0) || time_i[0] % 400 == 0) && yearday > yday[2])
		leapday = 1;
	for (int i = 0; i < 12; i++)
		if (yearday > (yday[i] + leapday))
			time_i[1] = i + 1;
	time_i[2] = yearday - yday[time_i[1] - 1] - leapday;

	if (verbose >= 2) {
		fprintf(stderr, "\nMBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       year:    %d\n", time_i[0]);
		fprintf(stderr, "dbg2       month:   %d\n", time_i[1]);
		fprintf(stderr, "dbg2       day:     %d\n", time_i[2]);
		fprintf(stderr, "dbg2       hour:    %d\n", time_i[3]);
		fprintf(stderr, "dbg2       minute:  %d\n", time_i[4]);
		fprintf(stderr, "dbg2       second:  %d\n", time_i[5]);
		fprintf(stderr, "dbg2       microsec:%d\n", time_i[6]);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", 1);
	}

	/* return success */
	return (1);
}
// End function mb_get_date

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
    char cmnem=0;
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
        char *scpy = NULL;
        char *shost = NULL;
        char *sport = NULL;
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


static int32_t s_read_mb1_rec( mb1_frame_t **pdest, mfile_file_t *src)
{
    int32_t retval=-1;
    byte *bp = NULL;
    uint32_t readlen = 0;
    uint32_t record_bytes=0;
    int64_t read_bytes=0;

    if(NULL!=src && NULL!=pdest){
        mb1_frame_t *dest = *pdest;

        // sync to start of record
        bp = (byte *)dest->sounding;
        readlen = 1;
        while( (read_bytes=mfile_read(src,(byte *)bp,readlen))==readlen){
            if(*bp=='M'){
                // found sync start
                //                fprintf(stderr,"%s:%d\n",__FUNCTION__,__LINE__);
                record_bytes+=readlen;
                bp++;
                readlen=MB1_HEADER_BYTES-1;
                //                fprintf(stderr,"%d: read_bytes[%lld] bp[%p] err[%d/%s]\n",__LINE__,read_bytes,bp,errno,strerror(errno));
                break;
            }
        }

//        fprintf(stderr,"%d: read sync err[%d/%s]\n",__LINE__,errno,strerror(errno));
//        fprintf(stderr,"%d: frame[%p]\n",__LINE__,dest);
//        fprintf(stderr,"%d: sounding[%p]\n",__LINE__,dest->sounding);
//        fprintf(stderr,"%d: chksum[%p]\n",__LINE__,dest->checksum);

        // if start of sync found, read header (fixed-length sounding bytes)
        if(record_bytes>0 && (read_bytes=mfile_read(src,(byte *)bp,readlen))==readlen){

            record_bytes+=readlen;
            bp=NULL;
            readlen=0;
            if(NULL!=dest && NULL!=dest->sounding && dest->sounding->nbeams>0 && dest->sounding->nbeams<=MB1_MAX_BEAMS){

                if(mb1_frame_resize(&dest, dest->sounding->nbeams, MB1_RS_BEAMS)!=NULL){
                    bp=(byte *)&dest->sounding->beams[0];
                    readlen = dest->sounding->size-(MB1_HEADER_BYTES+MB1_CHECKSUM_BYTES);
                    *pdest=dest;
                }
            }


//                        fprintf(stderr,"%d: sounding->sz[%u] readlen[%u] err[%d/%s]\n",__LINE__,dest->sounding->size,readlen,errno,strerror(errno));

             // if header OK, read sounding data (variable length)
            if(readlen>0) {
              if ((read_bytes=mfile_read(src,(byte *)bp,readlen))==readlen){
                record_bytes+=readlen;
                bp=(byte *)dest->checksum;
                readlen=MB1_CHECKSUM_BYTES;
//              fprintf(stderr,"%d: read_bytes[%lld] bp[%p] err[%d/%s]\n",__LINE__,read_bytes,bp,errno,strerror(errno));
                // read checksum
                if( (read_bytes=mfile_read(src,(byte *)bp,readlen))==readlen){
                    record_bytes+=readlen;
                    retval=record_bytes;

                    unsigned int checksum = mb1_frame_calc_checksum(dest);

//                    fprintf(stderr,"checksum (calc/read)[%08X/%08X] - %s\n",checksum,*(dest->checksum),(checksum== (*dest->checksum)?"VALID":"INVALID"));
//                  fprintf(stderr,"%d: read_bytes[%lld] bp[%p] err[%d/%s]\n",__LINE__,read_bytes,bp,errno,strerror(errno));
                }else{
                    fprintf(stderr,"%d: read failed err[%d/%s]\n",__LINE__,errno,strerror(errno));
                }
              }else{
                  fprintf(stderr,"%d: read failed err[%d/%s]\n",__LINE__,errno,strerror(errno));
              }
            }else{
                fprintf(stderr,"%d: read failed err[%d/%s]\n",__LINE__,errno,strerror(errno));
            }
        }else{
            fprintf(stderr,"%d: read failed err[%d/%s]\n",__LINE__,errno,strerror(errno));
        }
    }
    //    fprintf(stderr,"%d: record_bytes[%lu] retval[%d] err[%d/%s]\n",__LINE__,record_bytes,retval,errno,strerror(errno));
    return retval;
}

static int32_t s_mb1_to_mb71v5(byte **dest, int32_t size, mb1_frame_t *src, app_cfg_t *cfg)
{
    int32_t retval=-1;

    if(NULL!=dest && NULL!=src){

        int32_t mb71_size = (98+7*src->sounding->nbeams);
        byte *bp = *dest;
        int32_t msize = size;

        if(mb71_size>size){
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
            pmb71->heading     = 180.0 / M_PI * src->sounding->hdg;
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
            double x=0;
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
                //fprintf(stderr,"nb[%2d] mb71_sz[%d] beams[%p/%ld]\r\n", nbeams,mb71_size,&pmb71->beam_bytes[0],((long)&pmb71->beam_bytes[0]-(long)pmb71));
                //fprintf(stderr,"ts[%.3lf] lat[%.3lf] lon[%.3lf] sonar_depth[%.3lf]\r\n",src->sounding->ts,src->sounding->lat,src->sounding->lon,src->sounding->depth);
                //fprintf(stderr,"max_depth[%.4lf] max_distance[%.4lf]\r\n",depthmax,distmax);
                //fprintf(stderr,"depth_scale[%.4lf] distance_scale[%.4lf]\r\n\r\n",pmb71->depth_scale,pmb71->distance_scale);
int time_i[7];
mb_get_date(0, src->sounding->ts, time_i);
fprintf(stderr, "%4.4d/%2.2d/%2.2d-%2.2d:%2.2d:%2.2d.%6.6d %10.6f %10.6f %7.3f %2d\n",
time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6],
src->sounding->lon, src->sounding->lat, src->sounding->depth, nbeams);
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


    }// else invalid arg
    return retval;
}

static int s_app_main(app_cfg_t *cfg)
{
    int retval=-1;
    int32_t test[2]={0};
    int itest=0;
    int record_size=0;
    int64_t input_bytes=0;
    int64_t file_size=0;

    int32_t mb71_size=0;
    uint32_t output_bytes=0;
    uint32_t err_count=0;
    uint32_t rec_count=0;
    bool quit=false;
    mfile_file_t *ifile = mfile_file_new(cfg->ifile);
    mfile_file_t *ofile = mfile_file_new(cfg->ofile);
    byte *mb71_bytes=NULL;
    mb1_frame_t *mb1=NULL;
    mb71v5_t *pmb71 = NULL;

    if( (itest=mfile_open(ifile, MFILE_RONLY))>0 &&
       (itest=mfile_mopen(ofile, MFILE_RDWR|MFILE_CREATE,MFILE_RU|MFILE_WU|MFILE_RG|MFILE_WG))>0){

        file_size=mfile_fsize(ifile);

        while( !g_interrupt && !quit && input_bytes<file_size){

            // reset frame (or create if NULL)
            mb1_frame_resize(&mb1,0,MB1_RS_ALL);

            if((test[0]=s_read_mb1_rec(&mb1, ifile))>0){
                rec_count++;
                input_bytes+=test[0];

                if((mb71_size=s_mb1_to_mb71v5(&mb71_bytes,mb71_size,mb1, cfg))>0){
                    // cast bytes to mb71 record frame
                    pmb71 = (mb71v5_t *)mb71_bytes;
                    output_bytes+=mb71_size;
                    if(NULL!=cfg){
                        if( cfg->verbose>2){
                            mb1_frame_show(mb1,5,true);
                       }
                        if( cfg->verbose>1){
                            mb71v5_show(pmb71,5,true);
                        }
                    }

                    // byte swap mb71 frame, per config
                    // (once swapped, don't use data members)
                    if(cfg->bswap){
                        mb71v5_bswap(NULL, pmb71);
                    }

                    // write the bytes
                    mfile_write(ofile,(byte *)pmb71,mb71_size);
                }else{
                     err_count++;
                    if(NULL!=cfg && cfg->verbose>0){
                        fprintf(stderr,"s_mb1_to_mb71v5 failed [%d] ecount[%d]\n",record_size,err_count);
                    }
                }
            }else{
                err_count++;
                fprintf(stderr,"s_read_mb1_rec failed [%d] ecount[%d]\n",test[0],err_count);
                quit=true;
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

    if(NULL!=cfg && cfg->verbose>0){
        fprintf(stderr,"%s:%d rec/in/out/err[%d/%lld/%d/%d]\n",__FUNCTION__,__LINE__,rec_count,input_bytes,output_bytes,err_count);
    }
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
