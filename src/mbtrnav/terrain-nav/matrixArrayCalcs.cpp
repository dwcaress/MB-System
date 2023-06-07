/* FILENAME      : matrixArrayCalcs.cpp
 * AUTHOR        : Debbie Meduna
 * DATE          : 01/01/08
 *
 * LAST MODIFIED : 03/30/10
 * MODIFIED BY   : Debbie Meduna
 * -----------------------------------------------------------------------------
 * Modification History
 * -----------------------------------------------------------------------------
 ******************************************************************************/

#include "matrixArrayCalcs.h"
#include "trn_log.h"
#include "MathP.h"

#define _STR(x) #x
#define STR(x) _STR(x)

#ifdef TRN_NORAND
#pragma message( __FILE__":" STR(__LINE__) " - feature TRN_NORAND enabled (see FEATURE_OPTIONS in Makefile)" )
#endif

#ifdef USE_MATLAB
Engine* matlabEng;
#endif

static Real cubWeights[] = {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
							, 0, -0.5, 0, 0, 0, 0, 0, 0, 0, 0.5, 0, 0, 0, 0, 0, 0
							, 0, 1.0, 0, 0, 0, -2.5, 0, 0, 0, 2.0, 0, 0, 0, -0.5, 0, 0
							, 0, -0.5, 0, 0, 0, 1.5, 0, 0, 0, -1.5, 0, 0, 0, 0.5, 0, 0
							, 0, 0, 0, 0, -0.5, 0, 0.5, 0, 0, 0, 0, 0, 0, 0, 0, 0
							, 0.25, 0, -0.25, 0, 0, 0, 0, 0, -0.25, 0, 0.25, 0, 0, 0, 0, 0
							, -0.5, 0, 0.5, 0, 1.25, 0, -1.25, 0, -1, 0, 1, 0, 0.25, 0, -0.25, 0
							, 0.25, 0, -0.25, 0, -0.75, 0, 0.75, 0, 0.75, 0, -0.75, 0,
							-0.25, 0, 0.25, 0
							, 0, 0, 0, 0, 1, -2.5, 2, -0.5, 0, 0, 0, 0, 0, 0, 0, 0
							, -0.5, 1.25, -1, 0.25, 0, 0, 0, 0, 0.5, -1.25, 1, -0.25, 0, 0, 0, 0
							, 1, -2.5, 2, -0.5, -2.5, 6.25, -5, 1.25, 2, -5, 4, -1, -0.5,
							1.25, -1, 0.25
							, -0.5, 1.25, -1, 0.25, 1.5, -3.75, 3, -0.75, -1.5, 3.75, -3
							, 0.75, 0.5, -1.25, 1, -0.25
							, 0, 0, 0, 0, -0.5, 1.5, -1.5, 0.5, 0, 0, 0, 0, 0, 0, 0, 0
							, 0.25, -0.75, 0.75, -0.25, 0, 0, 0, 0, -0.25, 0.75, -0.75,
							0.25, 0, 0, 0, 0
							, -0.5, 1.5, -1.5, 0.5, 1.25, -3.75, 3.75, -1.25, -1, 3, -3,
							1, 0.25, -0.75, 0.75, -0.25
							, 0.25, -0.75, 0.75, -0.25, -0.75, 2.25, -2.25, 0.75, 0.75
							, -2.25, 2.25, -0.75, -0.25, 0.75, -0.75, 0.25
						   };


#ifdef _QNX
int round(const double& num) {
	double rem = num - floor(num);
	if(rem >= 0.5) {
		return int(ceil(num));
	} else {
		return int(floor(num));
	}
}
#endif

/* Function: seed_randn(unsigned int *p_seed)
 * Usage:    unsigned int seed = 27;
 *           seed_randn(&seed);
 * -------------------------------------------------------------------------*/
/*! Seed the random number generator using:
 *    - the seed pointed to by p_seed if p_seed is not NULL.
 *    - if p_seed is NULL, use the result of time(NULL) as the seed
 *    - zero if the compiler switch TRN_NORAND is set. 
 *  Returns the seed used.
 */
unsigned int seed_randn(unsigned int *p_seed)
{

#ifdef TRN_NORAND
   // Building to use the same set of random numbers
    unsigned int seed = 0;
#else
    unsigned int seed = (NULL != p_seed)? *p_seed : time(NULL);
#endif

	//Initialize random number generator
   srand(seed);
   return seed;
}


