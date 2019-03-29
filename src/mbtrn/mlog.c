///
/// @file mlog.c
/// @authors k. Headley
/// @date 01 jan 2018
 
/// General purpose application message logging
/// configurable segmentation and rotation
/// enables formatted and timestamped output

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
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#ifdef _WIN32
#	include "mlog.h"
#	include "windirent.h"
#else
#	include <dirent.h>
#	include "mlog.h"
#endif

#include "mdebug.h"

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


/////////////////////////
// Declarations 
/////////////////////////

/// @struct mlog_list_entry_s
/// @brief log list entry structure decl
struct mlog_list_entry_s;
/// @typedef struct mlog_list_entry_s mlog_list_entry_t
/// @brief log list entry structure type
typedef struct mlog_list_entry_s mlog_list_entry_t;
/// @struct mlog_list_entry_s
/// @brief log list entry structure definition
struct mlog_list_entry_s{
    /// @var mlog_list_entry_s::log
    /// @brief mlog_t reference
    mlog_t *log;
    /// @var mlog_list_entry_s::id
    /// @brief log ID
    mlog_id_t id;
    /// @var mlog_list_entry_s::name
    /// @brief log name
    char *name;
    /// @var mlog_list_entry_s::next
    /// @brief next entry in list
    mlog_list_entry_t *next;
};

static void s_mlog_list_entry_destroy(mlog_list_entry_t **pself);
static mlog_t *s_lookup_log(int32_t id);
static int s_parse_path(const char *src, mlog_t *dest);
static int16_t s_path_segno(char *file_path, char *name, char *fmt);
static int s_get_log_info(mlog_info_t *dest, char *path, char *name, char *fmt, iow_file_t *file);
static int s_log_rotate(mlog_t *self);
static int s_log_chklimits(mlog_t *self);
static int s_log_set_seg(mlog_t *self, int16_t segno);
static void s_init_log(mlog_t *self);
static char *s_seg_path(const char *file_path, mlog_t *self, uint16_t segno);

/////////////////////////
// Imports
/////////////////////////

/////////////////////////
// Module Global Variables
/////////////////////////

static mlog_list_entry_t *s_log_list=NULL;
// TODO; initialize this somewhere
static iow_mutex_t *mlog_list_mutex = NULL;


/////////////////////////
// Function Definitions
/////////////////////////

/// @fn void s_mlog_list_entry_destroy(mlog_list_entry_t ** pself)
/// @brief release log list entry resources.
/// @param[in] pself pointer to list reference head
/// @return none
static void s_mlog_list_entry_destroy(mlog_list_entry_t **pself)
{
    if (NULL!=pself) {
        mlog_list_entry_t *self = *pself;
        if (self) {
            if (self->name) {
                free(self->name);
            }
            free(self);
            *pself=NULL;
        }
    }
}
// End function s_mlog_list_entry_destroy


/// @fn void s_mlog_list_destroy(_Bool incl_logs)
/// @brief release mlog list resources.
/// @param[in] incl_logs release log resources if true
/// @return none
static void s_mlog_list_destroy(bool incl_logs)
{
    if (NULL != s_log_list) {
        mlog_list_entry_t *plist = s_log_list;
        mlog_list_entry_t *pafter = plist->next;
        do{
            if (incl_logs) {
                mlog_destroy(&plist->log);
            }
//            MDEBUG("releasing %s next[%p]\n",plist->name,plist->next);
            pafter = plist->next;
            s_mlog_list_entry_destroy(&plist);
            plist = pafter;
        }while (NULL != pafter);
    }
}
// End function s_mlog_list_destroy


/// @fn mlog_t * s_lookup_log(int32_t id)
/// @brief look up log by ID.
/// @param[in] id log ID
/// @return mlog_t reference on success, NULL otherwise
static mlog_t *s_lookup_log(int32_t id)
{
    mlog_t *retval = NULL;
    
    mlog_list_entry_t *plist = s_log_list;
    if (NULL!=plist) {
        do{
            if (plist->id == id) {
                retval = plist->log;
                break;
            }
            plist=plist->next;
        }while (NULL != plist);
    }
    return retval;
}
// End function s_lookup_log


/// @fn int s_parse_path(const char * src, mlog_t * dest)
/// @brief parse log name path string into mlog_t structure.
/// @param[in] src log name path string (e.g. foo.out, not including segment information, i.e. foo0000.out)
/// @param[in] dest mlog_t reference
/// @return 0 on success, -1 otherwise
static int s_parse_path(const char *src, mlog_t *dest)
{
    int retval = -1;

    if (NULL!=src && NULL!=dest && strcmp(src,".")!=0) {

       
        char *scopy = strdup(src);
        char *ps = scopy;
        char *pend = scopy+strlen(scopy);
        
        while ( (*ps=='\t' || *ps==' ') && ps<pend) {
            ps++;
        }
        while ( (*ps==ML_SYS_PATH_DEL) && ps<pend) {
            ps++;
        }
        if ( ps>scopy && ((*(ps-1))==ML_SYS_PATH_DEL) ) {
            ps--;
        }
        
        char *pathe  = strrchr(ps, ML_SYS_PATH_DEL);
        char *exte  = strrchr(ps, ML_SYS_EXT_DEL);
        char *ppath = NULL;
        char *pname = NULL;
        char *pext = NULL;
        
        if (pathe!=NULL) {
            pname = pathe+1;
            ppath = ps;
            *pathe='\0';
        }else{
            // path delimiter not found
            ppath = NULL;
            pname = ps;
        }
        
        if (exte!=NULL) {
            if (exte>pname) {
                // name and ext
                pext = ((exte+1)<pend ? (exte+1) : NULL );
                if (*(exte+1)!='\0') {
                    *exte = '\0';
                }
            }else if(exte==pname){
				// name starts with ext delimiter or
                // path may contain ext delimiter
                pext = NULL;
            }
        }else{
            // ext delimiter not found
            pext = NULL;
        }
        
        if (pname != NULL && strlen(pname)>=1) {
            if (NULL != dest->name) {
                free(dest->name);
            }
            dest->name = strdup(pname);
        }
        if (ppath != NULL) {
            if (NULL != dest->path) {
                free(dest->path);
            }
            if (ppath==pathe) {
                dest->path=(char *)malloc(2*sizeof(char));
                sprintf(dest->path,"%c",ML_SYS_PATH_DEL);
            }else{
                if (strcmp(ppath,".")==0) {
                    dest->path = strdup("./");
                }else{
                	dest->path = strdup(ppath);
                }
            }
        }
        if (pext != NULL && strlen(pext)>=1) {
            if (NULL != dest->ext) {
                free(dest->ext);
            }
           dest->ext = strdup(pext);
        }
        
        //MDEBUG("src[%s] path[%p] name[%p] ext[%p]\n",src,ppath,pname,pext);
//        MDEBUG("src[%10s] path[%10s] name[%10s] ext[%10s]\n",src,dest->path,dest->name,dest->ext);
        free(scopy);
    }
    return retval;
}
// End function s_parse_path


