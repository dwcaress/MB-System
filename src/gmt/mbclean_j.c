/*--------------------------------------------------------------------
 *    The MB-system:	mbclean.c	2/26/93
 *    $Id: mbclean.c 2238 2015-04-15 06:00:52Z caress $
 *
 *    Copyright (c) 1993-2015 by
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
 * mbclean identifies and flags artifacts in swath sonar bathymetry data.
 * The edit events are output to an edit save file which can be applied
 * to the data by the program mbprocess.
 * Several algorithms are available for identifying artifacts; multiple
 * algorithmscan be applied in a single pass.  The most commonly used
 * approach is to identify artifacts  based  on  excessive  bathymetric
 * slopes.  If desired, mbclean will also flag beams associated with
 * "rails" where outer beams have  smaller  acrosstrack distances than
 * more inner beams (-Q option).  Low and high bounds on acceptable depth
 * values can be set; depth values outside  the  acceptable  range  will  be
 * flagged.  The acceptable depth ranges can either be absolute (-B option), relative
 * to the local median depth (-A option) or defined by low and high fractions
 * of the local median depth (-G option).  A set number of outer beams can also be
 * flagged.

 * The order in which the flagging algorithms are applied is as follows:
 *      1. Flag specified number of outer beams (-X option).
 *      2. Flag soundings outside specified acceptable
 *         depth range (-B option).
 *      3. Flag soundings outside acceptable depth range using
 *         fractions of local median depth (-G option).
 *      4. Flag soundings outside acceptable depth range using
 *         deviation from local median depth (-A option).
 *      5. Flag soundings associated with excessive slopes
 *         (-C option or default).
 *      6. Zap "rails" (-Q option).
 *      7. Flag all soundings in pings with too few
 *         good soundings (-U option).
 *
 *
 * Author:	D. W. Caress
 * Date:	February 26, 1993 (buffered i/o version)
 * Date:	January 19, 2001 (edit save file version)
 *
 * Acknowledgments:
 * This program is based to a large extent on the program mbcleanx
 * by Alberto Malinverno (formerly at L-DEO, now at Schlumberger),
 * which was in turn based on the original program mbclean (v. 1.0)
 * by David Caress.
 *
 */

#define THIS_MODULE_NAME	"mbclean"
#define THIS_MODULE_LIB		"mbgmt"
#define THIS_MODULE_PURPOSE	"Identifies and flags artifacts in swath sonar bathymetry data"
#define THIS_MODULE_KEYS	""

/* GMT5 header file */
#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "->V"

/* mbio include files */
#include "mb_status.h"
#include "mb_format.h"
#include "mb_define.h"
#include "mb_io.h"
#include "mb_swap.h"
#include "mb_process.h"

/* local defines */
#define	MBCLEAN_FLAG_ONE	1
#define	MBCLEAN_FLAG_BOTH	2

/* MBIO buffer size default */
#define	MBCLEAN_BUFFER_DEFAULT	500

/* edit action defines */
#define	MBCLEAN_NOACTION	0

/* ping structure definition */
struct mbclean_ping_struct {
	int	time_i[7];
	double	time_d;
	int	multiplicity;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	int	beams_bath;
	char	*beamflag;
	char	*beamflagorg;
	double	*bath;
	double	*bathacrosstrack;
	double	*bathalongtrack;
	double	*bathx;
	double	*bathy;
};

/* bad beam identifier structure definition */
struct bad_struct {
	int	flag;
	int	ping;
	int	beam;
	double	bath;
};

EXTERN_MSC int GMT_mbclean_j(void *API, int mode, void *args);

/* Control structure for mbcontour */
struct MBCLEAN_CTRL {

	struct mbcln_A {	/* -A */
		bool   active;
		double deviation_max;
	} A;
	struct mbcln_B {	/* B- */
		bool   active;
		double depth_low;
		double depth_high;
	} B;
	struct mbcln_C {	/* -C */
		bool   active;
		int    slope_form;
		double slopemax;
	} C;
	struct mbcln_D {	/* -D */
		bool   active;
		double distancemin;
		double distancemax;
	} D;
	struct mbcln_E {	/* -E */
		bool   active;
		double max_acrosstrack;
	} E;
	struct mbcln_F {	/* F- */
		bool   active;
		int    format;
	} F;
	struct mbcln_G {	/* -G */
		bool   active;
		double fraction_low;
		double fraction_high;
	} G;
	struct mbcln_I {	/* -I */
		bool   active;
		char  *inputfile;
	} I;
	struct mbcln_K {	/* -K */
		bool   active;
		double range_min;
	} K;
	struct mbcln_L {	/* -L*/
		bool   active;
		int    lonflip;
	} L;
	struct mbcln_M {	/* -M */
		bool   active;
		int    mode;
	} M;
	struct mbcln_N {	/* -N */
		bool   active;
		char  *file;
		double grd_dist;
	} N;
	struct mbcln_P {	/* -P */
		bool   active;
		double speed_low;
		double speed_high;
	} P;
	struct mbcln_Q {	/* -Q */
		bool   active;
		double backup_dist;
	} Q;
	struct mbcln_R {	/* -R */
		bool   active;
		double max_heading_rate;
	} R;
	struct mbcln_S {	/* -S */
		bool   active;
		int    spike_mode;
		int    slope_form;
		double sspline_dist;
		double spikemax;
	} S;
	struct mbcln_T {	/* -T */
		bool   active;
		double tolerance;
	} T;
	struct mbcln_U {	/* -U */
		bool   active;
		int    num_good_min;
	} U;
	struct mbcln_W {	/* -W */
		bool   active;
		double west, east, south, north;
	} W;
	struct mbcln_X {	/* -X */
		bool   active;
		int    zap_beams_left, zap_beams_right;
	} X;
	struct mbcln_Y {	/* -Y */
		bool   active;
		double zap_distance_left, zap_distance_right;
	} Y;
	struct mbcln_Z {	/* -Z */
		bool   active;
		int    check_zero_position;
	} Z;

	struct mbcln_tranf {
		int    check_deviation;
		int    check_range;
		int    check_slope;
		int    zap_long_across;
		int    check_fraction;
		int    do_sspline;
		int    check_range_min;
		int    check_speed_good;
		int    zap_rails;
		int    zap_max_heading_rate;
		int    check_spike;
		int    fix_edit_timestamps;
		int    check_position_bounds;
		int    zap_beams;
		int    zap_distance;
		int    check_zero_position;
		int    check_num_good_min;
	} transf;
};


/* edit output function */
int mbclean_save_edit(int verbose, FILE *sofp, double time_d, int beam, int action, int *error);

static char rcs_id[] = "$Id: mbclean.c 2238 2015-04-15 06:00:52Z caress $";

#include "cubgcv.c"

/*-----------------------------------------------------------------------------------------*/
void *New_mbclean_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct MBCLEAN_CTRL *Ctrl;

	Ctrl = gmt_M_memory (GMT, NULL, 1, struct MBCLEAN_CTRL);

	Ctrl->F.format = 0;
	Ctrl->M.mode = MBCLEAN_FLAG_ONE;

	return (Ctrl);
}

