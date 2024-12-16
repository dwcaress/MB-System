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
      // slope = 4000.;
      // TEST: Generate random value
      slope = (float )std::rand();
    }
    slopes_->SetValue(i, slope);
  }

  // Copy input geometry and data to output
  output->CopyStructure(input);
  output->GetPointData()->PassData(input->GetPointData());
  output->GetCellData()->PassData(input->GetCellData());
  
  // Add new scalars to output
  output->GetPointData()->AddArray(slopes_);
  output->GetPointData()->SetActiveScalars("Slopes");
  
  return 1;
}





//----------------------------------------------------------------------------
void SlopeFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
