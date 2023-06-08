#include "TerrainMapDEM.h"

#include <iostream>
#include <cmath>
#include "mapio.h"
#include "genFilterDefs.h"
#include "trn_log.h"


double
TerrainMapDEM::
GetRangeError(double& mapVariance, const double* const startPoint, const double* const directionVector, double measuredDistance) {
	/* called with:
	tempExpectedMeasDiff[i] = terrainMap->GetRangeError(mapVar, particle.position, beamVector, beamRanges[i]);
	*/
	double rangeError = 0.;
	if(USE_RANGE_CORR){
		double predictedRange;
		double beamU[3];
		beamU[0] = directionVector[0]/measuredDistance;
		beamU[1] = directionVector[1]/measuredDistance;
		beamU[2] = directionVector[2]/measuredDistance;

		if(!computeMapRayIntersection(startPoint, beamU, predictedRange, mapVariance)){
			rangeError = measuredDistance - fabs(predictedRange);

		} else {
			if(!USE_MAP_NAN){
				return predictedRange;//NAN
			}
			//ADD IN CODE TO HANDLE NAN VALUES
		}
	} else {
		// Projection Method:
		double beamN, beamE, beamZ, mapZ;
		beamN = startPoint[0] + directionVector[0];
		beamE = startPoint[1] + directionVector[1];
		beamZ = startPoint[2] + directionVector[2];//the expected measurement

		interpolateDepth(beamN, beamE, mapZ, mapVariance);

		// if(!isnan(mapZ) && !isnan(beamZ)) {  USING ISNIN here
		if(!ISNIN(mapZ) && !ISNIN(beamZ)) {
			//UPDATE Expected Measurement Difference
			//particle.expectedMeasDiff[i] = beamZ-mapZ;  //measured - expected;
			rangeError = beamZ - mapZ;
			//mapSquared[i] += pow(beamZ-mapZ,2)*particle.weight;
			//mapMean[i] += (beamZ-mapZ)*particle.weight;
		} else {
			//particle.expectedMeasDiff[i] = 0;
			rangeError = 0;
			//if we don't want to use nan values, don't incorporate measurement
			if(!USE_MAP_NAN) {
				//return NAN;
				// if(isnan(mapZ)){
				if(ISNIN(mapZ)){
					return mapZ;
				}else{
					return beamZ;
				}
			}
			//ADD IN CODE TO HANDLE NAN VALUES
		}
	}
    return rangeError;
}
/*
double
TerrainMapDEM::
QueryMap(const double* const queryPoint) {
	//good luck
}
*/

/*---------------------------------------------------------------------------------*/

TerrainMapDEM::
TerrainMapDEM(const char* mapName) {
	interpMapMethod = 0;
	this->refMap = new refMapT;
	setRefMap(mapName);
}

int
TerrainMapDEM::
loadSubMap(const double xcen, const double ycen, double* mapWidth, double vehN, double vehE)
{
	int mapStatus = this->extractSubMap(xcen, ycen, mapWidth);

	switch(mapStatus) {

		case MAPBOUNDS_OUT_OF_BOUNDS:
			logs(TL_OMASK(TL_TERRAIN_MAP_DEM, TL_LOG),"TerrainNav:: Vehicle is operating outside of the given reference"
				   " map.\n");
			break;

		case MAPBOUNDS_OK:
			break;

		case MAPBOUNDS_NEAR_EDGE:
			logs(TL_OMASK(TL_TERRAIN_MAP_DEM, TL_LOG),"TerrainNav:: Vehicle is operating near the reference map boundary"
				   "; correlation area may be truncated\n");
			break;

		default:
			logs(TL_OMASK(TL_TERRAIN_MAP_DEM, TL_LOG),"TerrainNav:: No valid map status code returned from extract map"
				   " function\n");
			mapStatus = MAPBOUNDS_OUT_OF_BOUNDS;
			break;
	}

	return mapStatus;
}

TerrainMapDEM::
~TerrainMapDEM() {
	delete refMap;
}

