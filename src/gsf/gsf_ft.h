/********************************************************************
 *
 * Module Name : GSF_FT
 *
 * Author/Date : J. S. Byrne / 25 Aug 1995
 *
 * Description : This header file contains the data structure definitions
 *    for the library's internal file table.  These definitions are only
 *    needed by the library, and are not intended to be access by any
 *    calling applications.
 *
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 * who  when      what
 * ---  ----      ----
 * jsb  04-16-97  Added integer fields to GSF_FILE_TABLE for major and
 *                 minor version numbers. Changed type of previous_record
 *                 field from fpos_t to long. This was done so we can
 *                 compair file position contained in previous record with
 *                 file position information stored in the index file.
 * bac 06-28-06   Changed structure elements of type long to int, for
 *                 compilation on 64-bit architectures.
 *
 * Classification : Unclassified
 *
 * References :
 *
 *
 * © 2014 Leidos, Inc.
 * There is no charge to use the library, and it may be accessed at:
 * https://www.leidos.com/maritime/gsf.
 * This library may be redistributed and/or modified under the terms of
 * the GNU Lesser General Public License version 2.1, as published by the
 * Free Software Foundation.  A copy of the LGPL 2.1 license is included with
 * the GSF distribution and is avaialbe at: http://opensource.org/licenses/LGPL-2.1.
 *
 * Leidos, Inc. configuration manages GSF, and provides GSF releases. Users are
 * strongly encouraged to communicate change requests and change proposals to Leidos, Inc.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.
 *
 ********************************************************************/

#ifndef __GSF_FT_H__
#define __GSF_FT_H__

#include <limits.h>

/* Define the size of buffer of ping addresses kept for direct access */
#define PING_ADDR_BUF_SIZE 1024

/* Macro definitions for the state of the read_write_flag */
#define LAST_OP_FLUSH          0
#define LAST_OP_READ           1
#define LAST_OP_WRITE          2

/* Macro definitions for the limits of the scale factor multipler */
#define MIN_GSF_SF_MULT_VALUE  1
#define MAX_GSF_SF_MULT_VALUE  ULONG_MAX

/* Typedef structure to hold the record index information */
typedef struct
{
    int             sec;                              /* seconds from the epoch       */
    int             nsec;                             /* nanoseconds of the second    */
    long long       addr;                             /* address in the gsf file      */
}
INDEX_REC;

/* Typedef structure to hold index file information for direct access */
typedef struct t_index_data
{
    FILE           *fp;                               /* file pointer for the index   */
    int             swap;                             /* byte swap flag               */
    int             number_of_types;                  /* number of record types       */
    int             record_type[NUM_REC_TYPES];       /* record types                 */
    long long       start_addr[NUM_REC_TYPES];        /* start address of record type */
    int             number_of_records[NUM_REC_TYPES]; /* number of record type recs   */
    INDEX_REC      *scale_factor_addr;                /* scale factor index array     */
    int             last_scale_factor_index;          /* last scale index             */
}
INDEX_DATA;

/* Define a data structure to maintain a table of information for each
 * GSF file opened.
 */
typedef struct t_gsfFileTable
{
    FILE           *fp;                            /* File descriptor */
    char            file_name[1024];               /* The file's name */
    int             major_version_number;          /* The gsf library version ID which created this file */
    int             minor_version_number;          /* The gsf library version ID which created this file */
    long long       file_size;                     /* The file's size when gsfOpen is called */
    long long       previous_record;               /* File offset to previous record */
    int             buf_size;                      /* Standard library buffer size */
    int             bufferedBytes;                 /* How many bytes we've transfered */
    int             occupied;                      /* Is this table slot being used */
    int             update_flag;                   /* Is the file open for update */
    int             direct_access;                 /* Is the file open for direct acess */
    int             read_write_flag;               /* State variable for last I/O operation (1=read, 2=write) */
    int             scales_read;                   /* Set when scale factors are read in with ping record */
    int             access_mode;                   /* How was the file opened */
    int             last_record_type;              /* Record type of the last record we successfully read (or wrote) */
    INDEX_DATA      index_data;                    /* Index information used for direct file access */
    gsfRecords      rec;                           /* Our copy of pointers to dynamic memory and scale factors */
}
GSF_FILE_TABLE;

//------------------------------------------------------------------------------
// _LARGEFILE64_SOURCE
//------------------------------------------------------------------------------
//#undef fopen
//#undef fseek
//#undef ftell
//#undef stat
//#if (defined __WINDOWS__) || (defined __MINGW32__)
//#define stat(x,y) _stati64((x), (y))
//#define fopen(x, y)  fopen64((x), (y))
//#define fseek(x, y, z) fseeko64((x), (y), (z))
//#define ftell(x)   ftello64((x))
//#else
//#define stat(x,y) stat64((x), (y))
//#define fopen(x, y)  fopen64((x), (y))
//#define fseek(x, y, z) fseeko64((x), (y), (z))
//#define ftell(x)   ftello64((x))
//#endif


#endif   /* define __GSF_FT_H__ */

