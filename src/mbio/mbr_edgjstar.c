/*--------------------------------------------------------------------
 *    The MB-system:	mbr_edgjstar.c	5/2/2005
 *
 *    Copyright (c) 2005-2019 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbr_edgjstar.c contains the functions for reading
 * sidescan data in the EDGJSTAR format.
 * These functions include:
 *   mbr_alm_edgjstar	- allocate read/write memory
 *   mbr_dem_edgjstar	- deallocate read/write memory
 *   mbr_rt_edgjstar	- read and translate data
 *   mbr_wt_edgjstar	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	May 2, 2005
 *
 */

/* Debug flag */
//#define MBF_EDGJSTAR_DEBUG 1

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mb_swap.h"
#include "mbsys_jstar.h"

/*--------------------------------------------------------------------*/
int mbr_info_edgjstar(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error) {
	char *function_name = "mbr_info_edgjstar";

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
	}

	/* set format info parameters */
	*error = MB_ERROR_NO_ERROR;
	*system = MB_SYS_JSTAR;
	*beams_bath_max = 1;
	*beams_amp_max = 0;
	*pixels_ss_max = MBSYS_JSTAR_PIXELS_MAX;
	strncpy(format_name, "EDGJSTAR", MB_NAME_LENGTH);
	strncpy(system_name, "JSTAR", MB_NAME_LENGTH);
	strncpy(format_description,
	        "Format name:          MBF_EDGJSTAR\nInformal Description: Edgetech Jstar format\nAttributes:           variable "
	        "pixels, dual frequency sidescan and subbottom,\n                      binary SEGY variant, single files,\n          "
	        "            low frequency sidescan returned as\n                      survey data, Edgetech. \n",
	        MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = MB_NO;
	*traveltime = MB_NO;
	*beam_flagging = MB_NO;
	*platform_source = MB_DATA_NONE;
	*nav_source = MB_DATA_DATA;
	*sensordepth_source = MB_DATA_DATA;
	*heading_source = MB_DATA_DATA;
	*attitude_source = MB_DATA_DATA;
	*svp_source = MB_DATA_NONE;
	*beamwidth_xtrack = 0.0;
	*beamwidth_ltrack = 0.0;

	const int status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       system:             %d\n", *system);
		fprintf(stderr, "dbg2       beams_bath_max:     %d\n", *beams_bath_max);
		fprintf(stderr, "dbg2       beams_amp_max:      %d\n", *beams_amp_max);
		fprintf(stderr, "dbg2       pixels_ss_max:      %d\n", *pixels_ss_max);
		fprintf(stderr, "dbg2       format_name:        %s\n", format_name);
		fprintf(stderr, "dbg2       system_name:        %s\n", system_name);
		fprintf(stderr, "dbg2       format_description: %s\n", format_description);
		fprintf(stderr, "dbg2       numfile:            %d\n", *numfile);
		fprintf(stderr, "dbg2       filetype:           %d\n", *filetype);
		fprintf(stderr, "dbg2       variable_beams:     %d\n", *variable_beams);
		fprintf(stderr, "dbg2       traveltime:         %d\n", *traveltime);
		fprintf(stderr, "dbg2       beam_flagging:      %d\n", *beam_flagging);
		fprintf(stderr, "dbg2       platform_source:    %d\n", *platform_source);
		fprintf(stderr, "dbg2       nav_source:         %d\n", *nav_source);
		fprintf(stderr, "dbg2       sensordepth_source: %d\n", *sensordepth_source);
		fprintf(stderr, "dbg2       heading_source:     %d\n", *heading_source);
		fprintf(stderr, "dbg2       attitude_source:      %d\n", *attitude_source);
		fprintf(stderr, "dbg2       svp_source:         %d\n", *svp_source);
		fprintf(stderr, "dbg2       beamwidth_xtrack:   %f\n", *beamwidth_xtrack);
		fprintf(stderr, "dbg2       beamwidth_ltrack:   %f\n", *beamwidth_ltrack);
		fprintf(stderr, "dbg2       error:              %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:         %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbr_info_edgjstr2(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error) {
	char *function_name = "mbr_info_edgjstr2";

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
	}

	/* set format info parameters */
	*error = MB_ERROR_NO_ERROR;
	*system = MB_SYS_JSTAR;
	*beams_bath_max = 1;
	*beams_amp_max = 0;
	*pixels_ss_max = MBSYS_JSTAR_PIXELS_MAX;
	strncpy(format_name, "EDGJSTR2", MB_NAME_LENGTH);
	strncpy(system_name, "EDGJSTR2", MB_NAME_LENGTH);
	strncpy(format_description,
	        "Format name:          MBF_EDGJSTR2\nInformal Description: Edgetech Jstar format\nAttributes:           variable "
	        "pixels, dual frequency sidescan and subbottom,\n                      binary SEGY variant, single files,\n          "
	        "            high frequency sidescan returned as\n                      survey data, Edgetech. \n",
	        MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = MB_NO;
	*traveltime = MB_NO;
	*beam_flagging = MB_NO;
	*platform_source = MB_DATA_NONE;
	*nav_source = MB_DATA_DATA;
	*sensordepth_source = MB_DATA_DATA;
	*heading_source = MB_DATA_DATA;
	*attitude_source = MB_DATA_DATA;
	*svp_source = MB_DATA_NONE;
	*beamwidth_xtrack = 0.0;
	*beamwidth_ltrack = 0.0;

	const int status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       system:             %d\n", *system);
		fprintf(stderr, "dbg2       beams_bath_max:     %d\n", *beams_bath_max);
		fprintf(stderr, "dbg2       beams_amp_max:      %d\n", *beams_amp_max);
		fprintf(stderr, "dbg2       pixels_ss_max:      %d\n", *pixels_ss_max);
		fprintf(stderr, "dbg2       format_name:        %s\n", format_name);
		fprintf(stderr, "dbg2       system_name:        %s\n", system_name);
		fprintf(stderr, "dbg2       format_description: %s\n", format_description);
		fprintf(stderr, "dbg2       numfile:            %d\n", *numfile);
		fprintf(stderr, "dbg2       filetype:           %d\n", *filetype);
		fprintf(stderr, "dbg2       variable_beams:     %d\n", *variable_beams);
		fprintf(stderr, "dbg2       traveltime:         %d\n", *traveltime);
		fprintf(stderr, "dbg2       beam_flagging:      %d\n", *beam_flagging);
		fprintf(stderr, "dbg2       platform_source:    %d\n", *platform_source);
		fprintf(stderr, "dbg2       nav_source:         %d\n", *nav_source);
		fprintf(stderr, "dbg2       sensordepth_source: %d\n", *sensordepth_source);
		fprintf(stderr, "dbg2       heading_source:     %d\n", *heading_source);
		fprintf(stderr, "dbg2       attitude_source:      %d\n", *attitude_source);
		fprintf(stderr, "dbg2       svp_source:         %d\n", *svp_source);
		fprintf(stderr, "dbg2       beamwidth_xtrack:   %f\n", *beamwidth_xtrack);
		fprintf(stderr, "dbg2       beamwidth_ltrack:   %f\n", *beamwidth_ltrack);
		fprintf(stderr, "dbg2       error:              %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:         %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_alm_edgjstar(int verbose, void *mbio_ptr, int *error) {
	char *function_name = "mbr_alm_edgjstar";

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* allocate memory for data structure */
	mb_io_ptr->structure_size = 0;
	mbsys_jstar_alloc(verbose, mbio_ptr, &mb_io_ptr->store_data, error);

	const int status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_dem_edgjstar(int verbose, void *mbio_ptr, int *error) {
	char *function_name = "mbr_dem_edgjstar";

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* deallocate memory for data structure */
	const int status = mbsys_jstar_deall(verbose, mbio_ptr, &mb_io_ptr->store_data, error);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_rt_edgjstar(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	char *function_name = "mbr_rt_edgjstar";
	struct mbsys_jstar_message_struct message;
	struct mbsys_jstar_struct *store;
	struct mbsys_jstar_channel_struct *sbp;
	struct mbsys_jstar_channel_struct *ss;
	struct mbsys_jstar_nmea_struct *nmea;
	struct mbsys_jstar_pitchroll_struct *pitchroll;
	struct mbsys_jstar_dvl_struct *dvl;
	struct mbsys_jstar_pressure_struct *pressure;
	struct mbsys_jstar_sysinfo_struct *sysinfo;
    struct mbsys_jstar_filetimestamp_struct *filetimestamp;
	struct mbsys_jstar_comment_struct *comment;
	struct mbsys_jstar_ssold_struct ssold_tmp;
	char buffer[MBSYS_JSTAR_SYSINFO_MAX];
	char nmeastring[MB_COMMENT_MAXLINE];
	int index;
	int done;
	int read_status;
	int shortspersample;
	int trace_size;
    int testx1, testx2, testy1, testy2, obsolete_header;
	double time_d;
	double navlon, navlat, heading;
	double speed, sonardepth, altitude;
	double heave, roll, pitch;
	double rawvalue;
	int time_i[7];
	int time_j[5];
	double depthofsensor, offset;
	char **nap, *nargv[25], *string;
	int nargc;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get pointer to raw data structure */
	store = (struct mbsys_jstar_struct *)store_ptr;

	/* have a local struct mbsys_jstar_ss_struct ss_tmp for
	 * reading old "sidescan" records and translating them
	 * to the current form - initialize the trace to null */
	ssold_tmp.trace_alloc = 0;
	ssold_tmp.trace = NULL;

	/* make sure the sidescan ping numbers and channels are initialized so
	 * that it is impossible to mistakenly report a full ping read when only
	 * one channel has been read */
	store->ssport.pingNum = -1;
	store->ssstbd.pingNum = -1;
	store->ssport.message.subsystem = -1;
	store->ssstbd.message.subsystem = -1;

	int status = MB_SUCCESS;

	/* loop over reading data until a full record of some sort is read */
	done = MB_NO;
	while (done == MB_NO) {
		/* read message header */
		if ((read_status = fread(buffer, MBSYS_JSTAR_MESSAGE_SIZE, 1, mb_io_ptr->mbfp)) == 1) {
			/* extract the message header values */
			index = 0;
			mb_get_binary_short(MB_YES, &buffer[index], &(message.start_marker));
			index += 2;
			message.version = (mb_u_char)buffer[index];
			index++;
			message.session = (mb_u_char)buffer[index];
			index++;
			mb_get_binary_short(MB_YES, &buffer[index], &(message.type));
			index += 2;
			message.command = (mb_u_char)buffer[index];
			index++;
			message.subsystem = (mb_u_char)buffer[index];
			index++;
			message.channel = (mb_u_char)buffer[index];
			index++;
			message.sequence = (mb_u_char)buffer[index];
			index++;
			mb_get_binary_short(MB_YES, &buffer[index], &(message.reserved));
			index += 2;
			mb_get_binary_int(MB_YES, &buffer[index], &(message.size));
			index += 4;

			store->subsystem = message.subsystem;

			status = MB_SUCCESS;
#ifdef MBF_EDGJSTAR_DEBUG
			fprintf(stderr, "NEW MESSAGE HEADER: status:%d message.version:%d message.type:%d message.subsystem:%d channel:%d message.size:%d\n",
			        status, message.version, message.type, message.subsystem, message.channel, message.size);
#endif
		}

		/* end of file */
		else {
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			done = MB_YES;
			store->kind = MB_DATA_NONE;
#ifdef MBF_EDGJSTAR_DEBUG
			fprintf(stderr, "REACHED END OF FILE: status:%d\n", status);
#endif
		}

		/* if comment proceed to get data */
		if (status == MB_SUCCESS && message.type == MBSYS_JSTAR_DATA_COMMENT && message.size < MB_COMMENT_MAXLINE) {
			/* comment channel */
			comment = (struct mbsys_jstar_comment_struct *)&(store->comment);
			comment->message = message;

			/* read the comment */
			if ((read_status = fread(comment->comment, message.size, 1, mb_io_ptr->mbfp)) == 1) {
				comment->comment[message.size] = 0;
				done = MB_YES;
				store->kind = MB_DATA_COMMENT;
			}

			/* end of file */
			else {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
				done = MB_YES;
				store->kind = MB_DATA_NONE;
			}
		}

		/* if subbottom data and sonar trace 80 proceed to get data */
		else if (status == MB_SUCCESS && message.type == MBSYS_JSTAR_DATA_SONAR &&
		         message.subsystem == MBSYS_JSTAR_SUBSYSTEM_SBP) {
			/* sbp channel */
			sbp = (struct mbsys_jstar_channel_struct *)&(store->sbp);
			sbp->message = message;

			/* read the 240 byte trace header */
			if ((read_status = fread(buffer, MBSYS_JSTAR_SBPHEADER_SIZE, 1, mb_io_ptr->mbfp)) == 1) {
                // test if these data were written incorrectly by a prior version of MB-System
                // - early versions of jsf maintained the segy structure of separate source and
                //   receiver position, and MB-System wrote jsf files that way up through 5.5.2321
				mb_get_binary_int(MB_YES, &buffer[72], &testx1);
				mb_get_binary_int(MB_YES, &buffer[76], &testy1);
				mb_get_binary_int(MB_YES, &buffer[80], &testx2);
				mb_get_binary_int(MB_YES, &buffer[84], &testy2);
                if (testx1 == testx2 && testy1 == testy2) {
                    obsolete_header = MB_YES;
                } else {
                    obsolete_header = MB_NO;
                }
                
				index = 0;
				mb_get_binary_int(MB_YES, &buffer[index], &(sbp->pingTime));
				index += 4;
				mb_get_binary_int(MB_YES, &buffer[index], &(sbp->startDepth));
				index += 4;
				mb_get_binary_int(MB_YES, &buffer[index], &(sbp->pingNum));
				index += 4;
				for (int i = 0; i < 2; i++) {
					mb_get_binary_short(MB_YES, &buffer[index], &(sbp->reserved1[i]));
					index += 2;
				}
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->lsb1));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->lsb2));
				index += 2;
				for (int i = 0; i < 3; i++) {
					mb_get_binary_short(MB_YES, &buffer[index], &(sbp->reserved2[i]));
					index += 2;
				}
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->traceIDCode));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->validityFlag));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->reserved3));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->dataFormat));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->NMEAantennaeR));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->NMEAantennaeO));
				index += 2;
				for (int i = 0; i < 2; i++) {
					mb_get_binary_short(MB_YES, &buffer[index], &(sbp->reserved4[i]));
					index += 2;
				}
				mb_get_binary_float(MB_YES, &buffer[index], &(sbp->kmOfPipe));
				index += 4;
				for (int i = 0; i < 16; i++) {
					mb_get_binary_short(MB_YES, &buffer[index], &(sbp->reserved5[i]));
					index += 2;
				}
				mb_get_binary_int(MB_YES, &buffer[index], &(sbp->coordX));
				index += 4;
				mb_get_binary_int(MB_YES, &buffer[index], &(sbp->coordY));
				index += 4;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->coordUnits));
				index += 2;
				for (int i = 0; i < 24; i++) {
					sbp->annotation[i] = buffer[index];
					index++;
				}
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->samples));
				index += 2;
				mb_get_binary_int(MB_YES, &buffer[index], &(sbp->sampleInterval));
				index += 4;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->ADCGain));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->pulsePower));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->reserved6));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->startFreq));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->endFreq));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->sweepLength));   
				index += 2;
				mb_get_binary_int(MB_YES, &buffer[index], &(sbp->pressure));
				index += 4;
				mb_get_binary_int(MB_YES, &buffer[index], &(sbp->sonarDepth));
				index += 4;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->sampleFreq));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->pulseID));
				index += 2;
				mb_get_binary_int(MB_YES, &buffer[index], &(sbp->sonarAltitude));
				index += 4;
				mb_get_binary_float(MB_YES, &buffer[index], &(sbp->soundspeed));
				index += 4;
				mb_get_binary_float(MB_YES, &buffer[index], &(sbp->mixerFrequency));
				index += 4;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->year));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->day));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->hour));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->minute));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->second));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->timeBasis));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->weightingFactor));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->numberPulses));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->heading));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->pitch));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->roll));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->reserved8));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->reserved9));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->triggerSource));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->markNumber));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->NMEAHour));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->NMEAMinutes));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->NMEASeconds));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->NMEACourse));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->NMEASpeed));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->NMEADay));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->NMEAYear));
				index += 2;
				mb_get_binary_int(MB_YES, &buffer[index], &(sbp->millisecondsToday));
				index += 4;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->ADCMax));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->reserved10));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->reserved11));
				index += 2;
				for (int i = 0; i < 6; i++) {
					sbp->softwareVersion[i] = buffer[index];
					index++;
				}
				mb_get_binary_int(MB_YES, &buffer[index], &(sbp->sphericalCorrection));
				index += 4;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->packetNum));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->ADCDecimation));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->reserved12));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->temperature));
				index += 2;
				mb_get_binary_float(MB_YES, &buffer[index], &(sbp->layback));
				index += 4;
				mb_get_binary_int(MB_YES, &buffer[index], &(sbp->reserved13));
				index += 4;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->cableOut));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(sbp->reserved14));
				index += 2;
                
                /* fix problems created by writing obsolete header */
                if (obsolete_header == MB_YES) {
                    // set first time value
                    // zero extra position values
                    // move seafloor depth, sonar depth, and altitude values if nonzero
                    // set validity flags
                }

				/* allocate memory for the trace */
				if (sbp->dataFormat == 1)
					shortspersample = 2;
				else
					shortspersample = 1;
				trace_size = shortspersample * sbp->samples * sizeof(short);
				if (sbp->trace_alloc < trace_size) {
					if ((status = mb_reallocd(verbose, __FILE__, __LINE__, trace_size, (void **)&(sbp->trace), error)) ==
					    MB_SUCCESS) {
						sbp->trace_alloc = trace_size;
					}
					else
						fprintf(stderr, "TRACE ALLOCATION FAILED %s Line:%d Size:%d\n", __FILE__, __LINE__, trace_size);
				}

				/* read the trace */
				if (status == MB_SUCCESS && (read_status = fread(sbp->trace, trace_size, 1, mb_io_ptr->mbfp)) == 1) {
#ifndef BYTESWAPPED
					for (int i = 0; i < shortspersample * sbp->samples; i++) {
						sbp->trace[i] = mb_swap_short(sbp->trace[i]);
						;
					}
#endif
				}
				else {
					status = MB_FAILURE;
					*error = MB_ERROR_EOF;
					done = MB_YES;
					store->kind = MB_DATA_NONE;
				}

				/* get time */
				time_j[0] = sbp->year;
				time_j[1] = sbp->day;
				time_j[2] = 60 * sbp->hour + sbp->minute;
				time_j[3] = sbp->second;
				time_j[4] = (int)1000 * (sbp->millisecondsToday - 1000 * floor(0.001 * ((double)sbp->millisecondsToday)));
				mb_get_itime(verbose, time_j, time_i);
				mb_get_time(verbose, time_i, &time_d);

				/* set navigation and attitude if needed and available */
				if (sbp->heading == 0 && mb_io_ptr->nheading > 0) {
					mb_hedint_interp(verbose, mbio_ptr, time_d, &heading, error);
					sbp->heading = (short)(100.0 * heading);
				}
				if ((sbp->coordX == 0 || sbp->coordY == 0 || sbp->coordUnits == 2) && mb_io_ptr->nfix > 0) {
					mb_navint_interp(verbose, mbio_ptr, time_d, heading, 0.0, &navlon, &navlat, &speed, error);
					sbp->coordX = (int)(600000.0 * navlon);
					sbp->coordY = (int)(600000.0 * navlat);
				}
				if ((sbp->roll == 0 || sbp->pitch == 0) && mb_io_ptr->nattitude > 0) {
					mb_attint_interp(verbose, mbio_ptr, time_d, &heave, &roll, &pitch, error);
					sbp->roll = 32768 * roll / 180.0;
					sbp->pitch = 32768 * pitch / 180.0;
				}
				if (sbp->sonarAltitude == 0 && mb_io_ptr->naltitude > 0) {
					mb_altint_interp(verbose, mbio_ptr, time_d, &altitude, error);
					sbp->sonarAltitude = altitude / 1000.0;
				}
				if (sbp->sonarDepth == 0 && mb_io_ptr->nsonardepth > 0) {
					mb_depint_interp(verbose, mbio_ptr, time_d, &sonardepth, error);
					sbp->sonarDepth = sonardepth / 1000.0;
				}

				/* set kind */
				store->kind = MB_DATA_SUBBOTTOM_SUBBOTTOM;
				done = MB_YES;
			}

			/* end of file */
			else {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
				done = MB_YES;
				store->kind = MB_DATA_NONE;
			}
		}

		/* if sidescan data and sonar trace 80 proceed to get data */
		else if (status == MB_SUCCESS && message.type == MBSYS_JSTAR_DATA_SONAR &&
		         message.subsystem != MBSYS_JSTAR_SUBSYSTEM_SBP) {
			/* sidescan channel */
			if (message.channel == 0) {
				ss = (struct mbsys_jstar_channel_struct *)&(store->ssport);
			}
			else {
				ss = (struct mbsys_jstar_channel_struct *)&(store->ssstbd);
			}
			ss->message = message;

			/* read the 240 byte trace header */
			if ((read_status = fread(buffer, MBSYS_JSTAR_SBPHEADER_SIZE, 1, mb_io_ptr->mbfp)) == 1) {
                // test if these data were written incorrectly by a prior version of MB-System
                // - early versions of jsf maintained the segy structure of separate source and
                //   receiver position, and MB-System wrote jsf files that way up through 5.5.2321
				mb_get_binary_int(MB_YES, &buffer[72], &testx1);
				mb_get_binary_int(MB_YES, &buffer[76], &testy1);
				mb_get_binary_int(MB_YES, &buffer[80], &testx2);
				mb_get_binary_int(MB_YES, &buffer[84], &testy2);
                if (testx1 == testx2 && testy1 == testy2) {
                    obsolete_header = MB_YES;
                } else {
                    obsolete_header = MB_NO;
                }
                
				index = 0;
				mb_get_binary_int(MB_YES, &buffer[index], &(ss->pingTime));
				index += 4;
				mb_get_binary_int(MB_YES, &buffer[index], &(ss->startDepth));
				index += 4;
				mb_get_binary_int(MB_YES, &buffer[index], &(ss->pingNum));
				index += 4;
				for (int i = 0; i < 2; i++) {
					mb_get_binary_short(MB_YES, &buffer[index], &(ss->reserved1[i]));
					index += 2;
				}
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->lsb1));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->lsb2));
				index += 2;
				for (int i = 0; i < 3; i++) {
					mb_get_binary_short(MB_YES, &buffer[index], &(ss->reserved2[i]));
					index += 2;
				}
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->traceIDCode));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->validityFlag));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->reserved3));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->dataFormat));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->NMEAantennaeR));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->NMEAantennaeO));
				index += 2;
				for (int i = 0; i < 2; i++) {
					mb_get_binary_short(MB_YES, &buffer[index], &(ss->reserved4[i]));
					index += 2;
				}
				mb_get_binary_float(MB_YES, &buffer[index], &(ss->kmOfPipe));
				index += 4;
				for (int i = 0; i < 16; i++) {
					mb_get_binary_short(MB_YES, &buffer[index], &(ss->reserved5[i]));
					index += 2;
				}
				mb_get_binary_int(MB_YES, &buffer[index], &(ss->coordX));
				index += 4;
				mb_get_binary_int(MB_YES, &buffer[index], &(ss->coordY));
				index += 4;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->coordUnits));
				index += 2;
				for (int i = 0; i < 24; i++) {
					ss->annotation[i] = buffer[index];
					index++;
				}
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->samples));
				index += 2;
				mb_get_binary_int(MB_YES, &buffer[index], &(ss->sampleInterval));
				index += 4;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->ADCGain));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->pulsePower));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->reserved6));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->startFreq));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->endFreq));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->sweepLength));   
				index += 2;
				mb_get_binary_int(MB_YES, &buffer[index], &(ss->pressure));
				index += 4;
				mb_get_binary_int(MB_YES, &buffer[index], &(ss->sonarDepth));
				index += 4;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->sampleFreq));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->pulseID));
				index += 2;
				mb_get_binary_int(MB_YES, &buffer[index], &(ss->sonarAltitude));
				index += 4;
				mb_get_binary_float(MB_YES, &buffer[index], &(ss->soundspeed));
				index += 4;
				mb_get_binary_float(MB_YES, &buffer[index], &(ss->mixerFrequency));
				index += 4;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->year));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->day));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->hour));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->minute));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->second));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->timeBasis));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->weightingFactor));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->numberPulses));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->heading));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->pitch));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->roll));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->reserved8));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->reserved9));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->triggerSource));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->markNumber));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->NMEAHour));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->NMEAMinutes));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->NMEASeconds));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->NMEACourse));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->NMEASpeed));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->NMEADay));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->NMEAYear));
				index += 2;
				mb_get_binary_int(MB_YES, &buffer[index], &(ss->millisecondsToday));
				index += 4;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->ADCMax));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->reserved10));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->reserved11));
				index += 2;
				for (int i = 0; i < 6; i++) {
					ss->softwareVersion[i] = buffer[index];
					index++;
				}
				mb_get_binary_int(MB_YES, &buffer[index], &(ss->sphericalCorrection));
				index += 4;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->packetNum));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->ADCDecimation));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->reserved12));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->temperature));
				index += 2;
				mb_get_binary_float(MB_YES, &buffer[index], &(ss->layback));
				index += 4;
				mb_get_binary_int(MB_YES, &buffer[index], &(ss->reserved13));
				index += 4;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->cableOut));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ss->reserved14));
				index += 2;

				/* allocate memory for the trace */
				if (ss->dataFormat == 1)
					shortspersample = 2;
				else
					shortspersample = 1;
				trace_size = shortspersample * ss->samples * sizeof(short);
				if (ss->trace_alloc < trace_size) {
					if ((status = mb_reallocd(verbose, __FILE__, __LINE__, trace_size, (void **)&(ss->trace), error)) ==
					    MB_SUCCESS) {
						ss->trace_alloc = trace_size;
					}
					else
						fprintf(stderr, "TRACE ALLOCATION FAILED %s Line:%d Size:%d\n", __FILE__, __LINE__, trace_size);
				}

				/* read the trace */
				if (status == MB_SUCCESS && (read_status = fread(ss->trace, trace_size, 1, mb_io_ptr->mbfp)) == 1) {
#ifndef BYTESWAPPED
					for (int i = 0; i < shortspersample * ss->samples; i++) {
						ss->trace[i] = mb_swap_short(ss->trace[i]);
						;
					}
#endif
				}
				else {
					status = MB_FAILURE;
					*error = MB_ERROR_EOF;
					done = MB_YES;
					store->kind = MB_DATA_NONE;
				}

				/* get time */
				time_j[0] = ss->year;
				time_j[1] = ss->day;
				time_j[2] = 60 * ss->hour + ss->minute;
				time_j[3] = ss->second;
				time_j[4] = (int)1000 * (ss->millisecondsToday - 1000 * floor(0.001 * ((double)ss->millisecondsToday)));
				mb_get_itime(verbose, time_j, time_i);
				mb_get_time(verbose, time_i, &time_d);

				/* set navigation and attitude if needed and available */
				if (ss->heading == 0 && mb_io_ptr->nheading > 0) {
					mb_hedint_interp(verbose, mbio_ptr, time_d, &heading, error);
					ss->heading = (short)(100.0 * heading);
				}
				if ((ss->coordX == 0 || ss->coordY == 0 || ss->coordUnits == 2) && mb_io_ptr->nfix > 0) {
					mb_navint_interp(verbose, mbio_ptr, time_d, heading, 0.0, &navlon, &navlat, &speed, error);
					ss->coordX = (int)(600000.0 * navlon);
					ss->coordY = (int)(600000.0 * navlat);
				}
				if ((ss->roll == 0 || ss->pitch == 0) && mb_io_ptr->nattitude > 0) {
					mb_attint_interp(verbose, mbio_ptr, time_d, &heave, &roll, &pitch, error);
					ss->roll = 32768 * roll / 180.0;
					ss->pitch = 32768 * pitch / 180.0;
				}
				if (ss->sonarAltitude == 0 && mb_io_ptr->naltitude > 0) {
					mb_altint_interp(verbose, mbio_ptr, time_d, &altitude, error);
					ss->sonarAltitude = 1000 * altitude;
				}
				if (ss->sonarDepth == 0 && mb_io_ptr->nsonardepth > 0) {
					mb_depint_interp(verbose, mbio_ptr, time_d, &sonardepth, error);
					ss->sonarDepth = 1000 * sonardepth;
				}

				/* set kind */
				if (mb_io_ptr->format == MBF_EDGJSTAR) {
					if (message.subsystem == MBSYS_JSTAR_SUBSYSTEM_SSLOW)
						store->kind = MB_DATA_DATA;
					else if (message.subsystem == MBSYS_JSTAR_SUBSYSTEM_SSHIGH)
						store->kind = MB_DATA_SIDESCAN2;
//fprintf(stderr, "READ SIDESCAN format:%d subsystem:%d kind:%d\n", mb_io_ptr->format, message.subsystem, store->kind);
				}
				else {
					if (message.subsystem == MBSYS_JSTAR_SUBSYSTEM_SSHIGH)
						store->kind = MB_DATA_DATA;
					else if (message.subsystem == MBSYS_JSTAR_SUBSYSTEM_SSLOW)
						store->kind = MB_DATA_SIDESCAN2;
				}
				if (store->ssport.pingNum == store->ssstbd.pingNum &&
				    store->ssport.message.subsystem == store->ssstbd.message.subsystem) {
					done = MB_YES;
				}
