//
//  mapio.c
//  precisionControl
//
//  Created by Brian Schlining on 2007-08-31.
//  Copyright 2007 MBARI. All rights reserved.
//
// Documentation is formatted for HeaderDoc or HeaderBrowser

#include "mapio.h"


//TODO this function fails to print which file or directory doesn't exist, making its error message near useless.
int check_error(int status, struct mapsrc* src) {
	int err;
	
	if(status != NC_NOERR) {
		const char* status_tostring = nc_strerror(status);
		//char *status_tostring = nc_strerror(status);

		fprintf(stderr, "%s:%d %s\n", __func__,__LINE__, status_tostring);
		
		//free(status_tostring);
		// TODO how do we want to handle this error gracefully?
		src->status = src->status | MAPSRC_FILL_FAILURE;
		err = MAPIO_READERROR;
		//free(status_tostring);
	} else {
		err = MAPIO_OK;
	}
	return err;
}

void mapsrc_fill(const char* file, struct mapsrc* src) {

    if(NULL!=src){
        int err, i;
        double range[2];
        double delta;

        // We don't refill existign strctures unless they've been free'd first
        if(src->x != NULL || src->y != NULL) {
            fprintf(stderr,
                    "%s:%d WARN - Reallocate mapsrc structure that already contains data, which may leak memory. Use 'mapsrc_free' to release struct resources before calling %s",
                    __func__,__LINE__,__func__);
        }

        err = nc_open(file, NC_NOWRITE, &(src->ncid));
        if(MAPIO_OK != check_error(err, src)) {
            return;
        }

        /* get x variable */
        err = nc_inq_dimid(src->ncid, "x", &(src->xdimid));
        if(MAPIO_OK != check_error(err, src)) {
            return;
        }

        err = nc_inq_dimlen(src->ncid, src->xdimid, &(src->xdimlen));
        if(MAPIO_OK != check_error(err, src)) {
            return;
        }

        src->x = (double*) malloc(sizeof(double) * src->xdimlen);
        err = nc_inq_varid(src->ncid, "x", &(src->xid));
        if(MAPIO_OK != check_error(err, src)) {
            return;
        }

        err = nc_get_att_double(src->ncid, src->xid, "actual_range", range);
        if(MAPIO_OK != check_error(err, src)) {
            return;
        }

        //fill in xvec based on range boundaries.
        delta = (range[1] - range[0]) / (src->xdimlen - 1);
        for(i = 0; i < int(src->xdimlen); i++) {
            src->x[i] = range[0] + delta * i;
        }

        //OLD WAY TO GET X-VAR: USES X VALUES FROM GRD FILE
        /*err = nc_get_var_double(src->ncid, src->xid, src->x);
         if (MAPIO_OK != check_error(err, src)) { return; }
         */

        /* get y variable */
        err = nc_inq_dimid(src->ncid, "y", &(src->ydimid));
        if(MAPIO_OK != check_error(err, src)) {
            return;
        }

        err = nc_inq_dimlen(src->ncid, src->ydimid, &(src->ydimlen));
        if(MAPIO_OK != check_error(err, src)) {
            return;
        }

        src->y = (double*) malloc(sizeof(double) * src->ydimlen);
        if(MAPIO_OK != check_error(err, src)) {
            return;
        }

        err = nc_inq_varid(src->ncid, "y", &(src->yid));
        if(MAPIO_OK != check_error(err, src)) {
            return;
        }

        err = nc_get_att_double(src->ncid, src->yid, "actual_range", range);
        if(MAPIO_OK != check_error(err, src)) {
            return;
        }

        //fill in xvec based on range boundaries.
        delta = (range[1] - range[0]) / (src->ydimlen - 1);
        for(i = 0; i < int(src->ydimlen); i++) {
            src->y[i] = range[0] + delta * i;
        }

        //OLD WAY TO GET Y-VAR: USES Y VALUES FROM GRD FILE
        /*err = nc_get_var_double(src->ncid, src->yid, src->y);
         if (MAPIO_OK != check_error(err, src)) { return; }
         */

        /* get z variable */
        err = nc_inq_varid(src->ncid, "z", &(src->zid));
        if(MAPIO_OK != check_error(err, src)) {
            return;
        }

        src->status = src->status | MAPSRC_IS_FILLED;
    }else{
        fprintf(stderr,"%s:%d ERR src is NULL\n",__func__,__LINE__);
    }

}

