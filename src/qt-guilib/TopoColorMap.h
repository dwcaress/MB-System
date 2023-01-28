#ifndef TOPOCOLORMAP_H
#define TOPOCOLORMAP_H
#include <vtkLookupTable.h>

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

    /// Make vtkLookupTable for specified color scheme
    /// Return true on success, otherwise return false
    static bool makeLUT(Scheme scheme, vtkLookupTable *lut);

    /// Get Scheme from colorMap name, returns Scheme::Unknown if
    /// invalid/unsupported scheme
    static Scheme schemeFromName(const char *schemeName);
    
  };

  /// All schemes use 11 colors
  const int NTopoMapColors = 11;
  
  /// Haxby red values
  const float haxbyRed[NTopoMapColors] =
    {0.950, 1.000, 1.000, 1.000, 0.941, 0.804, 0.541, 0.416, 0.196,
     0.157, 0.145};

  /// Haxby green values  
  const float haxbyGreen[NTopoMapColors] =
   {0.950, 0.729, 0.631, 0.741, 0.925, 1.000, 0.925, 0.922, 0.745,
    0.498, 0.224};

  /// Haxby blue values
  const float haxbyBlue[NTopoMapColors] =
   {0.950, 0.522, 0.267, 0.341, 0.475, 0.635, 0.682, 1.000, 1.000,
    0.984, 0.686};


  const float brightRainbowRed[NTopoMapColors] =
    {1.000, 1.000, 1.000, 1.000, 0.500, 0.000, 0.000, 0.000, 0.000,
     0.500, 1.000};
  
  const float brightRainbowGreen[NTopoMapColors] =
    {0.000, 0.250, 0.500, 1.000, 1.000, 1.000, 1.000, 0.500, 0.000,
     0.000, 0.000};
  
  const float brightRainbowBlue[NTopoMapColors] =
    {0.000, 0.000, 0.000, 0.000, 0.000, 0.000, 1.000, 1.000, 1.000,
     1.000, 1.000};


  const float mutedRainbowRed[NTopoMapColors] =
    {0.784, 0.761, 0.702, 0.553, 0.353, 0.000, 0.000, 0.000, 0.000,
     0.353, 0.553};
  
  const float mutedRainbowGreen[NTopoMapColors] =
    {0.000, 0.192, 0.353, 0.553, 0.702, 0.784, 0.553, 0.353, 0.000,
     0.000, 0.000};
  
  const float mutedRainbowBlue[NTopoMapColors] =
    {0.000, 0.000, 0.000, 0.000, 0.000, 0.000, 0.553, 0.702, 0.784,
     0.702, 0.553};


  const float grayscaleRed[NTopoMapColors] =
    {0.000, 0.100, 0.200, 0.300, 0.400, 0.500, 0.600, 0.700, 0.800,
     0.900, 1.000};
  
  const float grayscaleGreen[NTopoMapColors] =
    {0.000, 0.100, 0.200, 0.300, 0.400, 0.500, 0.600, 0.700, 0.800,
     0.900, 1.000};
  
  const float grayscaleBlue[NTopoMapColors] =
    {0.000, 0.100, 0.200, 0.300, 0.400, 0.500, 0.600, 0.700, 0.800,
     0.900, 1.000};  
}



#endif
