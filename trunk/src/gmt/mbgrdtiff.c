/*--------------------------------------------------------------------
 *    The MB-system:	mbgrdtiff.c	5/30/93
 *    $Id: mbgrdtiff.c,v 4.1 2000-09-30 06:52:17 caress Exp $
 *
 *    Copyright (c) 1999, 2000 by
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
 *    mbgrdtiff generates a TIFF image from a GMT grid. The 
 *    image generation is similar to that of the GMT program 
 *    grdimage. In particular, the color map is applied from 
 *    a GMT CPT file, and shading overlay grids may be applied. 
 *    The output TIFF file contains information allowing
 *    the ArcView and ArcInfo GIS packages to import the image
 *    as a geographically located coverage. The image is 8 bits
 *    per pixel if the color map is a grayscale, and 24 bits
 *    per pixel otherwise.
 *
 *
 * The geotiff file variant produced by mbgrdtiff
 * has the structure given below. The image width in
 * pixels is denoted as nx and the image height as ny.
 * The images may be grayscale (1 byte per pixel) or
 * color (3 rgb bytes per pixel).
 *
 *    Byte  Size  Value      Meaning
 *    -----------------------------------------------------------------
 *
 *           ------- Header --------
 *
 *      0    2     'MM'      Big-endian byte order used in this file
 *      2    2      42       Tiff identifier
 *      4    4      8        Byte offset of image file directory (IFD)
 *
 *           ------- IFD -----------
 *
 *      8    2      14       Number of entries in IFD
 *     
 *     10    2      254      Tag:   NewSubfileType
 *     12    2      4        Type:  long (4 byte unsigned int)
 *     14    4      1        Count: one value
 *     18    4      0        Value: basic image, nothing fancy
 *     
 *     22    2      256      Tag:   ImageWidth
 *     24    2      4        Type:  long (4 byte unsigned int)
 *     26    4      1        Count: one value
 *     30    4      nx       Value: image width in pixels
 *     
 *     34    2      257      Tag:   ImageLength
 *     36    2      4        Type:  long (4 byte unsigned int)
 *     38    4      1        Count: one value
 *     42    4      ny       Value: image length (height) in pixels
 *     
 *     46    2      258      Tag:   BitsPerSample
 *     48    2      3        Type:  short (2 byte unsigned int)
 *                           Color case:
 *     50    4      3          Count: one value for each of r,g,b (8,8,8)
 *     54    4      256        Value: offset to three BitsPerSample values
 *                           Grayscale case:
 *     50    4      1          Count: one value for grayscale
 *     54    4      8          Value: one byte per sample
 *     
 *     58    2      259      Tag:   Compression
 *     60    2      3        Type:  short (2 byte unsigned int)
 *     62    4      1        Count: one value
 *     66    4      1        Value: no compression
 *     
 *     70    2      262      Tag:   PhotometricInterpretation
 *     72    2      3        Type:  short (2 byte unsigned int)
 *     74    4      1        Count: one value
 *     78    4      2        Value: RGB image
 *     
 *     82    2      273      Tag:   StripOffsets
 *     84    2      4        Type:  long (4 byte unsigned int)
 *     86    4      1        Count: entire image is one strip
 *     90    4               Value: offset to image data
 *     
 *     94    2      277      Tag:   SamplesPerPixel
 *     96    2      3        Type:  short (2 byte unsigned int)
 *     98    4      1        Count: one value
 *    102    4      3        Value: 3 samples per pixel for RGB
 *     
 *    106    2      278      Tag:   RowsPerStrip
 *    108    2      4        Type:  long (4 byte unsigned int)
 *    110    4      1        Count: one value
 *    114    4      ny       Value: ImageLength value (all rows in image)
 *     
 *    118    2      279      Tag:   StripByteCounts
 *    120    2      4        Type:  long (4 byte unsigned int)
 *    122    4      1        Count: one value
 *                           Color case:
 *    126    4      3*nx*ny    Value: 3 * ImageWidth * ImageLength
 *                           Grayscale case:
 *    126    4      nx*ny      Value: ImageWidth * ImageLength
 *     
 *    130    2      282      Tag:   XResolution
 *    132    2      5        Type:  rational (2 ints: numerator, denominator)
 *    134    4      1        Count: one value
 *    138    4               Value: offset to fraction representing dpi
 *     
 *    142    2      283      Tag:   YResolution
 *    144    2      4        Type:  rational (2 ints: numerator, denominator)
 *    146    4      1        Count: one value
 *    150    4               Value: offset to fraction representing dpi
 *     
 *    154    2      296      Tag:   ResolutionUnit
 *    156    2      3        Type:  short (2 byte unsigned int)
 *    158    4      1        Count: one value
 *    162    4      2        Value: Inches
 *     
 *    166    2      33550    Tag:   ModelPixelScaleTag
 *    168    2      12       Type:  double (IEEE double precision)
 *    170    4      3        Count: 3 for scalex,scaley,scalez where scalez=0
 *    174    4               Value: offset to values
 *     
 *    178    2      33922    Tag:   ModelTiepointTag
 *    180    2      12       Type:  double (IEEE double precision)
 *    182    4      6        Count: 6 for i,j,k,x,y,z where k=z=0
 *    186    4               Value: offset to values
 *
 *           ------- Values --------
 *
 *    256    2      8        BitsPerSample R (Color case only)
 *    258    2      8        BitsPerSample G (Color case only)
 *    260    2      8        BitsPerSample B (Color case only)
 *
 *    264    4      nx       XResolution numerator (ImageWidth)
 *    268    4      4        XResolution denominator (4 inches)
 *
 *    272    4      nx       YResolution numerator (ImageWidth)
 *    276    4      4        YResolution denominator (4 inches)
 *
 *    280    8      scalex   ModelPixelScaleTag scalex
 *    288    8      scaley   ModelPixelScaleTag scaley
 *    296    8      scalez   ModelPixelScaleTag scalez
 *
 *    304    8      0        ModelTiePointTag i
 *    312    8      0        ModelTiePointTag j
 *    320    8      0        ModelTiePointTag k
 *    328    8      minlon   ModelTiePointTag minimum longitude
 *    336    8      maxlat   ModelTiePointTag minimum latitude
 *    344    8      0        ModelTiePointTag minimum z
 *
 *           ------- Image ---------
 *
 *    512    3*nx*ny         Image in RGB bytes      
 *
 *
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.0  2000/03/06  21:59:34  caress
 * Initial version.
 *
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

/* MBIO include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_define.h"

/* GMT include files */
#include "gmt.h"

