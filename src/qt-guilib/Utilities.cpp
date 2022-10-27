#include <unistd.h>
#include <sys/types.h>
#include <proj.h>

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
  case BrewerDivergingSpectral: {
    // Make the lookup using a Brewer palette.
    std::cout << "Brewer diverging spectral LUT" << std::endl;    
    vtkSmartPointer<vtkColorSeries> colorSeries =
      vtkSmartPointer<vtkColorSeries>::New();
    colorSeries->SetNumberOfColors(256);
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
    std::cout << "WhiteToBlue LUT" << std::endl;    
    lut->SetHueRange(0.7, 0);
    lut->SetSaturationRange(1.0, 0);
    lut->SetValueRange(0.5, 1.0);
    break;
  }
  case Hawaii: {
    // Make the lookup table with a preset number of colours.
    std::cout << "Hawaii LUT" << std::endl;    
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
    std::cout << "RedToBlue LUT" << std::endl;
    
    lut->SetHueRange(0.7, 0.06);
    lut->SetSaturationRange(1.0, 0.78);
    lut->SetValueRange(0.5, 0.74);

    break;
  }
  case Haxby:
  default:  {

    std::cout << "Haxby LUT" << std::endl;
    
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


bool mb_system::mbLockFile(char *filename) {
  std::cout << "mbLockFile() not yet implemented" << std::endl;
  return true;
}


bool mb_system::mbUnlockFile(char *filename) {
  std::cout << "mbUnlockFile() not yet implemented" << std::endl;
  return true;
}


bool mb_system::projTestUtil(char *msg) {

  PJ_INFO projInfo = proj_info();
  std::cerr << "proj release: " << projInfo.release << std::endl;
  
  std::cerr << "projTestUtil(): " << msg << std::endl;
  int BSIZE = 1024;
  
  double xMin = 0.;
  
  // Get UTM zone of grid's W edge
  int utmZone = ((xMin + 180)/6 + 0.5);

  std::cerr << "UTM zone: " << utmZone << std::endl;
  
  PJ_CONTEXT *projContext = ::proj_context_create();
  if (projContext) {
    std::cerr << "Created projContext OK" << std::endl;
  }
  else {
    std::cerr << "Error creating projContext OK" << std::endl;
    return false;
  }

  const char *srcCRS = "EPSG:4326";
  char targCRS[64];
  sprintf(targCRS, "+proj=utm +zone=%d +datum=WGS84", utmZone); 
  std::cout << "targCRS: " << targCRS << std::endl;
  PJ *proj = ::proj_create_crs_to_crs (projContext,
                                     srcCRS,
                                     targCRS,
                                     nullptr);
  if (!proj) {
    std::cerr << "failed to create proj" << std::endl;
  }
  else {
    std::cerr << "created proj OK" << std::endl;    
  }
  
  char buffer[BSIZE];
  int const pid = getpid();
  snprintf(buffer, BSIZE, "/proc/%d/maps", pid);
  FILE * const maps = fopen(buffer, "r");
  while (fgets(buffer, BSIZE, maps) != NULL) {
    unsigned long from, to;
    int const r = sscanf(buffer, "%lx-%lx", &from, &to);
    if (r != 2) {
      puts("!");
      continue;
    }
    void *fptr = (void *)(&proj_create_crs_to_crs);
        
    if ((from <= (uintptr_t)fptr) &&
        ((uintptr_t)fptr < to)) {
      char const * name = strchr(buffer, '/');
      if (name) {
        printf("using %s\n", name);
        std::cerr << "fptr: " << fptr << std::endl << std::endl;        
      } else {
        puts("?");
      }
    }
  }
  return true;
}

