/*--------------------------------------------------------------------
 *    The MB-system:	mb7k2ss.c		8/15/2007
 *    $Id$
 *
 *    Copyright (c) 2007-2009 by
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
 * mb7k2ss extracts Edgetech sidescan data
 * from Reson 7k format data, lays the sidescan on the bottom, 
 * and outputs in format 71 mbldeoih.
 *
 * Author:	D. W. Caress
 * Date:	August 15, 2007
 *              R/V Atlantis, Axial Seamount
 *
 * $Log: mb7k2ss.c,v $
 * Revision 5.4  2008/11/16 21:51:18  caress
 * Updating all recent changes, including time lag analysis using mbeditviz and improvements to the mbgrid footprint gridding algorithm.
 *
 * Revision 5.3  2008/09/27 03:27:11  caress
 * Working towards release 5.1.1beta24
 *
 * Revision 5.2  2008/09/11 20:20:14  caress
 * Checking in updates made during cruise AT15-36.
 *
 * Revision 5.1  2008/05/16 22:44:37  caress
 * Release 5.1.1beta18
 *
 * Revision 5.0  2007/10/08 17:04:13  caress
 * Update version number.
 *
 * Revision 1.1  2007/10/08 16:51:01  caress
 * Initial version of a program to extract Edgetech sidescan from MBARI Mapping AUV data into format 71.
 *
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

/* MBIO include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_define.h"
#include "../../include/mb_io.h"
#include "../../include/mb_aux.h"
#include "../../include/mbsys_reson7k.h"
#include "../../include/mbsys_ldeoih.h"

/* GMT include files */
#include "gmt.h"

/* get NaN detector */
#if defined(isnanf)
#define check_fnan(x) isnanf((x))
#elif defined(isnan)
#define check_fnan(x) isnan((double)(x))
#elif HAVE_ISNANF == 1
#define check_fnan(x) isnanf(x)
extern int isnanf(float x);
#elif HAVE_ISNAN == 1
#define check_fnan(x) isnan((double)(x))
#elif HAVE_ISNAND == 1
#define check_fnan(x) isnand((double)(x))
#else
#define check_fnan(x) ((x) != (x))
#endif

/* local defines */
#define	MB7K2SS_SS_FLAT_BOTTOM			0
#define	MB7K2SS_SS_3D_BOTTOM			1
#define	MB7K2SS_SSDIMENSION			4001
#define	MB7K2SS_SSLOW				1
#define	MB7K2SS_SSHIGH				2

#define	MB7K2SS_BOTTOMPICK_NONE			0
#define	MB7K2SS_BOTTOMPICK_BATHYMETRY		1
#define	MB7K2SS_BOTTOMPICK_ALTITUDE		2
#define	MB7K2SS_BOTTOMPICK_ARRIVAL		3
#define	MB7K2SS_BOTTOMPICK_3DBATHY		4

#define	MB7K2SS_SSGAIN_OFF			0
#define	MB7K2SS_SSGAIN_TVG_1OVERR		1

#define MB7K2SS_ALLOC_NUM				128

#define MB7K2SS_ROUTE_WAYPOINT_NONE		0
#define MB7K2SS_ROUTE_WAYPOINT_SIMPLE		1
#define MB7K2SS_ROUTE_WAYPOINT_TRANSIT		2
#define MB7K2SS_ROUTE_WAYPOINT_STARTLINE		3
#define MB7K2SS_ROUTE_WAYPOINT_ENDLINE		4
#define MB7K2SS_ONLINE_THRESHOLD			15.0
#define MB7K2SS_ONLINE_COUNT			30

#define	MB7K2SS_NUM_ANGLES			171
#define	MB7K2SS_ANGLE_MAX			85.0

struct mb7k2ss_grid_struct 
	{
	mb_path	file;
        mb_path projectionname;
	int	projection_mode;
	mb_path	projection_id;
	float	nodatavalue;
	int	nxy;
	int	nx;
	int	ny;
	double	min;
	double	max;
	double	xmin;
	double	xmax;
	double	ymin;
	double	ymax;
	double	dx;
	double	dy;
	float	*data;
	};

int mb7k2ss_get_flatbottom_table(int verbose, int nangle, double angle_min, double angle_max, 
					double navlon, double navlat, double altitude, double pitch,
					double *table_xtrack, double *table_ltrack, 
					double *table_altitude, double *table_range,
					int *error);
int mb7k2ss_get_3Dbottom_table(int verbose, int nangle, double angle_min, double angle_max, 
					double navlon, double navlat, double heading,
					double altitude, double sonardepth, double pitch,
					struct mb7k2ss_grid_struct *grid,
					double *table_xtrack, double *table_ltrack, 
					double *table_altitude, double *table_range,
					int *error);
int mb7k2ss_intersect_grid(int verbose, double navlon, double navlat, double altitude, double sonardepth,
					double mtodeglon, double mtodeglat,
					double vx, double vy, double vz,
					struct mb7k2ss_grid_struct *grid,
					double	 *range, int *error);

static char rcs_id[] = "$Id$";
char program_name[] = "mb7k2ss";

/*--------------------------------------------------------------------*/

