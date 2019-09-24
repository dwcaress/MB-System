/*--------------------------------------------------------------------
 *    The MB-system:	mbgpstide.c	2018-05-23
 *
 *    Copyright (c) 2018
 *    Gordon J. Keith (gordon@keith.id.au)
 *    and David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbgpstide generates tide files from the GPS altitude data recorded in
 * the input files.
 *
 * Input (-I) may be a single data file or a datalist. The format of the input file may be specified
 * using the -F option. Default is -Idatalist.mb-1.
 *
 * Output is either to a file specified by -O ("-" for stdout) or to <file>.gps.tde where <file> is
 * the name of the input data file. The -S option specifies that the <file>.gps.tde will not be generated
 * if it already exists. The format of the tide file may be specified with -A where 1 is
 * a two column format, seconds since 1970-01-01 and tide height; and two is a seven column format,
 * year, month, day, hour, minute, second and tide height. Default is -A2.
 *
 * The -Dinterval indicates the time interval in seconds over which the tide values will be averaged to get a value.
 * The default is 300 seconds (5 minutes).
 *
 * -M will cause the program to set tide processing on for the input data file using the output file generated.
 *
 * -Roffset adds a constant offset to the tide value.
 *
 * -Tgrid takes a geoid difference grid and adds the offset for each location in the file.
 * The geoid difference grid is in GMT format. For example
 *		wget  http://earth-info.nga.mil/GandG/wgs84/gravitymod/egm2008/GIS/world_geoid/s45e135.zip
 * 		unzip s45e135.zip
 * 		grdconvert s45e135/s45e135/ s45e135.grd
 * 		mbgpstide -Ts45e135.grd
 *
 * -Usource specifies the source of GPS elipsoid height. 0 = Simrad Height telegram or GSF ping height,
 * 		1 = GSF ping height + separator.
 *
 * Author:	G. J. Keith
 * Date:	May 29, 2018
 */

#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "mb_define.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mb_format.h"
#include "mb_process.h"

#ifdef ENABLE_GSF
#include "mbsys_gsf.h"
#endif
#include "mbsys_simrad2.h"
#include "mbsys_simrad3.h"


/* system function declarations */
char *ctime();
#ifndef WIN32
char *getenv();
#endif

/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
	static char program_name[] = "mbgpstide";
	static char help_message[] =
	    "MBgpstide generates tide files from the GPS altitude data in the input files.";
	static char usage_message[] =
	    "mbgpstide [-Atideformat -Dinterval -Fformat -Idatalist -M -Ooutput -Roffset -S -Tgeoid -Usource,sensor -V]";
	int option_index;
	int errflg = 0;
	int c;
	int help = 0;

	/* command line option definitions */
	/* mbdatalist
	 * 		--verbose				-V
	 * 		--help					-H
	 * 		--tideformat=FORMAT		-A
	 * 		--interval=SECONDS		-D
	 * 		--format=FORMATID		-F format
	 * 		--input=FILE			-I inputfile
	 * 		--setparameters			-M
	 * 		--output=FILE			-O outputfile
	 * 		--offset=OFFSET`		-R offset
	 * 		--skipexisting			-S
	 * 		--geoid=MODELGRID		-T grdfile
	 * 		--use=HEIGHTSOURCE 		-U heightsource
	 */
	static struct option options[] =   {{"verbose", no_argument, NULL, 0},
										{"help", no_argument, NULL, 0},
										{"tideformat", required_argument, NULL, 0},
										{"interval", required_argument, NULL, 0},
										{"format", required_argument, NULL, 0},
										{"input", required_argument, NULL, 0},
										{"setparameters", no_argument, NULL, 0},
										{"output", required_argument, NULL, 0},
										{"offset", required_argument, NULL, 0},
										{"skipexisting", no_argument, NULL, 0},
										{"geoid",required_argument , NULL, 0},
										{"use", required_argument, NULL, 0},
										{NULL, 0, NULL, 0}};

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

	double tidelon;
	double tidelat;
	double btime_d;
	double etime_d;
	int btime_i[7];
	int etime_i[7];
	double interval = 300.0;
	mb_path tide_file;
	mb_path nav_file;
	int file_output = MB_NO;
	int mbprocess_update = MB_NO;
	int skip_existing = MB_NO;
	int tideformat = 2;
	int ngood = 0;

  	/* time parameters */
	time_t right_now;
	char date[32], user[MB_PATH_MAXLINE], *user_ptr, host[MB_PATH_MAXLINE];
	int pid;

	FILE *tfp, *mfp, *ofp;
	struct stat file_status;
	int fstat;
	int proceed = MB_YES;
	int input_size, input_modtime, output_size, output_modtime;
	mb_path line = "";
	mb_path geoidgrid = "";
	int read_data;
	int nread;
	double lasttime_d = 0.0;
	double last_heave = 0.0;
	double tide;

	int gps_source = 0;
	double tide_offset = 0.0;
	double geoid_offset = 0.0;
	int geoid_set = MB_NO;
	int read_geoid = MB_NO;
	int have_height = MB_NO;
	double height = 0.0;
	double ttime_d;
	double atime_d;
	double geoid_time = 0.0;

	double this_interval = 0.0;
	double next_interval = 0.0;
	int count_tide = 0;
	double sum_tide = 0.0;
	double atide = 0.0;

	struct mb_io_struct *mb_io_ptr = NULL;
	struct mbsys_simrad2_struct *simrad2_ptr;
	struct mbsys_simrad3_struct *simrad3_ptr;
