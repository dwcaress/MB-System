/*--------------------------------------------------------------------
 *    The MB-system:	mb_surface.c	5/2/94
 *    $Id: mb_surface.c,v 5.0 2003-03-22 03:10:36 caress Exp $
 *
 *    Inclusion in MB-System:
 *    Copyright (c) 1994, 2003 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *
 *    Algorithm and original code:
 *    Copyright (c) 1991 by P. Wessel and W. H. F. Smith
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * SURFUNC is a function for gridding data using a minimum curvature
 * algorithm developed by W.H.F. Smith and P. Wessel.  The source
 * code below is almost entirely taken from the source code
 * "surface.c" distributed as part of the GMT-system by Wessel
 * and Smith. The MB-System Copyright notice above applies only
 * to the inclusion of this code in MB-System and changes made to
 * the code as part of that inclusion.  The algorithm and the
 * bulk of the code remains Copyright (c) by P. Wessel and W. H. F. Smith.
 *
 *--------------------------------------------------------------------*
 *
 * The original Smith and Wessel comments follow:
 *
 * surface.c:  a gridding program.
 * reads xyz triples and fits a surface to the data.
 * surface satisfies (1 - T) D4 z - T D2 z = 0,
 * where D4 is the biharmonic operator,
 * D2 is the Laplacian,
 * and T is a "tension factor" between 0 and 1.
 * End member T = 0 is the classical minimum curvature
 * surface.  T = 1 gives a harmonic surface.  Use T = 0.25
 * or so for potential data; something more for topography.
 *
 * Program includes overrelaxation for fast convergence and
 * automatic optimal grid factorization.
 *
 * See Smith & Wessel (Geophysics, 3, 293-305, 1990) for details.
 *
 * Authors: Walter H. F. Smith and Paul Wessel
 * Date: April, 1988.
 *
 *--------------------------------------------------------------------*
 *
 * Particulars regarding turning the program "surface" version 4.3
 * (revision of 26 February, 1992) into a function "surfunc" follow:
 *
 * Author:	D. W. Caress
 * Date:	May 2, 1994
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.2  1994/10/21  13:02:31  caress
 * Release V4.0
 *
 * Revision 4.1  1994/06/04  02:02:01  caress
 * Fixed several bugs and made some stylistic changes to
 * the output.  Changed the data input bounds to be much
 * larger than the working grid bounds.
 *
 * Revision 4.0  1994/05/05  20:30:06  caress
 * First cut. This code derived from GMT program surface by
 * Walter Smith and Paul Wessel.
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_define.h"

#define GMT_CHUNK	2000
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#define CNULL		((char *)NULL)

#define OUTSIDE 2000000000	/* Index number indicating data is outside useable area */
 
int n_alloc = GMT_CHUNK;
int npoints=0;			/* Number of data points */
int nx=0;			/* Number of nodes in x-dir. */
int ny=0;			/* Number of nodes in y-dir. (Final grid) */
int	mx = 0;
int	my = 0;
int	ij_sw_corner, ij_se_corner, ij_nw_corner, ij_ne_corner;
int block_nx;			/* Number of nodes in x-dir for a given grid factor */
int block_ny;			/* Number of nodes in y-dir for a given grid factor */
int max_iterations=250;		/* Max iter per call to iterate */
int total_iterations = 0;
int grid, old_grid;		/* Node spacings  */
int	grid_east;
int n_fact = 0;			/* Number of factors in common (ny-1, nx-1) */
int factors[32];		/* Array of common factors */
int local_verbose = FALSE;
int error = MB_ERROR_NO_ERROR;
int status = MB_SUCCESS;
int n_empty;			/* No of unconstrained nodes at initialization  */
int set_low = 0;		/* 0 unconstrained,1 = by min data value, 2 = by user value */
int set_high = 0;		/* 0 unconstrained,1 = by max data value, 2 = by user value */
int constrained = FALSE;	/* TRUE if set_low or set_high is TRUE */
double low_limit, high_limit;	/* Constrains on range of solution */
double xmin, xmax, ymin, ymax;	/* minmax coordinates */
float *lower, *upper;		/* arrays for minmax values, if set */
double xinc, yinc;		/* Size of each grid cell (final size) */
double grid_xinc, grid_yinc;	/* size of each grid cell for a given grid factor */
double r_xinc, r_yinc, r_grid_xinc, r_grid_yinc;	/* Reciprocals  */
double converge_limit = 0.0;	/* Convergence limit */
double radius = 0.0;			/* Search radius for initializing grid  */
double	tension = 0.00;		/* Tension parameter on the surface  */
double	boundary_tension = 0.0;
double	interior_tension = 0.0;
double	a0_const_1, a0_const_2;	/* Constants for off grid point equation  */
double	e_2, e_m2, one_plus_e2;
double eps_p2, eps_m2, two_plus_ep2, two_plus_em2;
double	x_edge_const, y_edge_const;
double	epsilon = 1.0;
double	z_mean;
double	z_scale = 1.0;		/* Root mean square range of z after removing planar trend  */
double	r_z_scale = 1.0;	/* reciprocal of z_scale  */
double	plane_c0, plane_c1, plane_c2;	/* Coefficients of best fitting plane to data  */
double small;			/* Let data point coincide with node if distance < small */
float *u;			/* Pointer to grid array */
float *v2;			/* Pointer to v.2.0 grid array */
char *iu;			/* Pointer to grid info array */
char mode_type[2] = {'I','D'};	/* D means include data points when iterating
				 * I means just interpolate from larger grid */

int	offset[25][12];		/* Indices of 12 nearby points in 25 cases of edge conditions  */
double		coeff[2][12];	/* Coefficients for 12 nearby points, constrained and unconstrained  */

double relax_old, relax_new = 1.4;	/* Coefficients for relaxation factor to speed up convergence */

int compare_points();

struct DATA {
	float x;
	float y;
	float z;
	int index;
} *data;		/* Data point and index to node it currently constrains  */

struct BRIGGS {
	double b[6];
} *briggs;		/* Coefficients in Taylor series for Laplacian(z) a la I. C. Briggs (1974)  */

int mb_surface(verbose,ndat,xdat,ydat,zdat,
		xxmin,xxmax,yymin,yymax,xxinc,yyinc,
		ttension,sgrid)
int	verbose;
int	ndat;
float	*xdat;
float	*ydat;
float	*zdat;
double	xxmin;
double	xxmax;
double	yymin;
double	yymax;
double	xxinc;
double	yyinc;
double	ttension;
float	*sgrid;

