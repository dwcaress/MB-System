/*--------------------------------------------------------------------
 *    The MB-system:	mbabsorption.c	2/10/2008
 *
 *    Copyright (c) 2008-2023 by
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
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <getopt.h>
#include <unistd.h>

#include "mb_define.h"
#include "mb_status.h"

constexpr char program_name[] = "MBabsorption";
constexpr char help_message[] =
    "MBabsorption calculates the absorption of sound in sea water\n"
    "in dB/km as a function of frequency, temperature, salinity,\n"
    "sound speed, pH, and depth.";
constexpr char usage_message[] =
    "mbabsorption [-Csoundspeed -Ddepth -Ffrequency -Pph -Ssalinity -Ttemperature -V -H]";

/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
	int verbose = 0;

	double frequency = 200.0;   /* frequency (kHz) */
	double temperature = 10.0; /* temperature (deg C) */
	double salinity = 35.0;    /* salinity (per mil) */
	double soundspeed = 0.0;  /* speed of sound (m/sec) */
	double depth = 0.0;       /* depth (m) */
	double ph = 8.0;          /* pH */

	/* process argument list */
	bool help = false;
	{
		bool errflg = false;
		int c;
		while ((c = getopt(argc, argv, "VvHhC:c:D:d:F:f:P:p:S:s:T:t:")) != -1)
			switch (c) {
			case 'H':
			case 'h':
				help = true;
				break;
			case 'V':
			case 'v':
				verbose++;
				break;
			case 'C':
			case 'c':
				sscanf(optarg, "%lf", &soundspeed);
				break;
			case 'D':
			case 'd':
				sscanf(optarg, "%lf", &depth);
				break;
			case 'F':
			case 'f':
				sscanf(optarg, "%lf", &frequency);
				break;
			case 'P':
			case 'p':
				sscanf(optarg, "%lf", &ph);
				break;
			case 'S':
			case 's':
				sscanf(optarg, "%lf", &salinity);
				break;
			case 'T':
			case 't':
				sscanf(optarg, "%lf", &temperature);
				break;
			case '?':
				errflg = true;
			}

		if (errflg) {
			fprintf(stderr, "usage: %s\n", usage_message);
			exit(MB_ERROR_BAD_USAGE);
		}
	}

	/* set output stream */
	FILE *outfp;
	if (verbose <= 1)
		outfp = stdout;
	else
		outfp = stderr;

	if (verbose == 1 || help) {
		fprintf(outfp, "\nProgram %s\n", program_name);
		fprintf(outfp, "MB-system Version %s\n", MB_VERSION);
	}

	if (verbose >= 2) {
		fprintf(outfp, "\ndbg2  Program <%s>\n", program_name);
		fprintf(outfp, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(outfp, "dbg2  Control Parameters:\n");
		fprintf(outfp, "dbg2       verbose:    %d\n", verbose);
		fprintf(outfp, "dbg2       help:       %d\n", help);
		fprintf(outfp, "dbg2       frequency:  %f\n", frequency);
		fprintf(outfp, "dbg2       temperature:%f\n", temperature);
		fprintf(outfp, "dbg2       salinity:   %f\n", salinity);
		fprintf(outfp, "dbg2       soundspeed: %f\n", soundspeed);
		fprintf(outfp, "dbg2       depth:      %f\n", depth);
		fprintf(outfp, "dbg2       ph:         %f\n", ph);
	}

	/* if help desired then print it and exit */
	if (help) {
		fprintf(outfp, "\n%s\n", help_message);
		fprintf(outfp, "\nusage: %s\n", usage_message);
		exit(MB_ERROR_NO_ERROR);
	}

	/* call function to calculate absorption */
	double absorption;  /* absorption (dB/km) */
	double density;     /* kg/m3 */

	int error = MB_ERROR_NO_ERROR;
	int status = mb_absorption(verbose, frequency, temperature, salinity, depth, ph, soundspeed, &absorption, &error);
	const double pressure = 1.006 * depth;  /* depth (m) */
	status &= mb_seabird_density(verbose, salinity, temperature, pressure, &density, &error);

	if (verbose > 0) {
		fprintf(outfp, "\nProgram <%s>\n", program_name);
		fprintf(outfp, "MB-system Version %s\n", MB_VERSION);
		fprintf(outfp, "Input Parameters:\n");
		fprintf(outfp, "     Frequency:        %f kHz\n", frequency);
		fprintf(outfp, "     Temperature:      %f deg C\n", temperature);
		fprintf(outfp, "     Salinity:         %f per mil\n", salinity);
		if (soundspeed > 0.0)
			fprintf(outfp, "     Soundspeed:       %f m/sec\n", soundspeed);
		fprintf(outfp, "     Depth:            %f m\n", depth);
		fprintf(outfp, "     pH:               %f\n", ph);
		fprintf(outfp, "Result:\n");
		fprintf(outfp, "     Sound absorption: %f dB/km\n", absorption);
		fprintf(outfp, "     Density:          %f kg/m3\n", density);
	}
	else {
		fprintf(outfp, "%f\n", absorption);
	}

	if (verbose >= 2) {
		fprintf(outfp, "\ndbg2  Program <%s> completed\n", program_name);
		fprintf(outfp, "dbg2  Ending status:\n");
		fprintf(outfp, "dbg2       status:  %d\n", status);
	}

	exit(error);
}
/*--------------------------------------------------------------------*/
