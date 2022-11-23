#ifndef DISPLAYPROPERTIES_H
#define DISPLAYPROPERTIES_H
#include <vector>
#include "TopoColorMap.h"
#include "Point.h"

namespace mb_system {
  /**
     DisplayProperties describe surface display options, and are copied between
     QVtkItem and QVtkRenderer
  */
  class DisplayProperties {

  public:
    DisplayProperties();

    
    /// Software that changes any elements *MUST* set changed true!
    bool changed;
    
    /// Draw axes
    bool showAxes;

    /// Vertical exaggeration
    float verticalExagg;

    /// Topo colormap
    TopoColorMap::Scheme topoColorMapScheme;

    /// Site file
    char *siteFile;
    
    /// List of site points
    std::vector<mb_system::Point3D *> sitePoints;
    
  };

}


#endif
