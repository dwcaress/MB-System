
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
	char inFile[128];

	sprintf(inFile, "SantaMonica800mMound_Topo1m_UTM.bo");

	//Load the input octree
	std::cout << "Loading Octree" << inFile << endl;
	oct->LoadFromFile(inFile);

	oct->Collapse();

	Vector Nadir( 0., 0., 1. );
	
	//[ee nn zz]=geo2utm(-118.6045201, 33.8545593)
	//Depth -117
	double UpperNorthing = 3747188.;
	double UpperEasting  =  351566.;
	Vector Upper( UpperNorthing, UpperEasting, 0.);
	double UpperDepth;
	UpperDepth = oct->RayTrace(Upper, Nadir);
	std::cout << "Depth of Upper Point = " << UpperDepth << endl;

	//[ee nn zz]=geo2utm(-118.6466156, 33.7992242)   %The Mound
	//Depth -803
	double MoundNorthing = 3741113.;
	double MoundEasting  =  347573.;
	Vector Mound( MoundNorthing, MoundEasting, 0.);
	double MoundDepth;
	MoundDepth = oct->RayTrace(Mound, Nadir);
	std::cout << "Depth of Mound = " << MoundDepth << endl;

	//[ee nn zz]=geo2utm(-118.6722865, 33.7876487)   %Lower
	// Depth = -880.151
	double LowerNorthing = 3739868.;
	double LowerEasting  =  345176.;
	Vector Lower( LowerNorthing, LowerEasting, 0.);
	double LowerDepth;
	LowerDepth = oct->RayTrace(Lower, Nadir);
	std::cout << "Depth of Lower Point = " << LowerDepth << endl;

	//In a void:
	//[ee nn zz]=geo2utm(-118.640, 33.840)
	Vector Void( 3745625.28, 348258.26, 0 );
	std::cout << "Depth of Void Point = " << oct->RayTrace(Void, Nadir) << endl;
	



	
	delete oct;
	
}