/// @fn int16_t s_path_segno(char * file_path, char * name, char * fmt)
/// @brief parse path segment number as int16_t using specified basename and parse format.
/// @param[in] file_path filename path
/// @param[in] name file name (e.g. foo,/x/y/foo)
/// @param[in] fmt segment number parse format (e.g. "%04hd")
/// @return segment number (as int16_t)
static int16_t s_path_segno(char *file_path, char *name, char *fmt)
{
    int16_t retval=-1;
    if (NULL!=file_path && NULL!=fmt && NULL!=name) {
        char *np = NULL, *xp=NULL;
        // point to start of name
        if ( (xp=strrchr(file_path, ML_SYS_PATH_DEL))!=NULL) {
            // just after last path delimiter (if any)
            xp++;
        }else{
            // otherwise point to beginning of file_path
            xp=file_path;
        }
        if ((np=strstr(xp,name))!=0) {
            np+=strlen(name);
            int16_t x=-1;
//            MDEBUG("fp[%s] fmt[%s] name[%s] np[%s]\n",file_path,fmt,name,np);
            if( sscanf(np,fmt,&x)==1 && x>=0 && x<=ML_MAX_SEG){
                retval=x;
            }
        }
    }
    return retval;
}
// End function s_path_segno


/// @fn int s_get_log_info(mlog_info_t * dest, char * path, char * name, char * fmt, iow_file_t * file)
/// @brief fill mlog_info_t structure using specified log file info.
/// @param[in] dest mlog_info_t reference
/// @param[in] path log file path
/// @param[in] name log file name
/// @param[in] fmt segment number parse format (or NULL to use ML_SEG_FMT)
/// @param[in] file log file reference
/// @return 0 on success, -1 otherwise
static int s_get_log_info(mlog_info_t *dest, char *path, char *name, char *fmt, iow_file_t *file)
{
    int retval=-1;
    if (NULL!=dest && NULL!=name && NULL!=file) {
        DIR *dp;
        struct dirent *ep;
        time_t tseg=0;
        int16_t nseg=-1;
        
        memset(dest,0,sizeof(mlog_info_t));
        dest->seg_min=0xFFFF;
        dest->tb=time(NULL);
        
        iow_file_show(file,false,5);
        
        dp = opendir ((path==NULL?".":path));
        if (dp != NULL){
            while ( (ep = readdir(dp))!=NULL ){
                nseg=s_path_segno(ep->d_name,name,(fmt!=NULL?fmt:ML_SEG_FMT));
                // if it's a segment
                if ( nseg >= 0) {
                    retval=0;
                    dest->seg_count++;
                    
                    if (nseg > dest->seg_max) {
                        dest->seg_max = nseg;
                    }
                    if(nseg < dest->seg_min){
                        dest->seg_min = nseg;
                    }
                    
                    tseg=iow_mtime(ep->d_name);
//                    MDEBUG("tseg[%ld] nseg[%hu] tb[%ld] te[%ld]\n",tseg,nseg,dest->tb,dest->te);
                    if( tseg>0){
                        if (tseg > dest->te) {
                            dest->seg_e = nseg;
                            dest->te = tseg;
                        }
                        if (tseg < dest->tb) {
                            dest->seg_b = nseg;
                            dest->tb = tseg;
                        }
                    }
                }
            }
            (void) closedir (dp);
        }else{
            MERROR ("Couldn't open the directory\n");
        }
    }else{
        MERROR ("invalid argument\n");
    }
    return retval;
}
// End function s_get_log_info



/// @fn int s_log_rotate(mlog_t * self)
/// @brief rotate log file.
/// @param[in] self mlog_t reference
/// @return 0 on success, -1 otherwise
static int s_log_rotate(mlog_t *self)
{
    int retval=-1;
    
    // do any rotation required
    if (NULL!=self && NULL!=self->file) {
        mlog_flags_t flags=self->cfg->flags;
        
        if( flags&ML_OSEG ){
            // segmented
            mlog_info_t linfo={0};
            s_get_log_info(&linfo, self->path,self->name,ML_SEG_FMT, self->file);
            
            
            if ( self->cur_seg < (self->cfg->lim_s-1)) {
                // create new segment
                if(s_log_set_seg(self, self->cur_seg+1) > 0){
                    iow_ftruncate(self->file,0);
                    self->seg_len=0;
                    self->cur_seg++;
                    self->seg_count=linfo.seg_count;
                    self->stime=time(NULL);
                }else{
                    MERROR("s_log_set_seg failed\n");
                }
            }else{
                if(s_log_set_seg(self, 0) > 0){
                    iow_ftruncate(self->file,0);
                    self->cur_seg=0;
                    self->seg_len=0;
                    self->seg_count=linfo.seg_count;
                    self->stime=time(NULL);
                }else{
                    MERROR("s_log_set_seg failed\n");
                }
            }
        }else{
            // unsegmented
            if( flags&ML_OVWR ){
                // if overwrite enabled
                // overwrite current segment
                iow_ftruncate(self->file,0);
                self->seg_len=0;
            }
        }
        
    }else{
        MERROR("invalid argument\n");
    }
    
    return retval;
}
// End function s_log_rotate


// return 0 if any rotation limits reached

