/********************************************************************
 *
 * Module Name : DUMP_GSF
 *
 * Author/Date : J. S. Byrne / 12 May 1994
 *
 * Description : This file contains the source code for the dump_gsf
 *                test program.  This program was written during the
 *                development of the gsf library as a test program.  It
 *                can be used to view the contents of a gsf swath bathymetry
 *                ping record in text form.
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 * who  when      what
 * ---  ----      ----
 *
 *
 * Classification : Unclassified
 *
 * References : DoDBL Generic Sensor Format Sept. 30, 1993
 *
 * Copyright (C) Science Applications International Corp.
 ********************************************************************/

/* Get required standard c include files */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Get specific includes */
#include "gsf.h"

/* global external data required by this module */
extern int gsfError;

/* static global data for this module */
static gsfRecords gsfRec;
static gsfDataID id;
static int shortOutput;
static int pingTimeOutput;

/* function prototypes for this module */
static void printMBPing(int rec_number);

/********************************************************************
 *
 * Function Name : main
 *
 * Description : This is the main function of the dump_gsf program.
 *    This function receive the file name to open as a command line
 *    argument, opens the file, and then reads each record from the
 *    file one at a time.  Ping records are dumped in text form to the
 *    stdout device.
 *
 * Inputs :
 *    argc = an integer argument counter
 *    argv = a pointer to an array of character string command line arguments
 *
 * Returns : This function (program) returns a zero if successful, or a non
 *  zero value if unsuccessful to the invokation environment.
 *
 * Error Conditions :
 *
 ********************************************************************/

