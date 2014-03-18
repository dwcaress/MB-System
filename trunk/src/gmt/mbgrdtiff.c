/*--------------------------------------------------------------------
 *    The MB-system:	mbgrdtiff.c	5/30/93
 *    $Id$
 *
 *    Copyright (c) 1999-2014 by
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
 *      8    2      18       Number of entries in IFD
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
 *     90    4      512      Value: offset to image data
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
 *    138    4      264      Value: offset to fraction representing dpi
 *
 *    142    2      283      Tag:   YResolution
 *    144    2      4        Type:  rational (2 ints: numerator, denominator)
 *    146    4      1        Count: one value
 *    150    4      272      Value: offset to fraction representing dpi
 *
 *    154    2      296      Tag:   ResolutionUnit
 *    156    2      3        Type:  short (2 byte unsigned int)
 *    158    4      1        Count: one value
 *    162    4      2        Value: Inches
 *
 *    166    2      33550    Tag:   ModelPixelScaleTag
 *    168    2      12       Type:  double (IEEE double precision)
 *    170    4      3        Count: 3 for scalex,scaley,scalez where scalez=0
 *    174    4      280      Value: offset to values
 *
 *    178    2      33922    Tag:   ModelTiepointTag
 *    180    2      12       Type:  double (IEEE double precision)
 *    182    4      6        Count: 6 for i,j,k,x,y,z where k=z=0
 *    186    4      304      Value: offset to values
 *
 *    190    2      34735    Tag:   GeoKeyDirectoryTag
 *    192    2      3        Type:  short (2 byte unsigned int)
 *    194    4      20       Count: 20
 *    198    4      352      Value: offset to values
 *
 *    190    2      34736    Tag:   GeoDoubleParamsTag
 *    192    2      3        Type:  double (IEEE double precision)
 *    194    4      ndouble  Count: ndouble
 *    198    4      400      Value: offset to values
 *
 *    190    2      34737    Tag:   GeoAsciiParamsTag
 *    192    2      2        Type:  ASCII
 *    194    4      nascii   Count: nascii
 *    198    4      448      Value: offset to values
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
 *    ------- If GTModelTypeGeoKey = ModelTypeGeographic then:
 *        304    8      0        ModelTiePointTag i
 *        312    8      0        ModelTiePointTag j
 *        320    8      0        ModelTiePointTag k
 *        328    8      minlon   ModelTiePointTag minimum longitude
 *        336    8      maxlat   ModelTiePointTag minimum latitude
 *        344    8      0        ModelTiePointTag minimum z
 *
 *    ------- Else if GTModelTypeGeoKey = ModelTypeProjected then:
 *        304    8      0        ModelTiePointTag i
 *        312    8      0        ModelTiePointTag j
 *        320    8      0        ModelTiePointTag k
 *        328    8      minX     ModelTiePointTag minimum easting
 *        336    8      maxY     ModelTiePointTag minimum northing
 *        344    8      0        ModelTiePointTag minimum z
 *
 *    352    2      1        GeoKeyDirectoryTag KeyDirectoryVersion
 *    354    2      0        GeoKeyDirectoryTag KeyRevision
 *    356    2      2        GeoKeyDirectoryTag MinorRevision
 *    358    2      4        GeoKeyDirectoryTag NumberOfKeys
 *
 *    360    2      1024     GeoKeyDirectoryTag KeyId: GTModelTypeGeoKey
 *    362    2      0        GeoKeyDirectoryTag TiffTagLocation
 *    364    2      1        GeoKeyDirectoryTag Count
 *    366    2      1        GeoKeyDirectoryTag ModelType Value (ModelTypeProjected=1, ModelTypeGeographic=2)
 *
 *    368    2      1025     GeoKeyDirectoryTag KeyId: GTRasterTypeGeoKey
 *    370    2      0        GeoKeyDirectoryTag TiffTagLocation
 *    372    2      1        GeoKeyDirectoryTag Count
 *    374    2      2        GeoKeyDirectoryTag RasterType Value (RasterPixelIsArea=1, RasterPixelIsPoint=2)
 *
 *    368    2      1026     GeoKeyDirectoryTag KeyId: GTCitationGeoKey
 *    370    2      34737    GeoKeyDirectoryTag TiffTagLocation
 *    372    2      nascii   GeoKeyDirectoryTag Count
 *    374    2      0        GeoKeyDirectoryTag Value_Offset
 *
 *    ------- If GTModelTypeGeoKey = ModelTypeGeographic then:
 *        376    2      2048     GeoKeyDirectoryTag KeyId: GeographicTypeGeoKey
 *        378    2      0        GeoKeyDirectoryTag TiffTagLocation
 *        380    2      1        GeoKeyDirectoryTag Count
 *        382    2      4030     GeoKeyDirectoryTag GeographicType Value (GCSE_WGS84=4030)
 *
 *    ------- Else if GTModelTypeGeoKey = ModelTypeProjected then:
 *        376    2      3072     GeoKeyDirectoryTag KeyId: ProjectedCSTypeGeoKey
 *        378    2      0        GeoKeyDirectoryTag TiffTagLocation
 *        380    2      1        GeoKeyDirectoryTag Count
 *        382    2      32660    GeoKeyDirectoryTag ProjectedCSType Value (PCS_WGS84_UTM_zone_60N=32660)
 *
 *    400    8      ndouble  GeoDoubleParamsTag
 *
 *    448    8      nascii   GeoAsciiParamsTag
 *
 *           ------- Image ---------
 *
 *    512    3*nx*ny         Image in RGB bytes
 *
 *
 *
 * $Log: mbgrdtiff.c,v $
 * Revision 5.16  2008/09/13 06:08:09  caress
 * Updates to apply suggested patches to segy handling. Also fixes to remove compiler warnings.
 *
 * Revision 5.15  2006/09/11 18:55:52  caress
 * Changes during Western Flyer and Thomas Thompson cruises, August-September
 * 2006.
 *
 * Revision 5.14  2006/08/09 22:41:27  caress
 * Fixed programs that read or write grids so that they do not use the GMT_begin() function; these programs will now work when GMT is built in the default fashion, when GMT is built in the default fashion, with "advisory file locking" enabled.
 *
 * Revision 5.13  2006/07/27 18:42:51  caress
 * Working towards 5.1.0
 *
 * Revision 5.12  2006/06/22 04:45:42  caress
 * Working towards 5.1.0
 *
 * Revision 5.11  2006/01/11 07:25:53  caress
 * Working towards 5.0.8
 *
 * Revision 5.10  2005/11/04 20:18:04  caress
 * Altered the GeoTiff header so that images are compatible with more GIS packages.
 *
 * Revision 5.9  2004/05/21 23:13:35  caress
 * Changes to support GMT 4.0
 *
 * Revision 5.8  2003/04/17 20:43:37  caress
 * Release 5.0.beta30
 *
 * Revision 5.7  2002/11/12 06:47:19  caress
 * Proper GeoTIFF now created with projected coordinate systems.
 *
 * Revision 5.6  2001/11/08 02:22:17  caress
 * Fixed program so bounds can be taken from grd file if not
 * specified on the command line.
 *
 * Revision 5.5  2001/09/19  21:57:10  caress
 * Really really removed inadvertant debug messages.
 *
 * Revision 5.4  2001/09/19  21:53:26  caress
 * Really removed inadvertant debug messages.
 *
 * Revision 5.3  2001/09/19  21:51:56  caress
 * Removed inadvertant debug messages.
 *
 * Revision 5.2  2001/06/03  06:58:45  caress
 * Release 5.0.beta01.
 *
 * Revision 5.1  2001/01/22 05:03:25  caress
 * Release 5.0.beta01
 *
 * Revision 5.0  2000/12/01  22:52:16  caress
 * First cut at Version 5.0.
 *
 * Revision 4.2  2000/10/11  00:53:45  caress
 * Converted to ANSI C
 *
 * Revision 4.1  2000/09/30  06:52:17  caress
 * Snapshot for Dale.
 *
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

/* GMT include files */
#include "gmt.h"

