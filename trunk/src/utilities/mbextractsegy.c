/*--------------------------------------------------------------------
 *    The MB-system:	mbextractsegy.c	4/18/2004
 *    $Id: mbextractsegy.c,v 5.4 2004-09-16 01:01:12 caress Exp $
 *
 *    Copyright (c) 2004 by
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
 * mbextractsegy extracts subbottom profiler, center beam reflection,
 * or seismic reflection data from data supported by MB-System and
 * rewrites it as a SEGY file in the form used by SIOSEIS. .
 *
 * Author:	D. W. Caress
 * Date:	April 18, 2004
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.3  2004/07/27 19:48:35  caress
 * Working on handling subbottom data.
 *
 * Revision 5.2  2004/07/15 19:33:57  caress
 * Improvements to support for Reson 7k data.
 *
 * Revision 5.1  2004/06/18 05:20:05  caress
 * Working on adding support for segy i/o and for Reson 7k format 88.
 *
 * Revision 5.0  2004/05/21 23:50:44  caress
 * Progress supporting Reson 7k data, including support for extracing subbottom profiler data.
 *
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>

/* MBIO include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_define.h"
#include "../../include/mb_segy.h"

static char rcs_id[] = "$Id: mbextractsegy.c,v 5.4 2004-09-16 01:01:12 caress Exp $";

/*--------------------------------------------------------------------*/

