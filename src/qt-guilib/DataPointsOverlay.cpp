#include <string.h>
#include <iostream>
#include <fstream>

#include "DataPointsOverlay.h"

using namespace mb_system;

DataPointsOverlay::DataPointsOverlay() {
}


DataPointsOverlay::~DataPointsOverlay() {

  // Free vector elements
  for (int i = 0; i < points_.size(); i++) {
    DataPointsOverlay::Point *point =  points_[i];
    delete point;
  }

}


bool DataPointsOverlay::appendPoint(DataPointsOverlay::Point *point) {
  points_.insert(points_.end(), point);
  return true;
}


std::vector<DataPointsOverlay *> *load(std::string fileName) {
}