int main(int argc, char *argv[]) {
	char gsfFileName[128];
	char str[64];
	char *ptr;
	int gsfHandle;
	int bytes;
	int num_header = 0;
	int num_svp = 0;
	int num_pparam = 0;
	int num_sparam = 0;
	int num_comment = 0;
	int num_ping = 0;
	int num_history = 0;
	int num_nav_error = 0;
	int num_ping_sum = 0;
	int record_number = 0;
	int i;
	int val;
	struct tm *t;
	struct tm st;
	time_t StartTime = 0;

	/* check the command line arguments */
	if (argc < 3) {
		fprintf(stderr, "Usage: %s [-s] -f <gsf filename> [-pt] [-t mm/dd/yy hh:mm:ss]\n", argv[0]);
		fprintf(stderr, "-s: short output a page at a time\n");
		fprintf(stderr, "-f: for specifying the input file\n");
		fprintf(stderr, "-pt: short output showing only ping times, all pings printed to stdout\n");
		fprintf(stderr, "-t: for specifying start time\n");
		exit(0);
	}

	for (i = 1; i < argc; i++) {
		if ((strcmp(argv[i], "-f") == 0) && (i + 1 <= argc)) {
			sscanf(argv[i + 1], "%s", gsfFileName);
			break;
		}
	}

	for (i = 1; i < argc; i++) {
		if ((strcmp(argv[i], "-s") == 0) && (i + 1 <= argc)) {
			shortOutput = 1;
			break;
		}
	}

	for (i = 1; i < argc; i++) {
		if ((strcmp(argv[i], "-pt") == 0) && (i + 1 <= argc)) {
			pingTimeOutput = 1;
			shortOutput = 1;
			break;
		}
	}

	for (i = 1; i < argc; i++) {
		if ((strcmp(argv[i], "-t") == 0) && (i + 1 <= argc)) {
			/* Don't want to use snptime since this code is distributed */
			memset(&st, 0, sizeof(st));
			sscanf(argv[i + 1], "%d/%d/%d %d:%d:%d", &st.tm_mon, &st.tm_mday, &st.tm_year, &st.tm_hour, &st.tm_min, &st.tm_sec);
			if (st.tm_year < 69)
				st.tm_year += 100; /* Y2K mapping */
			st.tm_mon -= 1;
			putenv("TZ=GMT");
			tzset();
			StartTime = mktime(&st);
		}
	}

	/* Try to open the specified file */
	if (gsfOpen(gsfFileName, GSF_READONLY_INDEX, &gsfHandle)) {
		gsfPrintError(stderr);
		exit(1);
	}

	/* Reset the file pointer to the beginning of the file */
	if (gsfSeek(gsfHandle, GSF_REWIND)) {
		gsfPrintError(stderr);
		exit(1);
	}

	/* force the timezone to gmt */
	putenv("TZ=GMT");
	tzset();

	for (;;) {
		/* we want the next record, no matter what it is */
		bytes = gsfRead(gsfHandle, GSF_NEXT_RECORD, &id, &gsfRec, NULL, 0);
		if (bytes < 0) {
			if (gsfError == GSF_READ_TO_END_OF_FILE) {
				fprintf(stderr, "Finished processing input file: %s\n", gsfFileName);
				exit(0);
			}
			else {
				gsfPrintError(stderr);
			}
		}
		else if (bytes == 0) {
			fprintf(stderr, "Read to end of file: %s\n", gsfFileName);
			exit(0);
		}

		/* Window on time if we received a start time */
		if (gsfRec.mb_ping.ping_time.tv_sec < StartTime) {
			continue;
		}

		record_number++;
		if (((record_number % 20) == 0) && (!pingTimeOutput)) {
			fprintf(stdout, "Press return to continue, q to quit\n");
			val = fgetc(stdin);
			if (val == 'q') {
				return (0);
			}
		}
		switch (id.recordID) {
		case (GSF_RECORD_HEADER):
			num_header++;
			fprintf(stdout, "%05d - gsf header - %s\n", record_number, gsfRec.header.version);
			break;

		case (GSF_RECORD_SWATH_BATHYMETRY_PING):
			num_ping++;
			if (shortOutput) {
				t = gmtime(&gsfRec.mb_ping.ping_time.tv_sec);
				ptr = str + strftime(str, sizeof(str), " %Y/%j %H:%M:%S", t);
				ptr += sprintf(ptr, ".%03d", (int)(gsfRec.mb_ping.ping_time.tv_nsec / 1e6));
				ptr += sprintf(ptr, "%+11.6f %+11.6f", gsfRec.mb_ping.latitude, gsfRec.mb_ping.longitude);
				fprintf(stdout, "%05d - Ping at: %s\n", record_number, str);
			}
			else {
				printMBPing(record_number);
			}
			if (pingTimeOutput) {
				continue;
			}
			break;

		case (GSF_RECORD_SOUND_VELOCITY_PROFILE):
			num_svp++;
			t = gmtime(&gsfRec.svp.application_time.tv_sec);
			ptr = str + strftime(str, sizeof(str), " %Y/%j %H:%M:%S", t);
			ptr += sprintf(ptr, ".%02d", (int)(gsfRec.svp.application_time.tv_nsec / 1e7));
			fprintf(stdout, "%05d - gsf SVP at: %s\n", record_number, str);
			break;

		case (GSF_RECORD_PROCESSING_PARAMETERS):
			num_pparam++;
			t = gmtime(&gsfRec.process_parameters.param_time.tv_sec);
			ptr = str + strftime(str, sizeof(str), " %Y/%j %H:%M:%S", t);
			ptr += sprintf(ptr, ".%02d", (int)(gsfRec.process_parameters.param_time.tv_nsec / 1e7));
			fprintf(stdout, "%05d - gsf Processing Parameters at: %s\n", record_number, str);
			break;

		case (GSF_RECORD_SENSOR_PARAMETERS):
			num_sparam++;
			t = gmtime(&gsfRec.sensor_parameters.param_time.tv_sec);
			ptr = str + strftime(str, sizeof(str), " %Y/%j %H:%M:%S", t);
			ptr += sprintf(ptr, ".%02d", (int)(gsfRec.sensor_parameters.param_time.tv_nsec / 1e7));
			fprintf(stdout, "%05d - gsf Sensor Parameters at: %s\n", record_number, str);
			break;

		case (GSF_RECORD_COMMENT):
			num_comment++;
			t = gmtime(&gsfRec.comment.comment_time.tv_sec);
			ptr = str + strftime(str, sizeof(str), " %Y/%j %H:%M:%S", t);
			ptr += sprintf(ptr, ".%02d", (int)(gsfRec.comment.comment_time.tv_nsec / 1e7));
			fprintf(stdout, "%05d - gsf Comment at: %s\n", record_number, str);
			break;

		case (GSF_RECORD_HISTORY):
			num_history++;
			t = gmtime(&gsfRec.history.history_time.tv_sec);
			ptr = str + strftime(str, sizeof(str), " %Y/%j %H:%M:%S", t);
			ptr += sprintf(ptr, ".%02d", (int)(gsfRec.history.history_time.tv_nsec / 1e7));
			fprintf(stdout, "%05d - gsf History at: %s\n", record_number, str);
			break;

		case (GSF_RECORD_NAVIGATION_ERROR):
			num_nav_error++;
			t = gmtime(&gsfRec.nav_error.nav_error_time.tv_sec);
			ptr = str + strftime(str, sizeof(str), " %Y/%j %H:%M:%S", t);
			ptr += sprintf(ptr, ".%02d", (int)(gsfRec.nav_error.nav_error_time.tv_nsec / 1e7));
			fprintf(stdout, "%05d - gsf Navigation Error - %s\n", record_number, str);
			break;

		case (GSF_RECORD_SWATH_BATHY_SUMMARY):
			num_ping_sum++;
			fprintf(stdout, "%05d - gsf Ping Summary \n", record_number);
			break;

		default:
			break;
		}
	}
	return (0);
}

