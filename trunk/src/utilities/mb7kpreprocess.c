/*--------------------------------------------------------------------
 *    The MB-system:	mb7kpreprocess.c	10/12/2005
 *    $Id$
 *
 *    Copyright (c) 2005-2011 by
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
 * $Log: mb7kpreprocess.c,v $
 * Revision 5.23  2008/11/16 21:51:18  caress
 * Updating all recent changes, including time lag analysis using mbeditviz and improvements to the mbgrid footprint gridding algorithm.
 *
 * Revision 5.22  2008/10/17 07:52:44  caress
 * Check in on October 17, 2008.
 *
 * Revision 5.21  2008/09/11 20:20:14  caress
 * Checking in updates made during cruise AT15-36.
 *
 * Revision 5.20  2008/08/12 00:05:15  caress
 * Fixed handling of beam flags.
 *
 * Revision 5.19  2008/07/19 07:50:51  caress
 * Removed debug message.
 *
 * Revision 5.18  2008/07/10 18:16:33  caress
 * Proceeding towards 5.1.1beta20.
 *
 * Revision 5.16  2008/05/16 22:44:37  caress
 * Release 5.1.1beta18
 *
 * Revision 5.15  2008/02/12 02:47:42  caress
 * Fixed handling of time lag correction to attitude data. The raw attitude data in the output file are now time lag corrected. Before the corrections were applied to the attitude values in bathymetry calculation, but not otherwise kept in the output.
 *
 * Revision 5.14  2008/01/14 18:24:28  caress
 * Changed handling of beam quality values and beam flags following upgrade to Reson 7k multibeams.
 *
 * Revision 5.13  2007/11/16 17:53:02  caress
 * Fixes applied.
 *
 * Revision 5.12  2007/10/08 16:48:07  caress
 * State of the code on 8 October 2007.
 *
 * Revision 5.11  2007/07/03 17:34:37  caress
 * Added time delay value to bluefin nav records.
 *
 * Revision 5.10  2006/12/15 21:42:49  caress
 * Incremental CVS update.
 *
 * Revision 5.9  2006/11/10 22:36:05  caress
 * Working towards release 5.1.0
 *
 * Revision 5.8  2006/09/11 18:55:53  caress
 * Changes during Western Flyer and Thomas Thompson cruises, August-September
 * 2006.
 *
 * Revision 5.7  2006/07/06 05:30:57  caress
 * Working more towards 5.1.0beta
 *
 * Revision 5.6  2006/06/16 19:30:58  caress
 * Check in after the Santa Monica Basin Mapping AUV Expedition.
 *
 * Revision 5.5  2006/04/26 22:05:26  caress
 * Changes to handle MBARI Mapping AUV data better.
 *
 * Revision 5.4  2006/04/11 19:19:29  caress
 * Various fixes.
 *
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
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

/* MBIO include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_define.h"
#include "../../include/mb_io.h"
#include "../../include/mb_aux.h"
#include "../../include/mbsys_reson7k.h"

#define MB7KPREPROCESS_ALLOC_CHUNK 1000
#define MB7KPREPROCESS_PROCESS		1
#define MB7KPREPROCESS_TIMESTAMPLIST	2
#define	MB7KPREPROCESS_TIMELAG_OFF	0
#define	MB7KPREPROCESS_TIMELAG_CONSTANT	1
#define	MB7KPREPROCESS_TIMELAG_MODEL	2
#define	MB7KPREPROCESS_KLUGE_USEVERTICALDEPTH	1
#define	MB7KPREPROCESS_KLUGE_ZEROALONGTRACKANGLES	2
#define	MB7KPREPROCESS_KLUGE_ZEROATTITUDECORRECTION	3
static char rcs_id[] = "$Id$";

/*--------------------------------------------------------------------*/

