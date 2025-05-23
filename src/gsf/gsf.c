/********************************************************************
 *
 * Module Name : GSF
 *
 * Author/Date : J. S. Byrne / 3 May 1994
 *
 * Description : This source file contains the GSF library entry point
 *  functions for accessing multibeam sonar data in a generic byte stream
 *  format.  Each record in these binary files contains an ID and a size,
 *  and these two values are used to read and decode the rest of the data
 *  record.  Data records are read/written one at a time, in a sequential
 *  manner.  Refer to the DoDBL Generic Sensor format design documentation
 *  for a more detailed verbal description of the data format.
 *
 * Restrictions/Limitations :
 * 1) This library assumes the host computer uses the ASCII character set.
 * 2) This library assumes that the type short is 16 bits, and that the type
 *    int is 32 bits.
 *
 * Change Descriptions :
 * who  when      what
 * ---  ----      ----
 * jsb  10-25-94  Added gsfOpenBuffered function call, and call to setvbuf
 *                in gsfOpen to deal with high data rate multibeam data.
 * jsb  08-14-95  Direct and sequential access now work through common
 *                gsfRead and gsfWrite API. All pointers to dynamically
 *                allocated memory are now maintained by the library.
 *                Call this version "GSF-v01.01".
 * jsb  11/01/95  Completed modifications to indexing to support increase in
 *                GSF file size after initial index file creation.  The size
 *                of the file is now stored in the index file header. Index
 *                files without the expected header are recreated on the first
 *                open. This is still version GSF-v01.01. Also added a unique
 *                sensor specific subrecord for Simrad em1000.
 * jsb  12/22/95  Added gsfGetMBParams, gsfPutMBParams, gsfIsStarboardPing,
 *                and gsfGetSwathBathyBeamWidths. Also added GSF_APPEND as
 *                a file access mode, and modifed GSF_CREATE access mode so
 *                that files can be updated (read and written). This is GSF
 *                library version GSF-v01.02.
 * fd   04/15/96  Corrected the internals of gsfIsStarboardPing
 * hem  08/20/96  Added support for single beam pings; added gsfStringError;
 *                fixed 4 byte boundary padding.  This is GSF library
 *                version GSF-v1.03.
 * jsb  10/04/96  Changed fopen argument from "wb" to "a+b" for the GSF_APPEND
 *                access mode.  Also added logic to set file pointer to top prior
 *                to trying to read the GSF header record in gsfOpen/gsfOpenBuffered
 *                when the file access mode is GSF_APPEND.  Replaced use of
 *                numOpenFiles with *handle as argument to gsfRead and gsfWrite
 *                within gsfOpen and gsfOpenBuffered.  This repairs problems which
 *                can occur when a single application is accessing multiple files.
 * jsb  04/18/97  Added GSF version dependancy on approach to padding records out
 *                to four byte boundary. This is required in order to support the
 *                update access modes for versions prior to 1.03.  Replaced use of
 *                fgetpos, fsetpos with ftell, fseek.  This was done so that we can
 *                compair the previous_record field of the file table with addresses
 *                from an index file. Modified gsfStringError so that there is a single
 *                return statement, this was done to eliminate "statement not reached"
 *                compile warnings.
 * bac  10/27/97  Added a case in gsfGetSwathBathyBeamWidths for the Sea Beam 2112/36.
 * dwc 1/9/98     Added a case in gsfGetSwathBathyBeamWidths for the Elac Bottomchart MkII.
 * bac  03/15/98  Added an array subrecord for signal to noise ratio.
 * bac  03/15/98  Added an array subrecord for beam angle forward.
 * jsb  09/28/98  Added support for new navigation error record. Modified gsfPrintError
 *                 to use gsfStringError.
 * wkm  04/01/99  Added case for CmpSass (Compressed SASS) data to set beam widths to 1.0.
 * jsb  04/02/99  Added support for EM3000 series sonar systems.
 * jsb  07/20/99  Completed work on GSF version 1.08.  Added new functions gsfGetSwathBathyArrayMinMax,
 *                and gsfLoadDepthScaleFactorAutoOffset in support of signed depth.
 *                This release addresses the following CRs: GSF-99-002, GSF-99-006, GSF-99-007,
 *                GSF-99-008, GSF-99-009, GSF-99-010, GSF-99-011, GSF-99-012,
 * jsb  04/05/00  Updated so that an application can work with up to GSF_MAX_OPEN_FILES at
 *                a time.  Prior to these updates an application could only open (GSF_MAX_OPEN_FILES-1)
 *                files at a time. Also updated gsfOpen and gsfOpenBuffered to return the correct
 *                error code if a failure occures reading the file header.
 * bac 07-18-01   Made modifications for use with C++ code.  The typedef for each sensor
 *                specific structure has been modified to have a different name than the
 *                element of the SensorSpecific union.  Also removed the useage of C++
 *                reserved words "class" and "operator".  These modifications will potentially
 *                require some changes to application code. Added support for the Reson 8100 series of sonars.
 * bac 10-12-01   Added a new attitude record definition.  The attitude record provides a method for
 *                logging full time-series attitude measurements in the GSF file, instead of attitude
 *                samples only at ping time.  Each attitude record contains arrays of attitude
 *                measurements for time, roll, pitch, heave and heading.  The number of measurements
 *                is user-definable, but because of the way in which measurement times are stored, a
 *                single attitude record should never contain more than sixty seconds worth of
 *                data.
 * bac 11-09-01   Added motion sensor offsets to the gsfMBOffsets structure.  Added support for these
 *                new offsets in the gsfPutMBParams and gsfGetMBParams functions, so these offsets are
 *                encoded in the process_parameters record.
 * jsb 01-21-02   If the fread doesn't complete, rewind the file to the beginning of the current
 *                record, set gsfError to END_OF_FILE, and return -1.  Removed variables that were
 *                not used, fixed return code and gsfError for default case block in gsfGetBeamWidths,
 *                and update strncpy in gsfSetParam and gsfCopyRecords to ensure that the terminating
 *                NULL is copied to the target pointer.
 * bac 06-19-03   Added support for bathymetric receive beam time series intensity data (i.e., Simrad
 *                "Seabed image" and Reson "snippets").  Inlcluded RWL updates of 12-19-02 for adding
 *                sensor-specific singlebeam information to the MB sensor specific subrecords.
 * bac 12-28-04   Added support for EM3000D, EM3002, and EM3002D in gsfGetSwathBathyBeamWidths.  Updated
 *                gsfLoadDepthScaleFactorAutoOffset to vary the offset interval based on precision.
 *                Updated gsfFree to free and set to NULL the quality_flags, vertical_error, horizontal_error,
 *                and brb_inten arrays.  Added vertical_error and horizontal_error processing to gsfCopyRecords.
 * bac 06-28-06   Updated gsfIsStarboardPing to work with EM3000D and EM3002D.  Updated gsfCopyRecords
 *                to copy the hv_nav_error record.  Added  for EM121A data received via Kongsberg SIS.
 *                Replaced references to long types with int types, for compilation on 64-bit architectures.
 * jsb 11-06-07   Updates to utilize the subrecord size in termining the field size for the array subrecords
 *                that support more than one field size.  Also replaced use of strstr with strcmp in gsfGetMBParams
 *                to resolve potential problem where one keyword name may be fully contained in another.
 * DHG 2008/12/18 Add "PLATFORM_TYPE" to Processing Parameters for AUV vs Surface Ship discrimination.
 * mab 02-01-09   Updates to support Reson 7125. Added new subrecord IDs and subrecord definitions for Kongsberg
 *                sonar systems where TWTT and angle are populated from raw range and beam angle datagram. Added
 *                new subrecord definition for EM2000.  Bug fixes in gsfOpen and gsfPercent.
 * clb 05-17-11   Added depth sensor and receiver array offsets to the gsfGetMBParams() and gsfPutMBParams()
 * clb 10-04-11   Added check in gsfUnpackStream() for a partial record at the end of the file
 * clb 10-17-11   Handle all the error processing in gsfOpen() and gsfOpenBuffered() consistently
 * clb 11-09-aa   Added validity checks in gsfPutMBParams(); initialize param structure in gsfGetMBParams();
 *                added gsfInitializeMBParams(); validate handles in functions that use them
 *
 *
 * Classification : Unclassified
 *
 * References : DoDBL Generic Sensor Format Sept. 30, 1993
 *
 *
 * copyright 2019 Leidos, Inc.
 * There is no charge to use the library, and it may be accessed at:
 * https://www.leidos.com/products/ocean-marine#gsf.
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

/* standard c library includes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>

/* rely on the network type definitions of (u_short, and u_int) */
#if !defined WIN32 && !defined WIN64
#include <netinet/in.h>
#else
#include <winsock.h>
#endif

/* GSF library interface description */
#include "gsf.h"

/* Get the prototypes for the GSF encode and GSF decode functions */
#include "gsf_ft.h"
#include "gsf_enc.h"
#include "gsf_dec.h"
#include "gsf_indx.h"

/* Macros required for this module */
#ifndef USE_DEFAULT_FILE_FUNCTIONS // Added USE_DEFAULT_FILE_FUNCTIONS test for MB-System DW Caress 21 March 2017

#undef fseek
#undef ftell
#if (defined _WIN32) && (defined _MSC_VER)
#define fseek(x, y, z) _fseeki64((x), (y), (z))
#define ftell(x)   _ftelli64((x))
#else  // Linux, MingW, MacOS
#undef fopen
#define fopen(x, y)  fopen64((x), (y))
#define fseek(x, y, z) fseeko64((x), (y), (z))
#define ftell(x)   ftello64((x))
#endif

#endif

#define GSF_FILL_SIZE 8                   /* GSF packaging with no checksum */
#define GSF_FILL_SIZE_CHECKSUM 12         /* GSF packaging with checksum */
#define GSF_STREAM_BUF_SIZE 8192          /* GSF default stream buffer size */
#define GSF_UNKNOWN_PARAM_TEXT "UNKNWN"   /* Flag value for unknown parameter value */

#define GSF_MAX_PARAM    999999          /* used in gsfPutMBParams() to prevent bad values */
#define GSF_MIN_PARAM   -999999

/* JSB 07/15/99 Added these macros to support new gsfGetSwathBathyArrayMinMax function */
#define GSF_U_CHAR_MIN            (0.0)
#define GSF_U_CHAR_MAX          (255.0)
#define GSF_S_CHAR_MIN         (-128.0)
#define GSF_S_CHAR_MAX         (+127.0)
#define GSF_U_SHORT_MIN           (0.0)
#define GSF_U_SHORT_MAX       (65535.0)
#define GSF_S_SHORT_MIN      (-32768.0)
#define GSF_S_SHORT_MAX       (32767.0)
#define GSF_U_INT_MIN             (0.0)
#define GSF_U_INT_MAX    (4294967295.0)
#define GSF_S_INT_MIN   (-2147483648.0)
#define GSF_S_INT_MAX    (2147483647.0)

/* Static Global data for this module */
static unsigned char streamBuff[GSF_MAX_RECORD_SIZE];
static int      numOpenFiles;
static GSF_FILE_TABLE gsfFileTable[GSF_MAX_OPEN_FILES];

/* Global external data defined in this module */
int             gsfError;       /* used to report most recent error */

/* Static functions used, but not exported from this source file */
static gsfuLong gsfChecksum(unsigned char *buff, unsigned int num_bytes);
static int      gsfSeekRecord(int handle, gsfDataID *id);
static int      gsfUnpackStream (int handle, int desiredRecord, gsfDataID *dataID, gsfRecords *rptr, unsigned char *buf, int max_size);
static int      gsfSetParam(int handle, int index, const char *val, gsfRecords *rec);
static int      gsfNumberParams(const char *param);


/********************************************************************
 *
 * Function Name : gsfStat
 *
 * Description : This function attempts to stat a GSF file.
 *               Supports 64 bit file size.
 *
 * Inputs :
 *  filename = a fully qualified path to the gsf file
 *  sz       = pointer to an 8 byte long long for return
 *             of the GSF file size from the stat64 system call.
 *
 * Returns :
 *  This funciton returns zero if successful, or -1 if an error occured.
 *
 * Error Conditions :
 *     GSF_FOPEN_ERROR
 *     GSF_UNRECOGNIZED_FILE
 *
 ********************************************************************/
int
gsfStat (const char *filename, long long *sz)
{
    int rc;

    gsfError = 0;

    if (sz == NULL)
    {
        gsfError = GSF_FOPEN_ERROR;
        return -1;
    }

#ifdef USE_DEFAULT_FILE_FUNCTIONS // Added USE_DEFAULT_FILE_FUNCTIONS test for MB-System DW Caress 21 March 2017
	struct stat stbuf;
	rc = stat(filename, &stbuf);
#else

#if (defined __WINDOWS__) || (defined __MINGW32__)
    struct _stati64    stbuf;
    rc = _stati64(filename, &stbuf);
#else
    struct stat64      stbuf;
    rc = stat64(filename, &stbuf);
#endif

#endif

    if (!rc)
    {
        *sz = stbuf.st_size;
    }
    else
    {
        gsfError = GSF_UNRECOGNIZED_FILE;
    }

    return rc;
}


/********************************************************************
 *
 * Function Name : gsfOpen
 *
 * Description : This function attempts to open a GSF data file.  If the
 *   file exists and is opened readonly or update, the GSF header is read
 *   to confirm that this is a GSF data file.  If the file is opened create,
 *   the GSF header containing the version number of the software library is
 *   written into the header.  This function passes an integer handle back to
 *   the calling application.  The handle is used for all further access to the
 *   file. gsfOpen explicitly sets stream buffering to the value specified
 *   by GSF_STREAM_BUF_SIZE.  The internal file table is searched for an
 *   available entry whose name matches that specified in the argument list, if
 *   no match is found, then the first available entry is used.  Up to
 *   GSF_MAX_OPEN_FILES files may be open by an application at a time.
 *
 * Inputs :
 *   filename = a fully qualified path to the GSF file to open
 *   mode may have the following values:
 *     GSF_READONLY = open an existing file for read only access
 *     GSF_UPDATE   = open an existing file for reading and writing
 *     GSF_CREATE   = create a new GSF file
 *     GSF_READONLY_INDEX = open an existing file for read only access with index
 *     GSF_UPDATE_INDEX   = open an existing file for reading and writing with index
 *   handle = a pointer to an integer to be assigned a handle which will be
 *     referenced for all future file access.
 *
 * Returns :
 *   This function returns zero if successful, or -1 if an error occurred.
 *
 * Error Conditions :
 *   GSF_BAD_ACCESS_MODE
 *   GSF_TOO_MANY_OPEN_FILES
 *   GSF_FOPEN_ERROR
 *   GSF_SETVBUF_ERROR
 *   GSF_UNRECOGNIZED_FILE
 *   GSF_READ_ERROR
 *   GSF_FLUSH_ERROR
 *   GSF_FILE_SEEK_ERROR
 *   GSF_HEADER_RECORD_ENCODE_FAILED
 *   GSF_HEADER_RECORD_DECODE_FAILED
 *   GSF_INDEX_FILE_OPEN_ERROR
 *
 ********************************************************************/
int
gsfOpen(const char *filename, const int mode, int *handle)
{

    return gsfOpenBuffered (filename, mode, handle, GSF_STREAM_BUF_SIZE);
}

/********************************************************************
 *
 * Function Name : gsfOpenBuffered
 *
 * Description : This function attempts to open a GSF data file.  If the
 *   file exists and is opened readonly or update, the GSF header is read
 *   to confirm that this is a GSF data file.  If the file is opened create,
 *   the GSF header containing the version number of the software library is
 *   written into the header.  This function passes an integer handle back to
 *   the calling application.  The handle is used for all further access to the
 *   file. gsfOpenBufferd explicitly sets stream buffering to the value
 *   specified by the buf_size argument. The internal file table is searched
 *   for an available entry whose name matches that specified in the argument
 *   list, if no match is found, then the first available entry is used.  Up
 *   to GSF_MAX_OPEN_FILES files may be open by an application at a time.
 *   gsfOpenBuffered performs identical processing to gsfOpen, except here,
 *   the caller is allowed to explicitly set the standard system library level
 *   I/O buffer size.
 *
 * Inputs :
 *   filename = a fully qualified path to the GSF file to open
 *   mode may have the following values:
 *     GSF_READONLY = open an existing file for read only access
 *     GSF_UPDATE   = open an existing file for reading and writing
 *     GSF_CREATE   = create a new GSF file
 *     GSF_READONLY_INDEX = open an existing file for read only access with index
 *     GSF_UPDATE_INDEX   = open an existing file for reading an writing with index
 *  handle = a pointer to an integer to be assigned a handle which will be
 *     reference for all future file access.
 *  buf_size = an integer buffer size in bytes.
 *
 * Returns :
 *   This function returns zero if successful, or -1 if an error occurred.
 *
 * Error Conditions :
 *   GSF_BAD_ACCESS_MODE
 *   GSF_TOO_MANY_OPEN_FILES
 *   GSF_FOPEN_ERROR
 *   GSF_SETVBUF_ERROR
 *   GSF_UNRECOGNIZED_FILE
 *   GSF_READ_ERROR
 *   GSF_FLUSH_ERROR
 *   GSF_FILE_SEEK_ERROR
 *   GSF_HEADER_RECORD_ENCODE_FAILED
 *   GSF_HEADER_RECORD_DECODE_FAILED
 *   GSF_INDEX_FILE_OPEN_ERROR
 *
 ********************************************************************/
int
gsfOpenBuffered(const char *filename, const int mode, int *handle, int buf_size)
{
    const char     *access_mode;
    int             fileTableIndex;
    int             length;
    int             headerSize;
    int             ret;
    long long       stsize;
    gsfDataID       id;
    FILE           *fp;

    /* Clear the gsfError value each time a new file is opened */
    gsfError = 0;

    /* Make sure we don't inadvertently send a valid handle back. */
    *handle = 0;

    /* Get the desired file access mode. */
    switch (mode)
    {
        case GSF_CREATE:
            access_mode = "w+b";
            break;

        case GSF_READONLY:
            access_mode = "rb";
            break;

        case GSF_UPDATE:
            access_mode = "r+b";
            break;

        case GSF_READONLY_INDEX:
            access_mode = "rb";
            break;

        case GSF_UPDATE_INDEX:
            access_mode = "r+b";
            break;

        case GSF_APPEND:
            access_mode = "a+b";
            break;

        default:
            gsfError = GSF_BAD_ACCESS_MODE;
            return (-1);
    }

    /* Check the number of files currently opened. */
    if (numOpenFiles >= GSF_MAX_OPEN_FILES)
    {
        gsfError = GSF_TOO_MANY_OPEN_FILES;
        return (-1);
    }

    /* Try to open this file */
    if ((fp = fopen(filename, access_mode)) == (FILE *) NULL)
    {
        gsfError = GSF_FOPEN_ERROR;
        return (-1);
    }

    /* The file was successfully opened, load the GSF file table structure by
     * searching the GSF file table for the caller's filename.  This is done
     * so that the same file table slot may be re-used.  Applications which
     * want their file closed frequently, such as real-time data collection
     * programs may do this to assure data integrity, and it makes sense
     * to reuse the file table slot they occupied from a previous call to
     * gsfOpen, so that the ping scale factors don't have to be reset except
     * when a new file is created.
     */
    numOpenFiles++;
    length = strlen (filename);
    if (length >= sizeof(gsfFileTable[0].file_name))
    {
        length = sizeof(gsfFileTable[0].file_name) - 1;
    }
    for (fileTableIndex=0; fileTableIndex<GSF_MAX_OPEN_FILES; fileTableIndex++)
    {
        if ((memcmp(gsfFileTable[fileTableIndex].file_name, filename, length) == 0) &&
            (gsfFileTable[fileTableIndex].occupied == 0))
        {
            break;
        }
    }

    /* If no filename match was found then use the first available slot. */
    if (fileTableIndex == GSF_MAX_OPEN_FILES)
    {
        for (fileTableIndex=0; fileTableIndex<GSF_MAX_OPEN_FILES; fileTableIndex++)
        {
            if (gsfFileTable[fileTableIndex].occupied == 0)
            {
                strncpy (gsfFileTable[fileTableIndex].file_name, filename, sizeof(gsfFileTable[fileTableIndex].file_name));
                /* This is the first open slot for this file, so clear the pointers to dynamic memory. */
                gsfFree (&gsfFileTable[fileTableIndex].rec);
                break;
            }
        }
    }
    
    /* if still no free table is found error out */
    if (fileTableIndex == GSF_MAX_OPEN_FILES)
    {
        gsfError = GSF_TOO_MANY_OPEN_FILES;
        fclose(fp);
        return (-1);
    }

    gsfFileTable[fileTableIndex].fp = fp;
    gsfFileTable[fileTableIndex].buf_size = buf_size;
    gsfFileTable[fileTableIndex].occupied = 1;
    *handle = fileTableIndex + 1;

    /* Set the desired buffer size. */
    if (setvbuf(fp, NULL, _IOFBF, buf_size))
    {
        gsfClose ((int) *handle);
        gsfError = GSF_SETVBUF_ERROR;
        *handle = 0;
        return (-1);
    }

    /* Use stat to get the size of this file. File size is used by gsfPercent */
    if (gsfStat (filename, &stsize))
    {
        gsfClose (*handle);
        gsfError = GSF_READ_ERROR;
        *handle = 0;
        return(-1);
    }
    gsfFileTable[fileTableIndex].file_size = stsize;

    /* If this file was just created, (i.e., it has a size of 0 bytes) then
     * write the GSF file header record.  Also, set a flag to indicate
     * that the ping scale factors need to be written with the next swath
     * bathymetry ping record.
     */
    if (stsize == 0)
    {
        gsfFileTable[fileTableIndex].scales_read = 1;

        /* Write the GSF file header to the file. */
        id.checksumFlag = 0;
        id.reserved = 0;
        id.recordID = GSF_RECORD_HEADER;
        strncpy (gsfFileTable[fileTableIndex].rec.header.version, GSF_VERSION, GSF_VERSION_SIZE);
        gsfFileTable[fileTableIndex].rec.header.version[GSF_VERSION_SIZE-1] = 0;

        headerSize = gsfWrite (*handle, &id, &gsfFileTable[fileTableIndex].rec);

        if (headerSize < 0)
        {
            /* gsfError set in gsfWrite, save it since gsfClose can change it. */
			ret = gsfError;
            gsfClose (*handle);
            gsfError = ret;
            *handle = 0;
            return (-1);
        }
        gsfFileTable[fileTableIndex].bufferedBytes += headerSize;

        /* Flush this record to disk so that the file size will be non-zero
         * on the next call to gsfOpen.
         */
        if (fflush (gsfFileTable[fileTableIndex].fp))
        {
            gsfClose (*handle);
            gsfError = GSF_FLUSH_ERROR;
            *handle = 0;
            return(-1);
        }
    }
    else
    {
        /* Read the GSF header, if the access mode is append, we need to
         * seek back to the top of the file.
         */
        if (mode == GSF_APPEND)
        {
            if (fseek(gsfFileTable[fileTableIndex].fp, 0, SEEK_SET))
            {
                gsfClose (*handle);
                gsfError = GSF_FILE_SEEK_ERROR;
                *handle = 0;
                return (-1);
            }
        }
        /* Read the GSF header */
        headerSize = gsfRead(*handle, GSF_NEXT_RECORD, &id, &gsfFileTable[fileTableIndex].rec, NULL, 0);
        /* JSB 04/05/00 Updated to return correct error code */
        if (headerSize < 0)
        {
            gsfClose (*handle);
            gsfError = GSF_HEADER_RECORD_DECODE_FAILED;
            *handle = 0;
            return (-1);
        }
        /* JSB end of updates from 04/05/00 */
        if (!strstr(gsfFileTable[fileTableIndex].rec.header.version, "GSF-"))
        {
            gsfClose (*handle);
            gsfError = GSF_UNRECOGNIZED_FILE;
            *handle = 0;
            return (-1);
        }
        /* If the mode is append, then seek back to the end of the file. */
        if (mode == GSF_APPEND)
        {
            if (fseek(gsfFileTable[fileTableIndex].fp, 0, SEEK_END))
            {
                gsfClose (*handle);
                gsfError = GSF_FILE_SEEK_ERROR;
                *handle = 0;
                return (-1);
            }
        }
    }

    /* jsb 04/16/97 Save the GSF version ID into the file table */
    ret = sscanf (gsfFileTable[fileTableIndex].rec.header.version, "GSF-v%d.%d",
        &gsfFileTable[fileTableIndex].major_version_number,
        &gsfFileTable[fileTableIndex].minor_version_number);
    if (ret != 2)
    {
        gsfClose (*handle);
        gsfError = GSF_UNRECOGNIZED_FILE;
        *handle = 0;
        return (-1);
    }

    /*  Set the update flag if needed. This is used to force a call to fflush
     *  between read and write operations on files opened for update.
     */
    if ((mode == GSF_UPDATE) ||
        (mode == GSF_UPDATE_INDEX) ||
        (mode == GSF_CREATE))
    {
        gsfFileTable[fileTableIndex].update_flag = 1;
    }
    else
    {
        gsfFileTable[fileTableIndex].update_flag = 0;
    }

    /* Set the index flag and open the index file if needed. */
    if ((mode == GSF_READONLY_INDEX) || (mode == GSF_UPDATE_INDEX))
    {
        gsfFileTable[fileTableIndex].direct_access = 1;
        if (gsfOpenIndex (filename, *handle, &gsfFileTable[fileTableIndex]) == -1)
        {
            gsfFileTable[fileTableIndex].direct_access = 0;
            gsfClose (*handle);
			gsfError = GSF_INDEX_FILE_OPEN_ERROR;
            *handle = 0;
            return (-1);
        }

        /* Move the file pointer back to the first record past the GSF file header. This
         * is required since we will have to read the entire file to create the index.
         */
        if (fseek(gsfFileTable[fileTableIndex].fp, headerSize, SEEK_SET))
        {
            gsfClose (*handle);
            gsfError = GSF_FILE_SEEK_ERROR;
            *handle = 0;
            return (-1);
        }
    }
    else
    {
        gsfFileTable[fileTableIndex].direct_access = 0;
    }

    /* Save the file access mode. */
    switch (mode)
    {
        case GSF_CREATE:
            gsfFileTable[fileTableIndex].access_mode = GSF_CREATE;
            break;

        case GSF_READONLY:
            gsfFileTable[fileTableIndex].access_mode = GSF_READONLY;
            break;

        case GSF_UPDATE:
            gsfFileTable[fileTableIndex].access_mode = GSF_UPDATE;
            break;

        case GSF_READONLY_INDEX:
            gsfFileTable[fileTableIndex].access_mode = GSF_READONLY_INDEX;
            break;

        case GSF_UPDATE_INDEX:
            gsfFileTable[fileTableIndex].access_mode = GSF_UPDATE_INDEX;
            break;

        case GSF_APPEND:
            gsfFileTable[fileTableIndex].access_mode = GSF_APPEND;
            break;

        default:
            gsfClose (*handle);
            gsfError = GSF_BAD_ACCESS_MODE;
            *handle = 0;
            return (-1);
    }

    return (0);
}

/********************************************************************
 *
 * Function Name : gsfClose
 *
 * Description : This function closes a GSF file previously opened
 *   using gsfOpen.
 *
 * Inputs :
 *   handle = the handle of the GSF file to be closed.
 *
 * Returns :
 *   This function returns zero if successful, or -1 if an error occurred.
 *
 * Error Conditions :
 *    GSF_BAD_FILE_HANDLE
 *    GSF_FILE_CLOSE_ERROR
 *
 ********************************************************************/

int
gsfClose(const int handle)
{
    int ret = 0;

    /* JSB 04/05/00 replaced ">=" with ">" */
    if ((handle < 1) || (handle > GSF_MAX_OPEN_FILES))
    {
        gsfError = GSF_BAD_FILE_HANDLE;
        return (-1);
    }

    if (gsfFileTable[handle -1].direct_access)
    {
        if (gsfCloseIndex (&gsfFileTable[handle - 1]))
        {
            gsfError = GSF_FILE_CLOSE_ERROR;
            ret = -1;
        }
    }

    if (fclose(gsfFileTable[handle - 1].fp))
    {
        gsfError = GSF_FILE_CLOSE_ERROR;
        ret = -1;
    }

    numOpenFiles--;

    /* jsb 05/14/97 Clear the contents of the gsfFileTable fields. We don't
     * want to clear the filename, this allows a performance improvement for
     * programs which use append to log GSF files. (ie: data acquisition).
     */
    gsfFileTable[handle-1].major_version_number = 0;
    gsfFileTable[handle-1].minor_version_number = 0;
    gsfFileTable[handle-1].file_size = 0;
    gsfFileTable[handle-1].previous_record = 0;
    gsfFileTable[handle-1].buf_size = 0;
    gsfFileTable[handle-1].bufferedBytes = 0;
    gsfFileTable[handle-1].occupied = 0;
    gsfFileTable[handle-1].update_flag = 0;
    gsfFileTable[handle-1].direct_access = 0;
    gsfFileTable[handle-1].read_write_flag = 0;
    gsfFileTable[handle-1].scales_read = 0;
    gsfFileTable[handle-1].access_mode = 0;
    gsfFileTable[handle-1].last_record_type = 0;

    /* Clear the contents of the index data table. */
    if (gsfFileTable[handle-1].index_data.scale_factor_addr)
    {
        free(gsfFileTable[handle-1].index_data.scale_factor_addr);
    }
    memset (&gsfFileTable[handle-1].index_data, 0, sizeof(gsfFileTable[handle-1].index_data));

    /* Clear the necessary fields of the gsfRecords data structure */
    memset(&gsfFileTable[handle-1].rec.header, 0, sizeof(gsfHeader));

    return (ret);
}

/********************************************************************
 *
 * Function Name : gsfSeek
 *
 * Description : This function may be used to move the file pointer
 *   for a previously opened GSF file.
 *
 * Inputs :
 *   handle = the integer handle returned from gsfOpen
 *   option = the desired action for moving the file pointer, where:
 *    GSF_REWIND, move pointer to first record in the file.
 *    GSF_END_OF_FILE, move pointer to the end of the file.
 *    GSF_PREVIOUS_RECORD, backup to the beginning of the record just
 *     written or just read.
 *
 * Returns :
 *   This function returns zero if successful, or -1 if an error occurred.
 *
 * Error Conditions :
 *   GSF_BAD_FILE_HANDLE
 *   GSF_FLUSH_ERROR
 *   GSF_FILE_SEEK_ERROR
 *   GSF_BAD_SEEK_OPTION
 *
 ********************************************************************/

int
gsfSeek(int handle, int option)
{
    /* JSB 04/05/00 replaced ">=" with ">" */
    if ((handle < 1) || (handle > GSF_MAX_OPEN_FILES))
    {
        gsfError = GSF_BAD_FILE_HANDLE;
        return (-1);
    }

    switch (option)
    {
        case GSF_REWIND:
            /* If the last operation was a write then we need to fflush */
            if (gsfFileTable[handle - 1].read_write_flag == LAST_OP_WRITE)
            {
                if (fflush (gsfFileTable[handle - 1].fp))
                {
                    gsfError = GSF_FLUSH_ERROR;
                    return(-1);
                }
            }
            gsfFileTable[handle - 1].read_write_flag = LAST_OP_FLUSH;

            if (fseek(gsfFileTable[handle - 1].fp, 0, SEEK_SET))
            {
                gsfError = GSF_FILE_SEEK_ERROR;
                return (-1);
            }
            break;

        case GSF_END_OF_FILE:
            /* If the last operation was a write then we need to fflush */
            if (gsfFileTable[handle - 1].read_write_flag == LAST_OP_WRITE)
            {
                if (fflush (gsfFileTable[handle - 1].fp))
                {
                    gsfError = GSF_FLUSH_ERROR;
                    return(-1);
                }
            }
            gsfFileTable[handle - 1].read_write_flag = LAST_OP_FLUSH;

            if (fseek(gsfFileTable[handle - 1].fp, 0, SEEK_END))
            {
                gsfError = GSF_FILE_SEEK_ERROR;
                return (-1);
            }
            break;

        case GSF_PREVIOUS_RECORD:
            if (fseek(gsfFileTable[handle - 1].fp, gsfFileTable[handle - 1].previous_record, SEEK_SET))
            {
                gsfError = GSF_FILE_SEEK_ERROR;
                return (-1);
            }
            break;

        default:
            gsfError = GSF_BAD_SEEK_OPTION;
            return (-1);
    }

    return (0);
}

/********************************************************************
 *
 * Function Name : gsfRead
 *
 * Description : gsfRead supports both direct and sequential access. If the
 *   file is opened for sequential access, this function reads the desired
 *   record from the GSF data file specified by handle.  The "desiredRecord"
 *   argument may be set to GSF_NEXT_RECORD to read the next record in the
 *   data file, or "desiredRecord" record may be set to specify the record
 *   of interest, in which case the file will be read, skipping past
 *   intermediate records until the desired record is found.  When the desired
 *   record is found, it is read and then decoded from external to internal
 *   form. If the optional checksum is found with the data it will be verified.
 *   All of the fields of the gsfDataID structure, with the exception of the
 *   record_number field will be loaded with the values contained in the GSF
 *   record byte stream.  The record_number field will be undefined.  The
 *   stream and max_size arguments are normally set to NULL, unless the
 *   calling application is interested in a copy of the GSF byte stream.
 *
 *   If the file is opened for direct access, then the combination of the
 *   recordID and the record_number fields of the dataID structure are used
 *   to uniquely identify the record of interest.  The address for this record
 *   is retrieved from the index file, which was created on a previous call
 *   to gsfOpen or gsfOpenBuffered.  If the record of interest is a ping record
 *   for which we need to retrieve new scale factors, then the ping record
 *   containing the scale factors needed is read first, and then the ping
 *   record of interest is read.  Direct access applications should set the
 *   desiredRecord argument equal to the recordID field in the gsfDataID
 *   structure.
 *
 * Inputs :
 *    handle = the handle to the file as provided by gsfOpen
 *    desiredRecord = the desired record or GSF_NEXT_RECORD
 *    dataID = a pointer to a gsfDataID structure to be populated for the
 *             input record.
 *    rptr = a pointer to a gsfRecords structure to be populated with the
 *           data from the input record in internal form.
 *    buf = an optional pointer to caller memory to be populated with a copy
 *          of the GSF byte stream for this record.
 *    max_size = an optional maximum size to copy into buf
 *
 * Returns :
 *   This function returns the number of bytes read if successful,
 *   or -1 if an error occurred.
 *
 * Error Conditions :
 *   GSF_BAD_FILE_HANDLE
 *   GSF_FILE_SEEK_ERROR
 *   GSF_FLUSH_ERROR
 *   GSF_READ_TO_END_OF_FILE
 *   GSF_READ_ERROR
 *   GSF_RECORD_SIZE_ERROR
 *   GSF_INSUFFICIENT_SIZE
 *   GSF_CHECKSUM_FAILURE
 *   GSF_UNRECOGNIZED_RECORD_ID
 *   GSF_HEADER_RECORD_DECODE_FAILED
 *   GSF_SVP_RECORD_DECODE_FAILED
 *   GSF_PROCESS_PARAM_RECORD_DECODE_FAILED
 *   GSF_SENSOR_PARAM_RECORD_DECODE_FAILED
 *   GSF_COMMENT_RECORD_DECODE_FAILED
 *   GSF_HISTORY_RECORD_DECODE_FAILED
 *   GSF_NAV_ERROR_RECORD_DECODE_FAILED
 *   GSF_UNRECOGNIZED_SUBRECORD_ID
 *   GSF_INVALID_NUM_BEAMS
 *   GSF_MEMORY_ALLOCATION_FAILED
 *   GSF_TOO_MANY_ARRAY_SUBRECORDS
 *   GSF_CANNOT_REPRESENT_PRECISION
 *   GSF_ILLEGAL_SCALE_FACTOR_MULTIPLIER
 *   GSF_QUALITY_FLAGS_DECODE_ERROR
 *
 ********************************************************************/

int
gsfRead(int handle, int desiredRecord, gsfDataID *dataID, gsfRecords *rptr, unsigned char *buf, int max_size)
{
    int             ret;
    gsfDataID       tmpID;

    /* Clear gsfError before each read */
    gsfError = 0;

    /* JSB 04/05/00 replaced ">=" with ">" */
    if ((handle < 1) || (handle > GSF_MAX_OPEN_FILES))
    {
        gsfError = GSF_BAD_FILE_HANDLE;
        return (-1);
    }

    /* If this file is opened for direct access, then move the file pointer
     * to the record which the caller wants to read. Note that there is no
     * file re-positioning to be done if the caller wants the next record.
     */
    if ((gsfFileTable[handle - 1].direct_access) &&
        (desiredRecord != GSF_NEXT_RECORD))
    {
        memset(&tmpID, 0, sizeof(tmpID));
        tmpID.recordID = (unsigned int)desiredRecord;
        tmpID.record_number = dataID->record_number;

        ret = gsfSeekRecord(handle, &tmpID);
        if (ret < 0)
        {
            /* gsfError is set in gsfSeekRecord */
            return (-1);
        }
    }

    ret = gsfUnpackStream (handle, desiredRecord, dataID, rptr, buf, max_size);

    gsfFileTable[handle - 1].last_record_type = dataID->recordID;

    return (ret);
}

/********************************************************************
 *
 * Function Name : gsfUnpackStream
 *
 * Description : gsfUnpackStream is a static function (not available to
 *   application programs) which is used by gsfRead to read and decode
 *   GSF records. It performs the bulk of the processing required to read
 *   a GSF record.  This processing exists as a function separate from
 *   gsfRead since it is required both by gsfRead and by gsfSeekRecord.
 *   gsfUnpackStream is used by gsfSeekRecord to read a ping record with
 *   scale factors, which is required to support direct access.
 *
 * Inputs :
 *    handle = the handle to the file as provided by gsfOpen
 *    desiredRecord = the desired record or GSF_NEXT_RECORD
 *    dataID = a pointer to a gsfDataID structure to be populated for the
 *             input record.
 *    rptr = a pointer to a gsfRecords structure to be populated with the
 *           data from the input record in internal form.
 *    buf = an optional pointer to caller memory to be populated with a copy
 *          of the GSF byte stream for this record.
 *    max_size = an optional maximum size to copy into buf
 *
 * Returns :
 *  This function returns the number of bytes read if successful,
 *  or -1 if an error occured.
 *
 * Error Conditions :
 *   GSF_FILE_SEEK_ERROR
 *   GSF_FLUSH_ERROR
 *   GSF_READ_TO_END_OF_FILE
 *   GSF_RECORD_SIZE_ERROR
 *   GSF_INSUFFICIENT_SIZE
 *   GSF_CHECKSUM_FAILURE
 *   GSF_HEADER_RECORD_DECODE_FAILED
 *   GSF_SVP_RECORD_DECODE_FAILED
 *   GSF_PROCESS_PARAM_RECORD_DECODE_FAILED
 *   GSF_SENSOR_PARAM_RECORD_DECODE_FAILED
 *   GSF_COMMENT_RECORD_DECODE_FAILED
 *   GSF_HISTORY_RECORD_DECODE_FAILED
 *   GSF_NAV_ERROR_RECORD_DECODE_FAILED
 *   GSF_UNRECOGNIZED_SUBRECORD_ID
 *   GSF_INVALID_NUM_BEAMS
 *   GSF_MEMORY_ALLOCATION_FAILED
 *   GSF_TOO_MANY_ARRAY_SUBRECORDS
 *   GSF_CANNOT_REPRESENT_PRECISION
 *   GSF_ILLEGAL_SCALE_FACTOR_MULTIPLIER
 *   GSF_QUALITY_FLAGS_DECODE_ERROR
 *
 ********************************************************************/

