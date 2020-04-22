#ifndef Octree_H_Inside_Header
#error Do not include Octree.tcc directly, instead include Octree.hpp
#endif

#include "OctreeSupport.hpp"

#include <stdio.h>

#include <fstream>
#include <iostream>
#include <cmath>

/* Octree Class
stores root of an octree and general properties for working with that Octree.
Also defines some functions.
*/

// --------------------------------------------------------------------------------------

/* Class Private Variables:
Octree private variables:
	Vector LowerBounds
	Vector UpperBounds
	Vector Size
	Vector TrueResolution

	int MaxDepth
	ValueType OffMapValue
	OctreeType::EnumOctreeType OctreeNodeType
		OctreeType::PointCount
		OctreeType::BinaryOccupancy
		OctreeType::Data

	OctreeNode* OctreeRoot

OctreeNode private variables:
	OctreeNode* children[8]
	ValueType value

ps: they are both friends with each other
*/

// Meausrement functions

/* Here's your ray tracing function:
This will trace from the startPoint to along the directionVector until it hits a non EmptyValue node.
If the ray misses all nonempty nodes, '-1' will be returned.
*/
template <class ValueType>
double
Octree<ValueType>::
RayTrace(const Vector& startPoint, const Vector& directionVector) const {
	/*
	All right, first we are going get to the octree (if we need to).  Then we will loop until
	we either hit the object or go out of the map.  On each loop, we will step from the entry
	point for the current node to the exit point for that node.  If the next node would be out
	of the map, '-1' is returned.  If the next node is nonempty, the loop finishes and the
	distance will be returned.
	*/
	Vector transitionPoint;
	Vector deltaToTransitionPoint;
	Vector deltaToCorner;
	
	double distance;
	OctreeNode* nodePointer;
	Path path;
	int depth;
	
	//get to the octree
	if(ContainsPoint(startPoint))	{
		//the boring case
		transitionPoint = startPoint;
		distance = 0.0;
	} else {
		//the fun case.  Go read RayTraceToThisOctree if you dare.
		distance = RayTraceToThisOctree(transitionPoint, startPoint, directionVector);
		if(-1.0 == distance) {
			//we missed entirely
			return distance;
		}
	}
	
	// set up for the start of the loop
	path = FindPathToPoint(transitionPoint);
	nodePointer = GetPointerToLeafOnPath(depth, path);
	
	// loop until termination criteria
	// currently set to: hitting a node with non-zero value
	while(nodePointer->value == EmptyValue) {
		/*Use the bounds, transitionPoint into this node, and directionVector to figure
		out which side of the box the ray will exit.  Based on that determine the
		distance traveled through this node and set up for the next loop.
		*/
		
		/*
		each block of the switch will update the transitionPoint
		for stepping to the edge the current node.  The relevant path
		dimension will be incremented or decremented (then tested for
		still being in the map) to move to the next node.
		*/
		switch(GetExitSide(deltaToCorner, transitionPoint, directionVector, path, depth)) {
			case 1://X
				deltaToTransitionPoint.SetValues(
					deltaToCorner.x,
					deltaToCorner.x * directionVector.y / directionVector.x,
					deltaToCorner.x * directionVector.z / directionVector.x);
				transitionPoint = transitionPoint + deltaToTransitionPoint;
				//the extra arguments to FindPathToPoint get the path to the leaf within this node closest to the input point
				path = FindPathToPointFromNode(transitionPoint, path, depth);
				/*Finding adjacent nodes using the path is as simple as incrementing
				or decrementing the relevant path component:
				directionVector > 0 is '0' or '1' which I want to turn into '-1' or '1'
				in order to add it to the path component and get the new path.
				note: '<< 1' is equivalent to '*2' */
				path.x += ((directionVector.x > 0) << 1) - 1;
				if(! PathElementIsValid(path.x)) {
					//we are traveling out of the octree
					return -1.0;
				}
				break;
			case 2://Y
				deltaToTransitionPoint.SetValues(
					deltaToCorner.y * directionVector.x / directionVector.y,
					deltaToCorner.y,
					deltaToCorner.y * directionVector.z / directionVector.y);
				transitionPoint = transitionPoint + deltaToTransitionPoint;
				path = FindPathToPointFromNode(transitionPoint, path, depth);
				path.y += ((directionVector.y > 0) << 1) - 1;
				if(! PathElementIsValid(path.y)) {
					//we are traveling out of the octree
					return -1.0;
				}
				break;
			case 3://Z
				deltaToTransitionPoint.SetValues(
					deltaToCorner.z * directionVector.x / directionVector.z,
					deltaToCorner.z * directionVector.y / directionVector.z,
					deltaToCorner.z);
				transitionPoint = transitionPoint + deltaToTransitionPoint;
				path = FindPathToPointFromNode(transitionPoint, path, depth);
				path.z += ((directionVector.z > 0) << 1) - 1;
				if(! PathElementIsValid(path.z)) {
					//we are traveling out of the octree
					return -1.0;
				}
				break;
		}
		
		// update the distance from moving through this node
		distance += deltaToTransitionPoint.Norm();
		
		// update the node for the next iteration
		nodePointer = GetPointerToLeafOnPath(depth, path);
	}
	//if we got here, distance is the return value we want
	return distance;
}

/* Query Function:
This is intended for maps where the probability of getting a sonar reading at given point is 
stored in the map rather than the underlying surface.  With this map style, the combination of 
calling RayTrace and applying a sensor model is replaced with a call to Query or InterpolatingQuery.

This will return the value stored in the leaf containing the queryPoint.  If the
leaf is outside the Octree, the OffMapValue is returned.
*/
template <class ValueType>
ValueType
Octree<ValueType>::
Query(const Vector& queryPoint) const {
	if(ContainsPoint(queryPoint)) {
		return GetPointerToLeafOnPath(FindPathToPoint(queryPoint))->value;
	}
	return OffMapValue;
}

