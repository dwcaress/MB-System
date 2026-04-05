#include "PixmapImage.h"

using namespace mb_system;

PixmapImage::PixmapImage(QQuickItem *parent) :
  QQuickPaintedItem(parent) {
}

void PixmapImage::setImage(QPixmap *pixmap)
{
  pixmap_ = pixmap;
  update();
}

void PixmapImage::paint(QPainter *painter)
{
  qDebug() << "*** PixmapImage::paint()";
  painter->drawPixmap(0, 0, width(), height(), *pixmap_);  
}

