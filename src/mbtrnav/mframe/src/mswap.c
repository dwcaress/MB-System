///
/// @file mswap.c
/// @authors k. headley
/// @date 20 aug 2019

/// Byte swap macros
///.Not optimized, but more portable than byteswap.h

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

/////////////////////////
// Macros
/////////////////////////
#define MSWAP_NAME "mswap"
#ifndef MSWAP_BUILD
/// @def MSWAP_BUILD
/// @brief module build date.
/// Sourced from CFLAGS in Makefile
/// w/ -DMSWAP_BUILD=`date`
#define MSWAP_BUILD ""VERSION_STRING(APP_BUILD)
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
#include <stdlib.h>
#ifdef WITH_MSWAP_TEST
#include <stdio.h>
#endif
#include "mswap.h"

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

int  mswap_bytes(void *src, size_t len)
{
    int retval=-1;
    if( NULL!=src && (len>0) && ((len%2)==0) ){
        unsigned char *start=NULL, *end=NULL;

        // no dest or src==dest, swap in place
        for ( start = (unsigned char *)src, end = start + len - 1; start < end; ++start, --end )
        {
            unsigned char swap = *start;
            *start = *end;
            *end = swap;
        }
        retval=0;
    }
    return retval;
}

int  mswap_bytes_mem(void *dest, void *src, size_t len)
{
    int retval=-1;
    if( NULL!=src && NULL!=dest && (len>0) && ((len%2)==0) ){
        unsigned char *start=NULL, *end=NULL;
        unsigned char *dstart=NULL, *dend=NULL;
        
        dstart=(unsigned char *)dest;
        dend=dstart+len-1;
        for ( start = (unsigned char *)src, end = start + len - 1; start < end; ++start, --end, ++dstart, --dend )
        {
            *dstart=*end;
            *dend=*start;
        }
        retval=0;
    }
    return retval;
}

#ifdef WITH_MSWAP_TEST
int mswap_test(int verbose)
{
    int retval=-1;
    uint16_t err_count=0;
    
    uint16_t u16=0xABCD;
    uint32_t u32=0xABCD1234;
    uint64_t u64=0xABCD1234CAFEDEAD,d64=0;
    
    if(verbose)fprintf(stderr,"u16[%04X/%04X]\n",u16,mswap_16(u16));
    if(mswap_16(u16)!=0xCDAB)err_count|=(1<<0);
    
    if(verbose)fprintf(stderr,"u32[%04X/%08X]\n",u32,mswap_32(u32));
    if(mswap_32(u32)!=0x3412CDAB)err_count|=(1<<1);
    
    if(verbose)fprintf(stderr,"u64[%016lX/%016lX]\n",(long unsigned)u64,(long unsigned)mswap_64(u64));
    if(mswap_64(u64)!=0xADDEFECA3412CDAB)err_count|=(1<<2);

    if(verbose)fprintf(stderr,"u64[%016lX] (inplace)\n",(long unsigned)u64);
    if(!(mswap_bytes(&u64,8)==0 && u64==0xADDEFECA3412CDAB))err_count|=(1<<3);
    if(verbose)fprintf(stderr,"u64*[%016lX] (inplace)\n",(long unsigned)u64);
    
    if(!(mswap_bytes_mem(&d64,&u64,8)==0 && d64==0xABCD1234CAFEDEAD))err_count|=(1<<4);
    if(verbose)fprintf(stderr,"u64[%016lX/%016lX] (mem)\n",(long unsigned)u64,(long unsigned)d64);

    retval=err_count;
    
    return retval;
}
#endif