int minVal(const int* values, const int numValues) {
	int minVal = 10000;
	for(int i = 0; i < numValues; i++) {
		if(values[i] < minVal) {
			minVal = values[i];
		}
	}
	
	return minVal;
}

int maxVal(const int* values, const int numValues) {
	int maxVal = -10000;
	for(int i = 0; i < numValues; i++) {
		if(values[i] > maxVal) {
			maxVal = values[i];
		}
	}
	
	return maxVal;
}

Matrix conv2(const Matrix& A, const Matrix& H) {
	int filterSizeX, filterSizeY;
	Matrix tempMAT = H;
	Matrix newMAT = A;
	
	//filterSize corresponds to the number of rows/cols in the filter H
	filterSizeX = 2 * (int(floor(H.Nrows() / 2.0)));
	filterSizeY = 2 * (int(floor(H.Ncols() / 2.0)));
	
#if 0 //slower method of convolution, not currently being used
	int subIndices[4];
	//Compute convolution by applying filter H over all points in A
	for(int row = 1; row <= A.Nrows(); row++) {
		for(int col = 1; col <= A.Ncols(); col++) {
			//subIndices describes portion of H which has related values in A.
			subIndices[0] = 2 - min(1, row - filterSize / 2);
			subIndices[1] = filterSize / 2 + 1 + min(filterSize / 2, A.Nrows() - row);
			subIndices[2] = 2 - min(1, col - filterSize / 2);
			subIndices[3] = filterSize / 2 + 1 + min(filterSize / 2, A.Ncols() - col);
			
			tempMAT = 0.0;
			tempMAT.SubMatrix
			(subIndices[0], subIndices[1], subIndices[2], subIndices[3])
				= SP(H.SubMatrix(subIndices[0], subIndices[1], subIndices[2],
								 subIndices[3]),
					 A.SubMatrix(max(row - filterSize / 2, 1), min(row + filterSize / 2,
								 A.Nrows()),
								 max(col - filterSize / 2, 1), min(col + filterSize / 2,
										 A.Ncols())));
			newMAT(row, col) = tempMAT.Sum();
		}
	}
#endif
	
	
	//Alternative method: apply A to all points in H
	tempMAT.ReSize(A.Nrows() + filterSizeX, A.Ncols() + filterSizeY);
	tempMAT = 0.0;
	tempMAT.SubMatrix(filterSizeX / 2 + 1, A.Nrows() + filterSizeX / 2, filterSizeY / 2 + 1,
					  A.Ncols() + filterSizeY / 2) = A;
	newMAT = 0.0;
	for(int row = 1; row <= H.Nrows(); row++) {
		for(int col = 1; col <= H.Ncols(); col++) {
			newMAT += H(row, col) *
					  tempMAT.SubMatrix(row, A.Nrows() + row - 1, col, A.Ncols() + col - 1);
		}
	}
	
	return newMAT;
}

#ifdef WITH_RESAMPLE_MAP
#if 0
// TODO remove interp2 (unused, called by mapT::reSampleMap in structDefs.cpp)
void interp2(double* xpts, double* ypts, const Matrix Z, double* xi,
			 double* yi, double* zi, int numPts) {
	int yIndices[4];
	int xIndices[4];
	ColumnVector Weights(4);
	// klh: xi,yi,zi are being indexed as arrays here with no boundary checks.
    // xi, yi may or may not be arrays, and are being passed in (by interp2mat)
    // from potentially non-zero indices.
    // zi is defined and passed (by interp2mat) as a double (array dimension 1).
    // IOW, there is potential for array boundary violations.
	for(int i = 0; i < numPts; i++)
		bilinearInterp(xpts, ypts, Z, xi[i], yi[i], zi[i], xIndices,
					   yIndices, Weights);
					   
	return;
}

// TODO remove interp2mat (unused, called by mapT::reSampleMap in structDefs.cpp)
void interp2mat(double* xpts, double* ypts, const Matrix Z, double* xi,
				double* yi, Matrix& zi) {
	double znum;
	
	for(int i = 0; i < zi.Nrows(); i++) {
		for(int j = 0; j < zi.Ncols(); j++) {
			interp2(xpts, ypts, Z, &xi[i], &yi[j], &znum, 1);
			zi(i + 1, j + 1) = znum;
		}
	}
	
	return;
}
#endif
#endif