void Free_mbclean_Ctrl (struct GMT_CTRL *GMT, struct MBCLEAN_CTRL *Ctrl) {	/* Deallocate control structure */
	if (!Ctrl) return;

	if (Ctrl->I.inputfile) free (Ctrl->I.inputfile);
	gmt_M_free (GMT, Ctrl);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {

	gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: mbclean -I<inputfile> [-Amax -Blow/high -Cslope/unit -Dmin/max\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Fformat -Gfraction_low/fraction_high -Iinfile -Krange_min \n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Llonflip -Mmode -Ooutfile -Pmin_speed/max_speed -Q -Rmaxheadingrate \n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Sspike_slope/mode/format[+s<dist>] -Ttolerance -Wwest/east/south/north \n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Xbeamsleft/beamsright -Ydistanceleft/distanceright -Z\n\t-V -H]\n\n");

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\t<inputfile> is an MB-System datalist referencing the swath data to be cleaned.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	return (EXIT_FAILURE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct MBCLEAN_CTRL *Ctrl, struct GMT_OPTION *options) {

	unsigned int n_errors = 0, n_files = 0;
	char  *pch;
	int    n;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			case '<':	/* Input file (only one or three is accepted) */
				Ctrl->I.active = true;
#if GMT_MINOR_VERSION == 1 && GMT_RELEASE_VERSION < 2
				if (gmt_check_filearg (GMT, '<', opt->arg, GMT_IN)) {
#else
				if (gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) {
#endif
					Ctrl->I.inputfile = strdup (opt->arg);
					n_files = 1;
				}
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: only one input file is allowed.\n");
					n_errors++;
				}
				break;

			/* Processes program-specific parameters */

			case 'A':	/* */
				n = sscanf(opt->arg, "%lf", &(Ctrl->A.deviation_max));
				if (n > 0) {
					Ctrl->A.active = true;
					Ctrl->transf.check_deviation = MB_YES;
				}
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -A option: \n");
					n_errors++;
				}
 				break;

			case 'B':	/* */
				n = sscanf(opt->arg, "%lf/%lf", &(Ctrl->B.depth_low),&(Ctrl->B.depth_high));
				if (n > 1) {
					Ctrl->B.active = true;
					Ctrl->transf.check_range = MB_YES;
				}
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -B option: \n");
					n_errors++;
				}
 				break;

			case 'C':	/* */
				n = sscanf(opt->arg, "%lf/%d", &(Ctrl->C.slopemax), &(Ctrl->C.slope_form));
				if (n > 0) {
					Ctrl->C.active = true;
					Ctrl->transf.check_slope = MB_YES;
					if (Ctrl->C.slope_form == 1)
						Ctrl->C.slopemax = tan(Ctrl->C.slopemax);
					else if (Ctrl->C.slope_form == 2)
						Ctrl->C.slopemax = tan(DTR * Ctrl->C.slopemax);
				}
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -C option: \n");
					n_errors++;
				}
 				break;

			case 'D':	/* */
				n = sscanf(opt->arg, "%lf/%lf", &(Ctrl->D.distancemin), &(Ctrl->D.distancemax));
				if (n > 1) {
					Ctrl->D.active = true;
				}
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -D option: \n");
					n_errors++;
				}
 				break;

			case 'E':	/* */
				n = sscanf(opt->arg, "%lf", &(Ctrl->E.max_acrosstrack));
				if (n > 0) {
					Ctrl->E.active = true;
					Ctrl->transf.zap_long_across = MB_YES;
				}
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -E option: \n");
					n_errors++;
				}
 				break;

			case 'F':	/* */
				n = sscanf(opt->arg, "%d", &(Ctrl->F.format));
				if (n > 0) {
					Ctrl->F.active = true;
				}
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -F option: \n");
					n_errors++;
				}
 				break;

			case 'G':	/* */
				n = sscanf(opt->arg, "%lf/%lf", &(Ctrl->G.fraction_low),&(Ctrl->G.fraction_high));
				if (n > 1) {
					Ctrl->G.active = true;
					Ctrl->transf.check_fraction = MB_YES;
				}
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -G option: \n");
					n_errors++;
				}
 				break;

			case 'I':	/* */
				if (!gmt_access (GMT, opt->arg, R_OK)) {	/* Got a file */
					Ctrl->I.inputfile = strdup (opt->arg);
					Ctrl->I.active = true;
					n_files = 1;
				}
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -I option: \n");
					n_errors++;
				}
 				break;

			case 'K':	/* */
				n = sscanf(opt->arg, "%lf", &(Ctrl->K.range_min));
				if (n > 0) {
					Ctrl->K.active = true;
					Ctrl->transf.check_range_min = MB_YES;
				}
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -K option: \n");
					n_errors++;
				}
 				break;

			case 'L':	/* */
				n = sscanf(opt->arg, "%d", &(Ctrl->L.lonflip));
				if (n > 0) {
					Ctrl->L.active = true;
				}
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -L option: \n");
					n_errors++;
				}
 				break;

			case 'M':	/* */
				n = sscanf(opt->arg, "%d", &(Ctrl->M.mode));
				if (n > 0) {
					Ctrl->M.active = true;
				}
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -M option: \n");
					n_errors++;
				}
 				break;

			case 'N':	/* */
				pch = strstr(opt->arg, "+d");
				if (pch) {
					sscanf(&pch[2], "%lf", &Ctrl->N.grd_dist);
					pch[0] = '\0';			/* Now hide the +d... part so the the grid's name can be parsed by gmt_check_filearg() */
				}
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -N option: MUST specify the tolerance distance.\n");
					n_errors++;
				}
				if ((Ctrl->N.active = gmt_check_filearg(GMT, 'N', opt->arg, GMT_IN, GMT_IS_GRID)))
					Ctrl->N.file = strdup(opt->arg);
				else
					n_errors++;
 				break;

			case 'P':	/* */
				n = sscanf(opt->arg, "%lf/%lf", &(Ctrl->P.speed_low),&(Ctrl->P.speed_high));
				if (n > 0) {
					Ctrl->P.active = true;
					Ctrl->transf.check_speed_good = MB_YES;
				}
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -P option: \n");
					n_errors++;
				}
 				break;

			case 'Q':	/* */
				Ctrl->Q.backup_dist = 0.0;
				n = sscanf(opt->arg, "%lf", &(Ctrl->Q.backup_dist));
				Ctrl->transf.zap_rails = MB_YES;
				Ctrl->Q.active = true;
 				break;

			case 'R':	/* */
				n = sscanf(opt->arg, "%lf", &(Ctrl->R.max_heading_rate));
				if (n > 0) {
					Ctrl->R.active = true;
					Ctrl->transf.zap_max_heading_rate = MB_YES;
				}
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -R option: \n");
					n_errors++;
				}
 				break;

			case 'S':	/* */
				pch = strstr(opt->arg, "+s");	/* TEMP. 'Overload' the -S option with a +s<s_dist> option */
				if (pch) {
					n = sscanf(&pch[2], "%lf", &Ctrl->S.sspline_dist);
					if (n > 0) {
						Ctrl->transf.do_sspline = MB_YES;
						Ctrl->S.active = true;
					}
					pch[0] = '\0';			/* Now hide the +s... part so the the original -S option parsing can proceed */
				}
				n = sscanf(opt->arg, "%lf/%d/%d", &(Ctrl->S.spikemax), &(Ctrl->S.spike_mode), &(Ctrl->S.slope_form));
				if (n > 1) {
					Ctrl->S.active = true;
					Ctrl->transf.check_spike = MB_YES;
					if (Ctrl->S.slope_form == 1)
			  			Ctrl->S.spikemax = tan(Ctrl->S.spikemax);
					else if (Ctrl->S.slope_form == 2)
			  			Ctrl->S.spikemax = tan(DTR * Ctrl->S.spikemax);
				}
				else if (pch == NULL) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -S option:\n");
					n_errors++;
				}
 				break;

			case 'T':	/* */
				n = sscanf(opt->arg, "%lf", &(Ctrl->T.tolerance));
				Ctrl->T.active = true;
				Ctrl->transf.fix_edit_timestamps = MB_YES;
 				break;

			case 'U':	/* */
				n = sscanf(opt->arg, "%d", &(Ctrl->U.num_good_min));
				if (n > 0) {
					Ctrl->U.active = true;
					Ctrl->transf.check_num_good_min = MB_YES;
				}
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -U option: \n");
					n_errors++;
				}
 				break;

			case 'W':	/* */
				n = sscanf(opt->arg, "%lf/%lf/%lf/%lf",&(Ctrl->W.west),&(Ctrl->W.east),&(Ctrl->W.south),&(Ctrl->W.north));
				if (n > 3) {
					Ctrl->W.active = true;
					Ctrl->transf.check_position_bounds = MB_YES;
				}
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -W option: \n");
					n_errors++;
				}
 				break;

			case 'X':	/* */
				n = sscanf(opt->arg, "%d/%d", &(Ctrl->X.zap_beams_left), &(Ctrl->X.zap_beams_right));
				if (n > 1) {
					if (n == 1)
						Ctrl->X.zap_beams_right = Ctrl->X.zap_beams_left;
					Ctrl->X.active = true;
					Ctrl->transf.zap_beams = MB_YES;
				}
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -X option: \n");
					n_errors++;
				}
 				break;

			case 'Y':	/* */
				n = sscanf(opt->arg, "%lf/%lf", &(Ctrl->Y.zap_distance_left), &(Ctrl->Y.zap_distance_right));
				if (n > 0) {
					Ctrl->Y.active = true;
					if (n == 1)
						Ctrl->Y.zap_distance_right = Ctrl->Y.zap_distance_left;
					if (Ctrl->Y.zap_distance_left > 0.0)
						Ctrl->Y.zap_distance_left = -Ctrl->Y.zap_distance_left;
					if (Ctrl->Y.zap_distance_right < 0.0)
						Ctrl->Y.zap_distance_right = -Ctrl->Y.zap_distance_right;
					Ctrl->transf.zap_distance = MB_YES;
				}
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -Y option: \n");
					n_errors++;
				}
 				break;

			case 'Z':	/* */
				Ctrl->transf.check_zero_position = MB_YES;
 				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, n_files != 1, "Syntax error: Must specify one input file(s)\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_mbclean_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

