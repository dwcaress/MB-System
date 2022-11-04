#ifndef DISPLAYPROPERTIES_H
#define DISPLAYPROPERTIES_H
#include "TopoColorMap.h"

namespace mb_system {
  /**
     DisplayProperties describe surface display options, and are copied between
     QVtkItem and QVtkRenderer
  */
  struct DisplayProperties {
    /// Draw axes
    bool showAxes;

    /// Vertical exaggeration
    float verticalExagg;

    /// Topo colormap
    TopoColorMap::Scheme topoColorMapScheme;
    
    
  };

}


#endif