#ifdef MBF_EDGJSTAR_DEBUG
				fprintf(stderr, "Done reading 1: %d  pingNum:%d %d   subsystem:%d %d\n", done, store->ssport.pingNum,
				        store->ssstbd.pingNum, store->ssport.message.subsystem, store->ssstbd.message.subsystem);
#endif
			}

			/* else end of file */
			else {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
				done = MB_YES;
				store->kind = MB_DATA_NONE;
			}
		}

		/* if subbottom data and sonar trace 82 proceed to get data
		    - translate to current form */
		else if (status == MB_SUCCESS && message.type == MBSYS_JSTAR_DATA_SONAR2 &&
		         message.subsystem == MBSYS_JSTAR_SUBSYSTEM_SBP) {
			/* reset message type to current sonar trace 80 */
			message.type = MBSYS_JSTAR_DATA_SONAR;

			/* sbp channel */
			sbp = (struct mbsys_jstar_channel_struct *)&(store->sbp);
			sbp->message = message;

			/* temporary old format structure */
			ssold_tmp.message = message;

			/* read the 80 byte trace header */
			if ((read_status = fread(buffer, MBSYS_JSTAR_SSOLDHEADER_SIZE, 1, mb_io_ptr->mbfp)) == 1) {
				index = 0;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.subsystem));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.channelNum));
				index += 2;
				mb_get_binary_int(MB_YES, &buffer[index], &(ssold_tmp.pingNum));
				index += 4;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.packetNum));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.trigSource));
				index += 2;
				mb_get_binary_int(MB_YES, &buffer[index], &(ssold_tmp.samples));
				index += 4;
				mb_get_binary_int(MB_YES, &buffer[index], &(ssold_tmp.sampleInterval));
				index += 4;
				mb_get_binary_int(MB_YES, &buffer[index], &(ssold_tmp.startDepth));
				index += 4;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.weightingFactor));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.ADCGain));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.ADCMax));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.rangeSetting));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.pulseID));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.markNumber));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.dataFormat));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.reserved));
				index += 2;
				mb_get_binary_int(MB_YES, &buffer[index], &(ssold_tmp.millisecondsToday));
				index += 4;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.year));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.day));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.hour));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.minute));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.second));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.heading));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.pitch));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.roll));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.heave));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.yaw));
				index += 2;
				mb_get_binary_int(MB_YES, &buffer[index], &(ssold_tmp.depth));
				index += 4;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.temperature));
				index += 2;
				for (int i = 0; i < 10; i++) {
					ssold_tmp.reserved2[i] = buffer[index];
					index++;
				}

				/* translate traceheader to the current form */
				sbp->pingTime = 0;
				sbp->startDepth = ssold_tmp.startDepth;
				sbp->pingNum = ssold_tmp.pingNum;
                sbp->reserved1[0] = 0;
                sbp->reserved1[1] = 0;
                sbp->msb = 0;
                sbp->lsb1 = 0;
                sbp->lsb2 = 0;
				for (int i = 0; i < 3; i++) {
                    sbp->reserved2[0] = 0;
				}
				sbp->traceIDCode = 1;
				sbp->validityFlag = 0;
				sbp->reserved3 = 0;
				sbp->dataFormat = ssold_tmp.dataFormat;
				sbp->NMEAantennaeR = 0;
				sbp->NMEAantennaeO = 0;
				for (int i = 0; i < 2; i++) {
					sbp->reserved4[i] = 0;
				}
                sbp->kmOfPipe = 0.0;
				for (int i = 0; i < 16; i++) {
					sbp->reserved5[i] = 0;
				}
				sbp->coordX = 0;
				sbp->coordY = 0;
				sbp->coordUnits = 2;
				for (int i = 0; i < 24; i++) {
					sbp->annotation[i] = 0;
				}
				sbp->samples = ssold_tmp.samples;
				sbp->sampleInterval = ssold_tmp.sampleInterval;
				sbp->ADCGain = ssold_tmp.ADCGain;
				sbp->pulsePower = 0;
				sbp->reserved6 = 0;
				sbp->startFreq = 0;
				sbp->endFreq = 0;
				sbp->sweepLength = 0;
				sbp->pressure = 0;
				sbp->sonarDepth = 0;
				sbp->sampleFreq = (unsigned short)(500000000.0 / sbp->sampleInterval);
				sbp->pulseID = ssold_tmp.pulseID;
				sbp->sonarAltitude = 0;
				sbp->soundspeed = 0.0;
				sbp->mixerFrequency = 0.0;
				sbp->year = ssold_tmp.year;
				sbp->day = ssold_tmp.day;
				sbp->hour = ssold_tmp.hour;
				sbp->minute = ssold_tmp.minute;
				sbp->second = ssold_tmp.second;
				sbp->timeBasis = 3;
				sbp->weightingFactor = ssold_tmp.weightingFactor;
				sbp->numberPulses = 0;
				sbp->heading = (short)(100.0 * ssold_tmp.heading / 60.0);
				sbp->pitch = (short)(100.0 * ssold_tmp.pitch / 60.0);
				sbp->roll = (short)(100.0 * ssold_tmp.roll / 60.0);
				sbp->reserved8 = 0;
				sbp->reserved9 = 0;
				sbp->triggerSource = 0;
				sbp->markNumber = 0;
				sbp->NMEAHour = 0;
				sbp->NMEAMinutes = 0;
				sbp->NMEASeconds = 0;
				sbp->NMEACourse = 0;
				sbp->NMEASpeed = 0;
				sbp->NMEADay = 0;
				sbp->NMEAYear = 0;
				sbp->millisecondsToday = ssold_tmp.millisecondsToday;
				sbp->ADCMax = ssold_tmp.ADCMax;
				sbp->reserved10 = 0;
				sbp->reserved11 = 0;
				for (int i = 0; i < 6; i++) {
					sbp->softwareVersion[i] = 0;
				}
				sbp->sphericalCorrection = 0;
				sbp->packetNum = 1;
				sbp->ADCDecimation = 0;
				sbp->reserved12 = 0;
				sbp->temperature = ssold_tmp.temperature;
				sbp->layback = 0.0;
				sbp->reserved13 = 0;
				sbp->cableOut = 0;
				sbp->reserved14 = 0;

				/* allocate memory for the trace */
				if (sbp->dataFormat == 1)
					shortspersample = 2;
				else
					shortspersample = 1;
				trace_size = shortspersample * sbp->samples * sizeof(short);
				if (sbp->trace_alloc < trace_size) {
					if ((status = mb_reallocd(verbose, __FILE__, __LINE__, trace_size, (void **)&(sbp->trace), error)) ==
					    MB_SUCCESS) {
						sbp->trace_alloc = trace_size;
					}
					else
						fprintf(stderr, "TRACE ALLOCATION FAILED %s Line:%d Size:%d\n", __FILE__, __LINE__, trace_size);
				}

				/* read the trace */
				if (status == MB_SUCCESS && (read_status = fread(sbp->trace, trace_size, 1, mb_io_ptr->mbfp)) == 1) {
#ifndef BYTESWAPPED
					for (int i = 0; i < shortspersample * sbp->samples; i++) {
						sbp->trace[i] = mb_swap_short(sbp->trace[i]);
					}
#endif
				}
				else {
					status = MB_FAILURE;
					*error = MB_ERROR_EOF;
					done = MB_YES;
					store->kind = MB_DATA_NONE;
				}

				/* get time */
				time_j[0] = sbp->year;
				time_j[1] = sbp->day;
				time_j[2] = 60 * sbp->hour + sbp->minute;
				time_j[3] = sbp->second;
				time_j[4] = (int)1000 * (sbp->millisecondsToday - 1000 * floor(0.001 * ((double)sbp->millisecondsToday)));
				mb_get_itime(verbose, time_j, time_i);
				mb_get_time(verbose, time_i, &time_d);

				/* set navigation and attitude if needed and available */
				if (sbp->heading == 0 && mb_io_ptr->nheading > 0) {
					mb_hedint_interp(verbose, mbio_ptr, time_d, &heading, error);
					sbp->heading = (short)(100.0 * heading);
				}
				if ((sbp->coordX == 0 || sbp->coordY == 0 || sbp->coordUnits == 2) && mb_io_ptr->nfix > 0) {
					mb_navint_interp(verbose, mbio_ptr, time_d, heading, 0.0, &navlon, &navlat, &speed, error);
					sbp->coordX = (int)(600000.0 * navlon);
					sbp->coordY = (int)(600000.0 * navlat);
				}
				if ((sbp->roll == 0 || sbp->pitch == 0) && mb_io_ptr->nattitude > 0) {
					mb_attint_interp(verbose, mbio_ptr, time_d, &heave, &roll, &pitch, error);
					sbp->roll = 32768 * roll / 180.0;
					sbp->pitch = 32768 * pitch / 180.0;
				}
				if (sbp->sonarAltitude == 0 && mb_io_ptr->naltitude > 0) {
					mb_altint_interp(verbose, mbio_ptr, time_d, &altitude, error);
					sbp->sonarAltitude = altitude / 1000.0;
				}
				if (sbp->sonarDepth == 0 && mb_io_ptr->nsonardepth > 0) {
					mb_depint_interp(verbose, mbio_ptr, time_d, &sonardepth, error);
					sbp->sonarDepth = sonardepth / 1000.0;
				}

				/* set kind */
				store->kind = MB_DATA_SUBBOTTOM_SUBBOTTOM;
				done = MB_YES;
			}

			/* end of file */
			else {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
				done = MB_YES;
				store->kind = MB_DATA_NONE;
			}
		}

		/* if sidescan data and sonar trace 82 proceed to get data
		    - translate to current form */
		else if (status == MB_SUCCESS && message.type == MBSYS_JSTAR_DATA_SONAR2 &&
		         message.subsystem != MBSYS_JSTAR_SUBSYSTEM_SBP) {
			/* reset message type to current sonar trace 80 */
			message.type = MBSYS_JSTAR_DATA_SONAR;

			/* sidescan channel */
			if (message.channel == 0)
				ss = (struct mbsys_jstar_channel_struct *)&(store->ssport);
			else
				ss = (struct mbsys_jstar_channel_struct *)&(store->ssstbd);
			ss->message = message;

			/* temporary old format structure */
			ssold_tmp.message = message;

			/* read the 80 byte trace header */
			if ((read_status = fread(buffer, MBSYS_JSTAR_SSOLDHEADER_SIZE, 1, mb_io_ptr->mbfp)) == 1) {
				index = 0;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.subsystem));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.channelNum));
				index += 2;
				mb_get_binary_int(MB_YES, &buffer[index], &(ssold_tmp.pingNum));
				index += 4;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.packetNum));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.trigSource));
				index += 2;
				mb_get_binary_int(MB_YES, &buffer[index], &(ssold_tmp.samples));
				index += 4;
				mb_get_binary_int(MB_YES, &buffer[index], &(ssold_tmp.sampleInterval));
				index += 4;
				mb_get_binary_int(MB_YES, &buffer[index], &(ssold_tmp.startDepth));
				index += 4;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.weightingFactor));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.ADCGain));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.ADCMax));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.rangeSetting));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.pulseID));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.markNumber));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.dataFormat));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.reserved));
				index += 2;
				mb_get_binary_int(MB_YES, &buffer[index], &(ssold_tmp.millisecondsToday));
				index += 4;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.year));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.day));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.hour));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.minute));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.second));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.heading));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.pitch));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.roll));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.heave));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.yaw));
				index += 2;
				mb_get_binary_int(MB_YES, &buffer[index], &(ssold_tmp.depth));
				index += 4;
				mb_get_binary_short(MB_YES, &buffer[index], &(ssold_tmp.temperature));
				index += 2;
				for (int i = 0; i < 10; i++) {
					ssold_tmp.reserved2[i] = buffer[index];
					index++;
				}

				/* translate traceheader to the current form */
				ss->pingTime = 0;
				ss->startDepth = ssold_tmp.startDepth;
				ss->pingNum = ssold_tmp.pingNum;
                ss->reserved1[0] = 0;
                ss->reserved1[1] = 0;
                ss->msb = 0;
                ss->lsb1 = 0;
                ss->lsb2 = 0;
				for (int i = 0; i < 3; i++) {
                    ss->reserved2[0] = 0;
				}
				ss->traceIDCode = 1;
				ss->validityFlag = 0;
				ss->reserved3 = 0;
				ss->dataFormat = ssold_tmp.dataFormat;
				ss->NMEAantennaeR = 0;
				ss->NMEAantennaeO = 0;
				for (int i = 0; i < 2; i++) {
					ss->reserved4[i] = 0;
				}
                ss->kmOfPipe = 0.0;
				for (int i = 0; i < 16; i++) {
					ss->reserved5[i] = 0;
				}
				ss->coordX = 0;
				ss->coordY = 0;
				ss->coordUnits = 2;
				for (int i = 0; i < 24; i++) {
					ss->annotation[i] = 0;
				}
				ss->samples = ssold_tmp.samples;
				ss->sampleInterval = ssold_tmp.sampleInterval;
				ss->ADCGain = ssold_tmp.ADCGain;
				ss->pulsePower = 0;
				ss->reserved6 = 0;
				ss->startFreq = 0;
				ss->endFreq = 0;
				ss->sweepLength = 0;
				ss->pressure = 0;
				ss->sonarDepth = 0;
				ss->sampleFreq = (unsigned short)(500000000.0 / ss->sampleInterval);
				ss->pulseID = ssold_tmp.pulseID;
				ss->sonarAltitude = 0;
				ss->soundspeed = 0.0;
				ss->mixerFrequency = 0.0;
				ss->year = ssold_tmp.year;
				ss->day = ssold_tmp.day;
				ss->hour = ssold_tmp.hour;
				ss->minute = ssold_tmp.minute;
				ss->second = ssold_tmp.second;
				ss->timeBasis = 3;
				ss->weightingFactor = ssold_tmp.weightingFactor;
				ss->numberPulses = 0;
				ss->heading = (short)(100.0 * ssold_tmp.heading / 60.0);
				ss->pitch = (short)(100.0 * ssold_tmp.pitch / 60.0);
				ss->roll = (short)(100.0 * ssold_tmp.roll / 60.0);
				ss->reserved8 = 0;
				ss->reserved9 = 0;
				ss->triggerSource = 0;
				ss->markNumber = 0;
				ss->NMEAHour = 0;
				ss->NMEAMinutes = 0;
				ss->NMEASeconds = 0;
				ss->NMEACourse = 0;
				ss->NMEASpeed = 0;
				ss->NMEADay = 0;
				ss->NMEAYear = 0;
				ss->millisecondsToday = ssold_tmp.millisecondsToday;
				ss->ADCMax = ssold_tmp.ADCMax;
				ss->reserved10 = 0;
				ss->reserved11 = 0;
				for (int i = 0; i < 6; i++) {
					ss->softwareVersion[i] = 0;
				}
				ss->sphericalCorrection = 0;
				ss->packetNum = 1;
				ss->ADCDecimation = 0;
				ss->reserved12 = 0;
				ss->temperature = ssold_tmp.temperature;
				ss->layback = 0.0;
				ss->reserved13 = 0;
				ss->cableOut = 0;
				ss->reserved14 = 0;

				/* allocate memory for the trace */
				if (ss->dataFormat == 1)
					shortspersample = 2;
				else
					shortspersample = 1;
				trace_size = shortspersample * ss->samples * sizeof(short);
				if (ss->trace_alloc < trace_size) {
					if ((status = mb_reallocd(verbose, __FILE__, __LINE__, trace_size, (void **)&(ss->trace), error)) ==
					    MB_SUCCESS) {
						ss->trace_alloc = trace_size;
					}
					else
						fprintf(stderr, "TRACE ALLOCATION FAILED %s Line:%d Size:%d\n", __FILE__, __LINE__, trace_size);
				}

				/* read the trace */
				if (status == MB_SUCCESS && (read_status = fread(ss->trace, trace_size, 1, mb_io_ptr->mbfp)) == 1) {
#ifndef BYTESWAPPED
					for (int i = 0; i < shortspersample * ss->samples; i++) {
						ss->trace[i] = mb_swap_short(ss->trace[i]);
						;
					}
#endif
				}
				else {
					status = MB_FAILURE;
					*error = MB_ERROR_EOF;
					done = MB_YES;
					store->kind = MB_DATA_NONE;
				}

				/* get time */
				time_j[0] = ss->year;
				time_j[1] = ss->day;
				time_j[2] = 60 * ss->hour + ss->minute;
				time_j[3] = ss->second;
				time_j[4] = (int)1000 * (ss->millisecondsToday - 1000 * floor(0.001 * ((double)ss->millisecondsToday)));
				mb_get_itime(verbose, time_j, time_i);
				mb_get_time(verbose, time_i, &time_d);

				/* set navigation and attitude if needed and available */
				if (ss->heading == 0 && mb_io_ptr->nheading > 0) {
					mb_hedint_interp(verbose, mbio_ptr, time_d, &heading, error);
					ss->heading = (short)(100.0 * heading);
				}
				if ((ss->coordX == 0 || ss->coordY == 0 || ss->coordUnits == 2) && mb_io_ptr->nfix > 0) {
					mb_navint_interp(verbose, mbio_ptr, time_d, heading, 0.0, &navlon, &navlat, &speed, error);
					ss->coordX = (int)(600000.0 * navlon);
					ss->coordY = (int)(600000.0 * navlat);
				}
				if ((ss->roll == 0 || ss->pitch == 0) && mb_io_ptr->nattitude > 0) {
					mb_attint_interp(verbose, mbio_ptr, time_d, &heave, &roll, &pitch, error);
					ss->roll = 32768 * roll / 180.0;
					ss->pitch = 32768 * pitch / 180.0;
				}
				if (ss->sonarAltitude == 0 && mb_io_ptr->naltitude > 0) {
					mb_altint_interp(verbose, mbio_ptr, time_d, &altitude, error);
					ss->sonarAltitude = 1000 * altitude;
				}
				if (ss->sonarDepth == 0 && mb_io_ptr->nsonardepth > 0) {
					mb_depint_interp(verbose, mbio_ptr, time_d, &sonardepth, error);
					ss->sonarDepth = 1000 * sonardepth;
				}

				/* set kind */
				if (mb_io_ptr->format == MBF_EDGJSTAR) {
					if (message.subsystem == MBSYS_JSTAR_SUBSYSTEM_SSLOW)
						store->kind = MB_DATA_DATA;
					else if (message.subsystem == MBSYS_JSTAR_SUBSYSTEM_SSHIGH)
						store->kind = MB_DATA_SIDESCAN2;
				}
				else {
					if (message.subsystem == MBSYS_JSTAR_SUBSYSTEM_SSHIGH)
						store->kind = MB_DATA_DATA;
					else if (message.subsystem == MBSYS_JSTAR_SUBSYSTEM_SSLOW)
						store->kind = MB_DATA_SIDESCAN2;
				}
				if (store->ssport.pingNum == store->ssstbd.pingNum &&
				    store->ssport.message.subsystem == store->ssstbd.message.subsystem) {
					done = MB_YES;
				}
#ifdef MBF_EDGJSTAR_DEBUG
				fprintf(stderr, "Done reading 1: %d  pingNum:%d %d   subsystem:%d %d\n", done, store->ssport.pingNum,
				        store->ssstbd.pingNum, store->ssport.message.subsystem, store->ssstbd.message.subsystem);
