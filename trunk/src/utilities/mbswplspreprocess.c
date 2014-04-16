/*--------------------------------------------------------------------
 *    The MB-system:	mbsxppreprocess.c	09/12/2013
 *    $Id$
 *
 *    Copyright (c) 2005-2013 by
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
 * mbswplspreprocess reads a BathySwath (formerly SWATHplus) sonar file
 * and prepairs an output file from these data that can be processed
 * with MB System.
 *
 * Currently, the following procedures are offered (all optional):
 *
 * 1. Split each transducer channel into its own file. This is the only
 *practical
 * way to handle all of the various ping modes and transducer configurations.
 *
 * 2. Strip rejected samples from pings. In some aquisition configurations,
 * the BathySwath records thousands of samples per ping, rejects them all,
 * and then re-saves hundreds more "processed" samples which have been subject
 * to filtering, statistical aggredation, and other desirable data reduction
 * processes. Effectively, the same data has been recorded twice with different
 * filter settings. Unfortunately, there is no flag to indicate which type
 * of sample we are dealing with and this causes all kinds of problems when
 * editing and plotting the pings with MB System later on. Not to mention
 * that the files can grow by a factor of 10 or more. It is best to strip
 * out these original data before MB System is used to edit the remaining
 * "processed" data.
 *
 * 3. Copy raw amplitude to the processed amplitude slot. SXP files store
 * both a raw and processed amplitude value for each sample in the ping.
 * MB Sytem works only with the processed amplitude slot. This option will
 * reset the processed amplitude slot back to the original raw value stored
 * in the file.
 *
 * 4. Print out the data packets to stdout. Useful for debugging.
 *
 * The code below is designed to also support SXI files, however much still
 * needs to be done. SXI files are closer in nature to a traditional multibeam
 * data format. However, the files do not carry ANY configuration information
 * and almost no filtering has been done on the samples. MB System will need
 * extensive additional filtering options to find the seafloor in such files.
 * I am not aware of anyone who has large collections of SXI files that need
 * processing which makes it a lower priority item for me and the USGS.
 *
 * Author:	D. P. Finlayson
 * Email:   dfinlayson@usgs.gov
 * Date:	Feb 28, 2013
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/* MBIO include files */
#include "mb_status.h"
#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mbsys_swathplus.h"

#define MAX_ERROR_STRING              1024

typedef struct mbdefaults_struct
	{
	int verbose;
	int format;
	int pings_get;
	int lonflip;
	int btime_i[7];
	int etime_i[7];
	double speedmin;
	double timegap;
	double bounds[4];
	} mbdefaults;

typedef struct options_struct
	{
	int errflg;
	int split_txers;
	int help;
	int verbose;
	int format;
	int ofile_set;
	int projection_set;
	int write_output;
	int print_ascii;
	int remove_rejected;
	int flip_rejected;
	int copy_rawamp;
	mb_path proj4command;
	mb_path read_file;
	mb_path basename;
	} options;

typedef struct counts_struct
	{
	int files_read;
	int sxpheader;
	int sxiheader;
	int projection;
	int sxpping1;
	int sxpping2;
	int sxiping;
	int attitude;
	int posll;
	int posen;
	int ssv;
	int echosounder;
	int tide;
	int agds;
	int comment;
	int pos_offset;
	int imu_offset;
	int txer_offset;
	int wl_offset;
	int other;
	int pings_per_txer[SWPLS_MAX_TXERS];
	int ping_sel_off;
	int ping_sel_single;
	int ping_sel_alt;
	int ping_sel_sim;
	} counts;

static void default_options(options *opts);
static int parse_options(int verbose, int argc, char **argv, options *opts,
	int *error);
static void error_exit(int verbose, int error, char *funcname, char *message);
static int process_output(int verbose, mbdefaults *mbdflts, options *opts,
	mb_path ifile, counts *recs, int *error);
static int remove_rejected_samps(int verbose, swpls_sxpping *ping, int *error);
static int flip_sample_flags(int verbose, swpls_sxpping *ping, int *error);
static int copy_rawamp(int verbose, swpls_sxpping *ping, int *error);
static int print_mbdefaults(int verbose, options *opts, mbdefaults *dflts,
	int *error);
static int set_outfile_names(int verbose, mb_path *ofile, mb_path ifile,
	mb_path *basename, int ofile_set, int split_txers,
	int *error);
static int ping_mode(int verbose, struct mbsys_swathplus_struct *store,
	int *mode, int *error);
static int ping_txno(int verbose, struct mbsys_swathplus_struct *store,
	int *txno, int *error);
static int zero_counts(int verbose, counts *c, int *error);
static int add_counts(int verbose, counts *to, counts *from, int *error);
static int print_counts(int verbose, counts *c, int *error);
static int count_record(int verbose, counts *c,
	struct mbsys_swathplus_struct *store, int *error);
static int print_latest_record(int verbose,
	struct mbsys_swathplus_struct *store,
	int *error);

static char help_message[] = 
	"Preprocess SWATHplus SXP formatted files\n" 
	"\n"
	"Options:\n"
	"-A        overwrite processed amplitude with raw\n"
	"          amplitude data.\n"
	"-B        flip flag on rejected/accepted samples.\n"
	"-Fformat  MB System format id\n"
	"-G        print data records to stdout\n"
	"-H        print this help text\n"
	"-Iinfile  SXP file to process\n"
	"-Jproj4   Proj4 projection command\n"
	"-N        do not write output to file, mostly usefull with -G\n"
	"-Ooutfile basename for output files [default: same as input]\n"
	"-R        remove rejected samples from pings.\n"
	"-S        split each transducer channel into a seperate file\n"
	"-V        verbosity\n"	
	"\n"
	"Report bugs to the MB System development team\n";

