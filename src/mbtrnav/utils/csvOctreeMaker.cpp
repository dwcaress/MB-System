/*! grdOctreeMaker
This octree generator takes a utm grd file and generates an octree from it.  

The input file is set with INFILE and the name to write to is OUTFILE.  

For details on how octree works including the internals of how the octree generates from the inpot 
points, go look at Octree.hpp.
*/

/*! Coordinate Systems
Octrees used for TRN have historically been stored in the NED coordinate system.  This code can 
take a The define X_INDEX_FIRST sets the indexing order into the one dimensional array of z values.  
I have only ever worked with X_INDEX_FIRST = 1, but it should work to negate z and flip the x-y index 
ordering when set to 0. If things aren't working, look at the points or the north, east, and down 
bounds on the points and make sure they are correct for a NED coordinate system point in Monterey
Bay (X: 4E6, Y: 5E5, Z: >0).

The Raytrace function of the octree requires a Euclidean coordinate system to give meaningful
results. Otherwise, the octree will work fine in any coordinate system (like LLA), and querying 
the value at a location (with an LLA query point) will work fine.
*/

/*! This code has steps:
1) autodetect the size of the map (and resolution optionally)
2) generate the octree object
3) add points to the octree
4) fill the octree cell(s) below the added points
5) compress the octree
*/


#include "OctreeSupport.hpp"
#include "Octree.hpp"
#include <iostream>
//#include <vector>
//#include <fstream>
#include <sstream>
#include <string>

//#define RESOLUTION 100
//#define RESOLUTION 10
//#define RESOLUTION 5
  #define RESOLUTION 2
//#define RESOLUTION 1

// Monterey Bay:
//#define NORTH_OFFSET 4060000
//#define EAST_OFFSET   590000
// Aariaa fjord
//#define NORTH_OFFSET 7315089
//#define EAST_OFFSET   550634

#define NORTH_OFFSET 0
#define EAST_OFFSET  0


#define DEPTH_OFFSET 0


#include <netcdf.h>
#include "mapio.h"
#include "OctreeSupport.hpp"
#include "Octree.hpp"
#include <iostream>
#include <unistd.h>


//#define OUTFILE "tripleIsland.bo"
//#define INFILE  "tripleIslandPoints.csv" 

//#define OUTFILE "tripleIsland10.bo"
//#define INFILE  "cubes.csv" 

