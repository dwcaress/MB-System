/* Added HAVE_CONFIG_H for autogen files */
#ifdef HAVE_CONFIG_H
#  include <mbsystem_config.h>
#endif


/********************************************************************
 *
 * Module Name : gsf_info.c
 *
 * Author/Date : J. S. Byrne / December 2009
 *
 * Description :
 *  This source file contains GSF informational functions that provide
 *   general information or status of contents of the current file.
 *
 * Restrictions/Limitations :
 * 1) This library assumes the host computer uses the ASCII character set.
 * 2) This library assumes that the data types u_short and u_int are defined
 *    on the host machine, where a u_short is a 16 bit unsigned integer, and
 *    a u_int is a 32 bit unsigned integer.
 * 3) This library assumes that the type short is at least 16 bits, and that
 *    the type int is at least 32 bits.
 *
 *
 * Change Descriptions :
 * who          when      what
 * ---          ----      ----
 * jsb          12-29-09  New
 *
 * Classification : Unclassified
 *
 * References : DoDBL Generic Sensor Format Sept. 30, 1993
 *
 * Copyright (C) Science Applications International Corp.
 ********************************************************************/

/* standard c library includes */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* rely on the network type definitions of (u_short, and u_int) */
#include <sys/types.h>
#ifndef WIN32
#include <netinet/in.h>
#else
#include <winsock.h>
#endif

/* gsf library interface description */
#include "gsf.h"

/* Global external data defined in this module */
extern int      gsfError;                               /* defined in gsf.c */

/* Function prototypes for this file */

/********************************************************************
 *
 * Function Name : gsfFileSupportsRecalculateXYZ
 *
 * Description :
 *  This function reads the GSF file referenced by handle and determines
 *  if the file contains sufficient information to support a full recalculation
 *  of the platform relative XYZ values from raw measurements. This function
 *  rewinds the file to the first record and reads through the file looking for
 *  the information required to support a recalculation. On success, the file
 *  pointer is reset to the beginning of the file before the function returns.
 *
 * Inputs :
 *  handle = the integer handle returned from gsfOpen()
 *  status = A pointer to an integer allocated by caller into which the 
 *            function result is placed. *status is assigned a value of 1 
 *            if this file provides sufficient information to support full
 *            recalculation of the platform relative XYZ values, otherwise 
 *            *status is assigned a value of 0.
 *
 * Returns :
 *  This function returns zero if successful, or -1 if an error occurred.
 *
 * Error Conditions :
 *  GSF_BAD_FILE_HANDLE
 *  GSF_FILE_SEEK_ERROR
 *  GSF_FLUSH_ERROR
 *  GSF_READ_TO_END_OF_FILE
 *  GSF_READ_ERROR
 *  GSF_RECORD_SIZE_ERROR
 *  GSF_INSUFFICIENT_SIZE
 *  GSF_CHECKSUM_FAILURE
 *  GSF_UNRECOGNIZED_RECORD_ID
 *  GSF_HEADER_RECORD_DECODE_FAILED
 *  GSF_SVP_RECORD_DECODE_FAILED
 *  GSF_PROCESS_PARAM_RECORD_DECODE_FAILED
 *  GSF_SENSOR_PARAM_RECORD_DECODE_FAILED
 *  GSF_COMMENT_RECORD_DECODE_FAILED
 *  GSF_HISTORY_RECORD_DECODE_FAILED
 *  GSF_NAV_ERROR_RECORD_DECODE_FAILED
 *
 ********************************************************************/