// throws exception when loading results in error
void
TerrainMapDEM::
setLowResMap(const char* mapName){
	if(this->refMap->lowResSrc == NULL) {
		this->refMap->lowResSrc = mapsrc_init();
		mapsrc_fill(mapName, this->refMap->lowResSrc);
	}

	if(this->refMap->lowResSrc->status != MAPSRC_IS_FILLED) {
      logs(TL_OMASK(TL_TERRAIN_MAP_DEM, TL_LOG),"Error loading in low resolution map file...\n");
      throw Exception("TerrainMapDEM::setLowResMap() - Error loading map file");
	}
}

bool
TerrainMapDEM::
GetMapBounds(double* currMapBounds){
	if(map.xpts != NULL) {
		currMapBounds[0] = map.xpts[0];
		currMapBounds[1] = map.xpts[map.numX - 1];
		currMapBounds[2] = map.ypts[0];
		currMapBounds[3] = map.ypts[map.numY - 1];
		return true;
	}
	return false;
}

bool
TerrainMapDEM::
GetMapT(mapT& currMap){
	if(map.xpts != NULL){
		currMap = map;
		return true;
	}
	return false;
}

bool
TerrainMapDEM::
withinRefMap(const double northPos, const double eastPos) {
	int withinBounds = mapbounds_contains(this->refMap->bounds, northPos,
										  eastPos);
	if(withinBounds == MAPBOUNDS_OK) {
		return true;
	}

	return false;
}

bool
TerrainMapDEM::
withinValidMapRegion(const double northPos, const double eastPos) {
	if(withinRefMap(northPos, eastPos)) {
		double mapValue = mapsrc_find(this->refMap->src, eastPos, northPos);
		// if(!isnan(mapValue)) {
		if(!ISNIN(mapValue)) {
			return true;
		}
	}
	return false;
}

bool
TerrainMapDEM::
withinSubMap(const double northPos, const double eastPos){
	//Check to make sure a sub-map has been loaded
	if(this->map.xpts == NULL) {
		return false;
	}

	//Check if the point is within the loaded sub-map
	if((northPos > this->map.xpts[0]) &&
			(northPos < this->map.xpts[this->map.numX - 1]) &&
			(eastPos > this->map.ypts[0]) &&
			(eastPos < this->map.ypts[this->map.numY - 1])) {
		return true;
	} else {
		return false;
	}
}

// throws exception when loading results in error
void
TerrainMapDEM::
setRefMap(const char* mapName){
	//int check_error_code;
	mapbounds* tempBounds;
	char mapPrefix[1024];
	char mapVarName[1040];

	//clear memory for any currently stored ref maps
	this->refMap->clean();

	//set map source to new reference map
	this->refMap->src = mapsrc_init();
	mapsrc_fill(mapName, this->refMap->src);

	//check that map source fill was successful
	if(this->refMap->src->status != MAPSRC_IS_FILLED) {
      logs(TL_OMASK(TL_TERRAIN_MAP_DEM, TL_LOG),"Error loading in map file...\n");
      throw Exception("TerrainMapDEM::setRefMap() - Error loading map file");
	}

	//define variance map file name
	strcpy(mapPrefix, mapName);
	strtok(mapPrefix, ".");
	snprintf(mapVarName, 1040, "%s%s", mapPrefix, "_sd.grd");

	//set variance map source if provided
	this->refMap->varSrc = mapsrc_init();

	//TODO MAPIO::check_error No such file or directory
	//std::cout << "\n\nBetween here";
	mapsrc_fill(mapVarName, this->refMap->varSrc);
	//std::cout << "\nand here\n\n";

	if(this->refMap->varSrc->status != MAPSRC_IS_FILLED) {
		mapsrc_free(&this->refMap->varSrc);
	}

	//set map bounds structure for new reference map
	this->refMap->bounds = mapbounds_init();
	tempBounds = mapbounds_init();

	//check_error_code = mapbounds_fill1(this->refMap->src, tempBounds);
	mapbounds_fill1(this->refMap->src, tempBounds);

	//change labels to keep with a right-handed coordinate system
	this->refMap->bounds->xmin = tempBounds->ymin;
	this->refMap->bounds->xmax = tempBounds->ymax;
	this->refMap->bounds->ymin = tempBounds->xmin;
	this->refMap->bounds->ymax = tempBounds->xmax;
	this->refMap->bounds->dx = tempBounds->dy;
	this->refMap->bounds->dy = tempBounds->dx;

	free(tempBounds);
	tempBounds = NULL;

	//display reference map boundary information to screen
	char* outputString = mapbounds_tostring(this->refMap->bounds);
	logs(TL_OMASK(TL_TERRAIN_MAP_DEM, TL_LOG),outputString);
	free(outputString);

	return;
}

