/*--------------------------------------------------------------------
 *    The MB-system:	mbsxppreprocess.c	09/12/2013
 *
 *    Copyright (c) 2005-2023 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes 
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *     
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
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
 * 2. Strip rejected samples from pings. In some acquisition configurations,
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
 * MB System works only with the processed amplitude slot. This option will
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
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <getopt.h>
#include <unistd.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mbsys_swathplus.h"

typedef struct mbdefaults_struct {
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

typedef struct options_struct {
	int errflg;
	bool split_txers;
	bool help;
	int verbose;
	int format;
	bool ofile_set;
	bool projection_set;
	bool write_output;
	bool print_ascii;
	bool remove_rejected;
	bool flip_rejected;
	bool copy_rawamp;
	mb_path proj4command;
	mb_path read_file;
	mb_path basename;
} options;

typedef struct counts_struct {
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

constexpr char help_message[] =
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
	"-N        do not write output to file, mostly useful with -G\n"
	"-Ooutfile basename for output files [default: same as input]\n"
	"-R        remove rejected samples from pings.\n"
	"-S        split each transducer channel into a separate file\n"
	"-V        verbosity\n"
	"\n"
	"Report bugs to the MB System development team\n";

constexpr char usage_message[] = "mbswplspreprocess [-ABGHNRSV -Fformat -Jproj4command -Obasename] -Ifile";
constexpr char program_name[] = "mbswplspreprocess";

/*---------------------------------------------------------------*/
static void default_options(options *opts) {
	/* standard mb system options */
	opts->errflg = 0;
	opts->help = false;
	opts->verbose = 0;

	/* transducer processing options */
	opts->split_txers = false;
	opts->remove_rejected = false;
	opts->flip_rejected = false;
	opts->copy_rawamp = false;

	/* map projection */
	opts->projection_set = false;
	strcpy(opts->proj4command, "");

	/* print ascii? */
	opts->print_ascii = false;

	/* input and output file names */
	opts->format = 0;
	opts->ofile_set = false;
	strcpy(opts->read_file, "datalist.mb-1");
	strcpy(opts->basename, "");
	opts->write_output = true;
} /* default_options */
/*----------------------------------------------------------------------*/
static int parse_options(int verbose, int argc, char **argv, options *opts, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       argc:       %d\n", argc);
		fprintf(stderr, "dbg2       argv:       %p\n", (void *)argv);
		fprintf(stderr, "dbg2       options:    %p\n", (void *)opts);
	}

	int c;
	while ((c = getopt(argc, argv, "AaBbF:f:GgHhI:i:J:j:NnO:o:RrSsVv")) != -1) {
		switch (c) {
		case 'A':
		case 'a':
			opts->copy_rawamp = true;
			break;
		case 'B':
		case 'b':
			opts->flip_rejected = true;
			break;
		case 'F':
		case 'f':
			sscanf(optarg, "%d", &(opts->format));
			break;
		case 'G':
		case 'g':
			opts->print_ascii = true;
			break;
		case 'H':
		case 'h':
			opts->help = true;
			break;
		case 'I':
		case 'i':
			sscanf(optarg, "%1023s", &opts->read_file[0]);
			break;
		case 'J':
		case 'j':
			sscanf(optarg, "%1023s", &opts->proj4command[0]);
			opts->projection_set = true;
			break;
		case 'N':
		case 'n':
			opts->write_output = false;
			break;
		case 'O':
		case 'o':
			sscanf(optarg, "%1023s", &opts->basename[0]);
			opts->ofile_set = true;
			break;
		case 'R':
		case 'r':
			opts->remove_rejected = true;
			break;
		case 'S':
		case 's':
			opts->split_txers = true;
			break;
		case 'V':
		case 'v':
			opts->verbose++;
			break;
		case '?':
			opts->errflg++;
		} /* switch */
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2        error:     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", MB_SUCCESS);
	}

	return (MB_SUCCESS);
} /* parse_options */
/*---------------------------------------------------------------*/
static int print_mbdefaults(int verbose, options *opts, mbdefaults *dflts, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       options:    %p\n", (void *)opts);
	}

	const char *tagdebug2 = "dbg2 ";
	const char *tagdebug0 = "";
	const char *tag = verbose > 1 ? tagdebug2 : tagdebug0;

	fprintf(stderr, "\n%sProgram <%s>\n", tag, program_name);
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

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2        error:     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
} /* print_mbdefaults */
/*---------------------------------------------------------------*/
static void error_exit(int verbose, int error, const char *funcname, const char *message) {
	char *errmsg;

	mb_error(verbose, error, &errmsg);
	fprintf(stderr, "\nMBIO Error returned from function <%s>:\n%s\n", funcname, errmsg);
	fprintf(stderr, "\n%s\n", message);
	fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
	exit(error);
}
/*----------------------------------------------------------------------*/
static int print_latest_record(int verbose, struct mbsys_swathplus_struct *store, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
	}

	FILE *stream = verbose > 0 ? stderr : stdout;

	if (store->type == SWPLS_ID_SXP_HEADER_DATA) {
		swpls_pr_sxpheader(verbose, stream, &(store->sxp_header), error);
	}
	else if (store->type == SWPLS_ID_PROJECTION) {
		swpls_pr_projection(verbose, stream, &(store->projection), error);
	}
	else if (store->type == SWPLS_ID_PROCESSED_PING) {
		swpls_pr_sxpping(verbose, stream, &(store->sxp_ping), error);
	}
	else if (store->type == SWPLS_ID_PROCESSED_PING2) {
		swpls_pr_sxpping(verbose, stream, &(store->sxp_ping), error);
	}
	else if (store->type == SWPLS_ID_SXI_HEADER_DATA) {
		swpls_pr_sxiheader(verbose, stream, &(store->sxi_header), error);
	}
	else if (store->type == SWPLS_ID_PARSED_PING) {
		swpls_pr_sxiping(verbose, stream, &(store->sxi_ping), error);
	}
	else if (store->type == SWPLS_ID_PARSED_ATTITUDE) {
		swpls_pr_attitude(verbose, stream, &(store->attitude), error);
	}
	else if (store->type == SWPLS_ID_PARSED_POSITION_LL) {
		swpls_pr_posll(verbose, stream, &(store->posll), error);
	}
	else if (store->type == SWPLS_ID_PARSED_POSITION_EN) {
		swpls_pr_posen(verbose, stream, &(store->posen), error);
	}
	else if (store->type == SWPLS_ID_PARSED_SSV) {
		swpls_pr_ssv(verbose, stream, &(store->ssv), error);
	}
	else if (store->type == SWPLS_ID_PARSED_ECHOSOUNDER) {
		swpls_pr_echosounder(verbose, stream, &(store->echosounder), error);
	}
	else if (store->type == SWPLS_ID_PARSED_TIDE) {
		swpls_pr_tide(verbose, stream, &(store->tide), error);
	}
	else if (store->type == SWPLS_ID_PARSED_AGDS) {
		swpls_pr_agds(verbose, stream, &(store->agds), error);
	}
	else if (store->type == SWPLS_ID_COMMENT) {
		swpls_pr_comment(verbose, stream, &(store->comment), error);
	}
	else if (store->type == SWPLS_ID_POS_OFFSET) {
		swpls_pr_pos_offset(verbose, stream, &(store->pos_offset), error);
	}
	else if (store->type == SWPLS_ID_IMU_OFFSET) {
		swpls_pr_imu_offset(verbose, stream, &(store->imu_offset), error);
	}
	else if (store->type == SWPLS_ID_TXER_OFFSET) {
		swpls_pr_txer_offset(verbose, stream, &(store->txer_offset), error);
	}
	else if (store->type == SWPLS_ID_WL_OFFSET) {
		swpls_pr_wl_offset(verbose, stream, &(store->wl_offset), error);
	}
	else {
		fprintf(stream, "UNKNOWN RECORD [ID: 0x%o]\n", store->type);
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
} /* print_latest_record */

