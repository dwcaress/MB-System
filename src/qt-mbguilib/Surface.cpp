#include <math.h>
#include <GL/gl.h>
#include "Surface.h"

using namespace mb_system;

Surface::Surface() {
}


void Surface::center(float *x, float *y, float *z) {
  *x = (m_xMax + m_xMin) / 2.;
  *y = (m_yMax + m_yMin) / 2.;
  *z = (m_zMax + m_zMin) / 2.;    
}


void Surface::spans(float *xSpan, float *ySpan, float *zSpan) {
  *xSpan = m_xMax - m_xMin;
  *ySpan = m_yMax - m_yMin;
  *zSpan = m_zMax - m_zMin;
}


void Surface::initialize() {
  m_xMin = std::numeric_limits<float>::max();
  m_xMax = -std::numeric_limits<float>::max();
  m_yMin = std::numeric_limits<float>::max();
  m_yMax = -std::numeric_limits<float>::max();
  m_zMin = std::numeric_limits<float>::max();
  m_zMax = -std::numeric_limits<float>::max();
}
