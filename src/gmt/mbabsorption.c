/*--------------------------------------------------------------------
 *    The MB-system:	mbabsorption.c	2/10/2008
 *
 *    Copyright (c) 2008-2025 by
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
 * MBabsorption calculates the absorption of sound in sea water in dB/km
 * as a function of frequency, temperature, salinity, sound speed, and depth.
 *
 * We use the Francois and Garrison equations from:
 *     Francois, R.E., Garrison, G.R., "Sound absorption based
 *       on ocean measurements: Part I: Pure water and magnesium
 *       sulfate contributions", J. Acoust. Soc. Am., 72(3),
 *       896-907, 1982.
 *     Francois, R.E., Garrison, G.R., "Sound absorption based
 *       on ocean measurements: Part II: Boric acid contribution
 *       and equation for total absorption", J. Acoust. Soc. Am.,
 *       72(6), 1879-1890, 1982.
 *
 * Francois and Garrison [1982] model the sound absorption in
 * sea water as resulting from contributions from pure water,
 * magnesium sulfate, and boric acid. The boric acid contribution
 * is significant below 10 kHz. The equations are:
 *
 * absorption = Boric Acid Contribution
 * 		+ MbSO4 Contribution
 * 		+ Pure Water Contribution
 *
 * **************************
 *
 * Boric Acid Contribution
 * AlphaB = Ab * Pb * Fb * f**2
 *          -------------------
 *             f**2 + Fb**2
 *
 * Ab = 8.86 / c * 10**(0.78 * pH - 5) (dB/km/kHz)
 * Pb = 1
 * Fb = 2.8 * (S / 35)**0.5 * 10**(4 - 1245 / Tk) (kHz)
 *
 * **************************
 *
 * MgSO4 Contribution
 * AlphaM = Am * Pm * Fm * f**2
 *          -------------------
 *             f**2 + Fm**2
 *
 * Am = 21.44 * S * (1 + 0.025 * T) / c (dB/km/kHZ)
 * Pm = 1 - 0.000137 * D + 0.0000000062 * D**2
 * Fm = (8.17 * 10**(8 - 1990 / Tk)) / (1 + 0.0018 * (S - 35))  (kHz)
 *
 * **************************
 *
 * Pure Water Contribution
 * AlphaW = Aw * Pw * f**2
 *
 * For T <= 20 deg C
 *   Aw = 0.0004397 - 0.0000259 * T
 *           + 0.000000911 * T**2 - 0.000000015 * T**3 (dB/km/kHz)
 * For T > 20 deg C
 *   Aw = 0.0003964 - 0.00001146 * T
 *           + 0.000000145 * T**2 - 0.00000000049 * T**3 (dB/km/kHz)
 * Pw = 1 - 0.0000383 * D + 0.00000000049 * D**2
 *
 * **************************
 *
 * f = sound frequency (kHz)
 * c = speed of sound (m/s)
 *   =~ 1412 + 3.21 * T + 1.19 * S + 0.0167 * D
 * T = temperature (deg C)
 * Tk = temperature (deg K) = T + 273 (deg K)
 * S = salinity (per mil)
 * D = depth (m)
 *
 * **************************
 *
 * Author:	D. W. Caress
 * Date:	February 10, 2008
 *              R/V Zephyr
 *              Hanging out at the channel entrance to La Paz, BCS, MX
 *              helping out as MBARI tries to save the grounded
 *              R/V Western Flyer.
 *              Note: as I was writing this code the Flyer was refloated
 *              and successfully backed off the reef.
 *
 * GMT-module rewrite of mbabsorption.cc: wrapped as GMT_mbabsorption entry
 * so it can be invoked from the GMT API (Julia FFI / Matlab MEX).
 * Single-letter options (-C, -D, -F, -P, -S, -T, -V, -H) map directly to
 * GMT_OPTION entries — no getopt_long lookup table required.
 */

#define THIS_MODULE_NAME		"mbabsorption"
#define THIS_MODULE_LIB			"mbsystem"
#define THIS_MODULE_PURPOSE		"Compute absorption of sound in sea water (dB/km) from frequency, T, S, c, pH, depth"
#define THIS_MODULE_KEYS		">D}"
#define THIS_MODULE_NEEDS		""
#define THIS_MODULE_OPTIONS		"-:>Vh"

#include "gmt_dev.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_define.h"
#include "mb_status.h"

static const char help_message[] =
    "MBabsorption calculates the absorption of sound in sea water\n"
    "in dB/km as a function of frequency, temperature, salinity,\n"
    "sound speed, pH, and depth.";
