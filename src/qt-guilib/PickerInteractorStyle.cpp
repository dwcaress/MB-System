#include <locale.h>
#include <vtkPointPicker.h>
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
  int x = this->Interactor->GetEventPosition()[0];
  int y = this->Interactor->GetEventPosition()[1];
  
  std::cout << "Pixel x,y: " << x
            << " " << y << std::endl;

  vtkRenderer *renderer = GetDefaultRenderer();
  int *rendererSize = renderer->GetSize();
  
  std::cout << "rendererSize[1]: " << rendererSize[1] << " y: " << y <<
    std::endl;

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

  /* ***
  /// TEST TEST TEST
  FILE *fp = fopen(SELECTED_POINT_FILE, "w");
  fprintf(fp, "%d, %d, %d\n", (int )worldCoord[0], (int )worldCoord[1], (int )worldCoord[2]);
  fclose(fp);
  *** */
  
  // Display picked point coordinates via QVtkIem
  QString coordMsg(buf);
  qVtkRenderer_->getItem()->setPickedPoint(coordMsg);
  
  
  /// Print picker results for range of y values
  //  testPoints(x, y, renderer);
  
  // Forward event
  vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
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

