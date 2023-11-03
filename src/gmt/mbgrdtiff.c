/*--------------------------------------------------------------------
 *    The MB-system:	mbgrdtiff.c	5/30/93
 *
 *    Copyright (c) 1999-2023 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes 
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *     
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 *    mbgrdtiff generates a TIFF image from a GMT grid. The
 *    image generation is performed by a call to GMT_grdimage.
 *    The output TIFF file contains information allowing
 *    the ArcView and ArcInfo GIS packages to import the image
 *    as a geographically located coverage. The image is 8 bits
 *    per pixel if the color map is a grayscale, and 24 bits
 *    per pixel otherwise.
 */
/*--------------------------------------------------------------------*/
/*
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
 */
/*--------------------------------------------------------------------*/

/* MBIO include files */
#include "mb_status.h"
#include "mb_format.h"
#include "mb_define.h"

// include gmt_dev.h but first undefine PACKAGE variables to prevent
// warnings about name collision between GDAL's cpl_port.h and the 
// Autotools build system mb_config.hmb_config.h
#ifdef PACKAGE_BUGREPORT
#undef PACKAGE_BUGREPORT
#endif
#ifdef PACKAGE_NAME
#undef PACKAGE_NAME
#endif
#ifdef PACKAGE_STRING
#undef PACKAGE_STRING
#endif
#ifdef PACKAGE_TARNAME
#undef PACKAGE_TARNAME
#endif
#ifdef PACKAGE_URL
#undef PACKAGE_URL
#endif
#ifdef PACKAGE_VERSION
#undef PACKAGE_VERSION
#endif
#include "gmt_dev.h"

/* TIFF 6.0 and geoTIFF tag array */
#define TIFF_HEADER_SIZE 1024
#define IMAGE_OFFSET TIFF_HEADER_SIZE
#define TIFF_COMMENT_MAXLINE 64
#define NUMBER_TAGS 18
#define NewSubfileType 254
#define ImageWidth 256
#define ImageLength 257
#define BitsPerSample 258
#define Compression 259
#define PhotometricInterpretation 262
#define StripOffsets 273
#define SamplesPerPixel 277
#define RowsPerStrip 278
#define StripByteCounts 279
#define XResolution 282
#define YResolution 283
#define ResolutionUnit 296
#define ModelPixelScaleTag 33550
#define ModelTiepointTag 33922
#define GeoKeyDirectoryTag 34735
#define GeoDoubleParamsTag 34736
#define GeoAsciiParamsTag 34737
#define GTModelTypeGeoKey 1024
#define GTRasterTypeGeoKey 1025
#define GTCitationGeoKey 1026
#define GeographicTypeGeoKey 2048
#define ProjectedCSTypeGeoKey 3072

#define RasterPixelIsArea 1
#define RasterPixelIsPoint 2
#define ModelTypeProjected 1
#define ModelTypeGeographic 2
#define GCS_WGS_84 4326

static unsigned short tiff_tag[] = {
    NewSubfileType, ImageWidth,         ImageLength,      BitsPerSample,      Compression,        PhotometricInterpretation,
    StripOffsets,   SamplesPerPixel,    RowsPerStrip,     StripByteCounts,    XResolution,        YResolution,
    ResolutionUnit, ModelPixelScaleTag, ModelTiepointTag, GeoKeyDirectoryTag, GeoDoubleParamsTag, GeoAsciiParamsTag};
