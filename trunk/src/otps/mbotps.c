/*--------------------------------------------------------------------
 *    The MB-system:	mbotps.c	7/30/2009
 *    $Id$
 *
 *    Copyright (c) 2009-2017 by
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
 * MBotps predicts tides using methods and data derived
 * from the OSU Tidal Prediction Software (OTPS) distributions
 * distributed from:
 *      http://www.coas.oregonstate.edu/research/po/research/tide/
 * The OTPS distributions include programs written in Fortran 90 that
 * operate in batch mode with specified control files. This program is
 * intended to provide the same tidal prediction capability through a
 * command line interface more consistent with MB-System.
 *
 * The mbotps usage is:
 *     	mbotps [-Atideformat -Byear/month/day/hour/minute/second
 *     	       -Ctidestationformat -Dinterval
 *     	       -Eyear/month/day/hour/minute/second -Fformat -Idatalist
 *     	       -Lopts_path -Ntidestationfile -Ooutput -Potps_location
 *             -Rlon/lat -S -Tmodel -Utidestationlon/tidestationlat -V]
 *
 * This program can be used in two modes. In the first, the user
 * specifies a location (-Rlon/lat), start and end times (-B and -E),
 * and a tidal sampling interval (-D). The program then writes a two
 * column tide time series consisting of epoch time values in seconds followed
 * by tide values in meters for the specified location and times. The
 * output is to a file specified with -Otide_file.
 *
 * In the second mode, the user specifies one or more swath data files using
 * -Idatalist.mb-1. A tide file is generated for each swath file by
 * outputing the time and tide value for the sonar navigation sampled
 * according to -Dinterval. MBotps also sets the parameter file for each
 * swath file so that mbprocess applies the tide model during processing.
 *
 * The -Ctidestationformat, -Ntidestationfile, and  -Utidestationlon/tidestationlat
 * commands together allow users to input observations from a tide station;
 * these observations can be used to calculate corrections to tidal model values
 * in the vicinity of the tide station. If tide station data are specified,
 * then MBotps calculates the difference between the observed and modeled tide
 * at that station for each data point in the input tide station data. This
 * difference time series is then used as a correction to the output tide models,
 * whether at a location specified with the -Rlon/lat option or for swath data
 * specified with the -Idatalist option.

 * Author:	D. W. Caress
 * Date:	July 30,  2009
 * Date:	April 5,  2018
 *
 * $Log: mbotps.c,v $
 *
 */

/* standard include files */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

/* MBIO include files */
#include "mb_define.h"
#include "mb_status.h"
#include "mb_format.h"
#include "mb_process.h"

/* OTPS installation location include */
#include "otps.h"

/* local defines */
#define MBOTPS_MODE_POSITION            0x00
#define MBOTPS_MODE_NAVIGATION          0x01
#define MBOTPS_MODE_TIDESTATION         0x02
#define MBOTPS_MODE_NAV_WRT_STATION     0x03
#define MBOTPS_DEFAULT_MODEL "atlas_v1"

/* system function declarations */
char *ctime();
#ifndef WIN32
char *getenv();
#endif