#endif
			}

			/* else end of file */
			else {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
				done = MB_YES;
				store->kind = MB_DATA_NONE;
			}
		}

		/* system info record */
		else if (status == MB_SUCCESS && message.type == MBSYS_JSTAR_DATA_SYSINFO) {
			/* get message */
			sysinfo = (struct mbsys_jstar_sysinfo_struct *)&(store->sysinfo);
			sysinfo->message = message;

			/* read the system info record */
			if ((read_status = fread(buffer, message.size, 1, mb_io_ptr->mbfp)) == 1) {
				index = 0;
				mb_get_binary_int(MB_YES, &buffer[index], &(sysinfo->system_type));
				index += 4;
				mb_get_binary_int(MB_YES, &buffer[index], &(sysinfo->reserved1));
				index += 4;
				mb_get_binary_int(MB_YES, &buffer[index], &(sysinfo->version));
				index += 4;
				mb_get_binary_int(MB_YES, &buffer[index], &(sysinfo->reserved2));
				index += 4;
				mb_get_binary_int(MB_YES, &buffer[index], &(sysinfo->platformserialnumber));
				index += 4;
				sysinfo->sysinfosize = MIN((message.size - index), MBSYS_JSTAR_SYSINFO_MAX - 1);
				for (int i = 0; i < sysinfo->sysinfosize; i++) {
					sysinfo->sysinfo[i] = buffer[index];
					index++;
				}
				sysinfo->sysinfo[sysinfo->sysinfosize] = '\0';
#ifdef MBF_EDGJSTAR_DEBUG
				fprintf(stderr, "SYSINFO: system_type:%d version:%d platformserialnumber:%d sysinfosize:%d\n",
				        sysinfo->system_type, sysinfo->version, sysinfo->platformserialnumber, sysinfo->sysinfosize);
#endif

				done = MB_YES;
				store->kind = MB_DATA_HEADER;
			}
		}

		/* file timestamp record */
		else if (status == MB_SUCCESS && message.type == MBSYS_JSTAR_DATA_FILETIMESTAMP) {
			/* get message */
			filetimestamp = (struct mbsys_jstar_filetimestamp_struct *)&(store->filetimestamp);
			filetimestamp->message = message;

			/* read the file timestamp record */
			if ((read_status = fread(buffer, message.size, 1, mb_io_ptr->mbfp)) == 1) {
				index = 0;
				mb_get_binary_int(MB_YES, &buffer[index], &(filetimestamp->seconds));
				index += 4;
				mb_get_binary_int(MB_YES, &buffer[index], &(filetimestamp->milliseconds));
				index += 4;

				done = MB_YES;
				store->kind = MB_DATA_TIMESTAMP;
			}
		}

		/* if nmea data read it, parse it, and and store values for interpolation */
		else if (status == MB_SUCCESS && message.type == MBSYS_JSTAR_DATA_NMEA && message.size < MB_COMMENT_MAXLINE) {
			/* nmea channel */
			nmea = (struct mbsys_jstar_nmea_struct *)&(store->nmea);
			nmea->message = message;

			/* read the NMEA string */
			if ((read_status = fread(buffer, message.size, 1, mb_io_ptr->mbfp)) == 1) {
				index = 0;
				mb_get_binary_int(MB_YES, &buffer[index], &(nmea->seconds));
				index += 4;
				mb_get_binary_int(MB_YES, &buffer[index], &(nmea->milliseconds));
				index += 4;
				nmea->source = buffer[index];
				index++;
				nmea->reserve[0] = buffer[index];
				index++;
				nmea->reserve[1] = buffer[index];
				index++;
				nmea->reserve[2] = buffer[index];
				index++;
				for (int i = 0; i < message.size - 12; i++) {
					nmea->nmea[i] = buffer[index];
					index++;
				}
				nmea->nmea[message.size - 12] = 0;
				strcpy(nmeastring, nmea->nmea);

				time_d = ((double)nmea->seconds) + 0.001 * ((double)nmea->milliseconds);
				mb_get_date(verbose, time_d, time_i);

				/* break up NMEA string into arguments */
				nargc = 0;
				string = (char *)nmeastring;
				for (nap = nargv; (*nap = strtok(string, ",*")) != NULL;) {
					if (++nap >= &nargv[25])
						break;
					else
						nargc++;
				}

				/* parse NMEA string if possible */
				if (strncmp(&(nargv[0][3]), "RMC", 3) == 0) {
					rawvalue = atof(nargv[3]);
					navlat = floor(0.01 * rawvalue) + (rawvalue - 100.0 * floor(0.01 * rawvalue)) / 60.0;
					if (nargv[4][0] == 'S')
						navlat *= -1.0;
					rawvalue = atof(nargv[5]);
					navlon = floor(0.01 * rawvalue) + (rawvalue - 100.0 * floor(0.01 * rawvalue)) / 60.0;
					if (nargv[6][0] == 'W')
						navlon *= -1.0;
					heading = atof(nargv[8]);
#ifdef MBF_EDGJSTAR_DEBUG
					fprintf(stderr, "RMC: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d navlon:%f navlat:%f heading:%f    %s\n",
					        time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], navlon, navlat, heading,
					        nmea->nmea);
#endif

					mb_navint_add(verbose, mbio_ptr, time_d, navlon, navlat, error);
					mb_hedint_add(verbose, mbio_ptr, time_d, heading, error);

					store->kind = MB_DATA_NMEA_RMC;
				}

				/* parse NMEA string if possible */
				else if (strncmp(&(nargv[0][3]), "DBT", 3) == 0) {
					altitude = atof(nargv[3]);
					mb_altint_add(verbose, mbio_ptr, time_d, altitude, error);
#ifdef MBF_EDGJSTAR_DEBUG
					fprintf(stderr, "DBT: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d altitude:%f    %s\n", time_i[0], time_i[1],
					        time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], altitude, nmea->nmea);
#endif

					store->kind = MB_DATA_NMEA_DBT;
				}

				/* parse NMEA string if possible */
				else if (strncmp(&(nargv[0][3]), "DPT", 3) == 0) {
					depthofsensor = atof(nargv[1]);
					offset = atof(nargv[2]);
					sonardepth = depthofsensor + offset;
					mb_depint_add(verbose, mbio_ptr, time_d, sonardepth, error);
#ifdef MBF_EDGJSTAR_DEBUG
					fprintf(stderr,
					        "DPT: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d depthofsensor:%f offset:%f sonardepth:%f    %s\n",
					        time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], depthofsensor, offset,
					        sonardepth, nmea->nmea);
#endif

					store->kind = MB_DATA_NMEA_DPT;
				}

				done = MB_YES;
			}
		}

		/* if pitchroll data read it */
		else if (status == MB_SUCCESS && message.type == MBSYS_JSTAR_DATA_PITCHROLL && message.size < MB_COMMENT_MAXLINE) {
			/* nmea channel */
			pitchroll = (struct mbsys_jstar_pitchroll_struct *)&(store->pitchroll);
			pitchroll->message = message;

			/* read the pitchroll record */
			if ((read_status = fread(buffer, message.size, 1, mb_io_ptr->mbfp)) == 1) {
				index = 0;
				mb_get_binary_int(MB_YES, &buffer[index], &(pitchroll->seconds));
				index += 4;
				mb_get_binary_int(MB_YES, &buffer[index], &(pitchroll->milliseconds));
				index += 4;
				pitchroll->reserve1[0] = buffer[index];
				index++;
				pitchroll->reserve1[1] = buffer[index];
				index++;
				pitchroll->reserve1[2] = buffer[index];
				index++;
				pitchroll->reserve1[3] = buffer[index];
				index++;
				mb_get_binary_short(MB_YES, &buffer[index], &(pitchroll->accelerationx));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(pitchroll->accelerationy));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(pitchroll->accelerationz));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(pitchroll->gyroratex));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(pitchroll->gyroratey));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(pitchroll->gyroratez));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(pitchroll->pitch));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(pitchroll->roll));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(pitchroll->temperature));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(pitchroll->deviceinfo));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(pitchroll->heave));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(pitchroll->heading));
				index += 2;
				mb_get_binary_int(MB_YES, &buffer[index], &(pitchroll->datavalidflags));
				index += 4;
				mb_get_binary_int(MB_YES, &buffer[index], &(pitchroll->reserve2));
				index += 4;

				done = MB_YES;
				store->kind = MB_DATA_ATTITUDE;
			}
		}

		/* if pressure data read it */
		else if (status == MB_SUCCESS && message.type == MBSYS_JSTAR_DATA_PRESSURE && message.size < MB_COMMENT_MAXLINE) {
			/* nmea channel */
			pressure = (struct mbsys_jstar_pressure_struct *)&(store->pressure);
			pressure->message = message;

			/* read the pressure record */
			if ((read_status = fread(buffer, message.size, 1, mb_io_ptr->mbfp)) == 1) {
				index = 0;
				mb_get_binary_int(MB_YES, &buffer[index], &(pressure->seconds));
				index += 4;
				mb_get_binary_int(MB_YES, &buffer[index], &(pressure->milliseconds));
				index += 4;
				pressure->reserve1[0] = buffer[index];
				index++;
				pressure->reserve1[1] = buffer[index];
				index++;
				pressure->reserve1[2] = buffer[index];
				index++;
				pressure->reserve1[3] = buffer[index];
				index++;
				mb_get_binary_int(MB_YES, &buffer[index], &(pressure->pressure));
				index += 4;
				mb_get_binary_int(MB_YES, &buffer[index], &(pressure->salinity));
				index += 4;
				mb_get_binary_int(MB_YES, &buffer[index], &(pressure->datavalidflags));
				index += 4;
				mb_get_binary_int(MB_YES, &buffer[index], &(pressure->conductivity));
				index += 4;
				mb_get_binary_int(MB_YES, &buffer[index], &(pressure->soundspeed));
				index += 4;
				for (int i = 0; i < 10; i++) {
					mb_get_binary_int(MB_YES, &buffer[index], &(pressure->reserve2[i]));
					index += 4;
				}
#ifdef MBF_EDGJSTAR_DEBUG
				fprintf(stderr, "PRESSURE: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d pressure:%d soundspeed:%d\n", time_i[0],
				        time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], pressure->pressure,
				        pressure->soundspeed);
#endif

				done = MB_YES;
				store->kind = MB_DATA_CTD;
			}

			/* end of file */
			else {
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
				done = MB_YES;
				store->kind = MB_DATA_NONE;
			}
		}

		/* if dvl data read it */
		else if (status == MB_SUCCESS && message.type == MBSYS_JSTAR_DATA_DVL && message.size < MB_COMMENT_MAXLINE) {
			/* nmea channel */
			dvl = (struct mbsys_jstar_dvl_struct *)&(store->dvl);
			dvl->message = message;

			/* read the dvl record */
			if ((read_status = fread(buffer, message.size, 1, mb_io_ptr->mbfp)) == 1) {
				index = 0;
				mb_get_binary_int(MB_YES, &buffer[index], &(dvl->seconds));
				index += 4;
				mb_get_binary_int(MB_YES, &buffer[index], &(dvl->milliseconds));
				index += 4;
				dvl->reserve1[0] = buffer[index];
				index++;
				dvl->reserve1[1] = buffer[index];
				index++;
				dvl->reserve1[2] = buffer[index];
				index++;
				dvl->reserve1[3] = buffer[index];
				index++;
				mb_get_binary_int(MB_YES, &buffer[index], &(dvl->datavalidflags));
				index += 4;
				mb_get_binary_int(MB_YES, &buffer[index], &(dvl->beam1range));
				index += 4;
				mb_get_binary_int(MB_YES, &buffer[index], &(dvl->beam2range));
				index += 4;
				mb_get_binary_int(MB_YES, &buffer[index], &(dvl->beam3range));
				index += 4;
				mb_get_binary_int(MB_YES, &buffer[index], &(dvl->beam4range));
				index += 4;
				mb_get_binary_short(MB_YES, &buffer[index], &(dvl->velocitybottomx));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(dvl->velocitybottomy));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(dvl->velocitybottomz));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(dvl->velocitywaterx));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(dvl->velocitywatery));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(dvl->velocitywaterz));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(dvl->depth));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(dvl->pitch));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(dvl->roll));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(dvl->heading));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(dvl->salinity));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(dvl->temperature));
				index += 2;
				mb_get_binary_short(MB_YES, &buffer[index], &(dvl->soundspeed));
				index += 2;
				for (int i = 0; i < 7; i++) {
					mb_get_binary_short(MB_YES, &buffer[index], &(dvl->reserve2[i]));
					index += 2;
				}

				done = MB_YES;
				store->kind = MB_DATA_DVL;
#ifdef MBF_EDGJSTAR_DEBUG
				fprintf(stderr,
				        "DVL: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d  beams:%d %d %d %d   velocity:%d %d %d  depth:%d "
				        "pitch:%d roll:%d heading:%d soundspeed:%d\n",
				        time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], dvl->beam1range,
				        dvl->beam2range, dvl->beam3range, dvl->beam4range, dvl->velocitybottomx, dvl->velocitybottomy,
				        dvl->velocitybottomz, dvl->depth, dvl->pitch, dvl->roll, dvl->heading, dvl->soundspeed);
#endif
			}
		}

		/* if not supported data read it and throw it away */
		else if (status == MB_SUCCESS) {
#ifdef MBF_EDGJSTAR_DEBUG
			fprintf(stderr, "UNKNOWN: throwing away %d bytes\n", message.size);
#endif
			for (int i = 0; i < message.size; i++) {
				read_status = fread(buffer, 1, 1, mb_io_ptr->mbfp);
			}
			done = MB_YES;
			store->kind = MB_DATA_NONE;
			*error = MB_ERROR_UNINTELLIGIBLE;
		}

		/* end of file */
		else {
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			done = MB_YES;
			store->kind = MB_DATA_NONE;
		}
	}

	/* set kind and error in mb_io_ptr */
	mb_io_ptr->new_kind = store->kind;
	mb_io_ptr->new_error = *error;
#ifdef MBF_EDGJSTAR_DEBUG
	fprintf(stderr, "kind:%d error:%d status:%d\n", store->kind, *error, status);