static int
gsfUnpackStream (int handle, int desiredRecord, gsfDataID *dataID, gsfRecords *rptr, unsigned char *buf, int max_size)
{
    int             readNext = 1;
    int             ret;
    long long       readStat;
    gsfuLong        tmpBuff[2];
    gsfuLong        dataSize;
    gsfuLong        readSize;
    gsfuLong        did;
    gsfDataID       thisID;
    gsfuLong        temp;
    unsigned char  *dptr = streamBuff;
    gsfuLong        ckSum;

    if ((handle < 1) || (handle > GSF_MAX_OPEN_FILES))
    {
        gsfError = GSF_BAD_FILE_HANDLE;
        return (-1);
    }

    /* This loop will read one record at a time until the record type
     * desired by the caller is found.
     */
    while (readNext)
    {
        /* Get the current record pointer */
        if ((gsfFileTable[handle - 1].previous_record = ftell(gsfFileTable[handle - 1].fp)) == -1)
        {
            gsfError = GSF_FILE_SEEK_ERROR;
            return (-1);
        }

        /* If the file is open for update and the last operation was a write,
         * flush the buffer.
         */
        if ((gsfFileTable[handle - 1].update_flag) &&
            (gsfFileTable[handle - 1].read_write_flag == LAST_OP_WRITE))
        {
            if (fflush (gsfFileTable[handle - 1].fp))
            {
                gsfError = GSF_FLUSH_ERROR;
                return(-1);
            }
        }
        gsfFileTable[handle - 1].read_write_flag = LAST_OP_READ;

        /* Read the data size, and GSF ID fields */
        readStat = fread((void *) tmpBuff, GSF_LONG_SIZE, (size_t) 2, gsfFileTable[handle - 1].fp);
        if (readStat != 2)
        {
            if (feof(gsfFileTable[handle - 1].fp))
            {
                /* wkm 10-19-01: if error reading file and we're at the end of the file,
                 *               reset file pointer
                 */
                fseek (gsfFileTable[handle - 1].fp,
                       gsfFileTable[handle - 1].previous_record,
                       SEEK_SET);
                /* if anything was read, that's a different error code than nothing read */
                if (readStat == 0)
                    gsfError = GSF_READ_TO_END_OF_FILE;
                else
                    gsfError = GSF_PARTIAL_RECORD_AT_END_OF_FILE;
                return (-1);
            }
            gsfError = GSF_READ_ERROR;
            return (-1);
        }

        /* Convert from GSF to host byte order, GSF byte order = network byte order */
        dataSize = (gsfuLong) ntohl(tmpBuff[0]);
        readSize = dataSize;
        did = (gsfuLong) ntohl(tmpBuff[1]);

        /* Convert the did value into a gsfDataID struct
         * First the check sum value
         *
         * 1098 7654 3210 9876 5432 1098 7654 3210
         * 1000 0000 0000 0000 0000 0000 0000 0000
         *    8  0    0    0    0    0    0    0
         */
        thisID.checksumFlag = (did & 0x80000000);

        /* Now the reserved field
         * 1098 7654 3210 9876 5432 1098 7654 3210
         * 0111 1111 1100 0000 0000 0000 0000 0000
         *    7  F    C    0    0    0    0    0
         */
        temp = did & 0x7FC00000;
        thisID.reserved = (temp >> 22);

        /* Now the combination of registry number and data type number
         * 1098 7654 3210 9876 5432 1098 7654 3210
         * 0000 0000 0011 1111 1111 1111 1111 1111
         *    0  0    3    F    F    F    F    F
         */
        temp = did;
        thisID.recordID = (temp & 0x003FFFFF);

        /* if there is a checksum read it, we'll read four additional bytes */
        if (thisID.checksumFlag)
        {
            readSize = dataSize + 4;
            /* jsb 01-30-95
             * dptr += 4;
             */
            dptr = streamBuff + 4;
        }
        else
        {
            dptr = streamBuff;
        }

        /* Make sure that we have a big enough buffer to fit this record, then read it out. */
        if ((readSize <= 8) || (readSize > GSF_MAX_RECORD_SIZE))
        {
            /* wkm, may have an incomplete record here */
            gsfError = GSF_RECORD_SIZE_ERROR;
            return (-1);
        }

        /* Make sure we have a valid recordID since there is no point in reading the "size"
         * bytes of data if the ID is not recognized.
         */
        if ((thisID.recordID < 1) || (thisID.recordID >= NUM_REC_TYPES))
        {
            gsfError = GSF_UNRECOGNIZED_RECORD_ID;
            return (-1);
        }

        /* If the caller passed GSF_NEXT_RECORD, as the desiredRecord, they want the next record. */
        if ((desiredRecord == GSF_NEXT_RECORD) || (thisID.recordID == (unsigned int) desiredRecord))
        {
            readNext = 0;
            /* Set the caller's ID structure with those items we've read */
            dataID->checksumFlag = thisID.checksumFlag;
            dataID->reserved = thisID.reserved;
            dataID->recordID = thisID.recordID;

            readStat = fread(streamBuff, (size_t) 1, readSize, gsfFileTable[handle - 1].fp);
            if (readStat != readSize)
            {
                if (feof(gsfFileTable[handle - 1].fp))
                {
                    /* wkm 10-19-01: if error reading file and we're at the end of the file,
                     *               reset file pointer
                     */
                    fseek (gsfFileTable[handle - 1].fp,
                          gsfFileTable[handle - 1].previous_record,
                          SEEK_SET);
                    /* if anything was read, that's a different error code than nothing read */
                    if (readStat == 0)
                        gsfError = GSF_READ_TO_END_OF_FILE;
                    else
                        gsfError = GSF_PARTIAL_RECORD_AT_END_OF_FILE;
                    return (-1);
                }
                gsfError = GSF_READ_ERROR;
                return (-1);
            }
        }

        /* This record is not the requested record, advance the file pointer */
        else if (thisID.recordID != (unsigned int) desiredRecord)
        {
            readStat = fseek(gsfFileTable[handle - 1].fp, readSize, SEEK_CUR);
            if (readStat)
            {
                gsfError = GSF_FILE_SEEK_ERROR;
                return (-1);
            }
        }
    }

    /*
    * If the caller's buffer isn't null, move this data into their buffer.
    *  Don't move the 4 byte checksum into the buffer.
    */
    if ((buf) && (dataSize <= max_size))
    {
        memcpy(buf, dptr, dataSize);
    }
    else if ((buf) && (dataSize > max_size))
    {
        gsfError = GSF_INSUFFICIENT_SIZE;
        return (-1);
    }

    /* We have the record of interest, verify the checksum if required */
    if (thisID.checksumFlag)
    {
        memcpy(&tmpBuff[0], streamBuff, GSF_LONG_SIZE);
        ckSum = (gsfuLong) ntohl(tmpBuff[0]);
        if (ckSum != (gsfChecksum(dptr, dataSize)))
        {
            gsfError = GSF_CHECKSUM_FAILURE;
            return (-1);
        }
    }

    /* Invoke the appropriate function for unpacking this record into a standard GSF structure. */
    switch (thisID.recordID)
    {
        case (GSF_RECORD_HEADER):
            ret = gsfDecodeHeader(&rptr->header, dptr);
            if (ret < 0)
            {
                gsfError = GSF_HEADER_RECORD_DECODE_FAILED;
                return (-1);
            }
            break;

        case (GSF_RECORD_SWATH_BATHY_SUMMARY):
            ret = gsfDecodeSwathBathySummary(&rptr->summary, dptr);
            if (ret < 0)
            {
                gsfError = GSF_SUMMARY_RECORD_DECODE_FAILED;
                return (-1);
            }
            break;

        case (GSF_RECORD_SWATH_BATHYMETRY_PING):
            ret = gsfDecodeSwathBathymetryPing(&rptr->mb_ping, dptr, &gsfFileTable[handle - 1], handle, dataSize);
            if (ret < 0)
            {
                /* gsfError is set within gsfDecodeSwathBathymetryPing. */
                return (-1);
            }
            break;

        case (GSF_RECORD_SOUND_VELOCITY_PROFILE):
            ret = gsfDecodeSoundVelocityProfile(&rptr->svp, &gsfFileTable[handle - 1], dptr);
            if (ret < 0)
            {
                gsfError = GSF_SVP_RECORD_DECODE_FAILED;
                return (-1);
            }
            break;

        case (GSF_RECORD_PROCESSING_PARAMETERS):
            ret = gsfDecodeProcessingParameters(&rptr->process_parameters, &gsfFileTable[handle - 1], dptr);
            if (ret < 0)
            {
                gsfError = GSF_PROCESS_PARAM_RECORD_DECODE_FAILED;
                return (-1);
            }
            break;

        case (GSF_RECORD_SENSOR_PARAMETERS):
            ret = gsfDecodeSensorParameters(&rptr->sensor_parameters, &gsfFileTable[handle - 1], dptr);
            if (ret < 0)
            {
                gsfError = GSF_SENSOR_PARAM_RECORD_DECODE_FAILED;
                return (-1);
            }
            break;

        case (GSF_RECORD_COMMENT):
            ret = gsfDecodeComment(&rptr->comment, &gsfFileTable[handle - 1], dptr);
            if (ret < 0)
            {
                gsfError = GSF_COMMENT_RECORD_DECODE_FAILED;
                return (-1);
            }
            break;

        case (GSF_RECORD_HISTORY):
            ret = gsfDecodeHistory(&rptr->history, &gsfFileTable[handle - 1], dptr);
            if (ret < 0)
            {
                gsfError = GSF_HISTORY_RECORD_DECODE_FAILED;
                return (-1);
            }
            break;

        case (GSF_RECORD_NAVIGATION_ERROR):
            ret = gsfDecodeNavigationError(&rptr->nav_error, dptr);
            if (ret < 0)
            {
                gsfError = GSF_NAV_ERROR_RECORD_DECODE_FAILED;
                return (-1);
            }
            break;

        case (GSF_RECORD_SINGLE_BEAM_PING):
            ret = gsfDecodeSinglebeam(&rptr->sb_ping, dptr, dataSize);
            if (ret < 0)
            {
                /* gsfError is set within gsfDecodeSinglebeam */
                return (-1);
            }
            break;

        case (GSF_RECORD_HV_NAVIGATION_ERROR):
            ret = gsfDecodeHVNavigationError(&rptr->hv_nav_error, &gsfFileTable[handle - 1], dptr);
            if (ret < 0)
            {
                gsfError = GSF_HV_NAV_ERROR_RECORD_DECODE_FAILED;
                return (-1);
            }
            break;

        case (GSF_RECORD_ATTITUDE):
            ret = gsfDecodeAttitude(&rptr->attitude, &gsfFileTable[handle - 1], dptr);
            if (ret < 0)
            {
                gsfError = GSF_ATTITUDE_RECORD_DECODE_FAILED;
                return (-1);
            }
            break;


        default:
            gsfError = GSF_UNRECOGNIZED_RECORD_ID;
            return (-1);
    }

    return (readSize + GSF_FILL_SIZE);
}

/********************************************************************
 *
 * Function Name : gsfSeekRecord
 *
 * Description : This function moves the current GSF file position to the
 *   beginning of the nth record of a specific type.  The record number and
 *   type of interest are specified by id.record_number and id.recordID.
 *   The byte offset into the file for the record of interest is retrieved
 *   from the index file created by a previous call to gsfOpen with one of
 *   the supported direct access modes specified.  This function is
 *   maintained as static to the library since the functions gsfRead
 *   and gsfWrite may be called directly to access a specific record.
 *
 * Inputs :
 *  handle = the handle for this file as returned by gsfOpen
 *  id = a pointer to a gsfDataID containing the record id information for
 *       the record of interest.
 *
 * Returns :
 *  This function returns zero if successful, or -1 if an error occured.
 *
 * Error Conditions :
 *  GSF_BAD_FILE_HANDLE
 *  GSF_UNRECOGNIZED_RECORD_ID
 *  GSF_INVALID_RECORD_NUMBER
 *  GSF_RECORD_TYPE_NOT_AVAILABLE
 *  GSF_FILE_SEEK_ERROR
 *  GSF_INDEX_FILE_READ_ERROR
 *
 *******************************************************************/

static int
gsfSeekRecord(int handle, gsfDataID *id)
{
    int             ret;
    int             offset;
    int             i;
    int             scale_index;
    long long       addr;
    gsfRecords      scalesRecord;
    INDEX_REC       index_rec;

    /* Clear gsfError before each seek */
    gsfError = 0;

    /* JSB 04/05/00 replaced ">=" with ">" */
    if ((handle < 1) || (handle > GSF_MAX_OPEN_FILES))
    {
        gsfError = GSF_BAD_FILE_HANDLE;
        return (-1);
    }

    /* Make sure we have a valid recordID */
    if ((id->recordID < 1) || (id->recordID >= NUM_REC_TYPES))
    {
        gsfError = GSF_UNRECOGNIZED_RECORD_ID;
        return (-1);
    }

    /* Make sure we have a valid record_number */
    if ((id->record_number < -1) ||
        (id->record_number == 0) ||
        (id->record_number > gsfFileTable[handle - 1].index_data.number_of_records[id->recordID]))
    {
        gsfError = GSF_INVALID_RECORD_NUMBER;
        return (-1);
    }

    /* Check the record_types to see if the requested type is available */
    if (gsfFileTable[handle - 1].index_data.record_type[id->recordID] == -1)
    {
        /* The record type is not available. */
        gsfError = GSF_RECORD_TYPE_NOT_AVAILABLE;
        return (-1);
    }

    /* If the record number requested is -1, use the last record of
     * this type. Note that the record number counts from one.
     */
    if (id->record_number == -1)
    {
        offset = gsfFileTable[handle - 1].index_data.number_of_records[id->recordID] - 1;
    }
    else
    {
        offset = id->record_number - 1;
    }

    /* Compute the record address within the index file and read
     * the index record.
     */
    addr = gsfFileTable[handle - 1].index_data.start_addr[id->recordID] +
        (long long)(offset * sizeof(INDEX_REC));
    if (fseek(gsfFileTable[handle - 1].index_data.fp, addr, SEEK_SET))
    {
        gsfError = GSF_FILE_SEEK_ERROR;
        return (-1);
    }
    ret = fread(&index_rec, sizeof(INDEX_REC), 1, gsfFileTable[handle - 1].index_data.fp);
    if (ret != 1)
    {
        gsfError = GSF_INDEX_FILE_READ_ERROR;
        return(-1);
    }
    if (gsfFileTable[handle - 1].index_data.swap)
    {
        SwapLongLong((long long *) &index_rec.addr, 1);
    }

    /* If the record type is GSF_RECORD_SWATH_BATHYMETRY_PING then we
     * need to ensure that we have the ping record scale factors which
     * apply to the ping record of interest.  The scale factor subrecord
     * of the ping record will only be present in the file when the scale
     * factors change.
     */
    if (id->recordID == GSF_RECORD_SWATH_BATHYMETRY_PING)
    {
        /* Clear the scale index */
        scale_index = -1;
        for (i = 1; i < gsfFileTable[handle - 1].index_data.number_of_records[0]; i++)
        {
            /* When the address of the record containing scale factors is
             * greater than the address of the record of interest, get the
             * address of the record containing scale factors which is one
             * prior to this one.
             */
            if (gsfFileTable[handle - 1].index_data.scale_factor_addr[i].addr >
                index_rec.addr)
            {
                scale_index = i - 1;
                break;
            }
        }

        /* If we didn't find a record containing scale factors with an address
         * greater than the address of the record of interest, then use the
         * last record in the file with scale factors.  Note that this
         * condition is true if the file only contains one set of scale
         * factors.
         */
        if (scale_index == -1)
        {
            scale_index = gsfFileTable[handle - 1].index_data.number_of_records[0] - 1;
        }

        /* We only need to go read the ping record with scale factors if we
         * need to use a different set of scale factors than we did last time.
         */
        if (scale_index != gsfFileTable[handle - 1].index_data.last_scale_factor_index)
        {
            addr = gsfFileTable[handle - 1].index_data.scale_factor_addr[scale_index].addr;
            if (fseek(gsfFileTable[handle - 1].fp, addr, SEEK_SET))
            {
                gsfError = GSF_FILE_SEEK_ERROR;
                return (-1);
            }

            memset(&scalesRecord, 0, sizeof(scalesRecord));
            ret = gsfUnpackStream (handle, GSF_NEXT_RECORD, id, &scalesRecord, NULL, 0);
            if (ret < 0)
            {
                /* gsfError will have been set in gsfUnpackStream */
                return(-1);
            }
            memcpy(&gsfFileTable[handle - 1].rec, &scalesRecord, sizeof(gsfFileTable[handle - 1].rec));

            /* We now have the scale factors we need, save this index for
             * the comparison next time.
             */
            gsfFileTable[handle - 1].index_data.last_scale_factor_index = scale_index;
        }
    }

    /* Seek to this offset in the GSF file. */
    if (fseek(gsfFileTable[handle - 1].fp, index_rec.addr, SEEK_SET))
    {
        gsfError = GSF_FILE_SEEK_ERROR;
        return (-1);
    }

    return (0);
}

/********************************************************************
 *
 * Function Name : gsfWrite
 *
 * Description : gsfWrite encodes the data from internal to external form,
 *   and then writes the requested record into the file specified by handle,
 *   where handle is the value returned by gsfOpen.  The record is written to
 *   the current file pointer for handle.  An optional checksum may be computed
 *   and encoded with the data.
 *
 *   If the file is opened for sequential access (GSF_CREATE or GSF_UPDATE)
 *   then the recordID field of the gsfDataID structure is used to specify
 *   the record to be written.  The record is written at the current location
 *   in the file.
 *
 *   If the file is opened for direct access (GSF_UPDATE_INDEX), then the
 *   combination of the recordID and the record_number fields of the gsfDataID
 *   structure are used to uniquely identify the record to be written.  The
 *   address of the record of interest is read from the index file and the file
 *   pointer is moved to this offset before the record is encoded and written
 *   to disk.
 *
 * Inputs :
 *  handle = the handle for this file as returned by gsfOpen
 *  id = a pointer to a gsfDataID containing the record id information for
 *       the record to write.
 *  rptr = a pointer to a gsfRecords structure from which to get the internal
 *         form of the record to be written to the file.
 *
 * Returns :
 *   This function returns the number of bytes written if successful,
 *   or -1 if an error occurred.
 *
 * Error Conditions :
 *    GSF_BAD_FILE_HANDLE
 *    GSF_UNRECOGNIZED_RECORD_ID
 *    GSF_FILE_SEEK_ERROR
 *    GSF_WRITE_ERROR
 *    GSF_HEADER_RECORD_ENCODE_FAILED
 *    GSF_SVP_RECORD_ENCODE_FAILED
 *    GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED
 *    GSF_SENSOR_PARAM_RECORD_ENCODE_FAILED
 *    GSF_COMMENT_RECORD_ENCODE_FAILED
 *    GSF_HISTORY_RECORD_ENCODE_FAILED
 *    GSF_NAV_ERROR_RECORD_ENCODE_FAILED
 *    GSF_FLUSH_ERROR
 *    GSF_SINGLE_BEAM_ENCODE_FAILED
 *
 ********************************************************************/

int
gsfWrite(int handle, gsfDataID *id, gsfRecords *rptr)
{
    unsigned char  *ucptr;
    gsfuLong        tmpBuff[3] =
    {0, 0, 0};
    gsfuLong        gsfBuff[3];
    gsfuLong        temp;
    gsfuLong        dataSize;
    size_t          writeStat;
    int             i;
    int             pad;
    long long       ret;

    /* Clear gsfError before each write */
    gsfError = 0;

    /* JSB 04/05/00 replaced ">=" with ">" */
    if ((handle < 1) || (handle > GSF_MAX_OPEN_FILES))
    {
        gsfError = GSF_BAD_FILE_HANDLE;
        return (-1);
    }

    /* See if we need to make room for the optional checksum */
    if (id->checksumFlag)
    {
        /* four byte size, four byte id, and four byte checksum */
        ucptr = streamBuff + GSF_FILL_SIZE_CHECKSUM;
    }
    else
    {
        /* four byte size and four byte id */
        ucptr = streamBuff + GSF_FILL_SIZE;
    }

    /* Invoke the appropriate function for packing this record into a
    * byte stream.
    */
    switch (id->recordID)
    {
        case (GSF_RECORD_HEADER):
            ret = gsfEncodeHeader(ucptr, &rptr->header);
            if (ret < 0)
            {
                gsfError = GSF_HEADER_RECORD_ENCODE_FAILED;
                return (-1);
            }
            break;

        case (GSF_RECORD_SWATH_BATHY_SUMMARY):
            ret = gsfEncodeSwathBathySummary(ucptr, &rptr->summary);
            if (ret < 0)
            {
                gsfError = GSF_SUMMARY_RECORD_ENCODE_FAILED;
                return (-1);
            }
            break;

        case (GSF_RECORD_SWATH_BATHYMETRY_PING):
            ret = gsfEncodeSwathBathymetryPing(ucptr, &rptr->mb_ping, &gsfFileTable[handle - 1]);
            if (ret < 0)
            {
                /* gsfError is set within gsfEncodeSwathBathymetryPing */
                return (-1);
            }
            break;

        case (GSF_RECORD_SOUND_VELOCITY_PROFILE):
            ret = gsfEncodeSoundVelocityProfile(ucptr, &rptr->svp);
            if (ret < 0)
            {
                gsfError = GSF_SVP_RECORD_ENCODE_FAILED;
                return (-1);
            }
            break;

        case (GSF_RECORD_PROCESSING_PARAMETERS):
            ret = gsfEncodeProcessingParameters(ucptr, &rptr->process_parameters);
            if (ret < 0)
            {
                gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
                return (-1);
            }
            break;

        case (GSF_RECORD_SENSOR_PARAMETERS):
            ret = gsfEncodeSensorParameters(ucptr, &rptr->sensor_parameters);
            if (ret < 0)
            {
                gsfError = GSF_SENSOR_PARAM_RECORD_ENCODE_FAILED;
                return (-1);
            }
            break;

        case (GSF_RECORD_COMMENT):
            ret = gsfEncodeComment(ucptr, &rptr->comment);
            if (ret < 0)
            {
                gsfError = GSF_COMMENT_RECORD_ENCODE_FAILED;
                return (-1);
            }
            break;

        case (GSF_RECORD_HISTORY):
            ret = gsfEncodeHistory(ucptr, &rptr->history);
            if (ret < 0)
            {
                gsfError = GSF_HISTORY_RECORD_ENCODE_FAILED;
                return (-1);
            }
            break;

        case (GSF_RECORD_NAVIGATION_ERROR):
            ret = gsfEncodeNavigationError(ucptr, &rptr->nav_error);
            if (ret < 0)
            {
                gsfError = GSF_NAV_ERROR_RECORD_ENCODE_FAILED;
                return (-1);
            }
            break;

        case (GSF_RECORD_SINGLE_BEAM_PING):
            ret = gsfEncodeSinglebeam(ucptr, &rptr->sb_ping);
            if (ret < 0)
            {
                gsfError = GSF_SINGLE_BEAM_ENCODE_FAILED;
                return (-1);
            }
            break;

        /* jsb 09/29/98 Added support for new navigation errors record */
        case (GSF_RECORD_HV_NAVIGATION_ERROR):
            ret = gsfEncodeHVNavigationError(ucptr, &rptr->hv_nav_error);
            if (ret < 0)
            {
                gsfError = GSF_HV_NAV_ERROR_RECORD_ENCODE_FAILED;
                return (-1);
            }
            break;

        case (GSF_RECORD_ATTITUDE):
            ret = gsfEncodeAttitude(ucptr, &rptr->attitude);
            if (ret < 0)
            {
                gsfError = GSF_ATTITUDE_RECORD_ENCODE_FAILED;
                return (-1);
            }
            break;

        default:
            gsfError = GSF_UNRECOGNIZED_RECORD_ID;
            return (-1);
    }

    /* GSF specification requires all records to be a whole multiple of 4 bytes */
    dataSize = (gsfuLong) ret;
    pad = dataSize % 4;
    if (pad)
    {
        /* jsb 04/18/97 A bug was fixed here in version 1.03, if this file was
         * created with a version of GSF prior to 1.03 we need to support it the
         * old way.
         */
        if ((gsfFileTable[handle - 1].major_version_number == 1) &&
            (gsfFileTable[handle - 1].minor_version_number <= 2))
        {
            memset(ucptr + dataSize, 0, pad);
            dataSize += pad;
        }
        else
        {
            memset(ucptr + dataSize, 0, (4 - pad));
            dataSize += (4 - pad);
        }
    }

    /* Load the data identifier for this GSF record, first the checksum flag */
    if (id->checksumFlag)
    {
        /* Set the checksum bit */
        tmpBuff[1] |= 0x80000000;

        /* Compute the checksum */
        tmpBuff[2] = gsfChecksum(ucptr, dataSize);
    }

    /* Now the reserved field */
    temp = (gsfuLong) id->reserved;
    tmpBuff[1] |= (temp << 22);

    /* Now the recordID, goes in bits 00-21 */
    tmpBuff[1] |= (gsfuLong) id->recordID;

    /* Load the size of the data for this GSF record */
    tmpBuff[0] = dataSize;

    /* Now load the GSF packaging words into GSF byte order */
    for (i = 0; i < 3; i++)
    {
        gsfBuff[i] = htonl(tmpBuff[i]);
    }

    /* Set the buffer pointer back to the first byte */
    ucptr = streamBuff;

    /* Add the GSF packaging words to the GSF stream */
    if (id->checksumFlag)
    {
        memcpy(ucptr, gsfBuff, GSF_FILL_SIZE_CHECKSUM);
        dataSize += GSF_FILL_SIZE_CHECKSUM;
    }
    else
    {
        memcpy(ucptr, gsfBuff, GSF_FILL_SIZE);
        dataSize += GSF_FILL_SIZE;
    }

    /* Save the current record pointer */
    if ((gsfFileTable[handle - 1].previous_record = ftell(gsfFileTable[handle - 1].fp)) == -1)
    {
        gsfError = GSF_FILE_SEEK_ERROR;
        return (-1);
    }

    /*  If the file is open for update and the last operation was a read,
     *  flush the buffer.
     */
    if ((gsfFileTable[handle - 1].update_flag) &&
        (gsfFileTable[handle - 1].read_write_flag == LAST_OP_READ))
    {
        if (fflush (gsfFileTable[handle - 1].fp))
        {
            gsfError = GSF_FLUSH_ERROR;
            return(-1);
        }
        gsfFileTable[handle - 1].bufferedBytes = 0;
    }
    gsfFileTable[handle - 1].read_write_flag = LAST_OP_WRITE;

    /* Check to see if this record will fit into the current buffer, if not
     * force a flush of the stream before writting. This is done to ensure
     * that an output file will always contain whole GSF records.
     */
    gsfFileTable[handle-1].bufferedBytes += dataSize;
    if (gsfFileTable[handle-1].bufferedBytes >= gsfFileTable[handle-1].buf_size)
    {
        if (fflush(gsfFileTable[handle-1].fp))
        {
            gsfError = GSF_FLUSH_ERROR;
            return(-1);
        }
        gsfFileTable[handle-1].bufferedBytes = 0;
    }

    /* If this file is opened for direct access, then move the file pointer
     * to the record which the caller wants to write.
     */
    if (gsfFileTable[handle - 1].direct_access)
    {
        ret = gsfSeekRecord(handle, id);
        if (ret < 0)
        {
            /* gsfError is set in gsfSeekRecord */
            return (-1);
        }
    }

    /* Now write the data to the disk */
    writeStat = fwrite(ucptr, (size_t) 1, (size_t) dataSize, gsfFileTable[handle - 1].fp);
    if (writeStat != dataSize)
    {
        gsfError = GSF_WRITE_ERROR;
        return (-1);
    }

    gsfFileTable[handle - 1].last_record_type = id->recordID;

    /* Return the number of bytes written */
    return (dataSize);
}

/********************************************************************
 *
 * Function Name : gsfLoadScaleFactor
 *
 * Description : gsfLoadScaleFactor should be used to load the swath
 *  bathymetry ping record scale factor structure.  This function ensures
 *  that the multiplier and offset fields of the scale factor structure
 *  have a precision equal to that which will be stored in the GSF data file.
 *  This function should be called once for each beam array data type
 *  contained in your data.
 *
 * Inputs :
 *  sf = a pointer to the gsfScaleFactors structure to be loaded
 *  subrecordID = the subrecord id for the beam array data
 *  c_flag = the compression flag for the beam array
 *  precision = the precision to which the beam array data are to be stored
 *              (a value of 0.1 would indicate decimeter precision for depth)
 *  offset = the "DC" offset to scale the data by.
 *
 * Returns :
 *  This function returns zero if successful, or -1 if an error occured.
 *
 * Error Conditions :
 *   GSF_UNRECOGNIZED_ARRAY_SUBRECORD_ID
 *   GSF_TOO_MANY_ARRAY_SUBRECORDS
 *   GSF_CANNOT_REPRESENT_PRECISION
 *
 ********************************************************************/

int
gsfLoadScaleFactor(gsfScaleFactors *sf, unsigned int subrecordID, char c_flag, double precision, int offset)
{
    unsigned int    itemp;
    double          mult;

	/* Make sure we have a valid subrecordID. */
    if ((subrecordID < 1) || (subrecordID > GSF_MAX_PING_ARRAY_SUBRECORDS))
    {
        gsfError = GSF_UNRECOGNIZED_ARRAY_SUBRECORD_ID;
        return (-1);
    }

    /* Make sure precision is positive. */
    if (precision <= 0.0)
    {
        gsfError = GSF_CANNOT_REPRESENT_PRECISION;
        return (-1);
    }

    /* If we're adding a new subrecord, bump counter and check bounds */
    if (sf->scaleTable[subrecordID - 1].multiplier == 0.0)
    {
        if ((sf->numArraySubrecords + 1) > GSF_MAX_PING_ARRAY_SUBRECORDS)
        {
            sf->numArraySubrecords--;
            gsfError = GSF_TOO_MANY_ARRAY_SUBRECORDS;
            return (-1);
        }

        /* Compute the multiplier as one over the requested precision */
        mult = 1.0 / precision;

        /* In order to assure the same multiplier is used throughout, truncate
         *  to an integer.  This is the value which is stored with the data.
         *  The multiplier value is encoded on the byte stream as an integer value
         *  (i.e. it is not scaled) so the smallest supportable precision is 1.
         */
        itemp = (int) (mult + 0.001);

        /* QC test on the integer value as this is the number that will get encoded on the GSF byte stream */
        if ((itemp < MIN_GSF_SF_MULT_VALUE) || (itemp > MAX_GSF_SF_MULT_VALUE))
        {
            gsfError = GSF_CANNOT_REPRESENT_PRECISION;
            return (-1);
        }

        /* New scale factor has passed QC tests, it is now safe to bump the counter */
        sf->numArraySubrecords++;
    }
    else
    {
        /* Compute the multiplier as one over the requested precision */
        mult = 1.0 / precision;

        /* In order to assure the same multiplier is used throughout, truncate
         *  to an integer.  This is the value which is stored with the data.
         *  The multiplier value is encoded on the byte stream as an integer value
         *  (i.e. it is not scaled) so the smallest supportable precision is 1.
         */
        itemp = (int) (mult + 0.001);

        /* QC test on the integer value as this is the number that will get encoded on the GSF byte stream */
        if ((itemp < MIN_GSF_SF_MULT_VALUE) || (itemp > MAX_GSF_SF_MULT_VALUE))
        {
            gsfError = GSF_CANNOT_REPRESENT_PRECISION;
            return (-1);
        }
    }

    /* The multiplier to be applied to the data is converted back to a
     *  double here, for floating point performance.
     */
    sf->scaleTable[subrecordID - 1].compressionFlag = (unsigned char) c_flag;
    sf->scaleTable[subrecordID - 1].multiplier = ((double) itemp);
    sf->scaleTable[subrecordID - 1].offset = (double) offset;

    return (0);
}

/********************************************************************
 *
 * Function Name : gsfGetScaleFactor
 *
 * Description : gsfGetScaleFactor may be used to obtain the multiplier
 *   and DC offset values by which each swath bathymetry ping array subrecord
 *   is be scaled. gsfGetScalesFactor must be called once for each array
 *   subrecord of interest.  At leat one swath bathymetry ping record
 *   must have been read from, or written to the file specified by handle.
 *
 * Inputs :
 *  handle = the integer value set by a call to gsfOpen.
 *  subrecordID = an integer value containing the subrecord id of the requested scale factors
 *  c_flag = the address of an unsigned character to contain the the compression flag
 *  multiplier = the address of a double to contain the scaling multiplier
 *  offset = the address of a double to contain the scaling DC offset.
 *
 * Returns :
 *  This function returns zero if successful, or -1 if an error occurred.
 *
 * Error Conditions :
 *   GSF_UNRECOGNIZED_ARRAY_SUBRECORD_ID
 *   GSF_BAD_FILE_HANDLE
 *   GSF_ILLEGAL_SCALE_FACTOR_MULTIPLIER
 *
 ********************************************************************/

int
gsfGetScaleFactor(int handle, unsigned int subrecordID, unsigned char *c_flag, double *multiplier, double *offset)
{
    /* Make sure we have a valid subrecordID. */
    if ((subrecordID < 1) || (subrecordID > GSF_MAX_PING_ARRAY_SUBRECORDS))
    {
        gsfError = GSF_UNRECOGNIZED_ARRAY_SUBRECORD_ID;
        return (-1);
    }

    /* JSB 04/05/00 replaced ">=" with ">" */
    if ((handle < 1) || (handle > GSF_MAX_OPEN_FILES))
    {
        gsfError = GSF_BAD_FILE_HANDLE;
        return (-1);
    }

    /* Make sure the multiplier is not zero */
    if (gsfFileTable[handle-1].rec.mb_ping.scaleFactors.scaleTable[subrecordID-1].multiplier == 0.0)
    {
        gsfError = GSF_ILLEGAL_SCALE_FACTOR_MULTIPLIER;
        return (-1);
    }

    /* Set the compression flag */
    *c_flag = gsfFileTable[handle-1].rec.mb_ping.scaleFactors.scaleTable[subrecordID-1].compressionFlag;

    /* Set the multiplier */
    *multiplier = gsfFileTable[handle-1].rec.mb_ping.scaleFactors.scaleTable[subrecordID-1].multiplier;

    /* Set the offset */
    *offset = gsfFileTable[handle-1].rec.mb_ping.scaleFactors.scaleTable[subrecordID-1].offset;

    return (0);
}

/********************************************************************
 *
 * Function Name : gsfFree
 *
 * Description : This function frees all dynamically allocated memory
 *    from a gsfRecords data structure, and it then clears all the
 *    data elements in the structure.
 *
 * Inputs :
 *   rec = a pointer to a gsfRecords data structure
 *
 * Returns : none
 *
 * Error Conditions : none
 *
 ********************************************************************/

