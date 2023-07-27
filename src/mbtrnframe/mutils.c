///
/// @file mutils.c
/// @authors k. Headley
/// @date 11 aug 2017

/// mframe utilities implementation

/// @sa doxygen-examples.c for more examples of Doxygen markup


/////////////////////////
// Terms of use
/////////////////////////
/*
 Copyright Information
  
 Copyright 2002-2017 MBARI
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
#include "mutils.h"

/////////////////////////
// Macros
/////////////////////////

// These macros should only be defined for 
// application main files rather than general C files
/*
/// @def PRODUCT
/// @brief header software product name
#define PRODUCT "qframe"

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

/// @fn void mfu_hex_show(byte * data, uint32_t len, uint16_t cols, _Bool show_offsets, uint16_t indent)
/// @brief output data buffer bytes in hex to stderr.
/// @param[in] data buffer pointer
/// @param[in] len number of bytes to display
/// @param[in] cols number of columns to display
/// @param[in] show_offsets show starting offset for each row
/// @param[in] indent output indent spaces
/// @return none
void mfu_hex_show(byte *data, uint32_t len, uint16_t cols, bool show_offsets, uint16_t indent)
{
    if (NULL!=data && len>0 && cols>0) {
        int rows = len/cols;
        int rem = len%cols;
        int i=0;
        byte *p=data;
        for (i=0; i<rows; i++) {
            if (show_offsets) {
//                fprintf(stderr,"%*s%04zd [",indent,(indent>0?" ":""),(p-data));
                fprintf(stderr,"%*s%04lx [",indent,(indent>0?" ":""),(unsigned long)(p-data));
            }else{
                fprintf(stderr,"%*s[",indent,(indent>0?" ":""));
            }
            int j=0;
            for (j=0; j<cols; j++) {
                if (p>=data && p<(data+len)) {
                    byte b = (*p);
                    fprintf(stderr," %02x",b);
                    p++;
                }else{
                    fprintf(stderr,"   ");
                }
            }
            fprintf(stderr," ]\n");
        }
        if (rem>0) {
            if (show_offsets) {
//                fprintf(stderr,"%*s%04zd [",indent,(indent>0?" ":""),(p-data));
                fprintf(stderr,"%*s%04lx [",indent,(indent>0?" ":""),(unsigned long)(p-data));
            }else{
                fprintf(stderr,"%*s[",indent,(indent>0?" ":""));
            }
            int j=0;
            for (j=0; j<rem; j++) {
                fprintf(stderr," %02x",*p++);
            }
            fprintf(stderr,"%*s ]\n",3*(cols-rem)," ");
        }
        
    }
}
// End function mfu_hex_show

/// @fn uint32_t mfu_checksum(byte * pdata, uint32_t len)
/// @brief return uint32_t checksum for data.
/// @param[in] pdata data pointer
/// @param[in] len length of data.
/// @return uint32 checksum value (sum of bytes).
uint32_t mfu_checksum(byte *pdata, uint32_t len)
{
    uint32_t checksum=0;
    if (NULL!=pdata) {
        byte *bp = pdata;
        //        fprintf(stderr,"\n");
        uint32_t i=0;
        for (i=0; i<len; i++) {
            checksum += (byte)(*(bp+i));
            //            fprintf(stderr,"%x ",(*(bp+i)));
        }
    }
    //    fprintf(stderr,"\nret[%08X]\n",checksum);
    return checksum;
}
// End function mfu_checksum

/// @fn void mfu_trim(char *buf, uint32_t len)
/// @brief trim whitespace from both ends of string.
/// moves string to beginning of buffer, replaces whitespace with null.
/// @param[in] buf string to trim
/// @param[in] len length of data.
/// @return uint32 checksum value (sum of bytes).
void mfu_trim(char *buf, uint32_t len)
{
    if (NULL!=buf && len>0) {
        char *phead=buf;
        char *ptail=buf+len;
        
        while (phead<ptail) {
            if (!isspace(*phead) || *phead=='\0') {
                break;
            }
            phead++;
        }
        while (ptail>phead) {
            if (!isspace(*ptail) && *ptail!='\0') {
                *(ptail+1)='\0';
                break;
            }
            ptail--;
        }
        {
        size_t trim_len=ptail-phead+1;
        if (phead>buf) {
            memcpy(buf,phead,trim_len);
            buf[trim_len]='\0';
        }
        }

    }
}

char *mfu_vsprint(int sz_hint, const char *fmt, ...)
{
    char *retval=NULL;
    
    int size = (sz_hint>=0 ? sz_hint : 32);
    char *p=NULL, *np=NULL;
    va_list ap;
    
    if ((p = malloc(size)) == NULL)
        return NULL;
    
    while (1) {
        // Try to print in the allocated space
        va_start(ap, fmt);
        int n = vsnprintf(p, size, fmt, ap);
        va_end(ap);
        
        // Check error code
        if (n < 0){
            break;
        }
        
        // If that worked, return the string
        if (n < size){
            retval=p;
            break;
        }
        
        // Else try again with more space
        // Precisely what is needed
        size = n + 1;
        
        if ((np = realloc (p, size)) == NULL) {
            free(p);
            break;
        } else {
            p = np;
        }
    }
    
    return retval;
}

int mfu_vbprint(char **dest, size_t len, off_t ofs, const char *fmt, ...)
{
    int retval=-1;
    if (NULL!=dest && NULL!=*dest && ofs<(off_t)len) {
        int n=0;
        char *bp=*dest;
        
        va_list ap;
        va_start(ap,fmt);
        n = vsnprintf(bp,0,fmt,ap);
        va_end(ap);
        
        char *new_mem=NULL;
        if (n>0) {
            size_t new_len=strlen(*dest)+n+1;
            new_mem=*dest+ofs;
            if ( new_len>len ) {
                *dest=(char *)realloc(*dest,new_len);
                new_mem=*dest+ofs;
                memset(new_mem,0,n+1);
            }
            va_list ao;
            va_start(ao,fmt);
            va_list aq;
            va_copy(aq,ao);
            retval = vsnprintf(new_mem,n+1,fmt,aq);
            va_end(ao);
            va_end(aq);
        }
    }
    return retval;
}

void mfu_fmt_xml(int fd,const char *buf, const char *del, int indent)
{
    const char *end=NULL;
    const char *ip=buf;
    const char *tp=buf;
    typedef enum{CONTENT,OPENING,CLOSING,}state_t;
    typedef enum{NOP=0x0,SKIP_BYTE=0x1,PRINT_BYTE=0x2,NEWLINE=0x4,COLLAPSE=0x8}action_t;
   state_t state=CONTENT;
    action_t action=NOP;

    
    if (fd>=0 && NULL!=buf) {
    	int i=0;
        int level=0;

        end=buf+strlen(buf);
        
        // sync to first tag
        while(*ip!='<' && ip<end )ip++;
        
        for(i=0;i<(level+indent);i++){
            if(write(fd," ",1) < 0){
                fprintf(stderr,"%s:%d write error [%d/%s]\n", __func__, __LINE__, errno, strerror(errno));
            }
        }

        while(ip>=buf && ip<end ){
            switch (*ip) {
                case '<':
                    action=PRINT_BYTE;
                    tp=ip+1;
                    while(isspace(*tp) && tp<end)tp++;
                    if(*tp!='/'){
                        state=CLOSING;
                   }else{
                        state=OPENING;
                    }
                    break;
                case '>':
                   action=PRINT_BYTE;

                    if (state==CLOSING) {
                        // is next element tag?
                        tp=ip+1;
                        while(isspace(*tp) && tp<end)tp++;
                        if(*tp=='<'){
                            // is it a closing tag
                            tp++;
                            while(isspace(*tp) && tp<end)tp++;
                            if(*tp!='/'){
                                level++;
                                action|=NEWLINE;
                            }
                        }
                    }else if (state==OPENING){
                        tp=ip+1;
                        while(isspace(*tp) && tp<end)tp++;
                        if(*tp=='<'){
                            action|=NEWLINE;
                            tp++;
                            while(isspace(*tp) && tp<end)tp++;
                            if(*tp=='/'){
                                level--;
                            }
                        }
                    }
                    break;
                    
                case ' ':
                case '\t':
                case '\n':
                case '\r':
                    action=COLLAPSE;
                    break;
                    
                default:
                    action=PRINT_BYTE;
                    break;
            }
            if ( (action&PRINT_BYTE) !=0 ) {
                if(write(fd,ip++,1) < 0){
                    fprintf(stderr,"%s:%d write error [%d/%s]\n", __func__, __LINE__, errno, strerror(errno));
                }
            }

            if ( (action&NEWLINE) !=0 ) {
	            int j=0;
                if(NULL!=del){
                    if(write(fd,del,strlen(del)) < 0){
                        fprintf(stderr,"%s:%d write error [%d/%s]\n", __func__, __LINE__, errno, strerror(errno));
                    }
                }
                for(j=0;j<(level+indent);j++){
                    if(write(fd," ",1) < 0){
                        fprintf(stderr,"%s:%d write error [%d/%s]\n", __func__, __LINE__, errno, strerror(errno));
                    }
                }
            }
            
            if ( (action&COLLAPSE) !=0) {
                while(isspace(*ip))ip++;
            }
        }
        if(NULL!=del){
            if(write(fd,del,strlen(del)) < 0){
                fprintf(stderr,"%s:%d write error [%d/%s]\n", __func__, __LINE__, errno, strerror(errno));
            }
        }

    }else{
        fprintf(stderr,"ERR - invalid argument\n");
    }
}

#ifdef WITH_MUTILS_TEST
int mfu_test(int verbose)
{
    int retval=-1;
    unsigned int err_count=0;
    
    char buf[64]={0};
    char *bp=buf;

    snprintf(buf, 64, "ABCDEFGHIJK0123456789\n");
    mfu_hex_show((byte *)buf,64,16,true,5);

    memset(buf,0x02,64);
    if(mfu_checksum((byte *)buf,64)!=128)err_count|=(1<<0);
    
    char *str = mfu_vsprint(32,"mfu_vsprint\n");
    if(!(NULL!=str && strcmp(str,"mfu_vsprint\n")==0))err_count|=(1<<1);
    if(NULL!=str)free(str);

    memset(buf,0,64);
    mfu_vbprint(&bp, 64, 0, "  test vbprint \t\n");
    if(strcmp(buf,"  test vbprint \t\n")!=0)err_count|=(1<<2);
    fprintf(stderr,"vbprint[%s]\n",buf);

    mfu_trim(buf,strlen(buf));
    if(strcmp(buf,"test vbprint")!=0)err_count|=(1<<3);
    fprintf(stderr,"trim[%s]\n",buf);

    snprintf(buf, 64, "<a> foo <b> bar <c>baz\n<\\c><\\b><\\a>\n");
     mfu_fmt_xml(1, buf,"\n", 5);

    retval=err_count;
    return retval;
}
#endif // WITH_MUTILS_TEST