void nearestInterp(double* xpts, double* ypts, const Matrix& zvals,
				   double xi, double yi, double& zi, int* xIndices,
				   int* yIndices, ColumnVector& Weights) {
	Weights.ReSize(1);
	Weights(1) = 1;
	
	xIndices[0] = closestPtUniformArray(xi, xpts[0], xpts[zvals.Nrows() - 1], zvals.Nrows());
	yIndices[0] = closestPtUniformArray(yi, ypts[0], ypts[zvals.Ncols() - 1], zvals.Ncols());
	
	zi = zvals(xIndices[0] + 1, yIndices[0] + 1);
	
	//if the nearest value in the array is NaN, search radially
	//outward from the nearest point to find the closest non-NaN
	//grid value
	
	if(ISNIN(zi)) {
		xIndices[0]++;
		yIndices[0]++;
		findNearestValid(zvals, xIndices[0], yIndices[0]);
		
		xIndices[0]--;
		yIndices[0]--;
		zi = zvals(xIndices[0] + 1, yIndices[0] + 1);
	}
	
}

bool findNearestValid(const Matrix& zvals, int& xIndex, int& yIndex) {
    //radius power
	int rPower = 0;
    //radius base
    int rBase = 2;
	int maxRadius = 32;
	
	if(!ISNIN(zvals(xIndex, yIndex))) {
		return true;
	}

    //radius of search away from nominal point
	int r = (int)(pow(rBase, rPower));
	//   while ((r < zvals.Nrows()) || (r < zvals.Ncols()))
	while(r < maxRadius) {
		//search side columns
		for(int i = -r; i <= r; i++) {
			//check that we are indexing within the grid
			if((xIndex + i < 1) || (xIndex + i > zvals.Nrows())) {
				continue;
			}
			
			for(int j = -r; j <= r; j = j + 2 * r) {
				//check that we are indexing within the grid
				if((yIndex + j < 1) || (yIndex + j > zvals.Ncols())) {
					continue;
				}
				
				if(!ISNIN(zvals(xIndex + i, yIndex + j))) {
					xIndex += i;
					yIndex += j;
					return true;
				}
			}
		}
		
		//search top and bottom rows
		for(int j = -r + 1; j <= r - 1; j++) {
			//check that we are indexing within the grid
			if((yIndex + j < 1) || (yIndex + j > zvals.Ncols())) {
				continue;
			}
			
			for(int i = -r; i <= r; i = i + 2 * r) {
				//check that we are indexing within the grid
				if((xIndex + i < 1) || (xIndex + i > zvals.Nrows())) {
					continue;
				}
				
				if(!ISNIN(zvals(xIndex + i, yIndex + j))) {
					xIndex += i;
					yIndex += j;
					return true;
				}
			}
		}
		
		rPower++;
		r = (int)(pow(rBase, rPower));
	}
	
	return false;
}


