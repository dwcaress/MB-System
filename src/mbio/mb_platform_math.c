/*--------------------------------------------------------------------
 *    The MB-system:	mb_platform_math.c	11/1/00
 *
 *    Copyright (c) 2015-2023 by
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
 * This source file includes the math functions used on mb_platform.c.
 * Note that all matrices are assumed column-major.
 *
 * Author:	Giancarlo Troni
 * Date:	July 15, 2015
 *
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <mb_define.h>
#include <mb_status.h>

/*--------------------------------------------------------------------*/
void mb_platform_math_matrix_times_vector_3x1(double *A, double *b, double *Ab) {
	Ab[0] = (A[0] * b[0]) + (A[3] * b[1]) + (A[6] * b[2]);
	Ab[1] = (A[1] * b[0]) + (A[4] * b[1]) + (A[7] * b[2]);
	Ab[2] = (A[2] * b[0]) + (A[5] * b[1]) + (A[8] * b[2]);
}

/*--------------------------------------------------------------------*/
void mb_platform_math_matrix_times_matrix_3x3(double *A, double *B, double *AB) {
#define A(i, j) (A[j * 3 + i])
#define B(i, j) (B[j * 3 + i])
#define AB(i, j) (AB[j * 3 + i])

	AB(0, 0) = A(0, 0) * B(0, 0) + A(0, 1) * B(1, 0) + A(0, 2) * B(2, 0);
	AB(1, 0) = A(1, 0) * B(0, 0) + A(1, 1) * B(1, 0) + A(1, 2) * B(2, 0);
	AB(2, 0) = A(2, 0) * B(0, 0) + A(2, 1) * B(1, 0) + A(2, 2) * B(2, 0);

	AB(0, 1) = A(0, 0) * B(0, 1) + A(0, 1) * B(1, 1) + A(0, 2) * B(2, 1);
	AB(1, 1) = A(1, 0) * B(0, 1) + A(1, 1) * B(1, 1) + A(1, 2) * B(2, 1);
	AB(2, 1) = A(2, 0) * B(0, 1) + A(2, 1) * B(1, 1) + A(2, 2) * B(2, 1);

	AB(0, 2) = A(0, 0) * B(0, 2) + A(0, 1) * B(1, 2) + A(0, 2) * B(2, 2);
	AB(1, 2) = A(1, 0) * B(0, 2) + A(1, 1) * B(1, 2) + A(1, 2) * B(2, 2);
	AB(2, 2) = A(2, 0) * B(0, 2) + A(2, 1) * B(1, 2) + A(2, 2) * B(2, 2);

#undef A
#undef B
#undef AB
}

/*--------------------------------------------------------------------*/
void mb_platform_math_matrix_transpose_3x3(double *R, double *R_T) {
	R_T[0] = R[0];
	R_T[3] = R[1];
	R_T[6] = R[2];
	R_T[1] = R[3];
	R_T[4] = R[4];
	R_T[7] = R[5];
	R_T[2] = R[6];
	R_T[5] = R[7];
	R_T[8] = R[8];
}

/*--------------------------------------------------------------------*/
void mb_platform_math_rph2rot(double *rph, double *R) {
	const double sr = sin(DTR * rph[0]);
	const double sp = sin(DTR * rph[1]);
	const double sh = sin(DTR * rph[2]);
	const double cr = cos(DTR * rph[0]);
	const double cp = cos(DTR * rph[1]);
	const double ch = cos(DTR * rph[2]);

#define R(i, j) (R[j * 3 + i])
	R(0, 0) = ch * cp;
	R(0, 1) = -sh * cr + ch * sp * sr;
	R(0, 2) = sh * sr + ch * sp * cr;
	R(1, 0) = sh * cp;
	R(1, 1) = ch * cr + sh * sp * sr;
	R(1, 2) = -ch * sr + sh * sp * cr;
	R(2, 0) = -sp;
	R(2, 1) = cp * sr;
	R(2, 2) = cp * cr;
#undef R
}

/*--------------------------------------------------------------------*/
void mb_platform_math_rot2rph(double *R, double *rph) {
#define R(i, j) (R[j * 3 + i])

	/* Calculate heading */
	rph[2] = RTD * atan2(R(1, 0), R(0, 0));

	const double sh = sin(DTR * rph[2]);
	const double ch = cos(DTR * rph[2]);

	/* Calculate pitch */
	rph[1] = RTD * atan2(-R(2, 0), R(0, 0) * ch + R(1, 0) * sh);

	/* Calculate roll */
	rph[0] = RTD * atan2(R(0, 2) * sh - R(1, 2) * ch, -R(0, 1) * sh + R(1, 1) * ch);
#undef R
}

