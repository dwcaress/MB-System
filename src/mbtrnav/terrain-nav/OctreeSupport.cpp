#include "OctreeSupport.hpp"
#include <cmath>


#include <iostream>
/* Definitions of Vector functions:
For class and project design, look at the top of Octree.hpp
*/
double zGrid::getZ(unsigned int xIndex, unsigned int yIndex) const{
	if(xIndexFirst){//stored in NED
		return -static_cast<double>(zValues[xIndex * numYValues + yIndex]);
	}
	else{//stored in ENU
		return -static_cast<double>(zValues[yIndex * numXValues + xIndex]);
	}
}
void zGrid::Print(){
	std::cout << "X: " << numXValues << "\tY: " << numYValues << "\tXfirst: " << xIndexFirst << std::endl;
}


/***************************************************/
double Vector::Norm() const {
	return sqrt(x * x + y * y + z * z);
}
/***************************************************/
Vector& Vector::operator*=(const double scaleFactor) {
	x *= scaleFactor;
	y *= scaleFactor;
	z *= scaleFactor;
	return *this;
}
Vector& Vector::operator/=(const double scaleFactor) {
	x /= scaleFactor;
	y /= scaleFactor;
	z /= scaleFactor;
	return *this;
}
/***************************************************/
Vector& Vector::operator-=(const Vector& V) {
	x -= V.x;
	y -= V.y;
	z -= V.z;
	return *this;
}
/***************************************************/
Vector& Vector::operator+=(const Vector& V) {
	x += V.x;
	y += V.y;
	z += V.z;
	return *this;
}
/***************************************************/
void Vector::Print(void) const {
	std::cout << "X: " << x << "\tY: " << y << "\tZ: " << z << std::endl;
}
/***************************************************/
/***************************************************/
Vector operator+ (Vector U, const Vector& V) {
	U += V;
	return U;
}
Vector operator- (Vector U, const Vector& V) {
	U -= V;
	return U;
}
Vector operator* (Vector U, double multiplyBy) {
	U *= multiplyBy;
	return U;
}
Vector operator/ (Vector U, double divideBy) {
	U /= divideBy;
	return U;
}
/***************************************************/
void Path::Print(void) const {
	std::cout << "X: " << x << "\tY: " << y << "\tZ: " << z << std::endl;
}


/* local function performs specific task similar to Matlab's [value,index] = min(stuff)
where negative values have been replaced with NaN
*/
int Octree_PickMinPositiveRatio(const double Xratio, const double Yratio, const double Zratio) {
	// positive is to filter out the '-1' cases from having directionVector component == 0
	if(Xratio >= 0) {
		if((Xratio < Yratio) || (Yratio < 0)) {
			if((Xratio < Zratio) || (Zratio < 0)) {
				return 1;//X ratio is the one we want
			}
			return 3;//Z
		}
		if((Yratio < Zratio) || (Zratio < 0)) {
			return 2;//Y
		}
		return 3;//Z
	}//X ratio not valid
	if(Yratio >= 0) {
		if((Yratio < Zratio) || (Zratio < 0)) {
			return 2;//Y
		}
		return 3;//Z
	}//Y ratio not valid
	if(Zratio >= 0) {
		return 3;//Z
	}
	return -1;//error
}


// local function performs specific task similar to Matlab's [val,index] = max(stuff)
int Octree_PickMaxRatio(double& Xratio, const double Yratio, const double Zratio) {
	// returns the 'index' of the maximum value, and sets the first input to the corresponding value.
	if(Xratio < Yratio) {
		if(Yratio < Zratio)	{
			Xratio = Zratio;
			return 3;
		}
		Xratio = Yratio;
		return 2;
	}
	if(Xratio < Zratio) {
		Xratio = Zratio;
		return 3;
	}
	return 1;
}

// local functions:
void OctreeNode_PrintTabs(int num) {
	for(int ii = 0; ii < num; ii++) {
		std::cout << "  ";
	}
}

unsigned int OctreeNode_CalculateWeights(double* weights, const Vector* const points, const unsigned int* const indices,
								 unsigned int numPoints, const Vector& nodeLowerBounds, 
								 const Vector& nodeUpperBounds, const Vector& TrueResolution){
	unsigned int numNonZeroWeights = 0;
	for(unsigned int index = 0; index < numPoints; index++){
		Vector point = points[indices[index]];
		weights[index] = 1.0 - 1.0*sqrt(
			(pow((point.x - nodeLowerBounds.x) / TrueResolution.x, 2.0)*static_cast<double>(point.x < nodeLowerBounds.x))
			+ (pow((point.x - nodeUpperBounds.x) / TrueResolution.x, 2.0)*static_cast<double>(point.x > nodeUpperBounds.x))
			+ (pow((point.y - nodeLowerBounds.y) / TrueResolution.y, 2.0)*static_cast<double>(point.y < nodeLowerBounds.y))
			+ (pow((point.y - nodeUpperBounds.y) / TrueResolution.y, 2.0)*static_cast<double>(point.y > nodeUpperBounds.y))
			+ (pow((point.z - nodeLowerBounds.z) / TrueResolution.z, 2.0)*static_cast<double>(point.z < nodeLowerBounds.z))
			+ (pow((point.z - nodeUpperBounds.z) / TrueResolution.z, 2.0)*static_cast<double>(point.z > nodeUpperBounds.z)));
		
		if(weights[index] <= 0.0){
			weights[index] = 0.0;
		}else{
			numNonZeroWeights ++;
		}
		weights[index] = pow(weights[index], 2.0);
	}
	return numNonZeroWeights;
}







