#include "PixmapImage.h"

PixmapImage::PixmapImage(QQuickItem *parent) :
  QQuickPaintedItem(parent) {
}

void PixmapImage::setImage(QObject *pixmapContainer)
{
    PixmapContainer *pc = qobject_cast<PixmapContainer*>(pixmapContainer);
    Q_ASSERT(pc);
    pixmapContainer_.pixmap = pc->pixmap;
    update();
}

void PixmapImage::paint(QPainter *painter)
{
    painter->drawPixmap(0, 0, width(), height(), pixmapContainer_.pixmap);
}

