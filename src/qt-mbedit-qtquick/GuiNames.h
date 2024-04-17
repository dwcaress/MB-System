#ifndef GUINAMES_
#define GUINAMES_

#include <QObject>

class GuiNames : public QObject {

  Q_OBJECT

 public:
  GuiNames();
  
  Q_PROPERTY(QString xTrackSlider READ xTrackSlider)
  
  QString xTrackSlider() const {
    return "xTrackSlider";
  }

  Q_PROPERTY(QString pingsShownSlider READ pingsShownSlider)
  
  QString pingsShownSlider() const {
    return "pingsShownSlider";
  }

  Q_PROPERTY(QString verticalExaggSlider READ verticalExaggSlider)
  
  QString verticalExaggSlider() const {
    return "verticalExaggSlider";
  }


  Q_PROPERTY(QString pingStepSlider READ pingStepSlider)
  
  QString pingStepSlider() const {
    return "pingStepSlider";
  }

  Q_PROPERTY(QString swathCanvas READ swathCanvas)
  
  QString swathCanvas() const {
    return "swathCanvas";
  }        
  
};

#endif