void
gsfFree (gsfRecords *rec)
{
    int i;

    /* Free the array subrecords in ping data structure */
    if (rec->mb_ping.depth != (double *) NULL)
    {
        free (rec->mb_ping.depth);
        rec->mb_ping.depth = (double *) NULL;
    }

    if (rec->mb_ping.nominal_depth != (double *) NULL)
    {
        free (rec->mb_ping.nominal_depth);
        rec->mb_ping.nominal_depth = (double *) NULL;
    }

    if (rec->mb_ping.across_track != (double *) NULL)
    {
        free (rec->mb_ping.across_track);
        rec->mb_ping.across_track = (double *) NULL;
    }

    if (rec->mb_ping.along_track != (double *) NULL)
    {
        free (rec->mb_ping.along_track);
        rec->mb_ping.along_track = (double *) NULL;
    }

    if (rec->mb_ping.travel_time != (double *) NULL)
    {
        free (rec->mb_ping.travel_time);
        rec->mb_ping.travel_time = (double *) NULL;
    }

    if (rec->mb_ping.beam_angle != (double *) NULL)
    {
        free (rec->mb_ping.beam_angle);
        rec->mb_ping.beam_angle = (double *) NULL;
    }

    if (rec->mb_ping.mc_amplitude != (double *) NULL)
    {
        free (rec->mb_ping.mc_amplitude);
        rec->mb_ping.mc_amplitude = (double *) NULL;
    }

    if (rec->mb_ping.mr_amplitude != (double *) NULL)
    {
        free (rec->mb_ping.mr_amplitude);
        rec->mb_ping.mr_amplitude = (double *) NULL;
    }

    if (rec->mb_ping.echo_width != (double *) NULL)
    {
        free (rec->mb_ping.echo_width);
        rec->mb_ping.echo_width = (double *) NULL;
    }

    if (rec->mb_ping.quality_factor != (double *) NULL)
    {
        free (rec->mb_ping.quality_factor);
        rec->mb_ping.quality_factor = (double *) NULL;
    }

    if (rec->mb_ping.receive_heave != (double *) NULL)
    {
        free (rec->mb_ping.receive_heave);
        rec->mb_ping.receive_heave = (double *) NULL;
    }

    if (rec->mb_ping.depth_error != (double *) NULL)
    {
        free (rec->mb_ping.depth_error);
        rec->mb_ping.depth_error = (double *) NULL;
    }

    if (rec->mb_ping.across_track_error != (double *) NULL)
    {
        free (rec->mb_ping.across_track_error);
        rec->mb_ping.across_track_error = (double *) NULL;
    }

    if (rec->mb_ping.along_track_error != (double *) NULL)
    {
        free (rec->mb_ping.along_track_error);
        rec->mb_ping.along_track_error = (double *) NULL;
    }

    if (rec->mb_ping.quality_flags != (unsigned char *) NULL)
    {
        free (rec->mb_ping.quality_flags);
        rec->mb_ping.quality_flags = (unsigned char *) NULL;
    }

    if (rec->mb_ping.beam_flags != (unsigned char *) NULL)
    {
        free (rec->mb_ping.beam_flags);
        rec->mb_ping.beam_flags = (unsigned char *) NULL;
    }

    if (rec->mb_ping.signal_to_noise != (double *) NULL)
    {
        free (rec->mb_ping.signal_to_noise);
        rec->mb_ping.signal_to_noise = (double *) NULL;
    }

    if (rec->mb_ping.beam_angle_forward != (double *) NULL)
    {
        free (rec->mb_ping.beam_angle_forward);
        rec->mb_ping.beam_angle_forward = (double *) NULL;
    }

    if (rec->mb_ping.TVG_dB != (double *) NULL)
    {
        free (rec->mb_ping.TVG_dB);
        rec->mb_ping.TVG_dB = (double *) NULL;
    }

    if (rec->mb_ping.vertical_error != (double *) NULL)
    {
        free (rec->mb_ping.vertical_error);
        rec->mb_ping.vertical_error = (double *) NULL;
    }

    if (rec->mb_ping.horizontal_error != (double *) NULL)
    {
        free (rec->mb_ping.horizontal_error);
        rec->mb_ping.horizontal_error = (double *) NULL;
    }

    if (rec->mb_ping.sector_number != (unsigned short *) NULL)
    {
        free (rec->mb_ping.sector_number);
        rec->mb_ping.sector_number = (unsigned short *) NULL;
    }

    if (rec->mb_ping.detection_info != (unsigned short *) NULL)
    {
        free (rec->mb_ping.detection_info);
        rec->mb_ping.detection_info = (unsigned short *) NULL;
    }

    if (rec->mb_ping.incident_beam_adj != (double *) NULL)
    {
        free (rec->mb_ping.incident_beam_adj);
        rec->mb_ping.incident_beam_adj = (double *) NULL;
    }

    if (rec->mb_ping.system_cleaning != (unsigned short *) NULL)
    {
        free (rec->mb_ping.system_cleaning);
        rec->mb_ping.system_cleaning = (unsigned short *) NULL;
    }

    if (rec->mb_ping.doppler_corr != (double *) NULL)
    {
        free (rec->mb_ping.doppler_corr);
        rec->mb_ping.doppler_corr = (double *) NULL;
    }

    if (rec->mb_ping.sonar_vert_uncert != (double *) NULL)
    {
        free (rec->mb_ping.sonar_vert_uncert);
        rec->mb_ping.sonar_vert_uncert = (double *) NULL;
    }

    if (rec->mb_ping.sonar_horz_uncert != (double *) NULL)
    {
        free (rec->mb_ping.sonar_horz_uncert);
        rec->mb_ping.sonar_horz_uncert = (double *) NULL;
    }

    if (rec->mb_ping.detection_window != (double *) NULL)
    {
        free (rec->mb_ping.detection_window);
        rec->mb_ping.detection_window = (double *) NULL;
    }

    if (rec->mb_ping.mean_abs_coeff != (double *) NULL)
    {
        free (rec->mb_ping.mean_abs_coeff);
        rec->mb_ping.mean_abs_coeff = (double *) NULL;
    }

    /* we have an array of number_beams gsfIntensitySeries structures */
    if (rec->mb_ping.brb_inten != (gsfBRBIntensity *) NULL)
    {
        if (rec->mb_ping.brb_inten->time_series != (gsfTimeSeriesIntensity *) NULL)
        {
            for (i = 0; i < rec->mb_ping.number_beams; i++)
            {
                if (rec->mb_ping.brb_inten->time_series[i].samples != (unsigned int *) NULL)
                {
                    free (rec->mb_ping.brb_inten->time_series[i].samples);
                    rec->mb_ping.brb_inten->time_series[i].samples = NULL;
                }
            }
            free (rec->mb_ping.brb_inten->time_series);
            rec->mb_ping.brb_inten->time_series = NULL;
        }
        free (rec->mb_ping.brb_inten);
        rec->mb_ping.brb_inten = (gsfBRBIntensity *) NULL;
    }

    /* Free the dynamically allocated memory from the svp record */
    if (rec->svp.depth != (double *) NULL)
    {
        free (rec->svp.depth);
        rec->svp.depth = (double *) NULL;
    }

    if (rec->svp.sound_speed != (double *) NULL)
    {
        free (rec->svp.sound_speed);
        rec->svp.sound_speed = (double *) NULL;
    }

    /* Free the dynamically allocated memory from the processing parameters */
    for (i=0; i<rec->process_parameters.number_parameters; i++)
    {
        if (rec->process_parameters.param[i] != (char *) NULL)
        {
            free (rec->process_parameters.param[i]);
            rec->process_parameters.param[i] = (char *) NULL;
        }
    }
    rec->process_parameters.number_parameters = 0;

    /* Free the dynamically allocated memory from the sensor parameters */
    for (i=0; i<rec->sensor_parameters.number_parameters; i++)
    {
        if (rec->sensor_parameters.param[i] != (char *) NULL)
        {
            free (rec->sensor_parameters.param[i]);
            rec->sensor_parameters.param[i] = (char *) NULL;
        }
    }
    rec->sensor_parameters.number_parameters = 0;

    /* Free the dynamically allocated memory from the comment record */
    if (rec->comment.comment != (char *) NULL)
    {
        free (rec->comment.comment);
        rec->comment.comment = (char *) NULL;
    }

    /* Free the dynamically allocated memory from the history record */
    if (rec->history.command_line != (char *) NULL)
    {
        free (rec->history.command_line);
        rec->history.command_line = (char *) NULL;
    }

    if (rec->history.comment != (char *) NULL)
    {
        free (rec->history.comment);
        rec->history.comment = (char *) NULL;
    }

    if(rec->hv_nav_error.position_type != (char *) NULL)
    {
        free (rec->hv_nav_error.position_type);
        rec->hv_nav_error.position_type = (char *) NULL;
    }
    
    /* Free the dynamically allocated memory from the attitude record */
    if (rec->attitude.attitude_time != (struct timespec *) NULL)
    {
        free (rec->attitude.attitude_time);
        rec->attitude.attitude_time = (struct timespec *) NULL;
    }

    if (rec->attitude.pitch != (double *) NULL)
    {
        free (rec->attitude.pitch);
        rec->attitude.pitch = (double *) NULL;
    }

    if (rec->attitude.roll != (double *) NULL)
    {
        free (rec->attitude.roll);
        rec->attitude.roll = (double *) NULL;
    }

    if (rec->attitude.heave != (double *) NULL)
    {
        free (rec->attitude.heave);
        rec->attitude.heave = (double *) NULL;
    }

    if (rec->attitude.heading != (double *) NULL)
    {
        free (rec->attitude.heading);
        rec->attitude.heading = (double *) NULL;
    }

    /* Now clear all the data from the gsf Records structure */
    memset (rec, 0, sizeof(gsfRecords));

    return;
}

/********************************************************************
 *
 * Function Name : gsfPrintError
 *
 * Description : This function is used to print a short message describing
 *   the most recent error encountered.  This function need only be called if
 *   a -1 is returned from one of the GSF functions.
 *
 * Inputs :
 *  fp = a pointer to a FILE to which to write the message.
 *
 * Returns : none
 *
 * Error Conditions : none
 *
 ********************************************************************/

void
gsfPrintError(FILE * fp)
{

    fprintf(fp, "%s\n", gsfStringError());

    return;
}

/********************************************************************
 *
 * Function Name : gsfIntError
 *
 * Description : This function is used to return the error code of the
 *   most recent error encountered.  This function need only be called if
 *   a -1 is returned from one of the GSF functions.
 *
 * Inputs : none
 *
 * Returns : constant integer value representing the most recent error
 *
 * Error Conditions : none
 *
 ********************************************************************/

int gsfIntError(void)
{
    return gsfError;
}

/********************************************************************
 *
 * Function Name : gsfStringError
 *
 * Description : This function is used to return a string with
 *  a short message describing the most recent error encountered.
 *  This function need only be called if
 *  a -1 is returned from one of the GSF functions.
 *
 * Inputs : None
 *
 * Returns : A pointer to a static string describing the error.
 *
 * Error Conditions : none
 *
 ********************************************************************/

const char *
gsfStringError(void)
{
    const char             *ptr;

    switch (gsfError)
    {
        case GSF_NORMAL:
            ptr = "GSF Error: None";
            break;

        case GSF_FOPEN_ERROR:
            ptr = "GSF Error: Unable to open requested file";
            break;

        case GSF_UNRECOGNIZED_FILE:
            ptr = "GSF Error: Unrecognized file";
            break;

        case GSF_BAD_ACCESS_MODE:
            ptr = "GSF Error: Illegal access mode";
            break;

        case GSF_READ_ERROR:
            ptr = "GSF Error: Error occurred reading data";
            break;

        case GSF_WRITE_ERROR:
            ptr = "GSF Error: Error occurred writing data";
            break;

        case GSF_INSUFFICIENT_SIZE:
            ptr = "GSF Error: Insufficient size specified";
            break;

        case GSF_RECORD_SIZE_ERROR:
            ptr = "GSF Error: Record size is out of bounds";
            break;

        case GSF_CHECKSUM_FAILURE:
            ptr = "GSF Error: Data checksum failed";
            break;

        case GSF_FILE_CLOSE_ERROR:
            ptr = "GSF Error: Error occurred closing GSF file";
            break;

        case GSF_TOO_MANY_ARRAY_SUBRECORDS:
            ptr = "GSF Error: Too many array subrecords";
            break;

        case GSF_TOO_MANY_OPEN_FILES:
            ptr = "GSF Error: Too many open files";
            break;

        case GSF_MEMORY_ALLOCATION_FAILED:
            ptr = "GSF Error: Memory allocation failed";
            break;

        case GSF_UNRECOGNIZED_RECORD_ID:
            ptr = "GSF Error: Unrecognized record id";
            break;

        case GSF_STREAM_DECODE_FAILURE:
            ptr = "GSF Error: Stream decode failed";
            break;

        case GSF_BAD_SEEK_OPTION:
            ptr = "GSF Error: Unrecognized file seek option";
            break;

        case GSF_FILE_SEEK_ERROR:
            ptr = "GSF Error: File seek failed";
            break;

        case GSF_UNRECOGNIZED_SENSOR_ID:
            ptr = "GSF Error: Unrecognized sensor-specific subrecord id";
            break;

        case GSF_UNRECOGNIZED_DATA_RECORD:
            ptr = "GSF Error: Unrecognized data record id";
            break;

        case GSF_UNRECOGNIZED_ARRAY_SUBRECORD_ID:
            ptr = "GSF Error: Unrecognized array subrecord id";
            break;

        case GSF_UNRECOGNIZED_SUBRECORD_ID:
            ptr = "GSF Error: Unrecognized subrecord id";
            break;

        case GSF_ILLEGAL_SCALE_FACTOR_MULTIPLIER:
            ptr = "GSF Error: Illegal scale factor multiplier specified";
            break;

        case GSF_CANNOT_REPRESENT_PRECISION:
            ptr = "GSF Error: Can not represent requested precision";
            break;

        case GSF_READ_TO_END_OF_FILE:
            ptr = "GSF Error: End of file encountered";
            break;

        case GSF_BAD_FILE_HANDLE:
            ptr = "GSF Error: Bad file handle";
            break;

        case GSF_HEADER_RECORD_DECODE_FAILED:
            ptr = "GSF Error: Decoding header record failed";
            break;

        case GSF_MB_PING_RECORD_DECODE_FAILED:
            ptr = "GSF Error: Decoding multibeam ping record failed";
            break;

        case GSF_SVP_RECORD_DECODE_FAILED:
            ptr = "GSF Error: Decoding SVP record failed";
            break;

        case GSF_PROCESS_PARAM_RECORD_DECODE_FAILED:
            ptr = "GSF Error: Decoding processing parameters record failed";
            break;

        case GSF_SENSOR_PARAM_RECORD_DECODE_FAILED:
            ptr = "GSF Error: Decoding sensor parameters record failed";
            break;

        case GSF_COMMENT_RECORD_DECODE_FAILED:
            ptr = "GSF Error: Decoding comment record failed";
            break;

        case GSF_HISTORY_RECORD_DECODE_FAILED:
            ptr = "GSF Error: Decoding history record failed";
            break;

        case GSF_NAV_ERROR_RECORD_DECODE_FAILED:
            ptr = "GSF Error: Decoding navigation error record failed";
            break;

#if 0
        case GSF_HEADER_RECORD_ENCODE_FAILED:
            ptr = "GSF Error: Encoding header record failed";
            break;

        case GSF_MB_PING_RECORD_ENCODE_FAILED:
            ptr = "GSF Error: Encoding multibeam ping record failed";
            break;

         case GSF_SVP_RECORD_ENCODE_FAILED:
            ptr = "GSF Error: Encoding SVP record failed";
            break;

         case GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED:
            ptr = "GSF Error: Encoding processing parameters record failed";
            break;

         case GSF_SENSOR_PARAM_RECORD_ENCODE_FAILED:
            ptr = "GSF Error: Encoding sensor parameters record failed";
            break;

         case GSF_COMMENT_RECORD_ENCODE_FAILED:
            ptr = "GSF Error: Encoding comment record failed";
            break;

         case GSF_HISTORY_RECORD_ENCODE_FAILED:
            ptr = "GSF Error: Encoding history record failed";
            break;

         case GSF_NAV_ERROR_RECORD_ENCODE_FAILED:
            ptr = "GSF Error: Encoding navigation error record failed";
            break;
#endif

         case GSF_SETVBUF_ERROR:
            ptr = "GSF Error: Setting internal file buffering failed";
            break;

         case GSF_FLUSH_ERROR:
            ptr = "GSF Error: Flushing data buffer(s) failed";
            break;

         case GSF_FILE_TELL_ERROR:
            ptr = "GSF Error: File tell failed";
            break;

        case GSF_INDEX_FILE_OPEN_ERROR:
            ptr = "GSF Error: Open of index file failed";
            break;

        case GSF_CORRUPT_INDEX_FILE_ERROR:
            ptr = "GSF Error: Index file is corrupt (delete index file)";
            break;

        case GSF_SCALE_INDEX_CALLOC_ERROR:
            ptr = "GSF Error: Allocation of scale factor index memory failed";
            break;

        case GSF_RECORD_TYPE_NOT_AVAILABLE:
            ptr = "GSF Error: Requested indexed record type not in GSF file";
            break;

        case GSF_SUMMARY_RECORD_DECODE_FAILED:
            ptr = "GSF Error: Decoding summary record failed";
            break;

        case GSF_SUMMARY_RECORD_ENCODE_FAILED:
            ptr = "GSF Error: Encoding summary record failed";
            break;

        case GSF_INVALID_NUM_BEAMS:
            ptr = "GSF Error: Invalid number of beams/samples";
            break;

        case GSF_INVALID_RECORD_NUMBER:
            ptr = "GSF Error: Invalid record number";
            break;

        case GSF_INDEX_FILE_READ_ERROR:
            ptr = "GSF Error: Index file read error";
            break;

        case GSF_PARAM_SIZE_FIXED:
            ptr = "GSF Error: Unable to update existing file with increased record size";
            break;

        case GSF_SINGLE_BEAM_ENCODE_FAILED:
            ptr = "GSF Error: Encoding single beam record failed";
            break;

        case GSF_HV_NAV_ERROR_RECORD_ENCODE_FAILED:
            ptr = "GSF Error: Encoding horizontal/vertical navigation error record failed";
            break;

        case GSF_HV_NAV_ERROR_RECORD_DECODE_FAILED:
            ptr = "GSF Error: Decoding horizontal/vertical navigation error record failed";
            break;

        case GSF_ATTITUDE_RECORD_ENCODE_FAILED:
            ptr = "GSF Error: Decoding attitude record failed";
            break;

        case GSF_ATTITUDE_RECORD_DECODE_FAILED:
            ptr = "GSF Error: Decoding attitude record failed";
            break;

        case GSF_OPEN_TEMP_FILE_FAILED:
            ptr = "GSF Error: Failed to open temporary file for index creation";
            break;

        case GSF_PARTIAL_RECORD_AT_END_OF_FILE:
            ptr = "GSF Error: Corrupt/partial record at the end of the file";
            break;

        case GSF_QUALITY_FLAGS_DECODE_ERROR:
            ptr = "GSF Error: Decoding quality flags record failed";
            break;

        case GSF_COMPRESSION_UNSUPPORTED:
            ptr = "GSF Error: Compression method unsupported";
            break;

        case GSF_COMPRESSION_FAILED:
            ptr = "GSF Error: Compression/uncompression failed";
            break;

        default:
            ptr = "GSF Error: Unknown error";
            break;
    }

    return (ptr);
}

/********************************************************************
 *
 * Function Name : gsfIndexTime
 *
 * Description : This function returns the time (Posix.4) associated with
 *  a specified record number and type.  It also returns the record number
 *  that was read.
 *
 * Inputs :
 *   handle = GSF file handle assigned by gsfOpen or gsfOpenBuffered.
 *   record_type = Record type to be retrieved.
 *   record_number = Record number to be retrieved (-1 will get the time
 *                   and record number of the last record of this type).
 *   sec = Posix.4 seconds.
 *   nsec = Posix.4 nanoseconds.
 *
 * Returns :
 *  This function returns the record number if successful, or -1 if an
 *  error occured.
 *
 * Error Conditions :
 *   GSF_BAD_FILE_HANDLE
 *   GSF_UNRECOGNIZED_RECORD_ID
 *   GSF_INVALID_RECORD_NUMBER
 *   GSF_RECORD_TYPE_NOT_AVAILABLE
 *   GSF_FILE_SEEK_ERROR
 *   GSF_INDEX_FILE_READ_ERROR
 *
 ********************************************************************/

int
gsfIndexTime(int handle, int record_type, int record_number, time_t * sec, long *nsec)
{
    long long       addr;
    int             offset;
    INDEX_REC       index_rec;

    *sec = 0;
    *nsec = 0;

    /* Make sure we have a valid file handle. */
    if ((handle < 1) || (handle > GSF_MAX_OPEN_FILES))
    {
        gsfError = GSF_BAD_FILE_HANDLE;
        return (-1);
    }

    /* Make sure we have a valid recordID. */
    if ((record_type < 1) || (record_type >= NUM_REC_TYPES))
    {
        gsfError = GSF_UNRECOGNIZED_RECORD_ID;
        return (-1);
    }

    /* Make sure we have a valid record_number. */
    if ((record_number < -1) || (record_number == 0) ||
        (record_number > gsfFileTable[handle - 1].index_data.number_of_records[record_type]))
    {
        gsfError = GSF_INVALID_RECORD_NUMBER;
        return (-1);
    }

    /* Check the record_type to see if the requested type is available. */
    if (gsfFileTable[handle - 1].index_data.record_type[record_type] == -1)
    {
        gsfError = GSF_RECORD_TYPE_NOT_AVAILABLE;
        return (-1);
    }

    /*  If the record number requested is -1, use the last record of
     *  this type.
     */
    if (record_number == -1)
    {
        offset = gsfFileTable[handle - 1].index_data.number_of_records[record_type] - 1;
    }
    else
    {
        offset = record_number - 1;
    }

    /*  Compute the record address within the index file and read the
     *  index record.
     */
    addr = gsfFileTable[handle - 1].index_data.start_addr[record_type] +
        (offset * sizeof(INDEX_REC));
    if (fseek(gsfFileTable[handle - 1].index_data.fp, addr, SEEK_SET))
    {
        gsfError = GSF_FILE_SEEK_ERROR;
        return (-1);
    }
    if (fread(&index_rec, sizeof(INDEX_REC), 1, gsfFileTable[handle - 1].index_data.fp) != 1)
    {
        gsfError = GSF_INDEX_FILE_READ_ERROR;
        return (-1);
    }
    if (gsfFileTable[handle - 1].index_data.swap)
    {
        SwapLong((unsigned int *) &index_rec, 2);
        SwapLongLong((long long *) &(index_rec.addr), 1);
    }

    /*  Store the time and return the record number.    */
    *sec = index_rec.sec;
    *nsec = index_rec.nsec;

    return (offset+1);
}

/********************************************************************
 *
 * Function Name : gsfChecksum
 *
 * Description :  This function computes and returns the modulo-32 form
 *                 byte-wise sum of the num_bytes starting at buff.
 *
 * Inputs :
 *  buff = a pointer to an unsigned char buffer containing the data
 *  num_bytes = a integer containing the number of bytes of data
 *
 * Returns : a gsfuLong (u_int) data type containing the computed checksum.
 *
 * Error Conditions :
 *
 ********************************************************************/

static          gsfuLong
gsfChecksum(unsigned char *buff, unsigned int num_bytes)
{
    unsigned char  *ptr;
    gsfuLong        checkSum = 0;

    /*
    * Compute the checksum as the modulo-32 sum of all bytes
    * between the checksum value and the end of the record.
    */
    for (ptr = buff; ptr < buff + num_bytes; ptr++)
    {
        checkSum += *ptr;
    }
    return checkSum;
}

/********************************************************************
 *
 * Function Name : gsfPercent
 *
 * Description : This function returns an integer value representing
 *   the location of the file pointer as a percentage of the total file
 *   size.  It may be used to obtain an indication of how far along a
 *   program is in reading a GSF data file.  The file size is obtained
 *   when the file is opened.
 *
 * Inputs :
 *   handle = GSF file handle assigned by gsfOpen or gsfOpenBuffered
 *
 * Returns :
 *  This function returns the current file position as a percentage of
 *  the file size, or -1 if an error occurred. gsfError will be set to
 *  indicate the error.
 *
 * Error Conditions :
 *  GSF_BAD_FILE_HANDLE
 *  GSF_FILE_TELL_ERROR
 *
 ********************************************************************/

int
gsfPercent (int handle)
{
    int             percent;
    long long       addr, rc;

    /* Clear gsfError each time down */
    gsfError = 0;

    /* JSB 04/05/00 replaced ">=" with ">" */
    if ((handle < 1) || (handle > GSF_MAX_OPEN_FILES))
    {
        gsfError = GSF_BAD_FILE_HANDLE;
        return (-1);
    }

    /* the file is no longer open */
    if (!gsfFileTable[handle - 1].occupied)
    {
        gsfError = GSF_BAD_FILE_HANDLE;
        return (-1);
    }

    /* Retrieve the current file position. */
    rc = ftell (gsfFileTable[handle - 1].fp);
    if (rc == -1)
    {
        gsfError = GSF_FILE_TELL_ERROR;
        return (-1);
    }
    addr = rc;

    percent = 0;
    if (gsfFileTable[handle - 1].file_size > 0)
    {
        percent = 100.0 * (double)addr / (double)gsfFileTable[handle - 1].file_size;
    }
    return(percent);
}

/********************************************************************
 *
 * Function Name : gsfGetNumberRecords
 *
 * Description : This function will return the number of records of a
 *   given type to the caller. The number of records is retrieved from
 *   the index file, so the file must have been opened for direct
 *   access (GSF_READONLY_INDEX or GSF_UPDATE_INDEX).
 *
 * Inputs :
 *    handle = the handle to the file as provided by gsfOpen
 *    desiredRecord = the desired record or GSF_NEXT_RECORD
 *
 * Returns :
 *   This function returns the number of records of type desiredRecord
 *   contained in the GSF file designated by handle, or -1 if an error
 *   occurred.
 *
 * Error Conditions :
 *   GSF_BAD_FILE_HANDLE
 *   GSF_UNRECOGNIZED_RECORD_ID
 *   GSF_BAD_ACCESS_MODE
 *
 ********************************************************************/

int
gsfGetNumberRecords (int handle, int desiredRecord)
{
    /* Clear gsfError each time down */
    gsfError = 0;

    /* JSB 04/05/00 replaced ">=" with ">" */
    if ((handle < 1) || (handle > GSF_MAX_OPEN_FILES))
    {
        gsfError = GSF_BAD_FILE_HANDLE;
        return (-1);
    }

    if ((desiredRecord < 0) || (desiredRecord >= NUM_REC_TYPES))
    {
        gsfError = GSF_UNRECOGNIZED_RECORD_ID;
        return (-1);
    }

    if (gsfFileTable[handle - 1].direct_access == 0)
    {
        gsfError = GSF_BAD_ACCESS_MODE;
        return(-1);
    }

    return (gsfFileTable[handle - 1].index_data.number_of_records[desiredRecord]);
}

/********************************************************************
 *
 * Function Name : gsfCopyRecords
 *
 * Description : This function will copy all of the data contained in the
 *  source gsfRecords data structure to the target gsfRecords data
 *  structure. The target MUST be memset to zero before the first call to
 *  gsfCopyRecords.  This function allocates dynmanic memory which is NOT
 *  maintained by the library.  It is up to the calling application to
 *  release the memory allocated.  This may be done by maintaining the
 *  target data structure as static data, or by using gsfFree to release
 *  the memory.
 *
 * Inputs :
 *  target = a pointer to a gsfRecords data structure allocated by the
 *      calling application, into which the source data is to be copied.
 *  source = a pointer to a gsfRecords data structure allocated by the
 *      calling application, from which data is to be copied.
 *
 * Returns : This function returns zero if successful, or -1 if an error
 *  occurs.
 *
 * Error Conditions :
 *  GSF_MEMORY_ALLOCATION_FAILED
 *
 ********************************************************************/

