#ifndef Octree_H
#define Octree_H
//#define Octree_H_Inside_Header

#include "OctreeSupport.hpp"

#include <fstream>

/*! WHERE STUFF IS DOCUMENTED:

overarching descriptions of Octree Stuff: here (Octree.hpp)
	class Octree, class OtreeNode
	sub-pieces of Octree
		struct Vector, struct Path
	class function declarations only
		for documentation on the functions, look at Octree.tcc and OctreeNode.tcc
		functions in the .tcc files appear in the same order as the declarations in the classdef

Specifics of each Octree function: Octree.tcc
	description of functionality: above the function
	description of implementation: inside the function

Specifics of each OctreeNode function: OctreeNode.tcc
	description of functionality: above the function
	description of implementation: inside the function

Specifics for Vector functions and struct Path : OctreeSupport.cpp and OctreeSupport.hpp
	These are mostly things like operator+ for two vectors.  If you can't figure out what
	they do, I can't help you.
*/

/*! Overarching Goal of Octree.hpp and related stuff:
Octrees are intended to be used for mapping and navigation in fully 3D underwater environments.
Octrees are a compressed representation of discritized (gridded) space.  The compression
comes from groupoing together volumes of space which have the same "value" attached.  This
grouping is represented in a tree structure:
	- The tree is stored and accessed by the root (depth zero) node.
	- Nodes have either eight children (branch nodes) or no children (leaf nodes)
	- At each successive level of branch nodes, the parent's volume is divided in half
		on all three dimensions to get the volumes for the eight children.
	- Leaf nodes have a meaningful value attached; this is the whole point of the tree.
	- When a leaf node is not at the maximum depth, it represents a compression of all
		the leaves that would have fit in that space.
*/

/*! How this relates to code: */
/*! Description of Octree:
The Octree class is the primary class defining Octrees.  It stores general properties of the
octree (size, bounds, resolution, tree depth, octree 'type' ) as well as the OctreeNode which
is the root of the octree.  The Octree class also has all the functions assiociated with using
octrees:
	- Measurement functions (Ray traceing, Querying, and a linear interpolation Query)
	- Constructor and addPoint functions for making the octree structure and giving it data to
		represent
	- Save and Load functions
	- Private helper functions for using the Octree

You may notice that Octree is templated; this allows the same class to represent many
different possible types of data with the same functionality.  OctreeType is somewhat related
to ValueType; it tells the octree how to set node values based on the input points (more in the
OctreeTypes section below).
*/
/*! Description of OctreeNode:
The OctreeNode class is defined privately inside class Octree.  This is to hide the
implementation of Octree from the user.  The class has two variables: value and children.
- Value is the payload of the node.  If the node is a leaf, the OctreeType and the input
	points determine what the value is.  If the node is a branch node, value is EmptyValue
	(most likely zero).
- Children is declared as an OctreeNode**.  It has two valid configurations:
	NULL (indicating that the node is a leaf)
	pointing to an array of eight pointers to OctreeNodes all of which have been initialized
		(indicating that this is a branch node)

Notes:
- OctreeNode is declared inside Octree, so it always shares the template ValueType of
	the Octree from which it is called.
- OctreeNode and Octree are both friends with each other, so they have complete access to each
	other's private variables
*/
/*! Description of OctreeNodeTypes:
To represent a set of points in an octree structure, there needs to be a rule for setting
the value of a node based on the points inside of it.  This is where OctreeType comes in;
there are several methods for setting values based on points already, and you can add more
if you need to.  All that is required is adding the name to EnumOctreeType, adding the case
to the switch statements in the add*** functions (or leaving it in the the default
case: error), adding the NewTypeAddPoint(...) function to OctreeNode.tcc, and adding its
documentation here.

Current Octree Types:
	- BinaryOccupancy stores true in a leaf if any points fall inside that leaf, and false otherwise.
		<bool>
	
	- Data type: User specifies the value to store at each node by inputing (point, value) pairs.
		Unspecified nodes have value = EmptyValue.
		<Literally any type> but it is on you to figure out what should go in each voxel.

Note about ValueType and OctreeType: It is up to you to combine them inteligently.  You
are capable of making a BinaryOccupancy Octree with ValueType 'long double' and it
probably won't break anything. ...but I will think less of you as a person.  Also, some
combinations may cause unexpected results or not compile like a PointCount Octree with ValueType 'bool'
*/
/*! I should probably mention Vectors:
Vector has three public double variables: x, y, and z.  They are a nice simple way of representing
three-space vectors and points (think linear algebra column vector).  They have addition, subtraction,
and Norm functions.
*/
/*! and Paths:
Struct Path has three unsigned ints x, y, and z.  Path refers to the route from the root of the tree
to a specific leaf node.  Each binary digit in x, y, and z indicates which direction to go along the
corresponding axis (zero - towards negative; one - towards positive).  The least significant bit
refers to the last/lowest choice in the tree, while the more significant bits refer to paths higher
in the tree.  To convert the bits into a number for indexing the array of child pointers, take the
bits in x, y, z order and treat them as a three digit binary number.
*/