void bilinearInterp(double* xpts, double* ypts, const Matrix& zvals,
					double xi, double yi, double& zi, int* xIndices,
					int* yIndices, ColumnVector& Weights) {
	double dx, dy, t, u;
	double lowerX, lowerY, upperX, upperY;
	int i, x1, y1;
	
	Weights.ReSize(4);
	
	//find the lowest closest point in the reference data to desired point
	x1 = lowerBound(xi, xpts, zvals.Nrows());
	y1 = lowerBound(yi, ypts, zvals.Ncols());
	
	//ensure that the bounding box falls within the extracted map size
	//if the bounding box is outside the map, use nearestInterp
	if((x1 >= zvals.Nrows() - 1) || (x1 < 0) || (y1 >= zvals.Ncols() - 1) || (y1 < 0)) {
		nearestInterp(xpts, ypts, zvals, xi, yi, zi, xIndices,
					  yIndices, Weights);
		for(i = 1; i < 4; i++) {
			xIndices[i] = 0;
			yIndices[i] = 0;
		}
		return;
	}
	
	//define surrounding four corners in the grid
	lowerX = xpts[x1];
	upperX = xpts[x1 + 1];
	lowerY = ypts[y1];
	upperY = ypts[y1 + 1];
	
	dx = upperX - lowerX;
	dy = upperY - lowerY;
	
	/*Define interpolation indices into zvals
	 *The four interpolation points are labeled as follows:
	 * 0  2
	 * 1  3 */
	xIndices[0] = x1;
	yIndices[0] = y1;
	xIndices[1] = x1 + 1;
	yIndices[1] = y1;
	xIndices[2] = x1 + 1;
	yIndices[2] = y1 + 1;
	xIndices[3] = x1;
	yIndices[3] = y1 + 1;
	
	//Define bilinear interpolation weights
	t = (xi - lowerX) / dx;
	u = (yi - lowerY) / dy;
	Weights(1) = (1 - t) * (1 - u);
	Weights(2) = t * (1 - u);
	Weights(3) = t * u;
	Weights(4) = (1 - t) * u;
	
	//Compute interpolated zi by weighted sum of the four z points
	zi = 0.0;
	for(i = 0; i < 4; i++) {
		zi += Weights(i + 1) * zvals(xIndices[i] + 1, yIndices[i] + 1);
	}
	
	//Check that the interpolated value is not NaN, otherwise, try nearestInterp
	if(ISNIN(zi)) {
		nearestInterp(xpts, ypts, zvals, xi, yi, zi, xIndices,
					  yIndices, Weights);
					  
		for(i = 1; i < 4; i++) {
			xIndices[i] = 0;
			yIndices[i] = 0;
		}
	}
	
	return;
}

void bicubicInterp(double* xpts, double* ypts, const Matrix& zvals,
				   double xi, double yi, double& zi, int* xIndices,
				   int* yIndices, ColumnVector& Weights) {
	int i, j, x1, y1;
	Matrix W(16, 16);
	ColumnVector b(16);
	double dx, dy, t, u;
	Weights.ReSize(16);
	
	//find the lowest closest point in the reference data to desired point
	x1 = lowerBound(xi, xpts, zvals.Nrows());
	y1 = lowerBound(yi, ypts, zvals.Ncols());
	
	/*Define interpolation indices into zvals
	 *The sixteen interpolation points are labeled as follows:
	 *12 13 14 15
	 * 8  9 10 11
	 * 4  5  6  7
	 * 0  1  2  3*/
	xIndices[5] = x1;
	yIndices[5] = y1;
	
	//ensure that the bounding box falls within the extracted map size
	//if bounding box is outside the map, attempt bilinear interpolation
	if((x1 + 2 > zvals.Nrows() - 1) || (x1 - 1 < 0) || (y1 + 2 > zvals.Ncols() - 1)
			|| (y1 - 1 < 0)) {
		bilinearInterp(xpts, ypts, zvals, xi, yi, zi, xIndices,
					   yIndices, Weights);
		for(i = 4; i < 16; i++) {
			xIndices[i] = 0;
			yIndices[i] = 0;
		}
		return;
	}
	
	//define indices of interpolation z points
	for(i = 0; i < 16; i = i + 4) {
		xIndices[i] = xIndices[5] - 1;
	}
	for(i = 1; i < 16; i = i + 4) {
		xIndices[i] = xIndices[5];
	}
	for(i = 2; i < 16; i = i + 4) {
		xIndices[i] = xIndices[5] + 1;
	}
	for(i = 3; i < 16; i = i + 4) {
		xIndices[i] = xIndices[5] + 2;
	}
	
	for(i = 0; i < 4; i++) {
		yIndices[i] = yIndices[5] - 1;
	}
	for(i = 4; i < 8; i++) {
		yIndices[i] = yIndices[5];
	}
	for(i = 8; i < 12; i++) {
		yIndices[i] = yIndices[5] + 1;
	}
	for(i = 12; i < 16; i++) {
		yIndices[i] = yIndices[5] + 2;
	}
	
	//define t and u parameters to be used for calculating interpolation
	//weights
	dx = xpts[xIndices[6]] - xpts[xIndices[5]];
	dy = ypts[yIndices[9]] - ypts[yIndices[5]];
	t = (xi - xpts[x1]) / dx;
	u = (yi - ypts[y1]) / dy;
	
	
	//Define interpolation weights
	for(i = 1; i <= 4; i++) {
		for(j = 1; j <= 4; j++) {
			b((i - 1) * 4 + j) = pow(t, i - 1) * pow(u, j - 1);
		}
	}
	
	W << cubWeights;
	Weights = W.t() * b;
	
	//Determine z value based on interpolation weights:
	zi = 0.0;
	for(i = 0; i < 16; i++) {
		zi += Weights(i + 1) * zvals(xIndices[i] + 1, yIndices[i] + 1);
	}
	
	//Check that the interpolated value is not NaN, otherwise,try bilinearInterp
	if(ISNIN(zi)) {
		bilinearInterp(xpts, ypts, zvals, xi, yi, zi, xIndices,
					   yIndices, Weights);
		for(i = 4; i < 16; i++) {
			xIndices[i] = 0;
			yIndices[i] = 0;
		}
	}
	return;
}

