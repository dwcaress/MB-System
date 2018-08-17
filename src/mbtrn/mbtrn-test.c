///
/// @file mbtrn-test.c
/// @authors k. Headley
/// @date 06 nov 2012
///
/// MBTRN unit tests
///
/// @sa doxygen-examples.c for more examples of Doxygen markup


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
#include <string.h>
#include "mbtrn.h"
#include "mbtrn-server.h"
#include "r7kc.h"
#include "iowrap.h"
#include "cbuffer.h"
#include "mconfig.h"
#include "mlog.h"
#include "mlist.h"
#include "mdebug.h"

/////////////////////////
// Macros
/////////////////////////

/// @def TEST_HOST
/// @brief default host (TODO: remove this)
#define TEST_HOST "134.89.13.49"
//#define TEST_HOST "192.168.1.90"
/// @def MAX_TESTS
/// @brief number of unit tests
#define MAX_TESTS 16
/// @def TINDENT
/// @brief test output indent spaces
#define TINDENT 32
/// @def TREPORT(i)
/// @brief formatted report output
#define TREPORT(i) fprintf(stderr,"%s%*c[%3s][%3d]\n",tnames[i],(int)(TINDENT-strlen(tnames[i])),' ',(tstatus[i]==tsuccess[i]?"OK":"ERR"),tstatus[i]);
/// @def TCLEAR
/// @brief clear (zero) test structures
#define TCLEAR() { \
memset(tstatus,0,MAX_TESTS*sizeof(int)); \
memset(tsuccess,0,MAX_TESTS*sizeof(int)); \
memset(tnames,0,MAX_TESTS*sizeof(char *)); \
}while(0)

/////////////////////////
// Declarations
/////////////////////////

static int  tstatus[MAX_TESTS]={0};
static int tsuccess[MAX_TESTS]={0};
static char *tnames[MAX_TESTS]={0};

/////////////////////////
// Imports
/////////////////////////

/////////////////////////
// Module Global Variables
/////////////////////////

/////////////////////////
// Function Definitions
/////////////////////////


/// @fn int s_bad_test()
/// @brief bad test.
/// @return -1 always
static int s_bad_test()
{
    return -1;
}
// End function s_bad_test


/// @def SZ_1K
/// @brief 1Kbyte
#define SZ_1K 1024
/// @fn int s_test_mbtrn(const char * host, const char * file)
/// @brief test components used with MB System.
/// @param[in] host reson host address
/// @param[in] file input file (may be NULL, not implemented)
/// @return 0 on success, -1 otherwise
static int s_test_mbtrn(const char *host,const char *file)
{
    int retval=0;
    
    uint32_t nsubs=11;
    uint32_t subs[]={1003, 1006, 1008, 1010, 1012, 1013, 1015,
        1016, 7000, 7004, 7027};
    
    // initialize reader
    // create and open socket connection
    uint32_t reader_size = 100*SZ_1K;//10*R7K_MAX_RECORD_BYTES
    
    
    module_debug_config_t mcfg[]={
        {MBTRN,MDL_DEBUG}
    };
    mcfg_configure(mcfg,1);
   // mcfg_configure(NULL,0);
    
    mbtrn_reader_t *reader = mbtrn_reader_new(host,R7K_7KCENTER_PORT,reader_size, subs, nsubs);
 
    
    // show reader config
    mbtrn_reader_show(reader,true, 5);
    
    uint32_t read_bytes=0;
    byte buf[MBTRN_TRN_PING_BYTES]={0};


    // flush socket
    int32_t flush_retries=0;//MBTRN_FLUSH_RETRIES
    MDEBUG("flushing [optional retries[%d]]\n",flush_retries);
    mbtrn_reader_flush(reader,60000,flush_retries,500);
    usleep(MBTRN_PING_INTERVAL_USEC);
    
    // get a set of data record frames
    // here, separate poll, parse, enumeration and raw buffer reads
    MDEBUG("polling\n");
    if ( (read_bytes=mbtrn_reader_poll(reader, buf, MBTRN_TRN_PING_BYTES, 350)) > 0){
        MDEBUG("parsing\n");
        if(mbtrn_reader_parse(reader,buf,read_bytes,NULL)>0){
            MDEBUG("enumerating frames\n");
            // enumerate over the frames, show them
            r7k_drf_t *drfe = mbtrn_reader_enumerate(reader);
            int i=1;
            while (drfe!=NULL) {
                MDEBUG("\n\nframe [%d/%d]\n",i++,mbtrn_reader_frames(reader));
                r7k_drf_show(drfe,false,5);
                drfe = mbtrn_reader_next(reader);
            }
            
            MDEBUG("read raw\n");
            // alternatively, read the raw buffer
            int32_t ofs = mbtrn_reader_seek(reader, 0);
            uint32_t test=0;
            while ( (test=mbtrn_reader_read(reader,buf,SZ_1K)) > 0) {
                fprintf(stderr,"offset [%d]\n",ofs);
                // shows bytes, formatted in 16 columns w/ offsets
                r7k_hex_show(buf,test,16,true,5);
                // skip ahead
                ofs+=2048;
                if(mbtrn_reader_seek(reader,ofs)<0 || ofs>10240){
                    break;
                }
                memset(buf,0,SZ_1K);
            }
        }else{
            MDEBUG("parse err\n");
        }
    }

    // here, use reader_xread(), which automatically
    // refills the frame buffer when it becomes empty
    uint32_t len = 30*SZ_1K;
    uint32_t tmout = 350;
    int cycles = 25;
    int rstat=0;
    
//    mbtrn_reader_flush(reader,1024,MBTRN_FLUSH_RETRIES,500);
    mbtrn_reader_flush(reader,1024,0,500);
    int istat=0;
    for (int i=0; i<cycles; i++) {
        MDEBUG("calling xread\n");
//        if( (istat = mbtrn_reader_xread(reader,buf,len,tmout,MBR_ALLOW_PARTIAL)) > 0){
        if( (istat = mbtrn_reader_xread(reader,buf,len,tmout,MBR_BLOCK,0)) > 0){
            MDEBUG("xread %d/%d OK  [%d] - returned [%d/%d]\n",i+1,cycles,rstat,istat,len);
          //  MDEBUG("size/length/pending %u/%u/%u\n",r7k_drfcon_size(reader->fc),r7k_drfcon_length(reader->fc),r7k_drfcon_pending(reader->fc));
         }else{
            MERROR("xread %d/%d ERR [%d] - returned [%d/%d]\n",i+1,cycles,rstat,istat,len);
// MDEBUG("size/length/pending %u/%u/%u\n",r7k_drfcon_size(reader->fc),r7k_drfcon_length(reader->fc),r7k_drfcon_pending(reader->fc));
       }
        r7k_drfcon_show(reader->fc,false,5);
        MDEBUG("xread - done\n\n");
    }

    mbtrn_reader_destroy(&reader);

    return retval;
}
// End function s_test_mbtrn


