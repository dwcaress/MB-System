#ifndef COLORMAP_H
#define COLORMAP_H

#include <QList>
#include <QVector3D>

namespace mb_system {
  /**
     ColorMap maps data value to red, green and blue values according to a provided color scale, using
     continuous linear interpolation.
  */
  class ColorMap {

  public:

    /// Initialize with color scale
    bool initialize(const QList<QVector3D> &rgbScale);

    /// Get red, green, and blue values (in range 0-1) corresponding to zValue
    bool rgbValues(float zValue, float zMin, float zMax,
		   float *red, float *green, float *blue);

  protected:
  
    QList<QVector3D> rgbScale_;
  
  };

}

#endif // COLORGRADIENT_H
