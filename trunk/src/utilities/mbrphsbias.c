/*--------------------------------------------------------------------
 *    The MB-system:	mbrphsbias.c	9/29/2013
 *    $Id$
 *
 *    Copyright (c) 2013-2015 by
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
 * MBrphsbias analyzes sonar soundings to solve for bias parameters associated
 * with the attitude sensors and first order speed of sound. In particular,
 * mbrphsbias uses a brute force multi-dimensional search over roll-bias,
 * pitch-bias, heading-bias, and array-water-sound-speed-bias to minimize the
 * variance of unflagged soundings in the input bathymetry data.
 *
 * Author:	D. W. Caress
 * Date:	September 29, 2013
 *		CCGS Sir Wilfrid Laurier
 *		136d00.6832'W 70d45.3653'W
 *		Beaufort Sea, Arctic Ocean
 *
 * $Log: mbrphsbias.c,v $
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

/* mbio include files */
#include "mb_status.h"
#include "mb_format.h"
#include "mb_define.h"
#include "mb_io.h"
#include "mb_swap.h"
#include "mb_process.h"
#include "mb_info.h"

/* allocation */
#define FILEALLOCNUM	16
#define PINGALLOCNUM	128
#define SNDGALLOCNUM	128

/* mbrphsbias structures */
struct	mbrphsbias_ping_struct
	{
	int	time_i[7];
	double	time_d;
	int	multiplicity;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	double	distance;
	double	altitude;
	double	sonardepth;
	double	draft;
	double	roll;
	double	pitch;
	double	heave;
	double	ssv;
	int	beams_bath;
	char	*beamflag;
	double	*bath;
	double	*bathacrosstrack;
	double	*bathalongtrack;
	double	*bathcorr;
	double	*bathlon;
	double	*bathlat;
	double	*angles;
	double	*angles_forward;
	double	*angles_null;
	double	*ttimes;
	double	*bheave;
	double	*alongtrack_offset;
	};
struct mbrphsbias_file_struct
	{
	char 	path[MB_PATH_MAXLINE];
	int	format;
	int	num_pings;
	int	num_pings_alloc;
	int	num_beams_tot;
	int	num_beams_good;
	int	num_beams_flagged;
	int	num_beams_null;
	struct mbrphsbias_ping_struct *pings;
	};


static char rcs_id[] = "$Id$";

/*--------------------------------------------------------------------*/