#ifdef ENABLE_GSF
	struct mbsys_gsf_struct *gsf_ptr;
#endif

	/* get current default values */
	status = mb_defaults(verbose, &format, &pings, &lonflip, bounds, btime_i, etime_i, &speedmin, &timegap);

	/* set default input to datalist.mb-1 */
	strcpy(read_file, "datalist.mb-1");

	/* process argument list */
	while ((c = getopt_long(argc, argv, "A:a:D:d:F:f:I:i:MmO:o:R:r:SsT:t:U:u:VvHh", options, &option_index)) != -1)
		switch (c) {
		/* long options */
		case 0:
			/* verbose */
			if (strcmp("verbose", options[option_index].name) == 0) {
				verbose++;
			}

			/* help */
			else if (strcmp("help", options[option_index].name) == 0) {
				help = MB_YES;
			}

			/* tideformat */
			else if (strcmp("tideformat", options[option_index].name) == 0) {
				sscanf(optarg, "%d", &tideformat);
				if (tideformat != 2)
					tideformat = 1;
			}

			/* interval */
			else if (strcmp("interval", options[option_index].name) == 0) {
				sscanf(optarg, "%lf", &interval);
			}

			/* format */
			else if (strcmp("format", options[option_index].name) == 0) {
				sscanf(optarg, "%d", &format);
			}

			/* input */
			else if (strcmp("input", options[option_index].name) == 0) {
				sscanf(optarg, "%s", read_file);
			}

			/*  */
			else if (strcmp("setparameters", options[option_index].name) == 0) {
				mbprocess_update = MB_YES;
			}

			/*  */
			else if (strcmp("output", options[option_index].name) == 0) {
				sscanf(optarg, "%s", tide_file);
				file_output = MB_YES;
			}

			/*  */
			else if (strcmp("offset", options[option_index].name) == 0) {
				sscanf(optarg, "%lf", &tide_offset);
			}

			/*  */
			else if (strcmp("skipexisting", options[option_index].name) == 0) {
				skip_existing = MB_YES;
			}

			/*  */
			else if (strcmp("geoid", options[option_index].name) == 0) {
				sscanf(optarg, "%s", geoidgrid);
				geoid_set = MB_YES;
			}

			/*  */
			else if (strcmp("use", options[option_index].name) == 0) {
				sscanf(optarg, "%d", &gps_source);
			}
			break;

		/* short options */
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
			break;
		case 'D':
		case 'd':
			sscanf(optarg, "%lf", &interval);
			break;
		case 'F':
		case 'f':
			sscanf(optarg, "%d", &format);
			break;
		case 'I':
		case 'i':
			sscanf(optarg, "%s", read_file);
			break;
		case 'M':
		case 'm':
			mbprocess_update = MB_YES;
			break;
		case 'O':
		case 'o':
			sscanf(optarg, "%s", tide_file);
			file_output = MB_YES;
			break;
		case 'R':
		case 'r':
			sscanf(optarg, "%lf", &tide_offset);
			break;
		case 'S':
		case 's':
			skip_existing = MB_YES;
			break;
		case 'T':
		case 't':
			sscanf(optarg, "%s", geoidgrid);
			geoid_set = MB_YES;
			break;
		case 'U':
		case 'u':
			sscanf(optarg, "%d", &gps_source);
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

	if (verbose == 1 || help) {
		fprintf(stderr, "\nProgram %s\n", program_name);
		fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
	}

	/* if help desired then print it and exit */
	if (help) {
		fprintf(stderr, "\n%s\n", help_message);
		fprintf(stderr, "\nusage: %s\n", usage_message);
	}


	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Program <%s>\n", program_name);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Control Parameters:\n");
		fprintf(stderr, "dbg2       verbose:              %d\n", verbose);
		fprintf(stderr, "dbg2       help:                 %d\n", help);
		fprintf(stderr, "dbg2       interval:             %f\n", interval);
		fprintf(stderr, "dbg2       mbprocess_update:     %d\n", mbprocess_update);
		fprintf(stderr, "dbg2       skip_existing:        %d\n", skip_existing);
		fprintf(stderr, "dbg2       tideformat:           %d\n", tideformat);
		fprintf(stderr, "dbg2       format:               %d\n", format);
		fprintf(stderr, "dbg2       read_file:            %s\n", read_file);
	}

	/* if help was all that was desired then exit */
	if (help) {
		exit(error);
	}

	/* If a single output file is specified, open and initialise it */
	if (file_output == MB_YES) {
		if (strcmp(tide_file, "-") == 0) {
			ofp = stdout;
		} else {
			if ((ofp = fopen(tide_file, "w")) == NULL) {
				error = MB_ERROR_OPEN_FAIL;
				fprintf(stderr, "\nUnable to open tide output file <%s>\n", tide_file);
				fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
				exit(MB_FAILURE);
			}
		}

		if (tideformat == 5) {
			fprintf(ofp, "--------\n");
		} else {
			fprintf(ofp, "# Tide model generated by program %s\n", program_name);
			fprintf(ofp, "# MB-System Version: %s\n", MB_VERSION);
			fprintf(ofp, "#   ");
			for (int i = 0; i < argc; i++ ) {
				fprintf(ofp, " %s",argv[i]);
			}
			fprintf(ofp, " \n");
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
		}
	}

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
		if (file_output == MB_NO) {
			sprintf(tide_file, "%s.gps.tde", file);
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
		}
		/* skip the file */
		if (proceed == MB_NO) {
			/* some helpful output */
			fprintf(stderr, "\n---------------------------------------\n\nProcessing tides for %s\n\n", file);
		}

		/* generate the tide model */
		else {
			/* if one output file per input file then open and initialise it */
			if (file_output == MB_NO) {
				if ((ofp = fopen(tide_file, "w")) == NULL) {
					error = MB_ERROR_OPEN_FAIL;
					fprintf(stderr, "\nUnable to open tide output file <%s>\n", tide_file);
					fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
					exit(MB_FAILURE);
				}

				if (tideformat == 5) {
					fprintf(ofp, "--------\n");
				} else {
					fprintf(ofp, "# Tide model generated by program %s\n", program_name);
					fprintf(ofp, "# MB-System Version: %s\n", MB_VERSION);
					fprintf(ofp, "#   ");
					for (int i = 0; i < argc; i++ ) {
						fprintf(ofp, " %s",argv[i]);
					}
					fprintf(ofp, " \n");
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
				}
			}

			/* some helpful output */
			fprintf(stderr, "\n---------------------------------------\n\nProcessing tides for %s\n\n", file);

			strcpy(swath_file, file);

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

			/* get geoid corrections */
			if (geoid_set == MB_YES) {
				sprintf(nav_file, "%s.fnv", swath_file);
				if ((fstat = stat(nav_file, &file_status)) == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
					sprintf(line, "awk '{ print $8 \" \" $9 \" \" $7 }' %s | grdtrack -G%s", nav_file, geoidgrid);
				} else {
					sprintf(line, "mblist -F%d -I%s -OXYU | grdtrack -G%s", format, file, geoidgrid);
				}
				if ((tfp = popen(line, "r")) != NULL ) {
					read_geoid = MB_YES;
					if (EOF == fscanf(tfp, "%lf %lf %lf %lf\n", &tidelat, &tidelon, &geoid_time, &geoid_offset)) {
						pclose(tfp);
						fprintf(stderr,"\nError - Geoid model returned no data\n");
						exit(MB_FAILURE);
					}
				} else {
					fprintf(stderr, "\nUnable to read geoid model\n");
					exit(MB_FAILURE);
				}
			}

			/* read and use data */
			nread = 0;
			while (error <= MB_ERROR_NO_ERROR) {
				/* reset error */
				error = MB_ERROR_NO_ERROR;

				/* read next data record */
				status = mb_get_all(verbose, mbio_ptr, &store_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed, &heading,
									&distance, &altitude, &sonardepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp,
									bathacrosstrack, bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment, &error);

				if (verbose >= 2) {
					fprintf(stderr, "\ndbg2  Ping read in program <%s>\n", program_name);
					fprintf(stderr, "dbg2       kind:           %d\n", kind);
					fprintf(stderr, "dbg2       error:          %d\n", error);
					fprintf(stderr, "dbg2       status:         %d\n", status);
				}

				if (error <= MB_ERROR_NO_ERROR && kind == MB_DATA_START) {
					if (verbose >= 2)
						fprintf(stderr, "dbg2       Have Installation telegram\n");
				}

				mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

#ifdef ENABLE_GSF
				if (mb_io_ptr->format == MBF_GSFGENMB) {
					if (error <= MB_ERROR_NO_ERROR && kind == MB_DATA_DATA) {
						ttime_d = time_d;
						gsf_ptr = (struct mbsys_gsf_struct *)mb_io_ptr->store_data;
						height = gsf_ptr->records.mb_ping.height;
						if (gps_source == 1) {
							height += gsf_ptr->records.mb_ping.sep;
						}
						have_height = MB_YES;
						nread++;
					}
				}
#endif
				/* Read sounder height from height telegram */
				if (error <= MB_ERROR_NO_ERROR && kind == MB_DATA_HEIGHT && gps_source == 0) {

					if (mb_io_ptr->format == MBF_EM300MBA || mb_io_ptr->format == MBF_EM300RAW) {
						simrad2_ptr = (struct mbsys_simrad2_struct *)mb_io_ptr->store_data;
						height = simrad2_ptr->hgt_height * 0.01;
						time_i[0] = simrad2_ptr->hgt_date / 10000;
						time_i[1] = (simrad2_ptr->hgt_date % 10000) / 100;
						time_i[2] = simrad2_ptr->hgt_date % 100;
						time_i[3] = simrad2_ptr->hgt_msec / 3600000;
						time_i[4] = (simrad2_ptr->hgt_msec % 3600000) / 60000;
						time_i[5] = (simrad2_ptr->hgt_msec % 60000) / 1000;
						time_i[6] = (simrad2_ptr->hgt_msec % 1000) * 1000;
						mb_get_time(verbose, time_i, &ttime_d);
						have_height = MB_YES;

					} else if (mb_io_ptr->format == MBF_EM710MBA || mb_io_ptr->format == MBF_EM710RAW) {
						simrad3_ptr = (struct mbsys_simrad3_struct *)mb_io_ptr->store_data;
						height = simrad3_ptr->hgt_height * 0.01;
						time_i[0] = simrad3_ptr->hgt_date / 10000;
						time_i[1] = (simrad3_ptr->hgt_date % 10000) / 100;
						time_i[2] = simrad3_ptr->hgt_date % 100;
						time_i[3] = simrad3_ptr->hgt_msec / 3600000;
						time_i[4] = (simrad3_ptr->hgt_msec % 3600000) / 60000;
						time_i[5] = (simrad3_ptr->hgt_msec % 60000) / 1000;
						time_i[6] = (simrad3_ptr->hgt_msec % 1000) * 1000;
						mb_get_time(verbose, time_i, &ttime_d);
						have_height = MB_YES;

					}

					/* increment counter */
					nread++;
				}

				if (have_height == MB_YES) {

					if (ttime_d > next_interval || (file_output == MB_NO && error == MB_ERROR_EOF)) {
						if (count_tide > 0) {
							ngood++;
							atide = sum_tide / count_tide;
							if (tideformat == 1) {
								fprintf(ofp, "%.3f %9.4f\n", this_interval, atide);

							} else if (tideformat == 5) { // CARIS
								mb_get_date(verbose, this_interval, time_i);
								fprintf(ofp, "%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%.3f  %.6f\n",
										time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5] + time_i[6] * 0.000001, atide);

							} else {
								mb_get_date(verbose, this_interval, time_i);
								fprintf(ofp, "%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d %9.4f\n",
										time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5],
										atide);
							}
						}
						count_tide = 0;
						sum_tide = 0.0;
						if (interval == 0) {
							this_interval = ttime_d;
						} else {
							this_interval = (int)(ttime_d / interval + 0.5) * interval;
							next_interval = this_interval + interval / 2;
						}
					}

					/* find the first geoid offset along the track after the height time */
					while (read_geoid == MB_YES && geoid_time < ttime_d) {
						if (EOF == fscanf(tfp, "%lf %lf %lf %lf\n", &tidelat, &tidelon, &geoid_time, &geoid_offset)) {
							pclose(tfp);
							read_geoid = MB_NO;
						}
						if (verbose >= 2)
						fprintf(stderr, "tide %.0f, geoid %.0f, goff %.3f, %.4f %.4f\n",
								ttime_d, geoid_time, geoid_offset, tidelat, tidelon);
					}

					count_tide++;
					sum_tide += height + tide_offset - geoid_offset;
					have_height = MB_NO;
					if (verbose >= 1)
						fprintf(stderr, "time %f, interval %f, count %d, sum %.2f, tide %.2f, offset %.2f, geoid %.2f\n",
								ttime_d, next_interval, count_tide, sum_tide, height, tide_offset, geoid_offset);
				}
			}

			/* close the swath file */
			status = mb_close(verbose, &mbio_ptr, &error);

			if (read_geoid == MB_YES) {
				pclose(tfp);
				read_geoid = MB_NO;
			}

			if (file_output == MB_NO) {
				fclose(ofp);
			}

			/* output read statistics */
			fprintf(stderr, "%d records read from %s\n", nread, file);

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
	if (read_datalist == MB_YES) {
		mb_datalist_close(verbose, &datalist, &error);
	}

	/* if single output file specified, then finalise and close it */
	if (file_output == MB_YES) {
		if (count_tide > 0) {
			atide = sum_tide / count_tide;
			if (tideformat == 1) {
				fprintf(ofp, "%.3f %9.4f\n", this_interval, atide);

			} else if (tideformat == 5) { // CARIS
				mb_get_date(verbose, this_interval, time_i);
				fprintf(ofp, "%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%.3f  %.6f\n",
						time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5] + time_i[6] * 0.000001, atide);

			} else {
				mb_get_date(verbose, this_interval, time_i);
				fprintf(ofp, "%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d %9.4f\n",
						time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], atide);
			}
		}
		if (ofp != stdout) {
			fclose(ofp);
		}
	}

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose, &error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Program <%s> completed\n", program_name);
		fprintf(stderr, "dbg2  Ending status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	exit(error);
}
/*--------------------------------------------------------------------*/