float mapsrc_find(struct mapsrc* src, double x, double y) {
	float z_out=NAN;
    if(NULL!=src){
        int mapdata_code=0;

        // Make sure the data is within the map bounds
        struct mapbounds* bounds = mapbounds_init();
        mapbounds_fill1(src, bounds);
        mapdata_code = mapbounds_contains(bounds, x, y);
        free(bounds);

        if(mapdata_code != MAPBOUNDS_OUT_OF_BOUNDS) {
            int XI = 1, YI = 0;
            size_t start[2]={0};        // For NetCDF access -> {x0, y0}
            size_t count[2]={0};        // For NetCDF access -> {xdimlen, ydimlen}
            // Get the nearest point in our map
            start[XI] = nearest(x, src->x, src->xdimlen);
            start[YI] = nearest(y, src->y, src->ydimlen);
            count[XI] = 1;
            count[YI] = 1;
            float *z = (float*) malloc(count[YI] * count[XI] * sizeof(float));
            check_error(nc_get_vara_float(src->ncid, src->zid, start, count, z), src);
            z_out = *z;
            free(z);
        }
    }
	return z_out;
}


struct mapsrc* mapsrc_init(void) {
	struct mapsrc* src = (struct mapsrc*) malloc(sizeof(struct mapsrc));
	src->ncid = 0;
	src->x = NULL;
	src->xid = 0;
	src->xdimid = 0;
	src->xdimlen = 0;
	src->y = NULL;
	src->yid = 0;
	src->ydimlen = 0;
	src->ydimid = 0;
	src->zid = 0;
	src->status = MAPSRC_IS_EMPTY;
	return src;
}


void mapsrc_free(struct mapsrc** psrc) {
    if(NULL != psrc){
        struct mapsrc* src = *psrc;
        if(NULL!=src){
            if(src->x != NULL) {
                free(src->x);
            }
            if(src->y != NULL) {
                free(src->y);
            }
            free(src);
            *psrc = NULL;
        }
    }
}

char* mapsrc_tostring(struct mapsrc* src) {

    int ssz = 180 * sizeof(char);
    int bsz = 100;
    char* str = (char*) malloc(ssz);
	char buf[bsz];
	//char *buf = malloc(100 * sizeof(char));
	snprintf(str, ssz, "mapsrc {\n\tncid = %i\n", src->ncid);
	snprintf(buf, bsz, "\txid = %i\n", src->xid);
	strcat(str, buf);
	snprintf(buf,  bsz, "\txdimid = %i\n", src->xdimid);
	strcat(str, buf);
	snprintf(buf,  bsz, "\txdimlen = %d\n", int(src->xdimlen));
	strcat(str, buf);
	snprintf(buf,  bsz, "\tyid = %i\n", src->yid);
	strcat(str, buf);
	snprintf(buf,  bsz, "\tydimid = %i\n", src->ydimid);
	strcat(str, buf);
	snprintf(buf,  bsz, "\tydimlen = %d\n", int(src->ydimlen));
	strcat(str, buf);
	snprintf(buf,  bsz, "\tzid = %i\n", src->zid);
	strcat(str, buf);
	snprintf(buf,  bsz, "\tstatus = %i\n", src->status);
	strcat(str, buf);
	strcat(str, "}");
	return str;
}

