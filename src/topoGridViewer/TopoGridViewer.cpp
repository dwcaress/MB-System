// These first three lines address
// issue described at
// https://stackoverflow.com/questions/18642155/no-override-found-for-vtkpolydatamapper
#include "vtkAutoInit.h" 
VTK_MODULE_INIT(vtkRenderingOpenGL2); // VTK was built with vtkRenderingOpenGL2
VTK_MODULE_INIT(vtkInteractionStyle);

// This example reads and displays contents of a GMT or swath grid file 
#include <proj.h>
#include <vtkVersion.h>
#include <vtkSmartPointer.h>
#include <vtkCubeAxesActor.h>
#include <vtkCamera.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkActor.h>
#include <vtkActorCollection.h>
#include <vtkActor2D.h>
#include <vtkProperty.h>
#include <vtkStringArray.h>
#include <vtkPolyDataMapper.h>
#include <vtkImageMapper.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkElevationFilter.h>
#include <vtkLookupTable.h>
#include <vtkColorSeries.h>
#include <vtkNamedColors.h>
#include <vtkUnsignedCharArray.h>
#include <vtkPointData.h>
#include <vtkDataObject.h>
#include <vtkGradientFilter.h>
#include <vtkInteractorStyleTerrain.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkTextProperty.h>
#include <vtkAbstractPicker.h>
#include <vtkPointPicker.h>
#include <vtkRendererCollection.h>
#include <vtkDelaunay2D.h>
#include "Utilities.h"

#include "TopoGridReader.h"

using namespace mb_system;

void myProjTest(char *msg);

void setupAxes(vtkSmartPointer<vtkCubeAxesActor> axesActor,
               vtkSmartPointer<vtkRenderer> renderer,
               vtkSmartPointer<mb_system::TopoGridReader> reader);


namespace {

// Define interaction style
class MouseInteractorStyle : public vtkInteractorStyleTrackballCamera
{
public:
  static MouseInteractorStyle* New();
  vtkTypeMacro(MouseInteractorStyle, vtkInteractorStyleTrackballCamera);

  virtual void OnLeftButtonDown() override
  {
    std::cout << "Picking pixel: " << this->Interactor->GetEventPosition()[0]
              << " " << this->Interactor->GetEventPosition()[1] << std::endl;


    //    vtkPointPicker *picker = (vtkPointPicker *)this->Interactor->GetPicker();
    vtkNew<vtkPointPicker> picker;
    picker->SetTolerance(100);
    picker->Pick(this->Interactor->GetEventPosition()[0],
                 this->Interactor->GetEventPosition()[1],
                 0, // always zero.
                 this->Interactor->GetRenderWindow()
                 ->GetRenderers()->GetFirstRenderer());

    std::cout << "PointId: " << picker->GetPointId() << std::endl;
    
    double *picked = picker->GetPickPosition();
    
    std::cout << "Picked value: " << picked[0] << " " << picked[1] << " "
              << picked[2] << std::endl;
    // Forward events
    vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
  }
};
vtkStandardNewMacro(MouseInteractorStyle);

} // namespace


