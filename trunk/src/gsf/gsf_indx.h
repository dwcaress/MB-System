/********************************************************************
 *
 * Module Name : GSF_INDX
 *
 * Author/Date : J. S. Byrne / 23 Aug 1995
 *
 * Description : This header file contains function prototypes and
 *    definitions required for direct access to gsf files.  This header
 *    file is internal to the library, and none of these functions are
 *    intended to be exported from the library.
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 * who  when      what
 * ---  ----      ----
 * jsb  11/01/95  Start with index file version INDEX-GSF-v01.00
 * bac  06/28/06  Added J.Depner updates to support a progress callback
 *                 when writing to the index file, as an alternative to
 *                 the DISPLAY_SPINNER printouts.  Changed function
 *                 arguments and structure elements of type long to
 *                 int, for compilation on 64-bit architectures.
 *
 * Classification : Unclassified
 *
 * References : DoDBL Generic Sensor Format Sept. 30, 1993
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

#ifndef __GSF_INDX__
    #define __GSF_INDX__

#ifdef  __cplusplus
extern "C" {
#endif


    /* This header has the GSF File Table structure definition */
    #include "gsf_ft.h"

    /* Define a macro to hold a version tag for gsf index file. Note that
     * the macro definition for the version size defines the number of bytes
     * read/written for the version, so this should NOT be changed.
     */
    #define GSF_INDEX_VERSION       "INDEX-GSF-v02.00"
    #define GSF_INDEX_VERSION_SIZE  16

    /* Typedef a structure to hold the index file header information */
    typedef struct t_gsfIndexHeader
    {
        char        version[GSF_INDEX_VERSION_SIZE+1];
        long long   gsfFileSize;
        int         endian;
        int         number_record_types;
        int         spare1;
        int         spare2;
        int         spare3;
        int         spare4;
    } GSF_INDEX_HEADER;


    /* JCD: Typedef for index progress callback */
    typedef void (*GSF_PROGRESS_CALLBACK) (int state, int percent);


    /* Prototypes for this module */
    int OPTLK       gsfOpenIndex(const char *filename, int handle, GSF_FILE_TABLE *ft);
    int OPTLK       gsfCloseIndex(GSF_FILE_TABLE *ft);
    void OPTLK      SwapLong(unsigned int *base, int count);
    void OPTLK      SwapLongLong(long long *base_address, int count);

    void OPTLK      gsf_register_progress_callback (GSF_PROGRESS_CALLBACK progressCB);
    /*
     * Description : The gsf_register_progress_callback function registers a callback
     *                function, defined by the user, to be called to report the progress
     *                of the index file creation.  If no progress callback is registered,
     *                status is printed to stdout if the DISPLAY_SPINNER macro is defined
     *                during compilation of the GSF library.
     *
     * Inputs :
     *    GSF_PROGRESS_CALLBACK progressCB = Name of progress callback function to call
     *                                       when creating the GSF index file.  The
     *                                       progress callback will accept two integer
     *                                       arguments, and this function will be called
     *                                       whenever the percent complete changes.
     *                                       The first argument will be one of the
     *                                       following three values, to represent the
     *                                       state of the progress:
     *                                       1 = Reading GSF file
     *                                       2 = Creating new index file
     *                                       3 = Appending to existing index file
     *                                       The second argument contains the percent
     *                                       complete of the current state.
     *
     * Returns : none
     *
     * Error Conditions : none
     */

#ifdef  __cplusplus
}
#endif

#endif /* __GSF_INDX__ */
