/*--------------------------------------------------------------------
 *    The MB-system:	mbsxppreprocess.c	09/12/2013
 *    $Id: mbsxppreprocess.c 2129 2013-07-08 07:45:32Z caress $
 *
 *    Copyright (c) 2005-2014 by
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
 * mbsxppreprocess reads a BathySwath (formerly SWATHplus) SXP file
 * and seperates the data from each transducer into seperate SXP files,
 * one for each transducer. This scheme allows MB System to process all
 * configurations of the BathySwath interferometers, regardless of ping
 * mode or number of transducers installed. This program can also fix
 * various problems with SXP data.
 *
 * In the future, I would like to add the ability to combine
 * simultaneous port and starboard pings into a psuedo-ping, and
 * also convert SXP files into SXI format, which is much closer
 * to a conventional multibeam file. In both cases, I need to
 * handle up to 3 transducers (port, starboard, forward-facing)
 * pinging in various modes (alternatiting, simultaneous).
 *
 * Author:	D. P. Finlayson
 * Email:       dfinlayson@usgs.gov
 * Date:	Sep 12, 2013
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

/* MBIO include files */
#include "mb_status.h"
#include "mb_format.h"
#include "mb_define.h"
#include "mb_io.h"
#include "mb_aux.h"
#include "mbsys_swathplus.h"

static char rcs_id[] = "$Id: mbsxppreprocess.c 2129 2013-07-08 07:45:32Z caress $";

/*--------------------------------------------------------------------*/