#endif

	/* print debug statements */
	if (status == MB_SUCCESS && verbose >= 5 && store->kind == MB_DATA_COMMENT) {
		fprintf(stderr, "\ndbg5  New comment read by MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5  Subsystem ID:\n");
		fprintf(stderr, "dbg5       subsystem:        %d ", store->subsystem);
		if (store->subsystem == 0)
			fprintf(stderr, "(subbottom)\n");
		else if (store->subsystem == MBSYS_JSTAR_SUBSYSTEM_SSLOW)
			fprintf(stderr, "(75 or 120 kHz sidescan)\n");
		else if (store->subsystem == MBSYS_JSTAR_SUBSYSTEM_SSHIGH)
			fprintf(stderr, "(410 kHz sidescan)\n");

		fprintf(stderr, "\ndbg5  Comment:\n");
		comment = (struct mbsys_jstar_comment_struct *)&(store->comment);
		fprintf(stderr, "dbg5     start_marker:                %d\n", comment->message.start_marker);
		fprintf(stderr, "dbg5     version:                     %d\n", comment->message.version);
		fprintf(stderr, "dbg5     session:                     %d\n", comment->message.session);
		fprintf(stderr, "dbg5     type:                        %d\n", comment->message.type);
		fprintf(stderr, "dbg5     command:                     %d\n", comment->message.command);
		fprintf(stderr, "dbg5     subsystem:                   %d\n", comment->message.subsystem);
		fprintf(stderr, "dbg5     channel:                     %d\n", comment->message.channel);
		fprintf(stderr, "dbg5     sequence:                    %d\n", comment->message.sequence);
		fprintf(stderr, "dbg5     reserved:                    %d\n", comment->message.reserved);
		fprintf(stderr, "dbg5     size:                        %d\n", comment->message.size);

		fprintf(stderr, "dbg5     comment:                     %s\n", store->comment.comment);
	}
	else if (status == MB_SUCCESS && verbose >= 5 && store->kind == MB_DATA_SUBBOTTOM_SUBBOTTOM) {
		sbp = (struct mbsys_jstar_channel_struct *)&(store->sbp);
		fprintf(stderr, "\ndbg5  New subbottom data record read by MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5  Subsystem ID:\n");
		fprintf(stderr, "dbg5       subsystem:        %d (subbottom)\n", store->subsystem);
		fprintf(stderr, "\ndbg5  Channel:\n");
		fprintf(stderr, "dbg5     start_marker:                %d\n", sbp->message.start_marker);
		fprintf(stderr, "dbg5     version:                     %d\n", sbp->message.version);
		fprintf(stderr, "dbg5     session:                     %d\n", sbp->message.session);
		fprintf(stderr, "dbg5     type:                        %d\n", sbp->message.type);
		fprintf(stderr, "dbg5     command:                     %d\n", sbp->message.command);
		fprintf(stderr, "dbg5     subsystem:                   %d\n", sbp->message.subsystem);
		fprintf(stderr, "dbg5     channel:                     %d\n", sbp->message.channel);
		fprintf(stderr, "dbg5     sequence:                    %d\n", sbp->message.sequence);
		fprintf(stderr, "dbg5     reserved:                    %d\n", sbp->message.reserved);
		fprintf(stderr, "dbg5     size:                        %d\n", sbp->message.size);

		fprintf(stderr, "dbg5     pingTime:                    %d\n", sbp->pingTime);
		fprintf(stderr, "dbg5     startDepth:                  %d\n", sbp->startDepth);
		fprintf(stderr, "dbg5     pingNum:                     %d\n", sbp->pingNum);
		for (int i = 0; i < 2; i++)
            fprintf(stderr, "dbg5     reserved1[%d]:               %d\n", i, sbp->reserved1[i]);
		fprintf(stderr, "dbg5     msb:                         %d\n", sbp->msb);
		fprintf(stderr, "dbg5     lsb1:                        %d\n", sbp->lsb1);
		fprintf(stderr, "dbg5     lsb2:                        %d\n", sbp->lsb2);
		for (int i = 0; i < 3; i++)
            fprintf(stderr, "dbg5     reserved2[%d]:               %d\n", i, sbp->reserved2[i]);
		fprintf(stderr, "dbg5     validityFlag:                %d\n", sbp->validityFlag);
		fprintf(stderr, "dbg5     reserved3:                   %d\n", sbp->reserved3);
		fprintf(stderr, "dbg5     dataFormat:                  %d\n", sbp->dataFormat);
		fprintf(stderr, "dbg5     NMEAantennaeR:               %d\n", sbp->NMEAantennaeR);
		fprintf(stderr, "dbg5     NMEAantennaeO:               %d\n", sbp->NMEAantennaeO);
		for (int i = 0; i < 2; i++)
			fprintf(stderr, "dbg5     reserved4[%d]:               %d\n", i, sbp->reserved4[i]);
		fprintf(stderr, "dbg5     kmOfPipe:                    %f\n", sbp->kmOfPipe);
		for (int i = 0; i < 16; i++)
			fprintf(stderr, "dbg5     reserved5[%d]:               %d\n", i, sbp->reserved5[i]);
		fprintf(stderr, "dbg5     coordX:                      %d\n", sbp->coordX);
		fprintf(stderr, "dbg5     coordY:                      %d\n", sbp->coordY);
		fprintf(stderr, "dbg5     coordUnits:                  %d\n", sbp->coordUnits);
		fprintf(stderr, "dbg5     annotation:                  %s\n", sbp->annotation);
		fprintf(stderr, "dbg5     samples:                     %d\n", sbp->samples);
		fprintf(stderr, "dbg5     sampleInterval:              %d\n", sbp->sampleInterval);
		fprintf(stderr, "dbg5     ADCGain:                     %d\n", sbp->ADCGain);
		fprintf(stderr, "dbg5     pulsePower:                  %d\n", sbp->pulsePower);
		fprintf(stderr, "dbg5     reserved6:                   %d\n", sbp->reserved6);
		fprintf(stderr, "dbg5     startFreq:                   %d\n", sbp->startFreq);
		fprintf(stderr, "dbg5     endFreq:                     %d\n", sbp->endFreq);
		fprintf(stderr, "dbg5     sweepLength:                 %d\n", sbp->sweepLength);
		fprintf(stderr, "dbg5     pressure:                    %d\n", sbp->pressure);
		fprintf(stderr, "dbg5     sonarDepth:                  %d\n", sbp->sonarDepth);
		fprintf(stderr, "dbg5     sampleFreq:                  %d\n", sbp->sampleFreq);
		fprintf(stderr, "dbg5     pulseID:                     %d\n", sbp->pulseID);
		fprintf(stderr, "dbg5     sonarAltitude:               %d\n", sbp->sonarAltitude);
		fprintf(stderr, "dbg5     soundspeed:                  %f\n", sbp->soundspeed);
		fprintf(stderr, "dbg5     mixerFrequency:              %f\n", sbp->mixerFrequency);
		fprintf(stderr, "dbg5     year:                        %d\n", sbp->year);
		fprintf(stderr, "dbg5     day:                         %d\n", sbp->day);
		fprintf(stderr, "dbg5     hour:                        %d\n", sbp->hour);
		fprintf(stderr, "dbg5     minute:                      %d\n", sbp->minute);
		fprintf(stderr, "dbg5     second:                      %d\n", sbp->second);
		fprintf(stderr, "dbg5     timeBasis:                   %d\n", sbp->timeBasis);
		fprintf(stderr, "dbg5     weightingFactor:             %d\n", sbp->weightingFactor);
		fprintf(stderr, "dbg5     numberPulses:                %d\n", sbp->numberPulses);
		fprintf(stderr, "dbg5     heading:                     %d\n", sbp->heading);
		fprintf(stderr, "dbg5     pitch:                       %d\n", sbp->pitch);
		fprintf(stderr, "dbg5     roll:                        %d\n", sbp->roll);
		fprintf(stderr, "dbg5     reserved8:                   %d\n", sbp->reserved8);
		fprintf(stderr, "dbg5     reserved9:                   %d\n", sbp->reserved9);
		fprintf(stderr, "dbg5     triggerSource:               %d\n", sbp->triggerSource);
		fprintf(stderr, "dbg5     markNumber:                  %d\n", sbp->markNumber);
		fprintf(stderr, "dbg5     NMEAHour:                    %d\n", sbp->NMEAHour);
		fprintf(stderr, "dbg5     NMEAMinutes:                 %d\n", sbp->NMEAMinutes);
		fprintf(stderr, "dbg5     NMEASeconds:                 %d\n", sbp->NMEASeconds);
		fprintf(stderr, "dbg5     NMEACourse:                  %d\n", sbp->NMEACourse);
		fprintf(stderr, "dbg5     NMEASpeed:                   %d\n", sbp->NMEASpeed);
		fprintf(stderr, "dbg5     NMEADay:                     %d\n", sbp->NMEADay);
		fprintf(stderr, "dbg5     NMEAYear:                    %d\n", sbp->NMEAYear);
		fprintf(stderr, "dbg5     millisecondsToday:           %d\n", sbp->millisecondsToday);
		fprintf(stderr, "dbg5     ADCMax:                      %d\n", sbp->ADCMax);
		fprintf(stderr, "dbg5     reserved10:                  %d\n", sbp->reserved10);
		fprintf(stderr, "dbg5     reserved10:                  %d\n", sbp->reserved11);
		fprintf(stderr, "dbg5     softwareVersion:             %s\n", sbp->softwareVersion);
		fprintf(stderr, "dbg5     sphericalCorrection:         %d\n", sbp->sphericalCorrection);
		fprintf(stderr, "dbg5     packetNum:                   %d\n", sbp->packetNum);
		fprintf(stderr, "dbg5     ADCDecimation:               %d\n", sbp->ADCDecimation);
		fprintf(stderr, "dbg5     reserved12:                  %d\n", sbp->reserved12);
		fprintf(stderr, "dbg5     temperature:                 %d\n", sbp->temperature);
		fprintf(stderr, "dbg5     layback:                     %f\n", sbp->layback);
		fprintf(stderr, "dbg5     reserved13:                  %d\n", sbp->reserved13);
		fprintf(stderr, "dbg5     cableOut:                    %d\n", sbp->cableOut);
		fprintf(stderr, "dbg5     reserved14:                  %d\n", sbp->reserved14);
		if (sbp->dataFormat == 1) {
			for (int i = 0; i < sbp->samples; i++)
				fprintf(stderr, "dbg5     Channel[%d]: %10d %10d\n", i, sbp->trace[2 * i], sbp->trace[2 * i + 1]);
		}
		else {
			for (int i = 0; i < sbp->samples; i++)
				fprintf(stderr, "dbg5     Channel[%d]: %10d\n", i, sbp->trace[i]);
		}
	}
	else if (status == MB_SUCCESS && verbose >= 5 && (store->kind == MB_DATA_DATA || store->kind == MB_DATA_SIDESCAN2)) {
		fprintf(stderr, "\ndbg5  New sidescan data record read by MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5  Subsystem ID:\n");
		fprintf(stderr, "dbg5       subsystem:        %d ", store->subsystem);
		if (store->subsystem == MBSYS_JSTAR_SUBSYSTEM_SSLOW)
			fprintf(stderr, "(75 or 120 kHz sidescan)\n");
		else if (store->subsystem == MBSYS_JSTAR_SUBSYSTEM_SSHIGH)
			fprintf(stderr, "(410 kHz sidescan)\n");

		ss = (struct mbsys_jstar_channel_struct *)&(store->ssport);
		fprintf(stderr, "\ndbg5  Channel 0 (Port):\n");
		fprintf(stderr, "dbg5     start_marker:                %d\n", ss->message.start_marker);
		fprintf(stderr, "dbg5     version:                     %d\n", ss->message.version);
		fprintf(stderr, "dbg5     session:                     %d\n", ss->message.session);
		fprintf(stderr, "dbg5     type:                        %d\n", ss->message.type);
		fprintf(stderr, "dbg5     command:                     %d\n", ss->message.command);
		fprintf(stderr, "dbg5     subsystem:                   %d\n", ss->message.subsystem);
		fprintf(stderr, "dbg5     channel:                     %d\n", ss->message.channel);
		fprintf(stderr, "dbg5     sequence:                    %d\n", ss->message.sequence);
		fprintf(stderr, "dbg5     reserved:                    %d\n", ss->message.reserved);
		fprintf(stderr, "dbg5     size:                        %d\n", ss->message.size);

		fprintf(stderr, "dbg5     pingTime:                    %d\n", ss->pingTime);
		fprintf(stderr, "dbg5     startDepth:                  %d\n", ss->startDepth);
		fprintf(stderr, "dbg5     pingNum:                     %d\n", ss->pingNum);
		for (int i = 0; i < 2; i++)
            fprintf(stderr, "dbg5     reserved1[%d]:               %d\n", i, ss->reserved1[i]);
		fprintf(stderr, "dbg5     msb:                         %d\n", ss->msb);
		fprintf(stderr, "dbg5     lsb1:                        %d\n", ss->lsb1);
		fprintf(stderr, "dbg5     lsb2:                        %d\n", ss->lsb2);
		for (int i = 0; i < 3; i++)
            fprintf(stderr, "dbg5     reserved2[%d]:               %d\n", i, ss->reserved2[i]);
		fprintf(stderr, "dbg5     validityFlag:                %d\n", ss->validityFlag);
		fprintf(stderr, "dbg5     reserved3:                   %d\n", ss->reserved3);
		fprintf(stderr, "dbg5     dataFormat:                  %d\n", ss->dataFormat);
		fprintf(stderr, "dbg5     NMEAantennaeR:               %d\n", ss->NMEAantennaeR);
		fprintf(stderr, "dbg5     NMEAantennaeO:               %d\n", ss->NMEAantennaeO);
		for (int i = 0; i < 2; i++)
			fprintf(stderr, "dbg5     reserved4[%d]:               %d\n", i, ss->reserved4[i]);
		fprintf(stderr, "dbg5     kmOfPipe:                    %f\n", ss->kmOfPipe);
		for (int i = 0; i < 16; i++)
			fprintf(stderr, "dbg5     reserved5[%d]:               %d\n", i, ss->reserved5[i]);
		fprintf(stderr, "dbg5     coordX:                      %d\n", ss->coordX);
		fprintf(stderr, "dbg5     coordY:                      %d\n", ss->coordY);
		fprintf(stderr, "dbg5     coordUnits:                  %d\n", ss->coordUnits);
		fprintf(stderr, "dbg5     annotation:                  %s\n", ss->annotation);
		fprintf(stderr, "dbg5     samples:                     %d\n", ss->samples);
		fprintf(stderr, "dbg5     sampleInterval:              %d\n", ss->sampleInterval);
		fprintf(stderr, "dbg5     ADCGain:                     %d\n", ss->ADCGain);
		fprintf(stderr, "dbg5     pulsePower:                  %d\n", ss->pulsePower);
		fprintf(stderr, "dbg5     reserved6:                   %d\n", ss->reserved6);
		fprintf(stderr, "dbg5     startFreq:                   %d\n", ss->startFreq);
		fprintf(stderr, "dbg5     endFreq:                     %d\n", ss->endFreq);
		fprintf(stderr, "dbg5     sweepLength:                 %d\n", ss->sweepLength);
		fprintf(stderr, "dbg5     pressure:                    %d\n", ss->pressure);
		fprintf(stderr, "dbg5     sonarDepth:                  %d\n", ss->sonarDepth);
		fprintf(stderr, "dbg5     sampleFreq:                  %d\n", ss->sampleFreq);
		fprintf(stderr, "dbg5     pulseID:                     %d\n", ss->pulseID);
		fprintf(stderr, "dbg5     sonarAltitude:               %d\n", ss->sonarAltitude);
		fprintf(stderr, "dbg5     soundspeed:                  %f\n", ss->soundspeed);
		fprintf(stderr, "dbg5     mixerFrequency:              %f\n", ss->mixerFrequency);
		fprintf(stderr, "dbg5     year:                        %d\n", ss->year);
		fprintf(stderr, "dbg5     day:                         %d\n", ss->day);
		fprintf(stderr, "dbg5     hour:                        %d\n", ss->hour);
		fprintf(stderr, "dbg5     minute:                      %d\n", ss->minute);
		fprintf(stderr, "dbg5     second:                      %d\n", ss->second);
		fprintf(stderr, "dbg5     timeBasis:                   %d\n", ss->timeBasis);
		fprintf(stderr, "dbg5     weightingFactor:             %d\n", ss->weightingFactor);
		fprintf(stderr, "dbg5     numberPulses:                %d\n", ss->numberPulses);
		fprintf(stderr, "dbg5     heading:                     %d\n", ss->heading);
		fprintf(stderr, "dbg5     pitch:                       %d\n", ss->pitch);
		fprintf(stderr, "dbg5     roll:                        %d\n", ss->roll);
		fprintf(stderr, "dbg5     reserved8:                   %d\n", ss->reserved8);
		fprintf(stderr, "dbg5     reserved9:                   %d\n", ss->reserved9);
		fprintf(stderr, "dbg5     triggerSource:               %d\n", ss->triggerSource);
		fprintf(stderr, "dbg5     markNumber:                  %d\n", ss->markNumber);
		fprintf(stderr, "dbg5     NMEAHour:                    %d\n", ss->NMEAHour);
		fprintf(stderr, "dbg5     NMEAMinutes:                 %d\n", ss->NMEAMinutes);
		fprintf(stderr, "dbg5     NMEASeconds:                 %d\n", ss->NMEASeconds);
		fprintf(stderr, "dbg5     NMEACourse:                  %d\n", ss->NMEACourse);
		fprintf(stderr, "dbg5     NMEASpeed:                   %d\n", ss->NMEASpeed);
		fprintf(stderr, "dbg5     NMEADay:                     %d\n", ss->NMEADay);
		fprintf(stderr, "dbg5     NMEAYear:                    %d\n", ss->NMEAYear);
		fprintf(stderr, "dbg5     millisecondsToday:           %d\n", ss->millisecondsToday);
		fprintf(stderr, "dbg5     ADCMax:                      %d\n", ss->ADCMax);
		fprintf(stderr, "dbg5     reserved10:                  %d\n", ss->reserved10);
		fprintf(stderr, "dbg5     reserved10:                  %d\n", ss->reserved11);
		fprintf(stderr, "dbg5     softwareVersion:             %s\n", ss->softwareVersion);
		fprintf(stderr, "dbg5     sphericalCorrection:         %d\n", ss->sphericalCorrection);
		fprintf(stderr, "dbg5     packetNum:                   %d\n", ss->packetNum);
		fprintf(stderr, "dbg5     ADCDecimation:               %d\n", ss->ADCDecimation);
		fprintf(stderr, "dbg5     reserved12:                  %d\n", ss->reserved12);
		fprintf(stderr, "dbg5     temperature:                 %d\n", ss->temperature);
		fprintf(stderr, "dbg5     layback:                     %f\n", ss->layback);
		fprintf(stderr, "dbg5     reserved13:                  %d\n", ss->reserved13);
		fprintf(stderr, "dbg5     cableOut:                    %d\n", ss->cableOut);
		fprintf(stderr, "dbg5     reserved14:                  %d\n", ss->reserved14);
		if (ss->dataFormat == 1) {
			for (int i = 0; i < ss->samples; i++)
				fprintf(stderr, "dbg5     Channel 0[%d]: %10d %10d\n", i, ss->trace[2 * i], ss->trace[2 * i + 1]);
		}
		else {
			for (int i = 0; i < ss->samples; i++)
				fprintf(stderr, "dbg5     Channel 0[%d]: %10d\n", i, ss->trace[i]);
		}

		ss = (struct mbsys_jstar_channel_struct *)&(store->ssstbd);
		fprintf(stderr, "\ndbg5  Channel 1 (Starboard):\n");
		fprintf(stderr, "dbg5     start_marker:                %d\n", ss->message.start_marker);
		fprintf(stderr, "dbg5     version:                     %d\n", ss->message.version);
		fprintf(stderr, "dbg5     session:                     %d\n", ss->message.session);
		fprintf(stderr, "dbg5     type:                        %d\n", ss->message.type);
		fprintf(stderr, "dbg5     command:                     %d\n", ss->message.command);
		fprintf(stderr, "dbg5     subsystem:                   %d\n", ss->message.subsystem);
		fprintf(stderr, "dbg5     channel:                     %d\n", ss->message.channel);
		fprintf(stderr, "dbg5     sequence:                    %d\n", ss->message.sequence);
		fprintf(stderr, "dbg5     reserved:                    %d\n", ss->message.reserved);
		fprintf(stderr, "dbg5     size:                        %d\n", ss->message.size);

		fprintf(stderr, "dbg5     pingTime:                    %d\n", ss->pingTime);
		fprintf(stderr, "dbg5     startDepth:                  %d\n", ss->startDepth);
		fprintf(stderr, "dbg5     pingNum:                     %d\n", ss->pingNum);
		for (int i = 0; i < 2; i++)
            fprintf(stderr, "dbg5     reserved1[%d]:               %d\n", i, ss->reserved1[i]);
		fprintf(stderr, "dbg5     msb:                         %d\n", ss->msb);
		fprintf(stderr, "dbg5     lsb1:                        %d\n", ss->lsb1);
		fprintf(stderr, "dbg5     lsb2:                        %d\n", ss->lsb2);
		for (int i = 0; i < 3; i++)
            fprintf(stderr, "dbg5     reserved2[%d]:               %d\n", i, ss->reserved2[i]);
		fprintf(stderr, "dbg5     validityFlag:                %d\n", ss->validityFlag);
		fprintf(stderr, "dbg5     reserved3:                   %d\n", ss->reserved3);
		fprintf(stderr, "dbg5     dataFormat:                  %d\n", ss->dataFormat);
		fprintf(stderr, "dbg5     NMEAantennaeR:               %d\n", ss->NMEAantennaeR);
		fprintf(stderr, "dbg5     NMEAantennaeO:               %d\n", ss->NMEAantennaeO);
		for (int i = 0; i < 2; i++)
			fprintf(stderr, "dbg5     reserved4[%d]:               %d\n", i, ss->reserved4[i]);
		fprintf(stderr, "dbg5     kmOfPipe:                    %f\n", ss->kmOfPipe);
		for (int i = 0; i < 16; i++)
			fprintf(stderr, "dbg5     reserved5[%d]:               %d\n", i, ss->reserved5[i]);
		fprintf(stderr, "dbg5     coordX:                      %d\n", ss->coordX);
		fprintf(stderr, "dbg5     coordY:                      %d\n", ss->coordY);
		fprintf(stderr, "dbg5     coordUnits:                  %d\n", ss->coordUnits);
		fprintf(stderr, "dbg5     annotation:                  %s\n", ss->annotation);
		fprintf(stderr, "dbg5     samples:                     %d\n", ss->samples);
		fprintf(stderr, "dbg5     sampleInterval:              %d\n", ss->sampleInterval);
		fprintf(stderr, "dbg5     ADCGain:                     %d\n", ss->ADCGain);
		fprintf(stderr, "dbg5     pulsePower:                  %d\n", ss->pulsePower);
		fprintf(stderr, "dbg5     reserved6:                   %d\n", ss->reserved6);
		fprintf(stderr, "dbg5     startFreq:                   %d\n", ss->startFreq);
		fprintf(stderr, "dbg5     endFreq:                     %d\n", ss->endFreq);
		fprintf(stderr, "dbg5     sweepLength:                 %d\n", ss->sweepLength);
		fprintf(stderr, "dbg5     pressure:                    %d\n", ss->pressure);
		fprintf(stderr, "dbg5     sonarDepth:                  %d\n", ss->sonarDepth);
		fprintf(stderr, "dbg5     sampleFreq:                  %d\n", ss->sampleFreq);
		fprintf(stderr, "dbg5     pulseID:                     %d\n", ss->pulseID);
		fprintf(stderr, "dbg5     sonarAltitude:               %d\n", ss->sonarAltitude);
		fprintf(stderr, "dbg5     soundspeed:                  %f\n", ss->soundspeed);
		fprintf(stderr, "dbg5     mixerFrequency:              %f\n", ss->mixerFrequency);
		fprintf(stderr, "dbg5     year:                        %d\n", ss->year);
		fprintf(stderr, "dbg5     day:                         %d\n", ss->day);
		fprintf(stderr, "dbg5     hour:                        %d\n", ss->hour);
		fprintf(stderr, "dbg5     minute:                      %d\n", ss->minute);
		fprintf(stderr, "dbg5     second:                      %d\n", ss->second);
		fprintf(stderr, "dbg5     timeBasis:                   %d\n", ss->timeBasis);
		fprintf(stderr, "dbg5     weightingFactor:             %d\n", ss->weightingFactor);
		fprintf(stderr, "dbg5     numberPulses:                %d\n", ss->numberPulses);
		fprintf(stderr, "dbg5     heading:                     %d\n", ss->heading);
		fprintf(stderr, "dbg5     pitch:                       %d\n", ss->pitch);
		fprintf(stderr, "dbg5     roll:                        %d\n", ss->roll);
		fprintf(stderr, "dbg5     reserved8:                   %d\n", ss->reserved8);
		fprintf(stderr, "dbg5     reserved9:                   %d\n", ss->reserved9);
		fprintf(stderr, "dbg5     triggerSource:               %d\n", ss->triggerSource);
		fprintf(stderr, "dbg5     markNumber:                  %d\n", ss->markNumber);
		fprintf(stderr, "dbg5     NMEAHour:                    %d\n", ss->NMEAHour);
		fprintf(stderr, "dbg5     NMEAMinutes:                 %d\n", ss->NMEAMinutes);
		fprintf(stderr, "dbg5     NMEASeconds:                 %d\n", ss->NMEASeconds);
		fprintf(stderr, "dbg5     NMEACourse:                  %d\n", ss->NMEACourse);
		fprintf(stderr, "dbg5     NMEASpeed:                   %d\n", ss->NMEASpeed);
		fprintf(stderr, "dbg5     NMEADay:                     %d\n", ss->NMEADay);
		fprintf(stderr, "dbg5     NMEAYear:                    %d\n", ss->NMEAYear);
		fprintf(stderr, "dbg5     millisecondsToday:           %d\n", ss->millisecondsToday);
		fprintf(stderr, "dbg5     ADCMax:                      %d\n", ss->ADCMax);
		fprintf(stderr, "dbg5     reserved10:                  %d\n", ss->reserved10);
		fprintf(stderr, "dbg5     reserved10:                  %d\n", ss->reserved11);
		fprintf(stderr, "dbg5     softwareVersion:             %s\n", ss->softwareVersion);
		fprintf(stderr, "dbg5     sphericalCorrection:         %d\n", ss->sphericalCorrection);
		fprintf(stderr, "dbg5     packetNum:                   %d\n", ss->packetNum);
		fprintf(stderr, "dbg5     ADCDecimation:               %d\n", ss->ADCDecimation);
		fprintf(stderr, "dbg5     reserved12:                  %d\n", ss->reserved12);
		fprintf(stderr, "dbg5     temperature:                 %d\n", ss->temperature);
		fprintf(stderr, "dbg5     layback:                     %f\n", ss->layback);
		fprintf(stderr, "dbg5     reserved13:                  %d\n", ss->reserved13);
		fprintf(stderr, "dbg5     cableOut:                    %d\n", ss->cableOut);
		fprintf(stderr, "dbg5     reserved14:                  %d\n", ss->reserved14);
		if (ss->dataFormat == 1) {
			for (int i = 0; i < ss->samples; i++)
				fprintf(stderr, "dbg5     Channel 1[%d]: %10d %10d\n", i, ss->trace[2 * i], ss->trace[2 * i + 1]);
		}
		else {
			for (int i = 0; i < ss->samples; i++)
				fprintf(stderr, "dbg5     Channel 1[%d]: %10d\n", i, ss->trace[i]);
		}
	}
	else if (status == MB_SUCCESS && verbose >= 5 && store->kind == MB_DATA_ATTITUDE) {
		fprintf(stderr, "\ndbg5  New roll pitch data record read by MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5  Subsystem ID:\n");
		fprintf(stderr, "dbg5       subsystem:        %d ", store->subsystem);
		if (store->subsystem == MBSYS_JSTAR_SUBSYSTEM_SSLOW)
			fprintf(stderr, "(75 or 120 kHz sidescan)\n");
		else if (store->subsystem == MBSYS_JSTAR_SUBSYSTEM_SSHIGH)
			fprintf(stderr, "(410 kHz sidescan)\n");

		pitchroll = (struct mbsys_jstar_pitchroll_struct *)&(store->pitchroll);
		fprintf(stderr, "\ndbg5  Roll and Pitch:\n");
		fprintf(stderr, "dbg5     start_marker:                %d\n", pitchroll->message.start_marker);
		fprintf(stderr, "dbg5     version:                     %d\n", pitchroll->message.version);
		fprintf(stderr, "dbg5     session:                     %d\n", pitchroll->message.session);
		fprintf(stderr, "dbg5     type:                        %d\n", pitchroll->message.type);
		fprintf(stderr, "dbg5     command:                     %d\n", pitchroll->message.command);
		fprintf(stderr, "dbg5     subsystem:                   %d\n", pitchroll->message.subsystem);
		fprintf(stderr, "dbg5     channel:                     %d\n", pitchroll->message.channel);
		fprintf(stderr, "dbg5     sequence:                    %d\n", pitchroll->message.sequence);
		fprintf(stderr, "dbg5     reserved:                    %d\n", pitchroll->message.reserved);
		fprintf(stderr, "dbg5     size:                        %d\n", pitchroll->message.size);

		fprintf(stderr, "dbg5     seconds:                     %d\n", pitchroll->seconds);
		fprintf(stderr, "dbg5     milliseconds:                %d\n", pitchroll->milliseconds);
		fprintf(stderr, "dbg5     reserve1[0]:                 %d\n", pitchroll->reserve1[0]);
		fprintf(stderr, "dbg5     reserve1[1]:                 %d\n", pitchroll->reserve1[1]);
		fprintf(stderr, "dbg5     reserve1[2]:                 %d\n", pitchroll->reserve1[2]);
		fprintf(stderr, "dbg5     reserve1[3]:                 %d\n", pitchroll->reserve1[3]);
		fprintf(stderr, "dbg5     accelerationx:               %d\n", pitchroll->accelerationx);
		fprintf(stderr, "dbg5     accelerationy:               %d\n", pitchroll->accelerationy);
		fprintf(stderr, "dbg5     accelerationz:               %d\n", pitchroll->accelerationz);
		fprintf(stderr, "dbg5     gyroratex:                   %d\n", pitchroll->gyroratex);
		fprintf(stderr, "dbg5     gyroratey:                   %d\n", pitchroll->gyroratey);
		fprintf(stderr, "dbg5     gyroratez:                   %d\n", pitchroll->gyroratez);
		fprintf(stderr, "dbg5     pitch:                       %d\n", pitchroll->pitch);
		fprintf(stderr, "dbg5     roll:                        %d\n", pitchroll->roll);
		fprintf(stderr, "dbg5     temperature:                 %d\n", pitchroll->temperature);
		fprintf(stderr, "dbg5     deviceinfo:                  %d\n", pitchroll->deviceinfo);
		fprintf(stderr, "dbg5     heave:                       %d\n", pitchroll->heave);
		fprintf(stderr, "dbg5     heading:                     %d\n", pitchroll->heading);
		fprintf(stderr, "dbg5     datavalidflags:              %d\n", pitchroll->datavalidflags);
		fprintf(stderr, "dbg5     reserve2:                    %d\n", pitchroll->reserve2);
	}
	else if (status == MB_SUCCESS && verbose >= 5 && store->kind == MB_DATA_DVL) {
		fprintf(stderr, "\ndbg5  New dvl data record read by MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5  Subsystem ID:\n");
		fprintf(stderr, "dbg5       subsystem:        %d ", store->subsystem);
		if (store->subsystem == MBSYS_JSTAR_SUBSYSTEM_SSLOW)
			fprintf(stderr, "(75 or 120 kHz sidescan)\n");
		else if (store->subsystem == MBSYS_JSTAR_SUBSYSTEM_SSHIGH)
			fprintf(stderr, "(410 kHz sidescan)\n");

		dvl = (struct mbsys_jstar_dvl_struct *)&(store->dvl);
		fprintf(stderr, "\ndbg5  Roll and Pitch:\n");
		fprintf(stderr, "dbg5     start_marker:                %d\n", dvl->message.start_marker);
		fprintf(stderr, "dbg5     version:                     %d\n", dvl->message.version);
		fprintf(stderr, "dbg5     session:                     %d\n", dvl->message.session);
		fprintf(stderr, "dbg5     type:                        %d\n", dvl->message.type);
		fprintf(stderr, "dbg5     command:                     %d\n", dvl->message.command);
		fprintf(stderr, "dbg5     subsystem:                   %d\n", dvl->message.subsystem);
		fprintf(stderr, "dbg5     channel:                     %d\n", dvl->message.channel);
		fprintf(stderr, "dbg5     sequence:                    %d\n", dvl->message.sequence);
		fprintf(stderr, "dbg5     reserved:                    %d\n", dvl->message.reserved);
		fprintf(stderr, "dbg5     size:                        %d\n", dvl->message.size);

		fprintf(stderr, "dbg5     seconds:                     %d\n", dvl->seconds);
		fprintf(stderr, "dbg5     milliseconds:                %d\n", dvl->milliseconds);
		fprintf(stderr, "dbg5     reserve1[0]:                 %d\n", dvl->reserve1[0]);
		fprintf(stderr, "dbg5     reserve1[1]:                 %d\n", dvl->reserve1[1]);
		fprintf(stderr, "dbg5     reserve1[2]:                 %d\n", dvl->reserve1[2]);
		fprintf(stderr, "dbg5     reserve1[3]:                 %d\n", dvl->reserve1[3]);
		fprintf(stderr, "dbg5     datavalidflags:              %d\n", dvl->datavalidflags);
		fprintf(stderr, "dbg5     beam1range:                  %d\n", dvl->beam1range);
		fprintf(stderr, "dbg5     beam2range:                  %d\n", dvl->beam2range);
		fprintf(stderr, "dbg5     beam3range:                  %d\n", dvl->beam3range);
		fprintf(stderr, "dbg5     beam4range:                  %d\n", dvl->beam4range);
		fprintf(stderr, "dbg5     velocitybottomx:             %d\n", dvl->velocitybottomx);
		fprintf(stderr, "dbg5     velocitybottomy:             %d\n", dvl->velocitybottomy);
		fprintf(stderr, "dbg5     velocitybottomz:             %d\n", dvl->velocitybottomz);
		fprintf(stderr, "dbg5     velocitywaterx:              %d\n", dvl->velocitywaterx);
		fprintf(stderr, "dbg5     velocitywatery:              %d\n", dvl->velocitywatery);
		fprintf(stderr, "dbg5     velocitywaterz:              %d\n", dvl->velocitywaterz);
		fprintf(stderr, "dbg5     depth:                       %d\n", dvl->depth);
		fprintf(stderr, "dbg5     pitch:                       %d\n", dvl->pitch);
		fprintf(stderr, "dbg5     roll:                        %d\n", dvl->roll);
		fprintf(stderr, "dbg5     heading:                     %d\n", dvl->heading);
		fprintf(stderr, "dbg5     salinity:                    %d\n", dvl->salinity);
		fprintf(stderr, "dbg5     temperature:                 %d\n", dvl->temperature);
		fprintf(stderr, "dbg5     soundspeed:                  %d\n", dvl->soundspeed);
		for (int i = 0; i < 7; i++)
			fprintf(stderr, "dbg5     reserve2[%d]:                %d\n", i, dvl->reserve2[i]);
	}
	else if (status == MB_SUCCESS && verbose >= 5 && store->kind == MB_DATA_CTD) {
		pressure = (struct mbsys_jstar_pressure_struct *)&(store->pressure);
		fprintf(stderr, "\ndbg5  New pressure data record read by MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5     start_marker:                %d\n", pressure->message.start_marker);
		fprintf(stderr, "dbg5     version:                     %d\n", pressure->message.version);
		fprintf(stderr, "dbg5     session:                     %d\n", pressure->message.session);
		fprintf(stderr, "dbg5     type:                        %d\n", pressure->message.type);
		fprintf(stderr, "dbg5     command:                     %d\n", pressure->message.command);
		fprintf(stderr, "dbg5     subsystem:                   %d\n", pressure->message.subsystem);
		fprintf(stderr, "dbg5     channel:                     %d\n", pressure->message.channel);
		fprintf(stderr, "dbg5     sequence:                    %d\n", pressure->message.sequence);
		fprintf(stderr, "dbg5     reserved:                    %d\n", pressure->message.reserved);
		fprintf(stderr, "dbg5     size:                        %d\n", pressure->message.size);

		fprintf(stderr, "dbg5     seconds:                     %d\n", pressure->seconds);
		fprintf(stderr, "dbg5     milliseconds:                %d\n", pressure->milliseconds);
		fprintf(stderr, "dbg5     reserve1[0]:                 %d\n", pressure->reserve1[0]);
		fprintf(stderr, "dbg5     reserve1[1]:                 %d\n", pressure->reserve1[1]);
		fprintf(stderr, "dbg5     reserve1[2]:                 %d\n", pressure->reserve1[2]);
		fprintf(stderr, "dbg5     reserve1[3]:                 %d\n", pressure->reserve1[3]);
		fprintf(stderr, "dbg5     pressure:                    %d\n", pressure->pressure);
		fprintf(stderr, "dbg5     salinity:                    %d\n", pressure->salinity);
		fprintf(stderr, "dbg5     datavalidflags:              %d\n", pressure->datavalidflags);
		fprintf(stderr, "dbg5     conductivity:                %d\n", pressure->conductivity);
		fprintf(stderr, "dbg5     soundspeed:                  %d\n", pressure->soundspeed);
		for (int i = 0; i < 10; i++)
			fprintf(stderr, "dbg5     reserve2[%2d]:                 %d\n", i, pressure->reserve2[i]);
	}
	else if (status == MB_SUCCESS && verbose >= 5 &&
	         (store->kind == MB_DATA_NMEA_RMC || store->kind == MB_DATA_NMEA_DBT || store->kind == MB_DATA_NMEA_DPT)) {
		nmea = (struct mbsys_jstar_nmea_struct *)&(store->nmea);
		fprintf(stderr, "\ndbg5  New NMEA data record read by MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5     start_marker:                %d\n", nmea->message.start_marker);
		fprintf(stderr, "dbg5     version:                     %d\n", nmea->message.version);
		fprintf(stderr, "dbg5     session:                     %d\n", nmea->message.session);
		fprintf(stderr, "dbg5     type:                        %d\n", nmea->message.type);
		fprintf(stderr, "dbg5     command:                     %d\n", nmea->message.command);
		fprintf(stderr, "dbg5     subsystem:                   %d\n", nmea->message.subsystem);
		fprintf(stderr, "dbg5     channel:                     %d\n", nmea->message.channel);
		fprintf(stderr, "dbg5     sequence:                    %d\n", nmea->message.sequence);
		fprintf(stderr, "dbg5     reserved:                    %d\n", nmea->message.reserved);
		fprintf(stderr, "dbg5     size:                        %d\n", nmea->message.size);

		fprintf(stderr, "dbg5     seconds:                     %d\n", nmea->seconds);
		fprintf(stderr, "dbg5     milliseconds:                %d\n", nmea->milliseconds);
		fprintf(stderr, "dbg5     source:                      %d\n", nmea->source);
		fprintf(stderr, "dbg5     reserve[0]:                  %d\n", nmea->reserve[0]);
		fprintf(stderr, "dbg5     reserve[1]:                  %d\n", nmea->reserve[1]);
		fprintf(stderr, "dbg5     reserve[2]:                  %d\n", nmea->reserve[2]);
		fprintf(stderr, "dbg5     nmea:                        %s\n", nmea->nmea);
	}
	else if (status == MB_SUCCESS && verbose >= 5 && (store->kind == MB_DATA_HEADER)) {
		sysinfo = (struct mbsys_jstar_sysinfo_struct *)&(store->sysinfo);
		fprintf(stderr, "\ndbg5  New sysinfo data record read by MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5     start_marker:                %d\n", sysinfo->message.start_marker);
		fprintf(stderr, "dbg5     version:                     %d\n", sysinfo->message.version);
		fprintf(stderr, "dbg5     session:                     %d\n", sysinfo->message.session);
		fprintf(stderr, "dbg5     type:                        %d\n", sysinfo->message.type);
		fprintf(stderr, "dbg5     command:                     %d\n", sysinfo->message.command);
		fprintf(stderr, "dbg5     subsystem:                   %d\n", sysinfo->message.subsystem);
		fprintf(stderr, "dbg5     channel:                     %d\n", sysinfo->message.channel);
		fprintf(stderr, "dbg5     sequence:                    %d\n", sysinfo->message.sequence);
		fprintf(stderr, "dbg5     reserved:                    %d\n", sysinfo->message.reserved);
		fprintf(stderr, "dbg5     size:                        %d\n", sysinfo->message.size);

		fprintf(stderr, "dbg5     system_type:                 %d\n", sysinfo->system_type);
		fprintf(stderr, "dbg5     reserved1:                   %d\n", sysinfo->reserved1);
		fprintf(stderr, "dbg5     version:                     %d\n", sysinfo->version);
		fprintf(stderr, "dbg5     reserved2:                   %d\n", sysinfo->reserved2);
		fprintf(stderr, "dbg5     platformserialnumber:        %d\n", sysinfo->platformserialnumber);
		fprintf(stderr, "dbg5     sysinfosize:                 %d\n", sysinfo->sysinfosize);
		fprintf(stderr, "dbg5     sysinfo:                     \n%s\n", sysinfo->sysinfo);
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_wt_edgjstar(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	char *function_name = "mbr_wt_edgjstar";
	struct mbsys_jstar_struct *store;
	struct mbsys_jstar_channel_struct *sbp;
	struct mbsys_jstar_channel_struct *ss;
	struct mbsys_jstar_nmea_struct *nmea;
	struct mbsys_jstar_pitchroll_struct *pitchroll;
	struct mbsys_jstar_dvl_struct *dvl;
	struct mbsys_jstar_pressure_struct *pressure;
	struct mbsys_jstar_sysinfo_struct *sysinfo;
	struct mbsys_jstar_comment_struct *comment;
	char buffer[MBSYS_JSTAR_SBPHEADER_SIZE];
	int index;
	int write_len;
	int shortspersample;
	int trace_size;

	/* print input debug statements */
	if (verbose >= 3) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get pointer to raw data structure */
	store = (struct mbsys_jstar_struct *)store_ptr;

	/* print debug statements */
	if (verbose >= 5 && store->kind == MB_DATA_COMMENT) {
		fprintf(stderr, "\ndbg5  New comment to be written by MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5  Subsystem ID:\n");
		fprintf(stderr, "dbg5       subsystem:        %d ", store->subsystem);
		if (store->subsystem == 0)
			fprintf(stderr, "(subbottom)\n");
		else if (store->subsystem == MBSYS_JSTAR_SUBSYSTEM_SSLOW)
			fprintf(stderr, "(75 or 120 kHz sidescan)\n");
		else if (store->subsystem == MBSYS_JSTAR_SUBSYSTEM_SSHIGH)
			fprintf(stderr, "(410 kHz sidescan)\n");

		fprintf(stderr, "\ndbg5  Comment:\n");
		comment = (struct mbsys_jstar_comment_struct *)&(store->comment);
		fprintf(stderr, "dbg5     start_marker:                %d\n", comment->message.start_marker);
		fprintf(stderr, "dbg5     version:                     %d\n", comment->message.version);
		fprintf(stderr, "dbg5     session:                     %d\n", comment->message.session);
		fprintf(stderr, "dbg5     type:                        %d\n", comment->message.type);
		fprintf(stderr, "dbg5     command:                     %d\n", comment->message.command);
		fprintf(stderr, "dbg5     subsystem:                   %d\n", comment->message.subsystem);
		fprintf(stderr, "dbg5     channel:                     %d\n", comment->message.channel);
		fprintf(stderr, "dbg5     sequence:                    %d\n", comment->message.sequence);
		fprintf(stderr, "dbg5     reserved:                    %d\n", comment->message.reserved);
		fprintf(stderr, "dbg5     size:                        %d\n", comment->message.size);

		fprintf(stderr, "dbg5     comment:                     %s\n", store->comment.comment);
	}
	else if (verbose >= 5 && store->kind == MB_DATA_SUBBOTTOM_SUBBOTTOM) {
		sbp = (struct mbsys_jstar_channel_struct *)&(store->sbp);
		fprintf(stderr, "\ndbg5  New subbottom data record to be written by MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5  Subsystem ID:\n");
		fprintf(stderr, "dbg5       subsystem:        %d (subbottom)\n", store->subsystem);
		fprintf(stderr, "\ndbg5  Channel:\n");
		fprintf(stderr, "dbg5     start_marker:                %d\n", sbp->message.start_marker);
		fprintf(stderr, "dbg5     version:                     %d\n", sbp->message.version);
		fprintf(stderr, "dbg5     session:                     %d\n", sbp->message.session);
		fprintf(stderr, "dbg5     type:                        %d\n", sbp->message.type);
		fprintf(stderr, "dbg5     command:                     %d\n", sbp->message.command);
		fprintf(stderr, "dbg5     subsystem:                   %d\n", sbp->message.subsystem);
		fprintf(stderr, "dbg5     channel:                     %d\n", sbp->message.channel);
		fprintf(stderr, "dbg5     sequence:                    %d\n", sbp->message.sequence);
		fprintf(stderr, "dbg5     reserved:                    %d\n", sbp->message.reserved);
		fprintf(stderr, "dbg5     size:                        %d\n", sbp->message.size);

		fprintf(stderr, "dbg5     pingTime:                    %d\n", sbp->pingTime);
		fprintf(stderr, "dbg5     startDepth:                  %d\n", sbp->startDepth);
		fprintf(stderr, "dbg5     pingNum:                     %d\n", sbp->pingNum);
		for (int i = 0; i < 2; i++)
            fprintf(stderr, "dbg5     reserved1[%d]:               %d\n", i, sbp->reserved1[i]);
		fprintf(stderr, "dbg5     msb:                         %d\n", sbp->msb);
		fprintf(stderr, "dbg5     lsb1:                        %d\n", sbp->lsb1);
		fprintf(stderr, "dbg5     lsb2:                        %d\n", sbp->lsb2);
		for (int i = 0; i < 3; i++)
            fprintf(stderr, "dbg5     reserved2[%d]:               %d\n", i, sbp->reserved2[i]);
		fprintf(stderr, "dbg5     validityFlag:                %d\n", sbp->validityFlag);
		fprintf(stderr, "dbg5     reserved3:                   %d\n", sbp->reserved3);
		fprintf(stderr, "dbg5     dataFormat:                  %d\n", sbp->dataFormat);
		fprintf(stderr, "dbg5     NMEAantennaeR:               %d\n", sbp->NMEAantennaeR);
		fprintf(stderr, "dbg5     NMEAantennaeO:               %d\n", sbp->NMEAantennaeO);
		for (int i = 0; i < 2; i++)
			fprintf(stderr, "dbg5     reserved4[%d]:               %d\n", i, sbp->reserved4[i]);
		fprintf(stderr, "dbg5     kmOfPipe:                    %f\n", sbp->kmOfPipe);
		for (int i = 0; i < 16; i++)
			fprintf(stderr, "dbg5     reserved5[%d]:               %d\n", i, sbp->reserved5[i]);
		fprintf(stderr, "dbg5     coordX:                      %d\n", sbp->coordX);
		fprintf(stderr, "dbg5     coordY:                      %d\n", sbp->coordY);
		fprintf(stderr, "dbg5     coordUnits:                  %d\n", sbp->coordUnits);
		fprintf(stderr, "dbg5     annotation:                  %s\n", sbp->annotation);
		fprintf(stderr, "dbg5     samples:                     %d\n", sbp->samples);
		fprintf(stderr, "dbg5     sampleInterval:              %d\n", sbp->sampleInterval);
		fprintf(stderr, "dbg5     ADCGain:                     %d\n", sbp->ADCGain);
		fprintf(stderr, "dbg5     pulsePower:                  %d\n", sbp->pulsePower);
		fprintf(stderr, "dbg5     reserved6:                   %d\n", sbp->reserved6);
		fprintf(stderr, "dbg5     startFreq:                   %d\n", sbp->startFreq);
		fprintf(stderr, "dbg5     endFreq:                     %d\n", sbp->endFreq);
		fprintf(stderr, "dbg5     sweepLength:                 %d\n", sbp->sweepLength);
		fprintf(stderr, "dbg5     pressure:                    %d\n", sbp->pressure);
		fprintf(stderr, "dbg5     sonarDepth:                  %d\n", sbp->sonarDepth);
		fprintf(stderr, "dbg5     sampleFreq:                  %d\n", sbp->sampleFreq);
		fprintf(stderr, "dbg5     pulseID:                     %d\n", sbp->pulseID);
		fprintf(stderr, "dbg5     sonarAltitude:               %d\n", sbp->sonarAltitude);
		fprintf(stderr, "dbg5     soundspeed:                  %f\n", sbp->soundspeed);
		fprintf(stderr, "dbg5     mixerFrequency:              %f\n", sbp->mixerFrequency);
		fprintf(stderr, "dbg5     year:                        %d\n", sbp->year);
		fprintf(stderr, "dbg5     day:                         %d\n", sbp->day);
		fprintf(stderr, "dbg5     hour:                        %d\n", sbp->hour);
		fprintf(stderr, "dbg5     minute:                      %d\n", sbp->minute);
		fprintf(stderr, "dbg5     second:                      %d\n", sbp->second);
		fprintf(stderr, "dbg5     timeBasis:                   %d\n", sbp->timeBasis);
		fprintf(stderr, "dbg5     weightingFactor:             %d\n", sbp->weightingFactor);
		fprintf(stderr, "dbg5     numberPulses:                %d\n", sbp->numberPulses);
		fprintf(stderr, "dbg5     heading:                     %d\n", sbp->heading);
		fprintf(stderr, "dbg5     pitch:                       %d\n", sbp->pitch);
		fprintf(stderr, "dbg5     roll:                        %d\n", sbp->roll);
		fprintf(stderr, "dbg5     reserved8:                   %d\n", sbp->reserved8);
		fprintf(stderr, "dbg5     reserved9:                   %d\n", sbp->reserved9);
		fprintf(stderr, "dbg5     triggerSource:               %d\n", sbp->triggerSource);
		fprintf(stderr, "dbg5     markNumber:                  %d\n", sbp->markNumber);
		fprintf(stderr, "dbg5     NMEAHour:                    %d\n", sbp->NMEAHour);
		fprintf(stderr, "dbg5     NMEAMinutes:                 %d\n", sbp->NMEAMinutes);
		fprintf(stderr, "dbg5     NMEASeconds:                 %d\n", sbp->NMEASeconds);
		fprintf(stderr, "dbg5     NMEACourse:                  %d\n", sbp->NMEACourse);
		fprintf(stderr, "dbg5     NMEASpeed:                   %d\n", sbp->NMEASpeed);
		fprintf(stderr, "dbg5     NMEADay:                     %d\n", sbp->NMEADay);
		fprintf(stderr, "dbg5     NMEAYear:                    %d\n", sbp->NMEAYear);
		fprintf(stderr, "dbg5     millisecondsToday:           %d\n", sbp->millisecondsToday);
		fprintf(stderr, "dbg5     ADCMax:                      %d\n", sbp->ADCMax);
		fprintf(stderr, "dbg5     reserved10:                  %d\n", sbp->reserved10);
		fprintf(stderr, "dbg5     reserved10:                  %d\n", sbp->reserved11);
		fprintf(stderr, "dbg5     softwareVersion:             %s\n", sbp->softwareVersion);
		fprintf(stderr, "dbg5     sphericalCorrection:         %d\n", sbp->sphericalCorrection);
		fprintf(stderr, "dbg5     packetNum:                   %d\n", sbp->packetNum);
		fprintf(stderr, "dbg5     ADCDecimation:               %d\n", sbp->ADCDecimation);
		fprintf(stderr, "dbg5     reserved12:                  %d\n", sbp->reserved12);
		fprintf(stderr, "dbg5     temperature:                 %d\n", sbp->temperature);
		fprintf(stderr, "dbg5     layback:                     %f\n", sbp->layback);
		fprintf(stderr, "dbg5     reserved13:                  %d\n", sbp->reserved13);
		fprintf(stderr, "dbg5     cableOut:                    %d\n", sbp->cableOut);
		fprintf(stderr, "dbg5     reserved14:                  %d\n", sbp->reserved14);
		if (sbp->dataFormat == 1) {
			for (int i = 0; i < sbp->samples; i++)
				fprintf(stderr, "dbg5     Channel[%d]: %10d %10d\n", i, sbp->trace[2 * i], sbp->trace[2 * i + 1]);
		}
		else {
			for (int i = 0; i < sbp->samples; i++)
				fprintf(stderr, "dbg5     Channel[%d]: %10d\n", i, sbp->trace[i]);
		}
	}
	else if (verbose >= 5 && (store->kind == MB_DATA_DATA || store->kind == MB_DATA_SIDESCAN2)) {
		fprintf(stderr, "\ndbg5  New sidescan data record to be written by MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5  Subsystem ID:\n");
		fprintf(stderr, "dbg5       subsystem:        %d ", store->subsystem);
		if (store->subsystem == MBSYS_JSTAR_SUBSYSTEM_SSLOW)
			fprintf(stderr, "(75 or 120 kHz sidescan)\n");
		else if (store->subsystem == MBSYS_JSTAR_SUBSYSTEM_SSHIGH)
			fprintf(stderr, "(410 kHz sidescan)\n");

		ss = (struct mbsys_jstar_channel_struct *)&(store->ssport);
		fprintf(stderr, "\ndbg5  Channel 0 (Port):\n");
		fprintf(stderr, "dbg5     start_marker:                %d\n", ss->message.start_marker);
		fprintf(stderr, "dbg5     version:                     %d\n", ss->message.version);
		fprintf(stderr, "dbg5     session:                     %d\n", ss->message.session);
		fprintf(stderr, "dbg5     type:                        %d\n", ss->message.type);
		fprintf(stderr, "dbg5     command:                     %d\n", ss->message.command);
		fprintf(stderr, "dbg5     subsystem:                   %d\n", ss->message.subsystem);
		fprintf(stderr, "dbg5     channel:                     %d\n", ss->message.channel);
		fprintf(stderr, "dbg5     sequence:                    %d\n", ss->message.sequence);
		fprintf(stderr, "dbg5     reserved:                    %d\n", ss->message.reserved);
		fprintf(stderr, "dbg5     size:                        %d\n", ss->message.size);

		fprintf(stderr, "dbg5     pingTime:                    %d\n", ss->pingTime);
		fprintf(stderr, "dbg5     startDepth:                  %d\n", ss->startDepth);
		fprintf(stderr, "dbg5     pingNum:                     %d\n", ss->pingNum);
		for (int i = 0; i < 2; i++)
            fprintf(stderr, "dbg5     reserved1[%d]:               %d\n", i, ss->reserved1[i]);
		fprintf(stderr, "dbg5     msb:                         %d\n", ss->msb);
		fprintf(stderr, "dbg5     lsb1:                        %d\n", ss->lsb1);
		fprintf(stderr, "dbg5     lsb2:                        %d\n", ss->lsb2);
		for (int i = 0; i < 3; i++)
            fprintf(stderr, "dbg5     reserved2[%d]:               %d\n", i, ss->reserved2[i]);
		fprintf(stderr, "dbg5     validityFlag:                %d\n", ss->validityFlag);
		fprintf(stderr, "dbg5     reserved3:                   %d\n", ss->reserved3);
		fprintf(stderr, "dbg5     dataFormat:                  %d\n", ss->dataFormat);
		fprintf(stderr, "dbg5     NMEAantennaeR:               %d\n", ss->NMEAantennaeR);
		fprintf(stderr, "dbg5     NMEAantennaeO:               %d\n", ss->NMEAantennaeO);
		for (int i = 0; i < 2; i++)
			fprintf(stderr, "dbg5     reserved4[%d]:               %d\n", i, ss->reserved4[i]);
		fprintf(stderr, "dbg5     kmOfPipe:                    %f\n", ss->kmOfPipe);
		for (int i = 0; i < 16; i++)
			fprintf(stderr, "dbg5     reserved5[%d]:               %d\n", i, ss->reserved5[i]);
		fprintf(stderr, "dbg5     coordX:                      %d\n", ss->coordX);
		fprintf(stderr, "dbg5     coordY:                      %d\n", ss->coordY);
		fprintf(stderr, "dbg5     coordUnits:                  %d\n", ss->coordUnits);
		fprintf(stderr, "dbg5     annotation:                  %s\n", ss->annotation);
		fprintf(stderr, "dbg5     samples:                     %d\n", ss->samples);
		fprintf(stderr, "dbg5     sampleInterval:              %d\n", ss->sampleInterval);
		fprintf(stderr, "dbg5     ADCGain:                     %d\n", ss->ADCGain);
		fprintf(stderr, "dbg5     pulsePower:                  %d\n", ss->pulsePower);
		fprintf(stderr, "dbg5     reserved6:                   %d\n", ss->reserved6);
		fprintf(stderr, "dbg5     startFreq:                   %d\n", ss->startFreq);
		fprintf(stderr, "dbg5     endFreq:                     %d\n", ss->endFreq);
		fprintf(stderr, "dbg5     sweepLength:                 %d\n", ss->sweepLength);
		fprintf(stderr, "dbg5     pressure:                    %d\n", ss->pressure);
		fprintf(stderr, "dbg5     sonarDepth:                  %d\n", ss->sonarDepth);
		fprintf(stderr, "dbg5     sampleFreq:                  %d\n", ss->sampleFreq);
		fprintf(stderr, "dbg5     pulseID:                     %d\n", ss->pulseID);
		fprintf(stderr, "dbg5     sonarAltitude:               %d\n", ss->sonarAltitude);
		fprintf(stderr, "dbg5     soundspeed:                  %f\n", ss->soundspeed);
		fprintf(stderr, "dbg5     mixerFrequency:              %f\n", ss->mixerFrequency);
		fprintf(stderr, "dbg5     year:                        %d\n", ss->year);
		fprintf(stderr, "dbg5     day:                         %d\n", ss->day);
		fprintf(stderr, "dbg5     hour:                        %d\n", ss->hour);
		fprintf(stderr, "dbg5     minute:                      %d\n", ss->minute);
		fprintf(stderr, "dbg5     second:                      %d\n", ss->second);
		fprintf(stderr, "dbg5     timeBasis:                   %d\n", ss->timeBasis);
		fprintf(stderr, "dbg5     weightingFactor:             %d\n", ss->weightingFactor);
		fprintf(stderr, "dbg5     numberPulses:                %d\n", ss->numberPulses);
		fprintf(stderr, "dbg5     heading:                     %d\n", ss->heading);
		fprintf(stderr, "dbg5     pitch:                       %d\n", ss->pitch);
		fprintf(stderr, "dbg5     roll:                        %d\n", ss->roll);
		fprintf(stderr, "dbg5     reserved8:                   %d\n", ss->reserved8);
		fprintf(stderr, "dbg5     reserved9:                   %d\n", ss->reserved9);
		fprintf(stderr, "dbg5     triggerSource:               %d\n", ss->triggerSource);
		fprintf(stderr, "dbg5     markNumber:                  %d\n", ss->markNumber);
		fprintf(stderr, "dbg5     NMEAHour:                    %d\n", ss->NMEAHour);
		fprintf(stderr, "dbg5     NMEAMinutes:                 %d\n", ss->NMEAMinutes);
		fprintf(stderr, "dbg5     NMEASeconds:                 %d\n", ss->NMEASeconds);
		fprintf(stderr, "dbg5     NMEACourse:                  %d\n", ss->NMEACourse);
		fprintf(stderr, "dbg5     NMEASpeed:                   %d\n", ss->NMEASpeed);
		fprintf(stderr, "dbg5     NMEADay:                     %d\n", ss->NMEADay);
		fprintf(stderr, "dbg5     NMEAYear:                    %d\n", ss->NMEAYear);
		fprintf(stderr, "dbg5     millisecondsToday:           %d\n", ss->millisecondsToday);
		fprintf(stderr, "dbg5     ADCMax:                      %d\n", ss->ADCMax);
		fprintf(stderr, "dbg5     reserved10:                  %d\n", ss->reserved10);
		fprintf(stderr, "dbg5     reserved10:                  %d\n", ss->reserved11);
		fprintf(stderr, "dbg5     softwareVersion:             %s\n", ss->softwareVersion);
		fprintf(stderr, "dbg5     sphericalCorrection:         %d\n", ss->sphericalCorrection);
		fprintf(stderr, "dbg5     packetNum:                   %d\n", ss->packetNum);
		fprintf(stderr, "dbg5     ADCDecimation:               %d\n", ss->ADCDecimation);
		fprintf(stderr, "dbg5     reserved12:                  %d\n", ss->reserved12);
		fprintf(stderr, "dbg5     temperature:                 %d\n", ss->temperature);
		fprintf(stderr, "dbg5     layback:                     %f\n", ss->layback);
		fprintf(stderr, "dbg5     reserved13:                  %d\n", ss->reserved13);
		fprintf(stderr, "dbg5     cableOut:                    %d\n", ss->cableOut);
		fprintf(stderr, "dbg5     reserved14:                  %d\n", ss->reserved14);
		if (ss->dataFormat == 1) {
			for (int i = 0; i < ss->samples; i++)
				fprintf(stderr, "dbg5     Channel 0[%d]: %10d %10d\n", i, ss->trace[2 * i], ss->trace[2 * i + 1]);
		}
		else {
			for (int i = 0; i < ss->samples; i++)
				fprintf(stderr, "dbg5     Channel 0[%d]: %10d\n", i, ss->trace[i]);
		}

		ss = (struct mbsys_jstar_channel_struct *)&(store->ssstbd);
		fprintf(stderr, "\ndbg5  Channel 1 (Starboard):\n");
		fprintf(stderr, "dbg5     start_marker:                %d\n", ss->message.start_marker);
		fprintf(stderr, "dbg5     version:                     %d\n", ss->message.version);
		fprintf(stderr, "dbg5     session:                     %d\n", ss->message.session);
		fprintf(stderr, "dbg5     type:                        %d\n", ss->message.type);
		fprintf(stderr, "dbg5     command:                     %d\n", ss->message.command);
		fprintf(stderr, "dbg5     subsystem:                   %d\n", ss->message.subsystem);
		fprintf(stderr, "dbg5     channel:                     %d\n", ss->message.channel);
		fprintf(stderr, "dbg5     sequence:                    %d\n", ss->message.sequence);
		fprintf(stderr, "dbg5     reserved:                    %d\n", ss->message.reserved);
		fprintf(stderr, "dbg5     size:                        %d\n", ss->message.size);

		fprintf(stderr, "dbg5     pingTime:                    %d\n", ss->pingTime);
		fprintf(stderr, "dbg5     startDepth:                  %d\n", ss->startDepth);
		fprintf(stderr, "dbg5     pingNum:                     %d\n", ss->pingNum);
		for (int i = 0; i < 2; i++)
            fprintf(stderr, "dbg5     reserved1[%d]:               %d\n", i, ss->reserved1[i]);
		fprintf(stderr, "dbg5     msb:                         %d\n", ss->msb);
		fprintf(stderr, "dbg5     lsb1:                        %d\n", ss->lsb1);
		fprintf(stderr, "dbg5     lsb2:                        %d\n", ss->lsb2);
		for (int i = 0; i < 3; i++)
            fprintf(stderr, "dbg5     reserved2[%d]:               %d\n", i, ss->reserved2[i]);
		fprintf(stderr, "dbg5     validityFlag:                %d\n", ss->validityFlag);
		fprintf(stderr, "dbg5     reserved3:                   %d\n", ss->reserved3);
		fprintf(stderr, "dbg5     dataFormat:                  %d\n", ss->dataFormat);
		fprintf(stderr, "dbg5     NMEAantennaeR:               %d\n", ss->NMEAantennaeR);
		fprintf(stderr, "dbg5     NMEAantennaeO:               %d\n", ss->NMEAantennaeO);
		for (int i = 0; i < 2; i++)
			fprintf(stderr, "dbg5     reserved4[%d]:               %d\n", i, ss->reserved4[i]);
		fprintf(stderr, "dbg5     kmOfPipe:                    %f\n", ss->kmOfPipe);
		for (int i = 0; i < 16; i++)
			fprintf(stderr, "dbg5     reserved5[%d]:               %d\n", i, ss->reserved5[i]);
		fprintf(stderr, "dbg5     coordX:                      %d\n", ss->coordX);
		fprintf(stderr, "dbg5     coordY:                      %d\n", ss->coordY);
		fprintf(stderr, "dbg5     coordUnits:                  %d\n", ss->coordUnits);
		fprintf(stderr, "dbg5     annotation:                  %s\n", ss->annotation);
		fprintf(stderr, "dbg5     samples:                     %d\n", ss->samples);
		fprintf(stderr, "dbg5     sampleInterval:              %d\n", ss->sampleInterval);
		fprintf(stderr, "dbg5     ADCGain:                     %d\n", ss->ADCGain);
		fprintf(stderr, "dbg5     pulsePower:                  %d\n", ss->pulsePower);
		fprintf(stderr, "dbg5     reserved6:                   %d\n", ss->reserved6);
		fprintf(stderr, "dbg5     startFreq:                   %d\n", ss->startFreq);
		fprintf(stderr, "dbg5     endFreq:                     %d\n", ss->endFreq);
		fprintf(stderr, "dbg5     sweepLength:                 %d\n", ss->sweepLength);
		fprintf(stderr, "dbg5     pressure:                    %d\n", ss->pressure);
		fprintf(stderr, "dbg5     sonarDepth:                  %d\n", ss->sonarDepth);
		fprintf(stderr, "dbg5     sampleFreq:                  %d\n", ss->sampleFreq);
		fprintf(stderr, "dbg5     pulseID:                     %d\n", ss->pulseID);
		fprintf(stderr, "dbg5     sonarAltitude:               %d\n", ss->sonarAltitude);
		fprintf(stderr, "dbg5     soundspeed:                  %f\n", ss->soundspeed);
		fprintf(stderr, "dbg5     mixerFrequency:              %f\n", ss->mixerFrequency);
		fprintf(stderr, "dbg5     year:                        %d\n", ss->year);
		fprintf(stderr, "dbg5     day:                         %d\n", ss->day);
		fprintf(stderr, "dbg5     hour:                        %d\n", ss->hour);
		fprintf(stderr, "dbg5     minute:                      %d\n", ss->minute);
		fprintf(stderr, "dbg5     second:                      %d\n", ss->second);
		fprintf(stderr, "dbg5     timeBasis:                   %d\n", ss->timeBasis);
		fprintf(stderr, "dbg5     weightingFactor:             %d\n", ss->weightingFactor);
		fprintf(stderr, "dbg5     numberPulses:                %d\n", ss->numberPulses);
		fprintf(stderr, "dbg5     heading:                     %d\n", ss->heading);
		fprintf(stderr, "dbg5     pitch:                       %d\n", ss->pitch);
		fprintf(stderr, "dbg5     roll:                        %d\n", ss->roll);
		fprintf(stderr, "dbg5     reserved8:                   %d\n", ss->reserved8);
		fprintf(stderr, "dbg5     reserved9:                   %d\n", ss->reserved9);
		fprintf(stderr, "dbg5     triggerSource:               %d\n", ss->triggerSource);
		fprintf(stderr, "dbg5     markNumber:                  %d\n", ss->markNumber);
		fprintf(stderr, "dbg5     NMEAHour:                    %d\n", ss->NMEAHour);
		fprintf(stderr, "dbg5     NMEAMinutes:                 %d\n", ss->NMEAMinutes);
		fprintf(stderr, "dbg5     NMEASeconds:                 %d\n", ss->NMEASeconds);
		fprintf(stderr, "dbg5     NMEACourse:                  %d\n", ss->NMEACourse);
		fprintf(stderr, "dbg5     NMEASpeed:                   %d\n", ss->NMEASpeed);
		fprintf(stderr, "dbg5     NMEADay:                     %d\n", ss->NMEADay);
		fprintf(stderr, "dbg5     NMEAYear:                    %d\n", ss->NMEAYear);
		fprintf(stderr, "dbg5     millisecondsToday:           %d\n", ss->millisecondsToday);
		fprintf(stderr, "dbg5     ADCMax:                      %d\n", ss->ADCMax);
		fprintf(stderr, "dbg5     reserved10:                  %d\n", ss->reserved10);
		fprintf(stderr, "dbg5     reserved10:                  %d\n", ss->reserved11);
		fprintf(stderr, "dbg5     softwareVersion:             %s\n", ss->softwareVersion);
		fprintf(stderr, "dbg5     sphericalCorrection:         %d\n", ss->sphericalCorrection);
		fprintf(stderr, "dbg5     packetNum:                   %d\n", ss->packetNum);
		fprintf(stderr, "dbg5     ADCDecimation:               %d\n", ss->ADCDecimation);
		fprintf(stderr, "dbg5     reserved12:                  %d\n", ss->reserved12);
		fprintf(stderr, "dbg5     temperature:                 %d\n", ss->temperature);
		fprintf(stderr, "dbg5     layback:                     %f\n", ss->layback);
		fprintf(stderr, "dbg5     reserved13:                  %d\n", ss->reserved13);
		fprintf(stderr, "dbg5     cableOut:                    %d\n", ss->cableOut);
		fprintf(stderr, "dbg5     reserved14:                  %d\n", ss->reserved14);
		if (ss->dataFormat == 1) {
			for (int i = 0; i < ss->samples; i++)
				fprintf(stderr, "dbg5     Channel 1[%d]: %10d %10d\n", i, ss->trace[2 * i], ss->trace[2 * i + 1]);
		}
		else {
			for (int i = 0; i < ss->samples; i++)
				fprintf(stderr, "dbg5     Channel 1[%d]: %10d\n", i, ss->trace[i]);
		}
	}
	else if (verbose >= 5 && store->kind == MB_DATA_ATTITUDE) {
		fprintf(stderr, "\ndbg5  New roll pitch data record to be written by MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5  Subsystem ID:\n");
		fprintf(stderr, "dbg5       subsystem:        %d ", store->subsystem);
		if (store->subsystem == MBSYS_JSTAR_SUBSYSTEM_SSLOW)
			fprintf(stderr, "(75 or 120 kHz sidescan)\n");
		else if (store->subsystem == MBSYS_JSTAR_SUBSYSTEM_SSHIGH)
			fprintf(stderr, "(410 kHz sidescan)\n");

		pitchroll = (struct mbsys_jstar_pitchroll_struct *)&(store->pitchroll);
		fprintf(stderr, "\ndbg5  Roll and Pitch:\n");
		fprintf(stderr, "dbg5     start_marker:                %d\n", pitchroll->message.start_marker);
		fprintf(stderr, "dbg5     version:                     %d\n", pitchroll->message.version);
		fprintf(stderr, "dbg5     session:                     %d\n", pitchroll->message.session);
		fprintf(stderr, "dbg5     type:                        %d\n", pitchroll->message.type);
		fprintf(stderr, "dbg5     command:                     %d\n", pitchroll->message.command);
		fprintf(stderr, "dbg5     subsystem:                   %d\n", pitchroll->message.subsystem);
		fprintf(stderr, "dbg5     channel:                     %d\n", pitchroll->message.channel);
		fprintf(stderr, "dbg5     sequence:                    %d\n", pitchroll->message.sequence);
		fprintf(stderr, "dbg5     reserved:                    %d\n", pitchroll->message.reserved);
		fprintf(stderr, "dbg5     size:                        %d\n", pitchroll->message.size);

		fprintf(stderr, "dbg5     seconds:                     %d\n", pitchroll->seconds);
		fprintf(stderr, "dbg5     milliseconds:                %d\n", pitchroll->milliseconds);
		fprintf(stderr, "dbg5     reserve1[0]:                 %d\n", pitchroll->reserve1[0]);
		fprintf(stderr, "dbg5     reserve1[1]:                 %d\n", pitchroll->reserve1[1]);
		fprintf(stderr, "dbg5     reserve1[2]:                 %d\n", pitchroll->reserve1[2]);
		fprintf(stderr, "dbg5     reserve1[3]:                 %d\n", pitchroll->reserve1[3]);
		fprintf(stderr, "dbg5     accelerationx:               %d\n", pitchroll->accelerationx);
		fprintf(stderr, "dbg5     accelerationy:               %d\n", pitchroll->accelerationy);
		fprintf(stderr, "dbg5     accelerationz:               %d\n", pitchroll->accelerationz);
		fprintf(stderr, "dbg5     gyroratex:                   %d\n", pitchroll->gyroratex);
		fprintf(stderr, "dbg5     gyroratey:                   %d\n", pitchroll->gyroratey);
		fprintf(stderr, "dbg5     gyroratez:                   %d\n", pitchroll->gyroratez);
		fprintf(stderr, "dbg5     pitch:                       %d\n", pitchroll->pitch);
		fprintf(stderr, "dbg5     roll:                        %d\n", pitchroll->roll);
		fprintf(stderr, "dbg5     temperature:                 %d\n", pitchroll->temperature);
		fprintf(stderr, "dbg5     deviceinfo:                  %d\n", pitchroll->deviceinfo);
		fprintf(stderr, "dbg5     heave:                       %d\n", pitchroll->heave);
		fprintf(stderr, "dbg5     heading:                     %d\n", pitchroll->heading);
		fprintf(stderr, "dbg5     datavalidflags:              %d\n", pitchroll->datavalidflags);
		fprintf(stderr, "dbg5     reserve2:                    %d\n", pitchroll->reserve2);
	}
	else if (verbose >= 5 && store->kind == MB_DATA_DVL) {
		fprintf(stderr, "\ndbg5  New dvl data record to be written by MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5  Subsystem ID:\n");
		fprintf(stderr, "dbg5       subsystem:        %d ", store->subsystem);
		if (store->subsystem == MBSYS_JSTAR_SUBSYSTEM_SSLOW)
			fprintf(stderr, "(75 or 120 kHz sidescan)\n");
		else if (store->subsystem == MBSYS_JSTAR_SUBSYSTEM_SSHIGH)
			fprintf(stderr, "(410 kHz sidescan)\n");

		dvl = (struct mbsys_jstar_dvl_struct *)&(store->dvl);
		fprintf(stderr, "\ndbg5  Roll and Pitch:\n");
		fprintf(stderr, "dbg5     start_marker:                %d\n", dvl->message.start_marker);
		fprintf(stderr, "dbg5     version:                     %d\n", dvl->message.version);
		fprintf(stderr, "dbg5     session:                     %d\n", dvl->message.session);
		fprintf(stderr, "dbg5     type:                        %d\n", dvl->message.type);
		fprintf(stderr, "dbg5     command:                     %d\n", dvl->message.command);
		fprintf(stderr, "dbg5     subsystem:                   %d\n", dvl->message.subsystem);
		fprintf(stderr, "dbg5     channel:                     %d\n", dvl->message.channel);
		fprintf(stderr, "dbg5     sequence:                    %d\n", dvl->message.sequence);
		fprintf(stderr, "dbg5     reserved:                    %d\n", dvl->message.reserved);
		fprintf(stderr, "dbg5     size:                        %d\n", dvl->message.size);

		fprintf(stderr, "dbg5     seconds:                     %d\n", dvl->seconds);
		fprintf(stderr, "dbg5     milliseconds:                %d\n", dvl->milliseconds);
		fprintf(stderr, "dbg5     reserve1[0]:                 %d\n", dvl->reserve1[0]);
		fprintf(stderr, "dbg5     reserve1[1]:                 %d\n", dvl->reserve1[1]);
		fprintf(stderr, "dbg5     reserve1[2]:                 %d\n", dvl->reserve1[2]);
		fprintf(stderr, "dbg5     reserve1[3]:                 %d\n", dvl->reserve1[3]);
		fprintf(stderr, "dbg5     datavalidflags:              %d\n", dvl->datavalidflags);
		fprintf(stderr, "dbg5     beam1range:                  %d\n", dvl->beam1range);
		fprintf(stderr, "dbg5     beam2range:                  %d\n", dvl->beam2range);
		fprintf(stderr, "dbg5     beam3range:                  %d\n", dvl->beam3range);
		fprintf(stderr, "dbg5     beam4range:                  %d\n", dvl->beam4range);
		fprintf(stderr, "dbg5     velocitybottomx:             %d\n", dvl->velocitybottomx);
		fprintf(stderr, "dbg5     velocitybottomy:             %d\n", dvl->velocitybottomy);
		fprintf(stderr, "dbg5     velocitybottomz:             %d\n", dvl->velocitybottomz);
		fprintf(stderr, "dbg5     velocitywaterx:              %d\n", dvl->velocitywaterx);
		fprintf(stderr, "dbg5     velocitywatery:              %d\n", dvl->velocitywatery);
		fprintf(stderr, "dbg5     velocitywaterz:              %d\n", dvl->velocitywaterz);
		fprintf(stderr, "dbg5     depth:                       %d\n", dvl->depth);
		fprintf(stderr, "dbg5     pitch:                       %d\n", dvl->pitch);
		fprintf(stderr, "dbg5     roll:                        %d\n", dvl->roll);
		fprintf(stderr, "dbg5     heading:                     %d\n", dvl->heading);
		fprintf(stderr, "dbg5     salinity:                    %d\n", dvl->salinity);
		fprintf(stderr, "dbg5     temperature:                 %d\n", dvl->temperature);
		fprintf(stderr, "dbg5     soundspeed:                  %d\n", dvl->soundspeed);
		for (int i = 0; i < 7; i++)
			fprintf(stderr, "dbg5     reserve2[%d]:                %d\n", i, dvl->reserve2[i]);
	}
	else if (verbose >= 5 && store->kind == MB_DATA_CTD) {
		pressure = (struct mbsys_jstar_pressure_struct *)&(store->pressure);
		fprintf(stderr, "\ndbg5  New pressure data record to be written by MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5     start_marker:                %d\n", pressure->message.start_marker);
		fprintf(stderr, "dbg5     version:                     %d\n", pressure->message.version);
		fprintf(stderr, "dbg5     session:                     %d\n", pressure->message.session);
		fprintf(stderr, "dbg5     type:                        %d\n", pressure->message.type);
		fprintf(stderr, "dbg5     command:                     %d\n", pressure->message.command);
		fprintf(stderr, "dbg5     subsystem:                   %d\n", pressure->message.subsystem);
		fprintf(stderr, "dbg5     channel:                     %d\n", pressure->message.channel);
		fprintf(stderr, "dbg5     sequence:                    %d\n", pressure->message.sequence);
		fprintf(stderr, "dbg5     reserved:                    %d\n", pressure->message.reserved);
		fprintf(stderr, "dbg5     size:                        %d\n", pressure->message.size);

		fprintf(stderr, "dbg5     seconds:                     %d\n", pressure->seconds);
		fprintf(stderr, "dbg5     milliseconds:                %d\n", pressure->milliseconds);
		fprintf(stderr, "dbg5     reserve1[0]:                 %d\n", pressure->reserve1[0]);
		fprintf(stderr, "dbg5     reserve1[1]:                 %d\n", pressure->reserve1[1]);
		fprintf(stderr, "dbg5     reserve1[2]:                 %d\n", pressure->reserve1[2]);
		fprintf(stderr, "dbg5     reserve1[3]:                 %d\n", pressure->reserve1[3]);
		fprintf(stderr, "dbg5     pressure:                    %d\n", pressure->pressure);
		fprintf(stderr, "dbg5     salinity:                    %d\n", pressure->salinity);
		fprintf(stderr, "dbg5     datavalidflags:              %d\n", pressure->datavalidflags);
		fprintf(stderr, "dbg5     conductivity:                %d\n", pressure->conductivity);
		fprintf(stderr, "dbg5     soundspeed:                  %d\n", pressure->soundspeed);
		for (int i = 0; i < 10; i++)
			fprintf(stderr, "dbg5     reserve2[%2d]:                 %d\n", i, pressure->reserve2[i]);
	}
	else if (verbose >= 5 &&
	         (store->kind == MB_DATA_NMEA_RMC || store->kind == MB_DATA_NMEA_DBT || store->kind == MB_DATA_NMEA_DPT)) {
		nmea = (struct mbsys_jstar_nmea_struct *)&(store->nmea);
		fprintf(stderr, "\ndbg5  New NMEA data record to be written by MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5     start_marker:                %d\n", nmea->message.start_marker);
		fprintf(stderr, "dbg5     version:                     %d\n", nmea->message.version);
		fprintf(stderr, "dbg5     session:                     %d\n", nmea->message.session);
		fprintf(stderr, "dbg5     type:                        %d\n", nmea->message.type);
		fprintf(stderr, "dbg5     command:                     %d\n", nmea->message.command);
		fprintf(stderr, "dbg5     subsystem:                   %d\n", nmea->message.subsystem);
		fprintf(stderr, "dbg5     channel:                     %d\n", nmea->message.channel);
		fprintf(stderr, "dbg5     sequence:                    %d\n", nmea->message.sequence);
		fprintf(stderr, "dbg5     reserved:                    %d\n", nmea->message.reserved);
		fprintf(stderr, "dbg5     size:                        %d\n", nmea->message.size);

		fprintf(stderr, "dbg5     seconds:                     %d\n", nmea->seconds);
		fprintf(stderr, "dbg5     milliseconds:                %d\n", nmea->milliseconds);
		fprintf(stderr, "dbg5     source:                      %d\n", nmea->source);
		fprintf(stderr, "dbg5     reserve[0]:                  %d\n", nmea->reserve[0]);
		fprintf(stderr, "dbg5     reserve[1]:                  %d\n", nmea->reserve[1]);
		fprintf(stderr, "dbg5     reserve[2]:                  %d\n", nmea->reserve[2]);
		fprintf(stderr, "dbg5     nmea:                        %s\n", nmea->nmea);
	}
	else if (verbose >= 5 && (store->kind == MB_DATA_HEADER)) {
		sysinfo = (struct mbsys_jstar_sysinfo_struct *)&(store->sysinfo);
		fprintf(stderr, "\ndbg5  New sysinfo data record to be written by MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg5     start_marker:                %d\n", sysinfo->message.start_marker);
		fprintf(stderr, "dbg5     version:                     %d\n", sysinfo->message.version);
		fprintf(stderr, "dbg5     session:                     %d\n", sysinfo->message.session);
		fprintf(stderr, "dbg5     type:                        %d\n", sysinfo->message.type);
		fprintf(stderr, "dbg5     command:                     %d\n", sysinfo->message.command);
		fprintf(stderr, "dbg5     subsystem:                   %d\n", sysinfo->message.subsystem);
		fprintf(stderr, "dbg5     channel:                     %d\n", sysinfo->message.channel);
		fprintf(stderr, "dbg5     sequence:                    %d\n", sysinfo->message.sequence);
		fprintf(stderr, "dbg5     reserved:                    %d\n", sysinfo->message.reserved);
		fprintf(stderr, "dbg5     size:                        %d\n", sysinfo->message.size);

		fprintf(stderr, "dbg5     system_type:                 %d\n", sysinfo->system_type);
		fprintf(stderr, "dbg5     reserved1:                   %d\n", sysinfo->reserved1);
		fprintf(stderr, "dbg5     version:                     %d\n", sysinfo->version);
		fprintf(stderr, "dbg5     reserved2:                   %d\n", sysinfo->reserved2);
		fprintf(stderr, "dbg5     platformserialnumber:        %d\n", sysinfo->platformserialnumber);
		fprintf(stderr, "dbg5     sysinfosize:                 %d\n", sysinfo->sysinfosize);
		fprintf(stderr, "dbg5     sysinfo:                     \n%s\n", sysinfo->sysinfo);
	}

	int status = MB_SUCCESS;

	/* write out comment */
	if (store->kind == MB_DATA_COMMENT) {
		/* insert the message header values */
		index = 0;
		comment = (struct mbsys_jstar_comment_struct *)&(store->comment);
		comment->message.start_marker = 0x1601;
		comment->message.version = 10;
		comment->message.session = 0;
		comment->message.type = MBSYS_JSTAR_DATA_COMMENT;
		comment->message.subsystem = 0;
		comment->message.channel = 0;
		comment->message.sequence = 0;
		comment->message.reserved = 0;
		comment->message.size = strlen(comment->comment) + 1;
		mb_put_binary_short(MB_YES, comment->message.start_marker, &buffer[index]);
		index += 2;
		buffer[index] = comment->message.version;
		index++;
		buffer[index] = comment->message.session;
		index++;
		mb_put_binary_short(MB_YES, comment->message.type, &buffer[index]);
		index += 2;
		buffer[index] = comment->message.command;
		index++;
		buffer[index] = comment->message.subsystem;
		index++;
		buffer[index] = comment->message.channel;
		index++;
		buffer[index] = comment->message.sequence;
		index++;
		mb_put_binary_short(MB_YES, comment->message.reserved, &buffer[index]);
		index += 2;
		mb_put_binary_int(MB_YES, comment->message.size, &buffer[index]);
		index += 4;

		/* write the message header */
		if ((write_len = fwrite(buffer, 1, MBSYS_JSTAR_MESSAGE_SIZE, mb_io_ptr->mbfp)) != MBSYS_JSTAR_MESSAGE_SIZE) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}

		/* write the comment */
		if ((write_len = fwrite(comment->comment, 1, comment->message.size, mb_io_ptr->mbfp)) != comment->message.size) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
	}

	/* write out subbottom data */
	else if (store->kind == MB_DATA_SUBBOTTOM_SUBBOTTOM) {
		/* get the sbp structure */
		sbp = (struct mbsys_jstar_channel_struct *)&(store->sbp);

		/* insert the message header values */
		index = 0;
		if (sbp->dataFormat == 1)
			shortspersample = 2;
		else
			shortspersample = 1;
		sbp->message.size = shortspersample * sbp->samples * sizeof(short) + MBSYS_JSTAR_SBPHEADER_SIZE;
		mb_put_binary_short(MB_YES, sbp->message.start_marker, &buffer[index]);
		index += 2;
		buffer[index] = sbp->message.version;
		index++;
		buffer[index] = sbp->message.session;
		index++;
		mb_put_binary_short(MB_YES, sbp->message.type, &buffer[index]);
		index += 2;
		buffer[index] = sbp->message.command;
		index++;
		buffer[index] = sbp->message.subsystem;
		index++;
		buffer[index] = sbp->message.channel;
		index++;
		buffer[index] = sbp->message.sequence;
		index++;
		mb_put_binary_short(MB_YES, sbp->message.reserved, &buffer[index]);
		index += 2;
		mb_put_binary_int(MB_YES, sbp->message.size, &buffer[index]);
		index += 4;

		/* write the messsage header */
		if ((write_len = fwrite(buffer, 1, MBSYS_JSTAR_MESSAGE_SIZE, mb_io_ptr->mbfp)) != MBSYS_JSTAR_MESSAGE_SIZE) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}

		/* insert the trace header values */
		index = 0;
		mb_put_binary_int(MB_YES, sbp->pingTime, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, sbp->startDepth, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, sbp->pingNum, &buffer[index]);
		index += 4;
		for (int i = 0; i < 2; i++) {
			mb_put_binary_short(MB_YES, sbp->reserved1[i], &buffer[index]);
			index += 2;
		}
		mb_put_binary_short(MB_YES, sbp->msb, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sbp->lsb1, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sbp->lsb2, &buffer[index]);
		index += 2;
		for (int i = 0; i < 3; i++) {
			mb_put_binary_short(MB_YES, sbp->reserved2[i], &buffer[index]);
			index += 2;
		}
		mb_put_binary_short(MB_YES, sbp->traceIDCode, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sbp->validityFlag, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sbp->reserved3, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sbp->dataFormat, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sbp->NMEAantennaeR, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sbp->NMEAantennaeO, &buffer[index]);
		index += 2;
		for (int i = 0; i < 2; i++) {
			mb_put_binary_short(MB_YES, sbp->reserved4[i], &buffer[index]);
			index += 2;
		}
		mb_put_binary_float(MB_YES, sbp->kmOfPipe, &buffer[index]);
		index += 4;
		for (int i = 0; i < 16; i++) {
			mb_put_binary_short(MB_YES, sbp->reserved5[i], &buffer[index]);
			index += 2;
		}
		mb_put_binary_int(MB_YES, sbp->coordX, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, sbp->coordY, &buffer[index]);
		index += 4;
		mb_put_binary_short(MB_YES, sbp->coordUnits, &buffer[index]);
		index += 2;
		for (int i = 0; i < 24; i++) {
			buffer[index] = sbp->annotation[i];
			index++;
		}
		mb_put_binary_short(MB_YES, sbp->samples, &buffer[index]);
		index += 2;
		mb_put_binary_int(MB_YES, sbp->sampleInterval, &buffer[index]);
		index += 4;
		mb_put_binary_short(MB_YES, sbp->ADCGain, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sbp->pulsePower, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sbp->reserved6, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sbp->startFreq, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sbp->endFreq, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sbp->sweepLength, &buffer[index]);
		index += 2;
		mb_put_binary_int(MB_YES, sbp->pressure, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, sbp->sonarDepth, &buffer[index]);
		index += 4;
		mb_put_binary_short(MB_YES, sbp->sampleFreq, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sbp->pulseID, &buffer[index]);
		index += 2;
		mb_put_binary_int(MB_YES, sbp->sonarAltitude, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, sbp->soundspeed, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, sbp->mixerFrequency, &buffer[index]);
		index += 4;
		mb_put_binary_short(MB_YES, sbp->year, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sbp->day, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sbp->hour, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sbp->minute, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sbp->second, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sbp->timeBasis, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sbp->weightingFactor, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sbp->numberPulses, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sbp->heading, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sbp->pitch, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sbp->roll, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sbp->reserved8, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sbp->reserved9, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sbp->triggerSource, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sbp->markNumber, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sbp->NMEAHour, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sbp->NMEAMinutes, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sbp->NMEASeconds, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sbp->NMEACourse, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sbp->NMEASpeed, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sbp->NMEADay, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sbp->NMEAYear, &buffer[index]);
		index += 2;
		mb_put_binary_int(MB_YES, sbp->millisecondsToday, &buffer[index]);
		index += 4;
		mb_put_binary_short(MB_YES, sbp->ADCMax, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sbp->reserved10, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sbp->reserved11, &buffer[index]);
		index += 2;
		for (int i = 0; i < 6; i++) {
			buffer[index] = sbp->softwareVersion[i];
			index++;
		}
		mb_put_binary_int(MB_YES, sbp->sphericalCorrection, &buffer[index]);
		index += 4;
		mb_put_binary_short(MB_YES, sbp->packetNum, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sbp->ADCDecimation, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sbp->reserved12, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sbp->temperature, &buffer[index]);
		index += 2;
		mb_put_binary_float(MB_YES, sbp->layback, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, sbp->reserved13, &buffer[index]);
		index += 4;
		mb_put_binary_short(MB_YES, sbp->cableOut, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, sbp->reserved14, &buffer[index]);
		index += 2;

		/* write the trace header */
		if ((write_len = fwrite(buffer, 1, MBSYS_JSTAR_SBPHEADER_SIZE, mb_io_ptr->mbfp)) != MBSYS_JSTAR_SBPHEADER_SIZE) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}

