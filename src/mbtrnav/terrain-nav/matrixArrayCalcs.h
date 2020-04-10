/*! \file          matrixArrayCalcs.h
 *  \brief         matrixArrayCalcs defines various utility functions for use 
 *                 with Matrix objects and array objects.
 *  \author        Debbie Meduna
 *  \date          01/01/08 */
/* DEPENDENCIES  : newmat*.h
 *
 * LAST MODIFIED : 11/29/10
 * MODIFIED BY   : Debbie Meduna 
 * ----------------------------------------------------------------------------
 * Modification History:
 * ----------------------------------------------------------------------------
 *****************************************************************************/

#ifndef _matrixArrayCalcs_h_
#define _matrixArrayCalcs_h_

#include "myOutput.h"

#include "math.h"
#include "string.h"
#include <fstream>

#include <newmatap.h>
#include <newmatio.h>

#ifdef USE_MATLAB
#include "engine.h"
extern Engine* matlabEng;
#endif

//#define max(a,b)  (((a) > (b)) ? (a) : (b))
//#define min(a,b)  (((a) < (b)) ? (a) : (b))
#ifdef use_namespace
using namespace std;
using namespace NEWMAT;
#endif

// #undef isnan
// #define isnan(a)  ((fabs(a) >= 90000) || (a != a))

// Rename this macro to avoid conflict with native C isnan()
// ISNIN == is not initialized.
// Redefine NINVAL to suit needs
// 
#define NINVAL 90000    
#define ISNIN(a)  ((fabs(a) >= NINVAL) || (a != a))

#define sign(a)   ((a > 0) - (a < 0))

#ifndef PI
#define PI 3.14159265358979
#endif

/* Function: round
 * Usage: roundedNum = round(num);
 * -------------------------------------------------------------------------*/
/*! Rounds the number given to the nearest integer and returns the integer.
 */
#ifdef _QNX 
int round(const double &num);
#endif

/* Function: minVal
 * Usage: minNum = minVal(values, numValues);
 * -------------------------------------------------------------------------*/
/*! Finds and returns the minimum number in the integer array values.  
 */
int minVal(const int* values, const int numValues);


/* Function: maxVal
 * Usage: maxNum = maxVal(values, numValues);
 * -------------------------------------------------------------------------*/
/*! Finds and returns the maximum number in the integer array values.  
 */
int maxVal(const int* values, const int numValues);


/* Function: conv2
 * Usage: B = conv2(A,H);
 * -------------------------------------------------------------------------*/
/*! Computes the 2D convolution of A with filter matrix H. The returned matrix
 * B is the same size as A. H is assumed to be smaller than A.  
 */
Matrix conv2(const Matrix &A, const Matrix &H);


/* Function: interp2
 * Usage: interp2(xpts,ypts,Z,xi,yi,zi,N);
 * -------------------------------------------------------------------------*/
/*! Interpolates the matrix Z to the (x,y) point pairs in arrays xi and yi.
 * N indicates the number of point pairs to be interpolated.  The arrays
 * xpts and ypts contain the (x,y) values corresponding to entries in matrix
 * Z.  If Z is size m x n, xpts is size m x 1 and ypts is size n x 1.
 * Bilinear interpolation is used for this function.
 */
void interp2(double* xpts, double* ypts, const Matrix Z, double* xi, 
	     double* yi, double* zi, int numPts);


/* Function: interp2mat
 * Usage: interp2mat(xpts,ypts,Z,xi,yi,zi,Nx,Ny);
 * -------------------------------------------------------------------------*/
/*! Interpolates the matrix Z to a new matrix zi over the (x,y) points in
 * the arrays xi and yi.  Nx and Ny indicates the length of these arrays.
 * The arrays xpts and ypts contain the (x,y) values corresponding to entries 
 * in matrix Z.  If Z is size m x n, xpts is size m x 1 and ypts is size n x 1.
 * This function calls interp2() for performing interpolations.
 */
void interp2mat(double* xpts, double* ypts, const Matrix Z, double* xi, 
		double* yi, Matrix &zi);


/* Function: nearestInterp
 * Usage: nearestInterp(xpts, ypts, Z, xi, yi, zi, xIndices, yIndices, W);
 * --------------------------------------------------------------------------*/