static unsigned short tiff_type[] = {
    4,  /* NewSubfileType */
    4,  /* ImageWidth */
    4,  /* ImageLength */
    3,  /* BitsPerSample */
    3,  /* Compression */
    3,  /* PhotometricInterpretation */
    4,  /* StripOffsets */
    3,  /* SamplesPerPixel */
    4,  /* RowsPerStrip */
    4,  /* StripByteCounts */
    5,  /* XResolution */
    5,  /* YResolution */
    3,  /* ResolutionUnit */
    12, /* ModelPixelScaleTag */
    12, /* ModelTiepointTag */
    3,  /* GeoKeyDirectoryTag */
    12, /* GeoDoubleParamsTag */
    2   /* GeoAsciiParamsTag */
};
static int tiff_offset[] = {
    0,   /* NewSubfileType */
    0,   /* ImageWidth */
    0,   /* ImageLength */
    256, /* BitsPerSample */
    0,   /* Compression */
    0,   /* PhotometricInterpretation */
    0,   /* StripOffsets */
    0,   /* SamplesPerPixel */
    0,   /* RowsPerStrip */
    0,   /* StripByteCounts */
    264, /* XResolution */
    272, /* YResolution */
    0,   /* ResolutionUnit */
    280, /* ModelPixelScaleTag */
    304, /* ModelTiepointTag */
    352, /* GeoKeyDirectoryTag */
    400, /* GeoDoubleParamsTag */
    448  /* GeoAsciiParamsTag */
};

/*--------------------------------------------------------------------*/

#define THIS_MODULE_NAME "mbgrdtiff"
#define THIS_MODULE_LIB "mbgmt"
#define THIS_MODULE_PURPOSE "Create a GeoTiff image from a grid"
#define THIS_MODULE_KEYS ""

EXTERN_MSC int GMT_mbgrdtiff(void *API, int mode, void *args);

#define GMT_PROG_OPTIONS "->JRVn" GMT_OPT("S")

/* Control structure for mbgrdtiff */
struct MBGRDTIFF_CTRL {
	struct C { /* -C<cptfile> */
		bool active;
		char *arg;
	} C;
	struct E { /* -Ei|<dpi> */
		bool active;
		char *arg;
	} E;
	struct G { /* -G[f|b]<rgb> */
		bool active;
		char *arg;
	} G;
	struct I { /* -I<inputfile> */
		bool active;
		unsigned int n_files;
		char *file[3];
	} I;
	struct Intensity { /* -I<intensfile>|<value> */
		bool active;
		char *arg;
	} Intensity;
	struct M { /* -M */
		bool active;
	} M;
	struct Nudge { /* -N<nudge_x>/<nudge_y> */
		bool active;
		double nudge_x;
		double nudge_y;
	} Nudge;
	struct mbO { /* -O */
		bool active;
		char *file;
	} O;
	struct Q { /* -Q */
		bool active;
	} Q;
};

/*--------------------------------------------------------------------*/

static void *New_mbgrdtiff_Ctrl(struct GMT_CTRL *GMT) { /* Allocate and initialize a new control structure */
	struct MBGRDTIFF_CTRL *Ctrl;

	Ctrl = gmt_M_memory(GMT, NULL, 1, struct MBGRDTIFF_CTRL);

	return (Ctrl);
}
/*--------------------------------------------------------------------*/

static void Free_mbgrdtiff_Ctrl(struct GMT_CTRL *GMT, struct MBGRDTIFF_CTRL *Ctrl) { /* Deallocate control structure */
	int k;
	if (!Ctrl)
		return;
	for (k = 0; k < 3; k++)
		if (Ctrl->I.file[k])
			free(Ctrl->I.file[k]);
	if (Ctrl->C.arg)
		free(Ctrl->C.arg);
	if (Ctrl->E.arg)
		free(Ctrl->E.arg);
	if (Ctrl->G.arg)
		free(Ctrl->G.arg);
	if (Ctrl->Intensity.active)
		free(Ctrl->Intensity.arg);
	gmt_M_free(GMT, Ctrl);
}
/*--------------------------------------------------------------------*/