int
gsfFileSupportsRecalculateXYZ(int handle, int *status)
{
    int             ret;
    int             i;
    int             rec_size;
    int             att_rec;
    int             svp_rec;
    int             param_rec;
    int             ping_rec;
    gsfDataID       id;
    gsfRecords      rec;
    
    memset (&id, 0, sizeof(id));
    memset (&rec, 0, sizeof(rec));
    att_rec = 0;
    svp_rec = 0;
    param_rec = 0;
    ping_rec = 0;

    *status = 0;

    /* Rewind the file so that the pointer is at the first record. */
    ret = gsfSeek(handle, GSF_REWIND);
    if (ret) 
    {
        gsfError = GSF_FILE_SEEK_ERROR;
        return (-1);
    }

    for (i = 0; i < 100; i++) 
    {
        rec_size = gsfRead(handle, GSF_NEXT_RECORD, &id, &rec, NULL, 0);
        if (rec_size < 0) 
        {
            if (gsfError == GSF_READ_TO_END_OF_FILE) 
            {            
                ret = gsfSeek(handle, GSF_REWIND);
                if (ret) 
                {
                    gsfError = GSF_FILE_SEEK_ERROR;
                    return (-1);
                }
                return(0);
            }
            else
            {
                /* gsfError should already be set to indicate the type of failure that occurred */
                return (-1);
            }
        }
        
        switch (id.recordID)
        {
            default:
            case (GSF_RECORD_NAVIGATION_ERROR):
            case (GSF_RECORD_SWATH_BATHY_SUMMARY):
            case (GSF_RECORD_HISTORY):
            case (GSF_RECORD_SENSOR_PARAMETERS):
            case (GSF_RECORD_COMMENT):
            case (GSF_RECORD_HEADER):
                break;

            case (GSF_RECORD_SWATH_BATHYMETRY_PING):
                if ((rec.mb_ping.travel_time) && (rec.mb_ping.beam_angle))
                {
                    switch (rec.mb_ping.sensor_id) 
                    {
                        default:
                            break;

                        case GSF_SWATH_BATHY_SUBRECORD_RESON_8101_SPECIFIC:
                        case GSF_SWATH_BATHY_SUBRECORD_RESON_8111_SPECIFIC:
                        case GSF_SWATH_BATHY_SUBRECORD_RESON_8124_SPECIFIC:
                        case GSF_SWATH_BATHY_SUBRECORD_RESON_8125_SPECIFIC:
                        case GSF_SWATH_BATHY_SUBRECORD_RESON_8150_SPECIFIC:
                        case GSF_SWATH_BATHY_SUBRECORD_RESON_8160_SPECIFIC:
                        case GSF_SWATH_BATHY_SUBRECORD_RESON_7125_SPECIFIC:
                        case GSF_SWATH_BATHY_SUBRECORD_EM300_RAW_SPECIFIC:
                        case GSF_SWATH_BATHY_SUBRECORD_EM1002_RAW_SPECIFIC: 
                        case GSF_SWATH_BATHY_SUBRECORD_EM2000_RAW_SPECIFIC:
                        case GSF_SWATH_BATHY_SUBRECORD_EM3000_RAW_SPECIFIC:
                        case GSF_SWATH_BATHY_SUBRECORD_EM120_RAW_SPECIFIC:
                        case GSF_SWATH_BATHY_SUBRECORD_EM3002_RAW_SPECIFIC:
                        case GSF_SWATH_BATHY_SUBRECORD_EM3000D_RAW_SPECIFIC:
                        case GSF_SWATH_BATHY_SUBRECORD_EM3002D_RAW_SPECIFIC:
                            ping_rec++;
                            break;

                        case GSF_SWATH_BATHY_SUBRECORD_EM710_SPECIFIC:
                        case GSF_SWATH_BATHY_SUBRECORD_EM302_SPECIFIC:
                        case GSF_SWATH_BATHY_SUBRECORD_EM122_SPECIFIC:
                            if (rec.mb_ping.sector_number) 
                            {
                                ping_rec++;
                            }
                            break;
                    }
                }
                break;

            case (GSF_RECORD_SOUND_VELOCITY_PROFILE):
                if (rec.svp.number_points > 1)
                {
                    svp_rec++;
                }
                break;

            case (GSF_RECORD_PROCESSING_PARAMETERS):
                if (rec.process_parameters.number_parameters > 1)
                {
                    param_rec++;
                }
                break;

            case (GSF_RECORD_ATTITUDE):
                if (rec.attitude.num_measurements > 1) 
                {
                    att_rec++;
                }
                break;
        }

        if (ping_rec && svp_rec && param_rec && att_rec) 
        {
            *status = 1;
            break;
        }
    }

    /* reset the file pointer to where it was when function was called */
    /* Rewind the file so that the pointer is at the first record. */
    ret = gsfSeek(handle, GSF_REWIND);
    if (ret) 
    {
        gsfError = GSF_FILE_SEEK_ERROR;
        return (-1);
    }

    return (0);
}

