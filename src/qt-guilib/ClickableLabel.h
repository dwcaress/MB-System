
#ifndef CLICKABLELABLE_H
#define CLICKABLELABLE_H

#include <QObject>
#include <QLabel>
#include <QMouseEvent>
#include <Qt>

namespace mb_system {

  /**
     ClickableLabel is a QLabel widget that emits a signal when mouse button
     is pressed, released, or while the mouse is dragged within the widget, 
     with the QMouseEvent as signal payload. ClickableLabel can display a 
     QPixmap, which the user interacts with using the mouse.

   */
  class ClickableLabel : public QLabel {

    Q_OBJECT

  public:

    ClickableLabel(QWidget* parent = Q_NULLPTR,
		   Qt::WindowFlags f = Qt::WindowFlags());
    
  protected:

    /// Emit signal with mouse press event payload 
    virtual void mousePressEvent(QMouseEvent *event) override;

    /// Emit signal with mouse release event payload     
    virtual void mouseReleaseEvent(QMouseEvent *event) override;

    /// Emit signal with mouse drag event payload     
    virtual void mouseMoveEvent(QMouseEvent *event) override;    


  signals:
    void labelMouseEvent(QMouseEvent *event);
    
  };
}

#endif