void splineInterp(double* xpts, double* ypts, const Matrix& zvals,
				  double xi, double yi, double& zi, int* xIndices,
				  int* yIndices, ColumnVector& Weights) {
				  
#ifdef USE_MATLAB
	mxArray* Z = NULL;
	mxArray* X = NULL;
	mxArray* Y = NULL;
	mxArray* Xi;
	mxArray* Yi;
	mxArray* result = NULL;
	
	//copy contents of Surf into Matlab variable A
	Z = mxCreateDoubleMatrix(zvals.Ncols(), zvals.Nrows(), mxREAL);
	memcpy((void*)mxGetPr(Z), (void*) zvals.Store(), zvals.Storage()*sizeof(double));
	
	X = mxCreateDoubleMatrix(1, zvals.Nrows(), mxREAL);
	memcpy((void*)mxGetPr(X), (void*) xpts, sizeof(xpts));
	
	Y = mxCreateDoubleMatrix(1, zvals.Ncols(), mxREAL);
	memcpy((void*)mxGetPr(Y), (void*) ypts, sizeof(ypts));
	
	Xi = mxCreateDoubleMatrix(1, 1, mxREAL);
	memcpy(mxGetPr(Xi), (void*)&xi, sizeof(double));
	
	Yi = mxCreateDoubleMatrix(1, 1, mxREAL);
	memcpy(mxGetPr(Yi), (void*)&yi, sizeof(double));
	
	//put data into Matlab workspace
	engPutVariable(matlabEng, "Z", Z);
	engPutVariable(matlabEng, "X", X);
	engPutVariable(matlabEng, "Y", Y);
	engPutVariable(matlabEng, "Yi", Yi);
	engPutVariable(matlabEng, "Xi", Xi);
	
	//compute interpolation in Maltab
	engEvalString(matlabEng, "[Xm,Ym] = meshgrid(X,Y);");
	engEvalString(matlabEng, "Zi = interp2(Xm,Ym,Z,Xi,Yi,'spline');");
	engEvalString(matlabEng, "figure(10);");
	engEvalString(matlabEng, "surf(Zi, 'Linestyle', 'none');");
	
	//extract interpolated value from Matlab
	result = engGetVariable(matlabEng, "Zi");
	memcpy((void*)&zi, (void*) mxGetPr(result), sizeof(double));
	
	//remove memory in Matlab
	mxDestroyArray(Z);
	mxDestroyArray(X);
	mxDestroyArray(Y);
	mxDestroyArray(Xi);
	mxDestroyArray(Yi);
	mxDestroyArray(result);
	
#else
	nearestInterp(xpts, ypts, zvals, xi, yi, zi, xIndices, yIndices, Weights);
	
#endif
	
}


void nearestInterp_mat(double* xpts, double* ypts, const Matrix& zvals,
					   double* xi, double* yi, Matrix& zi, Matrix& var) { //TODO Matrix var unused?
	int* xIndices = NULL;
	int* yIndices = NULL;
	xIndices = new int[1];
	yIndices = new int[1];
	ColumnVector W;
	
	//perform nearest-neighbor interpolation for each point in (xi,yi)
	for(int i = 1; i <= zi.Nrows(); i++) {
		for(int j = 1; j <= zi.Ncols(); j++)
			nearestInterp(xpts, ypts, zvals, xi[i - 1], yi[j - 1], zi(i, j),
						  xIndices, yIndices, W);
	}
	
	delete [] xIndices;
	delete [] yIndices;
	
	return;
}