/*! Private functions*/
bool
TerrainMapDEM::
computeMapRayIntersection(const double* position, double* u, double& r, double& var) {
	double length, diff, z;
	double xI[3];
	int maxIter = 100;
	int numIter;
	double tol = 0.001;
	xI[0] = position[0];
	xI[1] = position[1];
	xI[2] = position[2];
	r = 0;

	//verify that u has unit length
	length = sqrt(u[0] * u[0] + u[1] * u[1] + u[2] * u[2]);
	if(length != 1.0) {
		u[0] = u[0] / length;
		u[1] = u[1] / length;
		u[2] = u[2] / length;
	}

	//compute initial intersection with the terrain and delta_z difference
	interpolateDepth(xI[0], xI[1], z, var);
	diff = xI[2] - fabs(z);
	numIter = 1;

	//iterate until converged or until max iterations
	while(fabs(diff) > tol && numIter < maxIter) {
		//step along direction vector
		xI[0] -= diff * u[0];
		xI[1] -= diff * u[1];
		xI[2] -= diff * u[2];

		//recalculate terrain intersection
		interpolateDepth(xI[0], xI[1], z, var);

		//if interpolated depth is NaN, return false
		// if(isnan(z)) {
		if(ISNIN(z)) {
			r = fabs(z);
			return false;
		}

		diff = xI[2] - fabs(z);
		numIter++;
	}

	/*if(numIter == maxIter)
	{
	   logs(TL_OMASK(TL_TERRAIN_MAP_DEM, TL_LOG),"reached max iter; diff = %f \n", diff);
	   exit(0);
	   }*/

	r = sqrt((xI[0] - position[0]) * (xI[0] - position[0]) +
			 (xI[1] - position[1]) * (xI[1] - position[1]) +
			 (xI[2] - position[2]) * (xI[2] - position[2]));

	return true;
}


