/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_ldeoih.c	3.00	2/26/93
 *	$Id: mbsys_ldeoih.c,v 3.0 1993-05-14 23:04:29 sohara Exp $
 *
 *    Copyright (c) 1993 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbsys_ldeoih.c contains the functions for handling the data structure
 * used by MBIO functions to store data from the 16-beam Sea Beam 
 * multibeam sonar systems.
 * The data formats which are commonly used to store Sea Beam
 * data in files include
 *      MBF_SBSIOMRG : MBIO ID 1
 *      MBF_SBSIOCEN : MBIO ID 2
 *      MBF_SBSIOLSI : MBIO ID 3
 *      MBF_SBURICEN : MBIO ID 4
 * These functions include:
 *   mbsys_ldeoih_alloc		- allocate memory for mbsys_ldeoih_struct 
 *					structure
 *   mbsys_ldeoih_deall		- deallocate memory for mbsys_ldeoih_struct 
 *					structure
 *   mbsys_ldeoih_extract	- extract basic data from mbsys_ldeoih_struct 
 *					structure
 *   mbsys_ldeoih_insert	- insert basic data into mbsys_ldeoih_struct 
 *					structure
 *   mbsys_ldeoih_copy		- copy data in one mbsys_ldeoih_struct 
 *					structure into another 
 *					mbsys_ldeoih_struct structure
 *
 * Author:	D. W. Caress
 * Date:	February 26, 1993
 * $Log: not supported by cvs2svn $
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <strings.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_io.h"
#include "../../include/mbsys_ldeoih.h"

