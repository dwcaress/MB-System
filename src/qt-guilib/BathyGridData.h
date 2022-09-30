#ifndef BathyGridData_h
#define BathyGridData_h

/* **
GridData defines interface to 3D gridded data such as GMT grid data or 
swath data. 
 ** */
class BathyGridData {

public:

  /// Read grid data from file
  virtual bool readDatafile(char *filename) = 0;
  
  /// Number of grid rows
  virtual int nRows() {
    return nRows_;
  }

  /// Number of grid columns
  virtual int nColumns() {
    return nColumns_;
  }
  
  /// Get x, y, z data at specified row and column.
  /// Returns false in case of error
  virtual bool data(int row, int col, double *x, double *y, double *z) = 0;
  
  /// Get min/max bounds on each axis
  void bounds(double *xMin, double *xMax,
              double *yMin, double *yMax,
              double *zMin, double *zMax) {
    *xMin = xMin_;
    *xMax = xMax_;
    *yMin = yMin_;
    *yMax = yMax_;
    *zMin = zMin_;
    *zMax = zMax_;
  }

  /// Get units on axes
  virtual void units(char **xUnits, char **yUnits, char **zUnits) {
    *xUnits = xUnits_;
    *yUnits = yUnits_;
    *zUnits = zUnits_;
  }

  /// Set parameter member variables
  virtual void setParameters() final {
    
    // Call subclass-implemented function to get member variable values
    getParameters(&nRows_, &nColumns_,
                  &xMin_, &xMax_, &yMin_, &yMax_, &zMin_, &zMax_,
                  &xUnits_, &yUnits_, &zUnits_);
  }
  
protected:
  
  /// Must be implemented by subclasses
  /// Base class calls this to set class parameter variables
  /// N.B.: xUnits, yUnits and zUnits memory should be unallocated
  /// before calling this funtion.
  virtual void getParameters(int *nRows, int *nColumns,
                             double *xMin, double *xMax,
                             double *yMin, double *yMax,
                             double *zMin, double *zMax,
                             char **xUnits, char **yUnits, char **zUnits) = 0;

  /// Grid rows and columns
  int nRows_, nColumns_;
    
  /// Grid data bounds
  double xMin_, xMax_, yMin_, yMax_, zMin_, zMax_;

  /// Grid data units
  char *xUnits_, *yUnits_, *zUnits_;
  
private:


};


#endif
