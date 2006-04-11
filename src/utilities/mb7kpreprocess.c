/*--------------------------------------------------------------------
 *    The MB-system:	mb7kpreprocess.c	10/12/2005
 *    $Id: mb7kpreprocess.c,v 5.4 2006-04-11 19:19:29 caress Exp $
 *
 *    Copyright (c) 2005 by
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
 * mb7kpreprocess reads a Reson 7k format file, interpolates the
 * asynchronous navigation and attitude onto the multibeam data, 
 * and writes a new 7k file with that information correctly embedded
 * in the multibeam data. This program can also fix various problems
 * with 7k data (early generations of the 6046 datalogger failed to
 * to meet the data format specification exactly).
 *
 * Author:	D. W. Caress
 * Date:	October 12, 2005
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.3  2006/03/06 21:44:27  caress
 * Changed to handle old style Reson beam quality flags.
 *
 * Revision 5.2  2006/01/18 15:17:00  caress
 * Added stdlib.h include.
 *
 * Revision 5.1  2006/01/06 18:19:58  caress
 * Working towards 5.0.8
 *
 * Revision 5.0  2005/11/05 01:09:17  caress
 * Program to preprocess Reson 7k format data.
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* MBIO include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_define.h"
#include "../../include/mb_io.h"
#include "../../include/mbsys_reson7k.h"

#define MB7KPREPROCESS_ALLOC_CHUNK 1000
#define MB7KPREPROCESS_PROCESS		1
#define MB7KPREPROCESS_TIMESTAMPLIST	2

static char rcs_id[] = "$Id: mb7kpreprocess.c,v 5.4 2006-04-11 19:19:29 caress Exp $";

/*--------------------------------------------------------------------*/

