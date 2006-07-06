/********************************************************************
 *
 * Module Name : GSF_ENC.C
 *
 * Author/Date : J. S. Byrne / 3 May 1994
 *
 * Description :
 *  This source file contains the gsf functions for encoding a gsf byte
 *   stream from host data structures.
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
 * jsb          10-17-94  Added support for Reson SeaBat data
 * jsb          03-10-95  Modified ping record to store dynamic depth
 *                        corrector and tide corrector.
 * jsb          11-13-95  Added Unique id for EM1000
 * jsb          12-22-95  Added nearest scaled integer rounding on latitude
 *                        longitude, roll, pitch, and heave.
 * hem          08-20-96  Added gsfEncodeSinglebeam; added EncodeSASSSpecific,
 *                        EncodeTypeIIISeaBeamSpecific, EncodeSeaMapSpecific,
 *                        EncodeEchotracSpecific, EncodeMGD77Specific,
 *                        EncodeBDBSpecific, & EncodeNOSHDBSpecific; changed
 *                        gsfEncodeComment so that it uses the length of the
 *                        comment specified in the record rather than using
 *                        strlen to find the length of the comment (to allow
 *                        nulls in the comment);
 * jsb          09-27-96  Added support for SeaBeam with amplitude data
 * jsb          03-24-97  Added support for gsfSeaBatIISpecific as a replacement
 *                        of gsfSeaBatSpecific for the Reson 900x series sonars.
 *                        Added gsfSeaBat8101Specific for the Reson 8101 series
 *                        sonar.
 * hem          07-23-97  Added code to gsfEncodeSwathBathySummary,
 *                        gsfEncodeSinglebeam, and
 *                        gsfEncodeSoundVelocityProfile to handle rounding
 *                        latitudes, longitudes, depths, etc. properly before
 *                        storage in the GSF file.
 * bac          10-27-97  Added EncodeSeaBeam2112Specific to support the Sea
 *                        Beam 2112/36 sonar.
 * dwc          1-9-98    Added EncodeElacMkIISpecific to support the Elac
 *                        Bottomchart MkII sonar.
 * jsb          09/28/98  Added gsfEncodeHVNavigationError. This change made
 *                        in responce to CRs: GSF-98-001, and GSF-98-002. Also
 *                        added support for horizontal error ping array subrecord
 *                        in responce to CR: GSF-98-003. Removed the computation of
 *                        error_sum from gsfEncodeSwathBathymetryPing, now the library
 *                        will write horizontal and vertical depth estimates for each
 *                        ping if the array pointers are non-null.
 * jsb          12/29/98  Added support for Simrad em3000 series sonar systems.
 * wkm          3-30-99   Added EncodeCmpSassSpecific to deal with CMP SASS data.
 * wkm          8-02-99   Updated EncodeCmpSassSpecific to include lntens (heave) with CMP SASS data.
 * bac          10-24-00  Updated EncodeEM3Specific to include data fields from updated
 *                        EM series runtime parameter datagram.
 * bac          07-18-01  Added support for the Reson 8100 series of sonars.  Also removed the useage
 *                        of C++ reserved words "class" and "operator".
 * bac          10-12-01  Added a new attitude record definition.  The attitude record provides
 *                        a method for logging full time-series attitude measurements in the GSF
 *                        file, instead of attitude samples only at ping time.  Each attitude
 *                        record contains arrays of attitude measurements for time, roll, pitch,
 *                        heave and heading.  The number of measurements is user-definable, but
 *                        because of the way in which measurement times are stored, a single
 *                        attitude record should never contain more than sixty seconds worth of
 *                        data.
 * jsb          01-19-02  Added support for Simrad EM120, and removed references to unsued variables.
 * bac          06-19-03  Added support for bathymetric receive beam time series intensity data (i.e., Simrad
 *                        "Seabed image" and Reson "snippets").  Inlcluded RWL updates of 12-19-02 for adding
 *                        sensor-specific singlebeam information to the MB sensor specific subrecords.
 * bac          12-28-04  Added support for Navisound singlebeam, EM3000D, EM3002, and EM3002D.  Fixed
 *                        encoding of 1-byte BRB intensity values.  Corrected the encode/decode of Reson
 *                        projector angle.  Added beam_spacing to the gsfReson8100Specific subrecord.
 * bac          06-28-06  Added support for EM121A data received via Kongsberg SIS, mapped to existing
 *                        EM3 series sensor specific data structure. Replaced references to long types
 *                        with int types, for compilation on 64-bit architectures.
 *
 * Classification : Unclassified
 *
 * References : DoDBL Generic Sensor Format Sept. 30, 1993
 *
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
#include "gsf_enc.h"

/* Global external data defined in this module */
extern int      gsfError;                               /* defined in gsf.c */

/* Function prototypes for this file */
static int      EncodeScaleFactors(unsigned char *sptr, gsfScaleFactors *sf);
static int      EncodeTwoByteArray(unsigned char *sptr, double *array, int num_beams, gsfScaleFactors * sf, int id);
static int      EncodeSignedTwoByteArray(unsigned char *sptr, double *array, int num_beams, gsfScaleFactors * sf, int id);
static int      EncodeByteArray(unsigned char *sptr, double *array, int num_beams, gsfScaleFactors * sf, int id);
static int      EncodeSignedByteArray(unsigned char *sptr, double *array, int num_beams, gsfScaleFactors * sf, int id);
static int      EncodeBeamFlagsArray(unsigned char *sptr, unsigned char *array, int num_beams);
static int      EncodeQualityFlagsArray(unsigned char *sptr, unsigned char *array, int num_beams);
static int      EncodeBRBIntensity(unsigned char *sptr, gsfBRBIntensity * idata, int num_beams, int sensor_id, int bytes_used);
static int      EncodeSeabeamSpecific(unsigned char *sptr, gsfSensorSpecific * sdata);
static int      EncodeEM12Specific(unsigned char *sptr, gsfSensorSpecific * sdata);
static int      EncodeEM100Specific(unsigned char *sptr, gsfSensorSpecific * sdata);
static int      EncodeEM950Specific(unsigned char *sptr, gsfSensorSpecific * sdata);
static int      EncodeEM1000Specific(unsigned char *sptr, gsfSensorSpecific * sdata);
static int      EncodeEM121ASpecific(unsigned char *sptr, gsfSensorSpecific * sdata);
static int      EncodeEM121Specific(unsigned char *sptr, gsfSensorSpecific * sdata);

#if 1
/* 3-30-99 wkm: obsolete */
static int      EncodeTypeIIISeaBeamSpecific(unsigned char *sptr, gsfSensorSpecific * sdata);
static int      EncodeSASSSpecific(unsigned char *sptr, gsfSensorSpecific * sdata);
#endif

static int      EncodeCmpSassSpecific(unsigned char *sptr, gsfSensorSpecific * sdata);

static int      EncodeSeaMapSpecific(unsigned char *sptr, gsfSensorSpecific * sdata);
static int      EncodeSeaBatSpecific(unsigned char *sptr, gsfSensorSpecific * sdata);
static int      EncodeEchotracSpecific(unsigned char *sptr, gsfSBSensorSpecific * sdata);
static int      EncodeMGD77Specific(unsigned char *sptr, gsfSBSensorSpecific * sdata);
static int      EncodeBDBSpecific(unsigned char *sptr, gsfSBSensorSpecific * sdata);
static int      EncodeNOSHDBSpecific(unsigned char *sptr, gsfSBSensorSpecific * sdata);
static int      EncodeSBAmpSpecific(unsigned char *sptr, gsfSensorSpecific * sdata);
static int      EncodeSeaBatIISpecific(unsigned char *sptr, gsfSensorSpecific * sdata);
static int      EncodeSeaBat8101Specific(unsigned char *sptr, gsfSensorSpecific * sdata);
static int      EncodeSeaBeam2112Specific(unsigned char *sptr, gsfSensorSpecific * sdata);
static int      EncodeElacMkIISpecific(unsigned char *sptr, gsfSensorSpecific * sdata);
static int      EncodeEM3Specific(unsigned char *sptr, gsfSensorSpecific * sdata, GSF_FILE_TABLE *ft);
static int      EncodeReson8100Specific(unsigned char *sptr, gsfSensorSpecific * sdata);
static int      EncodeSBEchotracSpecific(unsigned char *sptr, t_gsfSBEchotracSpecific * sdata);
static int      EncodeSBMGD77Specific(unsigned char *sptr, t_gsfSBMGD77Specific * sdata);
static int      EncodeSBBDBSpecific(unsigned char *sptr, t_gsfSBBDBSpecific * sdata);
static int      EncodeSBNOSHDBSpecific(unsigned char *sptr, t_gsfSBNOSHDBSpecific * sdata);
static int      EncodeSBNavisoundSpecific(unsigned char *sptr, t_gsfSBNavisoundSpecific * sdata);

/********************************************************************
 *
 * Function Name : gsfEncodeHeader
 *
 * Description : This function encodes a gsf header into external byte stream
 *  form from the passed gsfHeader structure.
 *
 * Inputs :
 *    sptr = a pointer to the unsigned char buffer to write into.
 *    header = a pointer to the gsfHeader structure to read from.
 *
 * Returns : This function returns the number of bytes encoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

int
gsfEncodeHeader(unsigned char *sptr, gsfHeader * header)
{
    unsigned char  *p = sptr;

    memset(header->version, 0, sizeof(header->version));
    sprintf(header->version, "%s", GSF_VERSION);
    memcpy(p, header->version, sizeof(gsfHeader));
    p += sizeof(gsfHeader);

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : gsfEncodeSwathBathySummary
 *
 * Description : This function encodes a gsf swath bathymetry summary record
 *  into external byte stream form from the passed gsfHeader structure.
 *
 * Inputs :
 *    sptr = a pointer to the unsigned char buffer to write into.
 *    header = a pointer to the gsfHeader structure to read from.
 *
 * Returns : This function returns the number of bytes encoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

int
gsfEncodeSwathBathySummary(unsigned char *sptr, gsfSwathBathySummary *sum)
{
    gsfuLong        ltemp;
    gsfsLong        signed_long;
    double          dtemp;
    unsigned char  *p = sptr;

    /* First 8 bytes contain the time of the first ping in the file */
    ltemp = htonl(sum->start_time.tv_sec);
    memcpy(p, &ltemp, 4);
    p += 4;

    ltemp = htonl(sum->start_time.tv_nsec);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* Next 8 bytes contain the time of the last ping in the file */
    ltemp = htonl(sum->end_time.tv_sec);
    memcpy(p, &ltemp, 4);
    p += 4;

    ltemp = htonl(sum->end_time.tv_nsec);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* Next four byte integer contains the minimum latitude. Round the scaled
     * quantity to the nearest whole integer.
     */
    dtemp = sum->min_latitude * 1.0e7;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    ltemp = htonl((gsfsLong) dtemp);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* Next four byte integer contains the minimum longitude. Round the scaled
     * quantity to the nearest whole integer.
     */
    dtemp = sum->min_longitude * 1.0e7;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    ltemp = htonl((gsfsLong) dtemp);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* Next four byte integer contains the maximum latitude. Round the scaled
     * quantity to the nearest whole integer.
     */
    dtemp = sum->max_latitude * 1.0e7;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    ltemp = htonl((gsfsLong) dtemp);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* Next four byte integer contains the maximum longitude. Round the scaled
     * quantity to the nearest whole integer.
     */
    dtemp = sum->max_longitude * 1.0e7;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    ltemp = htonl((gsfsLong) dtemp);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* Next four byte integer contains the minimum depth. Round the scaled
     * quantity to the nearest whole integer.
     */
    dtemp = sum->min_depth * 100.0;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    signed_long = htonl((gsfsLong) dtemp);
    memcpy(p, &signed_long, 4);
    p += 4;

    /* Next four byte integer contains the maximum depth. Round the scaled
     * quantity to the nearest whole integer.
     */
    dtemp = sum->max_depth * 100.0;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    signed_long = htonl((gsfsLong) dtemp);
    memcpy(p, &signed_long, 4);
    p += 4;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : EncodeEchotracSpecific
 *
 * Description : This function encodes the Bathy 2000 and echotrac
 *  sensor specific data from the HSPS source files.
 *
 * Inputs :
 *    sptr = a pointer to an unsigned char buffer to write into
 *    sdata = a pointer to a union of sensor specific data
 *
 * Returns : This function returns the number of bytes encoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
