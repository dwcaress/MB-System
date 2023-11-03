/*--------------------------------------------------------------------
 *    The MB-system:	mb_absorption.c		2/10/2008
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
 * This file mb_absorption.c includes mbio functions used to calculate
 * water properties from observations and models
 *
 * Original Functions:
 *   int mb_absorption(int verbose, double frequency, double temperature,
 *   					double salinity, double depth,
 *   					double ph, double soundspeed,
 *                		double *absorption, int *error)
 *   int mb_potential_temperature(int verbose, double temperature,
 *   					double salinity, double pressure,
 *   					double *potential_temperature, int *error)
 *
 * Author:	D. W. Caress
 * Date:	February 10, 2008
 *
 * SeaBird Functions:
 *    int mb_seabird_density(int verbose, double salinity, double temperature, double pressure, double *density, int *error);
 *    int mb_seabird_depth(int verbose, double pressure, double latitude, double *depth, int *error);
 *    int mb_seabird_salinity(int verbose, double conductivity, double temperature, double pressure, double *salinity, int *error);
 *    int mb_seabird_soundspeed(int verbose, int algorithm, double salinity, double temperature, double pressure, double *soundspeed, int *error);
 *
 * Author:	D. W. Caress
 * Date:	June 30, 2017
 *--------------------------------------------------------------------
 *
 */

#include <math.h>
#include <stdio.h>

#include "mb_define.h"
#include "mb_status.h"

/*--------------------------------------------------------------------*/
/*
 * mb_absorption() is a function used to calculate
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
 *   Aw = 0.0004937 - 0.0000259 * T
 *           + 0.000000911 * T**2 - 0.000000015 * T**3 (dB/km/kHz)
 * For T > 20 deg C
 *   Aw = 0.0003964 - 0.00001146 * T
 *           + 0.000000145 * T**2 - 0.00000000065 * T**3 (dB/km/kHz)
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
 */
