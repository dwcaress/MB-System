#ifndef TopoData_h
#define TopoData_h

namespace mb_system {

  
  
  /**
     TopoData abstract base class defines interface to 3D data 
     such as GMT grid data or swath data. 
  */
  class TopoData {

  public:

    /* ***
    /// Supported projections
    enum MapProjection {Unknown,
                        Geographic,
                        UTM};
                        *** */

    /// Constructor
    TopoData();
    
    /// "No data" value
    static const double NoData;

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
    virtual bool data(int row, int col, double *x, double *y,
                      double *z) = 0;
  
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

    /// Return proj-string corresponding to data's CRS, suitable for
    /// use with PROJ C/C++ API
    const char *projString() {
      return (const char *)projString_;
    }
    
    /// Set projString_ member to a valid proj-string corresponding
    /// to data's coordinate reference system.
    /// Returns true on success, false on error
    virtual bool setProjString() = 0;

  protected:
  
    /// Must be implemented by subclasses
    /// Base class calls this to set class parameter variables
    /// Note: xUnits, yUnits and zUnits memory should be unallocated
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

    /// proj-string describing map's CRS
    char projString_[100];


    /// Projection types specified in mb-system .grd and swath files
    static const char *GeographicType_;
    static const char *UtmType_;
    
  private:


  };

};

#endif
