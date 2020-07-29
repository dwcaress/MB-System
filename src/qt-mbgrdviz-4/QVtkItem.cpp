#include "QVtkItem.h"
#include "QVtkRenderer.h"


QVtkItem::QVtkItem() :
    gridFilename_(nullptr)
{
    qDebug() << "QVtkItem constructor";
    setAcceptedMouseButtons(Qt::RightButton);
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

    // Trigger synchronize with render thread
    update();
}

void QVtkItem::mousePressEvent(QMouseEvent *event) {
    qDebug() << "QVtkItem::mousePressEvent";
    if (event->buttons() & Qt::RightButton) {
        mouseButtonEvent_ = std::make_shared<QMouseEvent>(*event);
        mouseButtonEvent_->ignore();
        event->accept();

        update();
    }
}

void QVtkItem::mouseReleaseEvent(QMouseEvent *event) {
    qDebug() << "QVtkItem::mouseReleaseEvent";
    mouseButtonEvent_ = std::make_shared<QMouseEvent>(*event);
    mouseButtonEvent_->ignore();
    event->accept();

    update();
}

void QVtkItem::mouseMoveEvent(QMouseEvent *event) {
    qDebug() << "QVtkItem::mouseMoveEvent";
    mouseMoveEvent_ = std::make_shared<QMouseEvent>(*event);
    mouseMoveEvent_->ignore();
    event->accept();

    update();
}