static char usage_message[] =
	"mbswplspreprocess [-ABGHNRSV -Fformat -Jproj4command-Obasename] -Ifile";

static char rcs_id[] = "$Id: mbswplspreprocess.c";
static char program_name[] = "mbswplspreprocess";

/*----------------------------------------------------------------------*/
int main(int argc, char **argv)
{
	/* MBIO status variables */
	int status = MB_SUCCESS;
	int error = MB_ERROR_NO_ERROR;

	/* MBIO read control parameters */
	int read_datalist = MB_NO;
	void *datalist;
	int look_processed = MB_DATALIST_LOOK_UNSET;
	double file_weight;
	mb_path ifile;

	/* MBIO read values */
	int read_data;

	/* counting variables */
	counts filerecs;
	counts totrecs;

	/* processing variables */
	options opts;
	mbdefaults mbdflts;

	/* set default options */
	default_options(&opts);

	/* mb_mem_debug_on(opts.verbose, &error); */

	/* get mbsystem default values */
	status = mb_defaults(opts.verbose, &(mbdflts.format), &(mbdflts.pings_get),
		&(mbdflts.lonflip), mbdflts.bounds, mbdflts.btime_i,
		mbdflts.etime_i, &(mbdflts.speedmin),
		&(mbdflts.timegap));

	if (status == MB_SUCCESS)
		{
		parse_options(opts.verbose, argc, argv, &opts, &error);
		}

	if (opts.errflg)
		{
		fprintf(stderr, "usage: %s\n", usage_message);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		error = MB_ERROR_BAD_USAGE;
		exit(error);
		}

	/* print starting debug statements */
	if (opts.verbose >= 1)
		{
		print_mbdefaults(opts.verbose, &opts, &mbdflts, &error);
		}

	/* if help desired then print it and exit */
	if (opts.help)
		{
		fprintf(stderr, "\nProgram %s\n", program_name);
		fprintf(stderr, "Version %s\n", rcs_id);
		fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "\nusage: %s\n", usage_message);
		fprintf(stderr, "\n%s\n", help_message);
		exit(error);
		}
		
	/* get format if required */
	if (opts.format == 0)
		{
		mb_get_format(opts.verbose, opts.read_file, NULL, &(opts.format),
			&error);
		}

	/* determine whether to read one file or a list of files */
	if (opts.format < 0)
		{
		read_datalist = MB_YES;
		}

	/* open file list */
	if (read_datalist == MB_YES)
		{
		if ((status =
			mb_datalist_open(opts.verbose, &datalist, opts.read_file,
				look_processed, &error)) != MB_SUCCESS)
			{
			char message[MAX_ERROR_STRING];
			sprintf(message, "Unable to open data list file: %s\n",
				opts.read_file);
			error_exit(opts.verbose, MB_ERROR_OPEN_FAIL, "mb_datalist_open",
				message);
			}

		if ((status =
			mb_datalist_read(opts.verbose, datalist, ifile, &(opts.format),
				&file_weight, &error)) == MB_SUCCESS)
			{
			read_data = MB_YES;
			}
		else
			{
			read_data = MB_NO;
			}
		}
	/* else copy single filename to be read */
	else
		{
		strcpy(ifile, opts.read_file);
		read_data = MB_YES;
		}

	/* reset total record counter */
	zero_counts(opts.verbose, &totrecs, &error);

	/* loop over files to be read */
	while (read_data == MB_YES)
		{
		/* reset file record counter */
		zero_counts(opts.verbose, &filerecs, &error);

		/* process the output files */
		if (status == MB_SUCCESS)
			{
			status = process_output(opts.verbose, &mbdflts, &opts, ifile,
				&filerecs, &error);
			}

		/* output counts */
		filerecs.files_read++;
		if (opts.verbose >= 1)
			{
			fprintf(stdout, "\nData records read from: %s\n", ifile);
			print_counts(opts.verbose, &filerecs, &error);
			}

		/* add this file's counts to total */
		add_counts(opts.verbose, &totrecs, &filerecs, &error);

		/* figure out whether and what to read next */
		if (read_datalist == MB_YES)
			{
			if ((status =
				mb_datalist_read(opts.verbose, datalist, ifile, &(opts.format),
					&file_weight, &error)) == MB_SUCCESS)
				{
				read_data = MB_YES;
				}
			else
				{
				read_data = MB_NO;
				}
			}
		else
			{
			read_data = MB_NO;
			}
		}		/* end loop over files in list */

	/* output counts */
	if (opts.verbose >= 1)
		{
		fprintf(stdout, "\nTotal data records read:\n");
		print_counts(opts.verbose, &totrecs, &error);
		}

	if (read_datalist == MB_YES)
		{
		mb_datalist_close(opts.verbose, &datalist, &error);
		}

	/* check memory */
	status = mb_memory_list(opts.verbose, &error);

	/* mb_mem_debug_off(opts.verbose, &error); */

	return (status);
}	/* main */
/*---------------------------------------------------------------*/
static void default_options(options *opts)
{
	/* standard mb system options */
	opts->errflg = 0;
	opts->help = MB_NO;
	opts->verbose = 0;

	/* transducer processing options */
	opts->split_txers = MB_NO;
	opts->remove_rejected = MB_NO;
	opts->flip_rejected = MB_NO;
	opts->copy_rawamp = MB_NO;

	/* map projection */
	opts->projection_set = MB_NO;
	strcpy(opts->proj4command, "");

	/* print ascii? */
	opts->print_ascii = MB_NO;

	/* input and output file names */
	opts->format = 0;
	opts->ofile_set = MB_NO;
	strcpy(opts->read_file, "datalist.mb-1");
	strcpy(opts->basename, "");
	opts->write_output = MB_YES;
}	/* default_options */
/*----------------------------------------------------------------------*/
static int parse_options(int verbose, int argc, char **argv, options *opts,
	int *error)
{
	char *function_name = "parse_options";
	int status = MB_SUCCESS;
	extern char *optarg;
	int c;
	int flag = 0;

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       argc:       %d\n", argc);
		fprintf(stderr, "dbg2       argv:       %p\n", (void *)argv);
		fprintf(stderr, "dbg2       options:    %p\n", (void *)opts);
		}

	while ((c = getopt(argc, argv, "AaBbF:f:GgHhI:i:J:j:NnO:o:RrSsVv")) != -1)
		{
		switch (c)
			{
		case 'A':
		case 'a':
			opts->copy_rawamp = MB_YES;
			break;
		case 'B':
		case 'b':
			opts->flip_rejected = MB_YES;
			break;
		case 'F':
		case 'f':
			sscanf(optarg, "%d", &(opts->format));
			flag++;
			break;
		case 'G':
		case 'g':
			opts->print_ascii = MB_YES;
			break;
		case 'H':
		case 'h':
			opts->help++;
			break;
		case 'I':
		case 'i':
			sscanf(optarg, "%s", &opts->read_file[0]);
			flag++;
			break;
		case 'J':
		case 'j':
			sscanf(optarg, "%s", &opts->proj4command[0]);
			opts->projection_set = MB_YES;
			flag++;
			break;
		case 'N':
		case 'n':
			opts->write_output = MB_NO;
			break;
		case 'O':
		case 'o':
			sscanf(optarg, "%s", &opts->basename[0]);
			opts->ofile_set = MB_YES;
			flag++;
			break;
		case 'R':
		case 'r':
			opts->remove_rejected = MB_YES;
			break;
		case 'S':
		case 's':
			opts->split_txers = MB_YES;
			break;
		case 'V':
		case 'v':
			opts->verbose++;
			break;
		case '?':
			opts->errflg++;
			}	/* switch */
		}

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2        error:     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	return (status);
}	/* parse_options */
/*---------------------------------------------------------------*/
static int print_mbdefaults(int verbose, options *opts, mbdefaults *dflts,
	int *error)
{
	char *function_name = "print_mbdefaults";
	char *tagdebug2 = "dbg2 ";
	char *tagdebug0 = "";
	char *tag = NULL;
	int status = MB_SUCCESS;

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       options:    %p\n", (void *)opts);
		}
		
	if (verbose > 1)
		tag = tagdebug2;
	else
		tag = tagdebug0;

	fprintf(stderr, "\n%sProgram <%s>\n", tag, program_name);
	fprintf(stderr, "%sVersion %s\n", tag, rcs_id);
	fprintf(stderr, "%sMB-system Version %s\n", tag, MB_VERSION);
	fprintf(stderr, "\n%sControl Parameters:\n", tag);
	fprintf(stderr, "%sverbose:                  %d\n", tag, opts->verbose);
	fprintf(stderr, "%shelp:                     %d\n", tag, opts->help);
	fprintf(stderr, "%sformat:                   %d\n", tag, opts->format);
	fprintf(stderr, "%slonflip:                  %d\n", tag, dflts->lonflip);
	fprintf(stderr, "%sbounds[0]:                %f\n", tag, dflts->bounds[0]);
	fprintf(stderr, "%sbounds[1]:                %f\n", tag, dflts->bounds[1]);
	fprintf(stderr, "%sbounds[2]:                %f\n", tag, dflts->bounds[2]);
	fprintf(stderr, "%sbounds[3]:                %f\n", tag, dflts->bounds[3]);
	fprintf(stderr, "%sbtime_i[0]:               %d\n", tag, dflts->btime_i[0]);
	fprintf(stderr, "%sbtime_i[1]:               %d\n", tag, dflts->btime_i[1]);
	fprintf(stderr, "%sbtime_i[2]:               %d\n", tag, dflts->btime_i[2]);
	fprintf(stderr, "%sbtime_i[3]:               %d\n", tag, dflts->btime_i[3]);
	fprintf(stderr, "%sbtime_i[4]:               %d\n", tag, dflts->btime_i[4]);
	fprintf(stderr, "%sbtime_i[5]:               %d\n", tag, dflts->btime_i[5]);
	fprintf(stderr, "%sbtime_i[6]:               %d\n", tag, dflts->btime_i[6]);
	fprintf(stderr, "%setime_i[0]:               %d\n", tag, dflts->etime_i[0]);
	fprintf(stderr, "%setime_i[1]:               %d\n", tag, dflts->etime_i[1]);
	fprintf(stderr, "%setime_i[2]:               %d\n", tag, dflts->etime_i[2]);
	fprintf(stderr, "%setime_i[3]:               %d\n", tag, dflts->etime_i[3]);
	fprintf(stderr, "%setime_i[4]:               %d\n", tag, dflts->etime_i[4]);
	fprintf(stderr, "%setime_i[5]:               %d\n", tag, dflts->etime_i[5]);
	fprintf(stderr, "%setime_i[6]:               %d\n", tag, dflts->etime_i[6]);
	fprintf(stderr, "%sspeedmin:  		     %f\n", tag, dflts->speedmin);
	fprintf(stderr, "%stimegap:  		     %f\n", tag, dflts->timegap);
	fprintf(stderr, "%sread_file: 		     %s\n", tag, opts->read_file);
	fprintf(stderr, "%sbasename: 		     %s\n", tag, opts->basename);
	fprintf(stderr, "%sofile_set:                %d\n", tag, opts->ofile_set);
	fprintf(stderr, "%sprojection_set:           %d\n", tag, opts->projection_set);
	fprintf(stderr, "%sproj4command:             %s\n", tag, opts->proj4command);
	fprintf(stderr, "%swrite_output:             %d\n", tag, opts->write_output);
	fprintf(stderr, "%sprint_ascii:              %d\n", tag, opts->print_ascii);
	fprintf(stderr, "%sremove_rejected:          %d\n", tag, opts->remove_rejected);
	fprintf(stderr, "%sflip_rejected:            %d\n", tag, opts->flip_rejected);
	fprintf(stderr, "%scopy_rawamp:              %d\n", tag, opts->copy_rawamp);

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2        error:     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	return (status);
}	/* print_mbdefaults */
/*---------------------------------------------------------------*/
static void error_exit(int verbose, int error, char *funcname, char *message)
{
	char *errmsg;

	mb_error(verbose, error, &errmsg);
	fprintf(stderr, "\nMBIO Error returned from function %s>:\n%s\n", funcname,
		errmsg);
	fprintf(stderr, "\n%s\n", message);
	fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
	exit(error);
}
/*---------------------------------------------------------------*/
static int process_output(int verbose, mbdefaults *mbdflts, options *opts,
	mb_path ifile, counts *recs, int *error)
{
	char *function_name = "scan_input_heights";
	int status = MB_SUCCESS;
	int i;
	void *imbio_ptr = NULL;
	double btime_d, etime_d;
	int beams_bath_alloc, beams_amp_alloc, pixels_ss_alloc;
	void *ombio_ptr[SWPLS_MAX_TXERS];
	struct mb_io_struct *imb_io_ptr = NULL;
	void *istore_ptr = NULL;
	int ofile_init[SWPLS_MAX_TXERS];
	mb_path ofile[SWPLS_MAX_TXERS];
	struct mbsys_swathplus_struct *istore = NULL;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       options:    %p\n", (void *)opts);
		}

	/* open the input file */
	if ((status =
		mb_read_init(opts->verbose, ifile, opts->format, mbdflts->pings_get,
			mbdflts->lonflip, mbdflts->bounds, mbdflts->btime_i,
			mbdflts->etime_i,
			mbdflts->speedmin, mbdflts->timegap, &imbio_ptr, &btime_d, &etime_d,
			&beams_bath_alloc, &beams_amp_alloc, &pixels_ss_alloc,
			error)) != MB_SUCCESS)
		{
		char message[MAX_ERROR_STRING] = {0};
		sprintf(message, "Swath File <%s> not initialized for reading\n",
			ifile);
		error_exit(opts->verbose, *error, "mb_read_init", message);
		}
		
	/* get mbio and data structure descriptors */
	imb_io_ptr = (struct mb_io_struct *)imbio_ptr;
	istore_ptr = imb_io_ptr->store_data;
	
	/* set the projection for nav data */
	if (opts->projection_set == MB_YES)
		{
		mb_proj_init(opts->verbose, opts->proj4command, &(imb_io_ptr->pjptr), error);
		strncpy(imb_io_ptr->projection_id, opts->proj4command, MB_NAME_LENGTH);
		imb_io_ptr->projection_initialized = MB_YES;		
		}
		
	/* setup the output filename(s) for writing */
	status = set_outfile_names(opts->verbose, ofile, ifile, &opts->basename,
		opts->ofile_set, opts->split_txers, error);
	for (i = 0; i < SWPLS_MAX_TXERS; i++)
		{
		ombio_ptr[i] = NULL;
		ofile_init[i] = MB_NO;
		}

	/* start looping over data records */
	while (*error <= MB_ERROR_NO_ERROR)
		{
		int kind = MB_DATA_NONE;

		/* read the next record */
		status =
			mb_read_ping(opts->verbose, imbio_ptr, istore_ptr, &kind, error);

		/* some nonfatal errors do not matter */
		if ((*error < MB_ERROR_NO_ERROR) && (MB_ERROR_UNINTELLIGIBLE < *error))
			{
			error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}

		istore = (struct mbsys_swathplus_struct *)istore_ptr;

		if (status == MB_SUCCESS)
			{
			status = count_record(opts->verbose, recs, istore, error);
			}

		if ((status == MB_SUCCESS) && (opts->print_ascii == MB_YES))
			{
			status = print_latest_record(opts->verbose, istore, error);
			}

		/* process the sxp ping data to file */
		if ((status == MB_SUCCESS) && (istore->kind == MB_DATA_DATA) &&
			((istore->type == SWPLS_ID_PROCESSED_PING) ||
			(istore->type == SWPLS_ID_PROCESSED_PING2)))
			{
			int obeams_bath, obeams_amp, opixels_ss;
			struct mb_io_struct *omb_io_ptr = NULL;
			void *ostore_ptr = NULL;
			struct mbsys_swathplus_struct *ostore = NULL;
			int txno = 0;
			int txidx = 0;

			if ((status == MB_SUCCESS) && (opts->flip_rejected == MB_YES))
				{
				status = flip_sample_flags(opts->verbose, &(istore->sxp_ping),
					error);
				}

			if ((status == MB_SUCCESS) && (opts->remove_rejected == MB_YES))
				{
				status = remove_rejected_samps(opts->verbose,
					&(istore->sxp_ping), error);
				}

			if ((status == MB_SUCCESS) && (opts->copy_rawamp == MB_YES))
				{
				status = copy_rawamp(opts->verbose, &(istore->sxp_ping), error);
				}

			if ((status == MB_SUCCESS) && (opts->write_output == MB_YES))
				{
				/* select the output file based on the txer channel */
				status = ping_txno(opts->verbose, istore, &txno, error);
				txidx = (opts->split_txers == MB_YES) ? txno - 1 : 0;

				/* initialize the output file if necessary */
				if (ofile_init[txidx] == MB_NO)
					{
					status = mb_write_init(opts->verbose, ofile[txidx], opts->format, 
						&ombio_ptr[txidx],  &obeams_bath,
						&obeams_amp, &opixels_ss, error);
					if (status != MB_SUCCESS)
						{
						char message[MAX_ERROR_STRING] = {0};
						sprintf(message, "SWATHplus file <%s> not initialized for writing.\n",
							ofile[txidx]);
						error_exit(verbose, *error, "mb_write_init", message);
						}

					if (status == MB_SUCCESS)
						{
						ofile_init[txidx] = MB_YES;
						}
					}

				/* assign output pointers based on txer channel */
				omb_io_ptr = (struct mb_io_struct *)ombio_ptr[txidx];
				ostore_ptr = omb_io_ptr->store_data;
				ostore = (struct mbsys_swathplus_struct *)ostore_ptr;

				/* copy the ping from istore to ostore */
				if (status == MB_SUCCESS)
					{
					status = mbsys_swathplus_copy(opts->verbose, imbio_ptr,
						istore_ptr, ostore_ptr,
						error);
					}

				/* write the ping to file */
				if (status == MB_SUCCESS)
					{
					ostore->kind = MB_DATA_DATA;
					ostore->type = SWPLS_ID_PROCESSED_PING2;
					status = mb_write_ping(opts->verbose, ombio_ptr[txidx],
						ostore, error);
					}

				/* check for error writing here */
				if (status != MB_SUCCESS)
					{
					char message[MAX_ERROR_STRING] = {0};
					sprintf(message, "Data not written to file <%s>\n",
						ofile[txidx]);
					error_exit(opts->verbose, *error, "mb_write_ping", message);
					}
				}		/* end write sxp data to file */
			}		/* end processing sxp data */
		}		/* end looping over all records in file */

	/* close the files */
	status = mb_close(opts->verbose, &imbio_ptr, error);
	for (i = 0; i < SWPLS_MAX_TXERS; i++)
		{
		if (ofile_init[i] == MB_YES)
			{
			status = mb_close(opts->verbose, &ombio_ptr[i], error);
			ofile_init[i] = MB_NO;
			}
		}

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2        error:     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	return (status);
} /* process_output */
/*----------------------------------------------------------------------*/
static int set_outfile_names(int verbose, mb_path *ofile, mb_path ifile,
	mb_path *basename, int ofile_set, int split_txers,
	int *error)
{
	char *function_name = "set_outfile_names";
	mb_path fileroot;
	int format;
	int status = MB_SUCCESS;
	int i;

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       ofile:      %p\n", (void *)ofile);
		fprintf(stderr, "dbg2       ifile:      %p\n", (void *)ifile);
		}

	/* clear ofile array */
	for (i = 0; i < SWPLS_MAX_TXERS; i++)
		{
		strncpy(ofile[i], "", sizeof(ofile[i]));
		}

	/* get the fileroot name and format from the input name */
	status = mb_get_format(verbose, ifile, fileroot, &format, error);

	if ((ofile_set == MB_NO) && (split_txers == MB_NO))
		{
		if ((format == MBF_SWPLSSXP) &&
			(strncmp(".sxp", &ifile[strlen(ifile) - 4], 4) == 0))
			{
			sprintf(ofile[0], "%s.mb%d", fileroot, format);
			}
		else if ((format == MBF_SWPLSSXI) &&
			(strncmp(".sxi", &ifile[strlen(ifile) - 4], 4) == 0))
			{
			sprintf(ofile[0], "%s.mb%d", fileroot, format);
			}
		else
			{
			sprintf(ofile[0], "%s.mb%d", ifile, format);
			}
		}
	else if ((ofile_set == MB_NO) && (split_txers == MB_YES))
		{
		if ((format == MBF_SWPLSSXP) &&
			(strncmp(".sxp", &ifile[strlen(ifile) - 4], 4) == 0))
			{
			for (i = 0; i < SWPLS_MAX_TXERS; i++)
				{
				sprintf(ofile[i], "%s_txer%d.mb%d", fileroot, i + 1, format);
				}
			}
		else if ((format == MBF_SWPLSSXI) &&
			(strncmp(".sxi", &ifile[strlen(ifile) - 4], 4) == 0))
			{
			for (i = 0; i < SWPLS_MAX_TXERS; i++)
				{
				sprintf(ofile[i], "%s_txer%d.mb%d", fileroot, i + 1, format);
				}
			}
		else
			{
			for (i = 0; i < SWPLS_MAX_TXERS; i++)
				{
				sprintf(ofile[i], "%s_txer%d.mb%d", ifile, i + 1, format);
				}
			}
		}
	else if ((ofile_set == MB_YES) && (split_txers == MB_NO))
		{
		if ((format == MBF_SWPLSSXP) &&
			(strncmp(".sxp", &ifile[strlen(ifile) - 4], 4) == 0))
			{
			sprintf(ofile[0], "%s.mb%d", *basename, format);
			}
		else if ((format == MBF_SWPLSSXI) &&
			(strncmp(".sxi", &ifile[strlen(ifile) - 4], 4) == 0))
			{
			sprintf(ofile[0], "%s.mb%d", *basename, format);
			}
		else
			{
			sprintf(ofile[0], "%s.mb%d", ifile, format);
			}
		}
	else if ((ofile_set == MB_YES) && (split_txers == MB_YES))
		{
		if ((format == MBF_SWPLSSXP) &&
			(strncmp(".sxp", &ifile[strlen(ifile) - 4], 4) == 0))
			{
			for (i = 0; i < SWPLS_MAX_TXERS; i++)
				{
				sprintf(ofile[i], "%s_txer%d.mb%d", *basename, i + 1, format);
				}
			}
		else if ((format == MBF_SWPLSSXI) &&
			(strncmp(".sxi", &ifile[strlen(ifile) - 4], 4) == 0))
			{
			for (i = 0; i < SWPLS_MAX_TXERS; i++)
				{
				sprintf(ofile[i], "%s_txer%d.mb%d", *basename, i + 1, format);
				}
			}
		else
			{
			for (i = 0; i < SWPLS_MAX_TXERS; i++)
				{
				sprintf(ofile[i], "%s_txer%d.mb%d", ifile, i + 1, format);
				}
			}
		}

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		for (i = 0; i < SWPLS_MAX_TXERS; i++)
			{
			fprintf(stderr, "dbg2    ofile[%d]:      %s\n", i, ofile[i]);
			}
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	return (status);
}	/* set_outfile_names */

