#ifndef CPIXMAPDRAWER_H
#define CPIXMAPDRAWER_H

#include <QString>
#include <QColor>
#include <QPainter>
#include <QPixmap>
#include <QFontMetrics>
#include "mb_define.h"

namespace mb_system {
  /**
  Draws shapes and text into a QPainter.
  Static member functions can of course be called by C code
  and their signatures are consistent with those expected
  by C functions defined in mbedit_prog.h module. Thus mbedit_prog can
  draw using either these Qt-based
  functions (e.g. qt-mbedit application) or with X11-based C functions
  defined in mb_xgraphics.h (legacy mbedit X11/Motif application).
  The first argument of these static member functions is required
  by the mbedit_prog prototypes, but is unused by this Qt implementation. */
  class CPixmapDrawer {

  public:

    CPixmapDrawer(QPainter *painter);

    static void drawLine(void *dummy, int x1, int y1, int x2, int y2,
		       MB_DrawingColor color, int style);
  
    static void drawRect(void *dummy, int x, int y, int width, int height,
		       MB_DrawingColor color, int style);
  
    static void fillRect(void *dummy, int x, int y, int width, int height,
			 MB_DrawingColor color, int style);

    static void drawString(void *dummy, int x, int y, char *string,
			   MB_DrawingColor color, int style);
  
    static void justifyString(void *dummy, char *string, int *width,
			      int *ascent, int *descent);

    /// Set QPainter* static member (the QPainter has an associated
    /// QPixmap)
    static void setPainter(QPainter *painter) {
      painter_ = painter;
    }
    
  protected:

    /// Set QPainter pen color and style
    static void setPenColorAndStyle(MB_DrawingColor color, int style);

    /// Graphics are drawn through QPainter
    static QPainter *painter_;

    static QString textBuf_;
    static QFontMetrics *fontMetrics_;    
  };

  
}



#endif