EncodeEchotracSpecific(unsigned char *sptr, gsfSBSensorSpecific * sdata)
{
    unsigned char  *p = sptr;
    gsfuShort       stemp;

    /* First two byte integer contains the navigation error */
    stemp = htons((gsfuShort) (sdata->gsfEchotracSpecific.navigation_error));
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next byte contains the most probable position source navigation */
    *p = (unsigned char) sdata->gsfEchotracSpecific.mpp_source;
    p += 1;

    /* Next byte contains the tide source */
    *p = (unsigned char) sdata->gsfEchotracSpecific.tide_source;
    p += 1;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : EncodeMGD77Specific
 *
 * Description : This function encodes the MGD77 fields
 * into an MGD77 record. The MGD77 Singlebeam is essentially
 * survey trackline data, and actual survey data can be retrieved
 * from the source.
 *
 * Inputs :
 *    sptr = a pointer to an unsigned char buffer to write into
 *    sdata = a pointer to a union of sensor specific data
 *
 * Returns : This function returns the number of bytes encoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
EncodeMGD77Specific(unsigned char *sptr, gsfSBSensorSpecific * sdata)
{
    unsigned char  *p = sptr;
    gsfuLong        ltemp;
    gsfuShort       stemp;
    double          dtemp;

    /* First two byte integer contains the time zone correction */
    stemp = htons((gsfuShort) (sdata->gsfMGD77Specific.time_zone_corr));
    memcpy(p, &stemp, 2);
    p += 2;

    /* The Next two byte integer contains how the navigation was obtained */
    stemp = htons((gsfuShort) (sdata->gsfMGD77Specific.position_type_code));
    memcpy(p, &stemp, 2);
    p += 2;

    /* The Next two byte integer contains on how the sound velocity
       correction was made */
    stemp = htons((gsfuShort) (sdata->gsfMGD77Specific.correction_code));
    memcpy(p, &stemp, 2);
    p += 2;

    /* The Next two byte integer contains how the bathymetry was obtained */
    stemp = htons((gsfuShort) (sdata->gsfMGD77Specific.bathy_type_code));
    memcpy(p, &stemp, 2);
    p += 2;

    /* The Next two byte integer contains the quality code for Nav */
    stemp = htons((gsfuShort) (sdata->gsfMGD77Specific.quality_code));
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next four byte integer contains the the two way travel time */
    dtemp = sdata->gsfMGD77Specific.travel_time * 10000;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    ltemp = htonl((gsfuLong) dtemp);
    memcpy(p, &ltemp, 4);
    p += 4;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : EncodeBDBSpecific
 *
 * Description : This function encodes the BDB fields
 * into a BDB record.
 *
 * Inputs :
 *    sptr = a pointer to an unsigned char buffer to write into
 *    sdata = a pointer to a union of sensor specific data
 *
 * Returns : This function returns the number of bytes encoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
EncodeBDBSpecific(unsigned char *sptr, gsfSBSensorSpecific * sdata)
{
    unsigned char  *p = sptr;
    gsfuLong        ltemp;

    /* Next four byte integer contains the the document number */
    ltemp = htonl((gsfuLong) (sdata->gsfBDBSpecific.doc_no));
    memcpy(p, &ltemp, 4);
    p += 4;

    /* Next byte contains the evaluation flag */
    *p = (unsigned char) sdata->gsfBDBSpecific.eval;
    p += 1;

    /* Next byte contains the classification flag */
    *p = (unsigned char) sdata->gsfBDBSpecific.classification;
    p += 1;

    /* Next byte contains the track adjustment flag */
    *p = (unsigned char) sdata->gsfBDBSpecific.track_adj_flag;
    p += 1;

    /* Next byte contains the source flag */
    *p = (unsigned char) sdata->gsfBDBSpecific.source_flag;
    p += 1;

    /* Next byte contains the discrete point or track line flag */
    *p = (unsigned char) sdata->gsfBDBSpecific.pt_or_track_ln;
    p += 1;

    /* Next byte contains the datum flag */
    *p = (unsigned char) sdata->gsfBDBSpecific.datum_flag;
    p += 1;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : EncodeNOSHDBSpecific
 *
 * Description : This function encodes the NOSHDB fields into a
 *               NOSHDB record.
 *
 * Inputs :
 *    sptr = a pointer to an unsigned char buffer to write into
 *    sdata = a pointer to a union of sensor specific data
 *
 * Returns : This function returns the number of bytes encoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
EncodeNOSHDBSpecific(unsigned char *sptr, gsfSBSensorSpecific * sdata)
{
    unsigned char  *p = sptr;
    gsfuShort       stemp;

    /* First two byte integer contains the depth type code */
    stemp = htons((gsfuShort) (sdata->gsfNOSHDBSpecific.type_code));
    memcpy(p, &stemp, 2);
    p += 2;

    /* The Next two byte integer contains the cartographic code */
    stemp = htons((gsfuShort) (sdata->gsfNOSHDBSpecific.carto_code));
    memcpy(p, &stemp, 2);
    p += 2;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : gsfEncodeSinglebeam
 *
 * Description : This function encodes a gsf single beam ping record
 *  in external byte stream form from a gsfSwathBathyPing structure.
 *
 * Inputs :
 *    sptr = a pointer to the unsigned char buffer to write into
 *    ping = a pointer to the gsfSwathBathPing record to read from
 *    handle = an integer handle to the gsf data file, used to track the
 *             number of beams.
 *    ft = a pointer to the gsfFileTable, where the scale factors are
 *         maintained.
 *
 * Returns :
 *  This function returns the number of bytes encoded if successful, or
 *  -1 if an error occured.
 *
 * Error Conditions :
 *    GSF_UNRECOGNIZED_SENSOR_ID
 *
 ********************************************************************/
int
gsfEncodeSinglebeam(unsigned char *sptr, gsfSingleBeamPing * ping)
{
    gsfuLong        ltemp;
    gsfuShort       stemp;
    int             sensor_size = 0;
    double          dtemp;
    unsigned char  *p = sptr;
    unsigned char  *temp_ptr;

    /* First 8 bytes contain the time */
    ltemp = htonl(ping->ping_time.tv_sec);
    memcpy(p, &ltemp, 4);
    p += 4;

    ltemp = htonl(ping->ping_time.tv_nsec);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* Next four byte integer contains the longitude. Round the scaled
     * quantity to the nearest whole integer.
     */
    dtemp = ping->longitude * 1.0e7;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    ltemp = htonl((gsfsLong) dtemp);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* Next four byte integer contains the latitude. Round the scaled
     * quantity to the nearest whole integer.
     */
    dtemp = ping->latitude * 1.0e7;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    ltemp = htonl((gsfsLong) dtemp);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* Next two byte integer contains the tide corrector for this ping.
     * Round this to the nearest whole centimeter.
     */
    dtemp = ping->tide_corrector * 100.0;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    stemp = htons((gsfsShort) dtemp);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next four byte integer contains the depth corrector.
     * Round this to the nearest whole centimeter.
     */
    dtemp = ping->depth_corrector * 100.0;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    ltemp = htonl((gsfsLong) dtemp);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* Next two byte integer contains the ship heading, round this value to the
     * nearest one hundredth of a degree.
     */
    stemp = htons((gsfuShort) ((ping->heading * 100.0) + 0.501));
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the pitch. Round the scaled
     * quantity to the nearest whole integer.
     */
    dtemp = ping->pitch * 100.0;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    stemp = htons((gsfsShort) dtemp);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the roll. Round the scaled
     * quantity to the nearest whole integer.
     */
    dtemp = ping->roll * 100.0;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    stemp = htons((gsfsShort) dtemp);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the heave. Round the scaled
     * quantity to the nearest whole integer.
     */
    dtemp = ping->heave * 100.0;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    stemp = htons((gsfsShort) dtemp);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next four byte integer contains the depth. Round the scaled
     * quantity to the nearest whole integer.
     */
    dtemp = ping->depth * 100.0;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    ltemp = htonl((gsfsLong) dtemp);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* Next two byte integer contains the sound speed correction.
     * Round the scaled quantity to the nearest whole integer.
     */
    dtemp = ping->sound_speed_correction * 100.0;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    stemp = htons((gsfsShort) dtemp);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next byte contains the positioning system type */
    stemp = htons((gsfuShort) (ping->positioning_system_type));
    memcpy(p, &stemp, 2);
    p += 2;


    /* Next possible subrecord is the sensor specific subrecord. Save the
    *  current pointer, and leave room for the four byte subrecord identifier.
    */
    temp_ptr = p;
    p += 4;

    switch (ping->sensor_id)
    {
        case (GSF_SINGLE_BEAM_SUBRECORD_ECHOTRAC_SPECIFIC):
            sensor_size = EncodeEchotracSpecific(p, &ping->sensor_data);
            break;

        case (GSF_SINGLE_BEAM_SUBRECORD_BATHY2000_SPECIFIC):
            sensor_size = EncodeEchotracSpecific(p, &ping->sensor_data);
            break;

        case (GSF_SINGLE_BEAM_SUBRECORD_MGD77_SPECIFIC):
            sensor_size = EncodeMGD77Specific(p, &ping->sensor_data);
            break;

        case (GSF_SINGLE_BEAM_SUBRECORD_BDB_SPECIFIC):
            sensor_size = EncodeBDBSpecific(p, &ping->sensor_data);
            break;

        case (GSF_SINGLE_BEAM_SUBRECORD_NOSHDB_SPECIFIC):
            sensor_size = EncodeNOSHDBSpecific(p, &ping->sensor_data);
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_UNKNOWN):
            sensor_size = 0;
            break;

          default:
            gsfError = GSF_UNRECOGNIZED_SENSOR_ID;
            return (-1);
    }

    /*  Identifier has sensor specific id in first byte, and size in the
    *  remaining three bytes
    */
    ltemp = ping->sensor_id << 24;
    ltemp |= (gsfuLong) sensor_size;
    ltemp = htonl(ltemp);
    memcpy(temp_ptr, &ltemp, 4);
    p += sensor_size;

    /* Return the number of byte written into the buffer */

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : gsfEncodeSwathBathymetryPing
 *
 * Description : This function encodes a gsf swath bathymetry ping record
 *  in external byte stream form from a gsfSwathBathyPing structure.
 *
 * Inputs :
 *    sptr = a pointer to the unsigned char buffer to write into
 *    ping = a pointer to the gsfSwathBathPing record to read from
 *    handle = an integer handle to the gsf data file, used to track the
 *             number of beams.
 *    ft = a pointer to the gsfFileTable, where the scale factors are
 *         maintained.
 *
 * Returns :
 *  This function returns the number of bytes encoded if successful, or
 *  -1 if an error occured.
 *
 * Error Conditions :
 *    GSF_UNRECOGNIZED_SENSOR_ID
 *
 ********************************************************************/
int
gsfEncodeSwathBathymetryPing(unsigned char *sptr, gsfSwathBathyPing * ping, GSF_FILE_TABLE *ft, int handle)
{
    gsfuLong        ltemp;
    gsfuShort       stemp;
    int             sensor_size = 0;
    int             ret;
    double          dtemp;
    unsigned char  *p = sptr;
    unsigned char  *temp_ptr;

    /* First 8 bytes contain the time */
    ltemp = htonl(ping->ping_time.tv_sec);
    memcpy(p, &ltemp, 4);
    p += 4;

    ltemp = htonl(ping->ping_time.tv_nsec);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* Next four byte integer contains the longitude. Round the scaled
     * quantity to the nearest whole integer.
     */
    dtemp = ping->longitude * 1.0e7;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    ltemp = htonl((gsfsLong) dtemp);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* Next four byte integer contains the latitude. Round the scaled
     * quantity to the nearest whole integer.
     */
    dtemp = ping->latitude * 1.0e7;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    ltemp = htonl((gsfsLong) dtemp);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* Next two byte integer contains the number of beams */
    stemp = htons(ping->number_beams);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the center beam number, portmost
    *  outer beam is beam number 1.
    */
    stemp = htons(ping->center_beam);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the ping flags field */
    stemp = htons(ping->ping_flags);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer is a reserved field */
    stemp = htons(ping->reserved);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the tide corrector for this ping.
     * Round this to the nearest whole centimeter.
     */
    dtemp = ping->tide_corrector * 100.0;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    stemp = htons((gsfsShort) dtemp);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next four byte integer contains the depth corrector.
     * Round this to the nearest whole centimeter.
     */
    dtemp = ping->depth_corrector * 100.0;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    ltemp = htonl((gsfsLong) dtemp);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* Next two byte integer contains the ship heading, round this value to the
     * nearest one hundredth of a degree. (Heading is always a positive value)
     */
    stemp = htons((gsfuShort) ((ping->heading * 100.0) + 0.501));
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the pitch. Round the scaled quantity
     * to the nearest whole integer.
     */
    dtemp = ping->pitch * 100.0;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    stemp = htons((gsfsShort) dtemp);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the roll. Round the scaled quantity
     * to the nearest whole integer.
     */
    dtemp = ping->roll * 100.0;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    stemp = htons((gsfsShort) dtemp);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the heave. Round the scaled quantity
     * to the nearest whole integer.
     */
    dtemp = ping->heave * 100.0;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    stemp = htons((gsfsShort) dtemp);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the course, round this value to the
     * nearest one hundredth of a degree. (Course is always a positive value)
     */
    stemp = htons((gsfuShort) ((ping->course * 100.0) + 0.501));
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the speed. Round the scaled quantity
     * to the nearest integer. (Speed is always a positive quantity)
     */
    stemp = htons((gsfuShort) ((ping->speed * 100.0) + 0.501));
    memcpy(p, &stemp, 2);
    p += 2;

    /* The first possible subrecord is the scale factors.  The scale factor
     * record is encoded for writing to the file once at the begining of the
     * file, and again whenever the scale factors change.
     */
    if ((memcmp(&ft->rec.mb_ping.scaleFactors, &ping->scaleFactors, sizeof(gsfScaleFactors))) ||
        (ft->scales_read))
    {
        memcpy(&ft->rec.mb_ping.scaleFactors, &ping->scaleFactors, sizeof(gsfScaleFactors));
        p += EncodeScaleFactors(p, &ping->scaleFactors);
        /* scales_read is set in gsfOpen if the file is opened create to ensure that scale factors
         * are written with the first ping of each file. Clear scales_read here.
         */
        ft->scales_read = 0;
    }

    /* Next possible subrecord is the depth array.  For this array to be
    * encoded there must be data, and a scale factor.
    */
    if (ping->depth != (double *) NULL)
    {
        ret = EncodeTwoByteArray(p, ping->depth, ping->number_beams,
            &ping->scaleFactors, GSF_SWATH_BATHY_SUBRECORD_DEPTH_ARRAY);
        if (ret <= 0)
        {
            return (-1);
        }
        p += ret;
    }

    /* Next possible subrecord is the nominal depth array.  For this array
     * to be encoded there must be data, and a scale factor.
    */
    if (ping->nominal_depth != (double *) NULL)
    {
        ret = EncodeTwoByteArray(p, ping->nominal_depth, ping->number_beams,
            &ping->scaleFactors, GSF_SWATH_BATHY_SUBRECORD_NOMINAL_DEPTH_ARRAY);
        if (ret <= 0)
        {
            return (-1);
        }
        p += ret;
    }

    /* Next possible subrecord is the across track distance array.  For this
    * array to be encoded there must be data, and a scale factor.
    */
    if (ping->across_track != (double *) NULL)
    {
        ret = EncodeSignedTwoByteArray(p, ping->across_track, ping->number_beams,
            &ping->scaleFactors, GSF_SWATH_BATHY_SUBRECORD_ACROSS_TRACK_ARRAY);
        if (ret <= 0)
        {
            return (-1);
        }
        p += ret;
    }

    /* Next possible subrecord is the along track distance array.  For this
    * array to be encoded there must be data, and a scale factor.
    */
    if (ping->along_track != (double *) NULL)
    {
        ret = EncodeSignedTwoByteArray(p, ping->along_track, ping->number_beams,
            &ping->scaleFactors, GSF_SWATH_BATHY_SUBRECORD_ALONG_TRACK_ARRAY);
        if (ret <= 0)
        {
            return (-1);
        }
        p += ret;
    }

    /* Next possible subrecord is the travel time array.  For this
    * array to be encoded there must be data, and a scale factor.
    */
    if (ping->travel_time != (double *) NULL)
    {
        ret = EncodeTwoByteArray(p, ping->travel_time, ping->number_beams,
            &ping->scaleFactors, GSF_SWATH_BATHY_SUBRECORD_TRAVEL_TIME_ARRAY);
        if (ret <= 0)
        {
            return (-1);
        }
        p += ret;
    }

    /* Next possible subrecord is the beam angle array. For this
    * array to be encoded there must be data, and a scale factor.
    */
    if (ping->beam_angle != (double *) NULL)
    {
        ret = EncodeSignedTwoByteArray(p, ping->beam_angle, ping->number_beams,
            &ping->scaleFactors, GSF_SWATH_BATHY_SUBRECORD_BEAM_ANGLE_ARRAY);
        if (ret <= 0)
        {
            return (-1);
        }
        p += ret;
    }

    /* Next possible subrecord is the mean, calibrated amplitude array. For
    * this array to be encoded there must be data, and a scale factor.
    */
    if (ping->mc_amplitude != (double *) NULL)
    {
        ret = EncodeSignedByteArray(p, ping->mc_amplitude, ping->number_beams,
            &ping->scaleFactors, GSF_SWATH_BATHY_SUBRECORD_MEAN_CAL_AMPLITUDE_ARRAY);
        if (ret <= 0)
        {
            return (-1);
        }
        p += ret;
    }

    /* Next possible subrecord is the mean, relative amplitude array.  For
    * this array to be encoded there must be data, and a scale factor.
    */
    if (ping->mr_amplitude != (double *) NULL)
    {
        ret = EncodeByteArray(p, ping->mr_amplitude, ping->number_beams,
            &ping->scaleFactors, GSF_SWATH_BATHY_SUBRECORD_MEAN_REL_AMPLITUDE_ARRAY);
        if (ret <= 0)
        {
            return (-1);
        }
        p += ret;
    }

    /* Next possible subrecord is the echo width array. For this
    * array to be encoded there must be data, and a scale factor.
    */
    if (ping->echo_width != (double *) NULL)
    {
        ret = EncodeByteArray(p, ping->echo_width, ping->number_beams,
            &ping->scaleFactors, GSF_SWATH_BATHY_SUBRECORD_ECHO_WIDTH_ARRAY);
        if (ret <= 0)
        {
            return (-1);
        }
        p += ret;
    }

    /* Next possible subrecord is the quality factor array.  For this
    * array to be encoded there must be data, and a scale factor.
    */
    if (ping->quality_factor != (double *) NULL)
    {
        ret = EncodeByteArray(p, ping->quality_factor, ping->number_beams,
            &ping->scaleFactors, GSF_SWATH_BATHY_SUBRECORD_QUALITY_FACTOR_ARRAY);
        if (ret <= 0)
        {
            return (-1);
        }
        p += ret;
    }

    /* Next possible subrecord is the array of ship heave at beam reception
    * time. For this array to be encoded there must be data, and a scale factor.
    */
    if (ping->receive_heave != (double *) NULL)
    {
        ret = EncodeSignedByteArray(p, ping->receive_heave, ping->number_beams,
            &ping->scaleFactors, GSF_SWATH_BATHY_SUBRECORD_RECEIVE_HEAVE_ARRAY);
        if (ret <= 0)
        {
            return (-1);
        }
        p += ret;
    }

    /* Next possible subrecord is the array of estimated depth errors.
    * For this array to be encoded there must be data, and a scale factor.
    * jsb 10/19/98 This subrecord is obsolete, it is replaced with vertical_error.
    */
    if (ping->depth_error != (double *) NULL)
    {
        ret = EncodeTwoByteArray(p, ping->depth_error, ping->number_beams,
            &ping->scaleFactors, GSF_SWATH_BATHY_SUBRECORD_DEPTH_ERROR_ARRAY);
        if (ret <= 0)
        {
            return (-1);
        }
        p += ret;
    }

    /* Next possible subrecord is the array of estimated across track errors.
    * For this array to be encoded there must be data, and a scale factor.
    * jsb 10/19/98 This subrecord is obsolete, it is replaced with horizontal_error.
    */
    if (ping->across_track_error != (double *) NULL)
    {
        ret = EncodeTwoByteArray(p, ping->across_track_error, ping->number_beams,
            &ping->scaleFactors, GSF_SWATH_BATHY_SUBRECORD_ACROSS_TRACK_ERROR_ARRAY);
        if (ret <= 0)
        {
            return (-1);
        }
        p += ret;
    }

    /* Next possible subrecord is the array of estimated along track errors.
    * For this array to be encoded there must be data, and a scale factor.
    * jsb 10/19/98 This subrecord is obsolete, it is replaced with horizontal_error.
    */
    if (ping->along_track_error != (double *) NULL)
    {
        ret = EncodeTwoByteArray(p, ping->along_track_error, ping->number_beams,
            &ping->scaleFactors, GSF_SWATH_BATHY_SUBRECORD_ALONG_TRACK_ERROR_ARRAY);
        if (ret <= 0)
        {
            return (-1);
        }
        p += ret;
    }

    /* Next possible subrecord is the array of beam status flags */
    if (ping->beam_flags != (unsigned char *) NULL)
    {
        ret = EncodeBeamFlagsArray(p, ping->beam_flags, ping->number_beams);
        if (ret <= 0)
        {
            return (-1);
        }
        p += ret;
    }

    /* Next possible subrecord is the beam quality flags provided by the
     * Reson SeaBat system.  There are two bits per beam.
     */
    if (ping->quality_flags != (unsigned char *) NULL)
    {
        ret = EncodeQualityFlagsArray(p, ping->quality_flags, ping->number_beams);
        if (ret <= 0)
        {
            return (-1);
        }
        p += ret;
    }

    /* Next possible subrecord is the array of signal to noise ratios.
    * For this array to be encoded there must be new data, and a scale factor.
    */
    if (ping->signal_to_noise != (double *) NULL)
    {
        ret = EncodeByteArray(p, ping->signal_to_noise, ping->number_beams,
            &ping->scaleFactors, GSF_SWATH_BATHY_SUBRECORD_SIGNAL_TO_NOISE_ARRAY);
        if (ret <= 0)
        {
            return (-1);
        }
        p += ret;
    }

    /* Next possible subrecord is the beam angle forward array. For this
    * array to be encoded there must be data, and a scale factor.
    */
    if (ping->beam_angle_forward != (double *) NULL)
    {
        ret = EncodeTwoByteArray(p, ping->beam_angle_forward, ping->number_beams,
            &ping->scaleFactors, GSF_SWATH_BATHY_SUBRECORD_BEAM_ANGLE_FORWARD_ARRAY);
        if (ret <= 0)
        {
            return (-1);
        }
        p += ret;
    }

    /* Next possible subrecord is the array of estimated vertical errors.
    * For this array to be encoded there must be data, and a scale factor.
    */
    if (ping->vertical_error != (double *) NULL)
    {
        ret = EncodeTwoByteArray(p, ping->vertical_error, ping->number_beams,
            &ping->scaleFactors, GSF_SWATH_BATHY_SUBRECORD_VERTICAL_ERROR_ARRAY);
        if (ret <= 0)
        {
            return (-1);
        }
        p += ret;
    }


    /* Next possible subrecord is the array of estimated horizontal errors.
    * For this array to be encoded there must be data, and a scale factor.
    */
    if (ping->horizontal_error != (double *) NULL)
    {
        ret = EncodeTwoByteArray(p, ping->horizontal_error, ping->number_beams,
            &ping->scaleFactors, GSF_SWATH_BATHY_SUBRECORD_HORIZONTAL_ERROR_ARRAY);
        if (ret <= 0)
        {
            return (-1);
        }
        p += ret;
    }

    /* Next possible subrecord is the sensor specific subrecord. Save the
    *  current pointer, and leave room for the four byte subrecord identifier.
    */
    temp_ptr = p;
    p += 4;

    switch (ping->sensor_id)
    {
        case (GSF_SWATH_BATHY_SUBRECORD_UNKNOWN):
            sensor_size = 0;
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_SEABEAM_SPECIFIC):
            sensor_size = EncodeSeabeamSpecific(p, &ping->sensor_data);
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_EM100_SPECIFIC):
            sensor_size = EncodeEM100Specific(p, &ping->sensor_data);
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_EM12_SPECIFIC):
            sensor_size = EncodeEM12Specific(p, &ping->sensor_data);
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_EM950_SPECIFIC):
            sensor_size = EncodeEM950Specific(p, &ping->sensor_data);
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_EM121A_SPECIFIC):
            sensor_size = EncodeEM121ASpecific(p, &ping->sensor_data);
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_EM121_SPECIFIC):
            sensor_size = EncodeEM121Specific(p, &ping->sensor_data);
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_SASS_SPECIFIC):
            sensor_size = EncodeSASSSpecific(p, &ping->sensor_data);
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_SEAMAP_SPECIFIC):
            sensor_size = EncodeSeaMapSpecific(p, &ping->sensor_data);
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_SEABAT_SPECIFIC):
            sensor_size = EncodeSeaBatSpecific(p, &ping->sensor_data);
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_EM1000_SPECIFIC):
            sensor_size = EncodeEM1000Specific(p, &ping->sensor_data);
            break;

            #if 1
            /* 3-30-99 wkm: obsolete */
        case (GSF_SWATH_BATHY_SUBRECORD_TYPEIII_SEABEAM_SPECIFIC):
            sensor_size = EncodeTypeIIISeaBeamSpecific(p, &ping->sensor_data);
            break;
            #endif

        case (GSF_SWATH_BATHY_SUBRECORD_SB_AMP_SPECIFIC):
            sensor_size = EncodeSBAmpSpecific(p, &ping->sensor_data);
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_SEABAT_II_SPECIFIC):
            sensor_size = EncodeSeaBatIISpecific(p, &ping->sensor_data);
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_SEABAT_8101_SPECIFIC):
            sensor_size = EncodeSeaBat8101Specific(p, &ping->sensor_data);
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_SEABEAM_2112_SPECIFIC):
            sensor_size = EncodeSeaBeam2112Specific(p, &ping->sensor_data);
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_ELAC_MKII_SPECIFIC):
            sensor_size = EncodeElacMkIISpecific(p, &ping->sensor_data);
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_CMP_SASS_SPECIFIC):
            sensor_size = EncodeCmpSassSpecific(p, &ping->sensor_data);
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_EM3000_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_EM1002_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_EM300_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_EM120_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_EM3002_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_EM3000D_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_EM3002D_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_EM121A_SIS_SPECIFIC):
            sensor_size = EncodeEM3Specific(p, &ping->sensor_data, ft);
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_RESON_8101_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_RESON_8111_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_RESON_8124_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_RESON_8125_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_RESON_8150_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_RESON_8160_SPECIFIC):
            sensor_size = EncodeReson8100Specific(p, &ping->sensor_data);
            break;

        case (GSF_SWATH_BATHY_SB_SUBRECORD_ECHOTRAC_SPECIFIC):
            sensor_size = EncodeSBEchotracSpecific(p, &ping->sensor_data.gsfSBEchotracSpecific);
            break;

        case (GSF_SWATH_BATHY_SB_SUBRECORD_BATHY2000_SPECIFIC):
            sensor_size = EncodeSBEchotracSpecific(p, &ping->sensor_data.gsfSBEchotracSpecific);
            break;

        case (GSF_SWATH_BATHY_SB_SUBRECORD_MGD77_SPECIFIC):
            sensor_size = EncodeSBMGD77Specific(p, &ping->sensor_data.gsfSBMGD77Specific);
            break;

        case (GSF_SWATH_BATHY_SB_SUBRECORD_BDB_SPECIFIC):
            sensor_size = EncodeSBBDBSpecific(p, &ping->sensor_data.gsfSBBDBSpecific);
            break;

        case (GSF_SWATH_BATHY_SB_SUBRECORD_NOSHDB_SPECIFIC):
            sensor_size = EncodeSBNOSHDBSpecific(p, &ping->sensor_data.gsfSBNOSHDBSpecific);
            break;

        case (GSF_SWATH_BATHY_SB_SUBRECORD_PDD_SPECIFIC):
            sensor_size = EncodeSBEchotracSpecific(p, &ping->sensor_data.gsfSBPDDSpecific);
            break;

        case (GSF_SWATH_BATHY_SB_SUBRECORD_NAVISOUND_SPECIFIC):
            sensor_size = EncodeSBNavisoundSpecific(p, &ping->sensor_data.gsfSBNavisoundSpecific);
            break;

        default:
            gsfError = GSF_UNRECOGNIZED_SENSOR_ID;
            return (-1);
    }

   /*  Identifier has sensor specific id in first byte, and size in the
    *  remaining three bytes
    */
    ltemp = ping->sensor_id << 24;
    ltemp |= (gsfuLong) sensor_size;
    ltemp = htonl(ltemp);
    memcpy(temp_ptr, &ltemp, 4);
    p += sensor_size;

    /* Next possible subrecord is the array of intensity series */
    if (ping->brb_inten != (gsfBRBIntensity *) NULL)
    {
        ret = EncodeBRBIntensity(p, ping->brb_inten, ping->number_beams, ping->sensor_id, p-sptr-12);  /* 12 = GSF_FILL_SIZE_CHECKSUM */
        if (ret <= 0)
        {
            return (-1);
        }
        p += ret;
    }

    /* Return the number of byte written into the buffer */

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : EncodeScaleFacors
 *
 * Description : This function encodes the ping scale factor subrecord
 *  from internal form into external byte stream stream form.
 *
 * Inputs :
 *    sptr = a pointer to the unsigned char buffer to write into
 *    handle = the integer handle for the file being written to.
 *
 * Returns :
 *  This function returns the number of bytes encoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