/*----------------------------------------------------------------------*/
static int ping_txno(int verbose, struct mbsys_swathplus_struct *store,
	int *txno, int *error)
{
	char *function_name = "ping_txno";
	int status = MB_SUCCESS;

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		}

	/* get the transducer channe */
	if ((store->kind == MB_DATA_DATA) &&
		((store->type == SWPLS_ID_PROCESSED_PING) ||
		(store->type == SWPLS_ID_PROCESSED_PING2)))
		{
		*txno = store->sxp_ping.txno;
		}
	else if ((store->kind == MB_DATA_DATA) &&
		(store->type == SWPLS_ID_PARSED_PING))
		{
		*txno = store->sxi_ping.channel;
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
		}

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2        txno:      %d\n", *txno);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	return (status);
}	/* ping_txno */
/*----------------------------------------------------------------------*/
static int copy_rawamp(int verbose, swpls_sxpping *ping, int *error)
{
	char *function_name = "copy_rawamp";
	int status = MB_SUCCESS;
	int i;

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:     %d\n", verbose);
		fprintf(stderr, "dbg2       ping:        %p\n", (void *)ping);
		}

	for (i = 0; i < ping->nosampsfile; i++)
		{
		ping->points[i].procamp = ping->points[i].amp;
		}

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	return (status);
}	/* copy_rawamp */
/*----------------------------------------------------------------------*/
static int remove_rejected_samps(int verbose, swpls_sxpping *ping, int *error)
{
	char *function_name = "remove_rejected_samps";
	int status = MB_SUCCESS;
	swpls_point *points;
	int valid, i;

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:     %d\n", verbose);
		fprintf(stderr, "dbg2       ping:        %p\n", (void *)ping);
		}

	/* count the number of valid samples */
	valid = 0;
	for (i = 0; i < ping->nosampsfile; i++)
		{
		if (ping->points[i].status != SWPLS_POINT_REJECTED)
			{
			valid++;
			}
		}

	/* create a temporary array to hold the valid samples */
	status = mb_mallocd(verbose, __FILE__, __LINE__,
		valid * sizeof(swpls_point), (void **)&points, error);
	if (status != MB_SUCCESS)
		{
		char message[MAX_ERROR_STRING] = {0};
		sprintf(message,
			"Failure to allocate memory for temporary array (%lu bytes)",
			valid * sizeof(swpls_point));
		error_exit(verbose, *error, "mb_mallocd", message);
		}

	/* copy the valid samples to the temporary array */
	valid = 0;
	for (i = 0; i < ping->nosampsfile; i++)
		{
		if (ping->points[i].status != SWPLS_POINT_REJECTED)
			{
			points[valid++] = ping->points[i];
			}
		}

	/* copy the valid samples to the front of the ping->points array and adjust
	   the sample count. This effectively truncates the ping on write. */
	for (i = 0; i < valid; i++)
		{
		ping->points[i] = points[i];
		}
	ping->nosampsfile = valid;

	/* free memory for the temporary points array */
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)&points, error);

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	return (status);
}	/* remove_rejected_samps */
/*----------------------------------------------------------------------*/
static int flip_sample_flags(int verbose, swpls_sxpping *ping, int *error)
{
	char *function_name = "revcd";
	int status = MB_SUCCESS;
	int i;

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:     %d\n", verbose);
		fprintf(stderr, "dbg2       ping:        %p\n", (void *)ping);
		}

	for (i = 0; i < ping->nosampsfile; i++)
		{
		if (ping->points[i].status != SWPLS_POINT_REJECTED)
			{
			ping->points[i].status = SWPLS_POINT_REJECTED;
			}
		else
			{
			ping->points[i].status = SWPLS_POINT_ACCEPTED;
			}
		}

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	return (status);
}	/* flip_sample_flags */
/*----------------------------------------------------------------------*/
static int ping_mode(int verbose, struct mbsys_swathplus_struct *store,
	int *mode, int *error)
{
	char *function_name = "ping_mode";
	int status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		}

	/* use bitmask in ping status field to return ping mode */
	if (store->kind == MB_DATA_DATA)
		{
		if ((store->type == SWPLS_ID_PROCESSED_PING) ||
			(store->type == SWPLS_ID_PROCESSED_PING2))
			{
			*mode = store->sxp_ping.txstat & SWPLS_SONAR_SEL_MASK;
			}
		else if (store->type == SWPLS_ID_PARSED_PING)
			{
			*mode = store->sxi_ping.ping_state & SWPLS_SONAR_SEL_MASK;
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
		}
	else
		{
		/* this isn't a ping, can't get it's mode */
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
		}

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2        mode:      %d\n", *mode);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	return (status);
}	/* ping_mode */