static const char usage_message[] =
    "mbabsorption [-Csoundspeed -Ddepth -Ffrequency -Pph -Ssalinity -Ttemperature -V -H]";

/* --- Control structure ---------------------------------------------- */

struct MBABSORPTION_CTRL {
	struct mba_C { bool active; double soundspeed; } C;
	struct mba_D { bool active; double depth;      } D;
	struct mba_F { bool active; double frequency;  } F;
	struct mba_H { bool active; } H;
	struct mba_P { bool active; double ph;         } P;
	struct mba_S { bool active; double salinity;   } S;
	struct mba_T { bool active; double temperature;} T;
};

static void *New_mbabsorption_Ctrl(struct GMT_CTRL *GMT) {
	struct MBABSORPTION_CTRL *Ctrl = gmt_M_memory(GMT, NULL, 1, struct MBABSORPTION_CTRL);
	/* defaults match the original mbabsorption.cc */
	Ctrl->F.frequency   = 200.0;
	Ctrl->T.temperature = 10.0;
	Ctrl->S.salinity    = 35.0;
	Ctrl->C.soundspeed  = 0.0;
	Ctrl->D.depth       = 0.0;
	Ctrl->P.ph          = 8.0;
	return Ctrl;
}

static void Free_mbabsorption_Ctrl(struct GMT_CTRL *GMT, struct MBABSORPTION_CTRL *Ctrl) {
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
		"\t-C Speed of sound (m/sec). Default: derived from T, S, D.\n"
		"\t-D Depth (m). Default: 0.\n"
		"\t-F Sound frequency (kHz). Default: 200.\n"
		"\t-P pH. Default: 8.\n"
		"\t-S Salinity (per mil). Default: 35.\n"
		"\t-T Temperature (deg C). Default: 10.\n"
		"\t-H Print description and exit.\n");
	GMT_Option(API, "V,:");
	return GMT_PARSE_ERROR;
}

static int parse(struct GMT_CTRL *GMT, struct MBABSORPTION_CTRL *Ctrl, struct GMT_OPTION *options) {
	unsigned int n_errors = 0;
	struct GMT_OPTION *opt;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {
		case 'C':
			if (opt->arg && opt->arg[0] && sscanf(opt->arg, "%lf", &Ctrl->C.soundspeed) == 1)
				Ctrl->C.active = true;
			else {
				GMT_Report(API, GMT_MSG_NORMAL, "Syntax -C option: expected soundspeed (m/sec)\n");
				n_errors++;
			}
			break;
		case 'D':
			if (opt->arg && opt->arg[0] && sscanf(opt->arg, "%lf", &Ctrl->D.depth) == 1)
				Ctrl->D.active = true;
			else {
				GMT_Report(API, GMT_MSG_NORMAL, "Syntax -D option: expected depth (m)\n");
				n_errors++;
			}
			break;
		case 'F':
			if (opt->arg && opt->arg[0] && sscanf(opt->arg, "%lf", &Ctrl->F.frequency) == 1)
				Ctrl->F.active = true;
			else {
				GMT_Report(API, GMT_MSG_NORMAL, "Syntax -F option: expected frequency (kHz)\n");
				n_errors++;
			}
			break;
		case 'H':
			Ctrl->H.active = true;
			break;
		case 'P':
			if (opt->arg && opt->arg[0] && sscanf(opt->arg, "%lf", &Ctrl->P.ph) == 1)
				Ctrl->P.active = true;
			else {
				GMT_Report(API, GMT_MSG_NORMAL, "Syntax -P option: expected pH\n");
				n_errors++;
			}
			break;
		case 'S':
			if (opt->arg && opt->arg[0] && sscanf(opt->arg, "%lf", &Ctrl->S.salinity) == 1)
				Ctrl->S.active = true;
			else {
				GMT_Report(API, GMT_MSG_NORMAL, "Syntax -S option: expected salinity (per mil)\n");
				n_errors++;
			}
			break;
		case 'T':
			if (opt->arg && opt->arg[0] && sscanf(opt->arg, "%lf", &Ctrl->T.temperature) == 1)
				Ctrl->T.active = true;
			else {
				GMT_Report(API, GMT_MSG_NORMAL, "Syntax -T option: expected temperature (deg C)\n");
				n_errors++;
			}
			break;
		default:
			n_errors += gmt_default_error(GMT, opt->option);
			break;
		}
	}

	return n_errors ? GMT_PARSE_ERROR : GMT_OK;
}

#define bailout(code)  { gmt_M_free_options(mode); return code; }
#define Return(code)   { Free_mbabsorption_Ctrl(GMT, Ctrl); gmt_end_module(GMT, GMT_cpy); bailout(code); }

