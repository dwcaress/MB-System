/*--------------------------------------------------------------------
 *    The MB-system:	mbr_sbifremr.c	3/29/96
 *	$Id: mbr_sbifremr.c,v 4.2 1997-04-21 17:02:07 caress Exp $
 *
 *    Copyright (c) 1996 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbr_sbifremr.c contains the functions for reading and writing
 * multibeam data in the SBIFREMR format.  
 * These functions include:
 *   mbr_alm_sbifremr	- allocate read/write memory
 *   mbr_dem_sbifremr	- deallocate read/write memory
 *   mbr_rt_sbifremr	- read and translate data
 *   mbr_wt_sbifremr	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	March 29, 1996
 * Location:	152 39.061W; 34 09.150S on R/V Ewing
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.1  1996/04/22  13:21:19  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.1  1996/04/22  13:21:19  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.0  1996/04/22  11:01:20  caress
 * Initial version.
 *
 * Revision 1.1  1996/04/22  10:57:09  caress
 * Initial revision
 *
 * Revision 4.4  1995/03/06  19:38:54  caress
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_io.h"
#include "../../include/mb_define.h"
#include "../../include/mbsys_sb.h"
#include "../../include/mbf_sbifremr.h"

/* angle spacing for SeaBeam Classic */
#define	ANGLE_SPACING 3.75