int
gsfCopyRecords (gsfRecords *target, const gsfRecords *source)
{
    int             i;

    gsfError = 0;

    /* Copy the gsf header over to the  target */
    memcpy(&target->header, &source->header, sizeof(target->header));

    /* Copy the ping summary record over to the target */
    memcpy(&target->summary, &source->summary, sizeof(target->summary));

    /* Decide which arrays we need to allocate memory for */
    if (source->mb_ping.depth != (double *) NULL)
    {
        if (target->mb_ping.number_beams < source->mb_ping.number_beams)
        {
            if (target->mb_ping.depth != (double *) NULL) free (target->mb_ping.depth);
            target->mb_ping.depth = (double *) NULL;
        }
        if (target->mb_ping.depth == (double *) NULL)
        {
            target->mb_ping.depth = (double *) calloc (source->mb_ping.number_beams, sizeof(double));
            if (target->mb_ping.depth == (double *) NULL)
            {
                gsfError = GSF_MEMORY_ALLOCATION_FAILED;
                return(-1);
            }
        }
        memcpy (target->mb_ping.depth, source->mb_ping.depth, sizeof(double) * source->mb_ping.number_beams);
    }

    /* Decide which arrays we need to allocate memory for */
    if (source->mb_ping.nominal_depth != (double *) NULL)
    {
        if (target->mb_ping.number_beams < source->mb_ping.number_beams)
        {
            if (target->mb_ping.nominal_depth != (double *) NULL) free (target->mb_ping.nominal_depth);
            target->mb_ping.nominal_depth = (double *) NULL;
        }
        if (target->mb_ping.nominal_depth == (double *) NULL)
        {
            target->mb_ping.nominal_depth = (double *) calloc (source->mb_ping.number_beams, sizeof(double));
            if (target->mb_ping.nominal_depth == (double *) NULL)
            {
                gsfError = GSF_MEMORY_ALLOCATION_FAILED;
                return(-1);
            }
        }
        memcpy (target->mb_ping.nominal_depth, source->mb_ping.nominal_depth, sizeof(double) * source->mb_ping.number_beams);
    }

    if (source->mb_ping.across_track != (double *) NULL)
    {
        if (target->mb_ping.number_beams < source->mb_ping.number_beams)
        {
            if (target->mb_ping.across_track != (double *) NULL) free (target->mb_ping.across_track);
            target->mb_ping.across_track = (double *) NULL;
        }
        if (target->mb_ping.across_track == (double *) NULL)
        {
            target->mb_ping.across_track = (double *) calloc (source->mb_ping.number_beams, sizeof(double));
            if (target->mb_ping.across_track == (double *) NULL)
            {
                gsfError = GSF_MEMORY_ALLOCATION_FAILED;
                return(-1);
            }
        }
        memcpy (target->mb_ping.across_track, source->mb_ping.across_track, sizeof(double) * source->mb_ping.number_beams);
    }

    if (source->mb_ping.along_track != (double *) NULL)
    {
        if (target->mb_ping.number_beams < source->mb_ping.number_beams)
        {
            if (target->mb_ping.along_track != (double *) NULL) free (target->mb_ping.along_track);
            target->mb_ping.along_track = (double *) NULL;
        }
        if (target->mb_ping.along_track == (double *) NULL)
        {
            target->mb_ping.along_track = (double *) calloc (source->mb_ping.number_beams, sizeof(double));
            if (target->mb_ping.along_track == (double *) NULL)
            {
                gsfError = GSF_MEMORY_ALLOCATION_FAILED;
                return(-1);
            }
        }
        memcpy (target->mb_ping.along_track, source->mb_ping.along_track, sizeof(double) * source->mb_ping.number_beams);
    }

    if (source->mb_ping.travel_time != (double *) NULL)
    {
        if (target->mb_ping.number_beams < source->mb_ping.number_beams)
        {
            if (target->mb_ping.travel_time != (double *) NULL) free (target->mb_ping.travel_time);
            target->mb_ping.travel_time = (double *) NULL;
        }
        if (target->mb_ping.travel_time == (double *) NULL)
        {
            target->mb_ping.travel_time = (double *) calloc (source->mb_ping.number_beams, sizeof(double));
            if (target->mb_ping.travel_time == (double *) NULL)
            {
                gsfError = GSF_MEMORY_ALLOCATION_FAILED;
                return(-1);
            }
        }
        memcpy (target->mb_ping.travel_time, source->mb_ping.travel_time, sizeof(double) * source->mb_ping.number_beams);
    }

    if (source->mb_ping.beam_angle != (double *) NULL)
    {
        if (target->mb_ping.number_beams < source->mb_ping.number_beams)
        {
            if (target->mb_ping.beam_angle != (double *) NULL) free (target->mb_ping.beam_angle);
            target->mb_ping.beam_angle = (double *) NULL;
        }
        if (target->mb_ping.beam_angle == (double *) NULL)
        {
            target->mb_ping.beam_angle = (double *) calloc (source->mb_ping.number_beams, sizeof(double));
            if (target->mb_ping.beam_angle == (double *) NULL)
            {
                gsfError = GSF_MEMORY_ALLOCATION_FAILED;
                return(-1);
            }
        }
        memcpy (target->mb_ping.beam_angle, source->mb_ping.beam_angle, sizeof(double) * source->mb_ping.number_beams);
    }

    if (source->mb_ping.mc_amplitude != (double *) NULL)
    {
        if (target->mb_ping.number_beams < source->mb_ping.number_beams)
        {
            if (target->mb_ping.mc_amplitude != (double *) NULL) free (target->mb_ping.mc_amplitude);
            target->mb_ping.mc_amplitude = (double *) NULL;
        }
        if (target->mb_ping.mc_amplitude == (double *) NULL)
        {
            target->mb_ping.mc_amplitude = (double *) calloc (source->mb_ping.number_beams, sizeof(double));
            if (target->mb_ping.mc_amplitude == (double *) NULL)
            {
                gsfError = GSF_MEMORY_ALLOCATION_FAILED;
                return(-1);
            }
        }
        memcpy (target->mb_ping.mc_amplitude, source->mb_ping.mc_amplitude, sizeof(double) * source->mb_ping.number_beams);
    }

    if (source->mb_ping.mr_amplitude != (double *) NULL)
    {
        if (target->mb_ping.number_beams < source->mb_ping.number_beams)
        {
            if (target->mb_ping.mr_amplitude != (double *) NULL) free (target->mb_ping.mr_amplitude);
            target->mb_ping.mr_amplitude = (double *) NULL;
        }
        if (target->mb_ping.mr_amplitude == (double *) NULL)
        {
            target->mb_ping.mr_amplitude = (double *) calloc (source->mb_ping.number_beams, sizeof(double));
            if (target->mb_ping.mr_amplitude == (double *) NULL)
            {
                gsfError = GSF_MEMORY_ALLOCATION_FAILED;
                return(-1);
            }
        }
        memcpy (target->mb_ping.mr_amplitude, source->mb_ping.mr_amplitude, sizeof(double) * source->mb_ping.number_beams);
    }

    if (source->mb_ping.echo_width != (double *) NULL)
    {
        if (target->mb_ping.number_beams < source->mb_ping.number_beams)
        {
            if (target->mb_ping.echo_width != (double *) NULL) free (target->mb_ping.echo_width);
            target->mb_ping.echo_width = (double *) NULL;
        }
        if (target->mb_ping.echo_width == (double *) NULL)
        {
            target->mb_ping.echo_width = (double *) calloc (source->mb_ping.number_beams, sizeof(double));
            if (target->mb_ping.echo_width == (double *) NULL)
            {
                gsfError = GSF_MEMORY_ALLOCATION_FAILED;
                return(-1);
            }
        }
        memcpy (target->mb_ping.echo_width, source->mb_ping.echo_width, sizeof(double) * source->mb_ping.number_beams);
    }

    if (source->mb_ping.quality_factor != (double *) NULL)
    {
        if (target->mb_ping.number_beams < source->mb_ping.number_beams)
        {
            if (target->mb_ping.quality_factor != (double *) NULL) free (target->mb_ping.quality_factor);
            target->mb_ping.quality_factor = (double *) NULL;
        }
        if (target->mb_ping.quality_factor == (double *) NULL)
        {
            target->mb_ping.quality_factor = (double *) calloc (source->mb_ping.number_beams, sizeof(double));
            if (target->mb_ping.quality_factor == (double *) NULL)
            {
                gsfError = GSF_MEMORY_ALLOCATION_FAILED;
                return(-1);
            }
        }
        memcpy (target->mb_ping.quality_factor, source->mb_ping.quality_factor, sizeof(double) * source->mb_ping.number_beams);
    }

    if (source->mb_ping.receive_heave != (double *) NULL)
    {
        if (target->mb_ping.receive_heave == (double *) NULL)
        {
            target->mb_ping.receive_heave = (double *) calloc (source->mb_ping.number_beams, sizeof(double));
            if (target->mb_ping.receive_heave == (double *) NULL)
            {
                gsfError = GSF_MEMORY_ALLOCATION_FAILED;
                return(-1);
            }
        }
        memcpy (target->mb_ping.receive_heave, source->mb_ping.receive_heave, sizeof(double) * source->mb_ping.number_beams);
    }

    if (source->mb_ping.depth_error != (double *) NULL)
    {
        if (target->mb_ping.number_beams < source->mb_ping.number_beams)
        {
            if (target->mb_ping.depth_error != (double *) NULL) free (target->mb_ping.depth_error);
            target->mb_ping.depth_error = (double *) NULL;
        }
        if (target->mb_ping.depth_error == (double *) NULL)
        {
            target->mb_ping.depth_error = (double *) calloc (source->mb_ping.number_beams, sizeof(double));
            if (target->mb_ping.depth_error == (double *) NULL)
            {
                gsfError = GSF_MEMORY_ALLOCATION_FAILED;
                return(-1);
            }
        }
        memcpy (target->mb_ping.depth_error, source->mb_ping.depth_error, sizeof(double) * source->mb_ping.number_beams);
    }

    if (source->mb_ping.across_track_error != (double *) NULL)
    {
        if (target->mb_ping.number_beams < source->mb_ping.number_beams)
        {
            if (target->mb_ping.across_track_error != (double *) NULL) free (target->mb_ping.across_track_error);
            target->mb_ping.across_track_error = (double *) NULL;
        }
        if (target->mb_ping.across_track_error == (double *) NULL)
        {
            target->mb_ping.across_track_error = (double *) calloc (source->mb_ping.number_beams, sizeof(double));
            if (target->mb_ping.across_track_error == (double *) NULL)
            {
                gsfError = GSF_MEMORY_ALLOCATION_FAILED;
                return(-1);
            }
        }
        memcpy (target->mb_ping.across_track_error, source->mb_ping.across_track_error, sizeof(double) * source->mb_ping.number_beams);
    }

    if (source->mb_ping.along_track_error != (double *) NULL)
    {
        if (target->mb_ping.number_beams < source->mb_ping.number_beams)
        {
            if (target->mb_ping.along_track_error != (double *) NULL) free (target->mb_ping.along_track_error);
            target->mb_ping.along_track_error = (double *) NULL;
        }
        if (target->mb_ping.along_track_error == (double *) NULL)
        {
            target->mb_ping.along_track_error = (double *) calloc (source->mb_ping.number_beams, sizeof(double));
            if (target->mb_ping.along_track_error == (double *) NULL)
            {
                gsfError = GSF_MEMORY_ALLOCATION_FAILED;
                return(-1);
            }
        }
        memcpy (target->mb_ping.along_track_error, source->mb_ping.along_track_error, sizeof(double) * source->mb_ping.number_beams);
    }

    if (source->mb_ping.quality_flags != (unsigned char *) NULL)
    {
        if (target->mb_ping.number_beams < source->mb_ping.number_beams)
        {
            if (target->mb_ping.quality_flags != (unsigned char *) NULL) free (target->mb_ping.quality_flags);
            target->mb_ping.quality_flags = (unsigned char *) NULL;
        }
        if (target->mb_ping.quality_flags == (unsigned char *) NULL)
        {
            target->mb_ping.quality_flags = (unsigned char *) calloc (source->mb_ping.number_beams, sizeof(unsigned char));
            if (target->mb_ping.quality_flags == (unsigned char *) NULL)
            {
                gsfError = GSF_MEMORY_ALLOCATION_FAILED;
                return(-1);
            }
        }
        memcpy (target->mb_ping.quality_flags, source->mb_ping.quality_flags, sizeof(unsigned char) * source->mb_ping.number_beams);
    }

    if (source->mb_ping.beam_flags != (unsigned char *) NULL)
    {
        if (target->mb_ping.number_beams < source->mb_ping.number_beams)
        {
            if (target->mb_ping.beam_flags != (unsigned char *) NULL) free (target->mb_ping.beam_flags);
            target->mb_ping.beam_flags = (unsigned char *) NULL;
        }
        if (target->mb_ping.beam_flags == (unsigned char *) NULL)
        {
            target->mb_ping.beam_flags = (unsigned char *) calloc (source->mb_ping.number_beams, sizeof(unsigned char));
            if (target->mb_ping.beam_flags == (unsigned char *) NULL)
            {
                gsfError = GSF_MEMORY_ALLOCATION_FAILED;
                return(-1);
            }
        }
        memcpy (target->mb_ping.beam_flags, source->mb_ping.beam_flags, sizeof(unsigned char) * source->mb_ping.number_beams);
    }

    if (source->mb_ping.signal_to_noise != (double *) NULL)
    {
        if (target->mb_ping.number_beams < source->mb_ping.number_beams)
        {
            if (target->mb_ping.signal_to_noise != (double *) NULL) free (target->mb_ping.signal_to_noise);
            target->mb_ping.signal_to_noise = (double *) NULL;
        }
        if (target->mb_ping.signal_to_noise == (double *) NULL)
        {
            target->mb_ping.signal_to_noise = (double *) calloc (source->mb_ping.number_beams, sizeof(double));
            if (target->mb_ping.signal_to_noise == (double *) NULL)
            {
                gsfError = GSF_MEMORY_ALLOCATION_FAILED;
                return(-1);
            }
        }
        memcpy (target->mb_ping.signal_to_noise, source->mb_ping.signal_to_noise, sizeof(double) * source->mb_ping.number_beams);
    }

    if (source->mb_ping.beam_angle_forward != (double *) NULL)
    {
        if (target->mb_ping.number_beams < source->mb_ping.number_beams)
        {
            if (target->mb_ping.beam_angle_forward != (double *) NULL) free (target->mb_ping.beam_angle_forward);
            target->mb_ping.beam_angle_forward = (double *) NULL;
        }
        if (target->mb_ping.beam_angle_forward == (double *) NULL)
        {
            target->mb_ping.beam_angle_forward = (double *) calloc (source->mb_ping.number_beams, sizeof(double));
            if (target->mb_ping.beam_angle_forward == (double *) NULL)
            {
                gsfError = GSF_MEMORY_ALLOCATION_FAILED;
                return(-1);
            }
        }
        memcpy (target->mb_ping.beam_angle_forward, source->mb_ping.beam_angle_forward, sizeof(double) * source->mb_ping.number_beams);
    }

    if (source->mb_ping.TVG_dB != (double *) NULL)
    {
        if (target->mb_ping.number_beams < source->mb_ping.number_beams)
        {
            if (target->mb_ping.TVG_dB != (double *) NULL) free (target->mb_ping.TVG_dB);
            target->mb_ping.TVG_dB = (double *) NULL;
        }
        if (target->mb_ping.TVG_dB == (double *) NULL)
        {
            target->mb_ping.TVG_dB = (double *) calloc (source->mb_ping.number_beams, sizeof(double));
            if (target->mb_ping.TVG_dB == (double *) NULL)
            {
                gsfError = GSF_MEMORY_ALLOCATION_FAILED;
                return(-1);
            }
        }
        memcpy (target->mb_ping.TVG_dB, source->mb_ping.TVG_dB, sizeof(double) * source->mb_ping.number_beams);
    }

    if (source->mb_ping.vertical_error != (double *) NULL)
    {
        if (target->mb_ping.vertical_error == (double *) NULL)
        {
            target->mb_ping.vertical_error = (double *) calloc (source->mb_ping.number_beams, sizeof(double));
            if (target->mb_ping.vertical_error == (double *) NULL)
            {
                gsfError = GSF_MEMORY_ALLOCATION_FAILED;
                return(-1);
            }
        }
        memcpy (target->mb_ping.vertical_error, source->mb_ping.vertical_error, sizeof(double) * source->mb_ping.number_beams);
    }

    if (source->mb_ping.horizontal_error != (double *) NULL)
    {
        if (target->mb_ping.number_beams < source->mb_ping.number_beams)
        {
            if (target->mb_ping.horizontal_error != (double *) NULL) free (target->mb_ping.horizontal_error);
            target->mb_ping.horizontal_error = (double *) NULL;
        }
        if (target->mb_ping.horizontal_error == (double *) NULL)
        {
            target->mb_ping.horizontal_error = (double *) calloc (source->mb_ping.number_beams, sizeof(double));
            if (target->mb_ping.horizontal_error == (double *) NULL)
            {
                gsfError = GSF_MEMORY_ALLOCATION_FAILED;
                return(-1);
            }
        }
        memcpy (target->mb_ping.horizontal_error, source->mb_ping.horizontal_error, sizeof(double) * source->mb_ping.number_beams);
    }

    if (source->mb_ping.sector_number != (unsigned short *) NULL)
    {
        if (target->mb_ping.number_beams < source->mb_ping.number_beams)
        {
            if (target->mb_ping.sector_number != (unsigned short *) NULL) free (target->mb_ping.sector_number);
            target->mb_ping.sector_number = (unsigned short *) NULL;
        }
        if (target->mb_ping.sector_number == (unsigned short *) NULL)
        {
            target->mb_ping.sector_number = (unsigned short *) calloc (source->mb_ping.number_beams, sizeof(unsigned short));
            if (target->mb_ping.sector_number == (unsigned short *) NULL)
            {
                gsfError = GSF_MEMORY_ALLOCATION_FAILED;
                return(-1);
            }
        }
        memcpy (target->mb_ping.sector_number, source->mb_ping.sector_number, sizeof(unsigned short) * source->mb_ping.number_beams);
    }

    if (source->mb_ping.detection_info != (unsigned short *) NULL)
    {
        if (target->mb_ping.number_beams < source->mb_ping.number_beams)
        {
            if (target->mb_ping.detection_info != (unsigned short *) NULL) free (target->mb_ping.detection_info);
            target->mb_ping.detection_info = (unsigned short *) NULL;
        }
        if (target->mb_ping.detection_info == (unsigned short *) NULL)
        {
            target->mb_ping.detection_info = (unsigned short *) calloc (source->mb_ping.number_beams, sizeof(unsigned short));
            if (target->mb_ping.detection_info == (unsigned short *) NULL)
            {
                gsfError = GSF_MEMORY_ALLOCATION_FAILED;
                return(-1);
            }
        }
        memcpy (target->mb_ping.detection_info, source->mb_ping.detection_info, sizeof(unsigned short) * source->mb_ping.number_beams);
    }

    if (source->mb_ping.incident_beam_adj != (double *) NULL)
    {
        if (target->mb_ping.number_beams < source->mb_ping.number_beams)
        {
            if (target->mb_ping.incident_beam_adj != (double *) NULL) free (target->mb_ping.incident_beam_adj);
            target->mb_ping.incident_beam_adj = (double *) NULL;
        }
        if (target->mb_ping.incident_beam_adj == (double *) NULL)
        {
            target->mb_ping.incident_beam_adj = (double *) calloc (source->mb_ping.number_beams, sizeof(double));
            if (target->mb_ping.incident_beam_adj == (double *) NULL)
            {
                gsfError = GSF_MEMORY_ALLOCATION_FAILED;
                return(-1);
            }
        }
        memcpy (target->mb_ping.incident_beam_adj, source->mb_ping.incident_beam_adj, sizeof(double) * source->mb_ping.number_beams);
    }

    if (source->mb_ping.system_cleaning != (unsigned short *) NULL)
    {
        if (target->mb_ping.number_beams < source->mb_ping.number_beams)
        {
            if (target->mb_ping.system_cleaning != (unsigned short *) NULL) free (target->mb_ping.system_cleaning);
            target->mb_ping.system_cleaning = (unsigned short *) NULL;
        }
        if (target->mb_ping.system_cleaning == (unsigned short *) NULL)
        {
            target->mb_ping.system_cleaning = (unsigned short *) calloc (source->mb_ping.number_beams, sizeof(unsigned short));
            if (target->mb_ping.system_cleaning == (unsigned short *) NULL)
            {
                gsfError = GSF_MEMORY_ALLOCATION_FAILED;
                return(-1);
            }
        }
        memcpy (target->mb_ping.system_cleaning, source->mb_ping.system_cleaning, sizeof(unsigned short) * source->mb_ping.number_beams);
    }

    if (source->mb_ping.doppler_corr != (double *) NULL)
    {
        if (target->mb_ping.number_beams < source->mb_ping.number_beams)
        {
            if (target->mb_ping.doppler_corr != (double *) NULL) free (target->mb_ping.doppler_corr);
            target->mb_ping.doppler_corr = (double *) NULL;
        }
        if (target->mb_ping.doppler_corr == (double *) NULL)
        {
            target->mb_ping.doppler_corr = (double *) calloc (source->mb_ping.number_beams, sizeof(double));
            if (target->mb_ping.doppler_corr == (double *) NULL)
            {
                gsfError = GSF_MEMORY_ALLOCATION_FAILED;
                return(-1);
            }
        }
        memcpy (target->mb_ping.doppler_corr, source->mb_ping.doppler_corr, sizeof(double) * source->mb_ping.number_beams);
    }

    if (source->mb_ping.sonar_vert_uncert != (double *) NULL)
    {
        if (target->mb_ping.number_beams < source->mb_ping.number_beams)
        {
            if (target->mb_ping.sonar_vert_uncert != (double *) NULL) free (target->mb_ping.sonar_vert_uncert);
            target->mb_ping.sonar_vert_uncert = (double *) NULL;
        }
        if (target->mb_ping.sonar_vert_uncert == (double *) NULL)
        {
            target->mb_ping.sonar_vert_uncert = (double *) calloc (source->mb_ping.number_beams, sizeof(double));
            if (target->mb_ping.sonar_vert_uncert == (double *) NULL)
            {
                gsfError = GSF_MEMORY_ALLOCATION_FAILED;
                return(-1);
            }
        }
        memcpy (target->mb_ping.sonar_vert_uncert, source->mb_ping.sonar_vert_uncert, sizeof(double) * source->mb_ping.number_beams);
    }

    if (source->mb_ping.sonar_horz_uncert != (double *) NULL)
    {
        if (target->mb_ping.number_beams < source->mb_ping.number_beams)
        {
            if (target->mb_ping.sonar_horz_uncert != (double *) NULL) free (target->mb_ping.sonar_horz_uncert);
            target->mb_ping.sonar_horz_uncert = (double *) NULL;
        }
        if (target->mb_ping.sonar_horz_uncert == (double *) NULL)
        {
            target->mb_ping.sonar_horz_uncert = (double *) calloc (source->mb_ping.number_beams, sizeof(double));
            if (target->mb_ping.sonar_horz_uncert == (double *) NULL)
            {
                gsfError = GSF_MEMORY_ALLOCATION_FAILED;
                return(-1);
            }
        }
        memcpy (target->mb_ping.sonar_horz_uncert, source->mb_ping.sonar_horz_uncert, sizeof(double) * source->mb_ping.number_beams);
    }

    if (source->mb_ping.detection_window != (double *) NULL)
    {
        if (target->mb_ping.number_beams < source->mb_ping.number_beams)
        {
            if (target->mb_ping.detection_window != (double *) NULL) free (target->mb_ping.detection_window);
            target->mb_ping.detection_window = (double *) NULL;
        }
        if (target->mb_ping.detection_window == (double *) NULL)
        {
            target->mb_ping.detection_window = (double *) calloc (source->mb_ping.number_beams, sizeof(double));
            if (target->mb_ping.detection_window == (double *) NULL)
            {
                gsfError = GSF_MEMORY_ALLOCATION_FAILED;
                return(-1);
            }
        }
        memcpy (target->mb_ping.detection_window, source->mb_ping.detection_window, sizeof(double) * source->mb_ping.number_beams);
    }

    if (source->mb_ping.mean_abs_coeff != (double *) NULL)
    {
        if (target->mb_ping.number_beams < source->mb_ping.number_beams)
        {
            if (target->mb_ping.mean_abs_coeff != (double *) NULL) free (target->mb_ping.mean_abs_coeff);
            target->mb_ping.mean_abs_coeff = (double *) NULL;
        }
        if (target->mb_ping.mean_abs_coeff == (double *) NULL)
        {
            target->mb_ping.mean_abs_coeff = (double *) calloc (source->mb_ping.number_beams, sizeof(double));
            if (target->mb_ping.mean_abs_coeff == (double *) NULL)
            {
                gsfError = GSF_MEMORY_ALLOCATION_FAILED;
                return(-1);
            }
        }
        memcpy (target->mb_ping.mean_abs_coeff, source->mb_ping.mean_abs_coeff, sizeof(double) * source->mb_ping.number_beams);
    }

    if (source->mb_ping.brb_inten != (gsfBRBIntensity *) NULL)
    {
        if (target->mb_ping.brb_inten == (gsfBRBIntensity *) NULL)
        {
            target->mb_ping.brb_inten = (gsfBRBIntensity *) calloc (1, sizeof(gsfBRBIntensity));
            if (target->mb_ping.brb_inten == (gsfBRBIntensity *) NULL)
            {
                gsfError = GSF_MEMORY_ALLOCATION_FAILED;
                return(-1);
            }
        }

        target->mb_ping.brb_inten->bits_per_sample     = source->mb_ping.brb_inten->bits_per_sample;
        target->mb_ping.brb_inten->applied_corrections = source->mb_ping.brb_inten->applied_corrections;
        target->mb_ping.brb_inten->sensor_imagery      = source->mb_ping.brb_inten->sensor_imagery;
        memcpy (&target->mb_ping.brb_inten->spare, &source->mb_ping.brb_inten->spare, 16);

        if (source->mb_ping.brb_inten->time_series != (gsfTimeSeriesIntensity *) NULL)
        {
            if (target->mb_ping.number_beams < source->mb_ping.number_beams)
            {
                if (target->mb_ping.brb_inten->time_series != (gsfTimeSeriesIntensity *) NULL) free (target->mb_ping.brb_inten->time_series);
                target->mb_ping.brb_inten->time_series = (gsfTimeSeriesIntensity *) NULL;
            }
            if (target->mb_ping.brb_inten->time_series == (gsfTimeSeriesIntensity *) NULL)
            {
                target->mb_ping.brb_inten->time_series = (gsfTimeSeriesIntensity *) calloc (source->mb_ping.number_beams, sizeof(gsfTimeSeriesIntensity));
                if (target->mb_ping.brb_inten->time_series == (gsfTimeSeriesIntensity *) NULL)
                {
                    gsfError = GSF_MEMORY_ALLOCATION_FAILED;
                    return(-1);
                }
            }

            for (i = 0; i < source->mb_ping.number_beams; i++)
            {
                if (source->mb_ping.brb_inten->time_series[i].samples != (unsigned int *) NULL)
                {
                    if (target->mb_ping.brb_inten->time_series[i].sample_count < source->mb_ping.brb_inten->time_series[i].sample_count)
                    {
                        if (target->mb_ping.brb_inten->time_series[i].samples != (unsigned int *) NULL) free (target->mb_ping.brb_inten->time_series[i].samples);
                        target->mb_ping.brb_inten->time_series[i].samples = (unsigned int *) NULL;
                    }
                    if (target->mb_ping.brb_inten->time_series[i].samples == (unsigned int *) NULL)
                    {
                        target->mb_ping.brb_inten->time_series[i].samples = (unsigned int *) calloc (source->mb_ping.brb_inten->time_series[i].sample_count, sizeof(unsigned int));
                        if (target->mb_ping.brb_inten->time_series[i].samples == (unsigned int *) NULL)
                        {
                            gsfError = GSF_MEMORY_ALLOCATION_FAILED;
                            return(-1);
                        }
                    }
                    memcpy (target->mb_ping.brb_inten->time_series[i].samples, source->mb_ping.brb_inten->time_series[i].samples, sizeof(unsigned int) * source->mb_ping.brb_inten->time_series[i].sample_count);
                    target->mb_ping.brb_inten->time_series[i].sample_count = source->mb_ping.brb_inten->time_series[i].sample_count;
                    target->mb_ping.brb_inten->time_series[i].detect_sample = source->mb_ping.brb_inten->time_series[i].detect_sample;
                    target->mb_ping.brb_inten->time_series[i].start_range_samples = source->mb_ping.brb_inten->time_series[i].start_range_samples;
                }
            }
        }
    }

    /* Copy the swath bathymetry ping record over to the target by moving
     * the data over one item at a time so we don't overwrite the arrays.
     */
    target->mb_ping.ping_time           = source->mb_ping.ping_time;
    target->mb_ping.latitude            = source->mb_ping.latitude;
    target->mb_ping.longitude           = source->mb_ping.longitude;
    target->mb_ping.number_beams        = source->mb_ping.number_beams;
    target->mb_ping.center_beam         = source->mb_ping.center_beam;
    target->mb_ping.ping_flags          = source->mb_ping.ping_flags;
    target->mb_ping.reserved            = source->mb_ping.reserved;
    target->mb_ping.tide_corrector      = source->mb_ping.tide_corrector;
    target->mb_ping.gps_tide_corrector  = source->mb_ping.gps_tide_corrector;
    target->mb_ping.depth_corrector     = source->mb_ping.depth_corrector;
    target->mb_ping.heading             = source->mb_ping.heading;
    target->mb_ping.pitch               = source->mb_ping.pitch;
    target->mb_ping.roll                = source->mb_ping.roll;
    target->mb_ping.heave               = source->mb_ping.heave;
    target->mb_ping.course              = source->mb_ping.course;
    target->mb_ping.speed               = source->mb_ping.speed;
    target->mb_ping.height              = source->mb_ping.height;
    target->mb_ping.sep                 = source->mb_ping.sep;
    target->mb_ping.scaleFactors        = source->mb_ping.scaleFactors;
    target->mb_ping.sensor_id           = source->mb_ping.sensor_id;
    target->mb_ping.sensor_data         = source->mb_ping.sensor_data;

    /* Now hande the sound velocity profile dynamic memory */
    if (target->svp.depth == (double *) NULL)
    {
        target->svp.depth = (double *) calloc (source->svp.number_points, sizeof(double));
        if (target->svp.depth == (double *) NULL)
        {
            gsfError = GSF_MEMORY_ALLOCATION_FAILED;
            return(-1);
        }
        memcpy (target->svp.depth, source->svp.depth, sizeof(double) * source->svp.number_points);
    }
    else if (target->svp.number_points < source->svp.number_points)
    {
        double * dtemp = (double *) realloc (target->svp.depth, sizeof(double) * source->svp.number_points);
        if (dtemp == (double *) NULL)
        {
            free(target->svp.depth);
            target->svp.depth = (double *)NULL;
            gsfError = GSF_MEMORY_ALLOCATION_FAILED;
            return(-1);
        }
        target->svp.depth = dtemp;
        memcpy (target->svp.depth, source->svp.depth, sizeof(double) * source->svp.number_points);
    }

    if (target->svp.sound_speed == (double *) NULL)
    {
        target->svp.sound_speed = (double *) calloc (source->svp.number_points, sizeof(double));
        if (target->svp.sound_speed == (double *) NULL)
        {
            gsfError = GSF_MEMORY_ALLOCATION_FAILED;
            return(-1);
        }
        memcpy (target->svp.sound_speed, source->svp.sound_speed, sizeof(double) * source->svp.number_points);
    }
    else if (target->svp.number_points < source->svp.number_points)
    {
        double * dtemp = (double *) realloc (target->svp.sound_speed, sizeof(double) * source->svp.number_points);
        if (dtemp == (double *) NULL)
        {
            free(target->svp.sound_speed);
            target->svp.sound_speed = (double *)NULL;
            gsfError = GSF_MEMORY_ALLOCATION_FAILED;
            return(-1);
        }
        target->svp.sound_speed = dtemp;
        memcpy (target->svp.sound_speed, source->svp.sound_speed, sizeof(double) * source->svp.number_points);
    }

    /* Copy the sound velocity profile record from the source to the target */
    target->svp.observation_time = source->svp.observation_time;
    target->svp.application_time = source->svp.application_time;
    target->svp.latitude         = source->svp.latitude;
    target->svp.longitude        = source->svp.longitude;
    target->svp.number_points    = source->svp.number_points;

    /* Copy the processing parameters from the source to the target */
    target->process_parameters.param_time        = source->process_parameters.param_time;
    target->process_parameters.number_parameters = source->process_parameters.number_parameters;
    for (i=0; i<source->process_parameters.number_parameters; i++)
    {
        if (target->process_parameters.param_size[i] < source->process_parameters.param_size[i])
        {
            if (target->process_parameters.param[i] != (char *) NULL) free (target->process_parameters.param[i]);
            target->process_parameters.param[i] = (char *) NULL;
        }
        if (target->process_parameters.param[i] == (char *) NULL)
        {
            target->process_parameters.param[i] = (char *) calloc (source->process_parameters.param_size[i] + 1, sizeof(char));
            if (target->process_parameters.param[i] == (char *) NULL)
            {
                gsfError = GSF_MEMORY_ALLOCATION_FAILED;
                return(-1);
            }
        }
            strncpy (target->process_parameters.param[i], source->process_parameters.param[i], source->process_parameters.param_size[i] + 1);
            target->process_parameters.param_size[i] = source->process_parameters.param_size[i];
        }

    /* Copy the sensor parameters from the source to the target */
    target->sensor_parameters.param_time        = source->sensor_parameters.param_time;
    target->sensor_parameters.number_parameters = source->sensor_parameters.number_parameters;
    for (i=0; i<source->sensor_parameters.number_parameters; i++)
    {
        if (target->sensor_parameters.param_size[i] < source->sensor_parameters.param_size[i])
        {
            if (target->sensor_parameters.param[i] != (char *) NULL) free (target->sensor_parameters.param[i]);
            target->sensor_parameters.param[i] = (char *) NULL;
        }
        if (target->sensor_parameters.param[i] == (char *) NULL)
        {
            target->sensor_parameters.param[i] = (char *) calloc (source->sensor_parameters.param_size[i] + 1, sizeof(char));
            if (target->sensor_parameters.param[i] == (char *) NULL)
            {
                gsfError = GSF_MEMORY_ALLOCATION_FAILED;
                return(-1);
            }
        }
            strncpy (target->sensor_parameters.param[i], source->sensor_parameters.param[i], source->sensor_parameters.param_size[i] + 1);
            target->sensor_parameters.param_size[i] = source->sensor_parameters.param_size[i];
        }

    /* Copy the comment from the source to the target */
    target->comment.comment_time = source->comment.comment_time;
    target->comment.comment_length = source->comment.comment_length;
    if (source->comment.comment_length > 0)
    {
        if (target->comment.comment_length < source->comment.comment_length)
        {
            if (target->comment.comment != (char *) NULL) free (target->comment.comment);
            target->comment.comment = (char *) NULL;
        }
        if (target->comment.comment == (char *) NULL)
        {
            target->comment.comment = (char *) calloc (source->comment.comment_length + 1, sizeof(char));
            if (target->comment.comment == (char *) NULL)
            {
                gsfError = GSF_MEMORY_ALLOCATION_FAILED;
                return(-1);
            }
        }
            strncpy (target->comment.comment, source->comment.comment, source->comment.comment_length + 1);
            target->comment.comment_length = source->comment.comment_length;
        }

    /* Copy the history record from the source to the target */
    target->history.history_time = source->history.history_time;
    strncpy(target->history.host_name, source->history.host_name, GSF_HOST_NAME_LENGTH);
    strncpy(target->history.operator_name, source->history.operator_name, GSF_OPERATOR_LENGTH);

    if (target->history.command_line != (char *) NULL)
    {
        free(target->history.command_line);
        target->history.command_line = (char *) NULL;
    }
    if (source->history.command_line != (char *) NULL)
    {
        target->history.command_line = (char *) calloc (strlen(source->history.command_line) + 1, sizeof(char));
        if (target->history.command_line == (char *) NULL)
        {
            gsfError = GSF_MEMORY_ALLOCATION_FAILED;
            return(-1);
        }
        strncpy(target->history.command_line, source->history.command_line, strlen (source->history.command_line) + 1);
    }

    if (target->history.comment != (char *) NULL)
    {
        free(target->history.comment);
        target->history.comment = (char *) NULL;
    }
    if (source->history.comment != (char *) NULL)
    {
        target->history.comment = (char *) calloc (strlen(source->history.comment) + 1, sizeof(char));
        if (target->history.comment == (char *) NULL)
        {
            gsfError = GSF_MEMORY_ALLOCATION_FAILED;
            return(-1);
        }
        strncpy(target->history.comment, source->history.comment, strlen (source->history.comment) + 1);
    }

    /* Copy the navigation error record from the source to the target */
    target->nav_error = source->nav_error;

    /* Copy the non heap HV navigation error record from the source to the target */
    target->hv_nav_error.nav_error_time	  = source->hv_nav_error.nav_error_time;
    target->hv_nav_error.record_id        = source->hv_nav_error.record_id;
    target->hv_nav_error.horizontal_error = source->hv_nav_error.horizontal_error;
    target->hv_nav_error.vertical_error   = source->hv_nav_error.vertical_error;
    target->hv_nav_error.SEP_uncertainty  = source->hv_nav_error.SEP_uncertainty;

    /* Copy the HV navigation error position type from the source to the target */
    if (target->hv_nav_error.position_type != (char *) NULL)
    {
        free(target->hv_nav_error.position_type);
        target->hv_nav_error.position_type = (char *) NULL;
    }
    if (source->hv_nav_error.position_type != (char *) NULL)
    {
        target->hv_nav_error.position_type = (char *) calloc (strlen(source->hv_nav_error.position_type) + 1, sizeof(char));
        if (target->hv_nav_error.position_type == (char *) NULL)
        {
            gsfError = GSF_MEMORY_ALLOCATION_FAILED;
            return(-1);
        }
        strncpy(target->hv_nav_error.position_type, source->hv_nav_error.position_type, strlen (source->hv_nav_error.position_type) + 1);
    }

    /* Now hande the attitude record dynamic memory */
    if (source->attitude.num_measurements > 0)
    {
        if (target->attitude.num_measurements < source->attitude.num_measurements)
    	{
            if (target->attitude.attitude_time != (struct timespec *) NULL) free (target->attitude.attitude_time);
            if (target->attitude.roll != (double *) NULL) free (target->attitude.roll);
            if (target->attitude.pitch != (double *) NULL) free (target->attitude.pitch);
            if (target->attitude.heave != (double *) NULL) free (target->attitude.heave);
            if (target->attitude.heading != (double *) NULL) free (target->attitude.heading);
                target->attitude.attitude_time = (struct timespec *)NULL;
            target->attitude.roll = (double *) NULL;
            target->attitude.pitch = (double *) NULL;
            target->attitude.heave = (double *) NULL;
            target->attitude.heading = (double *) NULL;
            }
        if (target->attitude.attitude_time == (struct timespec *) NULL)
            target->attitude.attitude_time = (struct timespec *) calloc (source->attitude.num_measurements, sizeof(struct timespec));
    	if (target->attitude.roll == (double *) NULL)
            target->attitude.roll = (double *) calloc (source->attitude.num_measurements, sizeof(double));
        if (target->attitude.pitch == (double *) NULL)
            target->attitude.pitch = (double *) calloc (source->attitude.num_measurements, sizeof(double));
        if (target->attitude.heave == (double *) NULL)
            target->attitude.heave = (double *) calloc (source->attitude.num_measurements, sizeof(double));
        if (target->attitude.heading == (double *) NULL)
            target->attitude.heading = (double *) calloc (source->attitude.num_measurements, sizeof(double));

        if ((target->attitude.attitude_time == (struct timespec *) NULL) ||
            (target->attitude.roll == (double *) NULL) ||
            (target->attitude.pitch == (double *) NULL) ||
            (target->attitude.heave == (double *) NULL) ||
            (target->attitude.heading == (double *) NULL))
            {
                gsfError = GSF_MEMORY_ALLOCATION_FAILED;
                return(-1);
            }
        
        memcpy (target->attitude.attitude_time, source->attitude.attitude_time, sizeof(struct timespec) * source->attitude.num_measurements);
        memcpy (target->attitude.roll, source->attitude.roll, sizeof(double) * source->attitude.num_measurements);
        memcpy (target->attitude.pitch, source->attitude.pitch, sizeof(double) * source->attitude.num_measurements);
        memcpy (target->attitude.heave, source->attitude.heave, sizeof(double) * source->attitude.num_measurements);
            memcpy (target->attitude.heading, source->attitude.heading, sizeof(double) * source->attitude.num_measurements);
        }
    else
    {
	if (target->attitude.attitude_time != (struct timespec *) NULL)
	{
	     free(target->attitude.attitude_time);
	     target->attitude.attitude_time = NULL;
	}
	if (target->attitude.roll != (double *) NULL)
	{
	     free(target->attitude.roll);
	     target->attitude.roll = NULL;
	}
	if (target->attitude.pitch != (double *) NULL)
	{
	     free(target->attitude.pitch);
	     target->attitude.pitch = NULL;
	}
	if (target->attitude.heave != (double *) NULL)
	{
	     free(target->attitude.heave);
	     target->attitude.heave = NULL;
	}
	if (target->attitude.heading != (double *) NULL)
	{
	     free(target->attitude.heading);
	     target->attitude.heading = NULL;
	}
    }

    /* Copy the sound velocity profile record from the source to the target */
    target->attitude.num_measurements    = source->attitude.num_measurements;

    return(0);
}

/********************************************************************
 *
 * Function Name : gsfSetParam
 *
 * Description : This function allocates memory for the keyword=value
 *     style parameter and copies the parameter into the allocated space.
 *
 * Inputs :
 *     handle = an integer value containing the file handle, set by gsfOpen
 *     index  = an integer value specifing the index into the processing
 *         parameters array into which the value is to be written.
 *     val = a pointer to the character string containing the parameter.
 *     rec = a pointer to the gsfRecords data structure into which the
 *         parameter is written
 *
 * Returns : This function returns zero if successful, or -1 if an error
 *  occurs.
 *
 * Error Conditions :
 *   GSF_BAD_FILE_HANDLE
 *   GSF_MEMORY_ALLOCATION_FAILED
 *   GSF_PARAM_SIZE_FIXED
 *
 ********************************************************************/

static int
gsfSetParam(int handle, int index, const char *val, gsfRecords *rec)
{
    int             len;
    char           *ptr;

    if ((handle < 1) || (handle > GSF_MAX_OPEN_FILES))
    {
        gsfError = GSF_BAD_FILE_HANDLE;
        return (-1);
    }

    len = strlen (val);
    ptr = gsfFileTable[handle-1].rec.process_parameters.param[index];
    if (ptr == (char *) NULL)
    {
        ptr = (char *) calloc (len+1, sizeof(char));
        if (ptr == (char *) NULL)
        {
            gsfError = GSF_MEMORY_ALLOCATION_FAILED;
            return (-1);
        }
    }
    /* If memory has already been allocated, make sure we have enough space */
    else if (gsfFileTable[handle-1].rec.process_parameters.param_size[index] < len)
    {
        char *back_up_ptr;
        /* If the output file is open update, we cannot write a parameter
         * bigger in size than the one that exists on the disk now.
         */
        if ((gsfFileTable[handle-1].access_mode == GSF_UPDATE) ||
            (gsfFileTable[handle-1].access_mode == GSF_UPDATE_INDEX))
        {
            gsfError = GSF_PARAM_SIZE_FIXED;
            return(-1);
        }
        back_up_ptr = (char *) calloc (len+1, sizeof(char));
        if (back_up_ptr == (char *) NULL)
        {
            gsfError = GSF_MEMORY_ALLOCATION_FAILED;
            free(ptr);
            return (-1);
        }
        else
            ptr = back_up_ptr;
    }
    gsfFileTable[handle-1].rec.process_parameters.param[index] = ptr;
    gsfFileTable[handle-1].rec.process_parameters.param_size[index] = len;
    rec->process_parameters.param[index] = ptr;
    rec->process_parameters.param_size[index] = len;
    strncpy(rec->process_parameters.param[index], val, len+1);

    return(0);
}

/********************************************************************
 *
 * Function Name : gsfPutMBParams
 *
 * Description : This function moves swath bathymetry sonar processing
 *   parameters from internal form to "KEYWORD=VALUE" form.  The internal
 *   form parameters are read from a gsfMBParams data structure maintained
 *   by the caller.  The "KEYWORD=VALUE" form parameters are written into the
 *   processing_parameters structure of the gsfRecords data structure
 *   maintained by the caller. Parameters for up to two pairs of
 *   transmit/receive arrays are supported, for systems such as Reson SeaBat
 *   9002.
 *
 * Inputs :
 *   p = a pointer to the gsfMBParams data structure which contains
 *       the parameters in internal form.
 *   rec = a pointer to the gsfRecords data structure into which the
 *         parameters are to be written in the "KEYWORD=VALUE" form.
 *   handle = the integer handle to the file set by gsfOpen.
 *   numArrays = the integer value specifying the number of pairs of
 *               arrays which need to have separate parameters tracked.
 *
 * Returns : This function returns zero if successful, or -1 if an error
 *  occurs.
 *
 * Error Conditions :
 *   GSF_BAD_FILE_HANDLE
 *   GSF_PARAM_SIZE_FIXED
 *   GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED
 *   GSF_MEMORY_ALLOCATION_FAILED
 *
 ********************************************************************/
