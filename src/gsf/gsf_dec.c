/********************************************************************
 *
 * Module Name : GSF_DEC.C
 *
 * Author/Date : J. S. Byrne / 3 May 1994
 *
 * Description :
 *  This source file contains the gsf functions for dencoding a gsf byte
 *   stream given host data structures containing the data in engineering
 *   units.  Refer to gsf.h for a definition of the structures describing
 *   the internal form of the data.
 *
 * Restrictions/Limitations :
 * 1) This library assumes the host computer uses the ASCII character set.
 * 2) This library assumes that the data types u_short and u_long are defined
 *    on the host machine, where a u_short is a 16 bit unsigned integer, and
 *    a u_long is a 32 bit unsigned integer.
 * 3) This library assumes that the type short is at least 16 bits, and that
 *    the type int is at least 32 bits.
 *
 *
 * Change Descriptions :
 * who          when      what
 * ---          ----      ----
 * jsb          10-17-94  Added support for Reson SeaBat data
 * jsb          03-10-95  Modified the ping record structure to contain
 *                        dynamic depth corrector and tide corrector.
 * jsb          09-21-95  Modified so that the scale factor offset is
 *                        subtracted on decode.
 * jsb          11-13-95  Added unique subrecord id for EM1000
 * jsb          03-04-96  Fixed memset bug in DecodeSVP
 * hem          08-20-96  Added gsfDecodeSinglebeam, DecodeSASSSpecific,
 *                        DecodeTypeIIISeaBeamSpecific, DecodeEchotracSpecific,
 *                        DecodeMGD77Specific, DecodeBDBSpecific,
 *                        DecodeNOSHDBSpecific; fixed decoding of SVP latitudes
 *                        and longitudes to use signed values; added skipping
 *                        of unknown subrecords; added code to extract
 *                        sensor id's even if the subrecord has 0 length.
 * jsb          09-27-96  Added support for SeaBeam with amplitude data
 * jsb          03/24/97  Added gsfSeaBatIISpecific data structure to replace
 *                        the gsfSeaBatSpecific data structure, for the Reson 900x
 *                        series sonar systems.  Also added gsfSeaBat8101Specific
 *                        data structure for the Reson 8101 series sonar system.
 * bac          10/27/97  Added DecodeSeaBeam2112Specific to support the Sea Beam
 *                        2112/36 sonar.
 * dwc          1/9/98    Added DecodeElacMkIISpecific to support the Elac
 *                        Bottomchart MkII sonar.
 * jsb          09/28/98  Added gsfDecodeHVNavigationError. This addresses CRs:
 *                        98-001 and 98-002. In responce to NAVO CR: 98-003, added
 *                        support for horizontal_error ping array subrecord.
 * jsb          12/29/98  Added support for Simrad em3000 series sonar systems.
 * wkm          3-30-99   Added DecodeCmpSassSpecific to deal with Compressed SASS data.
 * wkm          8-02-99   Updated DecodeCmpSassSpecific to include lntens (heave) with Compressed SASS data.
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

/* rely on the network type definitions of (u_short, and u_long) */
#include <sys/types.h>
#include <netinet/in.h>

/* gsf library interface description */
#include "gsf.h"
#include "gsf_enc.h"
#include "gsf_dec.h"

/* Macro definitions for this file */
#define RESON_MASK1 192
#define RESON_MASK2  48
#define RESON_MASK3  12
#define RESON_MASK4   3

/* Global static data for this module */
/* Arrays have last number beams stored at [fileHandle][index=record ID-1] */
static short    arraySize[GSF_MAX_OPEN_FILES][GSF_MAX_PING_ARRAY_SUBRECORDS];

/* Global external data defined in this module */
extern int      gsfError;                               /* defined in gsf.c */

/* Function prototypes for this file */
static int      DecodeScaleFactors(gsfScaleFactors *sf, unsigned char *ptr);
static int      DecodeTwoByteArray(double **array, unsigned char *ptr, int num_beams, gsfScaleFactors * sf, int id, int handle);
static int      DecodeSignedTwoByteArray(double **array, char *ptr, int num_beams, gsfScaleFactors * sf, int id, int handle);
static int      DecodeByteArray(double **array, unsigned char *ptr, int num_beams, gsfScaleFactors * sf, int id, int handle);
static int      DecodeSignedByteArray(double **array, char *ptr, int num_beams, gsfScaleFactors * sf, int id, int handle);
static int      DecodeBeamFlagsArray(unsigned char **array, unsigned char *ptr, int num_beams, int handle);
static int      DecodeQualityFlagsArray(unsigned char **array, unsigned char *ptr, int num_beams, int handle);
static int      DecodeSeabeamSpecific(gsfSensorSpecific * sdata, unsigned char *sptr);
static int      DecodeEM12Specific(gsfSensorSpecific * sdata, unsigned char *sptr);
static int      DecodeEM100Specific(gsfSensorSpecific * sdata, unsigned char *sptr);
static int      DecodeEM950Specific(gsfSensorSpecific * sdata, unsigned char *sptr);
static int      DecodeEM1000Specific(gsfSensorSpecific * sdata, unsigned char *sptr);
static int      DecodeEM121ASpecific(gsfSensorSpecific * sdata, unsigned char *sptr);
static int      DecodeEM121Specific(gsfSensorSpecific * sdata, unsigned char *sptr);

#if 1
/* 3-30-99 wkm: obsolete */
static int      DecodeTypeIIISeaBeamSpecific(gsfSensorSpecific * sdata, unsigned char *sptr);
static int      DecodeSASSSpecific(gsfSensorSpecific * sdata, unsigned char *sptr);
#endif

static int      DecodeCmpSassSpecific(gsfSensorSpecific * sdata, unsigned char *sptr);

static int      DecodeSeaMapSpecific(gsfSensorSpecific * sdata, unsigned char *sptr);
static int      DecodeSeaBatSpecific(gsfSensorSpecific * sdata, unsigned char *sptr);
static int      DecodeEchotracSpecific(gsfSBSensorSpecific * sdata, unsigned char *sptr);
static int      DecodeMGD77Specific(gsfSBSensorSpecific * sdata, unsigned char *sptr);
static int      DecodeBDBSpecific(gsfSBSensorSpecific * sdata, unsigned char *sptr);
static int      DecodeNOSHDBSpecific(gsfSBSensorSpecific * sdata, unsigned char *sptr);
static int      DecodeSBAmpSpecific(gsfSensorSpecific * sdata, unsigned char *sptr);
static int      DecodeSeaBatIISpecific(gsfSensorSpecific * sdata, unsigned char *sptr);
static int      DecodeSeaBat8101Specific(gsfSensorSpecific * sdata, unsigned char *sptr);
static int      DecodeSeaBeam2112Specific(gsfSensorSpecific * sdata, unsigned char *sptr);
static int      DecodeElacMkIISpecific(gsfSensorSpecific * sdata, unsigned char *sptr);
static int      DecodeEM3Specific(gsfSensorSpecific *sdata, unsigned char *sptr, GSF_FILE_TABLE *ft);

/********************************************************************
 *
 * Function Name : gsfDecodeHeader
 *
 * Description :
 *  This function decodes a gsf header data record from external to internal
 *  form.
 *
 * Inputs :
 *  header = a pointer to a gsfHeader structure to be populated
 *  sptr = a pointer to the gsf byte stream containing the header record.
 *
 * Returns :
 *  This function returns the number of bytes decoded if successful, or
 *  -1 if an error occured.
 *
 * Error Conditions :
 *    none
 *
 ********************************************************************/
int
gsfDecodeHeader(gsfHeader * header, unsigned char *sptr)
{
    memset(header->version, 0, sizeof(gsfHeader));
    memcpy(header->version, sptr, sizeof(header->version));

    return ((int) strlen(header->version));
}

/********************************************************************
 *
 * Function Name : gsfDecodeSwathBathySummary
 *
 * Description :
 *  This function decodes a gsf swath bathymetry summary data record from
 *   external to internal form.
 *
 * Inputs :
 *  header = a pointer to a gsfSwathBathySummary structure to be populated
 *  sptr = a pointer to the gsf byte stream containing the header record.
 *
 * Returns :
 *  This function returns the number of bytes decoded if successful, or
 *  -1 if an error occured.
 *
 * Error Conditions :
 *    none
 *
 ********************************************************************/
