#include <vtkColorTransferFunction.h>
#include <vtkLookupTable.h>
#include <array>
#include "TopoColorMap.h"

using namespace mb_system;

const int TopoColorMap::NSchemes = 5;

/// Supported color schemes
const struct TopoColorMap::SchemeStruct colorScheme[TopoColorMap::NSchemes] =
  {"Haxby", TopoColorMap::Haxby,
   /// red
   {0.950, 1.000, 1.000, 1.000, 0.941, 0.804, 0.541, 0.416, 0.196,
    0.157, 0.145},
   /// green
   {0.950, 0.729, 0.631, 0.741, 0.925, 1.000, 0.925, 0.922, 0.745,
    0.498, 0.224},
   /// blue
   {0.950, 0.522, 0.267, 0.341, 0.475, 0.635, 0.682, 1.000, 1.000,
    0.984, 0.686},
		  
   "BrightRainbow", TopoColorMap::BrightRainbow,
   /// red
   {1.000, 1.000, 1.000, 1.000, 0.500, 0.000, 0.000, 0.000, 0.000,
    0.500, 1.000},
   /// green
   {0.000, 0.250, 0.500, 1.000, 1.000, 1.000, 1.000, 0.500, 0.000,
    0.000, 0.000},
   /// blue
   {0.000, 0.000, 0.000, 0.000, 0.000, 0.000, 1.000, 1.000, 1.000,
    1.000, 1.000},

   "MutedRainbow", TopoColorMap::MutedRainbow,
   /// red
   {0.784, 0.761, 0.702, 0.553, 0.353, 0.000, 0.000, 0.000, 0.000,
    0.353, 0.553},
   /// green
   {0.000, 0.192, 0.353, 0.553, 0.702, 0.784, 0.553, 0.353, 0.000,
    0.000, 0.000},
   /// blue
   {0.000, 0.000, 0.000, 0.000, 0.000, 0.000, 0.553, 0.702, 0.784,
    0.702, 0.553},
   
   "Grayscale", TopoColorMap::Grayscale,
   /// red
   {0.000, 0.100, 0.200, 0.300, 0.400, 0.500, 0.600, 0.700, 0.800,
     0.900, 1.000},
   /// green
   {0.000, 0.100, 0.200, 0.300, 0.400, 0.500, 0.600, 0.700, 0.800,
    0.900, 1.000},
   /// blue
   {0.000, 0.100, 0.200, 0.300, 0.400, 0.500, 0.600, 0.700, 0.800,
    0.900, 1.000},

   "FlatGrayscale", TopoColorMap::FlatGrayscale,
   /// red
   {0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500,
    0.500, 0.500},
   /// green
   {0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500,
    0.500, 0.500},
   /// blue
   {0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500, 0.500,
    0.500, 0.500}
   
  };

void TopoColorMap::schemeNames(std::vector<const char *> *names) {
  names->clear();
  for (int i = 0; i < NSchemes; i++) {
    names->push_back(colorScheme[i].name_);
  }
}



bool TopoColorMap::makeLUT(TopoColorMap::Scheme scheme,
                           vtkLookupTable *lut) {

  const float *red = nullptr, *green = nullptr, *blue = nullptr;
  int nColors = NSchemeColors;  

  bool found = false;
  for (int i = 0; i < TopoColorMap::NSchemes; i++) {
    if (scheme == colorScheme[i].scheme_) {
      red = colorScheme[i].red_;
      green = colorScheme[i].green_;
      blue = colorScheme[i].blue_;
      found = true;
    }
  }
  
  if (!found) {
    // Scheme not found
    std::cout << "Could not find scheme #" << scheme << "\n";
    return false;
  }

  vtkNew<vtkColorTransferFunction> ctf;
  for (int i = 0; i < nColors; i++) {
    // x ranges from 0. (i=0) to 1. (i=nColors-1)
    double x = (double )i / (double )(nColors - 1);
    int ind = nColors-1 - i; 
    ctf->AddRGBPoint(x, red[ind], green[ind], blue[ind]);
  }

  auto tableSize = 256;
  lut->SetNumberOfTableValues(tableSize);
  lut->Build();
  
  for (int i = 0; i < lut->GetNumberOfColors(); i++) {
    std::array<double, 3> rgb;
    ctf->GetColor(static_cast<double>(i) / lut->GetNumberOfColors(),
                  rgb.data());
    std::array<double, 4> rgbAlpha{0., 0., 0., 1.0};
    std::copy(std::begin(rgb), std::end(rgb), std::begin(rgbAlpha));
    lut->SetTableValue(static_cast<vtkIdType>(i), rgbAlpha.data());
  }
  return true;
}



TopoColorMap::Scheme TopoColorMap::schemeFromName(const char *name) {

  for (int i = 0; i < TopoColorMap::NSchemes; i++) {
    if (!strcmp(name, colorScheme[i].name_)) {
      return colorScheme[i].scheme_;
    }
  }

  return TopoColorMap::Scheme::Unknown;
}


  
 
