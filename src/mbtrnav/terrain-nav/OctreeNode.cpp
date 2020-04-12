#include "Octree.hpp"

#include <fstream>
#include <iostream>

#include <cmath>

#include "OctreeSupport.hpp"

template <class ValueType>
bool
Octree<ValueType>::OctreeNode::
ItterateThroughLeaves(Octree<ValueType>& OT, int& depth, ValueType Value){
	//std::cout << "\nPath\t";
	//OT.currentItterationPath.Print();
	//std::cout << "Depth\t" << depth << "\n";
	
	if(children != NULL){
		int childNumber = OT.GetPathChildNumber(OT.currentItterationPath, depth);
		//std::cout << "childNumber\t" << childNumber << "\n";
		if(children[childNumber]->ItterateThroughLeaves(OT, ++depth, Value)){
			//std::cout << "returning True\n";
			return true;
		}
		else{
			//std::cout << "children[childNumber]->ItterateThroughLeaves(OT, ++depth, Value returned FALSE\n";
			if(childNumber < 7){
				//increment childNumber, adjust path accordingly, dive down the tree to get depth to leaf node
				int maxDepthHit = 0;
				if(FindNextChildWithValueAndSetPath(OT, depth, Value, childNumber + 1, maxDepthHit)){
				
					
					depth = maxDepthHit;
					//std::cout << "returning True having gotten next child number\nPATH:\t";
					//OT.currentItterationPath.Print();
					//std::cout << "DEPTH\t" << depth << "\n";
					return true;
				}
				else{
					depth --;
					return false;
				}
			}
			else{
				depth --;
				return false;
			}
		}
	}
	else{
		//has no children, is leaf node, next leaf in sequence is the return
		depth --;
		return false;
	}

}
template <class ValueType>
bool
Octree<ValueType>::OctreeNode::
FindNextChildWithValueAndSetPath(Octree<ValueType>& OT, int depth, ValueType Value, int startChildNumber, int& maxDepthHit){
	//std::cout << "FCWVASP depth: " << depth << "\n";
	if(children == NULL){
		//std::cout << "returning: " << (value == Value) << "\n";
		maxDepthHit = depth;
		return value == Value;
	}
	else{
		
		for(int childNumber = startChildNumber; childNumber < 8; childNumber++){
			//std::cout << "Depth: " << depth << "\tCN: " << childNumber << "\n";
			if(children[childNumber]->FindNextChildWithValueAndSetPath(OT, depth+1, Value, 0, maxDepthHit)){
				unsigned int bitmask = 1 << (OT.MaxDepth - depth - 1);
				//std::cout << "Depth: " << depth << "\tBitmask: " << bitmask << "\n";
				//std::cout << "components: " << (OT.currentItterationPath.x & ~bitmask) << "\t" << (OT.currentItterationPath.y & ~bitmask) << "\t" << (OT.currentItterationPath.z & ~bitmask) << "\n";
				//std::cout << "components: " << (childNumber & (1U << 2)) << "\t" << (childNumber & (1U << 1)) << "\t" << (childNumber & (1U)) << "\n";
				OT.currentItterationPath.x = (OT.currentItterationPath.x & ~bitmask) | (((childNumber & (1U << 2)) !=0) * bitmask);
				OT.currentItterationPath.y = (OT.currentItterationPath.y & ~bitmask) | (((childNumber & (1U << 1)) !=0) * bitmask);
				OT.currentItterationPath.z = (OT.currentItterationPath.z & ~bitmask) | (((childNumber & (1U)) !=0) * bitmask);
				//OT.currentItterationPath.Print();
				return true;
			}
		}
		return false;
	}
}


// Point adding for each OctreeType:
/*! Binary Occupancy
Sets the value of the MaxDepth leaf containing the point to true.  If a node at any level is found
which has a value of true, the function returns without needing to make any changes.  If the tree
is not yet split down to MaxDepth at the input point and the value is not true at that location,
AddPoint will split nodes until it is MaxDepth.
*/
template <class ValueType>
void
Octree<ValueType>::OctreeNode::
AddPointBinaryOccupancy(const Octree<ValueType>& OT, const Vector& point, const int depth) {
	// if at depth, add the point, otherwise figure out which child it goes with and call the child's addPoint
	if(depth < OT.MaxDepth) {
		//if value is true, we are done, otherwise we need to split
		if((children == NULL) && !this->value) {
			children = new OctreeNode*[8];
			for(int index = 0; index < 8; index++) {
				children[index] = new OctreeNode;
			}
		}
		//add the point one layer down
		children[OT.GetPointChildNumber(point, depth)]->AddPointBinaryOccupancy(OT, point, depth + 1);
	} else {
		//at depth
		value = true;
	}
}

/*! AddData
Takes a point and a value, and assigns the value to the leaf at MaxDepth containing that point.
If the tree is not yet split down to MaxDepth at the input point, AddData will split nodes until it is.
Assignment is always done, so the last entry written to a given leaf wins.  Collapsed values are
perserved in all of the new children except the MaxDepth leaf in which the new data value is located.
*/
template <class ValueType>
void
Octree<ValueType>::OctreeNode::
AddData(const Octree<ValueType>& OT, const Vector& point, const ValueType data, const int depth) {
	// if at depth, add the point, otherwise figure out which child it goes with and call the child's addData
	if(depth < OT.MaxDepth) {
		//if we don't have children, we need to split
		if(children == NULL) {
			children = new OctreeNode*[8];
			for(int index = 0; index < 8; index++) {
				children[index] = new OctreeNode(this->value);
			}
			this->value = OT.EmptyValue;
		}
		//add the data one layer down
		children[OT.GetPointChildNumber(point, depth)]->AddData(OT, point, data, depth + 1);
	} else {
		//at depth
		value = data;
	}
}