EncodeScaleFactors(unsigned char *sptr, gsfScaleFactors *sf)
{
    unsigned char  *p = sptr;
    gsfuLong        ltemp, subID;
    double          dtemp;
    int             subrecordID;

    /* First byte contains the subrecord identifier */
    ltemp = GSF_SWATH_BATHY_SUBRECORD_SCALE_FACTORS << 24;

    /* Next four bytes contain the size of the subrecord, need to
    * compute this value:
    *  4 byte number of scale factors
    * 12 bytes per number of scale factors
    */
    ltemp |= 4 + (12 * sf->numArraySubrecords);
    subID = htonl(ltemp);
    memcpy(p, &subID, 4);
    p += 4;

    /* Next four byte integer contains the number of scale factors */
    ltemp = htonl((gsfuShort) (sf->numArraySubrecords));
    memcpy(p, &ltemp, 4);
    p += 4;

    /* Loop to encode each scale factor which has been defined.
    * The loop counter i indexes through the known subrecord ids
    */
    for (subrecordID = 1; subrecordID <= GSF_MAX_PING_ARRAY_SUBRECORDS; subrecordID++)
    {
        if (sf->scaleTable[subrecordID - 1].multiplier)
        {
            /* First four byte integer has the id in the first byte, the
             *  compression flags in the second byte, and the two lower
             *  order bytes are reserved
             */
            ltemp = ((gsfuLong) subrecordID) << 24;     /* ID = loop counter */
            ltemp |= (((gsfuLong) sf->scaleTable[subrecordID - 1].compressionFlag) << 16);

            ltemp = htonl(ltemp);

            memcpy(p, &ltemp, 4);
            p += 4;

            /* encode the scale factor multiplier */
            dtemp = sf->scaleTable[subrecordID - 1].multiplier;
            ltemp = htonl((gsfsLong) dtemp);
            memcpy(p, &ltemp, 4);
            p += 4;

            /* encode the scale factor offset */
            ltemp = htonl((gsfsLong) (sf->scaleTable[subrecordID - 1].offset));
            memcpy(p, &ltemp, 4);
            p += 4;
        }
    }
    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : EncodeTwoByteArray
 *
 * Description : This function encodes a two byte beam array subrecord
 *  from internal form to external byte stream form.
 *
 * Inputs :
 *    sptr = a pointer to the unsigned char buffer to write to
 *    array = a pointer to the array of doubles from which to read
 *    num_beams = the integer number of beams (number of doubles in the array)
 *    sf = a pointer to the gsf scale factors used to scale the data
 *    id = the array subrecord id
 *
 * Returns :
 *  This function returns the number of bytes encoded if successful, or
 *  -1 if an error occured.
 *
 * Error Conditions :
 *    GSF_ILLEGAL_SCALE_FACTOR_MULTIPLIER
 *
 ********************************************************************/

static int
EncodeTwoByteArray(unsigned char *sptr, double *array, int num_beams,
    gsfScaleFactors *sf, int id)
{
    unsigned char  *ptr = sptr;
    gsfuLong        ltemp;
    gsfuShort       stemp1;
    gsfuShort       stemp2;
    double         *dptr;
    double          dtemp;
    int             i;


    /* Make sure we have a multiplier for this array */
    if (sf->scaleTable[id - 1].multiplier < 1.0e-6)
    {
        gsfError = GSF_ILLEGAL_SCALE_FACTOR_MULTIPLIER;
        return (-1);
    }

    /* subrecord identifier has array id in first byte, and size in the
    *  remaining three bytes
    */
    ltemp = id << 24;
    ltemp |= num_beams * 2;
    ltemp = htonl(ltemp);
    memcpy(ptr, &ltemp, 4);
    ptr += 4;

    dptr = array;
    for (i = 0; i < num_beams; i++)
    {
        dtemp = ((*dptr) + sf->scaleTable[id - 1].offset) *
            sf->scaleTable[id - 1].multiplier;

        /* Make sure we round to the nearest whole integer */
        if (dtemp >= 0.0)
        {
            dtemp += 0.501;
        }
        else
        {
            dtemp -= 0.501;
        }
        stemp1 = (gsfuShort) dtemp;
        stemp2 = htons(stemp1);
        memcpy(ptr, &stemp2, 2);
        ptr += 2;
        dptr++;
    }
    return (ptr - sptr);
}


/********************************************************************
 *
 * Function Name : EncodeSignedTwoByteArray
 *
 * Description : This function encodes a two byte beam array subrecord
 *  from internal form to external byte stream form.
 *
 * Inputs :
 *    sptr = a pointer to the unsigned char buffer to write to
 *    array = a pointer to the array of doubles from which to read
 *    num_beams = the integer number of beams (number of doubles in the array)
 *    sf = a pointer to the gsf scale factors used to scale the data
 *    id = the array subrecord id
 *
 * Returns :
 *  This function returns the number of bytes encoded if successful, or
 *  -1 if an error occured.
 *
 * Error Conditions :
 *    GSF_ILLEGAL_SCALE_FACTOR_MULTIPLIER
 *
 ********************************************************************/

static int
EncodeSignedTwoByteArray(unsigned char *sptr, double *array, int num_beams,
    gsfScaleFactors *sf, int id)
{
    unsigned char  *ptr = sptr;
    gsfuLong        ltemp;
    gsfsShort       stemp1;
    gsfsShort       stemp2;
    double         *dptr;
    double          dtemp;
    int             i;


    /* Make sure we have a multiplier for this array */
    if (sf->scaleTable[id - 1].multiplier < 1.0e-6)
    {
        gsfError = GSF_ILLEGAL_SCALE_FACTOR_MULTIPLIER;
        return (-1);
    }

    /* subrecord identifier has array id in first byte, and size in the
    *  remaining three bytes
    */
    ltemp = id << 24;
    ltemp |= num_beams * 2;
    ltemp = htonl(ltemp);
    memcpy(ptr, &ltemp, 4);
    ptr += 4;

    dptr = array;
    for (i = 0; i < num_beams; i++)
    {
        dtemp = ((*dptr) + sf->scaleTable[id - 1].offset) *
            sf->scaleTable[id - 1].multiplier;

        /* Make sure we round to the nearest whole integer */
        if (dtemp >= 0.0)
        {
            dtemp += 0.501;
        }
        else
        {
            dtemp -= 0.501;
        }
        stemp1 = (gsfsShort) dtemp;
        stemp2 = htons(stemp1);
        memcpy(ptr, &stemp2, 2);
        ptr += 2;
        dptr++;
    }
    return (ptr - sptr);
}

/********************************************************************
 *
 * Function Name : EncodeByteArray
 *
 * Description : This function encodes a byte beam array subrecord
 *  from internal form to external byte stream form.
 *
 * Inputs :
 *    sptr = a pointer to the unsigned char buffer to write to
 *    array = a pointer to the array of doubles from which to read
 *    num_beams = the integer number of beams (number of doubles in the array)
 *    sf = a pointer to the gsf scale factors used to scale the data
 *    id = the array subrecord id
 *
 * Returns :
 *  This function returns the number of bytes encoded if successful, or
 *  -1 if an error occured.
 *
 * Error Conditions :
 *    GSF_ILLEGAL_SCALE_FACTOR_MULTIPLIER
 *
 ********************************************************************/

static int
EncodeByteArray(unsigned char *sptr, double *array, int num_beams,
    gsfScaleFactors * sf, int id)
{
    unsigned char  *ptr = sptr;
    unsigned char   ctemp;
    gsfuLong        ltemp;
    double         *dptr;
    double          dtemp;
    int             i;


    /* Make sure we have a multiplier for this array */
    if (sf->scaleTable[id - 1].multiplier < 1.0e-6)
    {
        gsfError = GSF_ILLEGAL_SCALE_FACTOR_MULTIPLIER;
        return (-1);
    }

    /* subrecord identifier has array id in first byte, and size in the
    *  remaining three bytes
    */

    ltemp = id << 24;
    ltemp |= num_beams;
    ltemp = htonl(ltemp);
    memcpy(ptr, &ltemp, 4);
    ptr += 4;

    dptr = array;
    for (i = 0; i < num_beams; i++)
    {
        dtemp = ((*dptr) + sf->scaleTable[id - 1].offset) *
            sf->scaleTable[id - 1].multiplier;

        /* Make sure we round to the nearest whole integer */
        if (dtemp >= 0.0)
        {
            dtemp += 0.501;
        }
        else
        {
            dtemp -= 0.501;
        }

        ctemp = (unsigned char) dtemp;
        memcpy(ptr, &ctemp, 1);
        ptr++;
        dptr++;
    }
    return (ptr - sptr);
}

/********************************************************************
 *
 * Function Name : EncodeSignedByteArray
 *
 * Description : This function encodes a byte beam array subrecord
 *  from internal form to external byte stream form.
 *
 * Inputs :
 *    sptr = a pointer to the unsigned char buffer to write to
 *    array = a pointer to the array of doubles from which to read
 *    num_beams = the integer number of beams (number of doubles in the array)
 *    sf = a pointer to the gsf scale factors used to scale the data
 *    id = the array subrecord id
 *
 * Returns :
 *  This function returns the number of bytes encoded if successful, or
 *  -1 if an error occured.
 *
 * Error Conditions :
 *    GSF_ILLEGAL_SCALE_FACTOR_MULTIPLIER
 *
 ********************************************************************/

static int
EncodeSignedByteArray(unsigned char *sptr, double *array, int num_beams,
    gsfScaleFactors * sf, int id)
{
    unsigned char  *ptr = sptr;
    char   ctemp;
    gsfuLong        ltemp;
    double         *dptr;
    double          dtemp;
    int             i;


    /* Make sure we have a multiplier for this array */
    if (sf->scaleTable[id - 1].multiplier < 1.0e-6)
    {
        gsfError = GSF_ILLEGAL_SCALE_FACTOR_MULTIPLIER;
        return (-1);
    }

    /* subrecord identifier has array id in first byte, and size in the
    *  remaining three bytes
    */

    ltemp = id << 24;
    ltemp |= num_beams;
    ltemp = htonl(ltemp);
    memcpy(ptr, &ltemp, 4);
    ptr += 4;

    dptr = array;
    for (i = 0; i < num_beams; i++)
    {
        dtemp = ((*dptr) + sf->scaleTable[id - 1].offset) *
            sf->scaleTable[id - 1].multiplier;

        /* Make sure we round to the nearest whole integer */
        if (dtemp >= 0.0)
        {
            dtemp += 0.501;
        }
        else
        {
            dtemp -= 0.501;
        }

        ctemp = (char) dtemp;
        memcpy(ptr, &ctemp, 1);
        ptr++;
        dptr++;
    }
    return (ptr - sptr);
}

/********************************************************************
 *
 * Function Name : EncodeBeamFlagsArray
 *
 * Description : This function encodes the array of beam flags from internal
 *  form to external byte stream form.
 *
 * Inputs :
 *    sptr = a pointer to the unsigned char buffer to write to
 *    array = a pointer to the unsigned char buffer to read from
 *    num_beams = an integer containing the number of beams (elements in array)
 *
 * Returns : This function returns the number of bytes encoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
EncodeBeamFlagsArray(unsigned char *sptr, unsigned char *array, int num_beams)
{
    unsigned char  *ptr = sptr;
    gsfuLong        ltemp;
    int             i;

    /* subrecord identifier has array id in first byte, and size in the
    *  remaining three bytes
    */
    ltemp = GSF_SWATH_BATHY_SUBRECORD_BEAM_FLAGS_ARRAY << 24;
    ltemp |= num_beams;
    ltemp = htonl(ltemp);
    memcpy(ptr, &ltemp, 4);
    ptr += 4;

    for (i = 0; i < num_beams; i++)
    {
        *ptr = array[i];
        ptr++;
    }

    return (ptr - sptr);
}

