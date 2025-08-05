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
  
  /** Radio button group implemented with VTK vtkButtonWidget. 
      Based on Google AI-generated code on 7/31/2025
  */
  class RadioButtonGroup : public vtkCommand {
  public:

    RadioButtonGroup() {
      
      // Selected button image
      
      // Unselected button image
    }
    
    static RadioButtonGroup* New() { return new RadioButtonGroup; }

    /// Callback invoked when any button in the group is selected
    void Execute(vtkObject* caller, unsigned long event,
		 void* calldata) override;

    /// Application-specific processing
    virtual bool processAction(int pressedButtonIndex) {
      // --- Add your radio button specific actions here ---
      // For example, you might print a message:
      std::cout << "Radio Button " << pressedButtonIndex <<
	" selected." << std::endl;

      return true;
    }
    
    // Add specified button to this radio group
    void addButton(vtkButtonWidget* button) {
      buttons_.push_back(button);
    }

    /* ***
    // Create and add a button to this radio group
    vtkButtonWidget *addButton(void) {
      // Create 'standard' representation
      
      // Create a button with 'standard' representation
      vtkButtonWidget *button = nullptr;
      
      buttons_.push_back(button);
      return button;
    }    
    *** */
  
    void setInteractor(vtkRenderWindowInteractor* interactor) {
      interactor_ = interactor;
    }

    void setActor(vtkActor* actor) {
     actor_ = actor;
    }    

  protected:
    std::vector<vtkSmartPointer<vtkButtonWidget>> buttons_;
    std::vector<vtkSmartPointer<vtkButtonWidget>> buttonRepresentations_;    
    vtkSmartPointer<vtkRenderWindowInteractor> interactor_;
    vtkSmartPointer<vtkActor> actor_;    
  };


}  // namespace


#endif