/*----------------------------------------------------------------------*/
static int ping_mode(int verbose, struct mbsys_swathplus_struct *store, int *mode, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
	}

	int status = MB_SUCCESS;

	/* use bitmask in ping status field to return ping mode */
	if (store->kind == MB_DATA_DATA) {
		if ((store->type == SWPLS_ID_PROCESSED_PING) || (store->type == SWPLS_ID_PROCESSED_PING2)) {
			*mode = store->sxp_ping.txstat & SWPLS_SONAR_SEL_MASK;
		}
		else if (store->type == SWPLS_ID_PARSED_PING) {
			*mode = store->sxi_ping.ping_state & SWPLS_SONAR_SEL_MASK;
		}
		else {
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
		}
	}
	else {
		/* this isn't a ping, can't get it's mode */
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2        mode:      %d\n", *mode);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
} /* ping_mode */

/*---------------------------------------------------------------*/
static int count_record(int verbose, counts *recs, struct mbsys_swathplus_struct *store, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       recs:       %p\n", (void *)recs);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
	}

	/* count the record type */
	switch (store->type) {
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
	} /* switch */

	if (store->kind == MB_DATA_DATA) {
		int mode;

		ping_mode(verbose, store, &mode, error);
		switch (mode) {
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

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
} /* count_record */

/*----------------------------------------------------------------------*/
static int flip_sample_flags(int verbose, swpls_sxpping *ping, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:     %d\n", verbose);
		fprintf(stderr, "dbg2       ping:        %p\n", (void *)ping);
	}

	for (int i = 0; i < ping->nosampsfile; i++) {
		if (ping->points[i].status != SWPLS_POINT_REJECTED) {
			ping->points[i].status = SWPLS_POINT_REJECTED;
		}
		else {
			ping->points[i].status = SWPLS_POINT_ACCEPTED;
		}
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
} /* flip_sample_flags */

/*----------------------------------------------------------------------*/
static int remove_rejected_samps(int verbose, swpls_sxpping *ping, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:     %d\n", verbose);
		fprintf(stderr, "dbg2       ping:        %p\n", (void *)ping);
	}

	/* count the number of valid samples */
	int valid = 0;
	for (int i = 0; i < ping->nosampsfile; i++) {
		if (ping->points[i].status != SWPLS_POINT_REJECTED) {
			valid++;
		}
	}

	/* create a temporary array to hold the valid samples */
	swpls_point *points;
	int status = mb_mallocd(verbose, __FILE__, __LINE__, valid * sizeof(swpls_point), (void **)&points, error);
	if (status != MB_SUCCESS) {
		mb_path message = {0};
		snprintf(message, sizeof(message), "Failure to allocate memory for temporary array (%lu bytes)", valid * sizeof(swpls_point));
		error_exit(verbose, *error, "mb_mallocd", message);
	}

	/* copy the valid samples to the temporary array */
	valid = 0;
	for (int i = 0; i < ping->nosampsfile; i++) {
		if (ping->points[i].status != SWPLS_POINT_REJECTED) {
			points[valid++] = ping->points[i];
		}
	}

	/* copy the valid samples to the front of the ping->points array and adjust
	   the sample count. This effectively truncates the ping on write. */
	for (int i = 0; i < valid; i++) {
		ping->points[i] = points[i];
	}
	ping->nosampsfile = valid;

	/* free memory for the temporary points array */
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)&points, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
} /* remove_rejected_samps */

