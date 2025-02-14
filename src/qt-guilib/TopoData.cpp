#include <limits>
#include <iostream>
#include <vector>
#include <math.h>
#include "TopoData.h"

using namespace std;

// Use of very large number here (e.g. std::numeric_limits<int>)
// causes problems with VTK
const double mb_system::TopoData::NoData = -10000000.;

const char *mb_system::TopoData::GeographicType_ = "Geographic";
const char *mb_system::TopoData::UtmType_ = "UTM";

using namespace mb_system;

TopoData::TopoData() {
}


bool TopoData::getElevProfile(int startRow, int startCol,
			      int endRow, int endCol,
			      int nPieces,
			      vector<array<double, 2>> *profile) {

  double startX, startY, startZ, endX, endY, endZ;

  if (!getXYZ(startRow, startCol, &startX, &startY, &startZ)) {
    cerr << "getElevProfile(): invalid start: " << startRow << \
      ", " << startCol << "\n";
    return false;
  }
  if (!getXYZ(endRow, endCol, &endX, &endY, &endZ)) {
    cerr << "getElevProfile(): invalid end: " << endRow << ", " <<
      endCol << "\n";
    return false;    
  }  

  double rowInterval = (endRow - startRow) / nPieces;
  double colInterval = (endCol - startCol) / nPieces;

  int n = 0;
  for (int col = startCol; col < endCol; col += colInterval) {
    for (int row = startRow; row < endRow; row += rowInterval) {
      double x, y, z;
      if (!getXYZ(row, col, &x, &y, &z)) {
	std::cerr << "Invalid data at row " << row << ", col " << col << "\n";
	continue;
      }
      array<double, 2> profilePoint;
      // Compute horizontal distance from starting point
      double distance = sqrt(pow(x - startX, 2) + pow(y - startY, 2));

      profilePoint[0] = distance;
      profilePoint[1] = z;
      
      profile->push_back(profilePoint);
    }
  }
  return true;
}
