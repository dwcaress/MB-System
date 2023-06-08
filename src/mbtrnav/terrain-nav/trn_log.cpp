#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>

#include <stdlib.h>           // For atoi()
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include "trn_log.h"

// The TRN log file isn't created until a connection is made,
// since the TRN caller passes in log file naming information.
// The TRN log prints messages to the console, but also
// writes messages into a ring buffer until the log file is
// created. When the file created, the buffered messages are written
// to the log first, and ring buffer is no longer used.

// Global module level debugging flags
// by default, modules are configured to write
// to the log only.
//
// !!! IMPORTANT !!!
// Global module logging settings are indexed using TLModuleID:
// Any changes must be reflected in BOTH
// - tl_modules (in trn_log.c)
// - TLModuleID (in trn_log.h)
static TLModule tl_module_config[TL_N_MODULES]={
    {TL_LOG,TL_NONE}, // TL_TRN_SERVER
    {TL_LOG,TL_NONE}, // TL_TRN_LCM_APP
    {TL_LOG,TL_NONE}, // TL_STRUCT_DEFS
    {TL_LOG,TL_NONE}, // TL_TERRAIN_NAV
    {TL_LOG,TL_NONE}, // TL_TERRAIN_NAV_AID_LOG
    {TL_LOG,TL_NONE}, // TL_TNAV_CONFIG
    {TL_LOG,TL_NONE}, // TL_TNAV_PARTICLE_FILTER
    {TL_LOG,TL_NONE}, // TL_TNAV_FILTER
    {TL_LOG,TL_NONE}, // TL_TNAV_POINT_MASS_FILTER
    {TL_LOG,TL_NONE}, // TL_TNAV_SIGMA_POINT_FILTER
    {TL_LOG,TL_NONE}, // TL_TNAV_EXT_KALMAN_FILTER
    {TL_LOG,TL_NONE}, // TL_MATRIX_ARRAY_CALCS
    {TL_LOG,TL_NONE}, // TL_TERRAIN_MAP
    {TL_LOG,TL_NONE}, // TL_TERRAIN_MAP_DEM
    {TL_LOG,TL_NONE}, // TL_TEST_TRN_LOG
    {TL_LOG,TL_NONE}  // TL_TNAV_BANK_FILTER
};
TLModule *tl_modules=&tl_module_config[0];

// ring buffer
static char ringbuf[TL_RING_BYTES];
// ring buffer input pointer
static char *p_ringbuf=ringbuf;
// temporary buffer
static char temp[TL_RING_BYTES];
// TRN log file
static FILE* tlog=NULL;

void tl_mconfig(TLModuleID id, TLStreams s_en, TLStreams s_di){
    
    if (id>=0 && id<TL_N_MODULES) {
        switch (s_en) {
            case TL_NONE:
            case TL_SOUT:
            case TL_SERR:
            case TL_LOG:
            case TL_BOTH:
            case TL_BOTHB:
            case TL_ALL:
//                fprintf(stderr,"tl_configure: enabling [%x]\n",s_en);
                tl_modules[id].g_en=s_en;
                break;
                
            default:
                // no change
//                fprintf(stderr,"tl_configure: NC or invalid enable [%x]\n",s_en);
               break;
        }
        switch (s_di) {
            case TL_NONE:
            case TL_SOUT:
            case TL_SERR:
            case TL_LOG:
            case TL_BOTH:
            case TL_BOTHB:
            case TL_ALL:
//                fprintf(stderr,"tl_configure: disabling [%x]\n",s_di);
                tl_modules[id].g_di=s_di;
                break;
                
            default:
               //  no change
//                fprintf(stderr,"tl_configure: NC or invalid disable [%x]\n",s_di);
                break;
        }
    }else{
        fprintf(stderr,"tl_configure: invalid module ID [%d]\n",id);
    }
}

