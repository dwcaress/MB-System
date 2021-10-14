#ifndef DISPLAYPROPERTIES_H
#define DISPLAYPROPERTIES_H

namespace mb_system {
  /**
     DisplayProperties describe surface display options, and are copied between
     QVtkItem and QVtkRenderer
  */
  struct DisplayProperties {
    /// Draw axes
    bool showAxes;
  };

}


#endif
