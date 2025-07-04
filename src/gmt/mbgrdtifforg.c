/*--------------------------------------------------------------------
 *    The MB-system:	mbgrdtiff.c	5/30/93
 *
 *    Copyright (c) 1999-2025 by
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
 *    image generation is similar to that of the GMT program
 *    grdimage. In particular, the color map is applied from
 *    a GMT CPT file, and shading overlay grids may be applied.
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

#include <stdbool.h>

#include "mb_status.h"
#include "mb_format.h"
#include "mb_define.h"

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

unsigned short tiff_tag[] = {
    NewSubfileType, ImageWidth,         ImageLength,      BitsPerSample,      Compression,        PhotometricInterpretation,
    StripOffsets,   SamplesPerPixel,    RowsPerStrip,     StripByteCounts,    XResolution,        YResolution,
    ResolutionUnit, ModelPixelScaleTag, ModelTiepointTag, GeoKeyDirectoryTag, GeoDoubleParamsTag, GeoAsciiParamsTag};
unsigned short tiff_type[] = {
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
int tiff_offset[] = {
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
#define THIS_MODULE_LIB "mbsystem"
#define THIS_MODULE_PURPOSE "Project grids or images and plot them on maps"
#define THIS_MODULE_KEYS ""

// Stop warnings about packaging collision between GDAL's cpl_port.h and mb_config.h
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
/*  Compatibility with old lower-function/macro names use prior to GMT 5.3.0 */
#if GMT_MAJOR_VERSION == 5 && GMT_MINOR_VERSION < 3
#define gmt_M_180_range GMT_180_RANGE
#define gmt_M_free_options GMT_Free_Options
#define gmt_M_ijp GMT_IJP
#define gmt_M_ijpgi GMT_IJPGI
#define gmt_M_check_condition GMT_check_condition
#define gmt_M_get_inc GMT_get_inc
#define gmt_M_get_n GMT_get_n
#define gmt_M_grd_is_global GMT_grd_is_global
#define gmt_M_grd_same_region GMT_grd_same_region
#define gmt_M_is255 GMT_is255
#define gmt_M_is_geographic GMT_is_geographic
#define gmt_M_is_nonlinear_graticule GMT_IS_NONLINEAR_GRATICULE
#define gmt_M_memcpy GMT_memcpy
#define gmt_M_rgb_copy GMT_rgb_copy
#define gmt_M_u255 GMT_u255
#define gmt_M_err_fail GMT_err_fail
#define gmt_M_free GMT_free
#define gmt_M_is_fnan GMT_is_fnan
#define gmt_M_memory GMT_memory
#define gmt_M_yiq GMT_YIQ
#define gmt_get_cpt GMT_Get_CPT
#define gmt_access GMT_access
#define gmt_begin_module GMT_begin_module
#define gmt_check_filearg GMT_check_filearg
#define gmt_default_error GMT_default_error
#define gmt_end_module GMT_end_module
#define gmt_geo_to_xy GMT_geo_to_xy
#define gmt_get_api_ptr GMT_get_API_ptr
#define gmt_get_rgb_from_z GMT_get_rgb_from_z
#define gmt_getrgb GMT_getrgb
#define gmt_grd_project GMT_grd_project
#define gmt_grd_setregion GMT_grd_setregion
#define gmt_illuminate GMT_illuminate
#define gmt_map_basemap GMT_map_basemap
#define gmt_map_clip_off GMT_map_clip_off
#define gmt_map_clip_on GMT_map_clip_on
#define gmt_map_setup GMT_map_setup
#define gmt_not_numeric GMT_not_numeric
#define gmt_plane_perspective GMT_plane_perspective
#define gmt_plotcanvas GMT_plotcanvas
#define gmt_plotend GMT_plotend
#define gmt_plotinit GMT_plotinit
#define gmt_project_init GMT_project_init
#define gmt_putrgb GMT_putrgb
#define gmt_rgb_syntax GMT_rgb_syntax
#define gmt_set_grddim GMT_set_grddim
#define gmt_show_name_and_purpose GMT_show_name_and_purpose
#elif GMT_MAJOR_VERSION == 6
#define gmt_M_grd_is_global gmt_grd_is_global
#endif

EXTERN_MSC int GMT_mbgrdtiff(void *API, int mode, void *args);

#define GMT_PROG_OPTIONS "->JRVn" GMT_OPT("S")