int main
(
	int argc,
	char **argv
)
{
	char program_name[] = "mbsxppreprocess";
	char help_message[] =
		"mbsxppreprocess reads a BathySwath (formerly SWATHplus) SXP file \nand seperates the data from each transducer into seperate SXP files,\none for each transducer. This scheme allows MB System to process all \nconfigurations of the BathySwath interferometers, regardless of ping \nmode or number of transducers installed. This program can also fix \nvarious problems with SXP data.";
	char usage_message[] = "mbsxppreprocess [-Fformat -Ifile -Ooutfile -H -V]";
	extern char *optarg;
	int errflg = 0;
	int c;
	int help = 0;
	int flag = 0;

	/* MBIO status variables */
	int status = MB_SUCCESS;
	int verbose = 0;
	int error = MB_ERROR_NO_ERROR;
	char    *message;

	/* MBIO read control parameters */
	int read_datalist = MB_NO;
	char read_file[MB_PATH_MAXLINE];
	void    *datalist;
	int look_processed = MB_DATALIST_LOOK_UNSET;
	double file_weight;
	int format = 0;
	int pings;
	int lonflip;
	double bounds[4];
	int btime_i[7];
	int etime_i[7];
	double btime_d;
	double etime_d;
	double speedmin;
	double timegap;
	char ifile[MB_PATH_MAXLINE];
	char ofilebase[MBSYS_SWPLS_MAX_TXERS][MB_PATH_MAXLINE];
	char ofile[MBSYS_SWPLS_MAX_TXERS][MB_PATH_MAXLINE];
	int ofileinit[MBSYS_SWPLS_MAX_TXERS];
	int ofile_set = MB_NO;
	int beams_bath;
	int beams_amp;
	int pixels_ss;
	int obeams_bath;
	int obeams_amp;
	int opixels_ss;
	int txidx;
	int read_data;
	int i;
	int testformat;
	char fileroot[MB_PATH_MAXLINE];

	/* MBIO read values */
	void    *imbio_ptr = NULL;
	struct mb_io_struct *imb_io_ptr = NULL;
	void    *istore_ptr = NULL;
	struct mbsys_swathplus_struct *istore = NULL;
	void *ombio_ptr = NULL;
	void *ombio_ptr_arr[MBSYS_SWPLS_MAX_TXERS];
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
	char    *beamflag = NULL;
	double  *bath = NULL;
	double  *bathacrosstrack = NULL;
	double  *bathalongtrack = NULL;
	double  *amp = NULL;
	double  *ss = NULL;
	double  *ssacrosstrack = NULL;
	double  *ssalongtrack = NULL;
	char comment[MB_COMMENT_MAXLINE];

	/* data structure pointers */
	swplssxp_header *header;
	swplssxp_ping   *ping;
	int type;
	int stored_header = MB_NO;

	/* counting variables */
	int nfile_read = 0;
	int nfile_write = 0;
	int nrec_sxp_header = 0;
	int nrec_xyza_ping1[MBSYS_SWPLS_MAX_TXERS];
	int nrec_xyza_ping2[MBSYS_SWPLS_MAX_TXERS];
	int nrec_other = 0;
	int nrec_ping_sel_off = 0;
	int nrec_ping_sel_single = 0;
	int nrec_ping_sel_alt = 0;
	int nrec_ping_sel_sim = 0;
	int nrec_filtered_time = 0;

	int nrec_sxp_header_tot = 0;
	int nrec_xyza_ping1_tot[MBSYS_SWPLS_MAX_TXERS];
	int nrec_xyza_ping2_tot[MBSYS_SWPLS_MAX_TXERS];
	int nrec_other_tot = 0;
	int nrec_ping_sel_off_tot = 0;
	int nrec_ping_sel_single_tot = 0;
	int nrec_ping_sel_alt_tot = 0;
	int nrec_ping_sel_sim_tot = 0;
	int nrec_filtered_time_tot = 0;

	/* last time_d variables - used to check for repeated data */
	double last_time_d[MBSYS_SWPLS_MAX_TXERS];

	/* MBARI data flag */

	/* initialize the array of ombio_ptr void pointers */
	for (i = 0; i < MBSYS_SWPLS_MAX_TXERS; i++)
		{
		ofileinit[i] = MB_NO;
		ombio_ptr_arr[i] = ombio_ptr;
		}

	/* initialize the total record count */
	for (i = 0; i < MBSYS_SWPLS_MAX_TXERS; i++)
		{
		nrec_xyza_ping1_tot[i] = 0;
		nrec_xyza_ping2_tot[i] = 0;
		}

	/* initialize the last time read array */
	for (i = 0; i < MBSYS_SWPLS_MAX_TXERS; i++)
		last_time_d[i] = 0.0;

	/* get current default values */
	status = mb_defaults(verbose,
		&format,
		&pings,
		&lonflip,
		bounds,
		btime_i,
		etime_i,
		&speedmin,
		&timegap);

	/* set default input to datalist.mb-1 */
	strcpy (read_file, "datalist.mb-1");

	/* process argument list */
	while ((c = getopt(argc, argv, "F:f:I:i:O:o:VvHh")) != -1)
		switch (c)
			{
		case 'H':
		case 'h':
			help++;
			break;
		case 'V':
		case 'v':
			verbose++;
			break;
		case 'F':
		case 'f':
			sscanf (optarg, "%d", &format);
			flag++;
			break;
		case 'I':
		case 'i':
			sscanf (optarg, "%s", read_file);
			flag++;
			break;
		case 'O':
		case 'o':
			for (i = 0; i < MBSYS_SWPLS_MAX_TXERS; i++)
				sscanf (optarg, "%s", &ofilebase[i][0]);
			ofile_set  = MB_YES;
			flag++;
			break;
		case '?':
			errflg++;
			}

	/* if error flagged then print it and exit */
	if (errflg)
		{
		fprintf(stderr, "usage: %s\n", usage_message);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		error = MB_ERROR_BAD_USAGE;
		exit(error);
		}

	/* print starting message */
	if ((verbose == 1) || help)
		{
		fprintf(stderr, "\nProgram %s\n", program_name);
		fprintf(stderr, "Version %s\n", rcs_id);
		fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
		}

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  Program <%s>\n", program_name);
		fprintf(stderr, "dbg2  Version %s\n", rcs_id);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Control Parameters:\n");
		fprintf(stderr, "dbg2       verbose:             %d\n", verbose);
		fprintf(stderr, "dbg2       help:                %d\n", help);
		fprintf(stderr, "dbg2       format:              %d\n", format);
		fprintf(stderr, "dbg2       pings:               %d\n", pings);
		fprintf(stderr, "dbg2       lonflip:             %d\n", lonflip);
		fprintf(stderr, "dbg2       bounds[0]:           %f\n", bounds[0]);
		fprintf(stderr, "dbg2       bounds[1]:           %f\n", bounds[1]);
		fprintf(stderr, "dbg2       bounds[2]:           %f\n", bounds[2]);
		fprintf(stderr, "dbg2       bounds[3]:           %f\n", bounds[3]);
		fprintf(stderr, "dbg2       btime_i[0]:          %d\n", btime_i[0]);
		fprintf(stderr, "dbg2       btime_i[1]:          %d\n", btime_i[1]);
		fprintf(stderr, "dbg2       btime_i[2]:          %d\n", btime_i[2]);
		fprintf(stderr, "dbg2       btime_i[3]:          %d\n", btime_i[3]);
		fprintf(stderr, "dbg2       btime_i[4]:          %d\n", btime_i[4]);
		fprintf(stderr, "dbg2       btime_i[5]:          %d\n", btime_i[5]);
		fprintf(stderr, "dbg2       btime_i[6]:          %d\n", btime_i[6]);
		fprintf(stderr, "dbg2       etime_i[0]:          %d\n", etime_i[0]);
		fprintf(stderr, "dbg2       etime_i[1]:          %d\n", etime_i[1]);
		fprintf(stderr, "dbg2       etime_i[2]:          %d\n", etime_i[2]);
		fprintf(stderr, "dbg2       etime_i[3]:          %d\n", etime_i[3]);
		fprintf(stderr, "dbg2       etime_i[4]:          %d\n", etime_i[4]);
		fprintf(stderr, "dbg2       etime_i[5]:          %d\n", etime_i[5]);
		fprintf(stderr, "dbg2       etime_i[6]:          %d\n", etime_i[6]);
		fprintf(stderr, "dbg2       speedmin:            %f\n", speedmin);
		fprintf(stderr, "dbg2       timegap:             %f\n", timegap);
		fprintf(stderr, "dbg2       read_file:           %s\n", read_file);
		for (i = 0; i < MBSYS_SWPLS_MAX_TXERS; i++)
			fprintf(stderr, "dbg2       ofilebase[i]:       %s\n", ofilebase[i]);
		fprintf(stderr, "dbg2       ofile_set:           %d\n", ofile_set);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr, "\n%s\n", help_message);
		fprintf(stderr, "\nusage: %s\n", usage_message);
		exit(error);
		}

	/* get format if required */
	if (format == 0)
		mb_get_format(verbose, read_file, NULL, &format, &error);

	/* determine whether to read one file or a list of files */
	if (format < 0)
		read_datalist = MB_YES;

	/* open file list */
	if (read_datalist == MB_YES)
		{
		if ((status =
			mb_datalist_open(verbose, &datalist, read_file, look_processed, &error)) != MB_SUCCESS)
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr, "\nUnable to open data list file: %s\n", read_file);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(error);
			}
		if ((status =
			mb_datalist_read(verbose, datalist, ifile, &format, &file_weight, &error))== MB_SUCCESS)
			read_data = MB_YES;
		else
			read_data = MB_NO;
		}
	/* else copy single filename to be read */
	else
		{
		strcpy(ifile, read_file);
		read_data = MB_YES;
		}

	/* loop over all files to be read */
	while (read_data == MB_YES && format == MBF_SWPLSSXP)
		{
		/* figure out the output file name (one for each transducer) */
		if (ofile_set == MB_NO)
			{
			status = mb_get_format(verbose, ifile, fileroot, &testformat, &error);
			if ((testformat == MBF_SWPLSSXP) && (strncmp(".sxp", &ifile[strlen(ifile)-4], 4) == 0))
				for (i = 0; i < MBSYS_SWPLS_MAX_TXERS; i++)
					sprintf(ofile[i], "%s_txer%d.mb%d", fileroot, i+1, testformat);
			else if (testformat == MBF_SWPLSSXP)
				for (i = 0; i < MBSYS_SWPLS_MAX_TXERS; i++)
					sprintf(ofile[i], "%s_txer%d.mb%d", fileroot, i+1, testformat);
			else
				for (i = 0; i < MBSYS_SWPLS_MAX_TXERS; i++)
					sprintf(ofile[i], "%s_txer%d.mb%d", ifile, i+1, testformat);

			if (verbose >= 2)
				{
				fprintf(stderr, "\n");
				for (i = 0; i < MBSYS_SWPLS_MAX_TXERS; i++)
					fprintf(stderr, "dbg2  Txer %d output file <%s>\n", i+1, ofile[i]);
				}
			}

		/* initialize reading the swath file */
		if ((status =
			mb_read_init(verbose, ifile, format, pings, lonflip, bounds, btime_i, etime_i, speedmin,
				timegap, &imbio_ptr, &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss,
				&error)) != MB_SUCCESS)
			{
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", message);
			fprintf(stderr, "\nMultibeam File <%s> not initialized for reading\n", ifile);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(error);
			}
		nfile_read++;

		/* if ofile has been set then there is only one set of output files, otherwise there
		   is an output file set for each input file */
		if ((ofile_set == MB_NO) || (nfile_write == 0))
			{
			/* initialize writing the output swath sonar file */
			for (i = 0; i < MBSYS_SWPLS_MAX_TXERS; i++)
				ofileinit[i] = MB_NO;

			if (verbose >= 2)
				fprintf(stderr, "\ndbg2  reset ofileinit array to MB_NO\n");
			}

		/* get pointers to data storage */
		imb_io_ptr = (struct mb_io_struct *) imbio_ptr;
		istore_ptr = imb_io_ptr->store_data;
		istore = (struct mbsys_swathplus_struct *) istore_ptr;

		if (error == MB_ERROR_NO_ERROR)
			{
			beamflag = NULL;
			bath = NULL;
			amp = NULL;
			bathacrosstrack = NULL;
			bathalongtrack = NULL;
			ss = NULL;
			ssacrosstrack = NULL;
			ssalongtrack = NULL;
			}
		if (error == MB_ERROR_NO_ERROR)
			status =
				mb_register_array(verbose,
				imbio_ptr,
				MB_MEM_TYPE_BATHYMETRY,
				sizeof(char),
				(void **)&beamflag,
				&error);
		if (error == MB_ERROR_NO_ERROR)
			status =
				mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
				(void **)&bath, &error);
		if (error == MB_ERROR_NO_ERROR)
			status =
				mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double),
				(void **)&amp, &error);
		if (error == MB_ERROR_NO_ERROR)
			status =
				mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
				(void **)&bathacrosstrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status =
				mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
				(void **)&bathalongtrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status =
				mb_register_array(verbose,
				imbio_ptr,
				MB_MEM_TYPE_SIDESCAN,
				sizeof(double),
				(void **)&ss,
				&error);
		if (error == MB_ERROR_NO_ERROR)
			status =
				mb_register_array(verbose,
				imbio_ptr,
				MB_MEM_TYPE_SIDESCAN,
				sizeof(double),
				(void **)&ssacrosstrack,
				&error);
		if (error == MB_ERROR_NO_ERROR)
			status =
				mb_register_array(verbose,
				imbio_ptr,
				MB_MEM_TYPE_SIDESCAN,
				sizeof(double),
				(void **)&ssalongtrack,
				&error);

		/* if error initializing memory then quit */
		if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(error);
			}

		/* reset file record counters */
		nrec_sxp_header = 0;
		for (i = 0; i < MBSYS_SWPLS_MAX_TXERS; i++)
			{
			nrec_xyza_ping1[i] = 0;
			nrec_xyza_ping2[i] = 0;
			}
		nrec_other = 0;
		nrec_ping_sel_off = 0;
		nrec_ping_sel_single = 0;
		nrec_ping_sel_alt = 0;
		nrec_ping_sel_sim = 0;
		nrec_filtered_time = 0;

		/* reset last time stamp */
		for (i = 0; i < MBSYS_SWPLS_MAX_TXERS; i++)
			last_time_d[i] = 0.0;

		/* read and print data */
		while (error <= MB_ERROR_NO_ERROR)
			{
			/* reset error */
			error = MB_ERROR_NO_ERROR;

			/* read next data record */
			status = mb_get_all(verbose,
				imbio_ptr,
				&istore_ptr,
				&kind,
				time_i,
				&time_d,
				&navlon,
				&navlat,
				&speed,
				&heading,
				&distance,
				&altitude,
				&sonardepth,
				&beams_bath,
				&beams_amp,
				&pixels_ss,
				beamflag,
				bath,
				amp,
				bathacrosstrack,
				bathalongtrack,
				ss,
				ssacrosstrack,
				ssalongtrack,
				comment,
				&error);
			type = istore->type;

			/* some nonfatal errors do not matter */
			if ((error < MB_ERROR_NO_ERROR) && (error > MB_ERROR_UNINTELLIGIBLE))
				{
				error = MB_ERROR_NO_ERROR;
				status = MB_SUCCESS;
				}

			/* handle ping data */
			if ((status == MB_SUCCESS) && (kind == MB_DATA_DATA))
				{
				ping = &(istore->ping);
				time_d = istore->time_d;
				txidx = istore->ping.txno - 1;
				for (i = 0; i < 7; i++)
					time_i[i] = istore->time_i[i];
				if (istore->type == SWPLS_ID_XYZA_PING)
					{
					nrec_xyza_ping1[txidx]++;
					if (verbose > 0)
						fprintf(stderr,
							"SWPLS_ID_XYZA_PING:  %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d txno: %d txstat: %d num: %d\n",
							time_i[0],
							time_i[1],
							time_i[2],
							time_i[3],
							time_i[4],
							time_i[5],
							time_i[6],
							istore->ping.txno,
							istore->ping.txstat & SWPLS_SONAR_SEL_MASK,
							istore->ping.pingnumber);
					}
				if (istore->type == SWPLS_ID_XYZA_PING2)
					{
					nrec_xyza_ping2[txidx]++;
					if (verbose > 0)
						fprintf(stderr,
							"SWPLS_ID_XYZA_PING2: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d txno: %d txstat: %d num: %d\n",
							time_i[0],
							time_i[1],
							time_i[2],
							time_i[3],
							time_i[4],
							time_i[5],
							time_i[6],
							istore->ping.txno,
							istore->ping.txstat & SWPLS_SONAR_SEL_MASK,
							istore->ping.pingnumber);
					}

				/* filter out bad time stamps here */
				if (time_d <= last_time_d[txidx])
					{
					nrec_filtered_time++;
					if (verbose > 0)
						fprintf(stderr, ">>> FILTERED OUT DUE TO BAD TIME STAMP\n");
					continue;
					}
				else
					{
					last_time_d[txidx] = time_d;
					}


				/* count ping status */
				if ((istore->ping.txstat & SWPLS_SONAR_SEL_MASK) == SWPLS_SONAR_SEL_OFF)
					nrec_ping_sel_off++;
				else if ((istore->ping.txstat & SWPLS_SONAR_SEL_MASK) == SWPLS_SONAR_SEL_SINGLE)
					nrec_ping_sel_single++;
				else if ((istore->ping.txstat & SWPLS_SONAR_SEL_MASK) == SWPLS_SONAR_SEL_ALT)
					nrec_ping_sel_alt++;
				else if ((istore->ping.txstat & SWPLS_SONAR_SEL_MASK) == SWPLS_SONAR_SEL_SIM)
					nrec_ping_sel_sim++;

				/* check that we have an output file open */
				if (ofileinit[txidx] == MB_NO)
					{
					if (verbose >= 2)
						fprintf(stderr,
							"\ndbg2  ofileinit[%d] == MB_NO, opening new file <%s>\n",
							txidx,
							ofile[txidx]);

					/* initialize the output file */
					if ((status ==
						mb_write_init(verbose, ofile[txidx], format, &(ombio_ptr_arr[txidx]),
						&obeams_bath, &obeams_amp, &opixels_ss, &error)) != MB_SUCCESS)
						{
						mb_error(verbose, error, &message);
						fprintf(stderr,
							"\nMBIO Error returned from function <mb_write_init>:\n%s\n",
							message);
						fprintf(stderr,
							"\nSWATHplus file <%s> not initialized for writing\n",
							ofile[i]);
						fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
						exit(error);
						}
					ofileinit[txidx] = MB_YES;

					/* if we've got a file header, insert it at the top of the output file */
					if (stored_header == MB_YES)
						{
						if (verbose >= 2)
							fprintf(stderr, "\ndbg2  inserting header into <%s>\n", ofile[txidx]);

						/* temporarily change istore to a header type for writing */
						istore->header = *header;
						istore->kind = MB_DATA_HEADER;
						istore->type = SWPLS_ID_SXP_HEADER_DATA;

						/* write the header */
						status = mb_put_all(verbose,
							ombio_ptr_arr[txidx],
							istore_ptr,
							MB_NO,
							MB_DATA_HEADER,
							time_i,
							time_d,
							navlon,
							navlat,
							speed,
							heading,
							obeams_bath,
							obeams_amp,
							opixels_ss,
							beamflag,
							bath,
							amp,
							bathacrosstrack,
							bathalongtrack,
							ss,
							ssacrosstrack,
							ssalongtrack,
							comment,
							&error);
						if (status != MB_SUCCESS)
							{
							mb_error(verbose, error, &message);
							fprintf(stderr,
								"\nMBIO Error returned from function <mb_put>:\n%s\n",
								message);
							fprintf(stderr, "\nSWATHplus Data Not Written To File <%s>\n",
								ofile[txidx]);
							fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
							exit(error);
							}

						/* reset the istore kind to a ping */
						istore->kind = kind;
						istore->type = type;
						}

					nfile_write++;
					}

				/* write the ping data into the appropriate output file */
				status = mb_put_all(verbose,
					ombio_ptr_arr[txidx],
					istore_ptr,
					MB_NO,
					kind,
					time_i,
					time_d,
					navlon,
					navlat,
					speed,
					heading,
					obeams_bath,
					obeams_amp,
					opixels_ss,
					beamflag,
					bath,
					amp,
					bathacrosstrack,
					bathalongtrack,
					ss,
					ssacrosstrack,
					ssalongtrack,
					comment,
					&error);

				if (status != MB_SUCCESS)
					{
					mb_error(verbose, error, &message);
					fprintf(stderr, "\nMBIO Error returned from function <mb_put>:\n%s\n", message);
					fprintf(stderr, "\nSWATHplus Data Not Written To File <%s>\n", ofile[txidx]);
					fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
					exit(error);
					}
				}

			/* store file header data if present in input file */
			else if ((status == MB_SUCCESS) && (kind == MB_DATA_HEADER))
				{
				nrec_sxp_header++;
				header = &(istore->header);
				stored_header = MB_YES;
				if (verbose > 0)
					fprintf(stderr,
						"SWPLS_ID_SXP_HEADER_DATA:  swver: %d fmtver: %d\n",
						header->swver,
						header->fmtver);
				}

			/* handle unknown data */
			else if (status == MB_SUCCESS)
				{
				if (verbose > 0)
					fprintf(stderr,
						"DATA TYPE UNKNOWN: status:%d error:%d kind:%d\n",
						status,
						error,
						kind);
				nrec_other++;
				}

			/* handle read error */
			else if (verbose > 0)
				fprintf(stderr, "READ FAILURE: status:%d error:%d kind:%d\n", status, error, kind);

			/* print debug statements */
			if (verbose >= 2)
				{
				fprintf(stderr, "\ndbg2  Ping read in program <%s>\n", program_name);
				fprintf(stderr, "dbg2       kind:           %d\n", kind);
				fprintf(stderr, "dbg2       error:          %d\n", error);
				fprintf(stderr, "dbg2       status:         %d\n", status);
				}
			}

		/* output counts */
		fprintf(stdout, "\nData records read from: %s\n", ifile);
		fprintf(stdout, "     SXP File Header:                   %d\n", nrec_sxp_header);
		fprintf(stdout, "     XYZA_Ping1:\n");
		for (i = 0; i < MBSYS_SWPLS_MAX_TXERS; i++)
			fprintf(stdout, "         Txer %d:                        %d\n", i + 1,
				nrec_xyza_ping1[i]);
		fprintf(stdout, "     XYZA_PING2:\n");
		for (i = 0; i < MBSYS_SWPLS_MAX_TXERS; i++)
			fprintf(stdout, "         Txer %d:                        %d\n", i + 1,
				nrec_xyza_ping2[i]);
		fprintf(stdout, "     Other:                             %d\n", nrec_other);
		fprintf(stdout, "     Ping Mode Off:                     %d\n", nrec_ping_sel_off);
		fprintf(stdout, "     Ping Mode Single-sided:            %d\n", nrec_ping_sel_single);
		fprintf(stdout, "     Ping Mode Alternating:             %d\n", nrec_ping_sel_alt);
		fprintf(stdout, "     Ping Mode Simultaneous:            %d\n", nrec_ping_sel_sim);
		fprintf(stdout, "     Pings filtered for bad time:       %d\n", nrec_filtered_time);

		nrec_sxp_header_tot += nrec_sxp_header;
		for(i = 0; i < MBSYS_SWPLS_MAX_TXERS; i++)
			{
			nrec_xyza_ping1_tot[i] += nrec_xyza_ping1[i];
			nrec_xyza_ping2_tot[i] += nrec_xyza_ping2[i];
			}
		nrec_other_tot += nrec_other;
		nrec_ping_sel_off_tot += nrec_ping_sel_off;
		nrec_ping_sel_single_tot += nrec_ping_sel_single;
		nrec_ping_sel_alt_tot += nrec_ping_sel_alt;
		nrec_ping_sel_sim_tot += nrec_ping_sel_sim;
		nrec_filtered_time_tot += nrec_filtered_time;

		/* figure out whether and what to read next */
		if (read_datalist == MB_YES)
			{
			if ((status =
				mb_datalist_read(verbose, datalist, ifile, &format, &file_weight,
					&error)) == MB_SUCCESS)
				read_data = MB_YES;
			else
				read_data = MB_NO;
			}
		else
			{
			read_data = MB_NO;
			}

		/* close the input swath file */
		status = mb_close(verbose, &imbio_ptr, &error);

		/* close the output swath file if necessary */
		if ((ofile_set == MB_NO) || (read_data == MB_NO))
			for (i = 0; i < MBSYS_SWPLS_MAX_TXERS; i++)
				if (ofileinit[i] == MB_YES)
					status = mb_close(verbose, &ombio_ptr_arr[i], &error);

					/* generate inf fnv and fbt files (useless without copying prj files!) */