int mb_absorption(int verbose, double frequency, double temperature, double salinity, double depth, double ph, double soundspeed,
                  double *absorption, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       frequency:  %f\n", frequency);
		fprintf(stderr, "dbg2       temperature:%f\n", temperature);
		fprintf(stderr, "dbg2       salinity:   %f\n", salinity);
		fprintf(stderr, "dbg2       soundspeed: %f\n", soundspeed);
		fprintf(stderr, "dbg2       depth:      %f\n", depth);
		fprintf(stderr, "dbg2       ph:         %f\n", ph);
	}

	/* calculate sound speed if needed */
	if (soundspeed <= 0.0)
		soundspeed = 1412.0 + 3.21 * temperature + 1.19 * salinity + 0.0167 * depth;

	/* get temperature in deg Kelvin
	    - I know the conversion is slightly wrong, but this
	        is what they published.... */
	const double tk = temperature + 273.0;

	/* calculate boric acid contribution */
	const double Ab = 8.86 / soundspeed * pow(10.0, (0.78 * ph - 5.0));
	const double Pb = 1.0;
	const double Fb = 2.8 * sqrt(salinity / 35.0) * pow(10.0, (4.0 - (1245.0 / tk)));
	const double Alphab = (Ab * Pb * Fb * frequency * frequency) / (Fb * Fb + frequency * frequency);

	/* calculate MgSO4 contribution */
	const double Am = 21.44 * salinity * (1 + 0.025 * temperature) / soundspeed;
	const double Pm = 1 - 0.000137 * depth + 0.0000000062 * depth * depth;
	const double Fm = (8.17 * pow(10.0, (8.0 - 1990.0 / tk))) / (1 + 0.0018 * (salinity - 35));
	const double Alpham = (Am * Pm * Fm * frequency * frequency) / (Fm * Fm + frequency * frequency);

	/* calculate pure water contribution */
	const double Aw =
		(temperature <= 20.0) ?
			0.0004937 - 0.0000259 * temperature + 0.000000911 * temperature * temperature -
				0.000000015 * temperature * temperature * temperature :
			0.0003964 - 0.00001146 * temperature + 0.000000145 * temperature * temperature -
			     0.00000000065 * temperature * temperature * temperature;

	const double Pw = 1 - 0.0000383 * depth + 0.00000000049 * depth * depth;
	const double Alphaw = Aw * Pw * frequency * frequency;

	/* add it all together */
	*absorption = Alphab + Alpham + Alphaw;

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       absorption:      %f\n", *absorption);
		fprintf(stderr, "dbg2       error:           %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
/*
 * mb_absorption.c includes a "mb_" function used to calculate
 * the potential temperature (deg C) of sea water as a function of in situ
 * temperature (deg C), salinity (PSU), and pressure (dbar). The algorithm
 * derives from:
 *     David R. Jackett, Trevor J. McDougall, Rainer Feistel, Daniel G. Wright,
 *     and Stephen M. Griffies, 2006: Algorithms for density, potential
 *     temperature, conservative temperature, and the freezing temperature of
 *     seawater. J. Atmos. Oceanic Technol., 23, 1709–1728.
 *     doi: http://dx.doi.org/10.1175/JTECH1946.1
 */
int mb_potential_temperature(int verbose, double temperature, double salinity, double pressure, double *potential_temperature,
                             int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       temperature:%f deg C\n", temperature);
		fprintf(stderr, "dbg2       salinity:   %f PSU\n", salinity);
		fprintf(stderr, "dbg2       pressure:   %f dbar\n", pressure);
	}

	/* Polynomial coefficients */
	const double a1 = 8.65483913395442e-6;
	const double a2 = -1.41636299744881e-6;
	const double a3 = -7.38286467135737e-9;
	const double a4 = -8.38241357039698e-6;
	const double a5 = 2.83933368585534e-8;
	const double a6 = 1.77803965218656e-8;
	const double a7 = 1.71155619208233e-10;

	/* Calculate potential temperature */
	*potential_temperature =
	    temperature + pressure * (a1 + (a2 * salinity) + (a3 * pressure) + (a4 * temperature) + (a5 * salinity * temperature) +
	                              (a6 * temperature * temperature) + (a7 * temperature * pressure));

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       potential_temperature: %f deg C\n", *potential_temperature);
		fprintf(stderr, "dbg2       error:                 %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
/*
 * The following functions derive from source code included in the
 * Sea-Bird Electronics software manual for
 *   Seasoft V2: SBE Data Processing (Release Date: 11/16/2016)
 */
/*--------------------------------------------------------------------*/
/*
 * Density calculation from page 151 of Seabird manual
 *
 * Parameter units:
 *   salinity: salinity in PSU
 *   temperature: temperature deg C ITPS-68
 *   pressure: pressure in decibars
 *   density: density in kg/m^3
 *
 * Use the following constants:
 *   B0 = 8.24493e-1, B1 = -4.0899e-3, B2 = 7.6438e-5, B3 = -8.2467e-7, B4 = 5.3875e-9,
 *   C0 = -5.72466e-3, C1 = 1.0227e-4, C2 = -1.6546e-6,
 *   D0 = 4.8314e-4,
 *   A0 = 999.842594, A1 = 6.793952e-2, A2 = -9.095290e-3, A3 = 1.001685e-4,
 *   A4 = -1.120083e-6, A5 = 6.536332e-9,
 *   FQ0 = 54.6746, FQ1 = -0.603459, FQ2 = 1.09987e-2, FQ3 = -6.1670e-5,
 *   G0 = 7.944e-2, G1 = 1.6483e-2, G2 = -5.3009e-4,
 *   i0 = 2.2838e-3, i1 = -1.0981e-5, i2 = -1.6078e-6,
 *   J0 =1.91075e-4,
 *   M0 = -9.9348e-7, M1 = 2.0816e-8, M2 = 9.1697e-10,
 *   E0 = 19652.21, E1 = 148.4206, E2 = -2.327105, E3 = 1.360477e-2, E4 = -5.155288e-5,
 *   H0 = 3.239908, H1 = 1.43713e-3, H2 = 1.16092e-4, H3 = -5.77905e-7,
 *   K0 = 8.50935e-5, K1 =-6.12293e-6, K2 = 5.2787e-8
 *
 */
int mb_seabird_density(int verbose, double salinity, double temperature, double pressure, double *density, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       salinity:   %f PSU\n", salinity);
		fprintf(stderr, "dbg2       temperature:%f deg C\n", temperature);
		fprintf(stderr, "dbg2       pressure:   %f dbar\n", pressure);
	}

	const double B0 = 8.24493e-1, B1 = -4.0899e-3, B2 = 7.6438e-5, B3 = -8.2467e-7, B4 = 5.3875e-9;
	const double C0 = -5.72466e-3, C1 = 1.0227e-4, C2 = -1.6546e-6;
	const double D0 = 4.8314e-4;
	const double A0 = 999.842594, A1 = 6.793952e-2, A2 = -9.095290e-3, A3 = 1.001685e-4;
	const double A4 = -1.120083e-6, A5 = 6.536332e-9;
	const double FQ0 = 54.6746, FQ1 = -0.603459, FQ2 = 1.09987e-2, FQ3 = -6.1670e-5;
	const double G0 = 7.944e-2, G1 = 1.6483e-2, G2 = -5.3009e-4;
	const double i0 = 2.2838e-3, i1 = -1.0981e-5, i2 = -1.6078e-6;
	const double J0 =1.91075e-4;
	const double M0 = -9.9348e-7, M1 = 2.0816e-8, M2 = 9.1697e-10;
	const double E0 = 19652.21, E1 = 148.4206, E2 = -2.327105, E3 = 1.360477e-2, E4 = -5.155288e-5;
	const double H0 = 3.239908, H1 = 1.43713e-3, H2 = 1.16092e-4, H3 = -5.77905e-7;
	const double K0 = 8.50935e-5, K1 =-6.12293e-6, K2 = 5.2787e-8;

	const double t2 = temperature * temperature;
	const double t3 = temperature * t2;
	const double t4 = temperature * t3;
	const double t5 = temperature * t4;
	if (salinity <= 0.0)
		salinity = 0.000001;
	const double s32 = pow(salinity, 1.5);
	pressure /= 10.0;
	*density = A0 + A1 * temperature + A2 * t2 + A3 * t3 + A4 * t4 + A5 * t5
			+ (B0 + B1 * temperature + B2 * t2 + B3 * t3 + B4 * t4) * salinity
			+ (C0 + C1 * temperature + C2 * t2) * s32 + D0 * salinity * salinity;
	const double kw = E0 + E1 * temperature + E2 * t2 + E3 * t3 + E4 * t4;
	const double aw = H0 + H1 * temperature + H2 * t2 + H3 * t3;
	const double bw = K0 + K1 * temperature + K2 * t2;
	const double k = kw + (FQ0 + FQ1 * temperature + FQ2 * t2 + FQ3 * t3) * salinity
			+ (G0 + G1 * temperature + G2 * t2) * s32
			+ (aw + (i0 + i1 * temperature + i2 * t2) * salinity + (J0 * s32)) * pressure
			+ (bw + (M0 + M1 * temperature + M2 * t2) * salinity) * pressure * pressure;
	const double val = 1 - pressure / k;
	if (val)
		*density = *density / val;

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       density:               %f kg/m^3\n", *density);
		fprintf(stderr, "dbg2       error:                 %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
/*
 * Depth calculation from page 152 of Seabird manual
 *
 * Parameter units:
 *   pressure: pressure in decibars
 *   latitude: latitude in degrees
 *   depth: depth in meters
 */
int mb_seabird_depth(int verbose, double pressure, double latitude, double *depth, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       pressure:   %f dbar\n", pressure);
		fprintf(stderr, "dbg2       latitude:   %f degrees\n", latitude);
	}

	double x = sin(latitude / 57.29578);
	x = x * x;
	const double gr = 9.780318 * (1.0 + (5.2788e-3 + 2.36e-5 * x) * x) + 1.092e-6 * pressure;
	*depth = (((-1.82e-15 * pressure + 2.279e-10) * pressure - 2.2512e-5) * pressure + 9.72659) * pressure;
	if (gr)
		*depth/=gr;

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       depth:                 %f m\n", *depth);
		fprintf(stderr, "dbg2       error:                 %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                %d\n", status);
	}

	return (status);

}
/*--------------------------------------------------------------------*/
int mb_seabird_salinity(int verbose, double conductivity, double temperature, double pressure, double *salinity, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:        %d\n", verbose);
		fprintf(stderr, "dbg2       conductivity:   %f S/m\n", conductivity);
		fprintf(stderr, "dbg2       temperature:    %f deg C ITPS-68\n", temperature);
		fprintf(stderr, "dbg2       pressure:       %f dbar\n", pressure);
	}

	double RT = 0.0;
	double RP = 0.0;

	/* constants for salinity calculation */
	const double A1 = 2.070e-5, A2 = -6.370e-10, A3 = 3.989e-15;
	const double B1 = 3.426e-2, B2 = 4.464e-4, B3 = 4.215e-1, B4 = -3.107e-3;
	const double C0 = 6.766097e-1, C1 = 2.00564e-2, C2 = 1.104259e-4, C3 = -6.9698e-7, C4 = 1.0031e-9;
	const double a[6] = { 0.0080, -0.1692, 25.3851, 14.0941, -7.0261, 2.7081 };
	const double b[6] = { 0.0005, -0.0056, -0.0066, -0.0375, 0.0636, -0.0144 };

	if (conductivity <= 0.0) {
		/* result = 0.0; */
	} else {
		conductivity *= 10.0; /* convert Siemens/meter to mmhos/cm */
		const double R = conductivity / 42.914;
		double val = 1 + B1 * temperature + B2 * temperature * temperature + B3 * R + B4 * R * temperature;
		if (val)
			RP = 1 + (pressure * (A1 + pressure * (A2 + pressure * A3))) / val;
		val = RP * (C0 + (temperature * (C1 + temperature * (C2 + temperature * (C3 + temperature * C4)))));
		if (val)
			RT = R / val;
        if (RT <= 0.0)
			RT = 0.000001;
        double sum1 = 0.0;
        double sum2 = 0.0;
        for (int i = 0; i < 6; i++) {
        		const double temp = pow(RT, (double)i / 2.0);
			sum1 += a[i] * temp;
			sum2 += b[i] * temp;
		}
        val = 1.0 + 0.0162 * (temperature - 15.0);
		if (val)
			*salinity = sum1 + sum2 * (temperature - 15.0) / val;
		else
			*salinity = -99.;
	}

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       salinity:              %f PSU\n", *salinity);
		fprintf(stderr, "dbg2       error:                 %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                %d\n", status);
	}

	return (status);

}
/*--------------------------------------------------------------------*/
/* Sound speed calculation using equations from
	Delgrosso JASA, Oct. 1974, Vol 56, No 4 */