/* TIFF 6.0 and geoTIFF tag array */
#define HEADER_SIZE 1024
#define NUMBER_TAGS 15
#define IMAGE_OFFSET HEADER_SIZE
#define NewSubfileType             254
#define ImageWidth                 256
#define ImageLength                257
#define BitsPerSample              258
#define Compression                259
#define PhotometricInterpretation  262
#define StripOffsets               273
#define SamplesPerPixel            277
#define RowsPerStrip               278
#define StripByteCounts            279
#define XResolution                282
#define YResolution                283
#define ResolutionUnit             296
#define ModelPixelScaleTag       33550
#define ModelTiepointTag         33922
unsigned short   tiff_tag[] = 
                      { NewSubfileType,
			ImageWidth,
			ImageLength,
			BitsPerSample,
			Compression,
			PhotometricInterpretation,
			StripOffsets,
			SamplesPerPixel,
			RowsPerStrip,
			StripByteCounts,
			XResolution,
			YResolution,
			ResolutionUnit,
		        ModelPixelScaleTag,
		        ModelTiepointTag
		      };
unsigned short   tiff_type[] = 
                      {   4,      /* NewSubfileType */
			  4,      /* ImageWidth */
			  4,      /* ImageLength */
			  3,      /* BitsPerSample */
			  3,      /* Compression */
			  3,      /* PhotometricInterpretation */
			  4,      /* StripOffsets */
			  3,      /* SamplesPerPixel */
			  4,      /* RowsPerStrip */
			  4,      /* StripByteCounts */
			  5,      /* XResolution */
			  5,      /* YResolution */
			  3,      /* ResolutionUnit */
			 12,      /* ModelPixelScaleTag */
			 12       /* ModelTiepointTag */
		      };
int              tiff_offset[] = 
                      {   0,      /* NewSubfileType */
			  0,      /* ImageWidth */
			  0,      /* ImageLength */
			256,      /* BitsPerSample */
			  0,      /* Compression */
			  0,      /* PhotometricInterpretation */
			  0,      /* StripOffsets */
			  0,      /* SamplesPerPixel */
			  0,      /* RowsPerStrip */
			  0,      /* StripByteCounts */
			264,      /* XResolution */
			272,      /* YResolution */
			  0,      /* ResolutionUnit */
			280,      /* ModelPixelScaleTag */
			304       /* ModelTiepointTag */
		      };

