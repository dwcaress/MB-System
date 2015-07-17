/*--------------------------------------------------------------------
 *    The MB-system:	mb_platform_math.c	11/1/00
 *    $Id: mb_platform_math.c 2253 2015-07-16 02:12:26Z gtroni $
 *
 *    Copyright (c) 2015-2015 by
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
 * This source file includes the math functions used on mb_platform.c.
 * Note that all matrices are assumed column-major.
 * 
 * Author:	G. Troni
 * Date:	July 15, 2015
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

/* mbio include files */

static char svn_id[]="$Id: mb_platform_math.c 2253 2015-07-16 02:12:26Z gtroni $";

/*--------------------------------------------------------------------*/
void
mb_platform_math_matrix_times_matrix_3x3 (double* A, double* B, double* AB)
{
#define A(i,j) (A[j*3+i])
#define B(i,j) (B[j*3+i])
#define AB(i,j) (AB[j*3+i])

	AB(0,0) = A(0,0)*B(0,0) + A(0,1)*B(1,0) + A(0,2)*B(2,0);
	AB(1,0) = A(1,0)*B(0,0) + A(1,1)*B(1,0) + A(1,2)*B(2,0);
	AB(2,0) = A(2,0)*B(0,0) + A(2,1)*B(1,0) + A(2,2)*B(2,0);

	AB(0,1) = A(0,0)*B(0,1) + A(0,1)*B(1,1) + A(0,2)*B(2,1);
	AB(1,1) = A(1,0)*B(0,1) + A(1,1)*B(1,1) + A(1,2)*B(2,1);
	AB(2,1) = A(2,0)*B(0,1) + A(2,1)*B(1,1) + A(2,2)*B(2,1);

	AB(0,2) = A(0,0)*B(0,2) + A(0,1)*B(1,2) + A(0,2)*B(2,2);
	AB(1,2) = A(1,0)*B(0,2) + A(1,1)*B(1,2) + A(1,2)*B(2,2);
	AB(2,2) = A(2,0)*B(0,2) + A(2,1)*B(1,2) + A(2,2)*B(2,2);

#undef A
#undef B
#undef AB
}
/*--------------------------------------------------------------------*/
void
mb_platform_math_matrix_transpose_3x3 (double* R, double* R_T)
{
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
void
mb_platform_math_rph2rot (double* rph, double* R)
{
#define R(i,j) (R[j*3+i])
	double sr, sp, sh, cr, cp, ch;
	sr = sin (rph[0]);
	sp = sin (rph[1]);
	sh = sin (rph[2]);
	cr = cos (rph[0]);
	cp = cos (rph[1]);
	ch = cos (rph[2]);

	R(0,0) = ch*cp; R(0,1) = -sh*cr + ch*sp*sr; R(0,2) = sh*sr + ch*sp*cr;
	R(1,0) = sh*cp; R(1,1) = ch*cr + sh*sp*sr; R(1,2) = -ch*sr + sh*sp*cr;
	R(2,0) = -sp; R(2,1) = cp*sr; R(2,2) = cp*cr;
#undef R
}
/*--------------------------------------------------------------------*/
void
mb_platform_math_rot2rph (double* R, double* rph)
{
#define R(i,j) (R[j*3+i])
	
	/* Calculate heading */
	rph[2] = atan2 (R(1,0), R(0,0));
	double sh, ch;
	sh = sin (rph[2]);
	ch = cos (rph[2]);

	/* Calculate pitch */
	rph[1] = atan2 (-R(2,0), R(0,0)*ch + R(1,0)*sh);

	/* Calculate roll */
	rph[0] = atan2 (R(0,2)*sh - R(1,2)*ch, -R(0,1)*sh + R(1,1)*ch);
#undef R
}
/*--------------------------------------------------------------------*/
void
mb_platform_math_attitude_offset (double  target_offset_roll,          
								  double  target_offset_pitch, 
								  double  target_offset_heading,
								  double  source_offset_roll,        
								  double  source_offset_pitch, 
								  double  source_offset_heading,
								  double* target2source_offset_roll, 
								  double* target2source_offset_pitch, 
								  double* target2source_offset_heading)
{
	double rph_target_offset[3];
	double rph_source_offset[3];
	double rph_offset_target_to_source[3];

	double R1[9];
	double R1T[9];
	double R2[9];
	double R1T2[9];

	/* Check trivial case */
	if (source_offset_roll == 0.0 && 
		source_offset_pitch == 0.0 && 
		source_offset_heading == 0.0 )
	{
		*target2source_offset_roll    = source_offset_roll;
		*target2source_offset_pitch   = source_offset_pitch;
		*target2source_offset_heading = source_offset_heading;
	}
	else
	{
		rph_source_offset[0]  = source_offset_roll;
		rph_source_offset[1]  = source_offset_pitch;
		rph_source_offset[2]  = source_offset_heading;

		rph_target_offset[0]  = target_offset_roll;
		rph_target_offset[1]  = target_offset_pitch;
		rph_target_offset[2]  = target_offset_heading;

		mb_platform_math_rph2rot (rph_source_offset, R1);
		mb_platform_math_matrix_transpose_3x3 (R1, R1T);
		mb_platform_math_rph2rot (rph_target_offset, R2);
		mb_platform_math_matrix_times_matrix_3x3 (R1T, R2, R1T2);
		mb_platform_math_rot2rph (R1T2, rph_offset_target_to_source);

		*target2source_offset_roll    = rph_offset_target_to_source[0];
		*target2source_offset_pitch   = rph_offset_target_to_source[1];
		*target2source_offset_heading = rph_offset_target_to_source[2];
	}

}
/*--------------------------------------------------------------------*/
void
mb_platform_math_attitude_platform (double  nav_attitude_roll,    
									double  nav_attitude_pitch, 
									double  nav_attitude_heading,
									double  attitude_offset_roll,          
									double  attitude_offset_pitch, 
									double  attitude_offset_heading,
									double* platform_roll, 
									double* platform_pitch, 
									double* platform_heading)
{
	double rph_nav_attitude[3];
	double rph_attitude_offset[3];
	double rph_platform_attitude[3];

	double R1[9];
	double R2[9];
	double R2T[9];
	double R12T[9];

	rph_nav_attitude[0]  = nav_attitude_roll;
	rph_nav_attitude[1]  = nav_attitude_pitch;
	rph_nav_attitude[2]  = nav_attitude_heading;

	rph_attitude_offset[0]  = attitude_offset_roll;
	rph_attitude_offset[1]  = attitude_offset_pitch;
	rph_attitude_offset[2]  = attitude_offset_heading;

	mb_platform_math_rph2rot (rph_nav_attitude, R1);
	mb_platform_math_rph2rot (rph_attitude_offset, R2);
	mb_platform_math_matrix_transpose_3x3 (R2, R2T);
	mb_platform_math_matrix_times_matrix_3x3 (R1, R2T, R12T);
	mb_platform_math_rot2rph (R12T, rph_platform_attitude);

	*platform_roll    = rph_platform_attitude[0];
	*platform_pitch   = rph_platform_attitude[1];
	*platform_heading = rph_platform_attitude[2];

}
/*--------------------------------------------------------------------*/
void
mb_platform_math_attitude_target   (double  source_attitude_roll,    
									double  source_attitude_pitch, 
									double  source_attitude_heading,
									double  target_offset_to_source_roll,          
									double  target_offset_to_source_pitch, 
									double  target_offset_to_source_heading,
									double* target_roll, 
									double* target_pitch, 
									double* target_heading)
{
	double rph_source_attitude[3];
	double rph_target_offset[3];
	double rph_target_attitude[3];

	double R1[9];
	double R2[9];
	double R12[9];

	rph_source_attitude[0]  = source_attitude_roll;
	rph_source_attitude[1]  = source_attitude_pitch;
	rph_source_attitude[2]  = source_attitude_heading;

	rph_target_offset[0]  = target_offset_to_source_roll;
	rph_target_offset[1]  = target_offset_to_source_pitch;
	rph_target_offset[2]  = target_offset_to_source_heading;

	mb_platform_math_rph2rot (rph_source_attitude, R1);
	mb_platform_math_rph2rot (rph_target_offset, R2);
	mb_platform_math_matrix_times_matrix_3x3 (R1, R2, R12);
	mb_platform_math_rot2rph (R12, rph_target_attitude);

	*target_roll    = rph_target_attitude[0];
	*target_pitch   = rph_target_attitude[1];
	*target_heading = rph_target_attitude[2];

}
