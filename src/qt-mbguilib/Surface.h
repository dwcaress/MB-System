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
    std::vector<Vertex> vertices() { return m_vertices; }

    /// Return normals to each triangle
    std::vector<Point3D> normals() { return m_normals; }

    /// Return triangle-drawing indices
    std::vector<unsigned int> drawingIndices() { return m_indices; }

    /// Return "center" of surface in world coordinates
    void center(float *x, float *y, float *z);

    /// Return x, y, and z spans of surface
    void spans(float *xSpan, float *ySpan, float *zSpan);

    /// Return span of x values
    float xSpan(float *xMin, float *xMax) {
      // qDebug() << "Surface::xSpan(): " << m_xMax - m_xMin;
      *xMin = m_xMin;
      *xMax = m_xMax;
      return (m_xMax - m_xMin);
    }

    /// Return span of y values  
    float ySpan(float *yMin, float *yMax) {
      // qDebug() << "Surface::ySpan(): " << m_yMax - m_yMin;    
      *yMin = m_yMin;
      *yMax = m_yMax;
      return (m_yMax - m_yMin);
    }

    /// Return span of z values  
    float zSpan(float *zMin, float *zMax) {
      // qDebug() << "Surface::zSpan(): " << m_zMax - m_zMin;        
      *zMin = m_zMin;
      *zMax = m_zMax;
      return (m_zMax - m_zMin);
    }
  
  
    /// Generate vertices, normals, indices
    virtual bool build(const char *filename = nullptr) = 0;
  
  protected:

    void initialize();
  
    /// Surface points
    std::vector<Vertex> m_vertices;

    /// Normals to surface points
    std::vector<Point3D> m_normals;

    /// Triangle drawing indices
    std::vector<unsigned int> m_indices;


    // Surface extents
    float m_xMin;
    float m_xMax;
    float m_yMin;
    float m_yMax;
    float m_zMin;
    float m_zMax;
  

  };
}

#endif // SURFACE_H
