
#ifndef ROUTE_H
#define ROUTE_H

#include <vector>
#include <string>
#include "DataPointsOverlay.h"

namespace mb_system {

  /***
      Define a route, which consists of individual points. Some of the
      points were specified by the user, others are interpolated between
      user-specified points.
   */
  class Route : public mb_system::DataPointsOverlay {

  public:

    /// Create an empty Route
    Route();
    
    /// Destructor
    virtual ~Route();

    /// Return vector of routes defined in specified file, else nullptr
    /// if file open error or no routes defined in file.
    static std::vector<Route *> *load(std::string routefileName);

    /// Set route name
    void setName(char *name) {
      name_.assign(name);
    }

    /// Set route color (mb-system scheme)
    void setColor(int color) {
      color_ = color;
    }

    
  protected:

    /// Route name
    std::string name_;

    /// Route render color (mb-system color scheme)
    int color_;
        
  };

};



#endif