int main (int argc, char **argv)
{
	char program_name[] = "mb7kpreprocess";
	char help_message[] =  "mb7kpreprocess reads a Reson 7k format file, interpolates the\nasynchronous navigation and attitude onto the multibeam data, \nand writes a new 7k file with that information correctly embedded\nin the multibeam data. This program can also fix various problems\nwith 7k data.";
	char usage_message[] = "mb7kpreprocess [-A -B -Doffx/offy -Fformat -Ifile -Kklugemode -L  -Ninsfile  -Ooutfile [-Psonardepthfile | -Plagmax/ratemax] -Ttimelag -H -V]";
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
	char	read_file[MB_PATH_MAXLINE];
	void	*datalist;
	int	look_processed = MB_DATALIST_LOOK_UNSET;
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
	char	ifile[MB_PATH_MAXLINE];
	char	ofile[MB_PATH_MAXLINE];
	char	ctdfile[MB_PATH_MAXLINE];
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
	double	roll, rollr;
	double	pitch, pitchr;
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
	
	/* program mode */
	int	mode = MB7KPREPROCESS_PROCESS;
	int	fix_time_stamps = MB_NO;
	int	goodnavattitudeonly = MB_YES;
	
	/* data structure pointers */
	s7k_header 		*header;
	s7kr_reference		*reference;
	s7kr_sensoruncal	*sensoruncal;
	s7kr_sensorcal		*sensorcal;
	s7kr_position 		*position;
	s7kr_customattitude	*customattitude;
	s7kr_tide		*tide;
	s7kr_altitude		*altituderec;
	s7kr_motion		*motion;
	s7kr_depth		*depth;
	s7kr_svp		*svp;
	s7kr_ctd		*ctd;
	s7kr_geodesy		*geodesy;
	s7kr_rollpitchheave 	*rollpitchheave;
	s7kr_heading		*headingrec;
	s7kr_surveyline		*surveyline;
	s7kr_navigation		*navigation;
	s7kr_attitude		*attitude;
	s7kr_fsdwss 		*fsdwsslo;
	s7kr_fsdwss 		*fsdwsshi;
	s7kr_fsdwsb 		*fsdwsb;
	s7k_fsdwchannel 	*fsdwchannel;
	s7k_fsdwssheader 	*fsdwssheader;
	s7k_fsdwsegyheader 	*fsdwsegyheader;
	s7kr_bluefin		*bluefin;
	s7kr_volatilesettings	*volatilesettings;
	s7kr_matchfilter	*matchfilter;
	s7kr_beamgeometry	*beamgeometry;
	s7kr_bathymetry		*bathymetry;
	s7kr_backscatter	*backscatter;
	s7kr_beam		*beam;
	s7kr_verticaldepth	*verticaldepth;
	s7kr_image		*image;
	s7kr_fileheader		*fileheader;
	s7kr_remotecontrolsettings	*remotecontrolsettings;
	
	/* counting variables */
	int	nrec_reference = 0;
	int	nrec_sensoruncal = 0;
	int	nrec_sensorcal = 0;
	int	nrec_position = 0;
	int	nrec_customattitude = 0;
	int	nrec_tide = 0;
	int	nrec_altitude = 0;
	int	nrec_motion = 0;
	int	nrec_depth = 0;
	int	nrec_svp = 0;
	int	nrec_ctd = 0;
	int	nrec_geodesy = 0;
	int	nrec_rollpitchheave = 0;
	int	nrec_heading = 0;
	int	nrec_surveyline = 0;
	int	nrec_navigation = 0;
	int	nrec_attitude = 0;
	int	nrec_fsdwsslo = 0;
	int	nrec_fsdwsshi = 0;
	int	nrec_fsdwsbp = 0;
	int	nrec_bluefinnav = 0;
	int	nrec_bluefinenv = 0;
	int	nrec_multibeam = 0;
	int	nrec_volatilesettings = 0;
	int	nrec_configuration = 0;
	int	nrec_matchfilter = 0;
	int	nrec_beamgeometry = 0;
	int	nrec_calibration = 0;
	int	nrec_bathymetry = 0;
	int	nrec_backscatter = 0;
	int	nrec_beam = 0;
	int	nrec_verticaldepth = 0;
	int	nrec_image = 0;
	int	nrec_installation = 0;
	int	nrec_systemeventmessage = 0;
	int	nrec_fileheader = 0;
	int	nrec_remotecontrolsettings = 0;
	int	nrec_other = 0;
	int	nrec_reference_tot = 0;
	int	nrec_sensoruncal_tot = 0;
	int	nrec_sensorcal_tot = 0;
	int	nrec_position_tot = 0;
	int	nrec_customattitude_tot = 0;
	int	nrec_tide_tot = 0;
	int	nrec_altitude_tot = 0;
	int	nrec_motion_tot = 0;
	int	nrec_depth_tot = 0;
	int	nrec_svp_tot = 0;
	int	nrec_ctd_tot = 0;
	int	nrec_geodesy_tot = 0;
	int	nrec_rollpitchheave_tot = 0;
	int	nrec_heading_tot = 0;
	int	nrec_surveyline_tot = 0;
	int	nrec_navigation_tot = 0;
	int	nrec_attitude_tot = 0;
	int	nrec_fsdwsslo_tot = 0;
	int	nrec_fsdwsshi_tot = 0;
	int	nrec_fsdwsbp_tot = 0;
	int	nrec_bluefinnav_tot = 0;
	int	nrec_bluefinenv_tot = 0;
	int	nrec_multibeam_tot = 0;
	int	nrec_volatilesettings_tot = 0;
	int	nrec_configuration_tot = 0;
	int	nrec_matchfilter_tot = 0;
	int	nrec_beamgeometry_tot = 0;
	int	nrec_calibration_tot = 0;
	int	nrec_bathymetry_tot = 0;
	int	nrec_backscatter_tot = 0;
	int	nrec_beam_tot = 0;
	int	nrec_verticaldepth_tot = 0;
	int	nrec_image_tot = 0;
	int	nrec_installation_tot = 0;
	int	nrec_systemeventmessage_tot = 0;
	int	nrec_fileheader_tot = 0;
	int	nrec_remotecontrolsettings_tot = 0;
	int	nrec_other_tot = 0;
	
	/* last time_d variables - used to check for repeated data */
	double	last_7k_time_d = 0.0;
	double	last_bluefinnav_time_d = 0.0;
	double	last_bluefinenv_time_d = 0.0;
	double	last_fsdwsbp_time_d = 0.0;
	double	last_fsdwsslo_time_d = 0.0;
	double	last_fsdwsshi_time_d = 0.0;
	
	/* merge navigation and attitude from separate Steve Rock data file */
	char	rockfile[MB_PATH_MAXLINE];
	int	rockdata = MB_NO;
	int	nrock = 0;
	double	*rock_time_d = NULL;
	double	*rock_lon = NULL;
	double	*rock_lat = NULL;
	double	*rock_heading = NULL;
	double	*rock_roll = NULL;
	double	*rock_pitch = NULL;
	double	*rock_sonardepth = NULL;
	double	*rock_sonardepthfilter = NULL;
	
	/* merge navigation and attitude from separate WHOI DSL data file */
	char	dslfile[MB_PATH_MAXLINE];
	int	dsldata = MB_NO;
	int	ndsl = 0;
	double	*dsl_time_d = NULL;
	double	*dsl_lon = NULL;
	double	*dsl_lat = NULL;
	double	*dsl_heading = NULL;
	double	*dsl_roll = NULL;
	double	*dsl_pitch = NULL;
	double	*dsl_sonardepth = NULL;
	double	*dsl_sonardepthfilter = NULL;
	
	/* merge navigation and attitude from separate ins data file */
	char	insfile[MB_PATH_MAXLINE];
	int	insdata = MB_NO;
	int	nins = 0;
	int	nins_altitude = 0;
	int	nins_speed = 0;
	double	*ins_time_d = NULL;
	double	*ins_lon = NULL;
	double	*ins_lat = NULL;
	double	*ins_heading = NULL;
	double	*ins_roll = NULL;
	double	*ins_pitch = NULL;
	double	*ins_sonardepth = NULL;
	double	*ins_sonardepthfilter = NULL;
	double	*ins_altitude_time_d = NULL;
	double	*ins_altitude = NULL;
	double	*ins_speed_time_d = NULL;
	double	*ins_speed = NULL;
	int	ins_output_index = -1;
	
	/* merge sonardepth from separate parosci pressure sensor data file */
	char	sonardepthfile[MB_PATH_MAXLINE];
	int	sonardepthdata = MB_NO;
	int	nsonardepth = 0;
	double	*sonardepth_time_d = NULL;
	double	*sonardepth_sonardepth = NULL;
	double	*sonardepth_sonardepthfilter = NULL;
	
	/* asynchronous navigation, heading, attitude data */
	int	ndat_nav = 0;
	int	ndat_nav_alloc = 0;
	double	*dat_nav_time_d = NULL;
	double	*dat_nav_lon = NULL;
	double	*dat_nav_lat = NULL;
	double	*dat_nav_speed = NULL;
	
	int	ndat_sonardepth = 0;
	int	ndat_sonardepth_alloc = 0;
	double	*dat_sonardepth_time_d = NULL;
	double	*dat_sonardepth_sonardepth = NULL;
	double	*dat_sonardepth_sonardepthrate = NULL;
	double	*dat_sonardepth_sonardepthfilter = NULL;
	
	int	ndat_heading = 0;
	int	ndat_heading_alloc = 0;
	double	*dat_heading_time_d = NULL;
	double	*dat_heading_heading = NULL;

	int	ndat_rph = 0;
	int	ndat_rph_alloc = 0;
	double	*dat_rph_time_d = NULL;
	double	*dat_rph_roll = NULL;
	double	*dat_rph_pitch = NULL;
	double	*dat_rph_heave = NULL;

	int	ndat_altitude = 0;
	int	ndat_altitude_alloc = 0;
	double	*dat_altitude_time_d = NULL;
	double	*dat_altitude_altitude = NULL;
	
	/* bathymetry time delay data */
	int	ntimedelay = 0;
	int	ntimedelaycount = 0;
	int	ntimedelay_alloc = 0;
	double	*timedelay_time_d = NULL;
	double	*timedelay_timedelay = NULL;
	
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
	int	timelagmode = MB7KPREPROCESS_TIMELAG_OFF;
	double	timelag = 0.0;
	double	timelagm = 0.0;
	double	timelagconstant = 0.0;
	char	timelagfile[MB_PATH_MAXLINE];
	int	ntimelag = 0;
	double	*timelag_time_d = NULL;
	double	*timelag_model = NULL;
	
	/* range offset parameters */
	int	nrangeoffset = 0;
	int	rangeoffsetstart[3];
	int	rangeoffsetend[3];
	double	rangeoffset[3];
	
	/* depth sensor filtering */
	int	sonardepthfilter = MB_NO;
	double	sonardepthfilterlength = 20.0;
	double	sonardepthfilterdepth = 20.0;
	
	/* depth sensor offset (+ makes vehicle deeper) */
	double	sonardepthoffset = 0.0;
	
	/* depth sensor lever arm parameter */
	double	depthsensoroffx = 0.0;
	double	depthsensoroffz = 0.0;
	
	/* depth sensor time lag parameters */
	int	sonardepthlagfix = MB_NO;
	double	sonardepthlagmax = 3.0 /* sec */;
	double	sonardepthratemax = 0.64; /* m/sec */
	double	sonardepthlag = 0.0;
	double	sonardepthrate;
	
	/* output asynchronous and synchronous time series ancilliary files */
	char	athfile[MB_PATH_MAXLINE];
	char	atsfile[MB_PATH_MAXLINE];
	char	atafile[MB_PATH_MAXLINE];
	char	stafile[MB_PATH_MAXLINE];
	FILE	*athfp;
	FILE	*atsfp;
	FILE	*atafp;
	FILE	*stafp;
	
	/* kluge modes */
	int	klugemode;
	int	kluge_useverticaldepth = MB_NO; /* kluge 1 */
	int	kluge_zeroalongtrackangles = MB_NO; /* kluge 2 */
	int	kluge_zeroattitudecorrection = MB_NO; /* kluge 3 */
	
	/* MBARI data flag */
	int	MBARIdata = MB_NO;
	
	int	interp_status;
	double	soundspeed;
	double	alpha, beta, theta, phi;
	double	rr, xx, zz;
	double	mtodeglon, mtodeglat;
	double	dx, dy, dist, dt;
	int	j1, j2;
	
	FILE	*tfp = NULL;
	struct stat file_status;
	int	fstat;
	char	buffer[MB_PATH_MAXLINE];
	char	*result;
	int	read_data;
	int	testformat;
	char	fileroot[MB_PATH_MAXLINE];
	int	found;
	int	sslo_lastread;
	double	sslo_last_time_d;
	int	sslo_last_ping;
	int	foundstart, foundend;
	int	start, end;
	int	nscan, startdata;
	int	ins_time_d_index = -1;
	int	ins_lon_index = -1;
	int	ins_lat_index = -1;
	int	ins_roll_index = -1;
	int	ins_pitch_index = -1;
	int	ins_heading_index = -1;
	int	ins_sonardepth_index = -1;
	int	ins_altitude_index = -1;
	int	ins_speed_index = -1;
	int	ins_velocityx_index = -1;
	int	ins_velocityy_index = -1;
	int	ins_velocityz_index = -1;
	int	ins_len;
	int	sonardepth_time_d_index;
	int	sonardepth_sonardepth_index;
	int	sonardepth_len;
	double	sonardepth_filterweight;
	double	dtol, weight;
	double	factor;
	double	velocityx, velocityy;
	int	type_save, kind_save;
	char	type[MB_PATH_MAXLINE], value[MB_PATH_MAXLINE];
	int	year, month, day, hour, minute;
	double	second, id;
	char	sensor[24];
	int	i, j;

	/* get current default values */
	status = mb_defaults(verbose,&format,&pings,&lonflip,bounds,
		btime_i,etime_i,&speedmin,&timegap);

	/* set default input to datalist.mb-1 */
	strcpy (read_file, "datalist.mb-1");

	/* process argument list */
	while ((c = getopt(argc, argv, "AaBbD:d:F:f:I:i:K:k:LlM:m:N:n:O:o:P:p:R:r:T:t:W:w:VvHh")) != -1)
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
			goodnavattitudeonly = MB_NO;
			flag++;
			break;
		case 'B':
		case 'b':
			fix_time_stamps = MB_YES;
			break;
		case 'D':
		case 'd':
			nscan = sscanf (optarg,"%lf/%lf/%lf", &depthsensoroffx, &depthsensoroffz, &sonardepthoffset);
			if (nscan < 3)
				sonardepthoffset = 0.0;
			if (nscan < 2)
				depthsensoroffz = 0.0;
			if (nscan < 1)
				depthsensoroffx = 0.0;
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
		case 'K':
		case 'k':
			sscanf (optarg,"%d", &klugemode);
			if (klugemode == MB7KPREPROCESS_KLUGE_USEVERTICALDEPTH)
				{
				kluge_useverticaldepth = MB_YES;
				}
			if (klugemode == MB7KPREPROCESS_KLUGE_ZEROALONGTRACKANGLES)
				{
				kluge_zeroalongtrackangles = MB_YES;
				}
			if (klugemode == MB7KPREPROCESS_KLUGE_ZEROATTITUDECORRECTION)
				{
				kluge_zeroattitudecorrection = MB_YES;
				}
			flag++;
			break;
		case 'L':
		case 'l':
			mode = MB7KPREPROCESS_TIMESTAMPLIST;
			flag++;
			break;
		case 'M':
		case 'm':
			sscanf (optarg,"%s", rockfile);
			rockdata  = MB_YES;
			flag++;
			break;
		case 'N':
		case 'n':
			sscanf (optarg,"%s", insfile);
			insdata  = MB_YES;
			flag++;
			break;
		case 'O':
		case 'o':
			sscanf (optarg,"%s", ofile);
			ofile_set  = MB_YES;
			flag++;
			break;
		case 'P':
		case 'p':
			sscanf (optarg,"%s", buffer);
			if ((fstat = stat(buffer, &file_status)) == 0
				&& (file_status.st_mode & S_IFMT) != S_IFDIR)
					{
					sonardepthdata  = MB_YES;
					strcpy(sonardepthfile,buffer);
					}
			else if (optarg[0] == 'F' || optarg[0] == 'f')
				{
				nscan = sscanf (&(optarg[1]),"%lf/%lf", &sonardepthfilterlength, &sonardepthfilterdepth);
				if (nscan == 1)
					sonardepthfilterdepth = 20.0;
				if (nscan >= 1) 
					sonardepthfilter = MB_YES;
				else
					sonardepthfilter = MB_NO;
				}
			else
				{
				nscan = sscanf (optarg,"%lf/%lf", &sonardepthlagmax, &sonardepthratemax);
				if (nscan > 0)
					sonardepthlagfix = MB_YES;
				if (nscan == 1)
					{
					sonardepthratemax = 0.0;
					sonardepthlag = sonardepthlagmax;
					}
				}

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
			sscanf (optarg,"%s", timelagfile);
			if ((fstat = stat(timelagfile, &file_status)) == 0
				&& (file_status.st_mode & S_IFMT) != S_IFDIR)
					{
					timelagmode = MB7KPREPROCESS_TIMELAG_MODEL;
					}
			else
				{
				sscanf (optarg,"%lf", &timelagconstant);
				timelagmode = MB7KPREPROCESS_TIMELAG_CONSTANT;
				}
			flag++;
			break;
		case 'W':
		case 'w':
			sscanf (optarg,"%s", dslfile);
			dsldata  = MB_YES;
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
		fprintf(stderr,"dbg2       rockfile:            %s\n",rockfile);
		fprintf(stderr,"dbg2       rockdata:            %d\n",rockdata);
		fprintf(stderr,"dbg2       dslfile:             %s\n",dslfile);
		fprintf(stderr,"dbg2       dsldata:             %d\n",dsldata);
		fprintf(stderr,"dbg2       insfile:             %s\n",insfile);
		fprintf(stderr,"dbg2       insdata:             %d\n",insdata);
		fprintf(stderr,"dbg2       mode:                %d\n",mode);
		fprintf(stderr,"dbg2       fix_time_stamps:     %d\n",fix_time_stamps);
		fprintf(stderr,"dbg2       goodnavattitudeonly: %d\n",goodnavattitudeonly);
		fprintf(stderr,"dbg2       timelagmode:         %d\n",timelagmode);
		fprintf(stderr,"dbg2       kluge_useverticaldepth: %d\n",kluge_useverticaldepth);
		if (timelagmode == MB7KPREPROCESS_TIMELAG_MODEL)
			{
			fprintf(stderr,"dbg2       timelagfile:         %s\n",timelagfile);
			fprintf(stderr,"dbg2       ntimelag:            %d\n",ntimelag);
			for (i=0;i<nrangeoffset;i++)
				fprintf(stderr,"dbg2       timelag[%d]:         %f %f\n",
					i, timelag_time_d[i], timelag_model[i]);
			}
		else
			{
			fprintf(stderr,"dbg2       timelag:             %f\n",timelag);
			}
		fprintf(stderr,"dbg2       timelag:                %f\n",timelag);
		fprintf(stderr,"dbg2       sonardepthfilter:       %d\n",sonardepthfilter);
		fprintf(stderr,"dbg2       sonardepthfilterlength: %f\n",sonardepthfilterlength);
		fprintf(stderr,"dbg2       sonardepthfilterdepth:  %f\n",sonardepthfilterdepth);
		fprintf(stderr,"dbg2       sonardepthfile:         %s\n",sonardepthfile);
		fprintf(stderr,"dbg2       sonardepthdata:         %d\n",sonardepthdata);
		fprintf(stderr,"dbg2       sonardepthlagfix:       %d\n",sonardepthlagfix);
		fprintf(stderr,"dbg2       sonardepthlagmax:       %f\n",sonardepthlagmax);
		fprintf(stderr,"dbg2       sonardepthratemax:      %f\n",sonardepthratemax);
		fprintf(stderr,"dbg2       sonardepthoffset:       %f\n",sonardepthoffset);
		fprintf(stderr,"dbg2       depthsensoroffx:        %f\n",depthsensoroffx);
		fprintf(stderr,"dbg2       depthsensoroffz:        %f\n",depthsensoroffz);
		for (i=0;i<nrangeoffset;i++)
			fprintf(stderr,"dbg2       rangeoffset[%d]:         %d %d %f\n",
				i,rangeoffsetstart[i], rangeoffsetend[i], rangeoffset[i]);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}
		
	/* read navigation and attitude data from AUV log file if specified */
	if (insdata == MB_YES)
		{
		/* count the data points in the auv log file */
		if ((tfp = fopen(insfile, "r")) == NULL) 
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to open ins data file <%s> for reading\n",insfile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}
			
		/* read the ascii header to determine how to parse the binary data */
		ins_len = 0;
		while ((result = fgets(buffer,MB_PATH_MAXLINE,tfp)) == buffer
			&& strncmp(buffer, "# begin",7) != 0)
			{
			nscan = sscanf(buffer, "# %s %s", type, value);
			if (nscan == 2)
				{
				if (strcmp(value, "time") == 0)
					ins_time_d_index = ins_len;
				if (strcmp(value, "mLonK") == 0)
					ins_lon_index = ins_len;
				if (strcmp(value, "mLatK") == 0)
					ins_lat_index = ins_len;
				if (strcmp(value, "longitude") == 0)
					ins_lon_index = ins_len;
				if (strcmp(value, "latitude") == 0)
					ins_lat_index = ins_len;
				if (strcmp(value, "mPhi") == 0)
					ins_roll_index = ins_len;
				if (strcmp(value, "mTheta") == 0)
					ins_pitch_index = ins_len;
				if (strcmp(value, "mPsi") == 0)
					ins_heading_index = ins_len;
				if (strcmp(value, "mDepth") == 0)
					ins_sonardepth_index = ins_len;
				if (strcmp(value, "mDepthK") == 0)
					ins_sonardepth_index = ins_len;
				if (strcmp(value, "mRollK") == 0)
					ins_roll_index = ins_len;
				if (strcmp(value, "mPitchK") == 0)
					ins_pitch_index = ins_len;
				if (strcmp(value, "mHeadK") == 0)
					ins_heading_index = ins_len;
				if (strcmp(value, "mAltitude") == 0)
					ins_altitude_index = ins_len;
				if (strcmp(value, "mWaterSpeed") == 0)
					ins_speed_index = ins_len;
				if (strcmp(value, "mVbodyxK") == 0)
					ins_velocityx_index = ins_len;
				if (strcmp(value, "mVbodyyK") == 0)
					ins_velocityy_index = ins_len;
				if (strcmp(value, "mVbodyzK") == 0)
					ins_velocityz_index = ins_len;

				if (strcmp(type, "double") == 0)
					ins_len += 8;
				else if (strcmp(type, "integer") == 0)
					ins_len += 4;
				else if (strcmp(type, "timeTag") == 0)
					ins_len += 8;
				}
			}
			
		/* count the binary data records described by the header 
			then rewind the file to the start of the binary data */
		startdata = ftell(tfp);
		nins = 0;
		while (fread(buffer, ins_len, 1, tfp) == 1)
			{
			nins++;
			}
		fseek(tfp, startdata, 0);

		/* allocate arrays for ins data */
		if (nins > 0)
		    {
		    status = mb_mallocd(verbose, __FILE__, __LINE__, nins * sizeof(double), (void **)&ins_time_d,&error);
		    if (error == MB_ERROR_NO_ERROR)
		    	status = mb_mallocd(verbose, __FILE__, __LINE__, nins * sizeof(double), (void **)&ins_lon,&error);
		    if (error == MB_ERROR_NO_ERROR)
		    	status = mb_mallocd(verbose, __FILE__, __LINE__, nins * sizeof(double), (void **)&ins_lat,&error);
		    if (error == MB_ERROR_NO_ERROR)
		    	status = mb_mallocd(verbose, __FILE__, __LINE__, nins * sizeof(double), (void **)&ins_heading,&error);
		    if (error == MB_ERROR_NO_ERROR)
		    	status = mb_mallocd(verbose, __FILE__, __LINE__, nins * sizeof(double), (void **)&ins_roll,&error);
		    if (error == MB_ERROR_NO_ERROR)
		    	status = mb_mallocd(verbose, __FILE__, __LINE__, nins * sizeof(double), (void **)&ins_pitch,&error);
		    if (error == MB_ERROR_NO_ERROR)
		    	status = mb_mallocd(verbose, __FILE__, __LINE__, nins * sizeof(double), (void **)&ins_sonardepth,&error);
		    if (error == MB_ERROR_NO_ERROR)
		    	status = mb_mallocd(verbose, __FILE__, __LINE__, nins * sizeof(double), (void **)&ins_sonardepthfilter,&error);
		    if (error == MB_ERROR_NO_ERROR)
		    	status = mb_mallocd(verbose, __FILE__, __LINE__, nins * sizeof(double), (void **)&ins_altitude_time_d,&error);
		    if (error == MB_ERROR_NO_ERROR)
		    	status = mb_mallocd(verbose, __FILE__, __LINE__, nins * sizeof(double), (void **)&ins_altitude,&error);
		    if (error == MB_ERROR_NO_ERROR)
		    	status = mb_mallocd(verbose, __FILE__, __LINE__, nins * sizeof(double), (void **)&ins_speed_time_d,&error);
		    if (error == MB_ERROR_NO_ERROR)
		    	status = mb_mallocd(verbose, __FILE__, __LINE__, nins * sizeof(double), (void **)&ins_speed,&error);
		    if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nMBIO Error allocating ins data arrays:\n%s\n",message);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}		    
		    }

		/* if no ins data then quit */
		else
		    {
		    error = MB_ERROR_BAD_DATA;
		    fprintf(stderr,"\nUnable to read data from MBARI AUV navigation file <%s>\n",insfile);
		    fprintf(stderr,"\nProgram <%s> Terminated\n",
			    program_name);
		    exit(error);
		    }		    

		/* read the data points in the auv log file */
		nins = 0;
		nins_altitude = 0;
		nins_speed = 0;
		while (fread(buffer, ins_len, 1, tfp) == 1)
			{
			if (ins_time_d_index >= 0)
				mb_get_binary_double(MB_YES, &buffer[ins_time_d_index], &(ins_time_d[nins]));
				
			if (ins_lon_index >= 0)
				mb_get_binary_double(MB_YES, &buffer[ins_lon_index], &(ins_lon[nins])); 
			ins_lon[nins] *= RTD;
			
			if (ins_lat_index >= 0)
				mb_get_binary_double(MB_YES, &buffer[ins_lat_index], &(ins_lat[nins])); 
			ins_lat[nins] *= RTD;
			
			if (ins_roll_index >= 0)
				mb_get_binary_double(MB_YES, &buffer[ins_roll_index], &(ins_roll[nins])); 
			ins_roll[nins] *= RTD;
			
			if (ins_pitch_index >= 0)
				mb_get_binary_double(MB_YES, &buffer[ins_pitch_index], &(ins_pitch[nins])); 
			ins_pitch[nins] *= RTD;
			
			if (ins_heading_index >= 0)
				mb_get_binary_double(MB_YES, &buffer[ins_heading_index], &(ins_heading[nins])); 
			ins_heading[nins] *= RTD;

			if (ins_sonardepth_index >= 0)
				mb_get_binary_double(MB_YES, &buffer[ins_sonardepth_index], &(ins_sonardepth[nins]));
			ins_sonardepth[nins] += sonardepthoffset;
			
			if (ins_altitude_index >= 0)
				mb_get_binary_double(MB_YES, &buffer[ins_altitude_index], &(ins_altitude[nins_altitude]));
			ins_altitude_time_d[nins_altitude] = ins_time_d[nins];
			
			if (ins_speed_index >= 0)
				mb_get_binary_double(MB_YES, &buffer[ins_speed_index], &(ins_speed[nins_speed]));
			ins_speed_time_d[nins_speed] = ins_time_d[nins];
			
			if (ins_velocityx_index >= 0 && ins_velocityy_index >= 0)
				{
				mb_get_binary_double(MB_YES, &buffer[ins_velocityx_index], &velocityx);
				mb_get_binary_double(MB_YES, &buffer[ins_velocityy_index], &velocityy);
				ins_speed[nins_speed] = sqrt(velocityx * velocityx + velocityy * velocityy);
				ins_speed_time_d[nins_speed] = ins_time_d[nins];
				}
/*fprintf(stderr,"INS DATA: %f %f %f %f %f %f %f %f %f\n", 
ins_time_d[nins],
ins_lon[nins],
ins_lat[nins],
ins_roll[nins],
ins_pitch[nins],
ins_heading[nins],
ins_sonardepth[nins],
ins_altitude[nins],
ins_speed[nins]);*/
			nins++;
			if (ins_altitude[nins_altitude] < 1000.0) 
				nins_altitude++;
			if (ins_speed[nins_speed] > 0.0) 
				nins_speed++;
			}
		fclose(tfp);
		}
		
	/* read navigation and attitude data from rock file if specified */
	if (rockdata == MB_YES)
		{
		/* count the data points in the rock file */
		if ((tfp = fopen(rockfile, "r")) == NULL) 
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to open rock data file <%s> for reading\n",rockfile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}

		/* count the data records */
		nrock = 0;
		while ((result = fgets(buffer,MB_PATH_MAXLINE,tfp)) == buffer)
			if (buffer[0] != '#')
			    nrock++;
		rewind(tfp);

		/* allocate arrays for rock data */
		if (nrock > 0)
		    {
		    status = mb_mallocd(verbose, __FILE__, __LINE__, nrock * sizeof(double), (void **)&rock_time_d,&error);
		    if (error == MB_ERROR_NO_ERROR)
		    	status = mb_mallocd(verbose, __FILE__, __LINE__, nrock * sizeof(double), (void **)&rock_lon,&error);
		    if (error == MB_ERROR_NO_ERROR)
		    	status = mb_mallocd(verbose, __FILE__, __LINE__, nrock * sizeof(double), (void **)&rock_lat,&error);
		    if (error == MB_ERROR_NO_ERROR)
		    	status = mb_mallocd(verbose, __FILE__, __LINE__, nrock * sizeof(double), (void **)&rock_sonardepth,&error);
		    if (error == MB_ERROR_NO_ERROR)
		    	status = mb_mallocd(verbose, __FILE__, __LINE__, nrock * sizeof(double), (void **)&rock_sonardepthfilter,&error);
		    if (error == MB_ERROR_NO_ERROR)
		    	status = mb_mallocd(verbose, __FILE__, __LINE__, nrock * sizeof(double), (void **)&rock_heading,&error);
		    if (error == MB_ERROR_NO_ERROR)
		    	status = mb_mallocd(verbose, __FILE__, __LINE__, nrock * sizeof(double), (void **)&rock_roll,&error);
		    if (error == MB_ERROR_NO_ERROR)
		    	status = mb_mallocd(verbose, __FILE__, __LINE__, nrock * sizeof(double), (void **)&rock_pitch,&error);
		    if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nMBIO Error allocating rock data arrays:\n%s\n",message);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}		    
		    }

		/* if no rock data then quit */
		else
		    {
		    error = MB_ERROR_BAD_DATA;
		    fprintf(stderr,"\nUnable to read data from rock file <%s>\n",rockfile);
		    fprintf(stderr,"\nProgram <%s> Terminated\n",
			    program_name);
		    exit(error);
		    }		    

		/* read the data points in the rock file */
		nrock = 0;
		while ((result = fgets(buffer,MB_PATH_MAXLINE,tfp)) == buffer)
			{
			if (buffer[0] != '#')
			    if (sscanf(buffer,"%lf %lf %lf %lf %lf %lf %lf",
					&rock_time_d[nrock], &rock_lon[nrock], &rock_lat[nrock], &rock_sonardepth[nrock],
					&rock_heading[nrock], &rock_roll[nrock], &rock_pitch[nrock]) == 7)
			    	{
/*fprintf(stderr,"ROCK DATA: %f %f %f %f %f %f\n", 
rock_time_d[nrock],
rock_lon[nrock],
rock_lat[nrock],
rock_sonardepth[nrock],
rock_heading[nrock],
rock_roll[nrock],
rock_pitch[nrock],
rock_heading[nrock]);*/
			    	nrock++;
				}
			}
		fclose(tfp);
		}
		
	/* read navigation and attitude data from dsl file if specified */
	if (dsldata == MB_YES)
		{
		/* count the data points in the dsl file */
		if ((tfp = fopen(dslfile, "r")) == NULL) 
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to open dsl data file <%s> for reading\n",dslfile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}

		/* count the data records */
		ndsl = 0;
		while ((result = fgets(buffer,MB_PATH_MAXLINE,tfp)) == buffer)
			if (buffer[0] != '#')
			    ndsl++;
		rewind(tfp);

		/* allocate arrays for dsl data */
		if (ndsl > 0)
		    {
		    status = mb_mallocd(verbose, __FILE__, __LINE__, ndsl * sizeof(double), (void **)&dsl_time_d,&error);
		    if (error == MB_ERROR_NO_ERROR)
		    	status = mb_mallocd(verbose, __FILE__, __LINE__, ndsl * sizeof(double), (void **)&dsl_lon,&error);
		    if (error == MB_ERROR_NO_ERROR)
		    	status = mb_mallocd(verbose, __FILE__, __LINE__, ndsl * sizeof(double), (void **)&dsl_lat,&error);
		    if (error == MB_ERROR_NO_ERROR)
		    	status = mb_mallocd(verbose, __FILE__, __LINE__, ndsl * sizeof(double), (void **)&dsl_sonardepth,&error);
		    if (error == MB_ERROR_NO_ERROR)
		    	status = mb_mallocd(verbose, __FILE__, __LINE__, ndsl * sizeof(double), (void **)&dsl_sonardepthfilter,&error);
		    if (error == MB_ERROR_NO_ERROR)
		    	status = mb_mallocd(verbose, __FILE__, __LINE__, ndsl * sizeof(double), (void **)&dsl_heading,&error);
		    if (error == MB_ERROR_NO_ERROR)
		    	status = mb_mallocd(verbose, __FILE__, __LINE__, ndsl * sizeof(double), (void **)&dsl_roll,&error);
		    if (error == MB_ERROR_NO_ERROR)
		    	status = mb_mallocd(verbose, __FILE__, __LINE__, ndsl * sizeof(double), (void **)&dsl_pitch,&error);
		    if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nMBIO Error allocating dsl data arrays:\n%s\n",message);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}		    
		    }

		/* if no dsl data then quit */
		else
		    {
		    error = MB_ERROR_BAD_DATA;
		    fprintf(stderr,"\nUnable to read data from dsl file <%s>\n",dslfile);
		    fprintf(stderr,"\nProgram <%s> Terminated\n",
			    program_name);
		    exit(error);
		    }		    

		/* read the data points in the dsl file */
		ndsl = 0;
		while ((result = fgets(buffer,MB_PATH_MAXLINE,tfp)) == buffer)
			{
			if (buffer[0] != '#')
			    {
			    nscan = sscanf(buffer,"PPL %d/%d/%d %d:%d:%lf %s %lf %lf %lf %lf %lf %lf %lf",
			    		&year, &month, &day, &hour, &minute, &second,
					sensor, &dsl_lat[ndsl], &dsl_lon[ndsl], &dsl_sonardepth[ndsl],
					&dsl_heading[ndsl], &dsl_pitch[ndsl], &dsl_roll[ndsl], &id);
/*fprintf(stderr,"nscan:%d year:%d month:%d day:%d hour:%d minute:%d second:%f sensor:%s %f %f %f %f %f %f %f\n",
nscan,year,month,day,hour,minute,second,sensor,dsl_lat[ndsl], dsl_lon[ndsl], dsl_sonardepth[ndsl],
dsl_heading[ndsl], dsl_pitch[ndsl], dsl_roll[ndsl], id);*/
			    if (nscan == 14)
			    	{
				time_i[0] = year;
				time_i[1] = month;
				time_i[2] = day;
				time_i[3] = hour;
				time_i[4] = minute;
				time_i[5] = (int)second;
				time_i[6] = (int)((second - time_i[5]) * 1000000);
				mb_get_time(verbose, time_i, &dsl_time_d[ndsl]);
/*fprintf(stderr,"dsl DATA: %f %f %f %f %f %f\n", 
dsl_time_d[ndsl],
dsl_lon[ndsl],
dsl_lat[ndsl],
dsl_sonardepth[ndsl],
dsl_heading[ndsl],
dsl_roll[ndsl],
dsl_pitch[ndsl],
dsl_heading[ndsl]);*/
			    	ndsl++;
				}
			    }
			}
		fclose(tfp);
		}

	/* read sonardepth data from AUV log file if specified */
	if (sonardepthdata == MB_YES)
		{
		/* count the data points in the auv log file */
		if ((tfp = fopen(sonardepthfile, "r")) == NULL) 
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to open sonardepth data file <%s> for reading\n",sonardepthfile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}
			
		/* read the ascii header to determine how to parse the binary data */
		sonardepth_len = 0;
		while ((result = fgets(buffer,MB_PATH_MAXLINE,tfp)) == buffer
			&& strncmp(buffer, "# begin",7) != 0)
			{
			nscan = sscanf(buffer, "# %s %s", type, value);
			if (nscan == 2)
				{
				if (strcmp(value, "time") == 0)
					sonardepth_time_d_index = sonardepth_len;
				if (strcmp(value, "depth") == 0)
					sonardepth_sonardepth_index = sonardepth_len;

				if (strcmp(type, "double") == 0)
					sonardepth_len += 8;
				else if (strcmp(type, "integer") == 0)
					sonardepth_len += 4;
				else if (strcmp(type, "timeTag") == 0)
					sonardepth_len += 8;
				}
			}
			
		/* count the binary data records described by the header 
			then rewind the file to the start of the binary data */
		startdata = ftell(tfp);
		nsonardepth = 0;
		while (fread(buffer, sonardepth_len, 1, tfp) == 1)
			{
			nsonardepth++;
			}
		fseek(tfp, startdata, 0);

		/* allocate arrays for sonardepth data */
		if (nsonardepth > 0)
		    {
		    status = mb_mallocd(verbose, __FILE__, __LINE__, nsonardepth * sizeof(double), (void **)&sonardepth_time_d,&error);
		    if (error == MB_ERROR_NO_ERROR)
		    	status = mb_mallocd(verbose, __FILE__, __LINE__, nsonardepth * sizeof(double), (void **)&sonardepth_sonardepth,&error);
		    if (error == MB_ERROR_NO_ERROR)
		    	status = mb_mallocd(verbose, __FILE__, __LINE__, nsonardepth * sizeof(double), (void **)&sonardepth_sonardepthfilter,&error);
		    if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nMBIO Error allocating sonardepth data arrays:\n%s\n",message);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}		    
		    }

		/* if no sonardepth data then quit */
		else
		    {
		    error = MB_ERROR_BAD_DATA;
		    fprintf(stderr,"\nUnable to read data from MBARI AUV sonardepth file <%s>\n",sonardepthfile);
		    fprintf(stderr,"\nProgram <%s> Terminated\n",
			    program_name);
		    exit(error);
		    }		    

		/* read the data points in the auv log file */
		nsonardepth = 0;
		while (fread(buffer, sonardepth_len, 1, tfp) == 1)
			{
			mb_get_binary_double(MB_YES, &buffer[sonardepth_time_d_index], &(sonardepth_time_d[nsonardepth]));	
			mb_get_binary_double(MB_YES, &buffer[sonardepth_sonardepth_index], &(sonardepth_sonardepth[nsonardepth])); 
/*fprintf(stderr,"SONARDEPTH DATA: %f %f\n", 
sonardepth_time_d[nsonardepth],
sonardepth_sonardepth[nsonardepth]);*/
			sonardepth_sonardepth[nsonardepth] += sonardepthoffset;
			nsonardepth++;
			}
		fclose(tfp);
		}
		
	/* get time lag model if specified */
	if (timelagmode == MB7KPREPROCESS_TIMELAG_MODEL)
		{
		/* count the data points in the timelag file */
		ntimelag = 0;
		if ((tfp = fopen(timelagfile, "r")) == NULL) 
			{
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr,"\nUnable to open time lag model File <%s> for reading\n",timelagfile);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}
		while ((result = fgets(buffer,MB_PATH_MAXLINE,tfp)) == buffer)
			if (buffer[0] != '#')
			    ntimelag++;
		rewind(tfp);

		/* allocate arrays for time lag */
		if (ntimelag > 0)
		    {
		    status = mb_mallocd(verbose, __FILE__, __LINE__,ntimelag * sizeof(double), (void **)&timelag_time_d,&error);
		    if (error == MB_ERROR_NO_ERROR)
		    	status = mb_mallocd(verbose, __FILE__, __LINE__,ntimelag * sizeof(double), (void **)&timelag_model,&error);
		    if (error != MB_ERROR_NO_ERROR)
			{
			mb_error(verbose,error,&message);
			fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
			fprintf(stderr,"\nProgram <%s> Terminated\n",
				program_name);
			exit(error);
			}		    
		    }

		/* if no time lag data then quit */
		else
		    {
		    error = MB_ERROR_BAD_DATA;
		    fprintf(stderr,"\nUnable to read data from time lag model file <%s>\n",timelagfile);
		    fprintf(stderr,"\nProgram <%s> Terminated\n",
			    program_name);
		    exit(error);
		    }		    

		/* read the data points in the timelag file */
		ntimelag = 0;
		while ((result = fgets(buffer,MB_PATH_MAXLINE,tfp)) == buffer)
		    {
		    if (buffer[0] != '#')
			{
			/* read the time and time lag pair */
			if (sscanf(buffer,"%lf %lf",&timelag_time_d[ntimelag],&timelag_model[ntimelag]) == 2)
			    ntimelag++;
			}
		    }
		fclose(tfp);
		}
		
	/* null tfp - allows detection of whether time delay file was opened, which only happens for MBARI AUV
		data with navigation and attitude in "bluefin" records */
	tfp = NULL;

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
			    ifile,&format,&file_weight,&error))
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

	/* reset file record counters */
	nrec_reference = 0;
	nrec_sensoruncal = 0;
	nrec_sensorcal = 0;
	nrec_position = 0;
	nrec_customattitude = 0;
	nrec_tide = 0;
	nrec_altitude = 0;
	nrec_motion = 0;
	nrec_depth = 0;
	nrec_svp = 0;
	nrec_ctd = 0;
	nrec_geodesy = 0;
	nrec_rollpitchheave = 0;
	nrec_heading = 0;
	nrec_surveyline = 0;
	nrec_navigation = 0;
	nrec_attitude = 0;
	nrec_fsdwsslo = 0;
	nrec_fsdwsshi = 0;
	nrec_fsdwsbp = 0;
	nrec_bluefinnav = 0;
	nrec_bluefinenv = 0;
	nrec_multibeam = 0;
	nrec_volatilesettings = 0;
	nrec_configuration = 0;
	nrec_matchfilter = 0;
	nrec_beamgeometry = 0;
	nrec_calibration = 0;
	nrec_bathymetry = 0;
	nrec_backscatter = 0;
	nrec_beam = 0;
	nrec_verticaldepth = 0;
	nrec_image = 0;
	nrec_installation = 0;
	nrec_systemeventmessage = 0;
	nrec_fileheader = 0;
	nrec_remotecontrolsettings = 0;
	nrec_other = 0;

	/* read and print data */
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
			nrec_multibeam++;
			
			bathymetry = &(istore->bathymetry);
			if (istore->read_volatilesettings == MB_YES)
				nrec_volatilesettings++;
			if (istore->read_matchfilter == MB_YES)
				nrec_matchfilter++;
			if (istore->read_beamgeometry == MB_YES)
				nrec_beamgeometry++;
			if (istore->read_remotecontrolsettings == MB_YES)
				nrec_remotecontrolsettings++;
			if (istore->read_bathymetry == MB_YES)
				nrec_bathymetry++;
			if (istore->read_backscatter == MB_YES)
				nrec_backscatter++;
			if (istore->read_beam == MB_YES)
				nrec_beam++;
			if (istore->read_verticaldepth == MB_YES)
				nrec_verticaldepth++;
			if (istore->read_image == MB_YES)
				nrec_image++;
				
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
			if (istore->read_matchfilter == MB_YES)
				{
				matchfilter = &(istore->matchfilter);
				header = &(matchfilter->header);
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
			if (istore->read_remotecontrolsettings == MB_YES)
				{
				remotecontrolsettings = &(istore->remotecontrolsettings);
				header = &(remotecontrolsettings->header);
				time_j[0] = header->s7kTime.Year;
				time_j[1] = header->s7kTime.Day;
				time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
				time_j[3] = (int) header->s7kTime.Seconds;
				time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
				mb_get_itime(verbose, time_j, time_i);
				mb_get_time(verbose, time_i, &time_d);
				if (verbose > 0)
				fprintf(stderr,"R7KRECID_7kremotecontrolsettings:  7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],time_i[6],
					header->RecordNumber);
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
					status = mb_reallocd(verbose,__FILE__,__LINE__,nbatht_alloc*sizeof(double),(void **)&batht_time_d,&error);
					status = mb_reallocd(verbose,__FILE__,__LINE__,nbatht_alloc*sizeof(int),(void **)&batht_ping,&error);
					status = mb_reallocd(verbose,__FILE__,__LINE__,nbatht_alloc*sizeof(double),(void **)&batht_time_d_new,&error);
					status = mb_reallocd(verbose,__FILE__,__LINE__,nbatht_alloc*sizeof(double),(void **)&batht_time_offset,&error);
					status = mb_reallocd(verbose,__FILE__,__LINE__,nbatht_alloc*sizeof(int),(void **)&batht_ping_offset,&error);
					status = mb_reallocd(verbose,__FILE__,__LINE__,nbatht_alloc*sizeof(int),(void **)&batht_good_offset,&error);
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
				fprintf(stderr,"R7KRECID_7kBeamData: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d ping:%d beams:%d samples:%d\n",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],time_i[6],
					header->RecordNumber,beam->ping_number,beam->number_beams,beam->number_samples);
				}
			if (istore->read_verticaldepth == MB_YES)
				{
				verticaldepth = &(istore->verticaldepth);
				header = &(verticaldepth->header);
				time_j[0] = header->s7kTime.Year;
				time_j[1] = header->s7kTime.Day;
				time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
				time_j[3] = (int) header->s7kTime.Seconds;
				time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
				mb_get_itime(verbose, time_j, time_i);
				mb_get_time(verbose, time_i, &time_d);
				if (verbose > 0)
				fprintf(stderr,"R7KRECID_7kVerticalDepth: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d ping:%d\n",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],time_i[6],
					header->RecordNumber,verticaldepth->ping_number);
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
			
	   	/* handle reference point data */
		else if (status == MB_SUCCESS && istore->type == R7KRECID_ReferencePoint) 
			{
			nrec_reference++;
			
			reference = &(istore->reference);
			header = &(reference->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);
			if (verbose > 0)
				fprintf(stderr,"R7KRECID_ReferencePoint: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
				time_i[0],time_i[1],time_i[2],
				time_i[3],time_i[4],time_i[5],time_i[6],
				header->RecordNumber);
			}
			
	   	/* handle uncalibrated sensor offset data */
		else if (status == MB_SUCCESS && istore->type == R7KRECID_UncalibratedSensorOffset) 
			{
			nrec_sensoruncal++;
			
			sensoruncal = &(istore->sensoruncal);
			header = &(sensoruncal->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);
			if (verbose > 0)
				fprintf(stderr,"R7KRECID_UncalibratedSensorOffset: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
				time_i[0],time_i[1],time_i[2],
				time_i[3],time_i[4],time_i[5],time_i[6],
				header->RecordNumber);
			}
			
	   	/* handle calibrated sensor offset data */
		else if (status == MB_SUCCESS && istore->type == R7KRECID_CalibratedSensorOffset) 
			{
			nrec_sensorcal++;
			
			sensorcal = &(istore->sensorcal);
			header = &(sensorcal->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);
			if (verbose > 0)
				fprintf(stderr,"R7KRECID_CalibratedSensorOffset: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
				time_i[0],time_i[1],time_i[2],
				time_i[3],time_i[4],time_i[5],time_i[6],
				header->RecordNumber);
			}
			
	   	/* handle position data */
		else if (status == MB_SUCCESS && istore->type == R7KRECID_Position) 
			{
			nrec_position++;
			
			position = &(istore->position);
			header = &(position->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);

			if (verbose > 0)
				fprintf(stderr,"R7KRECID_Position: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
			time_i[0],time_i[1],time_i[2],
			time_i[3],time_i[4],time_i[5],time_i[6],
			header->RecordNumber);
				
			/* allocate memory for position arrays if needed */
			if (ndat_nav + 1 >= ndat_nav_alloc)
				{
				ndat_nav_alloc +=  MB7KPREPROCESS_ALLOC_CHUNK;
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_nav_alloc*sizeof(double),(void **)&dat_nav_time_d,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_nav_alloc*sizeof(double),(void **)&dat_nav_lon,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_nav_alloc*sizeof(double),(void **)&dat_nav_lat,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_nav_alloc*sizeof(double),(void **)&dat_nav_speed,&error);
				if (error != MB_ERROR_NO_ERROR)
					{
					mb_error(verbose,error,&message);
					fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
					fprintf(stderr,"\nProgram <%s> Terminated\n",
					    program_name);
					exit(error);
					}		    
				}
				
			/* store the position data */
			if (ndat_nav == 0 || dat_nav_time_d[ndat_nav-1] < time_d)
				{
				dat_nav_time_d[ndat_nav] = time_d + position->latency;
				dat_nav_lon[ndat_nav] = RTD * position->longitude;
				dat_nav_lat[ndat_nav] = RTD * position->latitude;
				dat_nav_speed[ndat_nav] = 0.0;
				ndat_nav++;
				}
			
			}

	   	/* handle customattitude data */
		else if (status == MB_SUCCESS && istore->type == R7KRECID_CustomAttitude) 
			{
			nrec_customattitude++;
			
			customattitude = &(istore->customattitude);
			header = &(customattitude->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);

			if (verbose > 0)
				fprintf(stderr,"R7KRECID_CustomAttitude: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
			time_i[0],time_i[1],time_i[2],
			time_i[3],time_i[4],time_i[5],time_i[6],
			header->RecordNumber);
				
			/* allocate memory for customattitude arrays if needed */
			if (ndat_rph + customattitude->n >= ndat_rph_alloc)
				{
				ndat_rph_alloc += MAX(MB7KPREPROCESS_ALLOC_CHUNK, customattitude->n);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_rph_alloc*sizeof(double),(void **)&dat_rph_time_d,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_rph_alloc*sizeof(double),(void **)&dat_rph_roll,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_rph_alloc*sizeof(double),(void **)&dat_rph_pitch,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_rph_alloc*sizeof(double),(void **)&dat_rph_heave,&error);
				if (error != MB_ERROR_NO_ERROR)
					{
					mb_error(verbose,error,&message);
					fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
					fprintf(stderr,"\nProgram <%s> Terminated\n",
					    program_name);
					exit(error);
					}		    
				}
				
			/* store the customattitude data */
			for (i=0;i<customattitude->n;i++)
				{
				if (ndat_rph == 0 || dat_rph_time_d[ndat_rph-1] < time_d)
					{
					dat_rph_time_d[ndat_rph] = time_d + i / customattitude->frequency;
					dat_rph_roll[ndat_rph] = RTD * customattitude->roll[i];
					dat_rph_pitch[ndat_rph] = RTD * customattitude->pitch[i];
					dat_rph_heave[ndat_rph] = customattitude->heave[i];
					ndat_rph++;
					}
				}
			}
			
	   	/* handle tide data */
		else if (status == MB_SUCCESS && istore->type == R7KRECID_Tide) 
			{
			nrec_tide++;
			
			tide = &(istore->tide);
			header = &(fileheader->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);
			if (verbose > 0)
				fprintf(stderr,"R7KRECID_Tide: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
				time_i[0],time_i[1],time_i[2],
				time_i[3],time_i[4],time_i[5],time_i[6],
				header->RecordNumber);
			}
			
	   	/* handle altitude data */
		else if (status == MB_SUCCESS && istore->type == R7KRECID_Altitude) 
			{
			nrec_altitude++;
			
			altituderec = &(istore->altitude);
			header = &(fileheader->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);
			if (verbose > 0)
				fprintf(stderr,"R7KRECID_Altitude: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
				time_i[0],time_i[1],time_i[2],
				time_i[3],time_i[4],time_i[5],time_i[6],
				header->RecordNumber);
				
			/* allocate memory for altitude arrays if needed */
			if (ndat_altitude + 1 >= ndat_altitude_alloc)
				{
				ndat_altitude_alloc +=  MB7KPREPROCESS_ALLOC_CHUNK;
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_altitude_alloc*sizeof(double),(void **)&dat_altitude_time_d,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_altitude_alloc*sizeof(double),(void **)&dat_altitude_altitude,&error);
				if (error != MB_ERROR_NO_ERROR)
					{
					mb_error(verbose,error,&message);
					fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
					fprintf(stderr,"\nProgram <%s> Terminated\n",
					    program_name);
					exit(error);
					}		    
				}
				
			/* store the altitude data */
			if (ndat_altitude == 0 || dat_altitude_time_d[ndat_altitude-1] < time_d)
				{
				dat_altitude_time_d[ndat_altitude] = time_d;
				dat_altitude_altitude[ndat_altitude] = altituderec->altitude;
				ndat_altitude++;
				}
			}
			
	   	/* handle motion data */
		else if (status == MB_SUCCESS && istore->type == R7KRECID_MotionOverGround) 
			{
			nrec_motion++;
			
			motion = &(istore->motion);
			header = &(motion->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);
			if (verbose > 0)
				fprintf(stderr,"R7KRECID_MotionOverGround: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d n:%d\n",
				time_i[0],time_i[1],time_i[2],
				time_i[3],time_i[4],time_i[5],time_i[6],
				header->RecordNumber,motion->n);
			}
			
	   	/* handle depth data */
		else if (status == MB_SUCCESS && istore->type == R7KRECID_Depth) 
			{
			nrec_depth++;
			
			depth = &(istore->depth);
			header = &(depth->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);
			if (verbose > 0)
				fprintf(stderr,"R7KRECID_Depth: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
				time_i[0],time_i[1],time_i[2],
				time_i[3],time_i[4],time_i[5],time_i[6],
				header->RecordNumber);
				
			/* allocate memory for sonar depth arrays if needed */
			if (ndat_sonardepth + 1 >= ndat_sonardepth_alloc)
				{
				ndat_sonardepth_alloc +=  MB7KPREPROCESS_ALLOC_CHUNK;
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_sonardepth_alloc*sizeof(double),(void **)&dat_sonardepth_time_d,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_sonardepth_alloc*sizeof(double),(void **)&dat_sonardepth_sonardepth,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_sonardepth_alloc*sizeof(double),(void **)&dat_sonardepth_sonardepthrate,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_sonardepth_alloc*sizeof(double),(void **)&dat_sonardepth_sonardepthfilter,&error);
				if (error != MB_ERROR_NO_ERROR)
					{
					mb_error(verbose,error,&message);
					fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
					fprintf(stderr,"\nProgram <%s> Terminated\n",
					    program_name);
					exit(error);
					}		    
				}
				
			/* store the sonar depth data */
			if (ndat_sonardepth == 0 || dat_sonardepth_time_d[ndat_sonardepth-1] < time_d)
				{
				dat_sonardepth_time_d[ndat_sonardepth] = time_d;
				dat_sonardepth_sonardepth[ndat_sonardepth] = depth->depth;
				dat_sonardepth_sonardepthrate[ndat_sonardepth] = 0.0;
				dat_sonardepth_sonardepthfilter[ndat_sonardepth] = 0.0;
				ndat_sonardepth++;
				}
			}
			
	   	/* handle sound velocity data */
		else if (status == MB_SUCCESS && istore->type == R7KRECID_SoundVelocityProfile) 
			{
			nrec_svp++;
			
			svp = &(istore->svp);
			header = &(svp->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);
			if (verbose > 0)
				fprintf(stderr,"R7KRECID_SoundVelocityProfile: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d n:%d\n",
				time_i[0],time_i[1],time_i[2],
				time_i[3],time_i[4],time_i[5],time_i[6],
				header->RecordNumber,svp->n);
			}

	   	/* handle ctd data */
		else if (status == MB_SUCCESS && istore->type == R7KRECID_CTD) 
			{
			nrec_ctd++;
			
			ctd = &(istore->ctd);
			header = &(ctd->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);

			if (verbose > 0)
				fprintf(stderr,"R7KRECID_CTD: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d n:%d\n",
			time_i[0],time_i[1],time_i[2],
			time_i[3],time_i[4],time_i[5],time_i[6],
			header->RecordNumber,ctd->n);
			}

	   	/* handle geodesy data */
		else if (status == MB_SUCCESS && istore->type == R7KRECID_Geodesy) 
			{
			nrec_geodesy++;
			
			geodesy = &(istore->geodesy);
			header = &(geodesy->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);

			if (verbose > 0)
				fprintf(stderr,"R7KRECID_Geodesy: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
			time_i[0],time_i[1],time_i[2],
			time_i[3],time_i[4],time_i[5],time_i[6],
			header->RecordNumber);
			}

	   	/* handle rollpitchheave data */
		else if (status == MB_SUCCESS && istore->type == R7KRECID_RollPitchHeave) 
			{
			nrec_rollpitchheave++;
			
			rollpitchheave = &(istore->rollpitchheave);
			header = &(rollpitchheave->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);

			if (verbose > 0)
				fprintf(stderr,"R7KRECID_RollPitchHeave:               7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
			time_i[0],time_i[1],time_i[2],
			time_i[3],time_i[4],time_i[5],time_i[6],
			header->RecordNumber);
				
			/* allocate memory for rollpitchheave arrays if needed */
			if (ndat_rph + 1 >= ndat_rph_alloc)
				{
				ndat_rph_alloc +=  MB7KPREPROCESS_ALLOC_CHUNK;
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_rph_alloc*sizeof(double),(void **)&dat_rph_time_d,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_rph_alloc*sizeof(double),(void **)&dat_rph_roll,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_rph_alloc*sizeof(double),(void **)&dat_rph_pitch,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_rph_alloc*sizeof(double),(void **)&dat_rph_heave,&error);
				if (error != MB_ERROR_NO_ERROR)
					{
					mb_error(verbose,error,&message);
					fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
					fprintf(stderr,"\nProgram <%s> Terminated\n",
					    program_name);
					exit(error);
					}		    
				}
				
			/* store the rollpitchheave data */
			if (ndat_rph == 0 || dat_rph_time_d[ndat_rph-1] < time_d)
				{
				dat_rph_time_d[ndat_rph] = time_d;
				dat_rph_roll[ndat_rph] = RTD * rollpitchheave->roll;
				dat_rph_pitch[ndat_rph] = RTD * rollpitchheave->pitch;
				dat_rph_heave[ndat_rph] = rollpitchheave->heave;
				ndat_rph++;
				}
			
			}
			
	   	/* handle heading data */
		else if (status == MB_SUCCESS && istore->type == R7KRECID_Heading) 
			{
			nrec_heading++;
			
			headingrec = &(istore->heading);
			header = &(headingrec->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);
			if (verbose > 0)
				fprintf(stderr,"R7KRECID_Heading: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
				time_i[0],time_i[1],time_i[2],
				time_i[3],time_i[4],time_i[5],time_i[6],
				header->RecordNumber);
				
			/* allocate memory for sonar heading arrays if needed */
			if (ndat_heading + 1 >= ndat_heading_alloc)
				{
				ndat_heading_alloc +=  MB7KPREPROCESS_ALLOC_CHUNK;
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_heading_alloc*sizeof(double),(void **)&dat_heading_time_d,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_heading_alloc*sizeof(double),(void **)&dat_heading_heading,&error);
				if (error != MB_ERROR_NO_ERROR)
					{
					mb_error(verbose,error,&message);
					fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
					fprintf(stderr,"\nProgram <%s> Terminated\n",
					    program_name);
					exit(error);
					}		    
				}
				
			/* store the sonar heading data */
			if (ndat_heading == 0 || dat_heading_time_d[ndat_heading-1] < time_d)
				{
				dat_heading_time_d[ndat_heading] = time_d;
				dat_heading_heading[ndat_heading] = RTD * headingrec->heading;
				ndat_heading++;
				}
			}

	   	/* handle survey line data */
		else if (status == MB_SUCCESS && istore->type == R7KRECID_SurveyLine) 
			{
			nrec_surveyline++;
			
			surveyline = &(istore->surveyline);
			header = &(surveyline->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);

			if (verbose > 0)
				fprintf(stderr,"R7KRECID_SurveyLine: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
			time_i[0],time_i[1],time_i[2],
			time_i[3],time_i[4],time_i[5],time_i[6],
			header->RecordNumber);
			}
			
	   	/* handle navigation data */
		else if (status == MB_SUCCESS && istore->type == R7KRECID_Navigation) 
			{
			nrec_navigation++;
			
			navigation = &(istore->navigation);
			header = &(navigation->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);

			if (verbose > 0)
				fprintf(stderr,"R7KRECID_Navigation: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
			time_i[0],time_i[1],time_i[2],
			time_i[3],time_i[4],time_i[5],time_i[6],
			header->RecordNumber);
				
			/* allocate memory for position arrays if needed */
			if (ndat_nav + 1 >= ndat_nav_alloc)
				{
				ndat_nav_alloc +=  MB7KPREPROCESS_ALLOC_CHUNK;
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_nav_alloc*sizeof(double),(void **)&dat_nav_time_d,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_nav_alloc*sizeof(double),(void **)&dat_nav_lon,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_nav_alloc*sizeof(double),(void **)&dat_nav_lat,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_nav_alloc*sizeof(double),(void **)&dat_nav_speed,&error);
				if (error != MB_ERROR_NO_ERROR)
					{
					mb_error(verbose,error,&message);
					fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
					fprintf(stderr,"\nProgram <%s> Terminated\n",
					    program_name);
					exit(error);
					}		    
				}
				
			/* store the navigation data */
			if (ndat_nav == 0 || dat_nav_time_d[ndat_nav-1] < time_d)
				{
				dat_nav_time_d[ndat_nav] = time_d;
				dat_nav_lon[ndat_nav] = RTD * navigation->longitude;
				dat_nav_lat[ndat_nav] = RTD * navigation->latitude;
				dat_nav_speed[ndat_nav] = navigation->speed;
				ndat_nav++;
				}
				
			/* allocate memory for sonar heading arrays if needed */
			if (ndat_heading + 1 >= ndat_heading_alloc)
				{
				ndat_heading_alloc +=  MB7KPREPROCESS_ALLOC_CHUNK;
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_heading_alloc*sizeof(double),(void **)&dat_heading_time_d,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_heading_alloc*sizeof(double),(void **)&dat_heading_heading,&error);
				if (error != MB_ERROR_NO_ERROR)
					{
					mb_error(verbose,error,&message);
					fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
					fprintf(stderr,"\nProgram <%s> Terminated\n",
					    program_name);
					exit(error);
					}		    
				}
				
			/* store the sonar heading data */
			if (ndat_heading == 0 || dat_heading_time_d[ndat_heading-1] < time_d)
				{
				dat_heading_time_d[ndat_heading] = time_d;
				dat_heading_heading[ndat_heading] = RTD * navigation->heading;
				ndat_heading++;
				}			
			}

	   	/* handle attitude data */
		else if (status == MB_SUCCESS && istore->type == R7KRECID_Attitude) 
			{
			nrec_attitude++;
			
			attitude = &(istore->attitude);
			header = &(attitude->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);

			if (verbose > 0)
				fprintf(stderr,"R7KRECID_Attitude: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d n:%d\n",
			time_i[0],time_i[1],time_i[2],
			time_i[3],time_i[4],time_i[5],time_i[6],
			header->RecordNumber,attitude->n);
				
			/* allocate memory for attitude arrays if needed */
			if (ndat_rph + attitude->n >= ndat_rph_alloc)
				{
				ndat_rph_alloc += MB7KPREPROCESS_ALLOC_CHUNK;
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_rph_alloc*sizeof(double),(void **)&dat_rph_time_d,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_rph_alloc*sizeof(double),(void **)&dat_rph_roll,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_rph_alloc*sizeof(double),(void **)&dat_rph_pitch,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_rph_alloc*sizeof(double),(void **)&dat_rph_heave,&error);
				if (error != MB_ERROR_NO_ERROR)
					{
					mb_error(verbose,error,&message);
					fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
					fprintf(stderr,"\nProgram <%s> Terminated\n",
					    program_name);
					exit(error);
					}		    
				}
				
			/* store the attitude data */
			for (i=0;i<attitude->n;i++)
				{
				if (ndat_rph == 0 || dat_rph_time_d[ndat_rph-1] < time_d)
					{
					dat_rph_time_d[ndat_rph] = time_d + i * attitude->delta_time[i];
					dat_rph_roll[ndat_rph] = RTD * attitude->roll[i];
					dat_rph_pitch[ndat_rph] = RTD * attitude->pitch[i];
					dat_rph_heave[ndat_rph] = attitude->heave[i];
					ndat_rph++;
					}
				}
			}
			
	   	/* handle file header data */
		else if (status == MB_SUCCESS && istore->type == R7KRECID_7kFileHeader) 
			{
			nrec_fileheader++;
			
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
				fprintf(stderr,"R7KRECID_7kFileHeader: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
				time_i[0],time_i[1],time_i[2],
				time_i[3],time_i[4],time_i[5],time_i[6],
				header->RecordNumber);
			}
			
	   	/* handle bluefin ctd data */
		else if (status == MB_SUCCESS && istore->type == R7KRECID_Bluefin && kind == MB_DATA_SSV) 
			{
			nrec_bluefinenv++;
			MBARIdata = MB_YES;
			
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
				fprintf(stderr,"R7KRECID_BluefinEnvironmental: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d n:%d\n",
			time_i[0],time_i[1],time_i[2],
			time_i[3],time_i[4],time_i[5],time_i[6],
			header->RecordNumber,bluefin->number_frames);
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
		else if (status == MB_SUCCESS && istore->type == R7KRECID_Bluefin && kind == MB_DATA_NAV2) 
			{
			nrec_bluefinnav++;
			MBARIdata = MB_YES;
			
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
				fprintf(stderr,"R7KRECID_BluefinNav: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d n:%d\n",
			time_i[0],time_i[1],time_i[2],
			time_i[3],time_i[4],time_i[5],time_i[6],
			header->RecordNumber,bluefin->number_frames);
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

				/* output time delay from MBARI AUV */
				if (tfp == NULL)
					{	    
					/* open file for timedelay values */
					sprintf(timelagfile, "%s_timedelay.txt", read_file);
					if ((tfp = fopen(timelagfile, "w")) == NULL) 
						{
						error = MB_ERROR_OPEN_FAIL;
						fprintf(stderr,"\nUnable to open time delay file <%s> for writing\n",timelagfile);
						fprintf(stderr,"\nProgram <%s> Terminated\n",
							program_name);
						exit(error);
						}
					}
				fprintf(tfp,"%f %f\n",
				bluefin->nav[i].position_time,(-0.001*(double)bluefin->nav[i].timedelay));
				}
				
			/* allocate memory for position arrays if needed */
			if (bluefin->number_frames > 0 
				&& ndat_nav + bluefin->number_frames >= ndat_nav_alloc)
				{
				ndat_nav_alloc += MAX(MB7KPREPROCESS_ALLOC_CHUNK, bluefin->number_frames);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_nav_alloc*sizeof(double),(void **)&dat_nav_time_d,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_nav_alloc*sizeof(double),(void **)&dat_nav_lon,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_nav_alloc*sizeof(double),(void **)&dat_nav_lat,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_nav_alloc*sizeof(double),(void **)&dat_nav_speed,&error);
				if (error != MB_ERROR_NO_ERROR)
					{
					mb_error(verbose,error,&message);
					fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
					fprintf(stderr,"\nProgram <%s> Terminated\n",
					    program_name);
					exit(error);
					}		    
				}
				
			/* allocate memory for sonar heading arrays if needed */
			if (bluefin->number_frames > 0 
				&& ndat_heading + bluefin->number_frames >= ndat_heading_alloc)
				{
				ndat_heading_alloc += MAX(MB7KPREPROCESS_ALLOC_CHUNK, bluefin->number_frames);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_heading_alloc*sizeof(double),(void **)&dat_heading_time_d,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_heading_alloc*sizeof(double),(void **)&dat_heading_heading,&error);
				if (error != MB_ERROR_NO_ERROR)
					{
					mb_error(verbose,error,&message);
					fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
					fprintf(stderr,"\nProgram <%s> Terminated\n",
					    program_name);
					exit(error);
					}		    
				}
				
			/* allocate memory for attitude arrays if needed */
			if (bluefin->number_frames > 0 
				&& ndat_rph + bluefin->number_frames >= ndat_rph_alloc)
				{
				ndat_rph_alloc += MAX(MB7KPREPROCESS_ALLOC_CHUNK, bluefin->number_frames);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_rph_alloc*sizeof(double),(void **)&dat_rph_time_d,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_rph_alloc*sizeof(double),(void **)&dat_rph_roll,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_rph_alloc*sizeof(double),(void **)&dat_rph_pitch,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_rph_alloc*sizeof(double),(void **)&dat_rph_heave,&error);
				if (error != MB_ERROR_NO_ERROR)
					{
					mb_error(verbose,error,&message);
					fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
					fprintf(stderr,"\nProgram <%s> Terminated\n",
					    program_name);
					exit(error);
					}		    
				}
				
			/* allocate memory for altitude arrays if needed */
			if (bluefin->number_frames > 0 
				&& ndat_altitude + bluefin->number_frames >= ndat_altitude_alloc)
				{
				ndat_altitude_alloc += MAX(MB7KPREPROCESS_ALLOC_CHUNK, bluefin->number_frames);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_altitude_alloc*sizeof(double),(void **)&dat_altitude_time_d,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_altitude_alloc*sizeof(double),(void **)&dat_altitude_altitude,&error);
				if (error != MB_ERROR_NO_ERROR)
					{
					mb_error(verbose,error,&message);
					fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
					fprintf(stderr,"\nProgram <%s> Terminated\n",
					    program_name);
					exit(error);
					}		    
				}
				
			/* allocate memory for sonar depth arrays if needed */
			if (bluefin->number_frames > 0 
				&& ndat_sonardepth + bluefin->number_frames >= ndat_sonardepth_alloc)
				{
				ndat_sonardepth_alloc += MAX(MB7KPREPROCESS_ALLOC_CHUNK, bluefin->number_frames);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_sonardepth_alloc*sizeof(double),(void **)&dat_sonardepth_time_d,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_sonardepth_alloc*sizeof(double),(void **)&dat_sonardepth_sonardepth,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_sonardepth_alloc*sizeof(double),(void **)&dat_sonardepth_sonardepthrate,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ndat_sonardepth_alloc*sizeof(double),(void **)&dat_sonardepth_sonardepthfilter,&error);
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
				&& ntimedelay + bluefin->number_frames >= ntimedelay_alloc)
				{
				ntimedelay_alloc +=  MB7KPREPROCESS_ALLOC_CHUNK;
				status = mb_reallocd(verbose,__FILE__,__LINE__,ntimedelay_alloc*sizeof(double),(void **)&timedelay_time_d,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,ntimedelay_alloc*sizeof(double),(void **)&timedelay_timedelay,&error);
				}
				
			/* store the navigation and attitude data */
			for (i=0;i<bluefin->number_frames;i++)
				{
				/* store regular nav and attitude values */
				if (ndat_nav == 0 || dat_nav_time_d[ndat_nav-1] < bluefin->nav[i].position_time)
					{
					dat_nav_time_d[ndat_nav] = bluefin->nav[i].position_time;
					dat_nav_lon[ndat_nav] = RTD * bluefin->nav[i].longitude;
					dat_nav_lat[ndat_nav] = RTD * bluefin->nav[i].latitude;
					dat_nav_speed[ndat_nav] = bluefin->nav[i].speed;
					ndat_nav++;
					}
				if (ndat_heading == 0 || dat_heading_time_d[ndat_heading-1] < bluefin->nav[i].position_time)
					{
					dat_heading_time_d[ndat_heading] = bluefin->nav[i].position_time;
					dat_heading_heading[ndat_heading] = RTD * bluefin->nav[i].yaw;
					ndat_heading++;
					}			
				if (ndat_rph == 0 || dat_rph_time_d[ndat_rph-1] < bluefin->nav[i].position_time)
					{
					dat_rph_time_d[ndat_rph] = bluefin->nav[i].position_time;
					dat_rph_roll[ndat_rph] = RTD * bluefin->nav[i].roll;
					dat_rph_pitch[ndat_rph] = RTD * bluefin->nav[i].pitch;
					dat_rph_heave[ndat_rph] = 0.0;
					ndat_rph++;
					}
				if (ndat_altitude == 0 || dat_altitude_time_d[ndat_altitude-1] < bluefin->nav[i].position_time)
					{
					dat_altitude_time_d[ndat_altitude] = bluefin->nav[i].position_time;
					dat_altitude_altitude[ndat_altitude] = bluefin->nav[i].altitude;
					ndat_altitude++;
					}
				if (ndat_sonardepth == 0 || dat_sonardepth_time_d[ndat_sonardepth-1] < bluefin->nav[i].depth_time)
					{
					dat_sonardepth_time_d[ndat_sonardepth] = bluefin->nav[i].depth_time;
					dat_sonardepth_sonardepth[ndat_sonardepth] = bluefin->nav[i].depth;
					dat_sonardepth_sonardepthrate[ndat_sonardepth] = 0.0;
					dat_sonardepth_sonardepthfilter[ndat_sonardepth] = 0.0;
					ndat_sonardepth++;
					}
					
				/* deal with MBARI AUV time delay values */
/*fprintf(stderr,"TIMEDELAYS: count:%d delay: %d",ntimedelaycount,bluefin->nav[i].timedelay);*/
				if (ntimedelaycount == 0)
					{
					timedelay_time_d[ntimedelay] = bluefin->nav[i].position_time;
					timedelay_timedelay[ntimedelay] = (-0.001 * (double)bluefin->nav[i].timedelay);
/*fprintf(stderr,"   USED: %f",timedelay_timedelay[ntimedelay]);*/
					ntimedelay++;
					}
				else if (timedelay_timedelay[ntimedelay-1] 
							> (-0.001 * (double)bluefin->nav[i].timedelay))
					{
					timedelay_time_d[ntimedelay-1] = bluefin->nav[i].position_time;
					timedelay_timedelay[ntimedelay-1] = (-0.001 * (double)bluefin->nav[i].timedelay);
/*fprintf(stderr,"   USED: %d %f",ntimedelay,timedelay_timedelay[ntimedelay-1]);*/
					}
/*fprintf(stderr,"\n");*/
				ntimedelaycount++;
				if (ntimedelaycount >= 100)
					ntimedelaycount = 0;
				}
			
			}
			
			
	   	/* handle subbottom data */
		else if (status == MB_SUCCESS && kind == MB_DATA_SUBBOTTOM_SUBBOTTOM) 
			{
			nrec_fsdwsbp++;

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
			nrec_fsdwsslo++;

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
			if (nedget == 0 || nedget >= nedget_alloc)
				{
				nedget_alloc +=  MB7KPREPROCESS_ALLOC_CHUNK;
				status = mb_reallocd(verbose,__FILE__,__LINE__,nedget_alloc*sizeof(double),(void **)&edget_time_d,&error);
				status = mb_reallocd(verbose,__FILE__,__LINE__,nedget_alloc*sizeof(int),(void **)&edget_ping,&error);
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
			nrec_fsdwsshi++;

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
			nrec_other++;
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
	fprintf(stdout, "     File Header:                       %d\n", nrec_fileheader);
	fprintf(stdout, "     Multibeam:                         %d\n", nrec_multibeam);
	fprintf(stdout, "          Volatile Settings:                 %d\n", nrec_volatilesettings);
	fprintf(stdout, "          Match Filter:                      %d\n", nrec_matchfilter);
	fprintf(stdout, "          Beam Geometry:                     %d\n", nrec_beamgeometry);
	fprintf(stdout, "          Remote Control:                    %d\n", nrec_remotecontrolsettings);
	fprintf(stdout, "          Bathymetry:                        %d\n", nrec_bathymetry);
	fprintf(stdout, "          Backscatter:                       %d\n", nrec_backscatter);
	fprintf(stdout, "          Beam:                              %d\n", nrec_beam);
	fprintf(stdout, "          Image:                             %d\n", nrec_image);
	fprintf(stdout, "     Reference:                         %d\n", nrec_reference);
	fprintf(stdout, "     Uncalibrated Sensor Offset:        %d\n", nrec_sensoruncal);
	fprintf(stdout, "     Calibrated Sensor Offset:          %d\n", nrec_sensorcal);
	fprintf(stdout, "     Position:                          %d\n", nrec_position);
	fprintf(stdout, "     Custom Attitude:                   %d\n", nrec_customattitude);
	fprintf(stdout, "     Tide:                              %d\n", nrec_tide);
	fprintf(stdout, "     Altitude:                          %d\n", nrec_altitude);
	fprintf(stdout, "     Motion Over Ground:                %d\n", nrec_motion);
	fprintf(stdout, "     Depth:                             %d\n", nrec_depth);
	fprintf(stdout, "     Sound Speed Profile:               %d\n", nrec_svp);
	fprintf(stdout, "     CTD:                               %d\n", nrec_ctd);
	fprintf(stdout, "     Geodosy:                           %d\n", nrec_geodesy);
	fprintf(stdout, "     Roll Pitch Heave:                  %d\n", nrec_rollpitchheave);
	fprintf(stdout, "     Heading:                           %d\n", nrec_heading);
	fprintf(stdout, "     Survey Line:                       %d\n", nrec_surveyline);
	fprintf(stdout, "     Navigation:                        %d\n", nrec_navigation);
	fprintf(stdout, "     Attitude:                          %d\n", nrec_attitude);
	fprintf(stdout, "     Edgetech Low Frequency Sidescan:   %d\n", nrec_fsdwsslo);
	fprintf(stdout, "     Edgetech High Frequency Sidescan:  %d\n", nrec_fsdwsshi);
	fprintf(stdout, "     Edgetech Subbottom:                %d\n", nrec_fsdwsbp);
	fprintf(stdout, "     MBARI Mapping AUV Environmental:   %d\n", nrec_bluefinnav);
	fprintf(stdout, "     MBARI Mapping AUV Navigation:      %d\n", nrec_bluefinenv);
	fprintf(stdout, "     Configuration:                     %d\n", nrec_configuration);
	fprintf(stdout, "     Calibration:                       %d\n", nrec_calibration);
	fprintf(stdout, "     Vertical Depth:                    %d\n", nrec_verticaldepth);
	fprintf(stdout, "     Installation:                      %d\n", nrec_installation);
	fprintf(stdout, "     System Event Message:              %d\n", nrec_systemeventmessage);
	fprintf(stdout, "     Other:                             %d\n", nrec_other);
	nrec_fileheader_tot += nrec_fileheader;
	nrec_multibeam_tot += nrec_multibeam;
	nrec_volatilesettings_tot += nrec_volatilesettings;
	nrec_matchfilter_tot += nrec_matchfilter;
	nrec_beamgeometry_tot += nrec_beamgeometry;
	nrec_remotecontrolsettings_tot += nrec_remotecontrolsettings;
	nrec_bathymetry_tot += nrec_bathymetry;
	nrec_backscatter_tot += nrec_backscatter;
	nrec_beam_tot += nrec_beam;
	nrec_image_tot += nrec_image;	
	nrec_reference_tot += nrec_reference;
	nrec_sensoruncal_tot += nrec_sensoruncal;
	nrec_sensorcal_tot += nrec_sensorcal;
	nrec_position_tot += nrec_position;
	nrec_customattitude_tot += nrec_customattitude;
	nrec_tide_tot += nrec_tide;
	nrec_altitude_tot += nrec_altitude;
	nrec_motion_tot += nrec_motion;
	nrec_depth_tot += nrec_depth;
	nrec_svp_tot += nrec_svp;
	nrec_ctd_tot += nrec_ctd;
	nrec_geodesy_tot += nrec_geodesy;
	nrec_rollpitchheave_tot += nrec_rollpitchheave;
	nrec_heading_tot += nrec_heading;
	nrec_surveyline_tot += nrec_surveyline;
	nrec_navigation_tot += nrec_navigation;
	nrec_attitude_tot += nrec_attitude;
	nrec_fsdwsbp_tot += nrec_fsdwsbp;
	nrec_fsdwsslo_tot += nrec_fsdwsslo;
	nrec_fsdwsshi_tot += nrec_fsdwsshi;
	nrec_bluefinenv_tot += nrec_bluefinenv;
	nrec_position_tot += nrec_position;
	nrec_bluefinnav_tot += nrec_bluefinnav;
	nrec_configuration_tot += nrec_configuration;
	nrec_calibration_tot += nrec_calibration;
	nrec_verticaldepth_tot += nrec_verticaldepth;
	nrec_installation_tot += nrec_installation;
	nrec_systemeventmessage_tot += nrec_systemeventmessage;
	nrec_other_tot += nrec_other;

	/* figure out whether and what to read next */
        if (read_datalist == MB_YES)
                {
		if ((status = mb_datalist_read(verbose,datalist,
			    ifile,&format,&file_weight,&error))
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
		
	/* close time delay file */
	if (tfp != NULL)
		fclose(tfp);
	
	/* apply time lag to all relevant data 
		timelag value calculated either from model imported from file (timelagmode == MB7KPREPROCESS_TIMELAG_MODEL)
			or by a constant offset (timelagmode == MB7KPREPROCESS_TIMELAG_CONSTANT) plus any timedelay
				values embedded in the data (MBARI AUV bluefin nav records only) */
	if (timelagmode != MB7KPREPROCESS_TIMELAG_OFF)
		{
		/* correct time of navigation, heading, attitude, sonardepth, altitude 
			read from asynchronous records in 7k files */
fprintf(stderr,"Applying timelag to %d nav data\n", ndat_nav);
		j = 0;
		for (i=0;i<ndat_nav;i++)
			{
			/* get timelag value */
			timelag = 0.0;
			if (ntimedelay > 0)
			interp_status = mb_linear_interp(verbose, 
						timedelay_time_d-1, timedelay_timedelay-1,
						ntimedelay, dat_nav_time_d[i], &timelag, &j, 
						&error);
			if (timelagmode == MB7KPREPROCESS_TIMELAG_CONSTANT)
				{
				timelag -= timelagconstant;
				}
			else if (timelagmode == MB7KPREPROCESS_TIMELAG_MODEL && ntimelag > 0)
				{
				interp_status = mb_linear_interp(verbose, 
							timelag_time_d-1, timelag_model-1,
							ntimelag, dat_nav_time_d[i], &timelagm, &j, 
							&error);
				timelag -= timelagm;
				}
			dat_nav_time_d[i] += timelag;
			}
fprintf(stderr,"Applying timelag to %d heading data\n", ndat_heading);
		j = 0;
		for (i=0;i<ndat_heading;i++)
			{
			/* get timelag value */
			timelag = 0.0;
			if (ntimedelay > 0)
			interp_status = mb_linear_interp(verbose, 
						timedelay_time_d-1, timedelay_timedelay-1,
						ntimedelay, dat_heading_time_d[i], &timelag, &j, 
						&error);
			if (timelagmode == MB7KPREPROCESS_TIMELAG_CONSTANT)
				{
				timelag -= timelagconstant;
				}
			else if (timelagmode == MB7KPREPROCESS_TIMELAG_MODEL && ntimelag > 0)
				{
				interp_status = mb_linear_interp(verbose, 
							timelag_time_d-1, timelag_model-1,
							ntimelag, dat_heading_time_d[i], &timelagm, &j, 
							&error);
				timelag -= timelagm;
				}
			dat_heading_time_d[i] += timelag;
			}
fprintf(stderr,"Applying timelag to %d attitude data\n", ndat_rph);
		j = 0;
		for (i=0;i<ndat_rph;i++)
			{
			/* get timelag value */
			timelag = 0.0;
			if (ntimedelay > 0)
			interp_status = mb_linear_interp(verbose, 
						timedelay_time_d-1, timedelay_timedelay-1,
						ntimedelay, dat_rph_time_d[i], &timelag, &j, 
						&error);
			if (timelagmode == MB7KPREPROCESS_TIMELAG_CONSTANT)
				{
				timelag -= timelagconstant;
				}
			else if (timelagmode == MB7KPREPROCESS_TIMELAG_MODEL && ntimelag > 0)
				{
				interp_status = mb_linear_interp(verbose, 
							timelag_time_d-1, timelag_model-1,
							ntimelag, dat_rph_time_d[i], &timelagm, &j, 
							&error);
				timelag -= timelagm;
				}
			dat_rph_time_d[i] += timelag;
			}
fprintf(stderr,"Applying timelag to %d sonardepth data\n", ndat_sonardepth);
		j = 0;
		for (i=0;i<ndat_sonardepth;i++)
			{
			/* get timelag value */
			timelag = 0.0;
			if (ntimedelay > 0)
			interp_status = mb_linear_interp(verbose, 
						timedelay_time_d-1, timedelay_timedelay-1,
						ntimedelay, dat_sonardepth_time_d[i], &timelag, &j, 
						&error);
			if (timelagmode == MB7KPREPROCESS_TIMELAG_CONSTANT)
				{
				timelag -= timelagconstant;
				}
			else if (timelagmode == MB7KPREPROCESS_TIMELAG_MODEL && ntimelag > 0)
				{
				interp_status = mb_linear_interp(verbose, 
							timelag_time_d-1, timelag_model-1,
							ntimelag, dat_sonardepth_time_d[i], &timelagm, &j, 
							&error);
				timelag -= timelagm;
				}
			dat_sonardepth_time_d[i] += timelag;
			}
fprintf(stderr,"Applying timelag to %d altitude data\n", ndat_altitude);
		j = 0;
		for (i=0;i<ndat_altitude;i++)
			{
			/* get timelag value */
			timelag = 0.0;
			if (ntimedelay > 0)
			interp_status = mb_linear_interp(verbose, 
						timedelay_time_d-1, timedelay_timedelay-1,
						ntimedelay, dat_altitude_time_d[i], &timelag, &j, 
						&error);
			if (timelagmode == MB7KPREPROCESS_TIMELAG_CONSTANT)
				{
				timelag -= timelagconstant;
				}
			else if (timelagmode == MB7KPREPROCESS_TIMELAG_MODEL && ntimelag > 0)
				{
				interp_status = mb_linear_interp(verbose, 
							timelag_time_d-1, timelag_model-1,
							ntimelag, dat_altitude_time_d[i], &timelagm, &j, 
							&error);
				timelag -= timelagm;
				}
			dat_altitude_time_d[i] += timelag;
			}
			
		/* correct time of INS data read from MBARI AUV log file */
fprintf(stderr,"Applying timelag to %d INS data\n", nins);
		for (i=0;i<nins;i++)
			{
			/* get timelag value */
			timelag = 0.0;
			if (ntimedelay > 0)
			interp_status = mb_linear_interp(verbose, 
						timedelay_time_d-1, timedelay_timedelay-1,
						ntimedelay, ins_time_d[i], &timelag, &j, 
						&error);
			if (timelagmode == MB7KPREPROCESS_TIMELAG_CONSTANT)
				{
				timelag -= timelagconstant;
				}
			else if (timelagmode == MB7KPREPROCESS_TIMELAG_MODEL && ntimelag > 0)
				{
				interp_status = mb_linear_interp(verbose, 
							timelag_time_d-1, timelag_model-1,
							ntimelag, ins_time_d[i], &timelagm, &j, 
							&error);
				timelag -= timelagm;
				}
			ins_time_d[i] += timelag;
			}
fprintf(stderr,"Applying timelag to %d INS altitude data\n", nins_altitude);
		for (i=0;i<nins_altitude;i++)
			{
			/* get timelag value */
			timelag = 0.0;
			if (ntimedelay > 0)
			interp_status = mb_linear_interp(verbose, 
						timedelay_time_d-1, timedelay_timedelay-1,
						ntimedelay, ins_altitude_time_d[i], &timelag, &j, 
						&error);
			if (timelagmode == MB7KPREPROCESS_TIMELAG_CONSTANT)
				{
				timelag -= timelagconstant;
				}
			else if (timelagmode == MB7KPREPROCESS_TIMELAG_MODEL && ntimelag > 0)
				{
				interp_status = mb_linear_interp(verbose, 
							timelag_time_d-1, timelag_model-1,
							ntimelag, ins_altitude_time_d[i], &timelagm, &j, 
							&error);
				timelag -= timelagm;
				}
			ins_altitude_time_d[i] += timelag;
			}
fprintf(stderr,"Applying timelag to %d INS speed data\n", nins_speed);
		for (i=0;i<nins_speed;i++)
			{
			/* get timelag value */
			timelag = 0.0;
			if (ntimedelay > 0)
			interp_status = mb_linear_interp(verbose, 
						timedelay_time_d-1, timedelay_timedelay-1,
						ntimedelay, ins_speed_time_d[i], &timelag, &j, 
						&error);
			if (timelagmode == MB7KPREPROCESS_TIMELAG_CONSTANT)
				{
				timelag -= timelagconstant;
				}
			else if (timelagmode == MB7KPREPROCESS_TIMELAG_MODEL && ntimelag > 0)
				{
				interp_status = mb_linear_interp(verbose, 
							timelag_time_d-1, timelag_model-1,
							ntimelag, ins_speed_time_d[i], &timelagm, &j, 
							&error);
				timelag -= timelagm;
				}
			ins_speed_time_d[i] += timelag;
			}
			
		/* correct time of navigation and attitude data read from WHOI DSL nav and attitude file */
fprintf(stderr,"Applying timelag to %d DSL nav data\n", ndsl);
		for (i=0;i<ndsl;i++)
			{
			/* get timelag value */
			timelag = 0.0;
			if (ntimedelay > 0)
			interp_status = mb_linear_interp(verbose, 
						timedelay_time_d-1, timedelay_timedelay-1,
						ntimedelay, dsl_time_d[i], &timelag, &j, 
						&error);
			if (timelagmode == MB7KPREPROCESS_TIMELAG_CONSTANT)
				{
				timelag -= timelagconstant;
				}
			else if (timelagmode == MB7KPREPROCESS_TIMELAG_MODEL && ntimelag > 0)
				{
				interp_status = mb_linear_interp(verbose, 
							timelag_time_d-1, timelag_model-1,
							ntimelag, dsl_time_d[i], &timelagm, &j, 
							&error);
				timelag -= timelagm;
				}
			dsl_time_d[i] += timelag;
			}
			
		/* correct time of navigation and attitude data read from Steve Rock file */
fprintf(stderr,"Applying timelag to %d Steve Rock nav data\n", nrock);
		for (i=0;i<nrock;i++)
			{
			/* get timelag value */
			timelag = 0.0;
			if (ntimedelay > 0)
			interp_status = mb_linear_interp(verbose, 
						timedelay_time_d-1, timedelay_timedelay-1,
						ntimedelay, rock_time_d[i], &timelag, &j, 
						&error);
			if (timelagmode == MB7KPREPROCESS_TIMELAG_CONSTANT)
				{
				timelag -= timelagconstant;
				}
			else if (timelagmode == MB7KPREPROCESS_TIMELAG_MODEL && ntimelag > 0)
				{
				interp_status = mb_linear_interp(verbose, 
							timelag_time_d-1, timelag_model-1,
							ntimelag, rock_time_d[i], &timelagm, &j, 
							&error);
				timelag -= timelagm;
				}
			rock_time_d[i] += timelag;
			}
			
		/* correct time of sonar depth data read from separate file */
fprintf(stderr,"Applying timelag to %d sonardepth nav data\n", nsonardepth);
		for (i=0;i<nsonardepth;i++)
			{
			/* get timelag value */
			timelag = 0.0;
			if (ntimedelay > 0)
			interp_status = mb_linear_interp(verbose, 
						timedelay_time_d-1, timedelay_timedelay-1,
						ntimedelay, sonardepth_time_d[i], &timelag, &j, 
						&error);
			if (timelagmode == MB7KPREPROCESS_TIMELAG_CONSTANT)
				{
				timelag -= timelagconstant;
				}
			else if (timelagmode == MB7KPREPROCESS_TIMELAG_MODEL && ntimelag > 0)
				{
				interp_status = mb_linear_interp(verbose, 
							timelag_time_d-1, timelag_model-1,
							ntimelag, sonardepth_time_d[i], &timelagm, &j, 
							&error);
				timelag -= timelagm;
				}
			sonardepth_time_d[i] += timelag;
			}
		}
				
	/* if desired apply filtering to sonardepth data */
	if (sonardepthfilter == MB_YES)
		{
		/* apply filtering to sonardepth data 
			read from asynchronous records in 7k files */
fprintf(stderr,"Applying filtering to %d sonardepth data\n", ndat_sonardepth);
		for (i=0;i<ndat_sonardepth;i++)
			{
			dat_sonardepth_sonardepthfilter[i] = 0.0;
			sonardepth_filterweight = 0.0;
			for (j=0;j<ndat_sonardepth;j++)
				{
				dtol = (dat_sonardepth_time_d[j] - dat_sonardepth_time_d[i]) / sonardepthfilterlength;
				if (fabs(dtol) < 4.0)
					{
					weight = exp(-dtol * dtol);
					dat_sonardepth_sonardepthfilter[i] += weight * dat_sonardepth_sonardepth[j];
					sonardepth_filterweight += weight;
					}
				}
			if (sonardepth_filterweight > 0.0)
				dat_sonardepth_sonardepthfilter[i] /= sonardepth_filterweight;
			}
		for (i=0;i<ndat_sonardepth;i++)
			{
			if (dat_sonardepth_sonardepth[i] < 2.0 * sonardepthfilterdepth)
				factor = 1.0;
			else
				factor = exp(-(dat_sonardepth_sonardepth[i] - 2.0 * sonardepthfilterdepth)
						/ (sonardepthfilterdepth));
			dat_sonardepth_sonardepth[i] = (1.0 - factor) * dat_sonardepth_sonardepth[i]
						+ factor * dat_sonardepth_sonardepthfilter[i];
			}

		/* filter sonardepth data from separate file */
fprintf(stderr,"Applying filtering to %d sonardepth nav data\n", nsonardepth);
		for (i=0;i<nsonardepth;i++)
			{
			sonardepth_sonardepthfilter[i] = 0.0;
			sonardepth_filterweight = 0.0;
			for (j=0;j<nsonardepth;j++)
				{
				dtol = (sonardepth_time_d[j] - sonardepth_time_d[i]) / sonardepthfilterlength;
				if (fabs(dtol) < 4.0)
					{
					weight = exp(-dtol * dtol);
					sonardepth_sonardepthfilter[i] += weight * sonardepth_sonardepth[j];
					sonardepth_filterweight += weight;
					}
				}
			if (sonardepth_filterweight > 0.0)
				sonardepth_sonardepthfilter[i] /= sonardepth_filterweight;
			}
		for (i=0;i<nsonardepth;i++)
			{
			if (sonardepth_sonardepth[i] < 2.0 * sonardepthfilterdepth)
				factor = 1.0;
			else
				factor = exp(-(sonardepth_sonardepth[i] - 2.0 * sonardepthfilterdepth)
						/ (sonardepthfilterdepth));
			sonardepth_sonardepth[i] = (1.0 - factor) * sonardepth_sonardepth[i]
						+ factor * sonardepth_sonardepthfilter[i];
			}

		/* filter sonardepth data from separate INS file */
fprintf(stderr,"Applying filtering to %d INS nav data\n", nins);
		for (i=0;i<nins;i++)
			{
			ins_sonardepthfilter[i] = 0.0;
			sonardepth_filterweight = 0.0;
			for (j=0;j<nins;j++)
				{
				dtol = (ins_time_d[j] - ins_time_d[i]) / sonardepthfilterlength;
				if (fabs(dtol) < 4.0)
					{
					weight = exp(-dtol * dtol);
					ins_sonardepthfilter[i] += weight * ins_sonardepth[j];
					sonardepth_filterweight += weight;
					}
				}
			if (sonardepth_filterweight > 0.0)
				ins_sonardepthfilter[i] /= sonardepth_filterweight;
			}
		for (i=0;i<nins;i++)
			{
			if (ins_sonardepth[i] < 2.0 * sonardepthfilterdepth)
				factor = 1.0;
			else
				factor = exp(-(ins_sonardepth[i] - 2.0 * sonardepthfilterdepth)
						/ (sonardepthfilterdepth));
			ins_sonardepth[i] = (1.0 - factor) * ins_sonardepth[i]
						+ factor * ins_sonardepthfilter[i];
			}

		/* filter sonardepth data from separate WHOI DSL file */
fprintf(stderr,"Applying filtering to %d DSL nav data\n", ndsl);
		for (i=0;i<ndsl;i++)
			{
			dsl_sonardepthfilter[i] = 0.0;
			sonardepth_filterweight = 0.0;
			for (j=0;j<ndsl;j++)
				{
				dtol = (dsl_time_d[j] - dsl_time_d[i]) / sonardepthfilterlength;
				if (fabs(dtol) < 4.0)
					{
					weight = exp(-dtol * dtol);
					dsl_sonardepthfilter[i] += weight * dsl_sonardepth[j];
					sonardepth_filterweight += weight;
					}
				}
			if (sonardepth_filterweight > 0.0)
				dsl_sonardepthfilter[i] /= sonardepth_filterweight;
			}
		for (i=0;i<ndsl;i++)
			{
			if (dsl_sonardepth[i] < 2.0 * sonardepthfilterdepth)
				factor = 1.0;
			else
				factor = exp(-(dsl_sonardepth[i] - 2.0 * sonardepthfilterdepth)
						/ (sonardepthfilterdepth));
			dsl_sonardepth[i] = (1.0 - factor) * dsl_sonardepth[i]
						+ factor * dsl_sonardepthfilter[i];
			}

		/* filter sonardepth data from separate Steve Rock file */
fprintf(stderr,"Applying filtering to %d Rock nav data\n", nrock);
		for (i=0;i<nrock;i++)
			{
			rock_sonardepthfilter[i] = 0.0;
			sonardepth_filterweight = 0.0;
			for (j=0;j<nrock;j++)
				{
				dtol = (rock_time_d[j] - rock_time_d[i]) / sonardepthfilterlength;
				if (fabs(dtol) < 4.0)
					{
					weight = exp(-dtol * dtol);
					rock_sonardepthfilter[i] += weight * rock_sonardepth[j];
					sonardepth_filterweight += weight;
					}
				}
			if (sonardepth_filterweight > 0.0)
				rock_sonardepthfilter[i] /= sonardepth_filterweight;
			}
		for (i=0;i<nrock;i++)
			{
			if (rock_sonardepth[i] < 2.0 * sonardepthfilterdepth)
				factor = 1.0;
			else
				factor = exp(-(rock_sonardepth[i] - 2.0 * sonardepthfilterdepth)
						/ (sonardepthfilterdepth));
			rock_sonardepth[i] = (1.0 - factor) * rock_sonardepth[i]
						+ factor * rock_sonardepthfilter[i];
			}
		}
		
	/* calculate sonardepth change rate for variable lag correction - asynchronous data only */
	if (sonardepthlagfix == MB_YES && ndat_sonardepth > 1)
		{
fprintf(stderr,"Calculating sonardepth change rate for %d sonardepth data\n", ndat_sonardepth);
		for (i=0;i<ndat_sonardepth;i++)
			{
			if (i == 0)
				{
				dat_sonardepth_sonardepthrate[i] = (dat_sonardepth_sonardepth[i+1] - dat_sonardepth_sonardepth[i]) 
								/ (dat_sonardepth_time_d[i+1] - dat_sonardepth_time_d[i]);
				}
			else if (i == ndat_sonardepth - 1)
				{
				dat_sonardepth_sonardepthrate[i] = (dat_sonardepth_sonardepth[i] - dat_sonardepth_sonardepth[i-1]) 
								/ (dat_sonardepth_time_d[i] - dat_sonardepth_time_d[i-1]);
				}
			else
				{
				dat_sonardepth_sonardepthrate[i] = (dat_sonardepth_sonardepth[i+1] - dat_sonardepth_sonardepth[i-1]) 
								/ (dat_sonardepth_time_d[i+1] - dat_sonardepth_time_d[i-1]);
				}
			dat_sonardepth_sonardepthrate[i] = fabs(dat_sonardepth_sonardepthrate[i]);
			}
		}
		
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
		
	/* output ins navigation and attitude data */
	if (nins > 0 && (verbose > 0 || mode == MB7KPREPROCESS_TIMESTAMPLIST))
		{
		fprintf(stdout, "\nTotal INS navigation/attitude data read: %d\n", nins);
		for (i=0;i<nins;i++)
			{
			fprintf(stdout, "  INS: %12d %17.6f %11.6f %10.6f %8.3f %7.3f %6.3f %6.3f %6.3f %6.3f\n", 
				i, ins_time_d[i], ins_lon[i], ins_lat[i], ins_heading[i],
				ins_sonardepth[i], ins_altitude[i], ins_speed[i],
				ins_roll[i], ins_pitch[i]);
			}
		fprintf(stdout, "\nTotal INS altitude data read: %d\n", nins_altitude);
		for (i=0;i<nins_altitude;i++)
			{
			fprintf(stdout, "  INS ALT: %12d %17.6f %6.3f\n", 
				i, ins_altitude_time_d[i], ins_altitude[i]);
			}
		fprintf(stdout, "\nTotal INS speed data read: %d\n", nins_speed);
		for (i=0;i<nins_speed;i++)
			{
			fprintf(stdout, "  INS SPD: %12d %17.6f %6.3f\n", 
				i, ins_speed_time_d[i], ins_speed[i]);
			}
		}
		
	/* output auv sonardepth data */
	if (nsonardepth > 0 && (verbose >= 0 || mode == MB7KPREPROCESS_TIMESTAMPLIST))
		{
		fprintf(stdout, "\nTotal auv sonardepth data read: %d\n", nsonardepth);
		for (i=0;i<nins;i++)
			{
			fprintf(stdout, "  SONARDEPTH: %12d %8.3f %8.3f\n", 
				i, sonardepth_time_d[i], sonardepth_sonardepth[i]);
			}
		}

	/* output 7k navigation and attitude data */
	if (verbose > 0 || mode == MB7KPREPROCESS_TIMESTAMPLIST)
		{
		fprintf(stdout, "\nTotal 7k navigation data read: %d\n", ndat_nav);
		for (i=0;i<ndat_nav;i++)
			{
			fprintf(stdout, "  NAV: %5d %17.6f %11.6f %10.6f %6.3f\n", 
				i, dat_nav_time_d[i], dat_nav_lon[i], dat_nav_lat[i], dat_nav_speed[i]);
			}
		fprintf(stdout, "\nTotal heading data read: %d\n", ndat_heading);
		for (i=0;i<ndat_heading;i++)
			{
			fprintf(stdout, "  HDG: %5d %17.6f %8.3f\n", 
				i, dat_heading_time_d[i], dat_heading_heading[i]);
			}
		fprintf(stdout, "\nTotal sonardepth data read: %d\n", ndat_sonardepth);
		for (i=0;i<ndat_sonardepth;i++)
			{
			fprintf(stdout, "  DEP: %5d %17.6f %8.3f %8.3f\n", 
				i, dat_sonardepth_time_d[i], dat_sonardepth_sonardepth[i], dat_sonardepth_sonardepthrate[i]);
			}
		fprintf(stdout, "\nTotal altitude data read: %d\n", ndat_altitude);
		for (i=0;i<ndat_altitude;i++)
			{
			fprintf(stdout, "  ALT: %5d %17.6f %8.3f\n", 
				i, dat_altitude_time_d[i], dat_altitude_altitude[i]);
			}
		fprintf(stdout, "\nTotal attitude data read: %d\n", ndat_rph);
		for (i=0;i<ndat_rph;i++)
			{
			fprintf(stdout, "  ALT: %5d %17.6f %8.3f %8.3f %8.3f\n", 
				i, dat_rph_time_d[i], dat_rph_roll[i], dat_rph_pitch[i], dat_rph_heave[i]);
			}
		fprintf(stdout, "\nTotal Edgetech time stamp data read: %d\n", nedget);
		for (i=0;i<nedget;i++)
			{
			fprintf(stdout, "  EDG: %5d %17.6f %d\n", 
				i, edget_time_d[i], edget_ping[i]);
			}
		fprintf(stdout, "\nTotal multibeam time stamp data read: %d\n", nbatht);
		for (i=0;i<nbatht;i++)
			{
			fprintf(stdout, "  BAT: %5d %17.6f %17.6f %5d   offsets: %17.6f %5d  %5d\n", 
				i, batht_time_d[i], batht_time_d_new[i], batht_ping[i], batht_time_offset[i], batht_ping_offset[i], batht_good_offset[i]);
			}
		}
	
	/* output counts */
	fprintf(stdout, "\nTotal data records read from: %s\n", read_file);
	fprintf(stdout, "     File Header:                       %d\n", nrec_fileheader_tot);
	fprintf(stdout, "     Multibeam:                         %d\n", nrec_multibeam_tot);
	fprintf(stdout, "          Volatile Settings:                 %d\n", nrec_volatilesettings_tot);
	fprintf(stdout, "          Match Filter:                      %d\n", nrec_matchfilter_tot);
	fprintf(stdout, "          Beam Geometry:                     %d\n", nrec_beamgeometry_tot);
	fprintf(stdout, "          Remote Control:                    %d\n", nrec_remotecontrolsettings_tot);
	fprintf(stdout, "          Bathymetry:                        %d\n", nrec_bathymetry_tot);
	fprintf(stdout, "          Backscatter:                       %d\n", nrec_backscatter_tot);
	fprintf(stdout, "          Beam:                              %d\n", nrec_beam_tot);
	fprintf(stdout, "          Image:                             %d\n", nrec_image_tot);
	fprintf(stdout, "     Reference:                         %d\n", nrec_reference_tot);
	fprintf(stdout, "     Uncalibrated Sensor Offset:        %d\n", nrec_sensoruncal_tot);
	fprintf(stdout, "     Calibrated Sensor Offset:          %d\n", nrec_sensorcal_tot);
	fprintf(stdout, "     Position:                          %d\n", nrec_position_tot);
	fprintf(stdout, "     Custom Attitude:                   %d\n", nrec_customattitude_tot);
	fprintf(stdout, "     Tide:                              %d\n", nrec_tide_tot);
	fprintf(stdout, "     Altitude:                          %d\n", nrec_altitude_tot);
	fprintf(stdout, "     Motion Over Ground:                %d\n", nrec_motion_tot);
	fprintf(stdout, "     Depth:                             %d\n", nrec_depth_tot);
	fprintf(stdout, "     Sound Speed Profile:               %d\n", nrec_svp_tot);
	fprintf(stdout, "     CTD:                               %d\n", nrec_ctd_tot);
	fprintf(stdout, "     Geodosy:                           %d\n", nrec_geodesy_tot);
	fprintf(stdout, "     Roll Pitch Heave:                  %d\n", nrec_rollpitchheave_tot);
	fprintf(stdout, "     Heading:                           %d\n", nrec_heading_tot);
	fprintf(stdout, "     Survey Line:                       %d\n", nrec_surveyline_tot);
	fprintf(stdout, "     Navigation:                        %d\n", nrec_navigation_tot);
	fprintf(stdout, "     Attitude:                          %d\n", nrec_attitude_tot);
	fprintf(stdout, "     Edgetech Low Frequency Sidescan:   %d\n", nrec_fsdwsslo_tot);
	fprintf(stdout, "     Edgetech High Frequency Sidescan:  %d\n", nrec_fsdwsshi_tot);
	fprintf(stdout, "     Edgetech Subbottom:                %d\n", nrec_fsdwsbp_tot);
	fprintf(stdout, "     MBARI Mapping AUV Environmental:   %d\n", nrec_bluefinnav_tot);
	fprintf(stdout, "     MBARI Mapping AUV Navigation:      %d\n", nrec_bluefinenv_tot);
	fprintf(stdout, "     Configuration:                     %d\n", nrec_configuration_tot);
	fprintf(stdout, "     Calibration:                       %d\n", nrec_calibration_tot);
	fprintf(stdout, "     Vertical Depth:                    %d\n", nrec_verticaldepth_tot);
	fprintf(stdout, "     Installation:                      %d\n", nrec_installation_tot);
	fprintf(stdout, "     System Event Message:              %d\n", nrec_systemeventmessage_tot);
	fprintf(stdout, "     Other:                             %d\n", nrec_other_tot);

	nrec_reference_tot = 0;
	nrec_sensoruncal_tot = 0;
	nrec_sensorcal_tot = 0;
	nrec_position_tot = 0;
	nrec_customattitude_tot = 0;
	nrec_tide_tot = 0;
	nrec_altitude_tot = 0;
	nrec_motion_tot = 0;
	nrec_depth_tot = 0;
	nrec_svp_tot = 0;
	nrec_ctd_tot = 0;
	nrec_geodesy_tot = 0;
	nrec_rollpitchheave_tot = 0;
	nrec_heading_tot = 0;
	nrec_surveyline_tot = 0;
	nrec_navigation_tot = 0;
	nrec_attitude_tot = 0;
	nrec_fsdwsslo_tot = 0;
	nrec_fsdwsshi_tot = 0;
	nrec_fsdwsbp_tot = 0;
	nrec_bluefinnav_tot = 0;
	nrec_bluefinenv_tot = 0;
	nrec_multibeam_tot = 0;
	nrec_volatilesettings_tot = 0;
	nrec_configuration_tot = 0;
	nrec_matchfilter_tot = 0;
	nrec_beamgeometry_tot = 0;
	nrec_calibration_tot = 0;
	nrec_bathymetry_tot = 0;
	nrec_backscatter_tot = 0;
	nrec_beam_tot = 0;
	nrec_verticaldepth_tot = 0;
	nrec_image_tot = 0;
	nrec_installation_tot = 0;
	nrec_systemeventmessage_tot = 0;
	nrec_fileheader_tot = 0;
	nrec_remotecontrolsettings_tot = 0;
	nrec_other_tot = 0;

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
	    if ((status = mb_datalist_read(verbose,datalist,
			    ifile,&format,&file_weight,&error))
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
	status = mb_get_format(verbose, ifile, fileroot, &testformat, &error);
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
		
	/* initialize ctd output file */
	sprintf(ctdfile,"%s_ctd.txt",fileroot);
	if ((tfp = fopen(ctdfile, "w")) == NULL) 
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr,"\nUnable to open ctd data file <%s> for writing\n",ctdfile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
		
	/* initialize asynchronous heading output file */
	sprintf(athfile,"%s.ath",ofile);
	if ((athfp = fopen(athfile, "w")) == NULL) 
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr,"\nUnable to open asynchronous heading data file <%s> for writing\n",athfile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
		
	/* initialize asynchronous sonardepth output file */
	sprintf(atsfile,"%s.ats",ofile);
	if ((atsfp = fopen(atsfile, "w")) == NULL) 
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr,"\nUnable to open asynchronous sonardepth data file <%s> for writing\n",atsfile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
		
	/* initialize asynchronous attitude output file */
	sprintf(atafile,"%s.ata",ofile);
	if ((atafp = fopen(atafile, "w")) == NULL) 
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr,"\nUnable to open asynchronous attitude data file <%s> for writing\n",atafile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
		
	/* initialize synchronous attitude output file */
	sprintf(stafile,"%s.sta",ofile);
	if ((stafp = fopen(stafile, "w")) == NULL) 
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr,"\nUnable to open synchronous attitude data file <%s> for writing\n",stafile);
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

	/* reset file record counters */
	nrec_reference = 0;
	nrec_sensoruncal = 0;
	nrec_sensorcal = 0;
	nrec_position = 0;
	nrec_customattitude = 0;
	nrec_tide = 0;
	nrec_altitude = 0;
	nrec_motion = 0;
	nrec_depth = 0;
	nrec_svp = 0;
	nrec_ctd = 0;
	nrec_geodesy = 0;
	nrec_rollpitchheave = 0;
	nrec_heading = 0;
	nrec_surveyline = 0;
	nrec_navigation = 0;
	nrec_attitude = 0;
	nrec_fsdwsslo = 0;
	nrec_fsdwsshi = 0;
	nrec_fsdwsbp = 0;
	nrec_bluefinnav = 0;
	nrec_bluefinenv = 0;
	nrec_multibeam = 0;
	nrec_volatilesettings = 0;
	nrec_configuration = 0;
	nrec_matchfilter = 0;
	nrec_beamgeometry = 0;
	nrec_calibration = 0;
	nrec_bathymetry = 0;
	nrec_backscatter = 0;
	nrec_beam = 0;
	nrec_verticaldepth = 0;
	nrec_image = 0;
	nrec_installation = 0;
	nrec_systemeventmessage = 0;
	nrec_fileheader = 0;
	nrec_remotecontrolsettings = 0;
	nrec_other = 0;
		
	/* read and print data */
	while (error <= MB_ERROR_NO_ERROR)
		{
		/* reset error */
		status = MB_SUCCESS;
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
			nrec_multibeam++;
			
			bathymetry = &(istore->bathymetry);
			if (istore->read_volatilesettings == MB_YES)
				nrec_volatilesettings++;
			if (istore->read_matchfilter == MB_YES)
				nrec_matchfilter++;
			if (istore->read_beamgeometry == MB_YES)
				nrec_beamgeometry++;
			if (istore->read_remotecontrolsettings == MB_YES)
				nrec_remotecontrolsettings++;
			if (istore->read_bathymetry == MB_YES)
				nrec_bathymetry++;
			if (istore->read_backscatter == MB_YES)
				nrec_backscatter++;
			if (istore->read_beam == MB_YES)
				nrec_beam++;
			if (istore->read_verticaldepth == MB_YES)
				nrec_verticaldepth++;
			if (istore->read_image == MB_YES)
				nrec_image++;
				
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
			if (istore->read_matchfilter == MB_YES)
				{
				matchfilter = &(istore->matchfilter);
				header = &(matchfilter->header);
				time_j[0] = header->s7kTime.Year;
				time_j[1] = header->s7kTime.Day;
				time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
				time_j[3] = (int) header->s7kTime.Seconds;
				time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
				mb_get_itime(verbose, time_j, time_i);
				mb_get_time(verbose, time_i, &time_d);
				if (verbose > 0)
				fprintf(stderr,"R7KRECID_7kMatchFilter:            7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
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
			if (istore->read_remotecontrolsettings == MB_YES)
				{
				remotecontrolsettings = &(istore->remotecontrolsettings);
				header = &(remotecontrolsettings->header);
				time_j[0] = header->s7kTime.Year;
				time_j[1] = header->s7kTime.Day;
				time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
				time_j[3] = (int) header->s7kTime.Seconds;
				time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
				mb_get_itime(verbose, time_j, time_i);
				mb_get_time(verbose, time_i, &time_d);
				if (verbose > 0)
				fprintf(stderr,"R7KRECID_7kremotecontrolsettings:  7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],time_i[6],
					header->RecordNumber);
				}
			if (istore->read_bathymetry != MB_YES)
				{
				status = MB_FAILURE;
				error = MB_ERROR_IGNORE;
				}
			else if (istore->read_bathymetry == MB_YES)
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
				last_7k_time_d = MAX(last_7k_time_d, time_d);
				if (verbose > 0)
				fprintf(stderr,"R7KRECID_7kBathymetricData:        7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d ping:%d beams:%d\n",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],time_i[6],
					header->RecordNumber,bathymetry->ping_number,bathymetry->number_beams);
				if (last_7k_time_d > time_d)
					{
					status = MB_FAILURE;
					error = MB_ERROR_IGNORE;
					}
					
				/* apply fixes to good data */
				if (status == MB_SUCCESS)
					{
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
									bathymetry->quality[i] = 23;
									}
								else if (bathymetry->range[i] > 0.0)
									{
									bathymetry->quality[i] = 20;
									}
								else
									{
									bathymetry->quality[i] = 0;
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

					/* fix early MBARI version 5 quality flags */
					else if (bathymetry->header.Version == 5 
							&& MBARIdata == MB_YES
							&& header->s7kTime.Year < 2008)
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

					/* fix upgraded MBARI version 5 quality flags */
					else if (bathymetry->header.Version >= 5 
							&& MBARIdata == MB_YES
							&& header->s7kTime.Year <= 2010)
						{
						for (i=0;i<bathymetry->number_beams;i++)
							{
/* fprintf(stderr,"S Flag[%d]: %d\n",i,bathymetry->quality[i]); */
							bathymetry->quality[i] = bathymetry->quality[i] & 15;

							/* phase or amplitude picks */
							if (bathymetry->quality[i] & 8)
								{
/* fprintf(stderr,"beam %d: PHASE quality: %d",i,bathymetry->quality[i]); */
								bathymetry->quality[i] += 32;
/* fprintf(stderr," %d\n",bathymetry->quality[i]); */
								}
							else if (bathymetry->quality[i] & 4)
								{
/* fprintf(stderr,"beam %d: AMPLI quality: %d",i,bathymetry->quality[i]); */
								bathymetry->quality[i] += 16;
/* fprintf(stderr," %d\n",bathymetry->quality[i]); */
								}

							/* flagged by sonar */
							if ((bathymetry->quality[i] & 3) == 0 && bathymetry->quality[i] > 0)
								{
								bathymetry->quality[i] += 64;
								}
/* fprintf(stderr,"E Flag[%d]: %d\n\n",i,bathymetry->quality[i]); */
							}
						}

					/* fix upgraded version 5 quality flags */
					else if (bathymetry->header.Version >= 5)
						{
						for (i=0;i<bathymetry->number_beams;i++)
							{
/* fprintf(stderr,"S Flag[%d]: %d\n",i,bathymetry->quality[i]); */
							bathymetry->quality[i] = bathymetry->quality[i] & 15;

							/* phase or amplitude picks */
							if (bathymetry->quality[i] & 8)
								{
/* fprintf(stderr,"beam %d: PHASE quality: %d",i,bathymetry->quality[i]); */
								bathymetry->quality[i] += 32;
/* fprintf(stderr," %d\n",bathymetry->quality[i]); */
								}
							else if (bathymetry->quality[i] & 4)
								{
/* fprintf(stderr,"beam %d: AMPLI quality: %d",i,bathymetry->quality[i]); */
								bathymetry->quality[i] += 16;
/* fprintf(stderr," %d\n",bathymetry->quality[i]); */
								}

							/* flagged by sonar */
							if ((bathymetry->quality[i] & 3) == 3)
								{
								}
							else if ((bathymetry->quality[i] & 3) == 0 && bathymetry->quality[i] > 0)
								{
								bathymetry->quality[i] += 64;
								}
							else if (bathymetry->quality[i] > 0)
								{
								bathymetry->quality[i] += 64;
								}
/* fprintf(stderr,"E Flag[%d]: %d\n\n",i,bathymetry->quality[i]); */
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
					interp_status = MB_SUCCESS;

					/* get nav */
					j = 0;
					if (nins > 0)
						{
						interp_status = mb_linear_interp_degrees(verbose, 
									ins_time_d-1, ins_lon-1,
									nins, time_d, &navlon, &j, 
									&error);
						if (interp_status == MB_SUCCESS)
						interp_status = mb_linear_interp_degrees(verbose, 
									ins_time_d-1, ins_lat-1,
									nins, time_d, &navlat, &j, 
									&error);
						if (interp_status == MB_SUCCESS)
						interp_status = mb_linear_interp(verbose, 
									ins_speed_time_d-1, ins_speed-1,
									nins_speed, time_d, &speed, &j, 
									&error);
						}
					else if (nrock > 0)
						{
						interp_status = mb_linear_interp_degrees(verbose, 
									rock_time_d-1, rock_lon-1,
									nrock, time_d, &navlon, &j, 
									&error);
						if (interp_status == MB_SUCCESS)
						interp_status = mb_linear_interp_degrees(verbose, 
									rock_time_d-1, rock_lat-1,
									nrock, time_d, &navlat, &j, 
									&error);
						if (j > 1)
							{
							j1 = j - 2;
							j2 = j - 1;
							}
						else
							{
							j1 = j - 1;
							j2 = j;
							}
						mb_coor_scale(verbose,navlat,&mtodeglon,&mtodeglat);
						dx = (rock_lon[j2] - rock_lon[j1])/mtodeglon;
						dy = (rock_lat[j2] - rock_lat[j1])/mtodeglat;
						dist = sqrt(dx*dx + dy*dy);
						dt = (rock_time_d[j2] - rock_time_d[j1]);
						if (dt > 0.0)
							speed = 3.6 * dist / dt;
						}
					else if (ndsl > 0)
						{
						interp_status = mb_linear_interp_degrees(verbose, 
									dsl_time_d-1, dsl_lon-1,
									ndsl, time_d, &navlon, &j, 
									&error);
						if (interp_status == MB_SUCCESS)
						interp_status = mb_linear_interp_degrees(verbose, 
									dsl_time_d-1, dsl_lat-1,
									ndsl, time_d, &navlat, &j, 
									&error);
						if (j > 1)
							{
							j1 = j - 2;
							j2 = j - 1;
							}
						else
							{
							j1 = j - 1;
							j2 = j;
							}
						mb_coor_scale(verbose,navlat,&mtodeglon,&mtodeglat);
						dx = (dsl_lon[j2] - dsl_lon[j1])/mtodeglon;
						dy = (dsl_lat[j2] - dsl_lat[j1])/mtodeglat;
						dist = sqrt(dx*dx + dy*dy);
						dt = (dsl_time_d[j2] - dsl_time_d[j1]);
						if (dt > 0.0)
							speed = 3.6 * dist / dt;
						}
					else if (ndat_nav > 0)
						{
						interp_status = mb_linear_interp_degrees(verbose, 
									dat_nav_time_d-1, dat_nav_lon-1,
									ndat_nav, time_d, &navlon, &j, 
									&error);
						if (interp_status == MB_SUCCESS)
						interp_status = mb_linear_interp_degrees(verbose, 
									dat_nav_time_d-1, dat_nav_lat-1,
									ndat_nav, time_d, &navlat, &j, 
									&error);
						if (interp_status == MB_SUCCESS)
						interp_status = mb_linear_interp_degrees(verbose, 
									dat_nav_time_d-1, dat_nav_speed-1,
									ndat_nav, time_d, &speed, &j, 
									&error);
						}
					else
						{
						navlon = 0.0;
						navlat = 0.0;
						speed = 0.0;
						}

					/* get heading */
					j = 0;
					if (interp_status != MB_SUCCESS)
						{
						}
					else if (nins > 0)
						{
						interp_status = mb_linear_interp_degrees(verbose, 
									ins_time_d-1, ins_heading-1,
									nins, time_d, &heading, &j, 
									&error);
						}
					else if (nrock > 0)
						{
						interp_status = mb_linear_interp_degrees(verbose, 
									rock_time_d-1, rock_heading-1,
									nrock, time_d, &heading, &j, 
									&error);
						}
					else if (ndsl > 0)
						{
						interp_status = mb_linear_interp_degrees(verbose, 
									dsl_time_d-1, dsl_heading-1,
									ndsl, time_d, &heading, &j, 
									&error);
						}
					else if (ndat_heading > 0)
						{
						interp_status = mb_linear_interp_degrees(verbose, 
									dat_heading_time_d-1, dat_heading_heading-1,
									ndat_heading, time_d, &heading, &j, 
									&error);
						}
					else
						{
						heading = 0.0;
						}

					/* get altitude */
					if (interp_status != MB_SUCCESS)
						{
						}
					else if (nins > 0)
						{
						interp_status = mb_linear_interp(verbose, 
									ins_altitude_time_d-1, ins_altitude-1,
									nins_altitude, time_d, &altitude, &j, 
									&error);
						}
					else if (ndat_altitude > 0)
						{
						interp_status = mb_linear_interp(verbose, 
									dat_altitude_time_d-1, dat_altitude_altitude-1,
									ndat_altitude, time_d, &altitude, &j, 
									&error);
						}
					else
						{
						altitude = 0.0;
						}

					/* get attitude */
					if (interp_status != MB_SUCCESS)
						{
						}
					else if (nins > 0)
						{
						interp_status = mb_linear_interp(verbose, 
									ins_time_d-1, ins_roll-1,
									nins, time_d, &roll, &j, 
									&error);
						if (interp_status == MB_SUCCESS)
						interp_status = mb_linear_interp(verbose, 
									ins_time_d-1, ins_pitch-1,
									nins, time_d, &pitch, &j, 
									&error);
						}
					else if (nrock > 0)
						{
						interp_status = mb_linear_interp(verbose, 
									rock_time_d-1, rock_roll-1,
									nrock, time_d, &roll, &j, 
									&error);
						if (interp_status == MB_SUCCESS)
						interp_status = mb_linear_interp(verbose, 
									rock_time_d-1, rock_pitch-1,
									nrock, time_d, &pitch, &j, 
									&error);
						}
					else if (ndsl > 0)
						{
						interp_status = mb_linear_interp(verbose, 
									dsl_time_d-1, dsl_roll-1,
									ndsl, time_d, &roll, &j, 
									&error);
						if (interp_status == MB_SUCCESS)
						interp_status = mb_linear_interp(verbose, 
									dsl_time_d-1, dsl_pitch-1,
									ndsl, time_d, &pitch, &j, 
									&error);
						}
					else if (ndat_rph > 0)
						{
						interp_status = mb_linear_interp(verbose, 
									dat_rph_time_d-1, dat_rph_roll-1,
									ndat_rph, time_d, &roll, &j, 
									&error);
						if (interp_status == MB_SUCCESS)
						interp_status = mb_linear_interp(verbose, 
									dat_rph_time_d-1, dat_rph_pitch-1,
									ndat_rph, time_d, &pitch, &j, 
									&error);
						}
					else
						{
						roll = 0.0;
						pitch = 0.0;
						}

					/* get sonar depth */
					if (kluge_useverticaldepth == MB_YES)
						{
						verticaldepth = (s7kr_verticaldepth *)&(istore->verticaldepth);
						sonardepth = (double)(verticaldepth->vertical_depth);
						}
					else if (interp_status != MB_SUCCESS)
						{
						}
					else if (nsonardepth > 0)
						{
						if (interp_status == MB_SUCCESS)
						interp_status = mb_linear_interp(verbose, 
									sonardepth_time_d-1, sonardepth_sonardepth-1,
									nsonardepth, time_d, &sonardepth, &j, 
									&error);
						}
					else if (nins > 0)
						{
						interp_status = mb_linear_interp(verbose, 
									ins_time_d-1, ins_sonardepth-1,
									nins, time_d, &sonardepth, &j, 
									&error);
						}
					else if (nrock > 0)
						{
						interp_status = mb_linear_interp(verbose, 
									rock_time_d-1, rock_sonardepth-1,
									nrock, time_d, &sonardepth, &j, 
									&error);
						}
					else if (ndsl > 0)
						{
						interp_status = mb_linear_interp(verbose, 
									dsl_time_d-1, dsl_sonardepth-1,
									ndsl, time_d, &sonardepth, &j, 
									&error);
						}
					else if (ndat_sonardepth > 0)
						{
						sonardepthlag = 0.0;
						if (sonardepthlagfix == MB_YES && ndat_sonardepth > 1 
							&& sonardepthratemax > 0.0
							&& interp_status == MB_SUCCESS)
							{
							interp_status = mb_linear_interp(verbose, 
									dat_sonardepth_time_d-1, dat_sonardepth_sonardepthrate-1,
									ndat_sonardepth, time_d, &sonardepthrate, &j, 
									&error);
							sonardepthlag = sonardepthrate * sonardepthlagmax / sonardepthratemax;
							if (sonardepthrate >= sonardepthratemax)
								sonardepthlag = sonardepthlagmax;
							}
						if (interp_status == MB_SUCCESS)
						interp_status = mb_linear_interp(verbose, 
									dat_sonardepth_time_d-1, dat_sonardepth_sonardepth-1,
									ndat_sonardepth, time_d + sonardepthlag, &sonardepth, &j, 
									&error);
						}
					else if (ndat_rph > 0)
						{
						interp_status = mb_linear_interp(verbose, 
									dat_rph_time_d-1, dat_rph_heave-1,
									ndat_rph, time_d, &heave, &j, 
									&error);
						sonardepth = heave;
						}
					else
						{
						sonardepth = 0.0;
						}

					/* apply offset between depth sensor and sonar */
					sonardepth += sonardepthoffset 
							+ depthsensoroffx * sin(DTR * pitch)
							+ depthsensoroffz * cos(DTR * pitch);

					/* if the optional data are not all available, this ping
						is not useful, and is discarded by setting
						*error to MB_ERROR_MISSING_NAVATTITUDE
						unless the -N flag has been specified */
					if (interp_status == MB_FAILURE && goodnavattitudeonly == MB_YES)
						{
						status = MB_FAILURE;
						error = MB_ERROR_MISSING_NAVATTITUDE;
						}
					}

				/* if the optional data are available, then proceed */
				if (status == MB_SUCCESS)
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
					
					/* zero alongtrack angles if requested */
					if (kluge_zeroalongtrackangles == MB_YES)
						{
						for (i=0;i<bathymetry->number_beams;i++)
							{
							beamgeometry->angle_alongtrack[i] = 0.0;
							}
						}
					
					/* get bathymetry */
					if (volatilesettings->sound_velocity > 0.0)
						soundspeed = volatilesettings->sound_velocity;
					else if (bluefin->environmental[0].sound_speed > 0.0)
						soundspeed = bluefin->environmental[0].sound_speed;
					else
						soundspeed = 1500.0;
/* fprintf(stderr,"roll:%f pitch:%f\n",bathymetry->roll,bathymetry->pitch); */
					rollr = DTR * roll;
					pitchr = DTR * pitch;
					/* zero atttitude correction if requested */
					if (kluge_zeroattitudecorrection == MB_YES)
						{
						rollr = 0.0;
						pitchr = 0.0;
						}
					for (i=0;i<bathymetry->number_beams;i++)
						{
/* fprintf(stderr,"i:%d quality:%d range:%f\n",i,bathymetry->quality[i],bathymetry->range[i]); */
						if ((bathymetry->quality[i] & 15) > 0)
							{
							alpha = RTD * (beamgeometry->angle_alongtrack[i] + pitchr);
							beta = 90.0 - RTD * (beamgeometry->angle_acrosstrack[i] - rollr);
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
/* fprintf(stderr,"i:%d roll:%f %f pitch:%f %f alpha:%f beta:%f theta:%f phi:%f  depth:%f %f %f\n",
i,roll, bathymetry->roll,pitch, bathymetry->pitch,
alpha,beta,theta,phi,
bathymetry->depth[i],bathymetry->acrosstrack[i],bathymetry->alongtrack[i]); */
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
					
					/* output synchronous attitude */
					fprintf(stafp, "%0.6f\t%0.3f\t%0.3f\n", 
						time_d, roll, pitch);
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
				fprintf(stderr,"R7KRECID_7kBeamData: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d ping:%d beams:%d samples:%d\n",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],time_i[6],
					header->RecordNumber,beam->ping_number,beam->number_beams,beam->number_samples);
				}
			if (istore->read_verticaldepth == MB_YES)
				{
				verticaldepth = &(istore->verticaldepth);
				header = &(verticaldepth->header);
				time_j[0] = header->s7kTime.Year;
				time_j[1] = header->s7kTime.Day;
				time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
				time_j[3] = (int) header->s7kTime.Seconds;
				time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
				mb_get_itime(verbose, time_j, time_i);
				mb_get_time(verbose, time_i, &time_d);
				if (verbose > 0)
				fprintf(stderr,"R7KRECID_7kVerticalDepth: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d ping:%d\n",
					time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],time_i[6],
					header->RecordNumber,verticaldepth->ping_number);
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
			
	   	/* handle reference point data */
		else if (status == MB_SUCCESS && istore->type == R7KRECID_ReferencePoint) 
			{
			nrec_reference++;
			
			reference = &(istore->reference);
			header = &(reference->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);
			if (verbose > 0)
				fprintf(stderr,"R7KRECID_ReferencePoint: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
				time_i[0],time_i[1],time_i[2],
				time_i[3],time_i[4],time_i[5],time_i[6],
				header->RecordNumber);
			}
			
	   	/* handle uncalibrated sensor offset data */
		else if (status == MB_SUCCESS && istore->type == R7KRECID_UncalibratedSensorOffset) 
			{
			nrec_sensoruncal++;
			
			sensoruncal = &(istore->sensoruncal);
			header = &(sensoruncal->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);
			if (verbose > 0)
				fprintf(stderr,"R7KRECID_UncalibratedSensorOffset: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
				time_i[0],time_i[1],time_i[2],
				time_i[3],time_i[4],time_i[5],time_i[6],
				header->RecordNumber);
			}
			
	   	/* handle calibrated sensor offset data */
		else if (status == MB_SUCCESS && istore->type == R7KRECID_CalibratedSensorOffset) 
			{
			nrec_sensorcal++;
			
			sensorcal = &(istore->sensorcal);
			header = &(sensorcal->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);
			if (verbose > 0)
				fprintf(stderr,"R7KRECID_CalibratedSensorOffset: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
				time_i[0],time_i[1],time_i[2],
				time_i[3],time_i[4],time_i[5],time_i[6],
				header->RecordNumber);
			}
			
	   	/* handle position data */
		else if (status == MB_SUCCESS && istore->type == R7KRECID_Position) 
			{
			nrec_position++;
			
			position = &(istore->position);
			header = &(position->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);

			/* get timelag value */
			timelag = 0.0;
			if (ntimedelay > 0)
			interp_status = mb_linear_interp(verbose, 
						timedelay_time_d-1, timedelay_timedelay-1,
						ntimedelay, time_d, &timelag, &j, 
						&error);
			if (timelagmode == MB7KPREPROCESS_TIMELAG_CONSTANT)
				{
				timelag -= timelagconstant;
				}
			else if (timelagmode == MB7KPREPROCESS_TIMELAG_MODEL && ntimelag > 0)
				{
				interp_status = mb_linear_interp(verbose, 
							timelag_time_d-1, timelag_model-1,
							ntimelag, time_d, &timelagm, &j, 
							&error);
				timelag -= timelagm;
				}
			time_d += timelag;
			mb_get_date(verbose, time_d,time_i);
			mb_get_jtime(verbose, time_i, time_j);
			header->s7kTime.Year = time_i[0];
			header->s7kTime.Day = time_j[1];
			header->s7kTime.Hours = time_i[3];
			header->s7kTime.Minutes = time_i[4];
			header->s7kTime.Seconds = time_i[5] + 0.000001 * time_i[6];

			if (verbose > 0)
				fprintf(stderr,"R7KRECID_Position: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
			time_i[0],time_i[1],time_i[2],
			time_i[3],time_i[4],time_i[5],time_i[6],
			header->RecordNumber);
			}

	   	/* handle customattitude data */
		else if (status == MB_SUCCESS && istore->type == R7KRECID_CustomAttitude) 
			{
			nrec_customattitude++;
			
			customattitude = &(istore->customattitude);
			header = &(customattitude->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);

			/* get timelag value */
			timelag = 0.0;
			if (ntimedelay > 0)
			interp_status = mb_linear_interp(verbose, 
						timedelay_time_d-1, timedelay_timedelay-1,
						ntimedelay, time_d, &timelag, &j, 
						&error);
			if (timelagmode == MB7KPREPROCESS_TIMELAG_CONSTANT)
				{
				timelag -= timelagconstant;
				}
			else if (timelagmode == MB7KPREPROCESS_TIMELAG_MODEL && ntimelag > 0)
				{
				interp_status = mb_linear_interp(verbose, 
							timelag_time_d-1, timelag_model-1,
							ntimelag, time_d, &timelagm, &j, 
							&error);
				timelag -= timelagm;
				}
			time_d += timelag;
			mb_get_date(verbose, time_d,time_i);
			mb_get_jtime(verbose, time_i, time_j);
			header->s7kTime.Year = time_i[0];
			header->s7kTime.Day = time_j[1];
			header->s7kTime.Hours = time_i[3];
			header->s7kTime.Minutes = time_i[4];
			header->s7kTime.Seconds = time_i[5] + 0.000001 * time_i[6];

			if (verbose > 0)
				fprintf(stderr,"R7KRECID_CustomAttitude: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
			time_i[0],time_i[1],time_i[2],
			time_i[3],time_i[4],time_i[5],time_i[6],
			header->RecordNumber);
			}
			
	   	/* handle tide data */
		else if (status == MB_SUCCESS && istore->type == R7KRECID_Tide) 
			{
			nrec_tide++;
			
			tide = &(istore->tide);
			header = &(fileheader->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);
			if (verbose > 0)
				fprintf(stderr,"R7KRECID_Tide: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
				time_i[0],time_i[1],time_i[2],
				time_i[3],time_i[4],time_i[5],time_i[6],
				header->RecordNumber);
			}
			
	   	/* handle altitude data */
		else if (status == MB_SUCCESS && istore->type == R7KRECID_Altitude) 
			{
			nrec_altitude++;
			
			altituderec = &(istore->altitude);
			header = &(fileheader->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);

			/* get timelag value */
			timelag = 0.0;
			if (ntimedelay > 0)
			interp_status = mb_linear_interp(verbose, 
						timedelay_time_d-1, timedelay_timedelay-1,
						ntimedelay, time_d, &timelag, &j, 
						&error);
			if (timelagmode == MB7KPREPROCESS_TIMELAG_CONSTANT)
				{
				timelag -= timelagconstant;
				}
			else if (timelagmode == MB7KPREPROCESS_TIMELAG_MODEL && ntimelag > 0)
				{
				interp_status = mb_linear_interp(verbose, 
							timelag_time_d-1, timelag_model-1,
							ntimelag, time_d, &timelagm, &j, 
							&error);
				timelag -= timelagm;
				}
			time_d += timelag;
			mb_get_date(verbose, time_d,time_i);
			mb_get_jtime(verbose, time_i, time_j);
			header->s7kTime.Year = time_i[0];
			header->s7kTime.Day = time_j[1];
			header->s7kTime.Hours = time_i[3];
			header->s7kTime.Minutes = time_i[4];
			header->s7kTime.Seconds = time_i[5] + 0.000001 * time_i[6];

			if (verbose > 0)
				fprintf(stderr,"R7KRECID_Altitude: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
				time_i[0],time_i[1],time_i[2],
				time_i[3],time_i[4],time_i[5],time_i[6],
				header->RecordNumber);
			}
			
	   	/* handle motion data */
		else if (status == MB_SUCCESS && istore->type == R7KRECID_MotionOverGround) 
			{
			nrec_motion++;
			
			motion = &(istore->motion);
			header = &(motion->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);

			/* get timelag value */
			timelag = 0.0;
			if (ntimedelay > 0)
			interp_status = mb_linear_interp(verbose, 
						timedelay_time_d-1, timedelay_timedelay-1,
						ntimedelay, time_d, &timelag, &j, 
						&error);
			if (timelagmode == MB7KPREPROCESS_TIMELAG_CONSTANT)
				{
				timelag -= timelagconstant;
				}
			else if (timelagmode == MB7KPREPROCESS_TIMELAG_MODEL && ntimelag > 0)
				{
				interp_status = mb_linear_interp(verbose, 
							timelag_time_d-1, timelag_model-1,
							ntimelag, time_d, &timelagm, &j, 
							&error);
				timelag -= timelagm;
				}
			time_d += timelag;
			mb_get_date(verbose, time_d,time_i);
			mb_get_jtime(verbose, time_i, time_j);
			header->s7kTime.Year = time_i[0];
			header->s7kTime.Day = time_j[1];
			header->s7kTime.Hours = time_i[3];
			header->s7kTime.Minutes = time_i[4];
			header->s7kTime.Seconds = time_i[5] + 0.000001 * time_i[6];

			if (verbose > 0)
				fprintf(stderr,"R7KRECID_MotionOverGround: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d n:%d\n",
				time_i[0],time_i[1],time_i[2],
				time_i[3],time_i[4],time_i[5],time_i[6],
				header->RecordNumber,motion->n);
			}
			
	   	/* handle depth data */
		else if (status == MB_SUCCESS && istore->type == R7KRECID_Depth) 
			{
			nrec_depth++;
			
			depth = &(istore->depth);
			header = &(depth->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);

			/* get timelag value */
			timelag = 0.0;
			if (ntimedelay > 0)
			interp_status = mb_linear_interp(verbose, 
						timedelay_time_d-1, timedelay_timedelay-1,
						ntimedelay, time_d, &timelag, &j, 
						&error);
			if (timelagmode == MB7KPREPROCESS_TIMELAG_CONSTANT)
				{
				if (ntimedelay > 0)
				interp_status = mb_linear_interp(verbose, 
							timedelay_time_d-1, timedelay_timedelay-1,
							ntimedelay, time_d, &timelag, &j, 
							&error);
				timelag -= timelagconstant;
				}
			else if (timelagmode == MB7KPREPROCESS_TIMELAG_MODEL && ntimelag > 0)
				{
				interp_status = mb_linear_interp(verbose, 
							timelag_time_d-1, timelag_model-1,
							ntimelag, time_d, &timelagm, &j, 
							&error);
				timelag -= timelagm;
				}
			time_d += timelag;
			mb_get_date(verbose, time_d,time_i);
			mb_get_jtime(verbose, time_i, time_j);
			header->s7kTime.Year = time_i[0];
			header->s7kTime.Day = time_j[1];
			header->s7kTime.Hours = time_i[3];
			header->s7kTime.Minutes = time_i[4];
			header->s7kTime.Seconds = time_i[5] + 0.000001 * time_i[6];

			if (verbose > 0)
				fprintf(stderr,"R7KRECID_Depth: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
				time_i[0],time_i[1],time_i[2],
				time_i[3],time_i[4],time_i[5],time_i[6],
				header->RecordNumber);
			}
			
	   	/* handle sound velocity data */
		else if (status == MB_SUCCESS && istore->type == R7KRECID_SoundVelocityProfile) 
			{
			nrec_svp++;
			
			svp = &(istore->svp);
			header = &(svp->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);

			/* get timelag value */
			timelag = 0.0;
			if (ntimedelay > 0)
			interp_status = mb_linear_interp(verbose, 
						timedelay_time_d-1, timedelay_timedelay-1,
						ntimedelay, time_d, &timelag, &j, 
						&error);
			if (timelagmode == MB7KPREPROCESS_TIMELAG_CONSTANT)
				{
				timelag -= timelagconstant;
				}
			else if (timelagmode == MB7KPREPROCESS_TIMELAG_MODEL && ntimelag > 0)
				{
				interp_status = mb_linear_interp(verbose, 
							timelag_time_d-1, timelag_model-1,
							ntimelag, time_d, &timelagm, &j, 
							&error);
				timelag -= timelagm;
				}
			time_d += timelag;
			mb_get_date(verbose, time_d,time_i);
			mb_get_jtime(verbose, time_i, time_j);
			header->s7kTime.Year = time_i[0];
			header->s7kTime.Day = time_j[1];
			header->s7kTime.Hours = time_i[3];
			header->s7kTime.Minutes = time_i[4];
			header->s7kTime.Seconds = time_i[5] + 0.000001 * time_i[6];

			if (verbose > 0)
				fprintf(stderr,"R7KRECID_SoundVelocityProfile: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d n:%d\n",
				time_i[0],time_i[1],time_i[2],
				time_i[3],time_i[4],time_i[5],time_i[6],
				header->RecordNumber,svp->n);
			}

	   	/* handle ctd data */
		else if (status == MB_SUCCESS && istore->type == R7KRECID_CTD) 
			{
			nrec_ctd++;
			
			ctd = &(istore->ctd);
			header = &(ctd->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);

			/* get timelag value */
			timelag = 0.0;
			if (ntimedelay > 0)
			interp_status = mb_linear_interp(verbose, 
						timedelay_time_d-1, timedelay_timedelay-1,
						ntimedelay, time_d, &timelag, &j, 
						&error);
			if (timelagmode == MB7KPREPROCESS_TIMELAG_CONSTANT)
				{
				timelag -= timelagconstant;
				}
			else if (timelagmode == MB7KPREPROCESS_TIMELAG_MODEL && ntimelag > 0)
				{
				interp_status = mb_linear_interp(verbose, 
							timelag_time_d-1, timelag_model-1,
							ntimelag, time_d, &timelagm, &j, 
							&error);
				timelag -= timelagm;
				}
			time_d += timelag;
			mb_get_date(verbose, time_d,time_i);
			mb_get_jtime(verbose, time_i, time_j);
			header->s7kTime.Year = time_i[0];
			header->s7kTime.Day = time_j[1];
			header->s7kTime.Hours = time_i[3];
			header->s7kTime.Minutes = time_i[4];
			header->s7kTime.Seconds = time_i[5] + 0.000001 * time_i[6];

			if (verbose > 0)
				fprintf(stderr,"R7KRECID_CTD: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d n:%d\n",
			time_i[0],time_i[1],time_i[2],
			time_i[3],time_i[4],time_i[5],time_i[6],
			header->RecordNumber,ctd->n);
			}

	   	/* handle geodesy data */
		else if (status == MB_SUCCESS && istore->type == R7KRECID_Geodesy) 
			{
			nrec_geodesy++;
			
			geodesy = &(istore->geodesy);
			header = &(geodesy->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);

			if (verbose > 0)
				fprintf(stderr,"R7KRECID_Geodesy: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
			time_i[0],time_i[1],time_i[2],
			time_i[3],time_i[4],time_i[5],time_i[6],
			header->RecordNumber);
			}

	   	/* handle rollpitchheave data */
		else if (status == MB_SUCCESS && istore->type == R7KRECID_RollPitchHeave) 
			{
			nrec_rollpitchheave++;
			
			rollpitchheave = &(istore->rollpitchheave);
			header = &(rollpitchheave->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);

			/* get timelag value */
			timelag = 0.0;
			if (ntimedelay > 0)
			interp_status = mb_linear_interp(verbose, 
						timedelay_time_d-1, timedelay_timedelay-1,
						ntimedelay, time_d, &timelag, &j, 
						&error);
			if (timelagmode == MB7KPREPROCESS_TIMELAG_CONSTANT)
				{
				timelag -= timelagconstant;
				}
			else if (timelagmode == MB7KPREPROCESS_TIMELAG_MODEL && ntimelag > 0)
				{
				interp_status = mb_linear_interp(verbose, 
							timelag_time_d-1, timelag_model-1,
							ntimelag, time_d, &timelagm, &j, 
							&error);
				timelag -= timelagm;
				}
			time_d += timelag;
			mb_get_date(verbose, time_d,time_i);
			mb_get_jtime(verbose, time_i, time_j);
			header->s7kTime.Year = time_i[0];
			header->s7kTime.Day = time_j[1];
			header->s7kTime.Hours = time_i[3];
			header->s7kTime.Minutes = time_i[4];
			header->s7kTime.Seconds = time_i[5] + 0.000001 * time_i[6];

			if (verbose > 0)
				fprintf(stderr,"R7KRECID_RollPitchHeave:               7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
			time_i[0],time_i[1],time_i[2],
			time_i[3],time_i[4],time_i[5],time_i[6],
			header->RecordNumber);
			}
			
	   	/* handle heading data */
		else if (status == MB_SUCCESS && istore->type == R7KRECID_Heading) 
			{
			nrec_heading++;
			
			headingrec = &(istore->heading);
			header = &(headingrec->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);

			/* get timelag value */
			timelag = 0.0;
			if (ntimedelay > 0)
			interp_status = mb_linear_interp(verbose, 
						timedelay_time_d-1, timedelay_timedelay-1,
						ntimedelay, time_d, &timelag, &j, 
						&error);
			if (timelagmode == MB7KPREPROCESS_TIMELAG_CONSTANT)
				{
				timelag -= timelagconstant;
				}
			else if (timelagmode == MB7KPREPROCESS_TIMELAG_MODEL && ntimelag > 0)
				{
				interp_status = mb_linear_interp(verbose, 
							timelag_time_d-1, timelag_model-1,
							ntimelag, time_d, &timelagm, &j, 
							&error);
				timelag -= timelagm;
				}
			time_d += timelag;
			mb_get_date(verbose, time_d,time_i);
			mb_get_jtime(verbose, time_i, time_j);
			header->s7kTime.Year = time_i[0];
			header->s7kTime.Day = time_j[1];
			header->s7kTime.Hours = time_i[3];
			header->s7kTime.Minutes = time_i[4];
			header->s7kTime.Seconds = time_i[5] + 0.000001 * time_i[6];

			if (verbose > 0)
				fprintf(stderr,"R7KRECID_Heading: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
				time_i[0],time_i[1],time_i[2],
				time_i[3],time_i[4],time_i[5],time_i[6],
				header->RecordNumber);
			}

	   	/* handle survey line data */
		else if (status == MB_SUCCESS && istore->type == R7KRECID_SurveyLine) 
			{
			nrec_surveyline++;
			
			surveyline = &(istore->surveyline);
			header = &(surveyline->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);

			if (verbose > 0)
				fprintf(stderr,"R7KRECID_SurveyLine: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
			time_i[0],time_i[1],time_i[2],
			time_i[3],time_i[4],time_i[5],time_i[6],
			header->RecordNumber);
			}
			
	   	/* handle navigation data */
		else if (status == MB_SUCCESS && istore->type == R7KRECID_Navigation) 
			{
			nrec_navigation++;
			
			navigation = &(istore->navigation);
			header = &(navigation->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);

			/* get timelag value */
			timelag = 0.0;
			if (ntimedelay > 0)
			interp_status = mb_linear_interp(verbose, 
						timedelay_time_d-1, timedelay_timedelay-1,
						ntimedelay, time_d, &timelag, &j, 
						&error);
			if (timelagmode == MB7KPREPROCESS_TIMELAG_CONSTANT)
				{
				timelag -= timelagconstant;
				}
			else if (timelagmode == MB7KPREPROCESS_TIMELAG_MODEL && ntimelag > 0)
				{
				interp_status = mb_linear_interp(verbose, 
							timelag_time_d-1, timelag_model-1,
							ntimelag, time_d, &timelagm, &j, 
							&error);
				timelag -= timelagm;
				}
			time_d += timelag;
			mb_get_date(verbose, time_d,time_i);
			mb_get_jtime(verbose, time_i, time_j);
			header->s7kTime.Year = time_i[0];
			header->s7kTime.Day = time_j[1];
			header->s7kTime.Hours = time_i[3];
			header->s7kTime.Minutes = time_i[4];
			header->s7kTime.Seconds = time_i[5] + 0.000001 * time_i[6];

			if (verbose > 0)
				fprintf(stderr,"R7KRECID_Navigation: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
			time_i[0],time_i[1],time_i[2],
			time_i[3],time_i[4],time_i[5],time_i[6],
			header->RecordNumber);
			}

	   	/* handle attitude data */
		else if (status == MB_SUCCESS && istore->type == R7KRECID_Attitude) 
			{
			nrec_attitude++;
			
			attitude = &(istore->attitude);
			header = &(attitude->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);

			/* get timelag value */
			timelag = 0.0;
			if (ntimedelay > 0)
			interp_status = mb_linear_interp(verbose, 
						timedelay_time_d-1, timedelay_timedelay-1,
						ntimedelay, time_d, &timelag, &j, 
						&error);
			if (timelagmode == MB7KPREPROCESS_TIMELAG_CONSTANT)
				{
				timelag -= timelagconstant;
				}
			else if (timelagmode == MB7KPREPROCESS_TIMELAG_MODEL && ntimelag > 0)
				{
				interp_status = mb_linear_interp(verbose, 
							timelag_time_d-1, timelag_model-1,
							ntimelag, time_d, &timelagm, &j, 
							&error);
				timelag -= timelagm;
				}
			time_d += timelag;
			mb_get_date(verbose, time_d,time_i);
			mb_get_jtime(verbose, time_i, time_j);
			header->s7kTime.Year = time_i[0];
			header->s7kTime.Day = time_j[1];
			header->s7kTime.Hours = time_i[3];
			header->s7kTime.Minutes = time_i[4];
			header->s7kTime.Seconds = time_i[5] + 0.000001 * time_i[6];

			if (verbose > 0)
				fprintf(stderr,"R7KRECID_Attitude: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d n:%d\n",
			time_i[0],time_i[1],time_i[2],
			time_i[3],time_i[4],time_i[5],time_i[6],
			header->RecordNumber,attitude->n);
			}
			
	   	/* handle file header data */
		else if (status == MB_SUCCESS && istore->type == R7KRECID_7kFileHeader) 
			{
			nrec_fileheader++;
			
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
				fprintf(stderr,"R7KRECID_7kFileHeader: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
				time_i[0],time_i[1],time_i[2],
				time_i[3],time_i[4],time_i[5],time_i[6],
				header->RecordNumber);
			}
			
	   	/* handle bluefin ctd data */
		else if (status == MB_SUCCESS && istore->type == R7KRECID_Bluefin && kind == MB_DATA_SSV) 
			{
			nrec_bluefinenv++;
			
			bluefin = &(istore->bluefin);
			header = &(bluefin->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);
			last_bluefinenv_time_d = MAX(last_bluefinenv_time_d, time_d);
			if (last_bluefinenv_time_d > time_d)
				{
				status = MB_FAILURE;
				error = MB_ERROR_IGNORE;
				}

			/* get timelag value */
			timelag = 0.0;
			if (ntimedelay > 0)
			interp_status = mb_linear_interp(verbose, 
						timedelay_time_d-1, timedelay_timedelay-1,
						ntimedelay, time_d, &timelag, &j, 
						&error);
			if (timelagmode == MB7KPREPROCESS_TIMELAG_CONSTANT)
				{
				timelag -= timelagconstant;
				}
			else if (timelagmode == MB7KPREPROCESS_TIMELAG_MODEL && ntimelag > 0)
				{
				interp_status = mb_linear_interp(verbose, 
							timelag_time_d-1, timelag_model-1,
							ntimelag, time_d, &timelagm, &j, 
							&error);
				timelag -= timelagm;
				}
			time_d += timelag;
			mb_get_date(verbose, time_d,time_i);
			mb_get_jtime(verbose, time_i, time_j);
			header->s7kTime.Year = time_i[0];
			header->s7kTime.Day = time_j[1];
			header->s7kTime.Hours = time_i[3];
			header->s7kTime.Minutes = time_i[4];
			header->s7kTime.Seconds = time_i[5] + 0.000001 * time_i[6];

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
				mb_get_time(verbose, time_i, &time_d);
				time_d += timelag;
				bluefin->environmental[i].ctd_time = time_d;
				bluefin->environmental[i].temperature_time = time_d;
				mb_get_date(verbose, time_d,time_i);
				mb_get_jtime(verbose, time_i, time_j);
				bluefin->environmental[i].s7kTime.Year = time_i[0];
				bluefin->environmental[i].s7kTime.Day = time_j[1];
				bluefin->environmental[i].s7kTime.Hours = time_i[3];
				bluefin->environmental[i].s7kTime.Minutes = time_i[4];
				bluefin->environmental[i].s7kTime.Seconds = time_i[5] + 0.000001 * time_i[6];
				if (verbose > 0)
				fprintf(stderr,"                       %2.2d          7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) CTD_time:%f T_time:%f\n",
					i,time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],time_i[6],
					bluefin->environmental[i].ctd_time,
					bluefin->environmental[i].temperature_time);
				
				/* get nav */
				interp_status = MB_SUCCESS;
				j = 0;
				if (nins > 0)
					{
					interp_status = mb_linear_interp_degrees(verbose, 
								ins_time_d-1, ins_lon-1,
								nins, time_d, &navlon, &j, 
								&error);
					if (interp_status == MB_SUCCESS)
					interp_status = mb_linear_interp_degrees(verbose, 
								ins_time_d-1, ins_lat-1,
								nins, time_d, &navlat, &j, 
								&error);
					}
				else if (nrock > 0)
					{
					interp_status = mb_linear_interp_degrees(verbose, 
								rock_time_d-1, rock_lon-1,
								nrock, time_d, &navlon, &j, 
								&error);
					if (interp_status == MB_SUCCESS)
					interp_status = mb_linear_interp_degrees(verbose, 
								rock_time_d-1, rock_lat-1,
								nrock, time_d, &navlat, &j, 
								&error);
					}
				else if (ndsl > 0)
					{
					interp_status = mb_linear_interp_degrees(verbose, 
								dsl_time_d-1, dsl_lon-1,
								ndsl, time_d, &navlon, &j, 
								&error);
					if (interp_status == MB_SUCCESS)
					interp_status = mb_linear_interp_degrees(verbose, 
								dsl_time_d-1, dsl_lat-1,
								ndsl, time_d, &navlat, &j, 
								&error);
					}
				else if (ndat_nav > 0)
					{
					interp_status = mb_linear_interp_degrees(verbose, 
								dat_nav_time_d-1, dat_nav_lon-1,
								ndat_nav, time_d, &navlon, &j, 
								&error);
					if (interp_status == MB_SUCCESS)
					interp_status = mb_linear_interp_degrees(verbose, 
								dat_nav_time_d-1, dat_nav_lat-1,
								ndat_nav, time_d, &navlat, &j, 
								&error);
					}
				else
					{
					navlon = 0.0;
					navlat = 0.0;
					}
				
				/* get sonar depth */
				if (interp_status != MB_SUCCESS)
					{
					}
				else if (nsonardepth > 0)
					{
					if (interp_status == MB_SUCCESS)
					interp_status = mb_linear_interp(verbose, 
								sonardepth_time_d-1, sonardepth_sonardepth-1,
								nsonardepth, time_d, &sonardepth, &j, 
								&error);
					}
				else if (nins > 0)
					{
					interp_status = mb_linear_interp(verbose, 
								ins_time_d-1, ins_sonardepth-1,
								nins, time_d, &sonardepth, &j, 
								&error);
					}
				else if (nrock > 0)
					{
					interp_status = mb_linear_interp(verbose, 
								rock_time_d-1, rock_sonardepth-1,
								nrock, time_d, &sonardepth, &j, 
								&error);
					}
				else if (ndsl > 0)
					{
					interp_status = mb_linear_interp(verbose, 
								dsl_time_d-1, dsl_sonardepth-1,
								ndsl, time_d, &sonardepth, &j, 
								&error);
					}
				else if (ndat_sonardepth > 0)
					{
					sonardepthlag = 0.0;
					if (sonardepthlagfix == MB_YES && ndat_sonardepth > 1 
						&& sonardepthratemax > 0.0
						&& interp_status == MB_SUCCESS)
						{
						interp_status = mb_linear_interp(verbose, 
								dat_sonardepth_time_d-1, dat_sonardepth_sonardepthrate-1,
								ndat_sonardepth, time_d, &sonardepthrate, &j, 
								&error);
						sonardepthlag = sonardepthrate * sonardepthlagmax / sonardepthratemax;
						if (sonardepthrate >= sonardepthratemax)
							sonardepthlag = sonardepthlagmax;
						}
					if (interp_status == MB_SUCCESS)
					interp_status = mb_linear_interp(verbose, 
								dat_sonardepth_time_d-1, dat_sonardepth_sonardepth-1,
								ndat_sonardepth, time_d + sonardepthlag, &sonardepth, &j, 
								&error);
					}
				else if (ndat_rph > 0)
					{
					interp_status = mb_linear_interp(verbose, 
								dat_rph_time_d-1, dat_rph_heave-1,
								ndat_rph, time_d, &heave, &j, 
								&error);
					sonardepth = heave;
					}
				else
					{
					sonardepth = 0.0;
					}
				
				/* get altitude */
				if (interp_status != MB_SUCCESS)
					{
					}
				else if (nins > 0)
					{
					interp_status = mb_linear_interp(verbose, 
								ins_altitude_time_d-1, ins_altitude-1,
								nins_altitude, time_d, &altitude, &j, 
								&error);
					}
				else if (ndat_altitude > 0)
					{
					interp_status = mb_linear_interp(verbose, 
								dat_altitude_time_d-1, dat_altitude_altitude-1,
								ndat_altitude, time_d, &altitude, &j, 
								&error);
					}
				else
					{
					altitude = 0.0;
					}

				/* output ctd data to file */
				fprintf(tfp,"%.3f %11.6f %10.6f %.3f %.3f %.2f %.3f\n",
					time_d, navlon, navlat, sonardepth, altitude, 
					bluefin->environmental[i].temperature, 
					bluefin->environmental[i].conductivity);
				}
			}
			
	   	/* handle bluefin nav data */
		else if (status == MB_SUCCESS && istore->type == R7KRECID_Bluefin && kind == MB_DATA_NAV2) 
			{
			nrec_bluefinnav++;
			
			bluefin = &(istore->bluefin);
			header = &(bluefin->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);
			last_bluefinnav_time_d = MAX(last_bluefinnav_time_d, time_d);
			if (last_bluefinnav_time_d > time_d)
				{
				status = MB_FAILURE;
				error = MB_ERROR_IGNORE;
				}

			/* get timelag value */
			timelag = 0.0;
			if (ntimedelay > 0)
			interp_status = mb_linear_interp(verbose, 
						timedelay_time_d-1, timedelay_timedelay-1,
						ntimedelay, time_d, &timelag, &j, 
						&error);
			if (timelagmode == MB7KPREPROCESS_TIMELAG_CONSTANT)
				{
				timelag -= timelagconstant;
				}
			else if (timelagmode == MB7KPREPROCESS_TIMELAG_MODEL && ntimelag > 0)
				{
				interp_status = mb_linear_interp(verbose, 
							timelag_time_d-1, timelag_model-1,
							ntimelag, time_d, &timelagm, &j, 
							&error);
				timelag -= timelagm;
				}
			time_d += timelag;
			mb_get_date(verbose, time_d,time_i);
			mb_get_jtime(verbose, time_i, time_j);
			header->s7kTime.Year = time_i[0];
			header->s7kTime.Day = time_j[1];
			header->s7kTime.Hours = time_i[3];
			header->s7kTime.Minutes = time_i[4];
			header->s7kTime.Seconds = time_i[5] + 0.000001 * time_i[6];

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
				mb_get_time(verbose, time_i, &time_d);
				time_d += timelag;
				bluefin->nav[i].position_time += timelag;
				bluefin->nav[i].depth_time += timelag;
				mb_get_date(verbose, time_d,time_i);
				mb_get_jtime(verbose, time_i, time_j);
				bluefin->nav[i].s7kTime.Year = time_i[0];
				bluefin->nav[i].s7kTime.Day = time_j[1];
				bluefin->nav[i].s7kTime.Hours = time_i[3];
				bluefin->nav[i].s7kTime.Minutes = time_i[4];
				bluefin->nav[i].s7kTime.Seconds = time_i[5] + 0.000001 * time_i[6];
				if (verbose > 0)
				fprintf(stderr,"                       %2.2d          7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) Pos_time:%f\n",
					i,time_i[0],time_i[1],time_i[2],
					time_i[3],time_i[4],time_i[5],time_i[6],
					bluefin->nav[i].position_time);
					
				/* output asynchronous heading, sonardepth, and attitude */
				fprintf(athfp, "%0.6f\t%7.3f\n", time_d, RTD * bluefin->nav[i].yaw);
				sonardepth = bluefin->nav[i].depth 
						+ depthsensoroffx * sin(bluefin->nav[i].pitch)
						+ depthsensoroffz * cos(bluefin->nav[i].pitch)
						+ sonardepthoffset;
				fprintf(atsfp, "%0.6f\t%0.3f\n", time_d, sonardepth);
				fprintf(atafp, "%0.6f\t%0.3f\t%0.3f\n", 
						time_d, RTD * bluefin->nav[i].roll, RTD * bluefin->nav[i].pitch);
				}			
			}
			
	   	/* handle subbottom data */
		else if (status == MB_SUCCESS && kind == MB_DATA_SUBBOTTOM_SUBBOTTOM) 
			{
			nrec_fsdwsbp++;

			fsdwsb = &(istore->fsdwsb);
			header = &(fsdwsb->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);
			last_fsdwsbp_time_d = MAX(last_fsdwsbp_time_d, time_d);
			if (last_fsdwsbp_time_d > time_d)
				{
				status = MB_FAILURE;
				error = MB_ERROR_IGNORE;
				}
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
			nrec_fsdwsslo++;

			fsdwsslo = &(istore->fsdwsslo);
			header = &(fsdwsslo->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);
			last_fsdwsslo_time_d = MAX(last_fsdwsslo_time_d, time_d);
			if (last_fsdwsslo_time_d > time_d)
				{
				status = MB_FAILURE;
				error = MB_ERROR_IGNORE;
				}
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
			nrec_fsdwsshi++;

			fsdwsshi = &(istore->fsdwsshi);
			header = &(fsdwsshi->header);
			time_j[0] = header->s7kTime.Year;
			time_j[1] = header->s7kTime.Day;
			time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
			time_j[3] = (int) header->s7kTime.Seconds;
			time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
			mb_get_itime(verbose, time_j, time_i);
			mb_get_time(verbose, time_i, &time_d);
			last_fsdwsshi_time_d = MAX(last_fsdwsshi_time_d, time_d);
			if (last_fsdwsshi_time_d > time_d)
				{
				status = MB_FAILURE;
				error = MB_ERROR_IGNORE;
				}
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
			nrec_other++;
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
	  
	  	/* if using AUV ins data log for navigation and attitude, then
			output these data in new bluefin racords while not outputting
			any old bluefin records. */
		if (nins > 0 && error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA)
			{
			/* if first output find starting point in ins, attitude, and speed data */
			if (ins_output_index < 0)
				{
				for (i = 0; i < nins && ins_time_d[i] < time_d - 1; i++)
					{
					/*fprintf(stderr,"i:%d time: %f ins:%f\n",i,time_d,ins_time_d[i]);*/
					}
				ins_output_index = MAX(0,i-1);
				}
				
			/* output bluefin record with 25 samples if survey record has a time later than that
				of the last sample output */
			if (time_d > ins_time_d[ins_output_index])
				{
				bluefin = &(istore->bluefin);
				header = &(bluefin->header);
				type_save = istore->type;
				kind_save = istore->kind;
				istore->kind = MB_DATA_NAV2;
				istore->type = R7KRECID_Bluefin;
				bluefin->number_frames = MIN(25, nins - ins_output_index + 1);

				header->Version = 4;
				header->Offset = 60;
				header->SyncPattern = 65535;
				header->Size = 100 + 128 * bluefin->number_frames;
				header->OffsetToOptionalData = 0;
				header->OptionalDataIdentifier = 0;
				mb_get_jtime(verbose, istore->time_i, time_j);
				header->s7kTime.Year = istore->time_i[0];
				header->s7kTime.Day = time_j[1];
				header->s7kTime.Hours = istore->time_i[3];
				header->s7kTime.Minutes = istore->time_i[4];
				header->s7kTime.Seconds = istore->time_i[5] + 0.000001 * istore->time_i[6];
				header->Reserved = 0;
				header->RecordType = R7KRECID_Bluefin;
				header->DeviceId = R7KDEVID_Bluefin;
				header->Reserved2 = 0;
				header->SystemEnumerator = 0;
				header->DataSetNumber = 0;
				header->RecordNumber = 0;
				for (i=0;i<8;i++)
					{
					header->PreviousRecord[i] = 0;
					header->NextRecord[i] = 0;
					}
				header->Flags = 0;
				header->Reserved3 = 0;
				header->Reserved4 = 0;
				header->FragmentedTotal = 0;
				header->FragmentNumber = 0;
				
				bluefin->msec_timestamp = 0;
				/* bluefin->number_frames = MIN(25, nins - ins_output_index + 1); */
				bluefin->frame_size = 128;
				bluefin->data_format = R7KRECID_BluefinNav;
				for (i=0;i<16;i++)
					bluefin->reserved[i] = 0;
				if (verbose > 0)
					fprintf(stderr,"R7KRECID_BluefinNav:               7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
						time_i[0],time_i[1],time_i[2],
						time_i[3],time_i[4],time_i[5],time_i[6],
						header->RecordNumber);

				for (i=0;i<bluefin->number_frames;i++)
					{
					bluefin->nav[i].packet_size = 128;
					bluefin->nav[i].version = 2;
					bluefin->nav[i].offset = 32;
					bluefin->nav[i].data_type = 1;
					bluefin->nav[i].data_size = 96;
					mb_get_date(verbose, ins_time_d[ins_output_index], time_i);
					mb_get_jtime(verbose, time_i, time_j);
					bluefin->nav[i].s7kTime.Year = istore->time_i[0];
					bluefin->nav[i].s7kTime.Day = time_j[1];
					bluefin->nav[i].s7kTime.Hours = istore->time_i[3];
					bluefin->nav[i].s7kTime.Minutes = istore->time_i[4];
					bluefin->nav[i].s7kTime.Seconds = istore->time_i[5] + 0.000001 * istore->time_i[6];
					if (verbose > 0)
					fprintf(stderr,"                       %2.2d          7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) Pos_time:%f\n",
						i,time_i[0],time_i[1],time_i[2],
						time_i[3],time_i[4],time_i[5],time_i[6],
						bluefin->nav[i].position_time);
					bluefin->nav[i].checksum = 0;
					bluefin->nav[i].timedelay = 0;
					bluefin->nav[i].quality = 0;
					bluefin->nav[i].latitude = DTR * ins_lat[ins_output_index];
					bluefin->nav[i].longitude = DTR * ins_lon[ins_output_index];
					speed = bluefin->nav[i].speed;
					mb_linear_interp(verbose, 
								ins_speed_time_d-1, ins_speed-1,
								nins_speed, ins_time_d[ins_output_index], 
								&speed, &j, 
								&error);
					bluefin->nav[i].depth = ins_sonardepth[ins_output_index];
					mb_linear_interp(verbose, 
								ins_altitude_time_d-1, ins_altitude-1,
								nins_altitude, ins_time_d[ins_output_index], 
								&(bluefin->nav[i].altitude), &j, 
								&error);
					bluefin->nav[i].roll = DTR * ins_roll[ins_output_index];
					bluefin->nav[i].pitch = DTR * ins_pitch[ins_output_index];
					bluefin->nav[i].yaw = DTR * ins_heading[ins_output_index];
					bluefin->nav[i].northing_rate = 0;
					bluefin->nav[i].easting_rate = 0;
					bluefin->nav[i].depth_rate = 0;
					bluefin->nav[i].altitude_rate = 0;
					bluefin->nav[i].roll_rate = 0;
					bluefin->nav[i].pitch_rate = 0;
					bluefin->nav[i].yaw_rate = 0;
					bluefin->nav[i].position_time = ins_time_d[ins_output_index];
					bluefin->nav[i].depth_time = ins_time_d[ins_output_index];
					ins_output_index++;
					}

				/* write the new bluefin record */
				status = mb_put_all(verbose,ombio_ptr,
						istore_ptr,MB_NO,MB_DATA_NAV2,
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
			
				/* restore kind and time_i */
				istore->type = type_save;
				istore->kind = kind_save;
				mb_get_date(verbose, time_d, time_i);
				}
			}
			
		/* do not output full beam data */
		if (error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA
			&& istore->read_beam == MB_YES)
			istore->read_beam = MB_NO;

		/* write some data */
		if (error == MB_ERROR_NO_ERROR && (nins < 1 || kind != MB_DATA_NAV2))
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
	fclose(tfp);
	fclose(athfp);
	fclose(atsfp);
	fclose(atafp);
	fclose(stafp);
	
	/* output counts */
	fprintf(stdout, "\nData records written to: %s\n", ofile);
	fprintf(stdout, "     File Header:                       %d\n", nrec_fileheader);
	fprintf(stdout, "     Multibeam:                         %d\n", nrec_multibeam);
	fprintf(stdout, "          Volatile Settings:                 %d\n", nrec_volatilesettings);
	fprintf(stdout, "          Match Filter:                      %d\n", nrec_matchfilter);
	fprintf(stdout, "          Beam Geometry:                     %d\n", nrec_beamgeometry);
	fprintf(stdout, "          Remote Control:                    %d\n", nrec_remotecontrolsettings);
	fprintf(stdout, "          Bathymetry:                        %d\n", nrec_bathymetry);
	fprintf(stdout, "          Backscatter:                       %d\n", nrec_backscatter);
	fprintf(stdout, "          Beam:                              %d\n", nrec_beam);
	fprintf(stdout, "          Image:                             %d\n", nrec_image);
	fprintf(stdout, "     Reference:                         %d\n", nrec_reference);
	fprintf(stdout, "     Uncalibrated Sensor Offset:        %d\n", nrec_sensoruncal);
	fprintf(stdout, "     Calibrated Sensor Offset:          %d\n", nrec_sensorcal);
	fprintf(stdout, "     Position:                          %d\n", nrec_position);
	fprintf(stdout, "     Custom Attitude:                   %d\n", nrec_customattitude);
	fprintf(stdout, "     Tide:                              %d\n", nrec_tide);
	fprintf(stdout, "     Altitude:                          %d\n", nrec_altitude);
	fprintf(stdout, "     Motion Over Ground:                %d\n", nrec_motion);
	fprintf(stdout, "     Depth:                             %d\n", nrec_depth);
	fprintf(stdout, "     Sound Speed Profile:               %d\n", nrec_svp);
	fprintf(stdout, "     CTD:                               %d\n", nrec_ctd);
	fprintf(stdout, "     Geodosy:                           %d\n", nrec_geodesy);
	fprintf(stdout, "     Roll Pitch Heave:                  %d\n", nrec_rollpitchheave);
	fprintf(stdout, "     Heading:                           %d\n", nrec_heading);
	fprintf(stdout, "     Survey Line:                       %d\n", nrec_surveyline);
	fprintf(stdout, "     Navigation:                        %d\n", nrec_navigation);
	fprintf(stdout, "     Attitude:                          %d\n", nrec_attitude);
	fprintf(stdout, "     Edgetech Low Frequency Sidescan:   %d\n", nrec_fsdwsslo);
	fprintf(stdout, "     Edgetech High Frequency Sidescan:  %d\n", nrec_fsdwsshi);
	fprintf(stdout, "     Edgetech Subbottom:                %d\n", nrec_fsdwsbp);
	fprintf(stdout, "     MBARI Mapping AUV Environmental:   %d\n", nrec_bluefinnav);
	fprintf(stdout, "     MBARI Mapping AUV Navigation:      %d\n", nrec_bluefinenv);
	fprintf(stdout, "     Configuration:                     %d\n", nrec_configuration);
	fprintf(stdout, "     Calibration:                       %d\n", nrec_calibration);
	fprintf(stdout, "     Vertical Depth:                    %d\n", nrec_verticaldepth);
	fprintf(stdout, "     Installation:                      %d\n", nrec_installation);
	fprintf(stdout, "     System Event Message:              %d\n", nrec_systemeventmessage);
	fprintf(stdout, "     Other:                             %d\n", nrec_other);
	nrec_fileheader_tot += nrec_fileheader;
	nrec_multibeam_tot += nrec_multibeam;
	nrec_volatilesettings_tot += nrec_volatilesettings;
	nrec_matchfilter_tot += nrec_matchfilter;
	nrec_beamgeometry_tot += nrec_beamgeometry;
	nrec_remotecontrolsettings_tot += nrec_remotecontrolsettings;
	nrec_bathymetry_tot += nrec_bathymetry;
	nrec_backscatter_tot += nrec_backscatter;
	nrec_beam_tot += nrec_beam;
	nrec_image_tot += nrec_image;	
	nrec_reference_tot += nrec_reference;
	nrec_sensoruncal_tot += nrec_sensoruncal;
	nrec_sensorcal_tot += nrec_sensorcal;
	nrec_position_tot += nrec_position;
	nrec_customattitude_tot += nrec_customattitude;
	nrec_tide_tot += nrec_tide;
	nrec_altitude_tot += nrec_altitude;
	nrec_motion_tot += nrec_motion;
	nrec_depth_tot += nrec_depth;
	nrec_svp_tot += nrec_svp;
	nrec_ctd_tot += nrec_ctd;
	nrec_geodesy_tot += nrec_geodesy;
	nrec_rollpitchheave_tot += nrec_rollpitchheave;
	nrec_heading_tot += nrec_heading;
	nrec_surveyline_tot += nrec_surveyline;
	nrec_navigation_tot += nrec_navigation;
	nrec_attitude_tot += nrec_attitude;
	nrec_fsdwsbp_tot += nrec_fsdwsbp;
	nrec_fsdwsslo_tot += nrec_fsdwsslo;
	nrec_fsdwsshi_tot += nrec_fsdwsshi;
	nrec_bluefinenv_tot += nrec_bluefinenv;
	nrec_position_tot += nrec_position;
	nrec_bluefinnav_tot += nrec_bluefinnav;
	nrec_configuration_tot += nrec_configuration;
	nrec_calibration_tot += nrec_calibration;
	nrec_verticaldepth_tot += nrec_verticaldepth;
	nrec_installation_tot += nrec_installation;
	nrec_systemeventmessage_tot += nrec_systemeventmessage;
	nrec_other_tot += nrec_other;
		
	/* generate inf fnv and fbt files */
	if (status == MB_SUCCESS)
		{
		status = mb_make_info(verbose, MB_YES, ofile, format, &error);
		}

	/* figure out whether and what to read next */
        if (read_datalist == MB_YES)
                {
		if ((status = mb_datalist_read(verbose,datalist,
			    ifile,&format,&file_weight,&error))
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
	fprintf(stdout, "\nTotal data records written from: %s\n", read_file);
	fprintf(stdout, "     File Header:                       %d\n", nrec_fileheader_tot);
	fprintf(stdout, "     Multibeam:                         %d\n", nrec_multibeam_tot);
	fprintf(stdout, "          Volatile Settings:                 %d\n", nrec_volatilesettings_tot);
	fprintf(stdout, "          Match Filter:                      %d\n", nrec_matchfilter_tot);
	fprintf(stdout, "          Beam Geometry:                     %d\n", nrec_beamgeometry_tot);
	fprintf(stdout, "          Remote Control:                    %d\n", nrec_remotecontrolsettings_tot);
	fprintf(stdout, "          Bathymetry:                        %d\n", nrec_bathymetry_tot);
	fprintf(stdout, "          Backscatter:                       %d\n", nrec_backscatter_tot);
	fprintf(stdout, "          Beam:                              %d\n", nrec_beam_tot);
	fprintf(stdout, "          Image:                             %d\n", nrec_image_tot);
	fprintf(stdout, "     Reference:                         %d\n", nrec_reference_tot);
	fprintf(stdout, "     Uncalibrated Sensor Offset:        %d\n", nrec_sensoruncal_tot);
	fprintf(stdout, "     Calibrated Sensor Offset:          %d\n", nrec_sensorcal_tot);
	fprintf(stdout, "     Position:                          %d\n", nrec_position_tot);
	fprintf(stdout, "     Custom Attitude:                   %d\n", nrec_customattitude_tot);
	fprintf(stdout, "     Tide:                              %d\n", nrec_tide_tot);
	fprintf(stdout, "     Altitude:                          %d\n", nrec_altitude_tot);
	fprintf(stdout, "     Motion Over Ground:                %d\n", nrec_motion_tot);
	fprintf(stdout, "     Depth:                             %d\n", nrec_depth_tot);
	fprintf(stdout, "     Sound Speed Profile:               %d\n", nrec_svp_tot);
	fprintf(stdout, "     CTD:                               %d\n", nrec_ctd_tot);
	fprintf(stdout, "     Geodosy:                           %d\n", nrec_geodesy_tot);
	fprintf(stdout, "     Roll Pitch Heave:                  %d\n", nrec_rollpitchheave_tot);
	fprintf(stdout, "     Heading:                           %d\n", nrec_heading_tot);
	fprintf(stdout, "     Survey Line:                       %d\n", nrec_surveyline_tot);
	fprintf(stdout, "     Navigation:                        %d\n", nrec_navigation_tot);
	fprintf(stdout, "     Attitude:                          %d\n", nrec_attitude_tot);
	fprintf(stdout, "     Edgetech Low Frequency Sidescan:   %d\n", nrec_fsdwsslo_tot);
	fprintf(stdout, "     Edgetech High Frequency Sidescan:  %d\n", nrec_fsdwsshi_tot);
	fprintf(stdout, "     Edgetech Subbottom:                %d\n", nrec_fsdwsbp_tot);
	fprintf(stdout, "     MBARI Mapping AUV Environmental:   %d\n", nrec_bluefinnav_tot);
	fprintf(stdout, "     MBARI Mapping AUV Navigation:      %d\n", nrec_bluefinenv_tot);
	fprintf(stdout, "     Configuration:                     %d\n", nrec_configuration_tot);
	fprintf(stdout, "     Calibration:                       %d\n", nrec_calibration_tot);
	fprintf(stdout, "     Vertical Depth:                    %d\n", nrec_verticaldepth_tot);
	fprintf(stdout, "     Installation:                      %d\n", nrec_installation_tot);
	fprintf(stdout, "     System Event Message:              %d\n", nrec_systemeventmessage_tot);
	fprintf(stdout, "     Other:                             %d\n", nrec_other_tot);
	}
		
	
	/* deallocate navigation arrays */
	if (ndat_nav > 0)
		{
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&dat_nav_time_d,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&dat_nav_time_d,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&dat_nav_lon,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&dat_nav_lat,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&dat_nav_speed,&error);
		}
	if (ndat_sonardepth > 0)
		{
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&dat_sonardepth_sonardepth,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&dat_sonardepth_sonardepthrate,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&dat_sonardepth_sonardepthfilter,&error);
		}
	if (ndat_heading > 0)
		{
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&dat_heading_heading,&error);
		}
	if (ndat_rph > 0)
		{
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&dat_rph_roll,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&dat_rph_pitch,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&dat_rph_heave,&error);
		}
	if (ndat_altitude > 0)
		{
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&dat_altitude_time_d,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&dat_altitude_altitude,&error);
		}
	if (ntimedelay > 0)
		{
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&timedelay_time_d,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&timedelay_timedelay,&error);
		}
	if (nbatht > 0)
		{
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&batht_time_d,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&batht_ping,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&batht_time_d_new,&error);
		}
	if (nedget > 0)
		{
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&edget_time_d,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&edget_ping,&error);
		}
	if (nins > 0)
		{
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&ins_time_d,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&ins_lon,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&ins_lat,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&ins_heading,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&ins_roll,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&ins_pitch,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&ins_sonardepth,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&ins_sonardepthfilter,&error);
		}
	if (nrock > 0)
		{
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&rock_time_d,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&rock_lon,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&rock_lat,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&rock_sonardepth,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&rock_sonardepthfilter,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&rock_heading,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&rock_roll,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&rock_pitch,&error);
		}
	if (ndsl > 0)
		{
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&dsl_time_d,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&dsl_lon,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&dsl_lat,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&dsl_sonardepth,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&dsl_sonardepthfilter,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&dsl_heading,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&dsl_roll,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&dsl_pitch,&error);
		}
	if (nins_altitude > 0)
		{
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&ins_altitude_time_d,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&ins_altitude,&error);
		}
	if (nins_speed > 0)
		{
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&ins_speed_time_d,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&ins_speed,&error);
		}
	if (nsonardepth > 0)
		{
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&sonardepth_time_d,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&sonardepth_sonardepth,&error);
		status = mb_freed(verbose,__FILE__,__LINE__,(void **)&sonardepth_sonardepthfilter,&error);
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
