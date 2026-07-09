/*--------------------------------------------------------------------
 *    The MB-system:  mbextractsegy.c  4/18/2004
 *
 *    Copyright (c) 2004-2025 by
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
 * mbextractsegy extracts subbottom profiler, center beam reflection,
 * or seismic reflection data from data supported by MB-System and
 * rewrites it as a SEGY file in the form used by SIOSEIS. .
 *
 * Author:  D. W. Caress
 * Date:  April 18, 2004
 *
 * GMT-module rewrite of mbextractsegy.cc: wrapped as GMT_mbextractsegy entry
 * so it can be invoked from the GMT API (Julia FFI / Matlab MEX). The
 * original getopt() loop is replaced by a GMT_OPTION walk; the rest of the
 * extraction + plotting-script generation logic is preserved verbatim.
 */

#define THIS_MODULE_NAME		"mbextractsegy"
#define THIS_MODULE_LIB			"mbsystem"
#define THIS_MODULE_PURPOSE		"Extract subbottom/centerbeam/seismic data to SIOSEIS-style SEGY files and emit plotting scripts"
#define THIS_MODULE_KEYS		">D}"
#define THIS_MODULE_NEEDS		""
#define THIS_MODULE_OPTIONS		"-:>Vh"

#include "gmt_dev.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_segy.h"
#include "mb_status.h"

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#define MBES_ALLOC_NUM         128
#define MBES_ROUTE_WAYPOINT_NONE      0
#define MBES_ROUTE_WAYPOINT_SIMPLE    1
#define MBES_ROUTE_WAYPOINT_TRANSIT   2
#define MBES_ROUTE_WAYPOINT_STARTLINE 3
#define MBES_ROUTE_WAYPOINT_ENDLINE   4
#define MBES_ONLINE_THRESHOLD  15.0
#define MBES_ONLINE_COUNT      30
#define MBES_NUM_PLOT_MAX      50
#define MBES_MAX_SWEEP         1.0

static const char program_name[] = "MBextractsegy";
static const char help_message[] =
    "MBextractsegy extracts subbottom profiler, center beam reflection,\n"
    "or seismic reflection data from data supported by MB-System and\n"
    "rewrites it as a SEGY file in the form used by SIOSEIS.";
static const char usage_message[] =
    "mbextractsegy [-Byr/mo/dy/hr/mn/sc/us -Eyr/mo/dy/hr/mn/sc/us -Fformat\n"
    "    -Ifile -Jxscale/yscale -Lstartline/lineroot\n"
    "    -Osegyfile -Qtimelistfile -Rroutefile\n"
    "    -Ssampleformat -Zplotmax -H -V]";

EXTERN_MSC int GMT_mbextractsegy(void *API, int mode, void *args);

/* --- Control structure ---------------------------------------------- */

struct MBEXTRACTSEGY_CTRL {
	struct mbes_B { bool active; int btime_i[7]; }                B;
	struct mbes_E { bool active; int etime_i[7]; }                E;
	struct mbes_F { bool active; int format; }                    F;
	struct mbes_H { bool active; }                                H;
	struct mbes_I { bool active; char file[MB_PATH_MAXLINE]; }    I;
	struct mbes_J { bool active; double xscale, yscale, maxwidth; } J;
	struct mbes_L { bool active; int startline; char lineroot[MB_PATH_MAXLINE]; } L;
	struct mbes_M { bool active; }                                M;
	struct mbes_O { bool active; char root[MB_PATH_MAXLINE]; bool root_set;
	                char suffix[8]; }                             O;
	struct mbes_Q { bool active; char file[MB_PATH_MAXLINE]; }    Q;
	struct mbes_R { bool active; char file[MB_PATH_MAXLINE]; }    R;
	struct mbes_S { bool active; int sampleformat; }              S;
	struct mbes_T { bool active; double timeshift; }              T;
	struct mbes_U { bool active; double rangethreshold; }         U;
	struct mbes_Z { bool active; double zmax; }                   Z;
};

static void *New_mbextractsegy_Ctrl(struct GMT_CTRL *GMT) {
	struct MBEXTRACTSEGY_CTRL *Ctrl = gmt_M_memory(GMT, NULL, 1, struct MBEXTRACTSEGY_CTRL);
	/* defaults from mbextractsegy.cc */
	strncpy(Ctrl->I.file, "datalist.mb-1", MB_PATH_MAXLINE - 1);
	strncpy(Ctrl->L.lineroot, "sbp", MB_PATH_MAXLINE - 1);
	Ctrl->L.startline      = 1;
	Ctrl->S.sampleformat   = MB_SEGY_SAMPLEFORMAT_ENVELOPE;
	Ctrl->J.xscale         = 0.01;
	Ctrl->J.yscale         = 50.0;
	Ctrl->J.maxwidth       = 30.0;
	Ctrl->Z.zmax           = 50.0;
	Ctrl->U.rangethreshold = 25.0;
	Ctrl->T.timeshift      = 0.0;
	snprintf(Ctrl->O.suffix, sizeof(Ctrl->O.suffix), "segy");
	return Ctrl;
}

static void Free_mbextractsegy_Ctrl(struct GMT_CTRL *GMT, struct MBEXTRACTSEGY_CTRL *Ctrl) {
	if (!Ctrl) return;
	gmt_M_free(GMT, Ctrl);
}

static int usage(struct GMTAPI_CTRL *API, int level) {
	gmt_show_name_and_purpose(API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return GMT_NOERROR;
	GMT_Message(API, GMT_TIME_NONE, "usage: %s\n\n", usage_message);
	if (level == GMT_SYNOPSIS) return GMT_PARSE_ERROR;
	GMT_Message(API, GMT_TIME_NONE, "\n%s\n\n", help_message);
	GMT_Message(API, GMT_TIME_NONE,
		"\t-B Beginning time as yr/mo/dy/hr/mn/sc/us.\n"
		"\t-E Ending time as yr/mo/dy/hr/mn/sc/us.\n"
		"\t-F Input format id.\n"
		"\t-I Input swath file or datalist.\n"
		"\t-J xscale/yscale/maxwidth for plotting.\n"
		"\t-L startline/lineroot for auto line numbering.\n"
		"\t-M Check route bearing before writing.\n"
		"\t-O Output root, OR literal 'segy'/'sgy' to override file suffix.\n"
		"\t-Q Time-list file (overrides -R).\n"
		"\t-R Route file.\n"
		"\t-S Sample format: 1=trace 2=envelope 3=analytic.\n"
		"\t-T Time shift (seconds).\n"
		"\t-U Range threshold for waypoint approach (m).\n"
		"\t-Z Max amplitude for plot scaling.\n"
		"\t-H Print description and exit.\n");
	GMT_Option(API, "V,:");
	return GMT_PARSE_ERROR;
}

static int parse(struct GMT_CTRL *GMT, struct MBEXTRACTSEGY_CTRL *Ctrl, struct GMT_OPTION *options) {
	unsigned int n_errors = 0;
	struct GMT_OPTION *opt;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {
		case 'B':
			if (opt->arg && opt->arg[0]) {
				sscanf(opt->arg, "%d/%d/%d/%d/%d/%d", &Ctrl->B.btime_i[0], &Ctrl->B.btime_i[1],
				       &Ctrl->B.btime_i[2], &Ctrl->B.btime_i[3], &Ctrl->B.btime_i[4], &Ctrl->B.btime_i[5]);
				Ctrl->B.btime_i[6] = 0;
				Ctrl->B.active = true;
			} else n_errors++;
			break;
		case 'E':
			if (opt->arg && opt->arg[0]) {
				sscanf(opt->arg, "%d/%d/%d/%d/%d/%d", &Ctrl->E.etime_i[0], &Ctrl->E.etime_i[1],
				       &Ctrl->E.etime_i[2], &Ctrl->E.etime_i[3], &Ctrl->E.etime_i[4], &Ctrl->E.etime_i[5]);
				Ctrl->E.etime_i[6] = 0;
				Ctrl->E.active = true;
			} else n_errors++;
			break;
		case 'F':
			if (opt->arg && opt->arg[0] && sscanf(opt->arg, "%d", &Ctrl->F.format) == 1)
				Ctrl->F.active = true;
			else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax -F option: expected integer format id\n"); n_errors++; }
			break;
		case 'H':
			Ctrl->H.active = true;
			break;
		case 'I':
			if (opt->arg && opt->arg[0]) {
				strncpy(Ctrl->I.file, opt->arg, MB_PATH_MAXLINE - 1);
				Ctrl->I.file[MB_PATH_MAXLINE - 1] = '\0';
				Ctrl->I.active = true;
			} else n_errors++;
			break;
		case 'J':
			if (opt->arg && opt->arg[0]) {
				sscanf(opt->arg, "%lf/%lf/%lf", &Ctrl->J.xscale, &Ctrl->J.yscale, &Ctrl->J.maxwidth);
				Ctrl->J.active = true;
			} else n_errors++;
			break;
		case 'L':
			if (opt->arg && opt->arg[0]) {
				sscanf(opt->arg, "%d/%1023s", &Ctrl->L.startline, Ctrl->L.lineroot);
				Ctrl->L.active = true;
			} else n_errors++;
			break;
		case 'M':
			Ctrl->M.active = true;
			break;
		case 'O':
			if (opt->arg && opt->arg[0]) {
				char tmpstr[MB_PATH_MAXLINE] = {0};
				sscanf(opt->arg, "%1023s", tmpstr);
				if (strcmp(tmpstr, "segy") == 0)
					snprintf(Ctrl->O.suffix, sizeof(Ctrl->O.suffix), "segy");
				else if (strcmp(tmpstr, "sgy") == 0)
					snprintf(Ctrl->O.suffix, sizeof(Ctrl->O.suffix), "sgy");
				else {
					strncpy(Ctrl->O.root, tmpstr, MB_PATH_MAXLINE - 1);
					Ctrl->O.root[MB_PATH_MAXLINE - 1] = '\0';
					Ctrl->O.root_set = true;
				}
				Ctrl->O.active = true;
			} else n_errors++;
			break;
		case 'Q':
			if (opt->arg && opt->arg[0]) {
				sscanf(opt->arg, "%1023s", Ctrl->Q.file);
				Ctrl->Q.active = true;
			} else n_errors++;
			break;
		case 'R':
			if (opt->arg && opt->arg[0]) {
				sscanf(opt->arg, "%1023s", Ctrl->R.file);
				Ctrl->R.active = true;
			} else n_errors++;
			break;
		case 'S':
			if (opt->arg && opt->arg[0] && sscanf(opt->arg, "%d", &Ctrl->S.sampleformat) == 1)
				Ctrl->S.active = true;
			else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax -S option: expected integer sample format\n"); n_errors++; }
			break;
		case 'T':
			if (opt->arg && opt->arg[0] && sscanf(opt->arg, "%lf", &Ctrl->T.timeshift) == 1)
				Ctrl->T.active = true;
			else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax -T option: expected timeshift (s)\n"); n_errors++; }
			break;
		case 'U':
			if (opt->arg && opt->arg[0] && sscanf(opt->arg, "%lf", &Ctrl->U.rangethreshold) == 1)
				Ctrl->U.active = true;
			else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax -U option: expected range threshold (m)\n"); n_errors++; }
			break;
		case 'Z':
			if (opt->arg && opt->arg[0] && sscanf(opt->arg, "%lf", &Ctrl->Z.zmax) == 1)
				Ctrl->Z.active = true;
			else { GMT_Report(API, GMT_MSG_NORMAL, "Syntax -Z option: expected zmax\n"); n_errors++; }
			break;
		default:
			n_errors += gmt_default_error(GMT, opt->option);
			break;
		}
	}

	return n_errors ? GMT_PARSE_ERROR : GMT_OK;
}