/* Control structure for mbgrdtiff */
struct MBGRDTIFF_CTRL {
	struct A { /* -A to write a GDAL file */
		bool active;
		char *file;
		char *driver;
	} A;
	struct C { /* -C<cptfile> */
		bool active;
		char *file;
	} C;
	struct D { /* -D to read GDAL file */
		bool active;
		bool mode; /* Use info of -R option to reference image */
	} D;
	struct E { /* -Ei|<dpi> */
		bool active;
		bool device_dpi;
		unsigned int dpi;
	} E;
	struct G { /* -G[f|b]<rgb> */
		bool active;
		double f_rgb[4];
		double b_rgb[4];
	} G;
	struct I { /* -I<inputfile>> */
		bool active;
		bool do_rgb;
		char *file[3];
	} I;
	struct Intensity { /* -I<intensfile>|<value> */
		bool active;
		bool constant;
		double value;
		char *file;
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

void *New_mbgrdtiff_Ctrl(struct GMT_CTRL *GMT) { /* Allocate and initialize a new control structure */
	struct MBGRDTIFF_CTRL *Ctrl;

	Ctrl = gmt_M_memory(GMT, NULL, 1, struct MBGRDTIFF_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	Ctrl->G.b_rgb[0] = Ctrl->G.b_rgb[1] = Ctrl->G.b_rgb[2] = 1.0;

	return (Ctrl);
}
/*--------------------------------------------------------------------*/

void Free_mbgrdtiff_Ctrl(struct GMT_CTRL *GMT, struct MBGRDTIFF_CTRL *Ctrl) { /* Deallocate control structure */
	int k;
	if (!Ctrl)
		return;
	for (k = 0; k < 3; k++)
		if (Ctrl->I.file[k])
			free(Ctrl->I.file[k]);
	if (Ctrl->C.file)
		free(Ctrl->C.file);
	if (Ctrl->Intensity.file)
		free(Ctrl->Intensity.file);
	gmt_M_free(GMT, Ctrl);
}
/*--------------------------------------------------------------------*/

int GMT_mbgrdtiff_usage(struct GMTAPI_CTRL *API, int level) {
	gmt_show_name_and_purpose(API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE)
		return (GMT_NOERROR);
	GMT_Message(API, GMT_TIME_NONE, "usage: mbgrdtiff <grd_z>|<grd_r> <grd_g> <grd_b> %s [%s] [-C<cpt>] [-Ei[|<dpi>]]\n",
	            GMT_J_OPT, GMT_B_OPT);
	GMT_Message(API, GMT_TIME_NONE,
	            "\t[-G[f|b]<rgb>] [-I<intensgrid>|<value>] [-K] [-M] [-N<nudge_x>/<nudge_y>] [-O] [-P] [-Q]\n");
	GMT_Message(API, GMT_TIME_NONE, "\t[%s] [-T] [%s] [%s]\n", GMT_Rgeo_OPT, GMT_U_OPT, GMT_V_OPT);
#if GMT_MAJOR_VERSION >= 6
	GMT_Message(API, GMT_TIME_NONE, "\t[%s] [%s] [%s]\n\t[%s]\n\t[%s] [%s]\n\n", GMT_X_OPT, GMT_Y_OPT, GMT_f_OPT, GMT_n_OPT,
	            GMT_p_OPT, GMT_t_OPT);
#else
	GMT_Message(API, GMT_TIME_NONE, "\t[%s] [%s] [%s] [%s]\n\t[%s]\n\t[%s] [%s]\n\n", GMT_X_OPT, GMT_Y_OPT, GMT_c_OPT, GMT_f_OPT,
	            GMT_n_OPT, GMT_p_OPT, GMT_t_OPT);
#endif

	if (level == GMT_SYNOPSIS)
		return (EXIT_FAILURE);

	GMT_Message(API, GMT_TIME_NONE, "\t<grd_z> is data set to be plotted.  Its z-values are in user units and will be\n");
	GMT_Message(API, GMT_TIME_NONE, "\t  converted to rgb colors via the cpt file.  Alternatively, give three separate\n");
	GMT_Message(API, GMT_TIME_NONE, "\t  grid files that contain the red, green, and blue components in the 0-255 range.\n");
	GMT_Option(API, "J-");
	GMT_Message(API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option(API, "B-");
	GMT_Message(API, GMT_TIME_NONE,
	            "\t-C Color palette file to convert z to rgb.  Optionally, instead give name of a master cpt\n");
	GMT_Message(API, GMT_TIME_NONE, "\t   to automatically assign 16 continuous colors over the data range [rainbow].\n");
	GMT_Message(API, GMT_TIME_NONE, "\t-E Set dpi for the projected grid which must be constructed [100]\n");
	GMT_Message(API, GMT_TIME_NONE, "\t   if -Jx or -Jm is not selected [Default gives same size as input grid].\n");
	GMT_Message(API, GMT_TIME_NONE, "\t   Give i to do the interpolation in PostScript at device resolution.\n");
	gmt_rgb_syntax(API->GMT, 'G', "Set transparency color for images that otherwise would result in 1-bit images.\n\t  ");
	GMT_Message(API, GMT_TIME_NONE, "\t-I Use illumination. Append name of intensity grid file.\n");
	GMT_Message(API, GMT_TIME_NONE, "\t   For a constant intensity, just give the value instead.\n");
	GMT_Option(API, "K");
	GMT_Message(API, GMT_TIME_NONE, "\t-M Force monochrome image.\n");
	GMT_Message(API, GMT_TIME_NONE, "\t-N<nudge_x>/<nudge_y>\n");
	GMT_Option(API, "O,P");
	GMT_Message(API, GMT_TIME_NONE, "\t-Q Use PS Level 3 colormasking to make nodes with z = NaN transparent.\n");
	GMT_Option(API, "R");
	GMT_Option(API, "U,V,X,c,n,t,.");

	return (EXIT_FAILURE);
}
/*--------------------------------------------------------------------*/

int GMT_mbgrdtiff_parse(struct GMT_CTRL *GMT, struct MBGRDTIFF_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to mbgrdtiff and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) { /* Process all the options given */

		switch (opt->option) {
		case '<': /* Input file (only one or three is accepted) */
			Ctrl->I.active = true;
			if (n_files >= 3)
				break;
#if GMT_MAJOR_VERSION < 5 || (GMT_MAJOR_VERSION == 5 && GMT_MINOR_VERSION == 1 && GMT_RELEASE_VERSION < 2)
			if (gmt_check_filearg(GMT, '<', opt->arg, GMT_IN))
#else
			if (gmt_check_filearg(GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET))
#endif
				Ctrl->I.file[n_files++] = strdup(opt->arg);
			else
				n_errors++;
			break;

			/* Processes program-specific parameters */

		case 'C': /* CPT file */
			Ctrl->C.active = true;
			if (Ctrl->C.file)
				free(Ctrl->C.file);
			Ctrl->C.file = strdup(opt->arg);
			break;
		case 'E': /* Sets dpi */
			Ctrl->E.active = true;
			if (opt->arg[0] == 'i') /* Interpolate image to device resolution */
				Ctrl->E.device_dpi = true;
			else if (opt->arg[0] == '\0')
				Ctrl->E.dpi = 100; /* Default grid dpi */
			else
				Ctrl->E.dpi = (int)strtol(opt->arg, NULL, 10);
			break;
		case 'G': /* 1-bit fore or background color for transparent masks */
			Ctrl->G.active = true;
			switch (opt->arg[0]) {
			case 'F':
			case 'f':
				if (gmt_getrgb(GMT, &opt->arg[1], Ctrl->G.f_rgb)) {
					gmt_rgb_syntax(GMT, 'G', " ");
					n_errors++;
				}
				else
					Ctrl->G.b_rgb[0] = -1;
				break;
			case 'B':
			case 'b':
				if (gmt_getrgb(GMT, &opt->arg[1], Ctrl->G.b_rgb)) {
					gmt_rgb_syntax(GMT, 'G', " ");
					n_errors++;
				}
				else
					Ctrl->G.f_rgb[0] = -1;
				break;
			default: /* Same as -Gf */
				if (gmt_getrgb(GMT, opt->arg, Ctrl->G.f_rgb)) {
					gmt_rgb_syntax(GMT, 'G', " ");
					n_errors++;
				}
				else
					Ctrl->G.b_rgb[0] = -1;
				break;
			}
			break;
		case 'I': /* Input file (only one or three is accepted) */
			/* if no grid file specified yet then first -Ifile sets
			 * the primary grid file, and second -Ifile will be the
			 * intensity file */
			if (n_files == 0) {
				Ctrl->I.active = true;
				Ctrl->I.file[n_files++] = strdup(opt->arg);
			}
			else {
				Ctrl->Intensity.active = true;
				if (!gmt_access(GMT, opt->arg, R_OK)) /* Got a file */
					Ctrl->Intensity.file = strdup(opt->arg);
				else if (opt->arg[0] && !gmt_not_numeric(GMT, opt->arg)) { /* Looks like a constant value */
					Ctrl->Intensity.value = strtod(opt->arg, NULL);
					Ctrl->Intensity.constant = true;
				}
				else {
					GMT_Report(API, GMT_MSG_NORMAL, "Syntax error -I: Requires a valid grid file or a constant\n");
					n_errors++;
				}
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

	if (n_files == 3)
		Ctrl->I.do_rgb = true;
	//	n_errors += gmt_M_check_condition (GMT, !GMT->common.J.active,
	//					"Syntax error: Must specify a map projection with the -J option\n");
	n_errors += gmt_M_check_condition(GMT, !Ctrl->C.file && !Ctrl->I.do_rgb, "Syntax error: Must specify color palette table\n");
	n_errors +=
	    gmt_M_check_condition(GMT, !(n_files == 1 || n_files == 3), "Syntax error: Must specify one (or three) input file(s)\n");
	n_errors += gmt_M_check_condition(GMT, Ctrl->Intensity.active && !Ctrl->Intensity.constant && !Ctrl->Intensity.file,
	                                  "Syntax error -I option: Must specify intensity file or value\n");
	n_errors += gmt_M_check_condition(GMT, Ctrl->E.active && !Ctrl->E.device_dpi && Ctrl->E.dpi <= 0,
	                                  "Syntax error -E option: dpi must be positive\n");
	n_errors +=
	    gmt_M_check_condition(GMT, Ctrl->G.f_rgb[0] < 0 && Ctrl->G.b_rgb[0] < 0,
	                          "Syntax error -G option: Only one of fore/back-ground can be transparent for 1-bit images\n");
	n_errors += gmt_M_check_condition(GMT, Ctrl->M.active && Ctrl->Q.active,
	                                  "Syntax error -Q option:  Cannot use -M when doing colormasking\n");
	n_errors += gmt_M_check_condition(GMT, !Ctrl->O.active || !Ctrl->O.file,
	                                  "Syntax error -O option: Must specify the output file name.\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}
/*--------------------------------------------------------------------*/

void GMT_mbgrdtiff_set_proj_limits(struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *r, struct GMT_GRID_HEADER *g, bool projected) {
	/* Sets the projected extent of the grid given the map projection
	 * The extreme x/y coordinates are returned in r, and dx/dy, and
	 * nx/ny are set accordingly.  Not that some of these may change
	 * if gmt_project_init is called at a later stage */

	r->n_columns = g->n_columns;
	r->n_rows = g->n_rows;
	r->registration = g->registration;
	r->n_bands = g->n_bands;

	/* By default, use entire plot region */

	gmt_M_memcpy(r->wesn, GMT->current.proj.rect, 4, double);

	if (GMT->current.proj.projection == GMT_GENPER && GMT->current.proj.g_width != 0.0)
		return;

	bool all_lats = false;
	bool all_lons = false;

	if (gmt_M_is_geographic(GMT, GMT_IN)) {
		all_lats = gmt_M_180_range(g->wesn[YHI], g->wesn[YLO]);
		all_lons = gmt_M_grd_is_global(GMT, g);
		if (all_lons && all_lats)
			return; /* Whole globe */
	}

	/* Must search for extent along perimeter */

	r->wesn[XLO] = r->wesn[YLO] = +DBL_MAX;
	r->wesn[XHI] = r->wesn[YHI] = -DBL_MAX;
	const unsigned int k = (g->registration == GMT_GRID_NODE_REG) ? 1 : 0;

	double x;
	double y;

	for (unsigned int i = 0; i < g->n_columns - k; i++) { /* South and north sides */
		gmt_geo_to_xy(GMT, g->wesn[XLO] + i * g->inc[GMT_X], g->wesn[YLO], &x, &y);
		r->wesn[XLO] = MIN(r->wesn[XLO], x), r->wesn[XHI] = MAX(r->wesn[XHI], x);
		r->wesn[YLO] = MIN(r->wesn[YLO], y), r->wesn[YHI] = MAX(r->wesn[YHI], y);
		gmt_geo_to_xy(GMT, g->wesn[XHI] - i * g->inc[GMT_X], g->wesn[YHI], &x, &y);
		r->wesn[XLO] = MIN(r->wesn[XLO], x), r->wesn[XHI] = MAX(r->wesn[XHI], x);
		r->wesn[YLO] = MIN(r->wesn[YLO], y), r->wesn[YHI] = MAX(r->wesn[YHI], y);
	}
	for (unsigned int i = 0; i < g->n_rows - k; i++) { /* East and west sides */
		gmt_geo_to_xy(GMT, g->wesn[XLO], g->wesn[YHI] - i * g->inc[GMT_Y], &x, &y);
		r->wesn[XLO] = MIN(r->wesn[XLO], x), r->wesn[XHI] = MAX(r->wesn[XHI], x);
		r->wesn[YLO] = MIN(r->wesn[YLO], y), r->wesn[YHI] = MAX(r->wesn[YHI], y);
		gmt_geo_to_xy(GMT, g->wesn[XHI], g->wesn[YLO] + i * g->inc[GMT_Y], &x, &y);
		r->wesn[XLO] = MIN(r->wesn[XLO], x), r->wesn[XHI] = MAX(r->wesn[XHI], x);
		r->wesn[YLO] = MIN(r->wesn[YLO], y), r->wesn[YHI] = MAX(r->wesn[YHI], y);
	}
	if (projected) {
		if (all_lons) { /* Full 360, use min/max for x */
			r->wesn[XLO] = GMT->current.proj.rect[XLO];
			r->wesn[XHI] = GMT->current.proj.rect[XHI];
		}
		if (all_lats) { /* Full -90/+90, use min/max for y */
			r->wesn[YLO] = GMT->current.proj.rect[YLO];
			r->wesn[YHI] = GMT->current.proj.rect[YHI];
		}
	}
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
	static const char program_name[] = "mbgrdtiff";

	// struct PSL_CTRL *PSL = NULL;                      /* General PSL interal parameters */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr(V_API); /* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL)
		return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE)
		return (GMT_mbgrdtiff_usage(API, GMT_MODULE_PURPOSE)); /* Return the purpose of program */
	struct GMT_OPTION *options = GMT_Create_Options(API, mode, args);
	if (API->error)
		return (API->error); /* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE)
		bailout(GMT_mbgrdtiff_usage(API, GMT_USAGE)); /* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS)
		bailout(GMT_mbgrdtiff_usage(API, GMT_SYNOPSIS)); /* Return the synopsis */

	/* Parse the command-line arguments */

	struct GMT_CTRL *GMT_cpy = NULL;
#if GMT_MAJOR_VERSION >= 6 && GMT_MINOR_VERSION >= 1
	struct GMT_CTRL *GMT =
	    gmt_init_module(API, THIS_MODULE_LIB, THIS_MODULE_NAME, "", "", NULL, &options, &GMT_cpy); /* Save current state */
#elif GMT_MAJOR_VERSION >= 6
	struct GMT_CTRL *GMT =
	    gmt_init_module(API, THIS_MODULE_LIB, THIS_MODULE_NAME, "", "", &options, &GMT_cpy); /* Save current state */
#else
	struct GMT_CTRL *GMT = gmt_begin_module(API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
#endif

	struct MBGRDTIFF_CTRL *Ctrl = NULL;
	if (GMT_Parse_Common(API, GMT_PROG_OPTIONS, options))
		Return(API->error);
	Ctrl = New_mbgrdtiff_Ctrl(GMT); /* Allocate and initialize a new control structure */
	int error = GMT_mbgrdtiff_parse(GMT, Ctrl, options);
	if (error)
		Return(error);

	/*---------------------------- This is the mbgrdtiff main code ----------------------------*/

	const unsigned int n_grids = (Ctrl->I.do_rgb) ? 3 : 1;
	const bool use_intensity_grid = (Ctrl->Intensity.active && !Ctrl->Intensity.constant); /* We want to use the intensity grid */

	/* Read the illumination grid header right away so we can use its region to set that of an image (if requested) */
	struct GMT_GRID *Intens_orig = NULL;

	if (use_intensity_grid) { /* Illumination grid wanted */
		GMT_Report(API, GMT_MSG_VERBOSE, "Allocates memory and read intensity file\n");

		if ((Intens_orig = GMT_Read_Data(API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL,
		                                 Ctrl->Intensity.file, NULL)) == NULL) { /* Get header only */
			Return(API->error);
		}
	}

	GMT_Report(API, GMT_MSG_VERBOSE, "Allocates memory and read data file\n");

	struct GMT_GRID *Grid_orig[3] = {NULL, NULL, NULL};

	if (!Ctrl->D.active) {
		for (unsigned int k = 0; k < n_grids; k++) {
			if ((Grid_orig[k] = GMT_Read_Data(API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL,
			                                  Ctrl->I.file[k], NULL)) == NULL) { /* Get header only */
				Return(API->error);
			}
		}
		if (!Ctrl->C.active)
			Ctrl->C.active = true; /* Use default CPT stuff */
	}

	struct GMT_GRID_HEADER *header_work = NULL; /* Pointer to a GMT header for the image or grid */
	if (n_grids)
		header_work = Grid_orig[0]->header; /* OK, we are in GRID mode and this was not set further above. Do it now */

	if (n_grids && Ctrl->I.do_rgb) { /* Must ensure all three grids are coregistered */
		if (!gmt_M_grd_same_region(GMT, Grid_orig[0], Grid_orig[1]))
			error++;
		if (!gmt_M_grd_same_region(GMT, Grid_orig[0], Grid_orig[2]))
			error++;
		if (!(Grid_orig[0]->header->inc[GMT_X] == Grid_orig[1]->header->inc[GMT_X] &&
		      Grid_orig[0]->header->inc[GMT_X] == Grid_orig[2]->header->inc[GMT_X]))
			error++;
		if (!(Grid_orig[0]->header->n_columns == Grid_orig[1]->header->n_columns &&
		      Grid_orig[0]->header->n_columns == Grid_orig[2]->header->n_columns))
			error++;
		if (!(Grid_orig[0]->header->n_rows == Grid_orig[1]->header->n_rows &&
		      Grid_orig[0]->header->n_rows == Grid_orig[2]->header->n_rows))
			error++;
		if (!(Grid_orig[0]->header->registration == Grid_orig[1]->header->registration &&
		      Grid_orig[0]->header->registration == Grid_orig[2]->header->registration))
			error++;
		if (error) {
			GMT_Report(API, GMT_MSG_NORMAL, "The r, g, and b grids are not congruent\n");
			Return(EXIT_FAILURE);
		}
	}

	/* Determine what wesn to pass to map_setup */

#if GMT_MAJOR_VERSION == 5 && GMT_MINOR_VERSION <= 3
	if (!GMT->common.R.active && n_grids)
		gmt_M_memcpy(GMT->common.R.wesn, Grid_orig[0]->header->wesn, 4, double);
#else
	if (!GMT->common.R.active[RSET] && n_grids)
		gmt_M_memcpy(GMT->common.R.wesn, Grid_orig[0]->header->wesn, 4, double);
#endif
	gmt_M_err_fail(GMT, gmt_map_setup(GMT, GMT->common.R.wesn), "");

	/* Determine if grid is to be projected */

	const bool need_to_project = (gmt_M_is_nonlinear_graticule(GMT) || Ctrl->E.dpi > 0);
	if (need_to_project)
		GMT_Report(API, GMT_MSG_DEBUG, "Projected grid is non-orthogonal, nonlinear, or dpi was changed\n");

	/* Determine the wesn to be used to read the grid file; or bail if file is outside -R */

	bool nothing_inside = false;
	double wesn[4];
	if (!gmt_grd_setregion(GMT, header_work, wesn, need_to_project * GMT->common.n.interpolant))
		nothing_inside = true;
	else if (use_intensity_grid &&
	         !gmt_grd_setregion(GMT, Intens_orig->header, wesn, need_to_project * GMT->common.n.interpolant))
		nothing_inside = true;

	if (nothing_inside) {
		/* No grid to plot; just do empty map and bail */
		// struct PSL_CTRL *PSL =
		gmt_plotinit(GMT, options);
		gmt_plane_perspective(GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
		gmt_plotcanvas(GMT); /* Fill canvas if requested */
		gmt_map_basemap(GMT);
		gmt_plane_perspective(GMT, -1, 0.0);
		gmt_plotend(GMT);
		Return(EXIT_SUCCESS);
	}

	unsigned int nx = 0;
	unsigned int ny = 0;
	if (n_grids) {
		nx = gmt_M_get_n(GMT, wesn[XLO], wesn[XHI], Grid_orig[0]->header->inc[GMT_X], Grid_orig[0]->header->registration);
		ny = gmt_M_get_n(GMT, wesn[YLO], wesn[YHI], Grid_orig[0]->header->inc[GMT_Y], Grid_orig[0]->header->registration);
	}

	/* Read data */

	for (unsigned int k = 0; k < n_grids; k++) {
		if (GMT_Read_Data(API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, wesn, Ctrl->I.file[k],
		                  Grid_orig[k]) == NULL) { /* Get grid data */
			Return(API->error);
		}
	}

	/* If given, get intensity file or compute intensities */

	if (use_intensity_grid) { /* Illumination wanted */

		GMT_Report(API, GMT_MSG_VERBOSE, "Allocates memory and read intensity file\n");

		/* Remember, the illumination header was already read at the top */
		if (GMT_Read_Data(API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, wesn, Ctrl->Intensity.file,
		                  Intens_orig) == NULL) {
			Return(API->error); /* Get grid data */
		}
		if (n_grids && (Intens_orig->header->n_columns != Grid_orig[0]->header->n_columns ||
		                Intens_orig->header->n_rows != Grid_orig[0]->header->n_rows)) {
			GMT_Report(API, GMT_MSG_NORMAL, "Intensity file has improper dimensions!\n");
			Return(EXIT_FAILURE);
		}
	}

	struct GMT_GRID *Grid_proj[3] = {NULL, NULL, NULL};
	struct GMT_GRID *Intens_proj = NULL;
	// bool resampled = false;

	unsigned int grid_registration = GMT_GRID_NODE_REG;

	if (need_to_project) { /* Need to resample the grd file */
		int nx_proj = 0, ny_proj = 0;
		double inc[2] = {0.0, 0.0};
		GMT_Report(API, GMT_MSG_VERBOSE, "project grid files\n");

		if (Ctrl->E.dpi == 0) { /* Use input # of nodes as # of projected nodes */
			nx_proj = nx;
			ny_proj = ny;
		}
		for (unsigned int k = 0; k < n_grids; k++) {
			if (!Grid_proj[k] && (Grid_proj[k] = GMT_Duplicate_Data(API, GMT_IS_GRID, GMT_DUPLICATE_NONE, Grid_orig[k])) == NULL)
				Return(API->error); /* Just to get a header we can change */

			GMT_mbgrdtiff_set_proj_limits(GMT, Grid_proj[k]->header, Grid_orig[k]->header, need_to_project);
			if (grid_registration == GMT_GRID_NODE_REG) /* Force pixel if dpi is set */
				grid_registration = (Ctrl->E.dpi > 0) ? GMT_GRID_PIXEL_REG : Grid_orig[k]->header->registration;
			gmt_M_err_fail(GMT,
			               gmt_project_init(GMT, Grid_proj[k]->header, inc, nx_proj, ny_proj, Ctrl->E.dpi, grid_registration),
			               Ctrl->I.file[k]);
			gmt_set_grddim(GMT, Grid_proj[k]->header);
			if (GMT_Create_Data(API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, NULL, NULL, NULL, 0, 0, Grid_proj[k]) ==
			    NULL)
				Return(API->error);
			gmt_grd_project(GMT, Grid_orig[k], Grid_proj[k], false);
			if (GMT_Destroy_Data(API, &Grid_orig[k]) != GMT_OK) {
				Return(API->error);
			}
		}
		if (use_intensity_grid) {
			if ((Intens_proj = GMT_Duplicate_Data(API, GMT_IS_GRID, GMT_DUPLICATE_NONE, Intens_orig)) == NULL)
				Return(API->error); /* Just to get a header we can change */
			if (n_grids)
				gmt_M_memcpy(Intens_proj->header->wesn, Grid_proj[0]->header->wesn, 4, double);
			if (Ctrl->E.dpi == 0) { /* Use input # of nodes as # of projected nodes */
				nx_proj = Intens_orig->header->n_columns;
				ny_proj = Intens_orig->header->n_rows;
			}
			gmt_M_err_fail(GMT, gmt_project_init(GMT, Intens_proj->header, inc, nx_proj, ny_proj, Ctrl->E.dpi, grid_registration),
			               Ctrl->Intensity.file);
			gmt_set_grddim(GMT, Intens_proj->header);
			if (GMT_Create_Data(API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, NULL, NULL, NULL, 0, 0, Intens_proj) ==
			    NULL)
				Return(API->error);
			gmt_grd_project(GMT, Intens_orig, Intens_proj, false);
			if (GMT_Destroy_Data(API, &Intens_orig) != GMT_OK) {
				Return(API->error);
			}
		}
		// resampled = true;
	}
	else { /* Simply set Grid_proj[i]/Intens_proj to point to Grid_orig[i]/Intens_orig */
		struct GMT_GRID_HEADER tmp_header;
		for (unsigned int k = 0; k < n_grids; k++) { /* Must get a copy so we can change one without affecting the other */
			gmt_M_memcpy(&tmp_header, Grid_orig[k]->header, 1, struct GMT_GRID_HEADER);
			Grid_proj[k] = Grid_orig[k];
			// GMT_mbgrdtiff_set_proj_limits (GMT, Grid_proj[k]->header, &tmp_header, need_to_project);
		}
		if (use_intensity_grid)
			Intens_proj = Intens_orig;
		if (n_grids)
			grid_registration = Grid_orig[0]->header->registration;
	}

	if (n_grids) {
		Grid_proj[0]->header->n_bands = 1;
		header_work = Grid_proj[0]->header; /* Later when need to refer to the header, use this copy */
	}

	uint64_t nm = header_work->nm;
	nx = header_work->n_columns;
	ny = header_work->n_rows;

	struct GMT_PALETTE *P = NULL;

	bool gray_only = false;

	/* Get/calculate a color palette file */
	if (!Ctrl->I.do_rgb) {
		if (Ctrl->C.active) { /* Read palette file */
#if GMT_MAJOR_VERSION < 6
			if ((P = gmt_get_cpt(GMT, Ctrl->C.file, GMT_CPT_OPTIONAL, header_work->z_min, header_work->z_max)) == NULL) {
				Return(API->error);
			}
#elif GMT_MAJOR_VERSION == 6 && GMT_MINOR_VERSION == 0
			if ((P = gmt_get_palette(GMT, Ctrl->C.file, GMT_CPT_OPTIONAL, header_work->z_min, header_work->z_max, 0.0, 0)) ==
			    NULL) {
				Return(API->error);
			}
#else
			if ((P = gmt_get_palette(GMT, Ctrl->C.file, GMT_CPT_OPTIONAL, header_work->z_min, header_work->z_max, 0.0)) == NULL) {
				Return(API->error);
			}
#endif
			gray_only = (P && P->is_gray);
		}
	}

	if (P && P->has_pattern)
		GMT_Report(API, GMT_MSG_VERBOSE, "Warning: Patterns in cpt file only apply to -T\n");
	GMT_Report(API, GMT_MSG_VERBOSE, "Evaluate pixel colors\n");

	const double red[4] = {1.0, 0.0, 0.0, 0.0};
	const double *NaN_rgb = P ? P->bfn[GMT_NAN].rgb : GMT->current.setting.color_patch[GMT_NAN];
	unsigned char *rgb_used = NULL;
	if (Ctrl->Q.active) {
		if (gray_only) {
			GMT_Report(API, GMT_MSG_VERBOSE,
			           "Your image is grayscale only but -Q requires 24-bit; image will be converted to 24-bit.\n");
			gray_only = false;
			NaN_rgb = red; /* Arbitrarily pick red as the NaN color since image is gray only */
			gmt_M_memcpy(P->bfn[GMT_NAN].rgb, red, 4, double);
		}
		rgb_used = gmt_M_memory(GMT, NULL, 256 * 256 * 256, unsigned char);
	}

	size_t image_size = 0;
	void *tiff_image = NULL;
	unsigned int colormask_offset = 0;

	unsigned char *bitimage_8 = NULL;
	unsigned char *bitimage_24 = NULL;
	if (Ctrl->M.active || gray_only) {
		image_size = nm;
		bitimage_8 = gmt_M_memory(GMT, NULL, image_size, unsigned char);
		tiff_image = bitimage_8;
	}
	else {
		if (Ctrl->Q.active)
			colormask_offset = 3;
		image_size = 3 * nm + colormask_offset;
		bitimage_24 = gmt_M_memory(GMT, NULL, image_size, unsigned char);
		if (P && Ctrl->Q.active) {
			for (unsigned int k = 0; k < 3; k++)
				bitimage_24[k] = gmt_M_u255(P->bfn[GMT_NAN].rgb[k]);
		}
		tiff_image = bitimage_24;
	}

	// normal_x = !(GMT->current.proj.projection == GMT_LINEAR && !GMT->current.proj.xyz_pos[0] && !resampled);
	// normal_y = !(GMT->current.proj.projection == GMT_LINEAR && !GMT->current.proj.xyz_pos[1] && !resampled);
	const bool normal_x = true;
	const bool normal_y = true;
	// uint64_t node_RGBA = 0; /* uint64_t for the RGB(A) image array. */
	int index = 0;
	double rgb[4] = {0.0, 0.0, 0.0, 0.0};

	bool done = false;
	for (int try = 0; !done && try < 2; try ++) {
		/* Evaluate colors at least once, or twice if -Q and we need to select another NaN color */
		uint64_t byte = colormask_offset;
		for (unsigned int row = 0; row < ny; row++) {
			const unsigned int actual_row = normal_y ? row : ny - row - 1;
			const uint64_t kk = gmt_M_ijpgi(header_work, actual_row, 0);
			// if (Ctrl->D.active && row == 0)
			// 	node_RGBA = kk;                           /* First time per row equals 'node', after grows alone */
			for (unsigned int col = 0; col < nx; col++) { /* Compute rgb for each pixel */
				uint64_t node = kk + (normal_x ? col : nx - col - 1);
				if (Ctrl->I.do_rgb) {
					for (unsigned int k = 0; k < 3; k++) {
						if (gmt_M_is_fnan(Grid_proj[k]->data[node])) { /* If one is NaN they are all assumed to be NaN */
							k = 3;                                     /* To exit the k-loop */
							gmt_M_rgb_copy(rgb, NaN_rgb);
							index = GMT_NAN - 3; /* Ensures no illumination done later */
						}
						else { /* Set color, let index = 0 so illuminate test will work */
							rgb[k] = gmt_M_is255(Grid_proj[k]->data[node]);
							if (rgb[k] < 0.0)
								rgb[k] = 0.0;
							else if (rgb[k] > 1.0)
								rgb[k] = 1.0; /* Clip */
							index = 0;
						}
					}
				}
				else
					index = gmt_get_rgb_from_z(GMT, P, Grid_proj[0]->data[node], rgb);

				if (Ctrl->I.active && index != GMT_NAN - 3) {
					if (!n_grids) { /* Here we are illuminating an image. Must recompute "node" with the gmt_M_ijp macro */
						node = gmt_M_ijp(Intens_proj->header, actual_row, 0) + (normal_x ? col : nx - col - 1);
					}
					if (use_intensity_grid)
						gmt_illuminate(GMT, Intens_proj->data[node], rgb);
					else
						gmt_illuminate(GMT, Ctrl->Intensity.value, rgb);
				}

				if (P && gray_only) /* Color table only has grays, pick r */
					bitimage_8[byte++] = gmt_M_u255(rgb[0]);
				else if (Ctrl->M.active) /* Convert rgb to gray using the gmt_M_yiq transformation */
					bitimage_8[byte++] = gmt_M_u255(gmt_M_yiq(rgb));
				else {
					unsigned char i_rgb[3];
					for (unsigned int k = 0; k < 3; k++)
						bitimage_24[byte++] = i_rgb[k] = gmt_M_u255(rgb[k]);
					if (Ctrl->Q.active && index != GMT_NAN - 3) /* Keep track of all r/g/b combinations used except for NaN */
						rgb_used[(i_rgb[0] * 256 + i_rgb[1]) * 256 + i_rgb[2]] = true;
				}
			}

			// if (!n_grids)
			// 	node_RGBA += header_work->n_bands * (header_work->pad[XLO] + header_work->pad[XHI]);
		}

		if (P && Ctrl->Q.active) { /* Check that we found an unused r/g/b value so colormasking will work OK */
			index = (gmt_M_u255(P->bfn[GMT_NAN].rgb[0]) * 256 + gmt_M_u255(P->bfn[GMT_NAN].rgb[1])) * 256 +
			        gmt_M_u255(P->bfn[GMT_NAN].rgb[2]);
			if (rgb_used[index]) { /* This r/g/b already appears in the image as a non-NaN color; we must find a replacement NaN
				                      color */
				int ks = -1;
				for (index = 0; ks == -1 && index < 256 * 256 * 256; index++)
					if (!rgb_used[index])
						ks = index;
				if (ks == -1) {
					GMT_Report(API, GMT_MSG_NORMAL,
					           "Warning: Colormasking will fail as there is no unused color that can represent transparency\n");
					done = true;
				}
				else { /* Pick the first unused color (i.e., k) and let it play the role of the NaN color for transparency */
					bitimage_24[0] = (unsigned char)(ks >> 16);
					bitimage_24[1] = (unsigned char)((ks >> 8) & 255);
					bitimage_24[2] = (unsigned char)(ks & 255);
					GMT_Report(API, GMT_MSG_VERBOSE, "Warning: transparency color reset from %s to color %d/%d/%d\n",
					           gmt_putrgb(GMT, P->bfn[GMT_NAN].rgb), (int)bitimage_24[0], (int)bitimage_24[1],
					           (int)bitimage_24[2]);
					for (unsigned int k = 0; k < 3; k++)
						P->bfn[GMT_NAN].rgb[k] = gmt_M_is255(bitimage_24[k]); /* Set new NaN color */
				}
			}
		}
		else
			done = true;
	}
	if (Ctrl->Q.active)
		gmt_M_free(GMT, rgb_used);

	for (unsigned int k = 1; k < n_grids; k++) { /* Not done with Grid_proj[0] yet, hence we start loop at k = 1 */
		if (need_to_project && GMT_Destroy_Data(API, &Grid_proj[k]) != GMT_OK) {
			GMT_Report(API, GMT_MSG_NORMAL, "Failed to free Grid_proj[k]\n");
		}
	}
	if (use_intensity_grid) {
		if (need_to_project || !n_grids) {
			if (GMT_Destroy_Data(API, &Intens_proj) != GMT_OK) {
				GMT_Report(API, GMT_MSG_NORMAL, "Failed to free Intens_proj\n");
			}
		}
	}

	/* Get actual size of each pixel */
	const double dx =
	    gmt_M_get_inc(GMT, header_work->wesn[XLO], header_work->wesn[XHI], header_work->n_columns, header_work->registration);
	const double dy =
	    gmt_M_get_inc(GMT, header_work->wesn[YLO], header_work->wesn[YHI], header_work->n_rows, header_work->registration);

	/* Set lower left position of image on map */

	// double x0 = header_work->wesn[XLO];
	// double y0 = header_work->wesn[YLO];
	// if (grid_registration == GMT_GRID_NODE_REG) { /* Grid registration, move 1/2 pixel down/left */
	// 	x0 -= 0.5 * dx;
	// 	y0 -= 0.5 * dy;
	// }

	// double x_side = dx * header_work->n_columns;
	// double y_side = dy * header_work->n_rows;

	if (P && gray_only) {
		P->is_bw = true;
		for (uint64_t kk = 0; P->is_bw && kk < nm; kk++)
			if (!(bitimage_8[kk] == 0 || bitimage_8[kk] == 255))
				P->is_bw = false;
	}

	unsigned char *bitimage_1 = NULL;
	if (P && P->is_bw) { /* Can get away with 1 bit image */

		GMT_Report(API, GMT_MSG_VERBOSE, "Creating 1-bit B/W image\n");

		const int nx8 = irint(ceil(nx / 8.0)); /* Image width must equal a multiple of 8 bits */
		// const int nx_pixels = nx8 * 8;
		image_size = nx8 * ny;
		bitimage_1 = gmt_M_memory(GMT, NULL, image_size, unsigned char);
		tiff_image = bitimage_1;

		int k8 = 0;
		unsigned int k = 0;
		for (unsigned int row = 0; row < ny; row++) {
			int shift = 0;
			uint64_t byte = 0;
			for (unsigned int col = 0; col < nx; col++, k++) {
				const int b_or_w = (bitimage_8[k] == 255);
				byte |= b_or_w;
				shift++;
				if (shift == 8) { /* Time to dump out byte */
					bitimage_1[k8++] = (unsigned char)byte;
					byte = shift = 0;
				}
				else
					byte <<= 1; /* Move the bits we have so far 1 step to the left */
			}
			if (shift) { /* Set the remaining bits in this bit to white */
				byte |= 1;
				shift++;
				while (shift < 8) {
					byte <<= 1;
					byte |= 1;
					shift++;
				}
				bitimage_1[k8++] = (unsigned char)byte;
			}
		}

		// x_side = nx_pixels * dx;
		// PSL_plotbitimage (PSL, x0, y0, x_side, y_side, PSL_BL, bit, nx_pixels, ny, Ctrl->G.f_rgb, Ctrl->G.b_rgb);
	}
	else if ((P && gray_only) || Ctrl->M.active) {
		GMT_Report(API, GMT_MSG_VERBOSE, "Creating 8-bit grayshade image\n");
		// PSL_plotcolorimage (PSL, x0, y0, x_side, y_side, PSL_BL, bitimage_8, nx, ny, (Ctrl->E.device_dpi ? -8 : 8));
	}
	else {
		GMT_Report(API, GMT_MSG_VERBOSE, "Creating 24-bit color image\n");
		// PSL_plotcolorimage (PSL, x0, y0, x_side, y_side, PSL_BL, bitimage_24, (Ctrl->Q.active ? -1 : 1) *
		//    nx, ny, (Ctrl->E.device_dpi ? -24 : 24));
	}

	/*------------------------- Write out the GeoTiff and world files -------------------------*/

	int modeltype;
	int projectionid;
	mb_path projectionname;

	/* try to get projection from the grd file remark */
	if (strncmp(&(header_work->remark[2]), "Projection: ", 12) == 0) {
		char NorS;
		int nscan;
		int utmzone;
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
	if (Ctrl->Nudge.active) {
		/* geographic coordinates so convert Ctrl->Nudge.nudge_x and Ctrl->Nudge.nudge_y to degress lon and lat */
		if (modeltype == ModelTypeGeographic) {
			double mtodeglon, mtodeglat;
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
	char tiff_comment[TIFF_COMMENT_MAXLINE];
	sprintf(tiff_comment, "Image generated by %s|", program_name);

	/* set the TIFF header */
	char tiff_header[TIFF_HEADER_SIZE];
	memset(tiff_header, 0, TIFF_HEADER_SIZE);
	index = 0;
	tiff_header[0] = 'M';
	tiff_header[1] = 'M';
	index += 2;
	unsigned short value_short = 42;
	mb_put_binary_short(false, value_short, &tiff_header[index]);
	index += 2;
	int value_int = 8;
	mb_put_binary_int(false, value_int, &tiff_header[index]);
	index += 4;

	/* number of entries in IFD */
	value_short = NUMBER_TAGS;
	mb_put_binary_short(false, value_short, &tiff_header[index]);
	index += 2;

	/* loop over all tags */
	for (int i = 0; i < NUMBER_TAGS; i++) {
		mb_put_binary_short(false, tiff_tag[i], &tiff_header[index]);
		index += 2;
		mb_put_binary_short(false, tiff_type[i], &tiff_header[index]);
		index += 2;

		switch (tiff_tag[i]) {
		case NewSubfileType:
			value_int = 1;
			mb_put_binary_int(false, value_int, &tiff_header[index]);
			index += 4;
			value_int = 0;
			mb_put_binary_int(false, value_int, &tiff_header[index]);
			index += 4;
			break;
		case ImageWidth:
			value_int = 1;
			mb_put_binary_int(false, value_int, &tiff_header[index]);
			index += 4;
			value_int = nx;
			mb_put_binary_int(false, value_int, &tiff_header[index]);
			index += 4;
			break;
		case ImageLength:
			value_int = 1;
			mb_put_binary_int(false, value_int, &tiff_header[index]);
			index += 4;
			value_int = ny;
			mb_put_binary_int(false, value_int, &tiff_header[index]);
			index += 4;
			break;
		case BitsPerSample:
			if ((P && gray_only)) {
				value_int = 1;
				mb_put_binary_int(false, value_int, &tiff_header[index]);
				index += 4;
				value_short = 8;
				mb_put_binary_short(false, value_short, &tiff_header[index]);
				index += 4;
			}
			else {
				value_int = 3;
				mb_put_binary_int(false, value_int, &tiff_header[index]);
				index += 4;
				value_int = tiff_offset[i];
				mb_put_binary_int(false, value_int, &tiff_header[index]);
				index += 4;
				value_short = 8;
				mb_put_binary_short(false, value_short, &tiff_header[tiff_offset[i]]);
				mb_put_binary_short(false, value_short, &tiff_header[tiff_offset[i] + 2]);
				mb_put_binary_short(false, value_short, &tiff_header[tiff_offset[i] + 4]);
			}
			break;
		case Compression:
			value_int = 1;
			mb_put_binary_int(false, value_int, &tiff_header[index]);
			index += 4;
			value_short = 1;
			mb_put_binary_short(false, value_short, &tiff_header[index]);
			index += 4;
			break;
		case PhotometricInterpretation:
			value_int = 1;
			mb_put_binary_int(false, value_int, &tiff_header[index]);
			index += 4;
			if ((P && gray_only)) {
				value_short = 1;
			}
			else {
				value_short = 2;
			}
			mb_put_binary_short(false, value_short, &tiff_header[index]);
			index += 4;
			break;
		case StripOffsets:
			value_int = 1;
			mb_put_binary_int(false, value_int, &tiff_header[index]);
			index += 4;
			value_int = IMAGE_OFFSET;
			mb_put_binary_int(false, value_int, &tiff_header[index]);
			index += 4;
			break;
		case SamplesPerPixel:
			value_int = 1;
			mb_put_binary_int(false, value_int, &tiff_header[index]);
			index += 4;
			if ((P && gray_only))
				value_short = 1;
			else
				value_short = 3;
			mb_put_binary_short(false, value_short, &tiff_header[index]);
			index += 4;
			break;
		case RowsPerStrip:
			value_int = 1;
			mb_put_binary_int(false, value_int, &tiff_header[index]);
			index += 4;
			value_int = ny;
			mb_put_binary_int(false, value_int, &tiff_header[index]);
			index += 4;
			break;
		case StripByteCounts:
			value_int = 1;
			mb_put_binary_int(false, value_int, &tiff_header[index]);
			index += 4;
			if ((P && gray_only))
				value_int = nx * ny;
			else
				value_int = 3 * nx * ny;
			mb_put_binary_int(false, value_int, &tiff_header[index]);
			index += 4;
			break;
		case XResolution:
			value_int = 1;
			mb_put_binary_int(false, value_int, &tiff_header[index]);
			index += 4;
			value_int = tiff_offset[i];
			mb_put_binary_int(false, value_int, &tiff_header[index]);
			index += 4;
			value_int = MAX(nx, ny);
			mb_put_binary_int(false, value_int, &tiff_header[tiff_offset[i]]);
			value_int = 4;
			mb_put_binary_int(false, value_int, &tiff_header[tiff_offset[i] + 4]);
			break;
		case YResolution:
			value_int = 1;
			mb_put_binary_int(false, value_int, &tiff_header[index]);
			index += 4;
			value_int = tiff_offset[i];
			mb_put_binary_int(false, value_int, &tiff_header[index]);
			index += 4;
			value_int = MAX(nx, ny);
			mb_put_binary_int(false, value_int, &tiff_header[tiff_offset[i]]);
			value_int = 4;
			mb_put_binary_int(false, value_int, &tiff_header[tiff_offset[i] + 4]);
			break;
		case ResolutionUnit:
			value_int = 1;
			mb_put_binary_int(false, value_int, &tiff_header[index]);
			index += 4;
			value_short = 2;
			mb_put_binary_short(false, value_short, &tiff_header[index]);
			index += 4;
			break;
		case ModelPixelScaleTag: {
			value_int = 3;
			mb_put_binary_int(false, value_int, &tiff_header[index]);
			index += 4;
			value_int = tiff_offset[i];
			mb_put_binary_int(false, value_int, &tiff_header[index]);
			index += 4;
			double value_double = header_work->inc[0];
			mb_put_binary_double(false, value_double, &tiff_header[tiff_offset[i]]);
			value_double = header_work->inc[1];
			mb_put_binary_double(false, value_double, &tiff_header[tiff_offset[i] + 8]);
			value_double = 0.0;
			mb_put_binary_double(false, value_double, &tiff_header[tiff_offset[i] + 16]);
			break;
		}
		case ModelTiepointTag: {
			value_int = 6;
			mb_put_binary_int(false, value_int, &tiff_header[index]);
			index += 4;
			value_int = tiff_offset[i];
			mb_put_binary_int(false, value_int, &tiff_header[index]);
			index += 4;
			double value_double = 0;
			mb_put_binary_double(false, value_double, &tiff_header[tiff_offset[i]]);
			value_double = 0;
			mb_put_binary_double(false, value_double, &tiff_header[tiff_offset[i] + 1 * 8]);
			value_double = 0;
			mb_put_binary_double(false, value_double, &tiff_header[tiff_offset[i] + 2 * 8]);
			value_double = header_work->wesn[XLO] - 0.5 * header_work->inc[0];
			mb_put_binary_double(false, value_double, &tiff_header[tiff_offset[i] + 3 * 8]);
			value_double = header_work->wesn[YHI] + 0.5 * header_work->inc[1];
			mb_put_binary_double(false, value_double, &tiff_header[tiff_offset[i] + 4 * 8]);

			value_double = 0.0;
			mb_put_binary_double(false, value_double, &tiff_header[tiff_offset[i] + 5 * 8]);
			break;
		}
		case GeoKeyDirectoryTag: {
			value_int = 20;
			mb_put_binary_int(false, value_int, &tiff_header[index]);
			index += 4;
			value_int = tiff_offset[i];
			mb_put_binary_int(false, value_int, &tiff_header[index]);
			index += 4;

			/* index to geotiff geokey directory */
			int keyindex = tiff_offset[i];

			/* geokey directory header
			    (KeyDirectoryVersion, KeyRevision, MinorRevision, NumberOfKeys) */
			value_short = 1;
			mb_put_binary_short(false, value_short, &tiff_header[keyindex]);
			keyindex += 2;
			value_short = 0;
			mb_put_binary_short(false, value_short, &tiff_header[keyindex]);
			keyindex += 2;
			value_short = 2;
			mb_put_binary_short(false, value_short, &tiff_header[keyindex]);
			keyindex += 2;
			value_short = 4;
			mb_put_binary_short(false, value_short, &tiff_header[keyindex]);
			keyindex += 2;

			/* GTModelTypeGeoKey */
			value_short = GTModelTypeGeoKey;
			mb_put_binary_short(false, value_short, &tiff_header[keyindex]);
			keyindex += 2;
			value_short = 0;
			mb_put_binary_short(false, value_short, &tiff_header[keyindex]);
			keyindex += 2;
			value_short = 1;
			mb_put_binary_short(false, value_short, &tiff_header[keyindex]);
			keyindex += 2;
			value_short = modeltype;
			mb_put_binary_short(false, value_short, &tiff_header[keyindex]);
			keyindex += 2;

			/* GTRasterTypeGeoKey */
			value_short = GTRasterTypeGeoKey;
			mb_put_binary_short(false, value_short, &tiff_header[keyindex]);
			keyindex += 2;
			value_short = 0;
			mb_put_binary_short(false, value_short, &tiff_header[keyindex]);
			keyindex += 2;
			value_short = 1;
			mb_put_binary_short(false, value_short, &tiff_header[keyindex]);
			keyindex += 2;
			value_short = RasterPixelIsPoint;
			mb_put_binary_short(false, value_short, &tiff_header[keyindex]);
			keyindex += 2;

			/* GTCitationGeoKey */
			value_short = GTCitationGeoKey;
			mb_put_binary_short(false, value_short, &tiff_header[keyindex]);
			keyindex += 2;
			value_short = GeoAsciiParamsTag;
			mb_put_binary_short(false, value_short, &tiff_header[keyindex]);
			keyindex += 2;
			value_short = strlen(tiff_comment);
			mb_put_binary_short(false, value_short, &tiff_header[keyindex]);
			keyindex += 2;
			value_short = 0;
			mb_put_binary_short(false, value_short, &tiff_header[keyindex]);
			keyindex += 2;

			if (modeltype == ModelTypeGeographic) {
				/* GeographicTypeGeoKey */
				value_short = GeographicTypeGeoKey;
				mb_put_binary_short(false, value_short, &tiff_header[keyindex]);
				keyindex += 2;
				value_short = 0;
				mb_put_binary_short(false, value_short, &tiff_header[keyindex]);
				keyindex += 2;
				value_short = 1;
				mb_put_binary_short(false, value_short, &tiff_header[keyindex]);
				keyindex += 2;
				value_short = projectionid;
				mb_put_binary_short(false, value_short, &tiff_header[keyindex]);
				keyindex += 2;
			}

			else if (modeltype == ModelTypeProjected) {
				/* ProjectedCSTypeGeoKey */
				value_short = ProjectedCSTypeGeoKey;
				mb_put_binary_short(false, value_short, &tiff_header[keyindex]);
				keyindex += 2;
				value_short = 0;
				mb_put_binary_short(false, value_short, &tiff_header[keyindex]);
				keyindex += 2;
				value_short = 1;
				mb_put_binary_short(false, value_short, &tiff_header[keyindex]);
				keyindex += 2;
				value_short = projectionid;
				mb_put_binary_short(false, value_short, &tiff_header[keyindex]);
				keyindex += 2;
			}
			break;
		}
		case GeoDoubleParamsTag:
			value_int = 1;
			mb_put_binary_int(false, value_int, &tiff_header[index]);
			index += 4;
			value_int = tiff_offset[i];
			mb_put_binary_int(false, value_int, &tiff_header[index]);
			index += 4;
			break;
		case GeoAsciiParamsTag:
			value_int = strlen(tiff_comment);
			mb_put_binary_int(false, value_int, &tiff_header[index]);
			index += 4;
			value_int = tiff_offset[i];
			mb_put_binary_int(false, value_int, &tiff_header[index]);
			index += 4;

			/* put in the string */
			strncpy(&tiff_header[tiff_offset[i]], tiff_comment, 64);
			break;
		}
	}

	/* open TIFF file */
	FILE *tfp = fopen(Ctrl->O.file, "w");
	if (tfp == NULL) {
		API->error++;
		return (API->error);
	}

	/* set the TIFF comment */
	sprintf(tiff_comment, "Image generated by %s|", program_name);

	/* write the header */
	if (/* size_t write_size = */ fwrite(tiff_header, 1, TIFF_HEADER_SIZE, tfp) != TIFF_HEADER_SIZE) {
		API->error++;
		fclose(tfp);
		return (API->error);
	}

	/* write the image */
	if (/* size_t write_size = */ fwrite(tiff_image, 1, image_size, tfp) != image_size) {
		API->error++;
		fclose(tfp);
		return (API->error);
	}

	/* close the tiff file */
	fclose(tfp);

	/* open world file */
	mb_path world_file;
	strcpy(world_file, Ctrl->O.file);
	world_file[strlen(Ctrl->O.file) - 4] = '\0';
	strcat(world_file, ".tfw");
	if ((tfp = fopen(world_file, "w")) == NULL) {
		API->error++;
		return (API->error);
	}

	/* write out world file contents */
	fprintf(tfp, "%.9f\r\n0.0\r\n0.0\r\n%.9f\r\n%.9f\r\n%.9f\r\n", dx, -dy, header_work->wesn[XLO] - 0.5 * dx,
	        header_work->wesn[YHI] + 0.5 * dy);

	/* close the world file */
	fclose(tfp);
	fprintf(stderr, "3 Grid header:\n\tnx:%d ny:%d registration:%d\n\tWESN: %f %f %f %f\n\tinc: %f %f\n",
	        Grid_orig[0]->header->n_columns, Grid_orig[0]->header->n_rows, Grid_orig[0]->header->registration,
	        Grid_orig[0]->header->wesn[XLO], Grid_orig[0]->header->wesn[XHI], Grid_orig[0]->header->wesn[YLO],
	        Grid_orig[0]->header->wesn[YHI], Grid_orig[0]->header->inc[0], Grid_orig[0]->header->inc[1]);
	fprintf(stderr, "3 Work header:\n\tnx:%d ny:%d registration:%d\n\tWESN: %f %f %f %f\n\tinc: %f %f\n", header_work->n_columns,
	        header_work->n_rows, header_work->registration, header_work->wesn[XLO], header_work->wesn[XHI],
	        header_work->wesn[YLO], header_work->wesn[YHI], header_work->inc[0], header_work->inc[1]);

	/* Free bitimage arrays. gmt_M_free will not complain if they have not been used (NULL) */
	if (P && P->is_bw)
		gmt_M_free(GMT, bitimage_1);
	if (bitimage_8)
		gmt_M_free(GMT, bitimage_8);
	if (bitimage_24)
		gmt_M_free(GMT, bitimage_24);

	if (need_to_project && n_grids && GMT_Destroy_Data(API, &Grid_proj[0]) != GMT_OK) {
		GMT_Report(API, GMT_MSG_NORMAL, "Failed to free Grid_proj[0]\n");
	}

	if (!Ctrl->C.active && GMT_Destroy_Data(API, &P) != GMT_OK) {
		Return(API->error);
	}
	Return(EXIT_SUCCESS);
}