/* byteswap the trace if necessary */
#ifndef BYTESWAPPED
		if (sbp->dataFormat == 1)
			shortspersample = 2;
		else
			shortspersample = 1;
		for (int i = 0; i < shortspersample * sbp->samples; i++) {
			sbp->trace[i] = mb_swap_short(sbp->trace[i]);
			;
		}
#endif

		/* write the trace */
		trace_size = shortspersample * sbp->samples * sizeof(short);
		if ((write_len = fwrite(sbp->trace, 1, trace_size, mb_io_ptr->mbfp)) != trace_size) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
	}

	/* write out sidescan data */
	else if (store->kind == MB_DATA_DATA || store->kind == MB_DATA_SIDESCAN2) {
		/* get the port ss structure */
		ss = (struct mbsys_jstar_channel_struct *)&(store->ssport);

		/* insert the message header values */
		index = 0;
		if (ss->dataFormat == 1)
			shortspersample = 2;
		else
			shortspersample = 1;
		ss->message.size = shortspersample * ss->samples * sizeof(short) + MBSYS_JSTAR_SSHEADER_SIZE;
		mb_put_binary_short(MB_YES, ss->message.start_marker, &buffer[index]);
		index += 2;
		buffer[index] = ss->message.version;
		index++;
		buffer[index] = ss->message.session;
		index++;
		mb_put_binary_short(MB_YES, ss->message.type, &buffer[index]);
		index += 2;
		buffer[index] = ss->message.command;
		index++;
		buffer[index] = ss->message.subsystem;
		index++;
		buffer[index] = ss->message.channel;
		index++;
		buffer[index] = ss->message.sequence;
		index++;
		mb_put_binary_short(MB_YES, ss->message.reserved, &buffer[index]);
		index += 2;
		mb_put_binary_int(MB_YES, ss->message.size, &buffer[index]);
		index += 4;

		/* write the messsage header */
		if ((write_len = fwrite(buffer, 1, MBSYS_JSTAR_MESSAGE_SIZE, mb_io_ptr->mbfp)) != MBSYS_JSTAR_MESSAGE_SIZE) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}

		/* insert the trace header values */
		index = 0;
		mb_put_binary_int(MB_YES, ss->pingTime, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, ss->startDepth, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, ss->pingNum, &buffer[index]);
		index += 4;
		for (int i = 0; i < 2; i++) {
			mb_put_binary_short(MB_YES, ss->reserved1[i], &buffer[index]);
			index += 2;
		}
		mb_put_binary_short(MB_YES, ss->msb, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->lsb1, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->lsb2, &buffer[index]);
		index += 2;
		for (int i = 0; i < 3; i++) {
			mb_put_binary_short(MB_YES, ss->reserved2[i], &buffer[index]);
			index += 2;
		}
		mb_put_binary_short(MB_YES, ss->traceIDCode, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->validityFlag, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->reserved3, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->dataFormat, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->NMEAantennaeR, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->NMEAantennaeO, &buffer[index]);
		index += 2;
		for (int i = 0; i < 2; i++) {
			mb_put_binary_short(MB_YES, ss->reserved4[i], &buffer[index]);
			index += 2;
		}
		mb_put_binary_float(MB_YES, ss->kmOfPipe, &buffer[index]);
		index += 4;
		for (int i = 0; i < 16; i++) {
			mb_put_binary_short(MB_YES, ss->reserved5[i], &buffer[index]);
			index += 2;
		}
		mb_put_binary_int(MB_YES, ss->coordX, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, ss->coordY, &buffer[index]);
		index += 4;
		mb_put_binary_short(MB_YES, ss->coordUnits, &buffer[index]);
		index += 2;
		for (int i = 0; i < 24; i++) {
			buffer[index] = ss->annotation[i];
			index++;
		}
		mb_put_binary_short(MB_YES, ss->samples, &buffer[index]);
		index += 2;
		mb_put_binary_int(MB_YES, ss->sampleInterval, &buffer[index]);
		index += 4;
		mb_put_binary_short(MB_YES, ss->ADCGain, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->pulsePower, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->reserved6, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->startFreq, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->endFreq, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->sweepLength, &buffer[index]);
		index += 2;
		mb_put_binary_int(MB_YES, ss->pressure, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, ss->sonarDepth, &buffer[index]);
		index += 4;
		mb_put_binary_short(MB_YES, ss->sampleFreq, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->pulseID, &buffer[index]);
		index += 2;
		mb_put_binary_int(MB_YES, ss->sonarAltitude, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, ss->soundspeed, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, ss->mixerFrequency, &buffer[index]);
		index += 4;
		mb_put_binary_short(MB_YES, ss->year, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->day, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->hour, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->minute, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->second, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->timeBasis, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->weightingFactor, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->numberPulses, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->heading, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->pitch, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->roll, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->reserved8, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->reserved9, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->triggerSource, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->markNumber, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->NMEAHour, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->NMEAMinutes, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->NMEASeconds, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->NMEACourse, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->NMEASpeed, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->NMEADay, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->NMEAYear, &buffer[index]);
		index += 2;
		mb_put_binary_int(MB_YES, ss->millisecondsToday, &buffer[index]);
		index += 4;
		mb_put_binary_short(MB_YES, ss->ADCMax, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->reserved10, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->reserved11, &buffer[index]);
		index += 2;
		for (int i = 0; i < 6; i++) {
			buffer[index] = ss->softwareVersion[i];
			index++;
		}
		mb_put_binary_int(MB_YES, ss->sphericalCorrection, &buffer[index]);
		index += 4;
		mb_put_binary_short(MB_YES, ss->packetNum, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->ADCDecimation, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->reserved12, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->temperature, &buffer[index]);
		index += 2;
		mb_put_binary_float(MB_YES, ss->layback, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, ss->reserved13, &buffer[index]);
		index += 4;
		mb_put_binary_short(MB_YES, ss->cableOut, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->reserved14, &buffer[index]);
		index += 2;

		/* write the trace header */
		if ((write_len = fwrite(buffer, 1, MBSYS_JSTAR_SSHEADER_SIZE, mb_io_ptr->mbfp)) != MBSYS_JSTAR_SSHEADER_SIZE) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}