/*----------------------------------------------------------------------
 * Functions for counting records (both sxp and sxi supported)
 *----------------------------------------------------------------------*/

static int zero_counts(int verbose, counts *recs, int *error)
{
	char *function_name = "zero_counts";
	int status = MB_SUCCESS;
	int i;

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       recs:       %p\n", (void *)recs);
		}

	recs->files_read = 0;
	recs->sxpheader = 0;
	recs->sxiheader = 0;
	recs->sxpping1 = 0;
	recs->sxpping2 = 0;
	recs->sxiping = 0;
	recs->attitude = 0;
	recs->posll = 0;
	recs->posen = 0;
	recs->ssv = 0;
	recs->echosounder = 0;
	recs->tide = 0;
	recs->agds = 0;
	recs->comment = 0;
	recs->pos_offset = 0;
	recs->imu_offset = 0;
	recs->txer_offset = 0;
	recs->wl_offset = 0;
	recs->other = 0;
	for (i = 0; i < SWPLS_MAX_TXERS; i++)
		{
		recs->pings_per_txer[i] = 0;
		}
	recs->ping_sel_off = 0;
	recs->ping_sel_single = 0;
	recs->ping_sel_alt = 0;
	recs->ping_sel_sim = 0;

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	return (status);
}	/* zero_counts */
/*---------------------------------------------------------------*/
static int count_record(int verbose, counts *recs,
	struct mbsys_swathplus_struct *store, int *error)
{
	char *function_name = "count_record";
	int status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       recs:       %p\n", (void *)recs);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		}

