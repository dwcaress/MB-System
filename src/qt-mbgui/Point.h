#ifndef POINT_H
#define POINT_H

#define X 0
#define Y 1
#define Z 2
#define W 3

/** Point3D contains x, y, and z data 
 */
class Point3D {

 public:
  Point3D() {}
  Point3D(float x, float y, float z) {
    m_data[X] = x;
    m_data[Y] = y;
    m_data[Z] = z;
  }

  /// Return x element
  float x() { return m_data[X]; }

  /// Return y element
  float y() { return m_data[Y]; }

  /// Return z element
  float z() { return m_data[Z]; }    
  
  /// Set x element
  void setX(float x) { m_data[X] = x; }

  /// Set y element
  void setY(float y) { m_data[Y] = y; }

  /// Set z element
  void setZ(float z) { m_data[Z] = z; }  

 protected:
  float m_data[3];
};


/** Point4D contains x, y, z, and w data
 */
class Point4D {
 public:
  Point4D() {}
  Point4D(float x, float y, float z, float w) {
    m_data[X] = x;
    m_data[Y] = y;
    m_data[Z] = z;
    m_data[W] = w;
  }

 protected:
  float m_data[4];
};


#endif // POINT_H