{

	int	i, j, ij, error = FALSE, size_query = FALSE;
	char	modifier, low[100], high[100];
	char	*function_name = "mb_surface";

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       xxmin:      %f\n",xxmin);
		fprintf(stderr,"dbg2       xxmax:      %f\n",xxmax);
		fprintf(stderr,"dbg2       yymin:      %f\n",yymin);
		fprintf(stderr,"dbg2       yymax:      %f\n",yymax);
		fprintf(stderr,"dbg2       xxinc:      %f\n",xxinc);
		fprintf(stderr,"dbg2       xxinc:      %f\n",xxinc);
		fprintf(stderr,"dbg2       ttension:   %f\n",ttension);
		fprintf(stderr,"dbg2       ndat:       %d\n",ndat);
		for (i=0;i<ndat;i++)
		fprintf(stderr,"dbg2       data:       %f %f %f\n",xdat[i],ydat[i],zdat[i]);
		}

	/* copy parameters */
	xmin = xxmin;
	xmax = xxmax;
	ymin = yymin;
	ymax = yymax;
	xinc = xxinc;
	yinc = yyinc;
	tension = ttension;
	total_iterations = 0;

	/* set local verbose */
	if (verbose > 0)
		local_verbose = TRUE;
	else
		local_verbose = FALSE;

	/* New in v4.3:  Default to unconstrained:  */
	set_low = set_high = 0; 
	
	if (xmin >= xmax || ymin >= ymax) error = TRUE;
	if (xinc <= 0.0 || yinc <= 0.0) error = TRUE;

	if (tension != 0.0) {
		boundary_tension = tension;
		interior_tension = tension;
	}
	relax_old = 1.0 - relax_new;

	nx = rint((xmax - xmin)/xinc) + 1;
	ny = rint((ymax - ymin)/yinc) + 1;
	mx = nx + 4;
	my = ny + 4;
	r_xinc = 1.0 / xinc;
	r_yinc = 1.0 / yinc;

	/* New stuff here for v4.3:  Check out the grid dimensions:  */
	grid = gcd_euclid(nx-1, ny-1);

	/*
	if (local_verbose || size_query || grid == 1) fprintf (stderr, "W: %.3lf E: %.3lf S: %.3lf N: %.3lf nx: %d ny: %d\n",
		xmin, xmax, ymin, ymax, nx, ny);
	if (grid == 1) fprintf(stderr,"surface:  WARNING:  Your grid dimensions are mutually prime.\n");
	if (grid == 1 || size_query) suggest_sizes_for_surface(nx-1, ny-1);
	if (size_query) exit(0);
	*/

	/* New idea: set grid = 1, read data, setting index.  Then throw
		away data that can't be used in end game, constraining
		size of briggs->b[6] structure.  */
	
	grid = 1;
	set_grid_parameters();
	read_data(ndat,xdat,ydat,zdat);
	throw_away_unusables();
	remove_planar_trend();
	rescale_z_values();
	load_constraints(low, high);
	
	/* Set up factors and reset grid to first value  */
	
	grid = gcd_euclid(nx-1, ny-1);
	n_fact = get_prime_factors(grid, factors);
	set_grid_parameters();
	while ( block_nx < 4 || block_ny < 4 ) {
		smart_divide();
		set_grid_parameters();
	}
	set_offset();
	set_index();
	/* Now the data are ready to go for the first iteration.  */

	/* Allocate more space  */
	
	status = mb_malloc(local_verbose, npoints * sizeof(struct BRIGGS), &briggs, &error);
	status = mb_malloc(local_verbose, mx * my * sizeof(char), &iu, &error);
	status = mb_malloc(local_verbose, mx * my * sizeof(float), &u, &error);

	if (radius > 0) initialize_grid(); /* Fill in nodes with a weighted avg in a search radius  */

	/*
	if (local_verbose) fprintf(stderr,"Grid\tMode\tIteration\tMax Change\tConv Limit\tTotal Iterations\n");
	*/
	
	set_coefficients();
	
	old_grid = grid;
	find_nearest_point ();
	iterate (1);
	 
	while (grid > 1) {
		smart_divide ();
		set_grid_parameters();
		set_offset();
		set_index ();
		fill_in_forecast ();
		iterate(0);
		old_grid = grid;
		find_nearest_point ();
		iterate (1);
	}
	
	if (local_verbose) check_errors ();

	replace_planar_trend();

	get_output(sgrid);

	status = mb_free(verbose, &data, &error);
	status = mb_free(verbose, &briggs, &error);
	status = mb_free(verbose, &iu, &error);
	status = mb_free(verbose, &u, &error);
	if (set_low) status = mb_free(verbose, &lower, &error);
	if (set_high) status = mb_free(verbose, &upper, &error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",error);
		for (i=0;i<mx*my;i++)
			fprintf(stderr,"dbg2       grid:       %d %f\n",i,sgrid[i]);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}

int	set_coefficients()
{
	double	e_4, loose, a0;
	
	loose = 1.0 - interior_tension;
	e_2 = epsilon * epsilon;
	e_4 = e_2 * e_2;
	eps_p2 = e_2;
	eps_m2 = 1.0/e_2;
	one_plus_e2 = 1.0 + e_2;
	two_plus_ep2 = 2.0 + 2.0*eps_p2;
	two_plus_em2 = 2.0 + 2.0*eps_m2;
	
	x_edge_const = 4 * one_plus_e2 - 2 * (interior_tension / loose);
	e_m2 = 1.0 / e_2;
	y_edge_const = 4 * (1.0 + e_m2) - 2 * (interior_tension * e_m2 / loose);

	
	a0 = 1.0 / ( (6 * e_4 * loose + 10 * e_2 * loose + 8 * loose - 2 * one_plus_e2) + 4*interior_tension*one_plus_e2);
	a0_const_1 = 2 * loose * (1.0 + e_4);
	a0_const_2 = 2.0 - interior_tension + 2 * loose * e_2;
	
	coeff[1][4] = coeff[1][7] = -loose;
	coeff[1][0] = coeff[1][11] = -loose * e_4;
	coeff[0][4] = coeff[0][7] = -loose * a0;
	coeff[0][0] = coeff[0][11] = -loose * e_4 * a0;
	coeff[1][5] = coeff[1][6] = 2 * loose * one_plus_e2;
	coeff[0][5] = coeff[0][6] = (2 * coeff[1][5] + interior_tension) * a0;
	coeff[1][2] = coeff[1][9] = coeff[1][5] * e_2;
	coeff[0][2] = coeff[0][9] = coeff[0][5] * e_2;
	coeff[1][1] = coeff[1][3] = coeff[1][8] = coeff[1][10] = -2 * loose * e_2;
	coeff[0][1] = coeff[0][3] = coeff[0][8] = coeff[0][10] = coeff[1][1] * a0;
	
	e_2 *= 2;		/* We will need these in boundary conditions  */
	e_m2 *= 2;
	
	ij_sw_corner = 2 * my + 2;			/*  Corners of array of actual data  */
	ij_se_corner = ij_sw_corner + (nx - 1) * my;
	ij_nw_corner = ij_sw_corner + (ny - 1);
	ij_ne_corner = ij_se_corner + (ny - 1);

}

int	set_offset()
{
	int	add_w[5], add_e[5], add_s[5], add_n[5], add_w2[5], add_e2[5], add_s2[5], add_n2[5];
	int	i, j, kase;
	
	add_w[0] = -my; add_w[1] = add_w[2] = add_w[3] = add_w[4] = -grid_east;
	add_w2[0] = -2 * my;  add_w2[1] = -my - grid_east;  add_w2[2] = add_w2[3] = add_w2[4] = -2 * grid_east;
	add_e[4] = my; add_e[0] = add_e[1] = add_e[2] = add_e[3] = grid_east;
	add_e2[4] = 2 * my;  add_e2[3] = my + grid_east;  add_e2[2] = add_e2[1] = add_e2[0] = 2 * grid_east;

	add_n[4] = 1; add_n[3] = add_n[2] = add_n[1] = add_n[0] = grid;
	add_n2[4] = 2;  add_n2[3] = grid + 1;  add_n2[2] = add_n2[1] = add_n2[0] = 2 * grid;
	add_s[0] = -1; add_s[1] = add_s[2] = add_s[3] = add_s[4] = -grid;
	add_s2[0] = -2;  add_s2[1] = -grid - 1;  add_s2[2] = add_s2[3] = add_s2[4] = -2 * grid;

	for (i = 0, kase = 0; i < 5; i++) {
		for (j = 0; j < 5; j++, kase++) {
			offset[kase][0] = add_n2[j];
			offset[kase][1] = add_n[j] + add_w[i];
			offset[kase][2] = add_n[j];
			offset[kase][3] = add_n[j] + add_e[i];
			offset[kase][4] = add_w2[i];
			offset[kase][5] = add_w[i];
			offset[kase][6] = add_e[i];
			offset[kase][7] = add_e2[i];
			offset[kase][8] = add_s[j] + add_w[i];
			offset[kase][9] = add_s[j];
			offset[kase][10] = add_s[j] + add_e[i];
			offset[kase][11] = add_s2[j];
		}
	}
}