int
gsfPutMBParams(const gsfMBParams *p, gsfRecords *rec, int handle, int numArrays)
{
    char            temp[256];
    int             ret;
    int             number_parameters = 0, num_tx = 0, num_rx = 0;

    if ((handle < 1) || (handle > GSF_MAX_OPEN_FILES))
    {
        gsfError = GSF_BAD_FILE_HANDLE;
        return (-1);
    }

    /* If the file is open update, we do not want to allow a write with
     * a larger number of parameters than currently exist.
     */
    if ((gsfFileTable[handle-1].access_mode == GSF_UPDATE) ||
        (gsfFileTable[handle-1].access_mode == GSF_UPDATE_INDEX))
    {
        if ((gsfFileTable[handle-1].rec.process_parameters.number_parameters > 0) &&
            (gsfFileTable[handle-1].rec.process_parameters.number_parameters < GSF_NUMBER_PROCESSING_PARAMS))
        {
            gsfError = GSF_PARAM_SIZE_FIXED;
            return(-1);
        }
    }

    /* Load the text descriptor for the start of time epoch */
    snprintf(temp, sizeof(temp), "REFERENCE TIME=1970/001 00:00:00");
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    if ((p->number_of_transmitters < 1) || (p->number_of_transmitters > GSF_MAX_OFFSETS))
        num_tx = numArrays;
    else
        num_tx = p->number_of_transmitters;

    if ((p->number_of_receivers < 1) || (p->number_of_receivers > GSF_MAX_OFFSETS))
        num_rx = numArrays;
    else
        num_rx = p->number_of_receivers;

    /* DHG 2008/12/18 Add "PLATFORM_TYPE" Processing Parameter */
    if (p->vessel_type == GSF_PLATFORM_TYPE_AUV)
    {
        snprintf(temp, sizeof(temp), "PLATFORM_TYPE=AUV");
    }
    else if (p->vessel_type == GSF_PLATFORM_TYPE_ROTV)
    {
        snprintf(temp, sizeof(temp), "PLATFORM_TYPE=ROTV");
    }
    else // default to surface ship
    {
        snprintf(temp, sizeof(temp), "PLATFORM_TYPE=SURFACE_SHIP");
    }
    ret = gsfSetParam (handle, number_parameters++, temp, rec);
    if (ret)
    {
        return (-1);
    }

    if (p->full_raw_data == GSF_TRUE)
    {
        snprintf(temp, sizeof(temp), "FULL_RAW_DATA=TRUE ");
    }
    else
    {
        snprintf(temp, sizeof(temp), "FULL_RAW_DATA=FALSE");
    }
    ret = gsfSetParam (handle, number_parameters++, temp, rec);
    if (ret)
    {
        return (-1);
    }

    /* This parameter indicates whether the depth data has been roll compensated */
    if (p->roll_compensated == GSF_COMPENSATED)
    {
        snprintf(temp, sizeof(temp), "ROLL_COMPENSATED=YES");
    }
    else
    {
        snprintf(temp, sizeof(temp), "ROLL_COMPENSATED=NO ");
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* This parameter indicates whether the depth data has been pitch compensated */
    if (p->pitch_compensated == GSF_COMPENSATED)
    {
        snprintf(temp, sizeof(temp), "PITCH_COMPENSATED=YES");
    }
    else
    {
        snprintf(temp, sizeof(temp), "PITCH_COMPENSATED=NO ");
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* This parameter indicates whether the depth has been heave compensated */
    if (p->heave_compensated == GSF_COMPENSATED)
    {
        snprintf(temp, sizeof(temp), "HEAVE_COMPENSATED=YES");
    }
    else
    {
        snprintf(temp, sizeof(temp), "HEAVE_COMPENSATED=NO ");
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* This parameter indicates whether the depth has been tide compensated */
    if (p->tide_compensated == GSF_COMPENSATED)
    {
        snprintf(temp, sizeof(temp), "TIDE_COMPENSATED=YES");
    }
    else
    {
        snprintf(temp, sizeof(temp), "TIDE_COMPENSATED=NO ");
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* This parameter indicates the number of receivers. */
    if ((num_rx >= 1) && (num_rx <= 2))
    {
        snprintf(temp, sizeof(temp), "NUMBER_OF_RECEIVERS=%d", num_rx);
    }
    else
    {
        snprintf(temp, sizeof(temp), "NUMBER_OF_RECEIVERS=%s", GSF_UNKNOWN_PARAM_TEXT);
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* This parameter indicates the number of transmitters. */
    if ((num_tx >= 1) && (num_tx <= 2))
    {
        snprintf(temp, sizeof(temp), "NUMBER_OF_TRANSMITTERS=%d", num_tx);
    }
    else
    {
        snprintf(temp, sizeof(temp), "NUMBER_OF_TRANSMITTERS=%s", GSF_UNKNOWN_PARAM_TEXT);
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* If the depth field of the swath bathy ping data structure is true depth,
     * meaning depth is computed by integrating travel time through the sound
     * speed profile, then this parameter is set as DEPTH_CALCULATION=CORRECTED.
     * If the depth field of the swath bathymetry ping data structure is
     * relative to 1500 meters per second, then this parameter is set as
     * DEPTH_CALCULATION=RELATIVE_TO_1500_MS.
     */
    if (p->depth_calculation == GSF_TRUE_DEPTHS)
    {
        snprintf(temp, sizeof(temp), "DEPTH_CALCULATION=CORRECTED");
    }
    else if (p->depth_calculation == GSF_DEPTHS_RE_1500_MS)
    {
        snprintf(temp, sizeof(temp), "DEPTH_CALCULATION=RELATIVE_TO_1500_MS");
    }
    else
    {
        snprintf(temp, sizeof(temp), "DEPTH_CALCULATION=UNKNOWN");
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* This parameter indicates whether the angle travel time pairs
     * have been corrected for ray tracing.
     */
    if (p->ray_tracing == GSF_COMPENSATED)
    {
        snprintf(temp, sizeof(temp), "RAY_TRACING=YES");
    }
    else
    {
        snprintf(temp, sizeof(temp), "RAY_TRACING=NO");
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* This parameter indicates whether the motion sensor bias - measured from the
     *  patch test has been added to the attitude (roll, pitch, heading) data.
     */
    if (p->msb_applied_to_attitude == GSF_TRUE)
    {
        snprintf(temp, sizeof(temp), "MSB_APPLIED_TO_ATTITUDE=YES");
    }
    else
    {
        snprintf(temp, sizeof(temp), "MSB_APPLIED_TO_ATTITUDE=NO");
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* This parameter indicates whether the heave data has been subtracted from
     *  the GPS tide corrector value.
     */
    if (p->heave_removed_from_gps_tc == GSF_TRUE)
    {
        snprintf(temp, sizeof(temp), "HEAVE_REMOVED_FROM_GPS_TC=YES");
    }
    else
    {
        snprintf(temp, sizeof(temp), "HEAVE_REMOVED_FROM_GPS_TC=NO");
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* This parameter indicates the offset from UTC of the original data. */
    if ((p->utc_offset >= -12) && (p->utc_offset <= 12))
    {
        snprintf(temp, sizeof(temp), "UTC_OFFSET=%d", p->utc_offset);
    }
    else
    {
        snprintf(temp, sizeof(temp), "UTC_OFFSET=%s", GSF_UNKNOWN_PARAM_TEXT);
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* This parameter indicates the roll reference type. */
    if (p->roll_reference == GSF_HORIZONTAL_PITCH_AXIS)
    {
        snprintf(temp, sizeof(temp), "ROLL_REFERENCE=HORIZONTAL_PITCH_AXIS");
    }
    else if (p->roll_reference == GSF_ROTATED_PITCH_AXIS)
    {
        snprintf(temp, sizeof(temp), "ROLL_REFERENCE=ROTATED_PITCH_AXIS");
    }
    else
    {
        snprintf(temp, sizeof(temp), "ROLL_REFERENCE=%s", GSF_UNKNOWN_PARAM_TEXT);
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret) {
        return(-1);
    }

    /* The DRAFT_TO_APPLY parameter is a place holder for a new draft
     * value which is known, but not yet applied.
     */
    if (num_tx == 1)
    {
        if (p->to_apply.draft[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp, sizeof(temp), "DRAFT_TO_APPLY=%s",
                GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.draft[0] > GSF_MIN_PARAM) && (p->to_apply.draft[0] < GSF_MAX_PARAM))
        {
            snprintf(temp, sizeof(temp), "DRAFT_TO_APPLY=%+06.2f",
                p->to_apply.draft[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    else if (num_tx == 2)
    {
        if (p->to_apply.draft[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp, sizeof(temp), "DRAFT_TO_APPLY=%s,",
                GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.draft[0] > GSF_MIN_PARAM) && (p->to_apply.draft[0] < GSF_MAX_PARAM))
        {
            snprintf(temp, sizeof(temp), "DRAFT_TO_APPLY=%+06.2f,",
                p->to_apply.draft[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }

        if (p->to_apply.draft[1] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.draft[1] > GSF_MIN_PARAM) && (p->to_apply.draft[1] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f", p->to_apply.draft[1]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The PITCH_TO_APPLY parameter is a place holder for a pitch bias
     * value which is known but not yet applied.
     */
    if (num_tx == 1)
    {
        if (p->to_apply.pitch_bias[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp, sizeof(temp), "PITCH_TO_APPLY=%s",
                GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.pitch_bias[0] > GSF_MIN_PARAM) && (p->to_apply.pitch_bias[0] < GSF_MAX_PARAM))
        {
            snprintf(temp, sizeof(temp), "PITCH_TO_APPLY=%+06.2f",
                p->to_apply.pitch_bias[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    else if (num_tx == 2)
    {
        if (p->to_apply.pitch_bias[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp, sizeof(temp), "PITCH_TO_APPLY=%s,",
                GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.pitch_bias[0] > GSF_MIN_PARAM) && (p->to_apply.pitch_bias[0] < GSF_MAX_PARAM))
        {
            snprintf(temp, sizeof(temp), "PITCH_TO_APPLY=%+06.2f,",
                p->to_apply.pitch_bias[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->to_apply.pitch_bias[1] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.pitch_bias[1] > GSF_MIN_PARAM) && (p->to_apply.pitch_bias[1] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f", p->to_apply.pitch_bias[1]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The ROLL_TO_APPLY parameter is a place holder for a roll bias value
     * which is known, but not yet applied.
     */
    if (num_tx == 1)
    {
        if (p->to_apply.roll_bias[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp, sizeof(temp), "ROLL_TO_APPLY=%s",
                GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.roll_bias[0] > GSF_MIN_PARAM) && (p->to_apply.roll_bias[0] < GSF_MAX_PARAM))
        {
            snprintf(temp, sizeof(temp), "ROLL_TO_APPLY=%+06.2f",
                p->to_apply.roll_bias[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    else if (num_tx == 2)
    {
        if (p->to_apply.roll_bias[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp, sizeof(temp), "ROLL_TO_APPLY=%s,",
                GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.roll_bias[0] > GSF_MIN_PARAM) && (p->to_apply.roll_bias[0] < GSF_MAX_PARAM))
        {
            snprintf(temp, sizeof(temp), "ROLL_TO_APPLY=%+06.2f,",
                p->to_apply.roll_bias[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->to_apply.roll_bias[1] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.roll_bias[1] > GSF_MIN_PARAM) && (p->to_apply.roll_bias[1] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f", p->to_apply.roll_bias[1]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The GYRO_TO_APPLY parameter is a place holder for a gyro bias value
     * which is known, but not yet applied.
     */
    if (num_tx == 1)
    {
        if (p->to_apply.gyro_bias[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp, sizeof(temp), "GYRO_TO_APPLY=%s",
                GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.gyro_bias[0] > GSF_MIN_PARAM) && (p->to_apply.gyro_bias[0] < GSF_MAX_PARAM))
        {
            snprintf(temp, sizeof(temp), "GYRO_TO_APPLY=%+06.2f",
                p->to_apply.gyro_bias[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    else if (num_tx == 2)
    {
        if (p->to_apply.gyro_bias[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp, sizeof(temp), "GYRO_TO_APPLY=%s,",
                GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.gyro_bias[0] > GSF_MIN_PARAM) && (p->to_apply.gyro_bias[0] < GSF_MAX_PARAM))
        {
            snprintf(temp, sizeof(temp), "GYRO_TO_APPLY=%+06.2f,",
                p->to_apply.gyro_bias[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->to_apply.gyro_bias[1] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.gyro_bias[1] > GSF_MIN_PARAM) && (p->to_apply.gyro_bias[1] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f", p->to_apply.gyro_bias[1]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The POSITION_OFFSET_TO_APPLY parameter is place holder for a position
     *  offset which is known, but not yet applied.
     */
    memset(temp, 0, sizeof(temp));
    snprintf(temp, sizeof(temp), "POSITION_OFFSET_TO_APPLY=");
    if (p->to_apply.position_x_offset == GSF_UNKNOWN_PARAM_VALUE)
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,",
            GSF_UNKNOWN_PARAM_TEXT);
    }
    else if ((p->to_apply.position_x_offset > GSF_MIN_PARAM) && (p->to_apply.position_x_offset < GSF_MAX_PARAM))
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,",
            p->to_apply.position_x_offset);
    }
    else
    {
        gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
        return (-1);
    }
    if (p->to_apply.position_y_offset == GSF_UNKNOWN_PARAM_VALUE)
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,", GSF_UNKNOWN_PARAM_TEXT);
    }
    else if ((p->to_apply.position_y_offset > GSF_MIN_PARAM) && (p->to_apply.position_y_offset < GSF_MAX_PARAM))
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,",
            p->to_apply.position_y_offset);
    }
    else
    {
        gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
        return (-1);
    }
    if (p->to_apply.position_z_offset == GSF_UNKNOWN_PARAM_VALUE)
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s", GSF_UNKNOWN_PARAM_TEXT);
    }
    else if ((p->to_apply.position_z_offset > GSF_MIN_PARAM) && (p->to_apply.position_z_offset < GSF_MAX_PARAM))
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f",
            p->to_apply.position_z_offset);
    }
    else
    {
        gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
        return (-1);
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The ANTENNA_OFFSET_TO_APPLY parameter is place holder for a antenna
     *  offset which is known, but not yet applied.
     */
    memset(temp, 0, sizeof(temp));
    snprintf(temp, sizeof(temp), "ANTENNA_OFFSET_TO_APPLY=");
    if (p->to_apply.antenna_x_offset == GSF_UNKNOWN_PARAM_VALUE)
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,",
            GSF_UNKNOWN_PARAM_TEXT);
    }
    else if ((p->to_apply.antenna_x_offset > GSF_MIN_PARAM) && (p->to_apply.antenna_x_offset < GSF_MAX_PARAM))
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,",
            p->to_apply.antenna_x_offset);
    }
    else
    {
        gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
        return (-1);
    }
    if (p->to_apply.antenna_y_offset == GSF_UNKNOWN_PARAM_VALUE)
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,", GSF_UNKNOWN_PARAM_TEXT);
    }
    else if ((p->to_apply.antenna_y_offset > GSF_MIN_PARAM) && (p->to_apply.antenna_y_offset < GSF_MAX_PARAM))
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,",
            p->to_apply.antenna_y_offset);
    }
    else
    {
        gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
        return (-1);
    }
    if (p->to_apply.antenna_z_offset == GSF_UNKNOWN_PARAM_VALUE)
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s", GSF_UNKNOWN_PARAM_TEXT);
    }
    else if ((p->to_apply.antenna_z_offset > GSF_MIN_PARAM) && (p->to_apply.antenna_z_offset < GSF_MAX_PARAM))
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f",
            p->to_apply.antenna_z_offset);
    }
    else
    {
        gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
        return (-1);
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The TRANSDUCER_OFFSET_TO_APPLY parameter is place holder for a
     * transducer position offset which is known, but not yet applied.
     */
    memset(temp, 0, sizeof(temp));
    snprintf(temp, sizeof(temp), "TRANSDUCER_OFFSET_TO_APPLY=");
    if (num_tx == 1)
    {
        if (p->to_apply.transducer_x_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,",
                GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.transducer_x_offset[0] > GSF_MIN_PARAM) && (p->to_apply.transducer_x_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,",
                p->to_apply.transducer_x_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->to_apply.transducer_y_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,",
                GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.transducer_y_offset[0] > GSF_MIN_PARAM) && (p->to_apply.transducer_y_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,",
                p->to_apply.transducer_y_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->to_apply.transducer_z_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.transducer_z_offset[0] > GSF_MIN_PARAM) && (p->to_apply.transducer_z_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f",
                p->to_apply.transducer_z_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    else if (num_tx == 2)
    {
        if (p->to_apply.transducer_x_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,",
                GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.transducer_x_offset[0] > GSF_MIN_PARAM) && (p->to_apply.transducer_x_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,",
                p->to_apply.transducer_x_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->to_apply.transducer_y_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,",
                GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.transducer_y_offset[0] > GSF_MIN_PARAM) && (p->to_apply.transducer_y_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,",
                p->to_apply.transducer_y_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->to_apply.transducer_z_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,",
                GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.transducer_z_offset[0] > GSF_MIN_PARAM) && (p->to_apply.transducer_z_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,",
                p->to_apply.transducer_z_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->to_apply.transducer_x_offset[1] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,",
                GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.transducer_x_offset[1] > GSF_MIN_PARAM) && (p->to_apply.transducer_x_offset[1] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,",
                p->to_apply.transducer_x_offset[1]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->to_apply.transducer_y_offset[1] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,",
                GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.transducer_y_offset[1] > GSF_MIN_PARAM) && (p->to_apply.transducer_y_offset[1] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,",
                p->to_apply.transducer_y_offset[1]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->to_apply.transducer_z_offset[1] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s",
                GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.transducer_z_offset[1] > GSF_MIN_PARAM) && (p->to_apply.transducer_z_offset[1] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f",
                p->to_apply.transducer_z_offset[1]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The TRANSDUCER_PITCH_OFFSET_TO_APPLY parameter is a place holder for a transducer pitch angle
     *  installation offset which is known, but not yet applied.
     */
    if (num_tx == 1)
    {
        if (p->to_apply.transducer_pitch_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp, sizeof(temp), "TRANSDUCER_PITCH_OFFSET_TO_APPLY=%s",
                GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.transducer_pitch_offset[0] > GSF_MIN_PARAM) && (p->to_apply.transducer_pitch_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp, sizeof(temp), "TRANSDUCER_PITCH_OFFSET_TO_APPLY=%+06.2f",
                p->to_apply.transducer_pitch_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    else if (num_tx == 2)
    {
        if (p->to_apply.transducer_pitch_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp, sizeof(temp), "TRANSDUCER_PITCH_OFFSET_TO_APPLY=%s,", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.transducer_pitch_offset[0] > GSF_MIN_PARAM) && (p->to_apply.transducer_pitch_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp, sizeof(temp), "TRANSDUCER_PITCH_OFFSET_TO_APPLY=%+06.2f,",
                p->to_apply.transducer_pitch_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->to_apply.transducer_pitch_offset[1] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.transducer_pitch_offset[1] > GSF_MIN_PARAM) && (p->to_apply.transducer_pitch_offset[1] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f", p->to_apply.transducer_pitch_offset[1]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The TRANSDUCER_ROLL_OFFSET_TO_APPLY parameter is a place holder for a transducer roll angle
     *  installation offset which is known, but not yet applied.
     */
    if (num_tx == 1)
    {
        if (p->to_apply.transducer_roll_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp, sizeof(temp), "TRANSDUCER_ROLL_OFFSET_TO_APPLY=%s",
                GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.transducer_roll_offset[0] > GSF_MIN_PARAM) && (p->to_apply.transducer_roll_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp, sizeof(temp), "TRANSDUCER_ROLL_OFFSET_TO_APPLY=%+06.2f",
                p->to_apply.transducer_roll_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    else if (num_tx == 2)
    {
        if (p->to_apply.transducer_roll_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp, sizeof(temp), "TRANSDUCER_ROLL_OFFSET_TO_APPLY=%s,", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.transducer_roll_offset[0] > GSF_MIN_PARAM) && (p->to_apply.transducer_roll_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp, sizeof(temp), "TRANSDUCER_ROLL_OFFSET_TO_APPLY=%+06.2f,",
                p->to_apply.transducer_roll_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->to_apply.transducer_roll_offset[1] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.transducer_roll_offset[1] > GSF_MIN_PARAM) && (p->to_apply.transducer_roll_offset[1] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f", p->to_apply.transducer_roll_offset[1]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The TRANSDUCER_HEADING_OFFSET_TO_APPLY parameter is a place holder for a transducer heading angle
     *  installation offset which is known, but not yet applied.
     */
    if (num_tx == 1)
    {
        if (p->to_apply.transducer_heading_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp, sizeof(temp), "TRANSDUCER_HEADING_OFFSET_TO_APPLY=%s",
                GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.transducer_heading_offset[0] > GSF_MIN_PARAM) && (p->to_apply.transducer_heading_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp, sizeof(temp), "TRANSDUCER_HEADING_OFFSET_TO_APPLY=%+06.2f",
                p->to_apply.transducer_heading_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    else if (num_tx == 2)
    {
        if (p->to_apply.transducer_heading_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp, sizeof(temp), "TRANSDUCER_HEADING_OFFSET_TO_APPLY=%s,",
                GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.transducer_heading_offset[0] > GSF_MIN_PARAM) && (p->to_apply.transducer_heading_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp, sizeof(temp), "TRANSDUCER_HEADING_OFFSET_TO_APPLY=%+06.2f,",
                p->to_apply.transducer_heading_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->to_apply.transducer_heading_offset[1] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.transducer_heading_offset[1] > GSF_MIN_PARAM) && (p->to_apply.transducer_heading_offset[1] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f", p->to_apply.transducer_heading_offset[1]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The MRU_PITCH_TO_APPLY parameter is place holder for a motion
     * sensor pitch bias value which is known but not yet applied.
     */
    memset(temp, 0, sizeof(temp));
    if (p->to_apply.mru_pitch_bias == GSF_UNKNOWN_PARAM_VALUE)
    {
        snprintf(temp, sizeof(temp), "MRU_PITCH_TO_APPLY=%s",
            GSF_UNKNOWN_PARAM_TEXT);
    }
    else if ((p->to_apply.mru_pitch_bias > GSF_MIN_PARAM) && (p->to_apply.mru_pitch_bias < GSF_MAX_PARAM))
    {
        snprintf(temp, sizeof(temp), "MRU_PITCH_TO_APPLY=%+06.2f",
            p->to_apply.mru_pitch_bias);
    }
    else
    {
        gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
        return (-1);
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The MRU_ROLL_TO_APPLY parameter is place holder for a motion
     * sensor roll bias value which is known but not yet applied.
     */
    memset(temp, 0, sizeof(temp));
    if (p->to_apply.mru_roll_bias == GSF_UNKNOWN_PARAM_VALUE)
    {
        snprintf(temp, sizeof(temp), "MRU_ROLL_TO_APPLY=%s",
            GSF_UNKNOWN_PARAM_TEXT);
    }
    else if ((p->to_apply.mru_roll_bias > GSF_MIN_PARAM) && (p->to_apply.mru_roll_bias < GSF_MAX_PARAM))
    {
        snprintf(temp, sizeof(temp), "MRU_ROLL_TO_APPLY=%+06.2f",
            p->to_apply.mru_roll_bias);
    }
    else
    {
        gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
        return (-1);
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The MRU_HEADING_TO_APPLY parameter is place holder for a motion
     * sensor heading bias value which is known but not yet applied.
     */
    memset(temp, 0, sizeof(temp));
    if (p->to_apply.mru_heading_bias == GSF_UNKNOWN_PARAM_VALUE)
    {
        snprintf(temp, sizeof(temp), "MRU_HEADING_TO_APPLY=%s",
            GSF_UNKNOWN_PARAM_TEXT);
    }
    else if ((p->to_apply.mru_heading_bias > GSF_MIN_PARAM) && (p->to_apply.mru_heading_bias < GSF_MAX_PARAM))
    {
        snprintf(temp, sizeof(temp), "MRU_HEADING_TO_APPLY=%+06.2f",
            p->to_apply.mru_heading_bias);
    }
    else
    {
        gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
        return (-1);
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The MRU_OFFSET_TO_APPLY parameter is place holder for a mru
     *  offset which is known, but not yet applied.
     */
    memset(temp, 0, sizeof(temp));
    snprintf(temp, sizeof(temp), "MRU_OFFSET_TO_APPLY=");
    if (p->to_apply.mru_x_offset == GSF_UNKNOWN_PARAM_VALUE)
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,",
            GSF_UNKNOWN_PARAM_TEXT);
    }
    else if ((p->to_apply.mru_x_offset > GSF_MIN_PARAM) && (p->to_apply.mru_x_offset < GSF_MAX_PARAM))
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,",
            p->to_apply.mru_x_offset);
    }
    else
    {
        gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
        return (-1);
    }
    if (p->to_apply.mru_y_offset == GSF_UNKNOWN_PARAM_VALUE)
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,", GSF_UNKNOWN_PARAM_TEXT);
    }
    else if ((p->to_apply.mru_y_offset > GSF_MIN_PARAM) && (p->to_apply.mru_y_offset < GSF_MAX_PARAM))
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,",
            p->to_apply.mru_y_offset);
    }
    else
    {
        gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
        return (-1);
    }
    if (p->to_apply.mru_z_offset == GSF_UNKNOWN_PARAM_VALUE)
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s", GSF_UNKNOWN_PARAM_TEXT);
    }
    else if ((p->to_apply.mru_z_offset > GSF_MIN_PARAM) && (p->to_apply.mru_z_offset < GSF_MAX_PARAM))
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f",
            p->to_apply.mru_z_offset);
    }
    else
    {
        gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
        return (-1);
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The CENTER_OF_ROTATION_OFFSET_TO_APPLY parameter is place holder for a mru
     *  offset which is known, but not yet applied.
     */
    memset(temp, 0, sizeof(temp));
    snprintf(temp, sizeof(temp), "CENTER_OF_ROTATION_OFFSET_TO_APPLY=");
    if (p->to_apply.center_of_rotation_x_offset == GSF_UNKNOWN_PARAM_VALUE)
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,",
            GSF_UNKNOWN_PARAM_TEXT);
    }
    else if ((p->to_apply.center_of_rotation_x_offset > GSF_MIN_PARAM) && (p->to_apply.center_of_rotation_x_offset < GSF_MAX_PARAM))
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,",
            p->to_apply.center_of_rotation_x_offset);
    }
    else
    {
        gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
        return (-1);
    }
    if (p->to_apply.center_of_rotation_y_offset == GSF_UNKNOWN_PARAM_VALUE)
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,", GSF_UNKNOWN_PARAM_TEXT);
    }
    else if ((p->to_apply.center_of_rotation_y_offset > GSF_MIN_PARAM) && (p->to_apply.center_of_rotation_y_offset < GSF_MAX_PARAM))
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,",
            p->to_apply.center_of_rotation_y_offset);
    }
    else
    {
        gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
        return (-1);
    }
    if (p->to_apply.center_of_rotation_z_offset == GSF_UNKNOWN_PARAM_VALUE)
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s", GSF_UNKNOWN_PARAM_TEXT);
    }
    else if ((p->to_apply.center_of_rotation_z_offset > GSF_MIN_PARAM) && (p->to_apply.center_of_rotation_z_offset < GSF_MAX_PARAM))
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f",
            p->to_apply.center_of_rotation_z_offset);
    }
    else
    {
        gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
        return (-1);
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The POSITION_LATENCY_TO_APPLY parameter is a place holder for a navigation
     * sensor latency value which is known but not yet applied.
     */
    memset(temp, 0, sizeof(temp));
    if (p->to_apply.position_latency == GSF_UNKNOWN_PARAM_VALUE)
    {
        snprintf(temp, sizeof(temp), "POSITION_LATENCY_TO_APPLY=%s",
            GSF_UNKNOWN_PARAM_TEXT);
    }
    else if ((p->to_apply.position_latency > GSF_MIN_PARAM) && (p->to_apply.position_latency < GSF_MAX_PARAM))
    {
        snprintf(temp, sizeof(temp), "POSITION_LATENCY_TO_APPLY=%+06.3f",
            p->to_apply.position_latency);
    }
    else
    {
        gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
        return (-1);
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The ATTITUDE_LATENCY_TO_APPLY parameter is a place holder for an attitude
     * sensor latency value which is known but not yet applied.
     */
    memset(temp, 0, sizeof(temp));
    if (p->to_apply.attitude_latency == GSF_UNKNOWN_PARAM_VALUE)
    {
        snprintf(temp, sizeof(temp), "ATTITUDE_LATENCY_TO_APPLY=%s",
            GSF_UNKNOWN_PARAM_TEXT);
    }
    else if ((p->to_apply.attitude_latency > GSF_MIN_PARAM) && (p->to_apply.attitude_latency < GSF_MAX_PARAM))
    {
        snprintf(temp, sizeof(temp), "ATTITUDE_LATENCY_TO_APPLY=%+06.3f",
            p->to_apply.attitude_latency);
    }
    else
    {
        gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
        return (-1);
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The DEPTH_SENSOR_LATENCY_TO_APPLY parameter is a place holder for a depth
     *  sensor latency value which is known but not yet applied.
     */
    memset(temp, 0, sizeof(temp));
    if (p->to_apply.depth_sensor_latency == GSF_UNKNOWN_PARAM_VALUE)
    {
        snprintf(temp, sizeof(temp), "DEPTH_SENSOR_LATENCY_TO_APPLY=%s",
            GSF_UNKNOWN_PARAM_TEXT);
    }
    else if ((p->to_apply.depth_sensor_latency > GSF_MIN_PARAM) && (p->to_apply.depth_sensor_latency < GSF_MAX_PARAM))
    {
        snprintf(temp, sizeof(temp), "DEPTH_SENSOR_LATENCY_TO_APPLY=%+06.3f",
            p->to_apply.depth_sensor_latency);
    }
    else
    {
        gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
        return (-1);
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The DEPTH_SENSOR_OFFSET_TO_APPLY parameter is place holder for a depth
     *  sensor offset which is known, but not yet applied.
     */
    if (p->to_apply.depth_sensor_x_offset == GSF_UNKNOWN_PARAM_VALUE)
    {
        snprintf(temp, sizeof(temp), "DEPTH_SENSOR_OFFSET_TO_APPLY=%s,", GSF_UNKNOWN_PARAM_TEXT);
    }
    else if ((p->to_apply.depth_sensor_x_offset > GSF_MIN_PARAM) && (p->to_apply.depth_sensor_x_offset < GSF_MAX_PARAM))
    {
        snprintf(temp, sizeof(temp), "DEPTH_SENSOR_OFFSET_TO_APPLY=%+06.2f,",
                p->to_apply.depth_sensor_x_offset);
    }
    else
    {
        gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
        return (-1);
    }
    if (p->to_apply.depth_sensor_y_offset == GSF_UNKNOWN_PARAM_VALUE)
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,", GSF_UNKNOWN_PARAM_TEXT);
    }
    else if ((p->to_apply.depth_sensor_y_offset > GSF_MIN_PARAM) && (p->to_apply.depth_sensor_y_offset < GSF_MAX_PARAM))
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,",
            p->to_apply.depth_sensor_y_offset);
    }
    else
    {
        gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
        return (-1);
    }
    if (p->to_apply.depth_sensor_z_offset == GSF_UNKNOWN_PARAM_VALUE)
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s", GSF_UNKNOWN_PARAM_TEXT);
    }
    else if ((p->to_apply.depth_sensor_z_offset > GSF_MIN_PARAM) && (p->to_apply.depth_sensor_z_offset < GSF_MAX_PARAM))
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f", p->to_apply.depth_sensor_z_offset);
    }
    else
    {
        gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
        return (-1);
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The RX_TRANSDUCER_OFFSET_TO_APPLY parameter is place holder for a
     * receiver position offset which is known, but not yet applied.
     */
    snprintf(temp, sizeof(temp), "RX_TRANSDUCER_OFFSET_TO_APPLY=");
    if (num_rx == 1)
    {
        if (p->to_apply.rx_transducer_x_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.rx_transducer_x_offset[0] > GSF_MIN_PARAM) && (p->to_apply.rx_transducer_x_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,", p->to_apply.rx_transducer_x_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->to_apply.rx_transducer_y_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.rx_transducer_y_offset[0] > GSF_MIN_PARAM) && (p->to_apply.rx_transducer_y_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,", p->to_apply.rx_transducer_y_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->to_apply.rx_transducer_z_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.rx_transducer_z_offset[0] > GSF_MIN_PARAM) && (p->to_apply.rx_transducer_z_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f", p->to_apply.rx_transducer_z_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    else if (num_rx == 2)
    {
        if (p->to_apply.rx_transducer_x_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.rx_transducer_x_offset[0] > GSF_MIN_PARAM) && (p->to_apply.rx_transducer_x_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,", p->to_apply.rx_transducer_x_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->to_apply.rx_transducer_y_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.rx_transducer_y_offset[0] > GSF_MIN_PARAM) && (p->to_apply.rx_transducer_y_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,", p->to_apply.rx_transducer_y_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->to_apply.rx_transducer_z_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.rx_transducer_z_offset[0] > GSF_MIN_PARAM) && (p->to_apply.rx_transducer_z_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,", p->to_apply.rx_transducer_z_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->to_apply.rx_transducer_x_offset[1] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.rx_transducer_x_offset[1] > GSF_MIN_PARAM) && (p->to_apply.rx_transducer_x_offset[1] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,", p->to_apply.rx_transducer_x_offset[1]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->to_apply.rx_transducer_y_offset[1] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.rx_transducer_y_offset[1] > GSF_MIN_PARAM) && (p->to_apply.rx_transducer_y_offset[1] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,", p->to_apply.rx_transducer_y_offset[1]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->to_apply.rx_transducer_z_offset[1] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.rx_transducer_z_offset[1] > GSF_MIN_PARAM) && (p->to_apply.rx_transducer_z_offset[1] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f", p->to_apply.rx_transducer_z_offset[1]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The RX_TRANSDUCER_PITCH_OFFSET_TO_APPLY parameter is a place holder for a
     * receiver pitch offset which is known, but not yet applied.
     */
    snprintf(temp, sizeof(temp), "RX_TRANSDUCER_PITCH_OFFSET_TO_APPLY=");
    if (num_rx == 1)
    {
        if (p->to_apply.rx_transducer_pitch_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.rx_transducer_pitch_offset[0] > GSF_MIN_PARAM) && (p->to_apply.rx_transducer_pitch_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f", p->to_apply.rx_transducer_pitch_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    else if (num_rx == 2)
    {
        if (p->to_apply.rx_transducer_pitch_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.rx_transducer_pitch_offset[0] > GSF_MIN_PARAM) && (p->to_apply.rx_transducer_pitch_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,", p->to_apply.rx_transducer_pitch_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->to_apply.rx_transducer_pitch_offset[1] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.rx_transducer_pitch_offset[1] > GSF_MIN_PARAM) && (p->to_apply.rx_transducer_pitch_offset[1] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f", p->to_apply.rx_transducer_pitch_offset[1]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The RX_TRANSDUCER_ROLL_OFFSET_TO_APPLY parameter is a place holder for a
     * receiver roll offset which is known, but not yet applied.
     */
    snprintf(temp, sizeof(temp), "RX_TRANSDUCER_ROLL_OFFSET_TO_APPLY=");
    if (num_rx == 1)
    {
        if (p->to_apply.rx_transducer_roll_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.rx_transducer_roll_offset[0] > GSF_MIN_PARAM) && (p->to_apply.rx_transducer_roll_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f", p->to_apply.rx_transducer_roll_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    else if (num_rx == 2)
    {
        if (p->to_apply.rx_transducer_roll_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.rx_transducer_roll_offset[0] > GSF_MIN_PARAM) && (p->to_apply.rx_transducer_roll_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,", p->to_apply.rx_transducer_roll_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->to_apply.rx_transducer_roll_offset[1] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.rx_transducer_roll_offset[1] > GSF_MIN_PARAM) && (p->to_apply.rx_transducer_roll_offset[1] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f", p->to_apply.rx_transducer_roll_offset[1]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The RX_TRANSDUCER_HEADING_OFFSET_TO_APPLY parameter is a place holder for a
     * receiver heading offset which is known, but not yet applied.
     */
    snprintf(temp, sizeof(temp), "RX_TRANSDUCER_HEADING_OFFSET_TO_APPLY=");
    if (num_rx == 1)
    {
        if (p->to_apply.rx_transducer_heading_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.rx_transducer_heading_offset[0] > GSF_MIN_PARAM) && (p->to_apply.rx_transducer_heading_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f", p->to_apply.rx_transducer_heading_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    else if (num_rx == 2)
    {
        if (p->to_apply.rx_transducer_heading_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.rx_transducer_heading_offset[0] > GSF_MIN_PARAM) && (p->to_apply.rx_transducer_heading_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,", p->to_apply.rx_transducer_heading_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->to_apply.rx_transducer_heading_offset[1] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->to_apply.rx_transducer_heading_offset[1] > GSF_MIN_PARAM) && (p->to_apply.rx_transducer_heading_offset[1] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f", p->to_apply.rx_transducer_heading_offset[1]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /***** end of "to apply" parameters, on to "applied" ****/

    /* The APPLIED_DRAFT parameter defines the transducer draft value
     * previously applied to the depths.
     */
    if (num_tx == 1)
    {
        if (p->applied.draft[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp, sizeof(temp), "APPLIED_DRAFT=%s",
                GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.draft[0] > GSF_MIN_PARAM) && (p->applied.draft[0] < GSF_MAX_PARAM))
        {
            snprintf(temp, sizeof(temp), "APPLIED_DRAFT=%+06.2f",
                p->applied.draft[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    else if (num_tx == 2)
    {
        if (p->applied.draft[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp, sizeof(temp), "APPLIED_DRAFT=%s,", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.draft[0] > GSF_MIN_PARAM) && (p->applied.draft[0] < GSF_MAX_PARAM))
        {
            snprintf(temp, sizeof(temp), "APPLIED_DRAFT=%+06.2f,", p->applied.draft[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->applied.draft[1] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.draft[1] > GSF_MIN_PARAM) && (p->applied.draft[1] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f", p->applied.draft[1]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The APPLIED_PITCH_BIAS parameter defines the pitch bias previously
     * applied.
     */
    if (num_tx == 1)
    {
        if (p->applied.pitch_bias[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp, sizeof(temp), "APPLIED_PITCH_BIAS=%s",
                GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.pitch_bias[0] > GSF_MIN_PARAM) && (p->applied.pitch_bias[0] < GSF_MAX_PARAM))
        {
            snprintf(temp, sizeof(temp), "APPLIED_PITCH_BIAS=%+06.2f",
                p->applied.pitch_bias[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    else if (num_tx == 2)
    {
        if (p->applied.pitch_bias[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp, sizeof(temp), "APPLIED_PITCH_BIAS=%s,", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.pitch_bias[0] > GSF_MIN_PARAM) && (p->applied.pitch_bias[0] < GSF_MAX_PARAM))
        {
            snprintf(temp, sizeof(temp), "APPLIED_PITCH_BIAS=%+06.2f,", p->applied.pitch_bias[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->applied.pitch_bias[1] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.pitch_bias[1] > GSF_MIN_PARAM) && (p->applied.pitch_bias[1] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f", p->applied.pitch_bias[1]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The APPLIED_ROLL_BIAS parameter defines the roll bias previously
     * applied to the data.
     */
    if (num_tx == 1)
    {
        if (p->applied.roll_bias[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp, sizeof(temp), "APPLIED_ROLL_BIAS=%s",
                GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.roll_bias[0] > GSF_MIN_PARAM) && (p->applied.roll_bias[0] < GSF_MAX_PARAM))
        {
            snprintf(temp, sizeof(temp), "APPLIED_ROLL_BIAS=%+06.2f",
                p->applied.roll_bias[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    else if (num_tx == 2)
    {
        if (p->applied.roll_bias[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp, sizeof(temp), "APPLIED_ROLL_BIAS=%s,", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.roll_bias[0] > GSF_MIN_PARAM) && (p->applied.roll_bias[0] < GSF_MAX_PARAM))
        {
            snprintf(temp, sizeof(temp), "APPLIED_ROLL_BIAS=%+06.2f,", p->applied.roll_bias[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->applied.roll_bias[1] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.roll_bias[1] > GSF_MIN_PARAM) && (p->applied.roll_bias[1] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f", p->applied.roll_bias[1]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The APPLIED_GYRO_BIAS parameter defines the gyro bias previously
     * applied to the data.
     */
    if (num_tx == 1)
    {
        if (p->applied.gyro_bias[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp, sizeof(temp), "APPLIED_GYRO_BIAS=%s",
                GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.gyro_bias[0] > GSF_MIN_PARAM) && (p->applied.gyro_bias[0] < GSF_MAX_PARAM))
        {
            snprintf(temp, sizeof(temp), "APPLIED_GYRO_BIAS=%+06.2f",
                p->applied.gyro_bias[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    else if (num_tx == 2)
    {
        if (p->applied.gyro_bias[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp, sizeof(temp), "APPLIED_GYRO_BIAS=%s,", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.gyro_bias[0] > GSF_MIN_PARAM) && (p->applied.gyro_bias[0] < GSF_MAX_PARAM))
        {
            snprintf(temp, sizeof(temp), "APPLIED_GYRO_BIAS=%+06.2f,", p->applied.gyro_bias[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->applied.gyro_bias[1] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.gyro_bias[1] > GSF_MIN_PARAM) && (p->applied.gyro_bias[1] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f", p->applied.gyro_bias[1]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The APPLIED_POSITION_OFFSET parameter defines the x,y,z position in
     * ship coordinates to which the lat lons are relative.
     */
    memset(temp, 0, sizeof(temp));
    snprintf(temp, sizeof(temp), "APPLIED_POSITION_OFFSET=");
    if (p->applied.position_x_offset == GSF_UNKNOWN_PARAM_VALUE)
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,",
            GSF_UNKNOWN_PARAM_TEXT);
    }
    else if ((p->applied.position_x_offset > GSF_MIN_PARAM) && (p->applied.position_x_offset < GSF_MAX_PARAM))
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,",
            p->applied.position_x_offset);
    }
    else
    {
        gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
        return (-1);
    }
    if (p->applied.position_y_offset == GSF_UNKNOWN_PARAM_VALUE)
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,",
            GSF_UNKNOWN_PARAM_TEXT);
    }
    else if ((p->applied.position_y_offset > GSF_MIN_PARAM) && (p->applied.position_y_offset < GSF_MAX_PARAM))
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,",
            p->applied.position_y_offset);
    }
    else
    {
        gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
        return (-1);
    }
    if (p->applied.position_z_offset == GSF_UNKNOWN_PARAM_VALUE)
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s",
            GSF_UNKNOWN_PARAM_TEXT);
    }
    else if ((p->applied.position_z_offset > GSF_MIN_PARAM) && (p->applied.position_z_offset < GSF_MAX_PARAM))
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f",
            p->applied.position_z_offset);
    }
    else
    {
        gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
        return (-1);
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The APPLIED_ANTENNA_OFFSET parameter defines the x,y,z position of the antenna
     * in ship coordinates to which the lat lons are relative.
     */
    memset(temp, 0, sizeof(temp));
    snprintf(temp, sizeof(temp), "APPLIED_ANTENNA_OFFSET=");
    if (p->applied.antenna_x_offset == GSF_UNKNOWN_PARAM_VALUE)
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,",
            GSF_UNKNOWN_PARAM_TEXT);
    }
    else if ((p->applied.antenna_x_offset > GSF_MIN_PARAM) && (p->applied.antenna_x_offset < GSF_MAX_PARAM))
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,",
            p->applied.antenna_x_offset);
    }
    else
    {
        gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
        return (-1);
    }
    if (p->applied.antenna_y_offset == GSF_UNKNOWN_PARAM_VALUE)
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,",
            GSF_UNKNOWN_PARAM_TEXT);
    }
    else if ((p->applied.antenna_y_offset > GSF_MIN_PARAM) && (p->applied.antenna_y_offset < GSF_MAX_PARAM))
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,",
            p->applied.antenna_y_offset);
    }
    else
    {
        gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
        return (-1);
    }
    if (p->applied.antenna_z_offset == GSF_UNKNOWN_PARAM_VALUE)
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s",
            GSF_UNKNOWN_PARAM_TEXT);
    }
    else if ((p->applied.antenna_z_offset > GSF_MIN_PARAM) && (p->applied.antenna_z_offset < GSF_MAX_PARAM))
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f",
            p->applied.antenna_z_offset);
    }
    else
    {
        gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
        return (-1);
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The APPLIED_TRANSDUCER_OFFSET parameter defines the x,y,z offsets
     * in ship coordinates to which have been applied to refer the x,y,z
     * beam values to the ship reference point.
     */
    memset(temp, 0, sizeof(temp));
    snprintf(temp, sizeof(temp), "APPLIED_TRANSDUCER_OFFSET=");
    if (num_tx == 1)
    {
        if (p->applied.transducer_x_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,",
                GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.transducer_x_offset[0] > GSF_MIN_PARAM) && (p->applied.transducer_x_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,",
                p->applied.transducer_x_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->applied.transducer_y_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,",
                GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.transducer_y_offset[0] > GSF_MIN_PARAM) && (p->applied.transducer_y_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,",
                p->applied.transducer_y_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->applied.transducer_z_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s",
                GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.transducer_z_offset[0] > GSF_MIN_PARAM) && (p->applied.transducer_z_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f",
                p->applied.transducer_z_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    else if (num_tx == 2)
    {
        if (p->applied.transducer_x_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,",
                GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.transducer_x_offset[0] > GSF_MIN_PARAM) && (p->applied.transducer_x_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,",
                p->applied.transducer_x_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->applied.transducer_y_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,",
                GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.transducer_y_offset[0] > GSF_MIN_PARAM) && (p->applied.transducer_y_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,",
                p->applied.transducer_y_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->applied.transducer_z_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,",
                GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.transducer_z_offset[0] > GSF_MIN_PARAM) && (p->applied.transducer_z_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,",
                p->applied.transducer_z_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->applied.transducer_x_offset[1] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,",
                GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.transducer_x_offset[1] > GSF_MIN_PARAM) && (p->applied.transducer_x_offset[1] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,",
                p->applied.transducer_x_offset[1]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->applied.transducer_y_offset[1] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,",
                GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.transducer_y_offset[1] > GSF_MIN_PARAM) && (p->applied.transducer_y_offset[1] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,",
                p->applied.transducer_y_offset[1]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->applied.transducer_z_offset[1] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s",
                GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.transducer_z_offset[1] > GSF_MIN_PARAM) && (p->applied.transducer_z_offset[1] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f",
                p->applied.transducer_z_offset[1]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The APPLIED_TRANSDUCER_PITCH_OFFSET parameter defines the transducer pitch installation angle
     *   previously applied to the data.
     */
    if (num_tx == 1)
    {
        if (p->applied.transducer_pitch_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp, sizeof(temp), "APPLIED_TRANSDUCER_PITCH_OFFSET=%s",
                GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.transducer_pitch_offset[0] > GSF_MIN_PARAM) && (p->applied.transducer_pitch_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp, sizeof(temp), "APPLIED_TRANSDUCER_PITCH_OFFSET=%+06.2f",
                p->applied.transducer_pitch_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    else if (num_tx == 2)
    {
        if (p->applied.transducer_pitch_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp, sizeof(temp), "APPLIED_TRANSDUCER_PITCH_OFFSET=%s,",
                GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.transducer_pitch_offset[0] > GSF_MIN_PARAM) && (p->applied.transducer_pitch_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp, sizeof(temp), "APPLIED_TRANSDUCER_PITCH_OFFSET=%+06.2f,",
                p->applied.transducer_pitch_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->applied.transducer_pitch_offset[1] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.transducer_pitch_offset[1] > GSF_MIN_PARAM) && (p->applied.transducer_pitch_offset[1] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f", p->applied.transducer_pitch_offset[1]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The APPLIED_TRANSDUCER_ROLL_OFFSET parameter defines the transducer roll installation angle
     *   previously applied to the data.
     */
    if (num_tx == 1)
    {
        if (p->applied.transducer_roll_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp, sizeof(temp), "APPLIED_TRANSDUCER_ROLL_OFFSET=%s",
                GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.transducer_roll_offset[0] > GSF_MIN_PARAM) && (p->applied.transducer_roll_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp, sizeof(temp), "APPLIED_TRANSDUCER_ROLL_OFFSET=%+06.2f",
                p->applied.transducer_roll_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    else if (num_tx == 2)
    {
        if (p->applied.transducer_roll_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp, sizeof(temp), "APPLIED_TRANSDUCER_ROLL_OFFSET=%s,",
                GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.transducer_roll_offset[0] > GSF_MIN_PARAM) && (p->applied.transducer_roll_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp, sizeof(temp), "APPLIED_TRANSDUCER_ROLL_OFFSET=%+06.2f,",
                p->applied.transducer_roll_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->applied.transducer_roll_offset[1] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.transducer_roll_offset[1] > GSF_MIN_PARAM) && (p->applied.transducer_roll_offset[1] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f", p->applied.transducer_roll_offset[1]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The APPLIED_TRANSDUCER_HEADING_OFFSET parameter defines the transducer heading installation angle
     *   previously applied to the data.
     */
    if (num_tx == 1)
    {
        if (p->applied.transducer_heading_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp, sizeof(temp), "APPLIED_TRANSDUCER_HEADING_OFFSET=%s",
                GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.transducer_heading_offset[0] > GSF_MIN_PARAM) && (p->applied.transducer_heading_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp, sizeof(temp), "APPLIED_TRANSDUCER_HEADING_OFFSET=%+06.2f",
                p->applied.transducer_heading_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    else if (num_tx == 2)
    {
        if (p->applied.transducer_heading_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp, sizeof(temp), "APPLIED_TRANSDUCER_HEADING_OFFSET=%s,",
                GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.transducer_heading_offset[0] > GSF_MIN_PARAM) && (p->applied.transducer_heading_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp, sizeof(temp), "APPLIED_TRANSDUCER_HEADING_OFFSET=%+06.2f,",
                p->applied.transducer_heading_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->applied.transducer_heading_offset[1] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.transducer_heading_offset[1] > GSF_MIN_PARAM) && (p->applied.transducer_heading_offset[1] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f", p->applied.transducer_heading_offset[1]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The APPLIED_MRU_ROLL parameter defines the roll bias previously
     * applied to the data.
     */
    memset(temp, 0, sizeof(temp));
    if (p->applied.mru_roll_bias == GSF_UNKNOWN_PARAM_VALUE)
    {
        snprintf(temp, sizeof(temp), "APPLIED_MRU_ROLL=%s",
            GSF_UNKNOWN_PARAM_TEXT);
    }
    else if ((p->applied.mru_roll_bias > GSF_MIN_PARAM) && (p->applied.mru_roll_bias < GSF_MAX_PARAM))
    {
        snprintf(temp, sizeof(temp), "APPLIED_MRU_ROLL=%+06.2f",
            p->applied.mru_roll_bias);
    }
    else
    {
        gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
        return (-1);
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The APPLIED_MRU_PITCH parameter defines the pitch bias previously
     * applied to the data.
     */
    memset(temp, 0, sizeof(temp));
    if (p->applied.mru_pitch_bias == GSF_UNKNOWN_PARAM_VALUE)
    {
        snprintf(temp, sizeof(temp), "APPLIED_MRU_PITCH=%s",
            GSF_UNKNOWN_PARAM_TEXT);
    }
    else if ((p->applied.mru_pitch_bias > GSF_MIN_PARAM) && (p->applied.mru_pitch_bias < GSF_MAX_PARAM))
    {
        snprintf(temp, sizeof(temp), "APPLIED_MRU_PITCH=%+06.2f",
            p->applied.mru_pitch_bias);
    }
    else
    {
        gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
        return (-1);
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The APPLIED_MRU_HEADING parameter defines the heading bias previously
     * applied to the data.
     */
    memset(temp, 0, sizeof(temp));
    if (p->applied.mru_heading_bias == GSF_UNKNOWN_PARAM_VALUE)
    {
        snprintf(temp, sizeof(temp), "APPLIED_MRU_HEADING=%s",
            GSF_UNKNOWN_PARAM_TEXT);
    }
    else if ((p->applied.mru_heading_bias > GSF_MIN_PARAM) && (p->applied.mru_heading_bias < GSF_MAX_PARAM))
    {
        snprintf(temp, sizeof(temp), "APPLIED_MRU_HEADING=%+06.2f",
            p->applied.mru_heading_bias);
    }
    else
    {
        gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
        return (-1);
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The APPLIED_MRU_OFFSET parameter defines the x,y,z offsets
     * in ship coordinates to which have been used to calculate a heave
     * difference between the motion sensor and the ship reference point.
     */
    memset(temp, 0, sizeof(temp));
    snprintf(temp, sizeof(temp), "APPLIED_MRU_OFFSET=");
    if (p->applied.mru_x_offset == GSF_UNKNOWN_PARAM_VALUE)
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,",
            GSF_UNKNOWN_PARAM_TEXT);
    }
    else if ((p->applied.mru_x_offset > GSF_MIN_PARAM) && (p->applied.mru_x_offset < GSF_MAX_PARAM))
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,",
            p->applied.mru_x_offset);
    }
    else
    {
        gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
        return (-1);
    }
    if (p->applied.mru_y_offset == GSF_UNKNOWN_PARAM_VALUE)
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,", GSF_UNKNOWN_PARAM_TEXT);
    }
    else if ((p->applied.mru_y_offset > GSF_MIN_PARAM) && (p->applied.mru_y_offset < GSF_MAX_PARAM))
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,",
            p->applied.mru_y_offset);
    }
    else
    {
        gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
        return (-1);
    }
    if (p->applied.mru_z_offset == GSF_UNKNOWN_PARAM_VALUE)
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s", GSF_UNKNOWN_PARAM_TEXT);
    }
    else if ((p->applied.mru_z_offset > GSF_MIN_PARAM) && (p->applied.mru_z_offset < GSF_MAX_PARAM))
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f",
            p->applied.mru_z_offset);
    }
    else
    {
        gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
        return (-1);
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The APPLIED_CENTER_OF_ROTATION_OFFSET parameter defines the x,y,z offsets
     * in ship coordinates to which have been used to calculate a heave
     * difference between the motion sensor and the ship reference point.
     */
    memset(temp, 0, sizeof(temp));
    snprintf(temp, sizeof(temp), "APPLIED_CENTER_OF_ROTATION_OFFSET=");
    if (p->applied.center_of_rotation_x_offset == GSF_UNKNOWN_PARAM_VALUE)
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,",
            GSF_UNKNOWN_PARAM_TEXT);
    }
    else if ((p->applied.center_of_rotation_x_offset > GSF_MIN_PARAM) && (p->applied.center_of_rotation_x_offset < GSF_MAX_PARAM))
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,",
            p->applied.center_of_rotation_x_offset);
    }
    else
    {
        gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
        return (-1);
    }
    if (p->applied.center_of_rotation_y_offset == GSF_UNKNOWN_PARAM_VALUE)
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,", GSF_UNKNOWN_PARAM_TEXT);
    }
    else if ((p->applied.center_of_rotation_y_offset > GSF_MIN_PARAM) && (p->applied.center_of_rotation_y_offset < GSF_MAX_PARAM))
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,",
            p->applied.center_of_rotation_y_offset);
    }
    else
    {
        gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
        return (-1);
    }
    if (p->applied.center_of_rotation_z_offset == GSF_UNKNOWN_PARAM_VALUE)
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s", GSF_UNKNOWN_PARAM_TEXT);
    }
    else if ((p->applied.center_of_rotation_z_offset > GSF_MIN_PARAM) && (p->applied.center_of_rotation_z_offset < GSF_MAX_PARAM))
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f",
            p->applied.center_of_rotation_z_offset);
    }
    else
    {
        gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
        return (-1);
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The APPLIED_POSITION_LATENCY parameter defines the navigation
     * sensor latency value which has already been applied.
     */
    memset(temp, 0, sizeof(temp));
    if (p->applied.position_latency == GSF_UNKNOWN_PARAM_VALUE)
    {
        snprintf(temp, sizeof(temp), "APPLIED_POSITION_LATENCY=%s",
            GSF_UNKNOWN_PARAM_TEXT);
    }
    else if ((p->applied.position_latency > GSF_MIN_PARAM) && (p->applied.position_latency < GSF_MAX_PARAM))
    {
        snprintf(temp, sizeof(temp), "APPLIED_POSITION_LATENCY=%+06.3f",
            p->applied.position_latency);
    }
    else
    {
        gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
        return (-1);
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The APPLIED_ATTITUDE_LATENCY parameter defines the attitude
     * sensor latency value which has already been applied.
     */
    memset(temp, 0, sizeof(temp));
    if (p->applied.attitude_latency == GSF_UNKNOWN_PARAM_VALUE)
    {
        snprintf(temp, sizeof(temp), "APPLIED_ATTITUDE_LATENCY=%s",
            GSF_UNKNOWN_PARAM_TEXT);
    }
    else if ((p->applied.attitude_latency > GSF_MIN_PARAM) && (p->applied.attitude_latency < GSF_MAX_PARAM))
    {
        snprintf(temp, sizeof(temp), "APPLIED_ATTITUDE_LATENCY=%+06.3f",
            p->applied.attitude_latency);
    }
    else
    {
        gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
        return (-1);
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The APPLIED_DEPTH_SENSOR_LATENCY parameter defines the depth
     *  sensor latency value which has already been applied.
     */
    memset(temp, 0, sizeof(temp));
    if (p->applied.depth_sensor_latency == GSF_UNKNOWN_PARAM_VALUE)
    {
        snprintf(temp, sizeof(temp), "APPLIED_DEPTH_SENSOR_LATENCY=%s",
            GSF_UNKNOWN_PARAM_TEXT);
    }
    else if ((p->applied.depth_sensor_latency > GSF_MIN_PARAM) && (p->applied.depth_sensor_latency < GSF_MAX_PARAM))
    {
        snprintf(temp, sizeof(temp), "APPLIED_DEPTH_SENSOR_LATENCY=%+06.3f",
            p->applied.depth_sensor_latency);
    }
    else
    {
        gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
        return (-1);
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The APPLIED_DEPTH_SENSOR_OFFSET parameter defines the x,y,z position
     * offsets that have been applied
     */
    snprintf(temp, sizeof(temp), "APPLIED_DEPTH_SENSOR_OFFSET=");
    if (p->applied.depth_sensor_x_offset == GSF_UNKNOWN_PARAM_VALUE)
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,", GSF_UNKNOWN_PARAM_TEXT);
    }
    else if ((p->applied.depth_sensor_x_offset > GSF_MIN_PARAM) && (p->applied.depth_sensor_x_offset < GSF_MAX_PARAM))
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,", p->applied.depth_sensor_x_offset);
    }
    else
    {
        gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
        return (-1);
    }
    if (p->applied.depth_sensor_y_offset == GSF_UNKNOWN_PARAM_VALUE)
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,", GSF_UNKNOWN_PARAM_TEXT);
    }
    else if ((p->applied.depth_sensor_y_offset > GSF_MIN_PARAM) && (p->applied.depth_sensor_y_offset < GSF_MAX_PARAM))
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,", p->applied.depth_sensor_y_offset);
    }
    else
    {
        gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
        return (-1);
    }
    if (p->applied.depth_sensor_z_offset == GSF_UNKNOWN_PARAM_VALUE)
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s", GSF_UNKNOWN_PARAM_TEXT);
    }
    else if ((p->applied.depth_sensor_z_offset > GSF_MIN_PARAM) && (p->applied.depth_sensor_z_offset < GSF_MAX_PARAM))
    {
        snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f", p->applied.depth_sensor_z_offset);
    }
    else
    {
        gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
        return (-1);
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The APPLIED_RX_TRANSDUCER_OFFSET parameter is the x, y, z position
     * offsets of the receiver array that have been applied
     */
    snprintf(temp, sizeof(temp), "APPLIED_RX_TRANSDUCER_OFFSET=");
    if (num_rx == 1)
    {
        if (p->applied.rx_transducer_x_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.rx_transducer_x_offset[0] > GSF_MIN_PARAM) && (p->applied.rx_transducer_x_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,", p->applied.rx_transducer_x_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->applied.rx_transducer_y_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.rx_transducer_y_offset[0] > GSF_MIN_PARAM) && (p->applied.rx_transducer_y_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,", p->applied.rx_transducer_y_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->applied.rx_transducer_z_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.rx_transducer_z_offset[0] > GSF_MIN_PARAM) && (p->applied.rx_transducer_z_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f", p->applied.rx_transducer_z_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    else if (num_rx == 2)
    {
        if (p->applied.rx_transducer_x_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.rx_transducer_x_offset[0] > GSF_MIN_PARAM) && (p->applied.rx_transducer_x_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,", p->applied.rx_transducer_x_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->applied.rx_transducer_y_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.rx_transducer_y_offset[0] > GSF_MIN_PARAM) && (p->applied.rx_transducer_y_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,", p->applied.rx_transducer_y_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->applied.rx_transducer_z_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.rx_transducer_z_offset[0] > GSF_MIN_PARAM) && (p->applied.rx_transducer_z_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,", p->applied.rx_transducer_z_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->applied.rx_transducer_x_offset[1] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.rx_transducer_x_offset[1] > GSF_MIN_PARAM) && (p->applied.rx_transducer_x_offset[1] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,", p->applied.rx_transducer_x_offset[1]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->applied.rx_transducer_y_offset[1] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.rx_transducer_y_offset[1] > GSF_MIN_PARAM) && (p->applied.rx_transducer_y_offset[1] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,", p->applied.rx_transducer_y_offset[1]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->applied.rx_transducer_z_offset[1] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.rx_transducer_z_offset[1] > GSF_MIN_PARAM) && (p->applied.rx_transducer_z_offset[1] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f", p->applied.rx_transducer_z_offset[1]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The APPLIED_RX_TRANSDUCER_PITCH_OFFSET parameter is the receiver pitch offset that has been applied. */
    snprintf(temp, sizeof(temp), "APPLIED_RX_TRANSDUCER_PITCH_OFFSET=");
    if (num_rx == 1)
    {
        if (p->applied.rx_transducer_pitch_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.rx_transducer_pitch_offset[0] > GSF_MIN_PARAM) && (p->applied.rx_transducer_pitch_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f", p->applied.rx_transducer_pitch_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    else if (num_rx == 2)
    {
        if (p->applied.rx_transducer_pitch_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.rx_transducer_pitch_offset[0] > GSF_MIN_PARAM) && (p->applied.rx_transducer_pitch_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,", p->applied.rx_transducer_pitch_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->applied.rx_transducer_pitch_offset[1] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.rx_transducer_pitch_offset[1] > GSF_MIN_PARAM) && (p->applied.rx_transducer_pitch_offset[1] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f", p->applied.rx_transducer_pitch_offset[1]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The APPLIED_RX_TRANSDUCER_ROLL_OFFSET parameter is the receiver roll offset that has been applied. */
    snprintf(temp, sizeof(temp), "APPLIED_RX_TRANSDUCER_ROLL_OFFSET=");
    if (num_rx == 1)
    {
        if (p->applied.rx_transducer_roll_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.rx_transducer_roll_offset[0] > GSF_MIN_PARAM) && (p->applied.rx_transducer_roll_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f", p->applied.rx_transducer_roll_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    else if (num_rx == 2)
    {
        if (p->applied.rx_transducer_roll_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.rx_transducer_roll_offset[0] > GSF_MIN_PARAM) && (p->applied.rx_transducer_roll_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,", p->applied.rx_transducer_roll_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->applied.rx_transducer_roll_offset[1] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.rx_transducer_roll_offset[1] > GSF_MIN_PARAM) && (p->applied.rx_transducer_roll_offset[1] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f", p->applied.rx_transducer_roll_offset[1]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The APPLIED_RX_TRANSDUCER_HEADING_OFFSET parameter is the receiver heading offset that has been applied. */
    snprintf(temp, sizeof(temp), "APPLIED_RX_TRANSDUCER_HEADING_OFFSET=");
    if (num_rx == 1)
    {
        if (p->applied.rx_transducer_heading_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.rx_transducer_heading_offset[0] > GSF_MIN_PARAM) && (p->applied.rx_transducer_heading_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f", p->applied.rx_transducer_heading_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    else if (num_rx == 2)
    {
        if (p->applied.rx_transducer_heading_offset[0] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s,", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.rx_transducer_heading_offset[0] > GSF_MIN_PARAM) && (p->applied.rx_transducer_heading_offset[0] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f,", p->applied.rx_transducer_heading_offset[0]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
        if (p->applied.rx_transducer_heading_offset[1] == GSF_UNKNOWN_PARAM_VALUE)
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%s", GSF_UNKNOWN_PARAM_TEXT);
        }
        else if ((p->applied.rx_transducer_heading_offset[1] > GSF_MIN_PARAM) && (p->applied.rx_transducer_heading_offset[1] < GSF_MAX_PARAM))
        {
            snprintf(temp + strlen(temp), sizeof(temp) - strlen(temp), "%+06.2f", p->applied.rx_transducer_heading_offset[1]);
        }
        else
        {
            gsfError = GSF_PROCESS_PARAM_RECORD_ENCODE_FAILED;
            return (-1);
        }
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /******* end of the applied parameters *******/

    /* The horizontal datum parameter defines the ellipsoid to which the
     * latitude longitude values are referenced.
     */
    switch (p->horizontal_datum)
    {
        case (GSF_H_DATUM_WGE):
            snprintf(temp, sizeof(temp), "GEOID=WGS-84");
            break;

        case (GSF_H_DATUM_NAR):
            snprintf(temp, sizeof(temp), "GEOID=NAD-83");
            break;
        default:
            snprintf(temp, sizeof(temp), "GEOID=UNKNWN");
            break;

    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    /* The TIDAL_DATUM paremeter defines the reference datum for tide
     * corrections. See gsf.h for definitions.
     */
    switch (p->vertical_datum)
    {
        case (GSF_V_DATUM_MLLW):
            snprintf(temp, sizeof(temp), "TIDAL_DATUM=MLLW   ");
            break;

        case (GSF_V_DATUM_MLW):
            snprintf(temp, sizeof(temp), "TIDAL_DATUM=MLW    ");
            break;

        case (GSF_V_DATUM_ALAT):
             snprintf(temp, sizeof(temp), "TIDAL_DATUM=ALAT  ");
             break;

        case (GSF_V_DATUM_ESLW):
             snprintf(temp, sizeof(temp), "TIDAL_DATUM=ESLW  ");
             break;

        case (GSF_V_DATUM_ISLW):
             snprintf(temp, sizeof(temp), "TIDAL_DATUM=ISLW  ");
             break;

        case (GSF_V_DATUM_LAT):
             snprintf(temp, sizeof(temp), "TIDAL_DATUM=LAT   ");
             break;

        case (GSF_V_DATUM_LLW):
             snprintf(temp, sizeof(temp), "TIDAL_DATUM=LLW   ");
             break;

        case (GSF_V_DATUM_LNLW):
             snprintf(temp, sizeof(temp), "TIDAL_DATUM=LNLW  ");
             break;

        case (GSF_V_DATUM_LWD):
             snprintf(temp, sizeof(temp), "TIDAL_DATUM=LWD   ");
             break;

        case (GSF_V_DATUM_MLHW):
             snprintf(temp, sizeof(temp), "TIDAL_DATUM=MLHW  ");
             break;

        case (GSF_V_DATUM_MLLWS):
             snprintf(temp, sizeof(temp), "TIDAL_DATUM=MLLWS ");
             break;

        case (GSF_V_DATUM_MLWN):
             snprintf(temp, sizeof(temp), "TIDAL_DATUM=MLWN  ");
             break;

        case (GSF_V_DATUM_MSL):
             snprintf(temp, sizeof(temp), "TIDAL_DATUM=MSL   ");
             break;
        
        case (GSF_V_DATUM_ALLW):
             snprintf(temp, sizeof(temp), "TIDAL_DATUM=ALLW  ");
             break;
             
        case (GSF_V_DATUM_LNT):
             snprintf(temp, sizeof(temp), "TIDAL_DATUM=LNT   ");
             break;

        case (GSF_V_DATUM_AMLWS):
             snprintf(temp, sizeof(temp), "TIDAL_DATUM=AMLWS ");
             break;

        case (GSF_V_DATUM_AMLLW):
             snprintf(temp, sizeof(temp), "TIDAL_DATUM=AMLLW ");
             break;

        case (GSF_V_DATUM_MLWS):
             snprintf(temp, sizeof(temp), "TIDAL_DATUM=MLWS ");
             break;

        case (GSF_V_DATUM_AMSL):
             snprintf(temp, sizeof(temp), "TIDAL_DATUM=AMSL ");
             break;

        case (GSF_V_DATUM_AMLW):
             snprintf(temp, sizeof(temp), "TIDAL_DATUM=AMLW ");
             break;

        case (GSF_V_DATUM_AISLW):
             snprintf(temp, sizeof(temp), "TIDAL_DATUM=AISLW");
             break;

        case (GSF_V_DATUM_ALLWS):
             snprintf(temp, sizeof(temp), "TIDAL_DATUM=ALLWS");
             break;

        default:
            snprintf(temp, sizeof(temp), "TIDAL_DATUM=UNKNOWN");
            break;
    }
    ret = gsfSetParam(handle, number_parameters++, temp, rec);
    if (ret)
    {
        return(-1);
    }

    rec->process_parameters.number_parameters = number_parameters;

    return(0);
}

/********************************************************************
 *
 * Function Name : gsfGetMBParams
 *
 * Description : This function moves swath bathymetry sonar processing
 *    parameters from external, form to internal form.  The external
 *    "KEYWORD=VALUE" format parameters are read from a processing_params
 *    structure of a gsfRecords data structure maintained by the caller.
 *    The internal form parameters are written into a gsfMBParams data
 *    structure maintained by the caller. Parameters for up to two pairs of
 *    transmit/receive arrays are supported, for systems such as Reson SeaBat
 *    9002.  Any parameter not described in a "KEYWORD=VALUE" format will
 *    be set to "GSF_UNKNOWN_PARAM_VALUE" for float types or
 *   "GSF_UNKNOWN_PARAM_INT" for integer types.
 *
 * Inputs :
 *     rec = a pointer to the gsfRecords data structure from which the
 *         parameters in "KEYWORD=VALUE" form are to be read.
 *     p = a pointer to the gsfMBParams data structure which will be populated.
 *     numArrays = the integer value specifying the number of pairs of
 *         arrays which need to have separate parameters tracked.
 *
 * Returns : This function returns zero if successful, or -1 if an error
 *  occurs.
 *
 * Error Conditions :
 *  none.
 *
 ********************************************************************/
int
gsfGetMBParams(const gsfRecords *rec, gsfMBParams *p, int *numArrays)
{
    int i;
    char str[64];
    int num_tx = 0, num_rx = 0;

    gsfInitializeMBParams (p);   /* set everything to "unknown" */
    /* Set this value to zero in case we can't determine it */
    *numArrays = 0;

    for (i=0; i<rec->process_parameters.number_parameters; i++)
    {
        if (strncmp(rec->process_parameters.param[i], "REFERENCE TIME", strlen("REFERENCE TIME")) == 0)
        {
            memset(p->start_of_epoch, 0, sizeof(p->start_of_epoch));
            strncpy(p->start_of_epoch, rec->process_parameters.param[i], sizeof(p->start_of_epoch));
        }

        /* DHG 2008/12/18 Add "PLATFORM_TYPE" */

        else if (strncmp (rec->process_parameters.param[i], "PLATFORM_TYPE", strlen ("PLATFORM_TYPE")) == 0)
        {
            if (strstr(rec->process_parameters.param[i], "AUV"))
            {
                p->vessel_type = GSF_PLATFORM_TYPE_AUV;
            }
            else if (strstr(rec->process_parameters.param[i], "ROTV"))
            {
                p->vessel_type = GSF_PLATFORM_TYPE_ROTV;
            }
            else // default to surface ship
            {
                p->vessel_type = GSF_PLATFORM_TYPE_SURFACE_SHIP;
            }
        }
        else if (strncmp (rec->process_parameters.param[i], "FULL_RAW_DATA", strlen ("FULL_RAW_DATA")) == 0)
        {
            if (strstr(rec->process_parameters.param[i], "TRUE"))
            {
                p->full_raw_data = GSF_TRUE;
            }
            else
            {
                p->full_raw_data = GSF_FALSE;
            }
        }
        else if (strncmp(rec->process_parameters.param[i], "ROLL_COMPENSATED", strlen("ROLL_COMPENSATED")) == 0)
        {
            if (strstr(rec->process_parameters.param[i], "YES"))
            {
                p->roll_compensated = GSF_COMPENSATED;
            }
            else
            {
                p->roll_compensated = GSF_UNCOMPENSATED;
            }
        }
        else if (strncmp(rec->process_parameters.param[i], "PITCH_COMPENSATED", strlen("PITCH_COMPENSATED")) == 0)
        {
            if (strstr(rec->process_parameters.param[i], "YES"))
            {
                p->pitch_compensated = GSF_COMPENSATED;
            }
            else
            {
                p->pitch_compensated = GSF_UNCOMPENSATED;
            }
        }
        else if (strncmp(rec->process_parameters.param[i], "HEAVE_COMPENSATED", strlen("HEAVE_COMPENSATED")) == 0)
        {
            if (strstr(rec->process_parameters.param[i], "YES"))
            {
                p->heave_compensated = GSF_COMPENSATED;
            }
            else
            {
                p->heave_compensated = GSF_UNCOMPENSATED;
            }
        }
        else if (strncmp(rec->process_parameters.param[i], "TIDE_COMPENSATED", strlen("TIDE_COMPENSATED")) == 0)
        {
            if (strstr(rec->process_parameters.param[i], "YES"))
            {
                p->tide_compensated = GSF_COMPENSATED;
            }
            else
            {
                p->tide_compensated = GSF_UNCOMPENSATED;
            }
        }
        else if (strncmp(rec->process_parameters.param[i], "NUMBER_OF_TRANSMITTERS", strlen("NUMBER_OF_TRANSMITTERS")) == 0)
        {
            sscanf (rec->process_parameters.param[i], "NUMBER_OF_TRANSMITTERS=%3s", str);
            if ((atoi(str) >= 1) && (atoi(str) <= GSF_MAX_OFFSETS))
            {
                p->number_of_transmitters = atoi(str);
                num_tx = p->number_of_transmitters;
            }
            else
            {
                p->number_of_transmitters = GSF_UNKNOWN_PARAM_INT;
            }
        }
        else if (strncmp(rec->process_parameters.param[i], "NUMBER_OF_RECEIVERS", strlen("NUMBER_OF_RECEIVERS")) == 0)
        {
            sscanf (rec->process_parameters.param[i], "NUMBER_OF_RECEIVERS=%3s", str);
            if ((atoi(str) >= 1) && (atoi(str) <= GSF_MAX_OFFSETS))
            {
                p->number_of_receivers = atoi(str);
                num_rx = p->number_of_receivers;
            }
            else
            {
                p->number_of_receivers = GSF_UNKNOWN_PARAM_INT;
            }
        }
        else if (strncmp(rec->process_parameters.param[i], "DEPTH_CALCULATION", strlen("DEPTH_CALCULATION")) == 0)
        {
            sscanf (rec->process_parameters.param[i], "DEPTH_CALCULATION=%32s", str);
            if (strcmp(str, "CORRECTED") == 0)
            {
                p->depth_calculation = GSF_TRUE_DEPTHS;
            }
            else if (strcmp(str, "CALCULATED_RE_1500_MS") == 0)
            {
                p->depth_calculation = GSF_DEPTHS_RE_1500_MS;
            }
            else
            {
                p->depth_calculation = GSF_DEPTH_CALC_UNKNOWN;
            }
        }

        /* This parameter indicates whether the angle travel time
         * pairs have been corrected for ray tracing.
         */
        else if (strncmp(rec->process_parameters.param[i], "RAY_TRACING", strlen("RAY_TRACING")) == 0)
        {
            sscanf (rec->process_parameters.param[i], "RAY_TRACING=%5s", str);
            if (strcmp(str, "YES") == 0)
            {
                p->ray_tracing = GSF_COMPENSATED;
            }
            else
            {
                p->ray_tracing = GSF_UNCOMPENSATED;
            }
        }
        else if (strncmp(rec->process_parameters.param[i], "MSB_APPLIED_TO_ATTITUDE", strlen("MSB_APPLIED_TO_ATTITUDE")) == 0)
        {
            sscanf (rec->process_parameters.param[i], "MSB_APPLIED_TO_ATTITUDE=%5s", str);
            if (strcmp(str, "YES") == 0)
            {
                p->msb_applied_to_attitude = GSF_TRUE;
            }
            else
            {
                p->msb_applied_to_attitude = GSF_FALSE;
            }
        }
        else if (strncmp(rec->process_parameters.param[i], "HEAVE_REMOVED_FROM_GPS_TC", strlen("HEAVE_REMOVED_FROM_GPS_TC")) == 0)
        {
            sscanf (rec->process_parameters.param[i], "HEAVE_REMOVED_FROM_GPS_TC=%5s", str);
            if (strcmp(str, "YES") == 0)
            {
                p->heave_removed_from_gps_tc = GSF_TRUE;
            }
            else
            {
                p->heave_removed_from_gps_tc = GSF_FALSE;
            }
        }
        else if (strncmp(rec->process_parameters.param[i], "UTC_OFFSET", strlen("UTC_OFFSET")) == 0)
        {
            sscanf (rec->process_parameters.param[i], "UTC_OFFSET=%3s", str);
            if ((abs(atoi(str)) >= 0) && (abs(atoi(str)) <= 12))
            {
                p->utc_offset = atoi(str);
            }
            else
            {
                p->utc_offset = GSF_UNKNOWN_PARAM_INT;
            }
        }
        else if (strncmp(rec->process_parameters.param[i], "ROLL_REFERENCE", strlen("ROLL_REFERENCE")) == 0)
        {
            sscanf (rec->process_parameters.param[i], "ROLL_REFERENCE=%32s", str);
            if (strcmp(str, "HORIZONTAL_PITCH_AXIS") == 0)
            {
                p->roll_reference = GSF_HORIZONTAL_PITCH_AXIS;
            }
            else if (strcmp(str, "ROTATED_PITCH_AXIS") == 0)
            {
                p->roll_reference = GSF_ROTATED_PITCH_AXIS;
            }
            else
            {
                p->roll_reference = GSF_UNKNOWN_PARAM_INT;
            }
        }
        else if (strncmp(rec->process_parameters.param[i], "DRAFT_TO_APPLY", strlen("DRAFT_TO_APPLY")) == 0)
        {
            p->to_apply.draft[0] = GSF_UNKNOWN_PARAM_VALUE;
            p->to_apply.draft[1] = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "DRAFT_TO_APPLY=%lf,%lf",
                    &p->to_apply.draft[0],
                    &p->to_apply.draft[1]);
            }
            /* Get the number of array pairs from each sonar alignment parameter */
            *numArrays = gsfNumberParams(rec->process_parameters.param[i]);
            if (!num_tx)
                num_tx = *numArrays;
        }
        else if (strncmp(rec->process_parameters.param[i], "PITCH_TO_APPLY", strlen("PITCH_TO_APPLY")) == 0)
        {
            p->to_apply.pitch_bias[0] = GSF_UNKNOWN_PARAM_VALUE;
            p->to_apply.pitch_bias[1] = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "PITCH_TO_APPLY=%lf,%lf",
                    &p->to_apply.pitch_bias[0],
                    &p->to_apply.pitch_bias[1]);
            }
            /* Get the number of array pairs from each sonar alignment parameter */
            *numArrays = gsfNumberParams(rec->process_parameters.param[i]);
            if (!num_tx)
                num_tx = *numArrays;
        }
        else if (strncmp(rec->process_parameters.param[i], "ROLL_TO_APPLY", strlen("ROLL_TO_APPLY")) == 0)
        {
            p->to_apply.roll_bias[0] = GSF_UNKNOWN_PARAM_VALUE;
            p->to_apply.roll_bias[1] = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "ROLL_TO_APPLY=%lf,%lf",
                    &p->to_apply.roll_bias[0],
                    &p->to_apply.roll_bias[1]);
            }
            /* Get the number of array pairs from each sonar alignment parameter */
            *numArrays = gsfNumberParams(rec->process_parameters.param[i]);
            if (!num_tx)
                num_tx = *numArrays;
        }
        else if (strncmp(rec->process_parameters.param[i], "GYRO_TO_APPLY", strlen("GYRO_TO_APPLY")) == 0)
        {
            p->to_apply.gyro_bias[0] = GSF_UNKNOWN_PARAM_VALUE;
            p->to_apply.gyro_bias[1] = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "GYRO_TO_APPLY=%lf,%lf",
                    &p->to_apply.gyro_bias[0],
                    &p->to_apply.gyro_bias[1]);
            }
            /* Get the number of array pairs from each sonar alignment parameter */
            *numArrays = gsfNumberParams(rec->process_parameters.param[i]);
            if (!num_tx)
                num_tx = *numArrays;
        }
        /* The POSITION_OFFSET_TO_APPLY parameter is place holder for a known,
         * but presently uncorrected position reference offset. The string
         * contains an x,y,z triplet which defines the location on the
         * vessel to which the latitude and longitude are relative.
         */
        else if (strncmp(rec->process_parameters.param[i], "POSITION_OFFSET_TO_APPLY", strlen("POSITION_OFFSET_TO_APPLY")) == 0)
        {
            p->to_apply.position_x_offset = GSF_UNKNOWN_PARAM_VALUE;
            p->to_apply.position_y_offset = GSF_UNKNOWN_PARAM_VALUE;
            p->to_apply.position_z_offset = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "POSITION_OFFSET_TO_APPLY=%lf,%lf,%lf",
                    &p->to_apply.position_x_offset,
                    &p->to_apply.position_y_offset,
                    &p->to_apply.position_z_offset);
            }
        }
        else if (strncmp(rec->process_parameters.param[i], "TRANSDUCER_OFFSET_TO_APPLY", strlen("TRANSDUCER_OFFSET_TO_APPLY")) == 0)
        {
            p->to_apply.transducer_x_offset[0] = GSF_UNKNOWN_PARAM_VALUE;
            p->to_apply.transducer_y_offset[0] = GSF_UNKNOWN_PARAM_VALUE;
            p->to_apply.transducer_z_offset[0] = GSF_UNKNOWN_PARAM_VALUE;
            p->to_apply.transducer_x_offset[1] = GSF_UNKNOWN_PARAM_VALUE;
            p->to_apply.transducer_y_offset[1] = GSF_UNKNOWN_PARAM_VALUE;
            p->to_apply.transducer_z_offset[1] = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "TRANSDUCER_OFFSET_TO_APPLY=%lf,%lf,%lf,%lf,%lf,%lf",
                    &p->to_apply.transducer_x_offset[0],
                    &p->to_apply.transducer_y_offset[0],
                    &p->to_apply.transducer_z_offset[0],
                    &p->to_apply.transducer_x_offset[1],
                    &p->to_apply.transducer_y_offset[1],
                    &p->to_apply.transducer_z_offset[1]);
            }
        }
        else if (strncmp(rec->process_parameters.param[i], "TRANSDUCER_PITCH_OFFSET_TO_APPLY", strlen("TRANSDUCER_PITCH_OFFSET_TO_APPLY")) == 0)
        {
            p->to_apply.transducer_pitch_offset[0] = GSF_UNKNOWN_PARAM_VALUE;
            p->to_apply.transducer_pitch_offset[1] = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "TRANSDUCER_PITCH_OFFSET_TO_APPLY=%lf,%lf",
                    &p->to_apply.transducer_pitch_offset[0],
                    &p->to_apply.transducer_pitch_offset[1]);
            }
            /* Get the number of array pairs from each sonar alignment parameter */
            *numArrays = gsfNumberParams(rec->process_parameters.param[i]);
            if (!num_tx)
                num_tx = *numArrays;
        }
        else if (strncmp(rec->process_parameters.param[i], "TRANSDUCER_ROLL_OFFSET_TO_APPLY", strlen("TRANSDUCER_ROLL_OFFSET_TO_APPLY")) == 0)
        {
            p->to_apply.transducer_roll_offset[0] = GSF_UNKNOWN_PARAM_VALUE;
            p->to_apply.transducer_roll_offset[1] = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "TRANSDUCER_ROLL_OFFSET_TO_APPLY=%lf,%lf",
                    &p->to_apply.transducer_roll_offset[0],
                    &p->to_apply.transducer_roll_offset[1]);
            }
            /* Get the number of array pairs from each sonar alignment parameter */
            *numArrays = gsfNumberParams(rec->process_parameters.param[i]);
            if (!num_tx)
                num_tx = *numArrays;
        }
        else if (strncmp(rec->process_parameters.param[i], "TRANSDUCER_HEADING_OFFSET_TO_APPLY", strlen("TRANSDUCER_HEADING_OFFSET_TO_APPLY")) == 0)
        {
            p->to_apply.transducer_heading_offset[0] = GSF_UNKNOWN_PARAM_VALUE;
            p->to_apply.transducer_heading_offset[1] = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "TRANSDUCER_HEADING_OFFSET_TO_APPLY=%lf,%lf",
                    &p->to_apply.transducer_heading_offset[0],
                    &p->to_apply.transducer_heading_offset[1]);
            }
            /* Get the number of array pairs from each sonar alignment parameter */
            *numArrays = gsfNumberParams(rec->process_parameters.param[i]);
            if (!num_tx)
                num_tx = *numArrays;
        }
        else if (strncmp(rec->process_parameters.param[i], "MRU_PITCH_TO_APPLY", strlen("MRU_PITCH_TO_APPLY")) == 0)
        {
            p->to_apply.mru_pitch_bias = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "MRU_PITCH_TO_APPLY=%lf",
                    &p->to_apply.mru_pitch_bias);
            }
        }
        else if (strncmp(rec->process_parameters.param[i], "MRU_ROLL_TO_APPLY", strlen("MRU_ROLL_TO_APPLY")) == 0)
        {
            p->to_apply.mru_roll_bias = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "MRU_ROLL_TO_APPLY=%lf",
                    &p->to_apply.mru_roll_bias);
            }
        }
        else if (strncmp(rec->process_parameters.param[i], "MRU_HEADING_TO_APPLY", strlen("MRU_HEADING_TO_APPLY")) == 0)
        {
            p->to_apply.mru_heading_bias = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "MRU_HEADING_TO_APPLY=%lf",
                    &p->to_apply.mru_heading_bias);
            }
        }
        else if (strncmp(rec->process_parameters.param[i], "MRU_OFFSET_TO_APPLY", strlen("MRU_OFFSET_TO_APPLY")) == 0)
        {
            p->to_apply.mru_x_offset = GSF_UNKNOWN_PARAM_VALUE;
            p->to_apply.mru_y_offset = GSF_UNKNOWN_PARAM_VALUE;
            p->to_apply.mru_z_offset = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "MRU_OFFSET_TO_APPLY=%lf,%lf,%lf",
                    &p->to_apply.mru_x_offset,
                    &p->to_apply.mru_y_offset,
                    &p->to_apply.mru_z_offset);
            }
        }
        else if (strncmp(rec->process_parameters.param[i], "ANTENNA_OFFSET_TO_APPLY", strlen("ANTENNA_OFFSET_TO_APPLY")) == 0)
        {
            p->to_apply.antenna_x_offset = GSF_UNKNOWN_PARAM_VALUE;
            p->to_apply.antenna_y_offset = GSF_UNKNOWN_PARAM_VALUE;
            p->to_apply.antenna_z_offset = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "ANTENNA_OFFSET_TO_APPLY=%lf,%lf,%lf",
                    &p->to_apply.antenna_x_offset,
                    &p->to_apply.antenna_y_offset,
                    &p->to_apply.antenna_z_offset);
            }
        }
        else if (strncmp(rec->process_parameters.param[i], "CENTER_OF_ROTATION_OFFSET_TO_APPLY", strlen("CENTER_OF_ROTATION_OFFSET_TO_APPLY")) == 0)
        {
            p->to_apply.center_of_rotation_x_offset = GSF_UNKNOWN_PARAM_VALUE;
            p->to_apply.center_of_rotation_y_offset = GSF_UNKNOWN_PARAM_VALUE;
            p->to_apply.center_of_rotation_z_offset = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "CENTER_OF_ROTATION_OFFSET_TO_APPLY=%lf,%lf,%lf",
                    &p->to_apply.center_of_rotation_x_offset,
                    &p->to_apply.center_of_rotation_y_offset,
                    &p->to_apply.center_of_rotation_z_offset);
            }
        }
        else if (strncmp(rec->process_parameters.param[i], "POSITION_LATENCY_TO_APPLY", strlen("POSITION_LATENCY_TO_APPLY")) == 0)
        {
            p->to_apply.position_latency = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "POSITION_LATENCY_TO_APPLY=%lf",
                    &p->to_apply.position_latency);
            }
        }
        else if (strncmp(rec->process_parameters.param[i], "ATTITUDE_LATENCY_TO_APPLY", strlen("ATTITUDE_LATENCY_TO_APPLY")) == 0)
        {
            p->to_apply.attitude_latency = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "ATTITUDE_LATENCY_TO_APPLY=%lf",
                    &p->to_apply.attitude_latency);
            }
        }
        else if (strncmp(rec->process_parameters.param[i], "DEPTH_SENSOR_LATENCY_TO_APPLY", strlen("DEPTH_SENSOR_LATENCY_TO_APPLY")) == 0)
        {
            p->to_apply.depth_sensor_latency = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "DEPTH_SENSOR_LATENCY_TO_APPLY=%lf",
                    &p->to_apply.depth_sensor_latency);
            }
        }
        else if (strncmp(rec->process_parameters.param[i], "DEPTH_SENSOR_OFFSET_TO_APPLY", strlen("DEPTH_SENSOR_OFFSET_TO_APPLY")) == 0)
        {
            p->to_apply.depth_sensor_x_offset = GSF_UNKNOWN_PARAM_VALUE;
            p->to_apply.depth_sensor_y_offset = GSF_UNKNOWN_PARAM_VALUE;
            p->to_apply.depth_sensor_z_offset = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "DEPTH_SENSOR_OFFSET_TO_APPLY=%lf,%lf,%lf",
                    &p->to_apply.depth_sensor_x_offset,
                    &p->to_apply.depth_sensor_y_offset,
                    &p->to_apply.depth_sensor_z_offset);
            }
        }
        else if (strncmp(rec->process_parameters.param[i], "RX_TRANSDUCER_OFFSET_TO_APPLY", strlen("RX_TRANSDUCER_OFFSET_TO_APPLY")) == 0)
        {
            p->to_apply.rx_transducer_x_offset[0] = GSF_UNKNOWN_PARAM_VALUE;
            p->to_apply.rx_transducer_y_offset[0] = GSF_UNKNOWN_PARAM_VALUE;
            p->to_apply.rx_transducer_z_offset[0] = GSF_UNKNOWN_PARAM_VALUE;
            p->to_apply.rx_transducer_x_offset[1] = GSF_UNKNOWN_PARAM_VALUE;
            p->to_apply.rx_transducer_y_offset[1] = GSF_UNKNOWN_PARAM_VALUE;
            p->to_apply.rx_transducer_z_offset[1] = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "RX_TRANSDUCER_OFFSET_TO_APPLY=%lf,%lf,%lf,%lf,%lf,%lf",
                    &p->to_apply.rx_transducer_x_offset[0],
                    &p->to_apply.rx_transducer_y_offset[0],
                    &p->to_apply.rx_transducer_z_offset[0],
                    &p->to_apply.rx_transducer_x_offset[1],
                    &p->to_apply.rx_transducer_y_offset[1],
                    &p->to_apply.rx_transducer_z_offset[1]);
            }
        }
        else if (strncmp(rec->process_parameters.param[i], "RX_TRANSDUCER_PITCH_OFFSET_TO_APPLY", strlen("RX_TRANSDUCER_PITCH_OFFSET_TO_APPLY")) == 0)
        {
            p->to_apply.rx_transducer_pitch_offset[0] = GSF_UNKNOWN_PARAM_VALUE;
            p->to_apply.rx_transducer_pitch_offset[1] = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "RX_TRANSDUCER_PITCH_OFFSET_TO_APPLY=%lf,%lf",
                    &p->to_apply.rx_transducer_pitch_offset[0],
                    &p->to_apply.rx_transducer_pitch_offset[1]);
            }
            if (!num_rx)
                num_rx = gsfNumberParams(rec->process_parameters.param[i]);
        }
        else if (strncmp(rec->process_parameters.param[i], "RX_TRANSDUCER_ROLL_OFFSET_TO_APPLY", strlen("RX_TRANSDUCER_ROLL_OFFSET_TO_APPLY")) == 0)
        {
            p->to_apply.rx_transducer_roll_offset[0] = GSF_UNKNOWN_PARAM_VALUE;
            p->to_apply.rx_transducer_roll_offset[1] = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "RX_TRANSDUCER_ROLL_OFFSET_TO_APPLY=%lf,%lf",
                    &p->to_apply.rx_transducer_roll_offset[0],
                    &p->to_apply.rx_transducer_roll_offset[1]);
            }
            if (!num_rx)
                num_rx = gsfNumberParams(rec->process_parameters.param[i]);
        }
        else if (strncmp(rec->process_parameters.param[i], "RX_TRANSDUCER_HEADING_OFFSET_TO_APPLY", strlen("RX_TRANSDUCER_HEADING_OFFSET_TO_APPLY")) == 0)
        {
            p->to_apply.rx_transducer_heading_offset[0] = GSF_UNKNOWN_PARAM_VALUE;
            p->to_apply.rx_transducer_heading_offset[1] = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "RX_TRANSDUCER_HEADING_OFFSET_TO_APPLY=%lf,%lf",
                    &p->to_apply.rx_transducer_heading_offset[0],
                    &p->to_apply.rx_transducer_heading_offset[1]);
            }
            if (!num_rx)
                num_rx = gsfNumberParams(rec->process_parameters.param[i]);
        }  /** end of "to apply" values */
        else if (strncmp(rec->process_parameters.param[i], "APPLIED_DRAFT", strlen("APPLIED_DRAFT")) == 0)
        {
            p->applied.draft[0] = GSF_UNKNOWN_PARAM_VALUE;
            p->applied.draft[1] = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "APPLIED_DRAFT=%lf,%lf",
                    &p->applied.draft[0],
                    &p->applied.draft[1]);
            }
            /* Get the number of array pairs from each sonar alignment parameter */
            *numArrays = gsfNumberParams(rec->process_parameters.param[i]);
            if (!num_tx)
                num_tx = *numArrays;
        }
        else if (strncmp(rec->process_parameters.param[i], "APPLIED_PITCH_BIAS", strlen("APPLIED_PITCH_BIAS")) == 0)
        {
            p->applied.pitch_bias[0] = GSF_UNKNOWN_PARAM_VALUE;
            p->applied.pitch_bias[1] = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "APPLIED_PITCH_BIAS=%lf,%lf",
                    &p->applied.pitch_bias[0],
                    &p->applied.pitch_bias[1]);
            }
            /* Get the number of array pairs from each sonar alignment parameter */
            *numArrays = gsfNumberParams(rec->process_parameters.param[i]);
            if (!num_tx)
                num_tx = *numArrays;
        }
        else if (strncmp(rec->process_parameters.param[i], "APPLIED_ROLL_BIAS", strlen("APPLIED_ROLL_BIAS")) == 0)
        {
            p->applied.roll_bias[0] = GSF_UNKNOWN_PARAM_VALUE;
            p->applied.roll_bias[1] = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "APPLIED_ROLL_BIAS=%lf,%lf",
                    &p->applied.roll_bias[0],
                    &p->applied.roll_bias[1]);
            }
            /* Get the number of array pairs from each sonar alignment parameter */
            *numArrays = gsfNumberParams(rec->process_parameters.param[i]);
            if (!num_tx)
                num_tx = *numArrays;
        }
        else if (strncmp(rec->process_parameters.param[i], "APPLIED_GYRO_BIAS", strlen("APPLIED_GYRO_BIAS")) == 0)
        {
            p->applied.gyro_bias[0] = GSF_UNKNOWN_PARAM_VALUE;
            p->applied.gyro_bias[1] = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "APPLIED_GYRO_BIAS=%lf,%lf",
                    &p->applied.gyro_bias[0],
                    &p->applied.gyro_bias[1]);
            }
            /* Get the number of array pairs from each sonar alignment parameter */
            *numArrays = gsfNumberParams(rec->process_parameters.param[i]);
            if (!num_tx)
                num_tx = *numArrays;
        }
        /* The APPLIED_POSITION_OFFSET parameter defines the x,y,z position in
         * ship coordinates to which the lat lons are relative.
         */
         else if (strncmp(rec->process_parameters.param[i], "APPLIED_POSITION_OFFSET", strlen("APPLIED_POSITION_OFFSET")) == 0)
         {
            p->applied.position_x_offset = GSF_UNKNOWN_PARAM_VALUE;
            p->applied.position_y_offset = GSF_UNKNOWN_PARAM_VALUE;
            p->applied.position_z_offset = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "APPLIED_POSITION_OFFSET=%lf,%lf,%lf",
                    &p->applied.position_x_offset,
                    &p->applied.position_y_offset,
                    &p->applied.position_z_offset);
            }
         }
        else if (strncmp(rec->process_parameters.param[i], "APPLIED_ANTENNA_OFFSET", strlen("APPLIED_ANTENNA_OFFSET")) == 0)
        {
           p->applied.antenna_x_offset = GSF_UNKNOWN_PARAM_VALUE;
           p->applied.antenna_y_offset = GSF_UNKNOWN_PARAM_VALUE;
           p->applied.antenna_z_offset = GSF_UNKNOWN_PARAM_VALUE;
           if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
           {
               sscanf (rec->process_parameters.param[i], "APPLIED_ANTENNA_OFFSET=%lf,%lf,%lf",
                   &p->applied.antenna_x_offset,
                   &p->applied.antenna_y_offset,
                   &p->applied.antenna_z_offset);
           }
        }
        /* The APPLIED_TRANSDUCER_OFFSET parameter defines the x,y,z offsets
         * in ship coordinates to which have been applied to refer the x,y,z
         * beam values to the ship reference point.
         */
        else if (strncmp(rec->process_parameters.param[i], "APPLIED_TRANSDUCER_OFFSET", strlen("APPLIED_TRANSDUCER_OFFSET")) == 0)
        {
            p->applied.transducer_x_offset[0] = GSF_UNKNOWN_PARAM_VALUE;
            p->applied.transducer_y_offset[0] = GSF_UNKNOWN_PARAM_VALUE;
            p->applied.transducer_z_offset[0] = GSF_UNKNOWN_PARAM_VALUE;
            p->applied.transducer_x_offset[1] = GSF_UNKNOWN_PARAM_VALUE;
            p->applied.transducer_y_offset[1] = GSF_UNKNOWN_PARAM_VALUE;
            p->applied.transducer_z_offset[1] = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "APPLIED_TRANSDUCER_OFFSET=%lf,%lf,%lf,%lf,%lf,%lf",
                    &p->applied.transducer_x_offset[0],
                    &p->applied.transducer_y_offset[0],
                    &p->applied.transducer_z_offset[0],
                    &p->applied.transducer_x_offset[1],
                    &p->applied.transducer_y_offset[1],
                    &p->applied.transducer_z_offset[1]);
            }
        }
        else if (strncmp(rec->process_parameters.param[i], "APPLIED_TRANSDUCER_PITCH_OFFSET", strlen("APPLIED_TRANSDUCER_PITCH_OFFSET")) == 0)
        {
            p->applied.transducer_pitch_offset[0] = GSF_UNKNOWN_PARAM_VALUE;
            p->applied.transducer_pitch_offset[1] = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "APPLIED_TRANSDUCER_PITCH_OFFSET=%lf,%lf",
                    &p->applied.transducer_pitch_offset[0],
                    &p->applied.transducer_pitch_offset[1]);
            }
            /* Get the number of array pairs from each sonar alignment parameter */
            *numArrays = gsfNumberParams(rec->process_parameters.param[i]);
            if (!num_tx)
                num_tx = *numArrays;
        }
        else if (strncmp(rec->process_parameters.param[i], "APPLIED_TRANSDUCER_ROLL_OFFSET", strlen("APPLIED_TRANSDUCER_ROLL_OFFSET")) == 0)
        {
            p->applied.transducer_roll_offset[0] = GSF_UNKNOWN_PARAM_VALUE;
            p->applied.transducer_roll_offset[1] = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "APPLIED_TRANSDUCER_ROLL_OFFSET=%lf,%lf",
                    &p->applied.transducer_roll_offset[0],
                    &p->applied.transducer_roll_offset[1]);
            }
            /* Get the number of array pairs from each sonar alignment parameter */
            *numArrays = gsfNumberParams(rec->process_parameters.param[i]);
            if (!num_tx)
                num_tx = *numArrays;
        }
        else if (strncmp(rec->process_parameters.param[i], "APPLIED_TRANSDUCER_HEADING_OFFSET", strlen("APPLIED_TRANSDUCER_HEADING_OFFSET")) == 0)
        {
            p->applied.transducer_heading_offset[0] = GSF_UNKNOWN_PARAM_VALUE;
            p->applied.transducer_heading_offset[1] = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "APPLIED_TRANSDUCER_HEADING_OFFSET=%lf,%lf",
                    &p->applied.transducer_heading_offset[0],
                    &p->applied.transducer_heading_offset[1]);
            }
            /* Get the number of array pairs from each sonar alignment parameter */
            *numArrays = gsfNumberParams(rec->process_parameters.param[i]);
            if (!num_tx)
                num_tx = *numArrays;
        }
        else if (strncmp(rec->process_parameters.param[i], "APPLIED_MRU_PITCH", strlen("APPLIED_MRU_PITCH")) == 0)
        {
            p->applied.mru_pitch_bias = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "APPLIED_MRU_PITCH=%lf",
                    &p->applied.mru_pitch_bias);
            }
        }
        else if (strncmp(rec->process_parameters.param[i], "APPLIED_MRU_ROLL", strlen("APPLIED_MRU_ROLL")) == 0)
        {
            p->applied.mru_roll_bias = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "APPLIED_MRU_ROLL=%lf",
                    &p->applied.mru_roll_bias);
            }
        }
        else if (strncmp(rec->process_parameters.param[i], "APPLIED_MRU_HEADING", strlen("APPLIED_MRU_HEADING")) == 0)
        {
            p->applied.mru_heading_bias = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "APPLIED_MRU_HEADING=%lf",
                    &p->applied.mru_heading_bias);
            }
        }
        else if (strncmp(rec->process_parameters.param[i], "APPLIED_MRU_OFFSET", strlen("APPLIED_MRU_OFFSET")) == 0)
        {
            p->applied.mru_x_offset = GSF_UNKNOWN_PARAM_VALUE;
            p->applied.mru_y_offset = GSF_UNKNOWN_PARAM_VALUE;
            p->applied.mru_z_offset = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "APPLIED_MRU_OFFSET=%lf,%lf,%lf",
                    &p->applied.mru_x_offset,
                    &p->applied.mru_y_offset,
                    &p->applied.mru_z_offset);
            }
        }
        else if (strncmp(rec->process_parameters.param[i], "APPLIED_CENTER_OF_ROTATION_OFFSET", strlen("APPLIED_CENTER_OF_ROTATION_OFFSET")) == 0)
        {
            p->applied.center_of_rotation_x_offset = GSF_UNKNOWN_PARAM_VALUE;
            p->applied.center_of_rotation_y_offset = GSF_UNKNOWN_PARAM_VALUE;
            p->applied.center_of_rotation_z_offset = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "APPLIED_CENTER_OF_ROTATION_OFFSET=%lf,%lf,%lf",
                    &p->applied.center_of_rotation_x_offset,
                    &p->applied.center_of_rotation_y_offset,
                    &p->applied.center_of_rotation_z_offset);
            }
        }
        else if (strncmp(rec->process_parameters.param[i], "APPLIED_POSITION_LATENCY", strlen("APPLIED_POSITION_LATENCY")) == 0)
        {
            p->applied.position_latency = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "APPLIED_POSITION_LATENCY=%lf",
                    &p->applied.position_latency);
            }
        }
        else if (strncmp(rec->process_parameters.param[i], "APPLIED_ATTITUDE_LATENCY", strlen("APPLIED_ATTITUDE_LATENCY")) == 0)
        {
            p->applied.attitude_latency = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "APPLIED_ATTITUDE_LATENCY=%lf",
                    &p->applied.attitude_latency);
            }
        }
        else if (strncmp(rec->process_parameters.param[i], "APPLIED_DEPTH_SENSOR_LATENCY", strlen("APPLIED_DEPTH_SENSOR_LATENCY")) == 0)
        {
            p->applied.depth_sensor_latency = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "APPLIED_DEPTH_SENSOR_LATENCY=%lf",
                    &p->applied.depth_sensor_latency);
            }
        }
        else if (strncmp(rec->process_parameters.param[i], "APPLIED_DEPTH_SENSOR_OFFSET", strlen("APPLIED_DEPTH_SENSOR_OFFSET")) == 0)
        {
            p->applied.depth_sensor_x_offset = GSF_UNKNOWN_PARAM_VALUE;
            p->applied.depth_sensor_y_offset = GSF_UNKNOWN_PARAM_VALUE;
            p->applied.depth_sensor_z_offset = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "APPLIED_DEPTH_SENSOR_OFFSET=%lf,%lf,%lf",
                    &p->applied.depth_sensor_x_offset,
                    &p->applied.depth_sensor_y_offset,
                    &p->applied.depth_sensor_z_offset);
            }
        }
        else if (strncmp(rec->process_parameters.param[i], "APPLIED_RX_TRANSDUCER_OFFSET", strlen("APPLIED_RX_TRANSDUCER_OFFSET")) == 0)
        {
            p->applied.rx_transducer_x_offset[0] = GSF_UNKNOWN_PARAM_VALUE;
            p->applied.rx_transducer_y_offset[0] = GSF_UNKNOWN_PARAM_VALUE;
            p->applied.rx_transducer_z_offset[0] = GSF_UNKNOWN_PARAM_VALUE;
            p->applied.rx_transducer_x_offset[1] = GSF_UNKNOWN_PARAM_VALUE;
            p->applied.rx_transducer_y_offset[1] = GSF_UNKNOWN_PARAM_VALUE;
            p->applied.rx_transducer_z_offset[1] = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "APPLIED_RX_TRANSDUCER_OFFSET=%lf,%lf,%lf,%lf,%lf,%lf",
                    &p->applied.rx_transducer_x_offset[0],
                    &p->applied.rx_transducer_y_offset[0],
                    &p->applied.rx_transducer_z_offset[0],
                    &p->applied.rx_transducer_x_offset[1],
                    &p->applied.rx_transducer_y_offset[1],
                    &p->applied.rx_transducer_z_offset[1]);
            }
        }
        else if (strncmp(rec->process_parameters.param[i], "APPLIED_RX_TRANSDUCER_PITCH_OFFSET", strlen("APPLIED_RX_TRANSDUCER_PITCH_OFFSET")) == 0)
        {
            p->applied.rx_transducer_pitch_offset[0] = GSF_UNKNOWN_PARAM_VALUE;
            p->applied.rx_transducer_pitch_offset[1] = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "APPLIED_RX_TRANSDUCER_PITCH_OFFSET=%lf,%lf",
                    &p->applied.rx_transducer_pitch_offset[0],
                    &p->applied.rx_transducer_pitch_offset[1]);
            }
        }
        else if (strncmp(rec->process_parameters.param[i], "APPLIED_RX_TRANSDUCER_ROLL_OFFSET", strlen("APPLIED_RX_TRANSDUCER_ROLL_OFFSET")) == 0)
        {
            p->applied.rx_transducer_roll_offset[0] = GSF_UNKNOWN_PARAM_VALUE;
            p->applied.rx_transducer_roll_offset[1] = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "APPLIED_RX_TRANSDUCER_ROLL_OFFSET=%lf,%lf",
                    &p->applied.rx_transducer_roll_offset[0],
                    &p->applied.rx_transducer_roll_offset[1]);
            }
        }
        else if (strncmp(rec->process_parameters.param[i], "APPLIED_RX_TRANSDUCER_HEADING_OFFSET", strlen("APPLIED_RX_TRANSDUCER_HEADING_OFFSET")) == 0)
        {
            p->applied.rx_transducer_heading_offset[0] = GSF_UNKNOWN_PARAM_VALUE;
            p->applied.rx_transducer_heading_offset[1] = GSF_UNKNOWN_PARAM_VALUE;
            if (!strstr(rec->process_parameters.param[i], GSF_UNKNOWN_PARAM_TEXT))
            {
                sscanf (rec->process_parameters.param[i], "APPLIED_RX_TRANSDUCER_HEADING_OFFSET=%lf,%lf",
                    &p->applied.rx_transducer_heading_offset[0],
                    &p->applied.rx_transducer_heading_offset[1]);
            }
        }   /** end of "applied" parameters **/

        /* The horizontal datum parameter defines the elipsoid to which
         * the latitude and longitude values are referenced.
         */
        else if (strncmp(rec->process_parameters.param[i], "GEOID", strlen("GEOID")) == 0)
        {
            sscanf (rec->process_parameters.param[i], "GEOID=%6s", str);
            if (strstr(str, "WGS-84"))
            {
                p->horizontal_datum = GSF_H_DATUM_WGE;
            }
            else if (strstr(str, "NAD-83"))
            {
                p->horizontal_datum = GSF_H_DATUM_NAR;
            }
            else
            {
                p->horizontal_datum = GSF_H_DATUM_UND;
            }
        }

        /* The TIDAL_DATUM paremeter defines the reference datum for tide
         * corrections
         */
        else if (strncmp(rec->process_parameters.param[i], "TIDAL_DATUM", strlen("TIDAL_DATUM")) == 0)
        {
            sscanf (rec->process_parameters.param[i], "TIDAL_DATUM=%6s",
                str);

            if (strcmp(str, "MLLWS") == 0)
            {
                p->vertical_datum = GSF_V_DATUM_MLLWS;
            }
            else if (strcmp(str, "AMLLW") == 0)
            {
                p->vertical_datum = GSF_V_DATUM_AMLLW;
            }
            else if (strcmp(str, "MLLW") == 0)
            {
                p->vertical_datum = GSF_V_DATUM_MLLW;
            }
            else if (strcmp(str, "MLWN") == 0)
            {
                p->vertical_datum = GSF_V_DATUM_MLWN;
            }
            else if (strcmp(str, "AMLWS") == 0)
            {
                p->vertical_datum = GSF_V_DATUM_AMLWS;
            }
            else if (strcmp(str, "MLWS") == 0)
            {
                p->vertical_datum = GSF_V_DATUM_MLWS;
            }
            else if (strcmp(str, "AMLW") == 0)
            {
                p->vertical_datum = GSF_V_DATUM_AMLW;
            }
            else if (strcmp(str, "MLW") == 0)
            {
                p->vertical_datum = GSF_V_DATUM_MLW;
            }
            else if (strcmp(str, "ALAT") == 0)
            {
                p->vertical_datum = GSF_V_DATUM_ALAT;
            }
            else if (strcmp(str, "ESLW") == 0)
            {
                p->vertical_datum = GSF_V_DATUM_ESLW;
            }
            else if (strcmp(str, "ISLW") == 0)
            {
                p->vertical_datum = GSF_V_DATUM_ISLW;
            }
            else if (strcmp(str, "LAT") == 0)
            {
                p->vertical_datum = GSF_V_DATUM_LAT;
            }
            else if (strcmp(str, "ALLWS") == 0)
            {
                p->vertical_datum = GSF_V_DATUM_ALLWS;
            }
            else if (strcmp(str, "ALLW") == 0)
            {
                p->vertical_datum = GSF_V_DATUM_ALLW;
            }
            else if (strcmp(str, "LLW") == 0)
            {
                p->vertical_datum = GSF_V_DATUM_LLW;
            }
            else if (strcmp(str, "LNLW") == 0)
            {
                p->vertical_datum = GSF_V_DATUM_LNLW;
            }
            else if (strcmp(str, "LWD") == 0)
            {
                p->vertical_datum = GSF_V_DATUM_LWD;
            }
            else if (strcmp(str, "MLHW") == 0)
            {
                p->vertical_datum = GSF_V_DATUM_MLHW;
            } 
            else if (strcmp(str, "AMSL") == 0)
            {
                p->vertical_datum = GSF_V_DATUM_AMSL;
            }
            else if (strcmp(str, "MSL") == 0)
            {
                p->vertical_datum = GSF_V_DATUM_MSL;
            }
            else if (strcmp(str, "LNT") == 0)
            {
                p->vertical_datum = GSF_V_DATUM_LNT;
            }
            else if (strcmp(str, "AISLW") == 0)
            {
                p->vertical_datum = GSF_V_DATUM_AISLW;
            }
            else
            {
                p->vertical_datum = GSF_V_DATUM_UNKNOWN;
            }
        }
    }  // for
    p->number_of_transmitters = num_tx;
    p->number_of_receivers = num_rx;

    return(0);
}