/********************************************************************
 *
 * Function Name : EncodeQualityFlagsArray
 *
 * Description : This function encodes the array of beam detection
 *  quality flags for Reson SeaBat data from internal form to external
 *  byte stream form. This field only has two bits so it packed as two
 *  bits per beam.
 *
 * Inputs :
 *    sptr = a pointer to the unsigned char buffer to write to
 *    array = a pointer to the unsigned char buffer to read from
 *    num_beams = an integer containing the number of beams (elements in array)
 *
 * Returns : This function returns the number of bytes encoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
EncodeQualityFlagsArray(unsigned char *sptr, unsigned char *array, int num_beams)
{
    unsigned char  *ptr = sptr;
    gsfuLong        ltemp;
    int             i;
    int             shift;

    /* Leave four bytes free for the subrecord id and size */
    ptr += 4;

    /* Pack the array values */
    shift = 6;
    *ptr = 0;
    for (i = 0; i < num_beams; i++)
    {
        *ptr |= array[i] << shift;

        if (shift == 0)
        {
            ptr++;
            *ptr = 0;
            shift = 6;
        }
        else
        {
            shift -= 2;
        }
    }

    /* subrecord identifier has array id in first byte, and size in the
    *  remaining three bytes
    */
    ltemp = GSF_SWATH_BATHY_SUBRECORD_QUALITY_FLAGS_ARRAY << 24;
    ltemp |= (ptr - sptr - 4);
    ltemp = htonl(ltemp);
    memcpy(sptr, &ltemp, 4);

    return (ptr - sptr);
}

/********************************************************************
 *
 * Function Name : EncodeSeabeamSpecific
 *
 * Description : This function encodes the sensor specific data from
 *  a SeaBeam system into external byte stream form.
 *
 * Inputs :
 *    sptr = a pointer to an unsigned char to write into
 *    sdata = a pointer to a union of gsf sensor specific data
 *
 * Returns : This function returns the number of bytes encoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
EncodeSeabeamSpecific(unsigned char *sptr, gsfSensorSpecific * sdata)
{
    gsfuShort       stemp;

    stemp = htons((gsfuShort) sdata->gsfSeaBeamSpecific.EclipseTime);
    memcpy(sptr, &stemp, 2);

    return (2);
}

/********************************************************************
 *
 * Function Name : EncodeEM12Specific
 *
 * Description : Not Implimented yet
 *
 * Inputs :
 *    sptr = a pointer to an unsigned char buffer to write into
 *    sdata = a pointer to a union of sensor specific data
 *
 * Returns : This function returns the number of bytes encoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
EncodeEM12Specific(unsigned char *sptr, gsfSensorSpecific * sdata)
{
    return (0);
}

/********************************************************************
 *
 * Function Name : EncodeEm100Specific
 *
 * Description : This function encodes the EM100 sensor specific information
 *  into external byte stream form.
 *
 * Inputs :
 *    sptr = a pointer to an unsigned char buffer to write into
 *    sdata = a pointer to a union of sensor specific data
 *
 * Returns : This function returns the number of bytes encoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
EncodeEM100Specific(unsigned char *sptr, gsfSensorSpecific * sdata)
{
    unsigned char  *p = sptr;
    gsfsShort       signed_short;
    gsfuShort       stemp;
    double dtemp;

    /* First two byte integer contains the ship pitch */
    dtemp = sdata->gsfEM100Specific.ship_pitch * 100.0;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    signed_short = htons((gsfsShort) dtemp);
    memcpy(p, &signed_short, 2);
    p += 2;

    /* Next two byte integer contains the transducer pitch */
    dtemp = sdata->gsfEM100Specific.transducer_pitch * 100.0;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    signed_short = htons((gsfsShort) dtemp);
    memcpy(p, &signed_short, 2);
    p += 2;

    /* Next byte contains the sonar mode (from the em100 amplitude datagram) */
    *p = sdata->gsfEM100Specific.mode;
    p += 1;

    /* Next byte contains the power (from the em100 amplitude datagram) */
    *p = (char) sdata->gsfEM100Specific.power;
    p += 1;

    /* Next byte contains the attenuation (from the em100 amplitude datagram) */
    *p = (char) sdata->gsfEM100Specific.attenuation;
    p += 1;

    /* Next byte contains the tvg (from the em100 amplitude datagram) */
    *p = (char) sdata->gsfEM100Specific.tvg;
    p += 1;

    /* Next byte contains the pulse length from the em100 amplitude datagram) */
    *p = (char) sdata->gsfEM100Specific.pulse_length;
    p += 1;

    /* Next two byte integer contains the counter from the em100
    * amplitude datagram
    */
    stemp = htons((gsfuShort) (sdata->gsfEM100Specific.counter));
    memcpy(p, &stemp, 2);
    p += 2;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : EncodeEm950Specific
 *
 * Description :  This function encodes the EM950 sensor specific information
 *  into external byte stream form.
 *
 * Inputs :
 *    sptr = a pointer to an unsigned char buffer to write into
 *    sdata = a pointer to a union of sensor specific data
 *
 * Returns : This function returns the number of bytes encoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