int fill_in_forecast () {

	/* Fills in bilinear estimates into new node locations
	   after grid is divided.   
	 */

	int i, j, ii, jj, index_0, index_1, index_2, index_3;
	int index_new;
	double delta_x, delta_y, a0, a1, a2, a3;
	double old_size;
	
		
	old_size = 1.0 / (double)old_grid;

	/* first do from southwest corner */
	
	for (i = 0; i < nx-1; i += old_grid) {
		
		for (j = 0; j < ny-1; j += old_grid) {
			
			/* get indices of bilinear square */
			index_0 = ij_sw_corner + i * my + j;
			index_1 = index_0 + old_grid * my;
			index_2 = index_1 + old_grid;
			index_3 = index_0 + old_grid;
			
			/* get coefficients */
			a0 = u[index_0];
			a1 = u[index_1] - a0;
			a2 = u[index_3] - a0;
			a3 = u[index_2] - a0 - a1 - a2;
			
			/* find all possible new fill ins */
			
			for (ii = i;  ii < i + old_grid; ii += grid) {
				delta_x = (ii - i) * old_size;
				for (jj = j;  jj < j + old_grid; jj += grid) {
					index_new = ij_sw_corner + ii * my + jj;
					if (index_new == index_0) continue;
					delta_y = (jj - j) * old_size;
					u[index_new] = a0 + a1 * delta_x + delta_y * ( a2 + a3 * delta_x);	
					iu[index_new] = 0;
				}
			}
			iu[index_0] = 5;
		}
	}
	
	/* now do linear guess along east edge */
	
	for (j = 0; j < (ny-1); j += old_grid) {
		index_0 = ij_se_corner + j;
		index_3 = index_0 + old_grid;
		for (jj = j;  jj < j + old_grid; jj += grid) {
			index_new = ij_se_corner + jj;
			delta_y = (jj - j) * old_size;
			u[index_new] = u[index_0] + delta_y * (u[index_3] - u[index_0]);
			iu[index_new] = 0;
		}
		iu[index_0] = 5;
	}
	/* now do linear guess along north edge */
	for (i = 0; i < (nx-1); i += old_grid) {
		index_0 = ij_nw_corner + i * my;
		index_1 = index_0 + old_grid * my;
		for (ii = i;  ii < i + old_grid; ii += grid) {
			index_new = ij_nw_corner + ii * my;
			delta_x = (ii - i) * old_size;
			u[index_new] = u[index_0] + delta_x * (u[index_1] - u[index_0]);
			iu[index_new] = 0;
		}
		iu[index_0] = 5;
	}
	/* now set northeast corner to fixed and we're done */
	iu[ij_ne_corner] = 5;
}

int compare_points (point_1, point_2)
struct DATA *point_1, *point_2; {
		/*  Routine for qsort to sort data structure for fast access to data by node location.
		    Sorts on index first, then on radius to node corresponding to index, so that index
		    goes from low to high, and so does radius.
		*/
	int block_i, block_j, index_1, index_2;
	double x0, y0, dist_1, dist_2;
	
	index_1 = point_1->index;
	index_2 = point_2->index;
	if (index_1 < index_2)
		return (-1);
	else if (index_1 > index_2)
		return (1);
	else if (index_1 == OUTSIDE)
		return (0);
	else {	/* Points are in same grid cell, find the one who is nearest to grid point */
		block_i = point_1->index/block_ny;
		block_j = point_1->index%block_ny;
		x0 = xmin + block_i * grid_xinc;
		y0 = ymin + block_j * grid_yinc;
		dist_1 = (point_1->x - x0) * (point_1->x - x0) + (point_1->y - y0) * (point_1->y - y0);
		dist_2 = (point_2->x - x0) * (point_2->x - x0) + (point_2->y - y0) * (point_2->y - y0);
		if (dist_1 < dist_2)
			return (-1);
		if (dist_1 > dist_2)
			return (1);
		else
			return (0);
	}
}

int smart_divide () {
		/* Divide grid by its largest prime factor */
	grid /= factors[n_fact - 1];
	n_fact--;
}

int set_index () {
		/* recomputes data[k].index for new value of grid,
		   sorts data on index and radii, and throws away
		   data which are now outside the useable limits. */
	int i, j, k, k_skipped = 0;

	for (k = 0; k < npoints; k++) {
		i = floor(((data[k].x-xmin)*r_grid_xinc) + 0.5);
		j = floor(((data[k].y-ymin)*r_grid_yinc) + 0.5);
		if (i < 0 || i >= block_nx || j < 0 || j >= block_ny) {
			data[k].index = OUTSIDE;
			k_skipped++;
		}
		else
			data[k].index = i * block_ny + j;
	}
	
	qsort ((char *)data, npoints, sizeof (struct DATA), compare_points);
	
	npoints -= k_skipped;
	
}

int find_nearest_point() {
	int i, j, k, last_index, block_i, block_j, iu_index, briggs_index;
	double x0, y0, dx, dy, xys, xy1, btemp;
	double b0, b1, b2, b3, b4, b5;
	
	last_index = -1;
	small = 0.05 * ((grid_xinc < grid_yinc) ? grid_xinc : grid_yinc);

	for (i = 0; i < nx; i += grid)	/* Reset grid info */
		for (j = 0; j < ny; j += grid)
			iu[ij_sw_corner + i*my + j] = 0;
	
	briggs_index = 0;
	for (k = 0; k < npoints; k++) {	/* Find constraining value  */
		if (data[k].index != last_index) {
			block_i = data[k].index/block_ny;
			block_j = data[k].index%block_ny;
			last_index = data[k].index;
	 		iu_index = ij_sw_corner + (block_i * my + block_j) * grid;
	 		x0 = xmin + block_i*grid_xinc;
	 		y0 = ymin + block_j*grid_yinc;
	 		dx = (data[k].x - x0)*r_grid_xinc;
	 		dy = (data[k].y - y0)*r_grid_yinc;
	 		if (fabs(dx) < small && fabs(dy) < small) {
	 			iu[iu_index] = 5;
	 			u[iu_index] = data[k].z;
	 		}
	 		else {
	 			if (dx >= 0.0) {
	 				if (dy >= 0.0)
	 					iu[iu_index] = 1;
	 				else
	 					iu[iu_index] = 4;
	 			}
	 			else {
	 				if (dy >= 0.0)
	 					iu[iu_index] = 2;
	 				else
	 					iu[iu_index] = 3;
	 			}
	 			dx = fabs(dx);
	 			dy = fabs(dy);
	 			btemp = 2 * one_plus_e2 / ( (dx + dy) * (1.0 + dx + dy) );
	 			b0 = 1.0 - 0.5 * (dx + (dx * dx)) * btemp;
	 			b3 = 0.5 * (e_2 - (dy + (dy * dy)) * btemp);
	 			xys = 1.0 + dx + dy;
	 			xy1 = 1.0 / xys;
	 			b1 = (e_2 * xys - 4 * dy) * xy1;
	 			b2 = 2 * (dy - dx + 1.0) * xy1;
	 			b4 = b0 + b1 + b2 + b3 + btemp;
	 			b5 = btemp * data[k].z;
	 			briggs[briggs_index].b[0] = b0;
	 			briggs[briggs_index].b[1] = b1;
	 			briggs[briggs_index].b[2] = b2;
	 			briggs[briggs_index].b[3] = b3;
	 			briggs[briggs_index].b[4] = b4;
	 			briggs[briggs_index].b[5] = b5;
	 			briggs_index++;
	 		}
	 	}
	 }
}

						
int set_grid_parameters()
{			
	block_ny = (ny - 1) / grid + 1;
	block_nx = (nx - 1) / grid + 1;
	grid_xinc = grid * xinc;
	grid_yinc = grid * yinc;
	grid_east = grid * my;
	r_grid_xinc = 1.0 / grid_xinc;
	r_grid_yinc = 1.0 / grid_yinc;
}

