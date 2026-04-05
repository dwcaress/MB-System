#include <climits>
#include "SlopeFilter.h"

#include <vtkDataObject.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkCellData.h>
#include <vtkIdList.h>
#include <vtkCellArrayIterator.h>

using namespace mb_system;

vtkStandardNewMacro(SlopeFilter);

SlopeFilter::SlopeFilter()
{
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1);
}

SlopeFilter::~SlopeFilter()
{
}

int SlopeFilter::RequestData(vtkInformation* vtkNotUsed(request),
			     vtkInformationVector** inputVector,
			     vtkInformationVector* outputVector)
{

  // get the input and output
  vtkPolyData* input = vtkPolyData::GetData(inputVector[0], 0);
  vtkPolyData* output = vtkPolyData::GetData(outputVector, 0);

  int nPoints = input->GetNumberOfPoints();
  if (nPoints < 1) {
    std::cerr << "No input to SlopeFilter\n";
    return 1;
  }

  std::cerr << "SlopeFilter::RequestData()\n";
  
  std::cerr << "SlopeFilter input nPoints: " << nPoints << "\n";
  
  vtkPoints *points = input->GetPoints();

  vtkCellArray *polys = input->GetPolys();
  std::cerr << "SlopeFilter input #cells: " << polys->GetNumberOfCells()
	    << "\n";

  // Allocate slopes array
  slopes_ = vtkSmartPointer<vtkFloatArray>::New();
  slopes_->SetName("Slopes");
  slopes_->SetNumberOfComponents(1);
  slopes_->SetNumberOfTuples(nPoints);

  double point[3];
  bool derivativeOK = false;

  std::cerr << "RAND_MAX = " << RAND_MAX << "\n";
  
  // Compute slope at each point where possible (check for NoData values)
  for (int i = 0; i < nPoints; i++ ) {
    derivativeOK = true;

    // Check edge cases where slope can't be computed
    // HERE
    
    points->GetPoint(i, point);
    
    float slope = 0.;
    if (derivativeOK) {
      // TEST: Generate random value
      slope = (float )std::rand();
    }
    slopes_->SetValue(i, slope);
  }

  triSlopes_ = vtkSmartPointer<vtkFloatArray>::New();
  triSlopes_->SetName("TriSlopes");
  triSlopes_->SetNumberOfComponents(1);
  triSlopes_->SetNumberOfTuples(polys->GetNumberOfCells());

  // Compute slope of each triangle
  vtkIdList *cellPointList = vtkSmartPointer<vtkIdList>::New();

  int nCells = polys->GetNumberOfCells();

  int nCellsProcessed = 0;


  for (int i = 0; i < nCells; i++ ) {
    // NB: GetNextCell() is not thread-safe!
    if (!polys->GetNextCell(cellPointList)) {
      std::cerr << "Premature end of cells\n";
      return 0;
    }
    // Expecting each polygon to be a triangle
    if (cellPointList->GetNumberOfIds() != 3) {
      std::cerr << "Got " << cellPointList->GetNumberOfIds() <<
	" in cell; expecting 3 (triangle)\n";
      return 0;
    }
    nCellsProcessed++;
  }

  /* ***
  // Following thread-safe method encounters segfault
  vtkCellArrayIterator *iter = vtk::TakeSmartPointer(polys->NewIterator());
  for (iter->GoToFirstCell(); !iter->IsDoneWithTraversal();
       iter->GoToNextCell()) {


    /// iter->GetCurrentCell(cellPointList);
    
    // Expecting each polygon to be a triangle
    if (cellPointList->GetNumberOfIds() != 3) {
      std::cerr << "Got " << cellPointList->GetNumberOfIds() <<
	" in cell; expecting 3 (triangle)\n";
      return 0;
    }

    // Compute slope of plane defined by triangle

    nCellsProcessed++;
    std::cerr << "Processed " << nCellsProcessed << " cells so far\n";    
  }
  *** */
  
  std::cerr << "Processed " << nCellsProcessed << " cells\n";
    
  // Copy input geometry and data to output
  // e.g. in vtkElevationFilter::RequestData()
  output->CopyStructure(input);
  output->GetPointData()->PassData(input->GetPointData());
  output->GetCellData()->PassData(input->GetCellData());
  
  // Associate slope scalars with points
  output->GetPointData()->AddArray(slopes_);
  output->GetPointData()->SetActiveScalars("Slopes");

  // Associate triangle slope scalars with polygons
  output->GetCellData()->AddArray(triSlopes_);
  output->GetCellData()->SetActiveScalars("TriSlopes");
  
  return 1;
}





//----------------------------------------------------------------------------
void SlopeFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
