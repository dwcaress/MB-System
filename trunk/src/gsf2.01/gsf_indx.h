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
 * who	when	  what
 * ---	----	  ----
 * jsb  11/01/95  Start with index file version INDEX-GSF-v01.00 
 *
 * Classification : Unclassified
 *
 * References : DoDBL Generic Sensor Format Sept. 30, 1993
 *
 *
 * Copyright (C) Science Applications International Corp.
 ********************************************************************/

#ifndef __GSF_INDX__
    #define __GSF_INDX__

    /* This header has the GSF File Table structure definition */
    #include "gsf_ft.h"

    /* Define a macro to hold a version tag for gsf index file. Note that
     * the macro definition for the version size defines the number of bytes
     * read/written for the version, so this should NOT be changed.
     */
    #define GSF_INDEX_VERSION	    "INDEX-GSF-v01.00"
    #define GSF_INDEX_VERSION_SIZE  16

    /* Typedef a structure to hold the index file header information */
    typedef struct t_gsfIndexHeader
    {
	char	    version[GSF_INDEX_VERSION_SIZE+1];
	long	    gsfFileSize;
	long	    endian;
	int	    number_record_types;
	long	    spare1;
	long	    spare2;
	long	    spare3;
	long	    spare4;
    } GSF_INDEX_HEADER;

    /* Prototypes for this module */
    int OPTLK	    gsfOpenIndex(const char *filename, int handle, GSF_FILE_TABLE *ft);
    int OPTLK	    gsfCloseIndex(GSF_FILE_TABLE *ft);
    void OPTLK	    SwapLong(unsigned long *base, long count);

#endif /* __GSF_INDX__ */