/********************************************************************
 *
 * Function Name : gsfFileSupportsRecalculateTPU
 *
 * Description :
 *  This function reads the GSF file referenced by handle and determines
 *  if the file contains sufficient information to support a recalculation
 *  of the total propagated uncertainty (TPU) estimates. This function
 *  rewinds the file to the first record and reads through the file looking for
 *  the information required to support TPU estimation. On success, the file
 *  pointer is reset to the beginning of the file before the function returns.
 *
 * Inputs :
 *  handle = the integer handle returned from gsfOpen()
 *  status = A pointer to an integer allocated by caller into which the 
 *           function result is placed. *status is assigned a value of 1 
 *           if this file provides sufficient information to support TPU
 *           estimation, otherwise *status is assigned a value of 0.
 *
 * Returns :
 *  This function returns zero if successful, or -1 if an error occurred.
 *
 * Error Conditions :
 *  GSF_BAD_FILE_HANDLE
 *  GSF_FILE_SEEK_ERROR
 *  GSF_FLUSH_ERROR
 *  GSF_READ_TO_END_OF_FILE
 *  GSF_READ_ERROR
 *  GSF_RECORD_SIZE_ERROR
 *  GSF_INSUFFICIENT_SIZE
 *  GSF_CHECKSUM_FAILURE
 *  GSF_UNRECOGNIZED_RECORD_ID
 *  GSF_HEADER_RECORD_DECODE_FAILED
 *  GSF_SVP_RECORD_DECODE_FAILED
 *  GSF_PROCESS_PARAM_RECORD_DECODE_FAILED
 *  GSF_SENSOR_PARAM_RECORD_DECODE_FAILED
 *  GSF_COMMENT_RECORD_DECODE_FAILED
 *  GSF_HISTORY_RECORD_DECODE_FAILED
 *  GSF_NAV_ERROR_RECORD_DECODE_FAILED
 *
 ********************************************************************/
int
gsfFileSupportsRecalculateTPU(int handle, int *status)
{
    int             ret;
    int             i;
    int             rec_size;
    int             svp_rec;
    int             param_rec;
    int             ping_rec;
    gsfDataID       id;
    gsfRecords      rec;
    
    memset (&id, 0, sizeof(id));
    memset (&rec, 0, sizeof(rec));
    svp_rec = 0;
    param_rec = 0;
    ping_rec = 0;

    *status = 0;

    /* Rewind the file so that the pointer is at the first record. */
    ret = gsfSeek(handle, GSF_REWIND);
    if (ret) 
    {
        gsfError = GSF_FILE_SEEK_ERROR;
        return (-1);
    }

    for (i = 0; i < 100; i++) 
    {
        rec_size = gsfRead(handle, GSF_NEXT_RECORD, &id, &rec, NULL, 0);
        if (rec_size < 0) 
        {
            if (gsfError == GSF_READ_TO_END_OF_FILE) 
            {            
                ret = gsfSeek(handle, GSF_REWIND);
                if (ret) 
                {
                    gsfError = GSF_FILE_SEEK_ERROR;
                    return (-1);
                }
                return(0);
            }
            else
            {
                /* gsfError should already be set to indicate the type of failure that occurred */
                return (-1);
            }
        }
        
        switch (id.recordID)
        {
            default:
            case (GSF_RECORD_NAVIGATION_ERROR):
            case (GSF_RECORD_SWATH_BATHY_SUMMARY):
            case (GSF_RECORD_HISTORY):
            case (GSF_RECORD_SENSOR_PARAMETERS):
            case (GSF_RECORD_COMMENT):
            case (GSF_RECORD_HEADER):
                break;

            case (GSF_RECORD_SWATH_BATHYMETRY_PING):
                if ((rec.mb_ping.depth) && (rec.mb_ping.across_track))
                {
                    switch (rec.mb_ping.sensor_id) 
                    {
                        case GSF_SWATH_BATHY_SUBRECORD_EM710_SPECIFIC:
                        case GSF_SWATH_BATHY_SUBRECORD_EM302_SPECIFIC:
                        case GSF_SWATH_BATHY_SUBRECORD_EM122_SPECIFIC:
                            if ((rec.mb_ping.sector_number) && (rec.mb_ping.sensor_data.gsfEM4Specific.sector[0].signal_length > 0))
                            {
                                ping_rec++;
                            }
                            break;
                      
                        default:
                            ping_rec++;
                            break;
                    }
                }
                break;

            case (GSF_RECORD_SOUND_VELOCITY_PROFILE):
                if (rec.svp.number_points > 1)
                {
                    svp_rec++;
                }
                break;

            case (GSF_RECORD_PROCESSING_PARAMETERS):
                if (rec.process_parameters.number_parameters > 1)
                {
                    param_rec++;
                }
                break;
        }

        if (ping_rec && svp_rec && param_rec) 
        {
            *status = 1;
            break;
        }
    }

    /* reset the file pointer to where it was when function was called */
    /* Rewind the file so that the pointer is at the first record. */
    ret = gsfSeek(handle, GSF_REWIND);
    if (ret) 
    {
        gsfError = GSF_FILE_SEEK_ERROR;
        return (-1);
    }

    return (0);
}

