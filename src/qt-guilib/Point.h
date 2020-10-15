#ifndef POINT_H
#define POINT_H

#define X 0
#define Y 1
#define Z 2
#define W 3

namespace mb_system {
  
  /** Point3D contains x, y, and z data 
   */
  class Point3D {

  public:
    Point3D() {}
    Point3D(float x, float y, float z) {
      data_[X] = x;
      data_[Y] = y;
      data_[Z] = z;
    }

    /// Return x element
    float x() { return data_[X]; }

    /// Return y element
    float y() { return data_[Y]; }

    /// Return z element
    float z() { return data_[Z]; }    
  
    /// Set x element
    void setX(float x) { data_[X] = x; }

    /// Set y element
    void setY(float y) { data_[Y] = y; }

    /// Set z element
    void setZ(float z) { data_[Z] = z; }  

  protected:
    float data_[3];
  };


  /** Point4D contains x, y, z, and w data
   */
  class Point4D {
  public:
    Point4D() {}
    Point4D(float x, float y, float z, float w) {
      data_[X] = x;
      data_[Y] = y;
      data_[Z] = z;
      data_[W] = w;
    }

  protected:
    float data_[4];
  };

}

#endif // POINT_H