int initialize_grid()
{	/*
	 * For the initial gridsize, compute weighted averages of data inside the search radius
	 * and assign the values to u[i,j] where i,j are multiples of gridsize.
	 */
	 int	irad, jrad, i, j, imin, imax, jmin, jmax, index_1, index_2, k, ki, kj, k_index;
	 double	r, rfact, sum_w, sum_zw, weight, x0, y0;

	 irad = ceil(radius/grid_xinc);
	 jrad = ceil(radius/grid_yinc);
	 rfact = -4.5/(radius*radius);
	 
	 for (i = 0; i < block_nx; i ++ ) {
	 	x0 = xmin + i*grid_xinc;
	 	for (j = 0; j < block_ny; j ++ ) {
	 		y0 = ymin + j*grid_yinc;
	 		imin = i - irad;
	 		if (imin < 0) imin = 0;
	 		imax = i + irad;
	 		if (imax >= block_nx) imax = block_nx - 1;
	 		jmin = j - jrad;
	 		if (jmin < 0) jmin = 0;
	 		jmax = j + jrad;
	 		if (jmax >= block_ny) jmax = block_ny - 1;
	 		index_1 = imin*block_ny + jmin;
	 		index_2 = imax*block_ny + jmax + 1;
	 		sum_w = sum_zw = 0.0;
	 		k = 0;
	 		while (k < npoints && data[k].index < index_1) k++;
	 		for (ki = imin; k < npoints && ki <= imax && data[k].index < index_2; ki++) {
	 			for (kj = jmin; k < npoints && kj <= jmax && data[k].index < index_2; kj++) {
	 				k_index = ki*block_ny + kj;
	 				while (k < npoints && data[k].index < k_index) k++;
	 				while (k < npoints && data[k].index == k_index) {
	 					r = (data[k].x-x0)*(data[k].x-x0) + (data[k].y-y0)*(data[k].y-y0);
	 					weight = exp (rfact*r);
	 					sum_w += weight;
	 					sum_zw += weight*data[k].z;
	 					k++;
	 				}
	 			}
	 		}
	 		if (sum_w == 0.0) {
				/*
	 			fprintf (stderr, "surface: Warning: no data inside search radius at: %.8lg %.8lg\n", x0, y0);
				*/
	 			u[ij_sw_corner + (i * my + j) * grid] = z_mean;
	 		}
	 		else {
	 			u[ij_sw_corner + (i*my+j)*grid] = sum_zw/sum_w;
	 		}
		}
	}
}


int new_initialize_grid()
{	/*
	 * For the initial gridsize, load constrained nodes with weighted avg of their data;
	 * and then do something with the unconstrained ones.
	 */
	 int	k, k_index, u_index, block_i, block_j;
	 double	sum_w, sum_zw, weight, x0, y0, dx, dy, dx_scale, dy_scale;

	dx_scale = 4.0 / grid_xinc;
	dy_scale = 4.0 / grid_yinc;
	n_empty = block_ny * block_nx;
	k = 0;
	while (k < npoints) {
		block_i = data[k].index / block_ny;
		block_j = data[k].index % block_ny;
		x0 = xmin + block_i*grid_xinc;
		y0 = ymin + block_j*grid_yinc;
		u_index = ij_sw_corner + (block_i*my + block_j) * grid;
		k_index = data[k].index;
		
		dy = (data[k].y - y0) * dy_scale;
		dx = (data[k].x - x0) * dx_scale;
		sum_w = 1.0 / (1.0 + dx*dx + dy*dy);
		sum_zw = data[k].z * sum_w;
		k++;

		while (k < npoints && data[k].index == k_index) {
			
			dy = (data[k].y - y0) * dy_scale;
			dx = (data[k].x - x0) * dx_scale;
			weight = 1.0 / (1.0 + dx*dx + dy*dy);
			sum_zw += data[k].z * weight;
			sum_w += weight;
			sum_zw += weight*data[k].z;
			k++;
	 	}
	 	u[u_index] = sum_zw/sum_w;
	 	iu[u_index] = 5;
	 	n_empty--;
	 }
}

/* This function rewritten by D.W. Caress 5/3/94 */
int read_data(ndat,xdat,ydat,zdat)
int	ndat;
float	*xdat;
float	*ydat;
float	*zdat;
{
	int	i, j, k, kmax, kmin, idat;
	double	zmin = 1.0e38, zmax = -1.0e38;

	status = mb_malloc(local_verbose, ndat * sizeof(struct DATA), &data, &error);
	
	/* Read in xyz data and computes index no and store it in a structure */
	k = 0;
	z_mean = 0;
	for (idat=0;idat<ndat;idat++)
		{
		i = floor(((xdat[idat]-xmin)*r_grid_xinc) + 0.5);
		j = floor(((ydat[idat]-ymin)*r_grid_yinc) + 0.5);
		if (i >= 0 && i < block_nx && j >= 0 && j < block_ny)
			{
			data[k].index = i * block_ny + j;
			data[k].x = xdat[idat];
			data[k].y = ydat[idat];
			data[k].z = zdat[idat];
			if (zmin > zdat[idat]) 
				{
				zmin = zdat[idat];
				kmin = k;
				}
			if (zmax < zdat[idat]) 
				{
				zmax = zdat[idat];
				kmax = k;
				}
			k++;
			z_mean += zdat[idat];
			}
		}

	npoints = k;
	z_mean /= k;
	if( converge_limit == 0.0 ) {
		converge_limit = 0.001 * z_scale; /* c_l = 1 ppt of L2 scale */
	}
	/*
	if (local_verbose) {
		fprintf(stderr, "surface: Minimum value of your dataset x,y,z at: %g %g %g\n",
			data[kmin].x, data[kmin].y, data[kmin].z);
		fprintf(stderr, "surface: Maximum value of your dataset x,y,z at: %g %g %g\n",
			data[kmax].x, data[kmax].y, data[kmax].z);
	}
	*/
	
	if (set_low == 1)
		low_limit = data[kmin].z;
	else if (set_low == 2 && low_limit > data[kmin].z) {
	/*	low_limit = data[kmin].z;	*/
		/*
		fprintf (stderr, "surface: Warning:  Your lower value is > than min data value.\n");
		*/
	}
	if (set_high == 1)
		high_limit = data[kmax].z;
	else if (set_high == 2 && high_limit < data[kmax].z) {
	/*	high_limit = data[kmax].z;	*/
		/*
		fprintf (stderr, "surface: Warning:  Your upper value is < than max data value.\n");
		*/
	}

}

/* this function rewritten from write_output() by D.W. Caress 5/3/94 */
int get_output(sgrid)
float	*sgrid;
{
	int	index, i, j, ij;
	

	index = ij_sw_corner;
	for(i = 0; i < nx; i++, index += my) 
		for (j = 0; j < ny; j++) 
			{
			sgrid[j*nx+i] = u[index + ny - j - 1];
			}
}
	
