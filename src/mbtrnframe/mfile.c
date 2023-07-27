///
/// @file mfile.c
/// @authors k. Headley
/// @date 01 jan 2018
 
/// mframe cross-platform file IO wrappers implementation
/// for *nix/Cygwin

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
#include "mframe.h"
#include "mfile.h"
#include "mxdebug.h"

/////////////////////////
// Macros
/////////////////////////

// These macros should only be defined for 
// application main files rather than general C files
/*
/// @def PRODUCT
/// @brief header software product name
#define PRODUCT "MFRAME"

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


/////////////////////////
// Imports
/////////////////////////

/////////////////////////
// Module Global Variables
/////////////////////////

/////////////////////////
// Function Definitions
/////////////////////////


/// @fn int s_iow2posix_flags(int iflags)
/// @brief convert iow to posix file flags.
/// @param[in] iflags iow flags
/// @return posix file flags value
static int s_iow2posix_flags(int iflags)
{
    int pflags=0;
    
    pflags |= ( (iflags&MFILE_RONLY   )!=0 ? O_RDONLY   : 0 );
    pflags |= ( (iflags&MFILE_WONLY   )!=0 ? O_WRONLY   : 0 );
    pflags |= ( (iflags&MFILE_RDWR    )!=0 ? O_RDWR     : 0 );
    pflags |= ( (iflags&MFILE_APPEND  )!=0 ? O_APPEND   : 0 );
    pflags |= ( (iflags&MFILE_CREATE  )!=0 ? O_CREAT    : 0 );
    pflags |= ( (iflags&MFILE_TRUNC   )!=0 ? O_TRUNC    : 0 );
    pflags |= ( (iflags&MFILE_NONBLOCK)!=0 ? O_NONBLOCK : 0 );
    pflags |= ( (iflags&MFILE_DSYNC   )!=0 ? O_DSYNC    : 0 );
#if defined(__APPLE__)
    pflags |= ( (iflags&MFILE_RSYNC   )!=0 ? O_SYNC    : 0 );
#else
    // O_RSYNC=0
    pflags |=  0;
#endif
    pflags |= ( (iflags&MFILE_SYNC    )!=0 ? O_SYNC     : 0 );
#if defined(__CYGWIN__)
    pflags |= ( (iflags&MFILE_ASYNC   )!=0 ? O_SYNC    : 0 );
#else
    // O_ASYNC=0
    pflags |= 0;
#endif
    pflags |= ( (iflags&MFILE_EXCL    )!=0 ? O_EXCL     : 0 );
//    pflags |= ( (iflags&MFILE_DIRECT  )!=0 ? O_DIRECT   : 0 );
    
    return pflags;
}
// End function s_iow2posix_flags

/// @fn mode_t s_iow2posix_mode(mfile_mode_t imode)
/// @brief convert iow to posix file (permission) mode flags.
/// @param[in] imode iow file modes
/// @return posix file mode flags
static mode_t s_iow2posix_mode(mfile_mode_t imode)
{
    mode_t pmode=0;
    pmode |= ( (imode&MFILE_RWXU)!=0 ? S_IRWXU : 0 );
    pmode |= ( (imode&MFILE_RU  )!=0 ? S_IRUSR : 0 );
    pmode |= ( (imode&MFILE_WU  )!=0 ? S_IWUSR : 0 );
    pmode |= ( (imode&MFILE_XU  )!=0 ? S_IXUSR : 0 );
    pmode |= ( (imode&MFILE_RWXG)!=0 ? S_IRWXG : 0 );
    pmode |= ( (imode&MFILE_RG  )!=0 ? S_IRGRP : 0 );
    pmode |= ( (imode&MFILE_WG  )!=0 ? S_IWGRP : 0 );
    pmode |= ( (imode&MFILE_XG  )!=0 ? S_IXGRP : 0 );
    pmode |= ( (imode&MFILE_RWXO)!=0 ? S_IRWXO : 0 );
    pmode |= ( (imode&MFILE_RO  )!=0 ? S_IROTH : 0 );
    pmode |= ( (imode&MFILE_WO  )!=0 ? S_IWOTH : 0 );
    pmode |= ( (imode&MFILE_XO  )!=0 ? S_IXOTH : 0 );
//    MX_PRINT("imode[0x%0X] pmode[0x%0X] S_IRUSR[0x0%X] S_IWUSR[0x%0X] S_IRGRP[0x%0X] S_IWGRP[0x%0X]\n", imode, pmode, S_IRUSR, S_IWUSR, S_IRGRP, S_IWGRP);
//    MX_PRINT("S_IRWXU[0x%03X] S_IRUSR[0x%03X] S_IWUSR[0x%03X] S_IXUSR[0x%03X]\n", S_IRWXU, S_IRUSR, S_IWUSR, S_IXUSR);
//    MX_PRINT("S_IRWXG[0x%03X] S_IRGRP[0x%03X] S_IWGRP[0x%03X] S_IXGRP[0x%03X]\n", S_IRWXG, S_IRGRP, S_IWGRP, S_IXGRP);
//    MX_PRINT("S_IRWXO[0x%03X] S_IROTH[0x%03X] S_IWOTH[0x%03X] S_IXOTH[0x%03X]\n", S_IRWXO, S_IROTH, S_IWOTH, S_IXOTH);
    return pmode;
}
// End function s_iow2posix_mode

/// @fn mfile_file_t * mfile_file_new(const char * path)
/// @brief new file instance. This is the equivalent of a file descriptor.
/// @param[in] path file path
/// @return new file instance.
mfile_file_t *mfile_file_new(const char *path)
{
    mfile_file_t *self = (mfile_file_t *)malloc(sizeof(mfile_file_t));
    if (self) {
        memset(self,0,sizeof(mfile_file_t));
        self->fd = -1;
        if (NULL!=path) {
            self->path = strdup(path);
        }else{
            self->path=NULL;
        }
    }
    return self;
}
// End function mfile_file_new

/// @fn void mfile_file_destroy(mfile_file_t ** pself)
/// @brief release file resources.
/// @param[in] pself pointer to file reference
/// @return none
void mfile_file_destroy(mfile_file_t **pself)
{
    if (NULL!=pself) {
        mfile_file_t *self = *pself;
        if (NULL!=self) {
            if (NULL!=self->path) {
                free(self->path);
            }
            free(self);
            *pself = NULL;
        }
    }
}
// End function mfile_file_destroy

/// @fn void mfile_file_show(mfile_file_t * self, _Bool verbose, uint16_t indent)
/// @brief output file parameter summary to stderr.
/// @param[in] self file reference
/// @param[in] verbose true for verbose output
/// @param[in] indent indentation (spaces)
/// @return none
void mfile_file_show(mfile_file_t *self, bool verbose, uint16_t indent)
{
    if (NULL != self) {
        fprintf(stderr,"%*s[self     %10p]\n",indent,(indent>0?" ":""), self);
        fprintf(stderr,"%*s[path     %10s]\n",indent,(indent>0?" ":""), self->path);
        fprintf(stderr,"%*s[fd       %10d]\n",indent,(indent>0?" ":""), self->fd);
        fprintf(stderr,"%*s[flags    %010X]\n",indent,(indent>0?" ":""), self->flags);
        fprintf(stderr,"%*s[mode     %010X]\n",indent,(indent>0?" ":""), self->mode);
    }
}
// End function mfile_file_show

/// @fn int mfile_open(mfile_file_t * self, mfile_flags_t flags)
/// @brief open a file.
/// @param[in] self file reference
/// @param[in] flags flag values
/// @return file descriptor (>0) on success, -1 otherwise
int mfile_open(mfile_file_t *self,mfile_flags_t flags)
{
    int retval=-1;
    if (NULL!=self && NULL!=self->path) {
        int pflags=s_iow2posix_flags(flags);
        if ( (retval = open(self->path,pflags))>0){
           self->fd= retval;
            self->flags=pflags;
//             MX_PRINT("open setting fd[%d]\n", self->fd);
        }else{
            fprintf(stderr,"open failed [%d/%s]\n",errno,strerror(errno));
            self->fd=-1;
        }
    }else{
        fprintf(stderr,"%s: invalid argument\n",__func__);
    }
    return retval;
}
// End function mfile_open

/// @fn int mfile_mopen(mfile_file_t * self, mfile_flags_t flags, mfile_mode_t mode)
/// @brief open file, specify permission modes.
/// @param[in] self file reference
/// @param[in] flags flag values
/// @param[in] mode permission flags
/// @return file descriptor (>0) on success, -1 otherwise
int mfile_mopen(mfile_file_t *self,mfile_flags_t flags,mfile_mode_t mode )
{
    int retval=-1;
    if (NULL!=self && NULL!=self->path) {
        int pflags=s_iow2posix_flags(flags);
        mode_t pmode=s_iow2posix_mode(mode);
//        MX_PRINT("opening [%s] mode[%x] pmode[%x]\n", self->path, mode, pmode);
        if ( (retval = open(self->path,pflags,pmode) )>0){
            self->fd= retval;
            self->flags=pflags;
            self->mode=pmode;
//            MX_PRINT("open setting fd[%d]\n", self->fd);
        }else{
            fprintf(stderr,"open failed [%d/%s]\n",errno,strerror(errno));
            self->fd=-1;
        }
    }else{
        fprintf(stderr,"%s: invalid argument\n",__func__);
    }
    return retval;
}
// End function mfile_mopen

/// @fn int mfile_close(mfile_file_t * self)
/// @brief close a file.
/// @param[in] self file instance
/// @return 0 on success, -1 otherwise.
int mfile_close(mfile_file_t *self)
{
    int retval=-1;
    if (NULL!=self && self->fd>0) {
        retval = close(self->fd);
        self->fd = -1;
    }
    return retval;
}
// End function mfile_close

/// @fn int mfile_rename(mfile_file_t * self, const char * path)
/// @brief rename file. closes and reopens file; may change underlying
/// file descriptor/handle.
/// @param[in] self file
/// @param[in] path new name
/// @return new file descriptor/handle (>0) on success, -1 otherwise
int mfile_rename(mfile_file_t *self,const char *path)
{
    int retval = -1;
    
    if (NULL!=self && NULL!=path) {
        
        if (self->fd>0) {
            mfile_close(self);
        }

        if (NULL!=self->path) {
            free(self->path);
        }
        
//       MX_PRINT("renaming to %s\n", path);
        self->path = strdup(path);
        
        if ( (retval = open(self->path,self->flags|O_CREAT,self->mode|S_IWUSR|S_IRUSR) )>0){
            self->fd = retval;
//            MX_PRINT("opened %s fd[%d]\n", self->path, self->fd);
            //            MX_PRINT("open setting fd[%d]\n", self->fd);
        }else{
            fprintf(stderr,"open %s failed [%d/%s]\n",self->path,errno,strerror(errno));
            self->fd=-1;
        }

    }else{
        fprintf(stderr,"invalid arguments\n");
    }
    return retval;
}
// End function mfile_rename

/// @fn int64_t mfile_seek(mfile_file_t *self, uint32_t ofs)
/// @brief move cursor to specified offset
/// @param[in] self file instance
/// @param[in] ofs offset
/// @return number of bytes read >=0 on success, or -1 otherwise.
int64_t mfile_seek(mfile_file_t *self, uint32_t ofs, mfile_whence_t whence)
{
    off_t retval= (off_t)-1;
    if (NULL != self) {
        off_t test=0;
        off_t offset=ofs;
        int pwhence=-1;
        switch (whence) {
            case MFILE_SET:
                pwhence=SEEK_SET;
                break;
            case MFILE_CUR:
                pwhence=SEEK_CUR;
                break;
            case MFILE_END:
                pwhence=SEEK_END;
                break;
            default:
                fprintf(stderr,"ERR - invalid mode[%d]\n",whence);
                break;
        }
        if( (pwhence!=-1) && (test=lseek(self->fd, offset, pwhence))>=0){
            retval = test;
//            fprintf(stderr,"mfile_seek - seek fd[%d] returned[%"PRId32"] [%d/%s]\n",self->fd,(int32_t)test,errno,strerror(errno));
        }else{
            fprintf(stderr,"seek failed [%d/%s]\n",errno,strerror(errno));
        }
    }else{
        fprintf(stderr,"%s: invalid argument\n",__func__);
    }
    return retval;

}
// End function mfile_seek

/// @fn int64_t mfile_read(mfile_file_t * self, byte * dest, uint32_t len)
/// @brief read bytes from file (advances input pointer).
/// @param[in] self file instance
/// @param[in] dest data buffer
/// @param[in] len bytes to read
/// @return number of bytes read >=0 on success, or -1 otherwise.
int64_t mfile_read(mfile_file_t *self, byte *dest, uint32_t len)
{
    int64_t retval=-1;
    if (NULL != self && NULL!=dest && len!=0) {
        ssize_t test=0;
        if( (test=read(self->fd, dest, len))>=0){
            retval = (int64_t)test;
        }else{
            fprintf(stderr,"read failed [%d/%s]\n",errno,strerror(errno));
        }
    }else{
        fprintf(stderr,"%s: invalid argument\n",__func__);
    }
    return retval;
}
// End function mfile_read

/// @fn int64_t mfile_write(mfile_file_t * self, byte * src, uint32_t len)
/// @brief write bytes to file (advances input pointer.
/// @param[in] self file instance
/// @param[in] src data buffer
/// @param[in] len bytes to write
/// @return bytes written on success (>=0), -1 otherwise
int64_t mfile_write(mfile_file_t *self, byte *src, uint32_t len)
{
    int64_t retval=-1;
    if (NULL != self && NULL!=src && len>0) {
        ssize_t test=0;
        if( (test=write(self->fd, src, len))>0){
            retval = (int64_t)test;
        }else{
            fprintf(stderr,"write failed [%d/%s]\n",errno,strerror(errno));
        }
    }else{
        fprintf(stderr,"%s: invalid argument\n",__func__);
    }
    return retval;
}
// End function mfile_write

/// @fn int mfile_ftruncate(mfile_file_t * self, uint32_t len)
/// @brief truncate file to specified length.
/// @param[in] self file instance
/// @param[in] len len to truncate to
/// @return 0 on success, -1 otherwise
int mfile_ftruncate(mfile_file_t *self, uint32_t len)
{
    int retval=-1;
    if (NULL != self){
    	retval = ftruncate(self->fd,len);
    }
    return retval;
}
// End function mfile_ftruncate

/// @fn int mfile_fprintf(mfile_file_t * self, const char * fmt, ...)
/// @brief formatted print to file.
/// @param[in] self file instance
/// @param[in] fmt print format (e.g. stdio printf)
/// @param[in] ... print arguments
/// @return number of bytes output on success, -1 otherwise.
int mfile_fprintf(mfile_file_t *self, const char *fmt, ...)
{
    int retval=-1;
    
    if(NULL != self){
        //get the arguments
        va_list args;
        va_start(args,fmt);
        
        retval=vdprintf(self->fd,fmt,args);

        va_end(args);
    }
    return retval;
}
// End function mfile_fprintf

#if defined(__QNX__)
/// @fn int mfile_vfprintf(mfile_file_t * self, const char * fmt, va_list args)
/// @param[in] self file instance
/// @param[in] fmt print format (e.g. stdio printf)
/// @param[in] args print arguments (as va_list - posix, supported on many platforms)
/// @return number of bytes output on success, -1 otherwise.
int mfile_vfprintf(mfile_file_t *self, const char *fmt, va_list args)
{
    int retval=-1;
//    if(NULL != self && self->fd>0){
//        retval=vdprintf(self->fd,fmt,args);
//    }else{
//        fprintf(stderr,"%s: invalid argument\n",__func__);
//    }
    return retval;
}
// End function mfile_vfprintf
#else
/// @fn int mfile_vfprintf(mfile_file_t * self, const char * fmt, va_list args)
/// @param[in] self file instance
/// @param[in] fmt print format (e.g. stdio printf)
/// @param[in] args print arguments (as va_list - posix, supported on many platforms)
/// @return number of bytes output on success, -1 otherwise.
int mfile_vfprintf(mfile_file_t *self, const char *fmt, va_list args)
{
    int retval=-1;
    
    if(NULL != self && self->fd>0){
        va_list cargs;
        va_copy(cargs,args);
        retval=vdprintf(self->fd,fmt,cargs);
        // comment out this va_end if it causes an issue
        // (added per cppcheck)
        va_end(cargs);

    }else{
        fprintf(stderr,"%s: invalid argument\n",__func__);
    }
    return retval;
}
// End function mfile_vfprintf
#endif

/// @fn int mfile_flush(mfile_file_t * self)
/// @brief flush, attempt to sync to disk.
/// calls fsync; may not actually force write to disk until closed
/// @param[in] self file reference
/// @return 0 on success, -1 otherwise
int mfile_flush(mfile_file_t *self)
{
    int retval=-1;
    if (NULL!=self && self->fd>0) {
        retval=fsync(self->fd);
    }else{
        fprintf(stderr,"%s: invalid argument\n",__func__);
    }
    return retval;
    
}
// End function mfile_flush

/// @fn int64_t mfile_fsize(mfile_file_t * self)
/// @brief file size.
/// @param[in] self file instance
/// @return number of bytes on disk on success, -1 otherwise
int64_t mfile_fsize(mfile_file_t *self)
{
    int64_t retval=-1;
    struct stat info={0};
    if (NULL!=self) {
        if (stat(self->path,&info)==0) {
            retval=(int64_t)info.st_size;
        }else{
            fprintf(stderr,"stat failed[%d/%s]\n",errno,strerror(errno));
        }
    }else{
        fprintf(stderr,"%s: invalid argument\n",__func__);
    }
    return retval;
}
// End function mfile_fsize

/// @fn time_t mfile_mtime(const char * path)
/// @brief return modification time of file (seconds since 1/1/70).
/// @param[in] path file path
/// @return modification time of file on success, -1 otherwise
time_t mfile_mtime(const char *path)
{
    time_t retval=-1;
    struct stat info={0};
    if (NULL!=path) {
        if (stat(path,&info)==0) {
            retval=info.st_mtime;
        }else{
            fprintf(stderr,"stat failed[%d/%s]\n",errno,strerror(errno));
        }
    }else{
        fprintf(stderr,"%s: invalid argument\n",__func__);
    }
    return retval;
}
// End function mfile_mtime

/// @fn int mfile_fd(mfile_file_t *self)
/// @brief return underlying file descriptor
/// @param[in] self file instance
/// @return file descriptor on success, -1 otherwise
int mfile_fd(mfile_file_t *self)
{
  if (NULL!=self) {
      return self->fd;
  }
  return -1;
}