void bilinearInterp_mat(double* xpts, double* ypts, const Matrix& zvals,
						double* xi, double* yi, Matrix& zi, Matrix& var) { //TODO Matrix var unused?
	int* xIndices = NULL;
	int* yIndices = NULL;
	xIndices = new int[4];
	yIndices = new int[4];
	ColumnVector W;
	
	//perform nearest-neighbor interpolation for each point in (xi,yi)
	for(int i = 1; i <= zi.Nrows(); i++) {
		for(int j = 1; j <= zi.Ncols(); j++)
			bilinearInterp(xpts, ypts, zvals, xi[i - 1], yi[j - 1], zi(i, j),
						   xIndices, yIndices, W);
	}
	
	delete [] xIndices;
	delete [] yIndices;
	
	return;
}

void bicubicInterp_mat(double* xpts, double* ypts, const Matrix& zvals,
					   double* xi, double* yi, Matrix& zi, Matrix& var) { //TODO Matrix var unused?
	int* xIndices = NULL;
	int* yIndices = NULL;
	xIndices = new int[16];
	yIndices = new int[16];
	ColumnVector W;
	
	//perform nearest-neighbor interpolation for each point in (xi,yi)
	for(int i = 1; i <= zi.Nrows(); i++) {
		for(int j = 1; j <= zi.Ncols(); j++)
			bicubicInterp(xpts, ypts, zvals, xi[i - 1], yi[j - 1], zi(i, j),
						  xIndices, yIndices, W);
	}
	
	delete [] xIndices;
	delete [] yIndices;
	
	return;
}

void splineInterp_mat(double* xpts, double* ypts, const Matrix& zvals,
					  double* xi, double* yi, Matrix& zi, Matrix& var) {
#ifdef USE_MATLAB
	mxArray* Z = NULL;
	mxArray* X = NULL;
	mxArray* Y = NULL;
	mxArray* Xi;
	mxArray* Yi;
	mxArray* result = NULL;
	
	//copy contents of Surf into Matlab variable A
	Z = mxCreateDoubleMatrix(zvals.Ncols(),
							 zvals.Nrows(), mxREAL);
	memcpy((void*)mxGetPr(Z), (void*) zvals.Store(), zvals.Storage()*sizeof(double));
	
	X = mxCreateDoubleMatrix(1, zvals.Nrows(), mxREAL);
	memcpy((void*)mxGetPr(X), (void*) xpts, zvals.Nrows()*sizeof(double));
	
	Y = mxCreateDoubleMatrix(1, zvals.Ncols(), mxREAL);
	memcpy((void*)mxGetPr(Y), (void*) ypts, zvals.Ncols()*sizeof(double));
	
	Xi = mxCreateDoubleMatrix(1, zi.Nrows(), mxREAL);
	Yi = mxCreateDoubleMatrix(zi.Ncols(), 1, mxREAL);
	
	//put data into Matlab workspace
	engPutVariable(matlabEng, "Z", Z);
	engPutVariable(matlabEng, "X", X);
	engPutVariable(matlabEng, "Y", Y);
	engEvalString(matlabEng, "[Xm,Ym] = meshgrid(X,Y);");
	
	memcpy((void*)mxGetPr(Xi), (void*) xi,
		   zi.Nrows()*sizeof(double));
	memcpy((void*)mxGetPr(Yi), (void*) yi,
		   zi.Ncols()*sizeof(double));
	engPutVariable(matlabEng, "Yi", Yi);
	engPutVariable(matlabEng, "Xi", Xi);
	engEvalString(matlabEng, "[Xd,Yd] = meshgrid(Xi,Yi);");
	
	engEvalString(matlabEng,
				  "Zi = interp2(Xm,Ym,Z,Xi,Yi,'spline');");
	result = engGetVariable(matlabEng, "Zi");
	memcpy((void*)zi.Store(), (void*) mxGetPr(result),
		   zi.Storage()*sizeof(double));
		   
	//remove memory in Matlab
	mxDestroyArray(Z);
	mxDestroyArray(X);
	mxDestroyArray(Y);
	mxDestroyArray(Xi);
	mxDestroyArray(Yi);
	mxDestroyArray(result);
	
#endif
	
	nearestInterp_mat(xpts, ypts, zvals, xi, yi, zi, var);
	
	return;
}

