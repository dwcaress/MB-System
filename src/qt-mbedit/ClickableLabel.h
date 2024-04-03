
#ifndef CLICKABLELABLE_H
#define CLICKABLELABLE_H

#include <QObject>
#include <QLabel>
#include <QMouseEvent>
#include <Qt>

class ClickableLabel : public QLabel {

  Q_OBJECT

 public:

  ClickableLabel(QWidget* parent = Q_NULLPTR,
		 Qt::WindowFlags f = Qt::WindowFlags());
  
 protected:
  
  virtual void mousePressEvent(QMouseEvent *event) override;
  
};


#endif

