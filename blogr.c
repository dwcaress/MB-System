///
/// @file blogr
/// @authors k. Headley
/// @date 01 jan 2018

/// TRN preprocess binary log reader
/// reads binary packet format sent by mbtrnpreprocess

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
#include "mdebug.h"
#include "iowrap.h"
#include "mbtrn.h"

/////////////////////////
// Macros
/////////////////////////
#define BLOGR_NAME "blogr"
#ifndef BLOGR_BUILD
/// @def BLOGR_BUILD
/// @brief module build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DMBTRN_BUILD=`date`
#define BLOGR_BUILD ""VERSION_STRING(MBTRN_BUILD)
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

/// @def ID_APP
/// @brief debug module ID
#define ID_APP 1
/// @def ID_V1
/// @brief debug module ID
#define ID_V1 2
/// @def ID_V2
/// @brief debug module ID
#define ID_V2 3
/// @def ID_V3
/// @brief debug module ID
#define ID_V3 4

/////////////////////////
// Declarations
/////////////////////////
#define HSYNC_BYTES 4
#define HDR_FULL_BYTES (HSYNC_BYTES+4+5*8+4)
#define HDR_ONLY_BYTES  (HDR_FULL_BYTES-HSYNC_BYTES)
#define CHKSUM_BYTES 4
#define BEAM_BYTES (4+3*8)
#define CSV_NAME_DFL "tbin.csv"
#define MAX_VERBOSE 3

/// @typedef enum oflags_t oflags_t
/// @brief flags specifying output types
typedef enum{OF_NONE=0,OF_SOUT=0x1,OF_CSV=0x2}oflags_t;

/// @typedef struct app_cfg_s app_cfg_t
/// @brief application configuration parameter structure
typedef struct app_cfg_s{
    /// @var app_cfg_s::verbose
    /// @brief enable verbose output
    int verbose;
    /// @var app_cfg_s::nfiles
    /// @brief file list
     int nfiles;
    /// @var app_cfg_s::files
    /// @brief file list
    char **files;
    /// @var app_cfg_s::oflags
    /// @brief output type flags
    oflags_t oflags;
    /// @var app_cfg_s::csv_path
    /// @brief csv file name
    char *csv_path;
}app_cfg_t;

#pragma pack(push, 1)
/// @typedef struct trn_beam_s trn_beam_t
/// @brief application configuration parameter structure
typedef struct trn_beam_s{
    int32_t idx;
    double atrk;
    double xtrk;
    double bath;
}trn_beam_t;

/// @typedef struct trn_hdr_s trn_hdr_t
/// @brief trn binary data structure
typedef struct trn_hdr_s{
    char sync[4];
    int32_t len;
    double time;
    double lat;
    double lon;
    double dep;
    double hdg;
    int32_t bcount;
}trn_hdr_t;

