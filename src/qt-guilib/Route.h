
#ifndef ROUTE_H
#define ROUTE_H

#include <vector>
#include <string>

namespace mb_system {

  /***
      Define a route, which consists of individual points. Some of the
      points were specified by the user, others are interpolated between
      user-specified points.
   */
  class Route {

  public:

    /// Create an empty Route
    Route();
    
    /// Destructor
    ~Route();

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



    
    /// Route consists of waypoints
    struct Waypoint {
      double easting;
      double northing;
      double elevation;
      PointType type;
    };

    /// Append a waypoint; returns true if no error, else false
    bool appendPoint(Route::Waypoint *point);

    /// Load route from a route file
    bool load(std::string routefileName);



  protected:
    std::vector<Route::Waypoint *> points_;

  };

};



#endif