using namespace std;
int main( int argc, char **argv ) 
{
	cout << "0\n";
	double x, y, z;
	string line;
	// Read input data

	if( argc < 2 )
	{
	   printf("Please supply a file name, without the suffix.\n");
	   return 1;
	}
	
	char inFile[128], outFile[128];
	snprintf(inFile, 128, "%s.csv", argv[1]);
	snprintf(outFile, 128, "%s.bo", argv[1]);

	if( access(inFile, F_OK) )
	{
	   printf("File %s not found.\n", inFile);
	   return 1;
	}
	
	cout << "1\n";
	ifstream inputFile(inFile);
	cout << "2\n";
	
	int count = 0;
	if (inputFile.is_open()){
		/*printf("Loading map file... \n");
		for(int i=0; i<11; i++){
			getline (inputFile,line);
			cout << line << "\n";
		}
		
		cout << "\n\n";*/
		while ( getline(inputFile,line) ){
			count ++;
		}
	}
	inputFile.close();
	inputFile.open(inFile);
	unsigned int NumPoints = count; 	// Number of points in map
	std::cout << "NumPoints: " << NumPoints << std::endl;
	
	Vector* points = new Vector[NumPoints];
	
	count = 0;
	if (inputFile.is_open()){
		printf("Loading map file... \n");
		/*for(int i=0; i<11; i++){
			getline (inputFile,line);
			cout << line << "\n";
		}
		
		cout << "\n\n";*/
		while ( getline(inputFile,line) ){
			char lineAsCharStar[100] = "";
			//for(int i = 0; i < 12; i++){ cout << "char: " << static_cast<unsigned int>( line[i]) << "\n";}
			if(count %10000  == 0){std::cout << count << std::endl;}
			//cout << line << "\n";
			
			unsigned int temp = line.find(",", 0);
			for(unsigned int i = 0; i< temp; i++){
				lineAsCharStar[i] = line[i];
			}
			//std::cout << "LACS:" << lineAsCharStar << "\n";
			lineAsCharStar[temp] = 0;
			x = strtod(lineAsCharStar, NULL);
			for(unsigned int i=0;i<temp;i++){
				lineAsCharStar[i] = 0;
			}
			
			temp++;
			unsigned int temp2 = line.find(",", temp);
			
			//cout << temp2 << "\ttemp: " << temp << "\n";
			for(unsigned int i = 0; i< temp2-temp; i++){
				lineAsCharStar[i] = line[i+temp];
			}
			//std::cout << "LACS:" << lineAsCharStar << "\n";
			y = strtod(lineAsCharStar, NULL);
			
			for(unsigned int i = 0; i< temp2-temp; i++){
				lineAsCharStar[i] = 0;
			}
			temp2++;
			for(unsigned int i = 0; i< 10; i++){
				lineAsCharStar[i] = line[i+temp2];
				
			}
			//std::cout << "LACS:" << lineAsCharStar << "\n";
			z = strtod(lineAsCharStar, NULL);

			//This was for Marcus, who worked in ENU.  But then
			//he apparently had a sign error and had z negative??
			//points[count] = Vector( y, x, -z); //ENU -> NED
			points[count] = Vector( x, y, z);     //NED -> NED
			//points[count].Print();
			count ++;
			
			
		}
	}
	else {     // Invalid input filename?
		printf("****** Failed to open  ******\n");
		return 1;
	}
	
	
	
	
	Vector DesiredResolution(RESOLUTION,RESOLUTION,RESOLUTION);
	
	std::cout << "Detecting point cloud size:\n";
	
	Vector Lowermost;
	Vector Uppermost;
	bool uninitialized = true;
	
	//autodetect size
	Vector point;
	for(unsigned int index = 0; index < NumPoints; index++){
	
		if(points[index].z != 99999 && !isnan(points[index].z)){ 
			//test bounds
			//points[index] = Vector( x, y, z);


			/*
			if((NORTH_BOUND != -1) && (xVec[xIndex] > NORTH_BOUND)){continue;}
			if((SOUTH_BOUND != -1) && (xValues[xIndex] < SOUTH_BOUND)){continue;}
			if((EAST_BOUND != -1) && (yValues[yIndex] > EAST_BOUND)){continue;}
			if((WEST_BOUND != -1) && (yValues[yIndex] < WEST_BOUND)){continue;}
			if((MAX_ACCEPTED_DEPTH != -1) && (zValues.getZ(xIndex, yIndex) > MAX_ACCEPTED_DEPTH)){continue;}
			if((MIN_ACCEPTED_DEPTH != -1) && (zValues.getZ(xIndex, yIndex) < MIN_ACCEPTED_DEPTH)){continue;}
			*/		
			if(uninitialized){
				Lowermost = points[index];
				Uppermost = points[index];
				uninitialized = false;
				//Lowermost.Print();
			}
			
			point = points[index];
			if(Lowermost.x > point.x){ Lowermost.x = point.x;}
			if(Lowermost.y > point.y){ Lowermost.y = point.y;}
			if(Lowermost.z > point.z){ Lowermost.z = point.z;}
			if(Uppermost.x < point.x){ Uppermost.x = point.x;}
			if(Uppermost.y < point.y){ Uppermost.y = point.y;}
			if(Uppermost.z < point.z){ Uppermost.z = point.z;}
		}
	}
	std::cout << "Lowermost: ";
	Lowermost.Print();
	std::cout << "Uppermost: ";
	Uppermost.Print();
	
	Vector PointCloudSize = Uppermost - Lowermost + Vector(1.0, 1.0, 1.0);
	Vector OctreeSize = DesiredResolution;
	printf("PointCloudSize\t");
	PointCloudSize.Print();
	while(!OctreeSize.StrictlyGreaterOrEqualTo(PointCloudSize)){
		OctreeSize *= 2.0;
		printf("OctreeSize\t");
		OctreeSize.Print();
	}
	Vector LowerBounds = Lowermost - DesiredResolution *0.5;
	Vector UpperBounds = LowerBounds + OctreeSize;
	
	//initialize octree
	printf("about to build Octree\n");
	
	Vector positionOffset(NORTH_OFFSET, EAST_OFFSET, DEPTH_OFFSET);
	
	Octree<bool> newOctreeMap(DesiredResolution + Vector(0.001,0.001,0.001), LowerBounds + positionOffset, UpperBounds + positionOffset,
							OctreeType::BinaryOccupancy);
	
	newOctreeMap.Print();
	
	// Add points
	std::cout << "adding points\nrow\t# added\tLast Point Tested\n";
	int countPointsAdded = 0;
	for(unsigned int index = 0; index < NumPoints; index++){
		point = points[index];
		if( point.z != 99999 && !isnan(point.z)){
				
			countPointsAdded ++;
			newOctreeMap.AddPoint(point + positionOffset);
			
		}
		//report progress
		if(index %10000 == 0){std::cout << index << "\t" << countPointsAdded << "\t";point.Print();}
	}
	
	//Vector TrueResolution = newOctreeMap.GetTrueResolution();
	//compress
	printf("about to collapse\n");
	newOctreeMap.Collapse();
	/*
	//put any test ray traces you want here. 
	std::cout << "\nTest Ray Traces\n";
	std::cout << newOctreeMap.RayTrace(Vector(SOUTH_BOUND + 2, EAST_BOUND - 2,0), Vector(0,0,1)) << "\t";
	std::cout << newOctreeMap.RayTrace(Vector(SOUTH_BOUND + 2, EAST_BOUND + 2,0), Vector(0,0,1)) << "\n";
	std::cout << newOctreeMap.RayTrace(Vector(SOUTH_BOUND - 2, EAST_BOUND - 2,0), Vector(0,0,1)) << "\t";
	std::cout << newOctreeMap.RayTrace(Vector(SOUTH_BOUND - 2, EAST_BOUND + 2,0), Vector(0,0,1)) << "\n";
	*/
	std::cout << "\nDone building octree\n";
	newOctreeMap.SaveToFile(outFile);
	std::cout << "Done\n\n";
	
	newOctreeMap.Print();
	
	
	return 0;
}
