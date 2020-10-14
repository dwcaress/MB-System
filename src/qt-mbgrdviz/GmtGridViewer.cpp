// These first three lines address
// issue described at
// https://stackoverflow.com/questions/18642155/no-override-found-for-vtkpolydatamapper
#include "vtkAutoInit.h" 
VTK_MODULE_INIT(vtkRenderingOpenGL2); // VTK was built with vtkRenderingOpenGL2
VTK_MODULE_INIT(vtkInteractionStyle);

// This example reads and displays contents of a GMT grid file 
#include <vtkSmartPointer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkActor.h>
#include <vtkActor2D.h>
#include <vtkProperty.h>
#include <vtkStringArray.h>
#include <vtkPolyDataMapper.h>
#include <vtkImageMapper.h>
#include <vtkDelaunay2D.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkElevationFilter.h>
#include <vtkLookupTable.h>
#include <vtkUnsignedCharArray.h>
#include <vtkPointData.h>
#include <vtkDataObject.h>
#include <vtkSTLReader.h>  // TEST TEST TEST
#include "GmtGridReader.h"

  // needed to easily convert int to std::string
int main(int argc, char* argv[])
{
   // Verify input arguments
   if ( argc != 2 )
   {
      std::cerr << "Usage: " << argv[0]
      << " GMT-gridFile"  << std::endl;
      return EXIT_FAILURE;
   }

   // Read the grid file
   std::string filePath = argv[1];

   vtkSmartPointer<GmtGridReader> reader =
     vtkSmartPointer<GmtGridReader>::New();
   
   reader->SetFileName ( filePath.c_str() );
   std::cout << "reader->Update()" << std::endl;
   reader->Update();

   // Color data points based on z-value
   vtkSmartPointer<vtkElevationFilter> colorizer =
     vtkSmartPointer<vtkElevationFilter>::New();

   colorizer->SetInputConnection(reader->GetOutputPort());   
   float zMin, zMax;
   reader->zSpan(&zMax, &zMin);
   colorizer->SetLowPoint(0, 0, zMin);
   colorizer->SetHighPoint(0, 0, zMax);

   // Visualize the data...

   // Create mapper
   std::cout << "create mapper" << std::endl;
   vtkSmartPointer<vtkPolyDataMapper> mapper =
      vtkSmartPointer<vtkPolyDataMapper>::New();

   std::cout << "mapper->SetInputConnection()" << std::endl;
   //  mapper->SetInputConnection(delaunay->GetOutputPort());
   mapper->SetInputConnection(colorizer->GetOutputPort());   

   // Create actor
   std::cout << "create actor" << std::endl;   
   vtkSmartPointer<vtkActor> actor =
      vtkSmartPointer<vtkActor>::New();

   // Assign mapper to actor
   std::cout << "assign mapper to actor" << std::endl;      
   actor->SetMapper(mapper);

   // Create renderer
   std::cout << "create renderer" << std::endl;         
   vtkSmartPointer<vtkRenderer> renderer =
      vtkSmartPointer<vtkRenderer>::New();

   // Create renderWindow
   std::cout << "create renderWindow" << std::endl;            
   vtkSmartPointer<vtkRenderWindow> renderWindow =
      vtkSmartPointer<vtkRenderWindow>::New();

   // Add renderer to the renderWindow
   std::cout << "add renderer to renderWindow" << std::endl;               
   renderWindow->AddRenderer(renderer);

   // Create renderWindowInteractor
   std::cout << "create renderWindowInteractor" << std::endl;                  
   vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
      vtkSmartPointer<vtkRenderWindowInteractor>::New();

   std::cout << "renderWindowInteractor->SetRenderWindow()" << std::endl;                     
   renderWindowInteractor->SetRenderWindow(renderWindow);

   // Add actor to the renderer
   std::cout << "rendererr->AddActor()" << std::endl;                        
   renderer->AddActor(actor);
   
   // renderer->SetBackground(.2, .3, .4);
   renderer->SetBackground(.2, .3, .4);   

   std::cout << "renderWindow->Render()" << std::endl;                        
   renderWindow->Render();
   std::cout << "renderWindowInteractor->Start()" << std::endl;
   renderWindowInteractor->Start();

   return EXIT_SUCCESS;
}