int	iterate(mode)
	int	mode;
{

	int	i, j, k, ij, kase, briggs_index, ij_v2;
	int	x_case, y_case, x_w_case, x_e_case, y_s_case, y_n_case;
	int	iteration_count = 0;
	
	double	current_limit = converge_limit / grid;
	double	change, max_change = 0.0, busum, sum_ij;
	double	b0, b1, b2, b3, b4, b5;
	
	double	x_0_const = 4.0 * (1.0 - boundary_tension) / (2.0 - boundary_tension);
	double	x_1_const = (3 * boundary_tension - 2.0) / (2.0 - boundary_tension);
	double	y_denom = 2 * epsilon * (1.0 - boundary_tension) + boundary_tension;
	double	y_0_const = 4 * epsilon * (1.0 - boundary_tension) / y_denom;
	double	y_1_const = (boundary_tension - 2 * epsilon * (1.0 - boundary_tension) ) / y_denom;

	do {
		briggs_index = 0;	/* Reset the constraint table stack pointer  */
		
		max_change = -1.0;
		
		/* Fill in auxiliary boundary values (in new way) */
		
		/* First set d2[]/dn2 = 0 along edges:  */
		/* New experiment : (1-T)d2[]/dn2 + Td[]/dn = 0  */
		
		
		
		for (i = 0; i < nx; i += grid) {
			/* set d2[]/dy2 = 0 on south side:  */
			ij = ij_sw_corner + i * my;
			/* u[ij - 1] = 2 * u[ij] - u[ij + grid];  */
			u[ij - 1] = y_0_const * u[ij] + y_1_const * u[ij + grid];
			/* set d2[]/dy2 = 0 on north side:  */
			ij = ij_nw_corner + i * my;
			/* u[ij + 1] = 2 * u[ij] - u[ij - grid];  */
			u[ij + 1] = y_0_const * u[ij] + y_1_const * u[ij - grid];
			
		}
		
		for (j = 0; j < ny; j += grid) {
			/* set d2[]/dx2 = 0 on west side:  */
			ij = ij_sw_corner + j;
			/* u[ij - my] = 2 * u[ij] - u[ij + grid_east];  */
			u[ij - my] = x_1_const * u[ij + grid_east] + x_0_const * u[ij];
			/* set d2[]/dx2 = 0 on east side:  */
			ij = ij_se_corner + j;
			/* u[ij + my] = 2 * u[ij] - u[ij - grid_east];  */
			u[ij + my] = x_1_const * u[ij - grid_east] + x_0_const * u[ij];
		}
			
		/* Now set d2[]/dxdy = 0 at each corner:  */
		
		ij = ij_sw_corner;
		u[ij - my - 1] = u[ij + grid_east - 1] + u[ij - my + grid] - u[ij + grid_east + grid];
				
		ij = ij_nw_corner;
		u[ij - my + 1] = u[ij + grid_east + 1] + u[ij - my - grid] - u[ij + grid_east - grid];
				
		ij = ij_se_corner;
		u[ij + my - 1] = u[ij - grid_east - 1] + u[ij + my + grid] - u[ij - grid_east + grid];
				
		ij = ij_ne_corner;
		u[ij + my + 1] = u[ij - grid_east + 1] + u[ij + my - grid] - u[ij - grid_east - grid];
		
		/* Now set (1-T)dC/dn + Tdu/dn = 0 at each edge :  */
		/* New experiment:  only dC/dn = 0  */
		
		x_w_case = 0;
		x_e_case = block_nx - 1;
		for (i = 0; i < nx; i += grid, x_w_case++, x_e_case--) {
		
			if(x_w_case < 2)
				x_case = x_w_case;
			else if(x_e_case < 2)
				x_case = 4 - x_e_case;
			else
				x_case = 2;
				
			/* South side :  */
			kase = x_case * 5;
			ij = ij_sw_corner + i * my;
			u[ij + offset[kase][11]] = 
				(u[ij + offset[kase][0]] + eps_m2*(u[ij + offset[kase][1]] + u[ij + offset[kase][3]]
					- u[ij + offset[kase][8]] - u[ij + offset[kase][10]])
					+ two_plus_em2 * (u[ij + offset[kase][9]] - u[ij + offset[kase][2]]) );
				/*  + tense * eps_m2 * (u[ij + offset[kase][2]] - u[ij + offset[kase][9]]) / (1.0 - tense);  */
			/* North side :  */
			kase = x_case * 5 + 4;
			ij = ij_nw_corner + i * my;
			u[ij + offset[kase][0]] = 
				-(-u[ij + offset[kase][11]] + eps_m2 * (u[ij + offset[kase][1]] + u[ij + offset[kase][3]]
					- u[ij + offset[kase][8]] - u[ij + offset[kase][10]])
					+ two_plus_em2 * (u[ij + offset[kase][9]] - u[ij + offset[kase][2]]) );
				/*  - tense * eps_m2 * (u[ij + offset[kase][2]] - u[ij + offset[kase][9]]) / (1.0 - tense);  */
		}
		
		y_s_case = 0;
		y_n_case = block_ny - 1;
		for (j = 0; j < ny; j += grid, y_s_case++, y_n_case--) {
				
			if(y_s_case < 2)
				y_case = y_s_case;
			else if(y_n_case < 2)
				y_case = 4 - y_n_case;
			else
				y_case = 2;
			
			/* West side :  */
			kase = y_case;
			ij = ij_sw_corner + j;
			u[ij+offset[kase][4]] = 
				u[ij + offset[kase][7]] + eps_p2 * (u[ij + offset[kase][3]] + u[ij + offset[kase][10]]
				-u[ij + offset[kase][1]] - u[ij + offset[kase][8]])
				+ two_plus_ep2 * (u[ij + offset[kase][5]] - u[ij + offset[kase][6]]);
				/*  + tense * (u[ij + offset[kase][6]] - u[ij + offset[kase][5]]) / (1.0 - tense);  */
			/* East side :  */
			kase = 20 + y_case;
			ij = ij_se_corner + j;
			u[ij + offset[kase][7]] = 
				- (-u[ij + offset[kase][4]] + eps_p2 * (u[ij + offset[kase][3]] + u[ij + offset[kase][10]]
				- u[ij + offset[kase][1]] - u[ij + offset[kase][8]])
				+ two_plus_ep2 * (u[ij + offset[kase][5]] - u[ij + offset[kase][6]]) );
				/*  - tense * (u[ij + offset[kase][6]] - u[ij + offset[kase][5]]) / (1.0 - tense);  */
		}

			
			
		/* That's it for the boundary points.  Now loop over all data  */
		
		x_w_case = 0;
		x_e_case = block_nx - 1;
		for (i = 0; i < nx; i += grid, x_w_case++, x_e_case--) {
		
			if(x_w_case < 2)
				x_case = x_w_case;
			else if(x_e_case < 2)
				x_case = 4 - x_e_case;
			else
				x_case = 2;
			
			y_s_case = 0;
			y_n_case = block_ny - 1;
			
			ij = ij_sw_corner + i * my;
			
			for (j = 0; j < ny; j += grid, ij += grid, y_s_case++, y_n_case--) {
	
				if (iu[ij] == 5) continue;	/* Point is fixed  */
				
				if(y_s_case < 2)
					y_case = y_s_case;
				else if(y_n_case < 2)
					y_case = 4 - y_n_case;
				else
					y_case = 2;
				
				kase = x_case * 5 + y_case;
				sum_ij = 0.0;

				if (iu[ij] == 0) {		/* Point is unconstrained  */
					for (k = 0; k < 12; k++) {
						sum_ij += (u[ij + offset[kase][k]] * coeff[0][k]);
					}
				}
				else {				/* Point is constrained  */
				
					b0 = briggs[briggs_index].b[0];
					b1 = briggs[briggs_index].b[1];
					b2 = briggs[briggs_index].b[2];
					b3 = briggs[briggs_index].b[3];
					b4 = briggs[briggs_index].b[4];
					b5 = briggs[briggs_index].b[5];
					briggs_index++;
					if (iu[ij] < 3) {
						if (iu[ij] == 1) {	/* Point is in quadrant 1  */
							busum = b0 * u[ij + offset[kase][10]]
								+ b1 * u[ij + offset[kase][9]]
								+ b2 * u[ij + offset[kase][5]]
								+ b3 * u[ij + offset[kase][1]];
						}
						else {			/* Point is in quadrant 2  */
							busum = b0 * u[ij + offset[kase][8]]
								+ b1 * u[ij + offset[kase][9]]
								+ b2 * u[ij + offset[kase][6]]
								+ b3 * u[ij + offset[kase][3]];
						}
					}
					else {
						if (iu[ij] == 3) {	/* Point is in quadrant 3  */
							busum = b0 * u[ij + offset[kase][1]]
								+ b1 * u[ij + offset[kase][2]]
								+ b2 * u[ij + offset[kase][6]]
								+ b3 * u[ij + offset[kase][10]];
						}
						else {		/* Point is in quadrant 4  */
							busum = b0 * u[ij + offset[kase][3]]
								+ b1 * u[ij + offset[kase][2]]
								+ b2 * u[ij + offset[kase][5]]
								+ b3 * u[ij + offset[kase][8]];
						}
					}
					for (k = 0; k < 12; k++) {
						sum_ij += (u[ij + offset[kase][k]] * coeff[1][k]);
					}
					sum_ij = (sum_ij + a0_const_2 * (busum + b5))
						/ (a0_const_1 + a0_const_2 * b4);
				}
				
				/* New relaxation here  */
				sum_ij = u[ij] * relax_old + sum_ij * relax_new;
				
				if (constrained) {	/* Must check limits.  Note lower/upper is v2 format and need ij_v2! */
					ij_v2 = (ny - j - 1) * nx + i;
					if (set_low /*&& !GMT_is_fnan((double)lower[ij_v2])*/ && sum_ij < lower[ij_v2])
						sum_ij = lower[ij_v2];
					else if (set_high /*&& !GMT_is_fnan((double)upper[ij_v2])*/ && sum_ij > upper[ij_v2])
						sum_ij = upper[ij_v2];
				}
					
				change = fabs(sum_ij - u[ij]);
				u[ij] = sum_ij;
				if (change > max_change) max_change = change;
			}
		}
		iteration_count++;
		total_iterations++;
		max_change *= z_scale;	/* Put max_change into z units  */
		if (local_verbose > 1) 
			fprintf(stderr,"%4d\t%c\t%8d\t%10lg\t%10lg\t%10d\n",
				grid, mode_type[mode], 
				iteration_count, max_change, 
				current_limit, total_iterations);

	} while (max_change > current_limit && iteration_count < max_iterations);
	
	if (local_verbose) fprintf(stderr,"%4d\t%c\t%8d\t%10lg\t%10lg\t%10d\n",
		grid, mode_type[mode], iteration_count, max_change, current_limit, total_iterations);

	return(iteration_count);
}