/********************************************************************
 *
 * Function Name : printMBPing
 *
 * Description : This function prints the contents of a swath bathymetry
 *  ping structure to the stdout device.
 *
 * Inputs : rec_number = the file record number for this ping.
 *
 * Returns : none
 *
 * Error Conditions : none
 *
 ********************************************************************/
static void printMBPing(int rec_number) {

	char str[512];
	char tstr[512];
	char *ptr;
	int i, j;
	int ret;
	int line;
	struct tm *t;
	unsigned long max_intensity_sample;

	fprintf(stdout, "%05d GSF MB Ping:\n", rec_number);
	t = gmtime(&gsfRec.mb_ping.ping_time.tv_sec);
	ptr = str + strftime(str, sizeof(str), " %Y/%j %H:%M:%S", t);
	ptr += sprintf(ptr, ".%02d", (int)(gsfRec.mb_ping.ping_time.tv_nsec / 1e7));
	ptr += sprintf(ptr, "%+11.6f %+11.6f", gsfRec.mb_ping.latitude, gsfRec.mb_ping.longitude);
	fprintf(stdout, "                  %s\n", str);
	fprintf(stdout, "          heading: %06.2f course: %06.2f speed: %05.2f\n", gsfRec.mb_ping.heading, gsfRec.mb_ping.course,
	        gsfRec.mb_ping.speed);
	fprintf(stdout, "           sensor: %d beams: %d center: %d r: %+06.2f p: %+06.2f h: %+06.2f\n", gsfRec.mb_ping.sensor_id,
	        gsfRec.mb_ping.number_beams, gsfRec.mb_ping.center_beam, gsfRec.mb_ping.roll, gsfRec.mb_ping.pitch,
	        gsfRec.mb_ping.heave);

	/* build a header for the arrays */
	sprintf(str, "   Beam");
	if (gsfRec.mb_ping.depth != NULL) {
		// sprintf(str, "%s   Depth", str);
		strncat(str, "   Depth", (512 - strlen(str) - 1));
	}
	if (gsfRec.mb_ping.across_track != NULL) {
		strncat(str, "  XTrack", (512 - strlen(str) - 1));
	}
	if (gsfRec.mb_ping.along_track != NULL) {
		strncat(str, "  ATrack", (512 - strlen(str) - 1));
	}
	if (gsfRec.mb_ping.travel_time != NULL) {
		strncat(str, "   TTime", (512 - strlen(str) - 1));
	}
	if (gsfRec.mb_ping.beam_angle != NULL) {
		strncat(str, "   Angle", (512 - strlen(str) - 1));
	}
	if (gsfRec.mb_ping.beam_angle_forward != NULL) {
		strncat(str, " Ang Fwd", (512 - strlen(str) - 1));
	}
	if (gsfRec.mb_ping.mc_amplitude != NULL) {
		strncat(str, " Cal Amp", (512 - strlen(str) - 1));
	}
	if (gsfRec.mb_ping.mr_amplitude != NULL) {
		strncat(str, " Rel Amp", (512 - strlen(str) - 1));
	}
	if (gsfRec.mb_ping.echo_width != NULL) {
		strncat(str, "   Width", (512 - strlen(str) - 1));
	}
	if (gsfRec.mb_ping.quality_factor != NULL) {
		strncat(str, "  Qualit", (512 - strlen(str) - 1));
	}
	if (gsfRec.mb_ping.receive_heave != NULL) {
		strncat(str, "   Heave", (512 - strlen(str) - 1));
	}
	if (gsfRec.mb_ping.brb_inten != NULL) {
		strncat(str, " Samples", (512 - strlen(str) - 1)); /* number of intensity samples in the beam */
		strncat(str, " BotSmpl", (512 - strlen(str) - 1)); /* index to the bottom detect sample */
		strncat(str, " MaxInt.", (512 - strlen(str) - 1)); /* max intensity value for the beam */
	}
	if (gsfRec.mb_ping.quality_flags != NULL) {
		strncat(str, " Q Flags", (512 - strlen(str) - 1));
	}
	if (gsfRec.mb_ping.beam_flags != NULL) {
		strncat(str, " B Flags", (512 - strlen(str) - 1));
	}
	fprintf(stdout, "%s\n", str);

	for (i = 0, line = 0; i < gsfRec.mb_ping.number_beams; i++, line++) {
		sprintf(str, "    %03d", i + 1);
		if (gsfRec.mb_ping.depth != NULL) {
			if (gsfRec.mb_ping.depth[i] < 100.0) {
				sprintf(tstr, " %07.2f", gsfRec.mb_ping.depth[i]);
				strncat(str, tstr, (512 - strlen(str) - 1));
			}
			else {
				sprintf(tstr, " %07.1f", gsfRec.mb_ping.depth[i]);
				strncat(str, tstr, (512 - strlen(str) - 1));
			}
		}
		if (gsfRec.mb_ping.across_track != NULL) {
			sprintf(tstr, " %+07.1f", gsfRec.mb_ping.across_track[i]);
			strncat(str, tstr, (512 - strlen(str) - 1));
		}
		if (gsfRec.mb_ping.along_track != NULL) {
			sprintf(tstr, " %+07.1f", gsfRec.mb_ping.along_track[i]);
			strncat(str, tstr, (512 - strlen(str) - 1));
		}
		if (gsfRec.mb_ping.travel_time != NULL) {
			sprintf(tstr, " %07.5f", gsfRec.mb_ping.travel_time[i]);
			strncat(str, tstr, (512 - strlen(str) - 1));
		}
		if (gsfRec.mb_ping.beam_angle != NULL) {
			sprintf(tstr, " %07.1f", gsfRec.mb_ping.beam_angle[i]);
			strncat(str, tstr, (512 - strlen(str) - 1));
		}
		if (gsfRec.mb_ping.beam_angle_forward != NULL) {
			sprintf(tstr, " %07.1f", gsfRec.mb_ping.beam_angle_forward[i]);
			strncat(str, tstr, (512 - strlen(str) - 1));
		}
		if (gsfRec.mb_ping.mc_amplitude != NULL) {
			sprintf(tstr, " %07.1f", gsfRec.mb_ping.mc_amplitude[i]);
			strncat(str, tstr, (512 - strlen(str) - 1));
		}
		if (gsfRec.mb_ping.mr_amplitude != NULL) {
			sprintf(tstr, " %07.1f", gsfRec.mb_ping.mr_amplitude[i]);
			strncat(str, tstr, (512 - strlen(str) - 1));
		}
		if (gsfRec.mb_ping.echo_width != NULL) {
			sprintf(tstr, " %07.1f", gsfRec.mb_ping.echo_width[i]);
			strncat(str, tstr, (512 - strlen(str) - 1));
		}
		if (gsfRec.mb_ping.quality_factor != NULL) {
			sprintf(tstr, " %07.1f", gsfRec.mb_ping.quality_factor[i]);
			strncat(str, tstr, (512 - strlen(str) - 1));
		}
		if (gsfRec.mb_ping.receive_heave != NULL) {
			sprintf(tstr, " %0.7f", gsfRec.mb_ping.receive_heave[i]);
			strncat(str, tstr, (512 - strlen(str) - 1));
		}
		if (gsfRec.mb_ping.brb_inten != NULL) {
			sprintf(tstr, " %7d", gsfRec.mb_ping.brb_inten->time_series[i].sample_count);
			strncat(str, tstr, (512 - strlen(str) - 1));
			sprintf(tstr, " %7d", gsfRec.mb_ping.brb_inten->time_series[i].detect_sample);
			strncat(str, tstr, (512 - strlen(str) - 1));
			max_intensity_sample = 0;
			for (j = 0; j < gsfRec.mb_ping.brb_inten->time_series[i].sample_count; j++) {
				if (gsfRec.mb_ping.brb_inten->time_series[i].samples[j] > max_intensity_sample) {
					max_intensity_sample = gsfRec.mb_ping.brb_inten->time_series[i].samples[j];
				}
			}
			sprintf(tstr, " %07lX", max_intensity_sample);
			strncat(str, tstr, (512 - strlen(str) - 1));
		}
		if (gsfRec.mb_ping.quality_flags != NULL) {
			sprintf(tstr, " %.7d", gsfRec.mb_ping.quality_flags[i]);
			strncat(str, tstr, (512 - strlen(str) - 1));
		}
		if (gsfRec.mb_ping.beam_flags != NULL) {
			sprintf(tstr, " %.7d", gsfRec.mb_ping.beam_flags[i]);
			strncat(str, tstr, (512 - strlen(str) - 1));
		}
		fprintf(stdout, "%s\n", str);
		if (line > 20) {
			line = 0;
			fprintf(stdout, "Press return to continue, q to quit\n");
			ret = fgetc(stdin);
			if (ret == 'q') {
				exit(0);
			}
		}
	}

	return;
}
