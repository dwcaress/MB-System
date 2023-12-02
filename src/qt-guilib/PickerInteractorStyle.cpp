#include <locale.h>
#include <vtkPointPicker.h>
#include <vtkSphereSource.h>
#include <proj.h>
#include "PickerInteractorStyle.h"
#include "QVtkRenderer.h"
#include "QVtkItem.h"

#define VTK_CREATE(type, name) vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

using namespace mb_system;

PickerInteractorStyle::PickerInteractorStyle():
  qVtkRenderer_(nullptr) {
  selectedMapper_ = vtkSmartPointer<vtkDataSetMapper>::New();
  selectedActor_ = vtkSmartPointer<vtkActor>::New();
}


void PickerInteractorStyle::OnLeftButtonDown() {
  // Get starting mouse position
  interactor_->GetMousePosition(&startMousePos_[0],
				&startMousePos_[1]);
  
  // Forward event
  vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
}


void PickerInteractorStyle::OnLeftButtonUp() {

  // If mouse was dragged, don't do anything
  int mousePos[2];
  
  interactor_->GetMousePosition(&mousePos[0], &mousePos[1]);

  std::cerr << "startPosX: " << startMousePos_[0] << "   startPosY: "
	    << startMousePos_[1] << "\n";
  
  std::cerr << "mouseX: " << mousePos[0] << "   mouseY: "
	    << mousePos[1] << "\n";
  
  if (mousePos[0] != startMousePos_[0] ||
      mousePos[1] != startMousePos_[1]) {

    // Forward event and return
    vtkInteractorStyleTrackballCamera::OnLeftButtonUp();
    return;
  }
  
  int x = this->Interactor->GetEventPosition()[0];
  int y = this->Interactor->GetEventPosition()[1];
  int z = this->Interactor->GetEventPosition()[2];
  
  std::cout << "Pixel x,y,z: " << x
            << " " << y << " " << z << std::endl;

  vtkRenderer *renderer = GetDefaultRenderer();
  int *rendererSize = renderer->GetSize();
  
  std::cout << "rendererSize[1]: " << rendererSize[1] << " y: " << y <<
    std::endl;

  // Put a sphere at picked location
  vtkNew<vtkSphereSource> sphere;
  sphere->SetRadius(50.);
  sphere->SetCenter(x, y, z);
  sphere->SetPhiResolution(100);
  sphere->SetThetaResolution(100);
    
  
  // Convert from Qt coordinate system (origin at upper left) to
  // VTK coordinate system (origin at lower left)
  y = rendererSize[1] - y + 1;
    
  std::cout << "Corr Pixel x,y: " << x
            << " " << y << std::endl;  

  vtkNew<vtkPointPicker> picker;
    
  picker->Pick(x, y, 0, renderer);

  vtkIdType pointId = picker->GetPointId();
  
  std::cout << "PointId: " << pointId << std::endl;
    
  double *worldCoord = picker->GetPickPosition();
    
  std::cout << "WorldCoord value: " << worldCoord[0] << " " <<
    worldCoord[1] << " " << worldCoord[2] << std::endl;

  // If dataset in geographic CRS, display picked point in geogaphic
  // CRS
  const char *projString = qVtkRenderer_->getGridReader()->fileCRS();
  bool geographicCRS = qVtkRenderer_->getGridReader()->geographicCRS();
  std::cout << "file CRS proj-string: " << projString << std::endl;

  int xyUnits;
  int decp;
  if (geographicCRS) {
    // degree ascii code
    // xyUnits = 0370;
    xyUnits = 'd';
    decp = 4;
  }
  else {
    // meters
    xyUnits = 'm';
    decp = 0;
  }

  printf("xyUnits: %c\n", xyUnits);
  printf("degree symbol: %c\n", 0370);
  PJ *transform = qVtkRenderer_->getGridReader()->projFileToDisplay();

  if (transform) {
    PJ_COORD utm = proj_coord(worldCoord[0], worldCoord[1], 0, 0);
    PJ_COORD lonLat = proj_trans(transform, PJ_INV, utm);
    std::cout << "transformed" << std::endl;
    worldCoord[0] = lonLat.xyzt.x;
    worldCoord[1] = lonLat.xyzt.y;
  }
  else {
    std::cout << "No transform" << std::endl;
  }
  
  // Correct elevation for vertical exaggeration
  worldCoord[2] /=
    (qVtkRenderer_->getDisplayProperties()->verticalExagg() *
     qVtkRenderer_->getGridReader()->zScaleLatLon());
  
  char buf[256];
  if (pointId != -1) {

    if (worldCoord[2] == TopoGridData::NoData) {
      sprintf(buf, "%.*f%c, %.*f%c, NoData %s",
              decp, worldCoord[0], xyUnits, decp, worldCoord[1],
              xyUnits, projString);
      
    }
    else {
      sprintf(buf, "%.*f%c, %.*f%c, %.0fm %s",
              decp, worldCoord[0], xyUnits, decp, worldCoord[1], xyUnits,
              worldCoord[2], projString);
    }
  }
  else {
    //    sprintf(buf, "unknown position");
    sprintf(buf, "%.*f%c, %.*f%c, %.0fm ??? %s",
            decp, worldCoord[0], xyUnits, decp, worldCoord[1], xyUnits,
            worldCoord[2], projString);
  }

  std::cerr << "buf: " << buf << std::endl;
  
  // Store picked point coordinates in QVtkRenderer
  qVtkRenderer_->setPickedPoint(worldCoord);

  // Display picked point coordinates via QVtkIem
  QString coordMsg(buf);
  qVtkRenderer_->getItem()->setPickedPoint(coordMsg);
  
  // Forward event
  vtkInteractorStyleTrackballCamera::OnLeftButtonUp();

  // Re-render
  qVtkRenderer_->getItem()->update();
}


void PickerInteractorStyle::testPoints(int x, int y,
                                       vtkRenderer *renderer) {
  vtkNew<vtkPointPicker> picker;
  
  for (int y1 = 0; y1 < 1000; y1++) {
    picker->Pick(x, y1, 0, renderer);
    vtkIdType pointId = picker->GetPointId();
    double *worldCoord = picker->GetPickPosition();

    std::cout << "x: " << x << " y: " << y1 << "  pointId: " << pointId <<
      " worldCoord: " << worldCoord[0] << " " << worldCoord[1] << " " <<
      worldCoord[2] << std::endl;
  }
  
}

