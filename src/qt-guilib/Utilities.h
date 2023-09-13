#ifndef UTILITIES_H
#define UTILITIES_H

#include <vtkLookupTable.h>

namespace mb_system {


  const int NMapColors = 11;

  /// Define Haxby colormap red components
  const float haxbyRed[NMapColors] =
    {0.950, 1.000, 1.000, 1.000, 0.941, 0.804, 0.541, 0.416, 0.196,
     0.157, 0.145};

  /// Define Haxby colormap green components
  const float haxbyGreen[NMapColors] =
   {0.950, 0.729, 0.631, 0.741, 0.925, 1.000, 0.925, 0.922, 0.745,
    0.498, 0.224};

  /// Define Haxby colormap blue components
  const float haxbyBlue[NMapColors] =
   {0.950, 0.522, 0.267, 0.341, 0.475, 0.635, 0.682, 1.000, 1.000,
    0.984, 0.686};

  /// Enumerate colormaps
  enum ColorMapScheme { BrewerDivergingSpectral=0, WhiteToBlue,
                        Hawaii, RedToBlue, Haxby };

  /// Return name of ColorMapScheme
  const char *colorMapSchemeName(ColorMapScheme scheme);

  
  /// Make lookup table
  void makeLookupTable(ColorMapScheme colorMap, vtkLookupTable *lut);

  /// Lock MB-System file
  bool mbLockFile(char *filename);
  
  /// Unlock MB-System file
  bool mbUnlockFile(char *filename);  


  bool projTestUtil(char *msg);
}



#endif



