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

  profile->clear();
  
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

  double m = (endRow - startRow) / (endCol - startCol);
  double b = startRow - m * startCol;

  array<double, 2> profilePoint;
  float incr = (endCol - startCol) / nPieces;
  int colIncr = incr + 0.5;
  for (int col = startCol; col < endCol; col += colIncr) {
    int row = m * col + b;
    double x, y, z;
    if (!getXYZ(row, col, &x, &y, &z)) {
      std::cerr << "Invalid data at row " << row << ", col " << col << "\n";
      continue;
    }

    // Compute horizontal distance from starting point
    double h = sqrt(pow(x - startX, 2) + pow(y - startY, 2));

    profilePoint[0] = h;
    profilePoint[1] = z;
    profile->push_back(profilePoint);
  }

  return true;
}