int mb_seabird_soundspeed(int verbose, int algorithm, double salinity,
						  double temperature, double pressure,
						  double *soundspeed, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:        %d\n", verbose);
		fprintf(stderr, "dbg2       algorithm:      %d\n", algorithm);
		fprintf(stderr, "dbg2       salinity:       %f PSU\n", salinity);
		fprintf(stderr, "dbg2       temperature:    %f deg C ITPS-68\n", temperature);
		fprintf(stderr, "dbg2       pressure:       %f dbar\n", pressure);
	}

	/* algorithm 1: Chen and Millero 1977, JASA,62,1129-1135 */
	if (algorithm == MB_SOUNDSPEEDALGORITHM_CHENMILLERO) {

		pressure = pressure / 10.0; /* scale pressure to bars */
		if(salinity < 0.0)
			salinity = 0.0;
		const double d = 1.727e-3 - 7.9836e-6 * pressure;
		const double b1 = 7.3637e-5 + 1.7945e-7 * temperature;
		const double b0 = -1.922e-2 - 4.42e-5 * temperature;
		const double b = b0 + b1 * pressure;
		const double a3 = (-3.389e-13 * temperature + 6.649e-12) * temperature + 1.100e-10;
		const double a2 = ((7.988e-12 * temperature - 1.6002e-10) * temperature + 9.1041e-9) * temperature - 3.9064e-7;
		const double a1 = (((-2.0122e-10 * temperature + 1.0507e-8) * temperature - 6.4885e-8) * temperature - 1.2580e-5) * temperature + 9.4742e-5;
		const double a0 = (((-3.21e-8 * temperature + 2.006e-6) * temperature + 7.164e-5) * temperature -1.262e-2) * temperature + 1.389;
		const double a = ((a3 * pressure + a2) * pressure + a1) * pressure + a0;
		const double c3 = (-2.3643e-12 * temperature + 3.8504e-10) * temperature - 9.7729e-9;
		const double c2 = (((1.0405e-12 * temperature -2.5335e-10) * temperature + 2.5974e-8) * temperature - 1.7107e-6) * temperature + 3.1260e-5;
		const double c1 = (((-6.1185e-10 * temperature + 1.3621e-7) * temperature - 8.1788e-6) * temperature + 6.8982e-4) * temperature + 0.153563;
		const double c0 = ((((3.1464e-9 * temperature - 1.47800e-6) * temperature + 3.3420e-4) * temperature - 5.80852e-2) * temperature + 5.03711) * temperature + 1402.388;
		const double c = ((c3 * pressure + c2) * pressure + c1) * pressure + c0;
		*soundspeed = c + (a + b * sqrt(salinity) + d * salinity) * salinity;
	}

	/* algorithm 2: Wilson JASA, 1960, 32, 1357  */
	else if (algorithm == MB_SOUNDSPEEDALGORITHM_WILSON) {
		const double pr = 0.1019716 * (pressure + 10.1325);
		const double sd = salinity - 35.0;
		double a = (((7.9851e-6 * temperature - 2.6045e-4) * temperature - 4.4532e-2) * temperature + 4.5721) * temperature + 1449.14;
		*soundspeed = (7.7711e-7 * temperature - 1.1244e-2) * temperature + 1.39799;
		const double v0 = (1.69202e-3 * sd + *soundspeed) * sd + a;
		a = ((4.5283e-8 * temperature + 7.4812e-6) * temperature - 1.8607e-4) * temperature + 0.16072;
		*soundspeed = (1.579e-9 * temperature + 3.158e-8) * temperature + 7.7016e-5;
		const double v1 = *soundspeed * sd + a;
		a = (1.8563e-9 * temperature - 2.5294e-7) * temperature + 1.0268e-5;
		*soundspeed = -1.2943e-7 * sd + a;
		a = -1.9646e-10 * temperature + 3.5216e-9;
		*soundspeed = (((-3.3603e-12 * pr + a) * pr + *soundspeed) * pr + v1) * pr + v0;
	}

	/* algorithm 3 (default): Delgrosso JASA, Oct. 1974, Vol 56, No 4 */
	else /* if (algorithm == MB_SOUNDSPEEDALGORITHM_DELGROSSO) */ {
		const double c000 = 1402.392;
		pressure = pressure / 9.80665; /* convert pressure from decibars to KG / CM**2 */
		const double dct = (0.501109398873e1 - (0.550946843172e-1 - 0.22153596924e-3 * temperature) * temperature) * temperature;
		const double dcs = (0.132952290781e1 + 0.128955756844e-3 * salinity) * salinity;
		const double dcp = (0.156059257041e0 + (0.244998688441e-4 - 0.83392332513e-8 * pressure) * pressure) * pressure;
		const double dcstp =  -0.127562783426e-1 * temperature * salinity
				+ 0.635191613389e-2 * temperature * pressure
				+ 0.265484716608e-7 * temperature * temperature * pressure * pressure
				- 0.159349479045e-5 * temperature * pressure * pressure
				+ 0.522116437235e-9 * temperature * pressure * pressure * pressure
				- 0.438031096213e-6 * temperature * temperature * temperature * pressure
				- 0.161674495909e-8 * salinity * salinity * pressure * pressure
				+ 0.968403156410e-4 * temperature * temperature * salinity
				+ 0.485639620015e-5 * temperature * salinity * salinity * pressure
				- 0.340597039004e-3 * temperature * salinity * pressure;
		*soundspeed = c000 + dct + dcs + dcp + dcstp;
	}

	/* assume success */
	*error = MB_ERROR_NO_ERROR;
	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       soundspeed:            %f m/s\n", *soundspeed);
		fprintf(stderr, "dbg2       error:                 %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