/*--------------------------------------------------------------------*/
static int mbes_generateplots(struct GMTAPI_CTRL *API, int verbose, FILE *sfp, char *output_root, char *segy_suffix, int sampleformat,
                              double startlon, double endlon, double startlat, double endlat,
                              double xscale, double yscale, int nwrite, int nshotmax,
                              double seafloordepthmin, double seafloordepthmax,
                              double *seafloordepthminplot, double *seafloordepthmaxplot,
                              double linetracelength, double zmax, int startshot, int endshot,
                              char *lineroot, int linenumber,
                              int *error) {

	if (verbose >= 2) {
		GMT_Report(API, GMT_MSG_NORMAL, "\ndbg2  MBIO function <%s> called\n", __func__);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2  Input arguments:\n");
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       verbose:    %d\n", verbose);
	}

	int status = MB_SUCCESS;
	char command[MB_COMMAND_LENGTH] = {0};

	/* use mbsegyinfo to generate sinf files */
	if (sampleformat == MB_SEGY_SAMPLEFORMAT_ANALYTIC) {
		snprintf(command, sizeof(command), "mbsegyinfo -I %s_cor.%s -O", output_root, segy_suffix);
		GMT_Report(API, GMT_MSG_NORMAL, "Executing: %s\n", command);
		int shellstatus = system(command);
		if (shellstatus != 0) {
			GMT_Report(API, GMT_MSG_NORMAL, "\nSomething went wrong executing:\n\t%s\n", command);
		}
		snprintf(command, sizeof(command), "mbsegyinfo -I %s_hil.%s -O", output_root, segy_suffix);
		GMT_Report(API, GMT_MSG_NORMAL, "Executing: %s\n", command);
		shellstatus = system(command);
		if (shellstatus != 0) {
			GMT_Report(API, GMT_MSG_NORMAL, "\nSomething went wrong executing:\n\t%s\n", command);
		}
		snprintf(command, sizeof(command), "mbsegyinfo -I %s_env.%s -O", output_root, segy_suffix);
		GMT_Report(API, GMT_MSG_NORMAL, "Executing: %s\n", command);
		shellstatus = system(command);
		if (shellstatus != 0) {
			GMT_Report(API, GMT_MSG_NORMAL, "\nSomething went wrong executing:\n\t%s\n", command);
		}
	}
	else if (sampleformat == MB_SEGY_SAMPLEFORMAT_ENVELOPE) {
		snprintf(command, sizeof(command), "mbsegyinfo -I %s_env.%s -O", output_root, segy_suffix);
		GMT_Report(API, GMT_MSG_NORMAL, "Executing: %s\n", command);
		const int shellstatus = system(command);
		if (shellstatus != 0) {
			GMT_Report(API, GMT_MSG_NORMAL, "\nSomething went wrong executing:\n\t%s\n", command);
		}
	}
	else /* if (sampleformat == MB_SEGY_SAMPLEFORMAT_TRACE) */ {
		snprintf(command, sizeof(command), "mbsegyinfo -I %s.%s -O", output_root, segy_suffix);
		GMT_Report(API, GMT_MSG_NORMAL, "Executing: %s\n", command);
		const int shellstatus = system(command);
		if (shellstatus != 0) {
			GMT_Report(API, GMT_MSG_NORMAL, "\nSomething went wrong executing:\n\t%s\n", command);
		}
	}

	/* get bearing and plot scale */
	double mtodeglon, mtodeglat;
	mb_coor_scale(verbose, 0.5 * (startlat + endlat), &mtodeglon, &mtodeglat);
	const double dx = (endlon - startlon) / mtodeglon;
	const double dy = (endlat - startlat) / mtodeglat;
	const double linedistance = sqrt(dx * dx + dy * dy);
	double linebearing = RTD * atan2(dx, dy);
	if (linebearing < 0.0)
		linebearing += 360.0;
	char scale[MB_PATH_MAXLINE] = {0};
	if (linebearing >= 45.0 && linebearing <= 225.0)
		snprintf(scale, sizeof(scale), "-Jx%f/%f", xscale, yscale);
	else
		snprintf(scale, sizeof(scale), "-Jx-%f/%f", xscale, yscale);

	/* output commands to first cut plotting script file */
	/* The maximum useful plot length is about nshotmax shots, so
	    we break longer files up into multiple plots */
	/* const int nshot = endshot - startshot + 1; */
	int nplot = nwrite / nshotmax;
	if (nwrite % nshotmax > 0)
		nplot++;

	/* calculate sweep needed for all of the data in the line - if this is more than 1.0 seconds,
	  then make section plots using only the sweep needed for each section alone */
	double delay = seafloordepthmin / 750.0;
	delay = ((int)(delay / 0.05) - 1) * 0.05;
	double endofdata = seafloordepthmax / 750.0 + linetracelength;
	endofdata = (1 + (int)(endofdata / 0.05)) * 0.05;
	double sweep = endofdata - delay;
	const bool recalculatesweep = sweep > MBES_MAX_SWEEP;

	/* make section plots of correlate, conjugate and envelope from analytic signal */
	if (sampleformat == MB_SEGY_SAMPLEFORMAT_ANALYTIC) {
		fprintf(sfp, "#######################################################################\n");
		fprintf(sfp, "# Generate %d section plot(s) of segy files with root: %s\n", nplot, output_root);
		fprintf(sfp, "#   Three plots of analytic data: correlate, conjugate, and envelope\n");
		fprintf(sfp, "#   Section Start Position: %.6f %.6f\n", startlon, startlat);
		fprintf(sfp, "#   Section End Position:   %.6f %.6f\n", endlon, endlat);
		fprintf(sfp, "#   Section length: %f km\n", linedistance);
		fprintf(sfp, "#   Section bearing: %f degrees\n", linebearing);
		for (int i = 0; i < nplot; i++) {
			if (recalculatesweep) {
				seafloordepthmin = seafloordepthminplot[i];
				seafloordepthmax = seafloordepthmaxplot[i];
				delay = seafloordepthmin / 750.0;
				delay = ((int)(delay / 0.05) - 1) * 0.05;
				endofdata = seafloordepthmax / 750.0 + linetracelength;
				endofdata = (1 + (int)(endofdata / 0.05)) * 0.05;
				sweep = endofdata - delay;
			}

			char plot_type[32];
			char zbounds[32];
			char colormap[32];

			snprintf(plot_type, sizeof(plot_type), "Correlate");
			snprintf(zbounds, sizeof(zbounds),  "-%f/%f", zmax, zmax);
			snprintf(colormap, sizeof(colormap),  "-W1/10 -D");
			snprintf(command, sizeof(command), "\n#   Section %s Plot %d of %d\n", plot_type, i + 1, nplot);
			fprintf(sfp, "%s", command);
			snprintf(command, sizeof(command), "#     Seafloor depth min: %f (m) %f (s)  max: %f (m) %f (s)\n",
			        seafloordepthmin, seafloordepthmin / 750.0, seafloordepthmax, seafloordepthmax / 750.0);
			fprintf(sfp, "%s", command);
			snprintf(command, sizeof(command), "#     Trace length: %f (s)\n", linetracelength);
			fprintf(sfp, "%s", command);
			snprintf(command, sizeof(command), "#     Trace delay: %f (s)\n", delay);
			fprintf(sfp, "%s", command);
			snprintf(command, sizeof(command), "#     Trace sweep: %f (s)\n", sweep);
			fprintf(sfp, "%s", command);

			snprintf(command, sizeof(command), "mbsegygrid -I %s_cor.%s \\\n\t-S0/%d/%d/%d/%d -T%.2f/%.2f \\\n\t-O %s_cor_%d_section\n",
			        output_root, segy_suffix, (startshot + i * nshotmax), MIN((startshot + (i + 1) * nshotmax - 1), endshot),
			        1, 1, sweep, delay, output_root, i + 1);
			fprintf(sfp, "%s", command);

			snprintf(command, sizeof(command),
			        "mbm_grdplot -I %s_cor_%d_section.grd \\\n\t%s -MGO2/2 -Z%s \\\n\t-Ba250/a0.05g0.05 -G1 "
			        "%s -V \\\n\t-O %s_cor_%d_sectionplot \\\n\t-L\"%s Line %d %s Plot %d of %d\"\\\n\t-MIE300 -MITg\n",
			        output_root, i + 1, scale, zbounds, colormap, output_root, i + 1, lineroot, linenumber, plot_type, i + 1, nplot);
			fprintf(sfp, "%s", command);

			snprintf(command, sizeof(command), "%s_cor_%d_sectionplot.cmd $1\n\n", output_root, i + 1);
			fprintf(sfp, "%s", command);

			fflush(sfp);

			snprintf(plot_type, sizeof(plot_type), "Conjugate");
			snprintf(zbounds, sizeof(zbounds),  "-%f/%f", zmax, zmax);
			snprintf(colormap, sizeof(colormap),  "-W1/10 -D");
			snprintf(command, sizeof(command), "\n#   Section %s Plot %d of %d\n", plot_type, i + 1, nplot);
			fprintf(sfp, "%s", command);
			snprintf(command, sizeof(command), "#     Seafloor depth min: %f (m) %f (s)  max: %f (m) %f (s)\n",
			        seafloordepthmin, seafloordepthmin / 750.0, seafloordepthmax, seafloordepthmax / 750.0);
			fprintf(sfp, "%s", command);
			snprintf(command, sizeof(command), "#     Trace length: %f (s)\n", linetracelength);
			fprintf(sfp, "%s", command);
			snprintf(command, sizeof(command), "#     Trace delay: %f (s)\n", delay);
			fprintf(sfp, "%s", command);
			snprintf(command, sizeof(command), "#     Trace sweep: %f (s)\n", sweep);
			fprintf(sfp, "%s", command);

			snprintf(command, sizeof(command), "mbsegygrid -I %s_hil.%s \\\n\t-S0/%d/%d/%d/%d -T%.2f/%.2f \\\n\t-O %s_hil_%d_section\n",
			        output_root, segy_suffix, (startshot + i * nshotmax), MIN((startshot + (i + 1) * nshotmax - 1), endshot),
			        1, 1, sweep, delay, output_root, i + 1);
			fprintf(sfp, "%s", command);

			snprintf(command, sizeof(command),
			        "mbm_grdplot -I %s_hil_%d_section.grd \\\n\t%s -MGO2/2 -Z%s \\\n\t-Ba250/a0.05g0.05 -G1 "
			        "%s -V \\\n\t-O %s_hil_%d_sectionplot \\\n\t-L\"%s Line %d %s Plot %d of %d\"\\\n\t-MIE300 -MITg\n",
			        output_root, i + 1, scale, zbounds, colormap, output_root, i + 1, lineroot, linenumber, plot_type, i + 1, nplot);
			fprintf(sfp, "%s", command);

			snprintf(command, sizeof(command), "%s_hil_%d_sectionplot.cmd $1\n\n", output_root, i + 1);
			fprintf(sfp, "%s", command);

			fflush(sfp);

			snprintf(plot_type, sizeof(plot_type), "Envelope");
			snprintf(zbounds, sizeof(zbounds),  "0.0/%f", zmax);
			snprintf(colormap, sizeof(colormap),  "-W1/4 -D");
			snprintf(command, sizeof(command), "\n#   Section %s Plot %d of %d\n", plot_type, i + 1, nplot);
			fprintf(sfp, "%s", command);
			snprintf(command, sizeof(command), "#     Seafloor depth min: %f (m) %f (s)  max: %f (m) %f (s)\n",
			        seafloordepthmin, seafloordepthmin / 750.0, seafloordepthmax, seafloordepthmax / 750.0);
			fprintf(sfp, "%s", command);
			snprintf(command, sizeof(command), "#     Trace length: %f (s)\n", linetracelength);
			fprintf(sfp, "%s", command);
			snprintf(command, sizeof(command), "#     Trace delay: %f (s)\n", delay);
			fprintf(sfp, "%s", command);
			snprintf(command, sizeof(command), "#     Trace sweep: %f (s)\n", sweep);
			fprintf(sfp, "%s", command);

			snprintf(command, sizeof(command), "mbsegygrid -I %s_env.%s \\\n\t-S0/%d/%d/%d/%d -T%.2f/%.2f \\\n\t-O %s_env_%d_section\n",
			        output_root, segy_suffix, (startshot + i * nshotmax), MIN((startshot + (i + 1) * nshotmax - 1), endshot),
			        1, 1, sweep, delay, output_root, i + 1);
			fprintf(sfp, "%s", command);

			snprintf(command, sizeof(command),
			        "mbm_grdplot -I %s_env_%d_section.grd \\\n\t%s -MGO2/2 -Z%s \\\n\t-Ba250/a0.05g0.05 -G1 "
			        "%s -V \\\n\t-O %s_env_%d_sectionplot \\\n\t-L\"%s Line %d %s Plot %d of %d\"\\\n\t-MIE300 -MITg\n",
			        output_root, i + 1, scale, zbounds, colormap, output_root, i + 1, lineroot, linenumber, plot_type, i + 1, nplot);
			fprintf(sfp, "%s", command);

			snprintf(command, sizeof(command), "%s_env_%d_sectionplot.cmd $1\n\n", output_root, i + 1);
			fprintf(sfp, "%s", command);

			fflush(sfp);
		}
	}

	/* make section plots of envelope from analytic signal */
	else if (sampleformat == MB_SEGY_SAMPLEFORMAT_ENVELOPE) {
		fprintf(sfp, "#######################################################################\n");
		fprintf(sfp, "# Generate %d section plot(s) of segy files with root: %s\n", nplot, output_root);
		fprintf(sfp, "#   One plot of analytic data: envelope\n");
		fprintf(sfp, "#   Section Start Position: %.6f %.6f\n", startlon, startlat);
		fprintf(sfp, "#   Section End Position:   %.6f %.6f\n", endlon, endlat);
		fprintf(sfp, "#   Section length: %f km\n", linedistance);
		fprintf(sfp, "#   Section bearing: %f degrees\n", linebearing);
		for (int i = 0; i < nplot; i++) {
			if (recalculatesweep) {
				seafloordepthmin = seafloordepthminplot[i];
				seafloordepthmax = seafloordepthmaxplot[i];
				delay = seafloordepthmin / 750.0;
				delay = ((int)(delay / 0.05) - 1) * 0.05;
				endofdata = seafloordepthmax / 750.0 + linetracelength;
				endofdata = (1 + (int)(endofdata / 0.05)) * 0.05;
				sweep = endofdata - delay;
			}

			char plot_type[32];
			char zbounds[32];
			char colormap[32];


			snprintf(plot_type, sizeof(plot_type), "Envelope");
			snprintf(zbounds, sizeof(zbounds),  "0.0/%f", zmax);
			snprintf(colormap, sizeof(colormap),  "-W1/4 -D");
			snprintf(command, sizeof(command), "\n#   Section %s Plot %d of %d\n", plot_type, i + 1, nplot);
			fprintf(sfp, "%s", command);
			snprintf(command, sizeof(command), "#     Seafloor depth min: %f (m) %f (s)  max: %f (m) %f (s)\n",
			        seafloordepthmin, seafloordepthmin / 750.0, seafloordepthmax, seafloordepthmax / 750.0);
			fprintf(sfp, "%s", command);
			snprintf(command, sizeof(command), "#     Trace length: %f (s)\n", linetracelength);
			fprintf(sfp, "%s", command);
			snprintf(command, sizeof(command), "#     Trace delay: %f (s)\n", delay);
			fprintf(sfp, "%s", command);
			snprintf(command, sizeof(command), "#     Trace sweep: %f (s)\n", sweep);
			fprintf(sfp, "%s", command);

			snprintf(command, sizeof(command), "mbsegygrid -I %s_env.%s \\\n\t-S0/%d/%d/%d/%d -T%.2f/%.2f \\\n\t-O %s_env_%d_section\n",
			        output_root, segy_suffix, (startshot + i * nshotmax), MIN((startshot + (i + 1) * nshotmax - 1), endshot),
			        1, 1, sweep, delay, output_root, i + 1);
			fprintf(sfp, "%s", command);

			snprintf(command, sizeof(command),
			        "mbm_grdplot -I %s_env_%d_section.grd \\\n\t%s -MGO2/2 -Z%s \\\n\t-Ba250/a0.05g0.05 -G1 "
			        "%s -V \\\n\t-O %s_env_%d_sectionplot \\\n\t-L\"%s Line %d %s Plot %d of %d\"\\\n\t-MIE300 -MITg\n",
			        output_root, i + 1, scale, zbounds, colormap, output_root, i + 1, lineroot, linenumber, plot_type, i + 1, nplot);
			fprintf(sfp, "%s", command);

			snprintf(command, sizeof(command), "%s_env_%d_sectionplot.cmd $1\n\n", output_root, i + 1);
			fprintf(sfp, "%s", command);

			fflush(sfp);
		}
	}

	/* make section plots of single trace signal */
	else /* if (sampleformat == MB_SEGY_SAMPLEFORMAT_TRACE) */ {
		fprintf(sfp, "#######################################################################\n");
		fprintf(sfp, "# Generate %d section plot(s) of segy files with root: %s\n", nplot, output_root);
		fprintf(sfp, "#   One plot of single trace data\n");
		fprintf(sfp, "#   Section Start Position: %.6f %.6f\n", startlon, startlat);
		fprintf(sfp, "#   Section End Position:   %.6f %.6f\n", endlon, endlat);
		fprintf(sfp, "#   Section length: %f km\n", linedistance);
		fprintf(sfp, "#   Section bearing: %f degrees\n", linebearing);
		for (int i = 0; i < nplot; i++) {
			if (recalculatesweep) {
				seafloordepthmin = seafloordepthminplot[i];
				seafloordepthmax = seafloordepthmaxplot[i];
				delay = seafloordepthmin / 750.0;
				delay = ((int)(delay / 0.05) - 1) * 0.05;
				endofdata = seafloordepthmax / 750.0 + linetracelength;
				endofdata = (1 + (int)(endofdata / 0.05)) * 0.05;
				sweep = endofdata - delay;
			}

			char plot_type[32];
			char zbounds[32];
			char colormap[32];

			snprintf(plot_type, sizeof(plot_type), "Trace");
			snprintf(zbounds, sizeof(zbounds),  "-%f/%f", zmax, zmax);
			snprintf(colormap, sizeof(colormap),  "-W1/10 -D");
			snprintf(command, sizeof(command), "\n#   Section %s Plot %d of %d\n", plot_type, i + 1, nplot);
			fprintf(sfp, "%s", command);
			snprintf(command, sizeof(command), "#     Seafloor depth min: %f (m) %f (s)  max: %f (m) %f (s)\n",
			        seafloordepthmin, seafloordepthmin / 750.0, seafloordepthmax, seafloordepthmax / 750.0);
			fprintf(sfp, "%s", command);
			snprintf(command, sizeof(command), "#     Trace length: %f (s)\n", linetracelength);
			fprintf(sfp, "%s", command);
			snprintf(command, sizeof(command), "#     Trace delay: %f (s)\n", delay);
			fprintf(sfp, "%s", command);
			snprintf(command, sizeof(command), "#     Trace sweep: %f (s)\n", sweep);
			fprintf(sfp, "%s", command);

			snprintf(command, sizeof(command), "mbsegygrid -I %s.%s \\\n\t-S0/%d/%d/%d/%d -T%.2f/%.2f \\\n\t-O %s_%d_section\n",
			        output_root, segy_suffix, (startshot + i * nshotmax), MIN((startshot + (i + 1) * nshotmax - 1), endshot),
			        1, 1, sweep, delay, output_root, i + 1);
			fprintf(sfp, "%s", command);

			snprintf(command, sizeof(command),
			        "mbm_grdplot -I %s_%d_section.grd \\\n\t%s -MGO2/2 -Z%s \\\n\t-Ba250/a0.05g0.05 -G1 "
			        "%s -V \\\n\t-O %s_%d_sectionplot \\\n\t-L\"%s Line %d %s Plot %d of %d\"\\\n\t-MIE300 -MITg\n",
			        output_root, i + 1, scale, zbounds, colormap, output_root, i + 1, lineroot, linenumber, plot_type, i + 1, nplot);
			fprintf(sfp, "%s", command);

			snprintf(command, sizeof(command), "%s_%d_sectionplot.cmd $1\n\n", output_root, i + 1);
			fprintf(sfp, "%s", command);

			fflush(sfp);
		}
	}

	if (verbose >= 2) {
		GMT_Report(API, GMT_MSG_NORMAL, "\ndbg2  MBIO function <%s> completed\n", __func__);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2  Return values:\n");
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       error:      %d\n", *error);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2  Return status:\n");
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       status:  %d\n", status);
	}

	return (status);
}