	/* count the record type */
	switch (store->type)
		{
	case SWPLS_ID_SXP_HEADER_DATA:
		recs->sxpheader++;
		break;
	case SWPLS_ID_PROCESSED_PING:
		recs->sxpping1++;
		recs->pings_per_txer[store->sxp_ping.txno - 1]++;
		break;
	case SWPLS_ID_PROCESSED_PING2:
		recs->sxpping2++;
		recs->pings_per_txer[store->sxp_ping.txno - 1]++;
		break;
	case SWPLS_ID_SXI_HEADER_DATA:
		recs->sxiheader++;
		break;
	case SWPLS_ID_PARSED_PING:
		recs->sxiping++;
		recs->pings_per_txer[store->sxi_ping.channel - 1]++;
		break;
	case SWPLS_ID_PARSED_ATTITUDE:
		recs->attitude++;
		break;
	case SWPLS_ID_PARSED_POSITION_LL:
		recs->posll++;
		break;
	case SWPLS_ID_PARSED_POSITION_EN:
		recs->posen++;
		break;
	case SWPLS_ID_PARSED_SSV:
		recs->ssv++;
		break;
	case SWPLS_ID_PARSED_ECHOSOUNDER:
		recs->echosounder++;
		break;
	case SWPLS_ID_PARSED_TIDE:
		recs->tide++;
		break;
	case SWPLS_ID_PARSED_AGDS:
		recs->agds++;
		break;
	case SWPLS_ID_COMMENT:
		recs->comment++;
		break;
	case SWPLS_ID_POS_OFFSET:
		recs->pos_offset++;
		break;
	case SWPLS_ID_IMU_OFFSET:
		recs->imu_offset++;
		break;
	case SWPLS_ID_TXER_OFFSET:
		recs->txer_offset++;
		break;
	case SWPLS_ID_WL_OFFSET:
		recs->wl_offset++;
		break;
	default:
		recs->other++;
		break;
		}	/* switch */

