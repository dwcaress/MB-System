#include <vtkColorTransferFunction.h>
#include <vtkLookupTable.h>
#include <array>
#include "TopoColorMap.h"

using namespace mb_system;

bool TopoColorMap::makeLUT(TopoColorMap::Scheme scheme,
                           vtkLookupTable *lut) {

  const float *red = nullptr, *green = nullptr, *blue = nullptr;
  int nColors = NTopoMapColors;  

  switch (scheme) {
    
  case TopoColorMap::Haxby:
    red = haxbyRed;
    green = haxbyGreen;
    blue = haxbyBlue;
    break;

  case TopoColorMap::BrightRainbow:
    red = brightRainbowRed;
    green = brightRainbowGreen;
    blue = brightRainbowBlue;
    break;

  case TopoColorMap::MutedRainbow:
    red = mutedRainbowRed;
    green = mutedRainbowGreen;
    blue = mutedRainbowBlue;
    break;

  case TopoColorMap::Grayscale:
    red = grayscaleRed;
    green = grayscaleGreen;
    blue = grayscaleBlue;
    break;    

  default:
    std::cerr << "Unsupported colormap scheme" << std::endl;
    return false;
  };

  
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
  if (!strcmp(name, "Haxby")) {
    return Scheme::Haxby;
  }
  else if (!strcmp(name, "BrightRainbow")) {
    return Scheme::BrightRainbow;
  }
  else if (!strcmp(name, "MutedRainbow")) {
    return Scheme::MutedRainbow;
  }
  else if (!strcmp(name, "Grayscale")) {
    return Scheme::Grayscale;
  }
  else {
    return Scheme::Unknown;
  }
}