/*                    if (status == MB_SUCCESS) */
/*                        { */
/*                        status = mb_make_info(verbose, MB_YES, ofile[i], format, &error); */
/*                        } */

		/* end loop over files in list */
		}

	if (read_datalist == MB_YES)
		mb_datalist_close(verbose, &datalist, &error);

	/* output counts */
	fprintf(stdout, "\nTotal files read:    %d\n", nfile_read);
	fprintf(stdout, "Total files written: %d\n", nfile_write);
	fprintf(stdout, "\nTotal Data records read from: %s\n", read_file);
	fprintf(stdout, "     SXP File Header:                   %d\n", nrec_sxp_header_tot);
	fprintf(stdout, "     XYZA_Ping1:\n");
	for(i = 0; i < MBSYS_SWPLS_MAX_TXERS; i++)
		fprintf(stdout, "        Txer %d:                         %d\n", i+1,
			nrec_xyza_ping1_tot[i]);
	fprintf(stdout, "     XYZA_PING2:\n");
	for(i = 0; i < MBSYS_SWPLS_MAX_TXERS; i++)
		fprintf(stdout, "        Txer %d:                         %d\n", i+1,
			nrec_xyza_ping2_tot[i]);

	fprintf(stdout, "     Other:                             %d\n", nrec_other_tot);
	fprintf(stdout, "     Ping Mode Off:                     %d\n", nrec_ping_sel_off_tot);
	fprintf(stdout, "     Ping Mode Single-sided:            %d\n", nrec_ping_sel_single_tot);
	fprintf(stdout, "     Ping Mode Alternating:             %d\n", nrec_ping_sel_alt_tot);
	fprintf(stdout, "     Ping Mode Simultaneous:            %d\n", nrec_ping_sel_sim_tot);
	fprintf(stdout, "     Pings filtered for bad time:       %d\n", nrec_filtered_time_tot);

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose, &error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  Program <%s> completed\n", program_name);
		fprintf(stderr, "dbg2  Ending status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
		}

	/* end it all */
	exit(error);
} /* main */
/*--------------------------------------------------------------------*/