main (int argc, char **argv)
{
	static char program_name[] = "MBextractsegy";
	static char help_message[] =  "MBextractsegy extracts subbottom profiler, center beam reflection,\nor seismic reflection data from data supported by MB-System and\nrewrites it as a SEGY file in the form used by SIOSEIS.";
	static char usage_message[] = "mbextractsegy [-Byr/mo/dy/hr/mn/sc/us -Eyr/mo/dy/hr/mn/sc/us -Fformat -Ifile -H -Osegyfile -Ssampleformat -V]";
	extern char *optarg;
	extern int optkind;
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
	char	read_file[MB_PATH_MAXLINE];
	char	output_file[MB_PATH_MAXLINE];
	int	output_file_set = MB_NO;
	void	*datalist;
	int	look_processed = MB_DATALIST_LOOK_UNSET;
	double	file_weight;
	int	format;
	int	pings;
	int	pings_read;
	int	lonflip;
	double	bounds[4];
	int	btime_i[7];
	int	etime_i[7];
	double	btime_d;
	double	etime_d;
	double	speedmin;
	double	timegap;
	char	file[MB_PATH_MAXLINE];
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;
	double	timeshift = 0.0;

	/* MBIO read values */
	void	*mbio_ptr = NULL;
	void	*store_ptr = NULL;
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
	double	draft;
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
	char	comment[MB_COMMENT_MAXLINE];
	int	icomment = 0;
	
	/* segy data */
	int	sampleformat = MB_SEGY_SAMPLEFORMAT_NONE;
	int	samplesize = 0;
	struct mb_segyasciiheader_struct segyasciiheader;
	struct mb_segyfileheader_struct segyfileheader;
	struct mb_segytraceheader_struct segytraceheader;
	int	segydata_alloc = 0;
	float	*segydata = NULL;
	int	buffer_alloc = 0;
	char	*buffer = NULL;

	FILE	*ofp = NULL;
	int	read_data;
	double	distmin;
	int	found;
	int	nread;
	int	nwrite;
	int	first;
	int	index;
	double	tracemin, tracemax;
	int	i, j, k;

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* set default input to datalist.mb-1 */
	strcpy (read_file, "datalist.mb-1");
	
	/* initialize output segy structures */
	for (j=0;j<40;j++)
		for (i=0;i<80;i++)
			segyasciiheader.line[j][i] = 0;
	segyfileheader.jobid = 0;
	segyfileheader.line = 0;
	segyfileheader.reel = 0;
	segyfileheader.channels = 0;
	segyfileheader.aux_channels = 0;
	segyfileheader.sample_interval = 0;
	segyfileheader.sample_interval_org = 0;
	segyfileheader.number_samples = 0;
	segyfileheader.number_samples_org = 0;
	segyfileheader.format = 5;
	segyfileheader.cdp_fold = 0;
	segyfileheader.trace_sort = 0;
	segyfileheader.vertical_sum = 0;
	segyfileheader.sweep_start = 0;
	segyfileheader.sweep_end = 0;
	segyfileheader.sweep_length = 0;
	segyfileheader.sweep_type = 0;
	segyfileheader.sweep_trace = 0;
	segyfileheader.sweep_taper_start = 0;
	segyfileheader.sweep_taper_end = 0;
	segyfileheader.sweep_taper = 0;
	segyfileheader.correlated = 0;
	segyfileheader.binary_gain = 0;
	segyfileheader.amplitude = 0;
	segyfileheader.units = 0;
	segyfileheader.impulse_polarity = 0;
	segyfileheader.domain = 0;
	for (i=0;i<338;i++)
		segyfileheader.extra[i] = 0;

	/* process argument list */
	while ((c = getopt(argc, argv, "B:b:D:d:E:e:F:f:I:i:O:o:S:s:T:t:VvHh")) != -1)
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
		case 'B':
		case 'b':
			sscanf (optarg,"%d/%d/%d/%d/%d/%d",
				&btime_i[0],&btime_i[1],&btime_i[2],
				&btime_i[3],&btime_i[4],&btime_i[5]);
			btime_i[6] = 0;
			flag++;
			break;
		case 'E':
		case 'e':
			sscanf (optarg,"%d/%d/%d/%d/%d/%d",
				&etime_i[0],&etime_i[1],&etime_i[2],
				&etime_i[3],&etime_i[4],&etime_i[5]);
			etime_i[6] = 0;
			flag++;
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
		case 'O':
		case 'o':
			sscanf (optarg,"%s", output_file);
			output_file_set = MB_YES;
			flag++;
			break;
		case 'S':
		case 's':
			sscanf (optarg,"%d", &sampleformat);
			flag++;
			break;
		case 'T':
		case 't':
			sscanf (optarg,"%lf", &timeshift);
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
		fprintf(stderr,"dbg2       format:         %d\n",format);
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
		fprintf(stderr,"dbg2       sampleformat:   %d\n",sampleformat);
		fprintf(stderr,"dbg2       timeshift:      %f\n",timeshift);
		fprintf(stderr,"dbg2       file:           %s\n",file);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}

	/* get format if required */
	if (format == 0)
		mb_get_format(verbose,read_file,NULL,&format,&error);
		
	/* get sample size from sampleformat */
	if (sampleformat == MB_SEGY_SAMPLEFORMAT_ANALYTIC)
		samplesize = 2 * sizeof(float);
	else
		samplesize = sizeof(float);

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
	    if (status = mb_datalist_read(verbose,datalist,
			    file,&format,&file_weight,&error)
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
	
	/* loop over all files to be read */
	while (read_data == MB_YES)
	{

	/* initialize reading the swath file */
	if ((status = mb_read_init(
		verbose,file,format,pings,lonflip,bounds,
		btime_i,etime_i,speedmin,timegap,
		&mbio_ptr,&btime_d,&etime_d,
		&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
		fprintf(stderr,"\nMultibeam File <%s> not initialized for reading\n",file);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* allocate memory for data arrays */
	status = mb_malloc(verbose,beams_bath*sizeof(char),(char **)&beamflag,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),(char **)&bath,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),
			(char **)&bathacrosstrack,&error);
	status = mb_malloc(verbose,beams_bath*sizeof(double),
			(char **)&bathalongtrack,&error);
	status = mb_malloc(verbose,beams_amp*sizeof(double),(char **)&amp,&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(double),(char **)&ss,&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(double),(char **)&ssacrosstrack,
			&error);
	status = mb_malloc(verbose,pixels_ss*sizeof(double),(char **)&ssalongtrack,
			&error);

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
		
	/* set up output file */
	if (error == MB_ERROR_NO_ERROR)
		{
		if (output_file_set == MB_NO)
			{
			strcpy(output_file, file);
			strcat(output_file,".segy");
			}
			
		if ((output_file_set == MB_YES
			&& ofp == NULL)
			|| output_file_set == MB_NO)
			{
			/* close any old output file unless a single file has been specified */
			if (ofp != NULL)
				{
				fclose(ofp);
				}
				
			/* open the new file */
			nwrite = 0;
			if ((ofp = fopen(output_file, "w")) == NULL) 
				{
				status = MB_FAILURE;
				error = MB_ERROR_WRITE_FAIL;
				fprintf(stderr,"\nError opening output segy file:\n%s\n",
					output_file);
				fprintf(stderr,"\nProgram <%s> Terminated\n",
					program_name);
				exit(error);
				}
			}
		}

	/* read and print data */
	nread = 0;
	first = MB_YES;
	while (error <= MB_ERROR_NO_ERROR)
		{
		/* reset error */
		error = MB_ERROR_NO_ERROR;
		
		/* read next data record */
		status = mb_get_all(verbose,mbio_ptr,&store_ptr,&kind,
		    time_i,&time_d,&navlon,&navlat,
		    &speed,&heading,
		    &distance,&altitude,&sonardepth,
		    &beams_bath,&beams_amp,&pixels_ss,
		    beamflag,bath,amp,bathacrosstrack,bathalongtrack,
		    ss,ssacrosstrack,ssalongtrack,
		    comment,&error);

		/* if desired subbottom data extract */
		if (error == MB_ERROR_NO_ERROR
			&& (kind  == MB_DATA_SUBBOTTOM_MCS
				|| kind == MB_DATA_SUBBOTTOM_CNTRBEAM
				|| kind == MB_DATA_SUBBOTTOM_SUBBOTTOM))
		    {
		    /* extract the header */
		    status = mb_extract_segytraceheader(verbose,mbio_ptr,store_ptr,&kind,
				    (void *)&segytraceheader,&error);

		    /* allocate the required memory */
		    if (status == MB_SUCCESS 
		    	&& segytraceheader.nsamps > segydata_alloc)
			{
			status = mb_malloc(verbose, segytraceheader.nsamps * samplesize,
						(char **)&segydata, &error);
			if (status == MB_SUCCESS)
				segydata_alloc = segytraceheader.nsamps;
			else
				segydata_alloc = 0;
			}
		    if (status == MB_SUCCESS 
		    	&& (buffer_alloc < MB_SEGY_TRACEHEADER_LENGTH
				|| buffer_alloc < segytraceheader.nsamps * samplesize))
			{
			buffer_alloc = MAX(MB_SEGY_TRACEHEADER_LENGTH, segytraceheader.nsamps * samplesize);
			status = mb_malloc(verbose, buffer_alloc, (char **)&buffer, &error);
			if (status != MB_SUCCESS)
				buffer_alloc = 0;
			}

		    /* extract the data */
		    if (status == MB_SUCCESS)
			status = mb_extract_segy(verbose,mbio_ptr,store_ptr,&sampleformat,&kind,
				    (void *)&segytraceheader,segydata,&error);
				    
		    /* apply time shift if needed */
		    if (status == MB_SUCCESS && timeshift != 0.0)
		    	{
			time_j[0] = segytraceheader.year;
			time_j[1] = segytraceheader.day_of_yr;
			time_j[2] = 60 * segytraceheader.hour + segytraceheader.min;
			time_j[3] = segytraceheader.sec;
			time_j[4] = 1000 * segytraceheader.mils;
			mb_get_itime(verbose,time_j,time_i);
			mb_get_time(verbose,time_i,&time_d);
			time_d += timeshift;
			mb_get_date(verbose,time_d,time_i);
			mb_get_jtime(verbose,time_i,time_j);
			segytraceheader.year = time_i[0];
			segytraceheader.day_of_yr = time_j[1];
			segytraceheader.hour = time_i[3];
			segytraceheader.min = time_i[4];
			segytraceheader.sec = time_i[5];
			}
				    
		    /* write fileheader if needed */
		    if (status == MB_SUCCESS && nwrite == 0)
		    	{
			segyfileheader.format = 5;
			segyfileheader.channels = 1;
			segyfileheader.aux_channels = 0;
			segyfileheader.sample_interval = segytraceheader.si_micros;
			segyfileheader.sample_interval_org = segytraceheader.si_micros;
			segyfileheader.number_samples = segytraceheader.nsamps;
			segyfileheader.number_samples_org = segytraceheader.nsamps;
			if (fwrite(&segyasciiheader, 1, MB_SEGY_ASCIIHEADER_LENGTH, ofp) 
						!= MB_SEGY_ASCIIHEADER_LENGTH)
				{
				status = MB_FAILURE;
				error = MB_ERROR_WRITE_FAIL;
				}
			else if (fwrite(&segyfileheader, 1, MB_SEGY_FILEHEADER_LENGTH, ofp) 
						!= MB_SEGY_FILEHEADER_LENGTH)
				{
				status = MB_FAILURE;
				error = MB_ERROR_WRITE_FAIL;
				}
			}
				    
		    /* note good status */
		    if (status == MB_SUCCESS)
		    	{
			/* get trace min and max */
			tracemin = segydata[0];
			tracemax = segydata[0];
			for (i=0;i<segytraceheader.nsamps;i++)
				{
				tracemin = MIN(tracemin, segydata[i]);
				tracemax = MAX(tracemax, segydata[i]);
				}
			
			/* output info */
			nread++;
			if (nread % 10 == 0 || verbose > 0)
			fprintf(stderr,"file:%s record:%d shot:%d  %4.4d/%3.3d %2.2d:%2.2d:%2.2d.%3.3d samples:%d interval:%d usec  minmax: %f %f\n",
				file,nread,segytraceheader.shot_num,
				segytraceheader.year,segytraceheader.day_of_yr,
				segytraceheader.hour,segytraceheader.min,segytraceheader.sec,segytraceheader.mils,
				segytraceheader.nsamps,segytraceheader.si_micros,tracemin,tracemax);
			
			/* insert segy header data into output buffer */
			index = 0;
			mb_put_binary_int(MB_NO, segytraceheader.seq_num, (void *) &buffer[index]); index += 4;
			mb_put_binary_int(MB_NO, segytraceheader.seq_reel, (void *) &buffer[index]); index += 4;
			mb_put_binary_int(MB_NO, segytraceheader.shot_num, (void *) &buffer[index]); index += 4;
			mb_put_binary_int(MB_NO, segytraceheader.shot_tr, (void *) &buffer[index]); index += 4;
			mb_put_binary_int(MB_NO, segytraceheader.espn, (void *) &buffer[index]); index += 4;
			mb_put_binary_int(MB_NO, segytraceheader.rp_num, (void *) &buffer[index]); index += 4;
			mb_put_binary_int(MB_NO, segytraceheader.rp_tr, (void *) &buffer[index]); index += 4;
			mb_put_binary_short(MB_NO, segytraceheader.trc_id, (void *) &buffer[index]); index += 2;
			mb_put_binary_short(MB_NO, segytraceheader.num_vstk, (void *) &buffer[index]); index += 2;
			mb_put_binary_short(MB_NO, segytraceheader.cdp_fold, (void *) &buffer[index]); index += 2;
			mb_put_binary_short(MB_NO, segytraceheader.use, (void *) &buffer[index]); index += 2;
			mb_put_binary_int(MB_NO, segytraceheader.range, (void *) &buffer[index]); index += 4;
			mb_put_binary_int(MB_NO, segytraceheader.grp_elev, (void *) &buffer[index]); index += 4;
			mb_put_binary_int(MB_NO, segytraceheader.src_elev, (void *) &buffer[index]); index += 4;
			mb_put_binary_int(MB_NO, segytraceheader.src_depth, (void *) &buffer[index]); index += 4;
			mb_put_binary_int(MB_NO, segytraceheader.grp_datum, (void *) &buffer[index]); index += 4;
			mb_put_binary_int(MB_NO, segytraceheader.src_datum, (void *) &buffer[index]); index += 4;
			mb_put_binary_int(MB_NO, segytraceheader.src_wbd, (void *) &buffer[index]); index += 4;
			mb_put_binary_int(MB_NO, segytraceheader.grp_wbd, (void *) &buffer[index]); index += 4;
        		mb_put_binary_short(MB_NO, segytraceheader.elev_scalar, (void *) &buffer[index]); index += 2;
        		mb_put_binary_short(MB_NO, segytraceheader.coord_scalar, (void *) &buffer[index]); index += 2;
			mb_put_binary_int(MB_NO, segytraceheader.src_long, (void *) &buffer[index]); index += 4;
			mb_put_binary_int(MB_NO, segytraceheader.src_lat, (void *) &buffer[index]); index += 4;
			mb_put_binary_int(MB_NO, segytraceheader.grp_long, (void *) &buffer[index]); index += 4;
			mb_put_binary_int(MB_NO, segytraceheader.grp_lat, (void *) &buffer[index]); index += 4;
        		mb_put_binary_short(MB_NO, segytraceheader.coord_units, (void *) &buffer[index]); index += 2;
        		mb_put_binary_short(MB_NO, segytraceheader.wvel, (void *) &buffer[index]); index += 2;
        		mb_put_binary_short(MB_NO, segytraceheader.sbvel, (void *) &buffer[index]); index += 2;
        		mb_put_binary_short(MB_NO, segytraceheader.src_up_vel, (void *) &buffer[index]); index += 2;
        		mb_put_binary_short(MB_NO, segytraceheader.grp_up_vel, (void *) &buffer[index]); index += 2;
        		mb_put_binary_short(MB_NO, segytraceheader.src_static, (void *) &buffer[index]); index += 2;
        		mb_put_binary_short(MB_NO, segytraceheader.grp_static, (void *) &buffer[index]); index += 2;
        		mb_put_binary_short(MB_NO, segytraceheader.tot_static, (void *) &buffer[index]); index += 2;
        		mb_put_binary_short(MB_NO, segytraceheader.laga, (void *) &buffer[index]); index += 2;
			mb_put_binary_int(MB_NO, segytraceheader.delay_mils, (void *) &buffer[index]); index += 4;
        		mb_put_binary_short(MB_NO, segytraceheader.smute_mils, (void *) &buffer[index]); index += 2;
        		mb_put_binary_short(MB_NO, segytraceheader.emute_mils, (void *) &buffer[index]); index += 2;
        		mb_put_binary_short(MB_NO, segytraceheader.nsamps, (void *) &buffer[index]); index += 2;
        		mb_put_binary_short(MB_NO, segytraceheader.si_micros, (void *) &buffer[index]); index += 2;
			for (i=0;i<19;i++)
				{
        			mb_put_binary_short(MB_NO, segytraceheader.other_1[i], (void *) &buffer[index]); index += 2;
				}
        		mb_put_binary_short(MB_NO, segytraceheader.year, (void *) &buffer[index]); index += 2;
        		mb_put_binary_short(MB_NO, segytraceheader.day_of_yr, (void *) &buffer[index]); index += 2;
        		mb_put_binary_short(MB_NO, segytraceheader.hour, (void *) &buffer[index]); index += 2;
        		mb_put_binary_short(MB_NO, segytraceheader.min, (void *) &buffer[index]); index += 2;
        		mb_put_binary_short(MB_NO, segytraceheader.sec, (void *) &buffer[index]); index += 2;
        		mb_put_binary_short(MB_NO, segytraceheader.mils, (void *) &buffer[index]); index += 2;
        		mb_put_binary_short(MB_NO, segytraceheader.tr_weight, (void *) &buffer[index]); index += 2;
			for (i=0;i<5;i++)
				{
        			mb_put_binary_short(MB_NO, segytraceheader.other_2[i], (void *) &buffer[index]); index += 2;
				}
			mb_put_binary_float(MB_NO, segytraceheader.delay, (void *) &buffer[index]); index += 4;
        		mb_put_binary_float(MB_NO, segytraceheader.smute_sec, (void *) &buffer[index]); index += 4;
        		mb_put_binary_float(MB_NO, segytraceheader.emute_sec, (void *) &buffer[index]); index += 4;
        		mb_put_binary_float(MB_NO, segytraceheader.si_secs, (void *) &buffer[index]); index += 4;
        		mb_put_binary_float(MB_NO, segytraceheader.wbt_secs, (void *) &buffer[index]); index += 4;
			mb_put_binary_int(MB_NO, segytraceheader.end_of_rp, (void *) &buffer[index]); index += 4;
        		mb_put_binary_float(MB_NO, segytraceheader.dummy1, (void *) &buffer[index]); index += 4;
        		mb_put_binary_float(MB_NO, segytraceheader.dummy2, (void *) &buffer[index]); index += 4;
        		mb_put_binary_float(MB_NO, segytraceheader.dummy3, (void *) &buffer[index]); index += 4;
        		mb_put_binary_float(MB_NO, segytraceheader.dummy4, (void *) &buffer[index]); index += 4;
        		mb_put_binary_float(MB_NO, segytraceheader.dummy5, (void *) &buffer[index]); index += 4;
        		mb_put_binary_float(MB_NO, segytraceheader.dummy6, (void *) &buffer[index]); index += 4;
        		mb_put_binary_float(MB_NO, segytraceheader.dummy7, (void *) &buffer[index]); index += 4;
        		mb_put_binary_float(MB_NO, segytraceheader.dummy8, (void *) &buffer[index]); index += 4;
        		mb_put_binary_float(MB_NO, segytraceheader.dummy9, (void *) &buffer[index]); index += 4;

			/* write out segy header */
			if (fwrite(buffer,1,MB_SEGY_TRACEHEADER_LENGTH,ofp) 
						!= MB_SEGY_TRACEHEADER_LENGTH)
				{
				status = MB_FAILURE;
				error = MB_ERROR_WRITE_FAIL;
				}
			
			/* insert segy data into output buffer */
			index = 0;
			for (i=0;i<segytraceheader.nsamps;i++)
				{
        			mb_put_binary_float(MB_NO, segydata[i], (void *) &buffer[index]); index += 4;
				}

			/* write out data */
			nwrite++;
			if (status == MB_SUCCESS
				&& fwrite(buffer, 1, segytraceheader.nsamps * samplesize, ofp) 
						!= segytraceheader.nsamps * samplesize)
				{
				status = MB_FAILURE;
				error = MB_ERROR_WRITE_FAIL;
				}
			
			}
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
		if (verbose >= 1 && kind == MB_DATA_COMMENT)
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
	status = mb_close(verbose,&mbio_ptr,&error);

	/* deallocate memory used for data arrays */
	mb_free(verbose,(char **)&beamflag,&error); 
	mb_free(verbose,(char **)&bath,&error); 
	mb_free(verbose,(char **)&bathacrosstrack,&error); 
	mb_free(verbose,(char **)&bathalongtrack,&error); 
	mb_free(verbose,(char **)&amp,&error); 
	mb_free(verbose,(char **)&ss,&error); 
	mb_free(verbose,(char **)&ssacrosstrack,&error); 
	mb_free(verbose,(char **)&ssalongtrack,&error); 

	/* deallocate memory used for segy data arrays */
	mb_free(verbose,(char **)&segydata,&error); 
	segydata_alloc = 0;
	mb_free(verbose,(char **)&buffer,&error); 
	buffer_alloc = 0;

	/* figure out whether and what to read next */
        if (read_datalist == MB_YES)
                {
		if (status = mb_datalist_read(verbose,datalist,
			    file,&format,&file_weight,&error)
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

	/* close any old output file unless a single file has been specified */
	if (ofp != NULL)
		{
		fclose(ofp);
		}

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