/* MBIO include files */
#include "mb_status.h"
#include "mb_format.h"
#include "mb_define.h"

/* TIFF 6.0 and geoTIFF tag array */
#define TIFF_HEADER_SIZE 1024
#define IMAGE_OFFSET TIFF_HEADER_SIZE
#define TIFF_COMMENT_MAXLINE 64
#define NUMBER_TAGS 18
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
#define GeoKeyDirectoryTag       34735
#define GeoDoubleParamsTag       34736
#define GeoAsciiParamsTag        34737
#define GTModelTypeGeoKey         1024
#define GTRasterTypeGeoKey        1025
#define GTCitationGeoKey          1026
#define GeographicTypeGeoKey      2048
#define ProjectedCSTypeGeoKey     3072

#define RasterPixelIsArea	     1
#define RasterPixelIsPoint	     2
#define ModelTypeProjected	     1
#define ModelTypeGeographic	     2
#define GCS_WGS_84		  4326

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
		        ModelTiepointTag,
			GeoKeyDirectoryTag,
			GeoDoubleParamsTag,
			GeoAsciiParamsTag
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
			 12,      /* ModelTiepointTag */
			  3,      /* GeoKeyDirectoryTag */
			 12,      /* GeoDoubleParamsTag */
			  2       /* GeoAsciiParamsTag */
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
			304,      /* ModelTiepointTag */
			352,      /* GeoKeyDirectoryTag */
			400,      /* GeoDoubleParamsTag */
			448       /* GeoAsciiParamsTag */
		      };