/* global image variables and defines */
#ifndef YIQ
#ifdef GMT3_0
#define YIQ(r,g,b)	rint(0.299*(r) + 0.587*(g) + 0.114*(b))
#else
#define  YIQ(rgb)
#endif
#endif

/*--------------------------------------------------------------------*/

main (argc, argv)
int argc;
char **argv; 
{
	static char rcs_id[] = "$Id: mbgrdtiff.c,v 4.1 2000-09-30 06:52:17 caress Exp $";
	static char program_name[] = "mbgrdtiff";
	static char help_message[] = "mbgrdtiff generates a tiff image from a GMT grid. The \nimage generation is similar to that of the GMT program \ngrdimage. In particular, the color map is applied from \na GMT CPT file, and shading overlay grids may be applied. \nThe output TIFF file contains information allowing\nthe ArcView and ArcInfo GIS packages to import the image\nas a geographically located coverage.";
	static char usage_message[] = "mbgrdtiff -Ccptfile -Igrdfile -Otiff_file [-H -Kintensfile -V]";
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
	char	*message = NULL;

	/* control parameters */
	char    grdfile[128];
	char    cptfile[128];
        char    intensfile[128];
        char    tiff_file[128];
	int	intensity;
        double  bounds[4];
	int	pad[4];
        int     nx,ny,nxy;
        float   *grid = NULL;
        float   *igrid = NULL;
	struct GRD_HEADER header;
	struct GRD_HEADER iheader;

	/* TIFF arrays */
	int     image_size = 0;
	char    tiff_header[HEADER_SIZE];
	char    *tiff_image = NULL;
	
	/* other variables */
	FILE    *tfp;
	mb_u_char r, g, b;
	int     rgb[3];
	int	i, j, k, kk;
        int     index;
        short   value_short;
        int     value_int;
        float   value_float;
	double  value_double;

	/* initialize some values */
	strcpy (grdfile,"\0");
	strcpy (intensfile,"\0");
	strcpy (cptfile,"\0");
	strcpy (tiff_file,"\0");
	bounds[0] = 0.0;
	bounds[1] = 0.0;
	bounds[2] = 0.0;
	bounds[3] = 0.0;
	intensity = MB_NO;
	
	argc = GMT_begin (argc, argv);
	GMT_grd_init (&header, argc, argv, FALSE);
	GMT_grd_init (&iheader, argc, argv, FALSE);

	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhC:c:I:i:K:k:O:o:R:r:")) != -1)
	  switch (c) 
		{
		case 'C':
		case 'c':
			sscanf (optarg,"%s", cptfile);
			flag++;
			break;
		case 'H':
		case 'h':
			help++;
			break;
		case 'I':
		case 'i':
			sscanf (optarg,"%s", grdfile);
			flag++;
			break;
		case 'K':
		case 'k':
			intensity = MB_YES;
			sscanf (optarg,"%s", intensfile);
			flag++;
			break;
		case 'O':
		case 'o':
			sscanf (optarg,"%s", tiff_file);
			flag++;
			break;
		case 'R':
		case 'r':
			sscanf (optarg,"%lf/%lf/%lf/%lf", 
				&bounds[0],&bounds[1],&bounds[2],&bounds[3]);
			flag++;
			break;
		case 'V':
		case 'v':
			verbose++;
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
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       help:       %d\n",help);
		fprintf(stderr,"dbg2       cptfile:    %s\n",cptfile);
		fprintf(stderr,"dbg2       grdfile:    %s\n",grdfile);
		fprintf(stderr,"dbg2       intensfile: %s\n",intensfile);
		fprintf(stderr,"dbg2       tiff_file:  %s\n",tiff_file);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}

	/* get color palette file */
#ifdef GMT3_0
	read_cpt(cptfile);
	if (gmt_n_colors <= 0)
#else
	GMT_read_cpt(cptfile);
	if (GMT_n_colors <= 0)
#endif
	  {
	    fprintf(stderr,"\nColor pallette table not properly specified:\n");
	    fprintf(stderr,"\nProgram <%s> Terminated\n",
		    program_name);
	    error = MB_ERROR_BAD_PARAMETER;
	    exit(error);
	  }
	
	/* read input grd file header */
#ifdef GMT3_0
	if (read_grd_info (grdfile, &header)) 
#else
	if (GMT_read_grd_info (grdfile, &header)) 
#endif
	    {
	    error = MB_ERROR_OPEN_FAIL;
	    fprintf(stderr,"\nUnable to open grd file: %s\n",
		    grdfile);
	    fprintf(stderr,"\nProgram <%s> Terminated\n",
		    program_name);
	    exit(error);
	    }
	if (intensity == MB_YES)
	    {
#ifdef GMT3_0
	    if (read_grd_info (intensfile, &iheader)) 
#else
	    if (GMT_read_grd_info (intensfile, &iheader)) 
#endif
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr,"\nUnable to open intensity grd file: %s\n",
			intensfile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	    if (header.x_min != iheader.x_min
		|| header.x_max != iheader.x_max
		|| header.y_min != iheader.y_min
		|| header.y_max != iheader.y_max
		|| header.x_inc != iheader.x_inc
		|| header.y_inc != iheader.y_inc
		|| header.nx != iheader.nx
		|| header.ny != iheader.ny)
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr,"\nIntensity grd file %s header does not match grd file %s header\n",
			grdfile, intensfile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);		    
		}
	    }

	/* allocate memory */
	nx = header.nx;
	ny = header.ny;
	nxy = nx * ny;