/********************************************************************
 *
 * Function Name : gsfFileSupportsRecalculateNominalDepth
 *
 * Description :
 *  This function reads the GSF file referenced by handle and determines
 *  if the file contains sufficient information to support a recalculation
 *  of the nominal depth array. This function rewinds the file to the first 
 *  record and reads through the file looking for the information required 
 *  to support calculation of the nominal depth values. On success, the file
 *  pointer is reset to the beginning of the file before the function returns.
 *
 * Inputs :
 *  handle = the integer handle returned from gsfOpen()
 *  status = A pointer to an integer allocated by caller into which the 
 *           function result is placed. *status is assigned a value of 1 
 *           if this file provides sufficient information to support
 *           nominal depth calculation, otherwise *status is assigned 
 *           a value of 0.
 *
 * Returns :
 *  This function returns zero if successful, or -1 if an error occurred.
 *
 * Error Conditions :
 *  GSF_BAD_FILE_HANDLE
 *  GSF_FILE_SEEK_ERROR
 *  GSF_FLUSH_ERROR
 *  GSF_READ_TO_END_OF_FILE
 *  GSF_READ_ERROR
 *  GSF_RECORD_SIZE_ERROR
 *  GSF_INSUFFICIENT_SIZE
 *  GSF_CHECKSUM_FAILURE
 *  GSF_UNRECOGNIZED_RECORD_ID
 *  GSF_HEADER_RECORD_DECODE_FAILED
 *  GSF_SVP_RECORD_DECODE_FAILED
 *  GSF_PROCESS_PARAM_RECORD_DECODE_FAILED
 *  GSF_SENSOR_PARAM_RECORD_DECODE_FAILED
 *  GSF_COMMENT_RECORD_DECODE_FAILED
 *  GSF_HISTORY_RECORD_DECODE_FAILED
 *  GSF_NAV_ERROR_RECORD_DECODE_FAILED
 *
 ********************************************************************/
int
gsfFileSupportsRecalculateNominalDepth(int handle, int *status)
{
    int             ret;
    int             i;
    int             rec_size;
    int             svp_rec;
    int             param_rec;
    int             ping_rec;
    gsfDataID       id;
    gsfRecords      rec;
    
    memset (&id, 0, sizeof(id));
    memset (&rec, 0, sizeof(rec));
    svp_rec = 0;
    param_rec = 0;
    ping_rec = 0;

    *status = 0;

    /* Rewind the file so that the pointer is at the first record. */
    ret = gsfSeek(handle, GSF_REWIND);
    if (ret) 
    {
        gsfError = GSF_FILE_SEEK_ERROR;
        return (-1);
    }

    for (i = 0; i < 100; i++) 
    {
        rec_size = gsfRead(handle, GSF_NEXT_RECORD, &id, &rec, NULL, 0);
        if (rec_size < 0) 
        {
            if (gsfError == GSF_READ_TO_END_OF_FILE) 
            {            
                ret = gsfSeek(handle, GSF_REWIND);
                if (ret) 
                {
                    gsfError = GSF_FILE_SEEK_ERROR;
                    return (-1);
                }
                return(0);
            }
            else
            {
                /* gsfError should already be set to indicate the type of failure that occurred */
                return (-1);
            }
        }
        
        switch (id.recordID)
        {
            default:
            case (GSF_RECORD_NAVIGATION_ERROR):
            case (GSF_RECORD_SWATH_BATHY_SUMMARY):
            case (GSF_RECORD_HISTORY):
            case (GSF_RECORD_SENSOR_PARAMETERS):
            case (GSF_RECORD_COMMENT):
            case (GSF_RECORD_HEADER):
                break;

            case (GSF_RECORD_SWATH_BATHYMETRY_PING):
                if ((rec.mb_ping.depth))
                {
                     ping_rec++;
                }
                break;

            case (GSF_RECORD_SOUND_VELOCITY_PROFILE):
                if (rec.svp.number_points > 1)
                {
                    svp_rec++;
                }
                break;

            case (GSF_RECORD_PROCESSING_PARAMETERS):
                if (rec.process_parameters.number_parameters > 1)
                {
                    param_rec++;
                }
                break;
        }

        if (ping_rec && svp_rec && param_rec) 
        {
            *status = 1;
            break;
        }
    }

    /* reset the file pointer to where it was when function was called */
    /* Rewind the file so that the pointer is at the first record. */
    ret = gsfSeek(handle, GSF_REWIND);
    if (ret) 
    {
        gsfError = GSF_FILE_SEEK_ERROR;
        return (-1);
    }

    return (0);
}