/*--------------------------------------------------------------------*/
int mb_platform_math_attitude_offset(int verbose, double target_offset_roll, double target_offset_pitch,
                                     double target_offset_heading, double source_offset_roll, double source_offset_pitch,
                                     double source_offset_heading, double *target2source_offset_roll,
                                     double *target2source_offset_pitch, double *target2source_offset_heading, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                 %d\n", verbose);
		fprintf(stderr, "dbg2       target_offset_roll:      %f\n", target_offset_roll);
		fprintf(stderr, "dbg2       target_offset_pitch:     %f\n", target_offset_pitch);
		fprintf(stderr, "dbg2       target_offset_heading:   %f\n", target_offset_heading);
		fprintf(stderr, "dbg2       source_offset_roll:      %f\n", source_offset_roll);
		fprintf(stderr, "dbg2       source_offset_pitch:     %f\n", source_offset_pitch);
		fprintf(stderr, "dbg2       source_offset_heading:   %f\n", source_offset_heading);
	}


	/* Check trivial case */
	if (source_offset_roll == 0.0 && source_offset_pitch == 0.0 && source_offset_heading == 0.0) {
		*target2source_offset_roll = target_offset_roll;
		*target2source_offset_pitch = target_offset_pitch;
		*target2source_offset_heading = target_offset_heading;
	}
	else {
		double rph_source_offset[3];
		rph_source_offset[0] = source_offset_roll;
		rph_source_offset[1] = source_offset_pitch;
		rph_source_offset[2] = source_offset_heading;

		double rph_target_offset[3];
		rph_target_offset[0] = target_offset_roll;
		rph_target_offset[1] = target_offset_pitch;
		rph_target_offset[2] = target_offset_heading;

		double R1[9];
		double R1T[9];
		double R2[9];
		double R1T2[9];
		mb_platform_math_rph2rot(rph_source_offset, R1);
		mb_platform_math_matrix_transpose_3x3(R1, R1T);
		mb_platform_math_rph2rot(rph_target_offset, R2);
		mb_platform_math_matrix_times_matrix_3x3(R1T, R2, R1T2);
		double rph_offset_target_to_source[3];
		mb_platform_math_rot2rph(R1T2, rph_offset_target_to_source);

		*target2source_offset_roll = rph_offset_target_to_source[0];
		*target2source_offset_pitch = rph_offset_target_to_source[1];
		*target2source_offset_heading = rph_offset_target_to_source[2];
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       target2source_offset_roll:      %f\n", *target2source_offset_roll);
		fprintf(stderr, "dbg2       target2source_offset_pitch:     %f\n", *target2source_offset_pitch);
		fprintf(stderr, "dbg2       target2source_offset_heading:   %f\n", *target2source_offset_heading);
		fprintf(stderr, "dbg2       error:			               %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:			               %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mb_platform_math_attitude_platform(int verbose, double nav_attitude_roll, double nav_attitude_pitch,
                                       double nav_attitude_heading, double attitude_offset_roll, double attitude_offset_pitch,
                                       double attitude_offset_heading, double *platform_roll, double *platform_pitch,
                                       double *platform_heading, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                 %d\n", verbose);
		fprintf(stderr, "dbg2       nav_attitude_roll:       %f\n", nav_attitude_roll);
		fprintf(stderr, "dbg2       nav_attitude_pitch:      %f\n", nav_attitude_pitch);
		fprintf(stderr, "dbg2       nav_attitude_heading:    %f\n", nav_attitude_heading);
		fprintf(stderr, "dbg2       attitude_offset_roll:    %f\n", attitude_offset_roll);
		fprintf(stderr, "dbg2       attitude_offset_pitch:   %f\n", attitude_offset_pitch);
		fprintf(stderr, "dbg2       attitude_offset_heading: %f\n", attitude_offset_heading);
	}

	double rph_nav_attitude[3];
	rph_nav_attitude[0] = nav_attitude_roll;
	rph_nav_attitude[1] = nav_attitude_pitch;
	rph_nav_attitude[2] = nav_attitude_heading;

	double rph_attitude_offset[3];
	rph_attitude_offset[0] = attitude_offset_roll;
	rph_attitude_offset[1] = attitude_offset_pitch;
	rph_attitude_offset[2] = attitude_offset_heading;

	double R1[9];
	double R2[9];
	double R2T[9];
	double R12T[9];
	mb_platform_math_rph2rot(rph_nav_attitude, R1);
	mb_platform_math_rph2rot(rph_attitude_offset, R2);
	mb_platform_math_matrix_transpose_3x3(R2, R2T);
	mb_platform_math_matrix_times_matrix_3x3(R1, R2T, R12T);
	double rph_platform_attitude[3];
	mb_platform_math_rot2rph(R12T, rph_platform_attitude);

	*platform_roll = rph_platform_attitude[0];
	*platform_pitch = rph_platform_attitude[1];
	*platform_heading = rph_platform_attitude[2];

	if (*platform_heading < 0.0)
		*platform_heading += 360.0;
	else if (*platform_heading >= 360.0)
		*platform_heading -= 360.0;

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       platform_roll:      %f\n", *platform_roll);
		fprintf(stderr, "dbg2       platform_pitch:     %f\n", *platform_pitch);
		fprintf(stderr, "dbg2       platform_heading:   %f\n", *platform_heading);
		fprintf(stderr, "dbg2       error:			   %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:			   %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mb_platform_math_attitude_target(int verbose, double source_attitude_roll, double source_attitude_pitch,
                                     double source_attitude_heading, double target_offset_to_source_roll,
                                     double target_offset_to_source_pitch, double target_offset_to_source_heading,
                                     double *target_roll, double *target_pitch, double *target_heading, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                           %d\n", verbose);
		fprintf(stderr, "dbg2       source_attitude_roll:              %f\n", source_attitude_roll);
		fprintf(stderr, "dbg2       source_attitude_pitch:             %f\n", source_attitude_pitch);
		fprintf(stderr, "dbg2       source_attitude_heading:           %f\n", source_attitude_heading);
		fprintf(stderr, "dbg2       target_offset_to_source_roll:      %f\n", target_offset_to_source_roll);
		fprintf(stderr, "dbg2       target_offset_to_source_pitch:     %f\n", target_offset_to_source_pitch);
		fprintf(stderr, "dbg2       target_offset_to_source_heading:   %f\n", target_offset_to_source_heading);
	}

	double rph_source_attitude[3];
	rph_source_attitude[0] = source_attitude_roll;
	rph_source_attitude[1] = source_attitude_pitch;
	rph_source_attitude[2] = source_attitude_heading;

	double rph_target_offset[3];
	rph_target_offset[0] = target_offset_to_source_roll;
	rph_target_offset[1] = target_offset_to_source_pitch;
	rph_target_offset[2] = target_offset_to_source_heading;

	double R1[9];
	double R2[9];
	double R12[9];
	mb_platform_math_rph2rot(rph_source_attitude, R1);
	mb_platform_math_rph2rot(rph_target_offset, R2);
	mb_platform_math_matrix_times_matrix_3x3(R1, R2, R12);
	double rph_target_attitude[3];
	mb_platform_math_rot2rph(R12, rph_target_attitude);

	*target_roll = rph_target_attitude[0];
	*target_pitch = rph_target_attitude[1];
	*target_heading = rph_target_attitude[2];

	if (*target_heading < 0.0)
		*target_heading += 360.0;
	else if (*target_heading >= 360.0)
		*target_heading -= 360.0;

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       target_roll:      %f\n", *target_roll);
		fprintf(stderr, "dbg2       target_pitch:     %f\n", *target_pitch);
		fprintf(stderr, "dbg2       target_heading:   %f\n", *target_heading);
		fprintf(stderr, "dbg2       error:			 %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:			 %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mb_platform_math_attitude_offset_corrected_by_nav(int verbose, double prev_attitude_roll, double prev_attitude_pitch,
                                                      double prev_attitude_heading, double target_offset_to_source_roll,
                                                      double target_offset_to_source_pitch,
                                                      double target_offset_to_source_heading, double updated_attitude_roll,
                                                      double updated_attitude_pitch, double updated_attitude_heading,
                                                      double *corrected_offset_roll, double *corrected_offset_pitch,
                                                      double *corrected_offset_heading, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                           %d\n", verbose);
		fprintf(stderr, "dbg2       prev_attitude_roll:                %f\n", prev_attitude_roll);
		fprintf(stderr, "dbg2       prev_attitude_pitch:               %f\n", prev_attitude_pitch);
		fprintf(stderr, "dbg2       prev_attitude_heading:             %f\n", prev_attitude_heading);
		fprintf(stderr, "dbg2       target_offset_to_source_roll:      %f\n", target_offset_to_source_roll);
		fprintf(stderr, "dbg2       target_offset_to_source_pitch:     %f\n", target_offset_to_source_pitch);
		fprintf(stderr, "dbg2       target_offset_to_source_heading:   %f\n", target_offset_to_source_heading);
		fprintf(stderr, "dbg2       updated_attitude_roll:             %f\n", updated_attitude_roll);
		fprintf(stderr, "dbg2       updated_attitude_pitch:            %f\n", updated_attitude_pitch);
		fprintf(stderr, "dbg2       updated_attitude_heading:          %f\n", updated_attitude_heading);
	}

	double rph_prev_attitude[3];
	rph_prev_attitude[0] = prev_attitude_roll;
	rph_prev_attitude[1] = prev_attitude_pitch;
	rph_prev_attitude[2] = prev_attitude_heading;

	double rph_target_offset[3];
	rph_target_offset[0] = target_offset_to_source_roll;
	rph_target_offset[1] = target_offset_to_source_pitch;
	rph_target_offset[2] = target_offset_to_source_heading;

	double rph_updated_attitude[3];
	rph_updated_attitude[0] = updated_attitude_roll;
	rph_updated_attitude[1] = updated_attitude_pitch;
	rph_updated_attitude[2] = updated_attitude_heading;

	double R1[9];
	double R2[9];
	double R3[9];
	double R3T[9];
	double R12[9];
	double R123T[9];
	mb_platform_math_rph2rot(rph_prev_attitude, R3);
	mb_platform_math_rph2rot(rph_target_offset, R2);
	mb_platform_math_rph2rot(rph_updated_attitude, R1);
	mb_platform_math_matrix_times_matrix_3x3(R1, R2, R12);
	mb_platform_math_matrix_transpose_3x3(R3, R3T);
	mb_platform_math_matrix_times_matrix_3x3(R12, R3T, R123T);
	double rph_corrected_offset_attitude[3];
	mb_platform_math_rot2rph(R123T, rph_corrected_offset_attitude);

	*corrected_offset_roll = rph_corrected_offset_attitude[0];
	*corrected_offset_pitch = rph_corrected_offset_attitude[1];
	*corrected_offset_heading = rph_corrected_offset_attitude[2];

	if (*corrected_offset_heading >= 360.0)
		*corrected_offset_heading -= 360.0;
	else if (*corrected_offset_heading < 0.0)
		*corrected_offset_heading += 360.0;

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       corrected_offset_roll:      %f\n", *corrected_offset_roll);
		fprintf(stderr, "dbg2       corrected_offset_pitch:     %f\n", *corrected_offset_pitch);
		fprintf(stderr, "dbg2       corrected_offset_heading:   %f\n", *corrected_offset_heading);
		fprintf(stderr, "dbg2       error:			 %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:			 %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mb_platform_math_attitude_rotate_beam(int verbose, double beam_acrosstrack, double beam_alongtrack, double beam_bath,
                                          double attitude_roll, double attitude_pitch, double attitude_heading,
                                          double *newbeam_easting, double *newbeam_northing, double *newbeam_bath, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                           %d\n", verbose);
		fprintf(stderr, "dbg2       beam_acrosstrack:                  %f\n", beam_acrosstrack);
		fprintf(stderr, "dbg2       beam_alongtrack:                   %f\n", beam_alongtrack);
		fprintf(stderr, "dbg2       beam_bath:                         %f\n", beam_bath);
		fprintf(stderr, "dbg2       attitude_roll:                     %f\n", attitude_roll);
		fprintf(stderr, "dbg2       attitude_pitc:                     %f\n", attitude_pitch);
		fprintf(stderr, "dbg2       attitude_heading:                  %f\n", attitude_heading);
	}

	double beam[3];
	beam[0] = beam_alongtrack;  // Local X-axis (along track)
	beam[1] = beam_acrosstrack; // Local Y-axis (across track)
	beam[2] = beam_bath;        // Local Z-axis (down)

	double rph_attitude[3];
	rph_attitude[0] = attitude_roll;
	rph_attitude[1] = attitude_pitch;
	rph_attitude[2] = attitude_heading;

	double R[9];
	mb_platform_math_rph2rot(rph_attitude, R);
	double newbeam[3];
	mb_platform_math_matrix_times_vector_3x1(R, beam, newbeam);

	*newbeam_northing = newbeam[0]; // Local X-axis (North)
	*newbeam_easting = newbeam[1];  // Local Y-axis (East)
	*newbeam_bath = newbeam[2];     // Local Z-axis (Down)

	int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       newbeam_easting:  %f\n", *newbeam_easting);
		fprintf(stderr, "dbg2       newbeam_northing: %f\n", *newbeam_northing);
		fprintf(stderr, "dbg2       newbeam_bath:     %f\n", *newbeam_bath);
		fprintf(stderr, "dbg2       error:			 %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:			 %d\n", status);
	}

	return (status);
}