/* Interpolating Query (the second way of doing it):
Interpolates between the 8 adjacent nodes around the query point to get the value at that point.
If the query Point is off the map, OffMapValue is returned.  If the queryPoint is within half of
true resolution of an edge, the off map value is substituted for the nodes which lie off the map.
*/
template <class ValueType>
double
Octree<ValueType>::
InterpolatingQuery(const Vector& queryPoint) const {
	if(!ContainsPoint(queryPoint)) {
		return static_cast<double>(OffMapValue);
	}
	/*
	To do linear interpolation, you need the values to interpolate from, and the
	contribution to the total made by each value.  To do this efficiently in this
	octree structure the two of those tasks need to be slightly intertwined.
	
	Good luck.
	*/
	
	Vector percentageTowardThisNodeCenter;
	double interpolationConstant[8];
	double interpolatedValue;
	double queriedValues[8];
	Path path;
	signed int adjacentPathDirection[3];
	
	//get an extra bit of precision on the path in order to find which corner of the current box we are in
	path.x = static_cast<unsigned int>(2.0 * (queryPoint.x - LowerBounds.x) / TrueResolution.x);
	path.y = static_cast<unsigned int>(2.0 * (queryPoint.y - LowerBounds.y) / TrueResolution.y);
	path.z = static_cast<unsigned int>(2.0 * (queryPoint.z - LowerBounds.z) / TrueResolution.z);
	
	//convert LSB of path.* into '-1' or '1' for finding adjacent nodes
	adjacentPathDirection[0] = ((path.x & 1) << 1) - 1;//X
	adjacentPathDirection[1] = ((path.y & 1) << 1) - 1;//Y
	adjacentPathDirection[2] = ((path.z & 1) << 1) - 1;//Z
	
	//then throw the LSB away
	path.x >>= 1;
	path.y >>= 1;
	path.z >>= 1;
	
	//you can trust me on this one
	percentageTowardThisNodeCenter.SetValues(
		1 - std::abs(((queryPoint.x - LowerBounds.x) / TrueResolution.x) - path.x - 0.5),
		1 - std::abs(((queryPoint.y - LowerBounds.y) / TrueResolution.y) - path.y - 0.5),
		1 - std::abs(((queryPoint.z - LowerBounds.z) / TrueResolution.z) - path.z - 0.5));
		
	//weights for each of the interpolated nodes
	interpolationConstant[0] = percentageTowardThisNodeCenter.x *
							   percentageTowardThisNodeCenter.y *
							   percentageTowardThisNodeCenter.z;
	interpolationConstant[1] = percentageTowardThisNodeCenter.x *
							   percentageTowardThisNodeCenter.y *
							   (1 - percentageTowardThisNodeCenter.z);
	interpolationConstant[2] = percentageTowardThisNodeCenter.x *
							   (1 - percentageTowardThisNodeCenter.y) *
							   percentageTowardThisNodeCenter.z;
	interpolationConstant[3] = percentageTowardThisNodeCenter.x *
							   (1 - percentageTowardThisNodeCenter.y) *
							   (1 - percentageTowardThisNodeCenter.z);
	interpolationConstant[4] = (1 - percentageTowardThisNodeCenter.x) *
							   percentageTowardThisNodeCenter.y *
							   percentageTowardThisNodeCenter.z;
	interpolationConstant[5] = (1 - percentageTowardThisNodeCenter.x) *
							   percentageTowardThisNodeCenter.y *
							   (1 - percentageTowardThisNodeCenter.z);
	interpolationConstant[6] = (1 - percentageTowardThisNodeCenter.x) *
							   (1 - percentageTowardThisNodeCenter.y) *
							   percentageTowardThisNodeCenter.z;
	interpolationConstant[7] = (1 - percentageTowardThisNodeCenter.x) *
							   (1 - percentageTowardThisNodeCenter.y) *
							   (1 - percentageTowardThisNodeCenter.z);
							   
							   
	//already have the path for the first leaf
	queriedValues[0] = static_cast<double>(GetPointerToLeafOnPath(path)->value);
	/*
	The next hundred lines of three layer nested if/else will find all the nodes which are
	inside the map and get their values.  Also, all nodes not on the map will have a value
	of OffMapValue for the purposes of interpolation.
	
	Note: only one of the third layer of if/else will run, and queriedValues[###] will only be set
	once for each index (0-7).
	*/
	//test in X direction
	if(PathElementIsValid(path.x + adjacentPathDirection[0])) {
		queriedValues[4] = static_cast<double>(GetPointerToLeafOnPath(
				path.x + adjacentPathDirection[0],
				path.y,
				path.z)->value);
				
		//test in Y
		if(PathElementIsValid(path.y + adjacentPathDirection[1])) {
			queriedValues[2] = static_cast<double>(GetPointerToLeafOnPath(
					path.x,
					path.y + adjacentPathDirection[1],
					path.z)->value);
			queriedValues[6] = static_cast<double>(GetPointerToLeafOnPath(
					path.x + adjacentPathDirection[0],
					path.y + adjacentPathDirection[1],
					path.z)->value);
					
			//test in Z
			if(PathElementIsValid(path.z + adjacentPathDirection[2])) {
				//all three directions good
				queriedValues[1] = static_cast<double>(GetPointerToLeafOnPath(
						path.x,
						path.y,
						path.z + adjacentPathDirection[2])->value);
				queriedValues[3] = static_cast<double>(GetPointerToLeafOnPath(
						path.x,
						path.y + adjacentPathDirection[1],
						path.z + adjacentPathDirection[2])->value);
				queriedValues[5] = static_cast<double>(GetPointerToLeafOnPath(
						path.x + adjacentPathDirection[0],
						path.y,
						path.z + adjacentPathDirection[2])->value);
				queriedValues[7] = static_cast<double>(GetPointerToLeafOnPath(
						path.x + adjacentPathDirection[0],
						path.y + adjacentPathDirection[1],
						path.z + adjacentPathDirection[2])->value);
			} else {
				//X and Y only
				queriedValues[1] = static_cast<double>(OffMapValue);
				queriedValues[3] = static_cast<double>(OffMapValue);
				queriedValues[5] = static_cast<double>(OffMapValue);
				queriedValues[7] = static_cast<double>(OffMapValue);
			}
		} else {
			//No Y
			queriedValues[2] = static_cast<double>(OffMapValue);
			queriedValues[3] = static_cast<double>(OffMapValue);
			queriedValues[6] = static_cast<double>(OffMapValue);
			queriedValues[7] = static_cast<double>(OffMapValue);
			
			//test Z
			if(PathElementIsValid(path.z + adjacentPathDirection[2])) {
				//X and Z only
				queriedValues[1] = static_cast<double>(GetPointerToLeafOnPath(
						path.x,
						path.y,
						path.z + adjacentPathDirection[2])->value);
				queriedValues[5] = static_cast<double>(GetPointerToLeafOnPath(
						path.x + adjacentPathDirection[0],
						path.y,
						path.z + adjacentPathDirection[2])->value);
			} else {
				//X only
				queriedValues[1] = static_cast<double>(OffMapValue);
				queriedValues[5] = static_cast<double>(OffMapValue);
			}
		}
	} else {
		//no X
		queriedValues[4] = static_cast<double>(OffMapValue);
		queriedValues[5] = static_cast<double>(OffMapValue);
		queriedValues[6] = static_cast<double>(OffMapValue);
		queriedValues[7] = static_cast<double>(OffMapValue);
		
		//test Y
		if(PathElementIsValid(path.y + adjacentPathDirection[1])) {
			queriedValues[2] = static_cast<double>(GetPointerToLeafOnPath(
					path.x,
					path.y + adjacentPathDirection[1],
					path.z)->value);
					
			//test Z
			if(PathElementIsValid(path.z + adjacentPathDirection[2])) {
				//Y and Z only
				queriedValues[1] = static_cast<double>(GetPointerToLeafOnPath(
						path.x,
						path.y,
						path.z + adjacentPathDirection[2])->value);
				queriedValues[3] = static_cast<double>(GetPointerToLeafOnPath(
						path.x,
						path.y + adjacentPathDirection[1],
						path.z + adjacentPathDirection[2])->value);
			} else {
				//Y only
				queriedValues[1] = static_cast<double>(OffMapValue);
				queriedValues[3] = static_cast<double>(OffMapValue);
			}
		} else {
			//no Y
			queriedValues[2] = static_cast<double>(OffMapValue);
			queriedValues[3] = static_cast<double>(OffMapValue);
			
			//test Z
			if(PathElementIsValid(path.z + adjacentPathDirection[2])) {
				//Z only
				queriedValues[1] = static_cast<double>(GetPointerToLeafOnPath(
						path.x,
						path.y,
						path.z + adjacentPathDirection[2])->value);
			} else {
				queriedValues[1] = static_cast<double>(OffMapValue);
			}
		}
	}//done with getting queriedValues
	
	interpolatedValue = 0;
	int index;
	for(index = 0; index < 8; index++) {
		interpolatedValue += interpolationConstant[index] * queriedValues[index];
	}
	return interpolatedValue;
}

