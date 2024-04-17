#ifndef PIXMAPIMAGE_H
#define PIXMAPIMAGE_H

#include <QQuickPaintedItem>
#include <QPainter>
#include "PixmapContainer.h"

class PixmapImage : public QQuickPaintedItem
{
    Q_OBJECT
public:
    PixmapImage(QQuickItem *parent = Q_NULLPTR);
    
    Q_INVOKABLE void setImage(QObject *pixmapContainer);
    
protected:
    virtual void paint(QPainter *painter) Q_DECL_OVERRIDE;

    PixmapContainer pixmapContainer_;
};

#endif // PIXMAPIMAGE_H