#ifdef GMT3_0
	if (gmt_gray)
#else
	if (GMT_gray)
#endif
	  image_size = nx * ny;
	else
	  image_size = 3 * nx * ny;
	status = mb_malloc(verbose, nxy * sizeof(float), &grid, &error);
	if (intensity == MB_YES)
	    status = mb_malloc(verbose, nxy * sizeof(float), &igrid, &error);
	status = mb_malloc(verbose, image_size, &tiff_image, &error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR)
		{
		mb_error(verbose,error,&message);
		fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}

	/* read the grid */
	pad[0] = 0;
	pad[1] = 0;
	pad[2] = 0;
	pad[3] = 0;
#ifdef GMT3_0
	if (read_grd (grdfile, &header, grid, 
			    bounds[0],  bounds[1],  bounds[2],  bounds[3], 
			    pad, FALSE))
#else
	if (GMT_read_grd (grdfile, &header, grid, 
			    bounds[0],  bounds[1],  bounds[2],  bounds[3], 
			    pad, FALSE))
#endif
	    {
	    error = MB_ERROR_OPEN_FAIL;
	    fprintf(stderr,"\nUnable to read grd file: %s\n",
		    grdfile);
	    fprintf(stderr,"\nProgram <%s> Terminated\n",
		    program_name);
	    exit(error);
	    }
	if (intensity == MB_YES)
	    {
#ifdef GMT3_0
	    if (read_grd (intensfile, &iheader, igrid, 
				bounds[0],  bounds[1],  bounds[2],  bounds[3], 
				pad, FALSE))
#else
	    if (GMT_read_grd (intensfile, &iheader, igrid, 
				bounds[0],  bounds[1],  bounds[2],  bounds[3], 
				pad, FALSE))
#endif
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr,"\nUnable to read intensity grd file: %s\n",
			intensfile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	    }

	/* print debug info */
	if (verbose >= 0)
	    {
	    fprintf(stderr,"Grid read:\n");
	    fprintf(stderr,"  Dimensions: %d %d\n", header.nx, header.ny);
	    fprintf(stderr,"  Longitude:  %f %f  %f\n",
		  header.x_min, header.x_max, header.x_inc);
	    fprintf(stderr,"  Latitude:   %f %f  %f\n",
		  header.y_min, header.y_max, header.y_inc);
#ifdef GMT3_0
	    if (gmt_gray)
#else
	    if (GMT_gray)
#endif
		fprintf(stderr,"Writing 8 bit grayscale TIFF image\n");
	    else
		fprintf(stderr,"Writing 24 bit color TIFF image\n");
	    }

	/* open TIFF file */
	if ((tfp = fopen(tiff_file,"w")) == NULL)
	    {
	    error = MB_ERROR_OPEN_FAIL;
	    fprintf(stderr,"\nUnable to open output tiff file: %s\n",
		  tiff_file);
	    fprintf(stderr,"\nProgram <%s> Terminated\n",
		  program_name);
	    exit(error);
	    }

	/* set the TIFF header */
	memset(tiff_header,0,HEADER_SIZE);
        index = 0;
	tiff_header[0] = 'M';
	tiff_header[1] = 'M';
	index += 2;
	value_short = 42;
	mb_put_binary_short(MB_NO, value_short, &tiff_header[index]);
	index += 2;
	value_int = 8;
	mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
	index += 4;

	/* number of entries in IFD */
	value_short = NUMBER_TAGS;
	mb_put_binary_short(MB_NO, value_short, &tiff_header[index]);
	index += 2;

	/* loop over all tags */
	for (i=0;i<NUMBER_TAGS;i++)
	  {
	    mb_put_binary_short(MB_NO, tiff_tag[i], &tiff_header[index]);
	    index += 2;
	    mb_put_binary_short(MB_NO, tiff_type[i], &tiff_header[index]);
	    index += 2;

	    switch (tiff_tag[i])
	      {
	      case NewSubfileType:
		value_int = 1;
		mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
		index += 4;
		value_int = 0;
	        mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
		index += 4;
		break;
	      case ImageWidth:
		value_int = 1;
		mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
		index += 4;
		value_int = nx;
	        mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
		index += 4;
		break;
	      case ImageLength:
		value_int = 1;
		mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
		index += 4;
		value_int = ny;
	        mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
		index += 4;
		break;
	      case BitsPerSample:
#ifdef GMT3_0
		if (gmt_gray)
#else
		if (GMT_gray)
#endif
		  {
		    value_int = 1;
		    mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
		    index += 4;
		    value_short = 8;
		    mb_put_binary_short(MB_NO, value_short, &tiff_header[index]);
		    index += 4;
		  }
		else
		  {
		    value_int = 3;
		    mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
		    index += 4;
		    value_int = tiff_offset[i];
		    mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
		    index += 4;
		    value_short = 8;
		    mb_put_binary_short(MB_NO, value_short, &tiff_header[tiff_offset[i]]);
		    mb_put_binary_short(MB_NO, value_short, &tiff_header[tiff_offset[i]+2]);
		    mb_put_binary_short(MB_NO, value_short, &tiff_header[tiff_offset[i]+4]);
		  }
		break;
	      case Compression:
		value_int = 1;
		mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
		index += 4;
		value_short = 1;
	        mb_put_binary_short(MB_NO, value_short, &tiff_header[index]);
		index += 4;
		break;
	      case PhotometricInterpretation:
		value_int = 1;
		mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
		index += 4;
#ifdef GMT3_0
		if (gmt_gray)
#else
		if (GMT_gray)
#endif
		  {
		  value_short = 1;
		  }
		else
		  {
		  value_short = 2;
		  }
	        mb_put_binary_short(MB_NO, value_short, &tiff_header[index]);
		index += 4;
		break;
	      case StripOffsets:
		value_int = 1;
		mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
		index += 4;
		value_int = IMAGE_OFFSET;
	        mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
		index += 4;
		break;
	      case SamplesPerPixel:
		value_int = 1;
		mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
		index += 4;
#ifdef GMT3_0
		if (gmt_gray)
#else
		if (GMT_gray)
#endif
		  value_short = 1;
		else
		  value_short = 3;
	        mb_put_binary_short(MB_NO, value_short, &tiff_header[index]);
		index += 4;
		break;
	      case RowsPerStrip:
		value_int = 1;
		mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
		index += 4;
		value_int = ny;
	        mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
		index += 4;
		break;
	      case StripByteCounts:
		value_int = 1;
		mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
		index += 4;
#ifdef GMT3_0
		if (gmt_gray)
#else
		if (GMT_gray)
#endif
		  value_int = nx * ny;
		else
		  value_int = 3 * nx * ny;
	        mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
		index += 4;
		break;
	      case XResolution:
		value_int = 1;
		mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
		index += 4;
		value_int = tiff_offset[i];
	        mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
		index += 4;
		value_int = MAX(nx, ny);
		mb_put_binary_int(MB_NO, value_int, &tiff_header[tiff_offset[i]]);
		value_int = 4;
		mb_put_binary_int(MB_NO, value_int, &tiff_header[tiff_offset[i]+4]);
		break;
	      case YResolution:
		value_int = 1;
		mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
		index += 4;
		value_int = tiff_offset[i];
	        mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
		index += 4;
		value_int = MAX(nx, ny);
		mb_put_binary_int(MB_NO, value_int, &tiff_header[tiff_offset[i]]);
		value_int = 4;
		mb_put_binary_int(MB_NO, value_int, &tiff_header[tiff_offset[i]+4]);
		break;
	      case ResolutionUnit:
		value_int = 1;
		mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
		index += 4;
		value_short = 2;
	        mb_put_binary_short(MB_NO, value_short, &tiff_header[index]);
		index += 4;
		break;
	      case ModelPixelScaleTag:
		value_int = 3;
		mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
		index += 4;
		value_int = tiff_offset[i];
	        mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
		index += 4;
		value_double = header.x_inc;
		mb_put_binary_double(MB_NO, value_double, &tiff_header[tiff_offset[i]]);
		value_double = header.y_inc;
		mb_put_binary_double(MB_NO, value_double, &tiff_header[tiff_offset[i]+8]);
		value_double = 0.0;
		mb_put_binary_double(MB_NO, value_double, &tiff_header[tiff_offset[i]+16]);
		break;
	      case ModelTiepointTag:
		value_int = 6;
		mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
		index += 4;
		value_int = tiff_offset[i];
	        mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
		index += 4;
		value_double = 0;
		mb_put_binary_double(MB_NO, value_double, &tiff_header[tiff_offset[i]]);
		value_double = 0;
		mb_put_binary_double(MB_NO, value_double, &tiff_header[tiff_offset[i] + 1 * 8]);
		value_double = 0;
		mb_put_binary_double(MB_NO, value_double, &tiff_header[tiff_offset[i] + 2 * 8]);
		value_double = header.x_min;
		mb_put_binary_double(MB_NO, value_double, &tiff_header[tiff_offset[i] + 3 * 8]);
		value_double = header.y_max;
		mb_put_binary_double(MB_NO, value_double, &tiff_header[tiff_offset[i] + 4 * 8]);
		value_double = 0.0;
		mb_put_binary_double(MB_NO, value_double, &tiff_header[tiff_offset[i] + 5 * 8]);
		break;
	      }
	  }

	/* generate image */
	for (j=0;j<ny;j++)
	  for (i=0;i<nx;i++)
	    {
	      k = j * nx + i;
#ifdef GMT3_0
	      get_rgb24(grid[k], &r, &g, &b);
	      if (intensity == MB_YES)
		illuminate(igrid[k], &r, &g, &b);
	      if (gmt_gray)
		{
		  tiff_image[k] = (mb_u_char)r;
		}
	      else
		{
	          kk = 3 * k;
		  tiff_image[kk] = (mb_u_char)r;
		  tiff_image[kk+1] = (mb_u_char)g;
		  tiff_image[kk+2] = (mb_u_char)b;
		}
#else
	      GMT_get_rgb24(grid[k], rgb);
	      if (intensity == MB_YES)
		GMT_illuminate(igrid[k], rgb);
	      if (GMT_gray)
		{
		  tiff_image[k] = (mb_u_char)rgb[0];
		}
	      else
		{
	          kk = 3 * k;
		  tiff_image[kk] = (mb_u_char)rgb[0];
		  tiff_image[kk+1] = (mb_u_char)rgb[1];
		  tiff_image[kk+2] = (mb_u_char)rgb[2];
		}
#endif
	    }

	/* write the header */
	if ((status = fwrite(tiff_header,1,HEADER_SIZE,tfp)) 
			!= HEADER_SIZE) 
	  {
	    status = MB_FAILURE;
	    error = MB_ERROR_WRITE_FAIL;
	  }

	/* write the image */
	if ((status = fwrite(tiff_image,1,image_size,tfp)) 
			!= image_size) 
	  {
	    status = MB_FAILURE;
	    error = MB_ERROR_WRITE_FAIL;
	  }

	/* close the tiff file */
	fclose(tfp);

	/* deallocate arrays */
	status = mb_free(verbose,&grid,&error);
	if (intensity == MB_YES)
	  status = mb_free(verbose,&igrid,&error);
	status = mb_free(verbose,&tiff_image,&error);

	/* set the status */
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
	fprintf(stderr,"\n");
	exit(status);
}

/*--------------------------------------------------------------------*/

