// Constructors and such
//enpty constructor
template <class ValueType>
Octree<ValueType>::
Octree() {
	OffMapValue = static_cast<ValueType>(0);
	EmptyValue = static_cast<ValueType>(0);
	Size = Vector();
	UpperBounds = Vector();
	LowerBounds = Vector();
	TrueResolution = Vector();
	MaxDepth = 0;
	OctreeNodeType = OctreeType::PointCount;
	
	OctreeRoot = new OctreeNode;
}

//copy constructor
template <class ValueType>
Octree<ValueType>::
Octree(const Octree<ValueType>& octreeToCopy) {
	LowerBounds = Vector(octreeToCopy.LowerBounds);
	UpperBounds = Vector(octreeToCopy.UpperBounds);
	Size = Vector(octreeToCopy.Size);
	TrueResolution = Vector(octreeToCopy.TrueResolution);
	
	MaxDepth = int(octreeToCopy.MaxDepth);
	OffMapValue = ValueType(octreeToCopy.OffMapValue);
	EmptyValue = ValueType(octreeToCopy.EmptyValue);
	OctreeNodeType = OctreeType::EnumOctreeType(octreeToCopy.OctreeNodeType);
	
	OctreeRoot = new OctreeNode(*(octreeToCopy.OctreeRoot));
}

/* Constructor you want to use:
This will set MaxDepth so that we split the points down to TrueResolution <= desiredResolution.  It
also sets TrueResolution deals with the rest of the Octree properties:  EmptyValue and OffMapValue 
are set to zero.  Size is set to UpperBounds - LowerBounds.
*/
template <class ValueType>
Octree<ValueType>::
Octree(const Vector& desiredResolution, const Vector& lowerBounds, const Vector& upperBounds,
	   const OctreeType::EnumOctreeType octreeType) {
	   
	//all we are doing here is setting parameters for the tree as a whole, then calling the correct OctreeNode<type> constructor
	OffMapValue = static_cast<ValueType>(0);
	EmptyValue = static_cast<ValueType>(0);
	
	UpperBounds = Vector(upperBounds);
	LowerBounds = Vector(lowerBounds);
	
	Size = UpperBounds - LowerBounds;
	TrueResolution = Vector(Size);
	
	//set the true resolution and max depth
	MaxDepth = 0;
	while(! TrueResolution.StrictlyLessOrEqualTo(desiredResolution)) {
		TrueResolution.MultiplyBy(0.5);
		MaxDepth++;
	}
	OctreeNodeType = octreeType;
	
	OctreeRoot = new OctreeNode(EmptyValue);
}

/* copy assignment operator
defined because of the dynamic allocation of OctreeRoot
*/
template <class ValueType>
Octree<ValueType>&
Octree<ValueType>::
operator= (Octree<ValueType> rightHandSide) {
	//copy and swap
	this->Swap(rightHandSide);
	return *this;
}