	if (store->kind == MB_DATA_DATA)
		{
		int mode;

		ping_mode(verbose, store, &mode, error);
		switch (mode)
			{
		case SWPLS_SONAR_SEL_OFF:
			recs->ping_sel_off++;
			break;
		case SWPLS_SONAR_SEL_SINGLE:
			recs->ping_sel_single++;
			break;
		case SWPLS_SONAR_SEL_ALT:
			recs->ping_sel_alt++;
			break;
		case SWPLS_SONAR_SEL_SIM:
			recs->ping_sel_sim++;
			break;
			}
		}

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	return (status);
}	/* count_record */
/*----------------------------------------------------------------------*/
static int add_counts(int verbose, counts *to, counts *from, int *error)
{
	char *function_name = "add_counts";
	int status = MB_SUCCESS;
	int i;

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       to:         %p\n", (void *)to);
		fprintf(stderr, "dbg2       from:       %p\n", (void *)from);
		}

	to->files_read += from->files_read;
	to->sxpheader += from->sxpheader;
	to->sxiheader += from->sxiheader;
	to->sxpping1 += from->sxpping1;
	to->sxpping2 += from->sxpping2;
	to->sxiping += from->sxiping;
	to->attitude += from->attitude;
	to->posll += from->posll;
	to->posen += from->posen;
	to->ssv += from->ssv;
	to->echosounder += from->echosounder;
	to->tide += from->tide;
	to->agds += from->agds;
	to->comment += from->comment;
	to->pos_offset += from->pos_offset;
	to->imu_offset += from->imu_offset;
	to->txer_offset += from->txer_offset;
	to->wl_offset += from->wl_offset;
	to->other += from->other;
	for (i = 0; i < SWPLS_MAX_TXERS; i++)
		{
		to->pings_per_txer[i] += from->pings_per_txer[i];
		}
	to->ping_sel_off += from->ping_sel_off;
	to->ping_sel_single += from->ping_sel_single;
	to->ping_sel_alt += from->ping_sel_alt;
	to->ping_sel_sim += from->ping_sel_sim;

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	return (status);
}	/* add_counts */
/*----------------------------------------------------------------------*/
static int print_counts(int verbose, counts *recs, int *error)
{
	char *function_name = "print_nrecs";
	int status = MB_SUCCESS;
	int i;

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       recs:       %p\n", (void *)recs);
		}

	fprintf(stdout, "\nFiles Read: %d\n", recs->files_read);
	fprintf(stdout, "\nData Records Read:\n");
	fprintf(stdout, "  sxpheader        = %d\n", recs->sxpheader);
	fprintf(stdout, "  sxiheader        = %d\n", recs->sxiheader);
	fprintf(stdout, "  sxpping1         = %d\n", recs->sxpping1);
	fprintf(stdout, "  sxpping2         = %d\n", recs->sxpping2);
	fprintf(stdout, "  sxiping          = %d\n", recs->sxiping);
	fprintf(stdout, "  attitude         = %d\n", recs->attitude);
	fprintf(stdout, "  posll            = %d\n", recs->posll);
	fprintf(stdout, "  posen            = %d\n", recs->posen);
	fprintf(stdout, "  ssv              = %d\n", recs->ssv);
	fprintf(stdout, "  echosounder      = %d\n", recs->echosounder);
	fprintf(stdout, "  tide             = %d\n", recs->tide);
	fprintf(stdout, "  agds             = %d\n", recs->agds);
	fprintf(stdout, "  comment          = %d\n", recs->comment);
	fprintf(stdout, "  pos_offset       = %d\n", recs->pos_offset);
	fprintf(stdout, "  imu_offset       = %d\n", recs->imu_offset);
	fprintf(stdout, "  txer_offset      = %d\n", recs->txer_offset);
	fprintf(stdout, "  wl_offset        = %d\n", recs->wl_offset);
	fprintf(stdout, "  other            = %d\n", recs->other);
	fprintf(stdout, "\nTransducers Observed:\n");
	for (i = 0; i < SWPLS_MAX_TXERS; i++)
		{
		fprintf(stdout, "  Channel %d        = %d\n", i + 1,
			recs->pings_per_txer[i]);
		}
	fprintf(stdout, "\nPing Modes Observed:\n");
	fprintf(stdout, "  Sonar Off        = %d\n", recs->ping_sel_off);
	fprintf(stdout, "  Single-Sided:    = %d\n", recs->ping_sel_single);
	fprintf(stdout, "  Alternate Sides: = %d\n", recs->ping_sel_alt);
	fprintf(stdout, "  Simultaneous:    = %d\n", recs->ping_sel_sim);

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	return (status);
}	/* print_counts */
/*----------------------------------------------------------------------*/
static int print_latest_record(int verbose,
	struct mbsys_swathplus_struct *store, int *error)
{
	char *function_name = "print_latest_record";
	int status = MB_SUCCESS;
	FILE *stream;

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
		}

	stream = (verbose > 0) ? stderr : stdout;

	if (store->type == SWPLS_ID_SXP_HEADER_DATA)
		{
		swpls_pr_sxpheader(verbose, stream, &(store->sxp_header), error);
		}
	else if (store->type == SWPLS_ID_PROJECTION)
		{
		swpls_pr_projection(verbose, stream, &(store->projection), error);
		}
	else if (store->type == SWPLS_ID_PROCESSED_PING)
		{
		swpls_pr_sxpping(verbose, stream, &(store->sxp_ping), error);
		}
	else if (store->type == SWPLS_ID_PROCESSED_PING2)
		{
		swpls_pr_sxpping(verbose, stream, &(store->sxp_ping), error);
		}
	else if (store->type == SWPLS_ID_SXI_HEADER_DATA)
		{
		swpls_pr_sxiheader(verbose, stream, &(store->sxi_header), error);
		}
	else if (store->type == SWPLS_ID_PARSED_PING)
		{
		swpls_pr_sxiping(verbose, stream, &(store->sxi_ping), error);
		}
	else if (store->type == SWPLS_ID_PARSED_ATTITUDE)
		{
		swpls_pr_attitude(verbose, stream, &(store->attitude), error);
		}
	else if (store->type == SWPLS_ID_PARSED_POSITION_LL)
		{
		swpls_pr_posll(verbose, stream, &(store->posll), error);
		}
	else if (store->type == SWPLS_ID_PARSED_POSITION_EN)
		{
		swpls_pr_posen(verbose, stream, &(store->posen), error);
		}
	else if (store->type == SWPLS_ID_PARSED_SSV)
		{
		swpls_pr_ssv(verbose, stream, &(store->ssv), error);
		}
	else if (store->type == SWPLS_ID_PARSED_ECHOSOUNDER)
		{
		swpls_pr_echosounder(verbose, stream, &(store->echosounder), error);
		}
	else if (store->type == SWPLS_ID_PARSED_TIDE)
		{
		swpls_pr_tide(verbose, stream, &(store->tide), error);
		}
	else if (store->type == SWPLS_ID_PARSED_AGDS)
		{
		swpls_pr_agds(verbose, stream, &(store->agds), error);
		}
	else if (store->type == SWPLS_ID_COMMENT)
		{
		swpls_pr_comment(verbose, stream, &(store->comment), error);
		}
	else if (store->type == SWPLS_ID_POS_OFFSET)
		{
		swpls_pr_pos_offset(verbose, stream, &(store->pos_offset), error);
		}
	else if (store->type == SWPLS_ID_IMU_OFFSET)
		{
		swpls_pr_imu_offset(verbose, stream, &(store->imu_offset), error);
		}
	else if (store->type == SWPLS_ID_TXER_OFFSET)
		{
		swpls_pr_txer_offset(verbose, stream, &(store->txer_offset), error);
		}
	else if (store->type == SWPLS_ID_WL_OFFSET)
		{
		swpls_pr_wl_offset(verbose, stream, &(store->wl_offset), error);
		}
	else
		{
		fprintf(stream, "UNKNOWN RECORD [ID: 0x%o]\n", store->type);
		}

	if (verbose >= 2)
		{
		fprintf(stderr, "\ndbg2  function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
		}

	return (status);
}	/* print_latest_record */