//Interpolate functions
void
TerrainMapDEM::
interpolateDepth(double xi, double yi, double& zi, double& var) {
	//Check that a map has been extracted
	if(this->map.xpts == NULL) {
		logs(TL_OMASK(TL_TERRAIN_MAP_DEM, TL_LOG),"ERROR: tried to access map values without first extracting map"
			   " information");
		return;
	}

	ColumnVector W;
	int* xIndices = NULL;
	int* yIndices = NULL;
	int numPts;
	double h_sq;

	//define pointer to an interpolation function
	void (*pt2interpFunction)(double*, double*, const Matrix&, double, double,
							  double&, int*, int*, ColumnVector&) = NULL;

	//determine number of interpolation points and proper interpolation function
	switch(this->interpMapMethod) {
		case 0:
			numPts = 1;
			pt2interpFunction = &nearestInterp;
			break;

		case 1:
			numPts = 4;
			pt2interpFunction = &bilinearInterp;
			break;

		case 2:
			numPts = 16;
			pt2interpFunction = &bicubicInterp;
			break;

		case 3:
			numPts = 16;
			pt2interpFunction = &splineInterp;
			break;

		default:
			numPts = 1;
			pt2interpFunction = &nearestInterp;
			break;
	}

	//initialize indices arrays and Weights matrix
	xIndices = new int[numPts];
	yIndices = new int[numPts];
	W.ReSize(numPts);

	//Perform interpolation
	(*pt2interpFunction)(this->map.xpts, this->map.ypts,
						 this->map.depths, xi, yi, zi,
						 xIndices, yIndices, W);

	//If returned depth is NaN, extract data from low resolution map
	// if(isnan(zi) && this->refMap->lowResSrc != NULL) {
	if(ISNIN(zi) && this->refMap->lowResSrc != NULL) {
		double xPt, yPt;
		zi = this->getNearestLowResMapPoint(xi, yi, xPt, yPt);
		h_sq = pow(xPt - xi, 2) + pow(yPt - yi, 2);
		var = this->map.depthVariance(xIndices[0] + 1, yIndices[0] + 1) +
			  evalVariogram(sqrt(h_sq));
	} else {
		//Compute variance associated with interpolation method
		if(W.Nrows() == 1) {
			//If only using nearest neighbor, weight variance based on distance of
			//nearest point to the interpolation point
			h_sq = pow(this->map.xpts[xIndices[0]] - xi, 2) +
				   pow(this->map.ypts[yIndices[0]] - yi, 2);
			var = this->map.depthVariance(xIndices[0] + 1, yIndices[0] + 1) +
				  evalVariogram(sqrt(h_sq));
		} else {
			var = computeInterpDepthVariance(xIndices, yIndices, W);
		}
	}

	delete [] xIndices;
	delete [] yIndices;
	return;
}

double
TerrainMapDEM::
getNearestLowResMapPoint(const double north, const double east, double& nearestNorth, double& nearestEast) {
	double zi;

	if(this->refMap->lowResSrc == NULL) {
		return 0.0;
	}

	zi = mapsrc_find(this->refMap->lowResSrc, east, north);
	nearestNorth = refMap->lowResSrc->y
				   [closestPtUniformArray(north, refMap->lowResSrc->y[0],
										  refMap->lowResSrc->y
										  [refMap->lowResSrc->ydimlen - 1],
										  refMap->lowResSrc->ydimlen)];
	nearestEast = refMap->lowResSrc->x
				  [closestPtUniformArray(east, refMap->lowResSrc->x[0],
										 refMap->lowResSrc->x
										 [refMap->lowResSrc->xdimlen - 1],
										 refMap->lowResSrc->xdimlen)];

	return zi;
}

double
TerrainMapDEM::
computeInterpDepthVariance(int* xIndices, int* yIndices, ColumnVector Weights) {
	double var;
	Matrix varValue;
	int N = Weights.Nrows();
	SymmetricMatrix VarMat(N);
	ColumnVector VarVec(N);

	VarMat = 0.0;

	for(int i = 0; i < N; i++) {
		VarVec(i + 1) = this->map.depthVariance(xIndices[i] + 1, yIndices[i] + 1);
		double z1 = this->map.depths(xIndices[i] + 1, yIndices[i] + 1);

		//Compute cross-variance terms using variogram
		for(int j = i; j < N; j++) {
			double dx = this->map.xpts[xIndices[i]] -
				 this->map.xpts[xIndices[j]];
            double dy = this->map.ypts[yIndices[i]] -
				 this->map.ypts[yIndices[j]];
            double z2 = this->map.depths(xIndices[j] + 1, yIndices[j] + 1);
            double hsq = dx * dx + dy * dy;
			VarMat(i + 1, j + 1) = 0.5 * pow(z1 - z2, 2) - evalVariogram(sqrt(hsq));

			// if(isnan(VarMat(i + 1, j + 1))) {
			if(ISNIN(VarMat(i + 1, j + 1))) {
				VarMat(i + 1, j + 1) = 0.0;
			}
		}
	}

	//compute total variance value for current map point
	varValue = Weights.t() * VarVec + Weights.t() * VarMat * Weights;
	var = varValue.AsScalar();

	//check that the variance is positive and finite
	// if(isnan(var) || var < 0) {
	if(ISNIN(var) || var < 0) {
		varValue = Weights.t() * VarVec;
		var = varValue.AsScalar();
	}

	return var;
}