/* Destructor
defined becauseOctreeRoot is dynamically allocated
*/
template <class ValueType>
Octree<ValueType>::
~Octree() {
	delete OctreeRoot;
}

/*! Adding points:
Adding points will switch based on the OctreeType for determining what is done
as a result of a point being added.  See OctreeNode.tcc for the various AddPoint methods
or Octree.hpp for a description of the Octree Types.  The return indicates the number of points added.

If the point is outside the bounds of the Octree, it expands to include the new point.  This is not 
recommended as it can result in unnecessarily large Octrees.
*/
// One point
template <class ValueType>
bool
Octree<ValueType>::
AddPoint(const Vector& point) {
	switch(OctreeNodeType) {
		case OctreeType::PointCount: {
			if(!ContainsPoint(point)) {
				ExpandOctreeToIncludePoint(point);
			}
			OctreeRoot->AddPointPointCount(*this, point, 0);
			return true;
		}
		break;
		case OctreeType::BinaryOccupancy: {
			if(!ContainsPoint(point)) {
				ExpandOctreeToIncludePoint(point);
			}
			OctreeRoot->AddPointBinaryOccupancy(*this, point, 0);
			return true;
		}
		break;
		default: {
			std::cout << "\nData Type Octrees require the AddData Method\nNo points added\n\n";
		}
	}
	return false;
}

// Many points
template <class ValueType>
int
Octree<ValueType>::
AddPoints(const Vector points[], const int numPoints) {
	int index = 0;
	switch(OctreeNodeType) {
		case OctreeType::PointCount: {
			for(index = 0; index < numPoints; index++) {
				if(!ContainsPoint(points[index])) {
					ExpandOctreeToIncludePoint(points[index]);
				}
				OctreeRoot->AddPointPointCount(*this, points[index], 0);
			}
		}
		break;
		case OctreeType::BinaryOccupancy: {
			for(index = 0; index < numPoints; index++) {
				if(!ContainsPoint(points[index])) {
					ExpandOctreeToIncludePoint(points[index]);
				}
				OctreeRoot->AddPointBinaryOccupancy(*this, points[index], 0);
			}
		}
		break;
		default: {
			std::cout << "\nData Type Octrees require the AddData Method\nNo points added\n\n";
		}
	}
	return index;
}

// One data
template <class ValueType>
bool
Octree<ValueType>::
AddData(const Vector point, const ValueType data) {
	if(OctreeNodeType == OctreeType::Data) {
		if(!ContainsPoint(point)) {
			ExpandOctreeToIncludePoint(point);
		}
		OctreeRoot->AddData(*this, point, data, 0);
		return true;
	} else {
		std::cout << "\nAddData does not apply to OctreeNode Types other than Data\nNo points added\n\n";
	}
	return false;
}

// Multiple data
template <class ValueType>
int
Octree<ValueType>::
AddData(const Vector points[], const ValueType data[], const int numDatas) {
	if(OctreeNodeType == OctreeType::Data) {
		int index;
		for(index = 0; index < numDatas; index ++) {
			if(!ContainsPoint(points[index])) {
				ExpandOctreeToIncludePoint(points[index]);
			}
			OctreeRoot->AddData(*this, points[index], data[index], 0);
		}
		return index;
	} else {
		std::cout << "\nAddData does not apply to OctreeNode Types other than Data\nNo points added\n\n";
		return 0;
	}
}

// Memory reduction
/* Collapse
This runs through the tree and collapses voxels which are uniform.  
*/
template <class ValueType>
void
Octree<ValueType>::
Collapse(void) {
	OctreeRoot->Collapse();
}

// Save, Load, and Print
/* Save function:
Writes directly to a binary file.  
*/
template <class ValueType>
bool
Octree<ValueType>::
SaveToFile(const char* filename) const {
	/* Open a binary file for writing.  Save order is all the
	Octree Properties, then the tree in depth first order.
	*/
	std::FILE* saveFile;
	saveFile = std::fopen(filename , "wb");
	if(saveFile == NULL) {
		std::cout << "Unable to open: " << filename << std::endl;
		return false;
	}
	
	//LowerBounds
	std::fwrite(&LowerBounds.x, sizeof(LowerBounds.x), 1, saveFile);
	std::fwrite(&LowerBounds.y, sizeof(LowerBounds.y), 1, saveFile);
	std::fwrite(&LowerBounds.z, sizeof(LowerBounds.z), 1, saveFile);
	
	//UpperBounds
	std::fwrite(&UpperBounds.x, sizeof(UpperBounds.x), 1, saveFile);
	std::fwrite(&UpperBounds.y, sizeof(UpperBounds.y), 1, saveFile);
	std::fwrite(&UpperBounds.z, sizeof(UpperBounds.z), 1, saveFile);
	
	//Size
	std::fwrite(&Size.x, sizeof(Size.x), 1, saveFile);
	std::fwrite(&Size.y, sizeof(Size.y), 1, saveFile);
	std::fwrite(&Size.z, sizeof(Size.z), 1, saveFile);
	
	//True Resolution
	std::fwrite(&TrueResolution.x, sizeof(TrueResolution.x), 1, saveFile);
	std::fwrite(&TrueResolution.y, sizeof(TrueResolution.y), 1, saveFile);
	std::fwrite(&TrueResolution.z, sizeof(TrueResolution.z), 1, saveFile);
	
	//non-vector quantities
	std::fwrite(&MaxDepth, sizeof(MaxDepth), 1, saveFile);
	std::fwrite(&OffMapValue, sizeof(OffMapValue), 1, saveFile);
	std::fwrite(&EmptyValue, sizeof(EmptyValue), 1, saveFile);
	std::fwrite(&OctreeNodeType, sizeof(OctreeNodeType), 1, saveFile);
	
	//the tree itself
	OctreeRoot->SaveToFile(saveFile);
	
	if(ferror(saveFile)) {
		fclose(saveFile);
		return false;
	}
	std::fclose(saveFile);
	return true;
}

