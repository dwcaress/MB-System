#include <iostream>
#include <fstream>
#include "Route.h"

using namespace mb_system;

Route::Route() {
}


Route::~Route() {

  // Free vector elements
  for (int i = 0; i < points_.size(); i++) {
    RouteWaypoint *point =  points_[i];
    delete point;
  }

}


bool Route::appendPoint(RouteWaypoint *point) {
  points_.insert(points_.end(), point);
  return true;
}



bool Route::load(std::string filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Failed to open " << filename << std::endl;
    return false;
  }

  std::string line;
  while (getline(file, line)) {
    std::cout << line << '\n';
  }
  
  file.close();
  return true;

}