/********************************************************************
 *
 * Function Name : gsfNumberParams
 *
 * Description : This function parses a KEYWORD=VALUE style parameter
 *   and returns the number of comma delimited values which follow the
 *   equal sign.
 *
 * Inputs :
 *    param = a pointer to a character string containing a KEYWORD=VALUE
 *    style parameter.
 *
 * Returns : This function returns the number of comma delimited values
 *    which follow the equal sign.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
gsfNumberParams (const char *params)
{
    int number = 0;


    if (params)
    {
        char *s = strchr (params, '=');

        while (s)
        {
            s = strchr (++s, ',');
            number++;
        }
    }

    return (number);
}

/********************************************************************
 *
 * Function Name : gsfGetSwathBathyBeamWidths
 *
 * Description : This function returns to the caller the fore-aft and
 *    the port-starboard beam widths in degrees for a swath bathymetry
 *    multibeam sonar, given a gsfRecords data structure which contains
 *    a populated gsfSwathBathyPing structure.
 *
 * Inputs :
 *     data = The address of a gsfRecords data structure maintained by the
 *         caller which contains a populated gsfSwathBathyPing substructure.
 *     fore_aft = The address of a double allocated by the caller which will
 *         be loaded with the sonar's fore/aft beam width in degrees.
 *     athwartship = The address of a double allocated by the caller which will
 *         be loaded with the sonar's athwartship beam width in degrees.
 *
 * Returns : This function returns zero if successful, or -1 if an error occurred.
 *
 * Error Conditions :
 *   GSF_UNRECOGNIZED_SENSOR_ID
 *
 ********************************************************************/