/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
	static char rcs_id[] = "$Id$";
	static char program_name[] = "mbotps";
	static char help_message[] =
	    "MBotps predicts tides using methods and data derived from the OSU Tidal Prediction Software (OTPS) distributions.";
	static char usage_message[] =
	    "mbotps [-Atideformat -Byear/month/day/hour/minute/second -Ctidestationformat\n"
        "\t-Dinterval -Eyear/month/day/hour/minute/second -Fformat\n"
	    "\t-Idatalist -Lopts_path -Ntidestationfile -Ooutput -Potps_location\n"
        "\t-Rlon/lat -S -Tmodel -Utidestationlon/tidestationlat -V]";
	extern char *optarg;
	int errflg = 0;
	int c;
	int help = 0;
	int flag = 0;

	/* MBIO status variables */
	int status = MB_SUCCESS;
	int verbose = 0;
	int error = MB_ERROR_NO_ERROR;
	char *message;

	/* MBIO read control parameters */
	int read_datalist = MB_NO;
	mb_path read_file;
	void *datalist;
	int look_processed = MB_DATALIST_LOOK_UNSET;
	double file_weight;
	mb_path swath_file;
	mb_path file;
	mb_path dfile;
	int format;
	int pings;
	int lonflip;
	double bounds[4];
	double speedmin;
	double timegap;
	int beams_bath;
	int beams_amp;
	int pixels_ss;

	/* MBIO read values */
	void *mbio_ptr = NULL;
	void *store_ptr = NULL;
	int kind;
	int time_i[7];
	double time_d;
	double navlon;
	double navlat;
	double speed;
	double heading;
	double distance;
	double altitude;
	double sonardepth;
	char *beamflag = NULL;
	double *bath = NULL;
	double *bathacrosstrack = NULL;
	double *bathalongtrack = NULL;
	double *amp = NULL;
	double *ss = NULL;
	double *ssacrosstrack = NULL;
	double *ssalongtrack = NULL;
	char comment[MB_COMMENT_MAXLINE];

	/* mbotps control parameters */
	mb_path otps_location_use;
	int notpsmodels = 0;
	int nmodeldatafiles = 0;
	int mbotps_mode = MBOTPS_MODE_POSITION;
	double tidelon;
	double tidelat;
	double btime_d;
	double etime_d;
	int btime_i[7];
	int etime_i[7];
	double interval = 300.0;
	mb_path tide_file;
	int mbprocess_update = MB_NO;
	int skip_existing = MB_NO;
	int tideformat = 2;
	int ngood;
    
    /* tide station data */
    mb_path tidestation_file;
    double tidestation_lon = 0.0;
    double tidestation_lat = 0.0;
    int tidestation_format = 2;
    int tidestation_ok = MB_NO;
    int ntidestation = 0;
    double *tidestation_time_d = NULL;
    double *tidestation_tide = NULL;
    double *tidestation_model = NULL;
    double *tidestation_correction = NULL;
	int time_j[5], ihr, intstat, itime;
    int size;
	double sec, correction;

	/* time parameters */
	time_t right_now;
	char date[32], user[MB_PATH_MAXLINE], *user_ptr, host[MB_PATH_MAXLINE];
	int pid;
	
	FILE *tfp, *mfp, *ofp;
	struct stat file_status;
	int fstat;
	double start_time_d;
	double end_time_d;
	int istart, iend;
	int proceed = MB_YES;
	int input_size, input_modtime, output_size, output_modtime;
	mb_path lltfile = "";
	mb_path otpsfile = "";
	mb_path line = "";
	mb_path predict_tide = "";
	int otps_model_set = MB_NO;
	mb_path otps_model = "";
	mb_path modelname = "";
	mb_path modelfile = "";
	mb_path modeldatafile = "";
	int read_data;
	int ntime;
	int nread;
	int nline;
	int nget;
	int output;
	double savetime_d;
	double lasttime_d;
	double lastlon;
	double lastlat;
	double lon;
	double lat;
	double tide;
	double depth;
	char *result;
	int i;

	/* get current default values */
	status = mb_defaults(verbose, &format, &pings, &lonflip, bounds, btime_i, etime_i, &speedmin, &timegap);

	/* set default input to datalist.mb-1 */
	strcpy(read_file, "datalist.mb-1");

	/* set default location of the OTPS package */
	strcpy(otps_location_use, otps_location);

	/* set defaults for the AUV survey we were running on Coaxial Segment, Juan de Fuca Ridge
	    while I wrote this code */
	sprintf(otps_model, MBOTPS_DEFAULT_MODEL);
	sprintf(tide_file, "tide_model.txt");
	tidelon = -129.588618;
	tidelat = 46.50459;
	interval = 60.0;
	btime_i[0] = 2009;
	btime_i[1] = 7;
	btime_i[2] = 31;
	btime_i[3] = 0;
	btime_i[4] = 0;
	btime_i[5] = 0;
	btime_i[6] = 0;
	etime_i[0] = 2009;
	etime_i[1] = 8;
	etime_i[2] = 2;
	etime_i[3] = 1;
	etime_i[4] = 0;
	etime_i[5] = 0;
	etime_i[6] = 0;

	/* process argument list */
	while ((c = getopt(argc, argv, "A:a:B:b:D:d:E:e:F:f:I:i:MmO:o:P:p:R:r:SST:t:VvHh")) != -1)
		switch (c) {
		case 'H':
		case 'h':
			help++;
			break;
		case 'V':
		case 'v':
			verbose++;
			break;
		case 'A':
		case 'a':
			sscanf(optarg, "%d", &tideformat);
			if (tideformat != 2)
				tideformat = 1;
			break;
		case 'B':
		case 'b':
			sscanf(optarg, "%d/%d/%d/%d/%d/%d", &btime_i[0], &btime_i[1], &btime_i[2], &btime_i[3], &btime_i[4], &btime_i[5]);
			btime_i[6] = 0;
			flag++;
			break;
		case 'C':
		case 'c':
			sscanf(optarg, "%d", &tidestation_format);
			if (tidestation_format != 2)
				tidestation_format = 1;
			break;
		case 'D':
		case 'd':
			sscanf(optarg, "%lf", &interval);
			break;
		case 'E':
		case 'e':
			sscanf(optarg, "%d/%d/%d/%d/%d/%d", &etime_i[0], &etime_i[1], &etime_i[2], &etime_i[3], &etime_i[4], &etime_i[5]);
			etime_i[6] = 0;
			flag++;
			break;
		case 'F':
		case 'f':
			sscanf(optarg, "%d", &format);
			flag++;
			break;
		case 'I':
		case 'i':
			sscanf(optarg, "%s", read_file);
			mbotps_mode = mbotps_mode | MBOTPS_MODE_NAVIGATION;
			flag++;
			break;
		case 'M':
		case 'm':
			mbprocess_update = MB_YES;
			break;
		case 'N':
		case 'n':
			sscanf(optarg, "%s", tidestation_file);
			mbotps_mode = mbotps_mode | MBOTPS_MODE_TIDESTATION;
			break;
		case 'O':
		case 'o':
			sscanf(optarg, "%s", tide_file);
			break;
		case 'P':
		case 'p':
			sscanf(optarg, "%s", otps_location_use);
			break;
		case 'R':
		case 'r':
			sscanf(optarg, "%lf/%lf", &tidelon, &tidelat);
			break;
		case 'S':
		case 's':
			skip_existing = MB_YES;
			break;
		case 'T':
		case 't':
			sscanf(optarg, "%s", otps_model);
			otps_model_set = MB_YES;
			break;
		case 'U':
		case 'u':
			sscanf(optarg, "%lf/%lf", &tidestation_lon, &tidestation_lat);
			break;
		case '?':
			errflg++;
		}

	/* if error flagged then print it and exit */
	if (errflg) {
		fprintf(stderr, "usage: %s\n", usage_message);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		error = MB_ERROR_BAD_USAGE;
		exit(error);
	}

	/* print starting message */
	if (verbose == 1 || help) {
		fprintf(stderr, "\nProgram %s\n", program_name);
		fprintf(stderr, "Version %s\n", rcs_id);
		fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
	}

	/* if help desired then print it and exit */
	if (help) {
		fprintf(stderr, "\n%s\n", help_message);
		fprintf(stderr, "\nusage: %s\n", usage_message);
	}

	/* Check for available tide models */
	if (help || verbose > 0) {
		fprintf(stderr, "\nChecking for available OTPS tide models\n");
		fprintf(stderr, "OTPS location: %s\nValid OTPS tidal models:\n", otps_location_use);
	}
	notpsmodels = 0;
	sprintf(line, "/bin/ls -1 %s/DATA | grep Model_ | sed \"s/^Model_//\"", otps_location_use);
	if ((tfp = popen(line, "r")) != NULL) {
		/* send relevant input to predict_tide through its stdin stream */
		while (fgets(line, sizeof(line), tfp)) {
			sscanf(line, "%s", modelname);
			sprintf(modelfile, "%s/DATA/Model_%s", otps_location_use, modelname);
			nmodeldatafiles = 0;

			/* check the files referenced in the model file */
			if ((mfp = fopen(modelfile, "r")) != NULL) {
				/* stat the file referenced in each line */
				while (fgets(modeldatafile, MB_PATH_MAXLINE, mfp) != NULL) {
					if (strlen(modeldatafile) > 0)
						modeldatafile[strlen(modeldatafile) - 1] = '\0';
					if ((fstat = stat(modeldatafile, &file_status)) == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
						nmodeldatafiles++;
					}
				}
				fclose(mfp);
			}
			if (nmodeldatafiles >= 3) {
				if (help || verbose > 0)
					fprintf(stderr, "     %s\n", modelname);
				if (otps_model_set == MB_NO) {
					if (notpsmodels == 0 || strcmp(modelname, MBOTPS_DEFAULT_MODEL) == 0)
						strcpy(otps_model, modelname);
				}
				notpsmodels++;
			}
		}

		/* close the process */
		pclose(tfp);
	}
	else {
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr, "\nUnable to open ls using popen()\n");
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		exit(MB_FAILURE);
	}
	if (help || verbose > 0) {
		fprintf(stderr, "Number of available OTPS tide models: %d\n", notpsmodels);
		fprintf(stderr, "\nUsing OTPS tide model:            %s\n", otps_model);
	}

	/* print starting debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Program <%s>\n", program_name);
		fprintf(stderr, "dbg2  Version %s\n", rcs_id);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Control Parameters:\n");
		fprintf(stderr, "dbg2       verbose:              %d\n", verbose);
		fprintf(stderr, "dbg2       help:                 %d\n", help);
		fprintf(stderr, "dbg2       otps_location:        %s\n", otps_location);
		fprintf(stderr, "dbg2       otps_location_use:    %s\n", otps_location_use);
		fprintf(stderr, "dbg2       otps_model_set:       %d\n", otps_model_set);
		fprintf(stderr, "dbg2       otps_model:           %s\n", otps_model);
		fprintf(stderr, "dbg2       mbotps_mode:          %d\n", mbotps_mode);
		fprintf(stderr, "dbg2       tidelon:              %f\n", tidelon);
		fprintf(stderr, "dbg2       tidelat:              %f\n", tidelat);
		fprintf(stderr, "dbg2       tidestation_file:     %s\n", tidestation_file);
		fprintf(stderr, "dbg2       tidestation_lon:       %f\n", tidestation_lon);
		fprintf(stderr, "dbg2       tidestation_lat:       %f\n", tidestation_lat);
		fprintf(stderr, "dbg2       tidestation_format:    %d\n", tidestation_format);
		fprintf(stderr, "dbg2       btime_i[0]:           %d\n", btime_i[0]);
		fprintf(stderr, "dbg2       btime_i[1]:           %d\n", btime_i[1]);
		fprintf(stderr, "dbg2       btime_i[2]:           %d\n", btime_i[2]);
		fprintf(stderr, "dbg2       btime_i[3]:           %d\n", btime_i[3]);
		fprintf(stderr, "dbg2       btime_i[4]:           %d\n", btime_i[4]);
		fprintf(stderr, "dbg2       btime_i[5]:           %d\n", btime_i[5]);
		fprintf(stderr, "dbg2       btime_i[6]:           %d\n", btime_i[6]);
		fprintf(stderr, "dbg2       etime_i[0]:           %d\n", etime_i[0]);
		fprintf(stderr, "dbg2       etime_i[1]:           %d\n", etime_i[1]);
		fprintf(stderr, "dbg2       etime_i[2]:           %d\n", etime_i[2]);
		fprintf(stderr, "dbg2       etime_i[3]:           %d\n", etime_i[3]);
		fprintf(stderr, "dbg2       etime_i[4]:           %d\n", etime_i[4]);
		fprintf(stderr, "dbg2       etime_i[5]:           %d\n", etime_i[5]);
		fprintf(stderr, "dbg2       etime_i[6]:           %d\n", etime_i[6]);
		fprintf(stderr, "dbg2       interval:             %f\n", interval);
		fprintf(stderr, "dbg2       tide_file:            %s\n", tide_file);
		fprintf(stderr, "dbg2       mbprocess_update:     %d\n", mbprocess_update);
		fprintf(stderr, "dbg2       skip_existing:        %d\n", skip_existing);
		fprintf(stderr, "dbg2       tideformat:           %d\n", tideformat);
		fprintf(stderr, "dbg2       format:               %d\n", format);
		fprintf(stderr, "dbg2       read_file:            %s\n", read_file);
	}

	/* exit if no valid OTPS models can be found */
	if (notpsmodels <= 0) {
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr, "\nUnable to find a valid OTPS tidal model\n");
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		exit(MB_FAILURE);
	}

	/* if help was all that was desired then exit */
	if (help) {
		exit(error);
	}
    
    /* -------------------------------------------------------------------------
     * if specified read in tide station data and calculate model values for the
     * same location and times- the difference is applied as a correction to the
     * model values calculated at the desired locations and times
     * -----------------------------------------------------------------------*/
    if (mbotps_mode & MBOTPS_MODE_TIDESTATION) {
 		if ((tfp = fopen(tidestation_file, "r")) == NULL) {
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr, "\nUnable to open tide station file <%s> for writing\n", tidestation_file);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(MB_FAILURE);
		}
        
        /* count the lines in the tide station data */
        ntidestation = 0;
		while ((result = fgets(line, MB_PATH_MAXLINE, tfp)) == line) {
			ntidestation++;
        }
        rewind(tfp);
        
        /* allocate memory for tide station arrays */
        size = ntidestation * sizeof(double);
        status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)&tidestation_time_d, &error);
        if (error == MB_ERROR_NO_ERROR)
            status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)&tidestation_tide, &error);
        if (error == MB_ERROR_NO_ERROR)
            status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)&tidestation_model, &error);
        if (error == MB_ERROR_NO_ERROR)
            status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)&tidestation_correction, &error);
        if (error != MB_ERROR_NO_ERROR) {
            mb_error(verbose, error, &message);
            fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
            fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
            exit(error);
        }

        /* read the tide station data in the specified format */
        ntidestation = 0;
		while ((result = fgets(line, MB_PATH_MAXLINE, tfp)) == line) {
            tidestation_ok = MB_NO;

            /* ignore comments */
            if (line[0] != '#') {

                /* deal with tide station data in form: time_d tide */
                if (tidestation_format == 1) {
                    nget = sscanf(line, "%lf %lf", &tidestation_time_d[ntidestation], &tidestation_tide[ntidestation]);
                    if (nget == 2)
                        tidestation_ok = MB_YES;
                }

                /* deal with tide station data in form: yr mon day hour min sec tide */
                else if (tidestation_format == 2) {
                    nget = sscanf(line, "%d %d %d %d %d %lf %lf", &time_i[0], &time_i[1], &time_i[2], &time_i[3],
                                  &time_i[4], &sec, &tidestation_tide[ntidestation]);
                    time_i[5] = (int)sec;
                    time_i[6] = 1000000 * (sec - time_i[5]);
                    mb_get_time(verbose, time_i, &time_d);
                    tidestation_time_d[ntidestation] = time_d;
                    if (nget == 7)
                        tidestation_ok = MB_YES;
                }

                /* deal with tide station data in form: yr jday hour min sec tide */
                else if (tidestation_format == 3) {
                    nget = sscanf(line, "%d %d %d %d %lf %lf", &time_j[0], &time_j[1], &ihr, &time_j[2], &sec,
                                  &tidestation_tide[ntidestation]);
                    time_j[2] = time_j[2] + 60 * ihr;
                    time_j[3] = (int)sec;
                    time_j[4] = 1000000 * (sec - time_j[3]);
                    mb_get_itime(verbose, time_j, time_i);
                    mb_get_time(verbose, time_i, &time_d);
                    tidestation_time_d[ntidestation] = time_d;
                    if (nget == 6)
                        tidestation_ok = MB_YES;
                }

                /* deal with tide station data in form: yr jday daymin sec tide */
                else if (tidestation_format == 4) {
                    nget = sscanf(line, "%d %d %d %lf %lf",
                                  &time_j[0], &time_j[1], &time_j[2], &sec,
                                  &tidestation_tide[ntidestation]);
                    time_j[3] = (int)sec;
                    time_j[4] = 1000000 * (sec - time_j[3]);
                    mb_get_itime(verbose, time_j, time_i);
                    mb_get_time(verbose, time_i, &time_d);
                    tidestation_time_d[ntidestation] = time_d;
                    if (nget == 5)
                        tidestation_ok = MB_YES;
                }
            }

            /* output some debug values */
            if (verbose >= 5 && tidestation_ok == MB_YES) {
                fprintf(stderr, "\ndbg5  New tide point read in program <%s>\n", program_name);
                fprintf(stderr, "dbg5       tide[%d]: %f %f\n",
                        ntidestation, tidestation_time_d[ntidestation],
                        tidestation_tide[ntidestation]);
            }
            else if (verbose >= 5) {
                fprintf(stderr, "\ndbg5  Error parsing line in tide file in program <%s>\n",
                        program_name);
                fprintf(stderr, "dbg5       line: %s\n", line);
            }

            /* check for reverses or repeats in time */
            if (tidestation_ok == MB_YES) {
                if (ntidestation == 0)
                    ntidestation++;
                else if (tidestation_time_d[ntidestation] > tidestation_time_d[ntidestation - 1])
                    ntidestation++;
                else if (ntidestation > 0
                         && tidestation_time_d[ntidestation] <= tidestation_time_d[ntidestation - 1]
                         && verbose >= 5) {
                    fprintf(stderr, "\ndbg5  Tide time error in program <%s>\n",
                            program_name);
                    fprintf(stderr, "dbg5       tide[%d]: %f %f\n",
                            ntidestation - 1, tidestation_time_d[ntidestation - 1],
                            tidestation_tide[ntidestation - 1]);
                    fprintf(stderr, "dbg5       tide[%d]: %f %f\n",
                            ntidestation, tidestation_time_d[ntidestation],
                            tidestation_tide[ntidestation]);
                }
            }
            strncpy(line, "\0", sizeof(line));
        }
        fclose(tfp);
        
        /* now get time and tide model values at the tide station location */
		/* first open temporary file of lat lon time */
		pid = getpid();
		sprintf(lltfile, "tmp_mbotps_llt_%d.txt", pid);
		sprintf(otpsfile, "tmp_mbotps_llttd_%d.txt", pid);
		if ((tfp = fopen(lltfile, "w")) == NULL) {
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr, "\nUnable to open temporary lat-lon-time file <%s> for writing\n", lltfile);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(MB_FAILURE);
		}

		/* make sure longitude is positive */
		if (tidestation_lon < 0.0)
			tidestation_lon += 360.0;

		/* loop over the times of tide station values to generate the lat-lon-time values */
		for (i = 0; i < ntidestation; i++) {
			time_d = tidestation_time_d[i];
			mb_get_date(verbose, time_d, time_i);
			fprintf(tfp, "%.6f %.6f %4.4d %2.2d %2.2d %2.2d %2.2d %2.2d\n",
                    tidestation_lat, tidestation_lon, time_i[0], time_i[1], time_i[2],
			        time_i[3], time_i[4], time_i[5]);
		}

		/* close the llt file */
		fclose(tfp);

		/* call predict_tide with popen */
		sprintf(predict_tide, "%s/predict_tide", otps_location_use);
		if ((tfp = popen(predict_tide, "w")) == NULL) {
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr, "\nUnable to open predict_time program using popen()\n");
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(MB_FAILURE);
		}

		/* send relevant input to predict_tide through its stdin stream */
		fprintf(tfp, "%s/DATA/Model_%s\n", otps_location_use, otps_model);
		fprintf(tfp, "%s\n", lltfile);
		fprintf(tfp, "z\n\nAP\noce\n1\n");
		/*fprintf(tfp, "z\nm2,s2,n2,k2,k1,o1,p1,q1\nAP\noce\n1\n");*/
		fprintf(tfp, "%s\n", otpsfile);

		/* close the process */
		pclose(tfp);

		/* now read results from predict_tide and rewrite them in a useful form */
		if ((tfp = fopen(otpsfile, "r")) == NULL) {
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr, "\nUnable to open predict_time results temporary file <%s>\n", otpsfile);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(MB_FAILURE);
		}

		nline = 0;
		ngood = 0;
		while ((result = fgets(line, MB_PATH_MAXLINE, tfp)) == line) {
			nline++;
			if (nline > 6) {
				nget = sscanf(line, "%lf %lf %d.%d.%d %d:%d:%d %lf %lf", &lat, &lon, &time_i[1], &time_i[2], &time_i[0],
				              &time_i[3], &time_i[4], &time_i[5], &tide, &depth);
				if (nget == 10) {
                    tidestation_model[ngood] = tide;
                    tidestation_correction[ngood] = tidestation_tide[ngood] - tidestation_model[ngood];
					ngood++;
				}
			}
		}
		fclose(tfp);
        if (ngood != ntidestation) {
			error = MB_ERROR_BAD_FORMAT;
			fprintf(stderr, "\nNumber of tide station values does not match number of model values <%d != %d>\n", ntidestation, ngood);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(MB_FAILURE);
        }

		/* remove the temporary files */
		unlink(lltfile);
		unlink(otpsfile);
    }

	/* -------------------------------------------------------------------------
	 * calculate tide model  for a single position and time range
	 * -----------------------------------------------------------------------*/
	if (mbotps_mode == MBOTPS_MODE_POSITION) {
		/* first open temporary file of lat lon time */
		pid = getpid();
		sprintf(lltfile, "tmp_mbotps_llt_%d.txt", pid);
		sprintf(otpsfile, "tmp_mbotps_llttd_%d.txt", pid);
		if ((tfp = fopen(lltfile, "w")) == NULL) {
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr, "\nUnable to open temporary lat-lon-time file <%s> for writing\n", lltfile);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(MB_FAILURE);
		}

		/* make sure longitude is positive */
		if (tidelon < 0.0)
			tidelon += 360.0;

		/* loop over the time of interest generating the lat-lon-time values */
		mb_get_time(verbose, btime_i, &btime_d);
		mb_get_time(verbose, etime_i, &etime_d);
		ntime = 1 + (int)floor((etime_d - btime_d) / interval);
		for (i = 0; i < ntime; i++) {
			time_d = btime_d + i * interval;
			mb_get_date(verbose, time_d, time_i);
			fprintf(tfp, "%.6f %.6f %4.4d %2.2d %2.2d %2.2d %2.2d %2.2d\n", tidelat, tidelon, time_i[0], time_i[1], time_i[2],
			        time_i[3], time_i[4], time_i[5]);
		}

		/* close the llt file */
		fclose(tfp);

		/* call predict_tide with popen */
		sprintf(predict_tide, "%s/predict_tide", otps_location_use);
		if ((tfp = popen(predict_tide, "w")) == NULL) {
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr, "\nUnable to open predict_time program using popen()\n");
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(MB_FAILURE);
		}

		/* send relevant input to predict_tide through its stdin stream */
		fprintf(tfp, "%s/DATA/Model_%s\n", otps_location_use, otps_model);
		fprintf(tfp, "%s\n", lltfile);
		fprintf(tfp, "z\n\nAP\noce\n1\n");
		/*fprintf(tfp, "z\nm2,s2,n2,k2,k1,o1,p1,q1\nAP\noce\n1\n");*/
		fprintf(tfp, "%s\n", otpsfile);

		/* close the process */
		pclose(tfp);

		/* now read results from predict_tide and rewrite them in a useful form */
		if ((tfp = fopen(otpsfile, "r")) == NULL) {
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr, "\nUnable to open predict_time results temporary file <%s>\n", otpsfile);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(MB_FAILURE);
		}
		if ((ofp = fopen(tide_file, "w")) == NULL) {
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr, "\nUnable to open tide output file <%s>\n", tide_file);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(MB_FAILURE);
		}
		fprintf(ofp, "# Tide model generated by program %s\n", program_name);
		fprintf(ofp, "# Version: %s\n", rcs_id);
		fprintf(ofp, "# MB-System Version: %s\n", MB_VERSION);
		fprintf(ofp, "# Tide model generated by program %s\n", program_name);
		fprintf(ofp, "# which in turn calls OTPS program predict_tide obtained from:\n");
		fprintf(ofp, "#     http://www.coas.oregonstate.edu/research/po/research/tide/\n");
		fprintf(ofp, "#\n");
		fprintf(ofp, "# OTPSnc tide model: \n");
		fprintf(ofp, "#      %s\n", otps_model);
		if (tideformat == 2) {
			fprintf(ofp, "# Output format:\n");
			fprintf(ofp, "#      year month day hour minute second tide\n");
			fprintf(ofp, "# where tide is in meters\n");
		}
		else {
			fprintf(ofp, "# Output format:\n");
			fprintf(ofp, "#      time_d tide\n");
			fprintf(ofp, "# where time_d is in seconds since January 1, 1970\n");
			fprintf(ofp, "# and tide is in meters\n");
		}
		right_now = time((time_t *)0);
		strcpy(date, ctime(&right_now));
		date[strlen(date) - 1] = '\0';
		if ((user_ptr = getenv("USER")) == NULL)
			user_ptr = getenv("LOGNAME");
		if (user_ptr != NULL)
			strcpy(user, user_ptr);
		else
			strcpy(user, "unknown");
		gethostname(host, MBP_FILENAMESIZE);
		fprintf(ofp, "# Run by user <%s> on cpu <%s> at <%s>\n", user, host, date);

        /* loop over tide model values, writing them out in the specified format */
		nline = 0;
		ngood = 0;
		while ((result = fgets(line, MB_PATH_MAXLINE, tfp)) == line) {
			nline++;
			if (nline == 2 || nline == 3) {
				fprintf(ofp, "#%s", line);
			}
			else if (nline > 6) {
                
				nget = sscanf(line, "%lf %lf %d.%d.%d %d:%d:%d %lf %lf", &lat, &lon, &time_i[1], &time_i[2], &time_i[0],
				              &time_i[3], &time_i[4], &time_i[5], &tide, &depth);
				if (nget == 10) {
					ngood++;
                    
                    /* if tide station data have been loaded, interpolate the
                     * correction value to apply to the tide model */
                    if (mbotps_mode & MBOTPS_MODE_TIDESTATION
                        && ntidestation > 0) {
                        intstat = mb_linear_interp(verbose,
                                                   tidestation_time_d - 1,
                                                   tidestation_correction - 1,
                                                   ntidestation, time_d,
                                                   &correction, &itime, &error);
                        if (intstat == MB_SUCCESS) {
                            tide += correction;
fprintf(stderr,"TIDE STATION CORRECTION: intstat:%d itime:%dof%d time_d:%f correction:%f tide:%f\n",
        intstat, itime, ntidestation, time_d, correction, tide);
                        }
                    }
                    
                    /* write out the tide model */
					if (tideformat == 2) {
						fprintf(ofp, "%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d %9.4f\n", time_i[0], time_i[1], time_i[2], time_i[3],
						        time_i[4], time_i[5], tide);
					}
					else {
						mb_get_time(verbose, time_i, &time_d);
						fprintf(ofp, "%.3f %9.4f\n", time_d, tide);
					}
				}
			}
		}
		fclose(tfp);
		fclose(ofp);

		/* remove the temporary files */
		unlink(lltfile);
		unlink(otpsfile);

		/* some helpful output */
		fprintf(stderr, "\nResults are really in %s\n", tide_file);
	} /* end single position mode */

	/* -------------------------------------------------------------------------
	 * else get tides along the navigation contained in a set of swath files
	 * -----------------------------------------------------------------------*/
	else if (mbotps_mode == MBOTPS_MODE_NAVIGATION) {
		/* get format if required */
		if (format == 0)
			mb_get_format(verbose, read_file, NULL, &format, &error);

		/* determine whether to read one file or a list of files */
		if (format < 0)
			read_datalist = MB_YES;

		/* open file list */
		if (read_datalist == MB_YES) {
			if ((status = mb_datalist_open(verbose, &datalist, read_file, look_processed, &error)) != MB_SUCCESS) {
				error = MB_ERROR_OPEN_FAIL;
				fprintf(stderr, "\nUnable to open data list file: %s\n", read_file);
				fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
				exit(error);
			}
			if ((status = mb_datalist_read(verbose, datalist, file, dfile, &format, &file_weight, &error)) == MB_SUCCESS)
				read_data = MB_YES;
			else
				read_data = MB_NO;
		}
		/* else copy single filename to be read */
		else {
			strcpy(file, read_file);
			read_data = MB_YES;
		}

		/* loop over all files to be read */
		while (read_data == MB_YES) {
			
			/* Figure out if the file needs a tide model - don't generate a new tide
				model if one was made previously and is up to date AND the
				appropriate request has been made */
			proceed = MB_YES;
			sprintf(tide_file, "%s.tde", file);
			if (skip_existing == MB_YES) {
				if ((fstat = stat(file, &file_status)) == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
					input_modtime = file_status.st_mtime;
					input_size = file_status.st_size;
				}
				else {
					input_modtime = 0;
					input_size = 0;
				}
				if ((fstat = stat(tide_file, &file_status)) == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
					output_modtime = file_status.st_mtime;
					output_size = file_status.st_size;
				}
				else {
					output_modtime = 0;
					output_size = 0;
				}
				if (output_modtime > input_modtime && input_size > 0 && output_size > 0) {
					proceed = MB_NO;
				}
			}

			/* skip the file */
			if (proceed == MB_NO) {
				/* some helpful output */
				fprintf(stderr, "\n---------------------------------------\n\nProcessing tides for %s\n\n", file);
			}
			
			/* generate the tide model */
			else {
				/* some helpful output */
				fprintf(stderr, "\n---------------------------------------\n\nProcessing tides for %s\n\n", file);
	
				/* first open temporary file of lat lon time */
				pid = getpid();
				strcpy(swath_file, file);
				sprintf(lltfile, "tmp_mbotps_llt_%d.txt", pid);
				sprintf(otpsfile, "tmp_mbotps_llttd_%d.txt", pid);
				if ((tfp = fopen(lltfile, "w")) == NULL) {
					error = MB_ERROR_OPEN_FAIL;
					fprintf(stderr, "\nUnable to open temporary lat-lon-time file <%s> for writing\n", lltfile);
					fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
					exit(MB_FAILURE);
				}
	
				/* read fnv file if possible */
				mb_get_fnv(verbose, file, &format, &error);
	
				/* initialize reading the swath file */
				if ((status = mb_read_init(verbose, file, format, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap,
										   &mbio_ptr, &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error)) !=
					MB_SUCCESS) {
					mb_error(verbose, error, &message);
					fprintf(stderr, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", message);
					fprintf(stderr, "\nMultibeam File <%s> not initialized for reading\n", file);
					fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
					exit(error);
				}
	
				/* allocate memory for data arrays */
				if (error == MB_ERROR_NO_ERROR)
					status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag, &error);
				if (error == MB_ERROR_NO_ERROR)
					status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, &error);
				if (error == MB_ERROR_NO_ERROR)
					status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, &error);
				if (error == MB_ERROR_NO_ERROR)
					status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathacrosstrack,
											   &error);
				if (error == MB_ERROR_NO_ERROR)
					status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathalongtrack,
											   &error);
				if (error == MB_ERROR_NO_ERROR)
					status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &error);
				if (error == MB_ERROR_NO_ERROR)
					status =
						mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssacrosstrack, &error);
				if (error == MB_ERROR_NO_ERROR)
					status =
						mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssalongtrack, &error);
	
				/* if error initializing memory then quit */
				if (error != MB_ERROR_NO_ERROR) {
					mb_error(verbose, error, &message);
					fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
					fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
					exit(error);
				}
	
				/* read and use data */
				nread = 0;
				while (error <= MB_ERROR_NO_ERROR) {
					/* reset error */
					error = MB_ERROR_NO_ERROR;
					output = MB_NO;
	
					/* read next data record */
					status = mb_get_all(verbose, mbio_ptr, &store_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed, &heading,
										&distance, &altitude, &sonardepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp,
										bathacrosstrack, bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment, &error);
	
					/* print debug statements */
					if (verbose >= 2) {
						fprintf(stderr, "\ndbg2  Ping read in program <%s>\n", program_name);
						fprintf(stderr, "dbg2       kind:           %d\n", kind);
						fprintf(stderr, "dbg2       error:          %d\n", error);
						fprintf(stderr, "dbg2       status:         %d\n", status);
					}
	
					/* deal with nav and time from survey data only - not nav, sidescan, or subbottom */
					if (error <= MB_ERROR_NO_ERROR && kind == MB_DATA_DATA) {
						/* flag positions and times for output at specified interval */
						if (nread == 0 || time_d - savetime_d >= interval) {
							savetime_d = time_d;
							output = MB_YES;
						}
						lasttime_d = time_d;
						lastlon = navlon;
						lastlat = navlat;
	
						/* increment counter */
						nread++;
					}
	
					/* output position and time if flagged or end of file */
					if (output == MB_YES || error == MB_ERROR_EOF) {
						if (lastlon < 0.0)
							lastlon += 360.0;
						mb_get_date(verbose, lasttime_d, time_i);
						fprintf(tfp, "%.6f %.6f %4.4d %2.2d %2.2d %2.2d %2.2d %2.2d\n", lastlat, lastlon, time_i[0], time_i[1],
								time_i[2], time_i[3], time_i[4], time_i[5]);
					}
				}
	
				/* close the swath file */
				status = mb_close(verbose, &mbio_ptr, &error);
	
				/* output read statistics */
				fprintf(stderr, "%d records read from %s\n", nread, file);
	
				/* close the llt file */
				fclose(tfp);
	
				/* call predict_tide with popen */
				sprintf(predict_tide, "%s/predict_tide", otps_location_use);
				if ((tfp = popen(predict_tide, "w")) == NULL) {
					error = MB_ERROR_OPEN_FAIL;
					fprintf(stderr, "\nUnable to open predict_time program using popen()\n");
					fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
					exit(MB_FAILURE);
				}
	
				/* send relevant input to predict_tide through its stdin stream */
				fprintf(tfp, "%s/DATA/Model_%s\n", otps_location_use, otps_model);
				fprintf(tfp, "%s\n", lltfile);
				fprintf(tfp, "z\n\nAP\noce\n1\n");
				/*fprintf(tfp, "z\nm2,s2,n2,k2,k1,o1,p1,q1\nAP\noce\n1\n");*/
				fprintf(tfp, "%s\n", otpsfile);
	
				/* close the process */
				pclose(tfp);
	
				/* now read results from predict_tide and rewrite them in a useful form */
				if ((tfp = fopen(otpsfile, "r")) == NULL) {
					error = MB_ERROR_OPEN_FAIL;
					fprintf(stderr, "\nUnable to open predict_time results temporary file <%s>\n", otpsfile);
					fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
					exit(MB_FAILURE);
				}
				if ((ofp = fopen(tide_file, "w")) == NULL) {
					error = MB_ERROR_OPEN_FAIL;
					fprintf(stderr, "\nUnable to open tide output file <%s>\n", tide_file);
					fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
					exit(MB_FAILURE);
				}
				fprintf(ofp, "# Tide model generated by program %s\n", program_name);
				fprintf(ofp, "# Version: %s\n", rcs_id);
				fprintf(ofp, "# MB-System Version: %s\n", MB_VERSION);
				fprintf(ofp, "# Tide model generated by program %s\n", program_name);
				fprintf(ofp, "# which in turn calls OTPS program predict_tide obtained from:\n");
				fprintf(ofp, "#     http://www.coas.oregonstate.edu/research/po/research/tide/\n");
				right_now = time((time_t *)0);
				strcpy(date, ctime(&right_now));
				date[strlen(date) - 1] = '\0';
				if ((user_ptr = getenv("USER")) == NULL)
					user_ptr = getenv("LOGNAME");
				if (user_ptr != NULL)
					strcpy(user, user_ptr);
				else
					strcpy(user, "unknown");
				gethostname(host, MBP_FILENAMESIZE);
				fprintf(ofp, "# Run by user <%s> on cpu <%s> at <%s>\n", user, host, date);
	
                /* loop over tide model values, writing them out in the specified format */
				nline = 0;
				ngood = 0;
				while ((result = fgets(line, MB_PATH_MAXLINE, tfp)) == line) {
					nline++;
					if (nline == 2 || nline == 3) {
						fprintf(ofp, "#%s", line);
					}
					else if (nline > 6) {
						nget = sscanf(line, "%lf %lf %d.%d.%d %d:%d:%d %lf %lf", &lat, &lon, &time_i[1], &time_i[2], &time_i[0],
									  &time_i[3], &time_i[4], &time_i[5], &tide, &depth);
						if (nget == 10) {
							ngood++;
                    
                            /* if tide station data have been loaded, interpolate the
                             * correction value to apply to the tide model */
                            if (mbotps_mode & MBOTPS_MODE_TIDESTATION
                                && ntidestation > 0) {
                                intstat = mb_linear_interp(verbose,
                                                           tidestation_time_d - 1,
                                                           tidestation_correction - 1,
                                                           ntidestation, time_d,
                                                           &correction, &itime, &error);
                                if (intstat == MB_SUCCESS) {
                                    tide += correction;
fprintf(stderr,"TIDE STATION CORRECTION: intstat:%d itime:%dof%d time_d:%f correction:%f tide:%f\n",
        intstat, itime, ntidestation, time_d, correction, tide);
                                }
                            }

                            /* write out the tide model */
							if (tideformat == 2) {
								fprintf(ofp, "%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d %9.4f\n", time_i[0], time_i[1], time_i[2],
										time_i[3], time_i[4], time_i[5], tide);
							}
							else {
								mb_get_time(verbose, time_i, &time_d);
								fprintf(ofp, "%.3f %9.4f\n", time_d, tide);
							}
						}
					}
				}
				fclose(tfp);
				fclose(ofp);
	
				/* remove the temporary files */
				unlink(lltfile);
				unlink(otpsfile);
	
				/* some helpful output */
				fprintf(stderr, "\nResults are really in %s\n", tide_file);
	
				/* set mbprocess usage of tide file */
				if (mbprocess_update == MB_YES && ngood > 0) {
					status = mb_pr_update_tide(verbose, swath_file, MBP_TIDE_ON, tide_file, tideformat, &error);
					fprintf(stderr, "MBprocess set to apply tide correction to %s\n", swath_file);
				}
					
			}

			/* figure out whether and what to read next */
			if (read_datalist == MB_YES) {
				if ((status = mb_datalist_read(verbose, datalist, file, dfile, &format, &file_weight, &error)) == MB_SUCCESS)
					read_data = MB_YES;
				else
					read_data = MB_NO;
			}
			else {
				read_data = MB_NO;
			}

			/* end loop over files in list */
		}
		if (read_datalist == MB_YES)
			mb_datalist_close(verbose, &datalist, &error);
	}

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose, &error);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Program <%s> completed\n", program_name);
		fprintf(stderr, "dbg2  Ending status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	/* end it all */
	exit(error);
}
/*--------------------------------------------------------------------*/
