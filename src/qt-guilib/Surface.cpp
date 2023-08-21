#include <math.h>
#include <limits>

#if defined(__APPLE__)
# include <OpenGL/gl.h>
#else
# include <GL/g.h>
#endif

#include "Surface.h"

using namespace mb_system;

Surface::Surface() {
}


void Surface::center(float *x, float *y, float *z) {
  *x = (xMax_ + xMin_) / 2.;
  *y = (yMax_ + yMin_) / 2.;
  *z = (zMax_ + zMin_) / 2.;    
}


void Surface::spans(float *xSpan, float *ySpan, float *zSpan) {
  *xSpan = xMax_ - xMin_;
  *ySpan = yMax_ - yMin_;
  *zSpan = zMax_ - zMin_;
}


void Surface::initialize() {
  xMin_ = std::numeric_limits<float>::max();
  xMax_ = -std::numeric_limits<float>::max();
  yMin_ = std::numeric_limits<float>::max();
  yMax_ = -std::numeric_limits<float>::max();
  zMin_ = std::numeric_limits<float>::max();
  zMax_ = -std::numeric_limits<float>::max();
}