int
gsfGetSwathBathyBeamWidths(const gsfRecords *data, double *fore_aft, double *athwartship)
{
    int             ret=0;   /* Assume that we will be successful. */

    /* Switch on the type of sonar this data came from */
    switch (data->mb_ping.sensor_id)
    {
        case GSF_SWATH_BATHY_SUBRECORD_SEABEAM_SPECIFIC:
            *fore_aft = 2.666666666667;
            *athwartship = 2.666666666667;
            break;

        case GSF_SWATH_BATHY_SUBRECORD_EM12_SPECIFIC:
            *fore_aft = 1.7;
            *athwartship = 4.4;
            break;

        case GSF_SWATH_BATHY_SUBRECORD_EM100_SPECIFIC:
            switch (data->mb_ping.sensor_data.gsfEM100Specific.mode)
            {
                case(1):  /* wide */
                   *athwartship = 2.5;
                   break;

                case(2):  /* ultra-wide */
                   *athwartship = 5.5;
                   break;

                case(3):  /* narrow */
                   *athwartship = 2.0;
                   break;

                default:  /* Unrecognized sonar mode */
                   *athwartship = GSF_BEAM_WIDTH_UNKNOWN;
                   ret = -1;
                   break;
            }
            *fore_aft = 3.0;
            break;

        case GSF_SWATH_BATHY_SUBRECORD_EM950_SPECIFIC:
        case GSF_SWATH_BATHY_SUBRECORD_EM1000_SPECIFIC:
            *fore_aft = 3.3;
            *athwartship = 3.3;
            break;

        case GSF_SWATH_BATHY_SUBRECORD_EM121A_SPECIFIC:
            *fore_aft = data->mb_ping.sensor_data.gsfEM121ASpecific.beam_width;
            *athwartship = data->mb_ping.sensor_data.gsfEM121ASpecific.beam_width;
            break;

        case GSF_SWATH_BATHY_SUBRECORD_EM121_SPECIFIC:
            *fore_aft = data->mb_ping.sensor_data.gsfEM121Specific.beam_width;
            *athwartship = data->mb_ping.sensor_data.gsfEM121Specific.beam_width;
            break;

#if 1
/* 04-01-99 wkm/dbj: obsolete */
        case GSF_SWATH_BATHY_SUBRECORD_SASS_SPECIFIC:
            *fore_aft = GSF_BEAM_WIDTH_UNKNOWN;
            *athwartship = GSF_BEAM_WIDTH_UNKNOWN;
            ret = -1;
            break;
#endif

        case GSF_SWATH_BATHY_SUBRECORD_SEAMAP_SPECIFIC:
            *fore_aft = GSF_BEAM_WIDTH_UNKNOWN;
            *athwartship = GSF_BEAM_WIDTH_UNKNOWN;
            ret = -1;
            break;

        case GSF_SWATH_BATHY_SUBRECORD_SEABAT_SPECIFIC:
            if (data->mb_ping.sensor_data.gsfSeaBatSpecific.mode & GSF_SEABAT_WIDE_MODE)
            {
                *fore_aft = 10.0;
            }
            else
            {
                /* Set the F/A beam width to 1.5 here, but also set the return code to
                 * indicate failure.  This sonar supports multiple beam widths, and
                 * this information is NOT provided in the data stream from the sonar.
                 */
                *fore_aft = 1.5;
                ret = -1;
            }
            if (data->mb_ping.sensor_data.gsfSeaBatSpecific.mode & GSF_SEABAT_9003)
            {
                *athwartship = 3.0;
            }
            else
            {
                *athwartship = 1.5;
            }
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_SEABAT_II_SPECIFIC):
            *fore_aft = data->mb_ping.sensor_data.gsfSeaBatIISpecific.fore_aft_bw;
            *athwartship = data->mb_ping.sensor_data.gsfSeaBatIISpecific.athwart_bw;
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_SEABAT_8101_SPECIFIC):
            *fore_aft = data->mb_ping.sensor_data.gsfSeaBat8101Specific.fore_aft_bw;
            *athwartship = data->mb_ping.sensor_data.gsfSeaBat8101Specific.athwart_bw;
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_SEABEAM_2112_SPECIFIC):
            *fore_aft = 2.0;
            *athwartship = 2.0;
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_ELAC_MKII_SPECIFIC):
            *fore_aft = 2.0;
            *athwartship = 2.0;
            break;

        case GSF_SWATH_BATHY_SUBRECORD_CMP_SASS_SPECIFIC:
            *fore_aft    = 1.0;
            *athwartship = 1.0;
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_EM300_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_EM1002_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_EM2000_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_EM3000_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_EM120_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_EM3002_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_EM3000D_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_EM3002D_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_EM121A_SIS_SPECIFIC):
            *fore_aft = 1.5;
            *athwartship = 1.5;
            if (data->mb_ping.sensor_data.gsfEM3Specific.run_time[0].transmit_beam_width != 0.0)
            {
                *fore_aft = data->mb_ping.sensor_data.gsfEM3Specific.run_time[0].transmit_beam_width;
            }
            if (data->mb_ping.sensor_data.gsfEM3Specific.run_time[0].receive_beam_width != 0.0)
            {
                *athwartship = data->mb_ping.sensor_data.gsfEM3Specific.run_time[0].receive_beam_width;
            }
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_EM300_RAW_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_EM1002_RAW_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_EM2000_RAW_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_EM3000_RAW_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_EM120_RAW_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_EM3002_RAW_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_EM3000D_RAW_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_EM3002D_RAW_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_EM121A_SIS_RAW_SPECIFIC):
            *fore_aft = 1.5;
            *athwartship = 1.5;
            if (data->mb_ping.sensor_data.gsfEM3RawSpecific.run_time.tx_beam_width != 0.0)
            {
                *fore_aft = data->mb_ping.sensor_data.gsfEM3RawSpecific.run_time.tx_beam_width;
            }
            if (data->mb_ping.sensor_data.gsfEM3RawSpecific.run_time.rx_beam_width != 0.0)
            {
                *athwartship = data->mb_ping.sensor_data.gsfEM3RawSpecific.run_time.rx_beam_width;
            }
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_EM122_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_EM302_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_EM710_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_EM2040_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_ME70BO_SPECIFIC):
            *fore_aft = 1.0;
            *athwartship = 1.0;
            if (data->mb_ping.sensor_data.gsfEM4Specific.run_time.tx_beam_width != 0.0)
            {
                *fore_aft = data->mb_ping.sensor_data.gsfEM4Specific.run_time.tx_beam_width;
            }
            if (data->mb_ping.sensor_data.gsfEM4Specific.run_time.rx_beam_width != 0.0)
            {
                *athwartship = data->mb_ping.sensor_data.gsfEM4Specific.run_time.rx_beam_width;
            }
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_RESON_8101_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_RESON_8111_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_RESON_8124_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_RESON_8125_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_RESON_8150_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_RESON_8160_SPECIFIC):
            *fore_aft = data->mb_ping.sensor_data.gsfReson8100Specific.fore_aft_bw;
            *athwartship = data->mb_ping.sensor_data.gsfReson8100Specific.athwart_bw;
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_RESON_7125_SPECIFIC):
            *fore_aft = data->mb_ping.sensor_data.gsfReson7100Specific.projector_beam_wdth_vert;
            *athwartship = data->mb_ping.sensor_data.gsfReson7100Specific.receive_beam_width;
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_RESON_TSERIES_SPECIFIC):
            *fore_aft = data->mb_ping.sensor_data.gsfResonTSeriesSpecific.projector_beam_wdth_vert;
            *athwartship = data->mb_ping.sensor_data.gsfResonTSeriesSpecific.receive_beam_width;
            break;

        case GSF_SWATH_BATHY_SUBRECORD_GEOSWATH_PLUS_SPECIFIC:
            switch (data->mb_ping.sensor_data.gsfGeoSwathPlusSpecific.model_number)
            {
                case 100:
                    *fore_aft = 0.9;
                    *athwartship = GSF_BEAM_WIDTH_UNKNOWN;
                    break;

                case 250:
                    *fore_aft = 0.5;
                    *athwartship = GSF_BEAM_WIDTH_UNKNOWN;
                    break;

                case 500:
                    *fore_aft = 0.5;
                    *athwartship = GSF_BEAM_WIDTH_UNKNOWN;
                    break;

                default:
                    *fore_aft = GSF_BEAM_WIDTH_UNKNOWN;
                    *athwartship = GSF_BEAM_WIDTH_UNKNOWN;
                    break;
            }
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_KLEIN_5410_BSS_SPECIFIC):
            *fore_aft = GSF_BEAM_WIDTH_UNKNOWN;
            *athwartship = GSF_BEAM_WIDTH_UNKNOWN;
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_DELTA_T_SPECIFIC):
            *fore_aft = 3.0;
            *athwartship = 3.0;
            if (data->mb_ping.sensor_data.gsfDeltaTSpecific.fore_aft_beamwidth != 0.0)
            {
                *fore_aft = data->mb_ping.sensor_data.gsfDeltaTSpecific.fore_aft_beamwidth;
            }
            if (data->mb_ping.sensor_data.gsfDeltaTSpecific.athwartships_beamwidth != 0.0)
            {
                *athwartship = data->mb_ping.sensor_data.gsfDeltaTSpecific.athwartships_beamwidth;
            }
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_R2SONIC_2020_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_R2SONIC_2022_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_R2SONIC_2024_SPECIFIC):
            *fore_aft = data->mb_ping.sensor_data.gsfR2SonicSpecific.tx_beamwidth_vert;
            *athwartship = data->mb_ping.sensor_data.gsfR2SonicSpecific.tx_beamwidth_horiz;
            break;
        
		case (GSF_SWATH_BATHY_SUBRECORD_KMALL_SPECIFIC):
		    *fore_aft = data->mb_ping.sensor_data.gsfKMALLSpecific.transmitArraySizeUsed_deg;
			*athwartship = data->mb_ping.sensor_data.gsfKMALLSpecific.receiveArraySizeUsed_deg;
		    break;

        default:
            *fore_aft = GSF_BEAM_WIDTH_UNKNOWN;
            *athwartship = GSF_BEAM_WIDTH_UNKNOWN;
            gsfError = GSF_UNRECOGNIZED_SENSOR_ID;
            ret = -1;
            break;
    }
    return(ret);
}