/* Collapse function
Collapsing a node compresses the tree.  It only collapses nodes where the whole volume 
is uniform.
*/
template <class ValueType>
void
Octree<ValueType>::OctreeNode::
Collapse(void) {
	/* first run each child's collapse method.  Then test to see if they are all the
	same, and this node can represent all of them.
	Criteria for collapsing this node:
		all children have the same values
		all children have no children of their own
	*/
	
	if(children != NULL) {
	
		children[0]->Collapse();
		ValueType testValue = children[0]->value;
		bool collapseThisNode = (children[0]->children == NULL);
		for(int index = 1; index < 8; index++) {
			children[index]->Collapse();
			
			collapseThisNode &= (children[index]->children == NULL);
			collapseThisNode &= (testValue == children[index]->value);
			
		}
		if(collapseThisNode) {
			value = testValue;
			for(int index = 0; index < 8; index++) {
				delete children[index];
			}
			delete[] children;
			children = NULL;
		}
	}
}

// Save, Load, and Print functions
/* Save
For use by Octree SaveToFile.
*/
template <class ValueType>
bool
Octree<ValueType>::OctreeNode::
SaveToFile(std::FILE* saveFile) const {
	/* Depth first ordering of nodes.  Each node saves its value, then a boolean
	for if it has children or not.
	*/
	//value
	std::fwrite(&value, sizeof(value), 1, saveFile);
	
	//children
	bool hasChildren = (children != NULL);
	std::fwrite(&hasChildren, sizeof(hasChildren), 1, saveFile);
	
	//set up for children to write
	if(hasChildren) {
		for(int index = 0; index < 8; index++) {
			children[index]->SaveToFile(saveFile);
		}
	}
	return !std::ferror(saveFile);
}

/* Load
For use by Octree LoadFromFile.
*/
template <class ValueType>
bool
Octree<ValueType>::OctreeNode::
LoadFromFile(std::FILE* loadFile, int& numBranchNodes, int& numLeafNodes) {
	//first the value
	if(0 == std::fread(&value, sizeof(ValueType), 1, loadFile)) {
		return false;
	}
	//then the children
	bool hasChildren;
	if(0 == std::fread(&hasChildren, sizeof(bool), 1, loadFile)) {
		return false;
	}
	
	//clean up any old stuff in this structure regardless of hasChildren
	if(children != NULL) {
		for(int index = 0; index < 8; index++) {
			delete children[index];
		}
		delete[] children;
	}
	children = NULL;
	
	//and allocate the children if needed
	/*Old:
	bool returnValue = true;
	if(hasChildren) {
		children = new OctreeNode*[8];
		for(int index = 0; index < 8; index++) {
			children[index] = new OctreeNode;
			returnValue &= children[index]->LoadFromFile(loadFile);
		}
	}
	*/
	//New: 
	bool returnValue = true;
	if(hasChildren) {
		children = new OctreeNode*[8];
		for(int index = 0; index < 8; index++) {
			children[index] = new OctreeNode;
			returnValue &= children[index]->LoadFromFile(loadFile, numBranchNodes, numLeafNodes);
		}
		numBranchNodes += 1;
	}
	else{
		numLeafNodes += 1;
	}
	
	
	
	
	return returnValue;
}

/* Print
Bad idea for large Octrees.  Used mostly when debugging on small testcase Octrees.
*/
template <class ValueType>
void
Octree<ValueType>::OctreeNode::
Print(int num) const {
	//Depth first order
	//PrintTabs indents so that the tree is actually readable by a human.
	OctreeNode_PrintTabs(num);
	std::cout << "value:    " << value << std::endl;
	OctreeNode_PrintTabs(num);
	std::cout << "children: " << (children != NULL) << std::endl;
	OctreeNode_PrintTabs(num);
	std::cout << "---------------\n";
	
	if(children != NULL) {
		for(int index = 0; index < 8; index ++) {
			children[index]->Print(num + 1);
		}
	}
}

/* Constructors and such:
Some more are defined in the OctreeNode classdef at the bottom of class Octree in Octree.hpp
*/
/* Copy Constructor
Defined because children is dynamically allocated.
*/
template <class ValueType>
Octree<ValueType>::OctreeNode::
OctreeNode(const OctreeNode& nodeToCopy) {
	value = nodeToCopy.value;
	if(nodeToCopy.children) {
		int index;
		children = new OctreeNode*[8];
		for(index = 0; index < 8; index++) {
			children[index] = new OctreeNode(*(nodeToCopy.children[index]));
		}
	} else {
		children = NULL;
	}
}

/* copy assignment operator
Defined because children is dynamically allocated.
*/
template <class ValueType>
typename Octree<ValueType>::OctreeNode& //typename means Octree<ValueType>::OctreeNode is a type
Octree<ValueType>::OctreeNode::
operator=(OctreeNode rightHandSide) {
	//copy and swap
	this->Swap(rightHandSide);
	return *this;
}

/* Swap
For copy and swap idiom.
*/
template <class ValueType>
void
Octree<ValueType>::OctreeNode::
Swap(OctreeNode& nodeToSwap) {
	std::swap(nodeToSwap.value, value);
	
	OctreeNode** tempPointer = children;
	children = nodeToSwap.children;
	nodeToSwap.children = tempPointer;
}

/* Destructor
Defined because children is dynamically allocated.
*/
template <class ValueType>
Octree<ValueType>::OctreeNode::
~OctreeNode() {
	if(children) {
		for(int index = 0; index < 8; index ++) {
			delete children[index];
		}
	}
	delete[] children;
}



template class Octree<bool>::OctreeNode;

