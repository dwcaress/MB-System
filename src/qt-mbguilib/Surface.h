#ifndef SURFACE_H
#define SURFACE_H

#include <vector>
// #include <QVector>
// #include <QVector3D>
#include "Vertex.h"

namespace mb_system {
  /**
     Surface represents a surface as vertices, normals,
     and triangle-drawing indices.
  */
  class Surface {

  public:

    Surface();

    virtual ~Surface() { }
  
    /// Return vertex positions
    std::vector<Vertex> vertices() { return vertices_; }

    /// Return normals to each triangle
    std::vector<Point3D> normals() { return normals_; }

    /// Return triangle-drawing indices
    std::vector<unsigned int> drawingIndices() { return indices_; }

    /// Return "center" of surface in world coordinates
    void center(float *x, float *y, float *z);

    /// Return x, y, and z spans of surface
    void spans(float *xSpan, float *ySpan, float *zSpan);

    /// Return span of x values
    float xSpan(float *xMin, float *xMax) {
      // qDebug() << "Surface::xSpan(): " << xMax_ - xMin_;
      *xMin = xMin_;
      *xMax = xMax_;
      return (xMax_ - xMin_);
    }

    /// Return span of y values  
    float ySpan(float *yMin, float *yMax) {
      // qDebug() << "Surface::ySpan(): " << yMax_ - yMin_;    
      *yMin = yMin_;
      *yMax = yMax_;
      return (yMax_ - yMin_);
    }

    /// Return span of z values  
    float zSpan(float *zMin, float *zMax) {
      // qDebug() << "Surface::zSpan(): " << zMax_ - zMin_;        
      *zMin = zMin_;
      *zMax = zMax_;
      return (zMax_ - zMin_);
    }
  
  
    /// Generate vertices, normals, indices
    virtual bool build(const char *filename = nullptr) = 0;
  
  protected:

    void initialize();
  
    /// Surface points
    std::vector<Vertex> vertices_;

    /// Normals to surface points
    std::vector<Point3D> normals_;

    /// Triangle drawing indices
    std::vector<unsigned int> indices_;


    // Surface extents
    float xMin_;
    float xMax_;
    float yMin_;
    float yMax_;
    float zMin_;
    float zMax_;
  

  };
}

#endif // SURFACE_H
