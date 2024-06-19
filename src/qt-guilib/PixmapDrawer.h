#ifndef PIXMAPDRAWER_H
#define PIXMAPDRAWER_H

#include <QString>
#include <QColor>
#include <QPainter>
#include <QPixmap>
#include <QFontMetrics>
#include "mb_color.h"

namespace mb_system {

  ///
  /// Primitive methods for drawing shapes into QPixmap
  class PixmapDrawer {

  public:

    PixmapDrawer(QPainter *painter);
    
    static void drawLine(QPainter *painter, int x1, int y1, int x2, int y2,
			 DrawingColor color, int style);
  
    static void drawRect(QPainter *painter, int x, int y, int width, int height,
			 DrawingColor color, int style);
  
    static void fillRect(QPainter *painter, int x, int y, int width, int height,
			 DrawingColor color, int style);

    static void drawString(QPainter *painter, int x, int y, char *string,
			   DrawingColor color, int style);
  
    static void justifyString(QPainter *painter, char *string, int *width,
			      int *ascent, int *descent);

    /// Return color name corresponding to specified DrawingColor
    const char colorName(DrawingColor color);
  protected:

    /// Set QPainter pen color and style
    static void setPenColorAndStyle(QPainter *painter, DrawingColor color,
				    int style);

    
    static QString textBuf_;
    static QFontMetrics *fontMetrics_;    
  };

  
}



#endif

