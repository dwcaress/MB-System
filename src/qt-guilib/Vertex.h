#ifndef VERTEX_H
#define VERTEX_H
 
// #include <QVector3D>
// #include <QVector4D>
#include <QtGlobal>
#include "Point.h"

namespace mb_system {
  
  /**
     Vertex defines a point's position and color.
     Adapted from Trent Reed's code at this link:
     https://www.trentreed.net/blog/qt5-opengl-part-1-basic-rendering/
  */ 
  class Vertex
  {
  public:
    // Constructors
    Vertex();
    explicit Vertex(const Point3D &position);
    Vertex(const Point3D &position, const Point4D &color);
 
    // Accessors / Mutators
    const Point3D& position() const;
    const Point4D& color() const;
    void setPosition(const Point3D& position);
    void setColor(const Point4D& color);
 
    // OpenGL attribute parameters
    static const int PositionTupleSize = 3;
    static const int ColorTupleSize = 4;
    static int positionOffset();
    static int colorOffset();
    static int stride();
 
  private:
    /// x, y, z
    Point3D position_;

    /// R, G, B, A
    Point4D color_;
  };

  /*******************************************************************************
   * Inline Implementation
   ******************************************************************************/
 
  // Note: Q_MOVABLE_TYPE means it can be memcpy'd.
  //  Q_DECLARE_TYPEINFO(Vertex, Q_MOVABLE_TYPE);
 
  // Constructors
  inline Vertex::Vertex() {}

  inline Vertex::Vertex(const Point3D &position)
    : position_(position) {}

  inline Vertex::Vertex(const Point3D &position,
			const Point4D &color)
    : position_(position), color_(color) {}
 
  // Accessors / Mutators
  inline const Point3D& Vertex::position()
    const { return position_; }

  inline const Point4D& Vertex::color()
    const { return color_; }

  void inline Vertex::setPosition(const Point3D& position) {
    position_ = position; }

  void inline Vertex::setColor(const Point4D& color) { color_ = color; }
 
  // OpenGL Helpers
  inline int Vertex::positionOffset() {
    return offsetof(Vertex, position_);
  }

  inline int Vertex::colorOffset() {
    return offsetof(Vertex, color_);
  }

  inline int Vertex::stride() { return sizeof(Vertex); }
}

#endif // VERTEX_H