int mapdata_fill(struct mapsrc* src, struct mapdata* data, double x,
				 double y, double xwidth, double ywidth) {
				 
	const char* pname = "mapdata_fill";
	
	double xmin, xmax, ymin, ymax;
	// start = { x0, y0}, count = {xdimlen, ydimlen}
	size_t start[2];        // For NetCDF access -> {x0, y0}
	size_t count[2];        // For NetCDF access -> {xdimlen, ydimlen}
	//int i;
	int err, XI = 1, YI = 0;
	float* array;
	
	
	// Calculate the position of each corner of the submap
	xmin = x - xwidth / 2;
	xmax = x + xwidth / 2;
	ymin = y - ywidth / 2;
	ymax = y + ywidth / 2;
	//fprintf(stdout, "xmin: %f, xmax: %f, ymin: %f, ymax, %f", xmin, xmax, ymin, ymax);
	
	if(MAPIO_DEBUG) {
		fprintf(stdout, "MAPIO::%s: xmin: %f, xmax: %f, ymin: %f, ymax, %f", pname, xmin, xmax, ymin, ymax);
		fprintf(stdout, "MAPIO::%s: Looking for nearest values\n", pname);
	}
	// Get the indices into the map array for the corners of our submap
	start[XI] = nearest(xmin, src->x, src->xdimlen);
	start[YI] = nearest(ymin, src->y, src->ydimlen);
	count[XI] = nearest(xmax, src->x, src->xdimlen) - start[XI] + 1;
	count[YI] = nearest(ymax, src->y, src->ydimlen) - start[YI] + 1;
	
	
	// Assign values to 'data'
	if(MAPIO_DEBUG) {
		fprintf(stdout, "MAPIO::%s: Allocating x and y array memory\n", pname);
	}
	
	data->xdimlen = count[XI];
	data->ydimlen = count[YI];
	data->xpts = (double*) malloc(data->xdimlen * sizeof(double));
	memcpy(data->xpts, src->x + start[XI], count[XI] * sizeof(double));
	data->ypts = (double*) malloc(data->ydimlen * sizeof(double));
	memcpy(data->ypts, src->y + start[YI], count[YI] * sizeof(double));
	data->xcenter = (data->xpts[data->xdimlen - 1] + data->xpts[0]) / 2.0;
	data->ycenter = (data->ypts[data->ydimlen - 1] + data->ypts[0]) / 2.0;
	
	// Allocate memory for the arrays
	if(MAPIO_DEBUG) {
		fprintf(stdout, "MAPIO::%s: Allocating z array memory", pname);
	}
	array = (float*) malloc(count[YI] * count[XI] * sizeof(float));
	if(array == NULL) {
		fprintf(stderr, "MAPIO::%s: Out of memory. Failed to allocate memory for a mapdata structure\n", pname);
		data->status = data->status | MAPDATA_FILL_FAILURE;
	}
	
	//    if (MAPIO_DEBUG) {
	//        fprintf(stdout, "MAPIO::%s: Allocating z subarry memory", pname);
	//    }
	//    for (i = 0; i < count[YI]; i++) {
	//        *(array + i) = (float*) malloc(count[XI] * sizeof(float));
	//        if (*(array + i) = NULL) {
	//            fprintf(stderr, "MAPIO::%s: Out of memory. Failed to allocate memory for a mapdata structure\n", pname);
	//            // TODO OUT OF MEMORY!! handle error
	//            data->status = data->status | MAPDATA_FILL_FAILURE;
	//            return;
	//        }
	//    }
	
	
	// TODO Is this right...how the heck do I pass in an array of arrays to netcdf?
	data->z = array;
	
	// Extract the data from the netcdf source file.
	if(MAPIO_DEBUG) {
		fprintf(stdout, "MAPIO::%s: Reading z from netcdf", pname);
	}
	err = nc_get_vara_float(src->ncid, src->zid, start, count, (float*) data->z);
	check_error(err, src);
	data->status = MAPDATA_IS_FILLED;
	
	// Debug output used for compring results with matlab 'truth'
	if(MAPIO_DEBUG) {
		fprintf(stdout, "MAPIO::%s: ---- TEST REPORT ----\n", pname);
		
		fprintf(stdout, "Using data from %i\n", src->ncid);
		
		fprintf(stdout, "-- About X\n");
		fprintf(stdout, "X contains %i elements.\nMin = %f, Max = %f\n",
				int(src->xdimlen),
				src->x[0],
				src->x[int(src->xdimlen) - 1]);
		fprintf(stdout, "You specified: Center = %f, Width = %f\n", x, xwidth);
		fprintf(stdout, "Submap: x[%i] = %f to x[%i] = %f, %i elements\n",
				int(start[XI]),
				src->x[start[XI]],
				int(start[XI]) + int(data->xdimlen) - 1,
				src->x[start[XI] + int(data->xdimlen) - 1],
				int(data->xdimlen));
				
		fprintf(stdout, "-- About Y\n");
		fprintf(stdout, "Y contains %i elements.\nMin = %f, Max = %f\n",
				int(src->ydimlen),
				src->y[0],
				src->y[src->ydimlen - 1]);
		fprintf(stdout, "You specified: Center = %f, Width = %f\n", y, ywidth);
		fprintf(stdout, "Submap: y[%i] = %f to y[%i] = %f, %i elements\n",
				int(start[YI]),
				src->y[start[YI]],
				int(start[YI]) + int(data->ydimlen) - 1,
				src->y[start[YI] + int(data->ydimlen) - 1],
				int(data->ydimlen));
		fprintf(stdout, "\n");
		z_print((float*) data->z, data->ydimlen, data->xdimlen);
	}
	
	return mapdata_check(data, src, x, y, xwidth, ywidth);
}

