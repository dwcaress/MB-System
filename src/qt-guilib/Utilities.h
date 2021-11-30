#ifndef UTILITIES_H
#define UTILITIES_H

#include <vtkLookupTable.h>


namespace mb_system {

  const int NMapColors = 11;

  const float haxbyRed[NMapColors] =
    {0.950, 1.000, 1.000, 1.000, 0.941, 0.804, 0.541, 0.416, 0.196,
     0.157, 0.145};

  const float haxbyGreen[NMapColors] =
   {0.950, 0.729, 0.631, 0.741, 0.925, 1.000, 0.925, 0.922, 0.745,
    0.498, 0.224};
  
  const float haxbyBlue[NMapColors] =
   {0.950, 0.522, 0.267, 0.341, 0.475, 0.635, 0.682, 1.000, 1.000,
    0.984, 0.686};
  
  enum ColorMapScheme { BrewerDivergingSpectral, WhiteToBlue,
                        Hawaii, RedToBlue, Haxby };

  
  /// Make lookup table
  void makeLookupTable(ColorMapScheme colorMap, vtkLookupTable *lut);

}



#endif