int check_errors () {

	int	i, j, k, ij, n_nodes, move_over[12];	/* move_over = offset[kase][12], but grid = 1 so move_over is easy  */
	
	double	x0, y0, dx, dy, mean_error, mean_squared_error, z_est, z_err, curvature, c;
	double	du_dx, du_dy, d2u_dx2, d2u_dxdy, d2u_dy2, d3u_dx3, d3u_dx2dy, d3u_dxdy2, d3u_dy3;
	
	double	x_0_const = 4.0 * (1.0 - boundary_tension) / (2.0 - boundary_tension);
	double	x_1_const = (3 * boundary_tension - 2.0) / (2.0 - boundary_tension);
	double	y_denom = 2 * epsilon * (1.0 - boundary_tension) + boundary_tension;
	double	y_0_const = 4 * epsilon * (1.0 - boundary_tension) / y_denom;
	double	y_1_const = (boundary_tension - 2 * epsilon * (1.0 - boundary_tension) ) / y_denom;
	
	
	move_over[0] = 2;
	move_over[1] = 1 - my;
	move_over[2] = 1;
	move_over[3] = 1 + my;
	move_over[4] = -2 * my;
	move_over[5] = -my;
	move_over[6] = my;
	move_over[7] = 2 * my;
	move_over[8] = -1 - my;
	move_over[9] = -1;
	move_over[10] = -1 + my;
	move_over[11] = -2;

	mean_error = 0;
	mean_squared_error = 0;
	
	/* First update the boundary values  */

	for (i = 0; i < nx; i ++) {
		ij = ij_sw_corner + i * my;
		u[ij - 1] = y_0_const * u[ij] + y_1_const * u[ij + 1];
		ij = ij_nw_corner + i * my;
		u[ij + 1] = y_0_const * u[ij] + y_1_const * u[ij - 1];
	}

	for (j = 0; j < ny; j ++) {
		ij = ij_sw_corner + j;
		u[ij - my] = x_1_const * u[ij + my] + x_0_const * u[ij];
		ij = ij_se_corner + j;
		u[ij + my] = x_1_const * u[ij - my] + x_0_const * u[ij];
	}

	ij = ij_sw_corner;
	u[ij - my - 1] = u[ij + my - 1] + u[ij - my + 1] - u[ij + my + 1];
	ij = ij_nw_corner;
	u[ij - my + 1] = u[ij + my + 1] + u[ij - my - 1] - u[ij + my - 1];
	ij = ij_se_corner;
	u[ij + my - 1] = u[ij - my - 1] + u[ij + my + 1] - u[ij - my + 1];
	ij = ij_ne_corner;
	u[ij + my + 1] = u[ij - my + 1] + u[ij + my - 1] - u[ij - my - 1];

	for (i = 0; i < nx; i ++) {
				
		ij = ij_sw_corner + i * my;
		u[ij + move_over[11]] = 
			(u[ij + move_over[0]] + eps_m2*(u[ij + move_over[1]] + u[ij + move_over[3]]
				- u[ij + move_over[8]] - u[ij + move_over[10]])
				+ two_plus_em2 * (u[ij + move_over[9]] - u[ij + move_over[2]]) );
					
		ij = ij_nw_corner + i * my;
		u[ij + move_over[0]] = 
			-(-u[ij + move_over[11]] + eps_m2 * (u[ij + move_over[1]] + u[ij + move_over[3]]
				- u[ij + move_over[8]] - u[ij + move_over[10]])
				+ two_plus_em2 * (u[ij + move_over[9]] - u[ij + move_over[2]]) );
	}
		
	for (j = 0; j < ny; j ++) {
			
		ij = ij_sw_corner + j;
		u[ij+move_over[4]] = 
			u[ij + move_over[7]] + eps_p2 * (u[ij + move_over[3]] + u[ij + move_over[10]]
			-u[ij + move_over[1]] - u[ij + move_over[8]])
			+ two_plus_ep2 * (u[ij + move_over[5]] - u[ij + move_over[6]]);
				
		ij = ij_se_corner + j;
		u[ij + move_over[7]] = 
			- (-u[ij + move_over[4]] + eps_p2 * (u[ij + move_over[3]] + u[ij + move_over[10]]
			- u[ij + move_over[1]] - u[ij + move_over[8]])
			+ two_plus_ep2 * (u[ij + move_over[5]] - u[ij + move_over[6]]) );
	}

	/* That resets the boundary values.  Now we can test all data.  
		Note that this loop checks all values, even though only nearest were used.  */
	
	for (k = 0; k < npoints; k++) {
		i = data[k].index/ny;
		j = data[k].index%ny;
	 	ij = ij_sw_corner + i * my + j;
	 	if ( iu[ij] == 5 ) continue;
	 	x0 = xmin + i*xinc;
	 	y0 = ymin + j*yinc;
	 	dx = (data[k].x - x0)*r_xinc;
	 	dy = (data[k].y - y0)*r_yinc;
 
	 	du_dx = 0.5 * (u[ij + move_over[6]] - u[ij + move_over[5]]);
	 	du_dy = 0.5 * (u[ij + move_over[2]] - u[ij + move_over[9]]);
	 	d2u_dx2 = u[ij + move_over[6]] + u[ij + move_over[5]] - 2 * u[ij];
	 	d2u_dy2 = u[ij + move_over[2]] + u[ij + move_over[9]] - 2 * u[ij];
	 	d2u_dxdy = 0.25 * (u[ij + move_over[3]] - u[ij + move_over[1]]
	 			- u[ij + move_over[10]] + u[ij + move_over[8]]);
	 	d3u_dx3 = 0.5 * ( u[ij + move_over[7]] - 2 * u[ij + move_over[6]]
	 				+ 2 * u[ij + move_over[5]] - u[ij + move_over[4]]);
	 	d3u_dy3 = 0.5 * ( u[ij + move_over[0]] - 2 * u[ij + move_over[2]]
	 				+ 2 * u[ij + move_over[9]] - u[ij + move_over[11]]);
	 	d3u_dx2dy = 0.5 * ( ( u[ij + move_over[3]] + u[ij + move_over[1]] - 2 * u[ij + move_over[2]] )
	 				- ( u[ij + move_over[10]] + u[ij + move_over[8]] - 2 * u[ij + move_over[9]] ) );
	 	d3u_dxdy2 = 0.5 * ( ( u[ij + move_over[3]] + u[ij + move_over[10]] - 2 * u[ij + move_over[6]] )
	 				- ( u[ij + move_over[1]] + u[ij + move_over[8]] - 2 * u[ij + move_over[5]] ) );

	 	/* 3rd order Taylor approx:  */
	 		
	 	z_est = u[ij] + dx * (du_dx +  dx * ( (0.5 * d2u_dx2) + dx * (d3u_dx3 / 6.0) ) )
				+ dy * (du_dy +  dy * ( (0.5 * d2u_dy2) + dy * (d3u_dy3 / 6.0) ) )
	 			+ dx * dy * (d2u_dxdy) + (0.5 * dx * d3u_dx2dy) + (0.5 * dy * d3u_dxdy2);
	 		
	 	z_err = z_est - data[k].z;
	 	mean_error += z_err;
	 	mean_squared_error += (z_err * z_err);
	 }
	 mean_error /= npoints;
	 mean_squared_error = sqrt( mean_squared_error / npoints);
	 
	 curvature = 0.0;
	 n_nodes = nx * ny;
	 
	 for (i = 0; i < nx; i++) {
	 	for (j = 0; j < ny; j++) {
	 		ij = ij_sw_corner + i * my + j;
	 		c = u[ij + move_over[6]] + u[ij + move_over[5]]
	 			+ u[ij + move_over[2]] + u[ij + move_over[9]] - 4.0 * u[ij + move_over[6]];
			curvature += (c * c);
		}
	}

	 /*
	 fprintf(stderr, "Fit info: N data points  N nodes\tmean error\trms error\tcurvature\n");
	 fprintf (stderr,"\t%8d\t%8d\t%.8lg\t%.8lg\t%.8lg\n", npoints, n_nodes, mean_error, mean_squared_error,
	 	curvature);
	*/
	if (local_verbose)
		{
		fprintf(stderr,"\nSpline interpolation fit information:\n");
		fprintf(stderr,"Data points   nodes    mean error     rms error     curvature\n");
		fprintf(stderr,"%9d %9d   %10g   %10g  %10g\n",
			npoints, n_nodes, mean_error, mean_squared_error,
	 		curvature);
		}
 }

