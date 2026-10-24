/*--------------------------------------------------------------------
 *    The MB-system:	mb_gisfile.c	10/22/2026
 *
 *    Copyright (c) 2026-2026 by
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
 * Functions to hold GIS vector data (polylines, polygons, and points),
 * to read and write the MB-System polyline, polygon, and point file
 * formats, and to import from and export to ESRI shapefiles using GDAL/OGR.
 * Coordinates are always geographic longitude and latitude (WGS84) in
 * degrees, with z in meters (positive up, as in topography grids).
 *
 * The MB-System formats are ascii text files with "##" comment lines and
 * GMT compatible ">" segment headers, so that they can also be plotted
 * directly with GMT:
 *
 *   ## Polyline File Version 1.00
 *   ## Z values: present|absent
 *   ## LAYER i,layer name
 *   > ## STARTPOLYLINE npart,npoint,color,size,layer,name
 *   > ## PART ipart,npoint
 *   lon,lat,z
 *   > ## ENDPOLYLINE
 *
 *   ## Polygon File Version 1.00
 *   ## LAYER i,layer name
 *   > ## STARTPOLYGON nring,npoint,color,size,fillcolor,layer,name
 *   > ## RING iring,npoint,OUTER
 *   > -Ph ## RING iring,npoint,HOLE
 *   lon,lat,z
 *   > ## ENDPOLYGON
 *
 *   ## Point File Version 1.00
 *   ## LAYER i,layer name
 *   lon,lat,z,color,size,layer,name
 *
 * Author:	D. W. Caress
 * Date:	October 22, 2026
 */

#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_aux.h"
#include "mb_define.h"
#include "mb_status.h"

#include "gdal.h"
#include "gdal_version.h"
#include "ogr_api.h"
#include "ogr_srs_api.h"
#include "cpl_vsi.h"

/* number of array elements to allocate at a time */
#define MB_GIS_ALLOC_NUM 64

/* maximum layer index accepted from a file */
#define MB_GIS_LAYER_MAX 100000

/* DBF text field width limit */
#define MB_GIS_DBF_STRING_MAX 254

/* line buffer length for reading files */
#define MB_GIS_LINE_MAX (2 * MB_PATH_MAXLINE)

/* work arrays used to build a feature while reading */
struct mb_gis_accum_struct {
	int npart;
	int npart_alloc;
	int *partstart;
	int *parttype;
	int npoint;
	int npoint_alloc;
	double *lon;
	double *lat;
	double *z;
};