static int GMT_mbgrdtiff_usage(struct GMTAPI_CTRL *API, int level) {
	gmt_show_name_and_purpose(API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE)
		return (GMT_NOERROR);
	GMT_Message(API, GMT_TIME_NONE, "usage: mbgrdtiff <grd_z>|<grd_r> <grd_g> <grd_b> %s -O<tiff-file> [-C<cpt>] [-Ei[|<dpi>]]\n",
	            GMT_J_OPT);
	GMT_Message(API, GMT_TIME_NONE, "\t[-G[f|b]<rgb>] [-I<intensgrid>|<value>] [-M] [-N<nudge_x>/<nudge_y>]\n");
	GMT_Message(API, GMT_TIME_NONE, "\t[-Q] [%s] [-T] [%s]\n", GMT_Rgeo_OPT, GMT_V_OPT);
	GMT_Message(API, GMT_TIME_NONE, "\t[%s] [%s]\n\n", GMT_f_OPT, GMT_n_OPT);

	if (level == GMT_SYNOPSIS)
		return (EXIT_FAILURE);

	GMT_Message(API, GMT_TIME_NONE, "\t<grd_z> is data set to be plotted.  Its z-values are in user units and will be\n");
	GMT_Message(API, GMT_TIME_NONE, "\t  converted to rgb colors via the cpt file.  Alternatively, give three separate\n");
	GMT_Message(API, GMT_TIME_NONE, "\t  grid files that contain the red, green, and blue components in the 0-255 range.\n");
	GMT_Option(API, "J-");
	GMT_Message(API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message(API, GMT_TIME_NONE,
	            "\t-C Color palette file to convert z to rgb.  Optionally, instead give name of a master cpt\n");
	GMT_Message(API, GMT_TIME_NONE, "\t   to automatically assign 16 continuous colors over the data range [rainbow].\n");
	GMT_Message(API, GMT_TIME_NONE, "\t-E Set dpi for the projected grid which must be constructed [100]\n");
	GMT_Message(API, GMT_TIME_NONE, "\t   if -Jx or -Jm is not selected [Default gives same size as input grid].\n");
	GMT_Message(API, GMT_TIME_NONE, "\t   Give i to do the interpolation in PostScript at device resolution.\n");
	gmt_rgb_syntax(API->GMT, 'G', "Set transparency color for images that otherwise would result in 1-bit images.\n\t  ");
	GMT_Message(API, GMT_TIME_NONE, "\t-I Use illumination. Append name of intensity grid file.\n");
	GMT_Message(API, GMT_TIME_NONE, "\t   For a constant intensity, just give the value instead.\n");
	GMT_Message(API, GMT_TIME_NONE, "\t-M Force monochrome image.\n");
	GMT_Message(API, GMT_TIME_NONE, "\t-N<nudge_x>/<nudge_y>\n");
	GMT_Message(API, GMT_TIME_NONE, "\t-Q Use PS Level 3 colormasking to make nodes with z = NaN transparent.\n");
	GMT_Option(API, "R");
	GMT_Option(API, "V,n,.");

	return (EXIT_FAILURE);
}
/*--------------------------------------------------------------------*/

static int GMT_mbgrdtiff_parse(struct GMT_CTRL *GMT, struct MBGRDTIFF_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to mbgrdtiff and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) { /* Process all the options given */

		switch (opt->option) {
		case '<': /* Input file (only one or three is accepted) */
			Ctrl->I.active = true;
			if (Ctrl->I.n_files >= 3)
				break;
			if (gmt_check_filearg(GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET))
				Ctrl->I.file[Ctrl->I.n_files++] = strdup(opt->arg);
			else
				n_errors++;
			break;

			/* Processes program-specific parameters */

		case 'C': /* CPT file */
			Ctrl->C.active = true;
			if (Ctrl->C.arg)
				free(Ctrl->C.arg);
			Ctrl->C.arg = strdup(opt->arg);
			break;
		case 'E': /* Sets dpi */
			Ctrl->E.active = true;
			Ctrl->E.arg = strdup(opt->arg);
			break;
		case 'G': /* 1-bit fore or background color for transparent masks */
			Ctrl->G.active = true;
			Ctrl->G.arg = strdup(opt->arg);
			break;
		case 'I': /* Input file (only one or three is accepted) */
			/* if no grid file specified yet then first -Ifile sets
			 * the primary grid file, and second -Ifile will be the
			 * intensity file */
			if (Ctrl->I.n_files == 0) {
				Ctrl->I.active = true;
				Ctrl->I.file[Ctrl->I.n_files++] = strdup(opt->arg);
			}
			else {
				Ctrl->Intensity.active = true;
				Ctrl->Intensity.arg = strdup(opt->arg);
			}
			break;
		case 'M': /* Monochrome image */
			Ctrl->M.active = true;
			break;
		case 'N': /* -N<nudge_x>/<nudge_y> Offset location of image by
		          nudge_x meters east and nudge_y meters north */
			if (sscanf(opt->arg, "%lf/%lf", &Ctrl->Nudge.nudge_x, &Ctrl->Nudge.nudge_y) == 2) {
				Ctrl->Nudge.active = true;
			}
			else {
				Ctrl->Nudge.active = false;
			}
			break;
		case 'O': /* Output file */
			Ctrl->O.active = true;
			Ctrl->O.file = strdup(opt->arg);
			break;
		case 'Q': /* PS3 colormasking */
			Ctrl->Q.active = true;
			break;

		default: /* Report bad options */
			n_errors += gmt_default_error(GMT, opt->option);
			break;
		}
	}

	n_errors += gmt_M_check_condition(GMT, !Ctrl->O.active || !Ctrl->O.file,
	                                  "Syntax error -O option: Must specify the output file name.\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

GMT_LOCAL struct GMT_IMAGE *mbgrdtiff_get_image(struct GMTAPI_CTRL *API, struct MBGRDTIFF_CTRL *Ctrl) {
	/* use options to call grdimage -A and return the image back to the calling environment */
	char the_image[GMT_VF_LEN] = {""}, cmd[GMT_LEN256] = {""};
	struct GMT_IMAGE *I = NULL;

	/* Create grdimage command string */

	if (GMT_Open_VirtualFile(API, GMT_IS_IMAGE, GMT_IS_SURFACE, GMT_OUT, NULL, the_image)) {
		GMT_Report(API, GMT_MSG_ERROR, "Unable to create an output image reference\n");
		return NULL;
	}

	sprintf(cmd, "-A%s", the_image);
	for (unsigned int k = 0; k < Ctrl->I.n_files; k++) {
		strcat(cmd, " ");
		strcat(cmd, Ctrl->I.file[k]);
	}
	if (Ctrl->C.active) {
		strcat(cmd, " -C");
		strcat(cmd, Ctrl->C.arg);
	}
	if (Ctrl->E.active) {
		strcat(cmd, " -E");
		strcat(cmd, Ctrl->E.arg);
	}
	if (Ctrl->G.active) {
		strcat(cmd, " -G");
		strcat(cmd, Ctrl->G.arg);
	}
	if (Ctrl->Intensity.active) {
		strcat(cmd, " -I");
		strcat(cmd, Ctrl->Intensity.arg);
	}
	if (API->GMT->common.J.active) {
		strcat(cmd, " -J");
		strcat(cmd, API->GMT->common.J.string);
	}
	if (Ctrl->M.active)
		strcat(cmd, " -M");
	if (Ctrl->Q.active)
		strcat(cmd, " -Q");
	if (API->GMT->common.R.active[RSET]) {
		strcat(cmd, " -R");
		strcat(cmd, API->GMT->common.R.string);
	}

	GMT_Report(API, GMT_MSG_INFORMATION, "Calling grdimage with args %s\n", cmd);
	if (GMT_Call_Module(API, "grdimage", GMT_MODULE_CMD, cmd))
		return NULL;
	/* Obtain the data from the virtual file */
	if ((I = GMT_Read_VirtualFile(API, the_image)) == NULL)
		return NULL;
	GMT_Close_VirtualFile(API, the_image);

	return (I);
}

/*--------------------------------------------------------------------*/

#define bailout(code)                                                                                                            \
	{                                                                                                                            \
		gmt_M_free_options(mode);                                                                                                \
		return (code);                                                                                                           \
	}
#define Return(code)                                                                                                             \
	{                                                                                                                            \
		Free_mbgrdtiff_Ctrl(GMT, Ctrl);                                                                                          \
		gmt_end_module(GMT, GMT_cpy);                                                                                            \
		bailout(code);                                                                                                           \
	}

int GMT_mbgrdtiff(void *V_API, int mode, void *args) {
	char program_name[] = "mbgrdtiff";

	/* TIFF arrays */
	mb_path world_file;
	size_t image_size = 0;
	int modeltype;
	int projectionid;
	mb_path projectionname;
	char tiff_header[TIFF_HEADER_SIZE];
	void *tiff_image = NULL;
	char tiff_comment[TIFF_COMMENT_MAXLINE];
	char NorS;
	FILE *tfp;
	int nscan;
	int utmzone;
	int keyindex;

	double mtodeglon, mtodeglat;
	unsigned short value_short;
	int value_int;
	double value_double;
	size_t write_size;
	int i;

	int index = 0, error = 0;

	struct GMT_IMAGE *Image = NULL;
	struct MBGRDTIFF_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL; /* General GMT internal parameters */
	struct GMT_OPTION *options = NULL;
	struct GMT_GRID_HEADER *header_work = NULL;       /* Pointer to a GMT header for the image or grid */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr(V_API); /* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL)
		return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE)
		return (GMT_mbgrdtiff_usage(API, GMT_MODULE_PURPOSE)); /* Return the purpose of program */
	options = GMT_Create_Options(API, mode, args);
	if (API->error)
		return (API->error); /* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE)
		bailout(GMT_mbgrdtiff_usage(API, GMT_USAGE)); /* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS)
		bailout(GMT_mbgrdtiff_usage(API, GMT_SYNOPSIS)); /* Return the synopsis */

		/* Parse the command-line arguments */

#if GMT_MAJOR_VERSION >= 6
	GMT = gmt_init_module(API, THIS_MODULE_LIB, THIS_MODULE_NAME, "", "", NULL, &options, &GMT_cpy); /* Save current state */
#else
	GMT = gmt_begin_module(API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
#endif
	if (GMT_Parse_Common(API, GMT_PROG_OPTIONS, options))
		Return(API->error);
	Ctrl = New_mbgrdtiff_Ctrl(GMT); /* Allocate and initialize a new control structure */
	if ((error = GMT_mbgrdtiff_parse(GMT, Ctrl, options)))
		Return(error);

	/*---------------------------- This is the mbgrdtiff main code ----------------------------*/

	if ((Image = mbgrdtiff_get_image(API, Ctrl)) == NULL) {
		GMT_Report(API, GMT_MSG_ERROR, "Unable to create an output image reference\n");
		Return(API->error);
	}

	header_work = Image->header; /* Final image projected header */
	tiff_image = Image->data;    /* Final 1 or 3-band image */
	image_size = header_work->nm * header_work->n_bands;

	/*------------------------- Write out the GeoTiff and world files -------------------------*/

	/* try to get projection from the grd file remark */
	if (strncmp(&(header_work->remark[2]), "Projection: ", 12) == 0) {
		if ((nscan = sscanf(&(header_work->remark[2]), "Projection: UTM%d%c", &utmzone, &NorS)) == 2) {
			if (NorS == 'N') {
				projectionid = 32600 + utmzone;
			}
			else if (NorS == 'S') {
				projectionid = 32700 + utmzone;
			}
			modeltype = ModelTypeProjected;
			sprintf(projectionname, "UTM%2.2d%c", utmzone, NorS);
		}
		else if ((nscan = sscanf(&(header_work->remark[2]), "Projection: epsg%d", &projectionid)) == 1) {
			sprintf(projectionname, "epsg%d", projectionid);
			modeltype = ModelTypeProjected;
		}
		else if (strncmp(&(header_work->remark[2]), "Projection: SeismicProfile", 26) == 0) {
			sprintf(projectionname, "SeismicProfile");
			modeltype = ModelTypeProjected;
		}
		else {
			strcpy(projectionname, "Geographic WGS84");
			modeltype = ModelTypeGeographic;
			projectionid = GCS_WGS_84;
		}
	}
	else {
		strcpy(projectionname, "Geographic WGS84");
		modeltype = ModelTypeGeographic;
		projectionid = GCS_WGS_84;
	}

	/* apply shift or "nudge" to grid bounds so that the GeoTiff location is shifted
	    as desired - the nudge_x and nudge_y values are defined in meters and must
	    be translated to the image bounds coordinates */
	if (Ctrl->Nudge.active == true) {
		/* geographic coordinates so convert Ctrl->Nudge.nudge_x and Ctrl->Nudge.nudge_y to degrees lon and lat */
		if (modeltype == ModelTypeGeographic) {
			mb_coor_scale(0, 0.5 * (header_work->wesn[YLO] + header_work->wesn[YHI]), &mtodeglon, &mtodeglat);
			header_work->wesn[XLO] += Ctrl->Nudge.nudge_x * mtodeglon;
			header_work->wesn[XHI] += Ctrl->Nudge.nudge_x * mtodeglon;
			header_work->wesn[YLO] += Ctrl->Nudge.nudge_y * mtodeglat;
			header_work->wesn[YHI] += Ctrl->Nudge.nudge_y * mtodeglat;
		}
		else {
			header_work->wesn[XLO] += Ctrl->Nudge.nudge_x;
			header_work->wesn[XHI] += Ctrl->Nudge.nudge_x;
			header_work->wesn[YLO] += Ctrl->Nudge.nudge_y;
			header_work->wesn[YHI] += Ctrl->Nudge.nudge_y;
		}
	}

	/* Google Earth Pro requires GeoTiffs longitude to be in -180 to +180 domain
	 * make sure geographic images have the origin in the right domain unless
	 * that does not make sense so that most images will work with Google Earth Pro */
	if (modeltype == ModelTypeGeographic && header_work->wesn[XLO] > 180.0 && header_work->wesn[XHI] > 180.0) {
		header_work->wesn[XLO] -= 360.0;
		header_work->wesn[XHI] -= 360.0;
	}
	if (modeltype == ModelTypeGeographic && header_work->wesn[XLO] < -180.0 && header_work->wesn[XHI] < -180.0) {
		header_work->wesn[XLO] += 360.0;
		header_work->wesn[XHI] += 360.0;
	}

	/* set the TIFF comment */
	sprintf(tiff_comment, "Image generated by %s|", program_name);

	/* set the TIFF header */
	memset(tiff_header, 0, TIFF_HEADER_SIZE);
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
	for (i = 0; i < NUMBER_TAGS; i++) {
		mb_put_binary_short(MB_NO, tiff_tag[i], &tiff_header[index]);
		index += 2;
		mb_put_binary_short(MB_NO, tiff_type[i], &tiff_header[index]);
		index += 2;

		switch (tiff_tag[i]) {
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
			value_int = header_work->n_columns;
			mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
			index += 4;
			break;
		case ImageLength:
			value_int = 1;
			mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
			index += 4;
			value_int = header_work->n_rows;
			mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
			index += 4;
			break;
		case BitsPerSample:
			if (header_work->n_bands == 1) {
				value_int = 1;
				mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
				index += 4;
				value_short = 8;
				mb_put_binary_short(MB_NO, value_short, &tiff_header[index]);
				index += 4;
			}
			else {
				value_int = 3;
				mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
				index += 4;
				value_int = tiff_offset[i];
				mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
				index += 4;
				value_short = 8;
				mb_put_binary_short(MB_NO, value_short, &tiff_header[tiff_offset[i]]);
				mb_put_binary_short(MB_NO, value_short, &tiff_header[tiff_offset[i] + 2]);
				mb_put_binary_short(MB_NO, value_short, &tiff_header[tiff_offset[i] + 4]);
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
			if (header_work->n_bands == 1) {
				value_short = 1;
			}
			else {
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
			value_short = header_work->n_bands;
			mb_put_binary_short(MB_NO, value_short, &tiff_header[index]);
			index += 4;
			break;
		case RowsPerStrip:
			value_int = 1;
			mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
			index += 4;
			value_int = header_work->n_rows;
			mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
			index += 4;
			break;
		case StripByteCounts:
			value_int = 1;
			mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
			index += 4;
			if (header_work->n_bands == 1)
				value_int = header_work->n_columns * header_work->n_rows;
			else
				value_int = 3 * header_work->n_columns * header_work->n_rows;
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
			value_int = MAX(header_work->n_columns, header_work->n_rows);
			mb_put_binary_int(MB_NO, value_int, &tiff_header[tiff_offset[i]]);
			value_int = 4;
			mb_put_binary_int(MB_NO, value_int, &tiff_header[tiff_offset[i] + 4]);
			break;
		case YResolution:
			value_int = 1;
			mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
			index += 4;
			value_int = tiff_offset[i];
			mb_put_binary_int(MB_NO, value_int, &tiff_header[index]);
			index += 4;
			value_int = MAX(header_work->n_columns, header_work->n_rows);
			mb_put_binary_int(MB_NO, value_int, &tiff_header[tiff_offset[i]]);
			value_int = 4;
			mb_put_binary_int(MB_NO, value_int, &tiff_header[tiff_offset[i] + 4]);
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
			value_double = header_work->inc[0];
			mb_put_binary_double(MB_NO, value_double, &tiff_header[tiff_offset[i]]);
			value_double = header_work->inc[1];
			mb_put_binary_double(MB_NO, value_double, &tiff_header[tiff_offset[i] + 8]);
			value_double = 0.0;
			mb_put_binary_double(MB_NO, value_double, &tiff_header[tiff_offset[i] + 16]);
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
			value_double = header_work->wesn[XLO] - 0.5 * header_work->inc[0];
			mb_put_binary_double(MB_NO, value_double, &tiff_header[tiff_offset[i] + 3 * 8]);
			value_double = header_work->wesn[YHI] + 0.5 * header_work->inc[1];
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

			if (modeltype == ModelTypeGeographic) {
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

			else if (modeltype == ModelTypeProjected) {
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

	/* open TIFF file */
	if ((tfp = fopen(Ctrl->O.file, "w")) == NULL) {
		API->error++;
		return (API->error);
	}

	/* set the TIFF comment */
	sprintf(tiff_comment, "Image generated by %s|", program_name);

	/* write the header */
	if ((write_size = fwrite(tiff_header, 1, TIFF_HEADER_SIZE, tfp)) != TIFF_HEADER_SIZE) {
		API->error++;
		fclose(tfp);
		return (API->error);
	}

	/* write the image */
	if ((write_size = fwrite(tiff_image, 1, image_size, tfp)) != image_size) {
		API->error++;
		fclose(tfp);
		return (API->error);
	}

	/* close the tiff file */
	fclose(tfp);

	/* open world file */
	strcpy(world_file, Ctrl->O.file);
	world_file[strlen(Ctrl->O.file) - 4] = '\0';
	strcat(world_file, ".tfw");
	if ((tfp = fopen(world_file, "w")) == NULL) {
		API->error++;
		return (API->error);
	}

	/* write out world file contents */
	fprintf(tfp, "%.9f\r\n0.0\r\n0.0\r\n%.9f\r\n%.9f\r\n%.9f\r\n", header_work->inc[GMT_X], -header_work->inc[GMT_Y],
	        header_work->wesn[XLO] - 0.5 * header_work->inc[GMT_X], header_work->wesn[YHI] + 0.5 * header_work->inc[GMT_Y]);

	/* close the world file */
	fclose(tfp);
	fprintf(stderr, "3 Work header:\n\tnx:%d ny:%d registration:%d\n\tWESN: %f %f %f %f\n\tinc: %f %f\n", header_work->n_columns,
	        header_work->n_rows, header_work->registration, header_work->wesn[XLO], header_work->wesn[XHI],
	        header_work->wesn[YLO], header_work->wesn[YHI], header_work->inc[0], header_work->inc[1]);

	Return(EXIT_SUCCESS);
}