main (int argc, char **argv)
{
	static char program_name[] = "mb7kpreprocess";
	static char help_message[] =  "mb7kpreprocess reads a Reson 7k format file, interpolates the\nasynchronous navigation and attitude onto the multibeam data, \nand writes a new 7k file with that information correctly embedded\nin the multibeam data. This program can also fix various problems\nwith 7k data.";
	static char usage_message[] = "mb7kpreprocess [-Ifile -Ooutfile -H -V]";
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
	void	*datalist;
	int	look_processed = MB_DATALIST_LOOK_UNSET;
	double	file_weight;
	int	format = 0;
	int	iformat = MBF_RESON7KR;
	int	oformat = MBF_RESON7KR;
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
	char	ifile[MB_PATH_MAXLINE];
	char	ofile[MB_PATH_MAXLINE];
	int	ofile_set = MB_NO;
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;
	int	obeams_bath;
	int	obeams_amp;
	int	opixels_ss;

	/* MBIO read values */
	void	*imbio_ptr = NULL;
	struct mb_io_struct *imb_io_ptr = NULL;
	void	*istore_ptr = NULL;
	struct mbsys_reson7k_struct *istore = NULL;
	void	*ombio_ptr = NULL;
	struct mb_io_struct *omb_io_ptr = NULL;
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
	char	comment[MB_COMMENT_MAXLINE];
	int	icomment = 0;
	
	/* program mode */
	int	mode = MB7KPREPROCESS_PROCESS;
	int	fix_time_stamps = MB_NO;
	
	/* data structure pointers */
	s7kr_fileheader		*fileheader;
	s7kr_position 		*position;
	s7kr_attitude 		*attitude;
	s7kr_volatilesettings	*volatilesettings;
	s7kr_beamgeometry	*beamgeometry;
	s7kr_bathymetry		*bathymetry;
	s7kr_backscatter	*backscatter;
	s7kr_beam		*beam;
	s7kr_image		*image;
	s7kr_bluefin		*bluefin;
	s7kr_fsdwss 		*fsdwsslo;
	s7kr_fsdwss 		*fsdwsshi;
	s7kr_fsdwsb 		*fsdwsb;
	s7k_fsdwchannel 	*fsdwchannel;
	s7k_fsdwssheader 	*fsdwssheader;
	s7k_fsdwsegyheader 	*fsdwsegyheader;
	s7k_header 		*header;
	
	/* counting variables */
	int	nreadfileheader = 0;
	int	nreadmultibeam = 0;
		int	nreadmbvolatilesettings = 0;
		int	nreadmbbeamgeometry = 0;
		int	nreadmbbathymetry = 0;
		int	nreadmbbackscatter = 0;
		int	nreadmbbeam = 0;
		int	nreadmbimage = 0;
	int	nreadssv = 0;
	int	nreadnav1 = 0;
	int	nreadsbp = 0;
	int	nreadsslo = 0;
	int	nreadsshi = 0;
	int	nreadother = 0;
	int	nreadfileheadertot = 0;
	int	nreadmultibeamtot = 0;
		int	nreadmbvolatilesettingstot = 0;
		int	nreadmbbeamgeometrytot = 0;
		int	nreadmbbathymetrytot = 0;
		int	nreadmbbackscattertot = 0;
		int	nreadmbbeamtot = 0;
		int	nreadmbimagetot = 0;
	int	nreadssvtot = 0;
	int	nreadnav1tot = 0;
	int	nreadsbptot = 0;
	int	nreadsslotot = 0;
	int	nreadsshitot = 0;
	int	nreadothertot = 0;
	
	/* navigation, heading, attitude data */
	int	nnav = 0;
	int	nnav_alloc = 0;
	double	*nav_time_d = NULL;
	int	*nav_quality = NULL;
	double	*nav_lon = NULL;
	double	*nav_lat = NULL;
	double	*nav_sonardepth = NULL;
	double	*nav_heading = NULL;
	double	*nav_speed = NULL;
	double	*nav_roll = NULL;
	double	*nav_pitch = NULL;
	int	nalt = 0;
	int	nalt_alloc = 0;
	double	*alt_time_d = NULL;
	double	*alt_altitude = NULL;
	
	/* bathymetry timetag data */
	int	nbatht = 0;
	int	nbatht_alloc = 0;
	double	*batht_time_d = NULL;
	int	*batht_ping = NULL;
	double	*batht_time_d_new = NULL;
	double	*batht_time_offset = NULL;
	int	*batht_ping_offset = NULL;
	int	*batht_good_offset = NULL;
	
	/* edgetech timetag data */
	int	nedget = 0;
	int	nedget_alloc = 0;
	double	*edget_time_d = NULL;
	int	*edget_ping = NULL;
	
	/* timelag parameters */
	double	timelag = 0.0;
	
	/* range offset parameters */
	int	nrangeoffset = 0;
	int	rangeoffsetstart[3];
	int	rangeoffsetend[3];
	double	rangeoffset[3];
	
	/* depth sensor lever arm parameter */
	double	depthsensoroffx = 0.0;
	double	depthsensoroffz = 0.0;
	double	sonardepthcorrection;
	
	int	interp_status;
	double	soundspeed;
	double	alpha, beta, theta, phi;
	double	rr, xx, zz;
	
	int	read_data;
	int	testformat;
	char	fileroot[MB_PATH_MAXLINE];
	int	found, jfound;
	int	d1, d2;
	double	v;
	int	sslo_lastread;
	double	sslo_last_time_d;
	int	sslo_last_ping;
	int	foundstart, foundend;
	int	start, end;
	int	i, j, n;

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* set default input to datalist.mb-1 */
	strcpy (read_file, "datalist.mb-1");

	/* process argument list */
	while ((c = getopt(argc, argv, "BbD:d:F:f:I:i:LlO:o:R:r:T:t:VvHh")) != -1)
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
			fix_time_stamps = MB_YES;
			break;
		case 'D':
		case 'd':
			sscanf (optarg,"%lf/%lf", &depthsensoroffx, &depthsensoroffz);
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
		case 'L':
		case 'l':
			mode = MB7KPREPROCESS_TIMESTAMPLIST;
			flag++;
			break;
		case 'O':
		case 'o':
			sscanf (optarg,"%s", ofile);
			ofile_set  = MB_YES;
			flag++;
			break;
		case 'R':
		case 'r':
			if (nrangeoffset < 3)
				{
				sscanf (optarg,"%d/%d/%lf", 
					&rangeoffsetstart[nrangeoffset], 
					&rangeoffsetend[nrangeoffset], 
					&rangeoffset[nrangeoffset]);
				nrangeoffset++;
				}
			flag++;
			break;
		case 'T':
		case 't':
			sscanf (optarg,"%lf", &timelag);
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
		fprintf(stderr,"dbg2       read_file:           %s\n",read_file);
		fprintf(stderr,"dbg2       ofile:               %s\n",ofile);
		fprintf(stderr,"dbg2       ofile_set:           %d\n",ofile_set);
		fprintf(stderr,"dbg2       mode:                %d\n",mode);
		fprintf(stderr,"dbg2       fix_time_stamps:     %d\n",fix_time_stamps);
		fprintf(stderr,"dbg2       timelag:             %f\n",timelag);
		fprintf(stderr,"dbg2       depthsensoroffx:     %f\n",depthsensoroffx);
		fprintf(stderr,"dbg2       depthsensoroffz:     %f\n",depthsensoroffz);
		for (i=0;i<nrangeoffset;i++)
			fprintf(stderr,"dbg2       rangeoffset[%d]:         %d %d %f\n",
				rangeoffsetstart[i], rangeoffsetend[i], rangeoffset[i]);
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
			    ifile,&format,&file_weight,&error)
			    == MB_SUCCESS)
		read_data = MB_YES;
	    else
		read_data = MB_NO;
	    }
	/* else copy single filename to be read */
	else
	    {
	    strcpy(ifile, read_file);
	    read_data = MB_YES;
	    }
	
	/* loop over all files to be read */
	while (read_data == MB_YES && format == MBF_RESON7KR)
	{

	/* initialize reading the swath file */
	if ((status = mb_read_init(
		verbose,ifile,format,pings,lonflip,bounds,
		btime_i,etime_i,speedmin,timegap,
		&imbio_ptr,&btime_d,&etime_d,
		&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
		fprintf(stderr,"\nMultibeam File <%s> not initialized for reading\n",ifile);
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
		

	/* read and print data */
	nreadfileheader = 0;
	nreadmultibeam = 0;
	nreadmbvolatilesettings = 0;
	nreadmbbeamgeometry = 0;
	nreadmbbathymetry = 0;
	nreadmbbackscatter = 0;
	nreadmbbeam = 0;
	nreadmbimage = 0;
	nreadssv = 0;
	nreadnav1 = 0;
	nreadsbp = 0;
	nreadsslo = 0;
	nreadsshi = 0;
	nreadother = 0;
	sslo_lastread = MB_NO;
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
		    
		/* some nonfatal errors do not matter */
		if (error < MB_ERROR_NO_ERROR && error > MB_ERROR_UNINTELLIGIBLE)
			{
			error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
			
	   	/* handle multibeam data */
		if (status == MB_SUCCESS && kind == MB_DATA_DATA) 
			{
			nreadmultibeam++;
			
			bathymetry = &(istore->bathymetry);
			if (istore->read_volatilesettings == MB_YES)
				nreadmbvolatilesettings++;
			if (istore->read_beamgeometry == MB_YES)
				nreadmbbeamgeometry++;
			if (istore->read_bathymetry == MB_YES)
				nreadmbbathymetry++;
			if (istore->read_backscatter == MB_YES)
				nreadmbbackscatter++;
			if (istore->read_beam == MB_YES)
				nreadmbbeam++;
			if (istore->read_image == MB_YES)
				nreadmbimage++;
				
			/* print out record headers */
			if (istore->read_volatilesettings == MB_YES)
				{
				volatilesettings = &(istore->volatilesettings);
				header = &(volatilesettings->header);
				time_j[0] = header->s7kTime.Year;
				time_j[1] = header->s7kTime.Day;
				time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
				time_j[3] = (int) header->s7kTime.Seconds;
				time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
				mb_get_itime(verbose, time_j, time_i);
				mb_get_time(verbose, time_i, &time_d);
				if (verbose > 0)
				fprintf(stderr,"R7KRECID_7kVolatileSonarSettings:  7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],time_i[6],
					header->RecordNumber);
				}
			if (istore->read_beamgeometry == MB_YES)
				{
				beamgeometry = &(istore->beamgeometry);
				header = &(beamgeometry->header);
				time_j[0] = header->s7kTime.Year;
				time_j[1] = header->s7kTime.Day;
				time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
				time_j[3] = (int) header->s7kTime.Seconds;
				time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
				mb_get_itime(verbose, time_j, time_i);
				mb_get_time(verbose, time_i, &time_d);
				if (verbose > 0)
				fprintf(stderr,"R7KRECID_7kBeamGeometry:           7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d beams:%d\n",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],time_i[6],
					header->RecordNumber,beamgeometry->number_beams);
				}
			if (istore->read_bathymetry == MB_YES)
				{
				bathymetry = &(istore->bathymetry);
				header = &(bathymetry->header);
				time_j[0] = header->s7kTime.Year;
				time_j[1] = header->s7kTime.Day;
				time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
				time_j[3] = (int) header->s7kTime.Seconds;
				time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
				mb_get_itime(verbose, time_j, time_i);
				mb_get_time(verbose, time_i, &time_d);
				if (verbose > 0)
				fprintf(stderr,"R7KRECID_7kBathymetricData:        7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d ping:%d beams:%d\n",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],time_i[6],
					header->RecordNumber,bathymetry->ping_number,bathymetry->number_beams);
				
				/* allocate memory for bathymetry timetag arrays if needed */
				if (nbatht == 0 || nbatht >= nbatht_alloc)
					{
					nbatht_alloc +=  MB7KPREPROCESS_ALLOC_CHUNK;
					status = mb_realloc(verbose,nbatht_alloc*sizeof(double),&batht_time_d,&error);
					status = mb_realloc(verbose,nbatht_alloc*sizeof(int),&batht_ping,&error);
					status = mb_realloc(verbose,nbatht_alloc*sizeof(double),&batht_time_d_new,&error);
					status = mb_realloc(verbose,nbatht_alloc*sizeof(double),&batht_time_offset,&error);
					status = mb_realloc(verbose,nbatht_alloc*sizeof(int),&batht_ping_offset,&error);
					status = mb_realloc(verbose,nbatht_alloc*sizeof(int),&batht_good_offset,&error);
					if (error != MB_ERROR_NO_ERROR)
						{
						mb_error(verbose,error,&message);
						fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
						fprintf(stderr,"\nProgram <%s> Terminated\n",
						    program_name);
						exit(error);
						}		    
					}

				/* store the bathtech time stamp */
				if (nbatht == 0 || time_d > batht_time_d[nbatht-1])
					{
					batht_time_d[nbatht] = time_d;
					batht_ping[nbatht] = bathymetry->ping_number;
					
					/* grab the last sslo ping if it was the last thing read */
					if (sslo_lastread == MB_YES)
						{
						batht_time_offset[nbatht] = sslo_last_time_d - time_d;
						batht_ping_offset[nbatht] = sslo_last_ping - bathymetry->ping_number;
						batht_good_offset[nbatht] = MB_YES;
						}
					else
						{
						batht_time_offset[nbatht] = -9999.99;
						batht_ping_offset[nbatht] = 0;
						batht_good_offset[nbatht] = MB_NO;
						}
					nbatht++;
					}
				}
			if (istore->read_backscatter == MB_YES)
				{
				backscatter = &(istore->backscatter);
				header = &(backscatter->header);
				time_j[0] = header->s7kTime.Year;
				time_j[1] = header->s7kTime.Day;
				time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
				time_j[3] = (int) header->s7kTime.Seconds;
				time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
				mb_get_itime(verbose, time_j, time_i);
				mb_get_time(verbose, time_i, &time_d);
				if (verbose > 0)
				fprintf(stderr,"R7KRECID_7kBackscatterImageData:   7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d ping:%d samples:%d\n",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],time_i[6],
					header->RecordNumber,backscatter->ping_number,backscatter->number_samples);
				}
			if (istore->read_beam == MB_YES)
				{
				beam = &(istore->beam);
				header = &(beam->header);
				time_j[0] = header->s7kTime.Year;
				time_j[1] = header->s7kTime.Day;
				time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
				time_j[3] = (int) header->s7kTime.Seconds;
				time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
				mb_get_itime(verbose, time_j, time_i);
				mb_get_time(verbose, time_i, &time_d);
				if (verbose > 0)
				fprintf(stderr,"R7KHDRSIZE_7kBeamData: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d ping:%d beams:%d samples:%d\n",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],time_i[6],
					header->RecordNumber,beam->ping_number,beam->number_beams,beam->number_samples);
				}
			if (istore->read_image == MB_YES)
				{
				image = &(istore->image);
				header = &(image->header);
				time_j[0] = header->s7kTime.Year;
				time_j[1] = header->s7kTime.Day;
				time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
				time_j[3] = (int) header->s7kTime.Seconds;
				time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
				mb_get_itime(verbose, time_j, time_i);
				mb_get_time(verbose, time_i, &time_d);
				if (verbose > 0)
				fprintf(stderr,"R7KRECID_7kImageData:              7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d ping:%d width:%d height:%d\n",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],time_i[6],
					header->RecordNumber,image->ping_number,image->width,image->height);
				}

			}
			
	   	/* handle file header data */
		else if (status == MB_SUCCESS && kind == MB_DATA_HEADER) 
			{
			nreadfileheader++;
			
			fileheader = &(istore->fileheader);
			header = &(fileheader->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);
			if (verbose > 0)
				fprintf(stderr,"R7KRECID_7kFileHeader:             7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
				time_i[0],time_i[1],time_i[2],
				time_i[3],time_i[4],time_i[5],time_i[6],
				header->RecordNumber);
			}
			
	   	/* handle bluefin ctd data */
		else if (status == MB_SUCCESS && kind == MB_DATA_SSV) 
			{
			nreadssv++;
			
			bluefin = &(istore->bluefin);
			header = &(bluefin->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);

			if (verbose > 0)
				fprintf(stderr,"R7KRECID_BluefinEnvironmental:     7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
			time_i[0],time_i[1],time_i[2],
			time_i[3],time_i[4],time_i[5],time_i[6],
			header->RecordNumber);
			for (i=0;i<bluefin->number_frames;i++)
				{
				time_j[0] = bluefin->environmental[i].s7kTime.Year;
				time_j[1] = bluefin->environmental[i].s7kTime.Day;
				time_j[2] = 60 * bluefin->environmental[i].s7kTime.Hours + bluefin->environmental[i].s7kTime.Minutes;
				time_j[3] = (int) bluefin->environmental[i].s7kTime.Seconds;
				time_j[4] = (int) (1000000 * (bluefin->environmental[i].s7kTime.Seconds - time_j[3]));
				mb_get_itime(verbose, time_j, time_i);
				if (verbose > 0)
				fprintf(stderr,"                       %2.2d          7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) CTD_time:%f T_time:%f\n",
					i,time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],time_i[6],
					bluefin->environmental[i].ctd_time,
					bluefin->environmental[i].temperature_time);
				}
			}
			
	   	/* handle bluefin nav data */
		else if (status == MB_SUCCESS && kind == MB_DATA_NAV2) 
			{
			nreadnav1++;
			
			bluefin = &(istore->bluefin);
			header = &(bluefin->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);

			if (verbose > 0)
				fprintf(stderr,"R7KRECID_BluefinNav:               7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
			time_i[0],time_i[1],time_i[2],
			time_i[3],time_i[4],time_i[5],time_i[6],
			header->RecordNumber);
			for (i=0;i<bluefin->number_frames;i++)
				{
				time_j[0] = bluefin->nav[i].s7kTime.Year;
				time_j[1] = bluefin->nav[i].s7kTime.Day;
				time_j[2] = 60 * bluefin->nav[i].s7kTime.Hours + bluefin->nav[i].s7kTime.Minutes;
				time_j[3] = (int) bluefin->nav[i].s7kTime.Seconds;
				time_j[4] = (int) (1000000 * (bluefin->nav[i].s7kTime.Seconds - time_j[3]));
				mb_get_itime(verbose, time_j, time_i);
				if (verbose > 0)
				fprintf(stderr,"                       %2.2d          7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) Pos_time:%f\n",
					i,time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],time_i[6],
					bluefin->nav[i].position_time);
				}
				
			/* allocate memory for navigation/attitude arrays if needed */
			if (bluefin->number_frames > 0 
				&& nnav + bluefin->number_frames > nnav_alloc)
				{
				nnav_alloc +=  MB7KPREPROCESS_ALLOC_CHUNK;
				status = mb_realloc(verbose,nnav_alloc*sizeof(double),&nav_time_d,&error);
				status = mb_realloc(verbose,nnav_alloc*sizeof(int),&nav_quality,&error);
				status = mb_realloc(verbose,nnav_alloc*sizeof(double),&nav_lon,&error);
				status = mb_realloc(verbose,nnav_alloc*sizeof(double),&nav_lat,&error);
				status = mb_realloc(verbose,nnav_alloc*sizeof(double),&nav_speed,&error);
				status = mb_realloc(verbose,nnav_alloc*sizeof(double),&nav_sonardepth,&error);
				status = mb_realloc(verbose,nnav_alloc*sizeof(double),&nav_heading,&error);
				status = mb_realloc(verbose,nnav_alloc*sizeof(double),&nav_roll,&error);
				status = mb_realloc(verbose,nnav_alloc*sizeof(double),&nav_pitch,&error);
				if (error != MB_ERROR_NO_ERROR)
					{
					mb_error(verbose,error,&message);
					fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
					fprintf(stderr,"\nProgram <%s> Terminated\n",
					    program_name);
					exit(error);
					}		    
				}
			if (bluefin->number_frames > 0 
				&& nalt + bluefin->number_frames > nalt_alloc)
				{
				nalt_alloc +=  MB7KPREPROCESS_ALLOC_CHUNK;
				status = mb_realloc(verbose,nalt_alloc*sizeof(double),&alt_time_d,&error);
				status = mb_realloc(verbose,nalt_alloc*sizeof(double),&alt_altitude,&error);
				}
				
			/* store the navigation and attitude data */
			for (i=0;i<bluefin->number_frames;i++)
				{
				if (nnav == 0 || nav_time_d[nnav-1] < bluefin->nav[i].position_time)
					{
					nav_time_d[nnav] = bluefin->nav[i].position_time;
					nav_quality[nnav] = bluefin->nav[i].quality;
					nav_lon[nnav] = RTD * bluefin->nav[i].longitude;
					nav_lat[nnav] = RTD * bluefin->nav[i].latitude;
					nav_speed[nnav] = bluefin->nav[i].speed;
					nav_sonardepth[nnav] = bluefin->nav[i].depth 
						+ depthsensoroffx * sin(bluefin->nav[i].pitch)
						+ depthsensoroffz * cos(bluefin->nav[i].pitch);
					nav_heading[nnav] = RTD * bluefin->nav[i].yaw;
					nav_roll[nnav] = RTD * bluefin->nav[i].roll;
					nav_pitch[nnav] = RTD * bluefin->nav[i].pitch;
					nnav++;
					}
				if (nalt == 0 || (alt_time_d[nalt-1] < bluefin->nav[i].altitude_time
							&& alt_altitude[nalt-1] != bluefin->nav[i].altitude))
					{
					alt_time_d[nalt] = bluefin->nav[i].altitude_time;
					alt_altitude[nalt] = bluefin->nav[i].altitude;
					nalt++;
					}
				}
			
			}
			
	   	/* handle subbottom data */
		else if (status == MB_SUCCESS && kind == MB_DATA_SUBBOTTOM_SUBBOTTOM) 
			{
			nreadsbp++;

			fsdwsb = &(istore->fsdwsb);
			header = &(fsdwsb->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);
			fsdwchannel = &(fsdwsb->channel);
			fsdwsegyheader = &(fsdwsb->segyheader);
			if (verbose > 0)
				fprintf(stderr,"R7KRECID_FSDWsubbottom:            7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) FSDWtime(%4.4d-%3.3d %2.2d:%2.2d:%2.2d.%3.3d) ping:%d sampint:%d samples:%d\n",
				time_i[0],time_i[1],time_i[2],
				time_i[3],time_i[4],time_i[5],time_i[6],
				fsdwsegyheader->year,fsdwsegyheader->day,fsdwsegyheader->hour,fsdwsegyheader->minute,fsdwsegyheader->second,
				fsdwsegyheader->millisecondsToday - 1000 * (int)(0.001 * fsdwsegyheader->millisecondsToday),
				fsdwsb->ping_number,fsdwchannel->sample_interval,fsdwchannel->number_samples);
			}
			
	   	/* handle low frequency sidescan data */
		else if (status == MB_SUCCESS && kind == MB_DATA_SIDESCAN2) 
			{
			nreadsslo++;

			fsdwsslo = &(istore->fsdwsslo);
			header = &(fsdwsslo->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);
			for (i=0;i<fsdwsslo->number_channels;i++)
				{
				fsdwchannel = &(fsdwsslo->channel[i]);
				fsdwssheader = &(fsdwsslo->ssheader[i]);
				if (verbose > 0)
				fprintf(stderr,"R7KRECID_FSDWsidescanLo:           7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) FSDWtime(%4.4d-%3.3d %2.2d:%2.2d:%2.2d.%3.3d) ping:%d channel:%d sampint:%d samples:%d\n",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],time_i[6],
					fsdwssheader->year,fsdwssheader->day,fsdwssheader->hour,fsdwssheader->minute,fsdwssheader->second,
					fsdwssheader->millisecondsToday - 1000 * (int)(0.001 * fsdwssheader->millisecondsToday),
					fsdwsslo->ping_number,fsdwchannel->number,
					fsdwchannel->sample_interval,fsdwchannel->number_samples);
				}
				
			/* allocate memory for edgetech timetag arrays if needed */
			if (nedget == 0 || nedget > nedget_alloc)
				{
				nedget_alloc +=  MB7KPREPROCESS_ALLOC_CHUNK;
				status = mb_realloc(verbose,nedget_alloc*sizeof(double),&edget_time_d,&error);
				status = mb_realloc(verbose,nedget_alloc*sizeof(int),&edget_ping,&error);
				if (error != MB_ERROR_NO_ERROR)
					{
					mb_error(verbose,error,&message);
					fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
					fprintf(stderr,"\nProgram <%s> Terminated\n",
					    program_name);
					exit(error);
					}		    
				}
				
			/* store the edgetech time stamp */
			fsdwchannel = &(fsdwsslo->channel[0]);
			fsdwssheader = &(fsdwsslo->ssheader[0]);
			time_j[0] = fsdwssheader->year;
			time_j[1] = fsdwssheader->day;
			time_j[2] = 60 * fsdwssheader->hour + fsdwssheader->minute;
			time_j[3] = fsdwssheader->second;
			time_j[4] = 1000 * (fsdwssheader->millisecondsToday 
						- 1000 * ((int)(0.001 * fsdwssheader->millisecondsToday)));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);
			if (nedget == 0 || time_d > edget_time_d[nedget-1])
				{
				edget_time_d[nedget] = time_d;
				edget_ping[nedget] = fsdwssheader->pingNum;
				nedget++;
				}
			sslo_last_time_d = time_d;
			sslo_last_ping = fsdwssheader->pingNum;
			}
			
	   	/* handle high frequency sidescan data */
		else if (status == MB_SUCCESS && kind == MB_DATA_SIDESCAN3) 
			{
			nreadsshi++;

			fsdwsshi = &(istore->fsdwsshi);
			header = &(fsdwsshi->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);
			for (i=0;i<fsdwsshi->number_channels;i++)
				{
				fsdwchannel = &(fsdwsshi->channel[i]);
				fsdwssheader = &(fsdwsshi->ssheader[i]);
				if (verbose > 0)
				fprintf(stderr,"R7KRECID_FSDWsidescanHi:           7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) FSDWtime(%4.4d-%3.3d %2.2d:%2.2d:%2.2d.%3.3d) ping:%d channel:%d sampint:%d samples:%d\n",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],time_i[6],
					fsdwssheader->year,fsdwssheader->day,fsdwssheader->hour,fsdwssheader->minute,fsdwssheader->second,
					fsdwssheader->millisecondsToday - 1000 * (int)(0.001 * fsdwssheader->millisecondsToday),
					fsdwsshi->ping_number,fsdwchannel->number,
					fsdwchannel->sample_interval,fsdwchannel->number_samples);
				}
			}
			
	   	/* handle unknown data */
		else  if (status == MB_SUCCESS) 
			{
/*fprintf(stderr,"DATA TYPE UNKNOWN: status:%d error:%d kind:%d\n",status,error,kind);*/
			nreadother++;
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
			
		/* set sslo_lastread flag */
		if (status == MB_SUCCESS && kind == MB_DATA_SIDESCAN2)
			sslo_lastread = MB_YES;
		else
			sslo_lastread = MB_NO;
		}

	/* close the swath file */
	status = mb_close(verbose,&imbio_ptr,&error);
	
	/* output counts */
	fprintf(stdout, "\nData records read from: %s\n", ifile);
	fprintf(stdout, "     File Header:       %d\n", nreadfileheader);
	fprintf(stdout, "     Multibeam:         %d\n", nreadmultibeam);
	fprintf(stdout, "          Volatile Settings: %d\n", nreadmbvolatilesettings);
	fprintf(stdout, "          Beam Geometry:     %d\n", nreadmbbeamgeometry);
	fprintf(stdout, "          Bathymetry:        %d\n", nreadmbbathymetry);
	fprintf(stdout, "          Backscatter:       %d\n", nreadmbbackscatter);
	fprintf(stdout, "          Beam:              %d\n", nreadmbbeam);
	fprintf(stdout, "          Image:             %d\n", nreadmbimage);
	fprintf(stdout, "     Bluefin CTD:       %d\n", nreadssv);
	fprintf(stdout, "     Bluefin Nav:       %d\n", nreadnav1);
	fprintf(stdout, "     Subbottom:         %d\n", nreadsbp);
	fprintf(stdout, "     Low Sidescan:      %d\n", nreadsslo);
	fprintf(stdout, "     High Sidescan:     %d\n", nreadsshi);
	nreadfileheadertot += nreadfileheader;
	nreadmultibeamtot += nreadmultibeam;
	nreadmbvolatilesettingstot += nreadmbvolatilesettings;
	nreadmbbeamgeometrytot += nreadmbbeamgeometry;
	nreadmbbathymetrytot += nreadmbbathymetry;
	nreadmbbackscattertot += nreadmbbackscatter;
	nreadmbbeamtot += nreadmbbeam;
	nreadmbimagetot += nreadmbimage;
	nreadssvtot += nreadssv;
	nreadnav1tot += nreadnav1;
	nreadsbptot += nreadsbp;