EXTERN_MSC int GMT_mbabsorption(void *API, int mode, void *args);

/*--------------------------------------------------------------------*/
int GMT_mbabsorption(void *V_API, int mode, void *args) {
	int error = MB_ERROR_NO_ERROR;

	struct MBABSORPTION_CTRL *Ctrl = NULL;
	struct GMT_CTRL      *GMT  = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION    *options = NULL;
	struct GMTAPI_CTRL   *API = gmt_get_api_ptr(V_API);

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

	Ctrl = New_mbabsorption_Ctrl(GMT);
	if ((error = parse(GMT, Ctrl, options))) Return(error);

	int verbose = GMT->common.V.active;
	if (GMT->current.setting.verbose >= GMT_MSG_DEBUG) verbose = 2;

	const bool   help        = Ctrl->H.active;
	const double frequency   = Ctrl->F.frequency;
	const double temperature = Ctrl->T.temperature;
	const double salinity    = Ctrl->S.salinity;
	double       soundspeed  = Ctrl->C.soundspeed;
	const double depth       = Ctrl->D.depth;
	const double ph          = Ctrl->P.ph;

	if (verbose == 1 || help) {
		GMT_Report(API, GMT_MSG_NORMAL, "\nProgram %s\n", THIS_MODULE_NAME);
		GMT_Report(API, GMT_MSG_NORMAL, "MB-system Version %s\n", MB_VERSION);
	}

	if (verbose >= 2) {
		GMT_Report(API, GMT_MSG_NORMAL, "\ndbg2  Program <%s>\n", THIS_MODULE_NAME);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2  MB-system Version %s\n", MB_VERSION);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2  Control Parameters:\n");
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       verbose:    %d\n", verbose);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       help:       %d\n", (int)help);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       frequency:  %f\n", frequency);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       temperature:%f\n", temperature);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       salinity:   %f\n", salinity);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       soundspeed: %f\n", soundspeed);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       depth:      %f\n", depth);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       ph:         %f\n", ph);
	}

	/* if help desired then print it and exit */
	if (help) {
		GMT_Report(API, GMT_MSG_NORMAL, "\n%s\n", help_message);
		GMT_Report(API, GMT_MSG_NORMAL, "\nusage: %s\n", usage_message);
		Return(MB_ERROR_NO_ERROR);
	}

	/* call function to calculate absorption */
	double absorption;  /* absorption (dB/km) */
	double density;     /* kg/m3 */

	int status = mb_absorption(verbose, frequency, temperature, salinity, depth, ph, soundspeed, &absorption, &error);
	const double pressure = 1.006 * depth;  /* depth (m) */
	status &= mb_seabird_density(verbose, salinity, temperature, pressure, &density, &error);

	if (verbose > 0) {
		GMT_Report(API, GMT_MSG_NORMAL, "\nProgram <%s>\n", THIS_MODULE_NAME);
		GMT_Report(API, GMT_MSG_NORMAL, "MB-system Version %s\n", MB_VERSION);
		GMT_Report(API, GMT_MSG_NORMAL, "Input Parameters:\n");
		GMT_Report(API, GMT_MSG_NORMAL, "     Frequency:        %f kHz\n", frequency);
		GMT_Report(API, GMT_MSG_NORMAL, "     Temperature:      %f deg C\n", temperature);
		GMT_Report(API, GMT_MSG_NORMAL, "     Salinity:         %f per mil\n", salinity);
		if (soundspeed > 0.0)
			GMT_Report(API, GMT_MSG_NORMAL, "     Soundspeed:       %f m/sec\n", soundspeed);
		GMT_Report(API, GMT_MSG_NORMAL, "     Depth:            %f m\n", depth);
		GMT_Report(API, GMT_MSG_NORMAL, "     pH:               %f\n", ph);
		GMT_Report(API, GMT_MSG_NORMAL, "Result:\n");
		GMT_Report(API, GMT_MSG_NORMAL, "     Sound absorption: %f dB/km\n", absorption);
		GMT_Report(API, GMT_MSG_NORMAL, "     Density:          %f kg/m3\n", density);
	}
	else {
		GMT_Report(API, GMT_MSG_NORMAL, "%f\n", absorption);
	}

	if (verbose >= 2) {
		GMT_Report(API, GMT_MSG_NORMAL, "\ndbg2  Program <%s> completed\n", THIS_MODULE_NAME);
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2  Ending status:\n");
		GMT_Report(API, GMT_MSG_NORMAL, "dbg2       status:  %d\n", status);
	}

	Return(error);
}
/*--------------------------------------------------------------------*/