/*-----------------------------------------------------------------------------------------*/
int GMT_mbclean_j (void *V_API, int mode, void *args) {

	char program_name[] = "MBCLEAN";

	struct MBCLEAN_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_GRID *G = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/* MBIO status variables */
	int	status;
	int	verbose = 0;
	int	error = MB_ERROR_NO_ERROR;
	char	*message = NULL;

	/* swath file locking variables */
	int	uselockfiles;
	int	lock_status;
	int	locked;
	int	lock_purpose;
	mb_path	lock_program;
	mb_path lock_cpu;
	mb_path lock_user;
	char	lock_date[25];

	/* MBIO read control parameters */
	mb_path	read_file;
	int	read_datalist = MB_NO;
	char	swathfile[MB_PATH_MAXLINE];
	char	swathfileread[MB_PATH_MAXLINE];
	void	*datalist;
	int	look_processed = MB_DATALIST_LOOK_UNSET;
	int	oktoprocess;
	double	file_weight;
	int	format;
	int	formatread;
	int	variable_beams;
	int	traveltime;
	int	beam_flagging;
	int	pings;
	double	bounds[4];
	int	btime_i[7];
	int	etime_i[7];
	double	btime_d;
	double	etime_d;
	double	speedmin;
	double	timegap;
	double	distance;
	double	altitude;
	double	sonardepth;
	int     beams_bath, beams_amp, pixels_ss;
	double *amp, *ss, *ssacrosstrack, *ssalongtrack;

	/* mbio read and write values */
	void	*mbio_ptr = NULL;
	int	kind;
	struct mbclean_ping_struct ping[3];
	int	nrec, irec;
	int	pingsread;
	struct bad_struct bad[2];
	int	find_bad;
	int	nfiletot = 0;
	int	ndatatot = 0;
	int	ndepthrangetot = 0;
	int	nminrangetot = 0;
	int	nfractiontot = 0;
	int	nspeedtot = 0;
	int	nzeropostot = 0;
	int	ndeviationtot = 0;
	int	nouterbeamstot = 0;
	int	nouterdistancetot = 0;
	int	nrailtot = 0;
	int nlong_acrosstot=0; //2010/03/07 DY
	int	nmintot = 0;
	int	nbadtot = 0;
	int	nspiketot = 0;
	int	nflagtot = 0;
	int	nunflagtot = 0;
	int	nflagesftot = 0;
	int	nunflagesftot = 0;
	int	nzeroesftot = 0;
	int	ndata = 0;
	int	ndepthrange = 0;
	int	nminrange = 0;
	int	nfraction = 0;
	int	nspeed = 0;
	int	nzeropos = 0;
	int	nrangepos = 0;
	int	ndeviation = 0;
	int	nouterbeams = 0;
	int	nouterdistance = 0;
	int	nrail = 0;
	int nlong_across=0; //2010/03/07 DY
	int nmax_heading_rate=0; //2010/04/27 DY
	int nmax_heading_ratetot=0; //2010/04/27 DY
	int	nmin = 0;
	int	nbad = 0;
	int	nspike = 0;
	int	nflag = 0;
	int	nunflag = 0;
	int	nflagesf = 0;
	int	nunflagesf = 0;
	int	nzeroesf = 0;
	char	comment[MB_COMMENT_MAXLINE];
	int	check_slope = MB_NO;
	double	slopemax = 1.0;
	int	check_spike = MB_NO;
	double	spikemax = 1.0;
	int	spike_mode = 1;
	int	slope_form = 0;
	double	distancemin = 0.01;
	double	distancemax = 0.25;
	int	zap_beams = MB_NO;
	int	zap_beams_right = 0;
	int	zap_beams_left = 0;
	int	zap_distance = MB_NO;
	double	zap_distance_right = 0.0;
	double	zap_distance_left = 0.0;
	int	zap_rails = MB_NO;
	int     zap_long_across = MB_NO;  //2010/03/07 DY
	int     zap_max_heading_rate = MB_NO;  //2010/04/27 DY
	int	check_range = MB_NO;
	double	depth_low;
	double	depth_high;
	int	check_range_min = MB_NO;
	double	range_min;
	int	check_fraction = MB_NO;
	double	fraction_low;
	double	fraction_high;
	int	check_speed_good = MB_NO;
	int	check_zero_position = MB_NO;
	int	check_position_bounds = MB_NO;
	double	speed_low;
	double	speed_high;
	double	west,east,south,north;
	int	check_deviation = MB_NO;
	double	deviation_max;
	int	check_num_good_min = MB_NO;
	int	num_good_min;
	int	num_good;
	int	action;
	int do_sspline = MB_NO;
	int nSplSmoothed = 0;
	int nGrdSmoothed = 0;
	double	sspline_dist;

	/* rail processing variables */
	int	center;
	double	lowdist; //2010/03/07 DY changed these to doubles
	double	highdist;
	double  backup_dist = 0; //2010/04/27 DY

	/* max acrosstrack filter variable  2010/03/07 DY */
	double max_acrosstrack = 120;

	/* max heading_rate variable  2010/04/27 DY */
	double max_heading_rate;
	double last_heading = 0.0;
	double last_time = 0.0;

	/* slope processing variables */
	int	nlist;
	double	mtodeglon;
	double	mtodeglat;
	double	headingx;
	double	headingy;
	double	*list = NULL;
	double	median = 0.0;
	double	dd;
	double	dd2;
	double	slope;
	double	slope2;

	/* fix_edit_timestamps variables */
	int	fix_edit_timestamps = MB_NO;
	double	tolerance = 0.0;

	/* save file control variables */
	int	esffile_open = MB_NO;
	char	esffile[MB_PATH_MAXLINE];
	struct mb_esf_struct esf;

	/* processing variables */
	int	read_data;
	int	start, done;
	int	i, j, k, p, b;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) 		/* Return the purpose of program */
		return (usage (API, GMT_MODULE_PURPOSE));
	options = GMT_Create_Options (API, mode, args);
	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE)	/* Return the usage message */
		bailout (usage (API, GMT_USAGE));
	if (options->option == GMT_OPT_SYNOPSIS) 			/* Return the synopsis */
		bailout (usage (API, GMT_SYNOPSIS));

	/* Parse the command-line arguments */

	GMT = gmt_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
               
	Ctrl = New_mbclean_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options))) Return (error);

	/* Copy the parsing variables stored in the Ctrl struct to original names */
	check_deviation = Ctrl->transf.check_deviation;
	check_range = Ctrl->transf.check_range;
	check_slope = Ctrl->transf.check_slope;
	zap_long_across = Ctrl->transf.zap_long_across;
	check_fraction = Ctrl->transf.check_fraction;
	do_sspline = Ctrl->transf.do_sspline;
	check_range_min = Ctrl->transf.check_range_min;
	check_speed_good = Ctrl->transf.check_speed_good;
	zap_rails = Ctrl->transf.zap_rails;
	zap_max_heading_rate = Ctrl->transf.zap_max_heading_rate;
	check_spike = Ctrl->transf.check_spike;
	fix_edit_timestamps = Ctrl->transf.fix_edit_timestamps;
	check_position_bounds = Ctrl->transf.check_position_bounds;
	zap_beams = Ctrl->transf.zap_beams;
	zap_distance = Ctrl->transf.zap_distance;
	check_zero_position = Ctrl->transf.check_zero_position;
	verbose = GMT->common.V.active;

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&Ctrl->L.lonflip,bounds, btime_i,etime_i,&speedmin,&timegap);
	status = mb_uselockfiles(verbose,&uselockfiles);

	/* reset all defaults but the format and lonflip */
	pings = 1;
	bounds[0] = -360.;
	bounds[1] = 360.;
	bounds[2] = -90.;
	bounds[3] = 90.;
	btime_i[0] = 1962;
	btime_i[1] = 2;
	btime_i[2] = 21;
	btime_i[3] = 10;
	btime_i[4] = 30;
	btime_i[5] = 0;
	btime_i[6] = 0;
	etime_i[0] = 2062;
	etime_i[1] = 2;
	etime_i[2] = 21;
	etime_i[3] = 10;
	etime_i[4] = 30;
	etime_i[5] = 0;
	etime_i[6] = 0;
	speedmin = 0.0;
	timegap = 1000000000.0;
	strcpy(read_file, "datalist.mb-1");

	if (Ctrl->A.active)
		deviation_max = Ctrl->A.deviation_max;
	if (Ctrl->B.active) {
		depth_low = Ctrl->B.depth_low;
		depth_high = Ctrl->B.depth_high;
	}
	if (Ctrl->C.active) {
		slopemax = Ctrl->C.slopemax;
		slope_form = Ctrl->C.slope_form;
	}
	if (Ctrl->D.active) {
		distancemin = Ctrl->D.distancemin;
		distancemax = Ctrl->D.distancemax;
	}
	if (Ctrl->E.active) {
		max_acrosstrack = Ctrl->E.max_acrosstrack;
	}
	if (Ctrl->F.active) {
		format = Ctrl->F.format;
	}
	if (Ctrl->G.active) {
		fraction_low = Ctrl->G.fraction_low;
		fraction_high = Ctrl->G.fraction_high;
	}
	if (Ctrl->I.active) {
		strcpy(read_file, Ctrl->I.inputfile);
	}
	if (Ctrl->K.active) {
		range_min = Ctrl->K.range_min;
	}
	if (Ctrl->P.active) {
		speed_low = Ctrl->P.speed_low;
		speed_high = Ctrl->P.speed_high;
	}
	if (Ctrl->Q.active) {
		backup_dist = Ctrl->Q.backup_dist;
	}
	if (Ctrl->R.active) {
		max_heading_rate = Ctrl->R.max_heading_rate;
	}
	if (Ctrl->S.active) {
		spikemax   = Ctrl->S.spikemax;
		spike_mode = Ctrl->S.spike_mode;
		slope_form = Ctrl->S.slope_form;
		sspline_dist = Ctrl->S.sspline_dist;
		if (sspline_dist < 0.01) do_sspline = MB_NO;	/* Safety measure */
	}
	if (Ctrl->T.active) {
		tolerance = Ctrl->T.tolerance;
	}
	if (Ctrl->U.active) {
		num_good_min = Ctrl->U.num_good_min;
	}
	if (Ctrl->W.active) {
		west  = Ctrl->W.west;
		east  = Ctrl->W.east;
		south = Ctrl->W.south;
		north = Ctrl->W.north;
	}
	if (Ctrl->X.active) {
		zap_beams_left = Ctrl->X.zap_beams_left;
		zap_beams_right = Ctrl->X.zap_beams_right;
	}
	if (Ctrl->Y.active) {
		zap_distance_left = Ctrl->Y.zap_distance_left;
		zap_distance_right = Ctrl->Y.zap_distance_right;
	}
	if (Ctrl->Z.active) {
		check_zero_position = Ctrl->Z.check_zero_position;
	}

	/*---------------------------- This is the mbclean main code ----------------------------*/

	GMT_Report (API, GMT_MSG_VERBOSE, "Processing input files\nVersion %s\nMB-system Version %s\n",rcs_id,MB_VERSION);

	if (Ctrl->N.active && (G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->N.file, NULL)) == NULL)
		Return (API->error);	/* Get entire grid */

	/* turn on slope checking if nothing else is to be used */
	if (check_slope == MB_NO && zap_beams == MB_NO
		&& zap_distance == MB_NO && zap_rails == MB_NO
		&& check_spike == MB_NO && check_range == MB_NO
		&& check_fraction == MB_NO && check_speed_good == MB_NO
		&& check_deviation == MB_NO && check_num_good_min == MB_NO
		&& check_position_bounds == MB_NO && check_zero_position == MB_NO
		&& fix_edit_timestamps == MB_NO && do_sspline == MB_NO
		&& !Ctrl->N.active)
		check_slope = MB_YES;

	/* get format if required */
	if (format == 0)
		mb_get_format(verbose,read_file,NULL,&format,&error);

	/* determine whether to read one file or a list of files */
	if (format < 0)
		read_datalist = MB_YES;

	/* open file list */
	if (read_datalist == MB_YES) {
		if ((status = mb_datalist_open(verbose,&datalist, read_file,look_processed,&error)) != MB_SUCCESS) {
			error = MB_ERROR_OPEN_FAIL;
			GMT_Report(API, GMT_MSG_NORMAL, "\nUnable to open data list file: %s\n", read_file);
			GMT_Report(API, GMT_MSG_NORMAL, "\nProgram <%s> Terminated\n", program_name);
			Return (error);
		}
		if ((status = mb_datalist_read(verbose,datalist, swathfile,&format,&file_weight,&error)) == MB_SUCCESS)
			read_data = MB_YES;
		else
			read_data = MB_NO;
	}
	/* else copy single filename to be read */
	else {
		strcpy(swathfile, read_file);
		read_data = MB_YES;
	}

	/* loop over all files to be read */
	while (read_data == MB_YES) {
		oktoprocess = MB_YES;

		/* check format and get format flags */
		if ((status = mb_format_flags(verbose,&format, &variable_beams, &traveltime, &beam_flagging, &error)) != MB_SUCCESS) {
			mb_error(verbose,error,&message);
			GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error returned from function <mb_format_flags> regarding input format %d:\n%s\n",
			           format,message);
			GMT_Report(API, GMT_MSG_NORMAL, "\nFile <%s> skipped by program <%s>\n", swathfile,program_name);
			oktoprocess = MB_NO;
			status = MB_SUCCESS;
			error = MB_ERROR_NO_ERROR;
		}

		/* warn if beam flagging not supported for the current data format */
		if (beam_flagging == MB_NO) {
			GMT_Report(API, GMT_MSG_NORMAL, "\nWarning:\nMBIO format %d does not allow flagging of bad bathymetry data.\n",format);
			GMT_Report(API, GMT_MSG_NORMAL, "\nWhen mbprocess applies edits to file:\n\t%s\nthe soundings will be nulled (zeroed) rather than flagged.\n",
			        swathfile);
		}

		/* try to lock file */
		if (uselockfiles == MB_YES)
			status = mb_pr_lockswathfile(verbose, swathfile, MBP_LOCK_EDITBATHY, program_name, &error);
		else {
			lock_status = mb_pr_lockinfo(verbose, swathfile, &locked, &lock_purpose,
			                             lock_program, lock_user, lock_cpu, lock_date, &error);

			/* if locked get lock info */
			if (error == MB_ERROR_FILE_LOCKED) {
				GMT_Report(API, GMT_MSG_NORMAL, "\nFile %s locked but lock ignored\n", swathfile);
				GMT_Report(API, GMT_MSG_NORMAL, "File locked by <%s> running <%s>\n", lock_user, lock_program);
				GMT_Report(API, GMT_MSG_NORMAL, "on cpu <%s> at <%s>\n", lock_cpu, lock_date);
				error = MB_ERROR_NO_ERROR;
			}
		}

		/* if locked let the user know file can't be opened */
		if (status == MB_FAILURE) {
			/* if locked get lock info */
			if (error == MB_ERROR_FILE_LOCKED) {
				lock_status = mb_pr_lockinfo(verbose, swathfile, &locked, &lock_purpose, lock_program,
				                             lock_user, lock_cpu, lock_date, &error);
				GMT_Report(API, GMT_MSG_NORMAL, "\nUnable to open input file:\n");
				GMT_Report(API, GMT_MSG_NORMAL, "  %s\n", swathfile);
				GMT_Report(API, GMT_MSG_NORMAL, "File locked by <%s> running <%s>\n", lock_user, lock_program);
				GMT_Report(API, GMT_MSG_NORMAL, "on cpu <%s> at <%s>\n", lock_cpu, lock_date);
			}
			else if (error == MB_ERROR_OPEN_FAIL) {
				/* else if unable to create lock file there is a permissions problem */
				GMT_Report(API, GMT_MSG_NORMAL, "Unable to create lock for intended input file:\n");
				GMT_Report(API, GMT_MSG_NORMAL, "  %s\n", swathfile);
				GMT_Report(API, GMT_MSG_NORMAL, "-Likely permissions issue\n");
			}

			/* reset error and status */
			oktoprocess = MB_NO;
			status = MB_SUCCESS;
			error = MB_ERROR_NO_ERROR;
		}

		/* proceed if file locked and format ok */
		if (oktoprocess == MB_YES) {
			/* check for "fast bathymetry" or "fbt" file */
			strcpy(swathfileread, swathfile);
			formatread = format;
			mb_get_fbt(verbose, swathfileread, &formatread, &error);

			/* initialize reading the input swath sonar file */
			if ((status = mb_read_init(verbose,swathfileread,formatread,pings,Ctrl->L.lonflip,bounds, btime_i,etime_i,
			    speedmin,timegap, &mbio_ptr,&btime_d,&etime_d, &beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS) {
				mb_error(verbose,error,&message);
				GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
				GMT_Report(API, GMT_MSG_NORMAL, "\nMultibeam File <%s> not initialized for reading\n",swathfile);
				GMT_Report(API, GMT_MSG_NORMAL, "\nProgram <%s> Terminated\n", program_name);
				Return (error);
			}

			/* initialize and increment counting variables */
			ndata = ndepthrange = nminrange = nfraction = nspeed = nzeropos = nrangepos = 0;
			ndeviation = nouterbeams = nouterdistance = nrail = nlong_across=0; //2010/03/07 DY
			nmin = nbad = nspike = nflag = nunflag = nflagesf = nunflagesf = nzeroesf = 0;

			/* give the statistics */
			if (verbose >= 0)
				GMT_Report(API, GMT_MSG_NORMAL, "\nProcessing %s\n",swathfileread);

			/* allocate memory for data arrays */
			for (i = 0; i < 3; i++) {
				ping[i].beamflag = NULL;
				ping[i].beamflagorg = NULL;
				ping[i].bath = NULL;
				ping[i].bathacrosstrack = NULL;
				ping[i].bathalongtrack = NULL;
				ping[i].bathx = NULL;
				ping[i].bathy = NULL;
				if (error == MB_ERROR_NO_ERROR)
					status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char),
					                           (void **)&ping[i].beamflag, &error);
				if (error == MB_ERROR_NO_ERROR)
					status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char),
					                           (void **)&ping[i].beamflagorg, &error);
				if (error == MB_ERROR_NO_ERROR)
					status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
					                           (void **)&ping[i].bath, &error);
				if (error == MB_ERROR_NO_ERROR)
					status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
					                           (void **)&ping[i].bathacrosstrack, &error);
				if (error == MB_ERROR_NO_ERROR)
					status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
					                           (void **)&ping[i].bathalongtrack, &error);
				if (error == MB_ERROR_NO_ERROR)
					status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
					                           (void **)&ping[i].bathx, &error);
				if (error == MB_ERROR_NO_ERROR)
					status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
					                           (void **)&ping[i].bathy, &error);
			}
			amp = NULL;
			ss = NULL;
			ssacrosstrack = NULL;
			ssalongtrack = NULL;
			list = NULL;
			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double),
				                           (void **)&amp, &error);
			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double),
				                           (void **)&ss, &error);
			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double),
				                           (void **)&ssacrosstrack, &error);
			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double),
				                           (void **)&ssalongtrack, &error);
			if (error == MB_ERROR_NO_ERROR)
				status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, 4 * sizeof(double),
				                           (void **)&list, &error);

			/* if error initializing memory then quit */
			if (error != MB_ERROR_NO_ERROR) {
				mb_error(verbose,error,&message);
				GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error allocating data arrays:\n%s\n",message);
				GMT_Report(API, GMT_MSG_NORMAL, "\nProgram <%s> Terminated\n", program_name);
				Return (error);
			}

			/* now deal with old edit save file */
			if (status == MB_SUCCESS) {
				/* reset message */
				GMT_Report(API, GMT_MSG_NORMAL, "Sorting old edits...\n");

				/* handle esf edits */
				status = mb_esf_load(verbose, program_name, swathfile, MB_YES, MB_YES, esffile, &esf, &error);
				if (status == MB_SUCCESS && esf.esffp != NULL)
					esffile_open = MB_YES;
				if (status == MB_FAILURE && error == MB_ERROR_OPEN_FAIL) {
					esffile_open = MB_NO;
					GMT_Report(API, GMT_MSG_NORMAL, "\nUnable to open new edit save file %s\n", esf.esffile);
				}
				else if (status == MB_FAILURE && error == MB_ERROR_MEMORY_FAIL) {
					esffile_open = MB_NO;
					GMT_Report(API, GMT_MSG_NORMAL, "\nUnable to allocate memory for edits in esf file %s\n", esf.esffile);
				}
				/* reset message */
				GMT_Report(API, GMT_MSG_NORMAL, "%d old edits sorted...\n",esf.nedit);
			}

			/* read */
			done = MB_NO;
			start = 0;
			nrec = 0;
			GMT_Report(API, GMT_MSG_NORMAL, "Processing data...\n");
			while (done == MB_NO) {
				if (verbose > 1) GMT_Report(API, GMT_MSG_NORMAL, "\n");

				/* read next record */
				error = MB_ERROR_NO_ERROR;
				if (!Ctrl->N.active)
					status = mb_get(verbose, mbio_ptr,&kind,&pingsread, ping[nrec].time_i,&ping[nrec].time_d,
					                &ping[nrec].navlon,&ping[nrec].navlat, &ping[nrec].speed,&ping[nrec].heading,
					                &distance,&altitude,&sonardepth, &ping[nrec].beams_bath,&beams_amp,&pixels_ss,
					                ping[nrec].beamflag,ping[nrec].bath,amp, ping[nrec].bathacrosstrack,
					                ping[nrec].bathalongtrack, ss, ssacrosstrack,ssalongtrack, comment, &error);
				else
					status = mb_read(verbose, mbio_ptr,&kind,&pingsread, ping[nrec].time_i,&ping[nrec].time_d,
					                &ping[nrec].navlon,&ping[nrec].navlat, &ping[nrec].speed,&ping[nrec].heading,
					                &distance,&altitude,&sonardepth, &ping[nrec].beams_bath,&beams_amp,&pixels_ss,
					                ping[nrec].beamflag,ping[nrec].bath,amp, ping[nrec].bathacrosstrack,
					                ping[nrec].bathalongtrack, ss, ssacrosstrack,ssalongtrack, comment, &error);

				if (status == MB_SUCCESS && kind == MB_DATA_DATA) {
					/* check for multiple pings with the same time stamps */
					if (nrec > 0 && ping[nrec].time_d == ping[nrec-1].time_d)
						ping[nrec].multiplicity = ping[nrec-1].multiplicity + 1;
					else
						ping[nrec].multiplicity = 0;

					/* save original beamflags */
					for (i = 0; i < ping[nrec].beams_bath; i++)
						ping[nrec].beamflagorg[i] = ping[nrec].beamflag[i];

					/* get locations of data points in local coordinates */
					mb_coor_scale(verbose,ping[nrec].navlat, &mtodeglon,&mtodeglat);
					headingx = sin(ping[nrec].heading*DTR);
					headingy = cos(ping[nrec].heading*DTR);
					for (j = 0; j <= nrec; j++) {
						for (i = 0; i < ping[nrec].beams_bath; i++) {
							ping[j].bathx[i] = (ping[j].navlon - ping[0].navlon) / mtodeglon
								+ headingy * ping[j].bathacrosstrack[i] + headingx * ping[j].bathalongtrack[i];
							ping[j].bathy[i] = (ping[j].navlat - ping[0].navlat) / mtodeglat
								- headingx * ping[j].bathacrosstrack[i] + headingy * ping[j].bathalongtrack[i];
						}
					}

					/* if requested set all edit timestamps within tolerance of
						ping[nrec].time_d to ping[nrec].time_d */
					status = mb_esf_fixtimestamps(verbose, &esf, ping[nrec].time_d, tolerance, &error);

					/* apply saved edits */
					status = mb_esf_apply(verbose, &esf, ping[nrec].time_d, ping[nrec].multiplicity,
				                          ping[nrec].beams_bath, ping[nrec].beamflag, &error);

					/* update counters */
					for (i = 0; i < ping[nrec].beams_bath; i++) {
						if (ping[nrec].beamflag[i] != ping[nrec].beamflagorg[i]) {
							if (mb_beam_ok(ping[nrec].beamflag[i]))
								nunflagesf++;
							else
								nflagesf++;
						}
					}
					ndata++;
					nrec++;
				}
				else if (error > MB_ERROR_NO_ERROR)
					done = MB_YES;

				/* process a record */
				if (nrec > 0) {
					/* get record to process */
					if (nrec >= 2)
						irec = 1;
					else if (nrec == 1)
						irec = 0;

					center = ping[irec].beams_bath / 2;		/* get center beam */

					/* zap outer beams by number if requested */
					if (zap_beams == MB_YES) {
						for (i = 0; i < MIN(zap_beams_left, center); i++) {
							if (mb_beam_ok(ping[irec].beamflag[i])) {
								find_bad = MB_YES;
								ping[irec].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
								nouterbeams++;
								nflag++;
								mb_ess_save(verbose, &esf, ping[irec].time_d, i + ping[irec].multiplicity *
								            MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER, &error);
							}
						}
						for (i=0;i<MIN(zap_beams_right, center);i++) {
							j = ping[irec].beams_bath - i - 1;
							if (mb_beam_ok(ping[irec].beamflag[j])) {
								find_bad = MB_YES;
								ping[irec].beamflag[j] = MB_FLAG_FLAG + MB_FLAG_FILTER;
								nouterbeams++;
								nflag++;
								mb_ess_save(verbose, &esf, ping[irec].time_d, j + ping[irec].multiplicity *
								            MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER, &error);
							}
						}
					}

					/* zap outer beams by distance if requested */
					if (zap_distance == MB_YES) {
						for (i=0;i<ping[irec].beams_bath;i++) {
							if (mb_beam_ok(ping[irec].beamflag[i]) && (ping[irec].bathacrosstrack[i] <=
							    zap_distance_left || ping[irec].bathacrosstrack[i] >= zap_distance_right)) {
								find_bad = MB_YES;
								ping[irec].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
								nouterdistance++;
								nflag++;
								mb_ess_save(verbose, &esf, ping[irec].time_d, i + ping[irec].multiplicity *
								            MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER, &error);
							}
						}
					}

					/* check for speed range if requested */
					if (check_speed_good == MB_YES) {
						if (ping[irec].speed > speed_high || ping[irec].speed < speed_low) {
							for (i=0;i<ping[irec].beams_bath;i++) {
								find_bad = MB_YES;
								ping[irec].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
								nspeed++;
								nflag++;
								mb_ess_save(verbose, &esf, ping[irec].time_d, i + ping[irec].multiplicity *
								            MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER, &error);
							}
						}
					}

					/* check for range latitude and longitude if requested */
					if (check_position_bounds == MB_YES) {
						if (ping[irec].navlon < west || ping[irec].navlon > east || ping[irec].navlat < south || ping[irec].navlat>north) {
							for (i=0;i<ping[irec].beams_bath;i++) {
								find_bad = MB_YES;
								ping[irec].beamflag[i] = MB_FLAG_NULL;
								nrangepos++;
								nflag++;
								mb_ess_save(verbose, &esf, ping[irec].time_d, i + ping[irec].multiplicity *
								            MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_ZERO, &error);
							}
						}
					}

					/* check for zero latitude and longitude if requested */
					if(check_zero_position == MB_YES) {
						if (ping[irec].navlon == 0.0 && ping[irec].navlat == 0.0) {
							for (i=0;i<ping[irec].beams_bath;i++) {
								find_bad = MB_YES;
								ping[irec].beamflag[i] = MB_FLAG_NULL;
								nzeropos++;
								nflag++;
								mb_ess_save(verbose, &esf, ping[irec].time_d, i + ping[irec].multiplicity *
								            MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_ZERO, &error);
							}
						}
					}
					
					/* check depths for acceptable range if requested */
					if (check_range == MB_YES) {
						for (i = 0; i < ping[irec].beams_bath; i++) {
							if (mb_beam_ok(ping[irec].beamflag[i]) && (ping[irec].bath[i] < depth_low ||
							    ping[irec].bath[i] > depth_high))
							{
								find_bad = MB_YES;
								ping[irec].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
								ndepthrange++;
								nflag++;
								mb_ess_save(verbose, &esf, ping[irec].time_d, i + ping[irec].multiplicity *
								            MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER, &error);
							}
						}
					}

					/* check depths for minimum range if requested (replacement for Dana Yoerger test) */
					if (check_range_min == MB_YES) {
						for (i = 0; i < ping[irec].beams_bath; i++) {
							if (mb_beam_ok(ping[irec].beamflag[i]) && sqrt(ping[irec].bathacrosstrack[i] *
							    ping[irec].bathacrosstrack[i] + ping[irec].bathalongtrack[i] * ping[irec].bathalongtrack[i] +
							    (ping[irec].bath[i] - sonardepth) * (ping[irec].bath[i] - sonardepth)) < range_min)
							{
								find_bad = MB_YES;
								ping[irec].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
								nminrange++;
								nflag++;
								mb_ess_save(verbose, &esf, ping[irec].time_d, i + ping[irec].multiplicity *
								            MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER, &error);
							}
						}
					}

					/* check for max heading rate if requested */
					if (zap_max_heading_rate == MB_YES) {
						double dh, heading_rate;
						dh = (ping[irec].heading-last_heading);
						if(dh > 180)  dh -=360;
						if(dh < -180) dh +=360;
						heading_rate = dh / (ping[irec].time_d-last_time);
  
						last_time = ping[irec].time_d;
						last_heading = ping[irec].heading;
						for (i=0;i<ping[irec].beams_bath;i++) {
							if (fabs(heading_rate) > max_heading_rate) {
								find_bad = MB_YES;
								ping[irec].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
								nmax_heading_rate++;
								nflag++;
								mb_ess_save(verbose, &esf, ping[irec].time_d, i + ping[irec].multiplicity *
								            MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER, &error);
							}
						}
					}

					/* zap rails if requested */
					if (zap_rails == MB_YES) {
						/* flag all beams with acrosstrack distance less than the maximum out to that beam */
						lowdist = 0.0;
						highdist = 0.0;

						for (j = center; j < ping[irec].beams_bath; j++) {
							if (mb_beam_ok(ping[irec].beamflag[j]) && ping[irec].bathacrosstrack[j] <= highdist - backup_dist) {
								find_bad = MB_YES;
								ping[irec].beamflag[j] = MB_FLAG_FLAG + MB_FLAG_FILTER;
								nrail++;
								nflag++;
								mb_ess_save(verbose, &esf, ping[irec].time_d, j + ping[irec].multiplicity *
								            MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER, &error);
							}
							else
								highdist = ping[irec].bathacrosstrack[j];

							k = center - (j - center) - 1;
							if (mb_beam_ok(ping[irec].beamflag[k]) && ping[irec].bathacrosstrack[k] >= lowdist + backup_dist) {
								find_bad = MB_YES;
								ping[irec].beamflag[k] = MB_FLAG_FLAG + MB_FLAG_FILTER;
								nrail++;
								nflag++;
								mb_ess_save(verbose, &esf, ping[irec].time_d, k + ping[irec].multiplicity *
								            MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER, &error);
							}
							else
								lowdist = ping[irec].bathacrosstrack[k];
						}

					} // if zap_rails==yes

					/* zap long acrosstrack if requested */
					if (zap_long_across == MB_YES) {
						for (j = 0; j < ping[irec].beams_bath; j++) {
							if (mb_beam_ok(ping[irec].beamflag[j]) && fabs(ping[irec].bathacrosstrack[j]) > max_acrosstrack) {
								find_bad = MB_YES;
								ping[irec].beamflag[j] = MB_FLAG_FLAG + MB_FLAG_FILTER;
								nlong_across++;
								nflag++;
								mb_ess_save(verbose, &esf, ping[irec].time_d, j + ping[irec].multiplicity *
								            MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER, &error);
							}
						}
					}

					/* Detect outliers with the help of a smoothing spline */
					if (do_sspline == MB_YES) {
						int k, err;
						double *f = NULL, *x = NULL, *y = NULL, *se = NULL, *df = NULL, *c = NULL, *wk = NULL, var;
						f  = (double *)malloc(ping[irec].beams_bath * sizeof(double));
						x  = (double *)malloc(ping[irec].beams_bath * sizeof(double));
						y  = (double *)malloc(ping[irec].beams_bath * sizeof(double));
						df = (double *)malloc(ping[irec].beams_bath * sizeof(double));
						se = (double *)malloc(ping[irec].beams_bath * sizeof(double));
						c  = (double *)malloc((ping[irec].beams_bath-1) * 3 * sizeof(double));
						wk = (double *)malloc((ping[irec].beams_bath+2) * 7 * sizeof(double));
						for (i = k = 0; i < ping[irec].beams_bath; i++) {
							df[i] = 1;
							if (!mb_beam_ok(ping[irec].beamflag[i])) continue;
							x[k] = i;		/* Approximation to not have to compute dist across-track. Is it good enough? */
							f[k++] = ping[irec].bath[i];
						}
						k--;
						var = -1;
						err = cubgcv(x, f, df, k, y, c, k-1, &var, 1, se, wk);
						if (!err) {
							for (i = 0; i < k; i++) {
								if (fabs(y[i] - f[i]) > sspline_dist) {
									ping[irec].beamflag[(int)x[i]] = MB_FLAG_FLAG + MB_FLAG_FILTER;
									nSplSmoothed++;
									nflag++;
									mb_ess_save(verbose, &esf, ping[irec].time_d, (int)x[i] + ping[irec].multiplicity *
									            MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER, &error);
								}
							}
						}
						free(f);	free(x);	free(y);	free(se);	free(df);	free(c);	free(wk);
					}

					/* Interpolate the grid and compare with beam value */
					if (Ctrl->N.active) {
						double value;
						for (i = 0; i < ping[irec].beams_bath; i++) {
							if (!mb_beam_ok(ping[irec].beamflag[i])) continue;
							value = gmt_bcr_get_z(GMT, G, ping[irec].bathacrosstrack[i], ping[irec].bathalongtrack[i]);
							if (!gmt_M_is_dnan(value) && fabs(ping[irec].bath[i] + value) >= Ctrl->N.grd_dist) {	// TMP. value is < 0 and beam > 0
								ping[irec].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
								fprintf(stdout, "lon = %.10f\tlat = %.10f\tzb = %.4f\tzi = %.4f\n",
								        ping[irec].bathacrosstrack[i], ping[irec].bathalongtrack[i], -ping[irec].bath[i], value);
								nGrdSmoothed++;
								nflag++;
								mb_ess_save(verbose, &esf, ping[irec].time_d, i + ping[irec].multiplicity *
								            MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER, &error);
							}
						}
					}

					/* do tests that require looping over all available beams */
					if (check_fraction == MB_YES || check_deviation == MB_YES || check_spike == MB_YES || check_slope == MB_YES) {
						for (i = 0; i < ping[irec].beams_bath; i++) {
							if (mb_beam_ok(ping[irec].beamflag[i])) {
								/* get local median value from all available records */
								if (median <= 0.0)
									median = ping[irec].bath[i];
								nlist = 0;
								for (j = 0; j < nrec; j++) {
									for (k = 0; k < ping[j].beams_bath; k++) {
										if (mb_beam_ok(ping[j].beamflag[k])) {
											dd = sqrt((ping[j].bathx[k] - ping[irec].bathx[i]) * (ping[j].bathx[k] - ping[irec].bathx[i]) +
											          (ping[j].bathy[k] - ping[irec].bathy[i]) * (ping[j].bathy[k] - ping[irec].bathy[i]));
											if (dd <= distancemax * median) {
												list[nlist] = ping[j].bath[k];
												nlist++;
											}
										}
									}
								}
								qsort((char *)list,nlist,sizeof(double),(void *)mb_double_compare);
								median = list[nlist / 2];

								/* check fractional deviation from median if desired */
								if (check_fraction == MB_YES && median > 0.0) {
									if (ping[irec].bath[i]/median < fraction_low || ping[irec].bath[i]/median > fraction_high) {
										find_bad = MB_YES;
										ping[irec].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
										nfraction++;
										nflag++;
										mb_ess_save(verbose, &esf, ping[irec].time_d, i + ping[irec].multiplicity *
										            MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER, &error);
									}
								}

								/* check absolute deviation from median if desired */
								if (check_deviation == MB_YES && median > 0.0) {
									if (fabs(ping[irec].bath[i] - median) > deviation_max) {
										find_bad = MB_YES;
										ping[irec].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
										ndeviation++;
										nflag++;
										mb_ess_save(verbose, &esf, ping[irec].time_d, i + ping[irec].multiplicity *
										            MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER, &error);
									}
								}

								/* check spikes - acrosstrack */
								if (check_spike == MB_YES && 0 != (spike_mode & 1) && median > 0.0 && i > 0 && i < ping[irec].beams_bath -1 &&
								    mb_beam_ok(ping[irec].beamflag[i-1]) && mb_beam_ok(ping[irec].beamflag[i+1]))
								{
									dd = sqrt((ping[irec].bathx[i-1] - ping[irec].bathx[i]) * (ping[irec].bathx[i-1] - ping[irec].bathx[i]) +
									          (ping[irec].bathy[i-1] - ping[irec].bathy[i]) * (ping[irec].bathy[i-1] - ping[irec].bathy[i]));
									if (dd > distancemin * median && dd <= distancemax * median) {
										slope = (ping[irec].bath[i-1] - ping[irec].bath[i])/dd;
										dd2 = sqrt((ping[irec].bathx[i+1] - ping[irec].bathx[i]) * (ping[irec].bathx[i+1] - ping[irec].bathx[i]) +
										           (ping[irec].bathy[i+1] - ping[irec].bathy[i]) * (ping[irec].bathy[i+1] - ping[irec].bathy[i]));
										if (dd2 > distancemin * median && dd2 <= distancemax * median) {
											slope2 = (ping[irec].bath[i] - ping[irec].bath[i+1]) / dd2;
											if ((slope > spikemax && slope2 < -spikemax) || (slope2 > spikemax && slope < -spikemax)) {
												nspike++;
												nflag++;
												ping[irec].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
												mb_ess_save(verbose, &esf, ping[irec].time_d, i + ping[irec].multiplicity *
												            MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER, &error);
											}
										}
									}
								}

								/* check spikes - alongtrack */
								if (check_spike == MB_YES && nrec == 3 && 0 != (spike_mode & 2) && mb_beam_ok(ping[0].beamflag[i]) &&
								    mb_beam_ok(ping[2].beamflag[i])) {
									dd = sqrt((ping[0].bathx[i] - ping[1].bathx[i]) * (ping[0].bathx[i] - ping[1].bathx[i]) +
									          (ping[0].bathy[i] - ping[1].bathy[i]) * (ping[0].bathy[i] - ping[1].bathy[i]));
									if (dd > distancemin * median && dd <= distancemax * median) {
										slope = (ping[0].bath[i] - ping[1].bath[i]) / dd;
										dd2 = sqrt((ping[2].bathx[i] - ping[1].bathx[i]) * (ping[2].bathx[i] - ping[1].bathx[i]) +
										           (ping[2].bathy[i] - ping[1].bathy[i]) * (ping[2].bathy[i] - ping[1].bathy[i]));
										if (dd2 > distancemin * median && dd2 <= distancemax * median) {
											slope2 = (ping[1].bath[i] - ping[2].bath[i]) / dd2;
											if ((slope > spikemax && slope2 < -spikemax) || (slope2 > spikemax && slope < -spikemax)) {
												nspike++;
												nflag++;
												ping[1].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
												mb_ess_save(verbose, &esf, ping[1].time_d, i + ping[1].multiplicity *
												            MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER, &error);
											}
										}
									}
								}

								/* check slopes - loop over each of the beams in the current ping */
								if (check_slope == MB_YES && nrec == 3 && median > 0.0) {
									for (j = 0; j < nrec; j++) {
										for (k = 0; k < ping[j].beams_bath; k++) {
											if (mb_beam_ok(ping[j].beamflag[k])) {
												dd = sqrt((ping[j].bathx[k] - ping[1].bathx[i]) * (ping[j].bathx[k] - ping[1].bathx[i]) +
												          (ping[j].bathy[k] - ping[1].bathy[i]) * (ping[j].bathy[k] - ping[1].bathy[i]));
												if (dd > 0.0 && dd <= distancemax * median)
													slope = fabs((ping[j].bath[k] - ping[1].bath[i])/dd);
												else
													slope = 0.0;

												if (slope > slopemax && dd > distancemin * median) {
													find_bad = MB_YES;
													if (Ctrl->M.mode == MBCLEAN_FLAG_BOTH) {
														bad[0].flag = MB_YES;
														bad[0].ping = j;
														bad[0].beam = k;
														bad[0].bath =
														ping[j].bath[k];
														bad[1].flag = MB_YES;
														bad[1].ping = 1;
														bad[1].beam = i;
														bad[1].bath =
														ping[1].bath[i];
														ping[j].beamflag[k] = MB_FLAG_FLAG + MB_FLAG_FILTER;
														ping[1].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
														nbad++;
														nflag = nflag + 2;
														mb_ess_save(verbose, &esf, ping[j].time_d, k + ping[j].multiplicity *
														            MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER, &error);
														mb_ess_save(verbose, &esf, ping[1].time_d, i + ping[1].multiplicity *
														            MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER, &error);
													}
													else {
														if (fabs((double)ping[j].bath[k]-median) > fabs((double)ping[1].bath[i]-median)) {
															bad[0].flag = MB_YES;
															bad[0].ping = j;
															bad[0].beam = k;
															bad[0].bath = ping[j].bath[k];
															bad[1].flag = MB_NO;
															ping[j].beamflag[k] = MB_FLAG_FLAG + MB_FLAG_FILTER;
															mb_ess_save(verbose, &esf, ping[j].time_d, k + ping[j].multiplicity *
															            MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER, &error);
														}
														else {
															bad[0].flag = MB_YES;
															bad[0].ping = 1;
															bad[0].beam = i;
															bad[0].bath = ping[1].bath[i];
															bad[1].flag = MB_NO;
															ping[1].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
															mb_ess_save(verbose, &esf, ping[1].time_d, i + ping[1].multiplicity *
															            MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER, &error);
														}
														nbad++;
														nflag++;
													}
												}
												if (verbose >= 1 && slope > slopemax && dd > distancemin * median && bad[0].flag == MB_YES) {
													p = bad[0].ping;
													b = bad[0].beam;
												}
												if (verbose >= 1 && slope > slopemax && dd > distancemin * median && bad[1].flag == MB_YES) {
													p = bad[1].ping;
													b = bad[1].beam;
												}
											}
										}
									}
								}
							}	/* IF check slopes */ 
						}
					}			/* for (i = 0; i < ping[irec].beams_bath; i++)  */

					/* check for minimum number of good depths on each side of swath */
					if (check_num_good_min == MB_YES && num_good_min > 0) {
						/* do port */
						num_good = 0;
						for (i = 0; i < center; i++)
							if (mb_beam_ok(ping[irec].beamflag[i])) num_good++;

						if (num_good < num_good_min) {
							find_bad = MB_YES;
							for (i = 0; i < center; i++) {
								if (mb_beam_ok(ping[irec].beamflag[i])) {
									ping[irec].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
									nmin++;
									nflag++;
									mb_ess_save(verbose, &esf, ping[irec].time_d, i + ping[irec].multiplicity *
									            MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER, &error);
								}
							}
						}

						/* do starboard */
						num_good = 0;
						for (i=center+1;i<ping[irec].beams_bath;i++)
							if (mb_beam_ok(ping[irec].beamflag[i])) num_good++;

						if (num_good < num_good_min) {
							find_bad = MB_YES;
							for (i=center+1;i<ping[irec].beams_bath;i++) {
								if (mb_beam_ok(ping[irec].beamflag[i])) {
									ping[irec].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
									nmin++;
									nflag++;
									mb_ess_save(verbose, &esf, ping[irec].time_d, i + ping[irec].multiplicity *
									            MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER, &error);
								}
							}
						}
					}

				}			/* if (check_fraction ...  */

				/* write out edits from completed pings */
				if ((status == MB_SUCCESS && nrec == 3) || done == MB_YES) {
					if (done == MB_YES)
						k = nrec;
					else
						k = 1;

					for (irec=0;irec<k;irec++) {
						for (i=0;i<ping[irec].beams_bath;i++) {
							if (ping[irec].beamflag[i] != ping[irec].beamflagorg[i]) {
								if (mb_beam_ok(ping[irec].beamflag[i]))
									action = MBP_EDIT_UNFLAG;
								else if (mb_beam_check_flag_filter2(ping[irec].beamflag[i]))
									action = MBP_EDIT_FILTER;
								else if (mb_beam_check_flag_filter(ping[irec].beamflag[i]))
									action = MBP_EDIT_FILTER;
								else if (ping[irec].beamflag[i] != MB_FLAG_NULL)
									action = MBP_EDIT_FLAG;
								else
									action = MBP_EDIT_ZERO;
								mb_esf_save(verbose, &esf, ping[irec].time_d, i + ping[irec].multiplicity *
								            MB_ESF_MULTIPLICITY_FACTOR, action, &error);
							}
						}
				 	}
				}

				/* reset counters and data */
				if (status == MB_SUCCESS && nrec == 3) {
					/* copy data back one */
					nrec = 2;
					for (j=0;j<2;j++) {
						for (i=0;i<7;i++)
							ping[j].time_i[i] = ping[j+1].time_i[i];

						ping[j].time_d = ping[j+1].time_d;
						ping[j].navlon = ping[j+1].navlon;
						ping[j].navlat = ping[j+1].navlat;
						ping[j].speed = ping[j+1].speed;
						ping[j].heading = ping[j+1].heading;
						ping[j].beams_bath = ping[j+1].beams_bath;
						for (i=0;i<ping[j].beams_bath;i++) {
							ping[j].beamflag[i] = ping[j+1].beamflag[i];
							ping[j].beamflagorg[i] = ping[j+1].beamflagorg[i];
							ping[j].bath[i] = ping[j+1].bath[i];
							ping[j].bathacrosstrack[i] = ping[j+1].bathacrosstrack[i];
							ping[j].bathalongtrack[i]  = ping[j+1].bathalongtrack[i];
						}
					}
				}
			}			/* if (nrec > 0) // process a record */

			/* close the file */
			status = mb_close(verbose,&mbio_ptr,&error);

				/* close edit save file */
			status = mb_esf_close(verbose, &esf, &error);

			if (esffile_open == MB_YES) {
				/* update mbprocess parameter file */
				status = mb_pr_update_format(verbose, swathfile, MB_YES, format, &error);
				status = mb_pr_update_edit(verbose, swathfile, MBP_EDIT_ON, esffile, &error);
			}

			/* unlock the raw swath file */
			if (uselockfiles == MB_YES)
				status = mb_pr_unlockswathfile(verbose, swathfile, MBP_LOCK_EDITBATHY, program_name, &error);

			/* increment the total counting variables */
			nfiletot++;
			ndatatot += ndata;
			nflagesftot += nflagesf;
			nunflagesftot += nunflagesf;
			nzeroesftot += nzeroesf;
			ndepthrangetot += ndepthrange;
			nminrangetot += nminrange;
			nfractiontot += nfraction;
			ndeviationtot += ndeviation;
			nouterbeamstot += nouterbeams;
			nouterdistancetot += nouterdistance;
			nrailtot += nrail;
			nlong_acrosstot += nlong_across;
			nmax_heading_ratetot += nmax_heading_rate;
			nmintot    += nmin;
			nbadtot    += nbad;
			nspiketot  += nspike;
			nflagtot   += nflag;
			nunflagtot += nunflag;

			/* give the statistics */
			if (verbose >= 0) {
				GMT_Report(API, GMT_MSG_NORMAL, "%d bathymetry data records processed\n",ndata);
				if (esf.nedit > 0) {
					GMT_Report(API, GMT_MSG_NORMAL, "%d beams flagged in old esf file\n",nflagesf);
					GMT_Report(API, GMT_MSG_NORMAL, "%d beams unflagged in old esf file\n",nunflagesf);
					GMT_Report(API, GMT_MSG_NORMAL, "%d beams zeroed in old esf file\n",nzeroesf);
				}
				GMT_Report(API, GMT_MSG_NORMAL, "%d beams zapped by beam number\n",nouterbeams);
				GMT_Report(API, GMT_MSG_NORMAL, "%d beams zapped by distance\n",nouterdistance);
				GMT_Report(API, GMT_MSG_NORMAL, "%d beams zapped for too few good beams in ping\n",nmin);
				GMT_Report(API, GMT_MSG_NORMAL, "%d beams out of acceptable depth range\n",ndepthrange);
				GMT_Report(API, GMT_MSG_NORMAL, "%d beams less than minimum range\n",nminrange);
				GMT_Report(API, GMT_MSG_NORMAL, "%d beams out of acceptable fractional depth range\n",nfraction);
				GMT_Report(API, GMT_MSG_NORMAL, "%d beams out of acceptable speed range\n",nspeed);
				GMT_Report(API, GMT_MSG_NORMAL, "%d beams have zero position (lat/lon)\n",nzeropos);
				GMT_Report(API, GMT_MSG_NORMAL, "%d beams exceed acceptable deviation from median depth\n",ndeviation);
				GMT_Report(API, GMT_MSG_NORMAL, "%d bad rail beams identified\n",nrail);
				GMT_Report(API, GMT_MSG_NORMAL, "%d long acrosstrack beams identified\n",nlong_across);
				GMT_Report(API, GMT_MSG_NORMAL, "%d max heading rate pings identified\n",nmax_heading_rate);
				GMT_Report(API, GMT_MSG_NORMAL, "%d excessive slopes identified\n",nbad);
				GMT_Report(API, GMT_MSG_NORMAL, "%d excessive spikes identified\n",nspike);
				GMT_Report(API, GMT_MSG_NORMAL, "%d spline spikes identified\n",nSplSmoothed);
				GMT_Report(API, GMT_MSG_NORMAL, "%d from grid spikes identified\n",nGrdSmoothed);
				GMT_Report(API, GMT_MSG_NORMAL, "%d beams flagged\n",nflag);
				GMT_Report(API, GMT_MSG_NORMAL, "%d beams unflagged\n",nunflag);
			}

		}		/* while (done == MB_NO)  */

		/* figure out whether and what to read next */
		if (read_datalist == MB_YES) {
			if ((status = mb_datalist_read(verbose,datalist, swathfile,&format,&file_weight,&error)) == MB_SUCCESS)
				read_data = MB_YES;
			else
				read_data = MB_NO;
		}
		else
			read_data = MB_NO;

	} /* end loop over files in list */

	if (read_datalist == MB_YES)
		mb_datalist_close(verbose,&datalist,&error);

	/* give the total statistics */
	if (verbose >= 0) {
		GMT_Report(API, GMT_MSG_NORMAL, "\nMBclean Processing Totals:\n");
		GMT_Report(API, GMT_MSG_NORMAL, "-------------------------\n");
		GMT_Report(API, GMT_MSG_NORMAL, "%d total swath data files processed\n",nfiletot);
		GMT_Report(API, GMT_MSG_NORMAL, "%d total bathymetry data records processed\n",ndatatot);
		GMT_Report(API, GMT_MSG_NORMAL, "%d total beams flagged in old esf files\n",nflagesftot);
		GMT_Report(API, GMT_MSG_NORMAL, "%d total beams unflagged in old esf files\n",nunflagesftot);
		GMT_Report(API, GMT_MSG_NORMAL, "%d total beams zeroed in old esf files\n",nzeroesftot);
		GMT_Report(API, GMT_MSG_NORMAL, "%d total beams zapped by beam number\n",nouterbeamstot);
		GMT_Report(API, GMT_MSG_NORMAL, "%d total beams zapped by distance\n",nouterdistancetot);
		GMT_Report(API, GMT_MSG_NORMAL, "%d total beams zapped for too few good beams in ping\n",nmintot);
		GMT_Report(API, GMT_MSG_NORMAL, "%d total beams out of acceptable depth range\n",ndepthrangetot);
		GMT_Report(API, GMT_MSG_NORMAL, "%d total beams less than minimum range\n",nminrangetot);
		GMT_Report(API, GMT_MSG_NORMAL, "%d total beams out of acceptable fractional depth range\n",nfractiontot);
		GMT_Report(API, GMT_MSG_NORMAL, "%d total beams out of acceptable speed range\n",nspeedtot);
		GMT_Report(API, GMT_MSG_NORMAL, "%d total beams zero position (lat/lon)\n",nzeropostot);
		GMT_Report(API, GMT_MSG_NORMAL, "%d total beams exceed acceptable deviation from median depth\n",ndeviationtot);
		GMT_Report(API, GMT_MSG_NORMAL, "%d total bad rail beams identified\n",nrailtot);
		GMT_Report(API, GMT_MSG_NORMAL, "%d total long acrosstrack beams identified\n",nlong_acrosstot);
		GMT_Report(API, GMT_MSG_NORMAL, "%d total max heading rate beams identified\n",nmax_heading_ratetot);
		GMT_Report(API, GMT_MSG_NORMAL, "%d total excessive spikes identified\n",nspiketot);
		GMT_Report(API, GMT_MSG_NORMAL, "%d total excessive slopes identified\n",nbadtot);
		GMT_Report(API, GMT_MSG_NORMAL, "%d spline spikes identified\n",nSplSmoothed);
		GMT_Report(API, GMT_MSG_NORMAL, "%d from grid spikes identified\n",nGrdSmoothed);
		GMT_Report(API, GMT_MSG_NORMAL, "%d total beams flagged\n",nflagtot);
		GMT_Report(API, GMT_MSG_NORMAL, "%d total beams unflagged\n",nunflagtot);
	}

	/* set program status */
	status = MB_SUCCESS;

	/* end it all */
	Return (EXIT_SUCCESS);
}

/*--------------------------------------------------------------------*/
int mbclean_save_edit(int verbose, FILE *sofp, double time_d, int beam, int action, int *error) {
	/* local variables */
	int	status = MB_SUCCESS;

	/* write out the edit */
	if (sofp != NULL) {
#ifdef BYTESWAPPED
		mb_swap_double(&time_d);
		beam = mb_swap_int(beam);
		action = mb_swap_int(action);
#endif
		if (fwrite(&time_d, sizeof(double), 1, sofp) != 1) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		if (status == MB_SUCCESS && fwrite(&beam, sizeof(int), 1, sofp) != 1) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		if (status == MB_SUCCESS && fwrite(&action, sizeof(int), 1, sofp) != 1) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
	}

	return(status);
}
/*--------------------------------------------------------------------*/