/********************************************************************
 *
 * Function Name : gsfFileContainsMBAmplitude
 *
 * Description :
 *  This function reads the GSF file referenced by handle and determines
 *  if the file contains the average per receive beam amplitude data.
 *  This function rewinds the file to the first record and reads through 
 *  the file up to and including the first ping record. If amplitude data
 *  are contained in the first ping record it is assumed that amplitude 
 *  data are contained with all ping records in this file. On success, 
 *  the file pointer is reset to the beginning of the file before the 
 *  function returns.
 *
 * Inputs :
 *  handle = the integer handle returned from gsfOpen()
 *  status = A pointer to an integer allocated by caller into which the 
 *           function result is placed. *status is assigned a value of 1 
 *           if this file contains the per receive beam amplitude data,
 *           otherwise *status is assigned a value of 0.
 *
 * Returns :
 *  This function returns zero if successful, or -1 if an error occurred.
 *
 * Error Conditions :
 *  GSF_BAD_FILE_HANDLE
 *  GSF_FILE_SEEK_ERROR
 *  GSF_FLUSH_ERROR
 *  GSF_READ_TO_END_OF_FILE
 *  GSF_READ_ERROR
 *  GSF_RECORD_SIZE_ERROR
 *  GSF_INSUFFICIENT_SIZE
 *  GSF_CHECKSUM_FAILURE
 *  GSF_UNRECOGNIZED_RECORD_ID
 *  GSF_HEADER_RECORD_DECODE_FAILED
 *  GSF_SVP_RECORD_DECODE_FAILED
 *  GSF_PROCESS_PARAM_RECORD_DECODE_FAILED
 *  GSF_SENSOR_PARAM_RECORD_DECODE_FAILED
 *  GSF_COMMENT_RECORD_DECODE_FAILED
 *  GSF_HISTORY_RECORD_DECODE_FAILED
 *  GSF_NAV_ERROR_RECORD_DECODE_FAILED
 *
 ********************************************************************/
int
gsfFileContainsMBAmplitude(int handle, int *status)
{
    int             ret;
    int             i;
    int             rec_size;
    int             mb_ping;
    gsfDataID       id;
    gsfRecords      rec;
    
    memset (&id, 0, sizeof(id));
    memset (&rec, 0, sizeof(rec));
    mb_ping = 0;
    *status = 0;

    /* Rewind the file so that the pointer is at the first record. */
    ret = gsfSeek(handle, GSF_REWIND);
    if (ret) 
    {
        gsfError = GSF_FILE_SEEK_ERROR;
        return (-1);
    }

    for (i = 0; i < 100; i++) 
    {
        rec_size = gsfRead(handle, GSF_NEXT_RECORD, &id, &rec, NULL, 0);
        if (rec_size < 0) 
        {
            if (gsfError == GSF_READ_TO_END_OF_FILE) 
            {            
                ret = gsfSeek(handle, GSF_REWIND);
                if (ret) 
                {
                    gsfError = GSF_FILE_SEEK_ERROR;
                    return (-1);
                }
                return(0);
            }
            else
            {
                /* gsfError should already be set to indicate the type of failure that occurred */
                return (-1);
            }
        }
        
        switch (id.recordID)
        {
            case (GSF_RECORD_SWATH_BATHYMETRY_PING):
                if ((rec.mb_ping.mc_amplitude) || (rec.mb_ping.mr_amplitude))
                {
                    *status = 1;
                }
                mb_ping = 1;
                break;
        }
        if (mb_ping == 1) 
        {
            break;
        }
    }

    /* reset the file pointer to where it was when function was called */
    /* Rewind the file so that the pointer is at the first record. */
    ret = gsfSeek(handle, GSF_REWIND);
    if (ret) 
    {
        gsfError = GSF_FILE_SEEK_ERROR;
        return (-1);
    }

    return (0);
}