/// @fn int s_log_chklimits(mlog_t * self)
/// @brief return 0 if any log rotation condition limits reached.
/// @param[in] self mlog reference
/// @return 0 if ready for rotation, -1 otherwise
static int s_log_chklimits(mlog_t *self)
{
    int retval=-1;
    
    if (NULL!=self && NULL!=self->file) {
        mlog_flags_t flags=self->cfg->flags;
        mlog_dest_t dest = self->cfg->dest;
        // check limit flags
//        MDEBUG("flags[%0x] dest[%0x]\n",flags,dest);
        if ( flags==ML_MONO || flags&ML_DIS || (dest&ML_FILE)==0 ) {
            // - monolithic - no limits
            // - no destination defined
            // - is disabled
//            MDEBUG("no limit/disabled/no file\n");
        }else{

            if ( (self->cfg->flags&ML_LIMLEN) && (self->cfg->lim_b > 0) ) {
                if ((self->seg_len > self->cfg->lim_b)) {
                    MDEBUG("do rotate (lim_b)\n");
                    retval=0;
                }
            }
            if ( (self->cfg->flags&ML_LIMTIME) && (self->cfg->lim_t > 0) ) {
//                MDEBUG("do rotate (lim_t)\n");
                time_t now = time(NULL);
                if ( now-self->stime > self->cfg->lim_t) {
                    MTRACE();
                    retval=0;
                }
            }
        }
    }else{
        MERROR("invalid argument\n");
    }
    
    return retval;
}
// End function s_log_chklimits


/// @fn int s_log_set_seg(mlog_t * self, int16_t segno)
/// @brief set new log segment as active. creates new segment if needed.
/// @param[in] self mlog reference
/// @param[in] segno new segment number
/// @return 0 on success, -1 otherwise
static int s_log_set_seg(mlog_t *self, int16_t segno)
{
    int retval=-1;
    
    if (NULL!=self && NULL!=self->file && NULL!=self->name){
        
        size_t len = strlen(self->name);
        len += ( NULL==self->path ? 0 : strlen(self->path));
        // ext
        len += ( NULL==self->ext ? 0 : strlen(self->ext));
		// for segno
        len += ML_MAX_SEG_WIDTH;
		// for delimiter(s), NULL
        len += 2;
        // init buffer
        char new_name[1024];		// char new_name[len];  is not valid C. JL 
        memset(new_name,0,len);
        char *bp = new_name;
//        MDEBUG("path[%s] name[%s] ext[%s] len[%d]\n",self->path,self->name,self->ext,len);

        // format new file path string
        if (NULL!=self->path) {
            sprintf(bp,"%s",self->path);
            bp = new_name+strlen(new_name);
        }
        sprintf(bp,"%s",self->name);
        bp = new_name+strlen(new_name);
        sprintf(bp,ML_SEG_FMT,segno);
        bp = new_name+strlen(new_name);
        if (NULL!=self->ext) {
            sprintf(bp,"%c%s",ML_SYS_EXT_DEL,self->ext);
            bp = new_name+strlen(new_name);
        }
//        MDEBUG("new_name[%s] len[%d]\n",new_name,strlen(new_name));
        if( (retval = iow_rename(self->file,(const char *)new_name))>0){
//            MDEBUG("rename OK [%d]\n",retval);
        }else{
//            MDEBUG("rename failed [%d]\n",retval);
        }
        
    }

    return retval;
}
// End function s_log_set_seg


// initialize log segment state
/// @fn void s_init_log(mlog_t * self)
/// @brief initialize mlog.
/// @param[in] self mlog reference
/// @return none
static void s_init_log(mlog_t *self)
{
    if (NULL!=self && NULL!=self->file &&
        NULL!=self->cfg && NULL!=self->name) {
        // get latest and greatest segment
        
        mlog_info_t linfo={0};
        
        if(s_get_log_info(&linfo, self->path,self->name,ML_SEG_FMT, self->file)==0){
			
            if (linfo.seg_max == (self->cfg->lim_s-1)) {
//                MDEBUG("full segs, init to latest written [%d/%ld]\n",linfo.seg_e,linfo.te);
                // use most recently written segment
                
                // set latest segment [rename, close/reopen if open]
                s_log_set_seg(self,linfo.seg_e);
                self->cur_seg = linfo.seg_e;
                self->seg_len = iow_fsize(self->file);
                self->stime = time(NULL);
                // segments are zero indexed, so count is +1
                self->seg_count=linfo.seg_count;
            }else if (linfo.seg_max < self->cfg->lim_s) {
//                MDEBUG("init to last seg [%d/%d]\n",linfo.seg_max,self->cfg->lim_s);
                // not at segment limit, largest segment
                s_log_set_seg(self,linfo.seg_max);
                self->cur_seg = linfo.seg_max;
                self->seg_len = iow_fsize(self->file);
                self->stime = time(NULL);
                // segments are zero indexed, so count is +1
                self->seg_count=linfo.seg_count;
            }else if (linfo.seg_b >= 0) {
//                MDEBUG("init to latest written [%d/%ld]\n",linfo.seg_b,linfo.tb);
                // use most recently written segment
                
                // set latest segment [rename, close/reopen if open]
                s_log_set_seg(self,linfo.seg_b);
                self->cur_seg = linfo.seg_b;
                self->seg_len = iow_fsize(self->file);
                self->stime = time(NULL);
                // segments are zero indexed, so count is +1
                self->seg_count=linfo.seg_count;
                
            }else{
//                MDEBUG("init to first\n");
                // set current segment [rename, close/reopen if open]
                s_log_set_seg(self,0);
                iow_ftruncate(self->file,0);
                // if no log exists, init first segment
                self->cur_seg = 0;
                self->seg_len = 0;
                // segments are zero indexed, so count is +1
                self->seg_count=linfo.seg_count;
                self->stime = time(NULL);
            }
//            MDEBUG("check limits...\n");
            // if it's already full, rotate
            if (s_log_chklimits(self)==0) {
//                MDEBUG("rotating [%s]\n",self->file->path);
//                s_log_rotate(self);
//                MDEBUG("truncating [%s]\n",self->file->path);
                iow_ftruncate(self->file,0);
                self->seg_len = 0;
            }
//            else{
//                MDEBUG("no rotation needed\n");
//			}
        }
    }else{
        MERROR("invalid argument\n");
    }
}
// End function s_init_log

                
/// @fn char * s_seg_path(const char * file_path, mlog_t * self, uint16_t segno)
/// @brief return file segment path as new string. caller must free.
/// @param[in] file_path file path (w/o segments,
/// @param[in] self mlog reference
/// @param[in] segno segment number to use
/// @return new string containing full log segment path on success, NULL otherwise
static char *s_seg_path(const char *file_path, mlog_t *self, uint16_t segno)
{
    char *retval=NULL;
    if (NULL!=self && NULL!=file_path && segno<=ML_MAX_SEG) {

        s_parse_path(file_path,self);
        size_t len =strlen(file_path)+8;
        retval = (char *)malloc(len*sizeof(char));
        if (NULL!=retval) {
            memset(retval,0,len);
            char *op = retval;
            sprintf(op,"%s%s",(self->path==NULL?"":self->path),self->name);
            op=retval+strlen(retval);
            sprintf(op,ML_SEG_FMT,segno);
            op=retval+strlen(retval);
            sprintf(op,"%s%s",(self->ext==NULL?"":"."),(self->ext==NULL?"":self->ext));
        }
    }
    return retval;
}
// End function s_seg_path


