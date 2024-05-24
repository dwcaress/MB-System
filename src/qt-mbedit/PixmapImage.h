#ifndef PIXMAPIMAGE_H
#define PIXMAPIMAGE_H

#include <QQuickPaintedItem>
#include <QPainter>
#include <QPixmap>

// #include "PixmapContainer.h"
/** GUI element displays an image; can be instantiated by QML and added 
    to the QML scene graph (inherits QQuickPaintedItem). 
    Encapsulates QPixmap which holds data to be rendered 
    on screen via paint().
 */
class PixmapImage : public QQuickPaintedItem
{
  /// This macro needed to exchange info with QML
  Q_OBJECT

 public:
    PixmapImage(QQuickItem *parent = Q_NULLPTR);

    /// Set pixmap to be rendered
    void setImage(QPixmap *pixmap);  
    
protected:
  /// Trigger paint of GUI elements including encapsulated pixmap
  virtual void paint(QPainter *painter) Q_DECL_OVERRIDE;

  /// Data representation
  QPixmap *pixmap_;
  
};

#endif // PIXMAPIMAGE_H
