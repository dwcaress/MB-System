#ifndef SLOPEFILTER_H
#define SLOPEFILTER_H

// #include <vtkPolyDataAlgorithm.h>
#include <vtkDataSetAlgorithm.h>
#include <vtkFloatArray.h>
#include <vtkCellArray.h>

namespace mb_system {

class SlopeFilter : public vtkDataSetAlgorithm
{
public:
  vtkTypeMacro(SlopeFilter, vtkDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  static SlopeFilter* New();

protected:
  SlopeFilter();
  ~SlopeFilter();

  /// Transform incoming data to output
  int RequestData(vtkInformation* request,
		  vtkInformationVector** inputVector,
                  vtkInformationVector* outputVector);

  vtkSmartPointer<vtkFloatArray> slopes_;
  vtkSmartPointer<vtkPoints> points_;
  vtkSmartPointer<vtkCellArray> polygons_;
  
  
private:
  SlopeFilter(const mb_system::SlopeFilter&); // Not implemented.
  void operator=(const mb_system::SlopeFilter&);        // Not implemented.
};

  
}


#endif



