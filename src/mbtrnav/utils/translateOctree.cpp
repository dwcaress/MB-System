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

using namespace std;
int main( int argc, char **argv )
{
	Octree<bool>* oct = new Octree<bool>();

	if( argc < 2 )
	{
	   printf("Please supply a file name, without the suffix.\n");
	   return 1;
	}
	
	char inFile[128], outFile[128];
	snprintf(inFile, 128, "%s.bo", argv[1]);
	snprintf(outFile, 128, "%sSerm.bo", argv[1]);

	if( access(inFile, F_OK) )
	{
	   printf("File %s not found.\n", inFile);
	   return 1;
	}
	
	//Open the output .csv file
	std::cout << "Output File: " << outFile << endl;

	//Load the input octree
	std::cout << "Loading Octree" << inFile << endl;
	oct->LoadFromFile(inFile);

	oct->Collapse();

	// [SermEasting SermNorthing SermZone]= geo2utm(-37.886397, 65.952721)
	double SermNorthing = -7315089;
	double SermEasting  =  -550634;
	Vector newOrigin(SermNorthing, SermEasting, 0.);
	// [PeterEasting PeterNorthing PeterZone]= geo2utm(-38.3169802, 65.9356737)
	//double PeterNorthing = -7312909;
	//double PeterEasting  =  -531077;
	//Vector newOrigin(PeterNorthing, PeterEasting, 0.);
	
	oct->moveOctree( newOrigin );
	oct->SaveToFile( outFile );

	oct->Print();
	
	delete oct;
	
}
