/*--------------------------------------------------------------------
 *    The MB-system:  mbv_camera.c  11/12/2026
 *
 *    Copyright (c) 2026 by
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * Camera matrices of the mbview views (see mbv_camera.h). This file uses no
 * toolkit and no OpenGL.
 */

#include <math.h>
#include <string.h>

#include "mb_define.h"

#include "mbv_camera.h"

/*------------------------------------------------------------------------------*/
void mbv_camera_identity(double m[16]) {
	for (int k = 0; k < 16; k++)
		m[k] = (k % 5 == 0) ? 1.0 : 0.0;
}
/*------------------------------------------------------------------------------*/
/* m = m * b, as glMultMatrixd */
static void mbv_camera_multiply(double m[16], const double b[16]) {
	double r[16];
	for (int column = 0; column < 4; column++)
		for (int row = 0; row < 4; row++)
			r[4 * column + row] = m[row] * b[4 * column] + m[4 + row] * b[4 * column + 1] + m[8 + row] * b[4 * column + 2] +
			                      m[12 + row] * b[4 * column + 3];
	memcpy(m, r, sizeof(r));
}
/*------------------------------------------------------------------------------*/
void mbv_camera_ortho(double m[16], double left, double right, double bottom, double top, double znear, double zfar) {
	double o[16] = {0.0};
	o[0] = 2.0 / (right - left);
	o[5] = 2.0 / (top - bottom);
	o[10] = -2.0 / (zfar - znear);
	o[12] = -(right + left) / (right - left);
	o[13] = -(top + bottom) / (top - bottom);
	o[14] = -(zfar + znear) / (zfar - znear);
	o[15] = 1.0;
	mbv_camera_multiply(m, o);
}
/*------------------------------------------------------------------------------*/
void mbv_camera_perspective(double m[16], double fovy, double aspect, double znear, double zfar) {
	/* the formula of the GLU reference implementation, which also leaves the
	   matrix unchanged for these degenerate arguments */
	const double radians = fovy / 2.0 * DTR;
	const double deltaz = zfar - znear;
	const double sine = sin(radians);
	if (deltaz == 0.0 || sine == 0.0 || aspect == 0.0)
		return;
	const double cotangent = cos(radians) / sine;
	double p[16] = {0.0};
	p[0] = cotangent / aspect;
	p[5] = cotangent;
	p[10] = -(zfar + znear) / deltaz;
	p[11] = -1.0;
	p[14] = -2.0 * znear * zfar / deltaz;
	mbv_camera_multiply(m, p);
}
/*------------------------------------------------------------------------------*/
void mbv_camera_translate(double m[16], double x, double y, double z) {
	double t[16];
	mbv_camera_identity(t);
	t[12] = x;
	t[13] = y;
	t[14] = z;
	mbv_camera_multiply(m, t);
}
/*------------------------------------------------------------------------------*/
void mbv_camera_rotate(double m[16], double angle, double x, double y, double z) {
	/* the axis is normalised, as glRotated does; a zero axis leaves m unchanged */
	const double length = sqrt(x * x + y * y + z * z);
	if (length == 0.0)
		return;
	x /= length;
	y /= length;
	z /= length;
	const double c = cos(angle * DTR);
	const double s = sin(angle * DTR);
	const double one_c = 1.0 - c;
	double r[16] = {0.0};
	r[0] = x * x * one_c + c;
	r[1] = y * x * one_c + z * s;
	r[2] = x * z * one_c - y * s;
	r[4] = x * y * one_c - z * s;
	r[5] = y * y * one_c + c;
	r[6] = y * z * one_c + x * s;
	r[8] = x * z * one_c + y * s;
	r[9] = y * z * one_c - x * s;
	r[10] = z * z * one_c + c;
	r[15] = 1.0;
	mbv_camera_multiply(m, r);
}
/*------------------------------------------------------------------------------*/
