#include <string.h>
#include <iostream>
#include <fstream>

#include "Route.h"

using namespace mb_system;

Route::Route() {
}


Route::~Route() {

  // Free vector elements
  for (int i = 0; i < points_.size(); i++) {
    Route::Waypoint *point =  points_[i];
    delete point;
  }

}


bool Route::appendPoint(Route::Waypoint *point) {
  points_.insert(points_.end(), point);
  return true;
}



bool Route::load(std::string filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Failed to open " << filename << std::endl;
    return false;
  }

  bool rawRoutefile = true;
  char routename[4096];     // What is upper limit on path name???
  int routeColor = 0;
  int routeSize = 0;
  int routeEditMode = 0;
  int nPoint = 0;
  bool pointOK = false;
  
  std::string line;
  while (getline(file, line)) {
    std::cout << line << '\n';
    const char *buffer = line.data();

    // deal with comments
    if (buffer[0] == '#') {
      if (rawRoutefile && strncmp(buffer, "## Route File Version", 21) == 0) {
	rawRoutefile = false;
      }
      else if (strncmp(buffer, "## ROUTENAME", 12) == 0) {
	strcpy(routename, &buffer[13]);
	if (routename[strlen(routename) - 1] == '\n')
	  routename[strlen(routename) - 1] = '\0';
	if (routename[strlen(routename) - 1] == '\r')
	  routename[strlen(routename) - 1] = '\0';
      }
      else if (strncmp(buffer, "## ROUTECOLOR", 13) == 0) {
	sscanf(buffer, "## ROUTECOLOR %d", &routeColor);
      }
      else if (strncmp(buffer, "## ROUTESIZE", 12) == 0) {
	sscanf(buffer, "## ROUTESIZE %d", &routeSize);
      }
      else if (strncmp(buffer, "## ROUTEEDITMODE", 16) == 0) {
	sscanf(buffer, "## ROUTEEDITMODE %d", &routeEditMode);
      }
    }

    // deal with route segment marker
    else if (buffer[0] == '>') {
      // if data accumulated, add the route
      if (nPoint > 0) {
	std::cout << "TBD: Add the route\n";
      }
    }

    else {
      // read the route waypoint data from the buffer
      int waypointType;
      double lon, lat, elev;
      int nget = sscanf(buffer, "%lf %lf %lf %d", &lon, &lat, &elev,
			&waypointType);
      if ((rawRoutefile && nget >= 2) ||
	  (!rawRoutefile && nget >= 3 &&
	   waypointType > PointType::Interpolated))
	pointOK = true;
      else
	pointOK = false;

      // add good point to route
      if (pointOK) {
	Route::Waypoint *point = new Route::Waypoint();
	point->easting = lon;
	point->northing = lat;
	point->elevation = elev;
	point->type = (Route::PointType )waypointType;
	  
	appendPoint(point);
	
	nPoint++;
      }
    }
  }

  file.close();
  return true;
  

}