/********************************************************************
 *
 * Function Name : gsfFileContainsMBImagery
 *
 * Description :
 *  This function reads the GSF file referenced by handle and determines
 *  if the file contains the per receive beam imagery time series data.
 *  This function rewinds the file to the first record and reads through 
 *  the file up to and including the first ping record. If MB imagery data
 *  are contained in the first ping record it is assumed that MB imagery 
 *  data are contained with all ping records in this file. On success, 
 *  the file pointer is reset to the beginning of the file before the 
 *  function returns.
 *
 * Inputs :
 *  handle = the integer handle returned from gsfOpen()
 *  status = A pointer to an integer allocated by caller into which the 
 *           function result is placed. *status is assigned a value of 1 
 *           if this file contains the per receive beam imagery time  
 *           series data, otherwise *status is assigned a value of 0.
 *
 * Returns :
 *  This function returns zero if successful, or -1 if an error occurred.
 *
 * Error Conditions :
 *  GSF_BAD_FILE_HANDLE
 *  GSF_FILE_SEEK_ERROR
 *  GSF_FLUSH_ERROR
 *  GSF_READ_TO_END_OF_FILE
 *  GSF_READ_ERROR
 *  GSF_RECORD_SIZE_ERROR
 *  GSF_INSUFFICIENT_SIZE
 *  GSF_CHECKSUM_FAILURE
 *  GSF_UNRECOGNIZED_RECORD_ID
 *  GSF_HEADER_RECORD_DECODE_FAILED
 *  GSF_SVP_RECORD_DECODE_FAILED
 *  GSF_PROCESS_PARAM_RECORD_DECODE_FAILED
 *  GSF_SENSOR_PARAM_RECORD_DECODE_FAILED
 *  GSF_COMMENT_RECORD_DECODE_FAILED
 *  GSF_HISTORY_RECORD_DECODE_FAILED
 *  GSF_NAV_ERROR_RECORD_DECODE_FAILED
 *
 ********************************************************************/
int
gsfFileContainsMBImagery(int handle, int *status)
{
    int             ret;
    int             i;
    int             rec_size;
    int             mb_ping;
    gsfDataID       id;
    gsfRecords      rec;
    
    memset (&id, 0, sizeof(id));
    memset (&rec, 0, sizeof(rec));
    mb_ping = 0;
    *status = 0;

    /* Rewind the file so that the pointer is at the first record. */
    ret = gsfSeek(handle, GSF_REWIND);
    if (ret) 
    {
        gsfError = GSF_FILE_SEEK_ERROR;
        return (-1);
    }

    for (i = 0; i < 100; i++) 
    {
        rec_size = gsfRead(handle, GSF_NEXT_RECORD, &id, &rec, NULL, 0);
        if (rec_size < 0) 
        {
            if (gsfError == GSF_READ_TO_END_OF_FILE) 
            {            
                ret = gsfSeek(handle, GSF_REWIND);
                if (ret) 
                {
                    gsfError = GSF_FILE_SEEK_ERROR;
                    return (-1);
                }
                return(0);
            }
            else
            {
                /* gsfError should already be set to indicate the type of failure that occurred */
                return (-1);
            }
        }
        
        switch (id.recordID)
        {
            case (GSF_RECORD_SWATH_BATHYMETRY_PING):
                if ((rec.mb_ping.brb_inten) && (rec.mb_ping.brb_inten->time_series))
                {
                    *status = 1;
                }
                mb_ping = 1;
                break;
        }
        if (mb_ping == 1) 
        {
            break;
        }
    }

    /* reset the file pointer to where it was when function was called */
    /* Rewind the file so that the pointer is at the first record. */
    ret = gsfSeek(handle, GSF_REWIND);
    if (ret) 
    {
        gsfError = GSF_FILE_SEEK_ERROR;
        return (-1);
    }

    return (0);
}