int
TerrainMapDEM::
extractSubMap(const double north, const double east, double* mapParams) {
	int statusCode;

	//check that there is a reference map loaded to extract data from
	if(this->refMap->src == NULL) {
		logs(TL_OMASK(TL_TERRAIN_MAP_DEM, TL_LOG),"Attempted to extract map data with no reference map defined!!");
		return MAPBOUNDS_OUT_OF_BOUNDS;
	}

	//if map is already defined, clear memory
	if(this->map.xpts != NULL || this->map.ypts != NULL) {
		this->map.clean();
	}

	//load data from reference map
	struct mapdata* data = (struct mapdata*) malloc(sizeof(struct mapdata));
	statusCode = mapdata_fill(this->refMap->src, data, east, north, mapParams[1]
							  , mapParams[0]);

	//check status of loaded map data to ensure it worked properly
	if(statusCode != MAPBOUNDS_OUT_OF_BOUNDS) {
		convertMapdataToMapT(data);

		//load variance map data
		extractVarMap(north, east, mapParams);
	}

	mapdata_free(data, 1);
	return statusCode;
}
/**********************************************************************/


void
TerrainMapDEM::
convertMapdataToMapT(mapdata* currMapStruct) {
	int i;

	//define parameters in mapT structure based on parameters in currMapStruct
	this->map.numX = int(currMapStruct->ydimlen);
	this->map.numY = int(currMapStruct->xdimlen);
	this->map.xcen = currMapStruct->ycenter;
	this->map.ycen = currMapStruct->xcenter;

	//define map xpts and ypts vectors
	this->map.xpts = new double[this->map.numX];
	this->map.ypts = new double[this->map.numY];

	//Map is stored in E,N,U frame. Convert to N,E,D frame
	for(i = 0; i < this->map.numX; i++) {
		this->map.xpts[i] = currMapStruct->ypts[i];
	}

	for(i = 0; i < this->map.numY; i++) {
		this->map.ypts[i] = currMapStruct->xpts[i];
	}

	//define map parameters
	this->map.dx = this->refMap->bounds->dx;
	this->map.dy = this->refMap->bounds->dy;


	//convert zpts to Matrix of depths, positive downward
	Matrix temp(this->map.numX, this->map.numY);

	for(int row = 1; row <= this->map.numX; row++) {
		for(int col = 1; col <= this->map.numY; col++) {
			float* z = currMapStruct->z;
			float value = z[(row - 1) * this->map.numY + (col - 1)];
			temp(row, col) = fabs(value);
		}
	}

	this->map.depths = temp;

}

int
TerrainMapDEM::
extractVarMap(const double north, const double east, double* mapParams) {
	int statusCode;

	//check that there is a variance map loaded to extract data from
	if(this->refMap->varSrc == NULL) {
		this->map.depthVariance.ReSize(this->map.numX, this->map.numY);
		this->map.depthVariance = fabs(this->map.dx);

		//check for valid variance value
		// if(isnan(this->map.depthVariance(1, 1)) ||
		if(ISNIN(this->map.depthVariance(1, 1)) ||
				this->map.depthVariance(1, 1) == 0) {
			this->map.depthVariance = fabs(this->map.dx);
		}

		statusCode = MAPBOUNDS_OK;
	} else {
		struct mapdata* data = (struct mapdata*) malloc(sizeof(struct mapdata));
		statusCode = mapdata_fill(this->refMap->varSrc, data, east, north,
								  mapParams[1], mapParams[0]);

		//check status of loaded map data to ensure it worked properly
		if(statusCode != MAPBOUNDS_OUT_OF_BOUNDS) {
			//convert zpts to Matrix of depths
			Matrix temp(this->map.numX, this->map.numY);

			for(int row = 1; row <= this->map.numX; row++) {
				for(int col = 1; col <= this->map.numY; col++) {
					float* z = data->z;
					float value = z[(row - 1) * this->map.numY + (col - 1)];

					//estimate variance by stored std. dev. values plus variogram
					//variation at the given map resolution
					temp(row, col) = value * value + 1.0 +
									 evalVariogram(this->map.dx);

					//check for valid variance values
					// if(isnan(temp(row, col)) || value == 0) {
					if(ISNIN(temp(row, col)) || value == 0) {
						temp(row, col) = fabs(this->map.dx);
					}
				}
			}

			this->map.depthVariance = temp;

		}

		mapdata_free(data, 1);
	}

	return statusCode;
}
/******************************************************************************************/
/*---------------------------------------------------------------------------------*/





