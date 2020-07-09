#include <cmath>
#include <limits>
#include <QDebug>
#include "ColorMap.h"

bool ColorMap::initialize(const QList<QVector3D> &rgbScale) {

  m_rgbScale = rgbScale;
  
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
    int i = (int)(factor * (m_rgbScale.size() - 1));
    double ff = factor * (m_rgbScale.size() - 1) - i;
    *red = m_rgbScale[i].x() + ff * (m_rgbScale[i + 1].x() - m_rgbScale[i].x());
    *green = m_rgbScale[i].y() + ff * (m_rgbScale[i + 1].y() - m_rgbScale[i].y());
    *blue = m_rgbScale[i].z() + ff * (m_rgbScale[i + 1].z() - m_rgbScale[i].z());
  }

  return true;
}