/*--------------------------------------------------------------------*/
int mbsys_ldeoih_alloc(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	**store_ptr;
int	*error;
{
 static char res_id[]="$Id: mbsys_ldeoih.c,v 3.0 1993-05-14 23:04:29 sohara Exp $";
	char	*function_name = "mbsys_ldeoih_alloc";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_ldeoih_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* allocate memory for data structure */
	status = mb_malloc(verbose,sizeof(struct mbsys_ldeoih_struct),
				store_ptr,error);

	/* get pointer to data structure */
	store = (struct mbsys_ldeoih_struct *) *store_ptr;

	/* allocate memory for data arrays in structure */
	status = mb_malloc(verbose,mb_io_ptr->beams_bath*sizeof(int),
				&store->bath,error);
	status = mb_malloc(verbose,mb_io_ptr->beams_bath*sizeof(int),
				&store->bathdist,error);
	status = mb_malloc(verbose,mb_io_ptr->beams_back*sizeof(int),
				&store->back,error);
	status = mb_malloc(verbose,mb_io_ptr->beams_back*sizeof(int),
				&store->backdist,error);
	store->beams_bath = mb_io_ptr->beams_bath;
	store->beams_back = mb_io_ptr->beams_back;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       store_ptr:  %d\n",*store_ptr);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_ldeoih_deall(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
{
	char	*function_name = "mbsys_ldeoih_deall";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_ldeoih_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to data structure */
	store = (struct mbsys_ldeoih_struct *) store_ptr;

	/* deallocate memory for data structures */
	status = mb_free(verbose,store->bath,error);
	status = mb_free(verbose,store->bathdist,error);
	status = mb_free(verbose,store->back,error);
	status = mb_free(verbose,store->backdist,error);

	/* deallocate memory for data structure */
	status = mb_free(verbose,store_ptr,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_ldeoih_extract(verbose,mbio_ptr,store_ptr,kind,
		time_i,time_d,navlon,navlat,speed,heading,
		nbath,bath,bathdist,nback,back,backdist,
		comment,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*kind;
int	time_i[6];
double	*time_d;
double	*navlon;
double	*navlat;
double	*speed;
double	*heading;
int	*nbath;
int	*bath;
int	*bathdist;
int	*nback;
int	*back;
int	*backdist;
char	*comment;
int	*error;
{
	char	*function_name = "mbsys_ldeoih_extract";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_ldeoih_struct *store;
	int	time_j[4];
	int	id;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_ldeoih_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get time */
		time_j[0] = store->year;
		time_j[1] = store->day;
		time_j[2] = store->min;
		time_j[3] = store->sec;
		mb_get_itime(verbose,time_j,time_i);
		mb_get_time(verbose,time_i,time_d);

		/* get navigation */
		*navlon = store->lon2u/60. + store->lon2b/600000.;
		*navlat = store->lat2u/60. + store->lat2b/600000. - 90.;
		if (mb_io_ptr->lonflip < 0)
			{
			if (*navlon > 0.) 
				*navlon = *navlon - 360.;
			else if (*navlon < -360.)
				*navlon = *navlon + 360.;
			}
		else if (mb_io_ptr->lonflip == 0)
			{
			if (*navlon > 180.) 
				*navlon = *navlon - 360.;
			else if (*navlon < -180.)
				*navlon = *navlon + 360.;
			}
		else
			{
			if (*navlon > 360.) 
				*navlon = *navlon - 360.;
			else if (*navlon < 0.)
				*navlon = *navlon + 360.;
			}

		/* get heading (360 degrees = 65536) */
		*heading = store->heading*0.0054932;

		/* set speed to zero */
		*speed = 0.0;

		/* read distance, depth, and backscatter 
			values into storage arrays */
		*nbath = mb_io_ptr->beams_bath;
		*nback = mb_io_ptr->beams_back;
		for (i=0;i<*nbath;i++)
			{
			bathdist[i] = store->bathdist[i];
			bath[i] = store->bath[i];
			}
		for (i=0;i<*nback;i++)
			{
			backdist[i] = store->backdist[i];
			back[i] = store->back[i];
			}

		/* print debug statements */
		if (verbose >= 5)
			{
			fprintf(stderr,"\ndbg4  Data extracted by MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4  Extracted values:\n");
			fprintf(stderr,"dbg4       kind:       %d\n",
				*kind);
			fprintf(stderr,"dbg4       error:      %d\n",
				*error);
			fprintf(stderr,"dbg4       time_i[0]:  %d\n",
				time_i[0]);
			fprintf(stderr,"dbg4       time_i[1]:  %d\n",
				time_i[1]);
			fprintf(stderr,"dbg4       time_i[2]:  %d\n",
				time_i[2]);
			fprintf(stderr,"dbg4       time_i[3]:  %d\n",
				time_i[3]);
			fprintf(stderr,"dbg4       time_i[4]:  %d\n",
				time_i[4]);
			fprintf(stderr,"dbg4       time_i[5]:  %d\n",
				time_i[5]);
			fprintf(stderr,"dbg4       time_d:     %f\n",
				*time_d);
			fprintf(stderr,"dbg4       longitude:  %f\n",
				*navlon);
			fprintf(stderr,"dbg4       latitude:   %f\n",
				*navlat);
			fprintf(stderr,"dbg4       speed:      %f\n",
				*speed);
			fprintf(stderr,"dbg4       heading:    %f\n",
				*heading);
			fprintf(stderr,"dbg4       nbath:      %d\n",
				*nbath);
			for (i=0;i<*nbath;i++)
			  fprintf(stderr,"dbg4       bath[%d]: %d  bathdist[%d]: %d\n",
				i,bath[i],
				i,bathdist[i]);
			fprintf(stderr,"dbg4       nback:      %d\n",
				*nback);
			for (i=0;i<*nback;i++)
			  fprintf(stderr,"dbg4       back[%d]: %d  backdist[%d]: %d\n",
				i,back[i],
				i,backdist[i]);
			}

		/* done translating values */

		}

	/* extract comment from structure */
	else if (*kind == MB_DATA_COMMENT)
		{
		/* copy comment */
		strcpy(comment,store->comment);

		/* print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"\ndbg4  New ping read by MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4  New ping values:\n");
			fprintf(stderr,"dbg4       error:      %d\n",
				error);
			fprintf(stderr,"dbg4       comment:    %s\n",
				comment);
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       kind:       %d\n",*kind);
		}
	if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR 
		&& *kind == MB_DATA_COMMENT)
		{
		fprintf(stderr,"dbg2       comment:     \ndbg2       %s\n",
			comment);
		}
	else if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR 
		&& *kind != MB_DATA_COMMENT)
		{
		fprintf(stderr,"dbg2       time_i[0]:     %d\n",time_i[0]);
		fprintf(stderr,"dbg2       time_i[1]:     %d\n",time_i[1]);
		fprintf(stderr,"dbg2       time_i[2]:     %d\n",time_i[2]);
		fprintf(stderr,"dbg2       time_i[3]:     %d\n",time_i[3]);
		fprintf(stderr,"dbg2       time_i[4]:     %d\n",time_i[4]);
		fprintf(stderr,"dbg2       time_i[5]:     %d\n",time_i[5]);
		fprintf(stderr,"dbg2       time_d:        %f\n",*time_d);
		fprintf(stderr,"dbg2       longitude:     %f\n",*navlon);
		fprintf(stderr,"dbg2       latitude:      %f\n",*navlat);
		fprintf(stderr,"dbg2       speed:         %f\n",*speed);
		fprintf(stderr,"dbg2       heading:       %f\n",*heading);
		}
	if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR 
		&& *kind == MB_DATA_DATA)
		{
		fprintf(stderr,"dbg2       nbath:         %d\n",*nbath);
		for (i=0;i<*nbath;i++)
		  fprintf(stderr,"dbg2       bath[%d]: %d  bathdist[%d]: %d\n",
			i,bath[i],i,bathdist[i]);
		fprintf(stderr,"dbg2       nback:         %d\n",*nback);
		for (i=0;i<*nback;i++)
		  fprintf(stderr,"dbg2       back[%d]: %d  backdist[%d]: %d\n",
			i,back[i],i,backdist[i]);
		}
	if (verbose >= 2)
		{
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_ldeoih_insert(verbose,mbio_ptr,store_ptr,
		time_i,time_d,navlon,navlat,speed,heading,
		nbath,bath,bathdist,nback,back,backdist,
		comment,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	time_i[6];
double	time_d;
double	navlon;
double	navlat;
double	speed;
double	heading;
int	nbath;
int	*bath;
int	*bathdist;
int	nback;
int	*back;
int	*backdist;
char	*comment;
int	*error;
{
	char	*function_name = "mbsys_ldeoih_insert";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_ldeoih_struct *store;
	int	kind;
	int	time_j[4];
	int	id;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		fprintf(stderr,"dbg2       time_i[0]:  %d\n",time_i[0]);
		fprintf(stderr,"dbg2       time_i[1]:  %d\n",time_i[1]);
		fprintf(stderr,"dbg2       time_i[2]:  %d\n",time_i[2]);
		fprintf(stderr,"dbg2       time_i[3]:  %d\n",time_i[3]);
		fprintf(stderr,"dbg2       time_i[4]:  %d\n",time_i[4]);
		fprintf(stderr,"dbg2       time_i[5]:  %d\n",time_i[5]);
		fprintf(stderr,"dbg2       time_d:     %d\n",time_d);
		fprintf(stderr,"dbg2       navlon:     %f\n",navlon);
		fprintf(stderr,"dbg2       navlat:     %f\n",navlat);
		fprintf(stderr,"dbg2       speed:      %f\n",speed);
		fprintf(stderr,"dbg2       heading:    %f\n",heading);
		fprintf(stderr,"dbg2       nbath:      %d\n",nbath);
		if (verbose >= 3) 
		 for (i=0;i<nbath;i++)
		  fprintf(stderr,"dbg3       bath[%d]: %d  bathdist[%d]: %d\n",
			i,bath[i],i,bathdist[i]);
		fprintf(stderr,"dbg2       nback:      %d\n",nback);
		if (verbose >= 3) 
		 for (i=0;i<nback;i++)
		  fprintf(stderr,"dbg3       back[%d]: %d  backdist[%d]: %d\n",
			i,back[i],i,backdist[i]);
		fprintf(stderr,"dbg2       comment:    %s\n",comment);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_ldeoih_struct *) store_ptr;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA)
		{
		/* get time */
		mb_get_jtime(verbose,time_i,time_j);
		store->year = time_j[0];
		store->day = time_j[1];
		store->min = time_j[2];
		store->sec = time_j[3];

		/* get navigation */
		if (navlon < 0.0) navlon = navlon + 360.0;
		store->lon2u = (short int) 60.0*navlon;
		store->lon2b = (short int) (600000.0*(navlon 
			- store->lon2u/60.0));
		navlat = navlat + 90.0;
		store->lat2u = (short int) 60.0*navlat;
		store->lat2b = (short int) (600000.0*(navlat 
			- store->lat2u/60.0));

		/* get heading (360 degrees = 65536) */
		store->heading = 182.044444*heading;

		/* put distance, depth, and backscatter values 
			into data structure */
		for (i=0;i<nbath;i++)
			{
			store->bath[i] = bath[i];
			store->bathdist[i] = bathdist[i];
			}
		for (i=0;i<nback;i++)
			{
			store->back[i] = back[i];
			store->backdist[i] = backdist[i];
			}
		}

	/* insert comment in structure */
	else if (store->kind == MB_DATA_COMMENT)
		{
		strcpy(store->comment,comment);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_ldeoih_copy(verbose,mbio_ptr,store_ptr,copy_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
char	*copy_ptr;
int	*error;
{
	char	*function_name = "mbsys_ldeoih_copy";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_ldeoih_struct *store;
	struct mbsys_ldeoih_struct *copy;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		fprintf(stderr,"dbg2       copy_ptr:   %d\n",copy_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointers */
	store = (struct mbsys_ldeoih_struct *) store_ptr;
	copy = (struct mbsys_ldeoih_struct *) copy_ptr;

	/* copy the data */
	copy->kind = store->kind;
	copy->lon2u = store->lon2u;
	copy->lon2b = store->lon2b;
	copy->lat2u = store->lat2u;
	copy->lat2b = store->lat2b;
	copy->year = store->year;
	copy->day = store->day;
	copy->min = store->min;
	copy->sec = store->sec;
	copy->heading = store->heading;
	copy->speed = store->speed;
	if (copy->beams_bath <= 0)
		copy->beams_bath = store->beams_bath;
	if (copy->beams_back <= 0)
		copy->beams_back = store->beams_back;
	copy->bathscale = store->bathscale;
	copy->backscale = store->backscale;
	for (i=0;i<copy->beams_bath;i++)
		{
		copy->bath[i] = store->bath[i];
		copy->bathdist[i] = store->bathdist[i];
		}
	for (i=0;i<copy->beams_back;i++)
		{
		copy->back[i] = store->back[i];
		copy->backdist[i] = store->backdist[i];
		}
	strcpy(copy->comment,store->comment);	

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