/// @fn mlog_config_t * mlog_config_new(const char * tfmt, const char * del, mlog_flags_t flags, mlog_dest_t dest, int32_t lim_b, int32_t lim_s, int32_t lim_t)
/// @brief create new mlog configuration structure.
/// @param[in] tfmt time format string (i.e. strftime)
/// @param[in] del path delimiter string
/// @param[in] flags log attribute flags
/// @param[in] dest mlog reference
/// @param[in] lim_b segment limit (size, bytes)
/// @param[in] lim_s log limit (segments)
/// @param[in] lim_t segment limit (time, msec)
/// @return new mlog configuration reference on success, -1 otherwise
mlog_config_t *mlog_config_new(const char *tfmt,const char *del,
                               mlog_flags_t flags, mlog_dest_t dest,
                               int32_t lim_b, int32_t lim_s, int32_t lim_t)
{
    mlog_config_t *self = (mlog_config_t *)malloc(sizeof(mlog_config_t));
    if (NULL!=self) {
        memset(self,0,sizeof(mlog_config_t));
        self->tfmt   = (tfmt==NULL ? strdup(ML_DFL_TFMT) : strdup(tfmt));
        self->del    = (del==NULL ? strdup(ML_DFL_DEL) : strdup(del));
        self->lim_b  = lim_b;
        self->lim_s  = lim_s;
        self->lim_t  = lim_t;
        self->flags  = flags;
        self->dest   = dest;
    }
    return self;
}
// End function mlog_config_new


/// @fn void mlog_config_destroy(mlog_config_t ** pself)
/// @brief release mlog resources.
/// @param[in] pself pointer to instance reference
/// @return none
void mlog_config_destroy(mlog_config_t **pself)
{
    if (NULL!=pself) {
        mlog_config_t *self = *pself;
        if (self) {
            if (self->tfmt) {
                free(self->tfmt);
            }
            if (self->del) {
                free(self->del);
            }
            free(self);
            *pself=NULL;
        }
    }
}
// End function mlog_config_destroy


/// @fn void mlog_info_show(mlog_info_t * self, _Bool verbose, uint16_t indent)
/// @brief output mlog info structure parameters to stderr.
/// @param[in] self mlog_info_t structure reference
/// @param[in] verbose use verbose output
/// @param[in] indent output indentation spaces
/// @return none
void mlog_info_show(mlog_info_t *self, bool verbose, uint16_t indent)
{
    if (NULL != self) {
        fprintf(stderr,"%*s[self      %10p]\n",indent,(indent>0?" ":""), self);
        fprintf(stderr,"%*s[seg_count %10hu]\n",indent,(indent>0?" ":""), self->seg_count);
        fprintf(stderr,"%*s[seg_min   %10hu]\n",indent,(indent>0?" ":""), self->seg_min);
        fprintf(stderr,"%*s[seg_max   %10hu]\n",indent,(indent>0?" ":""), self->seg_max);
        fprintf(stderr,"%*s[seg_b     %10hu]\n",indent,(indent>0?" ":""), self->seg_b);
        fprintf(stderr,"%*s[seg_e     %10hu]\n",indent,(indent>0?" ":""), self->seg_e);
        fprintf(stderr,"%*s[tb        %10ld]\n",indent,(indent>0?" ":""), self->tb);
        fprintf(stderr,"%*s[te        %10ld]\n",indent,(indent>0?" ":""), self->te);
    }
}
// End function mlog_info_show


/// @fn void mlog_config_show(mlog_config_t * self, _Bool verbose, uint16_t indent)
/// @brief output mlog configuration structure parameters to stderr.
/// @param[in] self mlog_config_t structure reference
/// @param[in] verbose use verbose output
/// @param[in] indent output indentation spaces
/// @return none
void mlog_config_show(mlog_config_t *self, bool verbose, uint16_t indent)
{
    if (NULL != self) {
        fprintf(stderr,"%*s[self     %10p]\n",indent,(indent>0?" ":""), self);
        fprintf(stderr,"%*s[lim_b    %10u]\n",indent,(indent>0?" ":""), self->lim_b);
        fprintf(stderr,"%*s[lim_s    %10u]\n",indent,(indent>0?" ":""), self->lim_s);
        fprintf(stderr,"%*s[lim_t    %10ld]\n",indent,(indent>0?" ":""), self->lim_t);
        fprintf(stderr,"%*s[flags    %10X]\n",indent,(indent>0?" ":""), self->flags);
        fprintf(stderr,"%*s[dest     %10X]\n",indent,(indent>0?" ":""), self->dest);
        fprintf(stderr,"%*s[tfmt     %10s]\n",indent,(indent>0?" ":""), self->tfmt);
        fprintf(stderr,"%*s[del      %10s]\n",indent,(indent>0?" ":""), self->del);
    }
}
// End function mlog_config_show


/// @fn mlog_t * mlog_new(const char * file_path, mlog_config_t * config)
/// @brief create new mlog_t reference.
/// @param[in] file_path log file path (w/o segment info, e.g. /x/y/foo.out
/// @param[in] config mlog_config_t reference
/// @return new mlog_t reference on success, NULL otherwise
mlog_t *mlog_new(const char *file_path, mlog_config_t *config)
{
    mlog_t *self = (mlog_t *)malloc(sizeof(mlog_t));
    if (NULL!=self) {
        memset(self,0,sizeof(mlog_t));
        if (NULL!=config) {
            self->path = NULL;
            self->name = NULL;
            self->ext  = NULL;
            s_parse_path(file_path,self);

            self->stime = 0;
            
            
            self->file  = iow_file_new(NULL);
            char *tpath = s_seg_path(file_path,self,0);
            self->file  = iow_file_new(tpath);
            free(tpath);
            self->cfg = mlog_config_new(NULL,NULL,ML_MONO,ML_SERR,ML_NOLIMIT,ML_NOLIMIT,ML_NOLIMIT);
            // copy config if provided
            if (NULL != self->cfg) {
                // free allocated strings
                if (self->cfg->tfmt) {
                    free(self->cfg->tfmt);
                }
                if (self->cfg->del) {
                	free(self->cfg->del);
                }
                memcpy(self->cfg,config,sizeof(mlog_config_t));
                self->cfg->tfmt = (config->tfmt==NULL ? strdup(ML_DFL_TFMT)  : strdup(config->tfmt));
                self->cfg->del  = strdup(ML_DFL_DEL);
            }
            s_init_log(self);
       }
     }else{
        MERROR("malloc failed\n");
    }
    return self;
}
// End function mlog_new