EncodeEM950Specific(unsigned char *sptr, gsfSensorSpecific * sdata)
{
    unsigned char  *p = sptr;
    gsfuShort       stemp;
    gsfsShort       signed_short;
    double          dtemp;

    /* First two byte integer contains the ping number */
    stemp = htons((gsfuShort) (sdata->gsfEM950Specific.ping_number));
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next byte contains the sonar mode of operation */
    *p = (unsigned char) sdata->gsfEM950Specific.mode;
    p += 1;

    /* Next byte contains the ping quality factor*/
    *p = (unsigned char) sdata->gsfEM950Specific.ping_quality;
    p += 1;

    /* Next two byte integer contains the transducer pitch */
    dtemp = sdata->gsfEM950Specific.ship_pitch * 100.0;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    signed_short = htons((gsfsShort) dtemp);
    memcpy(p, &signed_short, 2);
    p += 2;

    /* Next two byte integer contains the transducer pitch */
    dtemp = sdata->gsfEM950Specific.transducer_pitch * 100.0;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    signed_short = htons((gsfsShort) dtemp);
    memcpy(p, &signed_short, 2);
    p += 2;

    /* Next two byte integer contains the sea surface sound speed * 10
     */
    dtemp = (sdata->gsfEM950Specific.surface_velocity * 10.0);
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    stemp = htons((gsfuShort) dtemp);
    memcpy(p, &stemp, 2);
    p += 2;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : EncodeEm1000Specific
 *
 * Description :  This function encodes the EM1000 sensor specific information
 *  into external byte stream form.
 *
 * Inputs :
 *    sptr = a pointer to an unsigned char buffer to write into
 *    sdata = a pointer to a union of sensor specific data
 *
 * Returns : This function returns the number of bytes encoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
EncodeEM1000Specific(unsigned char *sptr, gsfSensorSpecific * sdata)
{
    unsigned char  *p = sptr;
    gsfuShort       stemp;
    gsfsShort       signed_short;
    double          dtemp;

    /* First two byte integer contains the ping number */
    stemp = htons((gsfuShort) (sdata->gsfEM1000Specific.ping_number));
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next byte contains the sonar mode of operation */
    *p = (unsigned char) sdata->gsfEM1000Specific.mode;
    p += 1;

    /* Next byte contains the ping quality factor*/
    *p = (unsigned char) sdata->gsfEM1000Specific.ping_quality;
    p += 1;

    /* Next two byte integer contains the transducer pitch */
    dtemp = sdata->gsfEM1000Specific.ship_pitch * 100.0;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    signed_short = htons((gsfsShort) dtemp);
    memcpy(p, &signed_short, 2);
    p += 2;

    /* Next two byte integer contains the transducer pitch */
    dtemp = sdata->gsfEM1000Specific.transducer_pitch * 100.0;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    signed_short = htons((gsfsShort) dtemp);
    memcpy(p, &signed_short, 2);
    p += 2;

    /* Next two byte integer contains the sea surface sound speed * 10 */
    dtemp = (sdata->gsfEM1000Specific.surface_velocity * 10.0);
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    stemp = htons((gsfuShort) dtemp);
    memcpy(p, &stemp, 2);
    p += 2;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : EncodeEm121ASpecific
 *
 * Description : This function encodes the EM121A sensor specific
 *  information into external byte stream form.
 *
 * Inputs :
 *    sptr = a pointer to an unsigned char buffer to write into
 *    sdata = a pointer to a union of sensor specific data
 *
 * Returns : This function returns the number of bytes encoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
EncodeEM121ASpecific(unsigned char *sptr, gsfSensorSpecific *sdata)
{
    unsigned char  *p = sptr;
    gsfuShort       stemp;
    double          dtemp;

    /* First two byte integer contains the ping number */
    stemp = htons((gsfuShort) (sdata->gsfEM121ASpecific.ping_number));
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next byte contains the sonar mode of operation */
    *p = (unsigned char) sdata->gsfEM121ASpecific.mode;
    p += 1;

    /* Next byte contains the number of valid beams */
    *p = (unsigned char) sdata->gsfEM121ASpecific.valid_beams;
    p += 1;

    /* Next byte contains the transmit pulse length */
    *p = (unsigned char) sdata->gsfEM121ASpecific.pulse_length;
    p += 1;

    /* Next byte contains the sonar beam width */
    *p = (unsigned char) sdata->gsfEM121ASpecific.beam_width;
    p += 1;

    /* Next byte contains the transmit power level */
    *p = (unsigned char) sdata->gsfEM121ASpecific.tx_power;
    p += 1;

    /* Next byte contains the number of transmit channels NOT working */
    *p = (unsigned char) sdata->gsfEM121ASpecific.tx_status;
    p += 1;

    /* Next byte contains the number of receive channels NOT working */
    *p = (unsigned char) sdata->gsfEM121ASpecific.rx_status;
    p += 1;

    /* Next two byte integer contains the sea surface sound speed * 10
     */
    dtemp = (sdata->gsfEM121ASpecific.surface_velocity * 10.0);
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    stemp = htons((gsfuShort) dtemp);
    memcpy(p, &stemp, 2);
    p += 2;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : EncodeEm121Specific
 *
 * Description : This function encodes the EM121 sensor specific
 *  information into external byte stream form.
 *
 * Inputs :
 *    sptr = a pointer to an unsigned char buffer to write into
 *    sdata = a pointer to a union of sensor specific data
 *
 * Returns : This function returns the number of bytes encoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
EncodeEM121Specific(unsigned char *sptr, gsfSensorSpecific *sdata)
{
    unsigned char  *p = sptr;
    gsfuShort       stemp;
    double          dtemp;

    /* First two byte integer contains the ping number */
    stemp = htons((gsfuShort) (sdata->gsfEM121Specific.ping_number));
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next byte contains the sonar mode of operation */
    *p = (unsigned char) sdata->gsfEM121Specific.mode;
    p += 1;

    /* Next byte contains the number of valid beams */
    *p = (unsigned char) sdata->gsfEM121Specific.valid_beams;
    p += 1;

    /* Next byte contains the transmit pulse length */
    *p = (unsigned char) sdata->gsfEM121Specific.pulse_length;
    p += 1;

    /* Next byte contains the sonar beam width */
    *p = (unsigned char) sdata->gsfEM121Specific.beam_width;
    p += 1;

    /* Next byte contains the transmit power level */
    *p = (unsigned char) sdata->gsfEM121Specific.tx_power;
    p += 1;

    /* Next byte contains the number of transmit channels NOT working */
    *p = (unsigned char) sdata->gsfEM121Specific.tx_status;
    p += 1;

    /* Next byte contains the number of receive channels NOT working */
    *p = (unsigned char) sdata->gsfEM121Specific.rx_status;
    p += 1;

    /* Next two byte integer contains the sea surface sound speed * 10
     */
    dtemp = (sdata->gsfEM121Specific.surface_velocity * 10.0);
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    stemp = htons((gsfuShort) dtemp);
    memcpy(p, &stemp, 2);
    p += 2;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : EncodeCmpSassSpecific
 *
 * Description : This function encodes the Compressed SASS
 *               specific data record to external byte stream format.
 *
 * Inputs :
 *    sptr = a pointer to an unsigned char buffer to write into
 *    sdata = a pointer to a union of sensor specific data
 *
 * Returns : This function returns the number of bytes encoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
EncodeCmpSassSpecific(unsigned char *sptr, gsfSensorSpecific * sdata)
{
    unsigned char  *p = sptr;
    gsfuShort       stemp;
    double      dtemp;

    dtemp = (sdata->gsfCmpSassSpecific.lfreq * 10.0);
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    stemp = htons((gsfuShort) dtemp);
    memcpy(p, &stemp, 2);
    p += 2;

    dtemp = (sdata->gsfCmpSassSpecific.lntens * 10.0);
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    stemp = htons((gsfuShort) dtemp);
    memcpy(p, &stemp, 2);
    p += 2;

    return (p - sptr);
}

#if 1
/* 3-30-99 wkm: obsolete */
/********************************************************************
 *
 * Function Name : EncodeSASSSpecific
 *
 * Description : This function encodes the Typeiii SASS
 *               specific data record to external byte stream format.
 *
 * Inputs :
 *    sptr = a pointer to an unsigned char buffer to write into
 *    sdata = a pointer to a union of sensor specific data
 *
 * Returns : This function returns the number of bytes encoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
EncodeSASSSpecific(unsigned char *sptr, gsfSensorSpecific * sdata)
{
    unsigned char  *p = sptr;
    gsfuShort       stemp;

    /* First two byte integer contains the leftmost beam */
    stemp = htons((gsfuShort) (sdata->gsfSASSSpecific.leftmost_beam));
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the rightmost beam */
    stemp = htons((gsfuShort) (sdata->gsfSASSSpecific.rightmost_beam));
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the total number of beams */
    stemp = htons((gsfuShort) (sdata->gsfSASSSpecific.total_beams));
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the navigation mode */
    stemp = htons((gsfuShort) (sdata->gsfSASSSpecific.nav_mode));
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the ping number */
    stemp = htons((gsfuShort) (sdata->gsfSASSSpecific.ping_number));
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the mission number */
    stemp = htons((gsfuShort) (sdata->gsfSASSSpecific.mission_number));
    memcpy(p, &stemp, 2);
    p += 2;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : EncodeTypeIIISeaBeamSpecific
 *
 * Description : This function encodes the Type III Seabeam specific
 *               data record to external byte stream format.
 *
 * Inputs :
 *    sptr = a pointer to an unsigned char buffer to write into
 *    sdata = a pointer to a union of sensor specific data
 *
 * Returns : This function returns the number of bytes encoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
EncodeTypeIIISeaBeamSpecific(unsigned char *sptr, gsfSensorSpecific * sdata)
{
    unsigned char  *p = sptr;
    gsfuShort       stemp;

    /* First two byte integer contains the leftmost beam */
    stemp = htons((gsfuShort) (sdata->gsfTypeIIISeaBeamSpecific.leftmost_beam));
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the rightmost beam */
    stemp = htons((gsfuShort)(sdata->gsfTypeIIISeaBeamSpecific.rightmost_beam));
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the total number of beams */
    stemp = htons((gsfuShort)(sdata->gsfTypeIIISeaBeamSpecific.total_beams));
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the navigation mode */
    stemp = htons((gsfuShort)(sdata->gsfTypeIIISeaBeamSpecific.nav_mode));
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the ping number */
    stemp = htons((gsfuShort)(sdata->gsfTypeIIISeaBeamSpecific.ping_number));
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the mission number */
    stemp = htons((gsfuShort)(sdata->gsfTypeIIISeaBeamSpecific.mission_number));
    memcpy(p, &stemp, 2);
    p += 2;

    return (p - sptr);
}
#endif


/********************************************************************
 *
 * Function Name : EncodeSeaMapSpecific
 *
 * Description : not implimented yet
 *
 * Inputs :
 *    sptr = a pointer to an unsigned char buffer to write into
 *    sdata = a pointer to a union of sensor specific data
 *
 * Returns : This function returns the number of bytes encoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
EncodeSeaMapSpecific(unsigned char *sptr, gsfSensorSpecific * sdata)
{
    unsigned char  *p = sptr;
    gsfuShort       stemp;
    double          dtemp;

    dtemp = (sdata->gsfSeamapSpecific.portTransmitter[0] * 10.0);
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    stemp = htons((gsfuShort) dtemp);
    memcpy(p, &stemp, 2);
    p += 2;

    dtemp = (sdata->gsfSeamapSpecific.portTransmitter[1] * 10.0);
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    stemp = htons((gsfuShort) dtemp);
    memcpy(p, &stemp, 2);
    p += 2;

    dtemp = (sdata->gsfSeamapSpecific.stbdTransmitter[0] * 10.0);
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    stemp = htons((gsfuShort) dtemp);
    memcpy(p, &stemp, 2);
    p += 2;

    dtemp = (sdata->gsfSeamapSpecific.stbdTransmitter[1] * 10.0);
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    stemp = htons((gsfuShort) dtemp);
    memcpy(p, &stemp, 2);
    p += 2;

    dtemp = (sdata->gsfSeamapSpecific.portGain * 10.0);
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    stemp = htons((gsfuShort) dtemp);
    memcpy(p, &stemp, 2);
    p += 2;

    dtemp = (sdata->gsfSeamapSpecific.stbdGain * 10.0);
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    stemp = htons((gsfuShort) dtemp);
    memcpy(p, &stemp, 2);
    p += 2;

    dtemp = (sdata->gsfSeamapSpecific.portPulseLength * 10.0);
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    stemp = htons((gsfuShort) dtemp);
    memcpy(p, &stemp, 2);
    p += 2;

    dtemp = (sdata->gsfSeamapSpecific.stbdPulseLength * 10.0);
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    stemp = htons((gsfuShort) dtemp);
    memcpy(p, &stemp, 2);
    p += 2;

    dtemp = (sdata->gsfSeamapSpecific.pressureDepth * 10.0);
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    stemp = htons((gsfuShort) dtemp);
    memcpy(p, &stemp, 2);

    dtemp = (sdata->gsfSeamapSpecific.altitude * 10.0);
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    stemp = htons((gsfuShort) dtemp);
    memcpy(p, &stemp, 2);
    p += 2;

    dtemp = (sdata->gsfSeamapSpecific.temperature * 10.0);
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    stemp = htons((gsfuShort) dtemp);
    memcpy(p, &stemp, 2);
    p += 2;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : EncodeSeaBatSpecific
 *
 * Description : This function encodes the Reson SeaBat sensor specific
 *  information into external byte stream form.
 *
 * Inputs :
 *    sptr = a pointer to an unsigned char buffer to write into
 *    sdata = a pointer to a union of sensor specific data
 *
 * Returns : This function returns the number of bytes encoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
EncodeSeaBatSpecific(unsigned char *sptr, gsfSensorSpecific *sdata)
{
    unsigned char  *p = sptr;
    gsfuShort       stemp;
    double          dtemp;

    /* First two byte integer contains the ping number */
    stemp = htons((gsfuShort) sdata->gsfSeaBatSpecific.ping_number);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the sea surface sound speed * 10 */
    dtemp = sdata->gsfSeaBatSpecific.surface_velocity * 10.0;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    stemp = htons((gsfuShort) dtemp);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next byte contains the sonar mode of operation */
    *p = (unsigned char) sdata->gsfSeaBatSpecific.mode;
    p += 1;

    /* Next byte contains the sonar mode setting for this ping */
    *p = (unsigned char) sdata->gsfSeaBatSpecific.sonar_range;
    p += 1;

    /* Next byte contains the sonar transmit power for this ping */
    *p = (unsigned char) sdata->gsfSeaBatSpecific.transmit_power;
    p += 1;

    /* Next byte contains the sonar receive gain for this setting */
    *p = (unsigned char) sdata->gsfSeaBatSpecific.receive_gain;
    p += 1;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : EncodeSBAmpSpecific
 *
 * Description : This function encodes the Sea Beam with amplitude
 *  sensor specific information into external byte stream form.
 *
 * Inputs :
 *    sptr = a pointer to an unsigned char buffer to write into
 *    sdata = a pointer to a union of sensor specific data
 *
 * Returns : This function returns the number of bytes encoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
EncodeSBAmpSpecific(unsigned char *sptr, gsfSensorSpecific *sdata)
{
    unsigned char  *p = sptr;
    gsfuLong        ltemp;
    gsfuShort       stemp;

    /* First byte contains the hour from the Eclipse */
    *p = (unsigned char) sdata->gsfSBAmpSpecific.hour;
    p += 1;

    /* Next byte contains the minute from the Eclipse */
    *p = (unsigned char) sdata->gsfSBAmpSpecific.minute;
    p += 1;

    /* Next byte contains the second from the Eclipse */
    *p = (unsigned char) sdata->gsfSBAmpSpecific.second;
    p += 1;

    /* Next byte the hundredths of seconds from the Eclipse */
    *p = (unsigned char) sdata->gsfSBAmpSpecific.hundredths;
    p += 1;

    /* Next four byte integer contains the block number */
    ltemp = htonl((gsfuLong) sdata->gsfSBAmpSpecific.block_number);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* Next two byte integer contains the average gate depth */
    stemp = htons((gsfuShort) sdata->gsfSBAmpSpecific.avg_gate_depth);
    memcpy(p, &stemp, 2);
    p += 2;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : EncodeSeaBatIISpecific
 *
 * Description : This function encodes the Reson SeaBat II sensor specific
 *  information into external byte stream form.  The SeaBat II sensor
 *  specific data structure was defined to replace the gsfSeaBatSpecific
 *  data structure as of GSF version 1.04
 *
 * Inputs :
 *    sptr = a pointer to an unsigned char buffer to write into
 *    sdata = a pointer to a union of sensor specific data
 *
 * Returns : This function returns the number of bytes encoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
EncodeSeaBatIISpecific(unsigned char *sptr, gsfSensorSpecific *sdata)
{
    unsigned char  *p = sptr;
    gsfuShort       stemp;
    double          dtemp;

    /* First two byte integer contains the ping number */
    stemp = htons((gsfuShort) sdata->gsfSeaBatIISpecific.ping_number);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the sea surface sound speed * 10 */
    dtemp = sdata->gsfSeaBatIISpecific.surface_velocity * 10.0;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    stemp = htons((gsfuShort) dtemp);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the sonar mode of operation */
    stemp = htons((gsfuShort) sdata->gsfSeaBatIISpecific.mode);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the sonar range setting */
    stemp = htons((gsfuShort) sdata->gsfSeaBatIISpecific.sonar_range);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the power setting */
    stemp = htons((gsfuShort) sdata->gsfSeaBatIISpecific.transmit_power);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the gain setting */
    stemp = htons((gsfuShort) sdata->gsfSeaBatIISpecific.receive_gain);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next byte contains the fore/aft beam width */
    *p = (unsigned char) ((sdata->gsfSeaBatIISpecific.fore_aft_bw * 10.0) + 0.5);
    p += 1;

    /* Next byte contains the athwartships beam width */
    *p = (unsigned char) ((sdata->gsfSeaBatIISpecific.athwart_bw * 10.0) + 0.5);
    p += 1;

    /* Next four bytes are reserved as spare space. */
    *p = (unsigned char) (sdata->gsfSeaBatIISpecific.spare[0]);
    p += 1;
    *p = (unsigned char) (sdata->gsfSeaBatIISpecific.spare[1]);
    p += 1;
    *p = (unsigned char) (sdata->gsfSeaBatIISpecific.spare[2]);
    p += 1;
    *p = (unsigned char) (sdata->gsfSeaBatIISpecific.spare[3]);
    p += 1;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : EncodeSeaBat8101Specific
 *
 * Description : This function encodes the Reson SeaBat 8101 sensor specific
 *  information into external byte stream form.
 *
 * Inputs :
 *    sptr = a pointer to an unsigned char buffer to write into
 *    sdata = a pointer to a union of sensor specific data
 *
 * Returns : This function returns the number of bytes encoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
EncodeSeaBat8101Specific(unsigned char *sptr, gsfSensorSpecific *sdata)
{
    unsigned char  *p = sptr;
    gsfuShort       stemp;
    double          dtemp;

    /* First two byte integer contains the ping number */
    stemp = htons((gsfuShort) sdata->gsfSeaBat8101Specific.ping_number);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the sea surface sound speed * 10 */
    dtemp = sdata->gsfSeaBat8101Specific.surface_velocity * 10.0;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    stemp = htons((gsfuShort) dtemp);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the sonar mode of operation */
    stemp = htons((gsfuShort) sdata->gsfSeaBat8101Specific.mode);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the sonar range setting */
    stemp = htons((gsfuShort) sdata->gsfSeaBat8101Specific.range);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the power setting */
    stemp = htons((gsfuShort) sdata->gsfSeaBat8101Specific.power);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the gain setting */
    stemp = htons((gsfuShort) sdata->gsfSeaBat8101Specific.gain);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the pulse width, in microseconds */
    stemp = htons((gsfuShort) sdata->gsfSeaBat8101Specific.pulse_width);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next byte contains the tvg spreading coefficient * 4 */
    *p = (unsigned char) (sdata->gsfSeaBat8101Specific.tvg_spreading);
    p += 1;

    /* Next byte contains the tvg absorption coefficient */
    *p = (unsigned char) (sdata->gsfSeaBat8101Specific.tvg_absorption);
    p += 1;

    /* Next byte contains the fore/aft beam width */
    *p = (unsigned char) ((sdata->gsfSeaBat8101Specific.fore_aft_bw * 10.0) + 0.5);
    p += 1;

    /* Next byte contains the athwartships beam width */
    *p = (unsigned char) ((sdata->gsfSeaBat8101Specific.athwart_bw * 10.0) + 0.5);
    p += 1;

    /* Next two byte integer is reserved for future use, to store the range filter min */
    stemp = htons((gsfuShort) sdata->gsfSeaBat8101Specific.range_filt_min);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer is reserved for future use, to store the range filter max */
    stemp = htons((gsfuShort) sdata->gsfSeaBat8101Specific.range_filt_max);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer is reserved for future use, to store the depth filter min */
    stemp = htons((gsfuShort) sdata->gsfSeaBat8101Specific.depth_filt_min);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer is reserved for future use, to store the depth filter max */
    stemp = htons((gsfuShort) sdata->gsfSeaBat8101Specific.depth_filt_max);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next byte is reserved for future use, to store the projector type */
    *p = (unsigned char) (sdata->gsfSeaBat8101Specific.projector);
    p += 1;

    /* Next four bytes are reserved as spare space. */
    *p = (unsigned char) (sdata->gsfSeaBat8101Specific.spare[0]);
    p += 1;
    *p = (unsigned char) (sdata->gsfSeaBat8101Specific.spare[1]);
    p += 1;
    *p = (unsigned char) (sdata->gsfSeaBat8101Specific.spare[2]);
    p += 1;
    *p = (unsigned char) (sdata->gsfSeaBat8101Specific.spare[3]);
    p += 1;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : EncodeSeaBeam2112Specific
 *
 * Description : This function encodes the Sea Beam 2112/36 sensor specific
 *  information into external byte stream form.
 *
 * Inputs :
 *    sptr = a pointer to an unsigned char buffer to write into
 *    sdata = a pointer to a union of sensor specific data
 *
 * Returns : This function returns the number of bytes encoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
EncodeSeaBeam2112Specific(unsigned char *sptr, gsfSensorSpecific *sdata)
{
    unsigned char  *p = sptr;
    gsfuShort       stemp;
    double          dtemp;

    /* First byte contains the sonar mode of operation */
    *p = (unsigned char) (sdata->gsfSeaBeam2112Specific.mode);
    p += 1;

    /* Next two byte integer contains the surface sound velocity * 100 - 130000 */
    dtemp = (sdata->gsfSeaBeam2112Specific.surface_velocity * 100.0) - 130000;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    stemp = htons((gsfuShort) dtemp);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next byte contains the SSV source */
    *p = (unsigned char) (sdata->gsfSeaBeam2112Specific.ssv_source);
    p += 1;

    /* Next byte contains the ping gain */
    *p = (unsigned char) (sdata->gsfSeaBeam2112Specific.ping_gain);
    p += 1;

    /* Next byte contains the ping pulse width */
    *p = (unsigned char) (sdata->gsfSeaBeam2112Specific.pulse_width);
    p += 1;

    /* Next byte contains the transmitter attenuation */
    *p = (unsigned char) (sdata->gsfSeaBeam2112Specific.transmitter_attenuation);
    p += 1;

    /* Next byte contains the number of algorithms */
    *p = (unsigned char) (sdata->gsfSeaBeam2112Specific.number_algorithms);
    p += 1;

    /* Next 4 bytes contain the algorithm order */
    memcpy (p, (unsigned char *) (sdata->gsfSeaBeam2112Specific.algorithm_order), 4);
    p += 4;

    /* Next two bytes are reserved as spare space. */
    *p = (unsigned char) (sdata->gsfSeaBeam2112Specific.spare[0]);
    p += 1;
    *p = (unsigned char) (sdata->gsfSeaBeam2112Specific.spare[1]);
    p += 1;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : EncodeElacMkIISpecific
 *
 * Description : This function encodes the Elac Bottomchart MkII sensor specific
 *  information into external byte stream form.
 *
 * Inputs :
 *    sptr = a pointer to an unsigned char buffer to write into
 *    sdata = a pointer to a union of sensor specific data
 *
 * Returns : This function returns the number of bytes encoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
EncodeElacMkIISpecific(unsigned char *sptr, gsfSensorSpecific *sdata)
{
    unsigned char  *p = sptr;
    gsfuShort       stemp;

    /* First byte contains the sonar mode of operation */
    *p = (unsigned char) (sdata->gsfElacMkIISpecific.mode);
    p += 1;

    /* Next two byte integer contains the ping counter */
    stemp = htons((gsfuShort) sdata->gsfElacMkIISpecific.ping_num);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the surface sound velocity in m/s */
    stemp = htons((gsfuShort) sdata->gsfElacMkIISpecific.sound_vel);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the pulse length in 0.01 ms */
    stemp = htons((gsfuShort) sdata->gsfElacMkIISpecific.pulse_length);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next byte contains the starboard receiver gain in dB */
    *p = (unsigned char) (sdata->gsfElacMkIISpecific.receiver_gain_stbd);
    p += 1;

    /* Next byte contains the port receiver gain in dB */
    *p = (unsigned char) (sdata->gsfElacMkIISpecific.receiver_gain_port);
    p += 1;

    /* Next two byte integer is reserved for future use */
    stemp = htons((gsfuShort) sdata->gsfElacMkIISpecific.reserved);
    memcpy(p, &stemp, 2);
    p += 2;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : EncodeEM3Specific
 *
 * Description : This function encodes the Simrad EM3000 series sensor
 *  specific information into external byte stream form.
 *
 * Inputs :
 *    sptr = a pointer to an unsigned char buffer to write into
 *    sdata = a pointer to a union of sensor specific data.
 *    ft = a pointer to the gsf file table, this is used to determine
 *        is the run-time parameters should be written with this record.
 *
 * Returns : This function returns the number of bytes encoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
EncodeEM3Specific(unsigned char *sptr, gsfSensorSpecific *sdata, GSF_FILE_TABLE *ft)
{
    unsigned char  *p = sptr;
    gsfuShort       stemp;
    gsfuLong        ltemp;
    double          dtemp;
    int             run_time_id;

    /* Next two bytes contain the EM model number */
    stemp = htons((gsfuShort) sdata->gsfEM3Specific.model_number);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two bytes contain the ping number */
    stemp = htons((gsfuShort) sdata->gsfEM3Specific.ping_number);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two bytes contain the system 1 or 2 serial number */
    stemp = htons((gsfuShort) sdata->gsfEM3Specific.serial_number);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two bytes contain the sourface sound speed */
    dtemp = sdata->gsfEM3Specific.surface_velocity * 10.0;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    stemp = htons((gsfuShort) dtemp);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two bytes contain the transmit depth */
    dtemp = sdata->gsfEM3Specific.transducer_depth * 100.0;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    stemp = htons((gsfuShort) dtemp);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two bytes contain the maximum number of beams possible */
    stemp = htons((gsfuShort) sdata->gsfEM3Specific.valid_beams);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two bytes contain the sample rate */
    stemp = htons((gsfuShort) sdata->gsfEM3Specific.sample_rate);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two bytes contain the depth difference between sonar heads in the EM3000D */
    dtemp = sdata->gsfEM3Specific.depth_difference * 100.0;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    stemp = htons((gsfuShort) dtemp);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next byte contains the transducer depth offset multiplier */
    *p = (unsigned char) (sdata->gsfEM3Specific.offset_multiplier);
    p += 1;

    /* Encode and write the run-time paremeters if any of them have changed. */
    run_time_id = 1;

    /* JSB 3/31/99 Commented out the following code block.  For now we will encode all of the
     *  run-time parameter fields in the sensor specific sub-record for each ping, whether the
     *  values have been updated or not.  In a future release, we plan to support encoding this
     *  portion of the subrecord only when the values have been updated.  This can be done by
     *  using the model in place for the scale factors record.  We will need to support a
     *  "scales_read" flag for write after read access, and direct access back to the ping record
     *  with the updated run-time params prior to a direct access read.
     *
     * run_time_id = 0;
     * if (memcmp(&sdata->gsfEM3Specific.run_time[0], &ft->rec.mb_ping.sensor_data.gsfEM3Specific.run_time[0], sizeof(gsfEM3RunTime)))
     * {
     *     memcpy (&ft->rec.mb_ping.sensor_data.gsfEM3Specific.run_time[0], &sdata->gsfEM3Specific.run_time[0], sizeof(gsfEM3RunTime));
     *     run_time_id |= 0x00000001;
     * }
     *
     * If this is an em 3000 series dual head installation, then we'll need to save both sets of run time parameters when either set
     *  has been updated.
     *
     * if ((sdata->gsfEM3Specific.model_number == 3002) ||
     *    (sdata->gsfEM3Specific.model_number == 3003) ||
     *    (sdata->gsfEM3Specific.model_number == 3004) ||
     *    (sdata->gsfEM3Specific.model_number == 3005) ||
     *    (sdata->gsfEM3Specific.model_number == 3006) ||
     *    (sdata->gsfEM3Specific.model_number == 3007) ||
     *    (sdata->gsfEM3Specific.model_number == 3008))
     * {
     *     if ((memcmp(&sdata->gsfEM3Specific.run_time[0], &ft->rec.mb_ping.sensor_data.gsfEM3Specific.run_time[0], sizeof(gsfEM3RunTime))) ||
     *         (memcmp(&sdata->gsfEM3Specific.run_time[1], &ft->rec.mb_ping.sensor_data.gsfEM3Specific.run_time[1], sizeof(gsfEM3RunTime))))
     *     {
     *         memcpy (&ft->rec.mb_ping.sensor_data.gsfEM3Specific.run_time[0], &sdata->gsfEM3Specific.run_time[0], sizeof(gsfEM3RunTime));
     *         memcpy (&ft->rec.mb_ping.sensor_data.gsfEM3Specific.run_time[1], &sdata->gsfEM3Specific.run_time[1], sizeof(gsfEM3RunTime));
     *         run_time_id |= 0x00000001;
     *         run_time_id |= 0x00000002;
     *     }
     * }
     */

    /* The next four byte value specifies the existance of the run-time data strucutre */
    ltemp = htonl(run_time_id);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* If the first bit is set then this subrecord contains a new set of run-time parameters,
     *  for a single head system otherwise the run-time parameters have not changed.
     */
    if (run_time_id & 0x00000001)
    {
        /* The next two byte value contains the em model number */
        stemp = htons(sdata->gsfEM3Specific.run_time[0].model_number);
        memcpy(p, &stemp, 2);
        p += 2;

        /* First 8 bytes contain the time */
        ltemp = htonl(sdata->gsfEM3Specific.run_time[0].dg_time.tv_sec);
        memcpy(p, &ltemp, 4);
        p += 4;

        ltemp = htonl(sdata->gsfEM3Specific.run_time[0].dg_time.tv_nsec);
        memcpy(p, &ltemp, 4);
        p += 4;

        /* The next two byte value contains the sequential ping number */
        stemp = htons(sdata->gsfEM3Specific.run_time[0].ping_number);
        memcpy(p, &stemp, 2);
        p += 2;

        /* The next two byte value contains the sonar head serial number */
        stemp = htons(sdata->gsfEM3Specific.run_time[0].serial_number);
        memcpy(p, &stemp, 2);
        p += 2;

        /* The next four byte value contains the system status */
        ltemp = htonl(sdata->gsfEM3Specific.run_time[0].system_status);
        memcpy(p, &ltemp, 4);
        p += 4;

        /* The next one byte value contains the mode identifier */
        *p = (unsigned char) sdata->gsfEM3Specific.run_time[0].mode;
        p += 1;

        /* The next one byte value contains the filter identifier */
        *p = (unsigned char) sdata->gsfEM3Specific.run_time[0].filter_id;
        p += 1;

        /* The next two byte value contains the minimum depth */
        dtemp = sdata->gsfEM3Specific.run_time[0].min_depth;
        stemp = htons((gsfuShort) dtemp);
        memcpy(p, &stemp, 2);
        p += 2;

        /* The next two byte value contains the maximum depth */
        dtemp = sdata->gsfEM3Specific.run_time[0].max_depth;
        stemp = htons((gsfuShort) dtemp);
        memcpy(p, &stemp, 2);
        p += 2;

        /* The next two byte value contains the absorption coefficient */
        dtemp = sdata->gsfEM3Specific.run_time[0].absorption * 100.0;
        if (dtemp < 0.0)
        {
            dtemp -= 0.501;
        }
        else
        {
            dtemp += 0.501;
        }
        stemp = htons((gsfuShort) dtemp);
        memcpy(p, &stemp, 2);
        p += 2;

        /* The next two byte value contains the transmit pulse length */
        dtemp = sdata->gsfEM3Specific.run_time[0].pulse_length;
        stemp = htons((gsfuShort) dtemp);
        memcpy(p, &stemp, 2);
        p += 2;

        /* The next two byte value contains the transmit beam width */
        dtemp = sdata->gsfEM3Specific.run_time[0].transmit_beam_width * 10.0;
        if (dtemp < 0.0)
        {
            dtemp -= 0.501;
        }
        else
        {
            dtemp += 0.501;
        }
        stemp = htons((gsfuShort) dtemp);
        memcpy(p, &stemp, 2);
        p += 2;

        /* The next one byte value contains the transmit power reduction */
        *p = (unsigned char) sdata->gsfEM3Specific.run_time[0].power_reduction;
        p += 1;

        /* The next one byte value contains the receive beam width */
        *p = (unsigned char) (sdata->gsfEM3Specific.run_time[0].receive_beam_width * 10.0 + 0.501);
        p += 1;

        /* The next one byte value contains the receive band width in Hz. This value is
         *  provided by the sonar with a precision of 50 hz.
         */
        *p = sdata->gsfEM3Specific.run_time[0].receive_bandwidth / 50 + 0.501;
        p += 1;

        /* The next one byte value contains the receive gain */
        *p = sdata->gsfEM3Specific.run_time[0].receive_gain;
        p += 1;

        /* The next one byte value contains the TVG law cross-over angle */
        *p = sdata->gsfEM3Specific.run_time[0].cross_over_angle;
        p += 1;

        /* The next one byte value contains the source of the surface sound speed value */
        *p = sdata->gsfEM3Specific.run_time[0].ssv_source;
        p += 1;

        /* The next two byte value contains the maximum port swath width */
        stemp = htons(sdata->gsfEM3Specific.run_time[0].port_swath_width);
        memcpy(p, &stemp, 2);
        p += 2;

        /* The next one byte value contains the beam spacing value */
        *p = (unsigned char) sdata->gsfEM3Specific.run_time[0].beam_spacing;
        p += 1;

        /* The next one byte value contains the port coverage sector */
        *p = (unsigned char) sdata->gsfEM3Specific.run_time[0].port_coverage_sector;
        p += 1;

        /* The next one byte value contains the yaw and pitch stabilization mode */
        *p = (unsigned char) sdata->gsfEM3Specific.run_time[0].stabilization;
        p += 1;

        /* The next one byte value contains the starboard coverage sector */
        *p = (unsigned char) sdata->gsfEM3Specific.run_time[0].stbd_coverage_sector;
        p += 1;

        /* The next two byte value contains the maximum starboard swath width */
        stemp = htons(sdata->gsfEM3Specific.run_time[0].stbd_swath_width);
        memcpy(p, &stemp, 2);
        p += 2;

        /* The next one byte value contains the HiLo frequency absorption coefficient ratio */
        *p = (unsigned char) sdata->gsfEM3Specific.run_time[0].hilo_freq_absorp_ratio;
        p += 1;

        /* The next four bytes are reserved for future use */
        memset(p, 0, 4);
        p += 4;

        /* If the second bit is set then this subrecord contains a second set of new run-time
         *  for an em3000d series sonar system.
         */
        if (run_time_id & 0x00000002)
        {
            /* The next two byte value contains the em model number */
            stemp = htons(sdata->gsfEM3Specific.run_time[1].model_number);
            memcpy(p, &stemp, 2);
            p += 2;

            /* First 8 bytes contain the time */
            ltemp = htonl(sdata->gsfEM3Specific.run_time[1].dg_time.tv_sec);
            memcpy(p, &ltemp, 4);
            p += 4;

            ltemp = htonl(sdata->gsfEM3Specific.run_time[1].dg_time.tv_nsec);
            memcpy(p, &ltemp, 4);
            p += 4;

            /* The next two byte value contains the sequential ping number */
            stemp = htons(sdata->gsfEM3Specific.run_time[1].ping_number);
            memcpy(p, &stemp, 2);
            p += 2;

            /* The next two byte value contains the sonar head serial number */
            stemp = htons(sdata->gsfEM3Specific.run_time[1].serial_number);
            memcpy(p, &stemp, 2);
            p += 2;

            /* The next four byte value contains the system status */
            ltemp = htonl(sdata->gsfEM3Specific.run_time[1].system_status);
            memcpy(p, &ltemp, 4);
            p += 4;

            /* The next one byte value contains the mode identifier */
            *p = (unsigned char) sdata->gsfEM3Specific.run_time[1].mode;
            p += 1;

            /* The next one byte value contains the filter identifier */
            *p = (unsigned char) sdata->gsfEM3Specific.run_time[1].filter_id;
            p += 1;

            /* The next two byte value contains the minimum depth */
            dtemp = sdata->gsfEM3Specific.run_time[1].min_depth;
            stemp = htons((gsfuShort) dtemp);
            memcpy(p, &stemp, 2);
            p += 2;

            /* The next two byte value contains the maximum depth */
            dtemp = sdata->gsfEM3Specific.run_time[1].max_depth;
            stemp = htons((gsfuShort) dtemp);
            memcpy(p, &stemp, 2);
            p += 2;

            /* The next two byte value contains the absorption coefficient */
            dtemp = sdata->gsfEM3Specific.run_time[1].absorption * 100.0;
            if (dtemp < 0.0)
            {
                dtemp -= 0.501;
            }
            else
            {
                dtemp += 0.501;
            }
            stemp = htons((gsfuShort) dtemp);
            memcpy(p, &stemp, 2);
            p += 2;

            /* The next two byte value contains the transmit pulse length */
            dtemp = sdata->gsfEM3Specific.run_time[1].pulse_length;
            stemp = htons((gsfuShort) dtemp);
            memcpy(p, &stemp, 2);
            p += 2;

            /* The next two byte value contains the transmit beam width */
            dtemp = sdata->gsfEM3Specific.run_time[1].transmit_beam_width * 10.0;
            if (dtemp < 0.0)
            {
                dtemp -= 0.501;
            }
            else
            {
                dtemp += 0.501;
            }
            stemp = htons((gsfuShort) dtemp);
            memcpy(p, &stemp, 2);
            p += 2;

            /* The next one byte value contains the transmit power reduction */
            *p = (unsigned char) sdata->gsfEM3Specific.run_time[1].power_reduction;
            p += 1;

            /* The next one byte value contains the receive beam width */
            *p = (unsigned char) (sdata->gsfEM3Specific.run_time[1].receive_beam_width * 10.0 + 0.501);
            p += 1;

            /* The next one byte value contains the receive band width in Hz. This value is
             *  provided by the sonar with a precision of 50 hz.
             */
            *p = sdata->gsfEM3Specific.run_time[1].receive_bandwidth / 50 + 0.501;
            p += 1;

            /* The next one byte value contains the receive gain */
            *p = sdata->gsfEM3Specific.run_time[1].receive_gain;
            p += 1;

            /* The next one byte value contains the TVG law cross-over angle */
            *p = sdata->gsfEM3Specific.run_time[1].cross_over_angle;
            p += 1;

            /* The next one byte value contains the source of the surface sound speed value */
            *p = sdata->gsfEM3Specific.run_time[1].ssv_source;
            p += 1;

            /* The next two byte value contains the maximum port swath width */
            stemp = htons(sdata->gsfEM3Specific.run_time[1].port_swath_width);
            memcpy(p, &stemp, 2);
            p += 2;

            /* The next one byte value contains the beam spacing value */
            *p = (unsigned char) sdata->gsfEM3Specific.run_time[1].beam_spacing;
            p += 1;

            /* The next one byte value contains the port coverage sector */
            *p = (unsigned char) sdata->gsfEM3Specific.run_time[1].port_coverage_sector;
            p += 1;

            /* The next one byte value contains the yaw and pitch stabilization mode */
            *p = (unsigned char) sdata->gsfEM3Specific.run_time[1].stabilization;
            p += 1;

            /* The next one byte value contains the starboard coverage sector */
            *p = (unsigned char) sdata->gsfEM3Specific.run_time[1].stbd_coverage_sector;
            p += 1;

            /* The next two byte value contains the maximum starboard swath width */
            stemp = htons(sdata->gsfEM3Specific.run_time[1].stbd_swath_width);
            memcpy(p, &stemp, 2);
            p += 2;

            /* The next one byte value contains the HiLo frequency absorption coefficient ratio */
            *p = (unsigned char) sdata->gsfEM3Specific.run_time[1].hilo_freq_absorp_ratio;
            p += 1;

            /* The next four bytes are reserved for future use */
            memset(p, 0, 4);
            p += 4;
         }
    }

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : EncodeReson8100Specific
 *
 * Description : This function encodes the Reson 8100 sensor specific
 *  information into external byte stream form.
 *
 * Inputs :
 *    sptr = a pointer to an unsigned char buffer to write into
 *    sdata = a pointer to a union of sensor specific data
 *
 * Returns : This function returns the number of bytes encoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
EncodeReson8100Specific(unsigned char *sptr, gsfSensorSpecific *sdata)
{
    unsigned char  *p = sptr;
    gsfuShort       stemp;
    gsfuLong        ltemp;
    double          dtemp;

    /* First two byte integer contains the sonar latency */
    stemp = htons((gsfuShort) sdata->gsfReson8100Specific.latency);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next four byte integer contains the ping number */
    ltemp = htonl((gsfuLong) sdata->gsfReson8100Specific.ping_number);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* Next four byte integer contains the sonar id */
    ltemp = htonl((gsfuLong) sdata->gsfReson8100Specific.sonar_id);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* Next two byte integer contains the sonar model */
    stemp = htons((gsfuShort) sdata->gsfReson8100Specific.sonar_model);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the sonar frequency */
    stemp = htons((gsfuShort) sdata->gsfReson8100Specific.frequency);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the sea surface sound speed * 10 */
    dtemp = sdata->gsfReson8100Specific.surface_velocity * 10.0;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    stemp = htons((gsfuShort) dtemp);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the sample rate */
    stemp = htons((gsfuShort) sdata->gsfReson8100Specific.sample_rate);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the ping rate */
    stemp = htons((gsfuShort) sdata->gsfReson8100Specific.ping_rate);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the sonar mode of operation */
    stemp = htons((gsfuShort) sdata->gsfReson8100Specific.mode);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the sonar range setting */
    stemp = htons((gsfuShort) sdata->gsfReson8100Specific.range);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the power setting */
    stemp = htons((gsfuShort) sdata->gsfReson8100Specific.power);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the gain setting */
    stemp = htons((gsfuShort) sdata->gsfReson8100Specific.gain);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the pulse width, in microseconds */
    stemp = htons((gsfuShort) sdata->gsfReson8100Specific.pulse_width);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next byte contains the tvg spreading coefficient * 4 */
    *p = (unsigned char) (sdata->gsfReson8100Specific.tvg_spreading);
    p += 1;

    /* Next byte contains the tvg absorption coefficient */
    *p = (unsigned char) (sdata->gsfReson8100Specific.tvg_absorption);
    p += 1;

    /* Next byte contains the fore/aft beam width */
    *p = (unsigned char) ((sdata->gsfReson8100Specific.fore_aft_bw * 10.0) + 0.501);
    p += 1;

    /* Next byte contains the athwartships beam width */
    *p = (unsigned char) ((sdata->gsfReson8100Specific.athwart_bw * 10.0) + 0.501);
    p += 1;

    /* Next byte contains the projector type */
    *p = (unsigned char) (sdata->gsfReson8100Specific.projector_type);
    p += 1;

    /* Next two byte integer contains the projector angle, in deg * 100 */
    stemp = htons((gsfuShort) sdata->gsfReson8100Specific.projector_angle);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the range filter min */
    stemp = htons((gsfuShort) sdata->gsfReson8100Specific.range_filt_min);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the range filter max */
    stemp = htons((gsfuShort) sdata->gsfReson8100Specific.range_filt_max);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the depth filter min */
    stemp = htons((gsfuShort) sdata->gsfReson8100Specific.depth_filt_min);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the depth filter max */
    stemp = htons((gsfuShort) sdata->gsfReson8100Specific.depth_filt_max);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next byte contains the filters active flags */
    *p = (unsigned char) (sdata->gsfReson8100Specific.filters_active);
    p += 1;

    /* Next two byte integer contains the temperature at the sonar head */
    stemp = htons((gsfuShort) sdata->gsfReson8100Specific.temperature);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two byte integer contains the across track angular beam spacing * 10000 */
    dtemp = sdata->gsfReson8100Specific.beam_spacing * 10000.0;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    stemp = htons((gsfuShort) dtemp);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two bytes are reserved as spare space. */
    *p = (unsigned char) (sdata->gsfReson8100Specific.spare[0]);
    p += 1;
    *p = (unsigned char) (sdata->gsfReson8100Specific.spare[1]);
    p += 1;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : EncodeSBEchotracSpecific
 *
 * Description : This function encodes the Bathy 2000 and echotrac
 *  sensor specific data from the HSPS source files.
 *
 * Inputs :
 *    sptr = a pointer to an unsigned char buffer to write into
 *    sdata = a pointer to a union of sensor specific data
 *    major_version = the major version of GSF used to create the file.
 *    minor_version = the minor version of GSF used to create the file.
 *
 * Returns : This function returns the number of bytes encoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
EncodeSBEchotracSpecific(unsigned char *sptr, t_gsfSBEchotracSpecific * sdata)
{
    unsigned char  *p = sptr;
    gsfuShort       stemp;
    double          dtemp;

    /* First two byte integer contains the navigation error */
    stemp = htons((gsfuShort) (sdata->navigation_error));
    memcpy(p, &stemp, 2);
    p += 2;

        /* Next byte contains the most probable position source navigation */
    *p = (unsigned char) sdata->mpp_source;
    p += 1;

    /* Next byte contains the tide source */
    *p = (unsigned char) sdata->tide_source;
    p += 1;

    /* Next two byte integer contains the dynamic draft. Round the scaled
     * quantity to the nearest whole integer.
     */
    dtemp = sdata->dynamic_draft * 100.0;
    if (dtemp < 0.0)
    {
            dtemp -= 0.501;
    }
    else
    {
            dtemp += 0.501;
    }
    stemp = htons((gsfsShort) dtemp);
    memcpy(p, &stemp, 2);
    p += 2;

    /* write the spare bytes */
    memcpy(p, sdata->spare, 4);
    p += 4;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : EncodeSBMGD77Specific
 *
 * Description : This function encodes the MGD77 fields
 * into an MGD77 record. The MGD77 Singlebeam is essentially
 * survey trackline data, and actual survey data can be retrieved
 * from the source.
 *
 * Inputs :
 *    sptr = a pointer to an unsigned char buffer to write into
 *    sdata = a pointer to a union of sensor specific data
 *
 * Returns : This function returns the number of bytes encoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
EncodeSBMGD77Specific(unsigned char *sptr, t_gsfSBMGD77Specific * sdata)
{
    unsigned char  *p = sptr;
    gsfuLong        ltemp;
    gsfuShort       stemp;
    double          dtemp;

    /* First two byte integer contains the time zone correction */
    stemp = htons((gsfuShort) (sdata->time_zone_corr));
    memcpy(p, &stemp, 2);
    p += 2;

    /* The Next two byte integer contains how the navigation was obtained */
    stemp = htons((gsfuShort) (sdata->position_type_code));
    memcpy(p, &stemp, 2);
    p += 2;

    /* The Next two byte integer contains on how the sound velocity
       correction was made */
    stemp = htons((gsfuShort) (sdata->correction_code));
    memcpy(p, &stemp, 2);
    p += 2;

    /* The Next two byte integer contains how the bathymetry was obtained */
    stemp = htons((gsfuShort) (sdata->bathy_type_code));
    memcpy(p, &stemp, 2);
    p += 2;

    /* The Next two byte integer contains the quality code for Nav */
    stemp = htons((gsfuShort) (sdata->quality_code));
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next four byte integer contains the the two way travel time */
    dtemp = sdata->travel_time * 10000;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    ltemp = htonl((gsfuLong) dtemp);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* write the spare bytes */
    memcpy(p, sdata->spare, 4);
    p += 4;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : EncodeSBBDBSpecific
 *
 * Description : This function encodes the BDB fields
 * into a BDB record.
 *
 * Inputs :
 *    sptr = a pointer to an unsigned char buffer to write into
 *    sdata = a pointer to a union of sensor specific data
 *
 * Returns : This function returns the number of bytes encoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
EncodeSBBDBSpecific(unsigned char *sptr, t_gsfSBBDBSpecific * sdata)
{
    unsigned char  *p = sptr;
    gsfuLong        ltemp;

    /* Next four byte integer contains the the document number */
    ltemp = htonl((gsfuLong) (sdata->doc_no));
    memcpy(p, &ltemp, 4);
    p += 4;

    /* Next byte contains the evaluation flag */
    *p = (unsigned char) sdata->eval;
    p += 1;

    /* Next byte contains the classification flag */
    *p = (unsigned char) sdata->classification;
    p += 1;

    /* Next byte contains the track adjustment flag */
    *p = (unsigned char) sdata->track_adj_flag;
    p += 1;

    /* Next byte contains the source flag */
    *p = (unsigned char) sdata->source_flag;
    p += 1;

    /* Next byte contains the discrete point or track line flag */
    *p = (unsigned char) sdata->pt_or_track_ln;
    p += 1;

    /* Next byte contains the datum flag */
    *p = (unsigned char) sdata->datum_flag;
    p += 1;

    /* write the spare bytes */
    memcpy(p, sdata->spare, 4);
    p += 4;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : EncodeSBNOSHDBSpecific
 *
 * Description : This function encodes the NOSHDB fields into a
 *               NOSHDB record.
 *
 * Inputs :
 *    sptr = a pointer to an unsigned char buffer to write into
 *    sdata = a pointer to a union of sensor specific data
 *
 * Returns : This function returns the number of bytes encoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
EncodeSBNOSHDBSpecific(unsigned char *sptr, t_gsfSBNOSHDBSpecific * sdata)
{
    unsigned char  *p = sptr;
    gsfuShort       stemp;

    /* First two byte integer contains the depth type code */
    stemp = htons((gsfuShort) (sdata->type_code));
    memcpy(p, &stemp, 2);
    p += 2;

    /* The Next two byte integer contains the cartographic code */
    stemp = htons((gsfuShort) (sdata->carto_code));
    memcpy(p, &stemp, 2);
    p += 2;

    /* write the spare bytes */
    memcpy(p, sdata->spare, 4);
    p += 4;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : EncodeSBNavisoundSpecific
 *
 * Description : This function encodes the Navisound
 *  sensor specific data
 *
 * Inputs :
 *    sptr = a pointer to an unsigned char buffer to write into
 *    sdata = a pointer to a union of sensor specific data
 *    major_version = the major version of GSF used to create the file.
 *    minor_version = the minor version of GSF used to create the file.
 *
 * Returns : This function returns the number of bytes encoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
EncodeSBNavisoundSpecific(unsigned char *sptr, t_gsfSBNavisoundSpecific * sdata)
{
    unsigned char  *p = sptr;
    gsfuShort       stemp;
    double          dtemp;

    /* First two byte value contains the pulse length */
    dtemp = sdata->pulse_length * 100.0;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    stemp = htons((gsfuShort) dtemp);
    memcpy(p, &stemp, 2);
    p += 2;

    /* write the spare bytes */
    memcpy(p, sdata->spare, 8);
    p += 8;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : EncodeEM3ImagerySpecific
 *
 * Description : This function encodes the Simrad EM3000 series sensor
 *  specific imagery information into external byte stream form.
 *
 * Inputs :
 *    sptr = a pointer to an unsigned char buffer to write into
 *    sdata = a pointer to a union of sensor specific imagery data.
 *
 * Returns : This function returns the number of bytes encoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
EncodeEM3ImagerySpecific(unsigned char *sptr, gsfSensorImagery *sdata)
{
    unsigned char  *p = sptr;
    gsfuShort       stemp;
    double          dtemp;

    /* Next two bytes contain the range to normal incidence to correct amplitudes */
    stemp = htons((gsfuShort) sdata->gsfEM3ImagerySpecific.range_norm);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two bytes contain the start range sample of TVG ramp */
    stemp = htons((gsfuShort) sdata->gsfEM3ImagerySpecific.start_tvg_ramp);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next two bytes contain the stop range sample of TVG ramp */
    stemp = htons((gsfuShort) sdata->gsfEM3ImagerySpecific.stop_tvg_ramp);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next byte contains the normal incidence BS in dB */
    *p = (unsigned char) (sdata->gsfEM3ImagerySpecific.bsn);
    p += 1;

    /* Next byte contains the oblique BS in dB */
    *p = (unsigned char) (sdata->gsfEM3ImagerySpecific.bso);
    p += 1;

    /* The next two byte value contains the mean absorption coefficient */
    dtemp = sdata->gsfEM3ImagerySpecific.mean_absorption * 100.0;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    stemp = htons((gsfuShort) dtemp);
    memcpy(p, &stemp, 2);
    p += 2;

    /* write the spare bytes */
    memcpy(p, sdata->gsfEM3ImagerySpecific.spare, 8);
    p += 8;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : EncodeReson8100ImagerySpecific
 *
 * Description : This function encodes the Reson 8100 series sensor
 *  specific imagery information into external byte stream form.
 *
 * Inputs :
 *    sptr = a pointer to an unsigned char buffer to write into
 *    sdata = a pointer to a union of sensor specific imagery data.
 *
 * Returns : This function returns the number of bytes encoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
EncodeReson8100ImagerySpecific(unsigned char *sptr, gsfSensorImagery *sdata)
{
    unsigned char  *p = sptr;

    /* write the spare bytes */
    memcpy(p, sdata->gsfReson8100ImagerySpecific.spare, 8);
    p += 8;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : EncodeBRBIntensity
 *
 * Description : This function encodes the Bathymetric Receive Beam
 *  time series intensity information into external byte stream form.
 *
 * Inputs :
 *    sptr = a pointer to an unsigned char buffer to write into
 *    idata = a pointer to the gsfBRBIntensity structure containg
 *            imagery data
 *    num_beams = the integer number of beams (number of
 *                gsfTimeSeriesIntensity structures in the array)
 *    sensor_id = the sensor specific subrecord id
 *
 * Returns : This function returns the number of bytes encoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
EncodeBRBIntensity(unsigned char *sptr, gsfBRBIntensity *idata, int num_beams, int sensor_id, int bytes_used)
{
    unsigned char  *ptr = sptr;
    unsigned char  *temp_ptr;
    gsfuLong        ltemp;
    gsfuShort       stemp;
    int             i, j;
    int             size;
    int             sensor_size;
    int             bytes_per_sample;
    unsigned char   bytes_to_pack[4];

    /* check to make sure the bits per sample value is supported */
    if ((idata->bits_per_sample != 8) &&
        (idata->bits_per_sample != 12) &&
        (idata->bits_per_sample != 16) &&
        (idata->bits_per_sample != 32))
    {
        /* unsupported number of bits per sample, return an error */
        gsfError = GSF_MB_PING_RECORD_ENCODE_FAILED;
        return -1;
    }

    /* Save the current pointer, and leave room for the four byte subrecord identifier. */
    temp_ptr = ptr;
    ptr += 4;

    /* write the bits per sample */
    *ptr = idata->bits_per_sample;
    ptr += 1;

    /* write the sample applied corrections description */
    ltemp = htonl(idata->applied_corrections);
    memcpy(ptr, &ltemp, 4);
    ptr += 4;

    /* write the spare header bytes */
    memcpy(ptr, idata->spare, 16);
    ptr += 16;

    /* write the sensor specifiuc imagery info */
    switch (sensor_id)
    {
        case (GSF_SWATH_BATHY_SUBRECORD_EM3000_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_EM1002_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_EM300_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_EM120_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_EM3002_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_EM3000D_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_EM3002D_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_EM121A_SIS_SPECIFIC):
            sensor_size = EncodeEM3ImagerySpecific(ptr, &idata->sensor_imagery);
            break;

        case (GSF_SWATH_BATHY_SUBRECORD_RESON_8101_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_RESON_8111_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_RESON_8124_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_RESON_8125_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_RESON_8150_SPECIFIC):
        case (GSF_SWATH_BATHY_SUBRECORD_RESON_8160_SPECIFIC):
            sensor_size = EncodeReson8100ImagerySpecific(ptr, &idata->sensor_imagery);
            break;

        default:
            sensor_size = 0;
            break;
    }
    ptr += sensor_size;

    bytes_per_sample = idata->bits_per_sample / 8;
    for (i = 0; i < num_beams; i++)
    {

        /* check to see if the GSF_MAX_RECORD_SIZE will get exceeded */
        if (12 + idata->time_series[i].sample_count * idata->bits_per_sample / 8  + bytes_used + ptr - sptr > GSF_MAX_RECORD_SIZE)
        {
            /* the GSF_MAX_RECORD_SIZE would be exceeded. return an error */
            gsfError = GSF_RECORD_SIZE_ERROR;
            return -1;
        }

        stemp = htons ((gsfuShort) idata->time_series[i].sample_count);
        memcpy(ptr, &stemp, 2);
        ptr += 2;

        stemp = htons ((gsfuShort) idata->time_series[i].detect_sample);
        memcpy(ptr, &stemp, 2);
        ptr += 2;

        memset(ptr, 0, 8);
        ptr += 8;

        if (idata->bits_per_sample == 12)
        {
            for (j = 0; j < idata->time_series[i].sample_count; j+=2)
            {
                /* pack 2 samples into 3 bytes */

                /* pack the first sample */
                ltemp = htonl ((gsfuLong) idata->time_series[i].samples[j]);
                memcpy (bytes_to_pack, &ltemp, 4);
                ptr[0]  = bytes_to_pack[2];
                ptr[1]  = bytes_to_pack[3];
                if (j+1 < idata->time_series[i].sample_count)
                {
                    /* pack the second sample */
                    ltemp = htonl ((gsfuLong) idata->time_series[i].samples[j+1]);
                    memcpy (bytes_to_pack, &ltemp, 4);
                    ptr[1] |= bytes_to_pack[2] >> 4;
                    ptr[2] = bytes_to_pack[2] << 4;
                    ptr[2] |= bytes_to_pack[3] >> 4;
                }
                else
                {
                    ptr[2] = 0;
                }
                ptr += 3;
            }
        }
        else
        {
            for (j=0; j < idata->time_series[i].sample_count; j++)
            {
                if (bytes_per_sample == 1)
                {
                    *ptr = (unsigned char) idata->time_series[i].samples[j];
                    ptr++;;
                }
                else if (bytes_per_sample == 2)
                {
                    stemp = htons ((gsfuShort) idata->time_series[i].samples[j]);
                    memcpy(ptr, &stemp, 2);
                    ptr += 2;
                }
                else if (bytes_per_sample == 4)
                {
                    ltemp = htonl ((gsfuLong) idata->time_series[i].samples[j]);
                    memcpy(ptr, &ltemp, 4);
                    ptr += 4;
                }
                else
                {
                    memcpy (ptr, &idata->time_series[i].samples[j], bytes_per_sample);
                    ptr += bytes_per_sample;
                }
            }
        }
    }

    /* subrecord identifier has array id in first byte, and size in the
    *  remaining three bytes
    */
    size = ptr - sptr;
    ltemp = GSF_SWATH_BATHY_SUBRECORD_INTENSITY_SERIES_ARRAY << 24;
    ltemp |= size;
    ltemp = htonl(ltemp);
    memcpy(temp_ptr, &ltemp, 4);

    return (ptr - sptr);
}

/********************************************************************
 *
 * Function Name : gsfEncodeSoundVelocityProfile
 *
 * Description : This function encodes a gsf sound velocity profile record
 *  from internal form to external byte stream form.
 *
 * Inputs :
 *    sptr = a pointer to an unsigned char buffer to be written to
 *    svp = a pointer to the gsfSVP structure to read from
 *
 * Returns : This function returns the number of bytes encoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

int
gsfEncodeSoundVelocityProfile(unsigned char *sptr, gsfSVP * svp)
{
    unsigned char  *p = sptr;
    double          dtemp;
    gsfuLong        ltemp;
    gsfsLong        signed_int;
    int             i;

    /* First four byte integer contains the observation time seconds */
    ltemp = htonl(svp->observation_time.tv_sec);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* Next four byte integer contains the observation time nanoseconds */
    ltemp = htonl(svp->observation_time.tv_nsec);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* Next four byte integer contains the seconds portion of the time the
    *  new profile was put into use by the sonar system
    */
    ltemp = htonl(svp->application_time.tv_sec);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* Next four byte integer contains the application time nanoseconds */
    ltemp = htonl(svp->application_time.tv_nsec);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* Next four byte integer contains the longitude of profile observation.
     * Round the scaled quantity to the nearest whole integer.
     */
    dtemp = svp->longitude * 1.0e7;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    signed_int = htonl((gsfsLong) dtemp);
    memcpy(p, &signed_int, 4);
    p += 4;

    /* Next four byte integer contains the latitude. Round the scaled
     * quantity to the nearest whole integer.
     */
    dtemp = svp->latitude * 1.0e7;
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    signed_int = htonl((gsfsLong) dtemp);
    memcpy(p, &signed_int, 4);
    p += 4;

    /* Next four byte integer contains the number of points in the profile */
    ltemp = htonl((gsfuLong) (svp->number_points));
    memcpy(p, &ltemp, 4);
    p += 4;

    /* Now loop to encode the depth/sound speed pairs
    * Scale both the depth and sound speed by 100
    */
    for (i = 0; i < svp->number_points; i++)
    {
        /* Next four byte integer contains the depth. Round the scaled
         * quantity to the nearest whole integer.
         */
        dtemp = svp->depth[i] * 100.0;
        if (dtemp < 0.0)
        {
            dtemp -= 0.501;
        }
        else
        {
            dtemp += 0.501;
        }
        ltemp = htonl((gsfuLong) dtemp);
        memcpy(p, &ltemp, 4);
        p += 4;

        /* Next four byte integer contains the sound_speed. Round the scaled
         * quantity to the nearest whole integer.
         */
        dtemp = svp->sound_speed[i] * 100.0;
        if (dtemp < 0.0)
        {
            dtemp -= 0.501;
        }
        else
        {
            dtemp += 0.501;
        }
        ltemp = htonl((gsfuLong) dtemp);
        memcpy(p, &ltemp, 4);
        p += 4;
    }

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : gsfEncodeProcessingParameters
 *
 * Description : This function encodes into external gsf byte stream form
 *  a record of processing parameters.
 *
 * Inputs :
 *    sptr = a pointer to an unsigned char buffer to write to
 *    param = a pointer to the gsfProcessingParameters structure to read from
 *
 * Returns : This function returns the number of bytes encoded
 *
 * Error Conditions : none
 *
 ********************************************************************/

int
gsfEncodeProcessingParameters(unsigned char *sptr, gsfProcessingParameters * param)
{
    unsigned char  *p = sptr;
    gsfuLong        ltemp;
    gsfuShort       stemp;
    short           len;
    int             i;

    /* First four byte integer contains the seconds portion of the time
    *  application of the new parameters.
    */
    ltemp = htonl(param->param_time.tv_sec);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* Next four byte integer contains the application time nanoseconds */
    ltemp = htonl(param->param_time.tv_nsec);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* Next two byte integer contains the number of parameters in this record */
    stemp = htons(param->number_parameters);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Now loop to encode these parameters */
    for (i = 0; i < param->number_parameters; i++)
    {
        /* next is the size of the parameter name as a two byte integer */
        len = strlen(param->param[i]) + 1;       /* add one to carry null */
        if (len != param->param_size[i])
        {
            param->param_size[i] = len;
        }
        stemp = htons(param->param_size[i]);
        memcpy(p, &stemp, 2);
        p += 2;
        memcpy(p, param->param[i], param->param_size[i]);
        p += param->param_size[i];
    }
    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : gsfEncodeSensorParameters
 *
 * Description : This function encodes into external gsf byte stream form
 *  a record of sonar parameters.
 *
 * Inputs :
 *    sptr = a pointer to an unsigned char buffer to write to
 *    param = a pointer to a gsfSensorParameters structure to read from
 *
 * Returns : This function returns the number of bytes encoded
 *
 * Error Conditions : none
 *
 ********************************************************************/

int
gsfEncodeSensorParameters(unsigned char *sptr, gsfSensorParameters * param)
{
    unsigned char  *p = sptr;
    gsfuLong        ltemp;
    gsfuShort       stemp;
    short           len;
    int             i;

    /* First four byte integer contains the seconds portion of the time
    *  application of the new parameters.
    */
    ltemp = htonl(param->param_time.tv_sec);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* Next four byte integer contains the application time nanoseconds */
    ltemp = htonl(param->param_time.tv_nsec);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* Next two byte integer contains the number of parameters in this record */
    stemp = htons(param->number_parameters);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Now loop to encode these parameters */
    for (i = 0; i < param->number_parameters; i++)
    {
        /* next is the size of the parameter name as a two byte integer */
        len = strlen(param->param[i]) + 1;     /* add one to carry null */
        if (len != param->param_size[i])
        {
            param->param_size[i] = len;
        }
        stemp = htons(param->param_size[i]);
        memcpy(p, &stemp, 2);
        p += 2;
        memcpy(p, param->param[i], param->param_size[i]);
        p += param->param_size[i];
    }
    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : gsfEncodeComment
 *
 * Description : This function is used to encode a gsfComment record
 *  from internal to external byte stream form.
 *
 * Inputs :
 *    sptr = a pointer to an unsigned char buffer to be written to
 *    comment = a pointer to the gsfComment structure to read from
 *
 * Returns : This function returns the number of bytes encoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

int
gsfEncodeComment(unsigned char *sptr, gsfComment * comment)
{
    unsigned char  *p = sptr;
    gsfuLong        ltemp;

    /* First four byte integer contains the seconds portion of the time
    *  the operator comment was made.
    */
    ltemp = htonl(comment->comment_time.tv_sec);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* Next four byte integer contains the nanoseconds portion of the
    * comment time
    */
    ltemp = htonl(comment->comment_time.tv_nsec);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* Next four byte integer contains the length of the comment */
    ltemp = htonl(comment->comment_length);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* Next "length" bytes contain the operator comment */
    memcpy(p, comment->comment, comment->comment_length);
    p += comment->comment_length;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : gsfEncodeHistory
 *
 * Description : This function encodes a gsf history record from internal
 *  to external byte stream form.
 *
 * Inputs :
 *    sptr = a pointer to an unsigned char buffer to write to
 *    history = a pointer to a gsfHistory structure to read from
 *
 * Returns : This function returns the number of bytes encoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

int
gsfEncodeHistory(unsigned char *sptr, gsfHistory * history)
{
    unsigned char  *p = sptr;
    int             len;
    gsfuLong        ltemp;
    gsfuShort       stemp;

    /* First four byte integer contains the seconds portion of the time
    *  the history record was added to the data.
    */
    ltemp = htonl(history->history_time.tv_sec);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* Next four byte integer contains the nanoseconds portion of the
    * history time.
    */
    ltemp = htonl(history->history_time.tv_nsec);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* Next 2 bytes contains the size of the machines host name */
    len = strlen(history->host_name) + 1;
    stemp = htons(len);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next "len" bytes contains the host name of the machine used to process the data */
    memcpy(p, history->host_name, len);
    p += len;

    /* Next two byte integer contains the size of the of operator field  */
    len = strlen(history->operator_name) + 1;
    stemp = htons(len);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next "len" bytes contains the name of the operator who processed this data */
    memcpy(p, history->operator_name, len);
    p += len;

    /* Next two byte integer contains the size of the command line field */
    if (history->command_line == (char *) NULL)
    {
        history->command_line = "";
    }
    len = strlen(history->command_line) + 1;
    stemp = htons(len);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next "len" bytes contains the command line used to run the processing program */
    memcpy(p, history->command_line, len);
    p += len;

    /* Next two byte integer contains the size of the history record comment */
    if (history->comment == (char *) NULL)
    {
        history->comment = "";
    }
    len = strlen(history->comment);
    stemp = htons(len);
    memcpy(p, &stemp, 2);
    p += 2;

    /* Next "len" bytes contains the comment for this history record */
    memcpy(p, history->comment, len);
    p += len;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : gsfEncodeNavigationError
 *
 * Description : This function encodes a navigation error record from
 *  internal to external byte stream form.
 *
 * Inputs :
 *    sptr = a pointer to an unsigned char buffer to write to
 *    nav_error = a pointer to a gsfNavigationError structure to read from
 *
 * Returns : This function returns the number of bytes encoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

int
gsfEncodeNavigationError(unsigned char *sptr, gsfNavigationError * nav_error)
{
    unsigned char  *p = sptr;
    gsfuLong        ltemp;

    /* First four byte integer contains the seconds portion of the time
    *  of navigation error.
    */
    ltemp = htonl(nav_error->nav_error_time.tv_sec);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* Next four byte integer contains the nanoseconds portion of the
    * history time.
    */
    ltemp = htonl(nav_error->nav_error_time.tv_nsec);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* Next four byte integer contains the record id for the record
    *  containing a position with this error. (registry and type number)
    */
    ltemp = htonl(nav_error->record_id);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* Next four byte integer contains the longitude error estimate */
    ltemp = htonl((gsfuLong) (nav_error->longitude_error * 10.0 + 0.501));
    memcpy(p, &ltemp, 4);
    p += 4;

    /* Next four byte integer contains the latitude error estimate */
    ltemp = htonl((gsfuLong) (nav_error->latitude_error * 10.0 + 0.501));
    memcpy(p, &ltemp, 4);
    p += 4;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : gsfEncodeHVNavigationError
 *
 * Description : This function encodes the new horizontal and vertical
 *  navigation error record.
 *
 * Inputs :
 *    sptr = a pointer to an unsigned char buffer to write to
 *    nav_error = a pointer to a gsfHVNavigationError structure to read from
 *
 * Returns : This function returns the number of bytes encoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

int
gsfEncodeHVNavigationError(unsigned char *sptr, gsfHVNavigationError *hv_nav_error)
{
    double          dtemp;
    unsigned char  *p = sptr;
    int             length;
    gsfsLong        ltemp;
    gsfsShort       stemp;

    /* First four byte integer contains the seconds portion of the time
    *  of navigation error.
    */
    ltemp = htonl(hv_nav_error->nav_error_time.tv_sec);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* The next four byte integer contains the nanoseconds portion of the
    * history time.
    */
    ltemp = htonl(hv_nav_error->nav_error_time.tv_nsec);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* The next four byte integer contains the record id for the record
    *  containing a position with this error. (registry and type number)
    */
    ltemp = htonl(hv_nav_error->record_id);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* The next four byte integer contains the horizontal error estimate */
    dtemp = (hv_nav_error->horizontal_error * 1000.0);
    if (dtemp < 0.0)
    {
        dtemp -= 0.501;
    }
    else
    {
        dtemp += 0.501;
    }
    ltemp = htonl((gsfsLong) dtemp);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* The next four byte integer contains the vertical error estimate */
    dtemp = (hv_nav_error->vertical_error * 1000.0);
    if (dtemp < 0.0)
    {
        dtemp -= 0.5;
    }
    else
    {
        dtemp += 0.5;
    }
    ltemp = htonl((gsfsLong) dtemp);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* The next four bytes are resevered for future use */
    *p = (unsigned char) (hv_nav_error->spare[0]);
    p += 1;
    *p = (unsigned char) (hv_nav_error->spare[1]);
    p += 1;
    *p = (unsigned char) (hv_nav_error->spare[2]);
    p += 1;
    *p = (unsigned char) (hv_nav_error->spare[3]);
    p += 1;

    /* The next two byte integer contains the length of the positioning system type string */
    if (hv_nav_error->position_type == (char *) NULL)
    {
        length = 0;
    }
    else
    {
        length = strlen(hv_nav_error->position_type);
    }
    stemp = htons(length);
    memcpy(p, &stemp, 2);
    p += 2;

    /* The next length bytes contains the positioning system type string. */
    if (hv_nav_error->position_type == (char *) NULL)
    {
        /* Put a null chacter down only if there is no string to record */
        *p = '\0';
        p += 1;
    }
    else
    {
        memcpy(p, hv_nav_error->position_type, length);
        p += length;
    }

    return (p - sptr);
}

static void LocalSubtractTimes (struct timespec *base_time, struct timespec *subtrahend, double *difference)
{
    double fraction = 0.0;
    double seconds  = 0.0;

    seconds  = difftime(base_time->tv_sec, subtrahend->tv_sec);
    fraction = ((double)(base_time->tv_nsec - subtrahend->tv_nsec))/1.0e9;

    *difference = seconds + fraction;
}

/********************************************************************
 *
 * Function Name : gsfEncodeAttitude
 *
 * Description : This function encodes a gsf attitude record from internal
 *  to external byte stream form.
 *
 * Inputs :
 *    sptr = a pointer to an unsigned char buffer to write to
 *    attitude = a pointer to a gsfAttitude structure to read from
 *
 * Returns : This function returns the number of bytes encoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

int
gsfEncodeAttitude(unsigned char *sptr, gsfAttitude * attitude)
{
    unsigned char  *p = sptr;
    gsfuLong        ltemp;
    gsfuShort       stemp;
    double          dtemp;
    int             i;
    struct timespec basetime;
    double          time_offset;

    /* write the full time for the first time in the record, and save subsequent times
     *  as an offset from this basetime
     */
    basetime = attitude->attitude_time[0];

    /* First four byte integer contains the seconds portion of the base
     *  time for the attitude record
     */
    ltemp = htonl(basetime.tv_sec);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* Next four byte integer contains the nanoseconds portion of the
    * attitude base time.
    */
    ltemp = htonl(basetime.tv_nsec);
    memcpy(p, &ltemp, 4);
    p += 4;

    /* Next two byte integer contains the number of measurements */
    stemp = htons(attitude->num_measurements);
    memcpy(p, &stemp, 2);
    p += 2;

    for (i = 0; i < attitude->num_measurements; i++)
    {
        /* Next two byte integer contains the time offset from basetime for this
         * measurement. Round the scaled quantity to the nearest whole integer.
         */
        LocalSubtractTimes (&attitude->attitude_time[i], &basetime, &time_offset);
        stemp = htons((gsfuShort) (time_offset * 1000.0 + 0.501));
        memcpy(p, &stemp, 2);
        p += 2;

        /* Next two byte integer contains the pitch. Round the scaled quantity
         * to the nearest whole integer.
         */
        dtemp = attitude->pitch[i] * 100.0;
        if (dtemp < 0.0)
        {
            dtemp -= 0.501;
        }
        else
        {
            dtemp += 0.501;
        }
        stemp = htons((gsfsShort) dtemp);
        memcpy(p, &stemp, 2);
        p += 2;

        /* Next two byte integer contains the roll. Round the scaled quantity
         * to the nearest whole integer.
         */
        dtemp = attitude->roll[i] * 100.0;
        if (dtemp < 0.0)
        {
            dtemp -= 0.501;
        }
        else
        {
            dtemp += 0.501;
        }
        stemp = htons((gsfsShort) dtemp);
        memcpy(p, &stemp, 2);
        p += 2;

        /* Next two byte integer contains the heave. Round the scaled quantity
         * to the nearest whole integer.
         */
        dtemp = attitude->heave[i] * 100.0;
        if (dtemp < 0.0)
        {
            dtemp -= 0.501;
        }
        else
        {
            dtemp += 0.501;
        }
        stemp = htons((gsfsShort) dtemp);
        memcpy(p, &stemp, 2);
        p += 2;

        /* Next two byte integer contains the heading. Round the scaled quantity
         * to the nearest whole integer.
         */
        dtemp = attitude->heading[i] * 100.0;
        if (dtemp < 0.0)
        {
            dtemp -= 0.501;
        }
        else
        {
            dtemp += 0.501;
        }
        stemp = htons((gsfuShort) dtemp);
        memcpy(p, &stemp, 2);
        p += 2;
    }

    return (p - sptr);
}