int main (int argc, char **argv)
{
	char program_name[] = "MBrphsbias";
	char help_message[] =  "MBrphsbias analyzes sonar soundings to solve for bias parameters associated with the attitude sensors and first order speed of sound.\n";
	char usage_message[] = "mbrphsbias [-Fformat -Iinfile -Rwest/east/south/north -Sbinsize	\n\t-B]";
	extern char *optarg;
	int	errflg = 0;
	int	c;
	int	help = 0;
	int	flag = 0;

	/* MBIO status variables */
	int	status;
	int	verbose = 0;
	int	error = MB_ERROR_NO_ERROR;
	char	*message = NULL;

	/* MBIO read control parameters */
	void	*mbio_ptr = NULL;
	void	*store_ptr = NULL;
	int	kind;
	int	read_datalist = MB_NO;
	char	read_file[MB_PATH_MAXLINE];
	char	swathfile[MB_PATH_MAXLINE];
	char	swathfileread[MB_PATH_MAXLINE];
	void	*datalist;
	int	look_processed = MB_DATALIST_LOOK_UNSET;
	int	read_data;
	double	file_weight;
	int	format;
	int	formatread;
	int	variable_beams;
	int	traveltime;
	int	beam_flagging;
	int	pings;
	int	lonflip;
	double	bounds[4];
	int	btime_i[7];
	int	etime_i[7];
	double	btime_d;
	double	etime_d;
	double	speedmin;
	double	timegap;
	struct mb_info_struct mb_info;

	int	time_i[7];
	double	time_d;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	double	distance;
	double	altitude;
	double	sonardepth;
	double	draft;
	double	ssv;
	double	roll;
	double	pitch;
	double	heave;
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;
	char	*beamflag;
	double	*bath;
	double	*amp;
	double	*bathacrosstrack;
	double	*bathalongtrack;
	double	*ss;
	double	*ssacrosstrack;
	double	*ssalongtrack;
	double	*ttimes;
	double	*angles;
	double	*angles_forward;
	double	*angles_null;
	double	*bheave;
	double	*alongtrack_offset;
	char	comment[MB_COMMENT_MAXLINE];

        /* control parameters */
	double	areabounds[4];
	int	areaboundsset = MB_NO;
	double	binsize = 0.0;
	int	binsizeset = MB_NO;
	double	mtodeglon;
	double	mtodeglat;
	double	dx, dy;
	int	nx, ny;

        /* sounding atorage values and arrays */
        int	nfile = 0;
        int	nfile_alloc = 0;
        struct mbrphsbias_file_struct *files = NULL;
        int	*gsndgnum = NULL;
        double	*gsndgsqsum = NULL;

	/* counting parameters */
	int	files_tot = 0;
	int	pings_tot = 0;
	int	beams_tot = 0;
	int	beams_good_tot = 0;
	int	beams_flagged_tot = 0;
	int	beams_null_tot = 0;

	struct mbrphsbias_ping_struct *ping;
	struct mbrphsbias_file_struct *file;

	int	done;
	int	nbeams;
        int     i, j;

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* reset all defaults but the format and lonflip */
	strcpy(read_file,"datalist.mb-1");
	format = 0;
	pings = 1;
	bounds[0] = -360.;
	bounds[1] = 360.;
	bounds[2] = -90.;
	bounds[3] = 90.;
	btime_i[0] = 1962;
	btime_i[1] = 2;
	btime_i[2] = 21;
	btime_i[3] = 10;
	btime_i[4] = 30;
	btime_i[5] = 0;
	btime_i[6] = 0;
	etime_i[0] = 2062;
	etime_i[1] = 2;
	etime_i[2] = 21;
	etime_i[3] = 10;
	etime_i[4] = 30;
	etime_i[5] = 0;
	etime_i[6] = 0;
	speedmin = 0.0;
	timegap = 1000000000.0;

	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhF:f:I:i:R:r:S:s:")) != -1)
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
		case 'F':
		case 'f':
			sscanf (optarg,"%d", &format);
			flag++;
			break;
		case 'I':
		case 'i':
			sscanf (optarg,"%s", read_file);
			flag++;
			break;
		case 'R':
		case 'r':
				mb_get_bounds(optarg, areabounds);
				areaboundsset = MB_YES;
			flag++;
			break;
		case 'S':
		case 's':
			sscanf (optarg,"%lf", &binsize);
			binsizeset = MB_YES;
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
		fprintf(stderr,"dbg2       verbose:        %d\n",verbose);
		fprintf(stderr,"dbg2       help:           %d\n",help);
		fprintf(stderr,"dbg2       pings:          %d\n",pings);
		fprintf(stderr,"dbg2       lonflip:        %d\n",lonflip);
		fprintf(stderr,"dbg2       bounds[0]:      %f\n",bounds[0]);
		fprintf(stderr,"dbg2       bounds[1]:      %f\n",bounds[1]);
		fprintf(stderr,"dbg2       bounds[2]:      %f\n",bounds[2]);
		fprintf(stderr,"dbg2       bounds[3]:      %f\n",bounds[3]);
		fprintf(stderr,"dbg2       btime_i[0]:     %d\n",btime_i[0]);
		fprintf(stderr,"dbg2       btime_i[1]:     %d\n",btime_i[1]);
		fprintf(stderr,"dbg2       btime_i[2]:     %d\n",btime_i[2]);
		fprintf(stderr,"dbg2       btime_i[3]:     %d\n",btime_i[3]);
		fprintf(stderr,"dbg2       btime_i[4]:     %d\n",btime_i[4]);
		fprintf(stderr,"dbg2       btime_i[5]:     %d\n",btime_i[5]);
		fprintf(stderr,"dbg2       btime_i[6]:     %d\n",btime_i[6]);
		fprintf(stderr,"dbg2       etime_i[0]:     %d\n",etime_i[0]);
		fprintf(stderr,"dbg2       etime_i[1]:     %d\n",etime_i[1]);
		fprintf(stderr,"dbg2       etime_i[2]:     %d\n",etime_i[2]);
		fprintf(stderr,"dbg2       etime_i[3]:     %d\n",etime_i[3]);
		fprintf(stderr,"dbg2       etime_i[4]:     %d\n",etime_i[4]);
		fprintf(stderr,"dbg2       etime_i[5]:     %d\n",etime_i[5]);
		fprintf(stderr,"dbg2       etime_i[6]:     %d\n",etime_i[6]);
		fprintf(stderr,"dbg2       speedmin:       %f\n",speedmin);
		fprintf(stderr,"dbg2       timegap:        %f\n",timegap);
		fprintf(stderr,"dbg2       data format:    %d\n",format);
		fprintf(stderr,"dbg2       input file:     %s\n",read_file);
		fprintf(stderr,"dbg2       areaboundsset:  %d\n",areaboundsset);
		fprintf(stderr,"dbg2       areabounds[0]:  %f\n",areabounds[0]);
		fprintf(stderr,"dbg2       areabounds[1]:  %f\n",areabounds[1]);
		fprintf(stderr,"dbg2       areabounds[2]:  %f\n",areabounds[2]);
		fprintf(stderr,"dbg2       areabounds[3]:  %f\n",areabounds[3]);
		fprintf(stderr,"dbg2       binsizeset:     %d\n",binsizeset);
		fprintf(stderr,"dbg2       binsize:        %f\n",binsize);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}

	/* if bounds not set get bounds of input data */
	if (areaboundsset == MB_NO)
		{
		formatread = format;
		status = mb_get_info_datalist(verbose, read_file, &formatread,
				&mb_info, lonflip, &error);

		areabounds[0] = mb_info.lon_min;
		areabounds[1] = mb_info.lon_max;
		areabounds[2] = mb_info.lat_min;
		areabounds[3] = mb_info.lat_max;

		if (binsizeset == MB_NO)
			binsize = 0.2 * mb_info.altitude_max;
		}

	/* calculate area grid properties */
	mb_coor_scale(verbose,0.5*(areabounds[2]+areabounds[3]),&mtodeglon,&mtodeglat);
	if (binsize <= 0.0)
		binsize = (areabounds[1] - areabounds[0]) / 101 / mtodeglon;
	dx = binsize * mtodeglon;
	dy = binsize * mtodeglat;
	nx = 1 + (int)((areabounds[1] - areabounds[0]) / dx);
	ny = 1 + (int)((areabounds[3] - areabounds[2]) / dy);
	if (nx > 1 && ny > 1)
		{
		dx = (areabounds[1] - areabounds[0]) / (nx - 1);
		dy = (areabounds[3] - areabounds[2]) / (ny - 1);
		}

	/* allocate grid arrays */
	status = mb_mallocd(verbose,__FILE__,__LINE__, nx * ny * sizeof(int *),
			(void **)&gsndgnum, &error);
	if (status == MB_SUCCESS)
	status = mb_mallocd(verbose,__FILE__,__LINE__, nx * ny * sizeof(double),
			(void **)&gsndgsqsum, &error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* give the grid info */
	if (verbose >= 0)
		{
		fprintf(stderr,"\nMBrphsbias Processing Parameters:\n");
		fprintf(stderr,"-------------------------\n");
		fprintf(stderr,"Area Bounds:\n");
		fprintf(stderr,"  longitude: %f %f\n",areabounds[0],areabounds[1]);
		fprintf(stderr,"  latitude:  %f %f\n",areabounds[2],areabounds[3]);
		fprintf(stderr,"Binsize: %f meters\n",binsize);
		fprintf(stderr,"  longitude: %f\n",dx);
		fprintf(stderr,"  latitude:  %f\n",dy);
		fprintf(stderr,"Grid dimensions:\n");
		fprintf(stderr,"  longitude: %d\n",nx);
		fprintf(stderr,"  latitude:  %d\n",ny);
		fprintf(stderr,"-------------------------\n");
		}

	/* if error initializing memory then quit */
	for (i=0;i<nx*ny;i++)
		{
		gsndgnum[i] = 0;
		gsndgsqsum[i] = 0.0;
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
			    swathfile,&format,&file_weight,&error))
			    == MB_SUCCESS)
		read_data = MB_YES;
	    else
		read_data = MB_NO;
	    }
	/* else copy single filename to be read */
	else
	    {
	    strcpy(swathfile, read_file);
	    read_data = MB_YES;
	    }

	/* loop over all files to be read */
	while (read_data == MB_YES)
            {
            /* check format and get format flags */
            if ((status = mb_format_flags(verbose,&format,
                            &variable_beams, &traveltime, &beam_flagging,
                            &error))
                    != MB_SUCCESS)
                    {
                    mb_error(verbose,error,&message);
                    fprintf(stderr,"\nMBIO Error returned from function <mb_format_flags> regarding input format %d:\n%s\n",format,message);
                    fprintf(stderr,"\nProgram <%s> Terminated\n",
                            program_name);
                    exit(error);
                    }

            /* check for "fast bathymetry" or "fbt" file */
            strcpy(swathfileread, swathfile);
            formatread = format;
            mb_get_fbt(verbose, swathfileread, &formatread, &error);

            /* initialize reading the input swath sonar file */
            if ((status = mb_read_init(
                    verbose,swathfileread,formatread,pings,lonflip,bounds,
                    btime_i,etime_i,speedmin,timegap,
                    &mbio_ptr,&btime_d,&etime_d,
                    &beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
                    {
                    mb_error(verbose,error,&message);
                    fprintf(stderr,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
                    fprintf(stderr,"\nMultibeam File <%s> not initialized for reading\n",swathfileread);
                    fprintf(stderr,"\nProgram <%s> Terminated\n",
                            program_name);
                    exit(error);
                    }

            /* give the statistics */
            if (verbose >= 0)
                    {
                    fprintf(stderr,"\nProcessing %s\n",swathfileread);
                    }

            /* allocate memory for data arrays */
            beamflag = NULL;
            bath = NULL;
            amp = NULL;
            bathacrosstrack = NULL;
            bathalongtrack = NULL;
            ss = NULL;
            ssacrosstrack = NULL;
            ssalongtrack = NULL;
            ttimes = NULL;
            angles = NULL;
            angles_forward = NULL;
            angles_null = NULL;
            bheave = NULL;
            alongtrack_offset = NULL;
            if (error == MB_ERROR_NO_ERROR)
                    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
                                                    sizeof(char), (void **)&beamflag, &error);
            if (error == MB_ERROR_NO_ERROR)
                    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
                                                    sizeof(double), (void **)&bath, &error);
            if (error == MB_ERROR_NO_ERROR)
                    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE,
                                                    sizeof(double), (void **)&amp, &error);
            if (error == MB_ERROR_NO_ERROR)
                    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
                                                    sizeof(double), (void **)&bathacrosstrack, &error);
            if (error == MB_ERROR_NO_ERROR)
                    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
                                                    sizeof(double), (void **)&bathalongtrack, &error);
            if (error == MB_ERROR_NO_ERROR)
                    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
                                                    sizeof(double), (void **)&ttimes, &error);
            if (error == MB_ERROR_NO_ERROR)
                    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
                                                    sizeof(double), (void **)&angles, &error);
            if (error == MB_ERROR_NO_ERROR)
                    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
                                                    sizeof(double), (void **)&angles_forward, &error);
            if (error == MB_ERROR_NO_ERROR)
                    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
                                                    sizeof(double), (void **)&angles_null, &error);
            if (error == MB_ERROR_NO_ERROR)
                    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
                                                    sizeof(double), (void **)&bheave, &error);
            if (error == MB_ERROR_NO_ERROR)
                    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY,
                                                    sizeof(double), (void **)&alongtrack_offset, &error);
            if (error == MB_ERROR_NO_ERROR)
                    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN,
                                                    sizeof(double), (void **)&ss, &error);
            if (error == MB_ERROR_NO_ERROR)
                    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN,
                                                    sizeof(double), (void **)&ssacrosstrack, &error);
            if (error == MB_ERROR_NO_ERROR)
                    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN,
                                                    sizeof(double), (void **)&ssalongtrack, &error);

            /* if error initializing memory then quit */
            if (error != MB_ERROR_NO_ERROR)
                    {
                    mb_error(verbose,error,&message);
                    fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
                    fprintf(stderr,"\nProgram <%s> Terminated\n",
                            program_name);
                    exit(error);
                    }

            /* update memory for files */
            if (nfile >= nfile_alloc)
                {
                nfile_alloc += FILEALLOCNUM;
                status = mb_reallocd(verbose, __FILE__, __LINE__,
                                nfile_alloc * sizeof(struct mbrphsbias_file_struct),
                                (void **)&files, &error);

                /* if error initializing memory then quit */
                if (error != MB_ERROR_NO_ERROR)
                    {
                    mb_error(verbose,error,&message);
                    fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
                    fprintf(stderr,"\nProgram <%s> Terminated\n",
                            program_name);
                    exit(error);
                    }
                }

            /* initialize current file */
	    file = &(files[nfile]);
            nfile++;
            strcpy(file->path, swathfile);
            file->format = format;
            file->num_pings = 0;
            file->num_beams_tot = 0;
	    file->num_beams_good = 0;
            file->num_beams_flagged = 0;
            file->num_beams_null = 0;
            file->num_pings_alloc = PINGALLOCNUM;
            file->pings = NULL;
            status = mb_mallocd(verbose,__FILE__,__LINE__,
                            file->num_pings_alloc * sizeof(struct mbrphsbias_ping_struct),
                            (void **)&(file->pings), &error);
            if (error != MB_ERROR_NO_ERROR)
                {
                mb_error(verbose,error,&message);
                fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
                fprintf(stderr,"\nProgram <%s> Terminated\n",
                        program_name);
                exit(error);
                }

            /* read the pings into memory */
            done = MB_NO;
            while (done == MB_NO)
                {
                if (verbose > 1) fprintf(stderr,"\n");

                /* read next record */
                error = MB_ERROR_NO_ERROR;
		status = mb_get_all(verbose,mbio_ptr,&store_ptr,&kind,
				    time_i,&time_d,&navlon,&navlat,
				    &speed,&heading,
				    &distance,&altitude,&sonardepth,
				    &beams_bath,&beams_amp,&pixels_ss,
				    beamflag,bath,amp,bathacrosstrack,bathalongtrack,
				    ss,ssacrosstrack,ssalongtrack,
				    comment,&error);
		if (status == MB_FAILURE && error > MB_ERROR_NO_ERROR)
		    done = MB_YES;
                if (verbose >= 2)
                    {
                    fprintf(stderr,"\ndbg2  current data status:\n");
                    fprintf(stderr,"dbg2    kind:       %d\n",kind);
                    fprintf(stderr,"dbg2    status:     %d\n",status);
                    }
                if (status == MB_SUCCESS && kind == MB_DATA_DATA)
                    {
		    status = mb_extract_nav(verbose,mbio_ptr,store_ptr,&kind,
					time_i,&time_d,&navlon,&navlat,
					&speed,&heading,&draft,
					&roll,&pitch,&heave,&error);
		    status = mb_ttimes(verbose,mbio_ptr,
					store_ptr,&kind,&nbeams,
					ttimes,angles,
					angles_forward,angles_null,
					bheave,alongtrack_offset,
					&draft,&ssv,&error);

		    /* allocate memory if necessary */
		    if (file->num_pings >= file->num_pings_alloc)
			{
			file->num_pings_alloc += PINGALLOCNUM;
			status = mb_reallocd(verbose,__FILE__,__LINE__,
					    file->num_pings_alloc * sizeof(struct mbrphsbias_ping_struct),
					    (void **)&(file->pings), &error);
			if (error != MB_ERROR_NO_ERROR)
				{
				mb_error(verbose,error,&message);
				fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
				fprintf(stderr,"\nProgram <%s> Terminated\n",
					program_name);
				exit(error);
				}
			}
		    ping = &(file->pings[file->num_pings]);
		    status = mb_mallocd(verbose,__FILE__,__LINE__,
				beams_bath * sizeof(char),
				(void **)&(ping->beamflag), &error);
		    if (error == MB_ERROR_NO_ERROR)
		    status = mb_mallocd(verbose,__FILE__,__LINE__,
				beams_bath * sizeof(double),
				(void **)&(ping->bath), &error);
		    if (error == MB_ERROR_NO_ERROR)
		    status = mb_mallocd(verbose,__FILE__,__LINE__,
				beams_bath * sizeof(double),
				(void **)&(ping->bathacrosstrack), &error);
		    if (error == MB_ERROR_NO_ERROR)
		    status = mb_mallocd(verbose,__FILE__,__LINE__,
				beams_bath * sizeof(double),
				(void **)&(ping->bathalongtrack), &error);
		    if (error == MB_ERROR_NO_ERROR)
		    status = mb_mallocd(verbose,__FILE__,__LINE__,
				beams_bath * sizeof(double),
				(void **)&(ping->ttimes), &error);
		    if (error == MB_ERROR_NO_ERROR)
		    status = mb_mallocd(verbose,__FILE__,__LINE__,
				beams_bath * sizeof(double),
				(void **)&(ping->angles), &error);
		    if (error == MB_ERROR_NO_ERROR)
		    status = mb_mallocd(verbose,__FILE__,__LINE__,
				beams_bath * sizeof(double),
				(void **)&(ping->angles_forward), &error);
		    if (error == MB_ERROR_NO_ERROR)
		    status = mb_mallocd(verbose,__FILE__,__LINE__,
				beams_bath * sizeof(double),
				(void **)&(ping->angles_null), &error);
		    if (error == MB_ERROR_NO_ERROR)
		    status = mb_mallocd(verbose,__FILE__,__LINE__,
				beams_bath * sizeof(double),
				(void **)&(ping->bheave), &error);
		    if (error == MB_ERROR_NO_ERROR)
		    status = mb_mallocd(verbose,__FILE__,__LINE__,
				beams_bath * sizeof(double),
				(void **)&(ping->alongtrack_offset), &error);
		    if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}

                    /* update counters */
                    pings_tot++;
                    file->num_pings++;
                    for (i=0;i<beams_bath;i++)
				{
				beams_tot++;
				file->num_beams_tot++;
				if (mb_beam_ok(beamflag[i]))
					{
					beams_good_tot++;
					file->num_beams_good++;
					}
				else if (beamflag[i] == MB_FLAG_NULL)
					{
					beams_null_tot++;
					file->num_beams_null++;
					}
				else
					{
					beams_flagged_tot++;
					file->num_beams_flagged++;
					}
				}

                    /* store the ping data */
		    for (i=0;i<7;i++)
			ping->time_i[i] = time_i[i];
                    ping->time_d = time_d;
                    if (file->num_pings > 0
                            && ping->time_d
                                    == file->pings[file->num_pings - 1].time_d)
                            {
                            ping->multiplicity
                                    = file->pings[file->num_pings - 1].multiplicity + 1;
                            }
                    else
                            {
                            ping->multiplicity = 0;
                            }
                    ping->navlon = navlon;
                    ping->navlat = navlat;
                    ping->speed = speed;
                    ping->heading = heading;
                    ping->distance = distance;
                    ping->altitude = altitude;
                    ping->sonardepth = sonardepth;
                    ping->draft = sonardepth - heave;
                    ping->roll = roll;
                    ping->pitch = pitch;
                    ping->heave = heave;
                    ping->ssv = ssv;
                    ping->beams_bath = beams_bath;
		    for (i=0;i<ping->beams_bath;i++)
			{
			ping->beamflag[i] = beamflag[i];
			ping->bath[i] = bath[i];
			ping->bathacrosstrack[i] = bathacrosstrack[i];
			ping->bathalongtrack[i] = bathalongtrack[i];
			ping->ttimes[i] = ttimes[i];
			ping->angles[i] = angles[i];
			ping->angles_forward[i] = angles_forward[i];
			ping->angles_null[i] = angles_null[i];
			ping->bheave[i] = bheave[i];
			ping->alongtrack_offset[i] = alongtrack_offset[i];
			}
		    }
		}


            /* close the files */
            status = mb_close(verbose,&mbio_ptr,&error);

	    /* figure out whether and what to read next */
            if (read_datalist == MB_YES)
                {
		if ((status = mb_datalist_read(verbose,datalist,
			    swathfile,&format,&file_weight,&error))
			    == MB_SUCCESS)
		    read_data = MB_YES;
		else
		    read_data = MB_NO;
                }
            else
                {
                read_data = MB_NO;
                }
            }

	/* give the total statistics */
	if (verbose >= 0)
            {
            fprintf(stderr,"\nMBrphsbias Processing Totals:\n");
            fprintf(stderr,"-------------------------\n");
            fprintf(stderr,"%d total swath data files processed\n",files_tot);
            fprintf(stderr,"%d total pings processed\n",pings_tot);
            fprintf(stderr,"%d total soundings processed\n",beams_tot);
            fprintf(stderr,"-------------------------\n");
            for (i=0;i<nfile;i++)
                {
                }
            }

	/* free arrays */
	mb_freed(verbose,__FILE__, __LINE__, (void **)&gsndgnum,&error);
	mb_freed(verbose,__FILE__, __LINE__, (void **)&gsndgsqsum,&error);

	for (i=0;i<nfile;i++)
		{
		file = &(files[nfile]);
		for (j=0;j<file->num_pings;j++)
			{
			ping = &(file->pings[j]);
			mb_freed(verbose,__FILE__, __LINE__, (void **)&(ping->beamflag),&error);
			mb_freed(verbose,__FILE__, __LINE__, (void **)&(ping->bath),&error);
			mb_freed(verbose,__FILE__, __LINE__, (void **)&(ping->bathacrosstrack),&error);
			mb_freed(verbose,__FILE__, __LINE__, (void **)&(ping->bathalongtrack),&error);
			mb_freed(verbose,__FILE__, __LINE__, (void **)&(ping->ttimes),&error);
			mb_freed(verbose,__FILE__, __LINE__, (void **)&(ping->angles),&error);
			mb_freed(verbose,__FILE__, __LINE__, (void **)&(ping->angles_forward),&error);
			mb_freed(verbose,__FILE__, __LINE__, (void **)&(ping->angles_null),&error);
			mb_freed(verbose,__FILE__, __LINE__, (void **)&(ping->bheave),&error);
			mb_freed(verbose,__FILE__, __LINE__, (void **)&(ping->alongtrack_offset),&error);
			}
		mb_freed(verbose,__FILE__, __LINE__, (void **)&(file->pings),&error);
		}
	mb_freed(verbose,__FILE__, __LINE__, (void **)&files,&error);

	/* set program status */
	status = MB_SUCCESS;

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