/// @typedef struct trn_data_s trn_data_t
/// @brief trn binary data structure
typedef struct trn_data_s{
    trn_hdr_t hdr;
    trn_beam_t *pbeams;
    uint32_t chksum;
}trn_data_t;
#pragma pack(pop)


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
    char help_message[] = "\nmbtrnpreprocess binary log reader\n";
    char usage_message[] = "\nblogr [options]\n"
    "--verbose=n : verbose output, n>0\n"
    "--help      : output help message\n"
    "--version   : output version info\n"
    "--sout      : export to stdout\n"
    "--csv=file  : export to csv file\n"
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
        {"sout", no_argument, NULL, 0},
        {"csv", required_argument, NULL, 0},
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
                // help
                else if (strcmp("help", options[option_index].name) == 0) {
                    help = true;
                }
                // version
                else if (strcmp("version", options[option_index].name) == 0) {
                    version = true;
                }
                // sout
                else if (strcmp("sout", options[option_index].name) == 0) {
                    cfg->oflags|=OF_SOUT;
                }
                // csv
                else if (strcmp("csv", options[option_index].name) == 0) {
                    if (cfg->csv_path) {
                        free(cfg->csv_path);
                    }
                    cfg->oflags|=OF_CSV;
                    cfg->csv_path=strdup(optarg);
                }
                break;
            default:
                help=true;
                break;
        }
        if (version) {
            mbtrn_show_app_version(BLOGR_NAME,BLOGR_BUILD);
            exit(0);
        }
        if (help) {
            mbtrn_show_app_version(BLOGR_NAME,BLOGR_BUILD);
            s_show_help();
            exit(0);
        }
    }// while
    
    if (cfg->verbose>MAX_VERBOSE) {
        cfg->verbose=MAX_VERBOSE;
    }
    if (cfg->verbose<0) {
        cfg->verbose=0;
    }


    cfg->files=&argv[optind];
    cfg->nfiles=argc-optind;
    
    // initialize reader
    // create and open socket connection

    mdb_set_name(ID_APP,"mbtrnpreprocess");
    mdb_set_name(ID_V1,"verbose_1");
    mdb_set_name(ID_V2,"verbose_2");
    mdb_set_name(ID_V3,"verbose_3");
    
    // use MDI_ALL to globally set module debug output
    // may also set per module basis using module IDs
    // defined in mconfig.h:
    // ID_APP, ID_V1, ID_V2, ID_V3,
    // valid level values are
    // MDL_UNSET,MDL_NONE
    // MDL_FATAL, MDL_ERROR, MDL_WARN
    // MDL_INFO, MDL_DEBUG
    
    mdb_set(MDI_ALL,MDL_NONE);

    switch (cfg->verbose) {
        case 0:
            mdb_set(ID_APP,MDL_ERROR);
            break;
        case 1:
            mdb_set(ID_APP,MDL_DEBUG);
            mdb_set(ID_V1,MDL_DEBUG);
            break;
        case 2:
            mdb_set(ID_APP,MDL_DEBUG);
            mdb_set(ID_V1,MDL_DEBUG);
            mdb_set(ID_V2,MDL_DEBUG);
            break;
        case 3:
            mdb_set(ID_APP,MDL_DEBUG);
            mdb_set(ID_V1,MDL_DEBUG);
            mdb_set(ID_V2,MDL_DEBUG);
            mdb_set(ID_V3,MDL_DEBUG);
            break;

        default:
            mdb_set(ID_APP,MDL_INFO);
            break;
    }
    if (cfg->verbose!=0) {
        fprintf(stderr,"verbose [%s]\n",(cfg->verbose?"Y":"N"));
        fprintf(stderr,"nfiles  [%d]\n",cfg->nfiles);
        if (cfg->nfiles>0) {
            for (int i=0; i<cfg->nfiles; i++) {
                fprintf(stderr,"files[%d] [%s]\n",i,cfg->files[i]);
            }
        }
        fprintf(stderr,"sout    [%c]\n",(cfg->oflags&OF_SOUT?'Y':'N'));
        fprintf(stderr,"csv     [%c]\n",(cfg->oflags&OF_CSV?'Y':'N'));
    }

}
// End function parse_args

/// @fn int s_out_sout(trn_data_t *record)
/// @brief export record to stdout
int s_out_sout(trn_data_t *record)
{
    int retval=0;

    if (NULL!=record) {
        trn_hdr_t *phdr     = &record->hdr;
       fprintf(stdout,"\nts[%.3lf] beams[%03d]\nlat[%.4f] lon[%.4lf] hdg[%6.2lf] sd[%7.2lf]\n",\
               phdr->time,
               phdr->bcount,
               phdr->lat,
               phdr->lon,
               phdr->hdg,
               phdr->dep
               );
        if (phdr->bcount<=512) {
            for (int j = 0; j < phdr->bcount; j++) {
                fprintf(stdout,"n[%03"PRId32"] atrk/X[%+10.3lf] ctrk/Y[%+10.3lf] dpth/Z[%+10.3lf]\n",
                        record->pbeams[j].idx,
                        record->pbeams[j].atrk,
                        record->pbeams[j].xtrk,
                        record->pbeams[j].bath);
            }
        }
    }else{
        MMDEBUG(ID_V1,"invalid argument\n");
        retval=-1;
    }
    return retval;
}

