#ifndef GMTGRIDSURFACE_H
#define GMTGRIDSURFACE_H
#include <GL/gl.h>
#include <gmt/gmt.h>
#include "Surface.h"
#include "ColorMap.h"

namespace mb_system {
  /** 
      GMTGridSurface represents a 3D surface based on contents of a MB-System 
      GMT grid file
  */
  class GmtGridSurface : public Surface {

  public:

    GmtGridSurface();

    virtual ~GmtGridSurface();
  
    /// Build surface from GMT grid file
    virtual bool build(const char *gridFile);
  
    /// Read grid from GMT file; return pointer to GMT_GRID if
    /// successful, else return nullptr
    static GMT_GRID *readGridFile(const char *filename, void **gmtApi);  

    /// Set red, green, blue based on z-value and z range
    void setColor(float z, float zmin, float zrange,
		  float *red, float *green, float *blue);
  protected:

    /// Set vertex, color, and normal vector data from GMT_GRID data
    void setData(void *gmtApi, GMT_GRID *gmtGrid);

    /// Return index for col, row 
    GLuint vertexIndex(int col, int row, int nColumns) {
      return (GLuint)( row * nColumns + col );
    }

    ColorMap *m_colorMap;
  };
}

#endif // GMTGRIDSURFACE_H
