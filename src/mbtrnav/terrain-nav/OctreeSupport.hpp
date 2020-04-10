#ifndef OctreeSupport_H
#define OctreeSupport_H

#include <ostream>


/*
These structs are designed to be used by/with Octree and OctreeNode classes.
For documentation, go look at the top of Octree.hpp
*/

struct zGrid{
	zGrid(): zValues(NULL), numXValues(0), numYValues(0), xIndexFirst(false) {}
	zGrid(float* const z, unsigned int numX, unsigned int numY, bool xFirst): zValues(z), numXValues(numX), numYValues(numY), xIndexFirst(xFirst){}
	double getZ(unsigned int xIndex, unsigned int yIndex) const;
	void Print();
	
	float* zValues;
	unsigned int numXValues, numYValues;
	bool xIndexFirst;
};

struct Path {
	unsigned int x, y, z;
	Path(unsigned int X, unsigned int Y, unsigned int Z): x(X), y(Y), z(Z) {}
	Path(void): x(0), y(0), z(0) {}
	
	void Print(void) const;
};

/****************************************************************************/

struct Vector {
	double x, y, z;
	Vector(const Vector& V): x(V.x), y(V.y), z(V.z) {}
	Vector(const double X, const double Y, const double Z): x(X), y(Y), z(Z) {}
	Vector(void): x(0), y(0), z(0) {}
	void SetValues(const double X, const double Y, const double Z) {
		x = X;
		y = Y;
		z = Z;
	}
	Vector& operator*=(const double scaleFactor);
	Vector& operator/=(const double scaleFactor);
	
	double Norm() const;
	
	Vector& operator-=(const Vector& V);
	Vector& operator+=(const Vector& V);
	
	bool operator==(const Vector& V) const {return ((V.x == x) && (V.y == y) && (V.z == z));}
	double Dot(const Vector& V) const { return V.x * x + V.y * y + V.z * z;}
	
	bool StrictlyLessThan(const Vector& V)const {
		return ((x < V.x) && (y < V.y) && (z < V.z));
	}
	bool StrictlyGreaterThan(const Vector& V)const {
		return ((x > V.x) && (y > V.y) && (z > V.z));
	}
	bool StrictlyLessOrEqualTo(const Vector& V)const {
		return ((x <= V.x) && (y <= V.y) && (z <= V.z));
	}
	bool StrictlyGreaterOrEqualTo(const Vector& V)const {
		return ((x >= V.x) && (y >= V.y) && (z >= V.z));
	}
	
	void Print(void) const;
};

Vector operator+ (Vector U, const Vector& V);
Vector operator- (Vector U, const Vector& V);
Vector operator* (Vector U, double multiplyBy);
Vector operator/ (Vector U, double divideBy);
/****************************************************************************/
//for Octree.cpp
int Octree_PickMaxRatio(double& Xratio, const double Yratio, const double Zratio);
int Octree_PickMinPositiveRatio(const double Xratio, const double Yratio, const double Zratio);
//for OctreeNode.cpp
void OctreeNode_PrintTabs(int num);
unsigned int OctreeNode_CalculateWeights(double* weights, const Vector* const points, const unsigned int* const indices, 
								 unsigned int numPoints, const Vector& nodeLowerBounds, 
								 const Vector& nodeUpperBounds, const Vector& TrueResolution);




#endif