fprintf(stderr,"Set nreadsbptot:%d\n",nreadsbptot);
	nreadsslotot += nreadsslo;
	nreadsshitot += nreadsshi;
	nreadothertot += nreadother;

	/* figure out whether and what to read next */
        if (read_datalist == MB_YES)
                {
		if (status = mb_datalist_read(verbose,datalist,
			    ifile,&format,&file_weight,&error)
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
		
	/* fix problems with batht timestamp arrays */
	for (i=0;i<nbatht-1;i++)
		{
		if (batht_good_offset[i+1] == MB_NO)
			{
			batht_good_offset[i] = MB_NO;
			}
		}
	for (i=0;i<nbatht;i++)
		{
		if (batht_good_offset[i] == MB_NO)
			{
			foundstart = MB_NO;
			foundend = MB_NO;
			for (j = i - 1; j >= 0 && foundstart == MB_NO; j--)
				{
				if (batht_good_offset[j] == MB_YES)
					{
					foundstart = MB_YES;
					start = j;
					}
				}
			for (j = i + 1; j < nbatht && foundend == MB_NO; j++)
				{
				if (batht_good_offset[j] == MB_YES)
					{
					foundend = MB_YES;
					end = j;
					}
				}
			if (foundstart == MB_YES && foundend == MB_YES)
				{
				batht_time_offset[i] = batht_time_offset[start] 
							+ (batht_time_offset[end] - batht_time_offset[start])
								* ((double)(i - start)) / ((double)(end - start));
				}
			else if (foundstart == MB_YES)
				{
				batht_time_offset[i] = batht_time_offset[start];
				}
			else if (foundend == MB_YES)
				{
				batht_time_offset[i] = batht_time_offset[end];
				}
			}
		batht_time_d_new[i] = batht_time_d[i] + batht_time_offset[i];
		}	
		
	/* output navigation and attitude data */
	if (verbose > 0 || mode == MB7KPREPROCESS_TIMESTAMPLIST)
		{
		fprintf(stdout, "\nTotal navigation/attitude data read: %d\n", nnav);
		for (i=0;i<nnav;i++)
			{
			fprintf(stdout, "  NAV: %5d %12d %17.6f %11.6f %10.6f %8.3f %7.3f %6.3f %6.3f %6.3f\n", 
				i, nav_quality[i], nav_time_d[i], nav_lon[i], nav_lat[i],
				nav_sonardepth[i], nav_heading[i], nav_speed[i],
				nav_roll[i], nav_pitch[i]);
			}
		for (i=0;i<nalt;i++)
			{
			fprintf(stdout, "  ALT: %5d %17.6f %8.3f\n", 
				i, alt_time_d[i], alt_altitude[i]);
			}
		for (i=0;i<nedget;i++)
			{
			fprintf(stdout, "  EDG: %5d %17.6f %d\n", 
				i, edget_time_d[i], edget_ping[i]);
			}
		for (i=0;i<nbatht;i++)
			{
			fprintf(stdout, "  BAT: %5d %17.6f %17.6f %5d   offsets: %17.6f %5d  %5d\n", 
				i, batht_time_d[i], batht_time_d_new[i], batht_ping[i], batht_time_offset[i], batht_ping_offset[i], batht_good_offset[i]);
			}
		}
	
	/* output counts */
	fprintf(stdout, "\nTotal data records read from: %s\n", read_file);
	fprintf(stdout, "     File Header:       %d\n", nreadfileheadertot);
	fprintf(stdout, "     Multibeam:         %d\n", nreadmultibeamtot);
	fprintf(stdout, "          Volatile Settings: %d\n", nreadmbvolatilesettingstot);
	fprintf(stdout, "          Beam Geometry:     %d\n", nreadmbbeamgeometrytot);
	fprintf(stdout, "          Bathymetry:        %d\n", nreadmbbathymetrytot);
	fprintf(stdout, "          Backscatter:       %d\n", nreadmbbackscattertot);
	fprintf(stdout, "          Beam:              %d\n", nreadmbbeamtot);
	fprintf(stdout, "          Image:             %d\n", nreadmbimagetot);
	fprintf(stdout, "     Bluefin CTD:       %d\n", nreadssvtot);
	fprintf(stdout, "     Bluefin Nav:       %d\n", nreadnav1tot);
	fprintf(stdout, "     Subbottom:         %d\n", nreadsbptot);
	fprintf(stdout, "     Low Sidescan:      %d\n", nreadsslotot);
	fprintf(stdout, "     High Sidescan:     %d\n", nreadsshitot);

	nreadfileheadertot = 0;
	nreadmultibeamtot = 0;
	nreadmbvolatilesettingstot = 0;
	nreadmbbeamgeometrytot = 0;
	nreadmbbathymetrytot = 0;
	nreadmbbackscattertot = 0;
	nreadmbbeamtot = 0;
	nreadmbimagetot = 0;
	nreadssvtot = 0;
	nreadnav1tot = 0;
	nreadsbptot = 0;
	nreadsslotot = 0;
	nreadsshitot = 0;
	nreadothertot = 0;

	/* now read the data files again, this time interpolating nav and attitude
		into the multibeam records and fixing other problems found in the
		data */
	if (mode == MB7KPREPROCESS_PROCESS)
	{

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
			    ifile,&format,&file_weight,&error)
			    == MB_SUCCESS)
		read_data = MB_YES;
	    else
		read_data = MB_NO;
	    }
	/* else copy single filename to be read */
	else
	    {
	    strcpy(ifile, read_file);
	    read_data = MB_YES;
	    }
	
	/* loop over all files to be read */
	while (read_data == MB_YES && format == MBF_RESON7KR)
	{
	/* figure out the output file name */
	status = mb_get_format(verbose, ifile, fileroot, &testformat, error);
	if (testformat == MBF_RESON7KR 
		&& strncmp(".s7k",&ifile[strlen(ifile)-4],4) == 0)
		sprintf(ofile, "%s.mb%d", fileroot, testformat);
	else if (testformat == MBF_RESON7KR)
		sprintf(ofile, "%sf.mb%d", fileroot, testformat);
	else if (testformat == MBF_RESON7KR)
		sprintf(ofile, "%s.mb%d", ifile, testformat);

	/* initialize reading the input swath file */
	if ((status = mb_read_init(
		verbose,ifile,format,pings,lonflip,bounds,
		btime_i,etime_i,speedmin,timegap,
		&imbio_ptr,&btime_d,&etime_d,
		&beams_bath,&beams_amp,&pixels_ss,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_read_init>:\n%s\n",message);
		fprintf(stderr,"\nMultibeam File <%s> not initialized for reading\n",ifile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* initialize writing the output swath sonar file */
	if ((status = mb_write_init(
		verbose,ofile,format,&ombio_ptr,
		&obeams_bath,&obeams_amp,&opixels_ss,&error)) != MB_SUCCESS)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error returned from function <mb_write_init>:\n%s\n",message);
		fprintf(stderr,"\nMultibeam File <%s> not initialized for writing\n",ofile);
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
		
	/* read and print data */
	nreadfileheader = 0;
	nreadmultibeam = 0;
	nreadmbvolatilesettings = 0;
	nreadmbbeamgeometry = 0;
	nreadmbbathymetry = 0;
	nreadmbbackscatter = 0;
	nreadmbbeam = 0;
	nreadmbimage = 0;
	nreadssv = 0;
	nreadnav1 = 0;
	nreadsbp = 0;
	nreadsslo = 0;
	nreadsshi = 0;
	nreadother = 0;
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
		    
		/* some nonfatal errors do not matter */
		if (error < MB_ERROR_NO_ERROR && error > MB_ERROR_UNINTELLIGIBLE)
			{
			error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
			
	   	/* handle multibeam data */
		if (status == MB_SUCCESS && kind == MB_DATA_DATA) 
			{
			nreadmultibeam++;
			
			bathymetry = &(istore->bathymetry);
			if (istore->read_volatilesettings == MB_YES)
				nreadmbvolatilesettings++;
			if (istore->read_beamgeometry == MB_YES)
				nreadmbbeamgeometry++;
			if (istore->read_bathymetry == MB_YES)
				nreadmbbathymetry++;
			if (istore->read_backscatter == MB_YES)
				nreadmbbackscatter++;
			if (istore->read_beam == MB_YES)
				nreadmbbeam++;
			if (istore->read_image == MB_YES)
				nreadmbimage++;
				
			/* print out record headers */
			if (istore->read_volatilesettings == MB_YES)
				{
				volatilesettings = &(istore->volatilesettings);
				header = &(volatilesettings->header);
				time_j[0] = header->s7kTime.Year;
				time_j[1] = header->s7kTime.Day;
				time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
				time_j[3] = (int) header->s7kTime.Seconds;
				time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
				mb_get_itime(verbose, time_j, time_i);
				mb_get_time(verbose, time_i, &time_d);
				if (verbose > 0)
				fprintf(stderr,"R7KRECID_7kVolatileSonarSettings:  7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],time_i[6],
					header->RecordNumber);
				}
			if (istore->read_beamgeometry == MB_YES)
				{
				beamgeometry = &(istore->beamgeometry);
				header = &(beamgeometry->header);
				time_j[0] = header->s7kTime.Year;
				time_j[1] = header->s7kTime.Day;
				time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
				time_j[3] = (int) header->s7kTime.Seconds;
				time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
				mb_get_itime(verbose, time_j, time_i);
				mb_get_time(verbose, time_i, &time_d);
				if (verbose > 0)
				fprintf(stderr,"R7KRECID_7kBeamGeometry:           7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d beams:%d\n",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],time_i[6],
					header->RecordNumber,beamgeometry->number_beams);
				}
			if (istore->read_bathymetry == MB_YES)
				{
				bathymetry = &(istore->bathymetry);
				header = &(bathymetry->header);
				time_j[0] = header->s7kTime.Year;
				time_j[1] = header->s7kTime.Day;
				time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
				time_j[3] = (int) header->s7kTime.Seconds;
				time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
				mb_get_itime(verbose, time_j, time_i);
				mb_get_time(verbose, time_i, &time_d);
				if (verbose > 0)
				fprintf(stderr,"R7KRECID_7kBathymetricData:        7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d ping:%d beams:%d\n",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],time_i[6],
					header->RecordNumber,bathymetry->ping_number,bathymetry->number_beams);
					
				/* fix time stamp */
				if (fix_time_stamps == MB_YES)
					{
					found = MB_NO;
					for (j=0; j < nbatht && found == MB_NO; j++)
						{
						if (bathymetry->ping_number == batht_ping[j])
							{
							found = MB_YES;
							time_d = batht_time_d_new[j];
							mb_get_date(verbose, time_d, time_i);
							mb_get_jtime(verbose, time_i, time_j);
							header->s7kTime.Year = time_j[0];
							header->s7kTime.Day = time_j[1];
							header->s7kTime.Hours = time_i[3];
							header->s7kTime.Minutes = time_i[4];
							header->s7kTime.Seconds = time_i[5] + 0.000001 * time_i[6];
							}
						}
					}

				/* fix version 4 quality flags */
				if (bathymetry->header.Version < 5)
					{
					for (i=0;i<bathymetry->number_beams;i++)
						{
						if ((bathymetry->quality[i]) < 16)
							{
							if (bathymetry->range[i] > 0.007)
								{
								bathymetry->quality[i] = (bathymetry->quality[i] & 240) + 15;
								}
							else
								{
								bathymetry->quality[i] = (bathymetry->quality[i] & 240) + 3;
								}
							}
						}
					}
					
				/* fix early version 5 quality flags */
				else if (bathymetry->header.Version == 5
						&& header->s7kTime.Year < 2006)
					{
					for (i=0;i<bathymetry->number_beams;i++)
						{
						/* phase picks */
						if ((bathymetry->quality[i]) == 8)
							{
/*fprintf(stderr,"beam %d: PHASE quality: %d",i,bathymetry->quality[i]);*/
							bathymetry->quality[i] = 32 + 15;
/*fprintf(stderr," %d\n",bathymetry->quality[i]);*/
							}
						else if ((bathymetry->quality[i]) == 4)
							{
/*fprintf(stderr,"beam %d: AMPLI quality: %d",i,bathymetry->quality[i]);*/
							bathymetry->quality[i] = 16 + 15;
/*fprintf(stderr," %d\n",bathymetry->quality[i]);*/
							}
						}
					}
					
				/* fix early version 5 quality flags */
				else if (bathymetry->header.Version == 5)
					{
					for (i=0;i<bathymetry->number_beams;i++)
						{
						/* phase picks */
						if ((bathymetry->quality[i]) == 4)
							{
/*fprintf(stderr,"beam %d: PHASE quality: %d",i,bathymetry->quality[i]);*/
							bathymetry->quality[i] = 32 + 15;
/*fprintf(stderr," %d\n",bathymetry->quality[i]);*/
							}
						else if ((bathymetry->quality[i]) == 2)
							{
/*fprintf(stderr,"beam %d: AMPLI quality: %d",i,bathymetry->quality[i]);*/
							bathymetry->quality[i] = 16 + 15;
/*fprintf(stderr," %d\n",bathymetry->quality[i]);*/
							}
						}
					}
					
				/* apply specified offsets to range values */
				for (j=0;j<nrangeoffset;j++)
					{
					for (i=rangeoffsetstart[j];i<=rangeoffsetend[j];i++)
						{
						bathymetry->range[i] += rangeoffset[j];
						}
					}

				/* recalculate optional values in bathymetry record */
				
				/* get navigation, etc */
				speed = 0.0;
				interp_status = mb_linear_interp_degrees(verbose, 
							nav_time_d-1, nav_heading-1,
							nnav, (time_d + timelag), &heading, &j, 
							&error);
				interp_status = mb_linear_interp(verbose, 
							nav_time_d-1, nav_lon-1,
							nnav, (time_d + timelag), &navlon, &j, 
							&error);
				interp_status = mb_linear_interp(verbose, 
							nav_time_d-1, nav_lat-1,
							nnav, (time_d + timelag), &navlat, &j, 
							&error);
				interp_status = mb_linear_interp(verbose, 
							nav_time_d-1, nav_sonardepth-1,
							nnav, (time_d + timelag), &sonardepth, &j, 
							&error);
				interp_status = mb_linear_interp(verbose, 
							alt_time_d-1, alt_altitude-1,
							nalt, (time_d + timelag), &altitude, &j, 
							&error);
				interp_status = mb_linear_interp(verbose, 
							nav_time_d-1, nav_roll-1,
							nnav, (time_d + timelag), &roll, &j, 
							&error);
				interp_status = mb_linear_interp(verbose, 
							nav_time_d-1, nav_pitch-1,
							nnav, (time_d + timelag), &pitch, &j, 
							&error);

				/* if the optional data are not all available, this ping
					is not useful, and is discarded by setting
					*error to MB_ERROR_UNINTELLIGIBLE */
				if (interp_status == MB_FAILURE)
					{
					status = MB_FAILURE;
					error = MB_ERROR_UNINTELLIGIBLE;
					}

				/* if the optional data are available, then proceed */
				else
					{
					bathymetry->longitude = DTR * navlon;
					bathymetry->latitude = DTR * navlat;
					bathymetry->heading = DTR * heading;
					bathymetry->height_source = 1;
					bathymetry->tide = 0.0;
					bathymetry->roll = DTR * roll;
					bathymetry->pitch = DTR * pitch;
					bathymetry->heave = 0.0;
					bathymetry->vehicle_height = -sonardepth;

					/* get bathymetry */
					if (volatilesettings->sound_velocity > 0.0)
						soundspeed = volatilesettings->sound_velocity;
					else if (bluefin->environmental[0].sound_speed > 0.0)
						soundspeed = bluefin->environmental[0].sound_speed;
					else
						soundspeed = 1500.0;
					for (i=0;i<bathymetry->number_beams;i++)
						{
						if ((bathymetry->quality[i] & 15) > 0)
							{
							alpha = RTD * (beamgeometry->angle_alongtrack[i] + bathymetry->pitch);
							beta = 90.0 - RTD * (beamgeometry->angle_acrosstrack[i] - bathymetry->roll);
							mb_rollpitch_to_takeoff(
								verbose, 
								alpha, beta, 
								&theta, &phi, 
								&error);
							rr = 0.5 * soundspeed * bathymetry->range[i];
							xx = rr * sin(DTR * theta);
							zz = rr * cos(DTR * theta);
							bathymetry->acrosstrack[i] = xx * cos(DTR * phi);
							bathymetry->alongtrack[i] = xx * sin(DTR * phi);
							bathymetry->depth[i] = zz + sonardepth;
							bathymetry->pointing_angle[i] = DTR * theta;
							bathymetry->azimuth_angle[i] = DTR * phi;
/*fprintf(stderr,"i:%d roll:%f %f pitch:%f %f   depth:%f %f %f\n",
i,roll,pitch, bathymetry->roll, bathymetry->pitch,
bathymetry->depth[i],bathymetry->acrosstrack[i],bathymetry->alongtrack[i]);*/
							}
						else
							{
							bathymetry->depth[i] = 0.0;
							bathymetry->acrosstrack[i] = 0.0;
							bathymetry->alongtrack[i] = 0.0;
							bathymetry->pointing_angle[i] = 0.0;
							bathymetry->azimuth_angle[i] = 0.0;
							}
						}

					/* set flag */
					bathymetry->optionaldata = MB_YES;
					bathymetry->header.OffsetToOptionalData 
							= MBSYS_RESON7K_RECORDHEADER_SIZE 
								+ R7KHDRSIZE_7kBathymetricData
								+ bathymetry->number_beams * 9;
					}
				}
			if (istore->read_backscatter == MB_YES)
				{
				backscatter = &(istore->backscatter);
				header = &(backscatter->header);
				time_j[0] = header->s7kTime.Year;
				time_j[1] = header->s7kTime.Day;
				time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
				time_j[3] = (int) header->s7kTime.Seconds;
				time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
				mb_get_itime(verbose, time_j, time_i);
				mb_get_time(verbose, time_i, &time_d);
				if (verbose > 0)
				fprintf(stderr,"R7KRECID_7kBackscatterImageData:   7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d ping:%d samples:%d\n",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],time_i[6],
					header->RecordNumber,backscatter->ping_number,backscatter->number_samples);
				}
			if (istore->read_beam == MB_YES)
				{
				beam = &(istore->beam);
				header = &(beam->header);
				time_j[0] = header->s7kTime.Year;
				time_j[1] = header->s7kTime.Day;
				time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
				time_j[3] = (int) header->s7kTime.Seconds;
				time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
				mb_get_itime(verbose, time_j, time_i);
				mb_get_time(verbose, time_i, &time_d);
				if (verbose > 0)
				fprintf(stderr,"R7KHDRSIZE_7kBeamData: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d ping:%d beams:%d samples:%d\n",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],time_i[6],
					header->RecordNumber,beam->ping_number,beam->number_beams,beam->number_samples);
				}
			if (istore->read_image == MB_YES)
				{
				image = &(istore->image);
				header = &(image->header);
				time_j[0] = header->s7kTime.Year;
				time_j[1] = header->s7kTime.Day;
				time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
				time_j[3] = (int) header->s7kTime.Seconds;
				time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
				mb_get_itime(verbose, time_j, time_i);
				mb_get_time(verbose, time_i, &time_d);
				if (verbose > 0)
				fprintf(stderr,"R7KRECID_7kImageData:              7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d ping:%d width:%d height:%d\n",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],time_i[6],
					header->RecordNumber,image->ping_number,image->width,image->height);
				}

			}
			
	   	/* handle file header data */
		else if (status == MB_SUCCESS && kind == MB_DATA_HEADER) 
			{
			nreadfileheader++;
			
			fileheader = &(istore->fileheader);
			header = &(fileheader->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);
			if (verbose > 0)
				fprintf(stderr,"R7KRECID_7kFileHeader:             7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
				time_i[0],time_i[1],time_i[2],
				time_i[3],time_i[4],time_i[5],time_i[6],
				header->RecordNumber);
			}
			
	   	/* handle bluefin ctd data */
		else if (status == MB_SUCCESS && kind == MB_DATA_SSV) 
			{
			nreadssv++;
			
			bluefin = &(istore->bluefin);
			header = &(bluefin->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);

			if (verbose > 0)
				fprintf(stderr,"R7KRECID_BluefinEnvironmental:     7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
			time_i[0],time_i[1],time_i[2],
			time_i[3],time_i[4],time_i[5],time_i[6],
			header->RecordNumber);
			for (i=0;i<bluefin->number_frames;i++)
				{
				time_j[0] = bluefin->environmental[i].s7kTime.Year;
				time_j[1] = bluefin->environmental[i].s7kTime.Day;
				time_j[2] = 60 * bluefin->environmental[i].s7kTime.Hours + bluefin->environmental[i].s7kTime.Minutes;
				time_j[3] = (int) bluefin->environmental[i].s7kTime.Seconds;
				time_j[4] = (int) (1000000 * (bluefin->environmental[i].s7kTime.Seconds - time_j[3]));
				mb_get_itime(verbose, time_j, time_i);
				if (verbose > 0)
				fprintf(stderr,"                       %2.2d          7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) CTD_time:%f T_time:%f\n",
					i,time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],time_i[6],
					bluefin->environmental[i].ctd_time,
					bluefin->environmental[i].temperature_time);
				}
			}
			
	   	/* handle bluefin nav data */
		else if (status == MB_SUCCESS && kind == MB_DATA_NAV2) 
			{
			nreadnav1++;
			
			bluefin = &(istore->bluefin);
			header = &(bluefin->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);

			if (verbose > 0)
				fprintf(stderr,"R7KRECID_BluefinNav:               7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
			time_i[0],time_i[1],time_i[2],
			time_i[3],time_i[4],time_i[5],time_i[6],
			header->RecordNumber);
			for (i=0;i<bluefin->number_frames;i++)
				{
				time_j[0] = bluefin->nav[i].s7kTime.Year;
				time_j[1] = bluefin->nav[i].s7kTime.Day;
				time_j[2] = 60 * bluefin->nav[i].s7kTime.Hours + bluefin->nav[i].s7kTime.Minutes;
				time_j[3] = (int) bluefin->nav[i].s7kTime.Seconds;
				time_j[4] = (int) (1000000 * (bluefin->nav[i].s7kTime.Seconds - time_j[3]));
				mb_get_itime(verbose, time_j, time_i);
				if (verbose > 0)
				fprintf(stderr,"                       %2.2d          7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) Pos_time:%f\n",
					i,time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],time_i[6],
					bluefin->nav[i].position_time);
				}			
			}
			
	   	/* handle subbottom data */
		else if (status == MB_SUCCESS && kind == MB_DATA_SUBBOTTOM_SUBBOTTOM) 
			{
			nreadsbp++;

			fsdwsb = &(istore->fsdwsb);
			header = &(fsdwsb->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);
			fsdwchannel = &(fsdwsb->channel);
			fsdwsegyheader = &(fsdwsb->segyheader);
			if (verbose > 0)
				fprintf(stderr,"R7KRECID_FSDWsubbottom:            7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) FSDWtime(%4.4d-%3.3d %2.2d:%2.2d:%2.2d.%3.3d) ping:%d sampint:%d samples:%d\n",
				time_i[0],time_i[1],time_i[2],
				time_i[3],time_i[4],time_i[5],time_i[6],
				fsdwsegyheader->year,fsdwsegyheader->day,fsdwsegyheader->hour,fsdwsegyheader->minute,fsdwsegyheader->second,
				fsdwsegyheader->millisecondsToday - 1000 * (int)(0.001 * fsdwsegyheader->millisecondsToday),
				fsdwsb->ping_number,fsdwchannel->sample_interval,fsdwchannel->number_samples);
			}
			
	   	/* handle low frequency sidescan data */
		else if (status == MB_SUCCESS && kind == MB_DATA_SIDESCAN2) 
			{
			nreadsslo++;

			fsdwsslo = &(istore->fsdwsslo);
			header = &(fsdwsslo->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);
			for (i=0;i<fsdwsslo->number_channels;i++)
				{
				fsdwchannel = &(fsdwsslo->channel[i]);
				fsdwssheader = &(fsdwsslo->ssheader[i]);
				if (verbose > 0)
				fprintf(stderr,"R7KRECID_FSDWsidescanLo:           7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) FSDWtime(%4.4d-%3.3d %2.2d:%2.2d:%2.2d.%3.3d) ping:%d channel:%d sampint:%d samples:%d\n",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],time_i[6],
					fsdwssheader->year,fsdwssheader->day,fsdwssheader->hour,fsdwssheader->minute,fsdwssheader->second,
					fsdwssheader->millisecondsToday - 1000 * (int)(0.001 * fsdwssheader->millisecondsToday),
					fsdwsslo->ping_number,fsdwchannel->number,
					fsdwchannel->sample_interval,fsdwchannel->number_samples);
				}
			}
			
	   	/* handle high frequency sidescan data */
		else if (status == MB_SUCCESS && kind == MB_DATA_SIDESCAN3) 
			{
			nreadsshi++;

			fsdwsshi = &(istore->fsdwsshi);
			header = &(fsdwsshi->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);
			for (i=0;i<fsdwsshi->number_channels;i++)
				{
				fsdwchannel = &(fsdwsshi->channel[i]);
				fsdwssheader = &(fsdwsshi->ssheader[i]);
				if (verbose > 0)
				fprintf(stderr,"R7KRECID_FSDWsidescanHi:           7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) FSDWtime(%4.4d-%3.3d %2.2d:%2.2d:%2.2d.%3.3d) ping:%d channel:%d sampint:%d samples:%d\n",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],time_i[6],
					fsdwssheader->year,fsdwssheader->day,fsdwssheader->hour,fsdwssheader->minute,fsdwssheader->second,
					fsdwssheader->millisecondsToday - 1000 * (int)(0.001 * fsdwssheader->millisecondsToday),
					fsdwsshi->ping_number,fsdwchannel->number,
					fsdwchannel->sample_interval,fsdwchannel->number_samples);
				}
			}
			
	   	/* handle unknown data */
		else  if (status == MB_SUCCESS) 
			{
/*fprintf(stderr,"DATA TYPE UNKNOWN: status:%d error:%d kind:%d\n",status,error,kind);*/
			nreadother++;
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

	/*--------------------------------------------
	  write the processed data
	  --------------------------------------------*/

		/* write some data */
		if (error == MB_ERROR_NO_ERROR)
			{
			status = mb_put_all(verbose,ombio_ptr,
					istore_ptr,MB_NO,kind,
					time_i,time_d,
					navlon,navlat,speed,heading,
					obeams_bath,obeams_amp,opixels_ss,
					beamflag,bath,amp,bathacrosstrack,bathalongtrack,
					ss,ssacrosstrack,ssalongtrack,
					comment,&error);
			if (status != MB_SUCCESS)
				{
				mb_error(verbose,error,&message);
				fprintf(stderr,"\nMBIO Error returned from function <mb_put>:\n%s\n",message);
				fprintf(stderr,"\nMultibeam Data Not Written To File <%s>\n",ofile);
				fprintf(stderr,"\nProgram <%s> Terminated\n",
					program_name);
				exit(error);
				}
			}
		}

	/* close the swath file */
	status = mb_close(verbose,&imbio_ptr,&error);
	status = mb_close(verbose,&ombio_ptr,&error);
	
	/* output counts */
	fprintf(stdout, "\nData records read from: %s\n", ifile);
	fprintf(stdout, "     File Header:       %d\n", nreadfileheader);
	fprintf(stdout, "     Multibeam:         %d\n", nreadmultibeam);
	fprintf(stdout, "          Volatile Settings: %d\n", nreadmbvolatilesettings);
	fprintf(stdout, "          Beam Geometry:     %d\n", nreadmbbeamgeometry);
	fprintf(stdout, "          Bathymetry:        %d\n", nreadmbbathymetry);
	fprintf(stdout, "          Backscatter:       %d\n", nreadmbbackscatter);
	fprintf(stdout, "          Beam:              %d\n", nreadmbbeam);
	fprintf(stdout, "          Image:             %d\n", nreadmbimage);
	fprintf(stdout, "     Bluefin CTD:       %d\n", nreadssv);
	fprintf(stdout, "     Bluefin Nav:       %d\n", nreadnav1);
	fprintf(stdout, "     Subbottom:         %d\n", nreadsbp);
	fprintf(stdout, "     Low Sidescan:      %d\n", nreadsslo);
	fprintf(stdout, "     High Sidescan:     %d\n", nreadsshi);
	nreadfileheadertot += nreadfileheader;
	nreadmultibeamtot += nreadmultibeam;
	nreadmbvolatilesettingstot += nreadmbvolatilesettings;
	nreadmbbeamgeometrytot += nreadmbbeamgeometry;
	nreadmbbathymetrytot += nreadmbbathymetry;
	nreadmbbackscattertot += nreadmbbackscatter;
	nreadmbbeamtot += nreadmbbeam;
	nreadmbimagetot += nreadmbimage;
	nreadssvtot += nreadssv;
	nreadnav1tot += nreadnav1;
	nreadsbptot += nreadsbp;
	nreadsslotot += nreadsslo;
	nreadsshitot += nreadsshi;
	nreadothertot += nreadother;

	/* figure out whether and what to read next */
        if (read_datalist == MB_YES)
                {
		if (status = mb_datalist_read(verbose,datalist,
			    ifile,&format,&file_weight,&error)
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
			
	/* output counts */
	fprintf(stdout, "\nTotal data records read from: %s\n", read_file);
	fprintf(stdout, "     File Header:       %d\n", nreadfileheadertot);
	fprintf(stdout, "     Multibeam:         %d\n", nreadmultibeamtot);
	fprintf(stdout, "          Volatile Settings: %d\n", nreadmbvolatilesettingstot);
	fprintf(stdout, "          Beam Geometry:     %d\n", nreadmbbeamgeometrytot);
	fprintf(stdout, "          Bathymetry:        %d\n", nreadmbbathymetrytot);
	fprintf(stdout, "          Backscatter:       %d\n", nreadmbbackscattertot);
	fprintf(stdout, "          Beam:              %d\n", nreadmbbeamtot);
	fprintf(stdout, "          Image:             %d\n", nreadmbimagetot);
	fprintf(stdout, "     Bluefin CTD:       %d\n", nreadssvtot);
	fprintf(stdout, "     Bluefin Nav:       %d\n", nreadnav1tot);
	fprintf(stdout, "     Subbottom:         %d\n", nreadsbptot);
	fprintf(stdout, "     Low Sidescan:      %d\n", nreadsslotot);
	fprintf(stdout, "     High Sidescan:     %d\n", nreadsshitot);
	}
		
	
	/* deallocate navigation arrays */
	status = mb_free(verbose,&nav_time_d,&error);
	status = mb_free(verbose,&nav_quality,&error);
	status = mb_free(verbose,&nav_lon,&error);
	status = mb_free(verbose,&nav_lat,&error);
	status = mb_free(verbose,&nav_speed,&error);
	status = mb_free(verbose,&nav_sonardepth,&error);
	status = mb_free(verbose,&nav_heading,&error);
	status = mb_free(verbose,&nav_roll,&error);
	status = mb_free(verbose,&nav_pitch,&error);
	status = mb_free(verbose,&alt_time_d,&error);
	status = mb_free(verbose,&alt_altitude,&error);
	status = mb_free(verbose,&batht_time_d,&error);
	status = mb_free(verbose,&batht_ping,&error);
	status = mb_free(verbose,&batht_time_d_new,&error);
	status = mb_free(verbose,&edget_time_d,&error);
	status = mb_free(verbose,&edget_ping,&error);

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
