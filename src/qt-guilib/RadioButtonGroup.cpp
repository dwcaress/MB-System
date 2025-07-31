#include "RadioButtonGroup.h"

using namespace mb_system;


void RadioButtonGroup::Execute(vtkObject* caller, unsigned long event,
				  void* calldata) {

  if (event == vtkCommand::StateChangedEvent) {
    vtkButtonWidget* pressedButton = static_cast<vtkButtonWidget*>(caller);
    int pressedButtonIndex = -1;

    // Find the index of the pressed button
    for (size_t i = 0; i < buttons_.size(); ++i)   {
      if (buttons_[i] == pressedButton) {
	pressedButtonIndex = i;
	break;
      }
    }
    std::cerr << "pressedButtonIndex = " << pressedButtonIndex << "\n";
    // If the pressed button is not already "on", turn it "on" and
    // others "off"
    vtkButtonRepresentation* buttonRep = 
      vtkButtonRepresentation::SafeDownCast(pressedButton->
					    GetRepresentation());      

    std::cerr << "pressedButton state = " << buttonRep->GetState()
	      << "\n";      

    buttonRep->SetState(1);
      

    // Turn "on" the pressed button
    buttonRep->SetState(1);

    // Turn "off" all other buttons in the group
    for (size_t i = 0; i < buttons_.size(); ++i) {
      if (i != pressedButtonIndex) {
	vtkButtonRepresentation* buttonRep = 
	  vtkButtonRepresentation::SafeDownCast(buttons_[i]->
						GetRepresentation());
	buttonRep->SetState(0);	    
      }

    }        
    // --- Add your radio button specific actions here ---
    // For example, you might print a message:
    std::cout << "Radio Button " << pressedButtonIndex <<
      " selected." << std::endl;
      
    // Force a render to update the button appearances
    if (interactor_) {
      interactor_->GetRenderWindow()->Render();
    }
  }
}


#ifdef RADIOBUTTONS_MAIN_TEST
int main(int, char*[]) {
  // 1. Create a renderer and render window
  vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
  vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
  renderWindow->AddRenderer(renderer);

  // 2. Create an interactor
  vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
    vtkSmartPointer<vtkRenderWindowInteractor>::New();
  renderWindowInteractor->SetRenderWindow(renderWindow);

  // 3. Create textures for button states (on and off)
  vtkSmartPointer<vtkImageData> imageOn = vtkSmartPointer<vtkImageData>::New();
  vtkSmartPointer<vtkImageData> imageOff = vtkSmartPointer<vtkImageData>::New();
  
  // (Assuming you have functions like CreateButtonOn and CreateButtonOff to generate textures)
  // For simplicity, we'll create simple filled squares as textures.
  // You would typically load image files for your desired radio button appearance.
  imageOn->SetDimensions(10, 10, 1);
  imageOn->AllocateScalars(VTK_UNSIGNED_CHAR, 3);
  unsigned char* pixelsOn = static_cast<unsigned char*>(imageOn->GetScalarPointer());
  for (int i = 0; i < 10 * 10 * 3; i += 3) {
    pixelsOn[i] = 0;     // Red
    pixelsOn[i + 1] = 0; // Green
    pixelsOn[i + 2] = 255; // Blue (for "on" state)
  }

  imageOff->SetDimensions(10, 10, 1);
  imageOff->AllocateScalars(VTK_UNSIGNED_CHAR, 3);
  unsigned char* pixelsOff =
    static_cast<unsigned char*>(imageOff->GetScalarPointer());

  for (int i = 0; i < 10 * 10 * 3; i += 3) {
    pixelsOff[i] = 128; // Gray (for "off" state)
    pixelsOff[i + 1] = 128;
    pixelsOff[i + 2] = 128;
  }
  
  // 4. Create multiple vtkButtonWidget instances
  std::vector<vtkSmartPointer<vtkButtonWidget>> radioButtons;
  const int numRadioButtons = 3;
  for (int i = 0; i < numRadioButtons; ++i) {
    vtkSmartPointer<vtkTexturedButtonRepresentation2D> buttonRepresentation =
      vtkSmartPointer<vtkTexturedButtonRepresentation2D>::New();
    buttonRepresentation->SetNumberOfStates(2); // Two states: on and off
    buttonRepresentation->SetButtonTexture(0, imageOff); // State 0: off
    buttonRepresentation->SetButtonTexture(1, imageOn);  // State 1: on

    // Place the buttons in the scene
    double bounds[6];
    bounds[0] = 50.0 + i * 60.0; // Adjust for spacing
    bounds[1] = bounds[0] + 50.0;
    bounds[2] = 50.0;
    bounds[3] = 100.0;
    bounds[4] = 0.0;
    bounds[5] = 0.0;
    buttonRepresentation->PlaceWidget(bounds);

    vtkSmartPointer<vtkButtonWidget> buttonWidget =
      vtkSmartPointer<vtkButtonWidget>::New();

    buttonWidget->SetInteractor(renderWindowInteractor);
    buttonWidget->SetRepresentation(buttonRepresentation);
    buttonWidget->EnabledOn(); // Enable the button

    radioButtons.push_back(buttonWidget);
  }

  // 5. Create a callback and associate it with the radio buttons
  vtkSmartPointer<RadioButtonGroup> radioButtonGroup =
    vtkSmartPointer<RadioButtonGroup>::New();

  radioButtonGroup->SetInteractor(renderWindowInteractor);
  for (int i = 0; i < numRadioButtons; ++i) {
    radioButtonGroup->AddButton(radioButtons[i]);
    radioButtons[i]->AddObserver(vtkCommand::StateChangedEvent,
				 radioButtonGroup);
  }

  // Initialize the first radio button to "on"
  if (!radioButtons.empty()) {
    vtkButtonRepresentation* buttonRep = 
      vtkButtonRepresentation::SafeDownCast(radioButtons[0]->
					    GetRepresentation());
    buttonRep->SetState(1);
  }

  // 6. Start the interaction
  renderWindow->Render();
  renderWindowInteractor->Start();

  return 0;
}
#endif