void mapdata_free(struct mapdata* data, int free_all) {

	// Free the Z array
	//int i = 0;
	// TODO Confirm that I don' need to deallocate these memory blocks.
	//for (i = 0; i < data->xdimlen; i++) {
	//        printf("freeing z[%i] of %i\n", i, data->xdimlen);
	//        free(data->z[i]);
	//        data->z[i] = NULL;
	//    }
	if(data->z != NULL) {
		free(data->z);
		data->z = NULL;
	}
	
	if(data->xpts != NULL) {
		free(data->xpts);
		data->xpts = NULL;
	}
	
	if(data->ypts != NULL) {
		free(data->ypts);
		data->ypts = NULL;
	}
	
	if(free_all) {
		free(data);
	} else {
		data->xcenter = NAN;
		data->ycenter = NAN;
		data->xdimlen = 0;
		data->ydimlen = 0;
		data->status = MAPDATA_IS_EMPTY;
	}
	
}


char* mapdata_tostring(struct mapdata* data) {
    char *retval = NULL;
    if(NULL!=data){
        int ssz = 180 * sizeof(char);
        char* str = (char*) malloc(ssz);
        if(NULL!=str){
            memset(str,0,ssz * sizeof(char));
            int bsz = 100;
            char buf[bsz];
            strcpy(str, "mapdata {\n");
            snprintf(buf,  bsz, "\txcenter = %f\n", data->xcenter);
            strcat(str, buf);
            snprintf(buf,  bsz, "\tycenter = %f\n", data->ycenter);
            strcat(str, buf);
            if(data->xpts == NULL) {
                snprintf(buf,  bsz, "\tWARNING: x = NULL");
                strcat(str, buf);
            }
            if(data->ypts == NULL) {
                snprintf(buf,  bsz, "\tWARNING: y = NULL");
                strcat(str, buf);
            }
            snprintf(buf,  bsz, "\txdimlen = %zu\n", data->xdimlen);
            strcat(str, buf);
            snprintf(buf,  bsz, "\tydimlen = %zu\n", data->ydimlen);
            strcat(str, buf);
            snprintf(buf,  bsz, "\tstatus = %i\n", data->status);
            strcat(str, buf);
            strcat(str, "}");
            retval = str;
        }// else malloc failed
    }// else invalid arg
	return retval;
}

int nearest(double key, const double* base, size_t nmemb) {

	int i = 0, idx = 0, j;
	double a, dt, dt0;//, minval, maxval;
	//maxval = *(base + (nmemb - 1));
	//minval = *base;
	
	if(key > *(base + (nmemb - 1))) {
		idx = nmemb - 1;
		//fprintf(stderr, "MAPIO: Unable to find the nearest value to %f. It is more than the largest value, %f, in the array\n", key, maxval);
	} else if(key < *base) {
		idx = 0;
		//fprintf(stderr, "MAPIO: Unable to find the nearest value to %f. It is less than the smallest value, %f, in the array\n", key, minval);
	} else {
		dt0 = *(base + (nmemb - 1));
		for(j = 0; j < int(nmemb); j++) {
			a = *(base + j);
			dt = fabs(key - a);
			if(dt <= dt0) {
				dt0 = dt;
				idx = i;
			} else {
				break;
			}
			++i;
		}
	}
	return idx;
}

float getZ(const float* z, int row, int column, int columns) {
	return z[row * columns + column];
}