static char rcs_id[] = "$Id$";

/*--------------------------------------------------------------------*/

int main (int argc, char **argv)
{
	char program_name[] = "mbgrdtiff";
	char help_message[] = "mbgrdtiff generates a tiff image from a GMT grid. The \nimage generation is similar to that of the GMT program \ngrdimage. In particular, the color map is applied from \na GMT CPT file, and shading overlay grids may be applied. \nThe output TIFF file contains information allowing\nthe ArcView and ArcInfo GIS packages to import the image\nas a geographically located coverage.";
	char usage_message[] = "mbgrdtiff -Ccptfile -Igrdfile -Otiff_file [-H -Kintensfile -V]";
	extern char *optarg;
	int	errflg = 0;
	int	c;
	int	help = 0;
	int	flag = 0;

	/* MBIO status variables */
	int	status = MB_SUCCESS;
	int	verbose = 0;
	int	lonflip = 0;
	int	error = MB_ERROR_NO_ERROR;
	char	*message = NULL;

	/* control parameters */
	char    grdfile[MB_PATH_MAXLINE];
	char    cptfile[MB_PATH_MAXLINE];
        char    intensfile[MB_PATH_MAXLINE];
        char    tiff_file[MB_PATH_MAXLINE];
        char    world_file[MB_PATH_MAXLINE];
	int	intensity;
        double  bounds[4];
#ifdef GMT_MINOR_VERSION
	GMT_LONG	pad[4];
#else
	int	pad[4];
#endif
        int     nx,ny,nxy;
        float   *grid = NULL;
        float   *igrid = NULL;
	struct GRD_HEADER header;
	struct GRD_HEADER iheader;

	/* TIFF arrays */
	int     image_size = 0;
	int	modeltype;
	int	projectionid;
        char    projectionname[MB_PATH_MAXLINE];
	char    tiff_header[TIFF_HEADER_SIZE];
	char    *tiff_image = NULL;
	char	tiff_comment[TIFF_COMMENT_MAXLINE];
	char	NorS;

	/* other variables */
	FILE    *tfp;
	int     rgb[3];
	int	i, j, k, kk;
        int     index, keyindex;
	int	nscan;
	int	utmzone;
	int	off;
        short   value_short;
        int     value_int;
	double  value_double;
	char	*projection = "-Jx1.0";
        int     make_worldfile = MB_NO;

	/* get current mb default values */
	status = mb_lonflip(verbose,&lonflip);

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

	/* deal with gmt options */
	GMT_begin (1, argv);
	errflg += GMT_get_common_args (projection,
				&bounds[0], &bounds[1],
				&bounds[2], &bounds[3]);
	for (i = 1; i < argc; i++)
		{
		if (argv[i][0] == '-')
			{
			switch (argv[i][1])
				{
				/* Common parameters */

				case 'r':
					argv[i][1] = 'R';
				case 'R':
					errflg += GMT_get_common_args (argv[i],
						&bounds[0], &bounds[1],
						&bounds[2], &bounds[3]);
					break;

				/* Supplemental parameters */

 			}
			}
		}

	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhC:c:I:i:K:k:L:l:O:o:R:r:Ww")) != -1)
	  switch (c)
		{
		case 'H':
		case 'h':
			help++;
			break;
		case 'C':
		case 'c':
			sscanf (optarg,"%s", cptfile);
			flag++;
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
		case 'L':
		case 'l':
			sscanf (optarg,"%d", &lonflip);
			flag++;
			break;
		case 'O':
		case 'o':
			sscanf (optarg,"%s", tiff_file);
			flag++;
			break;
		case 'V':
		case 'v':
			verbose++;
			break;
                case 'W':
                case 'w':
                        make_worldfile = MB_YES;
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

	GMT_grd_init (&header, argc, argv, FALSE);
	GMT_grd_init (&iheader, argc, argv, FALSE);

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
		fprintf(stderr,"dbg2       verbose:         %d\n",verbose);
		fprintf(stderr,"dbg2       help:            %d\n",help);
		fprintf(stderr,"dbg2       cptfile:         %s\n",cptfile);
		fprintf(stderr,"dbg2       grdfile:         %s\n",grdfile);
		fprintf(stderr,"dbg2       intensfile:      %s\n",intensfile);
		fprintf(stderr,"dbg2       tiff_file:       %s\n",tiff_file);
		fprintf(stderr,"dbg2       make_worldfile:  %d\n",make_worldfile);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}

	/* get color palette file */
	GMT_read_cpt(cptfile);
	if (GMT_n_colors <= 0)
	  {
	    fprintf(stderr,"\nColor palette table not properly specified:\n");
	    fprintf(stderr,"\nProgram <%s> Terminated\n",
		    program_name);
	    error = MB_ERROR_BAD_PARAMETER;
	    exit(error);
	  }

	/* read input grd file header */
	if (GMT_read_grd_info (grdfile, &header))
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
	    if (GMT_read_grd_info (intensfile, &iheader))
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

	/* try to get projection from the grd file remark */
	if (strncmp(&(header.remark[2]), "Projection: ", 12) == 0)
		{
		if ((nscan = sscanf(&(header.remark[2]), "Projection: UTM%d%c", &utmzone, &NorS)) == 2)
			{
			if (NorS == 'N')
				{
				projectionid = 32600 + utmzone;
				}
			else if (NorS == 'S')
				{
				projectionid = 32700 + utmzone;
				}
				modeltype = ModelTypeProjected;
			sprintf(projectionname, "UTM%2.2d%c", utmzone, NorS);

			project_info.degree[0] = FALSE;
			}
		else if ((nscan = sscanf(&(header.remark[2]), "Projection: epsg%d", &projectionid)) == 1)
			{
			sprintf(projectionname, "epsg%d", projectionid);
			modeltype = ModelTypeProjected;

			project_info.degree[0] = FALSE;
			}
		else if (strncmp(&(header.remark[2]), "Projection: SeismicProfile", 26) == 0)
			{
			sprintf(projectionname, "SeismicProfile");
			modeltype = ModelTypeProjected;

			project_info.degree[0] = FALSE;
			}
		else
			{
			strcpy(projectionname, "Geographic WGS84");
			modeltype = ModelTypeGeographic;
			projectionid = GCS_WGS_84;

			project_info.degree[0] = TRUE;
			}
		}
	else
		{
		strcpy(projectionname, "Geographic WGS84");
		modeltype = ModelTypeGeographic;
		projectionid = GCS_WGS_84;

		project_info.degree[0] = TRUE;
		}

	/* set bounds from grd file if not set on command line */
	if (bounds[1] <= bounds[0] || bounds[3] <= bounds[2])
	    {
	    bounds[0] = header.x_min;
	    bounds[1] = header.x_max;
	    bounds[2] = header.y_min;
	    bounds[3] = header.y_max;
	    }

	/* Determine the wesn to be used to read the grdfile */
	off = (header.node_offset) ? 0 : 1;
	GMT_map_setup (bounds[0], bounds[1], bounds[2], bounds[3]);
#ifdef GMT_MINOR_VERSION
	GMT_grd_setregion (&header, &bounds[0], &bounds[1], &bounds[2], &bounds[3], BCR_BILINEAR);
#else
	GMT_grd_setregion (&header, &bounds[0], &bounds[1], &bounds[2], &bounds[3]);
#endif

	/* allocate memory */
	nx = irint ( (bounds[1] - bounds[0]) / header.x_inc) + off;
	ny = irint ( (bounds[3] - bounds[2]) / header.y_inc) + off;
	nxy = nx * ny;
	if (GMT_gray)
	  image_size = nx * ny;
	else
	  image_size = 3 * nx * ny;
	status = mb_mallocd(verbose, __FILE__, __LINE__, nxy * sizeof(float), (void **)&grid, &error);
	if (intensity == MB_YES)
	    status = mb_mallocd(verbose, __FILE__, __LINE__, nxy * sizeof(float), (void **)&igrid, &error);
	status = mb_mallocd(verbose, __FILE__, __LINE__, image_size, (void **)&tiff_image, &error);

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
	if (GMT_read_grd (grdfile, &header, grid,
			    bounds[0],  bounds[1],  bounds[2],  bounds[3],
			    pad, FALSE))
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
	    if (GMT_read_grd (intensfile, &iheader, igrid,
				bounds[0],  bounds[1],  bounds[2],  bounds[3],
				pad, FALSE))
		{
		error = MB_ERROR_OPEN_FAIL;
		fprintf(stderr,"\nUnable to read intensity grd file: %s\n",
			intensfile);
		fprintf(stderr,"\nProgram <%s> Terminated\n",
			program_name);
		exit(error);
		}
	    }

	/* apply lonflip */
	if (modeltype != ModelTypeProjected)
		{
		mb_apply_lonflip(verbose, lonflip, &header.x_min);
		mb_apply_lonflip(verbose, lonflip, &header.x_max);
		if (intensity == MB_YES)
	    		{
			mb_apply_lonflip(verbose, lonflip, &iheader.x_min);
			mb_apply_lonflip(verbose, lonflip, &iheader.x_max);
			}
		}

	/* print debug info */
	if (verbose >= 0)
	    {
	    fprintf(stderr,"Grid read:\n");
	    fprintf(stderr,"  Dimensions: %d %d\n", header.nx, header.ny);
	    if (modeltype == ModelTypeProjected)
	    	{
		fprintf(stderr,"  Projected Coordinate System Name: %s\n", projectionname);
		fprintf(stderr,"  Projected Coordinate System ID:   %d\n", projectionid);
	    	fprintf(stderr,"  Easting:    %f %f  %f\n",
		  	header.x_min, header.x_max, header.x_inc);
	    	fprintf(stderr,"  Northing:   %f %f  %f\n",
		  	header.y_min, header.y_max, header.y_inc);
		}
	    else
		{
		fprintf(stderr,"  Geographic Coordinate System Name: %s\n", projectionname);
		fprintf(stderr,"  Geographic Coordinate System ID:   %d\n", projectionid);
	    	fprintf(stderr,"  Longitude:  %f %f  %f\n",
		  	header.x_min, header.x_max, header.x_inc);
	    	fprintf(stderr,"  Latitude:   %f %f  %f\n",
		  	header.y_min, header.y_max, header.y_inc);
		}
	    if (GMT_gray)
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

	/* set the TIFF comment */
	sprintf(tiff_comment, "Image generated by %s|", program_name);

	/* set the TIFF header */
	memset(tiff_header,0,TIFF_HEADER_SIZE);
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
		if (GMT_gray)
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
		if (GMT_gray)
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
		if (GMT_gray)
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
		if (GMT_gray)
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
		value_double = header.x_min - 0.5 * header.x_inc;
		mb_put_binary_double(MB_NO, value_double, &tiff_header[tiff_offset[i] + 3 * 8]);
		value_double = header.y_max + 0.5 * header.y_inc;
		mb_put_binary_double(MB_NO, value_double, &tiff_header[tiff_offset[i] + 4 * 8]);
		value_double = 0.0;
		mb_put_binary_double(MB_NO, value_double, &tiff_header[tiff_offset[i] + 5 * 8]);
		break;
	      case GeoKeyDirectoryTag:
		value_int = 20;
		mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
		index += 4;
		value_int = tiff_offset[i];
	        mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
		index += 4;

		/* index to geotiff geokey directory */
		keyindex = tiff_offset[i];

		/* geokey directory header
			(KeyDirectoryVersion, KeyRevision, MinorRevision, NumberOfKeys) */
		value_short = 1;
		mb_put_binary_short(MB_NO, value_short, &tiff_header[keyindex]);
		keyindex += 2;
		value_short = 0;
		mb_put_binary_short(MB_NO, value_short, &tiff_header[keyindex]);
		keyindex += 2;
		value_short = 2;
		mb_put_binary_short(MB_NO, value_short, &tiff_header[keyindex]);
		keyindex += 2;
		value_short = 4;
		mb_put_binary_short(MB_NO, value_short, &tiff_header[keyindex]);
		keyindex += 2;

		/* GTModelTypeGeoKey */
		value_short = GTModelTypeGeoKey;
		mb_put_binary_short(MB_NO, value_short, &tiff_header[keyindex]);
		keyindex += 2;
		value_short = 0;
		mb_put_binary_short(MB_NO, value_short, &tiff_header[keyindex]);
		keyindex += 2;
		value_short = 1;
		mb_put_binary_short(MB_NO, value_short, &tiff_header[keyindex]);
		keyindex += 2;
		value_short = modeltype;
		mb_put_binary_short(MB_NO, value_short, &tiff_header[keyindex]);
		keyindex += 2;

		/* GTRasterTypeGeoKey */
		value_short = GTRasterTypeGeoKey;
		mb_put_binary_short(MB_NO, value_short, &tiff_header[keyindex]);
		keyindex += 2;
		value_short = 0;
		mb_put_binary_short(MB_NO, value_short, &tiff_header[keyindex]);
		keyindex += 2;
		value_short = 1;
		mb_put_binary_short(MB_NO, value_short, &tiff_header[keyindex]);
		keyindex += 2;
		value_short = RasterPixelIsPoint;
		mb_put_binary_short(MB_NO, value_short, &tiff_header[keyindex]);
		keyindex += 2;

		/* GTCitationGeoKey */
		value_short = GTCitationGeoKey;
		mb_put_binary_short(MB_NO, value_short, &tiff_header[keyindex]);
		keyindex += 2;
		value_short = GeoAsciiParamsTag;
		mb_put_binary_short(MB_NO, value_short, &tiff_header[keyindex]);
		keyindex += 2;
		value_short = strlen(tiff_comment);
		mb_put_binary_short(MB_NO, value_short, &tiff_header[keyindex]);
		keyindex += 2;
		value_short = 0;
		mb_put_binary_short(MB_NO, value_short, &tiff_header[keyindex]);
		keyindex += 2;

		if (modeltype == ModelTypeGeographic)
			{
			/* GeographicTypeGeoKey */
			value_short = GeographicTypeGeoKey;
			mb_put_binary_short(MB_NO, value_short, &tiff_header[keyindex]);
			keyindex += 2;
			value_short = 0;
			mb_put_binary_short(MB_NO, value_short, &tiff_header[keyindex]);
			keyindex += 2;
			value_short = 1;
			mb_put_binary_short(MB_NO, value_short, &tiff_header[keyindex]);
			keyindex += 2;
			value_short = projectionid;
			mb_put_binary_short(MB_NO, value_short, &tiff_header[keyindex]);
			keyindex += 2;
			}

		else if (modeltype == ModelTypeProjected)
			{
			/* ProjectedCSTypeGeoKey */
			value_short = ProjectedCSTypeGeoKey;
			mb_put_binary_short(MB_NO, value_short, &tiff_header[keyindex]);
			keyindex += 2;
			value_short = 0;
			mb_put_binary_short(MB_NO, value_short, &tiff_header[keyindex]);
			keyindex += 2;
			value_short = 1;
			mb_put_binary_short(MB_NO, value_short, &tiff_header[keyindex]);
			keyindex += 2;
			value_short = projectionid;
			mb_put_binary_short(MB_NO, value_short, &tiff_header[keyindex]);
			keyindex += 2;
			}
		break;
	      case GeoDoubleParamsTag:
		value_int = 1;
		mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
		index += 4;
		value_int = tiff_offset[i];
	        mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
		index += 4;
		break;
	      case GeoAsciiParamsTag:
		value_int = strlen(tiff_comment);
		mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
		index += 4;
		value_int = tiff_offset[i];
	        mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
		index += 4;

		/* put in the string */
		strncpy(&tiff_header[tiff_offset[i]], tiff_comment, 64);
		break;
	      }
	  }

	/* generate image */
	for (j=0;j<ny;j++)
	  for (i=0;i<nx;i++)
	    {
	      k = j * nx + i;
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
	    }

	/* write the header */
	if ((status = fwrite(tiff_header,1,TIFF_HEADER_SIZE,tfp))
			!= TIFF_HEADER_SIZE)
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

	/* open world file */
        if (make_worldfile == MB_YES)
            {
            strcpy(world_file, tiff_file);
            world_file[strlen(tiff_file)-4] = '\0';
            strcat(world_file,".tfw");
            if ((tfp = fopen(world_file,"w")) == NULL)
                {
                error = MB_ERROR_OPEN_FAIL;
                fprintf(stderr,"\nUnable to open output world file: %s\n",
                      world_file);
                fprintf(stderr,"\nProgram <%s> Terminated\n",
                      program_name);
                exit(error);
                }

            /* write out world file contents */
            fprintf(tfp, "%f\r\n0.0\r\n0.0\r\n%f\r\n%f\r\n%f\r\n",
                    header.x_inc, -header.y_inc,
                    header.x_min - 0.5 * header.x_inc,
                    header.y_max + 0.5 * header.y_inc);

            /* close the world file */
            fclose(tfp);
            }

	/* deallocate arrays */
	status = mb_freed(verbose,__FILE__, __LINE__, (void **)&grid, &error);
	if (intensity == MB_YES)
	  status = mb_freed(verbose,__FILE__, __LINE__, (void **)&igrid, &error);
	status = mb_freed(verbose,__FILE__, __LINE__, (void **)&tiff_image, &error);

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

	/* clean up GMT memory usage and file locking */
	fprintf(stderr,"\n");
	GMT_end(1, argv);

	/* end it all */
	exit(status);
}

/*--------------------------------------------------------------------*/