namespace OctreeType {
	enum EnumOctreeType {
		BinaryOccupancy,
		PlanarFitFromDEM,
		Data
	};
}

struct OTreeStats_s{
    unsigned long depth;
    unsigned long nodes;
    unsigned long leaves;
    unsigned long branches;
};
typedef OTreeStats_s OTreeStats;

template <class ValueType>
class Octree {
		class OctreeNode;//defined at the bottom of the classdef
		
	public:
    // byte-align these structs
#pragma pack(push, 1)
    struct MapHeader_s{
        Vector LowerBounds ;
        Vector UpperBounds;
        Vector Size;
        Vector TrueResolution;
        int MaxDepth;
        ValueType OffMapValue;
        ValueType EmptyValue;
        OctreeType::EnumOctreeType OctreeNodeType;
    };
    typedef struct  MapHeader_s MapHeader;

    struct OctreeNode_s{
        ValueType value;
        bool hasChildren;
    };
    typedef struct OctreeNode_s OTNode;
#pragma pack(pop)

        void moveOctree(const Vector& newOrigin){
			this->LowerBounds -= newOrigin;
			this->UpperBounds -= newOrigin;
		}
		
		//for making map measurements
		double RayTrace(const Vector& startPoint, const Vector& directionVector) const;
		
		//for Stevesie to plot
		bool IterateThroughLeaves(Vector& nodeLowerBounds, Vector& nodeUpperBounds, ValueType value);
		
		//for making map measurements
		ValueType Query(const Vector& queryPoint) const;
		double InterpolatingQuery(const Vector& queryPoint) const;
		
		//constructors and such
		Octree();
		Octree(const Octree<ValueType>& octreeToCopy);
		Octree(const Vector& desiredResolution, const Vector& lowerBounds,
			   const Vector& upperBounds, const OctreeType::EnumOctreeType octreeType);
		Octree& operator=(Octree<ValueType> rightHandSide);
		~Octree();
		
		//adding points to the Octree
		bool AddPoint(const Vector& point);
		int AddPoints(const Vector points[], const unsigned int numPoints);
		bool AddData(const Vector& point, const ValueType data);
		int AddData(const Vector points[], const ValueType data[], const unsigned int numDatas);
		
		
		void FillSmallestResolutionLeafAtPointIfEmpty(const Vector& point, ValueType fillValue);
		//reducing the memory requirements
		void FillIfEmpty(const Vector& point, ValueType fillValue);
		void FillIfEmpty(const Vector points[], unsigned int numPoints, ValueType fillValue);
		void Collapse(void);
		
		//save and load
		bool SaveToFile(const char* filename) const;
		bool LoadFromFile(const char* filename);
		
		//print
        void Print(OTreeStats *ts=NULL) const;
        static int DiskSize(OTreeStats *ts=NULL);
        static int MemSize(OTreeStats *ts=NULL);