/* Load function:
Load binary files created with SaveToFile function.
*/
template <class ValueType>
bool
Octree<ValueType>::
LoadFromFile(const char* filename) {
	// matched to Save
	std::FILE* loadFile;
	loadFile = std::fopen(filename , "rb");
	if(loadFile == NULL) {
		std::cout << "Unable to open: " << filename << std::endl;
		return false;
	}
	
	//LowerBounds
	std::fread(&LowerBounds.x, sizeof(LowerBounds.x), 1, loadFile);
	std::fread(&LowerBounds.y, sizeof(LowerBounds.y), 1, loadFile);
	std::fread(&LowerBounds.z, sizeof(LowerBounds.z), 1, loadFile);
	
	//UpperBounds
	std::fread(&UpperBounds.x, sizeof(UpperBounds.x), 1, loadFile);
	std::fread(&UpperBounds.y, sizeof(UpperBounds.y), 1, loadFile);
	std::fread(&UpperBounds.z, sizeof(UpperBounds.z), 1, loadFile);
	
	//Size
	std::fread(&Size.x, sizeof(Size.x), 1, loadFile);
	std::fread(&Size.y, sizeof(Size.y), 1, loadFile);
	std::fread(&Size.z, sizeof(Size.z), 1, loadFile);
	
	//True Resolution
	std::fread(&TrueResolution.x, sizeof(TrueResolution.x), 1, loadFile);
	std::fread(&TrueResolution.y, sizeof(TrueResolution.y), 1, loadFile);
	std::fread(&TrueResolution.z, sizeof(TrueResolution.z), 1, loadFile);
	
	//MaxDepth
	std::fread(&MaxDepth, sizeof(MaxDepth), 1, loadFile);
	//OffMapValue
	std::fread(&OffMapValue, sizeof(ValueType), 1, loadFile);
	//EmptyValue
	std::fread(&EmptyValue, sizeof(ValueType), 1, loadFile);
	//OctreeType
	std::fread(&OctreeNodeType, sizeof(OctreeType::EnumOctreeType), 1, loadFile);
	
	//OctreeRoot
	bool returnValue = true;
	delete OctreeRoot;
	OctreeRoot = new OctreeNode;
	returnValue &= OctreeRoot->LoadFromFile(loadFile);
	
	if(std::ferror(loadFile)) {
		std::fclose(loadFile);
		return false;
	}
	std::fclose(loadFile);
	return returnValue;
}

// print
template <class ValueType>
void
Octree<ValueType>::
Print(void) const {
	std::cout << "LowerBounds:\t";
	LowerBounds.Print();
	std::cout << "UpperBounds:\t";
	UpperBounds.Print();
	std::cout << "MaxDepth:\t" << MaxDepth << std::endl;
	std::cout << "Size:\t\t";
	Size.Print();
	std::cout << "TrueResolution:\t";
	TrueResolution.Print();
	std::cout << "OctreeType:\t" << OctreeNodeType << std::endl;
	
	//big octrees have LOTS to print
	//OctreeRoot->Print(0);
	
	std::cout << std::endl;
}

// Now for some private functions: first paths and bounds stuff

/* Path finding function:
If the point is outside the octree, it finds the path to the node closest to the desired point.
*/
template <class ValueType>
Path
Octree<ValueType>::
FindPathToPoint(const Vector& desiredPoint) const {
	//'1' represents going to the upper half along that axis
	Path path;
	//X
	if(desiredPoint.x <= LowerBounds.x) {
		path.x = 0;
	} else if(desiredPoint.x >= UpperBounds.x) {
		path.x = (1 << MaxDepth) - 1;
	} else {
		path.x = static_cast<unsigned int>((desiredPoint.x - LowerBounds.x) / TrueResolution.x);
	}
	//Y
	if(desiredPoint.y <= LowerBounds.y) {
		path.y = 0;
	} else if(desiredPoint.y >= UpperBounds.y) {
		path.y = (1 << MaxDepth) - 1;
	} else {
		path.y = static_cast<unsigned int>((desiredPoint.y - LowerBounds.y) / TrueResolution.y);
	}
	//Z
	if(desiredPoint.z <= LowerBounds.z) {
		path.z = 0;
	} else if(desiredPoint.z >= UpperBounds.z) {
		path.z = (1 << MaxDepth) - 1 ;
	} else {
		path.z = static_cast<unsigned int>((desiredPoint.z - LowerBounds.z) / TrueResolution.z);
	}
	return path;
}

/* Path and depth specify a node.  This function returns the path to
the leaf within the specified node such that the returned node is closest to
the input point.
*/
template <class ValueType>
Path
Octree<ValueType>::
FindPathToPointFromNode(const Vector& desiredPoint, const Path& path, const int depth) const {
	/* this one will return the path to the closest node to the input point from
	within the node on path	at depth
	*/
	Path tempPath = FindPathToPoint(desiredPoint);
	unsigned int lowerPathBitsHI = ((unsigned int)(1 << (MaxDepth - depth)) - 1);
	//X
	if(tempPath.x < (path.x & !lowerPathBitsHI)) {
		tempPath.x = path.x & !lowerPathBitsHI;
	} else if(tempPath.x > (path.x | lowerPathBitsHI)) {
		tempPath.x = path.x | lowerPathBitsHI;
	}
	//Y
	if(tempPath.y < (path.y & !lowerPathBitsHI)) {
		tempPath.y = path.y & !lowerPathBitsHI;
	} else if(tempPath.y > (path.y | lowerPathBitsHI)) {
		tempPath.y = path.y | lowerPathBitsHI;
	}
	//Z
	if(tempPath.z < (path.z & !lowerPathBitsHI)) {
		tempPath.z = path.z & !lowerPathBitsHI;
	} else if(tempPath.z > (path.z | lowerPathBitsHI)) {
		tempPath.z = path.z | lowerPathBitsHI;
	}
	return tempPath;
}

// For checking path elements:
template <class ValueType>
inline bool
Octree<ValueType>::
PathElementIsValid(const unsigned int pathElement) const {
	return (pathElement < (1U << MaxDepth));
	//pathElement is unsigned, so decrementing from zero will result in a value of at least '1 << MaxDepth'
}

