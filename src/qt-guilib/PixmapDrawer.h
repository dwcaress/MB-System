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
  /// Pointers to static methods of this class can be passed to and invoked
  /// by C code
  class PixmapDrawer {

  public:

    PixmapDrawer(QPainter *painter);
    
    static void drawLine(void *dummy, int x1, int y1, int x2, int y2,
		       DrawingColor color, int style);
  
    static void drawRect(void *dummy, int x, int y, int width, int height,
		       DrawingColor color, int style);
  
    static void fillRect(void *dummy, int x, int y, int width, int height,
			 DrawingColor color, int style);

    static void drawString(void *dummy, int x, int y, char *string,
			   DrawingColor color, int style);
  
    static void justifyString(void *dummy, char *string, int *width,
			      int *ascent, int *descent);

    /// Return color name corresponding to input mbedit_color
    static const char *colorName(DrawingColor color);

    /// Set QPainter static pointer member
    static void setPainter(QPainter *painter) {
      painter_ = painter;
    }
    
  protected:

    /// Set QPainter pen color and style
    static void setPenColorAndStyle(DrawingColor color, int style);

    
    static QPainter *painter_;
    static QString textBuf_;
    static QFontMetrics *fontMetrics_;    
  };

  
}



#endif