int
gsfDecodeSwathBathySummary(gsfSwathBathySummary *sum, unsigned char *sptr)
{
    gsfuLong        ltemp;
    unsigned char  *p = sptr;
    gsfsLong        signed_int;

    /* First 8 bytes contain the time of the first ping in this file */
    memcpy(&ltemp, p, 4);
    sum->start_time.tv_sec = ntohl(ltemp);
    p += 4;

    memcpy(&ltemp, p, 4);
    sum->start_time.tv_nsec = ntohl(ltemp);
    p += 4;

    /* Next 8 bytes contain the time of the last ping in this file */
    memcpy(&ltemp, p, 4);
    sum->end_time.tv_sec = ntohl(ltemp);
    p += 4;

    memcpy(&ltemp, p, 4);
    sum->end_time.tv_nsec = ntohl(ltemp);
    p += 4;

    /* Next four byte integer contains the minimum latitude */
    memcpy(&ltemp, p, 4);
    signed_int = (signed) ntohl(ltemp);
    sum->min_latitude = ((double) signed_int) / 1.0e7;
    p += 4;

    /* Next four byte integer contains the minimum longitude */
    memcpy(&ltemp, p, 4);
    signed_int = (signed) ntohl(ltemp);
    sum->min_longitude = ((double) signed_int) / 1.0e7;
    p += 4;

    /* Next four byte integer contains the maximum latitude */
    memcpy(&ltemp, p, 4);
    signed_int = (signed) ntohl(ltemp);
    sum->max_latitude = ((double) signed_int) / 1.0e7;
    p += 4;

    /* Next four byte integer contains the maximum longitude */
    memcpy(&ltemp, p, 4);
    signed_int = (signed) ntohl(ltemp);
    sum->max_longitude = ((double) signed_int) / 1.0e7;
    p += 4;

    /* Next four bytes contains the minimum depth */
    memcpy(&ltemp, p, 4);
    sum->min_depth = ntohl(ltemp) / 100.0;
    p += 4;

    /* Next four bytes contains the maximum depth */
    memcpy(&ltemp, p, 4);
    sum->max_depth = ntohl(ltemp) / 100.0;
    p += 4;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : DecodeEchotracSpecific
 *
 * Description :
 *  This function decodes the Bathy2000 and Echotrac sensor specific
 *  data.
 *
 * Inputs :
 *    sdata = a pointer to the union of sensor specific data to be loaded
 *    sptr = a pointer to an unsigned char buffer containing the byte stream
 *           to read.
 *
 * Returns : This function returns the number of bytes decoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
DecodeEchotracSpecific(gsfSBSensorSpecific * sdata, unsigned char *sptr)
{
    unsigned char  *p = sptr;
    gsfuShort       stemp;

    /* First two byte integer contains the navigation error */
    memcpy(&stemp, p, 2);
    sdata->gsfEchotracSpecific.navigation_error = (int) ntohs(stemp);
    p += 2;

    /* Next byte contains the most probable position source navigation */
    sdata->gsfEchotracSpecific.mpp_source = (int) *p;
    p += 1;

    /* Next byte contains the tide source */
    sdata->gsfEchotracSpecific.tide_source = (int) *p;
    p += 1;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : DecodeMGD77Specific
 *
 * Description :
 *  This function decodes the MGD77 fields.  The MGD77 is essentially
 *  survey trackline data, and actual survey data can be retrieved
 *  from the source.
 *
 * Inputs :
 *    sdata = a pointer to the union of sensor specific data to be loaded
 *    sptr = a pointer to an unsigned char buffer containing the byte stream
 *           to read.
 *
 * Returns : This function returns the number of bytes decoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
DecodeMGD77Specific(gsfSBSensorSpecific * sdata, unsigned char *sptr)
{
    unsigned char  *p = sptr;
    gsfuLong        ltemp;
    gsfuShort       stemp;

    /* First two byte integer contains the time zone correction */
    memcpy(&stemp, p, 2);
    sdata->gsfMGD77Specific.time_zone_corr = (int) ntohs(stemp);
    p += 2;

    /* The next two byte integer contains how the navigation was obtained */
    memcpy(&stemp, p, 2);
    sdata->gsfMGD77Specific.position_type_code = (int) ntohs(stemp);
    p += 2;

    /* The next two byte integer contains how the sound velocity
       correction was made */
    memcpy(&stemp, p, 2);
    sdata->gsfMGD77Specific.correction_code = (int) ntohs(stemp);
    p += 2;

    /* The next two byte integer contains how the bathymetry was obtained */
    memcpy(&stemp, p, 2);
    sdata->gsfMGD77Specific.bathy_type_code = (int) ntohs(stemp);
    p += 2;

    /* The next two byte integer contains the quality code for nav */
    memcpy(&stemp, p, 2);
    sdata->gsfMGD77Specific.quality_code = (int) ntohs(stemp);
    p += 2;

    /* The next four byte integer contains the two way travel time */
    memcpy(&ltemp, p, 4);
    sdata->gsfMGD77Specific.travel_time = (double) (ntohl(ltemp)) / 10000.0;
    p += 4;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : DecodeBDBSpecific
 *
 * Description :
 *  This function decodes the BDB fields.  The BDB is essentially
 *  survey trackline data.
 *
 * Inputs :
 *    sdata = a pointer to the union of sensor specific data to be loaded
 *    sptr = a pointer to an unsigned char buffer containing the byte stream
 *           to read.
 *
 * Returns : This function returns the number of bytes decoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
DecodeBDBSpecific(gsfSBSensorSpecific * sdata, unsigned char *sptr)
{
    unsigned char  *p = sptr;
    gsfuLong        ltemp;
    gsfuShort       stemp;

    /* The next four byte integer contains the two way travel time */
    memcpy(&ltemp, p, 4);
    sdata->gsfBDBSpecific.doc_no = (int) (ntohl(ltemp));
    p += 4;

    /* Next byte contains the evaluation flag */
    sdata->gsfBDBSpecific.eval = (char) *p;
    p += 1;

    /* Next byte contains the classification flag */
    sdata->gsfBDBSpecific.class = (char) *p;
    p += 1;

    /* Next byte contains the track adjustment flag */
    sdata->gsfBDBSpecific.track_adj_flag = (char) *p;
    p += 1;

    /* Next byte contains the source flag */
    sdata->gsfBDBSpecific.source_flag = (char) *p;
    p += 1;

    /* Next byte contains the discrete point or track line flag */
    sdata->gsfBDBSpecific.pt_or_track_ln = (char) *p;
    p += 1;

    /* Next byte contains the datum flag */
    sdata->gsfBDBSpecific.datum_flag = (char) *p;
    p += 1;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : DecodeNOSHDBSpecific
 *
 * Description :
 *  This function decodes the NOSHDB fields.  The NOSHDB is essentially
 *  survey trackline data.
 *
 * Inputs :
 *    sdata = a pointer to the union of sensor specific data to be loaded
 *    sptr = a pointer to an unsigned char buffer containing the byte stream
 *           to read.
 *
 * Returns : This function returns the number of bytes decoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
DecodeNOSHDBSpecific(gsfSBSensorSpecific * sdata, unsigned char *sptr)
{
    unsigned char  *p = sptr;
    gsfuShort       stemp;

    /* First two byte integer contains the depth type code */
    memcpy(&stemp, p, 2);
    sdata->gsfNOSHDBSpecific.type_code = (int) ntohs(stemp);
    p += 2;

    /* The next two byte integer contains the cartographic code */
    memcpy(&stemp, p, 2);
    sdata->gsfNOSHDBSpecific.carto_code = (int) ntohs(stemp);
    p += 2;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : gsfDecodeSinglebeam
 *
 * Description : This function decodes a gsf single beam ping record
 *  from external byte stream form to internal form.
 *
 * Inputs :
 *   ping = a pointer to the single beam ping structure to be populated
 *   sptr = a pointer to the gsf byte stream contain the ping record
 *   ft = a pointer to the GSF_FILE_TABLE entry for the data file being decoded
 *   handle = the handle to the gsf data file (used to track number beams)
 *   record_size = the number of bytes which ping byte stream occupies.
 *
 * Returns :
 *  This function returns the number of bytes decoded if successful, or
 *  -1 if an error occured.
 *
 * Error Conditions :
 *    GSF_UNRECOGNIZED_SENSOR_ID
 *
 ********************************************************************/
int
gsfDecodeSinglebeam(gsfSingleBeamPing * ping, unsigned char *sptr, GSF_FILE_TABLE *ft, int handle, int record_size)
{
    gsfuLong        ltemp;
    int             subrecord_size;
    int             subrecord_id;
    gsfsShort       signed_short;
    gsfsLong        signed_int;
    gsfuShort       stemp;
    int             ret;
    int             bytes;
    unsigned char  *p = sptr;

    /* First 8 bytes contain the time */
    memcpy(&ltemp, p, 4);
    ping->ping_time.tv_sec = ntohl(ltemp);
    p += 4;

    memcpy(&ltemp, p, 4);
    ping->ping_time.tv_nsec = ntohl(ltemp);
    p += 4;

    /* Next four byte integer contains the longitude */
    memcpy(&ltemp, p, 4);
    signed_int = (signed) ntohl(ltemp);
    ping->longitude = ((double) signed_int) / 1.0e7;
    p += 4;

    /* Next four byte integer contains the latitude */
    memcpy(&ltemp, p, 4);
    signed_int = (signed) ntohl(ltemp);
    ping->latitude = ((double) signed_int) / 1.0e7;
    p += 4;

    /* Next two byte integer contains the tide corrector for this ping. */
    memcpy(&stemp, p, 2);
    signed_short = (signed) ntohs(stemp);
    ping->tide_corrector = ((double) signed_short) / 100.0;
    p += 2;

    /* Next four byte integer contains the depth corrector. */
    memcpy(&ltemp, p, 4);
    signed_int = (signed) ntohl(ltemp);
    ping->depth_corrector = ((double) signed_int) / 100.0;
    p += 4;

    /* Next two byte integer contains the ship heading */
    memcpy(&stemp, p, 2);
    ping->heading = ((double) ntohs(stemp)) / 100.0;
    p += 2;

    /* Next two byte integer contains the pitch */
    memcpy(&stemp, p, 2);
    signed_short = (signed) ntohs(stemp);
    ping->pitch = ((double) signed_short) / 100.0;
    p += 2;

    /* Next two byte integer contains the roll */
    memcpy(&stemp, p, 2);
    signed_short = (signed) ntohs(stemp);
    ping->roll = ((double) signed_short) / 100.0;
    p += 2;

    /* Next two byte integer contains the heave */
    memcpy(&stemp, p, 2);
    signed_short = (signed) ntohs(stemp);
    ping->heave = ((double) signed_short) / 100.0;
    p += 2;

    /* Next four byte integer contains the depth */
    memcpy(&ltemp, p, 4);
    signed_int = (signed) ntohl(ltemp);
    ping->depth = ((double) signed_int) / 100.0;
    p += 4;

    /* Next two byte integer contains the sound speed correction */
    memcpy(&stemp, p, 2);
    signed_short = (signed) ntohs(stemp);
    ping->sound_speed_correction = ((double) signed_short) / 100.0;
    p += 2;

    /* Next byte contains the positioning system type */
    memcpy(&stemp, p, 2);
    ping->positioning_system_type = (unsigned) ntohs(stemp);
    p += 2;

    /* Determine which subrecord we have, and then decode it.
     * (Size may have been padded out to a four byte boundary.) */
    bytes = p - sptr;
    while ((record_size - bytes) > 4)
    {
        /* First four byte integer in subrecord contains the subrecord
        *  size and subrecord identifier.
        */
        memcpy(&ltemp, p, 4);
        p += 4;
        ltemp = ntohl(ltemp);
        subrecord_id = (ltemp & 0xFF000000) >> 24;
        subrecord_size = ltemp & 0x00FFFFFF;

       switch (subrecord_id)
       {
          case (GSF_SINGLE_BEAM_SUBRECORD_ECHOTRAC_SPECIFIC):
             p += DecodeEchotracSpecific(&ping->sensor_data, p);
             ping->sensor_id = GSF_SINGLE_BEAM_SUBRECORD_ECHOTRAC_SPECIFIC;
          break;

          case (GSF_SINGLE_BEAM_SUBRECORD_BATHY2000_SPECIFIC):
             p += DecodeEchotracSpecific(&ping->sensor_data, p);
             ping->sensor_id = GSF_SINGLE_BEAM_SUBRECORD_BATHY2000_SPECIFIC;

          break;

          case (GSF_SINGLE_BEAM_SUBRECORD_MGD77_SPECIFIC):
             p += DecodeMGD77Specific (&ping->sensor_data, p);
             ping->sensor_id = GSF_SINGLE_BEAM_SUBRECORD_MGD77_SPECIFIC;
          break;

          case (GSF_SINGLE_BEAM_SUBRECORD_BDB_SPECIFIC):
             p += DecodeBDBSpecific (&ping->sensor_data, p);
             ping->sensor_id = GSF_SINGLE_BEAM_SUBRECORD_BDB_SPECIFIC;
          break;

          case (GSF_SINGLE_BEAM_SUBRECORD_NOSHDB_SPECIFIC):
             p += DecodeNOSHDBSpecific (&ping->sensor_data, p);
             ping->sensor_id = GSF_SINGLE_BEAM_SUBRECORD_NOSHDB_SPECIFIC;
          break;

          case (GSF_SWATH_BATHY_SUBRECORD_UNKNOWN):
            ping->sensor_id =  GSF_SWATH_BATHY_SUBRECORD_UNKNOWN;
          break;

          default:
            gsfError = GSF_UNRECOGNIZED_SUBRECORD_ID;
            if ((((p - sptr) + subrecord_size) == record_size) ||
                ((record_size - ((p - sptr) + subrecord_size)) > 0))
              p+=subrecord_size;
            else
              return (-1);
            break;
       }
       bytes = p - sptr;
    }

    /*  Extract subrecord id if the subrecord size is 0 */
    if (((record_size - bytes) == 4) && (ping->sensor_id != subrecord_id))
    {
        /* First four byte integer in subrecord contains the subrecord
        *  size and subrecord identifier.
        */
        memcpy(&ltemp, p, 4);
        p += 4;
        ltemp = ntohl(ltemp);
        subrecord_id = (ltemp & 0xFF000000) >> 24;
        subrecord_size = ltemp & 0x00FFFFFF;

        ping->sensor_id = subrecord_id;
    }

    /* Return the number of byte written into the buffer */
    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : gsfDecodeSwathBathymetryPing
 *
 * Description :
 *  This function decodes a gsf swath bathymetry ping record from external
 *  to internal form.
 *
 * Inputs :
 *   ping = a pointer to the swath bathymetry ping structure to be populated
 *   sptr = a pointer to the gsf byte stream contain the ping record
 *   ft = a pointer to the GSF_FILE_TABLE entry for the data file being decoded
 *   handle = the handle to the gsf data file (used to track number beams)
 *   record_size = the number of bytes which ping byte stream occupies.
 *
 * Returns :
 *  This function returns the number of bytes decoded if successful, or
 *  -1 if an error occured.
 *
 * Error Conditions :
 *   GSF_UNRECOGNIZED_SUBRECORD_ID
 *
 ********************************************************************/
int
gsfDecodeSwathBathymetryPing(gsfSwathBathyPing *ping, unsigned char *sptr, GSF_FILE_TABLE *ft, int handle, int record_size)
{
    gsfuLong        ltemp;
    int             subrecord_size;
    int             subrecord_id;
    gsfsShort       signed_short;
    gsfsLong        signed_int;
    gsfuShort       stemp;
    int             ret;
    int             bytes;
    int             i;
    unsigned char  *p = sptr;

    /* First 8 bytes contain the time */
    memcpy(&ltemp, p, 4);
    ping->ping_time.tv_sec = ntohl(ltemp);
    p += 4;

    memcpy(&ltemp, p, 4);
    ping->ping_time.tv_nsec = ntohl(ltemp);
    p += 4;

    /* Next four byte integer contains the longitude */
    memcpy(&ltemp, p, 4);
    signed_int = (signed) ntohl(ltemp);
    ping->longitude = ((double) signed_int) / 1.0e7;
    p += 4;

    /* Next four byte integer contains the latitude */
    memcpy(&ltemp, p, 4);
    signed_int = (signed) ntohl(ltemp);
    ping->latitude = ((double) signed_int) / 1.0e7;
    p += 4;

    /* Next two byte integer contains the number of beams */
    memcpy(&stemp, p, 2);
    ping->number_beams = ntohs(stemp);
    p += 2;

    /* Next two byte integer contains the center beam number, portmost
    *  outer beam is beam number 0.
    */
    memcpy(&stemp, p, 2);
    ping->center_beam = ntohs(stemp);
    p += 2;

    /* Next two byte integer contains the ping flags field */
    memcpy(&stemp, p, 2);
    ping->ping_flags = ntohs(stemp);
    p += 2;

    /* Next two byte integer is a reserved field */
    memcpy(&stemp, p, 2);
    ping->reserved = ntohs(stemp);
    p += 2;

    /* Next two byte integer contains the tide corrector for this ping */
    memcpy(&stemp, p, 2);
    signed_short = (signed) ntohs(stemp);
    ping->tide_corrector = ((double) signed_short) / 100.0;
    p += 2;

    /* Next four byte integer contains the depth corrector for this ping */
    memcpy(&ltemp, p, 4);
    signed_int = (signed) ntohl(ltemp);
    ping->depth_corrector = ((double) signed_int) / 100.0;
    p += 4;

    /* Next two byte integer contains the ship heading */
    memcpy(&stemp, p, 2);
    ping->heading = ((double) ntohs(stemp)) / 100.0;
    p += 2;

    /* Next two byte integer contains the pitch */
    memcpy(&stemp, p, 2);
    signed_short = (signed) ntohs(stemp);
    ping->pitch = ((double) signed_short) / 100.0;
    p += 2;

    /* Next two byte integer contains the roll */
    memcpy(&stemp, p, 2);
    signed_short = (signed) ntohs(stemp);
    ping->roll = ((double) signed_short) / 100.0;
    p += 2;

    /* Next two byte integer contains the heave */
    memcpy(&stemp, p, 2);
    signed_short = (signed) ntohs(stemp);
    ping->heave = ((double) signed_short) / 100.0;
    p += 2;

    /* Next two byte integer contains the course */
    memcpy(&stemp, p, 2);
    ping->course = ((double) ntohs(stemp)) / 100.0;
    p += 2;

    /* Next two byte integer contains the speed */
    memcpy(&stemp, p, 2);
    ping->speed = ((double) ntohs(stemp)) / 100.0;
    p += 2;

    /* Set the caller's array pointers to NULL to guarrantee that non
     * NULL pointers define the array data for this file.
     */
    ping->depth = (double *) NULL;
    ping->nominal_depth = (double *) NULL;
    ping->across_track = (double *) NULL;
    ping->along_track = (double *) NULL;
    ping->travel_time = (double *) NULL;
    ping->beam_angle = (double *) NULL;
    ping->mc_amplitude = (double *) NULL;
    ping->mr_amplitude = (double *) NULL;
    ping->echo_width = (double *) NULL;
    ping->quality_factor = (double *) NULL;
    ping->receive_heave = (double *) NULL;
    ping->depth_error = (double *) NULL;
    ping->across_track_error = (double *) NULL;
    ping->along_track_error = (double *) NULL;
    ping->quality_flags = (unsigned char *) NULL;
    ping->beam_flags = (unsigned char *) NULL;
    ping->signal_to_noise = (double *) NULL;
    ping->beam_angle_forward = (double *) NULL;
    ping->vertical_error = (double *) NULL;
    ping->horizontal_error = (double *) NULL;

    /* Clear the flag which indicates that we've read scale factors */
    ft->scales_read = 0;

    /* Load the caller's structure with the last scale factors for this
     * file.  If this is the first ping of a file we can expect it to
     * have scale factors.
     */
    memcpy (&ping->scaleFactors, &ft->rec.mb_ping.scaleFactors, sizeof(gsfScaleFactors));

    /* Determine which subrecord(s) we have, and then decode them, until we've
    *  read through the entire ping record. (Size may have been padded out to
    *  a four byte boundary.)
    */
    bytes = p - sptr;
    while ((record_size - bytes) > 4)
    {
        /* First four byte integer in subrecord contains the subrecord
        *  size and subrecord identifier.
        */
        memcpy(&ltemp, p, 4);
        p += 4;
        ltemp = ntohl(ltemp);
        subrecord_id = (ltemp & 0xFF000000) >> 24;
        subrecord_size = ltemp & 0x00FFFFFF;

        switch (subrecord_id)
        {
            case (GSF_SWATH_BATHY_SUBRECORD_UNKNOWN):
                ping->sensor_id = GSF_SWATH_BATHY_SUBRECORD_UNKNOWN;
                break;

            case (GSF_SWATH_BATHY_SUBRECORD_SCALE_FACTORS):
                ret = DecodeScaleFactors(&ft->rec.mb_ping.scaleFactors, p);
                if (ret < 0)
                {
                    return (-1);
                }
                ft->scales_read = 1;

                /* Load the caller's structure with these scale factors */
                memcpy (&ping->scaleFactors, &ft->rec.mb_ping.scaleFactors, sizeof(gsfScaleFactors));
                p += ret;

                /* jsb 05/14/97  Set the gsfFileTable reference of the last scale factors read to point
                 * to this ping.  This is required for consistent tracking of scale factors.  This is
                 * necessary for programs, such as exammb, which do mixed access (direct and sequential).
                 * The outermost if block is necessary to protect from accessing the dynamically allocated
                 * scale_factor_addr array prior to allocation. This memory is allocated after we have
                 * completed the creation of the index file.
                 */
                if (ft->index_data.scale_factor_addr)
                {
                    for (i = 0; i < ft->index_data.number_of_records[0]; i++)
                    {
                        /* Search for the address of the ping record containing scale factors which matches
                         * the start address of this ping record.
                         */
                        if (ft->previous_record == ft->index_data.scale_factor_addr[i].addr)
                        {
                            ft->index_data.last_scale_factor_index = i;
                            break;
                        }
                    }
                }
                break;

            case (GSF_SWATH_BATHY_SUBRECORD_DEPTH_ARRAY):
                ret = DecodeTwoByteArray(&ft->rec.mb_ping.depth, p, ping->number_beams,
                    &ft->rec.mb_ping.scaleFactors, GSF_SWATH_BATHY_SUBRECORD_DEPTH_ARRAY, handle);
                if (ret < 0)
                {
                    return (-1);
                }
                ping->depth = ft->rec.mb_ping.depth;
                p += ret;
                break;

            case (GSF_SWATH_BATHY_SUBRECORD_NOMINAL_DEPTH_ARRAY):
                ret = DecodeTwoByteArray(&ft->rec.mb_ping.nominal_depth, p, ping->number_beams,
                    &ft->rec.mb_ping.scaleFactors, GSF_SWATH_BATHY_SUBRECORD_NOMINAL_DEPTH_ARRAY, handle);
                if (ret < 0)
                {
                    return (-1);
                }
                ping->nominal_depth = ft->rec.mb_ping.nominal_depth;
                p += ret;
                break;

            case (GSF_SWATH_BATHY_SUBRECORD_ACROSS_TRACK_ARRAY):
                ret = DecodeSignedTwoByteArray(&ft->rec.mb_ping.across_track, (char *)p, ping->number_beams,
                    &ft->rec.mb_ping.scaleFactors, GSF_SWATH_BATHY_SUBRECORD_ACROSS_TRACK_ARRAY, handle);
                if (ret < 0)
                {
                    return (-1);
                }
                ping->across_track = ft->rec.mb_ping.across_track;
                p += ret;
                break;

            case (GSF_SWATH_BATHY_SUBRECORD_ALONG_TRACK_ARRAY):
                ret = DecodeSignedTwoByteArray(&ft->rec.mb_ping.along_track, (char *)p, ping->number_beams,
                    &ft->rec.mb_ping.scaleFactors, GSF_SWATH_BATHY_SUBRECORD_ALONG_TRACK_ARRAY, handle);
                if (ret < 0)
                {
                    return (-1);
                }
                ping->along_track = ft->rec.mb_ping.along_track;
                p += ret;
                break;

            case (GSF_SWATH_BATHY_SUBRECORD_TRAVEL_TIME_ARRAY):
                ret = DecodeTwoByteArray(&ft->rec.mb_ping.travel_time, p, ping->number_beams,
                    &ft->rec.mb_ping.scaleFactors, GSF_SWATH_BATHY_SUBRECORD_TRAVEL_TIME_ARRAY, handle);
                if (ret < 0)
                {
                    return (-1);
                }
                ping->travel_time = ft->rec.mb_ping.travel_time;
                p += ret;
                break;

            case (GSF_SWATH_BATHY_SUBRECORD_BEAM_ANGLE_ARRAY):
                ret = DecodeSignedTwoByteArray(&ft->rec.mb_ping.beam_angle, (char *)p, ping->number_beams,
                    &ft->rec.mb_ping.scaleFactors, GSF_SWATH_BATHY_SUBRECORD_BEAM_ANGLE_ARRAY, handle);
                if (ret < 0)
                {
                    return (-1);
                }
                ping->beam_angle = ft->rec.mb_ping.beam_angle;
                p += ret;
                break;

            case (GSF_SWATH_BATHY_SUBRECORD_MEAN_CAL_AMPLITUDE_ARRAY):
                ret = DecodeSignedByteArray(&ft->rec.mb_ping.mc_amplitude, (char *)p, ping->number_beams,
                    &ft->rec.mb_ping.scaleFactors, GSF_SWATH_BATHY_SUBRECORD_MEAN_CAL_AMPLITUDE_ARRAY, handle);
                if (ret < 0)
                {
                    return (-1);
                }
                ping->mc_amplitude = ft->rec.mb_ping.mc_amplitude;
                p += ret;
                break;

            case (GSF_SWATH_BATHY_SUBRECORD_MEAN_REL_AMPLITUDE_ARRAY):
                ret = DecodeByteArray(&ft->rec.mb_ping.mr_amplitude, p, ping->number_beams,
                    &ft->rec.mb_ping.scaleFactors, GSF_SWATH_BATHY_SUBRECORD_MEAN_REL_AMPLITUDE_ARRAY, handle);
                if (ret < 0)
                {
                    return (-1);
                }
                ping->mr_amplitude = ft->rec.mb_ping.mr_amplitude;
                p += ret;
                break;

            case (GSF_SWATH_BATHY_SUBRECORD_ECHO_WIDTH_ARRAY):
                ret = DecodeByteArray(&ft->rec.mb_ping.echo_width, p, ping->number_beams,
                    &ft->rec.mb_ping.scaleFactors, GSF_SWATH_BATHY_SUBRECORD_ECHO_WIDTH_ARRAY, handle);
                if (ret < 0)
                {
                    return (-1);
                }
                ping->echo_width = ft->rec.mb_ping.echo_width;
                p += ret;
                break;

            case (GSF_SWATH_BATHY_SUBRECORD_QUALITY_FACTOR_ARRAY):
                ret = DecodeByteArray(&ft->rec.mb_ping.quality_factor, p, ping->number_beams,
                    &ft->rec.mb_ping.scaleFactors, GSF_SWATH_BATHY_SUBRECORD_QUALITY_FACTOR_ARRAY, handle);
                if (ret < 0)
                {
                    return (-1);
                }
                ping->quality_factor = ft->rec.mb_ping.quality_factor;
                p += ret;
                break;

            case (GSF_SWATH_BATHY_SUBRECORD_RECEIVE_HEAVE_ARRAY):
                ret = DecodeSignedByteArray(&ft->rec.mb_ping.receive_heave, (char *)p, ping->number_beams,
                    &ft->rec.mb_ping.scaleFactors, GSF_SWATH_BATHY_SUBRECORD_RECEIVE_HEAVE_ARRAY, handle);
                if (ret < 0)
                {
                    return (-1);
                }
                ping->receive_heave = ft->rec.mb_ping.receive_heave;
                p += ret;
                break;

            case (GSF_SWATH_BATHY_SUBRECORD_DEPTH_ERROR_ARRAY):
                ret = DecodeTwoByteArray(&ft->rec.mb_ping.depth_error, p, ping->number_beams,
                    &ft->rec.mb_ping.scaleFactors, GSF_SWATH_BATHY_SUBRECORD_DEPTH_ERROR_ARRAY, handle);
                if (ret < 0)
                {
                    return (-1);
                }
                ping->depth_error = ft->rec.mb_ping.depth_error;
                p += ret;
                break;

            case (GSF_SWATH_BATHY_SUBRECORD_ACROSS_TRACK_ERROR_ARRAY):
                ret = DecodeTwoByteArray(&ft->rec.mb_ping.across_track_error, p, ping->number_beams,
                    &ft->rec.mb_ping.scaleFactors, GSF_SWATH_BATHY_SUBRECORD_ACROSS_TRACK_ERROR_ARRAY, handle);
                if (ret < 0)
                {
                    return (-1);
                }
                ping->across_track_error = ft->rec.mb_ping.across_track_error;
                p += ret;
                break;

            case (GSF_SWATH_BATHY_SUBRECORD_ALONG_TRACK_ERROR_ARRAY):
                ret = DecodeTwoByteArray(&ft->rec.mb_ping.along_track_error, p, ping->number_beams,
                    &ft->rec.mb_ping.scaleFactors, GSF_SWATH_BATHY_SUBRECORD_ALONG_TRACK_ERROR_ARRAY, handle);
                if (ret < 0)
                {
                    return (-1);
                }
                ping->along_track_error =  ft->rec.mb_ping.along_track_error;
                p += ret;
                break;

            case (GSF_SWATH_BATHY_SUBRECORD_BEAM_FLAGS_ARRAY):
                ret = DecodeBeamFlagsArray(&ft->rec.mb_ping.beam_flags, p, ping->number_beams, handle);
                if (ret < 0)
                {
                    return (-1);
                }
                ping->beam_flags = ft->rec.mb_ping.beam_flags;
                p += ret;
                break;

            case (GSF_SWATH_BATHY_SUBRECORD_QUALITY_FLAGS_ARRAY):
                ret = DecodeQualityFlagsArray(&ft->rec.mb_ping.quality_flags, p, ping->number_beams, handle);
                if (ret < 0)
                {
                    return (-1);
                }
                ping->quality_flags = ft->rec.mb_ping.quality_flags;
                p += ret;
                break;

            case (GSF_SWATH_BATHY_SUBRECORD_SIGNAL_TO_NOISE_ARRAY):
                ret = DecodeSignedByteArray(&ft->rec.mb_ping.signal_to_noise, (char *)p, ping->number_beams,
                    &ft->rec.mb_ping.scaleFactors, GSF_SWATH_BATHY_SUBRECORD_SIGNAL_TO_NOISE_ARRAY, handle);
                if (ret < 0)
                {
                    return (-1);
                }
                ping->signal_to_noise = ft->rec.mb_ping.signal_to_noise;
                p += ret;
                break;

            case (GSF_SWATH_BATHY_SUBRECORD_BEAM_ANGLE_FORWARD_ARRAY):
                ret = DecodeTwoByteArray(&ft->rec.mb_ping.beam_angle_forward, p, ping->number_beams,
                    &ft->rec.mb_ping.scaleFactors, GSF_SWATH_BATHY_SUBRECORD_BEAM_ANGLE_FORWARD_ARRAY, handle);
                if (ret < 0)
                {
                    return (-1);
                }
                ping->beam_angle_forward = ft->rec.mb_ping.beam_angle_forward;
                p += ret;
                break;

            /* 09/28/98 jsb - added vertical error subrecord */
            case (GSF_SWATH_BATHY_SUBRECORD_VERTICAL_ERROR_ARRAY):
                ret = DecodeTwoByteArray(&ft->rec.mb_ping.vertical_error, p, ping->number_beams,
                    &ft->rec.mb_ping.scaleFactors, GSF_SWATH_BATHY_SUBRECORD_VERTICAL_ERROR_ARRAY, handle);
                if (ret < 0)
                {
                    return (-1);
                }
                ping->vertical_error =  ft->rec.mb_ping.vertical_error;
                p += ret;
                break;

            /* 09/28/98 jsb - added horizontal error subrecord */
            case (GSF_SWATH_BATHY_SUBRECORD_HORIZONTAL_ERROR_ARRAY):
                ret = DecodeTwoByteArray(&ft->rec.mb_ping.horizontal_error, p, ping->number_beams,
                    &ft->rec.mb_ping.scaleFactors, GSF_SWATH_BATHY_SUBRECORD_HORIZONTAL_ERROR_ARRAY, handle);
                if (ret < 0)
                {
                    return (-1);
                }
                ping->horizontal_error =  ft->rec.mb_ping.horizontal_error;
                p += ret;
                break;

            case (GSF_SWATH_BATHY_SUBRECORD_SEABEAM_SPECIFIC):
                p += DecodeSeabeamSpecific(&ping->sensor_data, p);
                ping->sensor_id = GSF_SWATH_BATHY_SUBRECORD_SEABEAM_SPECIFIC;
                break;

            case (GSF_SWATH_BATHY_SUBRECORD_EM12_SPECIFIC):
                p += DecodeEM12Specific(&ping->sensor_data, p);
                ping->sensor_id = GSF_SWATH_BATHY_SUBRECORD_EM12_SPECIFIC;
                break;

            case (GSF_SWATH_BATHY_SUBRECORD_EM100_SPECIFIC):
                p += DecodeEM100Specific(&ping->sensor_data, p);
                ping->sensor_id = GSF_SWATH_BATHY_SUBRECORD_EM100_SPECIFIC;
                break;

            case (GSF_SWATH_BATHY_SUBRECORD_EM950_SPECIFIC):
                p += DecodeEM950Specific(&ping->sensor_data, p);
                ping->sensor_id = GSF_SWATH_BATHY_SUBRECORD_EM950_SPECIFIC;
                break;

            case (GSF_SWATH_BATHY_SUBRECORD_EM121A_SPECIFIC):
                p += DecodeEM121ASpecific(&ping->sensor_data, p);
                ping->sensor_id = GSF_SWATH_BATHY_SUBRECORD_EM121A_SPECIFIC;
                break;

            case (GSF_SWATH_BATHY_SUBRECORD_EM121_SPECIFIC):
                p += DecodeEM121Specific(&ping->sensor_data, p);
                ping->sensor_id = GSF_SWATH_BATHY_SUBRECORD_EM121_SPECIFIC;
                break;

            case (GSF_SWATH_BATHY_SUBRECORD_SASS_SPECIFIC):
                p += DecodeSASSSpecific(&ping->sensor_data, p);
                ping->sensor_id = GSF_SWATH_BATHY_SUBRECORD_SASS_SPECIFIC;
                break;

            case (GSF_SWATH_BATHY_SUBRECORD_SEAMAP_SPECIFIC):
                p += DecodeSeaMapSpecific(&ping->sensor_data, p);
                ping->sensor_id = GSF_SWATH_BATHY_SUBRECORD_SEAMAP_SPECIFIC;
                break;

            case (GSF_SWATH_BATHY_SUBRECORD_SEABAT_SPECIFIC):
                p += DecodeSeaBatSpecific(&ping->sensor_data, p);
                ping->sensor_id = GSF_SWATH_BATHY_SUBRECORD_SEABAT_SPECIFIC;
                break;

            case (GSF_SWATH_BATHY_SUBRECORD_EM1000_SPECIFIC):
                p += DecodeEM1000Specific(&ping->sensor_data, p);
                ping->sensor_id = GSF_SWATH_BATHY_SUBRECORD_EM1000_SPECIFIC;
                break;

                #if 1
                /* 3-30-99 wkm: obsolete */
            case (GSF_SWATH_BATHY_SUBRECORD_TYPEIII_SEABEAM_SPECIFIC):
                p += DecodeTypeIIISeaBeamSpecific(&ping->sensor_data, p);
                ping->sensor_id =
                  GSF_SWATH_BATHY_SUBRECORD_TYPEIII_SEABEAM_SPECIFIC;
                break;
                #endif

            case (GSF_SWATH_BATHY_SUBRECORD_SB_AMP_SPECIFIC):
                p += DecodeSBAmpSpecific(&ping->sensor_data, p);
                ping->sensor_id = GSF_SWATH_BATHY_SUBRECORD_SB_AMP_SPECIFIC;
                break;

            case (GSF_SWATH_BATHY_SUBRECORD_SEABAT_II_SPECIFIC):
                p += DecodeSeaBatIISpecific(&ping->sensor_data, p);
                ping->sensor_id = GSF_SWATH_BATHY_SUBRECORD_SEABAT_II_SPECIFIC;
                break;

            case (GSF_SWATH_BATHY_SUBRECORD_SEABAT_8101_SPECIFIC):
                p += DecodeSeaBat8101Specific(&ping->sensor_data, p);
                ping->sensor_id = GSF_SWATH_BATHY_SUBRECORD_SEABAT_8101_SPECIFIC;
                break;

            case (GSF_SWATH_BATHY_SUBRECORD_SEABEAM_2112_SPECIFIC):
                p += DecodeSeaBeam2112Specific(&ping->sensor_data, p);
                ping->sensor_id = GSF_SWATH_BATHY_SUBRECORD_SEABEAM_2112_SPECIFIC;
                break;

            case (GSF_SWATH_BATHY_SUBRECORD_ELAC_MKII_SPECIFIC):
                p += DecodeElacMkIISpecific(&ping->sensor_data, p);
                ping->sensor_id = GSF_SWATH_BATHY_SUBRECORD_ELAC_MKII_SPECIFIC;
                break;

            case (GSF_SWATH_BATHY_SUBRECORD_CMP_SASS_SPECIFIC):
                p += DecodeCmpSassSpecific(&ping->sensor_data, p);
                ping->sensor_id = GSF_SWATH_BATHY_SUBRECORD_CMP_SASS_SPECIFIC;
                break;

            case (GSF_SWATH_BATHY_SUBRECORD_EM3000_SPECIFIC):
                p += DecodeEM3Specific(&ping->sensor_data, p, ft);
                ping->sensor_id = GSF_SWATH_BATHY_SUBRECORD_EM3000_SPECIFIC;
                break;

            case (GSF_SWATH_BATHY_SUBRECORD_EM1002_SPECIFIC):
                p += DecodeEM3Specific(&ping->sensor_data, p, ft);
                ping->sensor_id = GSF_SWATH_BATHY_SUBRECORD_EM1002_SPECIFIC;
                break;

             case (GSF_SWATH_BATHY_SUBRECORD_EM300_SPECIFIC):
                p += DecodeEM3Specific(&ping->sensor_data, p, ft);
                ping->sensor_id = GSF_SWATH_BATHY_SUBRECORD_EM300_SPECIFIC;
                break;

              default:
                gsfError = GSF_UNRECOGNIZED_SUBRECORD_ID;
                if ((((p - sptr) + subrecord_size) == record_size) ||
                    ((record_size - ((p - sptr) + subrecord_size)) > 0))
                    p+=subrecord_size;
                else
                    return (-1);
                break;
        }
        bytes = p - sptr;
    }

    /*  Extract subrecord id if the subrecord size is 0 */
    if (((record_size - bytes) == 4) && (ping->sensor_id != subrecord_id))
    {
        /* First four byte integer in subrecord contains the subrecord
        *  size and subrecord identifier.
        */
        memcpy(&ltemp, p, 4);
        p += 4;
        ltemp = ntohl(ltemp);
        subrecord_id = (ltemp & 0xFF000000) >> 24;
        subrecord_size = ltemp & 0x00FFFFFF;

        ping->sensor_id = subrecord_id;
    }

    /* Return the number of byte written into the buffer */
    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : DecodeScaleFactors
 *
 * Description :
 *  This function decodes the ping scale factor subrecord from external byte
 *  stream form to internal form.
 *
 * Inputs :
 *    ping = a pointer to the gsf swath bathymetry ping structure into which
 *           the scale factors will be loaded.
 *    sptr = a pointer to an unsigned char containing the byte stream to read
 *
 * Returns :
 *  This function returns the number of bytes decoded if successful, or
 *  -1 if an error occured.
 *
 * Error Conditions :
 *    GSF_UNRECOGNIZED_ARRAY_SUBRECORD_ID
 *
 ********************************************************************/

static int
DecodeScaleFactors(gsfScaleFactors *sf, unsigned char *sptr)
{
    unsigned char  *p = sptr;
    gsfuLong        ltemp;
    int             i;
    int             subrecordID;
    int             stemp;

    /* First four byte integer contains the number of scale factors */
    memcpy(&ltemp, p, 4);
    sf->numArraySubrecords = ntohl(ltemp);
    p += 4;

    /* Loop to decode each scale factor */
    for (i = 0; i < sf->numArraySubrecords; i++)
    {
        /* First four byte integer has the scaled array subrecord id in the
         *  first byte, the compression flags in the second byte, and the
         *  two lower order bytes are reserved.
         */
        memcpy(&ltemp, p, 4);
        p += 4;
        ltemp = ntohl(ltemp);
        subrecordID = (ltemp & 0xFF000000) >> 24;
        if ((subrecordID < 1) || (subrecordID > GSF_MAX_PING_ARRAY_SUBRECORDS))
        {
            gsfError = GSF_UNRECOGNIZED_ARRAY_SUBRECORD_ID;
            return (-1);
        }
        sf->scaleTable[subrecordID - 1].compressionFlag = (ltemp & 0x00FF0000) >> 16;

        /* decode the scale factor multiplier */
        memcpy(&ltemp, p, 4);
        p += 4;
        ltemp = ntohl(ltemp);

        sf->scaleTable[subrecordID - 1].multiplier = ((double) ltemp);

        /* decode the scale factor offset */
        memcpy(&ltemp, p, 4);
        p += 4;
        stemp = (signed) ntohl (ltemp);
        sf->scaleTable[subrecordID - 1].offset = (double) stemp;
    }
    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : DecodeTwoByteArray
 *
 * Description :
 *  This function decodes a two byte array of beam data from external byte
 *   stream form to internal engineering units form.  This function allocates
 *   the memory for the array if it does not exist.  This function also
 *   reallocates the memory for the array when the number of beams changes.
 *
 * Inputs :
 *    array = the address of a pointer to a double where the array of data
 *            will be stored
 *    sptr = a pointer to the unsigned char buffer containing the byte stream
 *           to read from.
 *    num_beams = an integer containing the number of beams which is used to
 *                dimension the array
 *    sf = a pointer to the scale factors structure containing the data scaling
 *         information
 *    id = the integer id for the subrecord, which is used as the index into
 *         the scale factors structure.
 *    handle = the integer handle for the data file being read, which is used
 *             to store the current number of beams
 *
 * Returns :
 *  This function returns the number of bytes decoded if successful, or
 *  -1 if an error occured.
 *
 * Error Conditions :
 *    GSF_ILLEGAL_SCALE_FACTOR_MULTIPLIER
 *    GSF_INVALID_NUM_BEAMS
 *    GSF_MEMORY_ALLOCATION_FAILED
 *
 ********************************************************************/

static int
DecodeTwoByteArray(double **array, unsigned char *sptr, int num_beams,
    gsfScaleFactors * sf, int id, int handle)
{
    double         *dptr;
    unsigned char  *ptr = sptr;
    gsfuShort       stemp;
    unsigned short  temp;
    int             i;

    /* make sure we have a scale factor multiplier */
    if (sf->scaleTable[id - 1].multiplier < 1.0e-6)
    {
        gsfError = GSF_ILLEGAL_SCALE_FACTOR_MULTIPLIER;
        return (-1);
    }

    /* Allocate memory for the array if none has been allocated yet */
    if (*array == (double *) NULL)
    {
        if (num_beams <= 0)
        {
            gsfError = GSF_INVALID_NUM_BEAMS;
            return(-1);
        }

        *array = (double *) calloc(num_beams, sizeof(double));

        if (*array == (double *) NULL)
        {
            gsfError = GSF_MEMORY_ALLOCATION_FAILED;
            return (-1);
        }
        arraySize[handle - 1][id - 1] = num_beams;
    }

    /* Make sure the memory allocated for the array is sufficient, some
    *  systems have a dynamic number of beams depending on depth
    */
    if (num_beams > arraySize[handle - 1][id - 1])
    {
        *array = (double *) realloc((void *) *array, num_beams * sizeof(double));

        if (*array == (double *) NULL)
        {
            gsfError = GSF_MEMORY_ALLOCATION_FAILED;
            return (-1);
        }
        memset(*array, 0, num_beams * sizeof(double));

        arraySize[handle - 1][id - 1] = num_beams;
    }

    dptr = *array;

    /* loop for the number of beams, loading each value from the byte stream
    *  into internal form
    */
    for (i = 0; i < num_beams; i++)
    {
        memcpy(&stemp, ptr, 2);
        temp = (unsigned short) ntohs(stemp);
        *dptr = (((double) temp) /
            sf->scaleTable[id - 1].multiplier) -
            sf->scaleTable[id - 1].offset;
        ptr += 2;
        dptr++;
    }

    return (ptr - sptr);
}

/********************************************************************
 *
 * Function Name : DecodeSignedTwoByteArray
 *
 * Description :
 *  This function decodes a two byte array of beam data from external byte
 *   stream form to internal engineering units form.  This function allocates
 *   the memory for the array if it does not exist.  This function also
 *   reallocates the memory for the array when the number of beams changes.
 *
 * Inputs :
 *    array = the address of a pointer to a double where the array of data
 *            will be stored
 *    sptr = a pointer the char buffer containing the byte stream
 *           to read from.
 *    num_beams = an integer containing the number of beams which is used to
 *                dimension the array
 *    sf = a pointer to the scale factors structure containing the data scaling
 *         information
 *    id = the integer id for the subrecord, which is used as the index into
 *         the scale factors structure.
 *    handle = the integer handle for the data file being read, which is used
 *             to store the current number of beams
 *
 * Returns :
 *  This function returns the number of bytes decoded if successful, or
 *  -1 if an error occured.
 *
 * Error Conditions :
 *    GSF_ILLEGAL_SCALE_FACTOR_MULTIPLIER
 *    GSF_INVALID_NUM_BEAMS
 *    GSF_MEMORY_ALLOCATION_FAILED
 *
 ********************************************************************/

static int
DecodeSignedTwoByteArray(double **array, char *sptr, int num_beams,
    gsfScaleFactors * sf, int id, int handle)
{
    double         *dptr;
    char           *ptr = sptr;
    gsfuShort       stemp;
    gsfsShort       signed_temp;
    int             i;

    /* make sure we have a scale factor multiplier */
    if (sf->scaleTable[id - 1].multiplier < 1.0e-6)
    {
        gsfError = GSF_ILLEGAL_SCALE_FACTOR_MULTIPLIER;
        return (-1);
    }

    /* Allocate memory for the array if none has been allocated yet */
    if (*array == (double *) NULL)
    {
        if (num_beams <= 0)
        {
            gsfError = GSF_INVALID_NUM_BEAMS;
            return(-1);
        }

        *array = (double *) calloc(num_beams, sizeof(double));

        if (*array == (double *) NULL)
        {
            gsfError = GSF_MEMORY_ALLOCATION_FAILED;
            return (-1);
        }
        arraySize[handle - 1][id - 1] = num_beams;
    }

    /* Make sure the memory allocated for the array is sufficient, some
    *  systems have a dynamic number of beams depending on depth
    */
    if (num_beams > arraySize[handle - 1][id - 1])
    {
        *array = (double *) realloc((void *) *array, num_beams * sizeof(double));

        if (*array == (double *) NULL)
        {
            gsfError = GSF_MEMORY_ALLOCATION_FAILED;
            return (-1);
        }
        memset(*array, 0, num_beams * sizeof(double));

        arraySize[handle - 1][id - 1] = num_beams;
    }

    dptr = *array;

    /* loop for the number of beams, loading each value from the byte stream
    *  into internal form
    */
    for (i = 0; i < num_beams; i++)
    {
        memcpy(&stemp, ptr, 2);
        signed_temp = (gsfsShort) ntohs(stemp);
        *dptr = (((double) signed_temp) /
            sf->scaleTable[id - 1].multiplier) -
            sf->scaleTable[id - 1].offset;
        ptr += 2;
        dptr++;
    }
    return (ptr - sptr);
}

/********************************************************************
 *
 * Function Name : DecodeByteArray
 *
 * Description :
 *  This function decodes a byte array of beam data from external byte
 *   stream form to internal engineering units form.  This function allocates
 *   the memory for the array if it does not exist.  This function also
 *   reallocates the memory for the array when the number of beams changes.
 *
 * Inputs :
 *    array = the address of a pointer to a double where the array of data
 *            will be stored
 *    sptr = a pointer the unsigned char buffer containing the byte stream
 *           to read from.
 *    num_beams = an integer containing the number of beams which is used to
 *                dimension the array
 *    sf = a pointer to the scale factors structure containing the data scaling
 *         information
 *    id = the integer id for the subrecord, which is used as the index into
 *         the scale factors structure.
 *    handle = the integer handle for the data file being read, which is used
 *             to store the current number of beams
 *
 * Returns :
 *  This function returns the number of bytes decoded if successful, or
 *  -1 if an error occured.
 *
 * Error Conditions :
 *    GSF_ILLEGAL_SCALE_FACTOR_MULTIPLIER
 *    GSF_INVALID_NUM_BEAMS
 *    GSF_MEMORY_ALLOCATION_FAILED
 *
 ********************************************************************/

static int
DecodeByteArray(double **array, unsigned char *sptr, int num_beams,
    gsfScaleFactors * sf, int id, int handle)
{
    double         *dptr;
    unsigned char  *ptr = sptr;
    unsigned char   ctemp;
    int             i;

    /* make sure we have a scale factor multiplier */
    if (sf->scaleTable[id - 1].multiplier < 1.0e-6)
    {
        gsfError = GSF_ILLEGAL_SCALE_FACTOR_MULTIPLIER;
        return(-1);
    }

    /* Allocate memory for the array if none has been allocated yet */
    if (*array == (double *) NULL)
    {
        if (num_beams <= 0)
        {
            gsfError = GSF_INVALID_NUM_BEAMS;
            return(-1);
        }

        *array = (double *) calloc(num_beams, sizeof(double));

        if (*array == (double *) NULL)
        {
            gsfError = GSF_MEMORY_ALLOCATION_FAILED;
            return (-1);
        }
        arraySize[handle - 1][id - 1] = num_beams;
    }

    /* Make sure there memory allocated for the array is sufficient, some
    *  system have a different number of beams depending on depth
    */
    if (num_beams > arraySize[handle - 1][id - 1])
    {
        *array = (double *) realloc((void *) *array, num_beams * sizeof(double));

        if (*array == (double *) NULL)
        {
            gsfError = GSF_MEMORY_ALLOCATION_FAILED;
            return (-1);
        }
        memset(*array, 0, num_beams * sizeof(double));

        arraySize[handle - 1][id - 1] = num_beams;
    }

    dptr = *array;

    /* loop for the number of beams, loading each value from the byte stream
     *  into internal form
     */
    for (i = 0; i < num_beams; i++)
    {
        ctemp = *ptr;
        *dptr = (((double) ctemp) /
            sf->scaleTable[id - 1].multiplier) -
            sf->scaleTable[id - 1].offset;
        ptr++;
        dptr++;
    }
    return (ptr - sptr);
}

/********************************************************************
 *
 * Function Name : DecodeSignedByteArray
 *
 * Description :
 *  This function decodes a byte array of beam data from external byte
 *   stream form to internal engineering units form.  This function allocates
 *   the memory for the array if it does not exist.  This function also
 *   reallocates the memory for the array when the number of beams changes.
 *
 * Inputs :
 *    array = the address of a pointer to a double where the array of data
 *            will be stored
 *    sptr = a pointer the char buffer containing the byte stream
 *           to read from.
 *    num_beams = an integer containing the number of beams which is used to
 *                dimension the array
 *    sf = a pointer to the scale factors structure containing the data scaling
 *         information
 *    id = the integer id for the subrecord, which is used as the index into
 *         the scale factors structure.
 *    handle = the integer handle for the data file being read, which is used
 *             to store the current number of beams
 *
 * Returns :
 *  This function returns the number of bytes decoded if successful, or
 *  -1 if an error occured.
 *
 * Error Conditions :
 *    GSF_ILLEGAL_SCALE_FACTOR_MULTIPLIER
 *    GSF_INVALID_NUM_BEAMS
 *    GSF_MEMORY_ALLOCATION_FAILED
 *
 ********************************************************************/
static int
DecodeSignedByteArray(double **array, char *sptr, int num_beams,
    gsfScaleFactors * sf, int id, int handle)
{
    double         *dptr;
    char           *ptr = sptr;
    char            ctemp;
    int             i;

    /* make sure we have a scale factor multiplier */
    if (sf->scaleTable[id - 1].multiplier < 1.0e-6)
    {
        gsfError = GSF_ILLEGAL_SCALE_FACTOR_MULTIPLIER;
    }

    /* Allocate memory for the array if none has been allocated yet */
    if (*array == (double *) NULL)
    {
        if (num_beams <= 0)
        {
            gsfError = GSF_INVALID_NUM_BEAMS;
            return(-1);
        }

        *array = (double *) calloc(num_beams, sizeof(double));

        if (*array == (double *) NULL)
        {
            gsfError = GSF_MEMORY_ALLOCATION_FAILED;
            return (-1);
        }
        arraySize[handle - 1][id - 1] = num_beams;
    }

    /* Make sure there memory allocated for the array is sufficient, some
    *  system have a different number of beams depending on depth
    */
    if (num_beams > arraySize[handle - 1][id - 1])
    {
        *array = (double *) realloc((void *) *array, num_beams * sizeof(double));

        if (*array == (double *) NULL)
        {
            gsfError = GSF_MEMORY_ALLOCATION_FAILED;
            return (-1);
        }
        memset(*array, 0, num_beams * sizeof(double));

        arraySize[handle - 1][id - 1] = num_beams;
    }

    dptr = *array;

    /* loop for the number of beams, loading each value from the byte stream
    *  into internal form
    */
    for (i = 0; i < num_beams; i++)
    {
        ctemp = *ptr;
        *dptr = (((double) ctemp) /
            sf->scaleTable[id - 1].multiplier) -
            sf->scaleTable[id - 1].offset;
        ptr++;
        dptr++;
    }
    return (ptr - sptr);
}

/********************************************************************
 *
 * Function Name : DecodeBeamFlagsArray
 *
 * Description :
 *  This function is used to decode the ping flags array from gsf byte stream
 *  form into internal forma.
 *
 * Inputs :
 *    array = the address of a pointer to an unsigned char to be loaded with
 *            the array of beam data
 *    sptr = a pointer to an unsigned char containing the byte stream to read
 *    num_beams = an integer containing the number of beams
 *    handle = an integer containing the handle for this file, used to record
 *             the number of beams for memory reallocation purposes.
 *
 * Returns :
 *  This function returns the number of bytes decoded if successful, or
 *  -1 if an error occured.
 *
 * Error Conditions :
 *    GSF_INVALID_NUM_BEAMS
 *    GSF_MEMORY_ALLOCATION_FAILED
 *
 ********************************************************************/

static int
DecodeBeamFlagsArray(unsigned char **array, unsigned char *sptr, int num_beams, int handle)
{
    unsigned char  *ptr = sptr;
    unsigned char  *aptr;
    int             i;
    int             id = GSF_SWATH_BATHY_SUBRECORD_BEAM_FLAGS_ARRAY;

    /* Allocate memory for the array if none has been allocated yet */
    if (*array == (unsigned char *) NULL)
    {
        if (num_beams <= 0)
        {
            gsfError = GSF_INVALID_NUM_BEAMS;
            return(-1);
        }

        *array = (unsigned char *) calloc(num_beams, sizeof(unsigned char));

        if (*array == (unsigned char *) NULL)
        {
            gsfError = GSF_MEMORY_ALLOCATION_FAILED;
            return (-1);
        }
        arraySize[handle - 1][id - 1] = num_beams;
    }

    /* Make sure there memory allocated for the array is sufficient, some
    *  system have a different number of beams depending on depth
    */
    if (num_beams > arraySize[handle - 1][id - 1])
    {
        *array = (unsigned char *) realloc((void *) *array, num_beams * sizeof(unsigned char));

        if (*array == (unsigned char *) NULL)
        {
            gsfError = GSF_MEMORY_ALLOCATION_FAILED;
            return (-1);
        }
        memset(*array, 0, num_beams * sizeof(unsigned char));

        arraySize[handle - 1][id - 1] = num_beams;
    }

    /* loop for the number of beams, loading each value from the byte stream
    *  into internal form
    */
    aptr = *array;
    for (i = 0; i < num_beams; i++)
    {
        *aptr = *ptr;
        ptr++;
        aptr++;
    }
    return (ptr - sptr);
}

/********************************************************************
 *
 * Function Name : DecodeQualityFlagsArray
 *
 * Description :
 *  This function is used to decode the two bit beam detection quality flags
 *  provided by Reson sonar systems.
 *
 * Inputs :
 *    array = the address of a pointer to an unsigned char to be loaded with
 *            the array of beam data
 *    sptr = a pointer to an unsigned char containing the byte stream to read
 *    num_beams = an integer containing the number of beams
 *    handle = an integer containing the handle for this file, used to record
 *             the number of beams for memory reallocation purposes.
 *
 * Returns :
 *  This function returns the number of bytes decoded if successful, or
 *  -1 if an error occured.
 *
 * Error Conditions :
 *    GSF_INVALID_NUM_BEAMS
 *    GSF_MEMORY_ALLOCATION_FAILED
 *
 ********************************************************************/
static int
DecodeQualityFlagsArray(unsigned char **array, unsigned char *sptr, int num_beams, int handle)
{
    unsigned char  *ptr = sptr;
    unsigned char  *aptr;
    int             i;
    int             j;
    int             shift;
    unsigned char   mask[4];
    int             id = GSF_SWATH_BATHY_SUBRECORD_QUALITY_FLAGS_ARRAY;

    /* Allocate memory for the array if none has been allocated yet */
    if (*array == (unsigned char *) NULL)
    {
        if (num_beams <= 0)
        {
            gsfError = GSF_INVALID_NUM_BEAMS;
            return(-1);
        }

        *array = (unsigned char *) calloc(num_beams, sizeof(unsigned char));

        if (*array == (unsigned char *) NULL)
        {
            gsfError = GSF_MEMORY_ALLOCATION_FAILED;
            return (-1);
        }
        arraySize[handle - 1][id - 1] = num_beams;
    }

    /* Make sure there memory allocated for the array is sufficient, some
    *  system have a different number of beams depending on depth
    */
    if (num_beams > arraySize[handle - 1][id - 1])
    {
        *array = (unsigned char *) realloc((void *) *array, num_beams * sizeof(unsigned char));

        if (*array == (unsigned char *) NULL)
        {
            gsfError = GSF_MEMORY_ALLOCATION_FAILED;
            return (-1);
        }
        memset(*array, 0, num_beams * sizeof(unsigned char));

        arraySize[handle - 1][id - 1] = num_beams;
    }

    /* Unpack the array values */
    shift = 6;
    aptr = *array;
    j = 0;
    mask[0] = 192;   /* bits 7 and 6 */
    mask[1] =  48;   /* bits 5 and 4 */
    mask[2] =  12;   /* bits 3 and 2 */
    mask[3] =   3;   /* bits 1 and 0 */
    for (i = 0; i < num_beams; i++)
    {
        *aptr = (*ptr & mask[j]) >> shift;
        aptr++;

        if (shift == 0)
        {
            ptr++;
            shift = 6;
            j = 0;
        }
        else
        {
            j++;
            shift -= 2;
        }
    }

    return (ptr - sptr);
}

/********************************************************************
 *
 * Function Name : DecodeSeabeamSpecific
 *
 * Description : This function decodes the SeaBeam specific ping subrecord
 *  from external byte stream form into internal form.
 *
 * Inputs :
 *    sdata = a pointer to the union of sensor specific data to be loaded
 *    sptr = a pointer to an unsigned char buffer containing the byte stream
 *           to read.
 *
 * Returns : This function returns the number of bytes enocoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
DecodeSeabeamSpecific(gsfSensorSpecific * sdata, unsigned char *sptr)
{
    gsfuShort       stemp;

    memcpy(&stemp, sptr, 2);
    sdata->gsfSeaBeamSpecific.EclipseTime = (unsigned short) ntohs(stemp);

    return (2);
}

/********************************************************************
 *
 * Function Name : DecodeEM12Specific
 *
 * Description : Not Implimented yet
 *
 * Inputs :
 *    sdata = a pointer to the union of sensor specific data to be loaded
 *    sptr = a pointer to an unsigned char buffer containing the byte stream
 *           to read.
 *
 * Returns : This function returns the number of bytes enocoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
DecodeEM12Specific(gsfSensorSpecific * sdata, unsigned char *sptr)
{
    return (0);
}

/********************************************************************
 *
 * Function Name : DecodeEm100Specific
 *
 * Description : This function decodes the simrad em100 specific ping subrecord
 *    from external byte stream form into internal form.
 *
 * Inputs :
 *    sdata = a pointer to the union of sensor specific data to be loaded
 *    sptr = a pointer to an unsigned char buffer containing the byte stream
 *           to read.
 *
 * Returns : This function returns the number of bytes enocoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
DecodeEM100Specific(gsfSensorSpecific * sdata, unsigned char *sptr)
{
    unsigned char  *p = sptr;
    gsfsShort       signed_short;
    gsfuShort       stemp;

    /* First two byte integer contains the ship pitch */
    memcpy(&stemp, p, 2);
    signed_short = (signed) ntohs(stemp);
    sdata->gsfEM100Specific.ship_pitch = ((double) signed_short) / 100.0;
    p += 2;

    /* Next two byte integer contains the transducer pitch */
    memcpy(&stemp, p, 2);
    signed_short = (signed) ntohs(stemp);
    sdata->gsfEM100Specific.transducer_pitch = ((double) signed_short) / 100.0;
    p += 2;

    /* Next byte contains the sonar mode (from the em100 amplitude datagram) */
    sdata->gsfEM100Specific.mode = (int) *p;
    p += 1;

    /* Next byte contains the power (from the em100 amplitude datagram) */
    sdata->gsfEM100Specific.power = (int) *p;
    p += 1;

    /* Next byte contains the attenuation (from the em100 amplitude datagram) */
    sdata->gsfEM100Specific.attenuation = (int) *p;
    p += 1;

    /* Next byte contains the tvg (from the em100 amplitude datagram) */
    sdata->gsfEM100Specific.tvg = (int) *p;
    p += 1;

    /* Next byte contains the pulse length from the em100 amplitude datagram) */
    sdata->gsfEM100Specific.pulse_length = (int) *p;
    p += 1;

    /* Next two byte integer contains the counter from the em100
    * amplitude datagram
    */
    memcpy(&stemp, p, 2);
    p += 2;
    sdata->gsfEM100Specific.counter = (int) ntohs(stemp);

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : DecodeEm950Specific
 *
 * Description :
 *
 * Inputs :
 *    sdata = a pointer to the union of sensor specific data to be loaded
 *    sptr = a pointer to an unsigned char buffer containing the byte stream
 *           to read.
 *
 * Returns : This function returns the number of bytes enocoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
DecodeEM950Specific(gsfSensorSpecific * sdata, unsigned char *sptr)
{
    unsigned char  *p = sptr;
    gsfuShort       stemp;
    gsfsShort       signed_short;

    /* First two byte integer contains the ping number */
    memcpy(&stemp, p, 2);
    sdata->gsfEM950Specific.ping_number = (int) ntohs(stemp);
    p += 2;

    /* Next byte contains the sonar mode of operation */
    sdata->gsfEM950Specific.mode = (int) *p;
    p += 1;

    /* Next byte contains the ping quality factor*/
    sdata->gsfEM950Specific.ping_quality = (char) *p;
    p += 1;

    /* Next two byte integer contains the transducer pitch */
    memcpy(&stemp, p, 2);
    signed_short = (signed) ntohs(stemp);
    p += 2;
    sdata->gsfEM950Specific.ship_pitch = ((double) signed_short) / 100.0;

    /* Next two byte integer contains the transducer pitch */
    memcpy(&stemp, p, 2);
    signed_short = (signed) ntohs(stemp);
    p += 2;
    sdata->gsfEM950Specific.transducer_pitch = ((double) signed_short) / 100.0;

    /* Next two byte integer contains the sea surface sound speed * 10 */
    memcpy(&stemp, p, 2);
    sdata->gsfEM950Specific.surface_velocity = ((double) ntohs(stemp)) / 10.0;
    p += 2;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : DecodeEm1000Specific
 *
 * Description :
 *
 * Inputs :
 *    sdata = a pointer to the union of sensor specific data to be loaded
 *    sptr = a pointer to an unsigned char buffer containing the byte stream
 *           to read.
 *
 * Returns : This function returns the number of bytes enocoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
DecodeEM1000Specific(gsfSensorSpecific * sdata, unsigned char *sptr)
{
    unsigned char  *p = sptr;
    gsfuShort       stemp;
    gsfsShort       signed_short;

    /* First two byte integer contains the ping number */
    memcpy(&stemp, p, 2);
    sdata->gsfEM1000Specific.ping_number = (int) ntohs(stemp);
    p += 2;

    /* Next byte contains the sonar mode of operation */
    sdata->gsfEM1000Specific.mode = (int) *p;
    p += 1;

    /* Next byte contains the ping quality factor*/
    sdata->gsfEM1000Specific.ping_quality = (char) *p;
    p += 1;

    /* Next two byte integer contains the transducer pitch */
    memcpy(&stemp, p, 2);
    signed_short = (signed) ntohs(stemp);
    p += 2;
    sdata->gsfEM1000Specific.ship_pitch = ((double) signed_short) / 100.0;

    /* Next two byte integer contains the transducer pitch */
    memcpy(&stemp, p, 2);
    signed_short = (signed) ntohs(stemp);
    p += 2;
    sdata->gsfEM1000Specific.transducer_pitch = ((double) signed_short) / 100.0;

    /* Next two byte integer contains the sea surface sound speed * 10 */
    memcpy(&stemp, p, 2);
    sdata->gsfEM1000Specific.surface_velocity = ((double) ntohs(stemp)) / 10.0;
    p += 2;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : DecodeEm121Specific
 *
 * Description : This function decodes the simrad em121 specific ping
 *    subrecord from external byte stream form into internal form.
 *
 * Inputs :
 *    sdata = a pointer to the union of sensor specific data to be loaded
 *    sptr = a pointer to an unsigned char buffer containing the byte stream
 *           to read.
 *
 * Returns : This function returns the number of bytes decoded.
 *
 * Error Conditions : none
    return (0);
}

/********************************************************************
 *
 * Function Name : DecodeEm121ASpecific
 *
 * Description : This function decodes the simrad em121a specific ping
 *    subrecord from external byte stream form into internal form.
 *
 * Inputs :
 *    sdata = a pointer to the union of sensor specific data to be loaded
 *    sptr = a pointer to an unsigned char buffer containing the byte stream
 *           to read.
 *
 * Returns : This function returns the number of bytes decoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
DecodeEM121ASpecific(gsfSensorSpecific *sdata, unsigned char *sptr)
{
    unsigned char  *p = sptr;
    gsfuShort       stemp;

    /* First two byte integer contains the ping number */
    memcpy(&stemp, p, 2);
    sdata->gsfEM121ASpecific.ping_number = (int) ntohs(stemp);
    p += 2;

    /* Next byte contains the sonar mode of operation */
    sdata->gsfEM121ASpecific.mode = (int) *p;
    p += 1;

    /* Next byte contains the number of valid beams */
    sdata->gsfEM121ASpecific.valid_beams = (int) *p;
    p += 1;

    /* Next byte contains the transmit pulse length */
    sdata->gsfEM121ASpecific.pulse_length = (int) *p;
    p += 1;

    /* Next byte contains the sonar beam width */
    sdata->gsfEM121ASpecific.beam_width = (int) *p;
    p += 1;

    /* Next byte contains the transmit power level */
    sdata->gsfEM121ASpecific.tx_power = (int) *p;
    p += 1;

    /* Next byte contains the number of transmit channels NOT working */
    sdata->gsfEM121ASpecific.tx_status = (int) *p;
    p += 1;

    /* Next byte contains the number of receive channels NOT working */
    sdata->gsfEM121ASpecific.rx_status = (int) *p;
    p += 1;

    /* Next two byte integer contains the sea surface sound speed * 10 */
    memcpy(&stemp, p, 2);
    sdata->gsfEM121ASpecific.surface_velocity = ((double) ntohs(stemp)) / 10.0;
    p += 2;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : DecodeEm121Specific
 *
 * Description : This function decodes the simrad em121 specific ping
 *    subrecord from external byte stream form into internal form.
 *
 * Inputs :
 *    sdata = a pointer to the union of sensor specific data to be loaded
 *    sptr = a pointer to an unsigned char buffer containing the byte stream
 *           to read.
 *
 * Returns : This function returns the number of bytes decoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
DecodeEM121Specific(gsfSensorSpecific *sdata, unsigned char *sptr)
{
    unsigned char  *p = sptr;
    gsfuShort       stemp;

    /* First two byte integer contains the ping number */
    memcpy(&stemp, p, 2);
    sdata->gsfEM121Specific.ping_number = (int) ntohs(stemp);
    p += 2;

    /* Next byte contains the sonar mode of operation */
    sdata->gsfEM121Specific.mode = (int) *p;
    p += 1;

    /* Next byte contains the number of valid beams */
    sdata->gsfEM121Specific.valid_beams = (int) *p;
    p += 1;

    /* Next byte contains the transmit pulse length */
    sdata->gsfEM121Specific.pulse_length = (int) *p;
    p += 1;

    /* Next byte contains the sonar beam width */
    sdata->gsfEM121Specific.beam_width = (int) *p;
    p += 1;

    /* Next byte contains the transmit power level */
    sdata->gsfEM121Specific.tx_power = (int) *p;
    p += 1;

    /* Next byte contains the number of transmit channels NOT working */
    sdata->gsfEM121Specific.tx_status = (int) *p;
    p += 1;

    /* Next byte contains the number of receive channels NOT working */
    sdata->gsfEM121Specific.rx_status = (int) *p;
    p += 1;

    /* Next two byte integer contains the sea surface sound speed * 10 */
    memcpy(&stemp, p, 2);
    sdata->gsfEM121Specific.surface_velocity = ((double) ntohs(stemp)) / 10.0;
    p += 2;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : DecodeCmpSassSpecific
 *
 * Description : This function decodes the Compressed SASS specific ping
 *    subrecord from external byte stream form into internal form.
 *
 * Inputs :
 *    sdata = a pointer to the union of sensor specific data to be loaded
 *    sptr = a pointer to an unsigned char buffer containing the byte stream
 *           to read.
 *
 * Returns : This function returns the number of bytes enocoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
DecodeCmpSassSpecific(gsfSensorSpecific * sdata, unsigned char *sptr)
{
    unsigned char  *p = sptr;
    gsfuShort       stemp;

    memcpy(&stemp, p, 2);
    sdata->gsfCmpSassSpecific.lfreq = ((double) ntohs(stemp)) / 10.0;
    p += 2;

    memcpy(&stemp, p, 2);
    sdata->gsfCmpSassSpecific.lntens = ((double) ntohs(stemp)) / 10.0;
    p += 2;

    return (p - sptr);
}


#if 1
/* 3-30-99: obsolete */
/********************************************************************
 *
 * Function Name : DecodeSassSpecific
 *
 * Description : This function decodes the Typeiii SASS specific ping
 *    subrecord from external byte stream form into internal form.
 *
 * Inputs :
 *    sdata = a pointer to the union of sensor specific data to be loaded
 *    sptr = a pointer to an unsigned char buffer containing the byte stream
 *           to read.
 *
 * Returns : This function returns the number of bytes enocoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
DecodeSASSSpecific(gsfSensorSpecific * sdata, unsigned char *sptr)
{
    unsigned char  *p = sptr;
    gsfuShort       stemp;

    /* First two byte integer contains the leftmost beam */
    memcpy(&stemp, p, 2);
    sdata->gsfSASSSpecific.leftmost_beam = (int) ntohs(stemp);
    p += 2;

    /* Next two byte integer contains the rightmost beam */
    memcpy(&stemp, p, 2);
    sdata->gsfSASSSpecific.rightmost_beam = (int) ntohs(stemp);
    p += 2;

    /* Next two byte integer contains the total number of beams */
    memcpy(&stemp, p, 2);
    sdata->gsfSASSSpecific.total_beams = (int) ntohs(stemp);
    p += 2;

    /* Next two byte integer contains the navigation mode */
    memcpy(&stemp, p, 2);
    sdata->gsfSASSSpecific.nav_mode = (int) ntohs(stemp);
    p += 2;

    /* Next two byte integer contains the ping number */
    memcpy(&stemp, p, 2);
    sdata->gsfSASSSpecific.ping_number = (int) ntohs(stemp);
    p += 2;

    /* Next two byte integer contains the mission number */
    memcpy(&stemp, p, 2);
    sdata->gsfSASSSpecific.mission_number = (int) ntohs(stemp);
    p += 2;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : DecodeTypeIIISeaBeamSpecific
 *
 * Description : not implimented yet
 *
 * Inputs :
 *    sdata = a pointer to the union of sensor specific data to be loaded
 *    sptr = a pointer to an unsigned char buffer containing the byte stream
 *           to read.
 *
 * Returns : This function returns the number of bytes enocoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
DecodeTypeIIISeaBeamSpecific(gsfSensorSpecific * sdata, unsigned char *sptr)
{
    unsigned char  *p = sptr;
    gsfuShort       stemp;

    /* First two byte integer contains the leftmost beam */
    memcpy(&stemp, p, 2);
    sdata->gsfTypeIIISeaBeamSpecific.leftmost_beam = (int) ntohs(stemp);
    p += 2;

    /* Next two byte integer contains the rightmost beam */
    memcpy(&stemp, p, 2);
    sdata->gsfTypeIIISeaBeamSpecific.rightmost_beam = (int) ntohs(stemp);
    p += 2;

    /* Next two byte integer contains the total number of beams */
    memcpy(&stemp, p, 2);
    sdata->gsfTypeIIISeaBeamSpecific.total_beams = (int) ntohs(stemp);
    p += 2;

    /* Next two byte integer contains the navigation mode */
    memcpy(&stemp, p, 2);
    sdata->gsfTypeIIISeaBeamSpecific.nav_mode = (int) ntohs(stemp);
    p += 2;

    /* Next two byte integer contains the ping number */
    memcpy(&stemp, p, 2);
    sdata->gsfTypeIIISeaBeamSpecific.nav_mode = (int) ntohs(stemp);
    p += 2;

    /* Next two byte integer contains the mission number */
    memcpy(&stemp, p, 2);
    sdata->gsfTypeIIISeaBeamSpecific.nav_mode = (int) ntohs(stemp);
    p += 2;

    return (p - sptr);
}
#endif

/********************************************************************
 *
 * Function Name : DecodeSeaMapSpecific
 *
 * Description : not implimented yet
 *
 * Inputs :
 *    sdata = a pointer to the union of sensor specific data to be loaded
 *    sptr = a pointer to an unsigned char buffer containing the byte stream
 *           to read.
 *
 * Returns : This function returns the number of bytes enocoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
DecodeSeaMapSpecific(gsfSensorSpecific * sdata, unsigned char *sptr)
{
    unsigned char  *p = sptr;
    gsfuShort       stemp;

    memcpy(&stemp, p, 2);
    sdata->gsfSeamapSpecific.portTransmitter[0] = ((double) ntohs(stemp)) / 10.0;
    p += 2;

    memcpy(&stemp, p, 2);
    sdata->gsfSeamapSpecific.portTransmitter[1] = ((double) ntohs(stemp)) / 10.0;
    p += 2;

    memcpy(&stemp, p, 2);
    sdata->gsfSeamapSpecific.stbdTransmitter[0] = ((double) ntohs(stemp)) / 10.0;
    p += 2;

    memcpy(&stemp, p, 2);
    sdata->gsfSeamapSpecific.stbdTransmitter[1] = ((double) ntohs(stemp)) / 10.0;
    p += 2;

    memcpy(&stemp, p, 2);
    sdata->gsfSeamapSpecific.portGain = ((double) ntohs(stemp)) / 10.0;
    p += 2;

    memcpy(&stemp, p, 2);
    sdata->gsfSeamapSpecific.stbdGain = ((double) ntohs(stemp)) / 10.0;
    p += 2;

    memcpy(&stemp, p, 2);
    sdata->gsfSeamapSpecific.portPulseLength = ((double) ntohs(stemp)) / 10.0;
    p += 2;

    memcpy(&stemp, p, 2);
    sdata->gsfSeamapSpecific.stbdPulseLength = ((double) ntohs(stemp)) / 10.0;
    p += 2;

    memcpy(&stemp, p, 2);
    sdata->gsfSeamapSpecific.pressureDepth = ((double) ntohs(stemp)) / 10.0;
    p += 2;

    memcpy(&stemp, p, 2);
    sdata->gsfSeamapSpecific.altitude = ((double) ntohs(stemp)) / 10.0;
    p += 2;

    memcpy(&stemp, p, 2);
    sdata->gsfSeamapSpecific.temperature = ((double) ntohs(stemp)) / 10.0;
    p += 2;


    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : DecodeSeaBatSpecific
 *
 * Description : This function decodes the sensor specific subrecord for
 *    Reson SeaBat data.
 *
 * Inputs :
 *    sdata = a pointer to the union of sensor specific data to be loaded
 *    sptr = a pointer to an unsigned char buffer containing the byte stream
 *           to read.
 *
 * Returns : This function returns the number of bytes enocoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
DecodeSeaBatSpecific(gsfSensorSpecific * sdata, unsigned char *sptr)
{
    unsigned char  *p = sptr;
    gsfuShort       stemp;

    /* First two byte integer contains the ping number */
    memcpy(&stemp, p, 2);
    sdata->gsfSeaBatSpecific.ping_number = (int) ntohs(stemp);
    p += 2;

    /* Next two byte integer contains the sea surface sound speed * 10 */
    memcpy(&stemp, p, 2);
    sdata->gsfSeaBatSpecific.surface_velocity = ((double) ntohs(stemp)) / 10.0;
    p += 2;

    /* Next byte contains the sonar mode of operation */
    sdata->gsfSeaBatSpecific.mode = (int) *p;
    p += 1;

    /* Next byte contains the sonar range for this ping */
    sdata->gsfSeaBatSpecific.sonar_range = (int) *p;
    p += 1;

    /* Next byte contains the sonar transmit power for this ping */
    sdata->gsfSeaBatSpecific.transmit_power = (int) *p;
    p += 1;

    /* Next byte contains the sonar receive gain for this ping */
    sdata->gsfSeaBatSpecific.receive_gain = (int) *p;
    p += 1;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : DecodeSBAmpSpecific
 *
 * Description : This function decodes the sensor specific subrecord for
 *    Sea Beam with amplitude data.
 *
 * Inputs :
 *    sdata = a pointer to the union of sensor specific data to be loaded
 *    sptr = a pointer to an unsigned char buffer containing the byte stream
 *           to read.
 *
 * Returns : This function returns the number of bytes enocoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
DecodeSBAmpSpecific(gsfSensorSpecific * sdata, unsigned char *sptr)
{
    unsigned char  *p = sptr;
    gsfuShort       stemp;
    gsfuLong        ltemp;

    /* First byte contains the hour from the Eclipse */
    sdata->gsfSBAmpSpecific.hour = (int) *p;
    p += 1;

    /* Next byte contains the minutes from the Eclipse */
    sdata->gsfSBAmpSpecific.minute = (int) *p;
    p += 1;

    /* Next byte contains the seconds from the Eclipse */
    sdata->gsfSBAmpSpecific.second = (int) *p;
    p += 1;

    /* Next byte contains the hundredths of seconds from the Eclipse */
    sdata->gsfSBAmpSpecific.hundredths = (int) *p;
    p += 1;

    /* Next four byte integer contains the block number */
    memcpy(&ltemp, p, 4);
    sdata->gsfSBAmpSpecific.block_number = (int) ntohl(ltemp);
    p += 4;

    /* Next two byte integer contains the average gate depth */
    memcpy(&stemp, p, 2);
    sdata->gsfSBAmpSpecific.avg_gate_depth = (int) ntohl(stemp);
    p += 2;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : DecodeSeaBatIISpecific
 *
 * Description : This function decodes the Reson SeaBat II sensor specific
 *    information from the GSF byte stream.
 *
 * Inputs :
 *    sdata = a pointer to the union of sensor specific data to be loaded
 *    sptr = a pointer to an unsigned char buffer containing the byte stream
 *           to read.
 *
 * Returns : This function returns the number of bytes enocoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
DecodeSeaBatIISpecific(gsfSensorSpecific * sdata, unsigned char *sptr)
{
    unsigned char  *p = sptr;
    gsfuShort       stemp;

    /* First two byte integer contains the ping number */
    memcpy(&stemp, p, 2);
    sdata->gsfSeaBatIISpecific.ping_number = (int) ntohs(stemp);
    p += 2;

    /* Next two byte integer contains the sea surface sound speed * 10 */
    memcpy(&stemp, p, 2);
    sdata->gsfSeaBatIISpecific.surface_velocity = ((double) ntohs(stemp)) / 10.0;
    p += 2;

    /* Next two byte integer contains the sonar mode of operation */
    memcpy(&stemp, p, 2);
    sdata->gsfSeaBatIISpecific.mode = (int) ntohs(stemp);
    p += 2;

    /* Next two byte integer contains the range setting */
    memcpy(&stemp, p, 2);
    sdata->gsfSeaBatIISpecific.sonar_range = (int) ntohs(stemp);
    p += 2;

    /* Next two byte integer contains the power setting */
    memcpy(&stemp, p, 2);
    sdata->gsfSeaBatIISpecific.transmit_power = (int) ntohs(stemp);
    p += 2;

    /* Next two byte integer contains the gain setting */
    memcpy(&stemp, p, 2);
    sdata->gsfSeaBatIISpecific.receive_gain = (int) ntohs(stemp);
    p += 2;

    /* Next byte contains the fore/aft beamwidth */
    sdata->gsfSeaBatIISpecific.fore_aft_bw = ((double) *p) / 10.0;
    p += 1;

    /* Next byte contains the athwartships beamwidth */
    sdata->gsfSeaBatIISpecific.athwart_bw = ((double) *p) / 10.0;
    p += 1;

    /* Next four bytes are reserved for future growth */
    sdata->gsfSeaBatIISpecific.spare[0] = (char) *p;
    p += 1;
    sdata->gsfSeaBatIISpecific.spare[1] = (char) *p;
    p += 1;
    sdata->gsfSeaBatIISpecific.spare[2] = (char) *p;
    p += 1;
    sdata->gsfSeaBatIISpecific.spare[3] = (char) *p;
    p += 1;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : DecodeSeaBat8101Specific
 *
 * Description : This function decodes the Reson SeaBat 8101 sensor specific
 *    information from the GSF byte stream.
 *
 * Inputs :
 *    sdata = a pointer to the union of sensor specific data to be loaded
 *    sptr = a pointer to an unsigned char buffer containing the byte stream
 *           to read.
 *
 * Returns : This function returns the number of bytes enocoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
DecodeSeaBat8101Specific(gsfSensorSpecific * sdata, unsigned char *sptr)
{
    unsigned char  *p = sptr;
    gsfuShort       stemp;

    /* First two byte integer contains the ping number */
    memcpy(&stemp, p, 2);
    sdata->gsfSeaBat8101Specific.ping_number = (int) ntohs(stemp);
    p += 2;

    /* Next two byte integer contains the sea surface sound speed * 10 */
    memcpy(&stemp, p, 2);
    sdata->gsfSeaBat8101Specific.surface_velocity = ((double) ntohs(stemp)) / 10.0;
    p += 2;

    /* Next two byte integer contains the sonar mode of operation */
    memcpy(&stemp, p, 2);
    sdata->gsfSeaBat8101Specific.mode = (int) ntohs(stemp);
    p += 2;

    /* Next two byte integer contains the range setting */
    memcpy(&stemp, p, 2);
    sdata->gsfSeaBat8101Specific.range = (int) ntohs(stemp);
    p += 2;

    /* Next two byte integer contains the power setting */
    memcpy(&stemp, p, 2);
    sdata->gsfSeaBat8101Specific.power = (int) ntohs(stemp);
    p += 2;

    /* Next two byte integer contains the gain setting */
    memcpy(&stemp, p, 2);
    sdata->gsfSeaBat8101Specific.gain = (int) ntohs(stemp);
    p += 2;

    /* Next two byte integer contains the transmit pulse width */
    memcpy(&stemp, p, 2);
    sdata->gsfSeaBat8101Specific.pulse_width = (int) ntohs(stemp);
    p += 2;

    /* Next byte contains the tvg spreading coefficient */
    sdata->gsfSeaBat8101Specific.tvg_spreading = (int) *p;
    p += 1;

    /* Next byte contains the tvg absorption coefficient */
    sdata->gsfSeaBat8101Specific.tvg_absorption = (int) *p;
    p += 1;

    /* Next byte contains the fore/aft beamwidth */
    sdata->gsfSeaBat8101Specific.fore_aft_bw = ((double) *p) / 10.0;
    p += 1;

    /* Next byte contains the athwartships beamwidth */
    sdata->gsfSeaBat8101Specific.athwart_bw = ((double) *p) / 10.0;
    p += 1;

    /* Next two byte integer is reserved for future storage of the range
     * filter minimum value.
     */
    memcpy(&stemp, p, 2);
    sdata->gsfSeaBat8101Specific.range_filt_min = (int) ntohs(stemp);
    p += 2;

    /* Next two byte integer is reserved for future storage of the range
     * filter maximum value.
     */
    memcpy(&stemp, p, 2);
    sdata->gsfSeaBat8101Specific.range_filt_max = (int) ntohs(stemp);
    p += 2;

    /* Next two byte integer is reserved for future storage of the depth
     * filter minimum value.
     */
    memcpy(&stemp, p, 2);
    sdata->gsfSeaBat8101Specific.depth_filt_min = (int) ntohs(stemp);
    p += 2;

    /* Next two byte integer is reserved for future storage of the depth
     * filter maximum value.
     */
    memcpy(&stemp, p, 2);
    sdata->gsfSeaBat8101Specific.depth_filt_max = (int) ntohs(stemp);
    p += 2;

    /* Next byte is reserved for future storage of the projector type */
    sdata->gsfSeaBat8101Specific.projector = (int) *p;
    p += 1;

    /* Next four bytes are reserved for future growth */
    sdata->gsfSeaBat8101Specific.spare[0] = (char) *p;
    p += 1;
    sdata->gsfSeaBat8101Specific.spare[1] = (char) *p;
    p += 1;
    sdata->gsfSeaBat8101Specific.spare[2] = (char) *p;
    p += 1;
    sdata->gsfSeaBat8101Specific.spare[3] = (char) *p;
    p += 1;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : DecodeSeaBeam2112Specific
 *
 * Description : This function decodes the Sea Beam 2112/36 sensor specific
 *    information from the GSF byte stream.
 *
 * Inputs :
 *    sdata = a pointer to the union of sensor specific data to be loaded
 *    sptr = a pointer to an unsigned char buffer containing the byte stream
 *           to read.
 *
 * Returns : This function returns the number of bytes enocoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
DecodeSeaBeam2112Specific(gsfSensorSpecific * sdata, unsigned char *sptr)
{
    unsigned char  *p = sptr;
    gsfuShort       stemp;

    /* First byte contains the sonar mode of operation */
    sdata->gsfSeaBeam2112Specific.mode = (int) *p;
    p += 1;

    /* Next two byte integer contains the sea surface sound speed * 100 - 130000 */
    memcpy(&stemp, p, 2);
    sdata->gsfSeaBeam2112Specific.surface_velocity = (((double) ntohs(stemp)) + 130000) / 100.0;
    p += 2;

    /* Next byte contains the SSV source */
    sdata->gsfSeaBeam2112Specific.ssv_source = (int) *p;
    p += 1;

    /* Next byte contains the ping gain */
    sdata->gsfSeaBeam2112Specific.ping_gain = (int) *p;
    p += 1;

    /* Next byte contains the ping pulse width */
    sdata->gsfSeaBeam2112Specific.pulse_width = (int) *p;
    p += 1;

    /* Next byte contains the transmitter attenuation */
    sdata->gsfSeaBeam2112Specific.transmitter_attenuation = (int) *p;
    p += 1;

    /* Next byte contains the number of algorithms */
    sdata->gsfSeaBeam2112Specific.number_algorithms = (int) *p;
    p += 1;

    /* Next 4 bytes contain the algorithm order */
    memset (sdata->gsfSeaBeam2112Specific.algorithm_order, 0, 5);
    memcpy (sdata->gsfSeaBeam2112Specific.algorithm_order, p, 4);
    p += 4;

    /* Next four bytes are reserved for future growth */
    sdata->gsfSeaBeam2112Specific.spare[0] = (char) *p;
    p += 1;
    sdata->gsfSeaBeam2112Specific.spare[1] = (char) *p;
    p += 1;

    return (p - sptr);
}


/********************************************************************
 *
 * Function Name : DecodeElacMkIISpecific
 *
 * Description : This function decodes the Elac Bottomchart MkII sensor specific
 *    information from the GSF byte stream.
 *
 * Inputs :
 *    sdata = a pointer to the union of sensor specific data to be loaded
 *    sptr = a pointer to an unsigned char buffer containing the byte stream
 *           to read.
 *
 * Returns : This function returns the number of bytes enocoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
DecodeElacMkIISpecific(gsfSensorSpecific * sdata, unsigned char *sptr)
{
    unsigned char   *p = sptr;
    gsfuShort       stemp;

    /* First byte contains the sonar mode of operation */
    sdata->gsfElacMkIISpecific.mode = (int) *p;
    p += 1;

    /* Next two byte integer contains the ping counter */
    memcpy(&stemp, p, 2);
    sdata->gsfElacMkIISpecific.ping_num = (int) ntohs(stemp);
    p += 2;

    /* Next two byte integer contains the surface sound velocity in m/s */
    memcpy(&stemp, p, 2);
    sdata->gsfElacMkIISpecific.sound_vel = (int) ntohs(stemp);
    p += 2;

    /* Next two byte integer contains the pulse length in 0.01 ms */
    memcpy(&stemp, p, 2);
    sdata->gsfElacMkIISpecific.pulse_length = (int) ntohs(stemp);
    p += 2;

    /* Next byte contains the starboard receiver gain in dB */
    sdata->gsfElacMkIISpecific.receiver_gain_stbd = (int) *p;
    p += 1;

    /* Next byte contains the port receiver gain in dB */
    sdata->gsfElacMkIISpecific.receiver_gain_port = (int) *p;
    p += 1;

    /* Next two byte integer is reserved for future use */
    memcpy(&stemp, p, 2);
    sdata->gsfElacMkIISpecific.reserved = (int) ntohs(stemp);
    p += 2;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : DecodeEM3Specific
 *
 * Description : This function decodes the Simrad EM3000 series sensor
 *    specific information from the GSF byte stream.
 *
 * Inputs :
 *    sdata = a pointer to the union of sensor specific data to be loaded
 *    sptr = a pointer to an unsigned char buffer containing the byte stream
 *           to read.
 *
 * Returns : This function returns the number of bytes enocoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

static int
DecodeEM3Specific(gsfSensorSpecific *sdata, unsigned char *sptr, GSF_FILE_TABLE *ft)
{
    unsigned char   *p = sptr;
    gsfuShort       stemp;
    gsfsShort       signed_short;
    gsfuLong        ltemp;
    int             run_time_id;

    /* The next two bytes contain the model number */
    memcpy(&stemp, p, 2);
    sdata->gsfEM3Specific.model_number = (int) ntohs(stemp);
    p += 2;

    /* The next two bytes contain the ping number */
    memcpy(&stemp, p, 2);
    sdata->gsfEM3Specific.ping_number = (int) ntohs(stemp);
    p += 2;

    /* The next two bytes contain the system 1 or 2 serial number */
    memcpy(&stemp, p, 2);
    sdata->gsfEM3Specific.serial_number = (int) ntohs(stemp);
    p += 2;

    /* The next two bytes contain the surface velocity */
    memcpy(&stemp, p, 2);
    sdata->gsfEM3Specific.surface_velocity = ((double) ntohs(stemp)) / 10.0;
    p += 2;

    /* The next two bytes contain the transmit depth */
    memcpy(&stemp, p, 2);
    sdata->gsfEM3Specific.transducer_depth = ((double) ntohs(stemp)) / 100.0;
    p += 2;

    /* The next two bytes contain the maximum number of beams */
    memcpy(&stemp, p, 2);
    sdata->gsfEM3Specific.valid_beams = (int) ntohs(stemp);
    p += 2;

    /* The next two bytes contain the sample rate */
    memcpy(&stemp, p, 2);
    sdata->gsfEM3Specific.sample_rate = (int) ntohs(stemp);
    p += 2;

    /* The next two bytes contain the depth difference between sonar heads in an em3000D configuration */
    memcpy(&stemp, p, 2);
    signed_short = (signed) ntohs(stemp);
    sdata->gsfEM3Specific.depth_difference = ((double) signed_short) / 100.0;
    p += 2;

    /* Next byte contains the transducer depth offset multiplier */
    sdata->gsfEM3Specific.offset_multiplier = (signed int) *p;
    p += 1;

    /* The next four byte value specifies the existance of the run-time data strucutre */
    memcpy(&ltemp, p, 4);
    run_time_id = ntohl(ltemp);
    p += 4;

    /* If the first bit is set then this subrecord contains a new set of run-time parameters,
     *  otherwise the run-time parameters have not changed.
     */
    if (run_time_id & 0x00000001)
    {
        /* The next two byte value contains the model number from the run-time parameters datagram */
        memcpy(&stemp, p, 2);
        sdata->gsfEM3Specific.run_time[0].model_number = (int) ntohs(stemp);
        p += 2;

        /* The next 8 bytes contain the time-tag for the run-time parameters datagram. */
        memcpy(&ltemp, p, 4);
        sdata->gsfEM3Specific.run_time[0].dg_time.tv_sec = ntohl(ltemp);
        p += 4;

        memcpy(&ltemp, p, 4);
        sdata->gsfEM3Specific.run_time[0].dg_time.tv_nsec = ntohl(ltemp);
        p += 4;

        /* The next two byte value contains the sequential ping number */
        memcpy(&stemp, p, 2);
        sdata->gsfEM3Specific.run_time[0].ping_number = (int) ntohs(stemp);
        p += 2;

        /* The next two byte value contains the sonar head serial number */
        memcpy(&stemp, p, 2);
        sdata->gsfEM3Specific.run_time[0].serial_number = (int) ntohs(stemp);
        p += 2;

        /* The next four byte value contains the system status */
        memcpy(&ltemp, p, 4);
        sdata->gsfEM3Specific.run_time[0].system_status = (int) ntohl(ltemp);
        p += 4;

        /* The next one byte value contains the mode identifier */
        sdata->gsfEM3Specific.run_time[0].mode = (int) *p;
        p += 1;

        /* The next one byte value contains the filter identifier */
        sdata->gsfEM3Specific.run_time[0].filter_id = (int) *p;
        p += 1;

        /* The next two byte value contains the minimum depth */
        memcpy(&stemp, p, 2);
        sdata->gsfEM3Specific.run_time[0].min_depth = (double) ntohs(stemp);
        p += 2;

        /* The next two byte value contains the maximum depth */
        memcpy(&stemp, p, 2);
        sdata->gsfEM3Specific.run_time[0].max_depth = (double) ntohs(stemp);
        p += 2;

        /* The next two byte value contains the absorption coefficient */
        memcpy(&stemp, p, 2);
        sdata->gsfEM3Specific.run_time[0].absorption = ((double) ntohs(stemp)) / 100.0;
        p += 2;

        /* The next two byte value contains the transmit pulse length */
        memcpy(&stemp, p, 2);
        sdata->gsfEM3Specific.run_time[0].pulse_length = (double) ntohs(stemp);
        p += 2;

        /* The next two byte value contains the transmit beam width */
        memcpy(&stemp, p, 2);
        sdata->gsfEM3Specific.run_time[0].transmit_beam_width = ((double) ntohs(stemp)) / 10.0;
        p += 2;

        /* The next one byte value contains the transmit power reduction */
        sdata->gsfEM3Specific.run_time[0].power_reduction = (int) *p;
        p += 1;

        /* The next one byte value contains the receive beam width */
        sdata->gsfEM3Specific.run_time[0].receive_beam_width = ((double) *p) / 10.0;
        p += 1;

        /* The next one byte value contains the receive band width. This value is provided
         *  by the sonar with a precision of 50 Hz.
         */
        sdata->gsfEM3Specific.run_time[0].receive_bandwidth = ((int) (*p)) * 50;
        p += 1;

        /* The next one byte value contains the receive gain */
        sdata->gsfEM3Specific.run_time[0].receive_gain = (int) *p;
        p += 1;

        /* The next one byte value contains the TVG law cross-over angle */
        sdata->gsfEM3Specific.run_time[0].cross_over_angle = (int) *p;
        p += 1;

        /* The next one byte value contains the source of the surface sound speed value */
        sdata->gsfEM3Specific.run_time[0].ssv_source = (int) *p;
        p += 1;

        /* The next two byte value contains the maximum swath width */
        memcpy(&stemp, p, 2);
        sdata->gsfEM3Specific.run_time[0].swath_width = (int) ntohs(stemp);
        p += 2;

        /* The next one byte value contains the beam spacing value */
        sdata->gsfEM3Specific.run_time[0].beam_spacing = (int) *p;
        p += 1;

        /* The next one byte value contains the coverage sector */
        sdata->gsfEM3Specific.run_time[0].coverage_sector = (int) *p;
        p += 1;

        /* The next one byte value contains the yaw and pitch stabilization mode */
        sdata->gsfEM3Specific.run_time[0].stabilization = (int) *p;
        p += 1;

        /* The next eight bytes are reserved for future use */
        sdata->gsfEM3Specific.run_time[0].spare1 = 0;
        sdata->gsfEM3Specific.run_time[0].spare2 = 0;
        p += 8;

        /* Since the run-time parameters only exist on the byte stream when they change, we need
         *  to save these to the file table so the'll be available to the caller for each ping.
         *
         * memcpy (&ft->rec.mb_ping.sensor_data.gsfEM3Specific.run_time[0], &sdata->gsfEM3Specific.run_time[0], sizeof(gsfEM3RunTime));
         */

        /* If the second bit is set then this subrecord contains a second set of new run-time
         *  for an em3000d series sonar system.
         */
        if (run_time_id & 0x00000002)
        {
            /* The next two byte value contains the model number from the run-time parameters datagram */
            memcpy(&stemp, p, 2);
            sdata->gsfEM3Specific.run_time[1].model_number = (int) ntohs(stemp);
            p += 2;

            /* The next 8 bytes contain the time-tag for the run-time parameters datagram. */
            memcpy(&ltemp, p, 4);
            sdata->gsfEM3Specific.run_time[1].dg_time.tv_sec = ntohl(ltemp);
            p += 4;

            memcpy(&ltemp, p, 4);
            sdata->gsfEM3Specific.run_time[1].dg_time.tv_nsec = ntohl(ltemp);
            p += 4;

            /* The next two byte value contains the sequential ping number */
            memcpy(&stemp, p, 2);
            sdata->gsfEM3Specific.run_time[1].ping_number = (int) ntohs(stemp);
            p += 2;

            /* The next two byte value contains the sonar head serial number */
            memcpy(&stemp, p, 2);
            sdata->gsfEM3Specific.run_time[1].serial_number = (int) ntohs(stemp);
            p += 2;

            /* The next four byte value contains the system status */
            memcpy(&ltemp, p, 4);
            sdata->gsfEM3Specific.run_time[1].system_status = (int) ntohl(ltemp);
            p += 4;

            /* The next one byte value contains the mode identifier */
            sdata->gsfEM3Specific.run_time[1].mode = (int) *p;
            p += 1;

            /* The next one byte value contains the filter identifier */
            sdata->gsfEM3Specific.run_time[1].filter_id = (int) *p;
            p += 1;

            /* The next two byte value contains the minimum depth */
            memcpy(&stemp, p, 2);
            sdata->gsfEM3Specific.run_time[1].min_depth = (double) ntohs(stemp);
            p += 2;

            /* The next two byte value contains the maximum depth */
            memcpy(&stemp, p, 2);
            sdata->gsfEM3Specific.run_time[1].max_depth = (double) ntohs(stemp);
            p += 2;

            /* The next two byte value contains the absorption coefficient */
            memcpy(&stemp, p, 2);
            sdata->gsfEM3Specific.run_time[1].absorption = ((double) ntohs(stemp)) / 100.0;
            p += 2;

            /* The next two byte value contains the transmit pulse length */
            memcpy(&stemp, p, 2);
            sdata->gsfEM3Specific.run_time[1].pulse_length = (double) ntohs(stemp);
            p += 2;

            /* The next two byte value contains the transmit beam width */
            memcpy(&stemp, p, 2);
            sdata->gsfEM3Specific.run_time[1].transmit_beam_width = ((double) ntohs(stemp)) / 10.0;
            p += 2;

            /* The next one byte value contains the transmit power reduction */
            sdata->gsfEM3Specific.run_time[1].power_reduction = (int) *p;
            p += 1;

            /* The next one byte value contains the receive beam width */
            sdata->gsfEM3Specific.run_time[1].receive_beam_width = ((double) *p) / 10.0;
            p += 1;

            /* The next one byte value contains the receive band width. This value is provided
             *  by the sonar with a precision of 50 Hz.
             */
            sdata->gsfEM3Specific.run_time[1].receive_bandwidth = ((int) (*p)) * 50;
            p += 1;

            /* The next one byte value contains the receive gain */
            sdata->gsfEM3Specific.run_time[1].receive_gain = (int) *p;
            p += 1;

            /* The next one byte value contains the TVG law cross-over angle */
            sdata->gsfEM3Specific.run_time[1].cross_over_angle = (int) *p;
            p += 1;

            /* The next one byte value contains the source of the surface sound speed value */
            sdata->gsfEM3Specific.run_time[1].ssv_source = (int) *p;
            p += 1;

            /* The next two byte value contains the maximum swath width */
            memcpy(&stemp, p, 2);
            sdata->gsfEM3Specific.run_time[1].swath_width = (int) ntohs(stemp);
            p += 2;

            /* The next one byte value contains the beam spacing value */
            sdata->gsfEM3Specific.run_time[1].beam_spacing = (int) *p;
            p += 1;

            /* The next one byte value contains the coverage sector */
            sdata->gsfEM3Specific.run_time[1].coverage_sector = (int) *p;
            p += 1;

            /* The next one byte value contains the yaw and pitch stabilization mode */
            sdata->gsfEM3Specific.run_time[1].stabilization = (int) *p;
            p += 1;

            /* The next eight bytes are reserved for future use */
            sdata->gsfEM3Specific.run_time[1].spare1 = 0;
            sdata->gsfEM3Specific.run_time[1].spare2 = 0;
            p += 8;

            /* Since the run-time parameters only exist on the byte stream when they change, we need
             *  to save these to the file table so the'll be available to the caller for each ping.
             *
             * memcpy (&ft->rec.mb_ping.sensor_data.gsfEM3Specific.run_time[1], &sdata->gsfEM3Specific.run_time[1], sizeof(gsfEM3RunTime));
             */
        }
    }

    /* jsb 3/31/99 Commented this code block out.  No need to do this until we encode this subrecord only when
     *   the values have changed. Refer to comments in gsfEncodeEM3Specific.
     *
     * else
     * {
     *     memcpy (&sdata->gsfEM3Specific.run_time[0], &ft->rec.mb_ping.sensor_data.gsfEM3Specific.run_time[0], sizeof(gsfEM3RunTime));
     *     memcpy (&sdata->gsfEM3Specific.run_time[1], &ft->rec.mb_ping.sensor_data.gsfEM3Specific.run_time[1], sizeof(gsfEM3RunTime));
     * }
     */
    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : gsfDecodeSoundVelocityProfile
 *
 * Description : This function decodes a gsf sound velocity profile record
 *  from external byte stream form into internal form.  Memory for the
 *  depth/sound speed arrays is allocated (or reallocted) each time this
 *  record is encountered since the number of points in the profile can
 *  change.
 *
 * Inputs :
 *    svp = a pointer to the gsfSVP structure to load
 *    ft = a pointer to the GSF_FILE_TABLE entry for the data file being decoded
 *    sptr = a pointer to the unsigned char buffer to read from
 *
 * Returns :
 *  This function returns the number of bytes decoded if successful, or
 *  -1 if an error occured.
 *
 * Error Conditions :
 *    GSF_MEMORY_ALLOCATION_FAILED
 *
 ********************************************************************/
int
gsfDecodeSoundVelocityProfile(gsfSVP *svp, GSF_FILE_TABLE *ft, unsigned char *sptr)
{
    unsigned char  *p = sptr;
    gsfuLong        ltemp;
    gsfsLong        signed_int;
    int             i;

    /* First four byte integer contains the observation time seconds */
    memcpy(&ltemp, p, 4);
    p += 4;
    svp->observation_time.tv_sec = ntohl(ltemp);

    /* Next four byte integer contains the observation time nanoseconds */
    memcpy(&ltemp, p, 4);
    p += 4;
    svp->observation_time.tv_nsec = ntohl(ltemp);

    /* Next four byte integer contains the seconds portion of the time the
    *  new profile was put into use by the sonar system
    */
    memcpy(&ltemp, p, 4);
    p += 4;
    svp->application_time.tv_sec = ntohl(ltemp);

    /* Next four byte integer contains the application time nanoseconds */
    memcpy(&ltemp, p, 4);
    p += 4;
    svp->application_time.tv_nsec = ntohl(ltemp);

    /* Next four byte integer contains the longitude of profile observation */
    memcpy(&ltemp, p, 4);
    signed_int = (signed) ntohl(ltemp);
    svp->longitude = ((double) signed_int) / 1.0e7;
    p += 4;

    /* Next four byte integer contains the latitude of profile observation */
    memcpy(&ltemp, p, 4);
    signed_int = (signed) ntohl(ltemp);
    svp->latitude = ((double) signed_int) / 1.0e7;
    p += 4;

    /* Next four byte integer contains the number of points in the profile */
    memcpy(&ltemp, p, 4);
    p += 4;
    svp->number_points = ntohl(ltemp);

    /* NULL out caller's memory pointers in case memory allocation fails */
    svp->depth = (double *) NULL;
    svp->sound_speed = (double *) NULL;

    /* make sure we have memory for the depth/speed pairs */
    if (ft->rec.svp.depth == (double *) NULL)
    {
        ft->rec.svp.depth = (double *) calloc(svp->number_points, sizeof(double));

        if (ft->rec.svp.depth == (double *) NULL)
        {
            gsfError = GSF_MEMORY_ALLOCATION_FAILED;
            return (-1);
        }
    }

    /* Re-allocate the dynamic memory if the number of points in the profile
     * is greater this time than it was last time.
     */
    else if (ft->rec.svp.number_points < svp->number_points)
    {
        ft->rec.svp.depth = (double *) realloc(ft->rec.svp.depth, svp->number_points * sizeof(double));

        if (ft->rec.svp.depth == (double *) NULL)
        {
            gsfError = GSF_MEMORY_ALLOCATION_FAILED;
            return (-1);
        }
        memset(ft->rec.svp.depth, 0, svp->number_points * sizeof(double));
    }
    /* Set the caller's pointer to this dynamic memory */
    svp->depth = ft->rec.svp.depth;

    if (ft->rec.svp.sound_speed == (double *) NULL)
    {
        ft->rec.svp.sound_speed = (double *) calloc(svp->number_points, sizeof(double));

        if (ft->rec.svp.sound_speed == (double *) NULL)
        {
            gsfError = GSF_MEMORY_ALLOCATION_FAILED;
            return (-1);
        }
    }

    /* Re-allocate the dynamic memory if the number of points in the profile
     * is greater this time than it was last time.
     */
    else if (ft->rec.svp.number_points < svp->number_points)
    {
        ft->rec.svp.sound_speed = (double *) realloc(ft->rec.svp.sound_speed, svp->number_points * sizeof(double));

        if (ft->rec.svp.sound_speed == (double *) NULL)
        {
            gsfError = GSF_MEMORY_ALLOCATION_FAILED;
            return (-1);
        }
        memset(ft->rec.svp.sound_speed, 0, svp->number_points * sizeof(double));
    }
    /* Set the caller's pointer to this dynamic memory */
    svp->sound_speed = ft->rec.svp.sound_speed;

    /* Save the number of points in this profile in the library's file table */
    ft->rec.svp.number_points = svp->number_points;

    /* Now loop to decode the depth/sound speed pairs */
    for (i = 0; i < svp->number_points; i++)
    {
        memcpy(&ltemp, p, 4);
        p += 4;
        svp->depth[i] = ((double) ntohl(ltemp)) / 100.0;

        memcpy(&ltemp, p, 4);
        p += 4;
        svp->sound_speed[i] = ((double) ntohl(ltemp)) / 100.0;
    }

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : gsfDecodeProcessingParameters
 *
 * Description : This function decodes a processing parameters record
 *  from external gsf byte stream form into internal form.
 *
 * Inputs :
 *   param = a pointer to the gsfProcessingParamters structure to populate
 *   ft = a pointer to the GSF_FILE_TABLE entry for the data file being decoded
 *   sptr = a pointer to the unsigned char buffer to read from
 *
 * Returns : This function returns the number of bytes decoded if succesful,
 *  or -1 on error.
 *
 * Error Conditions :
 *    GSF_MEMORY_ALLOCATION_FAILED
 *
 ********************************************************************/
int
gsfDecodeProcessingParameters(gsfProcessingParameters *param, GSF_FILE_TABLE *ft, unsigned char *sptr)
{
    unsigned char  *p = sptr;
    gsfuLong        ltemp;
    gsfuShort       stemp;
    int             i;

    /* First four byte integer contains the seconds portion of the time
    *  application of the new parameters.
    */
    memcpy(&ltemp, p, 4);
    p += 4;
    param->param_time.tv_sec = ntohl(ltemp);

    /* Next four byte integer contains the application time nanoseconds */
    memcpy(&ltemp, p, 4);
    p += 4;
    param->param_time.tv_nsec = ntohl(ltemp);

    /* Next two byte integer contains the number of parameters in this record */
    memcpy(&stemp, p, 2);
    p += 2;
    param->number_parameters = (int) ntohs(stemp);

    /* Now loop to decode these parameters */
    for (i = 0; (i < param->number_parameters) && (i < GSF_MAX_PROCESSING_PARAMETERS); i++)
    {
        /* two byte integer contains the size of the parameter */
        memcpy(&stemp, p, 2);
        p += 2;
        param->param_size[i] = (short) ntohs(stemp);

        /* NULL caller's pointer in case memory allocation fails */
        param->param[i] = (char *) NULL;

        /* Make sure we have memory to hold the parameter */
        if (ft->rec.process_parameters.param[i] == (char *) NULL)
        {
            ft->rec.process_parameters.param[i] = (char *) calloc(param->param_size[i] + 1, sizeof(char));

            if (ft->rec.process_parameters.param[i] == (char *) NULL)
            {
                gsfError = GSF_MEMORY_ALLOCATION_FAILED;
                return (-1);
            }
        }
        else if (ft->rec.process_parameters.param_size[i] < param->param_size[i])
        {
            ft->rec.process_parameters.param[i] = (char *) realloc((void *) ft->rec.process_parameters.param[i], param->param_size[i] + 1);
            if (ft->rec.process_parameters.param[i] == (char *) NULL)
            {
                gsfError = GSF_MEMORY_ALLOCATION_FAILED;
                return (-1);
            }
        }
        memcpy(ft->rec.process_parameters.param[i], p, param->param_size[i]);
        param->param[i] = ft->rec.process_parameters.param[i];
        ft->rec.process_parameters.param_size[i] = param->param_size[i];

        /* make sure the text is null terminated */
        param->param[i][param->param_size[i]] = '\0';
        p += param->param_size[i];
    }
    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : gsfDecodeSensorParameters
 *
 * Description : This function decodes a gsf sensor parameters record
 *  from external byte stream form into internal form.
 *
 * Inputs :
 *    param = a pointer to a gsfSensorParameters structure to be populated
 *    ft = a pointer to the GSF_FILE_TABLE entry for the data file being decoded
 *    sptr = a pointer to an unsigned char buffer to read from.
 *
 * Returns : This function returns the number of bytes decoded if successful,
 *  or -1 on error.
 *
 * Error Conditions :
 *    GSF_MEMORY_ALLOCATION_FAILED
 *
 ********************************************************************/
int
gsfDecodeSensorParameters(gsfSensorParameters *param, GSF_FILE_TABLE *ft, unsigned char *sptr)
{
    unsigned char  *p = sptr;
    gsfuLong        ltemp;
    gsfuShort       stemp;
    int             i;

    /* First four byte integer contains the seconds portion of the time
    *  application of the new parameters.
    */
    memcpy(&ltemp, p, 4);
    p += 4;
    param->param_time.tv_sec = ntohl(ltemp);

    /* Next four byte integer contains the application time nanoseconds */
    memcpy(&ltemp, p, 4);
    p += 4;
    param->param_time.tv_nsec = ntohl(ltemp);

    /* Next two byte integer contains the number of parameters in this record */
    memcpy(&stemp, p, 2);
    p += 2;
    param->number_parameters = (int) ntohs(stemp);

    /* Now loop to decode these parameters */
    for (i = 0; (i < param->number_parameters) && (i < GSF_MAX_SENSOR_PARAMETERS); i++)
    {
        /* two byte integer contains the size of the parameter */
        memcpy(&stemp, p, 2);
        p += 2;
        param->param_size[i] = (short) ntohs(stemp);

        /* NULL caller's pointer in case memory allocation fails */
        param->param[i] = (char *) NULL;

        /* Make sure we have memory to hold the parameter */
        if (ft->rec.sensor_parameters.param[i] == (char *) NULL)
        {
            ft->rec.sensor_parameters.param[i] = (char *) calloc(param->param_size[i] + 1, sizeof(char));
            if (ft->rec.sensor_parameters.param[i] == (char *) NULL)
            {
                gsfError = GSF_MEMORY_ALLOCATION_FAILED;
                return (-1);
            }
        }
        else if (ft->rec.sensor_parameters.param_size[i] < param->param_size[i])
        {
            ft->rec.sensor_parameters.param[i] = (char *) realloc((void *) ft->rec.sensor_parameters.param[i], param->param_size[i] + 1);
            if (ft->rec.sensor_parameters.param[i] == (char *) NULL)
            {
                gsfError = GSF_MEMORY_ALLOCATION_FAILED;
                return (-1);
            }
        }
        memcpy(ft->rec.sensor_parameters.param[i], p, param->param_size[i]);
        param->param[i] = ft->rec.sensor_parameters.param[i];
        ft->rec.sensor_parameters.param_size[i] = param->param_size[i];

        /* Make sure the text is null terminated */
        param->param[i][param->param_size[i]] = '\0';
        p += param->param_size[i];
    }
    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : gsfDecodeComment
 *
 * Description :  This function is used to decode a gsf comment record
 *  from external byte stream from to internal form.
 *
 * Inputs :
 *    comment = a pointer to the gsfComment structure to be loaded
 *    ft = a pointer to the GSF_FILE_TABLE entry for the data file being decoded
 *    sptr = a pointer to the unsigned char buffer to read from
 *
 * Returns :
 *  This function returns the number of bytes decoded if successful, or
 *  -1 if an error occured.
 *
 * Error Conditions :
 *    GSF_MEMORY_ALLOCATION_FAILED
 *
 ********************************************************************/
int
gsfDecodeComment(gsfComment *comment, GSF_FILE_TABLE *ft, unsigned char *sptr)
{
    unsigned char  *p = sptr;
    gsfuLong        ltemp;

    /* First four byte integer contains the seconds portion of the time
    *  the operator comment was made.
    */
    memcpy(&ltemp, p, 4);
    p += 4;
    comment->comment_time.tv_sec = ntohl(ltemp);

    /* Next four byte integer contains the nanoseconds portion of the
    * comment time
    */
    memcpy(&ltemp, p, 4);
    p += 4;
    comment->comment_time.tv_nsec = ntohl(ltemp);

    /* Next four byte integer contains the length of the comment */
    memcpy(&ltemp, p, 4);
    p += 4;
    comment->comment_length = ntohl(ltemp);

    /* NULL out caller's pointer to dynamic memory in case allocation fails */
    comment->comment = (char *) NULL;

    /* Make sure we have memory to hold the parameter */
    if (ft->rec.comment.comment == (char *) NULL)
    {
        ft->rec.comment.comment = (char *) calloc(comment->comment_length + 1, sizeof(char));
        if (ft->rec.comment.comment == (char *) NULL)
        {
            gsfError = GSF_MEMORY_ALLOCATION_FAILED;
            return (-1);
        }
    }
    else if (ft->rec.comment.comment_length < comment->comment_length)
    {
        ft->rec.comment.comment = (char *) realloc((void *) ft->rec.comment.comment, comment->comment_length + 1);
        if (ft->rec.comment.comment == (char *) NULL)
        {
            gsfError = GSF_MEMORY_ALLOCATION_FAILED;
            return (-1);
        }
    }

    memcpy(ft->rec.comment.comment, p, comment->comment_length);
    comment->comment = ft->rec.comment.comment;
    ft->rec.comment.comment_length = comment->comment_length;
    comment->comment[comment->comment_length] = '\0';
    p += comment->comment_length;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : gsfDecodeHistory
 *
 * Description : This function is used to decode a gsf history record
 *  from external byte stream form to internal form.
 *
 * Inputs :
 *    history = a pointer to the gsf history structure to load
 *    ft = a pointer to the GSF_FILE_TABLE entry for the data file being decoded
 *    sptr = a pointer to an unsigned char buffer to read from
 *
 * Returns :
 *  This function returns the number of bytes decoded if successful, or
 *  -1 if an error occured.
 *
 * Error Conditions :
 *    GSF_MEMORY_ALLOCATION_FAILED
 *
 ********************************************************************/

int
gsfDecodeHistory(gsfHistory * history, GSF_FILE_TABLE *ft, unsigned char *sptr)
{
    unsigned char  *p = sptr;
    int             len;
    gsfuLong        ltemp;
    gsfuShort       stemp;

    /* First four byte integer contains the seconds portion of the time
    *  the history record was added to the data.
    */
    memcpy(&ltemp, p, 4);
    p += 4;
    history->history_time.tv_sec = ntohl(ltemp);

    /* Next four byte integer contains the nanoseconds portion of the
    * history time.
    */
    memcpy(&ltemp, p, 4);
    p += 4;
    history->history_time.tv_nsec = ntohl(ltemp);

    /* two byte integer contains the size of the text to follow */
    memcpy(&stemp, p, 2);
    p += 2;
    len = (short) ntohs(stemp);

    /* Next len bytes contains the host name */
    if (len < GSF_HOST_NAME_LENGTH)
    {
        memcpy(history->host_name, p, len);
        history->host_name[len] = '\0';
        p += len;
    }
    else
    {
        gsfError = GSF_HISTORY_RECORD_DECODE_FAILED;
        return(-1);
    }

    /* two byte integer contains the size of the text to follow */
    memcpy(&stemp, p, 2);
    p += 2;
    len = (short) ntohs(stemp);

    /* Next len bytes contains the host name */
    if (len < GSF_OPERATOR_LENGTH)
    {
        memcpy(history->operator, p, len);
        history->operator[len] = '\0';
        p += len;
    }
    else
    {
        gsfError = GSF_HISTORY_RECORD_DECODE_FAILED;
        return(-1);
    }

    /* Next two byte integer contains the size of the command line used
    *  to invoke the program which processed this data.
    */
    memcpy(&stemp, p, 2);
    p += 2;
    len = ntohs(stemp);

    /* NULL out the caller's pointer in case memory allocation fails */
    history->command_line = (char *) NULL;

    /* Re-allocate the memory for the command line each time down */
    if (ft->rec.history.command_line != (char *) NULL)
    {
        free (ft->rec.history.command_line);
    }
    ft->rec.history.command_line = (char *) calloc(len + 1, sizeof(char));
    if (ft->rec.history.command_line == (char *) NULL)
    {
        gsfError = GSF_MEMORY_ALLOCATION_FAILED;
        return (-1);
    }

    /* Next "len" bytes contain the command line used to process the data */
    memcpy(ft->rec.history.command_line, p, len);
    history->command_line = ft->rec.history.command_line;
    history->command_line[len] = '\0';
    p += len;

    /* Next two byte integer contains the size of the comment for this history
    *  record
    */
    memcpy(&stemp, p, 2);
    p += 2;
    len = ntohs(stemp);

    /* NULL out the caller's memory in case the allocation fails */
    history->comment = (char *) NULL;

    /* Re-allocate the memory for the comment line each time down */
    if (ft->rec.history.comment != (char *) NULL)
    {
        free(ft->rec.history.comment);
    }
    ft->rec.history.comment = (char *) calloc(len + 1, sizeof(char));

    if (ft->rec.history.comment == (char *) NULL)
    {
        gsfError = GSF_MEMORY_ALLOCATION_FAILED;
        return (-1);
    }

    /* Next "len" bytes contain the comment for this history record */
    memcpy(ft->rec.history.comment, p, len);
    history->comment = ft->rec.history.comment;
    history->comment[len] = '\0';
    p += len;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : gsfDecodeNavigationError
 *
 * Description : This function decodes a gsf byte stream containing a
 *  navigation error record into internal form.
 *
 * Inputs :
 *   nav_error = a pointer to the gsfNavigationError structure to be loaded
 *   sptr = a pointer to the unsigned char buffer to read from
 *
 * Returns :
 *  This function returns the number of bytes decoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

int
gsfDecodeNavigationError(gsfNavigationError * nav_error, unsigned char *sptr)
{
    unsigned char  *p = sptr;
    gsfuLong        ltemp;

    /* First four byte integer contains the seconds portion of the time
    *  of navigation error.
    */
    memcpy(&ltemp, p, 4);
    p += 4;
    nav_error->nav_error_time.tv_sec = ntohl(ltemp);

    /* Next four byte integer contains the nanoseconds portion of the
    * history time.
    */
    memcpy(&ltemp, p, 4);
    p += 4;
    nav_error->nav_error_time.tv_nsec = ntohl(ltemp);

    /* Next four byte integer contains the record id for the record
    *  containing a position with this error. (registry and type number)
    */
    memcpy(&ltemp, p, 4);
    p += 4;
    nav_error->record_id = ntohl(ltemp);

    /* Next four byte integer contains the longitude error estimate */
    memcpy(&ltemp, p, 4);
    p += 4;
    nav_error->longitude_error = ((double) ntohs(ltemp)) / 10.0;

    /* Next four byte integer contains the latitude error estimate */
    memcpy(&ltemp, p, 4);
    p += 4;
    nav_error->latitude_error = ((double) ntohl(ltemp)) / 10.0;

    return (p - sptr);
}

/********************************************************************
 *
 * Function Name : gsfDecodeHVNavigationError
 *
 * Description : This function decodes a gsf byte stream containing
 *  the new horizontal and vertical navigation error record.
 *
 * Inputs :
 *   hv_nav_error = a pointer to the gsfHVNavigationError structure to be loaded
 *   sptr = a pointer to the unsigned char buffer to read from
 *
 * Returns :
 *  This function returns the number of bytes decoded.
 *
 * Error Conditions : none
 *
 ********************************************************************/

int
gsfDecodeHVNavigationError(gsfHVNavigationError *hv_nav_error, GSF_FILE_TABLE *ft, unsigned char *sptr)
{
    int             length;
    unsigned char  *p = sptr;
    gsfsShort       stemp;
    gsfsLong        ltemp;

    /* First four byte integer contains the seconds portion of the time
    *  of navigation error.
    */
    memcpy(&ltemp, p, 4);
    p += 4;
    hv_nav_error->nav_error_time.tv_sec = ntohl(ltemp);

    /* Next four byte integer contains the nanoseconds portion of the
    * history time.
    */
    memcpy(&ltemp, p, 4);
    p += 4;
    hv_nav_error->nav_error_time.tv_nsec = ntohl(ltemp);

    /* Next four byte integer contains the record id for the record
    *  containing a position with this error. (registry and type number)
    */
    memcpy(&ltemp, p, 4);
    p += 4;
    hv_nav_error->record_id = ntohl(ltemp);

    /* Next four byte integer contains the horizontal error estimate */
    memcpy(&ltemp, p, 4);
    p += 4;
    hv_nav_error->horizontal_error = ((double) ntohs(ltemp)) / 1000.0;

    /* Next four byte integer contains the vertical error estimate */
    memcpy(&ltemp, p, 4);
    p += 4;
    hv_nav_error->vertical_error = ((double) ntohl(ltemp)) / 1000.0;

    /* The next four bytes are reserved for future use */
    hv_nav_error->spare[0] = (char) *p;
    p += 1;
    hv_nav_error->spare[1] = (char) *p;
    p += 1;
    hv_nav_error->spare[2] = (char) *p;
    p += 1;
    hv_nav_error->spare[3] = (char) *p;
    p += 1;

    /* The next two byte integer contains the length of the positioning type string */
    memcpy(&stemp, p, 2);
    p += 2;
    length = ntohs(stemp);

    /* NULL out the caller's memory in case the allocation fails */
    hv_nav_error->position_type = (char *) NULL;

    /* Re-allocate the memory for the comment line each time down */
    if (ft->rec.hv_nav_error.position_type != (char *) NULL)
    {
        free(ft->rec.hv_nav_error.position_type);
    }
    ft->rec.hv_nav_error.position_type = (char *) calloc(length + 1, sizeof(char));

    if (ft->rec.hv_nav_error.position_type == (char *) NULL)
    {
        gsfError = GSF_MEMORY_ALLOCATION_FAILED;
        return (-1);
    }

    /* Next "len" bytes contain the positioning system type */
    memcpy(ft->rec.hv_nav_error.position_type, p, length);
    hv_nav_error->position_type = ft->rec.hv_nav_error.position_type;
    hv_nav_error->position_type[length] = '\0';
    p += length;

    return (p - sptr);
}