/* Bounds Function:
Set the input vectors to the correct values for the node on path at depth.
*/
template <class ValueType>
void
Octree<ValueType>::
CalculateBoundsFromPath(Vector& nodeLowerBounds, Vector& nodeUpperBounds, const Path& path, const int depth) const {
	//calculate the bounds of the node relitave to the octree
	Vector multiplier = Size;
	multiplier.MultiplyBy(1.0 / static_cast<double>(1 << depth));
	nodeLowerBounds.SetValues(
		(static_cast<double>(path.x >>(MaxDepth - depth))) * multiplier.x,
		(static_cast<double>(path.y >>(MaxDepth - depth))) * multiplier.y,
		(static_cast<double>(path.z >>(MaxDepth - depth))) * multiplier.z);
	nodeUpperBounds.SetValues(
		(static_cast<double>((path.x >>(MaxDepth - depth)) + 1)) * multiplier.x,
		(static_cast<double>((path.y >>(MaxDepth - depth)) + 1)) * multiplier.y,
		(static_cast<double>((path.z >>(MaxDepth - depth)) + 1)) * multiplier.z);
		
	//add the LowerBounds of the Octree to the the true bounds of the node
	nodeUpperBounds = nodeUpperBounds + LowerBounds;
	nodeLowerBounds = nodeLowerBounds + LowerBounds;
}

/* ContainsPoint for the whole Octree:
Octrees contain their lower edges, but not their upper edges
*/
template <class ValueType>
bool
Octree<ValueType>::
ContainsPoint(const Vector& point) const {
	return point.StrictlyLessThan(UpperBounds) && point.StrictlyGreaterOrEqualTo(LowerBounds);
}

/*! Accessing nodes by their path: (all four versions)
Returns a pointer to the leaf located along the input path.  Can also set
the depth of that node through a reference input.
*/
// GetPointerToLeafOnPath
template <class ValueType>
typename Octree<ValueType>::OctreeNode*
Octree<ValueType>::
GetPointerToLeafOnPath(const Path& path) const {
	int depthIterator;
	OctreeNode* nodePointer = OctreeRoot;
	unsigned int bitmask = 1 << (MaxDepth - 1);
	int childNumber;
	/* Each loop will test to see if there are children.  If there are no children,
	the current node is the one we want.  If there are, children, the path is
	followed to the next layer.
	*/
	for(depthIterator = 0; depthIterator < MaxDepth; depthIterator ++) {
		if(NULL == nodePointer->children) {
			return nodePointer;
		}
		childNumber =
			((0 != (path.x & bitmask)) << 2)
			| ((0 != (path.y & bitmask)) << 1)
			| (0 != (path.z & bitmask));
		bitmask >>= 1;
		nodePointer = nodePointer->children[childNumber];
	}
	return nodePointer;
}

// GetPointerToLeafOnPath
template <class ValueType>
typename Octree<ValueType>::OctreeNode*
Octree<ValueType>::
GetPointerToLeafOnPath(const unsigned int Xpath, const unsigned int Ypath, const unsigned int Zpath) const {
	int depthIterator;
	OctreeNode* nodePointer = OctreeRoot;
	unsigned int bitmask = 1 << (MaxDepth - 1);
	int childNumber;
	/* Each loop will test to see if there are children.  If there are no children,
	the current node is the one we want.  If there are, children, the path is
	followed to the next layer.
	*/
	for(depthIterator = 0; depthIterator < MaxDepth; depthIterator ++) {
		if(NULL == nodePointer->children) {
			return nodePointer;
		}
		childNumber =
			((0 != (Xpath & bitmask)) << 2)
			| ((0 != (Ypath & bitmask)) << 1)
			| (0 != (Zpath & bitmask));
		bitmask >>= 1;
		nodePointer = nodePointer->children[childNumber];
	}
	return nodePointer;
}

/* Add depth as a return (by reference through input):
*/
// GetPointerToLeafOnPath
template <class ValueType>
typename Octree<ValueType>::OctreeNode*
Octree<ValueType>::
GetPointerToLeafOnPath(int& depth, const Path& path) const {
	OctreeNode* nodePointer = OctreeRoot;
	unsigned int bitmask = 1 << (MaxDepth - 1);
	int childNumber;
	/* Each loop will test to see if there are children.  If there are no children,
	the current node is the one we want.  If there are, children, the path is
	followed to the next layer.
	*/
	for(depth = 0; depth < MaxDepth; depth ++) {
		if(NULL == nodePointer->children) {
			return nodePointer;
		}
		childNumber =
			((0 != (path.x & bitmask)) << 2)
			| ((0 != (path.y & bitmask)) << 1)
			| (0 != (path.z & bitmask));
		bitmask >>= 1;
		nodePointer = nodePointer->children[childNumber];
	}
	return nodePointer;
}

// GetPointerToLeafOnPath
template <class ValueType>
typename Octree<ValueType>::OctreeNode*
Octree<ValueType>::
GetPointerToLeafOnPath(int& depth, const unsigned int Xpath, const unsigned int Ypath, const unsigned int Zpath) const {
	OctreeNode* nodePointer = OctreeRoot;
	unsigned int bitmask = 1 << (MaxDepth - 1);
	int childNumber;
	/* Each loop will test to see if there are children.  If there are no children,
	the current node is the one we want.  If there are, children, the path is
	followed to the next layer.
	*/
	for(depth = 0; depth < MaxDepth; depth ++) {
		if(NULL == nodePointer->children) {
			return nodePointer;
		}
		childNumber =
			((0 != (Xpath & bitmask)) << 2)
			| ((0 != (Ypath & bitmask)) << 1)
			| (0 != (Zpath & bitmask));
		bitmask >>= 1;
		nodePointer = nodePointer->children[childNumber];
	}
	return nodePointer;
}