/// @fn void mlog_destroy(mlog_t ** pself)
/// @brief release mlog_t resources.
/// @param[in] pself pointer to instance reference
/// @return none
void mlog_destroy(mlog_t **pself)
{
    if (NULL!=pself) {
        mlog_t *self = *pself;
        if (NULL!=self) {
            if ( NULL!=(self->cfg)){
                mlog_config_destroy(&self->cfg);
            }
            iow_file_destroy(&self->file);
            free(self->path);
            free(self->ext);
            free(self->name);
            free(self);
            *pself=NULL;
        }
    }
}
// End function mlog_destroy


/// @fn void mlog_release(_Bool incl_logs)
/// @brief release mlog list resources (and optionally log resources).
/// @param[in] incl_logs close logs in list and release resources if true
/// @return none
void mlog_release(bool incl_logs)
{
    s_mlog_list_destroy(incl_logs);
    iow_mutex_destroy(&mlog_list_mutex);
}
// End function mlog_release


/// @fn void mlog_show(mlog_t * self, _Bool verbose, uint16_t indent)
/// @brief output mlog structure parameters to stderr.
/// @param[in] self mlog_config_t structure reference
/// @param[in] verbose use verbose output
/// @param[in] indent output indentation spaces
/// @return none
void mlog_show(mlog_t *self, bool verbose, uint16_t indent)
{
    if (NULL != self) {
        fprintf(stderr,"%*s[self     %10p]\n",indent,(indent>0?" ":""), self);
        fprintf(stderr,"%*s[file     %10p]\n",indent,(indent>0?" ":""), self->file);
        if (verbose) {
           iow_file_show(self->file,verbose,indent+3);
        }
        fprintf(stderr,"%*s[path     %10s]\n",indent,(indent>0?" ":""), self->path);
        fprintf(stderr,"%*s[name     %10s]\n",indent,(indent>0?" ":""), self->name);
        fprintf(stderr,"%*s[ext      %10s]\n",indent,(indent>0?" ":""), self->ext);
        fprintf(stderr,"%*s[cfg      %10p]\n",indent,(indent>0?" ":""), self->cfg);
        if (verbose) {
            mlog_config_show(self->cfg,verbose,indent+3);
        }
        char *cp = ctime(&self->stime);
        cp[strlen(cp)-1]='\0';
        fprintf(stderr,"%*s[stime      %s]\n",indent,(indent>0?" ":""), cp);
        fprintf(stderr,"%*s[slen     %10u]\n",indent,(indent>0?" ":""),  self->seg_len);
        fprintf(stderr,"%*s[scount   %10hu]\n",indent,(indent>0?" ":""), self->seg_count);
        fprintf(stderr,"%*s[scur     %10hu]\n",indent,(indent>0?" ":""), self->cur_seg);
        
    }
}
// End function mlog_show


/// @fn int mlog_open(mlog_t * self, iow_flags_t flags, iow_mode_t mode)
/// @brief open mlog.
/// @param[in] self mlog reference
/// @param[in] flags file attribute flags
/// @param[in] mode file mode (permission) flags
/// @return 0 on success, -1 otherwise
int mlog_open(mlog_t *self, iow_flags_t flags, iow_mode_t mode)
{
    int retval = -1;
    
    if (NULL!=self && NULL!=self->file) {
        retval = iow_mopen(self->file,flags,mode);
    }
    
    return retval;
}
// End function mlog_open


/// @fn int mlog_close(mlog_t * self)
/// @brief close mlog.
/// @param[in] self mlog reference
/// @return 0 on success, -1 otherwise
int mlog_close(mlog_t *self)
{
    int retval = -1;
    
    if (NULL!=self && NULL!=self->file) {
        retval = iow_close(self->file);
    }
    
    return retval;
}
// End function mlog_close


/// @fn int mlog_add(mlog_t * self, int32_t id, char * name)
/// @brief add mlog to list. listed logs may be looked up by id.
/// @param[in] self mlog reference
/// @param[in] id log ID
/// @param[in] name log name (mnemonic)
/// @return 0 on success, -1 otherwise
int mlog_add(mlog_t *self, int32_t id, char *name)
{
    int retval=-1;
    mlog_list_entry_t *new_entry = (mlog_list_entry_t *)malloc(sizeof(mlog_list_entry_t));
    
    if (NULL != new_entry) {
        new_entry->log  = self;
        new_entry->id   = id;
        new_entry->name = strdup(name);
        new_entry->next = NULL;
        
        mlog_list_entry_t *plist = s_log_list;
        int list_len = 0;
        if (NULL == plist){
            // start the list if it doesn't exist
            s_log_list = new_entry;
            list_len++;
        }else{
            do {
                list_len++;
                // check for duplicate IDs
                if (plist->id == new_entry->id) {
                    MERROR("id already in list [%d/%s]\n",plist->id,plist->name);
                    break;
                }
                // add entry at the end
                if (plist->next == NULL) {
                    plist->next = new_entry;
                    list_len++;
                    MDEBUG("adding entry id[%d] name[%s] list len[%d]\n",new_entry->id,new_entry->name,list_len);
                    retval=0;
                    break;
                }
                // otherwise, increment pointer
                plist=plist->next;
            }while (NULL != plist);
        }
    }
    return retval;
}
// End function mlog_add

/// @fn int mlog_delete(mlog_id_t id)
/// @brief remove log from list. Does not change log or release log resources.
/// @param[in] id log ID to remove
/// @return 0 on success, -1 otherwise
int mlog_delete(mlog_id_t id)
{
    int retval=-1;
    
    mlog_list_entry_t *plist = s_log_list;
    if (NULL != plist){
        mlog_list_entry_t *pbefore=NULL;
        mlog_list_entry_t *pafter=plist->next;
        do {
            // find id
            if (plist->id == id) {
                if (NULL != pbefore) {
                    if (pafter == NULL) {
                        // at list tail
                        pbefore->next=NULL;
                        s_mlog_list_entry_destroy(&plist);
                    }else{
                        pbefore->next = pafter;
                        s_mlog_list_entry_destroy(&plist);
                    }
                }else{
                	// at list head
                    s_log_list = pafter;
                    s_mlog_list_entry_destroy(&plist);
                }
                break;
            }
            // otherwise, increment pointers
            pbefore = plist;
            plist   = plist->next;
            pafter  = plist->next;
        }while (NULL != plist->next);
        
    }// else list is NULL
   
    return retval;
}
// End function mlog_delete