void z_print(const float* z, int rows, int columns) {
	float value;
	int i, j;
	
	for(i = 0; i < rows; i++) {
		for(j = 0; j < columns; j++) {
			//value = z[i * columns + j];
			value = getZ(z, i, j, columns);
			fprintf(stdout, "%f\t", value);
		}
		fprintf(stdout, "\n");
	}
	
}

int mapdata_check(struct mapdata* data, struct mapsrc* src, double xcenter, double ycenter, double xwidth, double ywidth) {

	int mapdata_code = MAPBOUNDS_OK;
	
	struct mapbounds* bounds = mapbounds_init();
	mapbounds_fill1(src, bounds);
	mapdata_code = mapbounds_contains(bounds, xcenter, ycenter);
	if(mapdata_code == MAPBOUNDS_OK) {
		mapdata_code = mapdata_checksize(bounds, data, xwidth, ywidth);
	}
	free(bounds);
	return mapdata_code;
}

struct mapbounds* mapbounds_init() {
	struct mapbounds* bounds = (struct mapbounds*) malloc(sizeof(struct mapbounds));
	bounds->dx = 0;
	bounds->dy = 0;
	bounds->ncid = 0;
	bounds->xmax = 0;
	bounds->xmin = 0;
	bounds->ymax = 0;
	bounds->ymin = 0;
	return bounds;
}


int mapbounds_fill1(struct mapsrc* src, struct mapbounds* bounds) {
	int return_code = MAPIO_OK;
	bounds->ncid = src->ncid;
	bounds->xmin = src->x[0];
	bounds->xmax = src->x[src->xdimlen - 1];
	bounds->dx = (bounds->xmax - bounds->xmin) / (float)(src->xdimlen - 1);
	bounds->ymin = src->y[0];
	bounds->ymax = src->y[src->ydimlen - 1];
	bounds->dy = (bounds->ymax - bounds->ymin) / (float)(src->ydimlen - 1);
	return return_code;
}

char* mapbounds_tostring(struct mapbounds* bounds) {
    int ssz = 256 * sizeof(char);
    int bsz = 100;
	char* str = (char*) malloc(ssz);
	char buf[100];
	strcpy(str, "mapbounds {\n");
	snprintf(buf,  bsz, "\tncid = %d\n", bounds->ncid);
	strcat(str, buf);
	snprintf(buf,  bsz, "\txmin = %f\n", bounds->xmin);
	strcat(str, buf);
	snprintf(buf,  bsz, "\txmax = %f\n", bounds->xmax);
	strcat(str, buf);
	snprintf(buf,  bsz, "\tdx = %f\n", bounds->dx);
	strcat(str, buf);
	snprintf(buf,  bsz, "\tymin = %f\n", bounds->ymin);
	strcat(str, buf);
	snprintf(buf,  bsz, "\tymax = %f\n", bounds->ymax);
	strcat(str, buf);
	snprintf(buf,  bsz, "\tdy = %f\n", bounds->dy);
	strcat(str, buf);
	strcat(str, "}");
	return str;
}


int mapbounds_fill2(const char* file, struct mapbounds* bounds) {
	int return_code;
	struct mapsrc* src = mapsrc_init();
	mapsrc_fill(file, src);
	return_code = mapbounds_fill1(src, bounds);
	mapsrc_free(&src);
	return return_code;
}

int mapbounds_contains(struct mapbounds* bounds, const double x, const double y) {
	int contains = MAPBOUNDS_OUT_OF_BOUNDS; // Default is False
	
	if(((double) bounds->xmax > x) &&
			((double) bounds->xmin < x) &&
			((double) bounds->ymax > y) &&
			((double) bounds->ymin < y)) {
		contains = MAPBOUNDS_OK;
	}
	
	return contains;
}

int mapdata_checksize(struct mapbounds* bounds, struct mapdata* data, const double xwidth, const double ywidth) {
	double x_size, y_size; // Number of expected pixels
	int size_is_ok = MAPBOUNDS_OK; // Default is True
	
	// Calculate expected size
	x_size = xwidth / bounds->dx;
	y_size = ywidth / bounds->dy;
	
	if(((double) data->xdimlen < (x_size - 1)) || ((double) data->ydimlen < (y_size - 1))) {
		size_is_ok = MAPBOUNDS_NEAR_EDGE;
	}
	return size_is_ok;
}
