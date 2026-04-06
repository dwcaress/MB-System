
#ifndef DATAPOINTSOVERLAY_H
#define DATAPOINTSOVERLAY_H

#include <vector>
#include <string>

#define ROUTEFILE_EXTENSION "rte"
#define SITEFILE_EXTENSION "ste"

namespace mb_system {

  /***
      DataPointsOverlay consists of individual points that can be overlaid on
      a map. Subclasses include SitePoints, Route.
   */
  class DataPointsOverlay {

  public:

    /// Constructor
    DataPointsOverlay();
    
    /// Destructor
    virtual ~DataPointsOverlay();

    /** Route is defined by series of points, each
	point classified by PointType.
    */
    enum PointType {Interpolated=0,
		    UserSpecified,
		    Transit,
		    StartSurvey,
		    EndSurvey,
		    StartSurvey2,
		    EndSurvey2,
		    StartSurvey3,
		    EndSurvey3,
		    StartSurvey4,
		    EndSurvey4,
		    StartSurvey5,
		    EndSurvey5
    };


    /// Overlay consists of individual points
    struct Point {
      double easting;
      double northing;
      double elevation;
      PointType type;
    };

    /// Append a point; returns true if no error, else false
    bool appendPoint(DataPointsOverlay::Point *point);


    /// Return vector of DataPointsOverlay object pointers defined
    /// in file with specified fileName
    static std::vector<DataPointsOverlay *> *loadFile(std::string fileName);
    

  protected:

    std::vector<DataPointsOverlay::Point *> points_;

  };

};



#endif