/* RayTrace to this Octree
*/
template <class ValueType>
double
Octree<ValueType>::
RayTraceToThisOctree(Vector& transitionPoint, const Vector& startPoint, const Vector& directionVector) const {
	/* transitionPoint is passed in to be set by this function.  First we are going to
	figure out which side we must enter based on the startPoint and directionVector.
	Then we will test the transitionPoint to make sure it is within the bounds of
	the octree on the other two dimesnions.
	*/
	Vector deltaToEntryPoint;
	Vector deltaToCorner;
	Vector relevantCorner;
	int entranceSide;
	double Xratio, Yratio, Zratio;
	/*
	function works similar to GetExitSide except we are going on the
	outside of the box not the inside.  Consequently, the opposite corner
	is the one which allows us to determine which side (if any) we pass
	through.
	*/
	SetRelevantExternalCorner(relevantCorner, directionVector);
	deltaToCorner = relevantCorner - startPoint;
	(directionVector.x == 0) ? (Xratio = -1.0) : (Xratio = deltaToCorner.x / directionVector.x);
	(directionVector.y == 0) ? (Yratio = -1.0) : (Yratio = deltaToCorner.y / directionVector.y);
	(directionVector.z == 0) ? (Zratio = -1.0) : (Zratio = deltaToCorner.z / directionVector.z);
	
	entranceSide = Octree_PickMaxRatio(Xratio, Yratio, Zratio);
	/* Ratios have served their purpose, so X ratio is used as to return the maximum value
	If that value is negative, the startPoint is past relevantCorner (projected onto directionVector).
	*/
	if(Xratio < 0.0) {
		//we missed completely
		return -1.0;
	}
	
	// we still need to check the transitionPoint and make sure it is within
	// bounds.
	switch(entranceSide) {
		case 1://X
			deltaToEntryPoint.SetValues(
				deltaToCorner.x,
				deltaToCorner.x * directionVector.y / directionVector.x,
				deltaToCorner.x * directionVector.z / directionVector.x);
			transitionPoint = startPoint + deltaToEntryPoint;
			// test transitionPoint
			if((transitionPoint.y < LowerBounds.y)
					|| (transitionPoint.y > UpperBounds.y)
					|| (transitionPoint.z < LowerBounds.z)
					|| (transitionPoint.z > UpperBounds.z)) {
				return -1.0;
			}
			break;
		case 2://Y
			deltaToEntryPoint.SetValues(
				deltaToCorner.y * directionVector.x / directionVector.y,
				deltaToCorner.y,
				deltaToCorner.y * directionVector.z / directionVector.y);
			transitionPoint = startPoint + deltaToEntryPoint;
			// test transitionPoint
			if((transitionPoint.x < LowerBounds.x)
					|| (transitionPoint.x > UpperBounds.x)
					|| (transitionPoint.z < LowerBounds.z)
					|| (transitionPoint.z > UpperBounds.z)) {
				return -1.0;
			}
			break;
		case 3://Z
			deltaToEntryPoint.SetValues(
				deltaToCorner.z * directionVector.x / directionVector.z,
				deltaToCorner.z * directionVector.y / directionVector.z,
				deltaToCorner.z);
			transitionPoint = startPoint + deltaToEntryPoint;
			// test transitionPoint
			if((transitionPoint.x < LowerBounds.x)
					|| (transitionPoint.x > UpperBounds.x)
					|| (transitionPoint.y < LowerBounds.y)
					|| (transitionPoint.y > UpperBounds.y)) {
				return -1.0;
			}
			break;
	}
	
	//we hit the octree
	return deltaToEntryPoint.Norm();
}

// helper for RayTraceToThisOctree
template <class ValueType>
void
Octree<ValueType>::
SetRelevantExternalCorner(Vector& relevantCorner, const Vector& directionVector) const {
	/* Sets the corner which separates the three sides we might pass through.  This is
	determined only be the directionVector.
	*/
	switch(((directionVector.x >= 0.0) << 2)
			| ((directionVector.y >= 0.0) << 1)
			| (directionVector.z >= 0.0)) {
		case 0:
			relevantCorner.SetValues(UpperBounds.x, UpperBounds.y, UpperBounds.z);
			break;
		case 1:
			relevantCorner.SetValues(UpperBounds.x, UpperBounds.y, LowerBounds.z);
			break;
		case 2:
			relevantCorner.SetValues(UpperBounds.x, LowerBounds.y, UpperBounds.z);
			break;
		case 3:
			relevantCorner.SetValues(UpperBounds.x, LowerBounds.y, LowerBounds.z);
			break;
		case 4:
			relevantCorner.SetValues(LowerBounds.x, UpperBounds.y, UpperBounds.z);
			break;
		case 5:
			relevantCorner.SetValues(LowerBounds.x, UpperBounds.y, LowerBounds.z);
			break;
		case 6:
			relevantCorner.SetValues(LowerBounds.x, LowerBounds.y, UpperBounds.z);
			break;
		case 7:
			relevantCorner.SetValues(LowerBounds.x, LowerBounds.y, LowerBounds.z);
			break;
	}
}

/* Some helper functions for ray tracing through the Octree:
This will set the the delta vector from traveling through
the current node.  The return value is an int indicating which side we exit:
	X: 1
	Y: 2
	Z: 3
This traces back to Matlab's max and min functions which can return the
index of the value counting from 1.
*/
template <class ValueType>
int
Octree<ValueType>::
GetExitSide(Vector& deltaToCorner, const Vector& transitionPoint, const Vector& directionVector, const Path& path,
			const int depth) const {
	double Xratio, Yratio, Zratio;
	Vector relevantCorner;
	
	// direction is used to pick which corner separates the three edges we might exit.
	SetRelevantInternalCorner(relevantCorner, directionVector, path, depth);
	deltaToCorner = relevantCorner - transitionPoint;
	
	//eliminate any directions in which the direction vector is 0
	(directionVector.x == 0.0) ? (Xratio = -1.0) : (Xratio = deltaToCorner.x / directionVector.x);
	(directionVector.y == 0.0) ? (Yratio = -1.0) : (Yratio = deltaToCorner.y / directionVector.y);
	(directionVector.z == 0.0) ? (Zratio = -1.0) : (Zratio = deltaToCorner.z / directionVector.z);
	
	//and return the side we exit: the min ratio of deltaToCorner/directionVector
	return Octree_PickMinPositiveRatio(Xratio, Yratio, Zratio);
}