/*--------------------------------------------------------------------*/
int mbr_alm_sbifremr(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
 static char res_id[]="$Id: mbr_sbifremr.c,v 4.2 1997-04-21 17:02:07 caress Exp $";
	char	*function_name = "mbr_alm_sbifremr";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* set initial status */
	status = MB_SUCCESS;

	/* allocate memory for data structure */
	mb_io_ptr->structure_size = sizeof(struct mbf_sbifremr_struct);
	mb_io_ptr->data_structure_size = 0;
	status = mb_malloc(verbose,mb_io_ptr->structure_size,
				&mb_io_ptr->raw_data,error);
	status = mb_malloc(verbose,sizeof(struct mbsys_sb_struct),
				&mb_io_ptr->store_data,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_dem_sbifremr(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
	char	*function_name = "mbr_dem_sbifremr";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* deallocate memory for data descriptor */
	status = mb_free(verbose,&mb_io_ptr->raw_data,error);
	status = mb_free(verbose,&mb_io_ptr->store_data,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_rt_sbifremr(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
{
	char	*function_name = "mbr_rt_sbifremr";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_sbifremr_struct *data;
	struct mbsys_sb_struct *store;
	int	time_j[5];
	int	i, j, k;
	int	id;

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

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	data = (struct mbf_sbifremr_struct *) mb_io_ptr->raw_data;
	data->kind = MB_DATA_DATA;
	store = (struct mbsys_sb_struct *) store_ptr;

	/* read next record from file */
	status = mbr_sbifremr_rd_data(verbose,mbio_ptr,error);

	/* set kind and error in mb_io_ptr */
	mb_io_ptr->new_kind = data->kind;
	mb_io_ptr->new_error = *error;

	/* translate values to current ping variables 
		in mbio descriptor structure */
	if (status == MB_SUCCESS 
		&& data->kind == MB_DATA_DATA)
		{
		/* get time */
		time_j[0] = data->year;
		time_j[1] = data->day;
		time_j[2] = data->min;
		time_j[3] = data->sec;
		time_j[4] = 0;
		mb_get_itime(verbose,time_j,mb_io_ptr->new_time_i);
		mb_get_time(verbose,mb_io_ptr->new_time_i,
			&(mb_io_ptr->new_time_d));

		/* get navigation */
		mb_io_ptr->new_lon = data->lon2u/60. 
			+ data->lon2b/600000.;
		mb_io_ptr->new_lat = data->lat2u/60. 
			+ data->lat2b/600000. - 90.;
		if (mb_io_ptr->lonflip < 0)
			{
			if (mb_io_ptr->new_lon > 0.) 
				mb_io_ptr->new_lon = mb_io_ptr->new_lon - 360.;
			else if (mb_io_ptr->new_lon < -360.)
				mb_io_ptr->new_lon = mb_io_ptr->new_lon + 360.;
			}
		else if (mb_io_ptr->lonflip == 0)
			{
			if (mb_io_ptr->new_lon > 180.) 
				mb_io_ptr->new_lon = mb_io_ptr->new_lon - 360.;
			else if (mb_io_ptr->new_lon < -180.)
				mb_io_ptr->new_lon = mb_io_ptr->new_lon + 360.;
			}
		else
			{
			if (mb_io_ptr->new_lon > 360.) 
				mb_io_ptr->new_lon = mb_io_ptr->new_lon - 360.;
			else if (mb_io_ptr->new_lon < 0.)
				mb_io_ptr->new_lon = mb_io_ptr->new_lon + 360.;
			}

		/* get heading (360 degrees = 65536) */
		mb_io_ptr->new_heading = data->sbhdg*0.0054932;

		/* set speed to zero */
		mb_io_ptr->new_speed = 0.0;

		/* read distance and depth values into storage arrays */
		/* switch order of data as it is read into the global arrays */
		id = mb_io_ptr->beams_bath - 1;
		for (i=0;i<mb_io_ptr->beams_bath;i++)
			{
			mb_io_ptr->new_bath[id-i] = data->deph[i];
			mb_io_ptr->new_bath_acrosstrack[id-i] = data->dist[i];
			mb_io_ptr->new_bath_alongtrack[id-i] = 0.0;
			}

		/* print debug statements */
		if (verbose >= 5)
			{
			fprintf(stderr,"\ndbg4  New ping read by MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4  New ping values:\n");
			fprintf(stderr,"dbg4       error:      %d\n",
				mb_io_ptr->new_error);
			fprintf(stderr,"dbg4       time_i[0]:  %d\n",
				mb_io_ptr->new_time_i[0]);
			fprintf(stderr,"dbg4       time_i[1]:  %d\n",
				mb_io_ptr->new_time_i[1]);
			fprintf(stderr,"dbg4       time_i[2]:  %d\n",
				mb_io_ptr->new_time_i[2]);
			fprintf(stderr,"dbg4       time_i[3]:  %d\n",
				mb_io_ptr->new_time_i[3]);
			fprintf(stderr,"dbg4       time_i[4]:  %d\n",
				mb_io_ptr->new_time_i[4]);
			fprintf(stderr,"dbg4       time_i[5]:  %d\n",
				mb_io_ptr->new_time_i[5]);
			fprintf(stderr,"dbg4       time_i[6]:  %d\n",
				mb_io_ptr->new_time_i[6]);
			fprintf(stderr,"dbg4       time_d:     %f\n",
				mb_io_ptr->new_time_d);
			fprintf(stderr,"dbg4       longitude:  %f\n",
				mb_io_ptr->new_lon);
			fprintf(stderr,"dbg4       latitude:   %f\n",
				mb_io_ptr->new_lat);
			fprintf(stderr,"dbg4       speed:      %f\n",
				mb_io_ptr->new_speed);
			fprintf(stderr,"dbg4       heading:    %f\n",
				mb_io_ptr->new_heading);
			fprintf(stderr,"dbg4       beams_bath: %d\n",
				mb_io_ptr->beams_bath);
			for (i=0;i<mb_io_ptr->beams_bath;i++)
			  fprintf(stderr,"dbg4       bath[%d]: %f  bathdist[%d]: %f\n",
				i,mb_io_ptr->new_bath[i],
				i,mb_io_ptr->new_bath_acrosstrack[i]);
			}

		/* done translating values */

		}
	else if (status == MB_SUCCESS 
		&& data->kind == MB_DATA_COMMENT)
		{
		/* copy comment */
		strncpy(mb_io_ptr->new_comment,data->comment,
			MBSYS_SB_MAXLINE-1);

		/* print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"\ndbg4  New ping read by MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4  New ping values:\n");
			fprintf(stderr,"dbg4       error:      %d\n",
				mb_io_ptr->new_error);
			fprintf(stderr,"dbg4       comment:    %s\n",
				mb_io_ptr->new_comment);
			}
		}

	/* translate values to sea beam data storage structure */
	if (status == MB_SUCCESS
		&& store != NULL)
		{
		/* type of data record */
		store->kind = data->kind;

		/* position */
		store->lon2u = data->lon2u;
		store->lon2b = data->lon2b;
		store->lat2u = data->lat2u;
		store->lat2b = data->lat2b;

		/* time stamp */
		store->year = data->year;
		store->day = data->day;
		store->min = data->min;
		store->sec = data->sec;

		/* depths and distances */
		for (i=0;i<mb_io_ptr->beams_bath;i++)
			{
			store->dist[i] = data->dist[i];
			store->deph[i] = data->deph[i];
			}

		/* additional values */
		store->sbtim = 0;
		store->sbhdg = data->sbhdg;
		store->axis = 0;
		store->major = 0;
		store->minor = 0;

		/* comment */
		strncpy(store->comment,mb_io_ptr->new_comment,
			MBSYS_SB_MAXLINE);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_wt_sbifremr(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
{
	char	*function_name = "mbr_wt_sbifremr";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_sbifremr_struct *data;
	struct mbsys_sb_struct *store;
	int	time_j[5];
	double	lon, lat;
	int	i, j;
	int	id;

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

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	data = (struct mbf_sbifremr_struct *) mb_io_ptr->raw_data;
	store = (struct mbsys_sb_struct *) store_ptr;

	/* second translate values from seabeam data storage structure */
	if (store != NULL)
		{
		data->kind = store->kind;
		if (store->kind == MB_DATA_DATA)
			{
			/* position */
			data->lon2u = store->lon2u;
			data->lon2b = store->lon2b;
			data->lat2u = store->lat2u;
			data->lat2b = store->lat2b;

			/* time stamp */
			data->year = store->year;
			data->day = store->day;
			data->min = store->min;
			data->sec = store->sec;

			/* depths and distances */
			for (i=0;i<mb_io_ptr->beams_bath;i++)
				{
				data->dist[i] = store->dist[i];
				data->deph[i] = store->deph[i];
				}

			/* additional values */
			data->sbhdg = store->sbhdg;
			}

		/* comment */
		else if (store->kind == MB_DATA_COMMENT)
			{
			strncpy(data->comment,store->comment,
				MBSYS_SB_MAXLINE-1);
			}
		}

	/* set kind from current ping */
	if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR)
		data->kind = mb_io_ptr->new_kind;

	/* check for comment */
	if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR
		&& mb_io_ptr->new_kind == MB_DATA_COMMENT)
		{
		strncpy(data->comment,mb_io_ptr->new_comment,
			MBSYS_SB_MAXLINE-1);
		}

	/* else translate current ping data to sbifremr data structure */
	else if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR
		&& mb_io_ptr->new_kind == MB_DATA_DATA)
		{
		/* get time */
		mb_get_jtime(verbose,mb_io_ptr->new_time_i,time_j);
		data->year = time_j[0];
		data->day = time_j[1];
		data->min = time_j[2];
		data->sec = time_j[3];

		/* get navigation */
		lon = mb_io_ptr->new_lon;
		if (lon < 0.0) lon = lon + 360.0;
		data->lon2u = (short int) 60.0*lon;
		data->lon2b = (short int) (600000.0*(lon - data->lon2u/60.0));
		lat = mb_io_ptr->new_lat + 90.0;
		data->lat2u = (short int) 60.0*lat;
		data->lat2b = (short int) (600000.0*(lat - data->lat2u/60.0));

		/* get heading (360 degrees = 65536) */
		data->sbhdg = 182.044444*mb_io_ptr->new_heading;

		/* put distance and depth values 
			into sbifremr data structure */
		for (i=0;i<mb_io_ptr->beams_bath;i++)
			{
			data->deph[i] = mb_io_ptr->new_bath[i];
			data->dist[i] = mb_io_ptr->new_bath_acrosstrack[i];
			}
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Ready to write data in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       kind:       %d\n",
			data->kind);
		fprintf(stderr,"dbg5       error:      %d\n",*error);
		fprintf(stderr,"dbg5       status:     %d\n",status);
		}

	/* write next record to file */
	if (data->kind == MB_DATA_DATA
		|| data->kind == MB_DATA_COMMENT)
		{
		status = mbr_sbifremr_wr_data(verbose,mbio_ptr,error);
		}
	else
		{
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		if (verbose >= 5)
			fprintf(stderr,"\ndbg5  No data written in MBIO function <%s>\n",
				function_name);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_sbifremr_rd_data(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
	char	*function_name = "mbr_sbifremr_rd_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_sbifremr_struct *data;
	FILE	*mbfp;
	int	done;
	static char line[MBF_SBIFREMR_MAXLINE];
	static int  line_save = MB_NO;
	static int  first = MB_YES;
	char	*result;
	int	nchars;
	char	NorS, EorW;
	int	lon_deg, lat_deg;
	double	lon_min, lat_min;
	double	depth;
	double  heading;
	static int ping_num_save = 0;
	int	ping_num;
	int	beam_num;
	int	day, month, year, hour, minute, second, tsecond;
	int	time_i[7], time_j[4];
	static double	heading_save = 0.0;
	int	center;
	double	mtodeglon, mtodeglat;
	int	beam_port, beam_starboard;
	double	denom;
	double	dx,  dy, distance, angle;
	double	headingx, headingy;
	int	count;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	data = (struct mbf_sbifremr_struct *) mb_io_ptr->raw_data;
	mbfp = mb_io_ptr->mbfp;

	/* initialize beams to zeros */
	for (i=0;i<MBF_SBIFREMR_NUM_BEAMS;i++)
		{
		data->deph[i] = 0;
		data->dist[i] = 0;
		}

	done = MB_NO;
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;
	center = MBF_SBIFREMR_NUM_BEAMS / 2;
	while (done == MB_NO)
		{

		/* get next line */
		if (line_save == MB_NO)
			{
			strncpy(line,"\0",MBF_SBIFREMR_MAXLINE);
			result = fgets(line,MBF_SBIFREMR_MAXLINE,mbfp);
			}
		else
			{
			line_save = MB_NO;
			result = line;
			}

		/* check size of line */
		nchars = strlen(line);

		/* deal with end of file */
		if (result == NULL)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			done = MB_YES;
			}
		
		/* deal with comment */
		else if (nchars > 2 && line[0] == '#' && line[1] == '#')
			{
			strncpy(data->comment, &line[2], MBF_SBIFREMR_MAXLINE-3);
			data->kind = MB_DATA_COMMENT;
			done = MB_YES;
			first = MB_YES;
			}
			
		/* deal with good line */
		else if (nchars > 96)
			{
			/* get ping number */
			mb_get_int    (&ping_num,   line+52,  7);
			mb_get_int    (&beam_num,   line+59,  4);
			beam_num = 19 - beam_num;

			/* check if new ping */
			if (ping_num != ping_num_save && first == MB_NO)
				{
				done = MB_YES;
				line_save = MB_YES;
				first = MB_YES;
				}
			
			/* else convert and store the data */
			else if (beam_num > -1 && beam_num < 19)
				{
				/* parse the line */
				NorS = line[0];
				mb_get_int    (&lat_deg,    line+1,   2);
				mb_get_double (&lat_min,    line+3,   8);
				EorW = line[12];
				mb_get_int    (&lon_deg,    line+13,  3);
				mb_get_double (&lon_min,    line+16,  8);
				mb_get_double (&depth,      line+24, 11);
				mb_get_int    (&day,        line+76,  2);
				mb_get_int    (&month,      line+79,  2);
				mb_get_int    (&year,       line+82,  2);
				mb_get_int    (&hour,       line+85,  2);
				mb_get_int    (&minute,     line+88,  2);
				mb_get_int    (&second,     line+91,  2);
				mb_get_int    (&tsecond,    line+94,  2);

				/* convert data */
				data->kind = MB_DATA_DATA;
				data->lon[beam_num] = 
					lon_deg + lon_min / 60.0;
				if (EorW == 'W')
					data->lon[beam_num] 
						= -1.0 * data->lon[beam_num];
				data->lat[beam_num] = 
					lat_deg + lat_min / 60.0;
				if (NorS == 'S')
					data->lat[beam_num] 
						= -1.0 * data->lat[beam_num];
				data->deph[beam_num] = -depth;
				first = MB_NO;
				ping_num_save = ping_num;
				}
			}
		}
		
	/* if success convert the data */
	if (status == MB_SUCCESS && data->kind == MB_DATA_DATA)
		{
		/* do time */
		time_i[0] = year + 1900;
		time_i[1] = month;
		time_i[2] = day;
		time_i[3] = hour;
		time_i[4] = minute;
		time_i[5] = second;
		time_i[6] = 0;
		mb_get_jtime(verbose,time_i,time_j);
		data->year = time_j[0];
		data->day = time_j[1];
		data->min = time_j[2];
		data->sec = time_j[3];
		
		/* do nav */
		data->lon2u = (short int) 60.0 * data->lon[center];
		data->lon2b = (short int) (600000.0 
			* (data->lon[center] - data->lon2u / 60.0));
		data->lat2u = (short int) 60.0 * (90.0 + data->lat[center]);
		data->lat2b = (short int) (600000.0
			* (data->lat[center] + 90.0 - data->lat2u/60.0));
			
		/* get coordinate scaling */
		mb_coor_scale(verbose,data->lat[center],&mtodeglon,&mtodeglat);
		
		/* find port-most and starboard-most beams */
		beam_port = MBF_SBIFREMR_NUM_BEAMS;
		beam_starboard = -1;
		for (i=0;i<MBF_SBIFREMR_NUM_BEAMS;i++)
			{
			if (data->deph[i] != 0)
				{
				if (beam_port > i)
					beam_port = i;
				if (beam_starboard < i)
					beam_starboard = i;
				}
			}
		if (beam_starboard > beam_port)
			{
			dx = (data->lon[beam_port] - data->lon[beam_starboard])
				/ mtodeglon;
			dy = (data->lat[beam_port] - data->lat[beam_starboard])
				/ mtodeglat;
			denom = sqrt(dx * dx + dy * dy);
			if (denom > 0.0)
				{
				dx = dx/denom;
				dy = dy/denom;
				heading = RTD*atan2(dx,dy) - 90.0;
				if (heading < 0.0)
					heading = heading + 360.0;
				if (heading > 360.0)
					heading = heading - 360.0;
				}
			else
				heading = heading_save;
			}
		else
			heading = heading_save;
		
			
		/* do heading */
		heading_save = heading;
		data->sbhdg = (short int) (heading * 182.044444);
		
		/* if needed try to get center beam lon and lat */
		if (data->deph[center] == 0)
			{
/* the code below commented out because it does not work well enough
	- simply ignore pings without center beams */
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;

/*			headingx = sin(heading * DTR);
			headingy = cos(heading * DTR);
			data->lon[center] = 0.0;
			data->lat[center] = 0.0;
			count = 0;
			for (i=0;i<MBF_SBIFREMR_NUM_BEAMS;i++)
				{
				if (data->deph[i] != 0)
					{
					depth = fabs((double)data->deph[i]);
					angle = DTR * fabs(MBF_SBIFREMR_ANGLE_SPACING * (i - center));
					distance = depth * tan(angle); 
					data->lon[center] += 
					    data->lon[i] 
					    - mtodeglon * headingy 
					    * distance;
					data->lat[center] += 
					    data->lat[i] 
					    + mtodeglat * headingx 
					    * distance;
					count++;
					}
				}
			if (count > 0)
				{
				data->lon[center] = 
					data->lon[center] / count;
				data->lat[center] = 
					data->lat[center] / count;
				}
			else
				{
				status = MB_FAILURE;
				*error = MB_ERROR_UNINTELLIGIBLE;
				}
*/
/* end code commented out */
			}
		
		/* do acrosstrack distances */
		if (status == MB_SUCCESS)
			{
			for (i=0;i<MBF_SBIFREMR_NUM_BEAMS;i++)
			    if (data->deph[i] != 0)
			    {
			    dx = (data->lon[i] - data->lon[center]) 
				    / mtodeglon;
			    dy = (data->lat[i] - data->lat[center]) 
				    / mtodeglat;
			    distance = sqrt(dx * dx + dy * dy);
			    if (i > center)
				distance = -distance;
			    data->dist[i] = distance; 
			    }
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_sbifremr_wr_data(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
	char	*function_name = "mbr_sbifremr_wr_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_sbifremr_struct *data;
	FILE	*mbfp;
	int	done;
	static char line[MBF_SBIFREMR_MAXLINE];
	static int  line_save = MB_NO;
	char	*result;
	int	nchars;
	char	NorS, EorW;
	int	lon_deg, lat_deg;
	double	lon, lat, lon_min, lat_min;
	double	depth;
	double  heading;
	static int ping_num_save = 0;
	static int sounding_num_save = 0;
	int	ping_num;
	int	beam_num;
	int	day, month, year, hour, minute, second, tsecond;
	int	time_i[7], time_j[4];
	static double	heading_save;
	int	center;
	double	mtodeglon, mtodeglat;
	double	headingx, headingy;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	data = (struct mbf_sbifremr_struct *) mb_io_ptr->raw_data;
	mbfp = mb_io_ptr->mbfp;

	/* write comment */
	if (data->kind == MB_DATA_COMMENT)
		{
		/* output the line */
		status = fprintf(mbfp,"##%s\n",data->comment);
		if (status >= 0)
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		else
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		}

	/* write data */
	else if (data->kind == MB_DATA_DATA)
		{
		/* increment ping counter */
		ping_num_save++;
		
		/* get time */
		time_j[0] = data->year;
		time_j[1] = data->day;
		time_j[2] = data->min;
		time_j[3] = data->sec;
		time_j[4] = 0;
		mb_get_itime(verbose,time_j,time_i);
		year = time_i[0] - 1900;
		month = time_i[1];
		day = time_i[2];
		hour = time_i[3];
		minute = time_i[4];
		second = time_i[5];
		tsecond = 0;
		
		/* get lon lat */
		lon = data->lon2u/60. + data->lon2b/600000.;
		lat = data->lat2u/60. + data->lat2b/600000. - 90.;
		if (lon > 180.0)
			lon = lon - 360.0;
		else if (lon < -180.0)
			lon = lon + 360.0;
		
		/* get coordinate scaling */
		heading = 0.0054932 * data->sbhdg;
		data->sbhdg = (short int) (heading * 182.044444);
		mb_coor_scale(verbose,lat,&mtodeglon,&mtodeglat);
		headingx = sin(heading * DTR);
		headingy = cos(heading * DTR);
		
		/* write beams */
		for (i=0;i<MBF_SBIFREMR_NUM_BEAMS;i++)
		    {
		    if (data->deph[i] != 0)
			{
			/* increment sounding counter */
			sounding_num_save++;
			
			/* get lon lat for beam */
			data->lon[i] = lon + headingy * mtodeglon
					* data->dist[i];
			data->lat[i] = lat - headingx * mtodeglat
					*data->dist[i];
			
			/* get printing values */
			beam_num = 19 - i;
			if (data->lon[i] > 180.0)
				data->lon[i] = data->lon[i] - 360.0;
			else if (data->lon[i] < -180.0)
				data->lon[i] = data->lon[i] + 360.0;
			if (data->lon[i] < 0.0)
				{
				EorW = 'W';
				data->lon[i] = -data->lon[i];
				}
			else
				EorW = 'E';
			lon_deg = (int) data->lon[i];
			lon_min = (data->lon[i] - lon_deg) * 60.0;
			if (data->lat[i] < 0.0)
				{
				NorS = 'S';
				data->lat[i] = -data->lat[i];
				}
			else
				NorS = 'N';
			lat_deg = (int) data->lat[i];
			lat_min = (data->lat[i] - lat_deg) * 60.0;
			
			/* print out beam */
			fprintf(mbfp, "%c%2.2d%8.4f %c%3.3d%8.4f", 
				NorS, lat_deg, lat_min, 
				EorW, lon_deg, lon_min);
			depth = -data->deph[i];
			fprintf(mbfp,"%11.3f ****************", 
				depth);
			fprintf(mbfp, "%7d%4d%7d    0 ", 
				ping_num_save, beam_num, sounding_num_save);
			fprintf(mbfp, "%2d/%2d/%2d %2dh%2dm%2ds00\n", 
				day, month, year, hour, minute, second);
			}
		    }
		}

	/* else fail */
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_KIND;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
