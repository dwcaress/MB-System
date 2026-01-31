#include "ZScaleCallback.h"
#include "PointCloudEditor.h"

void ZScaleCallback::Execute(vtkObject* caller, unsigned long, void *) {
  vtkSliderWidget* sliderWidget = reinterpret_cast<vtkSliderWidget*>(caller);

  double value = 
    static_cast<vtkSliderRepresentation*>
    (sliderWidget->GetRepresentation())->GetValue();
		 
  std::cerr << "SliderCallback: value = " << value << "\n";
  editor_->setVerticalExagg(value);
  editor_->visualize();
}
