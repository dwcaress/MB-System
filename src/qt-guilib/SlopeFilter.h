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

  /// Slopes at each point
  vtkSmartPointer<vtkFloatArray> slopes_;

  /// Slopes at each delauney triangle
  vtkSmartPointer<vtkFloatArray> triSlopes_;  

  
private:
  SlopeFilter(const mb_system::SlopeFilter&); // Not implemented.
  void operator=(const mb_system::SlopeFilter&);        // Not implemented.
};

  
}


#endif



