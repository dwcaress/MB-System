#include <string.h>
#include <iostream>
#include <fstream>

#include "Route.h"

#define ROUTENAME_COMMENT "## ROUTENAME"
#define ROUTEFILE_VERSION_COMMENT "## Route File Version"
#define ROUTECOLOR_COMMENT "## ROUTECOLOR"
#define ROUTESIZE_COMMENT "## ROUTESIZE"
#define ROUTEEDITMODE_COMMENT "## ROUTEEDITMODE"
#define STARTROUTE_DELIMITER "> ## STARTROUTE"
#define ENDROUTE_DELIMITER "> ## ENDROUTE"

using namespace mb_system;

Route::Route() {
  name_ = "No name";
  color_ = 0;
}


Route::~Route() {

}


std::vector<Route *> *Route::load(std::string filename) {

  std::cout << "In Route::load()\n";
  
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Failed to open " << filename << std::endl;
    return nullptr;
  }

  bool rawRoutefile = true;
  char routename[4096];     // What is upper limit on path name???
  int routeColor = 0;
  int routeSize = 0;
  int routeEditMode = 0;
  int nPoint = 0;
  bool pointOK = false;

  std::vector<Route *> *routes = new std::vector<Route *>();
  Route *route = nullptr;
  
  std::string line;
  while (getline(file, line)) {
    // Trim newline, blanks 
    line.erase(line.find_last_not_of(" \n\r\t")+1);
    std::cout << line << '\n';
    const char *buffer = line.data();

    if (!strncmp(buffer, STARTROUTE_DELIMITER, strlen(STARTROUTE_DELIMITER))) {

      // Create the new route and append to the routes vector
      route = new Route();
      routes->insert(routes->end(), route);
    }
      
    // deal with comments
    if (buffer[0] == '#') {
      if (rawRoutefile && strncmp(buffer, ROUTEFILE_VERSION_COMMENT,
				  strlen(ROUTEFILE_VERSION_COMMENT)) == 0) {
	rawRoutefile = false;
      }
      else if (!strncmp(buffer, ROUTENAME_COMMENT, strlen(ROUTENAME_COMMENT))) {
	strcpy(routename, &buffer[strlen(ROUTENAME_COMMENT) + 1]);
      }
      else if (!strncmp(buffer, ROUTECOLOR_COMMENT,
			strlen(ROUTECOLOR_COMMENT))) {
	routeColor = atoi((const char *)&buffer[strlen(ROUTECOLOR_COMMENT)+1]);
      }
      else if (!strncmp(buffer, ROUTESIZE_COMMENT, strlen(ROUTESIZE_COMMENT))) {
	routeSize = atoi((const char *)&buffer[strlen(ROUTESIZE_COMMENT)+1]);
      }
      else if (!strncmp(buffer, ROUTEEDITMODE_COMMENT,
			strlen(ROUTEEDITMODE_COMMENT))) {
	routeEditMode =
	  atoi((const char *)&buffer[strlen(ROUTEEDITMODE_COMMENT)+1]);
      }
    }

    // Deal with route end
    if (!strncmp(buffer, ENDROUTE_DELIMITER, strlen(ENDROUTE_DELIMITER))) {    
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
	DataPointsOverlay::Point *point = new DataPointsOverlay::Point();
	point->easting = lon;
	point->northing = lat;
	point->elevation = elev;
	point->type = (Route::PointType )waypointType;
	  
	route->appendPoint(point);
	
	nPoint++;
      }
    }
  }

  file.close();

  // Return routes vector pointer
  return routes;
}
