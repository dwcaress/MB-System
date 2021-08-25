#include "QVtkItem.h"
#include "QVtkRenderer.h"

using namespace mb_system;

QVtkItem::QVtkItem() :
    gridFilename_(nullptr)
{
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
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
    qDebug() << "QVtkItem::mouseReleaseEvent";

    mouseButtonEvent_ = std::make_shared<QMouseEvent>(*event);
    mouseButtonEvent_->ignore();
    event->accept();

    // Schedule re-render
    update();
}

void QVtkItem::mouseMoveEvent(QMouseEvent *event) {
    qDebug() << "QVtkItem::mouseMoveEvent";

    mouseMoveEvent_ = std::make_shared<QMouseEvent>(*event);
    mouseMoveEvent_->ignore();
    event->accept();

    // Schedule re-render
    update();
}