//used by point mass filter
void
TerrainMapDEM::
interpolateDepthMat(double* xi, double* yi, Matrix& zi, Matrix& var) {
	//Check that a map has been extracted
	if(this->map.xpts == NULL) {
		logs(TL_OMASK(TL_TERRAIN_MAP_DEM, TL_LOG),"ERROR: tried to access map values without first extracting map"
			   " information");
		return;
	}

	int N = zi.Nrows();
	int M = zi.Ncols();
	int numPts, i, j;
	ColumnVector W;
	int* xIndices = NULL;
	int* yIndices = NULL;
	double h_sq;

	//define pointer to an interpolation function
	void (*pt2interpFunction)(double*, double*, const Matrix&, double, double,
							  double&, int*, int*, ColumnVector&) = NULL;

	//determine number of interpolation points and proper interpolation function
	switch(this->interpMapMethod) {
		case 0:
			numPts = 1;
			pt2interpFunction = &nearestInterp;
			break;

		case 1:
			numPts = 4;
			pt2interpFunction = &bilinearInterp;
			break;

		case 2:
			numPts = 16;
			pt2interpFunction = &bicubicInterp;
			break;

		case 3:
			numPts = 16;
			pt2interpFunction = &splineInterp;
			break;

		default:
			numPts = 1;
			pt2interpFunction = &nearestInterp;
			break;
	}

	//initialize indices arrays and Weights matrix
	xIndices = new int[numPts];
	yIndices = new int[numPts];
	W.ReSize(numPts);

	for(i = 0; i < N; i++) {
		for(j = 0; j < M; j++) {
			(*pt2interpFunction)(this->map.xpts, this->map.ypts,
								 this->map.depths, xi[i], yi[j], zi(i + 1, j + 1)
								 , xIndices, yIndices, W);
			//If returned depth is NaN, extract data from low resolution map
			// if(isnan(zi(i + 1, j + 1)) && this->refMap->lowResSrc != NULL) {
			if(ISNIN(zi(i + 1, j + 1)) && this->refMap->lowResSrc != NULL) {
				double xPt, yPt;
				zi(i + 1, j + 1) = this->getNearestLowResMapPoint(xi[i], yi[j],
								   xPt, yPt);
				h_sq = pow(xPt - xi[i], 2) + pow(yPt - yi[j], 2);
				var(i + 1, j + 1) = this->map.depthVariance(xIndices[0] + 1,
									yIndices[0] + 1) +
									evalVariogram(sqrt(h_sq));
			} else {
				//Compute variance associated with interpolation method
				if(W.Nrows() == 1) {
					//If only using nearest neighbor, weight variance based on
					//distance of nearest point to the interpolation point
					h_sq = pow(this->map.xpts[xIndices[0]] - xi[i], 2) +
						   pow(this->map.ypts[yIndices[0]] - yi[j], 2);
					var(i + 1, j + 1) = this->map.depthVariance(xIndices[0] + 1,
										yIndices[0] + 1)
										+ evalVariogram(sqrt(h_sq));
				} else {
					var(i + 1, j + 1) = computeInterpDepthVariance(xIndices, yIndices, W);
				}
			}
		}
	}
	delete [] xIndices;
	delete [] yIndices;

	return;
}

