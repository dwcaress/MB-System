#include <limits>
#include <vector>
#include <QFont>
#include "PixmapDrawer.h"
#include "TopoProfileItem.h"

#define DEFAULT_PLOT_WIDTH 700
#define DEFAULT_PLOT_HEIGHT 300

using namespace std;
using namespace mb_system;

TopoProfileItem::TopoProfileItem(QObject *uiRoot, const char *name) {

  verticalExagg_ = 1.;
  
  if (!initialize(uiRoot, name)) {
    exit(1);
  }
}


  
bool TopoProfileItem::initialize(QObject *uiRoot, const char *itemName) {

  return true;
}


bool TopoProfileItem::draw(vector<array<double, 2>> *profile) {

  float xMin = numeric_limits<float>::max();
  float xMax = -numeric_limits<float>::max();
  float yMin = numeric_limits<float>::max();
  float yMax = -numeric_limits<float>::max();  
  for (int i = 0; i < profile->size(); i++) {
    // Find x and y limits
    array<double, 2> point = profile->at(0);
    if (point[0] < xMin) {
      xMin = point[0];
    }
    if (point[0] > xMax) {
      xMax = point[0];
    }
    if (point[1] < yMin) {
      yMin = point[1];
    }
    if (point[1] > yMax) {
      yMax = point[1];
    }
  }
  
}