/*! Uses nearest-neighbor interpolation to determine zi at the point (xi,yi) 
 * from the set of data xpts, ypts, and zvals.  If zvals is size m x n, 
 * xpts is size m x 1 and ypts is size n x 1.  The function modifies the 
 * the entries in xIndices, yIndices and W.  xIndices and yIndices are px1 
 * arrays containing the indices of xpts and ypts used to generate zi. W is 
 * a px1 matrix of corresponding weights where:
 * zi = sum_i W(i,1)*zvals(xIndices[i]+1,yIndices[i]+1).
 * If the nearest point is NaN, the closest non-NaN point is found.
 */
void nearestInterp(double* xpts, double* ypts, const Matrix &zvals, 
		   double xi, double yi, double &zi, int* xIndices, 
		   int* yIndices, ColumnVector &Weights);


/* Function: findNearestValid
 * Usage: findNearestValid(zvals, xIndex, yIndex);
 * --------------------------------------------------------------------------*/
/*! Finds the nearest valid (non-NaN) point in the grid zvals to the point 
 * (xIndex, yIndex). Replaces xIndex and yIndex with the indices corresponding
 * to this found location.  Returns a boolean indicating if a non-NaN point was
 * successfully found. 
 */
bool findNearestValid(const Matrix &zvals, int &xIndex, int &yIndex);


/* Function: bilinearInterp
 * Usage: bilinearInterp(xpts, ypts, Z, xi, yi, zi, xIndices, yIndices, W);
 * --------------------------------------------------------------------------*/
/*! Uses bilinear interpolation to determine zi at the point (xi,yi) 
 * from the set of data xpts, ypts, and zvals.  If zvals is size m x n, 
 * xpts is size m x 1 and ypts is size n x 1.  The function modifies the 
 * the entries in xIndices, yIndices and W.  xIndices and yIndices are 4x1 
 * arrays containing the indices of xpts and ypts used to generate zi. W is 
 * a 4x1 column vector of corresponding weights where:
 * zi = sum_i W(i,1)*zvals(xIndices[i]+1,yIndices[i]+1).
 * If a 2x2 bounding box does not exist for the current (xi,yi) data point 
 * (i.e., it is out of the bounds of Z), OR the interpolated point is NaN,
 * nearestInterp() is used.
 */
void bilinearInterp(double* xpts, double* ypts, const Matrix &zvals, 
		    double xi, double yi, double &zi, int* xIndices, 
		    int* yIndices, ColumnVector &Weights);


/* Function: bicubicInterp
 * Usage: bicubicInterp(xpts, ypts, Z, xi, yi, zi, xIndices, yIndices, W);
 * --------------------------------------------------------------------------*/
/*! Uses bicubic interpolation to determine zi at the point (xi,yi) 
 * from the set of data xpts, ypts, and zvals.  If zvals is size m x n, 
 * xpts is size m x 1 and ypts is size n x 1.  The function modifies the 
 * the entries in xIndices, yIndices and W.  xIndices and yIndices are px1 
 * arrays containing the indices of xpts and ypts used to generate zi. 
 * The method used for bicubic interpolation is very similar to that described
 * in Numerical Recipes in C++.
 * If a 4x4 bounding box does not exist for the current (xi,yi) data point,
 * OR the interpolated point is NaN, bilinearInterp() is used.
 */
void bicubicInterp(double* xpts, double* ypts, const Matrix &zvals, 
		   double xi, double yi, double &zi, int* xIndices, 
		   int* yIndices, ColumnVector &Weights);


/* Function: splineInterp
 * Usage: splineInterp(xpts, ypts, Z, xi, yi, zi, xIndices, yIndices, W);
 * --------------------------------------------------------------------------*/
/*! Uses spline interpolation to determine zi at the point (xi,yi) 
 * from the set of data xpts, ypts, and zvals.  If zvals is size m x n, 
 * xvals is size m x 1 and ypts is size n x 1.  The function modifies the 
 * the entries in xIndices, yIndices and W.  xIndices and yIndices are px1 
 * arrays containing the indices of xpts and ypts used to generate zi. 
 * Currently, this function is only implemented if a Matlab engine is running.
 * Otherwise, this function calls nearestInterp().
 */
void splineInterp(double* xpts, double* ypts, const Matrix &zvals, 
		  double xi, double yi, double &zi, int* xIndices, 
		  int* yIndices, ColumnVector &Weights);


/* Function: nearestInterp_mat
 * Usage: nearestInterp_mat(xpts, ypts, Z, xi, yi, zi, var);
 * --------------------------------------------------------------------------*/
/*! Interpolates all points in the vectors xi, yi to fill the matrix zi using
 *  the data in xpts, ypts and Z.  Calls nearestInterp() for each point in 
 *  the xi and yi arrays to perform this interpolation.
 */