/// @fn int s_out_csv(trn_data_t *record)
/// @brief export record to csv file
int s_out_csv(iow_file_t *dest, trn_data_t *record)
{
    int retval=0;
    
    if (NULL!=record) {
        trn_hdr_t *phdr     = &record->hdr;
        iow_fprintf(dest,"%.3lf,%d,%lf,%lf,%lf,%lf",\
               phdr->time,
               phdr->bcount,
               phdr->lat,
               phdr->lon,
               phdr->hdg,
               phdr->dep
               );
        for (int j = 0; j < phdr->bcount; j++) {
            iow_fprintf(dest,",%d,%+lf,%+lf,%+lf",
                   record->pbeams[j].idx,
                   record->pbeams[j].atrk,
                   record->pbeams[j].xtrk,
                   record->pbeams[j].bath);
        }
        iow_fprintf(dest,"\n");
    }else{
        MMDEBUG(ID_V1,"invalid argument\n");
        retval=-1;
    }
    return retval;
}

/// @fn int s_process_file(char *file)
/// @brief process file.
int s_process_file(app_cfg_t *cfg)
{
    int retval=0;
    
   if (NULL!=cfg && cfg->nfiles>0) {
        for (int i=0; i<cfg->nfiles; i++) {
            MMDEBUG(ID_V1,"processing %s\n",cfg->files[i]);
            iow_file_t *ifile = iow_file_new(cfg->files[i]);
            int test=0;
            iow_file_t *csv_file = NULL;
            
            if(cfg->csv_path!=NULL){
                csv_file=iow_file_new(cfg->csv_path);
                if ( (test=iow_mopen(csv_file,IOW_RDWR|IOW_CREATE,IOW_RU|IOW_WU|IOW_RG|IOW_WG)) <= 0) {
                    MMERROR(ID_APP,"could not open CSV file\n");
                    csv_file=NULL;
                }else{
                    MMDEBUG(ID_APP,"opened CSV file [%s]\n",cfg->csv_path);
                }
            }
            
            
            if ( (test=iow_open(ifile,IOW_RONLY)) > 0) {
                MMDEBUG(ID_V1,"open OK [%s]\n",cfg->files[i]);
                
                trn_data_t record;
                memset(&record,0,sizeof(trn_data_t));
                trn_data_t *precord = &record;
                trn_hdr_t *phdr     = &precord->hdr;
                bool ferror=false;
                int64_t rbytes=0;
                bool sync_valid=false;
                bool hdr_valid=false;
                bool rec_valid=false;
                while (!ferror) {
                    byte *sp = (byte *)phdr->sync;
                    sync_valid=false;
                    hdr_valid=false;
                    rec_valid=false;
                    while (!sync_valid) {
                        if( ((rbytes=iow_read(ifile,(byte *)sp,1))==1) && *sp=='M'){
                            sp++;
                            if( ((rbytes=iow_read(ifile,(byte *)sp,1))==1) && *sp=='B'){
                                sp++;
                                if( ((rbytes=iow_read(ifile,(byte *)sp,1))==1) && *sp=='1'){
                                    sp++;
                                    if( ((rbytes=iow_read(ifile,(byte *)sp,1))==1) && *sp=='\0'){
                                        MMDEBUG(ID_V1,"sync read slen[%d]\n",HSYNC_BYTES);
                                        MMDEBUG(ID_V2,"  sync     ['%c''%c''%c''%c']/[%02X %02X %02X %02X]\n",
                                                phdr->sync[0],
                                                phdr->sync[1],
                                                phdr->sync[2],
                                                phdr->sync[3],
                                                phdr->sync[0],
                                                phdr->sync[1],
                                                phdr->sync[2],
                                                phdr->sync[3]);
                                    	sync_valid=true;
                                        break;
                                    }else{
                                        sp=(byte *)phdr->sync;
                                    }
                                }else{
                                    sp=(byte *)phdr->sync;
                                }
                            }else{
                                sp=(byte *)phdr->sync;
                            }
                        }
                        if(rbytes<=0){
                            MMDEBUG(ID_APP,"reached EOF looking for sync\n");
                            ferror=true;
                            break;
                        }
                    }
                    if (sync_valid && !ferror) {
                        
                        byte *hp = (byte *)phdr+HSYNC_BYTES;

                        if( ((rbytes=iow_read(ifile,hp,HDR_ONLY_BYTES))==HDR_ONLY_BYTES)){
                            
                            int32_t cmplen = HDR_FULL_BYTES+phdr->bcount*BEAM_BYTES+CHKSUM_BYTES;
                            
                            if (phdr->len == cmplen ) {
                                MMDEBUG(ID_V1,"header read hlen[%d/%lld]\n",HDR_ONLY_BYTES,rbytes);
                                hdr_valid=true;
                                
                                MMDEBUG(ID_V2,"  len    [%d]\n",phdr->len);
                                MMDEBUG(ID_V2,"  time   [%.3f]\n",phdr->time);
                                MMDEBUG(ID_V2,"  lat    [%.3f]\n",phdr->lat);
                                MMDEBUG(ID_V2,"  lon    [%.3f]\n",phdr->lon);
                                MMDEBUG(ID_V2,"  dep    [%.3f]\n",phdr->dep);
                                MMDEBUG(ID_V2,"  hdg    [%.3f]\n",phdr->hdg);
                                MMDEBUG(ID_V2,"  bcount [%d]\n",phdr->bcount);
                            }else{
                                MMWARN(ID_APP,"record len invalid l[%d] l*[%d]\n",phdr->len,cmplen);
                            }
                        }else{
                            MMERROR(ID_APP,"could not read header bytes [%lld]\n",rbytes);
                            ferror=true;
                        }
                    }
                    
                   byte *cp = (byte *)&precord->chksum;
                    
                    if (hdr_valid && ferror==false && phdr->bcount>0) {
                        
                        int32_t beamsz = phdr->bcount*BEAM_BYTES;
                        precord->pbeams = (trn_beam_t *)malloc(beamsz);
                        
                        if( NULL != precord->pbeams ){
                            
                            byte *bp = (byte *)precord->pbeams;
                            cp = (byte *)&precord->chksum;

                            if( ((rbytes=iow_read(ifile,bp,beamsz))==beamsz)){
                                
                                MMDEBUG(ID_V1,"beams read blen[%d/%lld]\n",beamsz,rbytes);
                                
                                if( ((rbytes=iow_read(ifile,cp,CHKSUM_BYTES))==CHKSUM_BYTES)){
                                    MMDEBUG(ID_V1,"chksum read clen[%lld]\n",rbytes);
                                    MMDEBUG(ID_V2,"  chksum [%0X]\n",precord->chksum);

                                    rec_valid=true;
                                }else{
                                    MMWARN(ID_APP,"chksum read failed [%lld]\n",rbytes);
                                }
                                
                            }else{
                                MMDEBUG(ID_V1,"beam read failed pb[%p] read[%lld]\n",precord->pbeams,rbytes);
                            }
                            
                        }else{
                            MMERROR(ID_APP,"beam data malloc failed\n");
                        }
                    }else{
                        if( ((rbytes=iow_read(ifile,cp,CHKSUM_BYTES))==CHKSUM_BYTES)){
                            MMDEBUG(ID_V1,"read chksum clen[%lld]\n",rbytes);
                            MMDEBUG(ID_V2,"  chksum [%0X]\n",precord->chksum);
                            
                            rec_valid=true;
                        }else{
                            MMWARN(ID_APP,"chksum read failed [%lld]\n",rbytes);
                        }
                    }
                    
                    if (rec_valid && ferror==false) {
                        if (cfg->oflags&OF_SOUT) {
                            s_out_sout(precord) ;
                        }
                        if ( (cfg->oflags&OF_CSV) && (NULL!=csv_file) ) {
                            s_out_csv(csv_file,precord);
                        }
                    }
                    free(precord->pbeams);
                    precord->pbeams=NULL;

                }
                iow_close(ifile);
                iow_file_destroy(&ifile);
                iow_file_destroy(&csv_file);
            }else{
                MMERROR(ID_APP,"file open failed[%s] [%d/%s]\n",cfg->files[i],errno,strerror(errno));
            }
        }
    }
    
    return retval;
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
    int retval=0;
    
    // set default app configuration
    app_cfg_t cfg = {0,0,NULL,OF_SOUT,NULL};
    app_cfg_t *pcfg = &cfg;
    
    if (argc<2) {
        s_show_help();
    }else{
        // parse command line args (update config)
        parse_args(argc, argv, pcfg);
        s_process_file(pcfg);
    }

    free(pcfg->csv_path);
    return retval;
}
// End function main

