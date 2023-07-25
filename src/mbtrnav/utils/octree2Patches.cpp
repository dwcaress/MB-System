/*! This code has steps:

*/


#include "OctreeSupport.hpp"
#include "Octree.hpp"
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>


#include "OctreeSupport.hpp"
#include "Octree.hpp"
#include <iostream>
#include <iomanip>
#include <unistd.h>

//#define INFILE "tripleIsland.bo"
//#define OUTFILE "tripleIsland.csv" 

#define INFILE "tripleIsland10.bo"
#define OUTFILE "tripleIsland10.csv" 

using namespace std;
int main( int argc, char **argv )
{
	int totalBoxes;
	int totalFaces;
	ofstream outFile;
	Octree<bool>* oct = new Octree<bool>();
	Vector nodeLowerBounds;
	Vector nodeUpperBounds;
	Vector center;
	Vector size;
	Vector resolution;
	Vector rayTrace;
	Vector centerPlane;
	bool faceVisible;

	if( argc < 2 )
	{
	   printf("Please supply a file name, without the suffix.\n");
	   return 1;
	}
	
	char inFile[128], outFileAsc[128];
	snprintf(inFile, 128, "%s.bo", argv[1]);
	snprintf(outFileAsc, 128, "%sPatches.csv", argv[1]);

	if( access(inFile, F_OK) )
	{
	   printf("File %s not found.\n", inFile);
	   return 1;
	}
	
	//Open the output .csv file
	std::cout << "Output File: " << outFileAsc << endl;
	outFile.open(outFileAsc);

	//Load the input octree
	std::cout << "Loading Octree" << inFile << endl;
	oct->LoadFromFile(inFile);

	oct->Collapse();
	
	resolution = oct->GetTrueResolution();
	totalBoxes = 0;
	totalFaces = 0;
	
	//Iterate through all the filled octree boxes
	while(oct->IterateThroughLeaves(nodeLowerBounds, nodeUpperBounds, true)){
		//First 6 columns of the output file are: x_lower, x_upper, y_lower, y_upper, z_lower, z_upper
		outFile << setprecision(10) << nodeLowerBounds.x << "," << nodeUpperBounds.x << ",";
		outFile << nodeLowerBounds.y << "," << nodeUpperBounds.y << ",";
		outFile << nodeLowerBounds.z << "," << nodeUpperBounds.z << ",";
		totalBoxes++;
		
		//Calculate center and size of the octree
		center = (nodeUpperBounds+nodeLowerBounds)/2.0;
		size = (nodeUpperBounds-nodeLowerBounds);

		//Check all size faces of the cube to see if they are visible 
		
		//Two faces where x is constant
		for(double x=center.x-0.5*size.x-resolution.x/2;x<=center.x+0.5*size.x+resolution.x/2;x=x+size.x+resolution.x){
			faceVisible = false;
			for(double y=nodeLowerBounds.y+(resolution.y/2.0);
				y<=nodeUpperBounds.y-(resolution.y/2.0);
				y=y+resolution.y){
				for(double z=nodeLowerBounds.z+(resolution.z/2.0);
					z<=nodeUpperBounds.z-(resolution.z/2.0);
					z=z+resolution.z){
						rayTrace.x = x;
						rayTrace.y = y;
						rayTrace.z = z;
						centerPlane.x = center.x-x;
						centerPlane.y = 0.0;
						centerPlane.z = 0.0;
						
					if(oct->RayTrace(rayTrace,centerPlane)>=resolution.x/2.0){
						faceVisible = true;
					}
					if(faceVisible) break;
				}
				if(faceVisible) break;
			}
			
			//1 if the current face is visible, 0 if not
			if(faceVisible){
				outFile << 1 << ",";
				totalFaces++;
			}else{
				outFile << 0 << ",";
			}
		}

		//Two faces where y is constant
		for(double y=center.y-0.5*size.y-resolution.y/2;y<=center.y+0.5*size.y+resolution.y/2;y=y+size.y+resolution.y){
			faceVisible = false;
			for(double x=nodeLowerBounds.x+(resolution.x/2.0);
				x<=nodeUpperBounds.x-(resolution.x/2.0);
				x=x+resolution.x){
				for(double z=nodeLowerBounds.z+(resolution.z/2.0);
					z<=nodeUpperBounds.z-(resolution.z/2.0);
					z=z+resolution.z){
						rayTrace.x = x;
						rayTrace.y = y;
						rayTrace.z = z;
						centerPlane.x = 0.0;
						centerPlane.y = center.y-y;
						centerPlane.z = 0.0;
						
					if(oct->RayTrace(rayTrace,centerPlane)>=resolution.y/2.0){
						faceVisible = true;
					}
					if(faceVisible) break;
				}
				if(faceVisible) break;
			}
			if(faceVisible){
				outFile << 1 << ",";
				totalFaces++;
			}else{
				outFile << 0 << ",";
			}
			
		}

		//Two faces where z is constant
		for(double z=center.z-0.5*size.z-resolution.z/2;z<=center.z+0.5*size.z+resolution.z/2;z=z+size.z+resolution.z){
			faceVisible = false;
			for(double y=nodeLowerBounds.y+(resolution.y/2.0);
				y<=nodeUpperBounds.y-(resolution.y/2.0);
				y=y+resolution.y){
				for(double x=nodeLowerBounds.x+(resolution.x/2.0);
					x<=nodeUpperBounds.x-(resolution.x/2.0);
					x=x+resolution.x){
						rayTrace.x = x;
						rayTrace.y = y;
						rayTrace.z = z;
						centerPlane.x = 0.0;
						centerPlane.y = 0.0;
						centerPlane.z = center.z-z;
					if(oct->RayTrace(rayTrace,centerPlane)>=resolution.z/2.0){
						faceVisible = true;
					}
					
					if(faceVisible)
						break;
				}
				if(faceVisible)
					break;
			}
			if(faceVisible){
				outFile << 1 << ",";
				totalFaces++;
			}else{
				outFile << 0 << ",";
			}
		}
				
		outFile << endl;

	}
	std::cout << "Total Filled Boxes: " << totalBoxes << endl;
	std::cout << "Total Visible Faces: " << totalFaces << endl;
	outFile.close();
	delete oct; //Destroy octree object

	
}