int main (int argc, char **argv)
{
	char help_message[] =  "mb7k2ss extracts sidescan sonar data from Reson 7k format data, \nbins and lays the sidescan onto the seafloor, and outputs files \nin the MBF_MBLDEOIH formst (MBIO format id 71).\n";
	char usage_message[] = "mb7k2ss [-Ifile -Atype -Bmode[/threshold] -C -Fformat -Lstartline/lineroot -Ooutfile -Rroutefile -Ttopogridfile -X -H -V]";
	extern char *optarg;
	int	errflg = 0;
	int	c;
	int	help = 0;
	int	flag = 0;

	/* MBIO status variables */
	int	status = MB_SUCCESS;
	int	verbose = 0;
	int	error = MB_ERROR_NO_ERROR;
	char	*message;

	/* MBIO read control parameters */
	int	read_datalist = MB_NO;
	mb_path	read_file;
	mb_path	output_file;
	mb_path	current_output_file;
	int	new_output_file = MB_YES;
	int	output_file_set = MB_NO;
	void	*datalist;
	int	look_processed = MB_DATALIST_LOOK_YES;
	double	file_weight;
	int	format = 0;
	int	pings;
	int	lonflip;
	double	bounds[4];
	int	btime_i[7];
	int	etime_i[7];
	double	btime_d;
	double	etime_d;
	double	speedmin;
	double	timegap;
	mb_path	file;
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;

	/* topography parameters */
	struct mb7k2ss_grid_struct grid;

	/* MBIO read values */
	void	*imbio_ptr = NULL;
	struct mb_io_struct *imb_io_ptr = NULL;
	void	*istore_ptr = NULL;
	struct mbsys_reson7k_struct *istore = NULL;
	void	*ombio_ptr = NULL;
	struct mb_io_struct *omb_io_ptr = NULL;
	void	*ostore_ptr = NULL;
	struct mbsys_ldeoih_struct *ostore = NULL;
	int	kind;
	int	time_i[7];
	int	time_j[5];
	double	time_d;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	double	distance;
	double	altitude;
	double	sonardepth;
	double	roll;
	double	pitch;
	double	heave;
	char	*beamflag = NULL;
	double	*bath = NULL;
	double	*bathacrosstrack = NULL;
	double	*bathalongtrack = NULL;
	double	*amp = NULL;
	double	*ss = NULL;
	double	*ssacrosstrack = NULL;
	double	*ssalongtrack = NULL;
	double	*ttimes = NULL;
	double	*angles = NULL;
	double	*angles_forward = NULL;
	double	*angles_null = NULL;
	double	*bheave = NULL;
	double	*alongtrack_offset = NULL;
	double	draft;
	double	ssv;
	double	ssv_use = 1500.0;
	
	char	comment[MB_COMMENT_MAXLINE];
	int	icomment = 0;
	
	/* sidescan data data */
	s7k_fsdwchannel	*sschannelport;		/* Port hannel header and data */
	s7k_fsdwssheader *ssheaderport;		/* Port Edgetech sidescan header */
	s7k_fsdwchannel	*sschannelstbd;		/* Starboard channel header and data */
	s7k_fsdwssheader *ssheaderstbd;		/* Starboard Edgetech sidescan header */
	
	/* output sidescan data */
	int	obeams_bath;
	int	obeams_amp;
	int	opixels_ss;
	double	oss[MB7K2SS_SSDIMENSION];
	double	ossacrosstrack[MB7K2SS_SSDIMENSION];
	double	ossalongtrack[MB7K2SS_SSDIMENSION];
	int	ossbincount[MB7K2SS_SSDIMENSION];
	double	swath_width;
	double	pixel_width;
	
	/* extract modes */
	int	extract_type = MB7K2SS_SSLOW;
	int	target_kind = MB_DATA_SIDESCAN2;
	int	print_comments = MB_NO;
	
	/* bottompick mode */
	int	bottompickmode = MB7K2SS_BOTTOMPICK_ALTITUDE;
	double	bottompickthreshold = 0.4;
	
	/* sidescan layout mode */
	int	sslayoutmode = MB7K2SS_SS_FLAT_BOTTOM;
	double	ss_altitude;
	
	/* sidescan gain mode */
	int	gainmode = MB7K2SS_SSGAIN_OFF;
	double	gainfactor = 1.0;
	int	ssflip = MB_NO;
	
	/* sidescan interpolation scale */
	int	interpbins = 0;
	
	/* route and auto-line data */
	mb_path	route_file;
	int	route_file_set = MB_NO;
	int	checkroutebearing = MB_NO;
	int	rawroutefile = MB_NO;
	mb_path	lineroot;
	int	nroutepoint = 0;
	int	nroutepointalloc = 0;
	double	lon;
	double	lat;
	double	topo;
	int	waypoint;
	double	*routelon = NULL;
	double	*routelat = NULL;
	double	*routeheading = NULL;
	int	*routewaypoint = NULL;
	double	range;
	double	rangethreshold = 50.0;
	double	rangelast;
	int	rangeok = MB_NO;
	int	activewaypoint = 0;
	int	startline = 1;
	int	linenumber;
	
	/* bottom layout parameters */
	int	nangle = MB7K2SS_NUM_ANGLES;
	double	angle_min = -MB7K2SS_ANGLE_MAX;
	double	angle_max = MB7K2SS_ANGLE_MAX;
	double	table_xtrack[MB7K2SS_NUM_ANGLES];
	double	table_ltrack[MB7K2SS_NUM_ANGLES];
	double	table_altitude[MB7K2SS_NUM_ANGLES];
	double	table_range[MB7K2SS_NUM_ANGLES];
	
	/* counting variables */
	int	nreaddata = 0;
	int	nreadheader = 0;
	int	nreadssv = 0;
	int	nreadnav1 = 0;
	int	nreadsbp = 0;
	int	nreadsslo = 0;
	int	nreadsshi = 0;
	int	nwritesslo = 0;
	int	nwritesshi = 0;
	int	nreaddatatot = 0;
	int	nreadheadertot = 0;
	int	nreadssvtot = 0;
	int	nreadnav1tot = 0;
	int	nreadsbptot = 0;
	int	nreadsslotot = 0;
	int	nreadsshitot = 0;
	int	nwritesslotot = 0;
	int	nwritesshitot = 0;
		
	/* auto plotting */
	FILE	*sfp = NULL;
	char	scriptfile[MB_PATH_MAXLINE];
	char	command[MB_PATH_MAXLINE];

	int	mode;
	int	format_status, format_guess;
	int	format_output = MBF_MBLDEOIH;
	unsigned short	*datashort;
	double	value, threshold;
	double	channelmax;
	int	portchannelpick;
	int	stbdchannelpick;
	double	ttime;
	double	ttime_min;
	double	ttime_min_use;
	double	nadir_depth;
	int	istart;
	int	ttime_min_ok = MB_NO;
	int	beam_min;
	int	smooth = 0;
	double	weight;
	double	factor;
	double	mtodeglon, mtodeglat;
	double	lastlon;
	double	lastlat;
	double	lastheading;
	double	headingdiff;
	int	oktowrite;
	double	dx, dy;
	int	kangle, kstart;
	double	xtrack, ltrack, rr;
	FILE	*fp = NULL;
	char	*result;
	int	nget;
	int	point_ok;
	int	previous, jj, interpable;
	double	dss, dssl, fraction;
	
	int	read_data;
	int	found, done;
	int	i, j, n;
	
	startline = 1;
	strcpy(lineroot, "jstar");

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* set default input to datalist.mb-1 */
	strcpy (read_file, "datalist.mb-1");

	/* process argument list */
	while ((c = getopt(argc, argv, "A:a:B:b:CcD:d:F:f:G:g:I:i:L:l:MmO:o:R:r:S:s:T:t:U:u:XxVvHh")) != -1)
	  switch (c) 
		{
		case 'H':
		case 'h':
			help++;
			break;
		case 'V':
		case 'v':
			verbose++;
			break;
		case 'A':
		case 'a':
			if (strncmp(optarg, "SSLOW", 5) == 0
				|| strncmp(optarg, "sslow", 5) == 0)
				{
				extract_type = MB7K2SS_SSLOW;
				target_kind = MB_DATA_SIDESCAN2;
				}
			else if (strncmp(optarg, "SSHIGH", 6) == 0
				|| strncmp(optarg, "sshigh", 6) == 0)
				{
				extract_type = MB7K2SS_SSHIGH;
				target_kind = MB_DATA_SIDESCAN3;
				}
			else
				{
				sscanf (optarg,"%d", &mode);
				if (mode == MB7K2SS_SSLOW)
					{
					extract_type = MB7K2SS_SSLOW;
					target_kind = MB_DATA_SIDESCAN2;
					}
				else if (mode == MB7K2SS_SSHIGH)
					{
					extract_type = MB7K2SS_SSHIGH;
					target_kind = MB_DATA_SIDESCAN3;
					}
				}
			flag++;
			break;
		case 'B':
		case 'b':
			n = sscanf (optarg,"%d/%lf", &bottompickmode, &bottompickthreshold);
			if (n == 0)
				bottompickmode = MB7K2SS_BOTTOMPICK_ALTITUDE;
			else if (n == 1 && bottompickmode == MB7K2SS_BOTTOMPICK_ARRIVAL)
				bottompickthreshold = 0.5;
			flag++;
			break;
		case 'C':
		case 'c':
			print_comments = MB_YES;
			break;
		case 'D':
		case 'd':
			sscanf (optarg,"%d", &interpbins);
			break;
		case 'F':
		case 'f':
			sscanf (optarg,"%d", &format);
			flag++;
			break;
		case 'G':
		case 'g':
			sscanf (optarg,"%d/%lf", &gainmode, &gainfactor);
			flag++;
			break;
		case 'I':
		case 'i':
			sscanf (optarg,"%s", read_file);
			flag++;
			break;
		case 'L':
		case 'l':
			sscanf (optarg,"%d/%s", &startline, lineroot);
			flag++;
			break;
		case 'M':
		case 'm':
			checkroutebearing = MB_YES;
			flag++;
			break;
		case 'O':
		case 'o':
			sscanf (optarg,"%s", output_file);
			output_file_set  = MB_YES;
			flag++;
			break;
		case 'R':
		case 'r':
			sscanf (optarg,"%s", route_file);
			route_file_set = MB_YES;
			flag++;
			break;
		case 'S':
		case 's':
			sscanf (optarg,"%d", &smooth);
			flag++;
			break;
		case 'T':
		case 't':
			sscanf (optarg,"%s", grid.file);
			sslayoutmode = MB7K2SS_SS_3D_BOTTOM;
			flag++;
			break;
		case 'U':
		case 'u':
			sscanf (optarg,"%lf", &rangethreshold);
			flag++;
			break;
		case 'X':
		case 'x':
			ssflip = MB_YES;
			flag++;
			break;
		case '?':
			errflg++;
		}

	/* if error flagged then print it and exit */
	if (errflg)
		{
		fprintf(stderr,"usage: %s\n", usage_message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		error = MB_ERROR_BAD_USAGE;
		exit(error);
		}

	/* print starting message */
	if (verbose == 1 || help)
		{
		fprintf(stderr,"\nProgram %s\n",program_name);
		fprintf(stderr,"Version %s\n",rcs_id);
		fprintf(stderr,"MB-system Version %s\n",MB_VERSION);
		}

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Program <%s>\n",program_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Control Parameters:\n");
		fprintf(stderr,"dbg2       verbose:             %d\n",verbose);
		fprintf(stderr,"dbg2       help:                %d\n",help);
		fprintf(stderr,"dbg2       format:              %d\n",format);
		fprintf(stderr,"dbg2       pings:               %d\n",pings);
		fprintf(stderr,"dbg2       lonflip:             %d\n",lonflip);
		fprintf(stderr,"dbg2       bounds[0]:           %f\n",bounds[0]);
		fprintf(stderr,"dbg2       bounds[1]:           %f\n",bounds[1]);
		fprintf(stderr,"dbg2       bounds[2]:           %f\n",bounds[2]);
		fprintf(stderr,"dbg2       bounds[3]:           %f\n",bounds[3]);
		fprintf(stderr,"dbg2       btime_i[0]:          %d\n",btime_i[0]);
		fprintf(stderr,"dbg2       btime_i[1]:          %d\n",btime_i[1]);
		fprintf(stderr,"dbg2       btime_i[2]:          %d\n",btime_i[2]);
		fprintf(stderr,"dbg2       btime_i[3]:          %d\n",btime_i[3]);
		fprintf(stderr,"dbg2       btime_i[4]:          %d\n",btime_i[4]);
		fprintf(stderr,"dbg2       btime_i[5]:          %d\n",btime_i[5]);
		fprintf(stderr,"dbg2       btime_i[6]:          %d\n",btime_i[6]);
		fprintf(stderr,"dbg2       etime_i[0]:          %d\n",etime_i[0]);
		fprintf(stderr,"dbg2       etime_i[1]:          %d\n",etime_i[1]);
		fprintf(stderr,"dbg2       etime_i[2]:          %d\n",etime_i[2]);
		fprintf(stderr,"dbg2       etime_i[3]:          %d\n",etime_i[3]);
		fprintf(stderr,"dbg2       etime_i[4]:          %d\n",etime_i[4]);
		fprintf(stderr,"dbg2       etime_i[5]:          %d\n",etime_i[5]);
		fprintf(stderr,"dbg2       etime_i[6]:          %d\n",etime_i[6]);
		fprintf(stderr,"dbg2       speedmin:            %f\n",speedmin);
		fprintf(stderr,"dbg2       timegap:             %f\n",timegap);
		fprintf(stderr,"dbg2       bottompickmode:      %d\n",bottompickmode);
		fprintf(stderr,"dbg2       bottompickthreshold: %f\n",bottompickthreshold);
		fprintf(stderr,"dbg2       smooth:              %d\n",smooth);
		fprintf(stderr,"dbg2       interpbins:          %d\n",interpbins);
		fprintf(stderr,"dbg2       gainmode:            %d\n",gainmode);
		fprintf(stderr,"dbg2       gainfactor:          %f\n",gainfactor);
		fprintf(stderr,"dbg2       sslayoutmode:        %d\n",sslayoutmode);
		fprintf(stderr,"dbg2       grid.file:           %s\n",grid.file);
		fprintf(stderr,"dbg2       file:                %s\n",file);
		fprintf(stderr,"dbg2       route_file_set:      %d\n",route_file_set);
		fprintf(stderr,"dbg2       route_file:          %s\n",route_file);
		fprintf(stderr,"dbg2       checkroutebearing:   %d\n",checkroutebearing);
		fprintf(stderr,"dbg2       output_file:         %s\n",output_file);
		fprintf(stderr,"dbg2       output_file_set:     %d\n",output_file_set);
		fprintf(stderr,"dbg2       lineroot:            %s\n",lineroot);
		fprintf(stderr,"dbg2       extract_type:        %d\n",extract_type);
		fprintf(stderr,"dbg2       print_comments:      %d\n",print_comments);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(status);
		}
	
	/* output output types */
	fprintf(stdout, "\nData records to extract:\n");
	if (extract_type == MB7K2SS_SSLOW)
		fprintf(stdout, "     Low Sidescan\n");
	else if (extract_type == MB7K2SS_SSHIGH)
		fprintf(stdout, "     High Sidescan\n");
	if (ssflip == MB_YES)
		fprintf(stdout, "     Sidescan port and starboard exchanged\n");
		
	/* set starting line number and output file if route read */
	if (route_file_set == MB_YES)
		{
		linenumber = startline;
		if (extract_type == MB7K2SS_SSLOW)
			sprintf(output_file, "%s_%4.4d_sslo.mb71", lineroot, linenumber);
		else if (extract_type == MB7K2SS_SSHIGH)
			sprintf(output_file, "%s_%4.4d_sshi.mb71", lineroot, linenumber);
		}
	
	/* new output file obviously needed */
	new_output_file = MB_YES;

	/* if specified read route file */
	if (route_file_set == MB_YES)
		{	    
		/* open the input file */
		if ((fp = fopen(route_file, "r")) == NULL) 
			{
			error = MB_ERROR_OPEN_FAIL;
			status = MB_FAILURE;
			fprintf(stderr,"\nUnable to open route file <%s> for reading\n",route_file);
			exit(status);
			}
		rawroutefile = MB_NO;
		while ((result = fgets(comment,MB_PATH_MAXLINE,fp)) == comment)
		    	{
			if (comment[0] == '#')
				{
				if (strncmp(comment,"## Route File Version", 21) == 0)
					{
					rawroutefile = MB_NO;
					}
				}
			else
				{
				nget = sscanf(comment,"%lf %lf %lf %d %lf",
				    &lon, &lat, &topo, &waypoint, &heading);
				if (comment[0] == '#')
					{
					fprintf(stderr,"buffer:%s",comment);
					if (strncmp(comment,"## Route File Version", 21) == 0)
						{
						rawroutefile = MB_NO;
						}
					}
		    		if ((rawroutefile == MB_YES && nget >= 2)
					|| (rawroutefile == MB_NO && nget >= 3 && waypoint > MB7K2SS_ROUTE_WAYPOINT_NONE))
					point_ok = MB_YES;
				else
					point_ok = MB_NO;
			    
				/* if good data check for need to allocate more space */
				if (point_ok == MB_YES
					&& nroutepoint + 1 > nroutepointalloc)
				    	{
				    	nroutepointalloc += MB7K2SS_ALLOC_NUM;
					status = mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(double),
								(void **)&routelon, &error);
					status = mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(double),
								(void **)&routelat, &error);
					status = mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(double),
								(void **)&routeheading, &error);
					status = mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(int),
								(void **)&routewaypoint, &error);
				    	if (status != MB_SUCCESS)
					    	{
						mb_error(verbose,error,&message);
						fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",
							message);
						fprintf(stderr,"\nProgram <%s> Terminated\n",
							program_name);
						exit(error);
					    	}
				    	}

				/* add good point to route */
				if (point_ok == MB_YES && nroutepointalloc > nroutepoint + 1)
					{
					routelon[nroutepoint] = lon;
					routelat[nroutepoint] = lat;
					routeheading[nroutepoint] = heading;
					routewaypoint[nroutepoint] = waypoint;
					nroutepoint++;
					}
				}
			}

		/* close the file */
		fclose(fp);
		fp = NULL;
		
		/* set starting values */
		activewaypoint = 1;
		mb_coor_scale(verbose,routelat[activewaypoint], &mtodeglon, &mtodeglat);
		rangelast = 1000 * rangethreshold;
		oktowrite = 0;
		rangeok = MB_NO;

		/* output status */
		if (verbose > 0)
			{
			/* output info on file output */
			fprintf(stderr,"\nImported %d waypoints from route file: %s\n",
				nroutepoint, route_file);
			}
		}
		
	/* read topography grid if 3D bottom correction specified */
	if (sslayoutmode == MB7K2SS_SS_3D_BOTTOM)
		{
		grid.data = NULL;
		status = mb_readgrd(verbose, grid.file, &grid.projection_mode, grid.projection_id, &grid.nodatavalue,
					&grid.nxy, &grid.nx, &grid.ny, &grid.min, &grid.max,
					&grid.xmin, &grid.xmax, &grid.ymin, &grid.ymax,
					&grid.dx, &grid.dy, &grid.data, NULL, NULL, &error);
		if (status == MB_FAILURE) 
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to read grd file: %s\n",
			    grid.file);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
			    program_name);
			exit(error);
			}

		/* rationalize grid bounds and lonflip */
		if (lonflip == -1)
			{
			if (grid.xmax > 180.0)
				{
				grid.xmin -= 360.0;
				grid.xmax -= 360.0;
				}
			}
		else if (lonflip == 0)
			{
			if (grid.xmin > 180.0)
				{
				grid.xmin -= 360.0;
				grid.xmax -= 360.0;
				}
			else if (grid.xmax < -180.0)
				{
				grid.xmin += 360.0;
				grid.xmax += 360.0;
				}
			}
		else if (lonflip == 1)
			{
			if (grid.xmin < -180.0)
				{
				grid.xmin += 360.0;
				grid.xmax += 360.0;
				}
			}
		if (grid.xmax > 180.0)
			{
			lonflip = 1;
			}
		else if (grid.xmin < -180.0)
			{
			lonflip = -1;
			}
		else
			{
			lonflip = 0;
			}
		}

	/* get format if required */
	if (format == 0)
		mb_get_format(verbose,read_file,NULL,&format,&error);

	/* determine whether to read one file or a list of files */
	if (format < 0)
		read_datalist = MB_YES;

	/* open file list */
	if (read_datalist == MB_YES)
	    {
	    if ((status = mb_datalist_open(verbose,&datalist,
					    read_file,look_processed,&error)) != MB_SUCCESS)
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr,"\nUnable to open data list file: %s\n",
			read_file);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	    if ((status = mb_datalist_read(verbose,datalist,
			    file,&format,&file_weight,&error))
			    == MB_SUCCESS)
		read_data = MB_YES;
	    else
		read_data = MB_NO;
	    }
	/* else copy single filename to be read */
	else
	    {
	    strcpy(file, read_file);
	    read_data = MB_YES;
	    }
	    
	/* set up plotting script file */
	if (route_file_set == MB_YES && nroutepoint > 1)
		{
		sprintf(scriptfile, "%s_ssswathplot.cmd", lineroot);
		}
	else if (output_file_set == MB_NO || read_datalist == MB_YES)
		{
		sprintf(scriptfile, "%s_ssswathplot.cmd", read_file);
		}
	else
		{
		sprintf(scriptfile, "%s_ssswathplot.cmd", file);
		}
	if ((sfp = fopen(scriptfile, "w")) == NULL) 
		{
		error = MB_ERROR_OPEN_FAIL;
		status = MB_FAILURE;
		fprintf(stderr,"\nUnable to open plotting script file <%s> \n",scriptfile);
		exit(status);
		}
	
	/* loop over all files to be read */
	while (read_data == MB_YES && format == MBF_RESON7KR)
	{

	/* initialize reading the swath file */
	if ((status = mb_read_init(
		verbose,file,format,pings,lonflip,bounds,
		btime_i,etime_i,speedmin,timegap,
		&imbio_ptr,&btime_d,&etime_d,
		&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
		fprintf(stderr,"\nMultibeam File <%s> not initialized for reading\n",file);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* get pointers to data storage */
	imb_io_ptr = (struct mb_io_struct *) imbio_ptr;
	istore_ptr = imb_io_ptr->store_data;
	istore = (struct mbsys_reson7k_struct *) istore_ptr;

	if (error == MB_ERROR_NO_ERROR)
		{
		beamflag = NULL;
		bath = NULL;
		amp = NULL;
		bathacrosstrack = NULL;
		bathalongtrack = NULL;
		ss = NULL;
		ssacrosstrack = NULL;
		ssalongtrack = NULL;
		}
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(char), (void **)&beamflag, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&bath, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_AMPLITUDE,
						sizeof(double), (void **)&amp, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&bathacrosstrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&bathalongtrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, 
						sizeof(double), (void **)&ss, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, 
						sizeof(double), (void **)&ssacrosstrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, 
						sizeof(double), (void **)&ssalongtrack, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&ttimes, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&angles, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&angles_forward, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&angles_null, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&bheave, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY,
						sizeof(double), (void **)&alongtrack_offset, &error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",
			message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
		
	/* set up output file name if needed */
	if (error == MB_ERROR_NO_ERROR)
		{
		if (output_file_set == MB_YES && ombio_ptr == NULL)
			{
			/* set flag to open new output file */
			new_output_file = MB_YES;
			}
			
		else if (output_file_set == MB_NO && route_file_set == MB_NO)
			{
			new_output_file = MB_YES;
			format_status = mb_get_format(verbose, file, output_file, 
		    				&format_guess, &error);
			if (format_status != MB_SUCCESS || format_guess != format)
				{
				strcpy(output_file, file);
				}
			if (output_file[strlen(output_file)-1] == 'p')
				{
				output_file[strlen(output_file)-1] = '\0';
				}
			if (extract_type == MB7K2SS_SSLOW)
				{
				strcat(output_file,"_sslo.mb71");
				format_output = MBF_MBLDEOIH;
				}
			else if (extract_type == MB7K2SS_SSHIGH)
				{
				strcat(output_file,"_sshi.mb71");
				format_output = MBF_MBLDEOIH;
				}
			}
		}

	/* read and print data */
	nreaddata = 0;
	nreadheader = 0;
	nreadssv = 0;
	nreadnav1 = 0;
	nreadsbp = 0;
	nreadsslo = 0;
	nreadsshi = 0;
	ttime_min_ok = MB_NO;

	while (error <= MB_ERROR_NO_ERROR)
		{
		/* reset error */
		error = MB_ERROR_NO_ERROR;
		
		/* read next data record */
		status = mb_get_all(verbose,imbio_ptr,&istore_ptr,&kind,
				    time_i,&time_d,&navlon,&navlat,
				    &speed,&heading,
				    &distance,&altitude,&sonardepth,
				    &beams_bath,&beams_amp,&pixels_ss,
				    beamflag,bath,amp,bathacrosstrack,bathalongtrack,
				    ss,ssacrosstrack,ssalongtrack,
				    comment,&error);
/*fprintf(stderr,"kind:%d %s \n\ttime_i:%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d  %f    time_i:%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d  %f\n",
kind,notice_msg[kind],time_i[0],time_i[1],time_i[2],time_i[3],time_i[4],time_i[5],time_i[6],time_d,
istore->time_i[0],istore->time_i[1],istore->time_i[2],istore->time_i[3],istore->time_i[4],istore->time_i[5],istore->time_i[6],istore->time_d);*/

		/* reset nonfatal errors */
		if (kind == MB_DATA_DATA && error < 0)
			{
			status = MB_SUCCESS;
			error = MB_ERROR_NO_ERROR;
			}

		/* get nav and attitude */
		if (status == MB_SUCCESS
			&& (kind == MB_DATA_SUBBOTTOM_SUBBOTTOM
				|| kind == MB_DATA_DATA
				|| kind == MB_DATA_SIDESCAN2
				|| kind == MB_DATA_SIDESCAN3))
			{
/*for (i=MAX(0,imb_io_ptr->nfix-5);i<imb_io_ptr->nfix;i++)
fprintf(stderr,"dbg2       nav fix[%2d]:   %f %f %f\n",
i, imb_io_ptr->fix_time_d[i],
imb_io_ptr->fix_lon[i],
imb_io_ptr->fix_lat[i]);*/
			mb_get_jtime(verbose, istore->time_i, time_j);
			speed = 0.0;
			mb_hedint_interp(verbose, imbio_ptr, time_d,  
					&heading, &error);
			mb_navint_interp(verbose, imbio_ptr, time_d, heading, speed, 
					&navlon, &navlat, &speed, &error);
/*fprintf(stderr,"time:%4d/%2d/%2d-%2d:%2d:%2d.%6d navlon:%f navlat:%f\n",
time_i[0],time_i[1],time_i[2],time_i[3],time_i[4],time_i[5],time_i[6],navlon,navlat);*/
			mb_depint_interp(verbose, imbio_ptr, time_d,  
					&sonardepth, &error);
			mb_altint_interp(verbose, imbio_ptr, time_d,  
					&altitude, &error);
			mb_attint_interp(verbose, imbio_ptr, time_d,  
					&heave, &roll, &pitch, &error);
			}

		/* save last nav and heading */
		if (status == MB_SUCCESS && kind == target_kind)
			{
			if (navlon != 0.0)
				lastlon = navlon;
			if (navlat != 0.0)
				lastlat = navlat;
			if (heading != 0.0)
				lastheading = heading;
			}
		    
		/* check survey data position against waypoints */
/*fprintf(stderr,"status:%d error:%d | kind:%d %d | route_file_set:%d nroutepoint:%d navlon:%f navlat:%f\n",
status,error,kind,target_kind,route_file_set,nroutepoint,navlon,navlat);*/
		if (status == MB_SUCCESS && kind == target_kind
			&& route_file_set == MB_YES && nroutepoint > 1 
			&& navlon != 0.0 && navlat != 0.0)
			{
			dx = (navlon - routelon[activewaypoint]) / mtodeglon;
			dy = (navlat - routelat[activewaypoint]) / mtodeglat;
			range = sqrt(dx * dx + dy * dy);
/*fprintf(stderr,"activewaypoint:%d range:%f rangelast:%f lon:%f %f lat:%f %f\n",
activewaypoint, range, rangelast, navlon, routelon[activewaypoint], navlat, routelat[activewaypoint]);*/
			if (range < rangethreshold)
				rangeok = MB_YES;
			if (rangeok == MB_YES 
				&& (activewaypoint == 0 || range > rangelast) 
				&& activewaypoint < nroutepoint - 1)
				{
/*fprintf(stderr,"New output by range to routepoint: %f\n",range);*/
				/* if needed set flag to open new output file */
				if (new_output_file == MB_NO)
				    {
				    /* increment line number */
				    linenumber++;
				    
				    /* set output file name */
				    if (extract_type == MB7K2SS_SSLOW)
					    sprintf(output_file, "%s_%4.4d_sslo.mb71", lineroot, linenumber);
				    else if (extract_type == MB7K2SS_SSHIGH)
					    sprintf(output_file, "%s_%4.4d_sshi.mb71", lineroot, linenumber);
			   	    format_output = MBF_MBLDEOIH;
				    
				    /* set to open new output file */
				    new_output_file = MB_YES;
				    }

				/* increment active waypoint */
				activewaypoint++;
				mb_coor_scale(verbose,routelat[activewaypoint], &mtodeglon, &mtodeglat);
				rangelast = 1000 * rangethreshold;
				oktowrite = 0;
				rangeok = MB_NO;
				}
			else
				rangelast = range;
/*fprintf(stderr,"> activewaypoint:%d linenumber:%d range:%f   lon: %f %f   lat: %f %f oktowrite:%d\n", 
activewaypoint,linenumber,range, navlon, 
routelon[activewaypoint], navlat, routelat[activewaypoint], oktowrite);*/
			}
				
		if (kind == MB_DATA_DATA 
			&& error <= MB_ERROR_NO_ERROR)
			{
			/* extract travel times */
			status = mb_ttimes(verbose,imbio_ptr,
				istore_ptr,&kind,&beams_bath,
				ttimes,angles,
				angles_forward,angles_null,
				bheave,alongtrack_offset,
				&draft,&ssv,&error);
				
			/* check surface sound velocity */
			if (ssv > 0.0)
				ssv_use = ssv;
				
			/* get bottom arrival time, if possible */
			ttime_min = 0.0;
			found = MB_NO;
			for (i=0;i<beams_bath;i++)
				{
				if (mb_beam_ok(beamflag[i]))
					{
					if (found == MB_NO || ttimes[i] < ttime_min)
						{
						ttime_min = ttimes[i];
						nadir_depth = bath[i];
						beam_min = i;
						found = MB_YES;
						}
					}
				}
			if (found == MB_YES)
				{
				ttime_min_use = ttime_min;
				ttime_min_ok = MB_YES;
				}
			}
		    
		/* nonfatal errors do not matter */
		if (error < MB_ERROR_NO_ERROR)
			{
			error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
			
		/* if needed open new output file */
		if (status == MB_SUCCESS
			&& new_output_file == MB_YES
			&& (((extract_type == MB7K2SS_SSLOW && kind == MB_DATA_SIDESCAN2)
				|| (extract_type == MB7K2SS_SSHIGH && kind == MB_DATA_SIDESCAN3))))
			{
				
			/* close any old output file unless a single file has been specified */
			if (ombio_ptr != NULL)
				{
				/* close the swath file */
				status = mb_close(verbose,&ombio_ptr,&error);

				/* generate inf file */
				if (status == MB_SUCCESS)
					{
					status = mb_make_info(verbose, MB_YES, 
								current_output_file, 
								format_output, 
								&error);
					}
					
				/* output counts */
				fprintf(stdout, "\nData records written to: %s\n", current_output_file);
				fprintf(stdout, "     Low Sidescan:  %d\n", nwritesslo);
				fprintf(stdout, "     High Sidescan: %d\n", nwritesshi);
				nwritesslotot += nwritesslo;
				nwritesshitot += nwritesshi;
				    
				/* output commands to first cut plotting script file */
				fprintf(sfp, "# Generate swath plot of sidescan file: %s\n", current_output_file);
				fprintf(sfp, "mbm_plot -I %s -N -G5 -S -Pb -V -O %s_ssrawplot\n", 
					current_output_file, current_output_file);
				fprintf(sfp, "%s_ssrawplot.cmd\n\n", current_output_file);
				}
				
			/* open the new file */
			nwritesslo = 0;
			nwritesshi = 0;
			if ((status = mb_write_init(
				verbose,output_file,MBF_MBLDEOIH,
				&ombio_ptr,&obeams_bath,&obeams_amp,&opixels_ss,&error)) != MB_SUCCESS)
				{
				mb_error(verbose,error,&message);
				fprintf(stderr,"\nMBIO Error returned from function <mb_write_init>:\n%s\n",message);
				fprintf(stderr,"\nMultibeam File <%s> not initialized for writing\n",file);
				fprintf(stderr,"\nProgram <%s> Terminated\n",
					program_name);
				exit(error);
				}
				
			/* save current_output_file */
			strcpy(current_output_file, output_file);

			/* get pointers to data storage */
			omb_io_ptr = (struct mb_io_struct *) ombio_ptr;
			ostore_ptr = omb_io_ptr->store_data;
			ostore = (struct mbsys_ldeoih_struct *) ostore_ptr;
			
			/* reset new_output_file */
			new_output_file = MB_NO;
			}
			
		/* if following a route check that the vehicle has come on line 
		    	(within MB7K2SS_ONLINE_THRESHOLD degrees)
		    	before writing any data */
		if (checkroutebearing == MB_YES 
		    	&& nroutepoint > 1 && activewaypoint > 0)
		    	{
			headingdiff = fabs(routeheading[activewaypoint-1] - heading);
			if (headingdiff > 180.0)
				headingdiff = 360.0 - headingdiff;
			if (headingdiff < MB7K2SS_ONLINE_THRESHOLD)
				oktowrite++;
			else
				oktowrite = 0;
/* fprintf(stderr,"heading: %f %f %f oktowrite:%d\n", 
routeheading[activewaypoint-1],heading,headingdiff,oktowrite);*/
			}
		else
		    	oktowrite = MB7K2SS_ONLINE_COUNT;
/* if (status == MB_SUCCESS)
fprintf(stderr,"activewaypoint:%d linenumber:%d range:%f   lon: %f %f   lat: %f %f oktowrite:%d\n", 
activewaypoint,linenumber,range, navlon, 
routelon[activewaypoint], navlat, routelat[activewaypoint], oktowrite);*/
			
	   	/* handle multibeam data */
		if (status == MB_SUCCESS && kind == MB_DATA_DATA) 
			{
/*fprintf(stderr,"MB_DATA_DATA: status:%d error:%d kind:%d\n",status,error,kind);*/
			nreaddata++;
			}
			
	   	/* handle file header data */
		else if (status == MB_SUCCESS && kind == MB_DATA_HEADER) 
			{
/*fprintf(stderr,"MB_DATA_HEADER: status:%d error:%d kind:%d\n",status,error,kind);*/
			nreadheader++;
			}
			
	   	/* handle bluefin ctd data */
		else if (status == MB_SUCCESS && kind == MB_DATA_SSV) 
			{
/*fprintf(stderr,"MB_DATA_SSV: status:%d error:%d kind:%d\n",status,error,kind);*/
			nreadssv++;
			}
			
	   	/* handle bluefin nav data */
		else if (status == MB_SUCCESS && kind == MB_DATA_NAV2) 
			{
/*fprintf(stderr,"MB_DATA_NAV1: status:%d error:%d kind:%d\n",status,error,kind);*/
			nreadnav1++;
			}
			
	   	/* handle subbottom data */
		else if (status == MB_SUCCESS && kind == MB_DATA_SUBBOTTOM_SUBBOTTOM) 
			{
/*fprintf(stderr,"MB_DATA_SUBBOTTOM_SUBBOTTOM: status:%d error:%d kind:%d\n",status,error,kind);*/
			nreadsbp++;
			}
			
	   	/* handle low frequency sidescan data */
		else if (status == MB_SUCCESS && kind == MB_DATA_SIDESCAN2) 
			{
/*fprintf(stderr,"MB_DATA_SIDESCAN2: status:%d error:%d kind:%d\n",status,error,kind);*/
			nreadsslo++;
			
			/* output data if desired */
			if (extract_type == MB7K2SS_SSLOW && nreadnav1 > 0 && oktowrite >= MB7K2SS_ONLINE_COUNT)
				{
				/* get channels */
				if (ssflip == MB_YES)
					{
					sschannelport = (s7k_fsdwchannel *) &(istore->fsdwsslo.channel[1]);
					ssheaderport = (s7k_fsdwssheader *) &(istore->fsdwsslo.ssheader[1]);
					sschannelstbd = (s7k_fsdwchannel *) &(istore->fsdwsslo.channel[0]);
					ssheaderstbd = (s7k_fsdwssheader *) &(istore->fsdwsslo.ssheader[0]);
					}
				else
					{
					sschannelport = (s7k_fsdwchannel *) &(istore->fsdwsslo.channel[0]);
					ssheaderport = (s7k_fsdwssheader *) &(istore->fsdwsslo.ssheader[0]);
					sschannelstbd = (s7k_fsdwchannel *) &(istore->fsdwsslo.channel[1]);
					ssheaderstbd = (s7k_fsdwssheader *) &(istore->fsdwsslo.ssheader[1]);
					}

				/* set some values */
				ostore->depth_scale = 0;
				ostore->distance_scale = 0;
				ostore->beam_xwidth = 90;
				ostore->beam_lwidth = 90;
				ostore->kind = MB_DATA_DATA;
				ostore->ss_type = MB_SIDESCAN_LINEAR;	/* sets sidescan to be scaled linear */
				opixels_ss = MB7K2SS_SSDIMENSION;
				
				/* reset the sonar altitude using the specified mode */
				if (bottompickmode == MB7K2SS_BOTTOMPICK_ARRIVAL)
					{
					/* get bottom arrival in port trace */
					datashort = (unsigned short *) sschannelport->data;
					channelmax = 0.0;
					for (i=0;i<ssheaderport->samples;i++)
						{
						if (ssheaderport->dataFormat == EDGETECH_TRACEFORMAT_ANALYTIC)
							value = sqrt((double) (datashort[2*i] * datashort[2*i] 
								+ datashort[2*i+1] * datashort[2*i+1]));
						else
							value = (double)(datashort[i]);
						channelmax = MAX(value, channelmax);
						}
					portchannelpick = 0;
					threshold = bottompickthreshold * channelmax;
					for (i=0;i<ssheaderport->samples && portchannelpick == 0;i++)
						{
						if (ssheaderport->dataFormat == EDGETECH_TRACEFORMAT_ANALYTIC)
							value = sqrt((double) (datashort[2*i] * datashort[2*i] 
								+ datashort[2*i+1] * datashort[2*i+1]));
						else
							value = (double)(datashort[i]);
						if (value >= threshold)
							portchannelpick = i;
						}

					/* get bottom arrival in starboard trace */
					datashort = (unsigned short *) sschannelstbd->data;
					channelmax = 0.0;
					for (i=0;i<ssheaderstbd->samples;i++)
						{
						if (ssheaderstbd->dataFormat == EDGETECH_TRACEFORMAT_ANALYTIC)
							value = sqrt((double) (datashort[2*i] * datashort[2*i] 
								+ datashort[2*i+1] * datashort[2*i+1]));
						else
							value = (double)(datashort[i]);
						channelmax = MAX(value, channelmax);
						}
					stbdchannelpick = 0;
					threshold = bottompickthreshold * channelmax;
					for (i=0;i<ssheaderstbd->samples && stbdchannelpick == 0;i++)
						{
						if (ssheaderstbd->dataFormat == EDGETECH_TRACEFORMAT_ANALYTIC)
							value = sqrt((double) (datashort[2*i] * datashort[2*i] 
								+ datashort[2*i+1] * datashort[2*i+1]));
						else
							value = (double)(datashort[i]);
						if (value >= threshold)
							stbdchannelpick = i;
						}
						
					/* set sonar altitude */
					ttime = 0.0000000005 * ((portchannelpick + stbdchannelpick) * ssheaderport->sampleInterval);
					ss_altitude = 0.5 * ssv_use * ttime;
					}
				else if (bottompickmode == MB7K2SS_BOTTOMPICK_BATHYMETRY)
					{
					if (ttime_min_ok == MB_YES)
						{
						ss_altitude = 0.5 * ssv_use * ttime_min_use;
						}
					}
				/* else use the altitude we already have */
				else
					{
					ss_altitude = altitude;
					}
				
				/* get flat bottom layout table */
				if (sslayoutmode == MB7K2SS_SS_FLAT_BOTTOM)
					mb7k2ss_get_flatbottom_table(verbose, nangle, angle_min, angle_max, 
									navlon, navlat, ss_altitude, pitch,
									table_xtrack, table_ltrack, table_altitude, 
									table_range, &error);
				/* else get 3D bottom layout table */
				else
					mb7k2ss_get_3Dbottom_table(verbose, nangle, angle_min, angle_max, 
									navlon, navlat, heading, 
									ss_altitude, sonardepth, pitch, &grid,
									table_xtrack, table_ltrack, 
									table_altitude, table_range, &error);
					
				/* get swath width and pixel size */
				rr = 0.0000000005 * ssv_use *(ssheaderport->samples * ssheaderport->sampleInterval);
				swath_width = 2.2 * sqrt(rr * rr - ss_altitude * ss_altitude);
				pixel_width = swath_width / (opixels_ss - 1);

				/* initialize the output sidescan */
				for (j=0;j<opixels_ss;j++)
					{
					oss[j] = 0.0;
					ossacrosstrack[j] = pixel_width * (double)(j - (opixels_ss / 2));
					ossalongtrack[j] = 0.0;
					ossbincount[j] = 0;
					}
									
				/* find minimum range */
				rr = table_range[0];
				kstart = 0;
				for (kangle=1;kangle<nangle;kangle++)
					{
					if (table_range[kangle] < rr)
						{
						rr = table_range[kangle];
						kstart = kangle;
						}
					}
/*fprintf(stderr,"port minimum range:%f kstart:%d\n",rr,kstart);*/

				/* bin port trace */
				datashort = (unsigned short *) sschannelport->data;
				istart = ss_altitude / (0.0000000005 * ssv_use * ssheaderport->sampleInterval);
				weight = exp(MB_LN_2 * ((double)ssheaderport->weightingFactor));
				for (i=istart;i<ssheaderport->samples;i++)
					{
					/* get sample value */
					if (ssheaderport->dataFormat == EDGETECH_TRACEFORMAT_ANALYTIC)
						value = sqrt((double) (datashort[2*i] * datashort[2*i] 
							+ datashort[2*i+1] * datashort[2*i+1]));
					else
						value = (double)(datashort[i]);

					/* get sample range */
					rr = 0.0000000005 * ssv_use *(i * ssheaderport->sampleInterval);

					/* look up position(s) for this range */
					done = MB_NO;
					for (kangle=kstart;kangle>0 && done == MB_NO;kangle--)
						{
						found = MB_NO;
						if (rr <= table_range[kstart])
							{
							xtrack = table_xtrack[kstart];
							ltrack = table_ltrack[kstart];
							done = MB_YES;
							found = MB_YES;
							}
						else if (rr > table_range[kangle] && rr <= table_range[kangle-1])
							{
							factor = (rr - table_range[kangle]) 
								/ (table_range[kangle-1] - table_range[kangle]);
							xtrack = table_xtrack[kangle] 
								+ factor * (table_xtrack[kangle-1] - table_xtrack[kangle]);
							ltrack = table_ltrack[kangle] 
								+ factor * (table_ltrack[kangle-1] - table_ltrack[kangle]);
							found = MB_YES;
							}
						else if (rr < table_range[kangle] && rr >= table_range[kangle-1])
							{
							factor = (rr - table_range[kangle]) 
								/ (table_range[kangle-1] - table_range[kangle]);
							xtrack = table_xtrack[kangle] 
								+ factor * (table_xtrack[kangle-1] - table_xtrack[kangle]);
							ltrack = table_ltrack[kangle] 
								+ factor * (table_ltrack[kangle-1] - table_ltrack[kangle]);
							found = MB_YES;
							}
							
						/* bin the value and position */
						if (found == MB_YES)
							{
							j = opixels_ss / 2 + (int)(xtrack / pixel_width);
							if (j >= 0 && j < opixels_ss)
								{
								oss[j] += value / weight;
								ossbincount[j]++;
								ossalongtrack[j] += ltrack;
								}
/*fprintf(stderr,"port:%5d rr:%10.2f x:%10.2f l:%10.2f kangle:%d\n",
i,rr,xtrack,ltrack,kangle);*/
							}
						}
					}
									
				/* find minimum range */
				rr = table_range[0];
				kstart = 0;
				for (kangle=1;kangle<nangle;kangle++)
					{
					if (table_range[kangle] < rr)
						{
						rr = table_range[kangle];
						kstart = kangle;
						}
					}
/*fprintf(stderr,"stbd minimum range:%f kstart:%d\n",rr,kstart);*/
/*fprintf(stderr,"kstart:%d angle:%f range:%f xtrack:%f ltrack:%f\n",
kstart,
angle_min + kstart * (angle_max - angle_min) / (nangle - 1),
table_range[kstart],table_xtrack[kstart],table_ltrack[kstart]);*/

				/* bin stbd trace */
				datashort = (unsigned short *) sschannelstbd->data;
				istart = ss_altitude / (0.0000000005 * ssv_use * ssheaderstbd->sampleInterval);
				weight = exp(MB_LN_2 * ((double)ssheaderstbd->weightingFactor));
				for (i=istart;i<ssheaderstbd->samples;i++)
					{
					/* get sample value */
					if (ssheaderstbd->dataFormat == EDGETECH_TRACEFORMAT_ANALYTIC)
						value = sqrt((double) (datashort[2*i] * datashort[2*i] 
							+ datashort[2*i+1] * datashort[2*i+1]));
					else
						value = (double)(datashort[i]);

					/* get sample range */
					rr = 0.0000000005 * ssv_use *(i * ssheaderstbd->sampleInterval);

					/* look up position for this range */
					done = MB_NO;
					for (kangle=kstart;kangle<nangle-1 && done == MB_NO;kangle++)
						{
						found = MB_NO;
						if (rr <= table_range[kstart])
							{
							xtrack = table_xtrack[kstart];
							ltrack = table_ltrack[kstart];
							done = MB_YES;
							found = MB_YES;
							}
						else if (rr > table_range[kangle] && rr <= table_range[kangle+1])
							{
							factor = (rr - table_range[kangle]) 
								/ (table_range[kangle+1] - table_range[kangle]);
							xtrack = table_xtrack[kangle] 
								+ factor * (table_xtrack[kangle+1] - table_xtrack[kangle]);
							ltrack = table_ltrack[kangle] 
								+ factor * (table_ltrack[kangle+1] - table_ltrack[kangle]);
							found = MB_YES;
							}
						else if (rr < table_range[kangle] && rr >= table_range[kangle+1])
							{
							factor = (rr - table_range[kangle]) 
								/ (table_range[kangle+1] - table_range[kangle]);
							xtrack = table_xtrack[kangle] 
								+ factor * (table_xtrack[kangle+1] - table_xtrack[kangle]);
							ltrack = table_ltrack[kangle] 
								+ factor * (table_ltrack[kangle+1] - table_ltrack[kangle]);
							found = MB_YES;
							}

						/* bin the value and position */
						if (found == MB_YES)
							{
							j = opixels_ss / 2 + (int)(xtrack / pixel_width);
							if (j >= 0 && j < opixels_ss)
								{
								oss[j] += value / weight;
								ossbincount[j]++;
								ossalongtrack[j] += ltrack;
								}
/*fprintf(stderr,"stbd:%5d rr:%10.2f x:%10.2f l:%10.2f kangle:%d\n",
i,rr,xtrack,ltrack,kangle);*/
							}
						}
					}

				/* calculate the output sidescan */
				for (j=0;j<opixels_ss;j++)
					{
					if (ossbincount[j] > 0)
						{
						oss[j] /= (double) ossbincount[j];
						ossalongtrack[j] /= (double) ossbincount[j];
						}
					else
						oss[j] = MB_SIDESCAN_NULL;
					}
/*for (j=0;j<opixels_ss;j++)
{
fprintf(stderr,"AAA j:%d x:%7.2f l:%7.2f s:%6.2f\n",j,ossacrosstrack[j],ossalongtrack[j],oss[j]);
}*/

				/* interpolate gaps in the output sidescan */
				previous = opixels_ss;
				for (j=0;j<opixels_ss;j++)
					{
					if (ossbincount[j] > 0)
						{
						interpable = j - previous - 1;
						if (interpable > 0 && interpable <= interpbins)
							{
							dss = oss[j] - oss[previous];
							dssl = ossalongtrack[j] - ossalongtrack[previous];
							for (jj=previous+1;jj<j;jj++)
								{
								fraction = ((double)(jj - previous))
										/ ((double)(j - previous));
								oss[jj] = oss[previous] + fraction * dss;
								ossalongtrack[jj] = ossalongtrack[previous] + fraction * dssl;
								}
							}
						previous = j;
						}
					}
/*for (j=0;j<opixels_ss;j++)
{
fprintf(stderr,"III j:%d x:%7.2f l:%7.2f s:%6.2f\n",j,ossacrosstrack[j],ossalongtrack[j],oss[j]);
}*/

				/* insert data */
				mb_insert_nav(verbose, ombio_ptr, (void *)ostore, 
						time_i, time_d, 
						navlon, navlat, speed, heading, draft, 
						roll, pitch, heave, 
						&error);
				status = mb_insert_altitude(verbose, ombio_ptr, (void *)ostore,
						sonardepth, ss_altitude, &error);
				status = mb_insert(verbose, ombio_ptr, (void *)ostore,
						MB_DATA_DATA, time_i, time_d, 
						navlon, navlat, speed, heading, 
						beams_bath,beams_amp,opixels_ss,
						beamflag,bath,amp,bathacrosstrack,
						bathalongtrack,
						oss,ossacrosstrack,ossalongtrack,
						comment, &error);

				/* write the record */
				nwritesslo++;
				mb_write_ping(verbose, ombio_ptr, (void *)ostore, &error);
				}
			}
			
	   	/* handle high frequency sidescan data */
		else if (status == MB_SUCCESS && kind == MB_DATA_SIDESCAN3) 
			{
/*fprintf(stderr,"MB_DATA_SIDESCAN3: status:%d error:%d kind:%d\n",status,error,kind);*/
			nreadsshi++;
			
			/* output data if desired */
			if (extract_type == MB7K2SS_SSHIGH && nreadnav1 > 0 && oktowrite >= MB7K2SS_ONLINE_COUNT)
				{
				/* get channels */
				if (ssflip == MB_YES)
					{
					sschannelport = (s7k_fsdwchannel *) &(istore->fsdwsshi.channel[1]);
					ssheaderport = (s7k_fsdwssheader *) &(istore->fsdwsshi.ssheader[1]);
					sschannelstbd = (s7k_fsdwchannel *) &(istore->fsdwsshi.channel[0]);
					ssheaderstbd = (s7k_fsdwssheader *) &(istore->fsdwsshi.ssheader[0]);
					}
				else
					{
					sschannelport = (s7k_fsdwchannel *) &(istore->fsdwsshi.channel[0]);
					ssheaderport = (s7k_fsdwssheader *) &(istore->fsdwsshi.ssheader[0]);
					sschannelstbd = (s7k_fsdwchannel *) &(istore->fsdwsshi.channel[1]);
					ssheaderstbd = (s7k_fsdwssheader *) &(istore->fsdwsshi.ssheader[1]);
					}

				/* set some values */
				ostore->depth_scale = 0;
				ostore->distance_scale = 0;
				ostore->beam_xwidth = 60;
				ostore->beam_lwidth = 60;
				ostore->kind = MB_DATA_DATA;
				ostore->ss_type = MB_SIDESCAN_LINEAR;	/* sets sidescan to be scaled linear */
				opixels_ss = MB7K2SS_SSDIMENSION;
				
				/* reset the sonar altitude using the specified mode */
				if (bottompickmode == MB7K2SS_BOTTOMPICK_ARRIVAL)
					{
					/* get bottom arrival in port trace */
					datashort = (unsigned short *) sschannelport->data;
					channelmax = 0.0;
					for (i=0;i<ssheaderport->samples;i++)
						{
						if (ssheaderport->dataFormat == EDGETECH_TRACEFORMAT_ANALYTIC)
							value = sqrt((double) (datashort[2*i] * datashort[2*i] 
								+ datashort[2*i+1] * datashort[2*i+1]));
						else
							value = (double)(datashort[i]);
						channelmax = MAX(value, channelmax);
						}
					portchannelpick = 0;
					threshold = bottompickthreshold * channelmax;
					for (i=0;i<ssheaderport->samples && portchannelpick == 0;i++)
						{
						if (ssheaderport->dataFormat == EDGETECH_TRACEFORMAT_ANALYTIC)
							value = sqrt((double) (datashort[2*i] * datashort[2*i] 
								+ datashort[2*i+1] * datashort[2*i+1]));
						else
							value = (double)(datashort[i]);
						if (value >= threshold)
							portchannelpick = i;
						}

					/* get bottom arrival in starboard trace */
					datashort = (unsigned short *) sschannelstbd->data;
					channelmax = 0.0;
					for (i=0;i<ssheaderstbd->samples;i++)
						{
						if (ssheaderstbd->dataFormat == EDGETECH_TRACEFORMAT_ANALYTIC)
							value = sqrt((double) (datashort[2*i] * datashort[2*i] 
								+ datashort[2*i+1] * datashort[2*i+1]));
						else
							value = (double)(datashort[i]);
						channelmax = MAX(value, channelmax);
						}
					stbdchannelpick = 0;
					threshold = bottompickthreshold * channelmax;
					for (i=0;i<ssheaderstbd->samples && stbdchannelpick == 0;i++)
						{
						if (ssheaderstbd->dataFormat == EDGETECH_TRACEFORMAT_ANALYTIC)
							value = sqrt((double) (datashort[2*i] * datashort[2*i] 
								+ datashort[2*i+1] * datashort[2*i+1]));
						else
							value = (double)(datashort[i]);
						if (value >= threshold)
							stbdchannelpick = i;
						}
						
					/* set sonar altitude */
					ttime = 0.0000000005 * ((portchannelpick + stbdchannelpick) * ssheaderport->sampleInterval);
					ss_altitude = 0.5 * ssv_use * ttime;
					}
				else if (bottompickmode == MB7K2SS_BOTTOMPICK_BATHYMETRY)
					{
					if (ttime_min_ok == MB_YES)
						{
						ss_altitude = 0.5 * ssv_use * ttime_min_use;
						}
					}
				/* else use the altitude we already have */
				else
					{
					ss_altitude = altitude;
					}
				
				/* get flat bottom layout table */
				if (sslayoutmode == MB7K2SS_SS_FLAT_BOTTOM)
					mb7k2ss_get_flatbottom_table(verbose, nangle, angle_min, angle_max, 
									navlon, navlat, ss_altitude, pitch,
									table_xtrack, table_ltrack, table_altitude, 
									table_range, &error);
				/* else get 3D bottom layout table */
				else
					mb7k2ss_get_3Dbottom_table(verbose, nangle, angle_min, angle_max, 
									navlon, navlat, heading,
									ss_altitude, sonardepth, pitch, &grid,
									table_xtrack, table_ltrack, 
									table_altitude, table_range, &error);
					
				/* get swath width and pixel size */
				rr = 0.0000000005 * ssv_use *(ssheaderport->samples * ssheaderport->sampleInterval);
				swath_width = 2.2 * sqrt(rr * rr - ss_altitude * ss_altitude);
				pixel_width = swath_width / (opixels_ss - 1);

				/* initialize the output sidescan */
				for (j=0;j<opixels_ss;j++)
					{
					oss[j] = 0.0;
					ossacrosstrack[j] = pixel_width * (double)(j - (opixels_ss / 2));
					ossalongtrack[j] = 0.0;
					ossbincount[j] = 0;
					}
									
				/* find minimum range */
				rr = table_range[0];
				kstart = 0;
				for (kangle=1;kangle<nangle;kangle++)
					{
					if (table_range[kangle] < rr)
						{
						rr = table_range[kangle];
						kstart = kangle;
						}
					}

				/* bin port trace */
				datashort = (unsigned short *) sschannelport->data;
				istart = ss_altitude / (0.0000000005 * ssv_use * ssheaderport->sampleInterval);
				weight = exp(MB_LN_2 * ((double)ssheaderport->weightingFactor));
				for (i=istart;i<ssheaderport->samples;i++)
					{
					/* get sample value */
					if (ssheaderport->dataFormat == EDGETECH_TRACEFORMAT_ANALYTIC)
						value = sqrt((double) (datashort[2*i] * datashort[2*i] 
							+ datashort[2*i+1] * datashort[2*i+1]));
					else
						value = (double)(datashort[i]);

					/* get sample range */
					rr = 0.0000000005 * ssv_use *(i * ssheaderport->sampleInterval);

					/* look up position(s) for this range */
					done = MB_NO;
					for (kangle=kstart;kangle>0 && done == MB_NO;kangle--)
						{
						found = MB_NO;
						if (rr <= table_range[kstart])
							{
							xtrack = table_xtrack[kstart];
							ltrack = table_ltrack[kstart];
							done = MB_YES;
							found = MB_YES;
							}
						else if (rr > table_range[kangle] && rr <= table_range[kangle-1])
							{
							factor = (rr - table_range[kangle]) 
								/ (table_range[kangle-1] - table_range[kangle]);
							xtrack = table_xtrack[kangle] 
								+ factor * (table_xtrack[kangle-1] - table_xtrack[kangle]);
							ltrack = table_ltrack[kangle] 
								+ factor * (table_ltrack[kangle-1] - table_ltrack[kangle]);
							found = MB_YES;
							}
							
						/* bin the value and position */
						if (found == MB_YES)
							{
							j = opixels_ss / 2 + (int)(xtrack / pixel_width);
							if (j >= 0 && j < opixels_ss)
								{
								oss[j] += value / weight;
								ossbincount[j]++;
								ossalongtrack[j] += ltrack;
								}
							}
						}

					}
									
				/* find minimum range */
				rr = table_range[0];
				kstart = 0;
				for (kangle=1;kangle<nangle;kangle++)
					{
					if (table_range[kangle] < rr)
						{
						rr = table_range[kangle];
						kstart = kangle;
						}
					}

				/* bin stbd trace */
				datashort = (unsigned short *) sschannelstbd->data;
				istart = ss_altitude / (0.0000000005 * ssv_use * ssheaderstbd->sampleInterval);
				weight = exp(MB_LN_2 * ((double)ssheaderstbd->weightingFactor));
				for (i=istart;i<ssheaderstbd->samples;i++)
					{
					/* get sample value */
					if (ssheaderstbd->dataFormat == EDGETECH_TRACEFORMAT_ANALYTIC)
						value = sqrt((double) (datashort[2*i] * datashort[2*i] 
							+ datashort[2*i+1] * datashort[2*i+1]));
					else
						value = (double)(datashort[i]);

					/* get sample range */
					rr = 0.0000000005 * ssv_use *(i * ssheaderstbd->sampleInterval);

					/* look up position for this range */
					done = MB_NO;
					for (kangle=kstart;kangle<nangle-1 && done == MB_NO;kangle++)
						{
						found = MB_NO;
						if (rr <= table_range[kstart])
							{
							xtrack = table_xtrack[kstart];
							ltrack = table_ltrack[kstart];
							done = MB_YES;
							found = MB_YES;
							}
						else if (rr > table_range[kangle] && rr <= table_range[kangle+1])
							{
							factor = (rr - table_range[kangle]) 
								/ (table_range[kangle+1] - table_range[kangle]);
							xtrack = table_xtrack[kangle] 
								+ factor * (table_xtrack[kangle+1] - table_xtrack[kangle]);
							ltrack = table_ltrack[kangle] 
								+ factor * (table_ltrack[kangle+1] - table_ltrack[kangle]);
							found = MB_YES;
							}

						/* bin the value and position */
						if (found == MB_YES)
							{
							j = opixels_ss / 2 + (int)(xtrack / pixel_width);
							if (j >= 0 && j < opixels_ss)
								{
								oss[j] += value / weight;
								ossbincount[j]++;
								ossalongtrack[j] += ltrack;
								}
							}
						}
					}

				/* calculate the output sidescan */
				for (j=0;j<opixels_ss;j++)
					{
					if (ossbincount[j] > 0)
						{
						oss[j] /= (double) ossbincount[j];
						ossalongtrack[j] /= (double) ossbincount[j];
						}
					else
						oss[j] = MB_SIDESCAN_NULL;
					}

				/* interpolate gaps in the output sidescan */
				previous = opixels_ss;
				for (j=0;j<opixels_ss;j++)
					{
					if (ossbincount[j] > 0)
						{
						interpable = j - previous - 1;
						if (interpable > 0 && interpable <= interpbins)
							{
							dss = oss[j] - oss[previous];
							dssl = ossalongtrack[j] - ossalongtrack[previous];
							for (jj=previous+1;jj<j;jj++)
								{
								fraction = ((double)(jj - previous))
										/ ((double)(j - previous));
								oss[jj] = oss[previous] + fraction * dss;
								ossalongtrack[jj] = ossalongtrack[previous] + fraction * dssl;
								}
							}
						previous = j;
						}
					}

				/* insert data */
				mb_insert_nav(verbose, ombio_ptr, (void *)ostore, 
						time_i, time_d, 
						navlon, navlat, speed, heading, draft, 
						roll, pitch, heave, 
						&error);
				status = mb_insert_altitude(verbose, ombio_ptr, (void *)ostore,
						sonardepth, ss_altitude, &error);
				status = mb_insert(verbose, ombio_ptr, (void *)ostore,
						MB_DATA_DATA, time_i, time_d, 
						navlon, navlat, speed, heading, 
						beams_bath,beams_amp,opixels_ss,
						beamflag,bath,amp,bathacrosstrack,
						bathalongtrack,
						oss,ossacrosstrack,ossalongtrack,
						comment, &error);

				/* write the record */
				nwritesshi++;
				mb_write_ping(verbose, ombio_ptr, (void *)ostore, &error);
				}
			}
			
	   	/* handle unknown data */
		else  if (status == MB_SUCCESS) 
			{
/*fprintf(stderr,"DATA TYPE UNKNOWN: status:%d error:%d kind:%d\n",status,error,kind);*/
			}
			
	   	/* handle read error */
		else
			{
/*fprintf(stderr,"READ FAILURE: status:%d error:%d kind:%d\n",status,error,kind);*/
			}

		/* print debug statements */
		if (verbose >= 2)
			{
			fprintf(stderr,"\ndbg2  Ping read in program <%s>\n",
				program_name);
			fprintf(stderr,"dbg2       kind:           %d\n",kind);
			fprintf(stderr,"dbg2       error:          %d\n",error);
			fprintf(stderr,"dbg2       status:         %d\n",status);
			}

		/* print comments */
		if (print_comments == MB_YES && kind == MB_DATA_COMMENT)
			{
			if (icomment == 0)
				{
				fprintf(stderr,"\nComments:\n");
				icomment++;
				}
			fprintf(stderr,"%s\n",comment);
			}
		}

	/* close the swath file */
	status = mb_close(verbose,&imbio_ptr,&error);
	
	/* output counts */
	fprintf(stdout, "\nData records read from: %s\n", file);
	fprintf(stdout, "     Survey:        %d\n", nreaddata);
	fprintf(stdout, "     File Header:   %d\n", nreadheader);
	fprintf(stdout, "     Bluefin CTD:   %d\n", nreadssv);
	fprintf(stdout, "     Bluefin Nav:   %d\n", nreadnav1);
	fprintf(stdout, "     Subbottom:     %d\n", nreadsbp);
	fprintf(stdout, "     Low Sidescan:  %d\n", nreadsslo);
	fprintf(stdout, "     High Sidescan: %d\n", nreadsshi);
	nreaddatatot += nreaddata;
	nreadheadertot += nreadheader;
	nreadssvtot += nreadssv;
	nreadnav1tot += nreadnav1;
	nreadsbptot += nreadsbp;
	nreadsslotot += nreadsslo;
	nreadsshitot += nreadsshi;

	/* figure out whether and what to read next */
        if (read_datalist == MB_YES)
                {
		if ((status = mb_datalist_read(verbose,datalist,
			    file,&format,&file_weight,&error))
			    == MB_SUCCESS)
                        read_data = MB_YES;
                else
                        read_data = MB_NO;
                }
        else
                {
                read_data = MB_NO;
                }

	/* end loop over files in list */
	}
	if (read_datalist == MB_YES)
		mb_datalist_close(verbose,&datalist,&error);

	/* close output file if still open */
	if (ombio_ptr != NULL)
		{
		/* close the swath file */
		status = mb_close(verbose,&ombio_ptr,&error);
		
		/* generate inf file */
		if (status == MB_SUCCESS)
			{
			status = mb_make_info(verbose, MB_YES, 
						output_file, 
						format_output, 
						&error);
				    
			/* output commands to first cut plotting script file */
			fprintf(sfp, "# Generate swath plot of sidescan file: %s\n", output_file);
			fprintf(sfp, "mbm_plot -I %s -N -G5 -S -Pb -V -O %s_ssrawplot\n", 
				output_file, output_file);
			fprintf(sfp, "%s_ssrawplot.cmd\n\n", output_file);
			}
		}
	    
	/* close plotting script file */
	fclose(sfp);
	sprintf(command, "chmod +x %s", scriptfile);
	system(command);
	
	/* output counts */
	fprintf(stdout, "\nTotal data records read:\n");
	fprintf(stdout, "     Survey:        %d\n", nreaddatatot);
	fprintf(stdout, "     File Header:   %d\n", nreadheadertot);
	fprintf(stdout, "     Bluefin CTD:   %d\n", nreadssvtot);
	fprintf(stdout, "     Bluefin Nav:   %d\n", nreadnav1tot);
	fprintf(stdout, "     Subbottom:     %d\n", nreadsbptot);
	fprintf(stdout, "     Low Sidescan:  %d\n", nreadsslotot);
	fprintf(stdout, "     High Sidescan: %d\n", nreadsshitot);
	fprintf(stdout, "Total data records written:\n");
	fprintf(stdout, "     Low Sidescan:  %d\n", nwritesslotot);
	fprintf(stdout, "     High Sidescan: %d\n", nwritesshitot);
		
	/* deallocate route arrays */
	if (route_file_set == MB_YES)
		{	    
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&routelon, &error);
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&routelat, &error);
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&routeheading, &error);
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&routewaypoint, &error);
		}
		
	/* deallocate topography grid array */
	if (sslayoutmode == MB7K2SS_SS_3D_BOTTOM)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&grid.data, &error);

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose,&error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Program <%s> completed\n",
			program_name);
		fprintf(stderr,"dbg2  Ending status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* end it all */
	exit(error);
}
/*--------------------------------------------------------------------*/
int mb7k2ss_get_flatbottom_table(int verbose, int nangle, double angle_min, double angle_max, 
					double navlon, double navlat, double altitude, double pitch,
					double *table_xtrack, double *table_ltrack, 
					double *table_altitude, double *table_range,
					int *error)
{
	char	*function_name = "mb7k2ss_get_flatbottom_table";
	int	status = MB_SUCCESS;
	double	dangle, angle;
	double	rr, xx, zz;
	double	alpha, beta, theta, phi;
	int	i;
	
	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MB7K2SS function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:         %d\n", verbose);
		fprintf(stderr,"dbg2       nangle:          %d\n", nangle);
		fprintf(stderr,"dbg2       angle_min:       %f\n", angle_min);
		fprintf(stderr,"dbg2       angle_max:       %f\n", angle_max);
		fprintf(stderr,"dbg2       navlon:          %f\n", navlon);
		fprintf(stderr,"dbg2       navlat:          %f\n", navlat);
		fprintf(stderr,"dbg2       pitch:           %f\n", pitch);
		}
		
	/* loop over all of the angles */
	dangle = (angle_max - angle_min) / (nangle - 1);
	alpha = pitch;
	zz = altitude;
	for (i=0;i<nangle;i++)
		{
		/* get angles in takeoff coordinates */
		angle = angle_min + dangle * i;
		beta = 90.0 - angle;
		mb_rollpitch_to_takeoff(
			verbose, 
			alpha, beta, 
			&theta, &phi, 
			error);
			
		/* calculate range required to achieve desired altitude */
		rr = zz / cos(DTR * theta);
		
		/* get the position */
		xx = rr * sin(DTR * theta);
		table_xtrack[i] = xx * cos(DTR * phi);
		table_ltrack[i] = xx * sin(DTR * phi);
		table_altitude[i] = zz;
		table_range[i] = rr;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MB7K2SS function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       Lookup tables:\n");
		for (i=0;i<nangle;i++)
			fprintf(stderr,"dbg2         %d %f %f %f %f\n", 
				i, table_xtrack[i], table_ltrack[i], table_altitude[i], table_range[i]);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb7k2ss_get_3Dbottom_table(int verbose, int nangle, double angle_min, double angle_max, 
					double navlon, double navlat, double heading, 
					double altitude, double sonardepth, double pitch,
					struct mb7k2ss_grid_struct *grid,
					double *table_xtrack, double *table_ltrack, 
					double *table_altitude, double *table_range,
					int *error)
{
	char	*function_name = "mb7k2ss_get_3Dbottom_table";
	int	status = MB_SUCCESS;
	double	mtodeglon, mtodeglat;
	double	dangle, angle;
	double	rr, xx, zz;
	double	alpha, beta, theta, phi;
	double	vx, vy, vz;
	int	i;
	
	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MB7K2SS function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:               %d\n", verbose);
		fprintf(stderr,"dbg2       nangle:                %d\n", nangle);
		fprintf(stderr,"dbg2       angle_min:             %f\n", angle_min);
		fprintf(stderr,"dbg2       angle_max:             %f\n", angle_max);
		fprintf(stderr,"dbg2       navlon:                %f\n", navlon);
		fprintf(stderr,"dbg2       navlat:                %f\n", navlat);
		fprintf(stderr,"dbg2       heading:               %f\n", heading);
		fprintf(stderr,"dbg2       altitude:              %f\n", altitude);
		fprintf(stderr,"dbg2       sonardepth:            %f\n", sonardepth);
		fprintf(stderr,"dbg2       pitch:                 %f\n", pitch);
		fprintf(stderr,"dbg2       grid:                  %ld\n", (long)grid);
		fprintf(stderr,"dbg2       grid->projectionname:  %s\n", grid->projectionname);
		fprintf(stderr,"dbg2       grid->projection_mode: %d\n", grid->projection_mode);
		fprintf(stderr,"dbg2       grid->projection_id:   %s\n", grid->projection_id);
		fprintf(stderr,"dbg2       grid->nodatavalue:     %f\n", grid->nodatavalue);
		fprintf(stderr,"dbg2       grid->nxy:             %d\n", grid->nxy);
		fprintf(stderr,"dbg2       grid->nx:              %d\n", grid->nx);
		fprintf(stderr,"dbg2       grid->ny:              %d\n", grid->ny);
		fprintf(stderr,"dbg2       grid->min:             %f\n", grid->min);
		fprintf(stderr,"dbg2       grid->max:             %f\n", grid->max);
		fprintf(stderr,"dbg2       grid->xmin:            %f\n", grid->xmin);
		fprintf(stderr,"dbg2       grid->xmax:            %f\n", grid->xmax);
		fprintf(stderr,"dbg2       grid->ymin:            %f\n", grid->ymin);
		fprintf(stderr,"dbg2       grid->ymax:            %f\n", grid->ymax);
		fprintf(stderr,"dbg2       grid->dx:              %f\n", grid->dx);
		fprintf(stderr,"dbg2       grid->dy               %f\n", grid->dy);
		fprintf(stderr,"dbg2       grid->data:            %ld\n", (long)grid->data);
		}
		
	/* loop over all of the angles */
	mb_coor_scale(verbose,navlat, &mtodeglon, &mtodeglat);
	dangle = (angle_max - angle_min) / (nangle - 1);
	alpha = pitch;
	for (i=0;i<nangle;i++)
		{
		/* get angles in takeoff coordinates */
		angle = angle_min + dangle * i;
		beta = 90.0 - angle;
		mb_rollpitch_to_takeoff(
			verbose, 
			alpha, beta, 
			&theta, &phi, 
			error);
			
		/* calculate unit vector relative to the vehicle */
		vz = cos(DTR * theta);
		vx = sin(DTR * theta) * cos(DTR * phi);
		vy = sin(DTR * theta) * sin(DTR * phi);
			
		/* rotate unit vector by vehicle heading */
		vx = vx * cos(DTR * heading) + vy * sin(DTR * heading);
		vy = -vx * sin(DTR * heading) + vy * cos(DTR * heading);
		
		/* find the range where this vector intersects the grid */
		status = mb7k2ss_intersect_grid(verbose, navlon, navlat, altitude, sonardepth,
					mtodeglon, mtodeglat, vx, vy, vz,
					grid, &rr, error);
			
		/* get the position */
		zz = rr * cos(DTR * theta);
		xx = rr * sin(DTR * theta);
		table_xtrack[i] = xx * cos(DTR * phi);
		table_ltrack[i] = xx * sin(DTR * phi);
		table_altitude[i] = zz;
		table_range[i] = rr;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MB7K2SS function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       Lookup tables:\n");
		for (i=0;i<nangle;i++)
			fprintf(stderr,"dbg2         %d %f %f %f %f\n", 
				i, table_xtrack[i], table_ltrack[i], table_altitude[i], table_range[i]);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb7k2ss_intersect_grid(int verbose, double navlon, double navlat, double altitude, double sonardepth,
					double mtodeglon, double mtodeglat,
					double vx, double vy, double vz,
					struct mb7k2ss_grid_struct *grid,
					double	 *range, int *error)
{
	char	*function_name = "mb7k2ss_intersect";
	int	status = MB_SUCCESS;
	int	done;
	int	iteration;
	int	iteration_max = 25;
	double	topotolerance = 0.1;
	double	dr, r, lontest, lattest;
	double	rmin, rmax;
	double	topotest, topo, dtopo;
	int	nfound;
	int	i, j, ii, jj, k;
	
	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MB7K2SS function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:               %d\n", verbose);
		fprintf(stderr,"dbg2       navlon:                %f\n", navlon);
		fprintf(stderr,"dbg2       navlat:                %f\n", navlat);
		fprintf(stderr,"dbg2       altitude:              %f\n", altitude);
		fprintf(stderr,"dbg2       sonardepth:            %f\n", sonardepth);
		fprintf(stderr,"dbg2       mtodeglon:             %f\n", mtodeglon);
		fprintf(stderr,"dbg2       mtodeglat:             %f\n", mtodeglat);
		fprintf(stderr,"dbg2       vx:                    %f\n", vx);
		fprintf(stderr,"dbg2       vy:                    %f\n", vy);
		fprintf(stderr,"dbg2       vz:                    %f\n", vz);
		fprintf(stderr,"dbg2       grid:                  %ld\n", (long)grid);
		fprintf(stderr,"dbg2       grid->projectionname:  %s\n", grid->projectionname);
		fprintf(stderr,"dbg2       grid->projection_mode: %d\n", grid->projection_mode);
		fprintf(stderr,"dbg2       grid->projection_id:   %s\n", grid->projection_id);
		fprintf(stderr,"dbg2       grid->nodatavalue:     %f\n", grid->nodatavalue);
		fprintf(stderr,"dbg2       grid->nxy:             %d\n", grid->nxy);
		fprintf(stderr,"dbg2       grid->nx:              %d\n", grid->nx);
		fprintf(stderr,"dbg2       grid->ny:              %d\n", grid->ny);
		fprintf(stderr,"dbg2       grid->min:             %f\n", grid->min);
		fprintf(stderr,"dbg2       grid->max:             %f\n", grid->max);
		fprintf(stderr,"dbg2       grid->xmin:            %f\n", grid->xmin);
		fprintf(stderr,"dbg2       grid->xmax:            %f\n", grid->xmax);
		fprintf(stderr,"dbg2       grid->ymin:            %f\n", grid->ymin);
		fprintf(stderr,"dbg2       grid->ymax:            %f\n", grid->ymax);
		fprintf(stderr,"dbg2       grid->dx:              %f\n", grid->dx);
		fprintf(stderr,"dbg2       grid->dy               %f\n", grid->dy);
		fprintf(stderr,"dbg2       grid->data:            %ld\n", (long)grid->data);
		}
	
	/* test different ranges along the vector until the grid is intersected */
	done = MB_NO;
	iteration = 0;
	dr = altitude / 20;
	r = altitude / vz - dr;
	topo = 0.0;
	topotest = 0.0;
	dtopo = 0.0;
	rmin = 0.0;
	rmax = 4 * altitude / vz;
	while (done == MB_NO && iteration < iteration_max)
		{
		/* update the range to be tested */
		r += dr;
		
		/* get position of range estimate projected along the vector */
		lontest = navlon + mtodeglon * vx * r;
		lattest = navlat + mtodeglat * vy * r;
		topotest = -sonardepth - vz * r;
		
		/* get topography value at that point */
		nfound = 0;
		topo = 0.0;
		i = (int)((lontest - grid->xmin) / grid->dx);
		j = (int)((lattest - grid->ymin) / grid->dy);
		if (i >= 0 && i < grid->nx - 1
		    && j >= 0 && j < grid->ny - 1)
			{
			for (ii=i;ii<=i+1;ii++)
			for (jj=j;jj<=j+1;jj++)
			    {
			    k = ii * grid->ny + jj;
			    if (grid->data[k] != grid->nodatavalue)
				{
				nfound++;
				topo += grid->data[k];
				}
			    }
			}
		else
			{
			done = MB_YES;
			status = MB_FAILURE;
			*error = MB_ERROR_NOT_ENOUGH_DATA;
			}
		if (nfound > 0)
			{
			topo /= (double)nfound;
			}
		
		/* compare topographies at projected position */
		if (nfound > 0)
			{
			dtopo = topotest - topo;
			if (fabs(dtopo) < topotolerance)
				{
				done = MB_YES;
				}
			else
				{
				/* get bounds on where vector crosses the grid */
				if (dtopo < 0.0)
					rmax = MIN(rmax, r);
				else if (dtopo > 0.0)
					rmin = MIN(rmin, r);
				
				/* estimate distance to crossing point */
				dr = dtopo / vz;
				
				/* make sure we don't overshoot the bounds */
				if (r + dr >= rmax)
					dr = 0.5 * (rmax - r);
				if (r + dr <= rmin)
					dr = 0.5 * (rmin - r);
				}
			}
		/* keep trying */
		else
			{
			}
		
		/* keep count of iterations */
		iteration++;
/*fprintf(stderr,"iter:%d done:%d r:%f dr:%f topo:%f topotest:%f dtopo:%f vz:%f\n",
iteration,done,r,dr,topo,topotest,dtopo,vz);*/
		}
		
	/* if success return the result */
	*range = r;
	

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MB7K2SS function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       range:           %f\n",*range);
		fprintf(stderr,"dbg2       error:           %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:          %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
