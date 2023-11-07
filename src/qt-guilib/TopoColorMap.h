#ifndef TOPOCOLORMAP_H
#define TOPOCOLORMAP_H
#include <vtkLookupTable.h>

/// All colors schemes have 11 colors
#define NSchemeColors 11

namespace mb_system {

  /** 
      Color maps / LUTs for topographic grid display
  */
  class TopoColorMap {

  public:
    
    /// Supported colormap schemes
    enum Scheme { Unknown = 0,
                  Haxby,
                  BrightRainbow,
                  MutedRainbow,
                  Grayscale };

    /// Number of defined map schemes
    static const int NSchemes;

    /// Make vtkLookupTable for specified color scheme
    /// Return true on success, otherwise return false
    static bool makeLUT(Scheme scheme, vtkLookupTable *lut);

    /// Get Scheme from colorMap name, returns Scheme::Unknown if
    /// invalid/unsupported scheme
    static Scheme schemeFromName(const char *schemeName);

    /// Fill vector with supported scheme names
    static void schemeNames(std::vector<const char *> *names);
    
    struct SchemeStruct {
      const char *name_;
      Scheme scheme_;
      const float red_[NSchemeColors];
      const float green_[NSchemeColors];
      const float blue_[NSchemeColors];
    };
  };
}



#endif