void nearestInterp_mat(double* xpts, double* ypts, const Matrix &zvals, 
		       double* xi, double* yi, Matrix &zi, Matrix &var);


/* Function: bilinearInterp_mat
 * Usage: bilinearInterp_mat(xpts, ypts, Z, xi, yi, zi, var);
 * --------------------------------------------------------------------------*/
/*! Interpolates all points in the vectors xi, yi to fill the matrix zi using
 *  the data in xpts, ypts and Z.  Calls bilinearInterp() for each point in 
 *  the xi and yi arrays to perform this interpolation.
 */
void bilinearInterp_mat(double* xpts, double* ypts, const Matrix &zvals, 
			double* xi, double* yi, Matrix &zi, Matrix &var);


/* Function: bicubicInterp_mat
 * Usage: bicubicInterp_mat(xpts, ypts, Z, xi, yi, zi, var);
 * --------------------------------------------------------------------------*/
/*! Interpolates all points in the vectors xi, yi to fill the matrix zi using
 *  the data in xpts, ypts and Z.  Calls bicubicInterp() for each point in 
 *  the xi and yi arrays to perform this interpolation.
 */
void bicubicInterp_mat(double* xpts, double* ypts, const Matrix &zvals, 
		       double* xi, double* yi, Matrix &zi, Matrix &var);


/* Function: splineInterp_mat
 * Usage: splineInterp_mat(xpts, ypts, Z, xi, yi, zi, var);
 * --------------------------------------------------------------------------*/
/*! Interpolates all points in the vectors xi, yi to fill the matrix zi using
 *  the data in xpts, ypts and Z.  Currently, this function is only implemented
 *  if a Matlab engine is running. Otherwise, this function calls 
 *  nearestInterp_mat().
 */
void splineInterp_mat(double* xpts, double* ypts, const Matrix &zvals, 
		      double* xi, double* yi, Matrix &zi, Matrix &var);  


/* Function: closestPt
 * Usage: closestPt(value, pts, numPts);
 * -------------------------------------------------------------------------*/
/*! Finds the index into the array "pts" which is closest to "value". This 
 *  function is copied from the function nearest() in mapio.cpp.
 */
int closestPt(double key, const double *base, size_t nmemb);


/* Function: closestPtUniformArray
 * Usage: closestPt(value, firstPt, lastPt, numPts);
 * -------------------------------------------------------------------------*/
/*! Finds the index closest to "value" for the array whose first element is 
 *  "firstVal", last element is "lastVal", and has size "nmemb". This function
 *  assumes that the array specified is uniformly spaced and takes advantage of
 *  this to perform a faster search than closestPt().
 */
inline int closestPtUniformArray(double key, double firstVal, 
                          double lastVal, size_t nmemb)
{
   int idx = 0;
   double dt;

   //If key is larger than the largest value in the array OR
   //the key is smaller than the smallest value, return the 
   //largest or smallest array value accordingly
   if (key > lastVal)
      idx = nmemb - 1;
   else
     {
       if (key > firstVal)
       {
	 dt = (lastVal - firstVal)/(nmemb-1);
	 idx  = int(((key - firstVal)/dt)+0.5);
       }
     }
   
   return idx;
}


/* Function: lowerBound
 * Usage: lowerBound(val, vec, numVals);
 * -------------------------------------------------------------------------*/
/*! Finds the lowest index into the array "vec" which bounds the
 * value "val". This function does NOT check that the returned index is a 
 * valid index into the array.
 */
int lowerBound(double val, const double* vec, int numVals);


/* Function: unif
 * Usage: unif(mean, halfInterval);
 * -------------------------------------------------------------------------*/
/*! Generates a pseudorandom number from a uniform distribution with given 
 *  mean and halfInterval width.
 */
inline double unif(double mean, double halfInterval)
{
   //Generate uniform random number on the interval (0,1]
   double unifRand = (rand()+1)*(1.0/(RAND_MAX+1.0));

   //scale and shift random number
   unifRand = 2.0*halfInterval*unifRand + mean - halfInterval;

   return unifRand;
}


/* Function: unif_zeroMean
 * Usage: unif_zeroMean(halfInterval);
 * -------------------------------------------------------------------------*/
/*! Generates a pseudorandom number from a uniform distribution with zero 
 *  mean and given halfInterval width.
 */
inline double unif_zeroMean(const double& halfInterval)
{
   //Generate uniform random number on the interval (0,1]
   double unifRand = (rand()+1)*(1.0/(RAND_MAX+1.0));

   //scale and shift random number
   unifRand = 2.0*halfInterval*unifRand - halfInterval;

   return unifRand;
}