		//Get functions
		Vector GetTrueResolution(void) const {	return this->TrueResolution; }
		Vector GetLowerBounds(void) const { return this->LowerBounds; }
		Vector GetUpperBounds(void) const { return this->UpperBounds; }
        static size_t NodeSize(){return sizeof(Octree<ValueType>::OctreeNode);}
	private: // helper functions
		// Path functions
		Path FindPathToPoint(const Vector& desiredPoint) const;
		Path FindPathToPointFromNode(const Vector& desiredPoint, const Path& path, const int depth) const;
		bool PathElementIsValid(const unsigned int pathElement) const;
		
		// Bounds and ContainsPoint
		void CalculateBoundsFromPath(Vector& nodeLowerBounds, Vector& nodeUpperBounds, const Path& path, const int depth) const;
		bool ContainsPoint(const Vector& point) const;
		
		// accessing nodes by their path
		OctreeNode* GetPointerToLeafOnPath(const Path& path) const;
		OctreeNode* GetPointerToLeafOnPath(int& depth, const Path& path) const;
		OctreeNode* GetPointerToLeafOnPath(const unsigned int Xpath, const unsigned int Ypath, const unsigned int Zpath) const;
		OctreeNode* GetPointerToLeafOnPath(int& depth, const unsigned int Xpath, const unsigned int Ypath,
										   const unsigned int Zpath) const;
										   
		// RayTrace helpers (two pairs of functions)
		double RayTraceToThisOctree(Vector& transitionPoint, const Vector& startPoint, const Vector& directionVector) const;
		void SetRelevantExternalCorner(Vector& relevantCorner, const Vector& directionVector) const;
		
		int GetExitSide(Vector& deltaToCorner, const Vector& transitionPoint, const Vector& directionVector, const Path& path,
						const int depth) const;
		void SetRelevantInternalCorner(Vector& relevantCorner, const Vector& directionVector, const	Path& path, const int depth) const;
		
		// a few more functions
		void Swap(Octree<ValueType>& octreeToSwap);
		int GetPointChildNumber(const Vector& point, const int depth) const;
		int GetPathChildNumber(const Path& path, const int depth) const;
		void ExpandOctreeToIncludePoint(const Vector& Point);
		
	private: // variables
		Vector LowerBounds;
		Vector UpperBounds;
		Vector Size;
		Vector TrueResolution;
		
		int MaxDepth;
		ValueType OffMapValue;
		ValueType EmptyValue;
		OctreeType::EnumOctreeType OctreeNodeType;
		
		OctreeNode* OctreeRoot;
		
		Path currentIterationPath;
		bool treeComplete;
	private:
		friend class OctreeNode;
		class OctreeNode {
			bool IterateThroughLeaves(Octree<ValueType>& OT, int& depth, ValueType Value);
			bool FindNextChildWithValueAndSetPath(Octree<ValueType>& OT, int depth, ValueType Value, int startChildNumber, int& maxDepthHit);
			//add points
			void AddPointPointCount(const Octree<ValueType>& OT, const Vector& point, const int depth);
			void AddPointBinaryOccupancy(const Octree<ValueType>& OT, const Vector& point, const int depth);
			void AddData(const Octree<ValueType>& OT, const Vector& point, const ValueType data, const int depth);
			
			
			
			//collapse
			void Collapse(void);
			
			//save and load
			bool SaveToFile(std::FILE* saveFile) const;
			
			//Octree.hpp Old: 
			//bool LoadFromFile(std::FILE* loadFile);
			//New: 
			bool LoadFromFile(std::FILE* loadFile, int& numBranchNodes , int& numLeafNodes);
			
			//print
            void Print(int num, OTreeStats *ts=NULL) const;
			//constructors and such
			explicit OctreeNode(ValueType Value): value(Value), children(NULL) {}
			OctreeNode(): value(static_cast<ValueType>(0)), children(NULL) {}
			OctreeNode(const OctreeNode& nodeToCopy);
			OctreeNode& operator=(OctreeNode rightHandSide);
			void Swap(OctreeNode& nodeToSwap);
			~OctreeNode();
			
			//variables
			ValueType value;
			OctreeNode** children;
			
			friend class Octree<ValueType>;
		};
};


#endif