int main(int argc, char* argv[])
{
  std::cout << "VTK Version: " << vtkVersion::GetVTKVersion() << std::endl;

  //  myProjTest((char *)"from main()-#1");
  // projTestUtil((char *)"from main()");
  // AprojTest((char *)"from main()-#1");  
  // std::cout << "back from projTest()" << std::endl;
  //  TopoGridReader::projTest((char *)"from main()-#1");
  
  bool drawAxes = true;
  bool useLUT = false;
  ColorMapScheme colorMapScheme = (ColorMapScheme )0;

  bool showGradient = false;
  bool error = false;
  // Verify input arguments
   if ( argc < 2 )
   {
     error = true;
   }

   TopoGridType gridType = TopoGridType::Unknown;
   
   // Look for options preceding file name, which is always last arg
   for (int i = 1; i < argc-1; i++) {
     if (!strcmp(argv[i], "-grad")) {
       std::cout << "showGradient\n";
       showGradient = true;
     }
     else if (!strcmp(argv[i], "-lut") && i < argc-2) {
       useLUT = true;
       colorMapScheme = (ColorMapScheme )atoi(argv[++i]);
     }
     else if (!strcmp(argv[i], "-swath")) {
       gridType = TopoGridType::SwathGrid;
     }
     else if (!strcmp(argv[i], "-gmt")) {
       gridType = TopoGridType::GMTGrid;       
     }
     else if (!strcmp(argv[1], "-help")) {
       error = true;
       break;
     }
     
     else {
       std::cerr << argv[i] << ": unknown option" << std::endl;
       error = true;
     }
     
   }

   if (error) {
      std::cerr << "Usage: " << argv[0]
      << " [-grad][-lut colorscheme] <-swath|-gmt> gridFile"  << std::endl;
      return EXIT_FAILURE;
   }

   std::cout << "showGradient: " << showGradient << std::endl;
   
   // Read the grid file
   std::string filePath = argv[argc-1];
   if (filePath[0] == '-') {
     std::cerr << filePath << ": invalid grid file name" << std::endl;
     return EXIT_FAILURE;
   }

   // Determine gridType if not yet specified
   if (gridType == TopoGridType::Unknown) {
     gridType = TopoGridReader::getGridType(filePath.c_str());

     std::cout << filePath << " grid type: " << gridType << std::endl;
   }
   
   vtkSmartPointer<mb_system::TopoGridReader> reader =
     vtkSmartPointer<mb_system::TopoGridReader>::New();
   
   reader->SetFileName ( filePath.c_str() );
   reader->setGridType(gridType);
   std::cout << "*** reader->Update()" << std::endl;
   reader->Update();
   if (reader->GetErrorCode()) {
     std::cout << "Error during reader->Update(): " << reader->GetErrorCode()
               << std::endl;
     return EXIT_FAILURE;
   }
   /* ***
   vtkNew<vtkDelaunay2D> delaunay;
   delaunay->SetInputData(reader->GetOutput());
   *** */
   
   double xMin, xMax, yMin, yMax, zMin, zMax;
   reader->gridBounds(&xMin, &xMax, &yMin, &yMax, &zMin, &zMax);
   std::cout << "main(): xMin=" << xMin << ", xMax=" << xMax
             << ", yMin=" << yMin << ", yMax=" << yMax << std::endl;
   
   vtkAlgorithmOutput *port = nullptr;
   
   std::cout << "*** create elevationFilter" << std::endl;
   // Color data points based on z-value
   vtkSmartPointer<vtkElevationFilter> elevationFilter =
     vtkSmartPointer<vtkElevationFilter>::New();

   std::cout << "*** elevationFilter->SetInputConnection\n";
   elevationFilter->SetInputConnection(reader->GetOutputPort());

   elevationFilter->SetLowPoint(0, 0, zMin);
   elevationFilter->SetHighPoint(0, 0, zMax);
   std::cout << "zMin: " << zMin << ", zMax: " << zMax << std::endl;

   std::cout << "showGradient: " << showGradient << std::endl;
   
   port = elevationFilter->GetOutputPort();
   if (useLUT) {
     if (!showGradient) {
       elevationFilter->SetScalarRange(zMin, zMax);
     }
   }

   vtkSmartPointer<vtkGradientFilter> gradientFilter =
     vtkSmartPointer<vtkGradientFilter>::New();

   if (showGradient) {

     gradientFilter->SetInputConnection(port);
     port = gradientFilter->GetOutputPort();
   }
   
   // Visualize the data...

   // Create renderer
   std::cout << "*** create renderer" << std::endl;         
   vtkSmartPointer<vtkRenderer> renderer =
      vtkSmartPointer<vtkRenderer>::New();

   // Create gridMapper
   std::cout << "*** create gridMapper" << std::endl;
   vtkSmartPointer<vtkPolyDataMapper> gridMapper =
      vtkSmartPointer<vtkPolyDataMapper>::New();

   std::cout << "*** gridMapper->SetInputConnection()" << std::endl;
   gridMapper->SetInputConnection(port);
   std::cout << "done setting connection\n";
   
   if (useLUT) {
     if (showGradient)  {
       zMin = gradientFilter->GetOutput()->GetScalarRange()[0];
       zMin = gradientFilter->GetOutput()->GetScalarRange()[1];       
       std::cout << " zMIn: " << zMin << ", zMax: " << zMax << std::endl;
     }
     
     vtkSmartPointer<vtkLookupTable> lut =
       vtkSmartPointer<vtkLookupTable>::New();

     std::cout << "colorMapScheme: " << colorMapScheme << std::endl;
     makeLookupTable(colorMapScheme, lut);

     std::cout << "SetScalarRange " << zMin << "  " << zMax << std::endl;
     gridMapper->SetScalarRange(zMin, zMax);
     gridMapper->ScalarVisibilityOn();
     gridMapper->SetLookupTable(lut);
   }
   
   // Create actor
   std::cout << "*** create actor" << std::endl;   
   vtkSmartPointer<vtkActor> actor =
      vtkSmartPointer<vtkActor>::New();

   /// TEST TEST Lighting/surface properties
   actor->GetProperty()->SetOpacity(1.00);
   actor->GetProperty()->SetSpecularPower(10.00);   
   
   // Assign gridMapper to actor
   std::cout << "*** assign gridMapper to actor" << std::endl;      
   actor->SetMapper(gridMapper);

   if (drawAxes) {
     vtkSmartPointer<vtkCubeAxesActor> cubeAxesActor =
       vtkSmartPointer<vtkCubeAxesActor>::New();

     setupAxes(cubeAxesActor, renderer, reader);
   }
   else {
   }

   // Add actor to the renderer
   std::cout << "*** rendererr->AddActor()" << std::endl;                        
   renderer->AddActor(actor);
   
   // Create renderWindow
   std::cout << "*** create renderWindow" << std::endl;            
   vtkSmartPointer<vtkRenderWindow> renderWindow =
     vtkSmartPointer<vtkRenderWindow>::New();

   // Add renderer to the renderWindow
   std::cout << "*** add renderer to renderWindow" << std::endl;               
   renderWindow->AddRenderer(renderer);

   // Create renderWindowInteractor
   std::cout << "*** create renderWindowInteractor" << std::endl;                  
   vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
     vtkSmartPointer<vtkRenderWindowInteractor>::New();

   // Set interactor style
   //   vtkSmartPointer<vtkInteractorStyleTerrain> style =
   // vtkSmartPointer<vtkInteractorStyleTerrain>::New();

   // vtkSmartPointer<vtkInteractorStyleTrackballCamera> style =
   // vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();

   vtkNew<MouseInteractorStyle> style;
   renderWindowInteractor->SetInteractorStyle(style);

   renderWindowInteractor->SetInteractorStyle(style);
     
   renderWindowInteractor->SetRenderWindow(renderWindow);

   renderer->SetBackground(1.0, 1.0, 1.0);   

   renderer->ResetCamera();

   std::cout << "*** renderWindowInteractor->Start()" << std::endl;
   renderWindowInteractor->Start();

   std::cout << "*** renderWindow->Render()" << std::endl;                        
   renderWindow->Render();

   return EXIT_SUCCESS;
}