/// @fn int test(const char * host, char * dfile)
/// @brief run unit tests for libmbtrn.
/// @param[in] host reson host address
/// @param[in] dfile reson data file (not implemented
/// @return 0 on success, -1 otherwise
int test(const char *host,char *dfile)
{
    int si=0;
    
    TCLEAR();
    
    tnames[si]="mdebug_test";
    tsuccess[si]=0;
    tstatus[si++]=mdb_test();

    tnames[si]="bad_test";
    tsuccess[si]=0;
    tstatus[si++]=s_bad_test();
//
//    tnames[si]="mbtrn_net";
//    tsuccess[si]=0;
//    tstatus[si++]=iow_test();
// 
//    iow_file_t *mb_data    = iow_file_new(dfile);
//    iow_socket_t *svr_sock = iow_socket_new("localhost",R7K_7KCENTER_PORT, ST_TCP);
//    mbtrn_server_t *test_svr    = mbtrn_server_new(svr_sock, mb_data);
//    
//    MDEBUG("\nstarting test server [r7k test]\n");
//    mbtrn_server_start(test_svr);
//    tnames[si]="test_r7k";
//    tsuccess[si]=0;
//    tstatus[si++]=r7k_test();
//    mbtrn_server_stop(test_svr);
//    mbtrn_server_destroy(&test_svr);
//
//    tnames[si]="test_cbuffer";
//    tsuccess[si]=0;
//    tstatus[si++]=cbuf_test();
//
    tnames[si]="test_mbtrn";
    tsuccess[si]=0;
    tstatus[si++]=s_test_mbtrn(host,dfile);

    tnames[si]="mlog_test";
    tsuccess[si]=0;
    tstatus[si++]=mlog_test();

    tnames[si]="mlist_test";
    tsuccess[si]=0;
    tstatus[si++]=mlist_test();

    for (int j=0;j<(si<=MAX_TESTS?si:MAX_TESTS);j++){
        TREPORT(j);
    }
    return 0;
}
// End function test


/// @fn int main(int argc, char ** argv)
/// @brief libmbtrn unit test entry point.
/// TODO: pass in test selection args
/// TODO: remove defaults for files and IP addresses
/// @param[in] argc TBD
/// @param[in] argv TBD
/// @return TBD
int main(int argc, char **argv)
{
    
    char dfile[]="dat/20160721_233529.s7k";
    const char *host=NULL;
    
    if (argc>=1) {
        host=argv[1];
    }
    host = (host==NULL?TEST_HOST:host);
    
    return test(host,dfile);
}
// End function main