/// @fn void mlog_set_dest(mlog_id_t id, mlog_dest_t dest)
/// @brief set (listed) log destination flags.
/// @param[in] id log ID
/// @param[in] dest desintation flags
/// @return none
void mlog_set_dest(mlog_id_t id, mlog_dest_t dest)
{
    mlog_t *log = NULL;
    if ( ( log = s_lookup_log(id) ) != NULL) {
        log->cfg->dest = dest;
    }
}
// End function mlog_set_dest


/// @fn mlog_dest_t mlog_get_dest(mlog_id_t id)
/// @brief get (listed) log destination flags.
/// @param[in] id log ID
/// @return log destination flags on success, or ML_NODEST otherwise
mlog_dest_t mlog_get_dest(mlog_id_t id)
{
    mlog_dest_t retval=ML_NODEST;
    mlog_t *log = s_lookup_log(id);
    if ( log != NULL) {
        retval = log->cfg->dest;
    }else{
        MERROR("invalid argument");
    }
    return retval;
}
// End function mlog_get_dest


/// @fn int mlog_flush(mlog_id_t id)
/// @brief flush listed log contents to file. behavior is platform dependent.
/// on posix systems, uses fsync, which may not write to disk until file is
/// closed.
/// @param[in] id log id
/// @return 0 on success, -1 otherwise.
int mlog_flush(mlog_id_t id)
{
    int retval = -1;
    mlog_t *log = s_lookup_log(id);
    
    if( NULL!=log && NULL!=log->file){
        retval = iow_flush(log->file);
    }else{
        MERROR("invalid argument");
    }
    return retval;
}
// End function mlog_flush


/// @fn int mlog_printf(mlog_id_t id, char * fmt,...)
/// @brief formatted print to log destination(s).
/// @param[in] id log ID
/// @param[in] fmt print format (e.g. stdio printf)
/// @return number of bytes written (to file only) on success, -1 otherwise
int mlog_printf(mlog_id_t id, char *fmt, ...)
{
    int retval = -1;

    mlog_t *log = s_lookup_log(id);
    
    if(log!=NULL){
        
        mlog_dest_t dest = log->cfg->dest;
        mlog_flags_t flags = log->cfg->flags;
        
        //get the arguments
        va_list args;
        va_list cargs;
        va_start(args, fmt);
        
        char term=( (fmt!=NULL && fmt[strlen(fmt)-1]=='\n') ? 0x00 : '\n');

//        fprintf(stderr,"dest[%x]&ML_FILE[%x]\n",dest,(dest&ML_FILE));
//        fprintf(stderr,"dest[%x]&ML_SERR[%x]\n",dest,(dest&ML_SERR));
//        fprintf(stderr,"dest[%x]&ML_SOUT[%x]\n",dest,(dest&ML_SOUT));
        
        if (dest&ML_FILE  && (flags&ML_DIS)==0 ) {
            // print message to buffer
            va_copy(cargs,args);
            int wbytes=vsnprintf(NULL,0,fmt,cargs);
            retval=wbytes;
			// check write size and rotate if it will overflow
            // [will allow writes > segment/log size]
            if ( (log->cfg->lim_b > 0) && ((log->seg_len+wbytes) > log->cfg->lim_b) ) {
                s_log_rotate(log);
            }
            va_copy(cargs,args);
            if( (wbytes=iow_vfprintf(log->file,fmt,cargs))>0){
                // add one for the null char
                log->seg_len+=wbytes+1;
            }
        }else{
            MERROR("file output disabled d[%0X] f[%0X]\n",dest,flags);
        }
        
        // send to stderr, stdout
        if( (dest&ML_SERR) !=0 ){
            va_copy(cargs,args);
            vfprintf(stderr,fmt, cargs);
            if (term!=0x00) {
                fprintf(stderr,"\n");
            }
        }
        if( (dest&ML_SOUT) !=0 ){
            va_copy(cargs,args);
            vprintf(fmt, cargs);
            if (term!=0x00) {
                fprintf(stdout,"\n");
            }
        }
        
        va_end(args);
    }
    return retval;
}
// End function mlog_printf


/// @fn int mlog_tprintf(mlog_id_t id, char * fmt)
/// @brief formatted print with timestamp to log destination(s).
/// uses mlog time format defined in mlog.h
/// @param[in] id log ID
/// @param[in] fmt print format (e.g. stdio printf)
/// @return number of bytes written (to file only) on success, -1 otherwise
int mlog_tprintf(mlog_id_t id, char *fmt, ...)
{
    int retval = -1;
    
    mlog_t *log = s_lookup_log(id);
    
    if(log!=NULL && NULL!=log->cfg){
        
        mlog_dest_t dest = log->cfg->dest;
        mlog_flags_t flags = log->cfg->flags;
        struct tm now = {0};
        time_t tt = time(NULL);
        gmtime_r(&tt,&now);
        
        char *tfmt = NULL;
        if (NULL!=log->cfg->tfmt) {
            tfmt = log->cfg->tfmt;
        }else{
            tfmt = ML_DFL_TFMT;
        }
        
        char timestamp[ML_MAX_TS_BYTES]={0};
       	memset(timestamp,0,ML_MAX_TS_BYTES);
        strftime(timestamp,ML_MAX_TS_BYTES,tfmt,&now);
        
        char *del = ( (NULL!=log->cfg->del)? log->cfg->del : ML_DFL_DEL);
        
        //get the arguments
        va_list args;
        va_list cargs;
        va_start(args, fmt);
        
        char term=( (fmt!=NULL && fmt[strlen(fmt)-1]=='\n') ? 0x00 : '\n');
//        fprintf(stderr,"mask[%x]&TL_LOG[%x]\n",strmask,(strmask&TL_LOG));
//        fprintf(stderr,"mask[%x]&TL_SERR[%x]\n",strmask,(strmask&TL_SERR));
//        fprintf(stderr,"mask[%x]&TL_SOUT[%x]\n",strmask,(strmask&TL_SOUT));
        
        if (dest&ML_FILE  && (flags&ML_DIS)==0 ) {
            // print message to buffer
            va_copy(cargs,args);

            int wbytes=snprintf(NULL,0,"%s%s",timestamp,del);
            wbytes += vsnprintf(NULL,0,fmt,cargs);
            retval=wbytes;
            // check write size and rotate if it will overflow
            // [will allow writes > segment/log size]
            if ( (log->cfg->lim_b > 0) && ((log->seg_len+wbytes) > log->cfg->lim_b) ) {
                s_log_rotate(log);
            }

            va_copy(cargs,args);
            if((wbytes=iow_fprintf(log->file,"%s%s",timestamp,del))>0){
                log->seg_len+=wbytes;
            }
            if( (wbytes=iow_vfprintf(log->file,fmt,cargs))>0){
                log->seg_len+=wbytes;
            }
        }else{
            MERROR("file output disabled d[%0X] f[%0X]\n",dest,flags);
        }
        
        // send to stderr, stdout
        if( (dest&ML_SERR) !=0 ){
            va_copy(cargs,args);
            fprintf(stderr,"%s%s",timestamp,del);
            vfprintf(stderr,fmt, cargs);
            if (term!=0x00) {
                fprintf(stderr,"\n");
            }
        }
        if( (dest&ML_SOUT) !=0 ){
            va_copy(cargs,args);
            fprintf(stdout,"%s%s",timestamp,del);
            vprintf(fmt, cargs);
            if (term!=0x00) {
                fprintf(stdout,"\n");
            }
        }
        
        va_end(args);
    }
    return retval;
}
// End function mlog_tprintf