//used by point mass filter
void
TerrainMapDEM::
interpolateGradient(double xi, double yi, Matrix& gradient) {
	ColumnVector W;
	int* xIndices = NULL;
	int* yIndices = NULL;
	int numPts = 0;
	double zi;

	//define pointer to an interpolation function
	void (*pt2interpFunction)(double*, double*, const Matrix&, double, double,
							  double&, int*, int*, ColumnVector&) = NULL;

	//determine number of interpolation points and proper interpolation function
	switch(this->interpMapMethod) {
		case 0:
			numPts = 1;
			pt2interpFunction = &nearestInterp;
			break;

		case 1:
			numPts = 4;
			pt2interpFunction = &bilinearInterp;
			break;

		case 2:
			numPts = 16;
			pt2interpFunction = &bicubicInterp;
			break;

		case 3:
			numPts = 16;
			pt2interpFunction = &splineInterp;
			break;

		default:
			numPts = 1;
			pt2interpFunction = &nearestInterp;
			break;
	}

	//initialize indices arrays and Weights matrix
	xIndices = new int[numPts];
	yIndices = new int[numPts];
	W.ReSize(numPts);

	//Perform interpolation to find xIndices and yIndices
	(*pt2interpFunction)(this->map.xpts, this->map.ypts,
						 this->map.depths, xi, yi, zi,
						 xIndices, yIndices, W);

	//Compute terrain gradient based on interpolation scheme and returned
	//weights/indices
	computeInterpTerrainGradient(xIndices, yIndices, xi, yi, gradient);

	delete [] xIndices;
	delete [] yIndices;
}

/*//used in modifyBeamDir
void
TNavFilter::
interpolateDepthAndGradient(double xi, double yi, double& zi, double& var, Matrix& gradient) {
	ColumnVector W;
	int* xIndices = NULL;
	int* yIndices = NULL;
	int numPts = 0;
	double h_sq;

	//define pointer to an interpolation function
	void (*pt2interpFunction)(double*, double*, const Matrix&, double, double,
							  double&, int*, int*, ColumnVector&) = NULL;

	//determine number of interpolation points and proper interpolation function
	switch(this->interpMapMethod) {
		case 0:
			numPts = 1;
			pt2interpFunction = &nearestInterp;
			break;

		case 1:
			numPts = 4;
			pt2interpFunction = &bilinearInterp;
			break;

		case 2:
			numPts = 16;
			pt2interpFunction = &bicubicInterp;
			break;

		case 3:
			numPts = 16;
			pt2interpFunction = &splineInterp;
			break;

		default:
			numPts = 1;
			pt2interpFunction = &nearestInterp;
			break;
	}

	//initialize indices arrays and Weights matrix
	xIndices = new int[numPts];
	yIndices = new int[numPts];
	W.ReSize(numPts);

	//Perform interpolation to find zi
	(*pt2interpFunction)(terrainMap->map.xpts, terrainMap->map.ypts,
						 terrainMap->map.depths, xi, yi, zi,
						 xIndices, yIndices, W);

	//If returned depth is NaN, extract data from low resolution map
	if(isnan(zi) && terrainMap->refMap->lowResSrc != NULL) {
		double xPt, yPt;
		zi = terrainMap->getNearestLowResMapPoint(xi, yi, xPt, yPt);
		h_sq = pow(xPt - xi, 2) + pow(yPt - yi, 2);
		var = terrainMap->map.depthVariance(xIndices[0] + 1, yIndices[0] + 1) +
			  terrainMap->mapVariogram.evalVariogram(sqrt(h_sq));
	} else {
		//Compute variance associated with interpolation method
		if(W.Nrows() == 1) {
			//If only using nearest neighbor, weight variance based on distance of
			//nearest point to the interpolation point
			h_sq = pow(terrainMap->map.xpts[xIndices[0]] - xi, 2) +
				   pow(terrainMap->map.ypts[yIndices[0]] - yi, 2);
			var = terrainMap->map.depthVariance(xIndices[0] + 1, yIndices[0] + 1) +
				  terrainMap->mapVariogram.evalVariogram(sqrt(h_sq));
		} else {
			var = computeInterpDepthVariance(xIndices, yIndices, W);
		}
	}

	//Compute terrain gradient based on interpolation scheme and returned
	//weights/indices
	computeInterpTerrainGradient(xIndices, yIndices, xi, yi, gradient);

	delete [] xIndices;
	delete [] yIndices;
}
*/