int	remove_planar_trend()
{

	int	i;
	double	a, b, c, d, xx, yy, zz;
	double	sx, sy, sz, sxx, sxy, sxz, syy, syz;
	
	sx = sy = sz = sxx = sxy = sxz = syy = syz = 0.0;
	
	for (i = 0; i < npoints; i++) {

		xx = (data[i].x - xmin) * r_xinc;
		yy = (data[i].y - ymin) * r_yinc;
		zz = data[i].z;
		
		sx += xx;
		sy += yy;
		sz += zz;
		sxx +=(xx * xx);
		sxy +=(xx * yy);
		sxz +=(xx * zz);
		syy +=(yy * yy);
		syz +=(yy * zz);
	}
	
	d = npoints*sxx*syy + 2*sx*sy*sxy - npoints*sxy*sxy - sx*sx*syy - sy*sy*sxx;
	
	if (d == 0.0) {
		plane_c0 = plane_c1 = plane_c2 = 0.0;
		return(0);
	}
	
	a = sz*sxx*syy + sx*sxy*syz + sy*sxy*sxz - sz*sxy*sxy - sx*sxz*syy - sy*syz*sxx;
	b = npoints*sxz*syy + sz*sy*sxy + sy*sx*syz - npoints*sxy*syz - sz*sx*syy - sy*sy*sxz;
	c = npoints*sxx*syz + sx*sy*sxz + sz*sx*sxy - npoints*sxy*sxz - sx*sx*syz - sz*sy*sxx;

	plane_c0 = a / d;
	plane_c1 = b / d;
	plane_c2 = c / d;

	for (i = 0; i < npoints; i++) {

		xx = (data[i].x - xmin) * r_xinc;
		yy = (data[i].y - ymin) * r_yinc;
		
		data[i].z -=(plane_c0 + plane_c1 * xx + plane_c2 * yy);
	}

	return(0);
}

int	replace_planar_trend()
{
	int	i, j, ij;

	 for (i = 0; i < nx; i++) {
	 	for (j = 0; j < ny; j++) {
	 		ij = ij_sw_corner + i * my + j;
	 		u[ij] = (u[ij] * z_scale) + (plane_c0 + plane_c1 * i + plane_c2 * j);
		}
	}
	return(0);
}

int	throw_away_unusables()
{
	/* This is a new routine to eliminate data which will become
		unusable on the final iteration, when grid = 1.
		It assumes grid = 1 and set_grid_parameters has been
		called.  We sort, mark redundant data as OUTSIDE, and
		sort again, chopping off the excess.
		
		Experimental modification 5 Dec 1988 by Smith, as part
		of a new implementation using core memory for b[6]
		coefficients, eliminating calls to temp file.
	*/
	
	int	last_index, n_outside, k;
	
	/* Sort the data  */
	
	qsort ((char *)data, npoints, sizeof (struct DATA), compare_points);
	
	/* If more than one datum is indexed to same node, only the first should be kept.
		Mark the additional ones as OUTSIDE
	*/
	last_index = -1;
	n_outside = 0;
	for (k = 0; k < npoints; k++) {
		if (data[k].index == last_index) {
			data[k].index = OUTSIDE;
			n_outside++;
		}
		else {
			last_index = data[k].index;
		}
	}
	/* Sort again; this time the OUTSIDE points will be thrown away  */
	
	qsort ((char *)data, npoints, sizeof (struct DATA), compare_points);
	npoints -= n_outside;
	status = mb_realloc(local_verbose, npoints * sizeof(struct DATA), &data, &error);
	if (local_verbose && (n_outside)) {
		fprintf(stderr,"surface: %ld unusable points were supplied; these will be ignored.\n", n_outside);
		fprintf(stderr,"\tYou should have pre-processed the data with blockmean or blockmedian.\n");
	}

	return(0);
}

int	rescale_z_values()
{
	int	i;
	double	ssz = 0.0;

	for (i = 0; i < npoints; i++) {
		ssz += (data[i].z * data[i].z);
	}
	
	/* Set z_scale = rms(z):  */
	
	z_scale = sqrt(ssz / npoints);
	r_z_scale = 1.0 / z_scale;

	for (i = 0; i < npoints; i++) {
		data[i].z *= r_z_scale;
	}
	return (0);
}

int load_constraints (low, high)
char *low, *high; {
	int i, j, ij, n_trimmed;
	double xx, yy;
/*	struct GRD_HEADER hdr;*/
	
	/* Load lower/upper limits, verify range, deplane, and rescale */
	
	if (set_low > 0) {
		status = mb_malloc(local_verbose, nx * ny * sizeof(float), &lower, &error);
		if (set_low < 3)
			for (i = 0; i < nx * ny; i++) lower[i] = low_limit;
/* Comment this out:
		else {
			if (read_grd_info (low, &hdr)) {
				fprintf (stderr, "surface: Error opening file %s\n", low);
				exit (-1);
			}
			if (hdr.nx != nx || hdr.ny != ny) {
				fprintf (stderr, "surface: lower limit file not of proper dimension!\n");
				exit (-1);
			}
			if (read_grd (low, &hdr, lower)) {
				fprintf (stderr, "surface: Error reading file %s\n", low);
				exit (-1);
			}
			n_trimmed = 0;
			for (i = 0; i < nx * ny; i++) if (lower[i] > low_limit) {
				lower[i] = low_limit;
				n_trimmed++;
			}
			if (n_trimmed) fprintf (stderr, "surface: %d lower limit values > min data, reset to min data!\n");
		}
*/
			
		for (j = ij = 0; j < ny; j++) {
			yy = ny - j - 1;
			for (i = 0; i < nx; i++, ij++) {
				/*if (GMT_is_fnan ((double)lower[ij])) continue;*/
				lower[ij] -= (plane_c0 + plane_c1 * i + plane_c2 * yy);
				lower[ij] *= r_z_scale;
			}
		}
		constrained = TRUE;
	}
	if (set_high > 0) {
		status = mb_malloc(local_verbose, nx * ny * sizeof(float), &upper, &error);
		if (set_high < 3)
			for (i = 0; i < nx * ny; i++) upper[i] = high_limit;
/* Comment this out:
		else {
			if (read_grd_info (high, &hdr)) {
				fprintf (stderr, "surface: Error opening file %s\n", high);
				exit (-1);
			}
			if (hdr.nx != nx || hdr.ny != ny) {
				fprintf (stderr, "surface: upper limit file not of proper dimension!\n");
				exit (-1);
			}
			if (read_grd (high, &hdr, upper)) {
				fprintf (stderr, "surface: Error reading file %s\n", high);
				exit (-1);
			}
			n_trimmed = 0;
			for (i = 0; i < nx * ny; i++) if (upper[i] < high_limit) {
				upper[i] = high_limit;
				n_trimmed++;
			}
			if (n_trimmed) fprintf (stderr, "surface: %d upper limit values < max data, reset to max data!\n");
		}
*/
		for (j = ij = 0; j < ny; j++) {
			yy = ny - j - 1;
			for (i = 0; i < nx; i++, ij++) {
				/*if (GMT_is_fnan ((double)upper[ij])) continue;*/
				upper[ij] -= (plane_c0 + plane_c1 * i + plane_c2 * yy);
				upper[ij] *= r_z_scale;
			}
		}
		constrained = TRUE;
	}
}

