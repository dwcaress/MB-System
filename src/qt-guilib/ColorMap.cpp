#include <cmath>
#include <limits>
#include <QDebug>
#include "ColorMap.h"

using namespace mb_system;

bool ColorMap::initialize(const QList<QVector3D> &rgbScale) {

  rgbScale_ = rgbScale;
  
  return true;
}


bool ColorMap::rgbValues(float zValue, float zMin, float zMax,
			 float *red, float *green, float *blue) {

  double factor = (zMax - zValue) / (zMax - zMin);

  if (factor <= 0.0) {
    *red = 0.;
    *green = 0.;
    *blue = 0.;
  }
  else if (factor >= 1.0) {
    *red = 1.0;
    *green = 1.0;
    *blue = 1.0;
  }
  else {
    int i = (int)(factor * (rgbScale_.size() - 1));
    double ff = factor * (rgbScale_.size() - 1) - i;
    *red = rgbScale_[i].x() + ff * (rgbScale_[i + 1].x() - rgbScale_[i].x());
    *green = rgbScale_[i].y() + ff * (rgbScale_[i + 1].y() - rgbScale_[i].y());
    *blue = rgbScale_[i].z() + ff * (rgbScale_[i + 1].z() - rgbScale_[i].z());
  }

  return true;
}