//used by interpolate gradient
void
TerrainMapDEM::
computeInterpTerrainGradient(int* xIndices, int* yIndices, double xi, double yi,  Matrix& gradient) {

    double dx = this->map.dx;
	double dy = this->map.dy;

	//If using bilinear interpolation, compute terrain gradient based on the
	//bilinear interpolation function
	if(this->interpMapMethod == 1) {
		// The four points used for bilinear interp are numbered as:
		//          z11   z12
		//          z21   z22
		// ----------------------------------------------------
        double z11 = this->map.depths(xIndices[0] + 1, yIndices[0] + 1);
        double z21 = this->map.depths(xIndices[1] + 1, yIndices[1] + 1);
        double z12 = this->map.depths(xIndices[2] + 1, yIndices[2] + 1);
        double z22 = this->map.depths(xIndices[3] + 1, yIndices[3] + 1);

		// The resulting bilinear interpolation function is given
		// by:
		//        z(x,y)_interp = b1 + b2x + b3y + b4xy
		// where b2:b4 are defined as (don't need b1 for gradient):
		// ----------------------------------------------------
        double b2 = (1 / (dx * dy)) * (yIndices[0] * (z12 - z22) +
								yIndices[1] * (z21 - z11));
        double b3 = (1 / (dx * dy)) * (xIndices[0] * (z21 - z22) +
								xIndices[2] * (z12 - z11));
        double b4 = (1 / (dx * dy)) * (z11 - z12 - z21 + z22);

		// The associated derivatives are:
		//        dz/dx = b2 + b4y
		//        dz/dy = b3 + b4x
		// ----------------------------------------------------
		gradient(1, 1) = b2 + b4 * yi;
		gradient(1, 2) = b3 + b4 * xi;
	} else {
        //Use simple forward/backward differencing for gradient calculation
		//Compute X gradient
		//If we are at the lower bound, use a forward difference
		if(xIndices[0] == 0) {
			gradient(1, 1) = (1.0 / dx) * (this->map.depths(xIndices[0] + 2, yIndices[0] + 1) - this->map.depths(xIndices[0] + 1,
										   yIndices[0] + 1));
		} else {
			//If we are at the upper bound, use a backward difference,
			//otherwise do central difference
			if(xIndices[0] == this->map.numX - 1) {
				gradient(1, 1) = (1.0 / dx) * (this->map.depths(xIndices[0] + 1, yIndices[0] + 1) - this->map.depths(xIndices[0],
											   yIndices[0] + 1));
			} else {
				gradient(1, 1) = (1.0 / (2.0 * dx)) * (this->map.depths(xIndices[0] + 2,
													   yIndices[0] + 1) - this->map.depths(xIndices[0], yIndices[0] + 1));
			}
		}

		//Compute Y gradient
		//If we are at the lower bound, use a forward difference
		if(yIndices[0] == 0) {
			gradient(1, 2) = (1 / dy) * (this->map.depths(xIndices[0] + 1, yIndices[0] + 2) - this->map.depths(xIndices[0] + 1,
										 yIndices[0] + 1));
		} else {
			//If we are at the upper bound, use a backward difference,
			//otherwise do central difference
			if(yIndices[0] == this->map.numY - 1) {
				gradient(1, 2) = (1.0 / dy) * (this->map.depths(xIndices[0] + 1, yIndices[0] + 1) - this->map.depths(xIndices[0] + 1,
											   yIndices[0]));
			} else {
				gradient(1, 2) = (1.0 / (2.0 * dy)) * (this->map.depths(xIndices[0] + 1,
													   yIndices[0] + 2) - this->map.depths(xIndices[0] + 1, yIndices[0]));
			}
		}
	}
}
