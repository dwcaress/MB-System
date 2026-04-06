#include <unistd.h>
#include <sys/types.h>
#include <proj.h>

#include <vtkColorSeries.h>
#include <vtkNamedColors.h>
#include <vtkColorTransferFunction.h>
#include "mb_status.h"
#include "mb_define.h"
#include "mb_process.h"
#include "Utilities.h"

const char *mb_system::colorMapSchemeName(mb_system::ColorMapScheme scheme) {
  switch (scheme) {

  case BrewerDivergingSpectral:
    return "BrewerDivergingSpectral";

  case WhiteToBlue:
    return "WhiteToBlue";

  case Hawaii:
    return "Hawaii";

  case RedToBlue:
    return "RedToBlue";

  case Haxby:
    return "Haxby";
    
  default:
    return "unknown";
  }
  
}

void mb_system::makeLookupTable(mb_system::ColorMapScheme colorScheme,
                                vtkLookupTable* lut)
{
  vtkSmartPointer<vtkNamedColors> colors =
    vtkSmartPointer<vtkNamedColors>::New();

  std::cout << "makeLookupTable() w/ colorscheme " <<
    colorMapSchemeName(colorScheme) << std::endl;
  
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
    lut->SetRampToSCurve();
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

    vtkNew<vtkColorTransferFunction> ctf;
    // ctf->SetColorSpaceToDiverging();
    int nColors = 11;
    for (int i = 0; i < nColors; i++) {
      // x ranges from 0. (i=0) to 1. (i=nColors-1)
      double x = (double )i / (double )(nColors - 1);
      int ind = nColors-1 - i; 
      ctf->AddRGBPoint(x, haxbyRed[ind], haxbyGreen[ind], haxbyBlue[ind]);
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

    /* ***
    int nColors = 11;
    char nameBuf[16];
    lut->SetRampToSCurve();
    lut->SetNumberOfTableValues(nColors);
    lut->SetIndexedLookup(false);
    
    for (vtkIdType i = 0; i < nColors; i++) {
      lut->SetTableValue(i, haxbyRed[i], haxbyGreen[i], haxbyBlue[i],
                         0.8);
    }
    std::cout << "TEST TESTING first table value is red" << std::endl;
    lut->SetTableValue(0, 1., 0., 0., 1.);   /// TEST TEST TEST
    lut->Build();

    break;
    *** */
  }
  }    
};


bool mb_system::lockSwathfile(char *filename, char *appName) {
  std::cout << "lockSwathfile()\n";
  if (!filename) {

    return false;
  }

  int error = 0;
  int status = mb_pr_lockswathfile(0, filename,
				   MBP_LOCK_EDITBATHY, appName, &error);

  if (status == MB_SUCCESS) {
    return true;
  }
  else {
    std::cerr << "Error " << error << " from mb_pr_lockswathfile()\n";
    return false;
  }
}


bool mb_system::unlockSwathfile(char *filename, char *appName) {
  std::cout << "unlockSwathfile()\n";
  if (!filename) {

    return false;
  }
  
  int error = 0;

  int status = mb_pr_unlockswathfile(0, filename,
				     MBP_LOCK_EDITBATHY, appName, &error);

  if (status == MB_SUCCESS) {
    return true;
  }
  else {
    std::cerr << "Error " << error << " from mb_pr_unlockswathfile()\n";
    return false;
  }
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