// helper for GetExitSide
template <class ValueType>
void
Octree<ValueType>::
SetRelevantInternalCorner(Vector& relevantCorner, const Vector& directionVector, const Path& path, const int depth) const {
	/* use the direction vector to find the corner of the voxel which splits the
	three sides we could exit.
	*/
	Vector nodeUpperBounds;
	Vector nodeLowerBounds;
	
	CalculateBoundsFromPath(nodeLowerBounds, nodeUpperBounds, path, depth);
	
	switch(((directionVector.x >= 0) << 2)
			| ((directionVector.y >= 0) << 1)
			| (directionVector.z >= 0)) {
		case 7:
			relevantCorner.SetValues(nodeUpperBounds.x, nodeUpperBounds.y, nodeUpperBounds.z);
			break;
		case 6:
			relevantCorner.SetValues(nodeUpperBounds.x, nodeUpperBounds.y, nodeLowerBounds.z);
			break;
		case 5:
			relevantCorner.SetValues(nodeUpperBounds.x, nodeLowerBounds.y, nodeUpperBounds.z);
			break;
		case 4:
			relevantCorner.SetValues(nodeUpperBounds.x, nodeLowerBounds.y, nodeLowerBounds.z);
			break;
		case 3:
			relevantCorner.SetValues(nodeLowerBounds.x, nodeUpperBounds.y, nodeUpperBounds.z);
			break;
		case 2:
			relevantCorner.SetValues(nodeLowerBounds.x, nodeUpperBounds.y, nodeLowerBounds.z);
			break;
		case 1:
			relevantCorner.SetValues(nodeLowerBounds.x, nodeLowerBounds.y, nodeUpperBounds.z);
			break;
		case 0:
			relevantCorner.SetValues(nodeLowerBounds.x, nodeLowerBounds.y, nodeLowerBounds.z);
			break;
	}
}


// For copy and swap idiom:
template <class ValueType>
void
Octree<ValueType>::
Swap(Octree<ValueType>& octreeToSwap) {
	std::swap(LowerBounds, octreeToSwap.LowerBounds);
	std::swap(UpperBounds, octreeToSwap.UpperBounds);
	std::swap(Size, octreeToSwap.Size);
	std::swap(TrueResolution, octreeToSwap.TrueResolution);
	
	std::swap(MaxDepth, octreeToSwap.MaxDepth);
	std::swap(OffMapValue, octreeToSwap.OffMapValue);
	
	std::swap(OctreeNodeType, octreeToSwap.OctreeNodeType);
	
	OctreeNode* tempPointer = OctreeRoot;
	OctreeRoot = octreeToSwap.OctreeRoot;
	octreeToSwap.OctreeRoot = tempPointer;
}

/* Constructor helper function:
This will return the childNumber based on the bits in path x, y,
and z that would be followed for the node at depth along the path.
*/
template <class ValueType>
int
Octree<ValueType>::
GetPointChildNumber(const Vector& point, const int depth) const {
	/* Get the path, pick out the bits for this depth, then combine to get the childNumber
	Depth had better be less than max depth.
	*/
	Path path = FindPathToPoint(point);
	int childNumber =
		(((path.x & (1 << (MaxDepth - depth - 1))) != 0) << 2)
		| (((path.y & (1 << (MaxDepth - depth - 1))) != 0) << 1)
		| ((path.z & (1 << (MaxDepth - depth - 1))) != 0);
	return childNumber;
}

/* Expand Octree for adding points:
Expands the bounds of the Octree towards the point.
*/
template<class ValueType>
void
Octree<ValueType>::
ExpandOctreeToIncludePoint(const Vector& PointToInclude) {
	while(!ContainsPoint(PointToInclude)) {
		//determine the octant based on UpperBounds
		int childNumber = 0;
		childNumber = ((PointToInclude.x < UpperBounds.x) << 2)
					  | ((PointToInclude.y < UpperBounds.y) << 1)
					  | (PointToInclude.z < UpperBounds.z);
					  
		//expand the tree
		OctreeNode* currentRoot = OctreeRoot;
		OctreeRoot = new OctreeNode(EmptyValue);
		OctreeRoot->children = new OctreeNode*[8];
		for(int index = 0; index < 8; index++) {
			OctreeRoot->children[index] = new OctreeNode(EmptyValue);
		}
		delete OctreeRoot->children[childNumber];
		OctreeRoot->children[childNumber] = currentRoot;
		
		//deal with the Octree Properties
		switch(childNumber) {
			case 0: {
				UpperBounds += Size;
			}
			break;
			case 1: {
				UpperBounds.x += Size.x;
				UpperBounds.y += Size.y;
				LowerBounds.z -= Size.z;
			}
			break;
			case 2: {
				UpperBounds.x += Size.x;
				LowerBounds.y -= Size.y;
				UpperBounds.z += Size.z;
			}
			break;
			case 3: {
				UpperBounds.x += Size.x;
				LowerBounds.y -= Size.y;
				LowerBounds.z -= Size.z;
			}
			break;
			case 4: {
				LowerBounds.x -= Size.x;
				UpperBounds.y += Size.y;
				UpperBounds.z += Size.z;
			}
			break;
			case 5: {
				LowerBounds.x -= Size.x;
				UpperBounds.y += Size.y;
				LowerBounds.z -= Size.z;
			}
			break;
			case 6: {
				LowerBounds.x -= Size.x;
				LowerBounds.y -= Size.y;
				UpperBounds.z += Size.z;
			}
			break;
			case 7: {
				LowerBounds -= Size;
			}
			break;
		}
		Size.MultiplyBy(2.0);
		MaxDepth++;
	}
}