/* Function: unif_zeroOne
 * Usage: unif_zeroOne();
 * -------------------------------------------------------------------------*/
/*! Generates a pseudorandom number from a uniform distribution with zero 
 *  mean and halfInterval width of 1.
 */
inline double unif_zeroOne()
{
  return 2.0*(rand()+1)*(1.0/(RAND_MAX+1.0))-1.0;
}


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
unsigned int seed_randn(unsigned int *p_seed);


/* Function:  randn
 * Usage:  randn(mean, stddev);
 * -------------------------------------------------------------------------*/
/*! Uses a polar form Box-Muller Transform to generate a pseudorandom number
 *  from a gaussian distribution with specified mean and standard deviation.
 *  Generates two random numbers for each call and stores the unused number 
 *  for the next call to this function.
 */
double randn(double mean, double stddev);


/* Function:  randn_zeroMean
 * Usage:  randn_zeroMean(mean, stddev);
 * -------------------------------------------------------------------------*/
/*! Uses a polar form Box-Muller Transform to generate a pseudorandom number
 *  from a gaussian distribution with zero mean and specified standard
 *  deviation. Generates two random numbers for each call and stores the unused
 *  number for the next call to this function.
 */
inline double randn_zeroMean(const double& stddev)
{
   double rand1, rand2, gauss1, w;
   static int use_last = 0;
   static double gauss2;

   //If we already have a random variable waiting to be used, use it
   if (use_last)		
   {
      gauss1 = gauss2;
      use_last = 0;
   }
   //Otherwise, generate two new random numbers
   else
   {
      //Use the Polar Form of the Box-Muller transformation:
      //1.Generate two uniform random variables within the unit circle
      do {
         //Generate two uniform random variables between -1 and 1
         rand1 = unif_zeroOne();
         rand2 = unif_zeroOne();
         w = rand1*rand1 + rand2*rand2;
      } while ( w >= 1.0 );
      
      //2.Convert random variables to gaussian variables, N(0,1).
      w = sqrt(-2.0*log(w)/w);
      gauss1 = rand1 * w;
      gauss2 = rand2 * w;

      //Set flag that we have an extra variable that can be used
      use_last = 1;
   }
   
   return (stddev*gauss1);

}


/* Function: charCat
 * Usage: fileName = charCat(fileName, directory, file)
 * -------------------------------------------------------------------------*/
/*! This function is used to concatenate two character arrays (front and back)
 *  into a new character array, dest.  This differs from the C standard function
 *  strcat() in that the returned character array is an entirely new location 
 *  in memory, separate from the front and back arrays.  The length of the dest
 *  char array must be greater than or equal to the sum of the lengths of the
 *  front and back arrays.   
 */
char* charCat(char* dest, const char* front, const char* back);


/* Function: computeKLdiv_gaussian_mat
 * Usage: KL = computeKLdiv_gaussian_mat(xpts, ypts, P, mu, C)
 * --------------------------------------------------------------------------*/
/*! This function computes the KL divergence between a proposal distribution and
 * a Gaussian distribution with mean vector mu and covariance C.  The proposal
 * distribution is described by the matrix P, with corresponding (x,y) values
 * given in the vectors xpts and ypts.
 *
 */
double computeKLdiv_gaussian_mat(double* xpts, double* ypts, 
				 const Matrix &refPDF, 
                                 double* mu, const Matrix &Cov);

/* Function: computeMatrixSqrt
 * Usage: Asqrt = computeMatrixSqrt(A)
 * --------------------------------------------------------------------------*/
/*! This function computes the matrix square root of the given matrix, A.  The
 *  given matrix and the returned matrix are both symmetric matrices. The sqrt
 *  is computed by first performing the eigenvalue decomposition of the matrix
 *  A, square rooting the resulting eigenvalues, and reconstructing the matrix
 *  to get Asqrt.
 */
SymmetricMatrix computeMatrixSqrt(const SymmetricMatrix &A);


/* Function: computeArrayCrossProduct
 * Usage: computeArrayCrossProduct(array1, array2, result)
 * --------------------------------------------------------------------------*/
/*! This function computes the cross product: array1 x array2, and returns
 *  the result in result. The arrays are assumed to have 3 elements.
 */
inline void computeArrayCrossProd(double* a, double* b, double* result)
{
   //result = axb
   result[0] = a[1]*b[2] - a[2]*b[1];
   result[1] = a[2]*b[0] - a[0]*b[2];
   result[2] = a[0]*b[1] - a[1]*b[0];
}


#endif