/// @fn int mlog_write(mlog_id_t id, byte * data, uint32_t len)
/// @brief write bytes to log destination(s).
/// @param[in] id log ID
/// @param[in] data data buffer
/// @param[in] len number of bytes to write
/// @return number of bytes written on success, -1 otherwise
int mlog_write(mlog_id_t id, byte *data, uint32_t len)
{
    int retval = -1;
    
    mlog_t *log = s_lookup_log(id);
    
    if(log!=NULL){
        mlog_dest_t dest = log->cfg->dest;
        mlog_flags_t flags = log->cfg->flags;
        if (dest&ML_FILE  && (flags&ML_DIS)==0 ) {
            
            if ( (log->cfg->lim_b > 0) && (log->seg_len+len) > log->cfg->lim_b) {
                // if segment length is limited and write would overflow,
                // fill current segment and keep rotating
                // until all date written.
                byte *wp = data;
                uint32_t drem = (data+len-wp);
                while (drem>0) {
                    uint32_t srem = (log->cfg->lim_b - log->seg_len);
                    if (srem <= 0 ) {
                        s_log_rotate(log);
                        srem = log->cfg->lim_b;
                    }
                    if( (drem = (data+len-wp))<=0){
                        break;
                    }
                    uint32_t wlen = ( drem > srem ? srem : drem);
//                    MDEBUG("wlen[%u] drem[%u] srem[%u]\n",wlen,drem,srem);
                    if( (retval = iow_write(log->file,wp,wlen)) > 0 ){
                        log->seg_len+=retval;
                        wp += retval;
                    }else{
                        MERROR("iow_write failed [%d]\n",retval);
                        break;
                    }
                }
            }else{
//                MDEBUG("just writing");
                if( (retval = iow_write(log->file,data,len)) > 0 ){
                    log->seg_len+=retval;
                }else{
                    MERROR("iow_write failed [%d]\n",retval);
                }
            }
        }else{
            MERROR("file output disabled d[%0X] f[%0X]\n",dest,flags);
        }
    }
    return retval;
}
// End function mlog_write


/// @fn int mlog_puts(mlog_id_t id, char * data)
/// @brief write NULL terminated string to log destination(s).
/// @param[in] id log ID
/// @param[in] data string to write
/// @return number of bytes written on success, -1 otherwise
int mlog_puts(mlog_id_t id, char *data)
{
    int retval = -1;
    
    mlog_t *log = s_lookup_log(id);
    
    if(NULL!=log && NULL!=data){
        mlog_dest_t dest = log->cfg->dest;
        mlog_flags_t flags = log->cfg->flags;
        if (dest&ML_FILE  && (flags&ML_DIS)==0 ) {
            retval = mlog_write(id, (byte *)data, strlen(data)+1);
        }else{
            MERROR("file output disabled d[%0X] f[%0X]\n",dest,flags);
        }
    }
    return retval;
}
// End function mlog_puts


/// @fn int mlog_putc(mlog_id_t id, char data)
/// @brief write a character to log destination(s).
/// @param[in] id log ID
/// @param[in] data character to write
/// @return number of bytes written on success, -1 otherwise
int mlog_putc(mlog_id_t id, char data)
{
    int retval = -1;
    
    mlog_t *log = s_lookup_log(id);
    
    if(NULL!=log){
        
        mlog_dest_t dest = log->cfg->dest;
        mlog_flags_t flags = log->cfg->flags;
        if (dest&ML_FILE  && (flags&ML_DIS)==0 ) {
            retval=mlog_write(id, (byte *) &data, 1);
        }else{
            MERROR("file output disabled d[%0X] f[%0X]\n",dest,flags);
        }
    }
    return retval;
}
// End function mlog_putc