void trn_log(const char* log_msg) {

    if(log_msg==NULL){
        return;
    }

    char t_str[64]={0};
    time_t t=0;
    struct tm gt;
    
    // get time and format timestamp
    time(&t);
    gmtime_r(&t,&gt);
    strftime(t_str,64,"%FT%H:%M:%SZ",&gt);

    // append to timestamp string:
    // "* " for ring buffered messages
    // "- " for regular log messages
    strcat(t_str,(tlog==NULL ? "* " : "- "));
    
    // If the log file exists...
    if (tlog != NULL) {
        // If the ring buffer isn't empty,
        // write it to the log file first.
        if(p_ringbuf>ringbuf){
            char *op=ringbuf;
            while (op<p_ringbuf) {
                fputc(*op++, tlog);
            }
            
            // empty/reset the ringbuffer
            memset(ringbuf,0,TL_RING_BYTES);
            p_ringbuf=ringbuf;
        }
        // write the message and timestamp
        // to the log file
        fputs(t_str, tlog);
        fputs(log_msg, tlog);
        if (log_msg[strlen(log_msg)-1]!='\n') {
            fputs("\n", tlog);
        }
        // flush log buffer every write
        fflush(tlog);
        
    }else{ // log file doesn't exist...
        
        // get msg length (including NULL)
        int len=strlen(log_msg)+1;
        char term=( (log_msg[len-2]=='\n') ? 0x00 : '\n');
        int ts_len=strlen(t_str);
        // write len=msg length (w/ NULL)+newline (maybe)
        int wlen=len+ts_len+(term==0x00 ? 0 : 1);
        // space remaining
        int rem=ringbuf+TL_RING_BYTES-p_ringbuf;
        int wbytes=0;
        
		// compare message size with space remaining
        if ( wlen > rem ) {
            // if the message is larger than the ring buffer...
            if ( wlen > TL_RING_BYTES) {
                fprintf(stderr,"WARN: message larger than ring buffer - truncating\n");
                // truncate the message (set write len to ringbuf size)
                wlen=TL_RING_BYTES;
                // set input pointer to start of buffer
                p_ringbuf=ringbuf;
            }else{
                // wlen fits in buffer, but not in the remaining space...
                // clear the buffer, and overwrite
                fprintf(stderr,"WARN: overwriting start of ring buffer\n");
                memset(ringbuf,0,TL_RING_BYTES);
                p_ringbuf=ringbuf;
            }
        } // else message will fit in remaining space...

        // write to ring buffer
        wbytes=snprintf(p_ringbuf,wlen,"%s%s%c",t_str,log_msg,term);
        //fprintf(stderr,"## len[%d] ts_len[%d] rem[%d] wlen[%d] term[%x] wbytes[%d] ios[%d]\n",len,ts_len,rem,wlen,term,wbytes,(p_ringbuf-ringbuf));

        // update ring buffer input pointer
        p_ringbuf+=(wbytes>0 ? wbytes : 0);
        p_ringbuf-=(term=='\0' ? 1 : 0);
    }
    // check input pointer
    if (p_ringbuf >= (ringbuf+TL_RING_BYTES)) {
        p_ringbuf=ringbuf;
    }
}

// Create a new log file each time a client connects
// by appending 4-digit number to "trn.log"
//
void tl_new_logfile(const char* directory)
{

    if(directory != NULL){
        char buf[200]={0};
       const char *fname=(directory[strlen(directory)-1]=='/' ? "trn.log" : "/trn.log" );
        
        for (int i = 0; true; i++)
        {
            snprintf(buf, 200, "%s%s.%04d", directory,fname, i);
            if (0 > access(buf, F_OK)) break;
        }
        
        // Create a new log file and write a header
        //
        if (tlog)
        {
            fclose(tlog);
            tlog = NULL;
        }
        
        fprintf(stderr,"Opening log %s\n", buf);
        tlog = fopen(buf, "a");
        // write message to log (which will trigger ring buffer
        // contents to be written)
        logs((TL_BOTH),"File created[%s] returned[%p]\n",buf,tlog);
    }else{
        fprintf(stderr,"tl_new_logfile: directory is NULL\n");
	}
}

void tl_release(){
    if (NULL!=tlog) {
        fclose(tlog);
    }
    tlog=NULL;
}

void logs(int strmask, const char* format, ...) {
    //get the arguments
    va_list args;
    va_list cargs;
    va_start(args, format);
    
    char term=( (format!=NULL && format[strlen(format)-1]=='\n') ? 0x00 : '\n');
//    fprintf(stderr,"mask[%x]&TL_LOG[%x]\n",strmask,(strmask&TL_LOG));
//    fprintf(stderr,"mask[%x]&TL_SERR[%x]\n",strmask,(strmask&TL_SERR));
//    fprintf(stderr,"mask[%x]&TL_SOUT[%x]\n",strmask,(strmask&TL_SOUT));
    if( (strmask&TL_LOG) !=0 ){
        // clear output buffer
        memset(temp,0,TL_RING_BYTES);
        // print message to buffer
        va_copy(cargs,args);
        vsnprintf(temp, TL_RING_BYTES, format, cargs);
        trn_log(temp);
        // klh : fix cppcheck va_copy/va_start error
        // may have previously been omitted for QNX
        va_end(cargs);
    }
    
    // check tlog!=NULL and send to stderr
    //  when log file doesn't exist yet
    if( (tlog==NULL) || (strmask&TL_SERR) !=0 ){
        va_copy(cargs,args);
        vfprintf(stderr,format, cargs);
        if (term!=0x00) {
            fprintf(stderr,"\n");
        }
        // klh : fix cppcheck va_copy/va_start error
        // may have previously been omitted for QNX
        va_end(cargs);
    }
    if( (strmask&TL_SOUT) !=0 ){
        va_copy(cargs,args);
        vprintf(format, cargs);
        if (term!=0x00) {
            fprintf(stdout,"\n");
        }
        // klh : fix cppcheck va_copy/va_start error
        // may have previously been omitted for QNX
        va_end(cargs);
    }
    
   va_end(args);
}