int closestPt(double key, const double* base, size_t nmemb) {

	int i = 0, idx = 0, j;
	double a, dt, dt0, minValue, maxValue;
	maxValue = *(base + (nmemb - 1));
	minValue = *base;
	
	//If key is larger than the largest value in the array OR
	//the key is smaller than the smallest value, return the
	//largest or smallest array value accordingly
	if(key > maxValue) {
		idx = nmemb - 1;
	} else if(key < minValue) {
		idx = 0;
	} else {
		dt0 = maxValue;
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


int lowerBound(double val, const double* vec, int numVals) {
	int nearestIndex;
	int lowerIndex;
	double diff;
	
	nearestIndex = closestPtUniformArray(val, vec[0], vec[numVals - 1], numVals);
	diff = val - vec[nearestIndex];
	
	if(diff >= 0) {
		lowerIndex = nearestIndex;
	} else {
		lowerIndex = nearestIndex - 1;
	}
	
	return lowerIndex;
}

double randn(double mean, double stddev) {
    double gauss1=0.;
	static bool use_last = false;
	static double gauss2;
	
	//If we already have a random variable waiting to be used, use it
	if(use_last) {
		gauss1 = gauss2;
		use_last = false;
	}
	//Otherwise, generate two new random numbers
	else {
        double rand1, rand2, w;
		//Use the Polar Form of the Box-Muller transformation:
		//1.Generate two uniform random variables within the unit circle
		do {
			//Generate two uniform random variables between -1 and 1
            rand1 = unif(0, 1);
            rand2 = unif(0, 1);
            w = rand1 * rand1 + rand2 * rand2;
		} while(w >= 1.0);
		
		//2.Convert random variables to gaussian variables, N(0,1).
		w = sqrt(-2.0 * log(w) / w);
		gauss1 = rand1 * w;
		gauss2 = rand2 * w;
		
		//Set flag that we have an extra variable that can be used
		use_last = true;
	}
	
	return (mean + stddev * gauss1);
	
}


char* charCat(char* dest, const char* front, const char* back) {
	snprintf(dest, strlen(front)+strlen(back)+1, "%s%s", front, back);
	
	return dest;
}


double computeKLdiv_gaussian_mat(double* xpts, double* ypts,
								 const Matrix& refPDF,
								 double* mu, const Matrix& Cov) {
	Matrix A(Cov.Nrows(), Cov.Ncols());
	Matrix Value(1, 1);
	ColumnVector dx(2);
	double q;
	int i, j;
	double eta;
	double kl = 0;
	
	//compute gaussian normalization factor
	A = 2.0 * PI * Cov;
	eta = pow(A.Determinant(), -0.5);
	
	//compute inverse of covariance for gaussian calculation
	A = Cov.i();
	
	//sum KL over all entries in refPDF
	for(i = 1; i <= refPDF.Nrows(); i++) {
		dx(1) = xpts[i - 1] - mu[0];
		for(j = 1; j <= refPDF.Ncols(); j++) {
			//compute current gaussian probability
			dx(2) = ypts[j - 1] - mu[1];
			Value = dx.t() * A * dx;
			q = eta * exp(Value.AsScalar() * -0.5);
			
			//add current kl entry
			if(refPDF(i, j) / q > 1e-50 && refPDF(i, j) / q < 1e50) {
				kl += refPDF(i, j) * log(refPDF(i, j) / q);
			}
		}
	}
	
	return kl;
}

SymmetricMatrix computeMatrixSqrt(const SymmetricMatrix& A) {
	SymmetricMatrix Asqrt;
	DiagonalMatrix D;
	Matrix V;
	int i;
	
	//compute eigenvalue decomposition
	Jacobi(A, D, V);
	
	//compute sqrt of eigenvalue matrix
	for(i = 1; i <= A.Nrows(); i++) {
		//ensure positive definite matrix prior to taking the sqrt
		if(D(i) < 0) {
			logs(TL_OMASK(TL_MATRIX_ARRAY_CALCS, TL_LOG),"Error: Tried to take the sqrt of a non-positive definite "
				   "matrix. Exiting...\n");
			exit(0);
		}
		D(i) = sqrt(D(i));
	}
	
	//reconstruct sqrt matrix
	Asqrt << V* D* V.t();
	
	return Asqrt;
}


