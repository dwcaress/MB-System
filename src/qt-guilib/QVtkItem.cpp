#include "QVtkItem.h"
#include "QVtkRenderer.h"
#include "TopoColorMap.h"

using namespace mb_system;

QVtkItem::QVtkItem() :
  gridFilename_(nullptr),
  appTaskBusy_(false)
{
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
    qDebug() << "mirrorVertically: " << mirrorVertically();
    // Qt and OpenGL apparently have opposite y-axis direction
    setMirrorVertically(true);

}

QQuickFramebufferObject::Renderer *QVtkItem::createRenderer() const {
    qDebug() << "QVtkItem::createRenderer()";
    return new QVtkRenderer();
}

void QVtkItem::setGridFilename(char *gridFilename) {
    if (gridFilename_) {
        free((void *)gridFilename_);
    }
    gridFilename_ = strdup(gridFilename);
}


void QVtkItem::wheelEvent(QWheelEvent *event) {
    qDebug() << "QVtkItem::wheelEvent()";

    wheelEvent_ = std::make_shared<QWheelEvent>(*event);
    wheelEvent_->ignore();
    event->accept();

    // Schedule re-render
    update();
}

void QVtkItem::mousePressEvent(QMouseEvent *event) {
  
    qDebug() << "\nQVtkItem::mousePressEvent!!!";

    mouseButtonEvent_ = std::make_shared<QMouseEvent>(*event);
    mouseButtonEvent_->ignore();
    event->accept();

    // Schedule re-render
    update();
}

void QVtkItem::mouseReleaseEvent(QMouseEvent *event) {
    mouseButtonEvent_ = std::make_shared<QMouseEvent>(*event);
    mouseButtonEvent_->ignore();
    event->accept();

    // Schedule re-render
    update();
}

void QVtkItem::mouseMoveEvent(QMouseEvent *event) {
    mouseMoveEvent_ = std::make_shared<QMouseEvent>(*event);
    mouseMoveEvent_->ignore();
    event->accept();

    // Schedule re-render
    update();
}


void QVtkItem::setPickedPoint(QString coordMsg) {

  bool newPoint = false;
  if (pickedCoords_.compare(coordMsg)) {
    pickedCoords_ = QString(coordMsg);
    qDebug() << "emit new pointPicked()";
    emit pickedPointChanged(coordMsg);    
  }
}


QString QVtkItem::getPickedPoint() {
  return pickedCoords_;
}


void QVtkItem::setAppBusy(bool busy) {
  qDebug() << "**** setAppBusy to " << busy;
  appTaskBusy_ = busy;
  emit busyChanged(busy);
}


bool QVtkItem::getAppBusy() {
  return appTaskBusy_;
}


bool QVtkItem::setColorMapScheme(const char *colorMapName) {
  qDebug() << "setColormap() " << colorMapName;
  
  // Check for valid colorMap name

  TopoColorMap::Scheme scheme = TopoColorMap::schemeFromName(colorMapName);
  if (scheme == TopoColorMap::Scheme::Unknown) {
    return false;
  }

  displayProperties_.colorMapScheme(scheme);
  displayProperties_.changed(true);
  
  return true;

}
