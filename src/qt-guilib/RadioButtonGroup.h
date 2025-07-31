#ifndef RadioButtonGroup_H
#define RadioButtonGroup_H
#include <vtkButtonWidget.h>
#include <vtkTexturedButtonRepresentation2D.h>
#include <vtkImageData.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkCommand.h>
#include <vtkSmartPointer.h>
#include <vector>

namespace mb_system {
/** Radio buttons implemented with VTK vtkButtonWidget. 
    Based on Google AI-generated code on 7/31/2025
*/
class RadioButtonGroup : public vtkCommand {
public:
  static RadioButtonGroup* New() { return new RadioButtonGroup; }
  
  void Execute(vtkObject* caller, unsigned long event,
	       void* calldata) override;
  
  // Add the buttons that belong to this radio group
  void AddButton(vtkButtonWidget* button) {
    this->buttons_.push_back(button);
  }

  
  void SetInteractor(vtkRenderWindowInteractor* interactor) {
    interactor_ = interactor;
  }

private:
  std::vector<vtkSmartPointer<vtkButtonWidget>> buttons_;
  vtkSmartPointer<vtkRenderWindowInteractor> interactor_;
};
}


#endif
