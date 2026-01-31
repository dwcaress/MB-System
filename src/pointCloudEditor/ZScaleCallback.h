#ifndef ZScaleCallback_H
#define ZScaleCallback_H
#include <vtkObject.h>
#include <vtkCallbackCommand.h>
#include <vtkSliderWidget.h>

class PointCloudEditor;

/// Callback invoked by changes to zscale vtkSliderWidget value
class ZScaleCallback : public vtkCallbackCommand {
  public:
    static ZScaleCallback* New(PointCloudEditor *editor) {
      return new ZScaleCallback(editor);
    }

    /// Set scale and re-visualize data
    virtual void Execute(vtkObject* caller, unsigned long, void *);

    ZScaleCallback(PointCloudEditor *editor) {
      editor_ = editor;
    }

  protected:

    /// Interact with PointCloudEditor through this pointer
    PointCloudEditor *editor_;
  };

#endif
