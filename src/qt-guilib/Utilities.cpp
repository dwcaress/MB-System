#include <vtkColorSeries.h>
#include <vtkNamedColors.h>
#include "Utilities.h"

void mb_system::makeLookupTable(mb_system::ColorMapScheme colorScheme,
                                vtkLookupTable* lut)
{
  vtkSmartPointer<vtkNamedColors> colors =
    vtkSmartPointer<vtkNamedColors>::New();
  
  // Select a color scheme.
  switch (colorScheme)  {
  case BrewerDivergingSpectral:
  default:  {
    // Make the lookup using a Brewer palette.
    vtkSmartPointer<vtkColorSeries> colorSeries =
      vtkSmartPointer<vtkColorSeries>::New();
    colorSeries->SetNumberOfColors(11);
    int colorSeriesEnum = colorSeries->BREWER_DIVERGING_SPECTRAL_11;
    colorSeries->SetColorScheme(colorSeriesEnum);
    colorSeries->BuildLookupTable(lut, colorSeries->ORDINAL);
    lut->SetNanColor(1, 0, 0, 1);
    break;
  }
  case WhiteToBlue: {
    // A lookup table of 256 colours ranging from
    //  deep blue(water) to yellow - white(mountain top)
    //  is used to color map this figure.
    lut->SetHueRange(0.7, 0);
    lut->SetSaturationRange(1.0, 0);
    lut->SetValueRange(0.5, 1.0);
    break;
  }
  case Hawaii: {
    // Make the lookup table with a preset number of colours.
    vtkSmartPointer<vtkColorSeries> colorSeries =
      vtkSmartPointer<vtkColorSeries>::New();
    colorSeries->SetNumberOfColors(8);
    colorSeries->SetColorSchemeName("Hawaii");
    colorSeries->SetColor(0, colors->GetColor3ub("turquoise_blue"));
    colorSeries->SetColor(1, colors->GetColor3ub("sea_green_medium"));
    colorSeries->SetColor(2, colors->GetColor3ub("sap_green"));
    colorSeries->SetColor(3, colors->GetColor3ub("green_dark"));
    colorSeries->SetColor(4, colors->GetColor3ub("tan"));
    colorSeries->SetColor(5, colors->GetColor3ub("beige"));
    colorSeries->SetColor(6, colors->GetColor3ub("light_beige"));
    colorSeries->SetColor(7, colors->GetColor3ub("bisque"));
    colorSeries->BuildLookupTable(lut, colorSeries->ORDINAL);
    lut->SetNanColor(1, 0, 0, 1);
    break;
  }
  case RedToBlue: {
    // A lookup table of 256 colours ranging from
    //  deep blue(water) to yellow - white(mountain top)
    //  is used to color map this figure.
    lut->SetHueRange(0.7, 0.06);
    lut->SetSaturationRange(1.0, 0.78);
    lut->SetValueRange(0.5, 0.74);

    break;
  }
  case Haxby: {
    
    vtkSmartPointer<vtkColorSeries> colorSeries =
      vtkSmartPointer<vtkColorSeries>::New();

    int nColors = 11;
    char nameBuf[16];
    colorSeries->SetNumberOfColors(nColors);
    for (int i = 0; i < nColors; i++) {
      sprintf(nameBuf, "color-%d", i);
      vtkStdString name(nameBuf);
      colors->SetColor(name, haxbyRed[nColors-i], haxbyGreen[nColors-i],
                       haxbyBlue[nColors-i]);
      colorSeries->SetColor(i, colors->GetColor3ub(name));
    }

    colorSeries->BuildLookupTable(lut, colorSeries->ORDINAL);
    lut->SetRampToSCurve();
    
    break;
  }
    
  }

    
};


