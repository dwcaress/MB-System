#include <QDebug>
#include <QPoint>
#include "ClickableLabel.h"

using namespace mb_system;

ClickableLabel::ClickableLabel(QWidget* parent, Qt::WindowFlags f)
    : QLabel(parent) {

  /// setObjectName("swathCanvas");
}

void ClickableLabel::mousePressEvent(QMouseEvent *event) {

  qDebug() << "mousePressEvent() from " << objectName() << "!!!\n";
  QPoint globalPos = event->globalPos();
  qDebug() << "globalPos(): " << globalPos;
  QPoint pos = event->pos();
  qDebug() << "pos(): " << pos;

  emit labelMouseEvent(event);
  
  QLabel::mousePressEvent(event);

  
}



void ClickableLabel::mouseReleaseEvent(QMouseEvent *event) {

  qDebug() << "mouseReleaseEvent() from " << objectName() << "!!!\n";
  QPoint globalPos = event->globalPos();
  qDebug() << "globalPos(): " << globalPos;
  QPoint pos = event->pos();
  qDebug() << "pos(): " << pos;

  emit labelMouseEvent(event);
  
  QLabel::mousePressEvent(event);

  
}


void ClickableLabel::mouseMoveEvent(QMouseEvent *event) {

  qDebug() << "mouseMoveEvent() from " << objectName() << "!!!\n";
  QPoint globalPos = event->globalPos();
  qDebug() << "globalPos(): " << globalPos;
  QPoint pos = event->pos();
  qDebug() << "pos(): " << pos;

  emit labelMouseEvent(event);
  
  QLabel::mousePressEvent(event);

  
}
