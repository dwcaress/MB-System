#ifndef POINT_H
#define POINT_H

#define X_ 0
#define Y_ 1
#define Z_ 2
#define W_ 3

namespace mb_system {
  
  /** Point3D contains x, y, and z data 
   */
  class Point3D {

  public:
    Point3D() {}
    Point3D(float x, float y, float z) {
      data_[X_] = x;
      data_[Y_] = y;
      data_[Z_] = z;
    }

    /// Return x element
    float x() { return data_[X_]; }

    /// Return y element
    float y() { return data_[Y_]; }

    /// Return z element
    float z() { return data_[Z_]; }    
  
    /// Set x element
    void setX(float x) { data_[X_] = x; }

    /// Set y element
    void setY(float y) { data_[Y_] = y; }

    /// Set z element
    void setZ(float z) { data_[Z_] = z; }  

  protected:
    float data_[3];
  };


  /** Point4D contains x, y, z, and w data
   */
  class Point4D {
  public:
    Point4D() {}
    Point4D(float x, float y, float z, float w) {
      data_[X_] = x;
      data_[Y_] = y;
      data_[Z_] = z;
      data_[W_] = w;
    }

  protected:
    float data_[4];
  };

}

#endif // POINT_H
