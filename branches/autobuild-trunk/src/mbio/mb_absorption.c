/* Added HAVE_CONFIG_H for autogen files */
#ifdef HAVE_CONFIG_H
#  include <mbsystem_config.h>
#endif


/*--------------------------------------------------------------------
 *    The MB-system:	mb_absorption.c		2/10/2008
 *    $Id: mb_absorption.c 1891 2011-05-04 23:46:30Z caress $
 *
 *    Copyright (c) 2008-2011 by
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
 * mb_absorption.c includes a "mb_" function used to calculate 
 * the absorption of sound in sea water in dB/km as a 
 * function of frequency, temperature, salinity, and pressure.
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
 * $Log: mb_absorption.c,v $
 * Revision 5.0  2008/03/01 09:19:29  caress
 * Added function to calculate sound absoprtion in water from water properties parameters.
 *
 *
 */


/* standard include files */
#include <stdio.h>
#include <math.h>

/* mbio include files */
#include "mb_status.h"
#include "mb_define.h"

/*--------------------------------------------------------------------*/
int mb_absorption(int verbose,
		double frequency, double temperature,double salinity, 
		double depth, double ph, double soundspeed, 
		double *absorption, int *error)
{
	char	*function_name = "mb_absorption";
	int	status = MB_SUCCESS;
	double	Alphab, Alpham, Alphaw;
	double	tk;
	double	Ab, Pb, Fb;
	double	Am, Pm, Fm;
	double	Aw, Pw;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       frequency:  %f\n",frequency);
		fprintf(stderr,"dbg2       temperature:%f\n",temperature);
		fprintf(stderr,"dbg2       salinity:   %f\n",salinity);
		fprintf(stderr,"dbg2       soundspeed: %f\n",soundspeed);
		fprintf(stderr,"dbg2       depth:      %f\n",depth);
		fprintf(stderr,"dbg2       ph:         %f\n",ph);
		}
		
	/* calculate sound speed if needed */
	if (soundspeed <= 0.0)
		soundspeed = 1412.0 + 3.21 * temperature 
				+ 1.19 * salinity + 0.0167 * depth;
				
	/* get temperature in deg Kelvin
		- I know the conversion is slightly wrong, but this
			is what they published.... */
	tk = temperature + 273.0;

	/* calculate boric acid contribution */
	Ab = 8.86 / soundspeed * pow(10.0, (0.78 * ph - 5.0));
	Pb = 1.0;
	Fb = 2.8 * sqrt(salinity / 35.0) * pow(10.0, (4.0 - (1245.0 / tk)));
	Alphab = (Ab * Pb * Fb * frequency * frequency)
			/ (Fb * Fb + frequency * frequency);

	/* calculate MgSO4 contribution */
	Am = 21.44 * salinity * (1 + 0.025 * temperature) / soundspeed;
	Pm = 1 - 0.000137 * depth + 0.0000000062 * depth * depth;
	Fm = (8.17 * pow(10.0, (8.0 - 1990.0 / tk))) / (1 + 0.0018 * (salinity - 35));
	Alpham = (Am * Pm * Fm * frequency * frequency)
			/ (Fm * Fm + frequency * frequency);

	/* calculate pure water contribution */
	if (temperature <= 20.0)
		Aw = 0.0004397 - 0.0000259 * temperature
			+ 0.000000911 * temperature * temperature 
			- 0.000000015 * temperature * temperature * temperature;
	else
		Aw = 0.0003964 - 0.00001146 * temperature
			+ 0.000000145 * temperature * temperature 
			- 0.00000000049 * temperature * temperature * temperature;

	Pw = 1 - 0.0000383 * depth + 0.00000000049 *  depth * depth;
	Alphaw = Aw * Pw * frequency * frequency;
	
	/* add it all together */
	*absorption = Alphab + Alpham + Alphaw;

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       absorption:      %f\n",*absorption);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