/* byteswap the trace if necessary */
#ifndef BYTESWAPPED
		if (ss->dataFormat == 1)
			shortspersample = 2;
		else
			shortspersample = 1;
		for (int i = 0; i < shortspersample * ss->samples; i++) {
			ss->trace[i] = mb_swap_short(ss->trace[i]);
			;
		}
#endif

		/* write the trace */
		trace_size = shortspersample * ss->samples * sizeof(short);
		if ((write_len = fwrite(ss->trace, 1, trace_size, mb_io_ptr->mbfp)) != trace_size) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}

		/* get the starboard ss structure */
		ss = (struct mbsys_jstar_channel_struct *)&(store->ssstbd);

		/* insert the message header values */
		index = 0;
		if (ss->dataFormat == 1)
			shortspersample = 2;
		else
			shortspersample = 1;
		ss->message.size = shortspersample * ss->samples * sizeof(short) + MBSYS_JSTAR_SSHEADER_SIZE;
		mb_put_binary_short(MB_YES, ss->message.start_marker, &buffer[index]);
		index += 2;
		buffer[index] = ss->message.version;
		index++;
		buffer[index] = ss->message.session;
		index++;
		mb_put_binary_short(MB_YES, ss->message.type, &buffer[index]);
		index += 2;
		buffer[index] = ss->message.command;
		index++;
		buffer[index] = ss->message.subsystem;
		index++;
		buffer[index] = ss->message.channel;
		index++;
		buffer[index] = ss->message.sequence;
		index++;
		mb_put_binary_short(MB_YES, ss->message.reserved, &buffer[index]);
		index += 2;
		mb_put_binary_int(MB_YES, ss->message.size, &buffer[index]);
		index += 4;

		/* write the messsage header */
		if ((write_len = fwrite(buffer, 1, MBSYS_JSTAR_MESSAGE_SIZE, mb_io_ptr->mbfp)) != MBSYS_JSTAR_MESSAGE_SIZE) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}

		/* insert the trace header values */
		index = 0;
		mb_put_binary_int(MB_YES, ss->pingTime, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, ss->startDepth, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, ss->pingNum, &buffer[index]);
		index += 4;
		for (int i = 0; i < 2; i++) {
			mb_put_binary_short(MB_YES, ss->reserved1[i], &buffer[index]);
			index += 2;
		}
		mb_put_binary_short(MB_YES, ss->msb, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->lsb1, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->lsb2, &buffer[index]);
		index += 2;
		for (int i = 0; i < 3; i++) {
			mb_put_binary_short(MB_YES, ss->reserved2[i], &buffer[index]);
			index += 2;
		}
		mb_put_binary_short(MB_YES, ss->traceIDCode, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->validityFlag, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->reserved3, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->dataFormat, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->NMEAantennaeR, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->NMEAantennaeO, &buffer[index]);
		index += 2;
		for (int i = 0; i < 2; i++) {
			mb_put_binary_short(MB_YES, ss->reserved4[i], &buffer[index]);
			index += 2;
		}
		mb_put_binary_float(MB_YES, ss->kmOfPipe, &buffer[index]);
		index += 4;
		for (int i = 0; i < 16; i++) {
			mb_put_binary_short(MB_YES, ss->reserved5[i], &buffer[index]);
			index += 2;
		}
		mb_put_binary_int(MB_YES, ss->coordX, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, ss->coordY, &buffer[index]);
		index += 4;
		mb_put_binary_short(MB_YES, ss->coordUnits, &buffer[index]);
		index += 2;
		for (int i = 0; i < 24; i++) {
			buffer[index] = ss->annotation[i];
			index++;
		}
		mb_put_binary_short(MB_YES, ss->samples, &buffer[index]);
		index += 2;
		mb_put_binary_int(MB_YES, ss->sampleInterval, &buffer[index]);
		index += 4;
		mb_put_binary_short(MB_YES, ss->ADCGain, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->pulsePower, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->reserved6, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->startFreq, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->endFreq, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->sweepLength, &buffer[index]);
		index += 2;
		mb_put_binary_int(MB_YES, ss->pressure, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, ss->sonarDepth, &buffer[index]);
		index += 4;
		mb_put_binary_short(MB_YES, ss->sampleFreq, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->pulseID, &buffer[index]);
		index += 2;
		mb_put_binary_int(MB_YES, ss->sonarAltitude, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, ss->soundspeed, &buffer[index]);
		index += 4;
		mb_put_binary_float(MB_YES, ss->mixerFrequency, &buffer[index]);
		index += 4;
		mb_put_binary_short(MB_YES, ss->year, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->day, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->hour, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->minute, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->second, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->timeBasis, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->weightingFactor, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->numberPulses, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->heading, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->pitch, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->roll, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->reserved8, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->reserved9, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->triggerSource, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->markNumber, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->NMEAHour, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->NMEAMinutes, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->NMEASeconds, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->NMEACourse, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->NMEASpeed, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->NMEADay, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->NMEAYear, &buffer[index]);
		index += 2;
		mb_put_binary_int(MB_YES, ss->millisecondsToday, &buffer[index]);
		index += 4;
		mb_put_binary_short(MB_YES, ss->ADCMax, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->reserved10, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->reserved11, &buffer[index]);
		index += 2;
		for (int i = 0; i < 6; i++) {
			buffer[index] = ss->softwareVersion[i];
			index++;
		}
		mb_put_binary_int(MB_YES, ss->sphericalCorrection, &buffer[index]);
		index += 4;
		mb_put_binary_short(MB_YES, ss->packetNum, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->ADCDecimation, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->reserved12, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->temperature, &buffer[index]);
		index += 2;
		mb_put_binary_float(MB_YES, ss->layback, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, ss->reserved13, &buffer[index]);
		index += 4;
		mb_put_binary_short(MB_YES, ss->cableOut, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, ss->reserved14, &buffer[index]);
		index += 2;

		/* write the trace header */
		if ((write_len = fwrite(buffer, 1, MBSYS_JSTAR_SSHEADER_SIZE, mb_io_ptr->mbfp)) != MBSYS_JSTAR_SSHEADER_SIZE) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}