/*----------------------------------------------------------------------*/
static int set_outfile_names(int verbose, mb_pathplus *ofile, mb_path ifile, mb_path *basename, bool ofile_set, bool split_txers,
                             int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       ofile:      %p\n", (void *)ofile);
		fprintf(stderr, "dbg2       ifile:      %p\n", (void *)ifile);
	}

	/* clear ofile array */
	for (int i = 0; i < SWPLS_MAX_TXERS; i++) {
		strncpy(ofile[i], "", sizeof(ofile[i]));
	}

	/* get the fileroot name and format from the input name */
	mb_path fileroot;
	int format;
	int status = mb_get_format(verbose, ifile, fileroot, &format, error);

	if (!ofile_set && !split_txers) {
		if ((format == MBF_SWPLSSXP) && (strncmp(".sxp", &ifile[strlen(ifile) - 4], 4) == 0)) {
			snprintf(ofile[0], sizeof(ofile[0]), "%s.mb%d", fileroot, format);
		}
		else if ((format == MBF_SWPLSSXI) && (strncmp(".sxi", &ifile[strlen(ifile) - 4], 4) == 0)) {
			snprintf(ofile[0], sizeof(ofile[0]), "%s.mb%d", fileroot, format);
		}
		else {
			snprintf(ofile[0], sizeof(ofile[0]), "%s.mb%d", ifile, format);
		}
	}
	else if (!ofile_set && split_txers) {
		if ((format == MBF_SWPLSSXP) && (strncmp(".sxp", &ifile[strlen(ifile) - 4], 4) == 0)) {
			for (int i = 0; i < SWPLS_MAX_TXERS; i++) {
				snprintf(ofile[i], sizeof(ofile[i]), "%s_txer%d.mb%d", fileroot, i + 1, format);
			}
		}
		else if ((format == MBF_SWPLSSXI) && (strncmp(".sxi", &ifile[strlen(ifile) - 4], 4) == 0)) {
			for (int i = 0; i < SWPLS_MAX_TXERS; i++) {
				snprintf(ofile[i], sizeof(ofile[i]), "%s_txer%d.mb%d", fileroot, i + 1, format);
			}
		}
		else {
			for (int i = 0; i < SWPLS_MAX_TXERS; i++) {
				snprintf(ofile[i], sizeof(ofile[i]), "%s_txer%d.mb%d", ifile, i + 1, format);
			}
		}
	}
	else if (ofile_set && !split_txers) {
		if ((format == MBF_SWPLSSXP) && (strncmp(".sxp", &ifile[strlen(ifile) - 4], 4) == 0)) {
			snprintf(ofile[0], sizeof(ofile[0]), "%s.mb%d", *basename, format);
		}
		else if ((format == MBF_SWPLSSXI) && (strncmp(".sxi", &ifile[strlen(ifile) - 4], 4) == 0)) {
			snprintf(ofile[0], sizeof(ofile[0]), "%s.mb%d", *basename, format);
		}
		else {
			snprintf(ofile[0], sizeof(ofile[0]), "%s.mb%d", ifile, format);
		}
	}
	else if (ofile_set && split_txers) {
		if ((format == MBF_SWPLSSXP) && (strncmp(".sxp", &ifile[strlen(ifile) - 4], 4) == 0)) {
			for (int i = 0; i < SWPLS_MAX_TXERS; i++) {
				snprintf(ofile[i], sizeof(ofile[i]), "%s_txer%d.mb%d", *basename, i + 1, format);
			}
		}
		else if ((format == MBF_SWPLSSXI) && (strncmp(".sxi", &ifile[strlen(ifile) - 4], 4) == 0)) {
			for (int i = 0; i < SWPLS_MAX_TXERS; i++) {
				snprintf(ofile[i], sizeof(ofile[i]), "%s_txer%d.mb%d", *basename, i + 1, format);
			}
		}
		else {
			for (int i = 0; i < SWPLS_MAX_TXERS; i++) {
				snprintf(ofile[i], sizeof(ofile[i]), "%s_txer%d.mb%d", ifile, i + 1, format);
			}
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		for (int i = 0; i < SWPLS_MAX_TXERS; i++) {
			fprintf(stderr, "dbg2    ofile[%d]:      %s\n", i, ofile[i]);
		}
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
} /* set_outfile_names */

/*----------------------------------------------------------------------*/
static int ping_txno(int verbose, struct mbsys_swathplus_struct *store, int *txno, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       store:      %p\n", (void *)store);
	}

	int status = MB_SUCCESS;

	/* get the transducer channe */
	if ((store->kind == MB_DATA_DATA) &&
	    ((store->type == SWPLS_ID_PROCESSED_PING) || (store->type == SWPLS_ID_PROCESSED_PING2))) {
		*txno = store->sxp_ping.txno;
	}
	else if ((store->kind == MB_DATA_DATA) && (store->type == SWPLS_ID_PARSED_PING)) {
		*txno = store->sxi_ping.channel;
	}
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2        txno:      %d\n", *txno);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
} /* ping_txno */