/// @fn int mlog_test()
/// @brief mlog unit test(s). may throw assertions.
/// @return 0 on success, -1 otherwise
int mlog_test()
{
    int retval = -1;
	
    
    mlog_id_t SYSLOG_ID=0x1;
    mlog_id_t BINLOG_ID=0x2;

    mlog_config_t alog_conf = {
        1024, 6, ML_NOLIMIT,
        ML_OSEG|ML_LIMLEN|ML_OVWR,
        ML_FILE,ML_TFMT_ISO1806};
    
    mlog_t *syslog_orig = mlog_new("alog.out",&alog_conf);
    mlog_show(syslog_orig,true,5);
    
    mlog_config_t blog_conf = {
        ML_NOLIMIT, ML_NOLIMIT, ML_NOLIMIT,
        ML_MONO,
        ML_FILE,ML_TFMT_ISO1806};
    mlog_t *binlog = mlog_new("blog.out",&blog_conf);
    mlog_show(binlog,true,5);

    s_parse_path("x",syslog_orig);
    s_parse_path(".x",syslog_orig);
    s_parse_path("x.",syslog_orig);
    s_parse_path(".x.",syslog_orig);

    s_parse_path("x.y",syslog_orig);
    s_parse_path(".x.y",syslog_orig);
    s_parse_path("x.y",syslog_orig);
    s_parse_path(".x.y",syslog_orig);

    s_parse_path("/x",syslog_orig);
    s_parse_path("/.x",syslog_orig);
    s_parse_path("/x.",syslog_orig);
    s_parse_path("/.x.",syslog_orig);

    s_parse_path("  /x",syslog_orig);
    s_parse_path("  /.x",syslog_orig);
    s_parse_path("  //x.",syslog_orig);
    s_parse_path("  //.x.",syslog_orig);

    s_parse_path("./x",syslog_orig);
    s_parse_path("./.x",syslog_orig);
    s_parse_path("./x.",syslog_orig);
    s_parse_path("./.x.",syslog_orig);
    
    s_parse_path("../x",syslog_orig);
    s_parse_path("../.x",syslog_orig);
    s_parse_path("../x.",syslog_orig);
    s_parse_path("../.x.",syslog_orig);
    
    s_parse_path("p/x",syslog_orig);
    s_parse_path("p/.x",syslog_orig);
    s_parse_path("p/x.",syslog_orig);
    s_parse_path("p/.x.",syslog_orig);

    s_parse_path("./p/x",syslog_orig);
    s_parse_path("./p/.x",syslog_orig);
    s_parse_path("./p/x.",syslog_orig);
    s_parse_path("./p/.x.",syslog_orig);

    s_parse_path("../p/x.y",syslog_orig);
    s_parse_path("../p/.x.y",syslog_orig);
    s_parse_path("../p/x.y",syslog_orig);
    s_parse_path("../p/.x.y",syslog_orig);

    s_parse_path("../p/x.y.",syslog_orig);
    s_parse_path("../p/.x.y.",syslog_orig);
    s_parse_path("../p/x.y.",syslog_orig);
    s_parse_path("../p/.x.y.",syslog_orig);

    s_parse_path("./alog.out",syslog_orig);
    
    iow_flags_t flags = IOW_RDWR|IOW_APPEND|IOW_CREATE;
    iow_mode_t mode = IOW_RU|IOW_WU|IOW_RG|IOW_WG;

    mlog_info_t linfo={0};
    s_get_log_info(&linfo,syslog_orig->path,syslog_orig->name, ML_SEG_FMT,syslog_orig->file);
    mlog_info_show(&linfo,true,5);

    mlog_open(syslog_orig, flags, mode);
    mlog_add(syslog_orig,SYSLOG_ID,"test-syslog");

    // save dest configuration
    uint32_t odest = mlog_get_dest(SYSLOG_ID);
    
    // direct log to stderr only
    mlog_set_dest(SYSLOG_ID,ML_SERR);
    mlog_printf(SYSLOG_ID,"should appear only @ stderr\n");
    mlog_set_dest(SYSLOG_ID,ML_FILE);
    mlog_printf(SYSLOG_ID,"should appear only @ syslog file\n");
    mlog_set_dest(SYSLOG_ID,ML_FILE|ML_SOUT);
    mlog_printf(SYSLOG_ID,"should appear @ syslog file and stdout\n");
    mlog_tprintf(SYSLOG_ID,"should appear @ syslog file (w/ timestamp) and stdout\n");
    
    // restore original settings
    mlog_set_dest(SYSLOG_ID,odest);
    mlog_puts(SYSLOG_ID,"puts wrote this - putc follows:\n");
    for (int i=0x20; i<0x50; i++) {
        mlog_putc(SYSLOG_ID,i);
    }
    mlog_putc(SYSLOG_ID,'\n');
    char wdata[]="this is mlog write data\n\0";
    mlog_write(SYSLOG_ID,(byte *)wdata,strlen(wdata));

    MDEBUG("segno /x/y/z12345.log    [%04d]\n",s_path_segno("/x/y/z12345.log","/x/y/z1",ML_SEG_FMT));
    MDEBUG("segno z_19999.log/z_1    [%04d]\n",s_path_segno("z_19999.log","z_1",ML_SEG_FMT));
    MDEBUG("segno z_1999999.log/z_19 [%04d]\n",s_path_segno("z_1999999.log","z_19",ML_SEG_FMT));
    MDEBUG("segno z_1999999/z_16     [%04d]\n",s_path_segno("z_1999999","z_16",ML_SEG_FMT));
    MDEBUG("segno z_1999999/z_       [%04d]\n",s_path_segno("z_1999999","z_",ML_SEG_FMT));
    
    MDEBUG("looking for max seg in dir [%s] using name[%s]\n", syslog_orig->path, syslog_orig->name);
    s_get_log_info(&linfo,syslog_orig->path,syslog_orig->name, ML_SEG_FMT,syslog_orig->file);
    MDEBUG("max_seg [%04hd]\n",linfo.seg_max);
    
    MDEBUG("before write (should rotate)...\n\n");
    mlog_info_show(&linfo,true,5);
    byte x[2048]={0};
    mlog_write(SYSLOG_ID,x,1024);
    MDEBUG("after write 1024...\n\n");
    s_get_log_info(&linfo,syslog_orig->path,syslog_orig->name, ML_SEG_FMT,syslog_orig->file);
    mlog_info_show(&linfo,true,5);
    mlog_write(SYSLOG_ID,x,500);
    MDEBUG("after write 500...\n\n");
    s_get_log_info(&linfo,syslog_orig->path,syslog_orig->name, ML_SEG_FMT,syslog_orig->file);
    mlog_info_show(&linfo,true,5);
    sleep(1);
    MDEBUG("writing 2048 (> max segment) to seg[%d]\n\n",syslog_orig->cur_seg);
    mlog_write(SYSLOG_ID,x,2048);
    s_get_log_info(&linfo,syslog_orig->path,syslog_orig->name, ML_SEG_FMT,syslog_orig->file);
    mlog_info_show(&linfo,true,5);
    MDEBUG("opening binlog\n");
    mlog_open(binlog, flags, mode);
    mlog_add(binlog,BINLOG_ID,"test-binlog");
    MDEBUG("writing binlog\n");
    for (int i=0; i<5; i++) {
        mlog_write(BINLOG_ID,x,2048);
    }
    // release list and other internal resources
    // (including registered log(s)
    mlog_release(true);
    retval=0;
    return retval;
}
// End function mlog_test