/* byteswap the trace if necessary */
#ifndef BYTESWAPPED
		if (ss->dataFormat == 1)
			shortspersample = 2;
		else
			shortspersample = 1;
		for (int i = 0; i < shortspersample * ss->samples; i++) {
			ss->trace[i] = mb_swap_short(ss->trace[i]);
			;
		}
#endif

		/* write the trace */
		trace_size = shortspersample * ss->samples * sizeof(short);
		if ((write_len = fwrite(ss->trace, 1, trace_size, mb_io_ptr->mbfp)) != trace_size) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
	}

	/* write out pitch roll data */
	else if (store->kind == MB_DATA_ATTITUDE) {
		/* insert the message header values */
		index = 0;
		pitchroll = (struct mbsys_jstar_pitchroll_struct *)&(store->pitchroll);
		mb_put_binary_short(MB_YES, pitchroll->message.start_marker, &buffer[index]);
		index += 2;
		buffer[index] = pitchroll->message.version;
		index++;
		buffer[index] = pitchroll->message.session;
		index++;
		mb_put_binary_short(MB_YES, pitchroll->message.type, &buffer[index]);
		index += 2;
		buffer[index] = pitchroll->message.command;
		index++;
		buffer[index] = pitchroll->message.subsystem;
		index++;
		buffer[index] = pitchroll->message.channel;
		index++;
		buffer[index] = pitchroll->message.sequence;
		index++;
		mb_put_binary_short(MB_YES, pitchroll->message.reserved, &buffer[index]);
		index += 2;
		mb_put_binary_int(MB_YES, pitchroll->message.size, &buffer[index]);
		index += 4;

		/* write the message header */
		if ((write_len = fwrite(buffer, 1, MBSYS_JSTAR_MESSAGE_SIZE, mb_io_ptr->mbfp)) != MBSYS_JSTAR_MESSAGE_SIZE) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}

		index = 0;
		mb_put_binary_int(MB_YES, pitchroll->seconds, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, pitchroll->milliseconds, &buffer[index]);
		index += 4;
		buffer[index] = pitchroll->reserve1[0];
		index++;
		buffer[index] = pitchroll->reserve1[1];
		index++;
		buffer[index] = pitchroll->reserve1[2];
		index++;
		buffer[index] = pitchroll->reserve1[3];
		index++;
		mb_put_binary_short(MB_YES, pitchroll->accelerationx, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, pitchroll->accelerationy, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, pitchroll->accelerationz, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, pitchroll->gyroratex, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, pitchroll->gyroratey, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, pitchroll->gyroratez, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, pitchroll->pitch, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, pitchroll->roll, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, pitchroll->temperature, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, pitchroll->deviceinfo, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, pitchroll->heave, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, pitchroll->heading, &buffer[index]);
		index += 2;
		mb_put_binary_int(MB_YES, pitchroll->datavalidflags, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, pitchroll->reserve2, &buffer[index]);
		index += 4;

		/* write the pitchroll data */
		if ((write_len = fwrite(buffer, 1, pitchroll->message.size, mb_io_ptr->mbfp)) != pitchroll->message.size) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
	}

	/* write out dvl data */
	else if (store->kind == MB_DATA_DVL) {
		/* insert the message header values */
		index = 0;
		dvl = (struct mbsys_jstar_dvl_struct *)&(store->dvl);
		mb_put_binary_short(MB_YES, dvl->message.start_marker, &buffer[index]);
		index += 2;
		buffer[index] = dvl->message.version;
		index++;
		buffer[index] = dvl->message.session;
		index++;
		mb_put_binary_short(MB_YES, dvl->message.type, &buffer[index]);
		index += 2;
		buffer[index] = dvl->message.command;
		index++;
		buffer[index] = dvl->message.subsystem;
		index++;
		buffer[index] = dvl->message.channel;
		index++;
		buffer[index] = dvl->message.sequence;
		index++;
		mb_put_binary_short(MB_YES, dvl->message.reserved, &buffer[index]);
		index += 2;
		mb_put_binary_int(MB_YES, dvl->message.size, &buffer[index]);
		index += 4;

		/* write the message header */
		if ((write_len = fwrite(buffer, 1, MBSYS_JSTAR_MESSAGE_SIZE, mb_io_ptr->mbfp)) != MBSYS_JSTAR_MESSAGE_SIZE) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}

		index = 0;
		mb_put_binary_int(MB_YES, dvl->seconds, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, dvl->milliseconds, &buffer[index]);
		index += 4;
		buffer[index] = dvl->reserve1[0];
		index++;
		buffer[index] = dvl->reserve1[1];
		index++;
		buffer[index] = dvl->reserve1[2];
		index++;
		buffer[index] = dvl->reserve1[3];
		index++;
		mb_put_binary_int(MB_YES, dvl->datavalidflags, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, dvl->beam1range, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, dvl->beam2range, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, dvl->beam3range, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, dvl->beam4range, &buffer[index]);
		index += 4;
		mb_put_binary_short(MB_YES, dvl->velocitybottomx, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, dvl->velocitybottomy, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, dvl->velocitybottomz, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, dvl->velocitywaterx, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, dvl->velocitywatery, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, dvl->velocitywaterz, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, dvl->depth, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, dvl->pitch, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, dvl->roll, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, dvl->heading, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, dvl->salinity, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, dvl->temperature, &buffer[index]);
		index += 2;
		mb_put_binary_short(MB_YES, dvl->soundspeed, &buffer[index]);
		index += 2;
		for (int i = 0; i < 7; i++) {
			mb_put_binary_short(MB_YES, dvl->reserve2[i], &buffer[index]);
			index += 2;
		}

		/* write the dvl data */
		if ((write_len = fwrite(buffer, 1, dvl->message.size, mb_io_ptr->mbfp)) != dvl->message.size) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
	}

	/* write out NMEA data */
	else if (store->kind == MB_DATA_NMEA_RMC || store->kind == MB_DATA_NMEA_DBT || store->kind == MB_DATA_NMEA_DPT) {
		/* insert the message header values */
		index = 0;
		nmea = (struct mbsys_jstar_nmea_struct *)&(store->nmea);
		mb_put_binary_short(MB_YES, nmea->message.start_marker, &buffer[index]);
		index += 2;
		buffer[index] = nmea->message.version;
		index++;
		buffer[index] = nmea->message.session;
		index++;
		mb_put_binary_short(MB_YES, nmea->message.type, &buffer[index]);
		index += 2;
		buffer[index] = nmea->message.command;
		index++;
		buffer[index] = nmea->message.subsystem;
		index++;
		buffer[index] = nmea->message.channel;
		index++;
		buffer[index] = nmea->message.sequence;
		index++;
		mb_put_binary_short(MB_YES, nmea->message.reserved, &buffer[index]);
		index += 2;
		mb_put_binary_int(MB_YES, nmea->message.size, &buffer[index]);
		index += 4;

		/* write the message header */
		if ((write_len = fwrite(buffer, 1, MBSYS_JSTAR_MESSAGE_SIZE, mb_io_ptr->mbfp)) != MBSYS_JSTAR_MESSAGE_SIZE) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}

		index = 0;
		mb_put_binary_int(MB_YES, nmea->seconds, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, nmea->milliseconds, &buffer[index]);
		index += 4;
		buffer[index] = nmea->source;
		index++;
		buffer[index] = nmea->reserve[0];
		index++;
		buffer[index] = nmea->reserve[1];
		index++;
		buffer[index] = nmea->reserve[2];
		index++;
		for (int i = 0; i < (nmea->message.size - 12); i++) {
			buffer[index] = nmea->nmea[i];
			index++;
		}

		/* write the NMEA data */
		if ((write_len = fwrite(buffer, 1, nmea->message.size, mb_io_ptr->mbfp)) != nmea->message.size) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
	}

	/* write out pressure data */
	else if (store->kind == MB_DATA_CTD) {
		/* insert the message header values */
		index = 0;
		pressure = (struct mbsys_jstar_pressure_struct *)&(store->pressure);
		mb_put_binary_short(MB_YES, pressure->message.start_marker, &buffer[index]);
		index += 2;
		buffer[index] = pressure->message.version;
		index++;
		buffer[index] = pressure->message.session;
		index++;
		mb_put_binary_short(MB_YES, pressure->message.type, &buffer[index]);
		index += 2;
		buffer[index] = pressure->message.command;
		index++;
		buffer[index] = pressure->message.subsystem;
		index++;
		buffer[index] = pressure->message.channel;
		index++;
		buffer[index] = pressure->message.sequence;
		index++;
		mb_put_binary_short(MB_YES, pressure->message.reserved, &buffer[index]);
		index += 2;
		mb_put_binary_int(MB_YES, pressure->message.size, &buffer[index]);
		index += 4;

		/* write the message header */
		if ((write_len = fwrite(buffer, 1, MBSYS_JSTAR_MESSAGE_SIZE, mb_io_ptr->mbfp)) != MBSYS_JSTAR_MESSAGE_SIZE) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}

		index = 0;
		mb_put_binary_int(MB_YES, pressure->seconds, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, pressure->milliseconds, &buffer[index]);
		index += 4;
		buffer[index] = pressure->reserve1[0];
		index++;
		buffer[index] = pressure->reserve1[1];
		index++;
		buffer[index] = pressure->reserve1[2];
		index++;
		buffer[index] = pressure->reserve1[3];
		index++;
		mb_put_binary_int(MB_YES, pressure->pressure, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, pressure->salinity, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, pressure->datavalidflags, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, pressure->conductivity, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, pressure->soundspeed, &buffer[index]);
		index += 4;
		for (int i = 0; i < 10; i++) {
			mb_put_binary_int(MB_YES, pressure->reserve2[i], &buffer[index]);
			index += 4;
		}

		/* write the pressure data */
		if ((write_len = fwrite(buffer, 1, pressure->message.size, mb_io_ptr->mbfp)) != pressure->message.size) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
	}

	/* write out sysinfo data */
	else if (store->kind == MB_DATA_HEADER) {
		/* insert the message header values */
		index = 0;
		sysinfo = (struct mbsys_jstar_sysinfo_struct *)&(store->sysinfo);
		mb_put_binary_short(MB_YES, sysinfo->message.start_marker, &buffer[index]);
		index += 2;
		buffer[index] = sysinfo->message.version;
		index++;
		buffer[index] = sysinfo->message.session;
		index++;
		mb_put_binary_short(MB_YES, sysinfo->message.type, &buffer[index]);
		index += 2;
		buffer[index] = sysinfo->message.command;
		index++;
		buffer[index] = sysinfo->message.subsystem;
		index++;
		buffer[index] = sysinfo->message.channel;
		index++;
		buffer[index] = sysinfo->message.sequence;
		index++;
		mb_put_binary_short(MB_YES, sysinfo->message.reserved, &buffer[index]);
		index += 2;
		mb_put_binary_int(MB_YES, sysinfo->message.size, &buffer[index]);
		index += 4;

		/* write the message header */
		if ((write_len = fwrite(buffer, 1, MBSYS_JSTAR_MESSAGE_SIZE, mb_io_ptr->mbfp)) != MBSYS_JSTAR_MESSAGE_SIZE) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}

		index = 0;
		mb_put_binary_int(MB_YES, sysinfo->system_type, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, sysinfo->reserved1, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, sysinfo->version, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, sysinfo->reserved2, &buffer[index]);
		index += 4;
		mb_put_binary_int(MB_YES, sysinfo->platformserialnumber, &buffer[index]);
		index += 4;
		for (int i = 0; i < sysinfo->sysinfosize; i++) {
			buffer[index] = sysinfo->sysinfo[i];
		}

		/* write the sysinfo data */
		if ((write_len = fwrite(buffer, 1, sysinfo->message.size, mb_io_ptr->mbfp)) != sysinfo->message.size) {
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
		}
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mbr_register_edgjstar(int verbose, void *mbio_ptr, int *error) {
	char *function_name = "mbr_register_edgjstar";

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
	}

	/* get mb_io_ptr */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* set format info parameters */
	const int status = mbr_info_edgjstar(
	    verbose, &mb_io_ptr->system, &mb_io_ptr->beams_bath_max, &mb_io_ptr->beams_amp_max, &mb_io_ptr->pixels_ss_max,
	    mb_io_ptr->format_name, mb_io_ptr->system_name, mb_io_ptr->format_description, &mb_io_ptr->numfile, &mb_io_ptr->filetype,
	    &mb_io_ptr->variable_beams, &mb_io_ptr->traveltime, &mb_io_ptr->beam_flagging, &mb_io_ptr->platform_source,
	    &mb_io_ptr->nav_source, &mb_io_ptr->sensordepth_source, &mb_io_ptr->heading_source, &mb_io_ptr->attitude_source,
	    &mb_io_ptr->svp_source, &mb_io_ptr->beamwidth_xtrack, &mb_io_ptr->beamwidth_ltrack, error);

	/* set format and system specific function pointers */
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_edgjstar;
	mb_io_ptr->mb_io_format_free = &mbr_dem_edgjstar;
	mb_io_ptr->mb_io_store_alloc = &mbsys_jstar_alloc;
	mb_io_ptr->mb_io_store_free = &mbsys_jstar_deall;
	mb_io_ptr->mb_io_read_ping = &mbr_rt_edgjstar;
	mb_io_ptr->mb_io_write_ping = &mbr_wt_edgjstar;
	mb_io_ptr->mb_io_dimensions = &mbsys_jstar_dimensions;
	mb_io_ptr->mb_io_pingnumber = &mbsys_jstar_pingnumber;
	mb_io_ptr->mb_io_preprocess = &mbsys_jstar_preprocess;
	mb_io_ptr->mb_io_extract = &mbsys_jstar_extract;
	mb_io_ptr->mb_io_insert = &mbsys_jstar_insert;
	mb_io_ptr->mb_io_extract_nav = &mbsys_jstar_extract_nav;
	mb_io_ptr->mb_io_insert_nav = &mbsys_jstar_insert_nav;
	mb_io_ptr->mb_io_extract_altitude = &mbsys_jstar_extract_altitude;
	mb_io_ptr->mb_io_insert_altitude = &mbsys_jstar_insert_altitude;
	mb_io_ptr->mb_io_extract_svp = NULL;
	mb_io_ptr->mb_io_insert_svp = NULL;
	mb_io_ptr->mb_io_ttimes = &mbsys_jstar_ttimes;
	mb_io_ptr->mb_io_detects = &mbsys_jstar_detects;
	mb_io_ptr->mb_io_extract_rawssdimensions = &mbsys_jstar_extract_rawssdimensions;
	mb_io_ptr->mb_io_extract_rawss = &mbsys_jstar_extract_rawss;
	mb_io_ptr->mb_io_insert_rawss = &mbsys_jstar_insert_rawss;
	mb_io_ptr->mb_io_extract_segytraceheader = &mbsys_jstar_extract_segytraceheader;
	mb_io_ptr->mb_io_extract_segy = &mbsys_jstar_extract_segy;
	mb_io_ptr->mb_io_insert_segy = &mbsys_jstar_insert_segy;
	mb_io_ptr->mb_io_ctd = &mbsys_jstar_ctd;
	mb_io_ptr->mb_io_copyrecord = &mbsys_jstar_copyrecord;

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       system:             %d\n", mb_io_ptr->system);
		fprintf(stderr, "dbg2       beams_bath_max:     %d\n", mb_io_ptr->beams_bath_max);
		fprintf(stderr, "dbg2       beams_amp_max:      %d\n", mb_io_ptr->beams_amp_max);
		fprintf(stderr, "dbg2       pixels_ss_max:      %d\n", mb_io_ptr->pixels_ss_max);
		fprintf(stderr, "dbg2       format_name:        %s\n", mb_io_ptr->format_name);
		fprintf(stderr, "dbg2       system_name:        %s\n", mb_io_ptr->system_name);
		fprintf(stderr, "dbg2       format_description: %s\n", mb_io_ptr->format_description);
		fprintf(stderr, "dbg2       numfile:            %d\n", mb_io_ptr->numfile);
		fprintf(stderr, "dbg2       filetype:           %d\n", mb_io_ptr->filetype);
		fprintf(stderr, "dbg2       variable_beams:     %d\n", mb_io_ptr->variable_beams);
		fprintf(stderr, "dbg2       traveltime:         %d\n", mb_io_ptr->traveltime);
		fprintf(stderr, "dbg2       beam_flagging:      %d\n", mb_io_ptr->beam_flagging);
		fprintf(stderr, "dbg2       platform_source:    %d\n", mb_io_ptr->platform_source);
		fprintf(stderr, "dbg2       nav_source:         %d\n", mb_io_ptr->nav_source);
		fprintf(stderr, "dbg2       sensordepth_source: %d\n", mb_io_ptr->nav_source);
		fprintf(stderr, "dbg2       heading_source:     %d\n", mb_io_ptr->heading_source);
		fprintf(stderr, "dbg2       attitude_source:    %d\n", mb_io_ptr->attitude_source);
		fprintf(stderr, "dbg2       svp_source:         %d\n", mb_io_ptr->svp_source);
		fprintf(stderr, "dbg2       beamwidth_xtrack:   %f\n", mb_io_ptr->beamwidth_xtrack);
		fprintf(stderr, "dbg2       beamwidth_ltrack:   %f\n", mb_io_ptr->beamwidth_ltrack);
		fprintf(stderr, "dbg2       format_alloc:       %p\n", (void *)mb_io_ptr->mb_io_format_alloc);
		fprintf(stderr, "dbg2       format_free:        %p\n", (void *)mb_io_ptr->mb_io_format_free);
		fprintf(stderr, "dbg2       store_alloc:        %p\n", (void *)mb_io_ptr->mb_io_store_alloc);
		fprintf(stderr, "dbg2       store_free:         %p\n", (void *)mb_io_ptr->mb_io_store_free);
		fprintf(stderr, "dbg2       read_ping:          %p\n", (void *)mb_io_ptr->mb_io_read_ping);
		fprintf(stderr, "dbg2       write_ping:         %p\n", (void *)mb_io_ptr->mb_io_write_ping);
		fprintf(stderr, "dbg2       extract:            %p\n", (void *)mb_io_ptr->mb_io_extract);
		fprintf(stderr, "dbg2       insert:             %p\n", (void *)mb_io_ptr->mb_io_insert);
		fprintf(stderr, "dbg2       extract_nav:        %p\n", (void *)mb_io_ptr->mb_io_extract_nav);
		fprintf(stderr, "dbg2       insert_nav:         %p\n", (void *)mb_io_ptr->mb_io_insert_nav);
		fprintf(stderr, "dbg2       extract_altitude:   %p\n", (void *)mb_io_ptr->mb_io_extract_altitude);
		fprintf(stderr, "dbg2       insert_altitude:    %p\n", (void *)mb_io_ptr->mb_io_insert_altitude);
		fprintf(stderr, "dbg2       extract_svp:        %p\n", (void *)mb_io_ptr->mb_io_extract_svp);
		fprintf(stderr, "dbg2       insert_svp:         %p\n", (void *)mb_io_ptr->mb_io_insert_svp);
		fprintf(stderr, "dbg2       ttimes:             %p\n", (void *)mb_io_ptr->mb_io_ttimes);
		fprintf(stderr, "dbg2       detects:            %p\n", (void *)mb_io_ptr->mb_io_detects);
		fprintf(stderr, "dbg2       extract_rawssdimensions: %p\n", (void *)mb_io_ptr->mb_io_extract_rawssdimensions);
		fprintf(stderr, "dbg2       extract_rawss:      %p\n", (void *)mb_io_ptr->mb_io_extract_rawss);
		fprintf(stderr, "dbg2       insert_rawss:       %p\n", (void *)mb_io_ptr->mb_io_insert_rawss);
		fprintf(stderr, "dbg2       extract_segytraceheader: %p\n", (void *)mb_io_ptr->mb_io_extract_segytraceheader);
		fprintf(stderr, "dbg2       extract_segy:       %p\n", (void *)mb_io_ptr->mb_io_extract_segy);
		fprintf(stderr, "dbg2       insert_segy:        %p\n", (void *)mb_io_ptr->mb_io_insert_segy);
		fprintf(stderr, "dbg2       ctd:                %p\n", (void *)mb_io_ptr->mb_io_ctd);
		fprintf(stderr, "dbg2       copyrecord:         %p\n", (void *)mb_io_ptr->mb_io_copyrecord);
		fprintf(stderr, "dbg2       error:              %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:         %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_register_edgjstr2(int verbose, void *mbio_ptr, int *error) {
	char *function_name = "mbr_register_edgjstr2";
	int status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
	}

	/* get mb_io_ptr */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* set format info parameters */
	status = mbr_info_edgjstr2(
	    verbose, &mb_io_ptr->system, &mb_io_ptr->beams_bath_max, &mb_io_ptr->beams_amp_max, &mb_io_ptr->pixels_ss_max,
	    mb_io_ptr->format_name, mb_io_ptr->system_name, mb_io_ptr->format_description, &mb_io_ptr->numfile, &mb_io_ptr->filetype,
	    &mb_io_ptr->variable_beams, &mb_io_ptr->traveltime, &mb_io_ptr->beam_flagging, &mb_io_ptr->platform_source,
	    &mb_io_ptr->nav_source, &mb_io_ptr->sensordepth_source, &mb_io_ptr->heading_source, &mb_io_ptr->attitude_source,
	    &mb_io_ptr->svp_source, &mb_io_ptr->beamwidth_xtrack, &mb_io_ptr->beamwidth_ltrack, error);

	/* set format and system specific function pointers */
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_edgjstar;
	mb_io_ptr->mb_io_format_free = &mbr_dem_edgjstar;
	mb_io_ptr->mb_io_store_alloc = &mbsys_jstar_alloc;
	mb_io_ptr->mb_io_store_free = &mbsys_jstar_deall;
	mb_io_ptr->mb_io_read_ping = &mbr_rt_edgjstar;
	mb_io_ptr->mb_io_write_ping = &mbr_wt_edgjstar;
	mb_io_ptr->mb_io_dimensions = &mbsys_jstar_dimensions;
	mb_io_ptr->mb_io_pingnumber = &mbsys_jstar_pingnumber;
	mb_io_ptr->mb_io_preprocess = &mbsys_jstar_preprocess;
	mb_io_ptr->mb_io_extract = &mbsys_jstar_extract;
	mb_io_ptr->mb_io_insert = &mbsys_jstar_insert;
	mb_io_ptr->mb_io_extract_nav = &mbsys_jstar_extract_nav;
	mb_io_ptr->mb_io_insert_nav = &mbsys_jstar_insert_nav;
	mb_io_ptr->mb_io_extract_altitude = &mbsys_jstar_extract_altitude;
	mb_io_ptr->mb_io_insert_altitude = NULL;
	mb_io_ptr->mb_io_extract_svp = NULL;
	mb_io_ptr->mb_io_insert_svp = NULL;
	mb_io_ptr->mb_io_ttimes = &mbsys_jstar_ttimes;
	mb_io_ptr->mb_io_detects = &mbsys_jstar_detects;
	mb_io_ptr->mb_io_extract_rawssdimensions = &mbsys_jstar_extract_rawssdimensions;
	mb_io_ptr->mb_io_extract_rawss = &mbsys_jstar_extract_rawss;
	mb_io_ptr->mb_io_insert_rawss = &mbsys_jstar_insert_rawss;
	mb_io_ptr->mb_io_extract_segytraceheader = &mbsys_jstar_extract_segytraceheader;
	mb_io_ptr->mb_io_extract_segy = &mbsys_jstar_extract_segy;
	mb_io_ptr->mb_io_insert_segy = &mbsys_jstar_insert_segy;
	mb_io_ptr->mb_io_ctd = &mbsys_jstar_ctd;
	mb_io_ptr->mb_io_copyrecord = &mbsys_jstar_copyrecord;

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       system:             %d\n", mb_io_ptr->system);
		fprintf(stderr, "dbg2       beams_bath_max:     %d\n", mb_io_ptr->beams_bath_max);
		fprintf(stderr, "dbg2       beams_amp_max:      %d\n", mb_io_ptr->beams_amp_max);
		fprintf(stderr, "dbg2       pixels_ss_max:      %d\n", mb_io_ptr->pixels_ss_max);
		fprintf(stderr, "dbg2       format_name:        %s\n", mb_io_ptr->format_name);
		fprintf(stderr, "dbg2       system_name:        %s\n", mb_io_ptr->system_name);
		fprintf(stderr, "dbg2       format_description: %s\n", mb_io_ptr->format_description);
		fprintf(stderr, "dbg2       numfile:            %d\n", mb_io_ptr->numfile);
		fprintf(stderr, "dbg2       filetype:           %d\n", mb_io_ptr->filetype);
		fprintf(stderr, "dbg2       variable_beams:     %d\n", mb_io_ptr->variable_beams);
		fprintf(stderr, "dbg2       traveltime:         %d\n", mb_io_ptr->traveltime);
		fprintf(stderr, "dbg2       beam_flagging:      %d\n", mb_io_ptr->beam_flagging);
		fprintf(stderr, "dbg2       platform_source:    %d\n", mb_io_ptr->platform_source);
		fprintf(stderr, "dbg2       nav_source:         %d\n", mb_io_ptr->nav_source);
		fprintf(stderr, "dbg2       sensordepth_source: %d\n", mb_io_ptr->nav_source);
		fprintf(stderr, "dbg2       heading_source:     %d\n", mb_io_ptr->heading_source);
		fprintf(stderr, "dbg2       attitude_source:    %d\n", mb_io_ptr->attitude_source);
		fprintf(stderr, "dbg2       svp_source:         %d\n", mb_io_ptr->svp_source);
		fprintf(stderr, "dbg2       beamwidth_xtrack:   %f\n", mb_io_ptr->beamwidth_xtrack);
		fprintf(stderr, "dbg2       beamwidth_ltrack:   %f\n", mb_io_ptr->beamwidth_ltrack);
		fprintf(stderr, "dbg2       format_alloc:       %p\n", (void *)mb_io_ptr->mb_io_format_alloc);
		fprintf(stderr, "dbg2       format_free:        %p\n", (void *)mb_io_ptr->mb_io_format_free);
		fprintf(stderr, "dbg2       store_alloc:        %p\n", (void *)mb_io_ptr->mb_io_store_alloc);
		fprintf(stderr, "dbg2       store_free:         %p\n", (void *)mb_io_ptr->mb_io_store_free);
		fprintf(stderr, "dbg2       read_ping:          %p\n", (void *)mb_io_ptr->mb_io_read_ping);
		fprintf(stderr, "dbg2       write_ping:         %p\n", (void *)mb_io_ptr->mb_io_write_ping);
		fprintf(stderr, "dbg2       extract:            %p\n", (void *)mb_io_ptr->mb_io_extract);
		fprintf(stderr, "dbg2       insert:             %p\n", (void *)mb_io_ptr->mb_io_insert);
		fprintf(stderr, "dbg2       extract_nav:        %p\n", (void *)mb_io_ptr->mb_io_extract_nav);
		fprintf(stderr, "dbg2       insert_nav:         %p\n", (void *)mb_io_ptr->mb_io_insert_nav);
		fprintf(stderr, "dbg2       extract_altitude:   %p\n", (void *)mb_io_ptr->mb_io_extract_altitude);
		fprintf(stderr, "dbg2       insert_altitude:    %p\n", (void *)mb_io_ptr->mb_io_insert_altitude);
		fprintf(stderr, "dbg2       extract_svp:        %p\n", (void *)mb_io_ptr->mb_io_extract_svp);
		fprintf(stderr, "dbg2       insert_svp:         %p\n", (void *)mb_io_ptr->mb_io_insert_svp);
		fprintf(stderr, "dbg2       ttimes:             %p\n", (void *)mb_io_ptr->mb_io_ttimes);
		fprintf(stderr, "dbg2       detects:            %p\n", (void *)mb_io_ptr->mb_io_detects);
		fprintf(stderr, "dbg2       extract_rawssdimensions: %p\n", (void *)mb_io_ptr->mb_io_extract_rawssdimensions);
		fprintf(stderr, "dbg2       extract_rawss:      %p\n", (void *)mb_io_ptr->mb_io_extract_rawss);
		fprintf(stderr, "dbg2       insert_rawss:       %p\n", (void *)mb_io_ptr->mb_io_insert_rawss);
		fprintf(stderr, "dbg2       extract_segytraceheader: %p\n", (void *)mb_io_ptr->mb_io_extract_segytraceheader);
		fprintf(stderr, "dbg2       extract_segy:       %p\n", (void *)mb_io_ptr->mb_io_extract_segy);
		fprintf(stderr, "dbg2       insert_segy:        %p\n", (void *)mb_io_ptr->mb_io_insert_segy);
		fprintf(stderr, "dbg2       ctd:                %p\n", (void *)mb_io_ptr->mb_io_ctd);
		fprintf(stderr, "dbg2       copyrecord:         %p\n", (void *)mb_io_ptr->mb_io_copyrecord);
		fprintf(stderr, "dbg2       error:              %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:         %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