double	guess_surface_time(nx, ny)
int	nx, ny;
{
	/* Routine to guess a number proportional to the operations
	 * required by surface working on a user-desired grid of
	 * size nx by ny, where nx = (xmax - xmin)/dx, and same for
	 * ny.  (That is, one less than actually used in routine.)
	 *
	 * This is based on the following untested conjecture:
	 * 	The operations are proportional to T = nxg*nyg*L,
	 *	where L is a measure of the distance that data
	 *	constraints must propagate, and nxg, nyg are the
	 * 	current size of the grid.
	 *	For nx,ny relatively prime, we will go through only
	 * 	one grid cycle, L = max(nx,ny), and T = nx*ny*L.
	 *	But for nx,ny whose greatest common divisor is a highly
	 * 	composite number, we will have L equal to the division
	 * 	step made at each new grid cycle, and nxg,nyg will
	 * 	also be smaller than nx,ny.  Thus we can hope to find
	 *	some nx,ny for which the total value of T is small.
	 *
	 * The above is pure speculation and has not been derived
	 * empirically.  In actual practice, the distribution of the
	 * data, both spatially and in terms of their values, will
	 * have a strong effect on convergence.
	 *
	 * W. H. F. Smith, 26 Feb 1992.  */

	int	gcd_euclid();	/* Finds the greatest common divisor  */
	int	get_prime_factors();
	int	gcd;		/* Current value of the gcd  */
	int	nxg, nyg;	/* Current value of the grid dimensions  */
	int	nfactors;	/* Number of prime factors of current gcd  */
	int	factor;		/* Currently used factor  */
	/* Doubles are used below, even though the values will be integers,
		because the multiplications might reach sizes of O(n**3)  */
	double	t_sum;		/* Sum of values of T at each grid cycle  */
	double	length;		/* Current propagation distance.  */


	gcd = gcd_euclid(nx, ny);
	if (gcd > 1) {
		nfactors = get_prime_factors(gcd, factors);
		nxg = nx/gcd;
		nyg = ny/gcd;
		if (nxg < 3 || nyg < 3) {
			factor = factors[nfactors - 1];
			nfactors--;
			gcd /= factor;
			nxg *= factor;
			nyg *= factor;
		}
	}
	else {
		nxg = nx;
		nyg = ny;
	}
	length = MAX(nxg, nyg);
	t_sum = nxg * (nyg * length);	/* Make it double at each multiply  */

	/* Are there more grid cycles ?  */
	while (gcd > 1) {
		factor = factors[nfactors - 1];
		nfactors--;
		gcd /= factor;
		nxg *= factor;
		nyg *= factor;
		length = factor;
		t_sum += nxg * (nyg * length);
	}
	return(t_sum);
}


int	get_prime_factors(n, f)
int	n, f[];
{
	/* Fills the integer array f with the prime factors of n.
	 * Returns the number of locations filled in f, which is
	 * one if n is prime.
	 *
	 * f[] should have been malloc'ed to enough space before
	 * calling prime_factors().  We can be certain that f[32]
	 * is enough space, for if n fits in a long, then n < 2**32,
	 * and so it must have fewer than 32 prime factors.  I think
	 * that in general, ceil(log2((double)n)) is enough storage
	 * space for f[].
	 *
	 * Tries 2,3,5 explicitly; then alternately adds 2 or 4
	 * to the previously tried factor to obtain the next trial
	 * factor.  This is done with the variable two_four_toggle.
	 * With this method we try 7,11,13,17,19,23,25,29,31,35,...
	 * up to a maximum of sqrt(n).  This shortened list results
	 * in 1/3 fewer divisions than if we simply tried all integers
	 * between 5 and sqrt(n).  We can reduce the size of the list
	 * of trials by an additional 20% by removing the multiples
	 * of 5, which are equal to 30m +/- 5, where m >= 1.  Starting
	 * from 25, these are found by alternately adding 10 or 20.
	 * To do this, we use the variable ten_twenty_toggle.
	 *
	 * W. H. F. Smith, 26 Feb 1992, after D.E. Knuth, vol. II  */

	int	current_factor;	/* The factor currently being tried  */
	int	max_factor;	/* Don't try any factors bigger than this  */
	int	n_factors = 0;	/* Returned; one if n is prime  */
	int	two_four_toggle = 0;	/* Used to add 2 or 4 to get next trial factor  */
	int	ten_twenty_toggle = 0;	/* Used to add 10 or 20 to skip_five  */
	int	skip_five = 25;	/* Used to skip multiples of 5 in the list  */
	int	m;	/* Used to keep a working copy of n  */


	/* Initialize m and max_factor  */
	m = abs(n);
	if (m < 2) return(0);
	max_factor = floor(sqrt((double)m));

	/* First find the 2s  */
	current_factor = 2;
	while(!(m%current_factor)) {
		m /= current_factor;
		f[n_factors] = current_factor;
		n_factors++;
	}
	if (m == 1) return(n_factors);

	/* Next find the 3s  */
	current_factor = 3;
	while(!(m%current_factor)) {
		m /= current_factor;
		f[n_factors] = current_factor;
		n_factors++;
	}
	if (m == 1) return(n_factors);

	/* Next find the 5s  */
	current_factor = 5;
	while(!(m%current_factor)) {
		m /= current_factor;
		f[n_factors] = current_factor;
		n_factors++;
	}
	if (m == 1) return(n_factors);

	/* Now try all the rest  */

	while (m > 1 && current_factor <= max_factor) {

		/* Current factor is either 2 or 4 more than previous value  */

		if (two_four_toggle) {
			current_factor += 4;
			two_four_toggle = 0;
		}
		else {
			current_factor += 2;
			two_four_toggle = 1;
		}

		/* If current factor is a multiple of 5, skip it.  But first,
			set next value of skip_five according to 10/20 toggle:  */

		if (current_factor == skip_five) {
			if (ten_twenty_toggle) {
				skip_five += 20;
				ten_twenty_toggle = 0;
			}
			else {
				skip_five += 10;
				ten_twenty_toggle = 1;
			}
			continue;
		}

		/* Get here when current_factor is not a multiple of 2,3 or 5:  */

		while(!(m%current_factor)) {
			m /= current_factor;
			f[n_factors] = current_factor;
			n_factors++;
		}
	}

	/* Get here when all factors up to floor(sqrt(n)) have been tried.  */

	if (m > 1) {
		/* m is an additional prime factor of n  */
		f[n_factors] = m;
		n_factors++;
	}
	return (n_factors);
}
/* gcd_euclid.c  Greatest common divisor routine  */

#define IABS(i)	(((i) < 0) ? -(i) : (i))

int	gcd_euclid(a,b)
int	a, b;
{
	/* Returns the greatest common divisor of u and v by Euclid's method.
	 * I have experimented also with Stein's method, which involves only
	 * subtraction and left/right shifting; Euclid is faster, both for
	 * integers of size 0 - 1024 and also for random integers of a size
	 * which fits in a long integer.  Stein's algorithm might be better
	 * when the integers are HUGE, but for our purposes, Euclid is fine.
	 *
	 * Walter H. F. Smith, 25 Feb 1992, after D. E. Knuth, vol. II  */

	int	u,v,r;

	u = MAX(IABS(a), IABS(b));
	v = MIN(IABS(a), IABS(b));

	while (v > 0) {
		r = u%v;	/* Knuth notes that u < 2v 40% of the time;  */
		u = v;		/* thus we could have tried a subtraction  */
		v = r;		/* followed by an if test to do r = u%v  */
	}
	return(u);
}