#define bailout(code)  { gmt_M_free_options(mode); return code; }
#define Return(code)   { Free_mbextractsegy_Ctrl(GMT, Ctrl); gmt_end_module(GMT, GMT_cpy); bailout(code); }

/*--------------------------------------------------------------------*/
int GMT_mbextractsegy(void *V_API, int mode, void *args) {
	struct MBEXTRACTSEGY_CTRL *Ctrl = NULL;
	struct GMT_CTRL        *GMT  = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION      *options = NULL;
	struct GMTAPI_CTRL     *API = gmt_get_api_ptr(V_API);

	if (API == NULL) return GMT_NOT_A_SESSION;
	if (mode == GMT_MODULE_PURPOSE) return usage(API, GMT_MODULE_PURPOSE);
	options = GMT_Create_Options(API, mode, args);
	if (API->error) return API->error;
	if (!options || options->option == GMT_OPT_USAGE)   bailout(usage(API, GMT_USAGE));
	if (options->option == GMT_OPT_SYNOPSIS)            bailout(usage(API, GMT_SYNOPSIS));

#if GMT_MAJOR_VERSION >= 6
	if ((GMT = gmt_init_module(API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout(API->error);
#else
	GMT = gmt_begin_module(API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy);
#endif
	if (GMT_Parse_Common(API, THIS_MODULE_OPTIONS, options)) Return(API->error);

	Ctrl = New_mbextractsegy_Ctrl(GMT);
	{ int perr = parse(GMT, Ctrl, options); if (perr) Return(perr); }

	int verbose = GMT->common.V.active;
	if (GMT->current.setting.verbose >= GMT_MSG_DEBUG) verbose = 2;

	int format;
	int pings;
	int lonflip;
	double bounds[4];
	int btime_i[7];
	int etime_i[7];
	double speedmin;
	double timegap;
	int status = mb_defaults(verbose, &format, &pings, &lonflip, bounds, btime_i, etime_i, &speedmin, &timegap);

	/* apply parsed control values */
	if (Ctrl->B.active) memcpy(btime_i, Ctrl->B.btime_i, sizeof(btime_i));
	if (Ctrl->E.active) memcpy(etime_i, Ctrl->E.etime_i, sizeof(etime_i));
	if (Ctrl->F.active) format = Ctrl->F.format;

	char read_file[MB_PATH_MAXLINE];
	strncpy(read_file, Ctrl->I.file, MB_PATH_MAXLINE - 1);
	read_file[MB_PATH_MAXLINE - 1] = '\0';

	char lineroot[MB_PATH_MAXLINE];
	strncpy(lineroot, Ctrl->L.lineroot, MB_PATH_MAXLINE - 1);
	lineroot[MB_PATH_MAXLINE - 1] = '\0';
	int startline = Ctrl->L.startline;

	int sampleformat = Ctrl->S.sampleformat;

	char route_file[MB_PATH_MAXLINE] = {0};
	bool route_file_set = false;
	if (Ctrl->R.active) {
		strncpy(route_file, Ctrl->R.file, MB_PATH_MAXLINE - 1);
		route_file_set = true;
	}
	double timeshift      = Ctrl->T.timeshift;
	double maxwidth       = Ctrl->J.maxwidth;
	double xscale         = Ctrl->J.xscale;
	double yscale         = Ctrl->J.yscale;
	double zmax           = Ctrl->Z.zmax;
	bool checkroutebearing = Ctrl->M.active;
	double rangethreshold = Ctrl->U.rangethreshold;
	char timelist_file[MB_PATH_MAXLINE] = {0};
	bool timelist_file_set = false;
	if (Ctrl->Q.active) {
		strncpy(timelist_file, Ctrl->Q.file, MB_PATH_MAXLINE - 1);
		timelist_file_set = true;
	}
	char output_root[MB_PATH_MAXLINE] = {0};
	bool output_root_set = Ctrl->O.root_set;
	if (output_root_set) {
		strncpy(output_root, Ctrl->O.root, MB_PATH_MAXLINE - 1);
	}
	int waypoint;
	char segy_suffix[8];
	strncpy(segy_suffix, Ctrl->O.suffix, sizeof(segy_suffix) - 1);
	segy_suffix[sizeof(segy_suffix) - 1] = '\0';

	const bool help = Ctrl->H.active;

	if (verbose == 1 || help) {
		GMT_Report(API, GMT_MSG_NORMAL, "\nProgram %s\n", program_name);
		GMT_Report(API, GMT_MSG_NORMAL, "MB-system Version %s\n", MB_VERSION);
	}

	if (verbose >= 2) {
		GMT_Report(API, GMT_MSG_NORMAL, "\ndbg2  Program <%s>\n", program_name);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2  MB-system Version %s\n", MB_VERSION);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2  Control Parameters:\n");
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       verbose:           %d\n", verbose);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       help:              %d\n", (int)help);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       format:            %d\n", format);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       pings:             %d\n", pings);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       lonflip:           %d\n", lonflip);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       bounds[0]:         %f\n", bounds[0]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       bounds[1]:         %f\n", bounds[1]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       bounds[2]:         %f\n", bounds[2]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       bounds[3]:         %f\n", bounds[3]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       btime_i[0]:        %d\n", btime_i[0]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       btime_i[1]:        %d\n", btime_i[1]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       btime_i[2]:        %d\n", btime_i[2]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       btime_i[3]:        %d\n", btime_i[3]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       btime_i[4]:        %d\n", btime_i[4]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       btime_i[5]:        %d\n", btime_i[5]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       btime_i[6]:        %d\n", btime_i[6]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       etime_i[0]:        %d\n", etime_i[0]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       etime_i[1]:        %d\n", etime_i[1]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       etime_i[2]:        %d\n", etime_i[2]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       etime_i[3]:        %d\n", etime_i[3]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       etime_i[4]:        %d\n", etime_i[4]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       etime_i[5]:        %d\n", etime_i[5]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       etime_i[6]:        %d\n", etime_i[6]);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       speedmin:          %f\n", speedmin);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       timegap:           %f\n", timegap);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       sampleformat:      %d\n", sampleformat);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       timeshift:         %f\n", timeshift);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       timelist_file_set: %d\n", (int)timelist_file_set);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       timelist_file:     %s\n", timelist_file);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       route_file_set:    %d\n", (int)route_file_set);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       route_file:        %s\n", route_file);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       checkroutebearing: %d\n", (int)checkroutebearing);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       output_root_set:   %d\n", (int)output_root_set);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       output_root:       %s\n", output_root);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       lineroot:          %s\n", lineroot);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       segy_suffix:       %s\n", segy_suffix);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       xscale:            %f\n", xscale);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       yscale:            %f\n", yscale);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       maxwidth:          %f\n", maxwidth);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       rangethreshold:    %f\n", rangethreshold);
	}

	if (help) {
		GMT_Report(API, GMT_MSG_NORMAL, "\n%s\n", help_message);
		GMT_Report(API, GMT_MSG_NORMAL, "\nusage: %s\n", usage_message);
		Return(MB_ERROR_NO_ERROR);
	}

	void *datalist = NULL;
	double file_weight = 1.0;
	double btime_d;
	double etime_d;
	char file[MB_PATH_MAXLINE] = "";
	char dfile[MB_PATH_MAXLINE] = "";
	int beams_bath;
	int beams_amp;
	int pixels_ss;

	/* MBIO read values */
	void *mbio_ptr = NULL;
	void *store_ptr = NULL;
	int kind;
	int time_i[7];
	int time_j[5];
	double time_d;
	double navlon;
	double navlat;
	double speed;
	double heading;
	double distance;
	double altitude;
	double sensordepth;
	char *beamflag = NULL;
	double *bath = NULL;
	double *bathacrosstrack = NULL;
	double *bathalongtrack = NULL;
	double *amp = NULL;
	double *ss = NULL;
	double *ssacrosstrack = NULL;
	double *ssalongtrack = NULL;
	char comment[MB_COMMENT_MAXLINE];
	int icomment = 0;

	/* segy data */
	int samplesize = 0;
	struct mb_segytraceheader_struct segytraceheader;
	int segydata_alloc = 0;
	float *segydata = NULL;
	int buffer_alloc = 0;
	char *buffer = NULL;

	/* route and auto-line data */
	double *routetime_d = NULL;
	int nroutepoint = 0;
	double lon;
	double lat;
	double topo;
	double *routelon = NULL;
	double *routelat = NULL;
	double *routeheading = NULL;
	int *routewaypoint = NULL;
	double range = 0.0;
	double rangelast = 0.0;
	int activewaypoint = 0;

	/* auto plotting */
	FILE *sfp = NULL;
	char scriptfile[MB_PATHPLUS_MAXLINE] = "";
	double seafloordepthmin = -1.0;
	double seafloordepthmax = -1.0;
	double seafloordepthminplot[MBES_NUM_PLOT_MAX];
	double seafloordepthmaxplot[MBES_NUM_PLOT_MAX];
	double sweep;
	double delay;
	double startlon;
	double startlat;
	int startshot;
	double endlon;
	double endlat;
	int endshot;
	double linedistance;
	double linebearing;
	int nplot = 0;
	(void)sweep;
	(void)delay;
	(void)linedistance;
	(void)linebearing;

	char command[MB_COMMAND_LENGTH] = {0};
	double mtodeglon, mtodeglat;
	double headingdiff;
	int oktowrite;
	FILE *fpc = NULL;
	FILE *fph = NULL;
	FILE *fpe = NULL;
	char *result;
	int nwrite;
	double tracemin, tracemax, tracerms, tracelength;
	double linetracemin, linetracemax, linetracelength, endofdata;
	double draft, roll, pitch, heave;
	(void)linetracemin;
	(void)linetracemax;
	(void)endofdata;

	/* set starting line number */
	int linenumber = startline;

	/* set maximum number of shots per plot */
	int nshotmax = (int)(maxwidth / xscale);

	/* bool rawroutefile = false; */
	bool rangeok;
	int error = MB_ERROR_NO_ERROR;

	/* if specified read route time list file */
	int nroutepointalloc = 0;
	if (timelist_file_set) {
		/* open the input file */
		FILE *fp = NULL;
		if ((fp = fopen(timelist_file, "r")) == NULL) {
			error = MB_ERROR_OPEN_FAIL;
			GMT_Report(API, GMT_MSG_NORMAL, "\nUnable to open time list file <%s> for reading\n", timelist_file);
			Return(status);
		}
		/* rawroutefile = false; */
		while ((result = fgets(comment, MB_PATH_MAXLINE, fp)) == comment) {
			if (comment[0] != '#') {
				{
					int cnt;
					int tmp;
					/* const int nget = */
					sscanf(comment, "%d %d %lf %lf %lf %lf", &cnt, &tmp, &lon, &lat, &heading, &time_d);
					if (tmp >= 0 && tmp <= 4)
						waypoint = tmp;
					else
						waypoint = MBES_ROUTE_WAYPOINT_NONE;
				}

				/* if good data check for need to allocate more space */
				if (nroutepoint + 1 > nroutepointalloc) {
					nroutepointalloc += MBES_ALLOC_NUM;
					int reallocd_status = mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(double), (void **)&routelon, &error);
					reallocd_status &= mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(double), (void **)&routelat, &error);
					reallocd_status &= mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(double), (void **)&routeheading, &error);
					reallocd_status &= mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(int), (void **)&routewaypoint, &error);
					reallocd_status &= mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(double), (void **)&routetime_d, &error);
					if (reallocd_status != MB_SUCCESS) {
						char *message;
						mb_error(verbose, error, &message);
						GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error allocating data arrays:\n%s\n", message);
						GMT_Report(API, GMT_MSG_NORMAL, "\nProgram <%s> Terminated\n", program_name);
						Return(error);
					}
				}

				/* add good point to route */
				if (nroutepointalloc > nroutepoint) {
					routewaypoint[nroutepoint] = waypoint;
					routelon[nroutepoint] = lon;
					routelat[nroutepoint] = lat;
					routeheading[nroutepoint] = heading;
					routetime_d[nroutepoint] = time_d;
					nroutepoint++;
				}
			}
		}

		/* close the file */
		fclose(fp);

		/* set starting values */
		activewaypoint = 0;
		mb_coor_scale(verbose, routelat[activewaypoint], &mtodeglon, &mtodeglat);
		rangelast = 1000 * rangethreshold;
		seafloordepthmin = -1.0;
		seafloordepthmax = -1.0;
		nplot = 0;
		for (int i = 0; i < MBES_NUM_PLOT_MAX; i++) {
			seafloordepthminplot[i] = -1;
			seafloordepthmaxplot[i] = -1;
		}
		oktowrite = 0;
		rangeok = false;

		/* output status */
		if (verbose > 0) {
			/* output info on file output */
			GMT_Report(API, GMT_MSG_NORMAL, "Read %d waypoints from time list file: %s\n", nroutepoint, timelist_file);
		}
	}

	/* if specified read route file */
	else if (route_file_set) {
		/* open the input file */
		FILE *fp = NULL;
		if ((fp = fopen(route_file, "r")) == NULL) {
			error = MB_ERROR_OPEN_FAIL;
			status = MB_FAILURE;
			GMT_Report(API, GMT_MSG_NORMAL, "\nUnable to open route file <%s> for reading\n", route_file);
			Return(status);
		}
		bool rawroutefile = false;  /* TODO(schwehr): Explain how this flag works.  Suspicious */
		while ((result = fgets(comment, MB_PATH_MAXLINE, fp)) == comment) {
			if (comment[0] == '#') {
				if (strncmp(comment, "## Route File Version", 21) == 0) {
					rawroutefile = false;
				}
			}
			else {
				int waypoint_tmp;
				const int nget = sscanf(comment, "%lf %lf %lf %d %lf", &lon, &lat, &topo, &waypoint_tmp, &heading);
				waypoint = waypoint_tmp;  /* TODO(schwehr): Range check */
				if (comment[0] == '#') {
					GMT_Report(API, GMT_MSG_NORMAL, "buffer:%s", comment);
					if (strncmp(comment, "## Route File Version", 21) == 0) {
						rawroutefile = false;
					}
				}
				bool point_ok;
				if ((rawroutefile && nget >= 2) ||
				    (!rawroutefile && nget >= 3 && waypoint > MBES_ROUTE_WAYPOINT_NONE))
					point_ok = true;
				else
					point_ok = false;

				/* if good data check for need to allocate more space */
				if (point_ok && nroutepoint + 1 > nroutepointalloc) {
					nroutepointalloc += MBES_ALLOC_NUM;
					int reallocd_status = mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(double), (void **)&routelon, &error);
					reallocd_status &= mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(double), (void **)&routelat, &error);
					reallocd_status &= mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(double), (void **)&routeheading, &error);
					reallocd_status &= mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(int), (void **)&routewaypoint, &error);
					if (reallocd_status != MB_SUCCESS) {
						char *message;
						mb_error(verbose, error, &message);
						GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error allocating data arrays:\n%s\n", message);
						GMT_Report(API, GMT_MSG_NORMAL, "\nProgram <%s> Terminated\n", program_name);
						Return(error);
					}
				}

				/* add good point to route */
				if (point_ok && nroutepointalloc > nroutepoint + 1) {
					routelon[nroutepoint] = lon;
					routelat[nroutepoint] = lat;
					routeheading[nroutepoint] = heading;
					routewaypoint[nroutepoint] = waypoint;
					nroutepoint++;
				}
			}
		}

		/* close the file */
		fclose(fp);

		/* set starting values */
		activewaypoint = 1;
		mb_coor_scale(verbose, routelat[activewaypoint], &mtodeglon, &mtodeglat);
		rangelast = 1000 * rangethreshold;
		seafloordepthmin = -1.0;
		seafloordepthmax = -1.0;
		nplot = 0;
		for (int i = 0; i < MBES_NUM_PLOT_MAX; i++) {
			seafloordepthminplot[i] = -1;
			seafloordepthmaxplot[i] = -1;
		}
		oktowrite = 0;
		rangeok = false;

		/* output status */
		if (verbose > 0) {
			/* output info on file output */
			GMT_Report(API, GMT_MSG_NORMAL, "Read %d waypoints from route file: %s\n", nroutepoint, route_file);
		}
	}

	/* output segy file(s) not open */
	bool output_open = false;

	/* get format if required */
	if (format == 0)
		mb_get_format(verbose, read_file, NULL, &format, &error);

	/* get sample size from sampleformat */
	if (sampleformat == MB_SEGY_SAMPLEFORMAT_ANALYTIC)
		samplesize = 2 * sizeof(float);
	else
		samplesize = sizeof(float);

	/* determine whether to read one file or a list of files */
	const bool read_datalist = format < 0;
	bool read_data;

	/* open file list */
	if (read_datalist) {
		const int look_processed = MB_DATALIST_LOOK_UNSET;
		if (mb_datalist_open(verbose, &datalist, read_file, look_processed, &error) != MB_SUCCESS) {
			GMT_Report(API, GMT_MSG_NORMAL, "\nUnable to open data list file: %s\n", read_file);
			GMT_Report(API, GMT_MSG_NORMAL, "\nProgram <%s> Terminated\n", program_name);
			Return(MB_ERROR_OPEN_FAIL);
		}
		read_data = mb_datalist_read(verbose, datalist, file, dfile, &format, &file_weight, &error) == MB_SUCCESS;
	} else {
		/* else copy single filename to be read */
		strcpy(file, read_file);
		read_data = true;
	}

	/* set up plotting script file */
	if ((route_file_set && nroutepoint > 1) || (timelist_file_set && nroutepoint > 1)) {
		snprintf(scriptfile, sizeof(scriptfile), "%s_section.cmd", lineroot);
	}
	else if (!output_root_set || read_datalist) {
		snprintf(scriptfile, sizeof(scriptfile), "%s_section.cmd", read_file);
	}
	else {
		snprintf(scriptfile, sizeof(scriptfile), "%s_section.cmd", output_root);
	}
	if ((sfp = fopen(scriptfile, "w")) == NULL) {
		error = MB_ERROR_OPEN_FAIL;
		status = MB_FAILURE;
		GMT_Report(API, GMT_MSG_NORMAL, "\nUnable to open plotting script file <%s> \n", scriptfile);
		Return(status);
	}

	struct mb_segyasciiheader_struct segyasciiheader;
	for (int j = 0; j < 40; j++)
		for (int i = 0; i < 80; i++)
			segyasciiheader.line[j][i] = 0;

	struct mb_segyfileheader_struct segyfileheader;
	segyfileheader.jobid = 0;
	segyfileheader.line = 0;
	segyfileheader.reel = 0;
	segyfileheader.channels = 0;
	segyfileheader.aux_channels = 0;
	segyfileheader.sample_interval = 0;
	segyfileheader.sample_interval_org = 0;
	segyfileheader.number_samples = 0;
	segyfileheader.number_samples_org = 0;
	segyfileheader.format = 5;
	segyfileheader.cdp_fold = 0;
	segyfileheader.trace_sort = 0;
	segyfileheader.vertical_sum = 0;
	segyfileheader.sweep_start = 0;
	segyfileheader.sweep_end = 0;
	segyfileheader.sweep_length = 0;
	segyfileheader.sweep_type = 0;
	segyfileheader.sweep_trace = 0;
	segyfileheader.sweep_taper_start = 0;
	segyfileheader.sweep_taper_end = 0;
	segyfileheader.sweep_taper = 0;
	segyfileheader.correlated = 0;
	segyfileheader.binary_gain = 0;
	segyfileheader.amplitude = 0;
	segyfileheader.units = 0;
	segyfileheader.impulse_polarity = 0;
	segyfileheader.domain = 0;
	for (int i = 0; i < 338; i++)
		segyfileheader.extra[i] = 0;

	bool linechange = false;

	/* loop over all files to be read */
	while (read_data) {

		/* initialize reading the swath file */
		if (mb_read_init(verbose, file, format, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap, &mbio_ptr,
		                           &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error) != MB_SUCCESS) {
			char *message;
			mb_error(verbose, error, &message);
			GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", message);
			GMT_Report(API, GMT_MSG_NORMAL, "\nMultibeam File <%s> not initialized for reading\n", file);
			GMT_Report(API, GMT_MSG_NORMAL, "\nProgram <%s> Terminated\n", program_name);
			Return(error);
		}

		/* allocate memory for data arrays */
		if (error == MB_ERROR_NO_ERROR)
			/* status &= */ mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag, &error);
		if (error == MB_ERROR_NO_ERROR)
			/* status &= */ mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, &error);
		if (error == MB_ERROR_NO_ERROR)
			/* status &= */ mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, &error);
		if (error == MB_ERROR_NO_ERROR)
			/* status &= */
			    mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathacrosstrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			/* status &= */
			    mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathalongtrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			/* status &= */ mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &error);
		if (error == MB_ERROR_NO_ERROR)
			/* status &= */ mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssacrosstrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			/* status &= */ mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssalongtrack, &error);

		/* if error initializing memory then quit */
		if (error != MB_ERROR_NO_ERROR) {
			char *message;
			mb_error(verbose, error, &message);
			GMT_Report(API, GMT_MSG_NORMAL, "\nMBIO Error allocating data arrays:\n%s\n", message);
			GMT_Report(API, GMT_MSG_NORMAL, "\nProgram <%s> Terminated\n", program_name);
			Return(error);
		}

		/* read and print data */
		int nread = 0;
		/* bool first = true; */
		double lastlon = 0.0;
		double lastlat = 0.0;
		double lastheading = 0.0;
		double lastdistance = 0.0;
		while (error <= MB_ERROR_NO_ERROR) {
			/* reset error */
			error = MB_ERROR_NO_ERROR;

			/* read next data record */
			status = mb_get_all(verbose, mbio_ptr, &store_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed, &heading,
			                    &distance, &altitude, &sensordepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp,
			                    bathacrosstrack, bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment, &error);

			/* ignore nonfatal errors */
			if (error < 0) {
				error = MB_ERROR_NO_ERROR;
				status = MB_SUCCESS;
			}

			/* deal with nav and time from survey data only - not nav, sidescan, or subbottom */
			if (error <= MB_ERROR_NO_ERROR && kind == MB_DATA_DATA) {
				/* reset output flag */
				linechange = false;

				/* get nav data */
				status = mb_extract_nav(verbose, mbio_ptr, store_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed, &heading,
				                        &draft, &roll, &pitch, &heave, &error);

				/* save last nav and heading */
				if (navlon != 0.0)
					lastlon = navlon;
				if (navlat != 0.0)
					lastlat = navlat;
				if (heading != 0.0)
					lastheading = heading;
				if (distance != 0.0)
					lastdistance = distance;

				/* to set lines check survey data time against time list */
				if (nroutepoint > 1) {
					const double dx = (navlon - routelon[activewaypoint]) / mtodeglon;
					const double dy = (navlat - routelat[activewaypoint]) / mtodeglat;
					range = sqrt(dx * dx + dy * dy);
					if (activewaypoint < nroutepoint && time_d >= routetime_d[activewaypoint]) {
						linechange = true;
					}
				}

				/* else to set lines check survey data position against waypoints */
				else if (nroutepoint > 1 && navlon != 0.0 && navlat != 0.0) {
					const double dx = (navlon - routelon[activewaypoint]) / mtodeglon;
					const double dy = (navlat - routelat[activewaypoint]) / mtodeglat;
					range = sqrt(dx * dx + dy * dy);
					if (range < rangethreshold)
						rangeok = true;
					if (rangeok && (activewaypoint == 0 || range > rangelast) && activewaypoint < nroutepoint - 1) {
						linechange = true;
					}
				}

				/* apply line change */
				if (linechange) {
					/* close current output file */
					if (output_open) {
						if (fpc != NULL) {
							fclose(fpc);
							fpc = NULL;
						}
						if (fph != NULL) {
							fclose(fph);
							fph = NULL;
						}
						if (fpe != NULL) {
							fclose(fpe);
							fpe = NULL;
						}
						output_open = false;

						/* output count of segy records */
						GMT_Report(API, GMT_MSG_NORMAL, "%d records output to segy files with root %s\n", nwrite, output_root);
						if (verbose > 0)
							GMT_Report(API, GMT_MSG_NORMAL, "\n");

						status = mbes_generateplots(API, verbose, sfp, output_root, segy_suffix, sampleformat,
						                startlon, endlon, startlat, endlat,
						                xscale, yscale, nwrite, nshotmax,
						                seafloordepthmin, seafloordepthmax,
						                seafloordepthminplot, seafloordepthmaxplot,
						                linetracelength, zmax, startshot, endshot,
						                lineroot, linenumber,
						                &error);
					}

					/* increment line number */
					if (activewaypoint > 0)
						linenumber++;

					/* increment active waypoint */
					activewaypoint++;

					mb_coor_scale(verbose, routelat[activewaypoint], &mtodeglon, &mtodeglat);
					rangelast = 1000 * rangethreshold;
					seafloordepthmin = -1.0;
					seafloordepthmax = -1.0;
					nplot = 0;
					for (int i = 0; i < MBES_NUM_PLOT_MAX; i++) {
						seafloordepthminplot[i] = -1;
						seafloordepthmaxplot[i] = -1;
					}
					oktowrite = 0;
					rangeok = false;
				}
				else
					rangelast = range;
				if (verbose > 0 && nroutepoint > 0)
					GMT_Report(API, GMT_MSG_NORMAL,
					        "> activewaypoint:%d linenumber:%d time_d:%f range:%f   lon: %f %f   lat: %f %f oktowrite:%d "
					        "rangeok:%d kind:%d\n",
					        activewaypoint, linenumber, time_d, range, navlon, routelon[activewaypoint], navlat,
					        routelat[activewaypoint], oktowrite, rangeok, kind);
			}

			/* if desired extract subbottom data */
			if (error == MB_ERROR_NO_ERROR &&
			    (kind == MB_DATA_SUBBOTTOM_MCS || kind == MB_DATA_SUBBOTTOM_CNTRBEAM || kind == MB_DATA_SUBBOTTOM_SUBBOTTOM)) {
				/* extract the header */
				status = mb_extract_segytraceheader(verbose, mbio_ptr, store_ptr, &kind, (void *)&segytraceheader, &error);

				/* allocate the required memory */
				if (status == MB_SUCCESS && segytraceheader.nsamps > segydata_alloc) {
					status =
					    mb_mallocd(verbose, __FILE__, __LINE__, segytraceheader.nsamps * samplesize, (void **)&segydata, &error);
					if (status == MB_SUCCESS)
						segydata_alloc = segytraceheader.nsamps;
					else
						segydata_alloc = 0;
				}
				if (status == MB_SUCCESS &&
				    (buffer_alloc < MB_SEGY_TRACEHEADER_LENGTH || buffer_alloc < segytraceheader.nsamps * samplesize)) {
					buffer_alloc = MAX(MB_SEGY_TRACEHEADER_LENGTH, segytraceheader.nsamps * samplesize);
					status = mb_mallocd(verbose, __FILE__, __LINE__, buffer_alloc, (void **)&buffer, &error);
					if (status != MB_SUCCESS)
						buffer_alloc = 0;
				}

				/* extract the data */
				if (status == MB_SUCCESS)
					status = mb_extract_segy(verbose, mbio_ptr, store_ptr, &sampleformat, &kind, (void *)&segytraceheader,
					                         segydata, &error);

				/* apply time shift if needed */
				if (status == MB_SUCCESS && timeshift != 0.0) {
					time_j[0] = segytraceheader.year;
					time_j[1] = segytraceheader.day_of_yr;
					time_j[2] = 60 * segytraceheader.hour + segytraceheader.min;
					time_j[3] = segytraceheader.sec;
					time_j[4] = 1000 * segytraceheader.mils;
					mb_get_itime(verbose, time_j, time_i);
					mb_get_time(verbose, time_i, &time_d);
					time_d += timeshift;
					mb_get_date(verbose, time_d, time_i);
					mb_get_jtime(verbose, time_i, time_j);
					segytraceheader.year = time_i[0];
					segytraceheader.day_of_yr = time_j[1];
					segytraceheader.hour = time_i[3];
					segytraceheader.min = time_i[4];
					segytraceheader.sec = time_i[5];
					segytraceheader.mils = time_i[6] / 1000;
				}

				/* set nav and heading using most recent survey data */
				segytraceheader.src_long = (int)(lastlon * 360000.0);
				segytraceheader.src_lat = (int)(lastlat * 360000.0);
				segytraceheader.grp_long = (int)(lastlon * 360000.0);
				segytraceheader.grp_lat = (int)(lastlat * 360000.0);
				segytraceheader.heading = lastheading;
				segytraceheader.roll = roll;
				segytraceheader.pitch = pitch;
				segytraceheader.distance = 1000.0 * lastdistance;

				/* if following a route check that the vehicle has come on line
				    (within MBES_ONLINE_THRESHOLD degrees)
				    before writing any data */
				if (activewaypoint > 0 && checkroutebearing && nroutepoint > 1) {
					headingdiff = fabs(routeheading[activewaypoint - 1] - segytraceheader.heading);
					if (headingdiff > 180.0)
						headingdiff = 360.0 - headingdiff;
					if (headingdiff < MBES_ONLINE_THRESHOLD)
						oktowrite++;
					else
						oktowrite = 0;
				}
				else if (activewaypoint > 0)
					oktowrite = MBES_ONLINE_COUNT;
				else if (nroutepoint == 0 && nroutepoint == 0)
					oktowrite = MBES_ONLINE_COUNT;

				/* open output segy file(s) if needed */
				if (!output_open && oktowrite) {
					nwrite = 0;
					char output_file[MB_PATHPLUS_MAXLINE];
					if (!output_root_set) {
						snprintf(output_root, sizeof(output_root), "%s_%4.4d", lineroot, linenumber);
					}
					if (sampleformat == MB_SEGY_SAMPLEFORMAT_ANALYTIC) {
						snprintf(output_file, sizeof(output_file), "%s_cor.%s", output_root, segy_suffix);
						if ((fpc = fopen(output_file, "w")) == NULL) {
							GMT_Report(API, GMT_MSG_NORMAL, "\nError opening output segy file:\n%s\n", output_file);
							GMT_Report(API, GMT_MSG_NORMAL, "\nProgram <%s> Terminated\n", program_name);
							Return(MB_ERROR_WRITE_FAIL);
						}
						else if (verbose > 0) {
							/* output info on file output */
							GMT_Report(API, GMT_MSG_NORMAL, "Outputting subbottom correlate data to segy file %s\n", output_file);
						}

						snprintf(output_file, sizeof(output_file), "%s_hil.%s", output_root, segy_suffix);
						if ((fph = fopen(output_file, "w")) == NULL) {
							GMT_Report(API, GMT_MSG_NORMAL, "\nError opening output segy file:\n%s\n", output_file);
							GMT_Report(API, GMT_MSG_NORMAL, "\nProgram <%s> Terminated\n", program_name);
							Return(MB_ERROR_WRITE_FAIL);
						}
						else if (verbose > 0) {
							/* output info on file output */
							GMT_Report(API, GMT_MSG_NORMAL, "Outputting subbottom correlate conjugate data to segy file %s\n", output_file);
						}

						snprintf(output_file, sizeof(output_file), "%s_env.%s", output_root, segy_suffix);
						if ((fpe = fopen(output_file, "w")) == NULL) {
							GMT_Report(API, GMT_MSG_NORMAL, "\nError opening output segy file:\n%s\n", output_file);
							GMT_Report(API, GMT_MSG_NORMAL, "\nProgram <%s> Terminated\n", program_name);
							Return(MB_ERROR_WRITE_FAIL);
						}
						else if (verbose > 0) {
							/* output info on file output */
							GMT_Report(API, GMT_MSG_NORMAL, "Outputting subbottom envelope data to segy file %s\n", output_file);
						}
					}
					else if (sampleformat == MB_SEGY_SAMPLEFORMAT_ENVELOPE) {
						snprintf(output_file, sizeof(output_file), "%s_env.%s", output_root, segy_suffix);
						if ((fpe = fopen(output_file, "w")) == NULL) {
							GMT_Report(API, GMT_MSG_NORMAL, "\nError opening output segy file:\n%s\n", output_file);
							GMT_Report(API, GMT_MSG_NORMAL, "\nProgram <%s> Terminated\n", program_name);
							Return(MB_ERROR_WRITE_FAIL);
						}
						else if (verbose > 0) {
							/* output info on file output */
							GMT_Report(API, GMT_MSG_NORMAL, "Outputting subbottom envelope data to segy file %s\n", output_file);
						}
					}
					else /* if (sampleformat == MB_SEGY_SAMPLEFORMAT_TRACE) */ {
						snprintf(output_file, sizeof(output_file), "%s.%s", output_root, segy_suffix);
						if ((fpe = fopen(output_file, "w")) == NULL) {
							GMT_Report(API, GMT_MSG_NORMAL, "\nError opening output segy file:\n%s\n", output_file);
							GMT_Report(API, GMT_MSG_NORMAL, "\nProgram <%s> Terminated\n", program_name);
							Return(MB_ERROR_WRITE_FAIL);
						}
						else if (verbose > 0) {
							/* output info on file output */
							GMT_Report(API, GMT_MSG_NORMAL, "Outputting trace data to segy file %s\n", output_file);
						}
					}
					output_open = true;
				}

				/* note good status */
				if (status == MB_SUCCESS) {
					/* get trace min and max */
					tracemin = segydata[0];
					tracemax = segydata[0];
					tracerms = 0.0;
					for (int i = 0; i < segytraceheader.nsamps; i++) {
						tracemin = MIN(tracemin, (double)segydata[i]);
						tracemax = MAX(tracemax, (double)segydata[i]);
						tracerms += segydata[i] * segydata[i];
					}
					tracerms = sqrt(tracerms / segytraceheader.nsamps);
					tracelength = 0.000001 * segytraceheader.si_micros * segytraceheader.nsamps;

					/* get starting and ending positions */
					if (nwrite == 0) {
						startlon = ((double)segytraceheader.src_long) / 360000.0;
						startlat = ((double)segytraceheader.src_lat) / 360000.0;
						startshot = segytraceheader.shot_num;
						linetracemin = tracemin;
						linetracemax = tracemax;
						linetracelength = tracelength;
					}
					else {
						endlon = ((double)segytraceheader.src_long) / 360000.0;
						endlat = ((double)segytraceheader.src_lat) / 360000.0;
						endshot = segytraceheader.shot_num;
						linetracemin = MIN(tracemin, linetracemin);
						linetracemax = MAX(tracemax, linetracemax);
						linetracelength = MAX(tracelength, linetracelength);
					}

					/* check for new section plot */
					if (nwrite > 0 && (nwrite % nshotmax) == 0)
						nplot++;

					/* get seafloor depth min and max */
					if (segytraceheader.src_wbd > 0) {
						if (seafloordepthmin < 0.0) {
							seafloordepthmin = 0.01 * ((double)segytraceheader.src_wbd);
							seafloordepthmax = 0.01 * ((double)segytraceheader.src_wbd);
						}
						else {
							seafloordepthmin = MIN(seafloordepthmin, 0.01 * ((double)segytraceheader.src_wbd));
							seafloordepthmax = MAX(seafloordepthmax, 0.01 * ((double)segytraceheader.src_wbd));
						}
						if (seafloordepthminplot[nplot] < 0.0) {
							seafloordepthminplot[nplot] = 0.01 * ((double)segytraceheader.src_wbd);
							seafloordepthmaxplot[nplot] = 0.01 * ((double)segytraceheader.src_wbd);
						}
						else {
							seafloordepthminplot[nplot] =
							    MIN(seafloordepthminplot[nplot], 0.01 * ((double)segytraceheader.src_wbd));
							seafloordepthmaxplot[nplot] =
							    MAX(seafloordepthmaxplot[nplot], 0.01 * ((double)segytraceheader.src_wbd));
						}
					}

					/* output info */
					nread++;
					if (nread % 10 == 0 && verbose > 0)
						GMT_Report(API, GMT_MSG_NORMAL,
						        "file:%s record:%d shot:%d  %4.4d/%3.3d %2.2d:%2.2d:%2.2d.%3.3d samples:%d interval:%d usec  "
						        "minmax: %f %f\n",
						        file, nread, segytraceheader.shot_num, segytraceheader.year, segytraceheader.day_of_yr,
						        segytraceheader.hour, segytraceheader.min, segytraceheader.sec, segytraceheader.mils,
						        segytraceheader.nsamps, segytraceheader.si_micros, tracemin, tracemax);

					/* only write data if ok */
					if (oktowrite >= MBES_ONLINE_COUNT) {
						/* write characteristics file */
						GMT_Report(API, GMT_MSG_NORMAL, "%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d  %d %d %d   %f %f %f  %f %f %f %f\n",
						        time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6],
						        segytraceheader.shot_num, segytraceheader.nsamps, segytraceheader.si_micros, tracemin, tracemax,
						        tracerms, sensordepth, altitude, roll, pitch);

						/* write fileheader if needed */
						if (status == MB_SUCCESS && nwrite == 0) {
							segyfileheader.line = linenumber;
							segyfileheader.format = 5;
							if (sampleformat == MB_SEGY_SAMPLEFORMAT_ANALYTIC)
								segyfileheader.channels = 4;
							else
								segyfileheader.channels = 1;
							segyfileheader.aux_channels = 0;
							segyfileheader.sample_interval = segytraceheader.si_micros;
							segyfileheader.sample_interval_org = segytraceheader.si_micros;
							segyfileheader.number_samples = segytraceheader.nsamps;

							/* insert file header data into output buffer */
							int index = 0;
							mb_put_binary_int(false, segyfileheader.jobid, (void *)&(buffer[index]));
							index += 4;
							mb_put_binary_int(false, segyfileheader.line, (void *)&(buffer[index]));
							index += 4;
							mb_put_binary_int(false, segyfileheader.reel, (void *)&(buffer[index]));
							index += 4;
							mb_put_binary_short(false, segyfileheader.channels, (void *)&(buffer[index]));
							index += 2;
							mb_put_binary_short(false, segyfileheader.aux_channels, (void *)&(buffer[index]));
							index += 2;
							mb_put_binary_short(false, segyfileheader.sample_interval, (void *)&(buffer[index]));
							index += 2;
							mb_put_binary_short(false, segyfileheader.sample_interval_org, (void *)&(buffer[index]));
							index += 2;
							mb_put_binary_short(false, segyfileheader.number_samples, (void *)&(buffer[index]));
							index += 2;
							mb_put_binary_short(false, segyfileheader.number_samples_org, (void *)&(buffer[index]));
							index += 2;
							mb_put_binary_short(false, segyfileheader.format, (void *)&(buffer[index]));
							index += 2;
							mb_put_binary_short(false, segyfileheader.cdp_fold, (void *)&(buffer[index]));
							index += 2;
							mb_put_binary_short(false, segyfileheader.trace_sort, (void *)&(buffer[index]));
							index += 2;
							mb_put_binary_short(false, segyfileheader.vertical_sum, (void *)&(buffer[index]));
							index += 2;
							mb_put_binary_short(false, segyfileheader.sweep_start, (void *)&(buffer[index]));
							index += 2;
							mb_put_binary_short(false, segyfileheader.sweep_end, (void *)&(buffer[index]));
							index += 2;
							mb_put_binary_short(false, segyfileheader.sweep_length, (void *)&(buffer[index]));
							index += 2;
							mb_put_binary_short(false, segyfileheader.sweep_type, (void *)&(buffer[index]));
							index += 2;
							mb_put_binary_short(false, segyfileheader.sweep_trace, (void *)&(buffer[index]));
							index += 2;
							mb_put_binary_short(false, segyfileheader.sweep_taper_start, (void *)&(buffer[index]));
							index += 2;
							mb_put_binary_short(false, segyfileheader.sweep_taper_end, (void *)&(buffer[index]));
							index += 2;
							mb_put_binary_short(false, segyfileheader.sweep_taper, (void *)&(buffer[index]));
							index += 2;
							mb_put_binary_short(false, segyfileheader.correlated, (void *)&(buffer[index]));
							index += 2;
							mb_put_binary_short(false, segyfileheader.binary_gain, (void *)&(buffer[index]));
							index += 2;
							mb_put_binary_short(false, segyfileheader.amplitude, (void *)&(buffer[index]));
							index += 2;
							mb_put_binary_short(false, segyfileheader.units, (void *)&(buffer[index]));
							index += 2;
							mb_put_binary_short(false, segyfileheader.impulse_polarity, (void *)&(buffer[index]));
							index += 2;
							mb_put_binary_short(false, segyfileheader.vibrate_polarity, (void *)&(buffer[index]));
							index += 2;
							mb_put_binary_short(false, segyfileheader.domain, (void *)&(buffer[index]));
							index += 2;
							for (int i = 0; i < 338; i++) {
								buffer[index] = segyfileheader.extra[i];
								index++;
							}

							/* write either a single segy file or, in the case of analytic data,
							   write three files - correlate, conjugate, and envelope */
							if (sampleformat == MB_SEGY_SAMPLEFORMAT_ANALYTIC) {
								if (fwrite(&segyasciiheader, 1, MB_SEGY_ASCIIHEADER_LENGTH, fpc) != MB_SEGY_ASCIIHEADER_LENGTH) {
									status = MB_FAILURE;
									error = MB_ERROR_WRITE_FAIL;
								}
								else if (fwrite(buffer, 1, MB_SEGY_FILEHEADER_LENGTH, fpc) != MB_SEGY_FILEHEADER_LENGTH) {
									status = MB_FAILURE;
									error = MB_ERROR_WRITE_FAIL;
								}
								if (fwrite(&segyasciiheader, 1, MB_SEGY_ASCIIHEADER_LENGTH, fph) != MB_SEGY_ASCIIHEADER_LENGTH) {
									status = MB_FAILURE;
									error = MB_ERROR_WRITE_FAIL;
								}
								else if (fwrite(buffer, 1, MB_SEGY_FILEHEADER_LENGTH, fph) != MB_SEGY_FILEHEADER_LENGTH) {
									status = MB_FAILURE;
									error = MB_ERROR_WRITE_FAIL;
								}
							}
							segyfileheader.number_samples_org = segytraceheader.nsamps;
							if (fwrite(&segyasciiheader, 1, MB_SEGY_ASCIIHEADER_LENGTH, fpe) != MB_SEGY_ASCIIHEADER_LENGTH) {
								status = MB_FAILURE;
								error = MB_ERROR_WRITE_FAIL;
							}
							else if (fwrite(buffer, 1, MB_SEGY_FILEHEADER_LENGTH, fpe) != MB_SEGY_FILEHEADER_LENGTH) {
								status = MB_FAILURE;
								error = MB_ERROR_WRITE_FAIL;
							}
						}

						/* insert segy traceheader data into output buffer */
						int index = 0;
						mb_put_binary_int(false, segytraceheader.seq_num, (void *)&buffer[index]);
						index += 4;
						mb_put_binary_int(false, segytraceheader.seq_reel, (void *)&buffer[index]);
						index += 4;
						mb_put_binary_int(false, segytraceheader.shot_num, (void *)&buffer[index]);
						index += 4;
						mb_put_binary_int(false, segytraceheader.shot_tr, (void *)&buffer[index]);
						index += 4;
						mb_put_binary_int(false, segytraceheader.espn, (void *)&buffer[index]);
						index += 4;
						mb_put_binary_int(false, segytraceheader.rp_num, (void *)&buffer[index]);
						index += 4;
						mb_put_binary_int(false, segytraceheader.rp_tr, (void *)&buffer[index]);
						index += 4;
						mb_put_binary_short(false, segytraceheader.trc_id, (void *)&buffer[index]);
						index += 2;
						mb_put_binary_short(false, segytraceheader.num_vstk, (void *)&buffer[index]);
						index += 2;
						mb_put_binary_short(false, segytraceheader.cdp_fold, (void *)&buffer[index]);
						index += 2;
						mb_put_binary_short(false, segytraceheader.use, (void *)&buffer[index]);
						index += 2;
						mb_put_binary_int(false, segytraceheader.range, (void *)&buffer[index]);
						index += 4;
						mb_put_binary_int(false, segytraceheader.grp_elev, (void *)&buffer[index]);
						index += 4;
						mb_put_binary_int(false, segytraceheader.src_elev, (void *)&buffer[index]);
						index += 4;
						mb_put_binary_int(false, segytraceheader.src_depth, (void *)&buffer[index]);
						index += 4;
						mb_put_binary_int(false, segytraceheader.grp_datum, (void *)&buffer[index]);
						index += 4;
						mb_put_binary_int(false, segytraceheader.src_datum, (void *)&buffer[index]);
						index += 4;
						mb_put_binary_int(false, segytraceheader.src_wbd, (void *)&buffer[index]);
						index += 4;
						mb_put_binary_int(false, segytraceheader.grp_wbd, (void *)&buffer[index]);
						index += 4;
						mb_put_binary_short(false, segytraceheader.elev_scalar, (void *)&buffer[index]);
						index += 2;
						mb_put_binary_short(false, segytraceheader.coord_scalar, (void *)&buffer[index]);
						index += 2;
						mb_put_binary_int(false, segytraceheader.src_long, (void *)&buffer[index]);
						index += 4;
						mb_put_binary_int(false, segytraceheader.src_lat, (void *)&buffer[index]);
						index += 4;
						mb_put_binary_int(false, segytraceheader.grp_long, (void *)&buffer[index]);
						index += 4;
						mb_put_binary_int(false, segytraceheader.grp_lat, (void *)&buffer[index]);
						index += 4;
						mb_put_binary_short(false, segytraceheader.coord_units, (void *)&buffer[index]);
						index += 2;
						mb_put_binary_short(false, segytraceheader.wvel, (void *)&buffer[index]);
						index += 2;
						mb_put_binary_short(false, segytraceheader.sbvel, (void *)&buffer[index]);
						index += 2;
						mb_put_binary_short(false, segytraceheader.src_up_vel, (void *)&buffer[index]);
						index += 2;
						mb_put_binary_short(false, segytraceheader.grp_up_vel, (void *)&buffer[index]);
						index += 2;
						mb_put_binary_short(false, segytraceheader.src_static, (void *)&buffer[index]);
						index += 2;
						mb_put_binary_short(false, segytraceheader.grp_static, (void *)&buffer[index]);
						index += 2;
						mb_put_binary_short(false, segytraceheader.tot_static, (void *)&buffer[index]);
						index += 2;
						mb_put_binary_short(false, segytraceheader.laga, (void *)&buffer[index]);
						index += 2;
						mb_put_binary_int(false, segytraceheader.delay_mils, (void *)&buffer[index]);
						index += 4;
						mb_put_binary_short(false, segytraceheader.smute_mils, (void *)&buffer[index]);
						index += 2;
						mb_put_binary_short(false, segytraceheader.emute_mils, (void *)&buffer[index]);
						index += 2;
						mb_put_binary_short(false, segytraceheader.nsamps, (void *)&buffer[index]);
						index += 2;
						mb_put_binary_short(false, segytraceheader.si_micros, (void *)&buffer[index]);
						index += 2;
						for (int i = 0; i < 19; i++) {
							mb_put_binary_short(false, segytraceheader.other_1[i], (void *)&buffer[index]);
							index += 2;
						}
						mb_put_binary_short(false, segytraceheader.year, (void *)&buffer[index]);
						index += 2;
						mb_put_binary_short(false, segytraceheader.day_of_yr, (void *)&buffer[index]);
						index += 2;
						mb_put_binary_short(false, segytraceheader.hour, (void *)&buffer[index]);
						index += 2;
						mb_put_binary_short(false, segytraceheader.min, (void *)&buffer[index]);
						index += 2;
						mb_put_binary_short(false, segytraceheader.sec, (void *)&buffer[index]);
						index += 2;
						mb_put_binary_short(false, segytraceheader.mils, (void *)&buffer[index]);
						index += 2;
						mb_put_binary_short(false, segytraceheader.tr_weight, (void *)&buffer[index]);
						index += 2;
						for (int i = 0; i < 5; i++) {
							mb_put_binary_short(false, segytraceheader.other_2[i], (void *)&buffer[index]);
							index += 2;
						}
						mb_put_binary_float(false, segytraceheader.delay, (void *)&buffer[index]);
						index += 4;
						mb_put_binary_float(false, segytraceheader.smute_sec, (void *)&buffer[index]);
						index += 4;
						mb_put_binary_float(false, segytraceheader.emute_sec, (void *)&buffer[index]);
						index += 4;
						mb_put_binary_float(false, segytraceheader.si_secs, (void *)&buffer[index]);
						index += 4;
						mb_put_binary_float(false, segytraceheader.wbt_secs, (void *)&buffer[index]);
						index += 4;
						mb_put_binary_int(false, segytraceheader.end_of_rp, (void *)&buffer[index]);
						index += 4;
						mb_put_binary_float(false, segytraceheader.dummy1, (void *)&buffer[index]);
						index += 4;
						mb_put_binary_float(false, segytraceheader.dummy2, (void *)&buffer[index]);
						index += 4;
						mb_put_binary_float(false, segytraceheader.dummy3, (void *)&buffer[index]);
						index += 4;
						mb_put_binary_float(false, segytraceheader.sensordepthtime, (void *)&buffer[index]);
						index += 4;
						mb_put_binary_float(false, segytraceheader.soundspeed, (void *)&buffer[index]);
						index += 4;
						mb_put_binary_float(false, segytraceheader.distance, (void *)&buffer[index]);
						index += 4;
						mb_put_binary_float(false, segytraceheader.roll, (void *)&buffer[index]);
						index += 4;
						mb_put_binary_float(false, segytraceheader.pitch, (void *)&buffer[index]);
						index += 4;
						mb_put_binary_float(false, segytraceheader.heading, (void *)&buffer[index]);
						/* index += 4; */

						/* write out segy trace header to one or three files */
						if (sampleformat == MB_SEGY_SAMPLEFORMAT_ANALYTIC) {
							if (fwrite(buffer, 1, MB_SEGY_TRACEHEADER_LENGTH, fpc) != MB_SEGY_TRACEHEADER_LENGTH) {
								status = MB_FAILURE;
								error = MB_ERROR_WRITE_FAIL;
							}
							if (fwrite(buffer, 1, MB_SEGY_TRACEHEADER_LENGTH, fph) != MB_SEGY_TRACEHEADER_LENGTH) {
								status = MB_FAILURE;
								error = MB_ERROR_WRITE_FAIL;
							}
						}
						if (fwrite(buffer, 1, MB_SEGY_TRACEHEADER_LENGTH, fpe) != MB_SEGY_TRACEHEADER_LENGTH) {
							status = MB_FAILURE;
							error = MB_ERROR_WRITE_FAIL;
						}

						/* write trace data */
						if (sampleformat == MB_SEGY_SAMPLEFORMAT_ANALYTIC) {

							/* correlate time series */
							index = 0;
							for (int i = 0; i < segytraceheader.nsamps; i++) {

								mb_put_binary_float(false, segydata[2 * i], (void *)&buffer[index]);
								index += 4;
							}
							if (status == MB_SUCCESS &&
							    fwrite(buffer, 1, segytraceheader.nsamps * sizeof(float), fpc) != segytraceheader.nsamps * sizeof(float)) {
								status = MB_FAILURE;
								error = MB_ERROR_WRITE_FAIL;
							}

							/* conjugate time series */
							index = 0;
							for (int i = 0; i < segytraceheader.nsamps; i++) {

								mb_put_binary_float(false, segydata[2 * i + 1], (void *)&buffer[index]);
								index += 4;
							}
							if (status == MB_SUCCESS &&
							    fwrite(buffer, 1, segytraceheader.nsamps * sizeof(float), fph) != segytraceheader.nsamps * sizeof(float)) {
								status = MB_FAILURE;
								error = MB_ERROR_WRITE_FAIL;
							}

							/* envelope time series */
							index = 0;
							for (int i = 0; i < segytraceheader.nsamps; i++) {
								float magnitude = (float)sqrt(segydata[2 * i] * segydata[2 * i] + segydata[2 * i + 1] * segydata[2 * i + 1]);
								mb_put_binary_float(false, magnitude, (void *)&buffer[index]);
								index += 4;
							}
							if (status == MB_SUCCESS &&
							    fwrite(buffer, 1, segytraceheader.nsamps * sizeof(float), fpe) != segytraceheader.nsamps * sizeof(float)) {
								status = MB_FAILURE;
								error = MB_ERROR_WRITE_FAIL;
							}
						}
						else if (sampleformat == MB_SEGY_SAMPLEFORMAT_ENVELOPE) {
							/* envelope time series */
							index = 0;
							for (int i = 0; i < segytraceheader.nsamps; i++) {
								mb_put_binary_float(false, segydata[i], (void *)&buffer[index]);
								index += 4;
							}
							if (status == MB_SUCCESS &&
							    fwrite(buffer, 1, segytraceheader.nsamps * sizeof(float), fpe) != segytraceheader.nsamps * sizeof(float)) {
								status = MB_FAILURE;
								error = MB_ERROR_WRITE_FAIL;
							}
						}

						else /* if (sampleformat == MB_SEGY_SAMPLEFORMAT_TRACE) */ {
							/* envelope time series */
							index = 0;
							for (int i = 0; i < segytraceheader.nsamps; i++) {
								mb_put_binary_float(false, segydata[i], (void *)&buffer[index]);
								index += 4;
							}
							if (status == MB_SUCCESS &&
							    fwrite(buffer, 1, segytraceheader.nsamps * sizeof(float), fpe) != segytraceheader.nsamps * sizeof(float)) {
								status = MB_FAILURE;
								error = MB_ERROR_WRITE_FAIL;
							}
						}
						nwrite++;
					}
				}
			}

			if (verbose >= 2) {
				GMT_Report(API, GMT_MSG_NORMAL, "\ndbg2  Ping read in program <%s>\n", program_name);
				GMT_Report(API, GMT_MSG_NORMAL, "dbg2       kind:           %d\n", kind);
				GMT_Report(API, GMT_MSG_NORMAL, "dbg2       error:          %d\n", error);
				GMT_Report(API, GMT_MSG_NORMAL, "dbg2       status:         %d\n", status);
			}

			/* print comments */
			if (verbose >= 1 && kind == MB_DATA_COMMENT) {
				if (icomment == 0) {
					GMT_Report(API, GMT_MSG_NORMAL, "\nComments:\n");
					icomment++;
				}
				GMT_Report(API, GMT_MSG_NORMAL, "%s\n", comment);
			}
		}

		/* close the swath file */
		status = mb_close(verbose, &mbio_ptr, &error);

		/* output read statistics */
		GMT_Report(API, GMT_MSG_NORMAL, "%d records read from %s\n", nread, file);

		/* deallocate memory used for segy data arrays */
		mb_freed(verbose, __FILE__, __LINE__, (void **)&segydata, &error);
		segydata_alloc = 0;
		mb_freed(verbose, __FILE__, __LINE__, (void **)&buffer, &error);
		buffer_alloc = 0;

		/* figure out whether and what to read next */
		if (read_datalist) {
			read_data = mb_datalist_read(verbose, datalist, file, dfile, &format, &file_weight, &error) == MB_SUCCESS;
		} else {
			read_data = false;
		}

		/* close output file if conditions warrant */
		if (!read_data || (!output_root_set && nroutepoint < 2)) {
			/* close current output file */
			if (output_open) {
				if (fpc != NULL) {
					fclose(fpc);
					fpc = NULL;
				}
				if (fph != NULL) {
					fclose(fph);
					fph = NULL;
				}
				if (fpe != NULL) {
					fclose(fpe);
					fpe = NULL;
				}
				output_open = false;

				/* output count of segy records */
				GMT_Report(API, GMT_MSG_NORMAL, "\n%d records output to segy files with root %s\n", nwrite, output_root);
				if (verbose > 0)
					GMT_Report(API, GMT_MSG_NORMAL, "\n");

				/* generate code to generate plots */
				status = mbes_generateplots(API, verbose, sfp, output_root, segy_suffix, sampleformat,
				                startlon, endlon, startlat, endlat,
				                xscale, yscale, nwrite, nshotmax,
				                seafloordepthmin, seafloordepthmax,
				                seafloordepthminplot, seafloordepthmaxplot,
				                linetracelength, zmax, startshot, endshot,
				                lineroot, linenumber,
				                &error);

				/* increment line number */
				linenumber++;
			}
		}

		/* end loop over files in list */
	}
	if (read_datalist)
		mb_datalist_close(verbose, &datalist, &error);

	/* close plotting script file */
	fclose(sfp);
	snprintf(command, sizeof(command), "chmod +x %s", scriptfile);
	/* const int shellstatus = */ system(command);
	/* TODO(schwehr): Check the shellstatus */

	/* deallocate route arrays */
	if (nroutepointalloc > 0) {
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&routelon, &error);
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&routelat, &error);
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&routeheading, &error);
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&routewaypoint, &error);
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&routetime_d, &error);
	}

	/* check memory */
	if ((status = mb_memory_list(verbose, &error)) == MB_FAILURE) {
		GMT_Report(API, GMT_MSG_NORMAL, "Program %s completed but failed to deallocate all allocated memory - the code has a memory leak somewhere!\n", program_name);
	}

	Return(error);
}
/*--------------------------------------------------------------------*/