/*----------------------------------------------------------------------*/
static int copy_rawamp(int verbose, swpls_sxpping *ping, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:     %d\n", verbose);
		fprintf(stderr, "dbg2       ping:        %p\n", (void *)ping);
	}

	for (int i = 0; i < ping->nosampsfile; i++) {
		ping->points[i].procamp = ping->points[i].amp;
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
} /* copy_rawamp */

/*---------------------------------------------------------------*/
static int process_output(int verbose, mbdefaults *mbdflts, options *opts, mb_path ifile, counts *recs, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       options:    %p\n", (void *)opts);
	}

	int status = MB_SUCCESS;

	void *imbio_ptr = nullptr;
	double btime_d;
	double etime_d;
	int beams_bath_alloc;
	int beams_amp_alloc;
	int pixels_ss_alloc;
	mb_pathplus ofile[SWPLS_MAX_TXERS];
	struct mbsys_swathplus_struct *istore = nullptr;

	/* open the input file */
	if (mb_read_init(opts->verbose, ifile, opts->format, mbdflts->pings_get, mbdflts->lonflip, mbdflts->bounds,
	                           mbdflts->btime_i, mbdflts->etime_i, mbdflts->speedmin, mbdflts->timegap, &imbio_ptr, &btime_d,
	                           &etime_d, &beams_bath_alloc, &beams_amp_alloc, &pixels_ss_alloc, error) != MB_SUCCESS) {
		mb_pathplus message = "";
		snprintf(message, sizeof(message), "Swath File <%s> not initialized for reading\n", ifile);
		error_exit(opts->verbose, *error, "mb_read_init", message);
	}

	/* get mbio and data structure descriptors */
	struct mb_io_struct *imb_io_ptr = (struct mb_io_struct *)imbio_ptr;
	void *istore_ptr = imb_io_ptr->store_data;

	/* set the projection for nav data */
	if (opts->projection_set) {
		mb_proj_init(opts->verbose, opts->proj4command, &(imb_io_ptr->pjptr), error);
		strncpy(imb_io_ptr->projection_id, opts->proj4command, MB_NAME_LENGTH);
		imb_io_ptr->projection_initialized = true;
	}

	/* setup the output filename(s) for writing */
	status = set_outfile_names(opts->verbose, ofile, ifile, &opts->basename, opts->ofile_set, opts->split_txers, error);

	bool ofile_init[SWPLS_MAX_TXERS];
	void *ombio_ptr[SWPLS_MAX_TXERS];
	for (int i = 0; i < SWPLS_MAX_TXERS; i++) {
		ombio_ptr[i] = nullptr;
		ofile_init[i] = false;
	}

	/* start looping over data records */
	while (*error <= MB_ERROR_NO_ERROR) {
		int kind = MB_DATA_NONE;

		/* read the next record */
		status = mb_read_ping(opts->verbose, imbio_ptr, istore_ptr, &kind, error);

		/* some nonfatal errors do not matter */
		if ((*error < MB_ERROR_NO_ERROR) && (MB_ERROR_UNINTELLIGIBLE < *error)) {
			error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}

		istore = (struct mbsys_swathplus_struct *)istore_ptr;

		if (status == MB_SUCCESS) {
			status = count_record(opts->verbose, recs, istore, error);
		}

		if (status == MB_SUCCESS && opts->print_ascii) {
			status = print_latest_record(opts->verbose, istore, error);
		}

		/* process the sxp ping data to file */
		if ((status == MB_SUCCESS) && (istore->kind == MB_DATA_DATA) &&
		    ((istore->type == SWPLS_ID_PROCESSED_PING) || (istore->type == SWPLS_ID_PROCESSED_PING2))) {
			int obeams_bath, obeams_amp, opixels_ss;
			struct mb_io_struct *omb_io_ptr = nullptr;
			void *ostore_ptr = nullptr;
			struct mbsys_swathplus_struct *ostore = nullptr;
			int txno = 0;
			int txidx = 0;

			if (status == MB_SUCCESS && opts->flip_rejected) {
				status = flip_sample_flags(opts->verbose, &(istore->sxp_ping), error);
			}

			if (status == MB_SUCCESS && opts->remove_rejected) {
				status = remove_rejected_samps(opts->verbose, &(istore->sxp_ping), error);
			}

			if (status == MB_SUCCESS && opts->copy_rawamp) {
				status = copy_rawamp(opts->verbose, &(istore->sxp_ping), error);
			}

			if (status == MB_SUCCESS && opts->write_output) {
				/* select the output file based on the txer channel */
				status = ping_txno(opts->verbose, istore, &txno, error);
				txidx = opts->split_txers ? txno - 1 : 0;

				/* initialize the output file if necessary */
				if (!ofile_init[txidx]) {
					status = mb_write_init(opts->verbose, ofile[txidx], opts->format, &ombio_ptr[txidx], &obeams_bath,
					                       &obeams_amp, &opixels_ss, error);
					if (status != MB_SUCCESS) {
						char message[MB_PATHPLUS_MAXLINE] = {0};
						snprintf(message, sizeof(message), "SWATHplus file <%s> not initialized for writing.\n", ofile[txidx]);
						error_exit(verbose, *error, "mb_write_init", message);
					}

					if (status == MB_SUCCESS) {
						ofile_init[txidx] = true;
					}
				}

				/* assign output pointers based on txer channel */
				omb_io_ptr = (struct mb_io_struct *)ombio_ptr[txidx];
				ostore_ptr = omb_io_ptr->store_data;
				ostore = (struct mbsys_swathplus_struct *)ostore_ptr;

				/* copy the ping from istore to ostore */
				if (status == MB_SUCCESS) {
					status = mbsys_swathplus_copy(opts->verbose, imbio_ptr, istore_ptr, ostore_ptr, error);
				}

				/* write the ping to file */
				if (status == MB_SUCCESS) {
					ostore->kind = MB_DATA_DATA;
					ostore->type = SWPLS_ID_PROCESSED_PING2;
					status = mb_write_ping(opts->verbose, ombio_ptr[txidx], ostore, error);
				}

				/* check for error writing here */
				if (status != MB_SUCCESS) {
					char message[MB_PATHPLUS_MAXLINE] = {0};
					snprintf(message, sizeof(message), "Data not written to file <%s>\n", ofile[txidx]);
					error_exit(opts->verbose, *error, "mb_write_ping", message);
				}
			} /* end write sxp data to file */
		}     /* end processing sxp data */
	}         /* end looping over all records in file */

	/* close the files */
	status = mb_close(opts->verbose, &imbio_ptr, error);
	for (int i = 0; i < SWPLS_MAX_TXERS; i++) {
		if (ofile_init[i]) {
			status = mb_close(opts->verbose, &ombio_ptr[i], error);
			ofile_init[i] = false;
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2        error:     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
} /* process_output */


/*----------------------------------------------------------------------
 * Functions for counting records (both sxp and sxi supported)
 *----------------------------------------------------------------------*/

static int zero_counts(int verbose, counts *recs, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  function <%s> called\n", __func__);
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
	for (int i = 0; i < SWPLS_MAX_TXERS; i++) {
		recs->pings_per_txer[i] = 0;
	}
	recs->ping_sel_off = 0;
	recs->ping_sel_single = 0;
	recs->ping_sel_alt = 0;
	recs->ping_sel_sim = 0;

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
} /* zero_counts */
/*----------------------------------------------------------------------*/
static int add_counts(int verbose, counts *to, counts *from, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  function <%s> called\n", __func__);
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
	for (int i = 0; i < SWPLS_MAX_TXERS; i++) {
		to->pings_per_txer[i] += from->pings_per_txer[i];
	}
	to->ping_sel_off += from->ping_sel_off;
	to->ping_sel_single += from->ping_sel_single;
	to->ping_sel_alt += from->ping_sel_alt;
	to->ping_sel_sim += from->ping_sel_sim;

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
} /* add_counts */
/*----------------------------------------------------------------------*/
static int print_counts(int verbose, counts *recs, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  function <%s> called\n", __func__);
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
	for (int i = 0; i < SWPLS_MAX_TXERS; i++) {
		fprintf(stdout, "  Channel %d        = %d\n", i + 1, recs->pings_per_txer[i]);
	}
	fprintf(stdout, "\nPing Modes Observed:\n");
	fprintf(stdout, "  Sonar Off        = %d\n", recs->ping_sel_off);
	fprintf(stdout, "  Single-Sided:    = %d\n", recs->ping_sel_single);
	fprintf(stdout, "  Alternate Sides: = %d\n", recs->ping_sel_alt);
	fprintf(stdout, "  Simultaneous:    = %d\n", recs->ping_sel_sim);

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
} /* print_counts */

/*----------------------------------------------------------------------*/
int main(int argc, char **argv) {
	options opts;
	default_options(&opts);
	mbdefaults mbdflts;
	int status = mb_defaults(opts.verbose, &(mbdflts.format), &(mbdflts.pings_get), &(mbdflts.lonflip), mbdflts.bounds,
	                     mbdflts.btime_i, mbdflts.etime_i, &(mbdflts.speedmin), &(mbdflts.timegap));

	int error = MB_ERROR_NO_ERROR;

	double file_weight;
	mb_path ifile;
	mb_path dfile;

	counts totrecs;

	/* mb_mem_debug_on(opts.verbose, &error); */

	if (status == MB_SUCCESS) {
		parse_options(opts.verbose, argc, argv, &opts, &error);
	}

	if (opts.errflg) {
		fprintf(stderr, "usage: %s\n", usage_message);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		exit(MB_ERROR_BAD_USAGE);
	}

	if (opts.verbose >= 1) {
		print_mbdefaults(opts.verbose, &opts, &mbdflts, &error);
	}

	if (opts.help) {
		fprintf(stderr, "\nProgram %s\n", program_name);
		fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "\nusage: %s\n", usage_message);
		fprintf(stderr, "\n%s\n", help_message);
		exit(error);
	}

	/* get format if required */
	if (opts.format == 0) {
		mb_get_format(opts.verbose, opts.read_file, nullptr, &(opts.format), &error);
	}

	/* determine whether to read one file or a list of files */
	const bool read_datalist = opts.format < 0;
	bool read_data;

	void *datalist;

	/* open file list */
	if (read_datalist) {
		const int look_processed = MB_DATALIST_LOOK_UNSET;
		if (mb_datalist_open(opts.verbose, &datalist, opts.read_file, look_processed, &error) != MB_SUCCESS) {
			mb_pathplus message;
			snprintf(message, sizeof(message), "Unable to open data list file: %s\n", opts.read_file);
			error_exit(opts.verbose, MB_ERROR_OPEN_FAIL, "mb_datalist_open", message);
		}

		read_data = mb_datalist_read(opts.verbose, datalist, ifile, dfile, &(opts.format), &file_weight, &error) ==
                     MB_SUCCESS;
	} else {
		/* else copy single filename to be read */
		strcpy(ifile, opts.read_file);
		read_data = true;
	}

	zero_counts(opts.verbose, &totrecs, &error);

       	counts filerecs;

	/* loop over files to be read */
	while (read_data) {
		/* reset file record counter */
		zero_counts(opts.verbose, &filerecs, &error);

		/* process the output files */
		if (status == MB_SUCCESS) {
			status = process_output(opts.verbose, &mbdflts, &opts, ifile, &filerecs, &error);
		}

		/* output counts */
		filerecs.files_read++;
		if (opts.verbose >= 1) {
			fprintf(stdout, "\nData records read from: %s\n", ifile);
			print_counts(opts.verbose, &filerecs, &error);
		}

		/* add this file's counts to total */
		add_counts(opts.verbose, &totrecs, &filerecs, &error);

		/* figure out whether and what to read next */
		if (read_datalist) {
			read_data = mb_datalist_read(opts.verbose, datalist, ifile, dfile, &(opts.format), &file_weight, &error) == MB_SUCCESS;
		} else {
			read_data = false;
		}
	} /* end loop over files in list */

	/* output counts */
	if (opts.verbose >= 1) {
		fprintf(stdout, "\nTotal data records read:\n");
		print_counts(opts.verbose, &totrecs, &error);
	}

	if (read_datalist) {
		mb_datalist_close(opts.verbose, &datalist, &error);
	}

	/* check memory */
	status = mb_memory_list(opts.verbose, &error);

	/* mb_mem_debug_off(opts.verbose, &error); */

	return (status);
} /* main */