/********************************************************************
 *
 * Function Name : gsfIsStarboardPing
 *
 * Description : This function uses the sonar specific data union
 *     of a gsfSwathBathymetry ping structure to determine if the ping
 *     is from the starboard arrays of a multibeam installation with
 *     dual transmit receive sonar arrays.
 *
 * Inputs :
 *     data = The address of a gsfRecords data structure maintained by the
 *         caller which contains a populated gsfSwathBathyPing substructure.
 *
 * Returns : This function returns non-zero if the ping contained in the
 *     passed data represents a starboard looking ping from a dual headed
 *     sonar installation.  Otherwise, zero is returned.
 *
 * Error Conditions : none
 *
 ********************************************************************/
int
gsfIsStarboardPing(const gsfRecords *data)
{
    int ret = 0;


    /* Switch on the type of sonar this data came from */
    switch (data->mb_ping.sensor_id)
    {
        case GSF_SWATH_BATHY_SUBRECORD_GEOSWATH_PLUS_SPECIFIC:
            return data->mb_ping.sensor_data.gsfGeoSwathPlusSpecific.side;
            break;
        case GSF_SWATH_BATHY_SUBRECORD_KLEIN_5410_BSS_SPECIFIC:
            return data->mb_ping.sensor_data.gsfKlein5410BssSpecific.side;
            break;
        case GSF_SWATH_BATHY_SUBRECORD_SEABAT_SPECIFIC:
/* zzz_ */
/*          if (data->mb_ping.sensor_data.gsfSeaBatSpecific.mode &   */
/*               (GSF_SEABAT_9002 | GSF_SEABAT_STBD_HEAD))           */
        if ( data->mb_ping.sensor_data.gsfSeaBatSpecific.mode  &  GSF_SEABAT_STBD_HEAD )
/* zzz_ */
        {
           ret = 1;
        }
        break;

        case GSF_SWATH_BATHY_SUBRECORD_ELAC_MKII_SPECIFIC:
        if ( data->mb_ping.sensor_data.gsfElacMkIISpecific.mode  &  GSF_MKII_STBD_HEAD )
        {
           ret = 1;
        }
        break;

        case GSF_SWATH_BATHY_SUBRECORD_EM3000D_SPECIFIC:
        case GSF_SWATH_BATHY_SUBRECORD_EM3002D_SPECIFIC:
        case GSF_SWATH_BATHY_SUBRECORD_EM3000D_RAW_SPECIFIC:
        case GSF_SWATH_BATHY_SUBRECORD_EM3002D_RAW_SPECIFIC:
        case GSF_SWATH_BATHY_SUBRECORD_EM2040_SPECIFIC:
            /* it is assumed that the center_beam is set to the vertical beam. */
            if (data->mb_ping.center_beam < data->mb_ping.number_beams / 2)
            {
                /* most of the beams are to starboard of vertical */
                ret = 1;
            }
            else
            {
                /* most of the beams are to port of vertical */
                ret = 0;
            }
        break;

        case GSF_SWATH_BATHY_SUBRECORD_KMALL_SPECIFIC:
            /* KMALL docs suggest rxTransducerInd == 0 for port, == 1 for starboard */
		    if ( data->mb_ping.sensor_data.gsfKMALLSpecific.rxTransducerInd == 1 )
            {
               ret = 1;
            }
			break;

        default:
            ret = 0;
            break;
    }

    return(ret);
}

/********************************************************************
 *
 * Function Name : gsfLoadDepthScaleFactorAutoOffset
 *
 * Description : gsfLoadDepthScaleFactorAutoOffset may be used to load
 *  the scale factors for the depth subrecords of the swath bathymetry ping
 *  record scale factor structure. The approach uses the tide and depth
 *  correction fields to help establish the offset component of the scale
 *  factor such that negative depth values may be supported.  Negative
 *  depth values may be encountered when surveying above the tidal datum.
 *  In addition, this function may be used for systems mounted on subsea
 *  platforms where high depth precision may be supported even in deep
 *  water.
 *
 * Inputs :
 *   ping = A pointer to the gsfSwathBathyPing which contains the depth
 *      and tide correction values, and the scale factors data structure.
 *  subrecordID = the subrecord id for the beam array data.  This must be
 *      either GSF_SWATH_BATHY_SUBRECORD_DEPTH_ARRAY, or
 *      GSF_SWATH_BATHY_SUBRECORD_NOMINAL_DEPTH_ARRAY
 *  reset = An integer value which will cause the internal logic to be
 *      refreshed when the value is non-zero.  The first call to this function
 *      should use a non-zero reset, from then on, this value may be passed
 *      as zero.
 *  min_depth = A double value which should be set to the minimum depth value
 *      contained in the depth array specified by subrecordID.  This argument
 *      exists for completeness, but is currently not used.
 *  max_depth = A double value which should be set to the maximum depth value
 *      contained in the depth array specified by subrecordID.  When a depth
 *      threshold is exceeded, the offset used to support "signed depth" is
 *      no longer required and will no longer be used.  This approach is
 *      necessary to avoid an integer overflow when the array data are scaled.
 *  last_corrector = The address of a double value stored as permanent memory.
 *      Successive calls to this function must pass the same address for this
 *      argument.  This function will take care of setting the value at this
 *      address, but the caller is responsible for ensuring that the same
 *      permanent memory address is used for each call to this function.
 *    c_flag = The compression flag for the beam array.
 *    precision = The precision to which the beam array data are to be stored
 *      (a value of 0.1 would indicate decimeter precision for depth).
 *
 * Returns :
 *   This function returns zero if successful, or -1 if an error occurred.
 *
 * Error Conditions :
 *   GSF_UNRECOGNIZED_ARRAY_SUBRECORD_ID
 *
 ********************************************************************/
int
gsfLoadDepthScaleFactorAutoOffset(gsfSwathBathyPing *ping, unsigned int subrecordID, int reset, double min_depth, double max_depth, double *last_corrector, char c_flag, double precision)
{
    double          offset;
    double          fraction;
    double          layer;
    double          next_layer;
    double          corrector = 0.0;
    double          layer_interval = 100.0;
    double          max_depth_threshold = 400.0;
    double          max_depth_hysteresis = 30.0;
    double          increasing_threshold;
    double          decreasing_threshold;
    int             dc_offset;
    int             percent;
    int             ret_code = 0;

    /* Test for valid subrecordID, we only supported automated establishement of the DC offset for the depth subrecords */
    if ((subrecordID != GSF_SWATH_BATHY_SUBRECORD_DEPTH_ARRAY) && (subrecordID != GSF_SWATH_BATHY_SUBRECORD_NOMINAL_DEPTH_ARRAY))
    {
        gsfError = GSF_UNRECOGNIZED_ARRAY_SUBRECORD_ID;
        return (-1);
    }

    if (precision < 0.01)
    {
        layer_interval = 10.0;
    }

    /* Get the current offset scaling factor from the ping data structure */
    offset    = ping->scaleFactors.scaleTable[subrecordID - 1].offset;

    /* Break the total correction value into integer and fractional components based on the layering interval */
	if ((ping->ping_flags & GSF_PING_USER_FLAG_14) || (ping->ping_flags & GSF_PING_USER_FLAG_15))
		corrector = ping->depth_corrector + ping->tide_corrector;
	else if ((ping->ping_flags & GSF_PING_USER_FLAG_13))
		corrector = ping->gps_tide_corrector;
    fraction = modf (corrector / layer_interval, &layer);
    layer = layer * layer_interval;

    /* Handle the startup and/or reset situation */
    if (reset)
    {
        if (layer < layer_interval)
        {
            offset = -1.0 * (layer - layer_interval);
        }
        else
        {
            offset = -1.0 * layer;
        }
        *last_corrector = 0.0;
    }

    /* Set the corrector layer trip thresholds based on the sign of the current layer */
    if (fraction < 0.0)
    {
        percent = (int) (fraction * layer_interval);
        increasing_threshold = -70.0;
        decreasing_threshold = -90.0;
        next_layer = layer - layer_interval;
    }
    else
    {
        percent = (int) (fraction * layer_interval);
        increasing_threshold = 30.0;
        decreasing_threshold = 10.0;
        next_layer = layer;
    }

    /* The transition from one corrector layer to the next occurs if the
     * total corrector is increasing and passes through one threshold, or
     * if the total corrector is decreasing and passed through another
     * threshold.
     */
    if (*last_corrector < corrector)
    {
        /* If the depth is greater than 400 meters then there is no need to
         *  use a positive DC offset for signed depth.  This check is necessary
         *  to avoid a potential integer overflow that may exist for sonar
         *  systems which do not decrease the precision as the depth increases.
         */
        if ((fabs(corrector) < layer_interval) && (max_depth > (max_depth_threshold - max_depth_hysteresis)))
        {
            if (max_depth > (max_depth_threshold + max_depth_hysteresis))
            {
                offset = 0.0;
            }
        }

        /* If corrector is increasing, change offset to next deeper
         * depth layer when we pass through the threshold.
         */
        else if (percent > increasing_threshold)
        {
            offset = -1.0 * next_layer;
        }
    }
    else
    {
        /* If the depth is greater than 400 meters then there is no need to
         *  use a positive DC offset for signed depth.  This check is necessary
         *  to avoid a potential integer overflow that may exist for sonar
         *  systems which do not decrease the precision as the depth increases.
         */
        if ((fabs(corrector) < layer_interval) && (max_depth > (max_depth_threshold - max_depth_hysteresis)))
        {
            if (max_depth > (max_depth_threshold + max_depth_hysteresis))
            {
                offset = 0.0;
            }
        }

        /* If corrector is decreasing, change offset to next shallower
         *  depth layer when we pass through the threshold.
         */
        else if (percent < decreasing_threshold)
        {
            offset = -1.0 * (next_layer - layer_interval);
        }
    }

    /* the maximum possible tidal height is just under 11 meters, so a maximum
     *  offset of 20 is sufficient for surveying above the tidal datum.
     *  bac, 09-12-04
     */
    if (offset > 20)
    {
        offset = 20;
    }

    /* Round to the nearest whole integer */
    if (offset < 0.0)
    {
        dc_offset = (int) (offset - 0.5);
    }
    else
    {
        dc_offset = (int) (offset + 0.5);
    }

    /* Call the load scale factors function to set the computed DC offset and
     *  the c_flag and precision arguments.
     */
    if (gsfLoadScaleFactor(&ping->scaleFactors, subrecordID, c_flag, precision, dc_offset) != 0)
    {
        return (-1);
    }

    if (corrector != *last_corrector)
    {
        *last_corrector = corrector;
    }

    return (ret_code);
}

/********************************************************************
 *
 * Function Name : gsfGetSwathBathyArrayMinMax
 *
 * Description : This function may be used to obtain the minimum and maximum
 *  supportable values for each of the swath bathymetry arrays.  The minimum
 *  and maximum values are determined based on the scale factors and the array
 *  type.
 *
 * Inputs :
 *   ping = A pointer to the gsfSwathBathyPing which contains the depth
 *      and tide correction values, and the scale factors data structure.
 *  subrecordID = The subrecord id for the beam array data.  This must be
 *      either GSF_SWATH_BATHY_SUBRECORD_DEPTH_ARRAY, or
 *      GSF_SWATH_BATHY_SUBRECORD_NOMINAL_DEPTH_ARRAY
 *  min_value = The address of a double value allocated by the caller into
 *      which will be placed the minimum value which may be represented for
 *      this array type.
 *  max_value = The address of a double value allocated by the caller into
 *      which will be placed the maximum value which may be represented for
 *      this array type.
 *
 * Returns :
 *   This function returns zero if successful, or -1 if an error occurred.
 *
 * Error Conditions :
 *    GSF_UNRECOGNIZED_ARRAY_SUBRECORD_ID
 *    GSF_ILLEGAL_SCALE_FACTOR_MULTIPLIER
 *
 ********************************************************************/
int
gsfGetSwathBathyArrayMinMax(const gsfSwathBathyPing *ping, unsigned int subrecordID, double *min_value, double *max_value)
{
    double          minimum;
    double          maximum;
    double          multiplier;
    double          offset;

	/* Make sure scale factors have been established for this array. */
    if (ping->scaleFactors.scaleTable[subrecordID - 1].multiplier == 0.0)
    {
        gsfError = GSF_ILLEGAL_SCALE_FACTOR_MULTIPLIER;
        return (-1);
    }

    switch (subrecordID)
    {
        case (GSF_SWATH_BATHY_SUBRECORD_DEPTH_ARRAY):
        case (GSF_SWATH_BATHY_SUBRECORD_NOMINAL_DEPTH_ARRAY):
        case (GSF_SWATH_BATHY_SUBRECORD_TRAVEL_TIME_ARRAY):
            switch (ping->scaleFactors.scaleTable[subrecordID - 1].compressionFlag & 0xf0)
            {
                case GSF_FIELD_SIZE_ONE:
                    minimum = GSF_U_CHAR_MIN;
                    maximum = GSF_U_CHAR_MAX;
                    break;
                default:
                case GSF_FIELD_SIZE_DEFAULT:
                case GSF_FIELD_SIZE_TWO:
                    minimum = GSF_U_SHORT_MIN;
                    maximum = GSF_U_SHORT_MAX;
                    break;
                case GSF_FIELD_SIZE_FOUR:
                    minimum = GSF_U_INT_MIN;
                    maximum = GSF_U_INT_MAX;
                    break;
            }
            break;
        case (GSF_SWATH_BATHY_SUBRECORD_ACROSS_TRACK_ARRAY):
        case (GSF_SWATH_BATHY_SUBRECORD_ALONG_TRACK_ARRAY):
            switch (ping->scaleFactors.scaleTable[subrecordID - 1].compressionFlag & 0xf0)
            {
                case GSF_FIELD_SIZE_ONE:
                    minimum = GSF_S_CHAR_MIN;
                    maximum = GSF_S_CHAR_MAX;
                    break;
                default:
                case GSF_FIELD_SIZE_DEFAULT:
                case GSF_FIELD_SIZE_TWO:
                    minimum = GSF_S_SHORT_MIN;
                    maximum = GSF_S_SHORT_MAX;
                    break;
                case GSF_FIELD_SIZE_FOUR:
                    minimum = GSF_S_INT_MIN;
                    maximum = GSF_S_INT_MAX;
                    break;
            }
            break;
        case (GSF_SWATH_BATHY_SUBRECORD_MEAN_CAL_AMPLITUDE_ARRAY):
            switch (ping->scaleFactors.scaleTable[subrecordID - 1].compressionFlag & 0xf0)
            {
                default:
                case GSF_FIELD_SIZE_DEFAULT:
                case GSF_FIELD_SIZE_ONE:
                    minimum = GSF_S_CHAR_MIN;
                    maximum = GSF_S_CHAR_MAX;
                    break;
                case GSF_FIELD_SIZE_TWO:
                    minimum = GSF_S_SHORT_MIN;
                    maximum = GSF_S_SHORT_MAX;
                    break;
            }
            break;
        case (GSF_SWATH_BATHY_SUBRECORD_MEAN_REL_AMPLITUDE_ARRAY):
        case (GSF_SWATH_BATHY_SUBRECORD_ECHO_WIDTH_ARRAY):
            switch (ping->scaleFactors.scaleTable[subrecordID - 1].compressionFlag & 0xf0)
            {
                default:
                case GSF_FIELD_SIZE_DEFAULT:
                case GSF_FIELD_SIZE_ONE:
                    minimum = GSF_U_CHAR_MIN;
                    maximum = GSF_U_CHAR_MAX;
                    break;
                case GSF_FIELD_SIZE_TWO:
                    minimum = GSF_U_SHORT_MIN;
                    maximum = GSF_U_SHORT_MAX;
                    break;
            }
            break;
        case (GSF_SWATH_BATHY_SUBRECORD_BEAM_ANGLE_ARRAY):
            minimum = GSF_S_SHORT_MIN;
            maximum = GSF_S_SHORT_MAX;
            break;
        case (GSF_SWATH_BATHY_SUBRECORD_DEPTH_ERROR_ARRAY):
        case (GSF_SWATH_BATHY_SUBRECORD_ACROSS_TRACK_ERROR_ARRAY):
        case (GSF_SWATH_BATHY_SUBRECORD_ALONG_TRACK_ERROR_ARRAY):
        case (GSF_SWATH_BATHY_SUBRECORD_BEAM_ANGLE_FORWARD_ARRAY):
        case (GSF_SWATH_BATHY_SUBRECORD_VERTICAL_ERROR_ARRAY):
        case (GSF_SWATH_BATHY_SUBRECORD_HORIZONTAL_ERROR_ARRAY):
            minimum = GSF_U_SHORT_MIN;
            maximum = GSF_U_SHORT_MAX;
            break;
        case (GSF_SWATH_BATHY_SUBRECORD_QUALITY_FACTOR_ARRAY):
        case (GSF_SWATH_BATHY_SUBRECORD_QUALITY_FLAGS_ARRAY):
        case (GSF_SWATH_BATHY_SUBRECORD_BEAM_FLAGS_ARRAY):
        case (GSF_SWATH_BATHY_SUBRECORD_SIGNAL_TO_NOISE_ARRAY):
        case (GSF_SWATH_BATHY_SUBRECORD_SECTOR_NUMBER_ARRAY):
        case (GSF_SWATH_BATHY_SUBRECORD_DETECTION_INFO_ARRAY):
        case (GSF_SWATH_BATHY_SUBRECORD_SYSTEM_CLEANING_ARRAY):
        case (GSF_SWATH_BATHY_SUBRECORD_TVG_ARRAY):
            minimum = GSF_U_CHAR_MIN;
            maximum = GSF_U_CHAR_MAX;
            break;
        case (GSF_SWATH_BATHY_SUBRECORD_RECEIVE_HEAVE_ARRAY):
        case (GSF_SWATH_BATHY_SUBRECORD_INCIDENT_BEAM_ADJ_ARRAY):
        case (GSF_SWATH_BATHY_SUBRECORD_DOPPLER_CORRECTION_ARRAY):
            minimum = GSF_S_CHAR_MIN;
            maximum = GSF_S_CHAR_MAX;
            break;
        default:
            gsfError = GSF_UNRECOGNIZED_ARRAY_SUBRECORD_ID;
            return (-1);
            break;
    }

    multiplier = ping->scaleFactors.scaleTable[subrecordID - 1].multiplier;
    offset     = ping->scaleFactors.scaleTable[subrecordID - 1].offset;

    *min_value = ((minimum / multiplier) - offset);
    *max_value = ((maximum / multiplier) - offset);

    return (0);
}

/********************************************************************
 *
 * Function Name : gsfGetSonarTextName
 *
 * Description : This function provides a textual name for the sonar
 *  given a populated ping structure.
 *
 * Inputs :
 *  ping = A pointer to a populated gsfSwathBathyPing structure.
 *
 * Returns :
 *  This function returns a pointer to a character string containing
 *   the name of the sonar.
 *
 * Error Conditions : none
 *
 ********************************************************************/
const char *gsfGetSonarTextName(const gsfSwathBathyPing *ping)
{
    const char             *ptr;

    switch (ping->sensor_id)
    {
        case (GSF_SWATH_BATHY_SUBRECORD_SEABEAM_SPECIFIC):
            ptr = "SeaBeam";
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_EM12_SPECIFIC):
            ptr = "Simrad EM12";
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_EM100_SPECIFIC):
            ptr = "Simrad EM100";
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_EM950_SPECIFIC):
            ptr = "Simrad EM950";
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_EM1000_SPECIFIC):
            ptr = "Simrad EM1000";
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_EM121A_SPECIFIC):
            ptr = "Simrad EM121A";
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_SASS_SPECIFIC):
            ptr = "SASS";
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_SEAMAP_SPECIFIC):
            ptr = "SeaMap";
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_SB_AMP_SPECIFIC):
            ptr = "Sea Beam (w/amp)";
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_SEABAT_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_SEABAT_II_SPECIFIC):
            if (ping->sensor_data.gsfSeaBatIISpecific.mode & GSF_SEABAT_9002)
            {
                ptr = "Reson SeaBat 9002";
            }
            else if (ping->sensor_data.gsfSeaBatIISpecific.mode & GSF_SEABAT_9003)
            {
                ptr = "Reson SeaBat 9003";
            }
            else
            {
                ptr = "Reson SeaBat 9001";
            }
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_SEABAT_8101_SPECIFIC):
            ptr = "Reson SeaBat 8101";
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_SEABEAM_2112_SPECIFIC):
            ptr = "Sea Beam 2112/36";
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_ELAC_MKII_SPECIFIC):
            ptr = "ELAC MKII";
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_EM120_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_EM120_RAW_SPECIFIC):
            ptr = "Kongsberg EM120";
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_EM300_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_EM300_RAW_SPECIFIC):
            ptr = "Kongsberg EM300";
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_EM1002_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_EM1002_RAW_SPECIFIC):
            ptr = "Kongsberg EM1002";
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_EM2000_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_EM2000_RAW_SPECIFIC):
            ptr = "Kongsberg EM2000";
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_EM3000_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_EM3000_RAW_SPECIFIC):
            ptr = "Kongsberg EM3000";
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_EM3000D_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_EM3000D_RAW_SPECIFIC):
            ptr = "Kongsberg EM3000D";
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_EM3002_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_EM3002_RAW_SPECIFIC):
            ptr = "Kongsberg EM3002";
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_EM3002D_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_EM3002D_RAW_SPECIFIC):
            ptr = "Kongsberg EM3002D";
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_EM121A_SIS_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_EM121A_SIS_RAW_SPECIFIC):
            ptr = "Kongsberg EM121A (SIS)";
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_RESON_7125_SPECIFIC):
            ptr = "Reson SeaBat 7125";
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_RESON_TSERIES_SPECIFIC):
            ptr = "Reson SeaBat T Series";
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_RESON_8101_SPECIFIC):
            ptr = "Reson SeaBat 8101";
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_RESON_8111_SPECIFIC):
            ptr = "Reson SeaBat 8111";
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_RESON_8124_SPECIFIC):
            ptr = "Reson SeaBat 8124";
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_RESON_8125_SPECIFIC):
            ptr = "Reson SeaBat 8125";
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_RESON_8150_SPECIFIC):
            ptr = "Reson SeaBat 8150";
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_RESON_8160_SPECIFIC):
            ptr = "Reson SeaBat 8160";
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_EM122_SPECIFIC):
            ptr = "Kongsberg EM122";
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_ME70BO_SPECIFIC):
            ptr = "Kongsberg ME70";
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_EM302_SPECIFIC):
            ptr = "Kongsberg EM302";
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_EM710_SPECIFIC):
            ptr = "Kongsberg EM710";
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_KLEIN_5410_BSS_SPECIFIC):
            ptr = "Klein 5410";
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_GEOSWATH_PLUS_SPECIFIC):
            ptr = "GeoAcoustics GeoSwath+";
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_EM2040_SPECIFIC):
            ptr = "Kongsberg EM2040";
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_DELTA_T_SPECIFIC):
            ptr = "Imagenex Delta T";
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_R2SONIC_2020_SPECIFIC):
            ptr = "R2Sonic 2020";
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_R2SONIC_2022_SPECIFIC):
            ptr = "R2Sonic 2022";
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_R2SONIC_2024_SPECIFIC):
            ptr = "R2Sonic 2024";
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_KMALL_SPECIFIC):
            switch (ping->sensor_data.gsfKMALLSpecific.echoSounderID)
            {
                case 122:
                    ptr = "Kongsberg EM122";
                    break;     
                case 124:
                    ptr = "Kongsberg EM124";
                    break;    
				case 302:
                    ptr = "Kongsberg EM302";
                    break;     
                case 304:
                    ptr = "Kongsberg EM304";
                    break;
				case 710:
                    ptr = "Kongsberg EM710";
                    break;     
                case 712:
                    ptr = "Kongsberg EM712";
                    break;     
				case 2040:
                    ptr = "Kongsberg EM2040";
                    break;       
			}    
            break;

        default:
            ptr = "Unknown";
            break;
    }

    return (ptr);
}

/********************************************************************
 *
 * Function Name : gsfIsNewSurveyLine
 *
 * Description : This function provides an approach for calling applications
 *  to determine if the last ping read from a GSF file is from the same survey
 *  transect line, or if the last ping is from a newly started survey line. The
 *  implementation looks for a change in platform heading to determine that the
 *  last ping read is from a new survey line. External to this function, calling
 *  applications can decide on their own if the first ping read from a newly opened
 *  GSF file should be considered to be from a new survey transect line or not.
 *  This function assumes that the GSF file is read in chronological order from
 *  the beginning of the file, file access can be either direct or sequential
 *
 * Inputs :
 *  handle         = The handle to the file as provided by gsfOpen
 *  rec            = A pointer to a gsfRecords structure containing the data from the most
 *                    recent call to gsfRead
 *  azimuth_change = The trigger value specifying the change in platform heading that
 *                    must be exceeded for a new survey transect line to be determined
 *  last_heading   = A pointer to a double allocated by the caller and into which this
 *                    function will place the heading value for each detected line. The
 *                    value must be allocated as permanent memory that persists through
 *                    all calls to this function. Startup or reset events can be handled
 *                    by the caller by placing a negative value in this memory location.
 *
 * Returns :
 *  This function returns 1 if this ping is considered to be the first ping of a new
 *   survey transect line, otherwise, 0 is returned.
 *
 * Error Conditions :
 *   GSF_BAD_FILE_HANDLE
 *
 ********************************************************************/
int
gsfIsNewSurveyLine(int handle, const gsfRecords *rec, double azimuth_change, double *last_heading)
{
    double diff;
    int    new_line;

    new_line = 0;

    if ((handle < 1) || (handle > GSF_MAX_OPEN_FILES))
    {
        gsfError = GSF_BAD_FILE_HANDLE;
        return (-1);
    }
    if (gsfFileTable[handle-1].last_record_type == GSF_RECORD_SWATH_BATHYMETRY_PING)
    {
        /* A negative value for last heading is the start/reset trigger. */
        if (*last_heading < 0.0)
        {
            new_line = 1;
            *last_heading = rec->mb_ping.heading;
        }
        else
        {
            diff = fabs (rec->mb_ping.heading - *last_heading);
            if ((diff > azimuth_change) && (diff < 350.0))
            {
                new_line = 1;
                *last_heading = rec->mb_ping.heading;
            }
        }
    }

    return(new_line);
}

/********************************************************************
 *
 * Function Name : gsfInitializeMBParams
 *
 * Description : This function provides a way to initialize all the
 *    sonar processing parameters to "unknown"
 *
 * Inputs :
 *    p = a pointer to the gsfMBParams data structure that needs initializing
 *
 * Returns :
 *    None
 *
 * Error Conditions :
 *    None
 *
 ********************************************************************/
void
gsfInitializeMBParams (gsfMBParams *p)
{
    int i;

    memset(p->start_of_epoch, 0, sizeof(p->start_of_epoch));
    p->horizontal_datum = GSF_UNKNOWN_PARAM_INT;
    p->vertical_datum = GSF_UNKNOWN_PARAM_INT;
    p->roll_compensated = GSF_UNKNOWN_PARAM_INT;
    p->pitch_compensated = GSF_UNKNOWN_PARAM_INT;
    p->heave_compensated = GSF_UNKNOWN_PARAM_INT;
    p->tide_compensated = GSF_UNKNOWN_PARAM_INT;
    p->ray_tracing = GSF_UNKNOWN_PARAM_INT;
    p->depth_calculation = GSF_UNKNOWN_PARAM_INT;
    p->vessel_type = GSF_PLATFORM_TYPE_SURFACE_SHIP; // default to surface ship
    p->full_raw_data = GSF_UNKNOWN_PARAM_INT;
    p->msb_applied_to_attitude = GSF_UNKNOWN_PARAM_INT;
    p->heave_removed_from_gps_tc = GSF_UNKNOWN_PARAM_INT;
    p->utc_offset = GSF_UNKNOWN_PARAM_INT;
    p->roll_reference = GSF_UNKNOWN_PARAM_INT;
    p->number_of_transmitters = GSF_UNKNOWN_PARAM_INT;
    p->number_of_receivers = GSF_UNKNOWN_PARAM_INT;

    /* initialize the "to apply" fields */
    p->to_apply.position_x_offset = GSF_UNKNOWN_PARAM_VALUE;
    p->to_apply.position_y_offset = GSF_UNKNOWN_PARAM_VALUE;
    p->to_apply.position_z_offset = GSF_UNKNOWN_PARAM_VALUE;
    p->to_apply.antenna_x_offset = GSF_UNKNOWN_PARAM_VALUE;
    p->to_apply.antenna_y_offset = GSF_UNKNOWN_PARAM_VALUE;
    p->to_apply.antenna_z_offset = GSF_UNKNOWN_PARAM_VALUE;
    p->to_apply.mru_pitch_bias = GSF_UNKNOWN_PARAM_VALUE;
    p->to_apply.mru_roll_bias = GSF_UNKNOWN_PARAM_VALUE;
    p->to_apply.mru_heading_bias = GSF_UNKNOWN_PARAM_VALUE;
    p->to_apply.mru_x_offset = GSF_UNKNOWN_PARAM_VALUE;
    p->to_apply.mru_y_offset = GSF_UNKNOWN_PARAM_VALUE;
    p->to_apply.mru_z_offset = GSF_UNKNOWN_PARAM_VALUE;
    p->to_apply.center_of_rotation_x_offset = GSF_UNKNOWN_PARAM_VALUE;
    p->to_apply.center_of_rotation_y_offset = GSF_UNKNOWN_PARAM_VALUE;
    p->to_apply.center_of_rotation_z_offset = GSF_UNKNOWN_PARAM_VALUE;
    p->to_apply.position_latency = GSF_UNKNOWN_PARAM_VALUE;
    p->to_apply.attitude_latency = GSF_UNKNOWN_PARAM_VALUE;
    p->to_apply.depth_sensor_latency = GSF_UNKNOWN_PARAM_VALUE;
    p->to_apply.depth_sensor_x_offset = GSF_UNKNOWN_PARAM_VALUE;
    p->to_apply.depth_sensor_y_offset = GSF_UNKNOWN_PARAM_VALUE;
    p->to_apply.depth_sensor_z_offset = GSF_UNKNOWN_PARAM_VALUE;
    for (i = 0; i < GSF_MAX_OFFSETS; i++)
    {
        p->to_apply.draft[i] = GSF_UNKNOWN_PARAM_VALUE;
        p->to_apply.pitch_bias[i] = GSF_UNKNOWN_PARAM_VALUE;
        p->to_apply.roll_bias[i] = GSF_UNKNOWN_PARAM_VALUE;
        p->to_apply.gyro_bias[i] = GSF_UNKNOWN_PARAM_VALUE;
        p->to_apply.transducer_x_offset[i] = GSF_UNKNOWN_PARAM_VALUE;
        p->to_apply.transducer_y_offset[i] = GSF_UNKNOWN_PARAM_VALUE;
        p->to_apply.transducer_z_offset[i] = GSF_UNKNOWN_PARAM_VALUE;
        p->to_apply.transducer_pitch_offset[i] = GSF_UNKNOWN_PARAM_VALUE;
        p->to_apply.transducer_roll_offset[i] = GSF_UNKNOWN_PARAM_VALUE;
        p->to_apply.transducer_heading_offset[i] = GSF_UNKNOWN_PARAM_VALUE;
        p->to_apply.rx_transducer_x_offset[i] = GSF_UNKNOWN_PARAM_VALUE;
        p->to_apply.rx_transducer_y_offset[i] = GSF_UNKNOWN_PARAM_VALUE;
        p->to_apply.rx_transducer_z_offset[i] = GSF_UNKNOWN_PARAM_VALUE;
        p->to_apply.rx_transducer_pitch_offset[i] = GSF_UNKNOWN_PARAM_VALUE;
        p->to_apply.rx_transducer_roll_offset[i] = GSF_UNKNOWN_PARAM_VALUE;
        p->to_apply.rx_transducer_heading_offset[i] = GSF_UNKNOWN_PARAM_VALUE;
    }

    /* initialize the "applied" fields */
    p->applied.position_x_offset = GSF_UNKNOWN_PARAM_VALUE;
    p->applied.position_y_offset = GSF_UNKNOWN_PARAM_VALUE;
    p->applied.position_z_offset = GSF_UNKNOWN_PARAM_VALUE;
    p->applied.antenna_x_offset = GSF_UNKNOWN_PARAM_VALUE;
    p->applied.antenna_y_offset = GSF_UNKNOWN_PARAM_VALUE;
    p->applied.antenna_z_offset = GSF_UNKNOWN_PARAM_VALUE;
    p->applied.mru_pitch_bias = GSF_UNKNOWN_PARAM_VALUE;
    p->applied.mru_roll_bias = GSF_UNKNOWN_PARAM_VALUE;
    p->applied.mru_heading_bias = GSF_UNKNOWN_PARAM_VALUE;
    p->applied.mru_x_offset = GSF_UNKNOWN_PARAM_VALUE;
    p->applied.mru_y_offset = GSF_UNKNOWN_PARAM_VALUE;
    p->applied.mru_z_offset = GSF_UNKNOWN_PARAM_VALUE;
    p->applied.center_of_rotation_x_offset = GSF_UNKNOWN_PARAM_VALUE;
    p->applied.center_of_rotation_y_offset = GSF_UNKNOWN_PARAM_VALUE;
    p->applied.center_of_rotation_z_offset = GSF_UNKNOWN_PARAM_VALUE;
    p->applied.position_latency = GSF_UNKNOWN_PARAM_VALUE;
    p->applied.attitude_latency = GSF_UNKNOWN_PARAM_VALUE;
    p->applied.depth_sensor_latency = GSF_UNKNOWN_PARAM_VALUE;
    p->applied.depth_sensor_x_offset = GSF_UNKNOWN_PARAM_VALUE;
    p->applied.depth_sensor_y_offset = GSF_UNKNOWN_PARAM_VALUE;
    p->applied.depth_sensor_z_offset = GSF_UNKNOWN_PARAM_VALUE;
    for (i = 0; i < GSF_MAX_OFFSETS; i++)
    {
        p->applied.draft[i] = GSF_UNKNOWN_PARAM_VALUE;
        p->applied.pitch_bias[i] = GSF_UNKNOWN_PARAM_VALUE;
        p->applied.roll_bias[i] = GSF_UNKNOWN_PARAM_VALUE;
        p->applied.gyro_bias[i] = GSF_UNKNOWN_PARAM_VALUE;
        p->applied.transducer_x_offset[i] = GSF_UNKNOWN_PARAM_VALUE;
        p->applied.transducer_y_offset[i] = GSF_UNKNOWN_PARAM_VALUE;
        p->applied.transducer_z_offset[i] = GSF_UNKNOWN_PARAM_VALUE;
        p->applied.transducer_pitch_offset[i] = GSF_UNKNOWN_PARAM_VALUE;
        p->applied.transducer_roll_offset[i] = GSF_UNKNOWN_PARAM_VALUE;
        p->applied.transducer_heading_offset[i] = GSF_UNKNOWN_PARAM_VALUE;
        p->applied.rx_transducer_x_offset[i] = GSF_UNKNOWN_PARAM_VALUE;
        p->applied.rx_transducer_y_offset[i] = GSF_UNKNOWN_PARAM_VALUE;
        p->applied.rx_transducer_z_offset[i] = GSF_UNKNOWN_PARAM_VALUE;
        p->applied.rx_transducer_pitch_offset[i] = GSF_UNKNOWN_PARAM_VALUE;
        p->applied.rx_transducer_roll_offset[i] = GSF_UNKNOWN_PARAM_VALUE;
        p->applied.rx_transducer_heading_offset[i] = GSF_UNKNOWN_PARAM_VALUE;
    }
}