void setupAxes(vtkSmartPointer<vtkCubeAxesActor> axesActor,
               vtkSmartPointer<vtkRenderer> renderer,
               vtkSmartPointer<mb_system::TopoGridReader> reader) {

  // Colors for axes
  vtkSmartPointer<vtkNamedColors> colors = 
    vtkSmartPointer<vtkNamedColors>::New();

  vtkColor3d axisColor = colors->GetColor3d("Black");
  //  vtkColor3d labelColor = colors->GetColor3d("Red");

  // Axes actor
  axesActor = vtkSmartPointer<vtkCubeAxesActor>::New();
  axesActor->SetUseTextActor3D(0);


  axesActor->SetBounds(reader->GetOutput()->GetBounds());
  axesActor->SetCamera(renderer->GetActiveCamera());
  axesActor->GetTitleTextProperty(0)->SetColor(axisColor.GetData());
  axesActor->GetTitleTextProperty(0)->SetFontSize(48);
  axesActor->GetLabelTextProperty(0)->SetColor(axisColor.GetData());

  axesActor->GetTitleTextProperty(1)->SetColor(axisColor.GetData());
  axesActor->GetLabelTextProperty(1)->SetColor(axisColor.GetData());

  axesActor->GetTitleTextProperty(2)->SetColor(axisColor.GetData());
  axesActor->GetLabelTextProperty(2)->SetColor(axisColor.GetData());

  axesActor->GetXAxesLinesProperty()->SetColor(axisColor.GetData());
  axesActor->GetYAxesLinesProperty()->SetColor(axisColor.GetData());
  axesActor->GetZAxesLinesProperty()->SetColor(axisColor.GetData());
  
  axesActor->DrawXGridlinesOn();
  axesActor->DrawYGridlinesOn();
  axesActor->DrawZGridlinesOn();
  
  axesActor->SetXTitle("Easting");
  axesActor->SetYTitle("Northing");
  axesActor->SetZTitle("Depth");

#if VTK_MAJOR_VERSION == 6
  axesActor->SetGridLineLocation(VTK_GRID_LINES_FURTHEST);
#endif
#if VTK_MAJOR_VERSION > 6
  axesActor->SetGridLineLocation(
				  axesActor->VTK_GRID_LINES_FURTHEST);
#endif
  
  axesActor->XAxisMinorTickVisibilityOff();
  axesActor->YAxisMinorTickVisibilityOff();
  axesActor->ZAxisMinorTickVisibilityOff();

  //  axesActor->SetFlyModeToStaticEdges();
  
  renderer->AddActor(axesActor);    
}


void myProjTest(char *msg) {
  std::cout << "standalone projTest(): " << msg << std::endl;
  PJ_INFO projInfo = proj_info();
  std::cerr << "proj release: " << projInfo.release << std::endl;
  
  double xMin = 0.;
  
  // Get UTM zone of grid's W edge
  int utmZone = ((xMin + 180)/6 + 0.5);

  std::cerr << "UTM zone: " << utmZone << std::endl;
  
  PJ_CONTEXT *projContext = proj_context_create();
  if (projContext) {
    std::cerr << "Created projContext OK" << std::endl;
  }
  else {
    std::cerr << "Error creating projContext OK" << std::endl;
  }

  const char *srcCRS = "EPSG:4326";
  char targCRS[64];
  sprintf(targCRS, "+proj=utm +zone=%d +datum=WGS84", utmZone); 
  PJ *proj = proj_create_crs_to_crs (projContext,
                                     srcCRS,
                                     targCRS,
                                     nullptr);
  if (!proj) {
    std::cerr << "failed to create proj" << std::endl;
  }
  else {
    std::cerr << "created proj OK" << std::endl;    
  }

  int BSIZE = 1024;
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
      } else {
        puts("using ?");
      }
    }
  }  
}
