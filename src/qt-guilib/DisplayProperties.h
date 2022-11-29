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

    /// Acknowledge changes to display properties
    void ackChanged() {
      changed_ = false;
    }

    /// Indicate whether properties have changed
    void changed(bool val) {
      changed_ = val;
    }

    /// Return true if properties changed
    bool changed() {
      return changed_;
    }

    /// Set whether to show axes
    void showAxes(bool set) {
      showAxes_ = set;
    }

    /// Return whether to show axes
    bool showAxes() {
      return showAxes_;
    }

    /// Set vertical exaggeration value
    void verticalExagg(float val) {
      verticalExagg_ = val;
    }

    /// Return vertical exaggeration
    float verticalExagg() {
      return verticalExagg_;
    }

    /// Set topo colormap scheme
    void colorMapScheme(TopoColorMap::Scheme scheme) {
      topoColorMapScheme_ = scheme;
    }

    /// Return topo colormap scheme
    TopoColorMap::Scheme colorMapScheme() {
      return topoColorMapScheme_;
    }
    
    /// Set name of site file
    void siteFile(char *file) {
      siteFile_ = strdup(file);
    }
    
    /// Return name of selected site file
    const char *siteFile() {
      return (const char *)siteFile_;
    }


  protected:

    /// Set true when any member has changed value
    /// Set false to acknowledge change
    bool changed_;

    /// Draw axes
    bool showAxes_;

    /// Vertical exaggeration
    float verticalExagg_;

    /// Topo colormap
    TopoColorMap::Scheme topoColorMapScheme_;
    
    /// Site file
    char *siteFile_;
    
    /// List of site points
    std::vector<mb_system::Point3D *> *sitePoints_;
    
  };

}


#endif