/*--------------------------------------------------------------------*/
static const char *mb_gis_typename(int type) {
	if (type == MB_GIS_POLYLINE)
		return ("Polyline");
	else if (type == MB_GIS_POLYGON)
		return ("Polygon");
	else if (type == MB_GIS_POINT)
		return ("Point");
	return ("Unknown");
}
/*--------------------------------------------------------------------*/
/* copy a name, replacing line breaks so the name stays on one line */
static void mb_gis_copyname(char *dest, const char *src) {
	snprintf(dest, MB_PATH_MAXLINE, "%s", src != NULL ? src : "");
	for (char *c = dest; *c != '\0'; c++) {
		if (*c == '\r' || *c == '\n')
			*c = ' ';
	}
}
/*--------------------------------------------------------------------*/
/* get the file name without directory and, optionally, extension */
static void mb_gis_basename(const char *file, char *base, bool stripextension) {
	const char *start = file;
	for (const char *c = file; *c != '\0'; c++) {
		if (*c == '/' || *c == '\\')
			start = c + 1;
	}
	snprintf(base, MB_PATH_MAXLINE, "%s", start);
	if (stripextension) {
		char *dot = strrchr(base, '.');
		if (dot != NULL && dot != base)
			*dot = '\0';
	}
}
/*--------------------------------------------------------------------*/
static void mb_gis_accum_reset(struct mb_gis_accum_struct *accum) {
	accum->npart = 0;
	accum->npoint = 0;
}
/*--------------------------------------------------------------------*/
static void mb_gis_accum_free(int verbose, struct mb_gis_accum_struct *accum) {
	int error = MB_ERROR_NO_ERROR;
	mb_freed(verbose, __FILE__, __LINE__, (void **)&accum->partstart, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&accum->parttype, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&accum->lon, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&accum->lat, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&accum->z, &error);
	memset(accum, 0, sizeof(struct mb_gis_accum_struct));
}
/*--------------------------------------------------------------------*/
static int mb_gis_accum_addpart(int verbose, struct mb_gis_accum_struct *accum, int parttype, int *error) {
	int status = MB_SUCCESS;
	if (accum->npart >= accum->npart_alloc) {
		const int nalloc = accum->npart_alloc + MB_GIS_ALLOC_NUM;
		status = mb_reallocd(verbose, __FILE__, __LINE__, nalloc * sizeof(int), (void **)&accum->partstart, error);
		if (status == MB_SUCCESS)
			status = mb_reallocd(verbose, __FILE__, __LINE__, nalloc * sizeof(int), (void **)&accum->parttype, error);
		if (status == MB_SUCCESS) {
			accum->npart_alloc = nalloc;
		}
		else {
			mb_gis_accum_free(verbose, accum);
			return (status);
		}
	}
	accum->partstart[accum->npart] = accum->npoint;
	accum->parttype[accum->npart] = parttype;
	accum->npart++;
	return (status);
}
/*--------------------------------------------------------------------*/
static int mb_gis_accum_addpoint(int verbose, struct mb_gis_accum_struct *accum, double lon, double lat, double z,
                                 int *error) {
	int status = MB_SUCCESS;
	if (accum->npoint >= accum->npoint_alloc) {
		const int nalloc = accum->npoint_alloc > 0 ? 2 * accum->npoint_alloc : MB_GIS_ALLOC_NUM;
		status = mb_reallocd(verbose, __FILE__, __LINE__, nalloc * sizeof(double), (void **)&accum->lon, error);
		if (status == MB_SUCCESS)
			status = mb_reallocd(verbose, __FILE__, __LINE__, nalloc * sizeof(double), (void **)&accum->lat, error);
		if (status == MB_SUCCESS)
			status = mb_reallocd(verbose, __FILE__, __LINE__, nalloc * sizeof(double), (void **)&accum->z, error);
		if (status == MB_SUCCESS) {
			accum->npoint_alloc = nalloc;
		}
		else {
			mb_gis_accum_free(verbose, accum);
			return (status);
		}
	}
	accum->lon[accum->npoint] = lon;
	accum->lat[accum->npoint] = lat;
	accum->z[accum->npoint] = z;
	accum->npoint++;
	return (status);
}
/*--------------------------------------------------------------------*/
static void mb_gis_feature_free(int verbose, struct mb_gisfeature_struct *feature) {
	int error = MB_ERROR_NO_ERROR;
	mb_freed(verbose, __FILE__, __LINE__, (void **)&feature->partstart, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&feature->parttype, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&feature->lon, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&feature->lat, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&feature->z, &error);
	feature->npart = 0;
	feature->npoint = 0;
}
/*--------------------------------------------------------------------*/
int mb_gis_init(int verbose, int type, struct mb_gisdata_struct **gis, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       type:                      %d\n", type);
		fprintf(stderr, "dbg2       gis:                       %p\n", (void *)gis);
	}

	int status = MB_SUCCESS;
	*gis = NULL;
	if (type < MB_GIS_NONE || type > MB_GIS_POINT) {
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_PARAMETER;
	}
	else {
		status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mb_gisdata_struct), (void **)gis, error);
		if (status == MB_SUCCESS) {
			memset(*gis, 0, sizeof(struct mb_gisdata_struct));
			(*gis)->type = type;
			(*gis)->hasz = false;
			*error = MB_ERROR_NO_ERROR;
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       gis:                       %p\n", (void *)*gis);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_gis_deall(int verbose, struct mb_gisdata_struct **gis, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       gis:                       %p\n", (void *)*gis);
	}

	int status = MB_SUCCESS;
	if (*gis != NULL) {
		for (int i = 0; i < (*gis)->nfeature; i++)
			mb_gis_feature_free(verbose, &(*gis)->features[i]);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&(*gis)->features, error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&(*gis)->layers, error);
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)gis, error);
	}
	*error = MB_ERROR_NO_ERROR;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_gis_addlayer(int verbose, struct mb_gisdata_struct *gis, const char *name, int *ilayer, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       gis:                       %p\n", (void *)gis);
		fprintf(stderr, "dbg2       name:                      %s\n", name != NULL ? name : "");
	}

	int status = MB_SUCCESS;
	*ilayer = -1;
	if (gis == NULL) {
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_PARAMETER;
	}
	else if (gis->nlayer >= gis->nlayer_alloc) {
		const int nalloc = gis->nlayer_alloc + MB_GIS_ALLOC_NUM;
		status = mb_reallocd(verbose, __FILE__, __LINE__, nalloc * sizeof(struct mb_gislayer_struct), (void **)&gis->layers,
		                     error);
		if (status == MB_SUCCESS) {
			gis->nlayer_alloc = nalloc;
		}
		else {
			gis->nlayer = 0;
			gis->nlayer_alloc = 0;
		}
	}
	if (status == MB_SUCCESS) {
		mb_gis_copyname(gis->layers[gis->nlayer].name, name);
		*ilayer = gis->nlayer;
		gis->nlayer++;
		*error = MB_ERROR_NO_ERROR;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       ilayer:                    %d\n", *ilayer);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
/* Add a copy of a feature. For polylines and polygons partstart gives the
    index of the first point of each part (NULL allowed when npart is 1) and
    parttype the part types (NULL for the default type). Polygon rings that
    are not closed are closed by repeating their first point. z may be NULL. */
int mb_gis_addfeature(int verbose, struct mb_gisdata_struct *gis, int layer, int color, int size, int fillcolor,
                      const char *name, int npart, const int *partstart, const int *parttype, int npoint, const double *lon,
                      const double *lat, const double *z, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       gis:                       %p\n", (void *)gis);
		fprintf(stderr, "dbg2       layer:                     %d\n", layer);
		fprintf(stderr, "dbg2       color:                     %d\n", color);
		fprintf(stderr, "dbg2       size:                      %d\n", size);
		fprintf(stderr, "dbg2       fillcolor:                 %d\n", fillcolor);
		fprintf(stderr, "dbg2       name:                      %s\n", name != NULL ? name : "");
		fprintf(stderr, "dbg2       npart:                     %d\n", npart);
		fprintf(stderr, "dbg2       npoint:                    %d\n", npoint);
	}

	int status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

	/* check the arguments */
	if (gis == NULL || gis->type == MB_GIS_NONE || layer < 0 || layer >= gis->nlayer || color < 0 || color > MB_GIS_COLOR_MAX ||
	    size < 1 || fillcolor < MB_GIS_COLOR_NONE || fillcolor > MB_GIS_COLOR_MAX || npart < 1 || npoint < 1 || lon == NULL ||
	    lat == NULL || (npart > 1 && partstart == NULL) || (gis->type == MB_GIS_POINT && (npart != 1 || npoint != 1))) {
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_PARAMETER;
	}

	/* check the parts and count the polygon rings that must be closed */
	int nclose = 0;
	for (int i = 0; status == MB_SUCCESS && i < npart; i++) {
		const int start = partstart != NULL ? partstart[i] : 0;
		const int end = i < npart - 1 ? partstart[i + 1] : npoint;
		const int nring = end - start;
		const int type = parttype != NULL ? parttype[i] : (gis->type == MB_GIS_POLYGON ? MB_GIS_PART_OUTER : MB_GIS_PART_LINE);
		if ((i == 0 && start != 0) || nring < 1 || (gis->type == MB_GIS_POLYLINE && (nring < 2 || type != MB_GIS_PART_LINE)) ||
		    (gis->type == MB_GIS_POLYGON && (nring < 3 || (type != MB_GIS_PART_OUTER && type != MB_GIS_PART_HOLE)))) {
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_PARAMETER;
		}
		else if (gis->type == MB_GIS_POLYGON) {
			const bool closed = lon[start] == lon[end - 1] && lat[start] == lat[end - 1];
			if (!closed)
				nclose++;
			else if (nring < 4) {
				status = MB_FAILURE;
				*error = MB_ERROR_BAD_PARAMETER;
			}
		}
	}
	for (int i = 0; status == MB_SUCCESS && i < npoint; i++) {
		if (!isfinite(lon[i]) || !isfinite(lat[i]) || fabs(lat[i]) > 90.0 || (z != NULL && !isfinite(z[i]))) {
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_PARAMETER;
		}
	}

	/* make room for the new feature */
	if (status == MB_SUCCESS && gis->nfeature >= gis->nfeature_alloc) {
		const int nalloc = gis->nfeature_alloc > 0 ? 2 * gis->nfeature_alloc : MB_GIS_ALLOC_NUM;
		status = mb_reallocd(verbose, __FILE__, __LINE__, nalloc * sizeof(struct mb_gisfeature_struct),
		                     (void **)&gis->features, error);
		if (status == MB_SUCCESS) {
			gis->nfeature_alloc = nalloc;
		}
		else {
			gis->nfeature = 0;
			gis->nfeature_alloc = 0;
		}
	}

	/* copy the feature */
	if (status == MB_SUCCESS) {
		struct mb_gisfeature_struct *feature = &gis->features[gis->nfeature];
		memset(feature, 0, sizeof(struct mb_gisfeature_struct));
		feature->layer = layer;
		feature->color = color;
		feature->size = size;
		feature->fillcolor = gis->type == MB_GIS_POLYGON ? fillcolor : MB_GIS_COLOR_NONE;
		mb_gis_copyname(feature->name, name);
		const int npointuse = npoint + nclose;
		status = mb_mallocd(verbose, __FILE__, __LINE__, npart * sizeof(int), (void **)&feature->partstart, error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose, __FILE__, __LINE__, npart * sizeof(int), (void **)&feature->parttype, error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose, __FILE__, __LINE__, npointuse * sizeof(double), (void **)&feature->lon, error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose, __FILE__, __LINE__, npointuse * sizeof(double), (void **)&feature->lat, error);
		if (status == MB_SUCCESS)
			status = mb_mallocd(verbose, __FILE__, __LINE__, npointuse * sizeof(double), (void **)&feature->z, error);
		if (status == MB_SUCCESS) {
			int k = 0;
			for (int i = 0; i < npart; i++) {
				const int start = partstart != NULL ? partstart[i] : 0;
				const int end = i < npart - 1 ? partstart[i + 1] : npoint;
				feature->partstart[i] = k;
				feature->parttype[i] = parttype != NULL ? parttype[i]
				                                        : (gis->type == MB_GIS_POLYGON ? MB_GIS_PART_OUTER : MB_GIS_PART_LINE);
				for (int j = start; j < end; j++) {
					feature->lon[k] = lon[j];
					feature->lat[k] = lat[j];
					feature->z[k] = z != NULL ? z[j] : 0.0;
					k++;
				}
				if (gis->type == MB_GIS_POLYGON && (lon[start] != lon[end - 1] || lat[start] != lat[end - 1])) {
					feature->lon[k] = lon[start];
					feature->lat[k] = lat[start];
					feature->z[k] = z != NULL ? z[start] : 0.0;
					k++;
				}
			}
			feature->npart = npart;
			feature->npoint = k;
			gis->nfeature++;
		}
		else {
			mb_gis_feature_free(verbose, feature);
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       nfeature:                  %d\n", gis != NULL ? gis->nfeature : 0);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
/* get the gis layer index for a layer index read from a file, adding a
    layer if the file did not define it */
static int mb_gis_maplayer(int verbose, struct mb_gisdata_struct *gis, const char *file, int filelayer, int **layermap,
                           int *nlayermap, int *ilayer, int *error) {
	int status = MB_SUCCESS;
	*ilayer = -1;
	if (filelayer < 0 || filelayer > MB_GIS_LAYER_MAX) {
		*error = MB_ERROR_BAD_FORMAT;
		return (MB_FAILURE);
	}
	if (filelayer >= *nlayermap) {
		const int nalloc = filelayer + MB_GIS_ALLOC_NUM;
		status = mb_reallocd(verbose, __FILE__, __LINE__, nalloc * sizeof(int), (void **)layermap, error);
		if (status == MB_FAILURE) {
			*nlayermap = 0;
			return (status);
		}
		for (int i = *nlayermap; i < nalloc; i++)
			(*layermap)[i] = -1;
		*nlayermap = nalloc;
	}
	if ((*layermap)[filelayer] < 0) {
		mb_path name;
		mb_gis_basename(file, name, true);
		if (filelayer > 0) {
			mb_path base;
			snprintf(base, sizeof(base), "%s", name);
			snprintf(name, sizeof(name), "%.1000s %d", base, filelayer);
		}
		status = mb_gis_addlayer(verbose, gis, name, &(*layermap)[filelayer], error);
	}
	*ilayer = (*layermap)[filelayer];
	return (status);
}
/*--------------------------------------------------------------------*/
/* Read a MB-System polyline, polygon, or point file. If gis->type is
    MB_GIS_NONE the type is set from the file header, otherwise the file
    must be of that type. Features and layers are appended to gis. */
int mb_gis_read(int verbose, const char *file, struct mb_gisdata_struct *gis, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       file:                      %s\n", file);
		fprintf(stderr, "dbg2       gis:                       %p\n", (void *)gis);
	}

	int status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;
	if (gis == NULL) {
		*error = MB_ERROR_BAD_PARAMETER;
		return (MB_FAILURE);
	}

	FILE *fp = fopen(file, "r");
	if (fp == NULL) {
		*error = MB_ERROR_OPEN_FAIL;
		return (MB_FAILURE);
	}

	char buffer[MB_GIS_LINE_MAX];
	int nline = 0;
	int filetype = MB_GIS_NONE;
	int version_major = 0;
	int version_minor = 0;
	int *layermap = NULL;
	int nlayermap = 0;
	struct mb_gis_accum_struct accum;
	memset(&accum, 0, sizeof(accum));
	bool infeature = false;
	int expect_npart = 0;
	int expect_npoint = 0;
	int expect_partnpoint = 0;
	int color = 0;
	int size = 1;
	int fillcolor = MB_GIS_COLOR_NONE;
	int filelayer = 0;
	mb_path name = "";
	const int nfeature_start = gis->nfeature;

	while (status == MB_SUCCESS && fgets(buffer, sizeof(buffer), fp) == buffer) {
		nline++;

		/* reject lines too long for the buffer, then strip the line end */
		size_t len = strlen(buffer);
		if (len == sizeof(buffer) - 1 && buffer[len - 1] != '\n' && !feof(fp)) {
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_FORMAT;
			break;
		}
		while (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r'))
			buffer[--len] = '\0';

		/* the first line must identify the file type and version */
		if (filetype == MB_GIS_NONE) {
			for (int type = MB_GIS_POLYLINE; type <= MB_GIS_POINT; type++) {
				char header[64];
				snprintf(header, sizeof(header), "## %s File Version %%d.%%d", mb_gis_typename(type));
				if (sscanf(buffer, header, &version_major, &version_minor) == 2)
					filetype = type;
			}
			if (filetype == MB_GIS_NONE || version_major < 1 || version_major > MB_GISFILE_VERSION_MAJOR ||
			    (gis->type != MB_GIS_NONE && gis->type != filetype)) {
				status = MB_FAILURE;
				*error = MB_ERROR_BAD_FORMAT;
			}
			else {
				gis->type = filetype;
				gis->hasz = true;
			}
		}

		/* z values are present unless the file says otherwise */
		else if (strncmp(buffer, "## Z values: absent", 19) == 0) {
			gis->hasz = false;
		}

		/* layer definition */
		else if (strncmp(buffer, "## LAYER ", 9) == 0) {
			int ilayer;
			mb_path layername = "";
			/* name width is MB_PATH_MAXLINE - 1 */
			if (sscanf(buffer, "## LAYER %d,%1023[^\n]", &filelayer, layername) < 1 || filelayer < 0 ||
			    filelayer > MB_GIS_LAYER_MAX) {
				status = MB_FAILURE;
				*error = MB_ERROR_BAD_FORMAT;
			}
			else if (filelayer < nlayermap && layermap[filelayer] >= 0) {
				/* a repeated definition renames the layer */
				mb_gis_copyname(gis->layers[layermap[filelayer]].name, layername);
			}
			else {
				status = mb_gis_maplayer(verbose, gis, file, filelayer, &layermap, &nlayermap, &ilayer, error);
				if (status == MB_SUCCESS)
					mb_gis_copyname(gis->layers[ilayer].name, layername);
			}
		}

		/* other comments and empty lines */
		else if (buffer[0] == '#' || buffer[0] == '\0') {
		}

		/* point records */
		else if (filetype == MB_GIS_POINT) {
			double lon, lat, z;
			int ilayer;
			name[0] = '\0';
			/* name width is MB_PATH_MAXLINE - 1 */
			const int nget = sscanf(buffer, "%lf,%lf,%lf,%d,%d,%d,%1023[^\n]", &lon, &lat, &z, &color, &size, &filelayer, name);
			if (nget < 6) {
				status = MB_FAILURE;
				*error = MB_ERROR_BAD_FORMAT;
			}
			if (status == MB_SUCCESS)
				status = mb_gis_maplayer(verbose, gis, file, filelayer, &layermap, &nlayermap, &ilayer, error);
			if (status == MB_SUCCESS) {
				status = mb_gis_addfeature(verbose, gis, ilayer, color, size, MB_GIS_COLOR_NONE, name, 1, NULL, NULL, 1, &lon, &lat,
				                           &z, error);
				if (status == MB_FAILURE && *error == MB_ERROR_BAD_PARAMETER)
					*error = MB_ERROR_BAD_FORMAT;
			}
		}

		/* start of a polyline or polygon */
		else if ((filetype == MB_GIS_POLYLINE && strncmp(buffer, "> ## STARTPOLYLINE ", 19) == 0) ||
		         (filetype == MB_GIS_POLYGON && strncmp(buffer, "> ## STARTPOLYGON ", 18) == 0)) {
			int nget;
			name[0] = '\0';
			fillcolor = MB_GIS_COLOR_NONE;
			/* name width is MB_PATH_MAXLINE - 1 */
			if (filetype == MB_GIS_POLYLINE)
				nget = sscanf(buffer, "> ## STARTPOLYLINE %d,%d,%d,%d,%d,%1023[^\n]", &expect_npart, &expect_npoint, &color, &size,
				              &filelayer, name) + 1;
			else
				nget = sscanf(buffer, "> ## STARTPOLYGON %d,%d,%d,%d,%d,%d,%1023[^\n]", &expect_npart, &expect_npoint, &color, &size,
				              &fillcolor, &filelayer, name);
			if (infeature || nget < 6 || expect_npart < 1 || expect_npoint < 1) {
				status = MB_FAILURE;
				*error = MB_ERROR_BAD_FORMAT;
			}
			else {
				infeature = true;
				mb_gis_accum_reset(&accum);
			}
		}

		/* start of a polyline part or polygon ring */
		else if ((filetype == MB_GIS_POLYLINE && strncmp(buffer, "> ## PART ", 10) == 0) ||
		         (filetype == MB_GIS_POLYGON && buffer[0] == '>' && strstr(buffer, "## RING ") != NULL)) {
			int ipart = -1;
			int partnpoint = 0;
			int parttype = MB_GIS_PART_LINE;
			char typestring[16] = "";
			if (filetype == MB_GIS_POLYLINE) {
				if (sscanf(buffer, "> ## PART %d,%d", &ipart, &partnpoint) != 2)
					ipart = -1;
			}
			else {
				if (sscanf(strstr(buffer, "## RING "), "## RING %d,%d,%15s", &ipart, &partnpoint, typestring) != 3)
					ipart = -1;
				else if (strcmp(typestring, "OUTER") == 0)
					parttype = MB_GIS_PART_OUTER;
				else if (strcmp(typestring, "HOLE") == 0)
					parttype = MB_GIS_PART_HOLE;
				else
					ipart = -1;
			}

			/* the previous part must have had the number of points it declared */
			if (!infeature || ipart != accum.npart || partnpoint < 1 ||
			    (accum.npart > 0 && accum.npoint - accum.partstart[accum.npart - 1] != expect_partnpoint)) {
				status = MB_FAILURE;
				*error = MB_ERROR_BAD_FORMAT;
			}
			else {
				expect_partnpoint = partnpoint;
				status = mb_gis_accum_addpart(verbose, &accum, parttype, error);
			}
		}

		/* end of a polyline or polygon */
		else if ((filetype == MB_GIS_POLYLINE && strncmp(buffer, "> ## ENDPOLYLINE", 16) == 0) ||
		         (filetype == MB_GIS_POLYGON && strncmp(buffer, "> ## ENDPOLYGON", 15) == 0)) {
			int ilayer;
			if (!infeature || accum.npart != expect_npart || accum.npoint != expect_npoint ||
			    accum.npoint - accum.partstart[accum.npart - 1] != expect_partnpoint) {
				status = MB_FAILURE;
				*error = MB_ERROR_BAD_FORMAT;
			}
			if (status == MB_SUCCESS)
				status = mb_gis_maplayer(verbose, gis, file, filelayer, &layermap, &nlayermap, &ilayer, error);
			if (status == MB_SUCCESS) {
				status = mb_gis_addfeature(verbose, gis, ilayer, color, size, fillcolor, name, accum.npart, accum.partstart,
				                           accum.parttype, accum.npoint, accum.lon, accum.lat, accum.z, error);
				if (status == MB_FAILURE && *error == MB_ERROR_BAD_PARAMETER)
					*error = MB_ERROR_BAD_FORMAT;
			}
			infeature = false;
		}

		/* other segment headers are ignored */
		else if (buffer[0] == '>') {
		}

		/* polyline or polygon points */
		else {
			double lon, lat, z = 0.0;
			if (!infeature || accum.npart < 1 || sscanf(buffer, "%lf,%lf,%lf", &lon, &lat, &z) < 2) {
				status = MB_FAILURE;
				*error = MB_ERROR_BAD_FORMAT;
			}
			else {
				status = mb_gis_accum_addpoint(verbose, &accum, lon, lat, z, error);
			}
		}
	}

	/* the file must have a header and must not end inside a feature */
	if (status == MB_SUCCESS && (filetype == MB_GIS_NONE || infeature)) {
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_FORMAT;
	}
	if (status == MB_FAILURE && *error == MB_ERROR_BAD_FORMAT)
		fprintf(stderr, "\nUnable to read %s file <%s>: bad format at line %d\n", mb_gis_typename(gis->type), file, nline);

	fclose(fp);
	mb_gis_accum_free(verbose, &accum);
	int error_free = MB_ERROR_NO_ERROR;
	mb_freed(verbose, __FILE__, __LINE__, (void **)&layermap, &error_free);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       type:                      %d\n", gis->type);
		fprintf(stderr, "dbg2       nlayer:                    %d\n", gis->nlayer);
		fprintf(stderr, "dbg2       nfeature read:             %d\n", gis->nfeature - nfeature_start);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
/* Write a MB-System polyline, polygon, or point file. */
int mb_gis_write(int verbose, const char *file, const char *program_name, struct mb_gisdata_struct *gis, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       file:                      %s\n", file);
		fprintf(stderr, "dbg2       program_name:              %s\n", program_name);
		fprintf(stderr, "dbg2       gis:                       %p\n", (void *)gis);
	}

	int status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;
	if (gis == NULL || gis->type < MB_GIS_POLYLINE || gis->type > MB_GIS_POINT) {
		*error = MB_ERROR_BAD_PARAMETER;
		return (MB_FAILURE);
	}

	FILE *fp = fopen(file, "w");
	if (fp == NULL) {
		*error = MB_ERROR_OPEN_FAIL;
		return (MB_FAILURE);
	}

	/* write the header */
	const char *typename = mb_gis_typename(gis->type);
	fprintf(fp, "## %s File Version %d.%2.2d\n", typename, MB_GISFILE_VERSION_MAJOR, MB_GISFILE_VERSION_MINOR);
	fprintf(fp, "## Output by Program %s\n", program_name);
	fprintf(fp, "## MB-System Version %s\n", MB_VERSION);
	char user[256], host[256], date[32];
	int error_ignore = MB_ERROR_NO_ERROR;
	mb_user_host_date(verbose, user, host, date, &error_ignore);
	fprintf(fp, "## Run by user <%s> on cpu <%s> at <%s>\n", user, host, date);
	fprintf(fp, "##\n");
	fprintf(fp, "## Each layer is defined by a line of the form:\n");
	fprintf(fp, "##   ## LAYER l,String\n");
	if (gis->type == MB_GIS_POLYLINE) {
		fprintf(fp, "## Each polyline starts with a line of the form:\n");
		fprintf(fp, "##   > ## STARTPOLYLINE npart,np,c,s,l,String\n");
		fprintf(fp, "## Each part of a polyline starts with a line of the form:\n");
		fprintf(fp, "##   > ## PART ipart,np\n");
		fprintf(fp, "## Each polyline ends with a line of the form:\n");
		fprintf(fp, "##   > ## ENDPOLYLINE\n");
	}
	else if (gis->type == MB_GIS_POLYGON) {
		fprintf(fp, "## Each polygon starts with a line of the form:\n");
		fprintf(fp, "##   > ## STARTPOLYGON nring,np,c,s,f,l,String\n");
		fprintf(fp, "## Each closed ring of a polygon starts with a line of the form:\n");
		fprintf(fp, "##   > ## RING iring,np,OUTER\n");
		fprintf(fp, "##   > -Ph ## RING iring,np,HOLE\n");
		fprintf(fp, "## Each polygon ends with a line of the form:\n");
		fprintf(fp, "##   > ## ENDPOLYGON\n");
	}
	fprintf(fp, "## where:\n");
	if (gis->type == MB_GIS_POLYLINE) {
		fprintf(fp, "##   npart:                Number of parts\n");
		fprintf(fp, "##   ipart:                Part index\n");
	}
	else if (gis->type == MB_GIS_POLYGON) {
		fprintf(fp, "##   nring:                Number of rings\n");
		fprintf(fp, "##   iring:                Ring index\n");
	}
	if (gis->type != MB_GIS_POINT)
		fprintf(fp, "##   np:                   Number of points\n");
	fprintf(fp, "##   c:                    Color\n");
	fprintf(fp, "##   s:                    Size\n");
	if (gis->type == MB_GIS_POLYGON)
		fprintf(fp, "##   f:                    Fill color (-1 = no fill)\n");
	fprintf(fp, "##   l:                    Layer\n");
	fprintf(fp, "##   String:               Name (can have spaces)\n");
	fprintf(fp, "## Color definitions:\n");
	fprintf(fp, "##   BLACK                 0\n");
	fprintf(fp, "##   WHITE                 1\n");
	fprintf(fp, "##   RED                   2\n");
	fprintf(fp, "##   YELLOW                3\n");
	fprintf(fp, "##   GREEN                 4\n");
	fprintf(fp, "##   BLUEGREEN             5\n");
	fprintf(fp, "##   BLUE                  6\n");
	fprintf(fp, "##   PURPLE                7\n");
	fprintf(fp, "## %s format:\n", gis->type == MB_GIS_POINT ? "Point" : "Point coordinate");
	if (gis->type == MB_GIS_POINT)
		fprintf(fp, "##   <longitude (deg)>,<latitude (deg)>,<z (m)>,<c>,<s>,<l>,<String>\n");
	else
		fprintf(fp, "##   <longitude (deg)>,<latitude (deg)>,<z (m)>\n");
	fprintf(fp, "##\n");
	fprintf(fp, "## Z values: %s\n", gis->hasz ? "present" : "absent");
	fprintf(fp, "## Number of layers: %d\n", gis->nlayer);
	fprintf(fp, "## Number of %ss: %d\n", gis->type == MB_GIS_POLYLINE ? "polyline" : (gis->type == MB_GIS_POLYGON ? "polygon" : "point"),
	        gis->nfeature);
	for (int i = 0; i < gis->nlayer; i++)
		fprintf(fp, "## LAYER %d,%s\n", i, gis->layers[i].name);

	/* write the features */
	for (int i = 0; i < gis->nfeature; i++) {
		const struct mb_gisfeature_struct *feature = &gis->features[i];
		if (gis->type == MB_GIS_POINT) {
			fprintf(fp, "%.9f,%.9f,%.3f,%d,%d,%d,%s\n", feature->lon[0], feature->lat[0], feature->z[0], feature->color, feature->size,
			        feature->layer, feature->name);
			continue;
		}
		if (gis->type == MB_GIS_POLYLINE)
			fprintf(fp, "> ## STARTPOLYLINE %d,%d,%d,%d,%d,%s\n", feature->npart, feature->npoint, feature->color, feature->size,
			        feature->layer, feature->name);
		else
			fprintf(fp, "> ## STARTPOLYGON %d,%d,%d,%d,%d,%d,%s\n", feature->npart, feature->npoint, feature->color, feature->size,
			        feature->fillcolor, feature->layer, feature->name);
		for (int j = 0; j < feature->npart; j++) {
			const int start = feature->partstart[j];
			const int end = j < feature->npart - 1 ? feature->partstart[j + 1] : feature->npoint;
			if (gis->type == MB_GIS_POLYLINE)
				fprintf(fp, "> ## PART %d,%d\n", j, end - start);
			else if (feature->parttype[j] == MB_GIS_PART_HOLE)
				fprintf(fp, "> -Ph ## RING %d,%d,HOLE\n", j, end - start);
			else
				fprintf(fp, "> ## RING %d,%d,OUTER\n", j, end - start);
			for (int k = start; k < end; k++)
				fprintf(fp, "%.9f,%.9f,%.3f\n", feature->lon[k], feature->lat[k], feature->z[k]);
		}
		fprintf(fp, "> ## END%s\n", gis->type == MB_GIS_POLYLINE ? "POLYLINE" : "POLYGON");
	}

	if (ferror(fp) != 0) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}
	if (fclose(fp) != 0) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
/* get the mb_gis type corresponding to an OGR geometry type */
static int mb_gis_ogrtype(OGRwkbGeometryType ogrtype) {
	switch (wkbFlatten(ogrtype)) {
	case wkbPoint:
	case wkbMultiPoint:
		return (MB_GIS_POINT);
	case wkbLineString:
	case wkbMultiLineString:
		return (MB_GIS_POLYLINE);
	case wkbPolygon:
	case wkbMultiPolygon:
		return (MB_GIS_POLYGON);
	default:
		return (MB_GIS_NONE);
	}
}
/*--------------------------------------------------------------------*/
/* get the mb_gis type of an OGR layer, from the layer definition or,
    if that is not specific, from the first feature with a geometry */
static int mb_gis_layertype(OGRLayerH layer) {
	int type = mb_gis_ogrtype(OGR_L_GetGeomType(layer));
	if (type == MB_GIS_NONE) {
		OGR_L_ResetReading(layer);
		OGRFeatureH feature;
		while (type == MB_GIS_NONE && (feature = OGR_L_GetNextFeature(layer)) != NULL) {
			OGRGeometryH geometry = OGR_F_GetGeometryRef(feature);
			if (geometry != NULL)
				type = mb_gis_ogrtype(OGR_G_GetGeometryType(geometry));
			OGR_F_Destroy(feature);
		}
		OGR_L_ResetReading(layer);
	}
	return (type);
}
/*--------------------------------------------------------------------*/
/* Get the type of the first polyline, polygon, or point layer in a
    shapefile (or other GDAL/OGR vector file). */
int mb_gis_shapefile_type(int verbose, const char *file, int *type, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       file:                      %s\n", file);
	}

	int status = MB_SUCCESS;
	*type = MB_GIS_NONE;
	*error = MB_ERROR_NO_ERROR;

	GDALAllRegister();
	GDALDatasetH dataset = GDALOpenEx(file, GDAL_OF_VECTOR | GDAL_OF_READONLY, NULL, NULL, NULL);
	if (dataset == NULL) {
		status = MB_FAILURE;
		*error = MB_ERROR_OPEN_FAIL;
	}
	else {
		const int nlayer = GDALDatasetGetLayerCount(dataset);
		for (int i = 0; i < nlayer && *type == MB_GIS_NONE; i++)
			*type = mb_gis_layertype(GDALDatasetGetLayer(dataset, i));
		GDALClose(dataset);
		if (*type == MB_GIS_NONE) {
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_FORMAT;
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       type:                      %d\n", *type);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
/* add the points of an OGR line string or ring to a feature being built */
static int mb_gis_accum_addogrpoints(int verbose, struct mb_gis_accum_struct *accum, OGRGeometryH geometry, int parttype,
                                     int *error) {
	int status = mb_gis_accum_addpart(verbose, accum, parttype, error);
	const int npoint = OGR_G_GetPointCount(geometry);
	for (int i = 0; status == MB_SUCCESS && i < npoint; i++) {
		double x, y, z;
		OGR_G_GetPoint(geometry, i, &x, &y, &z);
		status = mb_gis_accum_addpoint(verbose, accum, x, y, z, error);
	}
	return (status);
}
/*--------------------------------------------------------------------*/
/* add the parts of an OGR line string, polygon, or multi geometry */
static int mb_gis_accum_addogrgeometry(int verbose, struct mb_gis_accum_struct *accum, OGRGeometryH geometry, int *error) {
	int status = MB_SUCCESS;
	switch (wkbFlatten(OGR_G_GetGeometryType(geometry))) {
	case wkbLineString:
		status = mb_gis_accum_addogrpoints(verbose, accum, geometry, MB_GIS_PART_LINE, error);
		break;
	case wkbPolygon:
		for (int i = 0; status == MB_SUCCESS && i < OGR_G_GetGeometryCount(geometry); i++)
			status = mb_gis_accum_addogrpoints(verbose, accum, OGR_G_GetGeometryRef(geometry, i),
			                                   i == 0 ? MB_GIS_PART_OUTER : MB_GIS_PART_HOLE, error);
		break;
	case wkbMultiLineString:
	case wkbMultiPolygon:
		for (int i = 0; status == MB_SUCCESS && i < OGR_G_GetGeometryCount(geometry); i++)
			status = mb_gis_accum_addogrgeometry(verbose, accum, OGR_G_GetGeometryRef(geometry, i), error);
		break;
	default:
		break;
	}
	return (status);
}
/*--------------------------------------------------------------------*/
/* Check whether a feature overlaps the bounds (west, east, south, north),
    trying longitude shifts of 0, -360 and +360 degrees; if it does, shift
    the feature longitudes into the convention of the bounds. */
static bool mb_gis_accum_inbounds(struct mb_gis_accum_struct *accum, const double *bounds) {
	if (bounds == NULL)
		return (true);
	if (accum->npoint < 1)
		return (false);
	double lonmin = accum->lon[0];
	double lonmax = accum->lon[0];
	double latmin = accum->lat[0];
	double latmax = accum->lat[0];
	for (int i = 1; i < accum->npoint; i++) {
		lonmin = MIN(lonmin, accum->lon[i]);
		lonmax = MAX(lonmax, accum->lon[i]);
		latmin = MIN(latmin, accum->lat[i]);
		latmax = MAX(latmax, accum->lat[i]);
	}
	if (latmax < bounds[2] || latmin > bounds[3])
		return (false);
	const double shifts[3] = {0.0, -360.0, 360.0};
	for (int j = 0; j < 3; j++) {
		if (lonmax + shifts[j] >= bounds[0] && lonmin + shifts[j] <= bounds[1]) {
			if (shifts[j] != 0.0) {
				for (int i = 0; i < accum->npoint; i++)
					accum->lon[i] += shifts[j];
			}
			return (true);
		}
	}
	return (false);
}
/*--------------------------------------------------------------------*/
/* get an integer attribute if it is set and within a range */
static int mb_gis_intfield(OGRFeatureH feature, int field, int valuemin, int valuemax, int valuedefault) {
	if (field < 0 || !OGR_F_IsFieldSetAndNotNull(feature, field))
		return (valuedefault);
	const int value = OGR_F_GetFieldAsInteger(feature, field);
	if (value < valuemin || value > valuemax)
		return (valuedefault);
	return (value);
}
/*--------------------------------------------------------------------*/
/* check whether an attribute field name is ID or ends in _ID, ignoring case */
static bool mb_gis_isidfield(const char *fieldname) {
	const size_t len = strlen(fieldname);
	if (len < 2 || toupper((unsigned char)fieldname[len - 2]) != 'I' || toupper((unsigned char)fieldname[len - 1]) != 'D')
		return (false);
	return (len == 2 || fieldname[len - 3] == '_');
}
/*--------------------------------------------------------------------*/
/* Import the features of type gis->type from a shapefile (or other
    GDAL/OGR vector file), transforming coordinates to WGS84 longitude
    and latitude if the file defines a coordinate system. If bounds
    (west, east, south, north) is not NULL, only features overlapping the
    bounds are imported, with longitudes shifted into the convention of
    the bounds. Each file layer with features imported becomes a gis layer.
    The color, size, and fillcolor values are used unless the file has
    valid COLOR, SIZE, and FILLCOLOR attributes. Names come from a NAME,
    LABEL, or TITLE attribute, else from an ID or *_ID attribute as
    "<layer> <id>", else from the first text attribute, else are
    "<layer> <feature id>". nimport and nskip return the numbers of
    features imported and skipped. */
int mb_gis_import_shapefile(int verbose, const char *file, const double *bounds, int color, int size, int fillcolor,
                            struct mb_gisdata_struct *gis, int *nimport, int *nskip, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       file:                      %s\n", file);
		if (bounds != NULL)
			fprintf(stderr, "dbg2       bounds:                    %f %f %f %f\n", bounds[0], bounds[1], bounds[2], bounds[3]);
		else
			fprintf(stderr, "dbg2       bounds:                    NULL\n");
		fprintf(stderr, "dbg2       color:                     %d\n", color);
		fprintf(stderr, "dbg2       size:                      %d\n", size);
		fprintf(stderr, "dbg2       fillcolor:                 %d\n", fillcolor);
		fprintf(stderr, "dbg2       gis:                       %p\n", (void *)gis);
	}

	int status = MB_SUCCESS;
	*nimport = 0;
	*nskip = 0;
	*error = MB_ERROR_NO_ERROR;
	if (gis == NULL || gis->type < MB_GIS_POLYLINE || gis->type > MB_GIS_POINT || color < 0 || color > MB_GIS_COLOR_MAX ||
	    size < 1 || fillcolor < MB_GIS_COLOR_NONE || fillcolor > MB_GIS_COLOR_MAX) {
		*error = MB_ERROR_BAD_PARAMETER;
		return (MB_FAILURE);
	}

	GDALAllRegister();
	GDALDatasetH dataset = GDALOpenEx(file, GDAL_OF_VECTOR | GDAL_OF_READONLY, NULL, NULL, NULL);
	if (dataset == NULL) {
		*error = MB_ERROR_OPEN_FAIL;
		return (MB_FAILURE);
	}

	/* target coordinate system is WGS84 longitude and latitude */
	OGRSpatialReferenceH srs_wgs84 = OSRNewSpatialReference(NULL);
	OSRSetWellKnownGeogCS(srs_wgs84, "WGS84");
#if GDAL_VERSION_MAJOR >= 3
	OSRSetAxisMappingStrategy(srs_wgs84, OAMS_TRADITIONAL_GIS_ORDER);
#endif

	mb_path filebase;
	mb_gis_basename(file, filebase, true);
	struct mb_gis_accum_struct accum;
	memset(&accum, 0, sizeof(accum));
	int noutside = 0;
	const int nlayer = GDALDatasetGetLayerCount(dataset);
	for (int ilayer = 0; status == MB_SUCCESS && ilayer < nlayer; ilayer++) {
		OGRLayerH layer = GDALDatasetGetLayer(dataset, ilayer);
		if (mb_gis_layertype(layer) != gis->type) {
			const GIntBig count = OGR_L_GetFeatureCount(layer, TRUE);
			if (count > 0)
				*nskip += (int)count;
			continue;
		}

		/* set up the coordinate transformation */
		OGRCoordinateTransformationH transform = NULL;
		OGRSpatialReferenceH srs_layer = OGR_L_GetSpatialRef(layer);
		if (srs_layer != NULL) {
			OGRSpatialReferenceH srs_source = OSRClone(srs_layer);
#if GDAL_VERSION_MAJOR >= 3
			OSRSetAxisMappingStrategy(srs_source, OAMS_TRADITIONAL_GIS_ORDER);
#endif
			transform = OCTNewCoordinateTransformation(srs_source, srs_wgs84);
			OSRDestroySpatialReference(srs_source);
			if (transform == NULL) {
				fprintf(stderr, "\nUnable to transform the coordinate system of shapefile <%s> to WGS84 longitude and latitude\n",
				        file);
				status = MB_FAILURE;
				*error = MB_ERROR_BAD_PROJECTION;
				break;
			}
		}

		/* get the attribute fields: the name is from a NAME, LABEL, or TITLE
		    field, else "<layer> <id>" from an ID or *_ID field, else from the
		    first text field, else "<layer> <feature id>" */
		OGRFeatureDefnH definition = OGR_L_GetLayerDefn(layer);
		const char *namefields[3] = {"NAME", "LABEL", "TITLE"};
		int field_name = -1;
		int field_id = -1;
		for (int i = 0; field_name < 0 && i < 3; i++)
			field_name = OGR_FD_GetFieldIndex(definition, namefields[i]);
		for (int i = 0; field_name < 0 && field_id < 0 && i < OGR_FD_GetFieldCount(definition); i++) {
			if (mb_gis_isidfield(OGR_Fld_GetNameRef(OGR_FD_GetFieldDefn(definition, i))))
				field_id = i;
		}
		for (int i = 0; field_name < 0 && field_id < 0 && i < OGR_FD_GetFieldCount(definition); i++) {
			if (OGR_Fld_GetType(OGR_FD_GetFieldDefn(definition, i)) == OFTString)
				field_name = i;
		}
		const int field_color = OGR_FD_GetFieldIndex(definition, "COLOR");
		const int field_size = OGR_FD_GetFieldIndex(definition, "SIZE");
		const int field_fillcolor = OGR_FD_GetFieldIndex(definition, "FILLCOLOR");

		mb_path layername;
		if (nlayer > 1)
			snprintf(layername, sizeof(layername), "%.500s:%.500s", filebase, OGR_L_GetName(layer));
		else
			snprintf(layername, sizeof(layername), "%s", filebase);
		int gislayer = -1;

		/* loop over the features */
		OGR_L_ResetReading(layer);
		OGRFeatureH feature;
		while (status == MB_SUCCESS && (feature = OGR_L_GetNextFeature(layer)) != NULL) {
			OGRGeometryH geometry = OGR_F_GetGeometryRef(feature);
			if (geometry == NULL || mb_gis_ogrtype(OGR_G_GetGeometryType(geometry)) != gis->type ||
			    (transform != NULL && OGR_G_Transform(geometry, transform) != OGRERR_NONE)) {
				(*nskip)++;
				OGR_F_Destroy(feature);
				continue;
			}
			if (OGR_G_Is3D(geometry))
				gis->hasz = true;

			/* get the attributes */
			const int fcolor = mb_gis_intfield(feature, field_color, 0, MB_GIS_COLOR_MAX, color);
			const int fsize = mb_gis_intfield(feature, field_size, 1, INT_MAX, size);
			const int ffillcolor = mb_gis_intfield(feature, field_fillcolor, MB_GIS_COLOR_NONE, MB_GIS_COLOR_MAX, fillcolor);
			mb_path name = "";
			if (field_name >= 0 && OGR_F_IsFieldSetAndNotNull(feature, field_name))
				snprintf(name, sizeof(name), "%s", OGR_F_GetFieldAsString(feature, field_name));
			else if (field_id >= 0 && OGR_F_IsFieldSetAndNotNull(feature, field_id) &&
			         OGR_F_GetFieldAsString(feature, field_id)[0] != '\0')
				snprintf(name, sizeof(name), "%.500s %.500s", OGR_L_GetName(layer), OGR_F_GetFieldAsString(feature, field_id));
			if (name[0] == '\0')
				snprintf(name, sizeof(name), "%.1000s %lld", OGR_L_GetName(layer), (long long)OGR_F_GetFID(feature));

			/* points: each point of a multipoint becomes a separate feature */
			const bool multipoint = wkbFlatten(OGR_G_GetGeometryType(geometry)) == wkbMultiPoint;
			const int nsub = multipoint ? OGR_G_GetGeometryCount(geometry) : 1;
			for (int isub = 0; status == MB_SUCCESS && isub < nsub; isub++) {
				mb_gis_accum_reset(&accum);
				if (gis->type == MB_GIS_POINT) {
					OGRGeometryH point = multipoint ? OGR_G_GetGeometryRef(geometry, isub) : geometry;
					if (OGR_G_GetPointCount(point) == 1)
						status = mb_gis_accum_addogrpoints(verbose, &accum, point, MB_GIS_PART_LINE, error);
				}
				else {
					status = mb_gis_accum_addogrgeometry(verbose, &accum, geometry, error);
				}
				if (status == MB_FAILURE)
					break;
				if (accum.npoint < 1) {
					(*nskip)++;
					continue;
				}

				/* skip features outside the bounds */
				if (!mb_gis_accum_inbounds(&accum, bounds)) {
					(*nskip)++;
					noutside++;
					continue;
				}

				/* add the layer when its first feature is imported */
				if (gislayer < 0) {
					status = mb_gis_addlayer(verbose, gis, layername, &gislayer, error);
					if (status == MB_FAILURE)
						break;
				}

				mb_path subname;
				if (nsub > 1)
					snprintf(subname, sizeof(subname), "%.1000s %d", name, isub);
				else
					snprintf(subname, sizeof(subname), "%s", name);
				status = mb_gis_addfeature(verbose, gis, gislayer, fcolor, fsize, ffillcolor, subname, accum.npart, accum.partstart,
				                           accum.parttype, accum.npoint, accum.lon, accum.lat, accum.z, error);
				if (status == MB_SUCCESS) {
					(*nimport)++;
				}
				else if (*error == MB_ERROR_BAD_PARAMETER) {
					/* degenerate or out of range geometry */
					(*nskip)++;
					status = MB_SUCCESS;
					*error = MB_ERROR_NO_ERROR;
				}
			}
			OGR_F_Destroy(feature);
		}

		if (transform != NULL)
			OCTDestroyCoordinateTransformation(transform);
	}

	if (status == MB_SUCCESS && *nskip > noutside)
		fprintf(stderr, "\nSkipped %d features of other types or with unusable geometry in shapefile <%s>\n", *nskip - noutside,
		        file);

	mb_gis_accum_free(verbose, &accum);
	OSRDestroySpatialReference(srs_wgs84);
	GDALClose(dataset);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       nimport:                   %d\n", *nimport);
		fprintf(stderr, "dbg2       nskip:                     %d\n", *nskip);
		fprintf(stderr, "dbg2       hasz:                      %d\n", gis->hasz);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
/* make an OGR line string or linear ring from a feature part */
static OGRGeometryH mb_gis_ogrpart(const struct mb_gisdata_struct *gis, const struct mb_gisfeature_struct *feature, int ipart,
                                   OGRwkbGeometryType ogrtype) {
	OGRGeometryH geometry = OGR_G_CreateGeometry(ogrtype);
	const int start = feature->partstart[ipart];
	const int end = ipart < feature->npart - 1 ? feature->partstart[ipart + 1] : feature->npoint;
	for (int k = start; k < end; k++) {
		if (gis->hasz)
			OGR_G_AddPoint(geometry, feature->lon[k], feature->lat[k], feature->z[k]);
		else
			OGR_G_AddPoint_2D(geometry, feature->lon[k], feature->lat[k]);
	}
	return (geometry);
}
/*--------------------------------------------------------------------*/
/* Export the features to an ESRI shapefile with WGS84 longitude and latitude
    coordinates and NAME, COLOR, SIZE (and for polygons FILLCOLOR) attributes.
    The ".shp" suffix is added if missing; an existing shapefile is replaced. */
int mb_gis_export_shapefile(int verbose, const char *file, struct mb_gisdata_struct *gis, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:                   %d\n", verbose);
		fprintf(stderr, "dbg2       file:                      %s\n", file);
		fprintf(stderr, "dbg2       gis:                       %p\n", (void *)gis);
	}

	int status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;
	if (gis == NULL || gis->type < MB_GIS_POLYLINE || gis->type > MB_GIS_POINT) {
		*error = MB_ERROR_BAD_PARAMETER;
		return (MB_FAILURE);
	}

	/* make sure the file name ends with .shp */
	mb_path shpfile;
	const size_t len = strlen(file);
	if (len >= 4 && (strcmp(&file[len - 4], ".shp") == 0 || strcmp(&file[len - 4], ".SHP") == 0))
		snprintf(shpfile, sizeof(shpfile), "%s", file);
	else
		snprintf(shpfile, sizeof(shpfile), "%.1018s.shp", file);

	GDALAllRegister();
	GDALDriverH driver = GDALGetDriverByName("ESRI Shapefile");
	if (driver == NULL) {
		*error = MB_ERROR_OPEN_FAIL;
		return (MB_FAILURE);
	}

	/* replace an existing shapefile */
	VSIStatBufL statbuf;
	if (VSIStatL(shpfile, &statbuf) == 0)
		GDALDeleteDataset(driver, shpfile);

	GDALDatasetH dataset = GDALCreate(driver, shpfile, 0, 0, 0, GDT_Unknown, NULL);
	if (dataset == NULL) {
		*error = MB_ERROR_OPEN_FAIL;
		return (MB_FAILURE);
	}

	OGRSpatialReferenceH srs_wgs84 = OSRNewSpatialReference(NULL);
	OSRSetWellKnownGeogCS(srs_wgs84, "WGS84");
#if GDAL_VERSION_MAJOR >= 3
	OSRSetAxisMappingStrategy(srs_wgs84, OAMS_TRADITIONAL_GIS_ORDER);
#endif
	OGRwkbGeometryType layertype = wkbPoint;
	if (gis->type == MB_GIS_POLYLINE)
		layertype = wkbLineString;
	else if (gis->type == MB_GIS_POLYGON)
		layertype = wkbPolygon;
	if (gis->hasz)
		layertype = OGR_GT_SetZ(layertype);
	mb_path layername;
	mb_gis_basename(shpfile, layername, true);
	char option_encoding[] = "ENCODING=UTF-8";
	char *options[] = {option_encoding, NULL};
	OGRLayerH layer = GDALDatasetCreateLayer(dataset, layername, srs_wgs84, layertype, options);
	if (layer == NULL) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}

	/* define the attribute fields */
	const char *fieldnames[4] = {"NAME", "COLOR", "SIZE", "FILLCOLOR"};
	const int nfield = gis->type == MB_GIS_POLYGON ? 4 : 3;
	for (int i = 0; status == MB_SUCCESS && i < nfield; i++) {
		OGRFieldDefnH field = OGR_Fld_Create(fieldnames[i], i == 0 ? OFTString : OFTInteger);
		if (i == 0)
			OGR_Fld_SetWidth(field, MB_GIS_DBF_STRING_MAX);
		if (OGR_L_CreateField(layer, field, TRUE) != OGRERR_NONE) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		OGR_Fld_Destroy(field);
	}

	/* write the features */
	for (int i = 0; status == MB_SUCCESS && i < gis->nfeature; i++) {
		const struct mb_gisfeature_struct *feature = &gis->features[i];
		OGRFeatureH ogrfeature = OGR_F_Create(OGR_L_GetLayerDefn(layer));

		/* DBF text is limited to 254 bytes; do not split a UTF-8 character */
		mb_path name;
		snprintf(name, sizeof(name), "%s", feature->name);
		if (strlen(name) > MB_GIS_DBF_STRING_MAX) {
			int n = MB_GIS_DBF_STRING_MAX;
			while (n > 0 && (name[n] & 0xC0) == 0x80)
				n--;
			name[n] = '\0';
		}
		OGR_F_SetFieldString(ogrfeature, 0, name);
		OGR_F_SetFieldInteger(ogrfeature, 1, feature->color);
		OGR_F_SetFieldInteger(ogrfeature, 2, feature->size);
		if (gis->type == MB_GIS_POLYGON)
			OGR_F_SetFieldInteger(ogrfeature, 3, feature->fillcolor);

		/* build the geometry */
		OGRGeometryH geometry = NULL;
		if (gis->type == MB_GIS_POINT) {
			geometry = OGR_G_CreateGeometry(gis->hasz ? wkbPoint25D : wkbPoint);
			if (gis->hasz)
				OGR_G_SetPoint(geometry, 0, feature->lon[0], feature->lat[0], feature->z[0]);
			else
				OGR_G_SetPoint_2D(geometry, 0, feature->lon[0], feature->lat[0]);
		}
		else if (gis->type == MB_GIS_POLYLINE) {
			if (feature->npart == 1) {
				geometry = mb_gis_ogrpart(gis, feature, 0, wkbLineString);
			}
			else {
				geometry = OGR_G_CreateGeometry(wkbMultiLineString);
				for (int j = 0; j < feature->npart; j++)
					OGR_G_AddGeometryDirectly(geometry, mb_gis_ogrpart(gis, feature, j, wkbLineString));
			}
		}
		else {
			/* each outer ring starts a polygon, holes belong to the preceding outer ring */
			geometry = OGR_G_CreateGeometry(wkbMultiPolygon);
			OGRGeometryH polygon = NULL;
			for (int j = 0; j < feature->npart; j++) {
				if (polygon == NULL || feature->parttype[j] != MB_GIS_PART_HOLE) {
					polygon = OGR_G_CreateGeometry(wkbPolygon);
					OGR_G_AddGeometryDirectly(geometry, polygon);
				}
				OGR_G_AddGeometryDirectly(polygon, mb_gis_ogrpart(gis, feature, j, wkbLinearRing));
			}
			if (OGR_G_GetGeometryCount(geometry) == 1) {
				OGRGeometryH single = OGR_G_Clone(OGR_G_GetGeometryRef(geometry, 0));
				OGR_G_DestroyGeometry(geometry);
				geometry = single;
			}
		}
		OGR_F_SetGeometryDirectly(ogrfeature, geometry);

		if (OGR_L_CreateFeature(layer, ogrfeature) != OGRERR_NONE) {
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
		}
		OGR_F_Destroy(ogrfeature);
	}

	OSRDestroySpatialReference(srs_wgs84);
#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3, 7, 0)
	if (GDALClose(dataset) != CE_None && status == MB_SUCCESS) {
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
	}
#else
	GDALClose(dataset);
#endif

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       file:                      %s\n", shpfile);
		fprintf(stderr, "dbg2       error:                     %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:                    %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
