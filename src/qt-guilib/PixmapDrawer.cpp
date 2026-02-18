#include <iostream>
#include <QTextStream>
#include <QtGlobal>
#include "PixmapDrawer.h"


QFontMetrics *mb_system::PixmapDrawer::fontMetrics_ = nullptr;
QString mb_system::PixmapDrawer::textBuf_;

using namespace mb_system;


PixmapDrawer::PixmapDrawer() {
}


void PixmapDrawer::drawLine(QPainter *painter,
			    int x1, int y1, int x2, int y2,
			    MB_DrawingColor color, int style) {

  setPenColorAndStyle(painter, color, style);
  
  painter->drawLine(x1, y1, x2, y2);
}



void PixmapDrawer::drawRectangle(QPainter *painter,
			  int x, int y, int width, int height,
			  MB_DrawingColor color, int style) {

  setPenColorAndStyle(painter, color, style);

  painter->drawRect(x, y, width, height);
}


void PixmapDrawer::drawString(QPainter *painter, int x, int y, char *string,
			    MB_DrawingColor color, int style) {

  QString textBuf;
  QTextStream(&textBuf) << string;
  setPenColorAndStyle(painter, color, style);
  painter->drawText(x, y, textBuf);
}


void PixmapDrawer::fillRectangle(QPainter *painter,
			  int x, int y, int width, int height,
			  MB_DrawingColor color, int style) {

  setPenColorAndStyle(painter, color, style);

  // Set fill color
  painter->fillRect(x, y, width, height, colorName(color));
}


void PixmapDrawer::justifyString(QPainter *painter, char *string,
			       int *width, int *ascent, int *descent) {

  if (!fontMetrics_) {
    fontMetrics_ = new QFontMetrics(painter->font());
  }
  
  *width = fontMetrics_->boundingRect(string).width();
  *ascent = fontMetrics_->ascent();
  *descent = fontMetrics_->descent();
}

const char *PixmapDrawer::colorName(MB_DrawingColor color) {
  switch (color) {
  case MB_WHITE:
    return "white";

  case MB_BLACK:
    return "black";

  case MB_RED:
    return "red";

  case MB_GREEN:
    return "green";
    
  case MB_BLUE:
    return "blue";

  case MB_ORANGE:
    return "orange";

  case MB_PURPLE:
    return "purple";
    
  case MB_CORAL:
    return "coral";

  case MB_LIGHTGREY:
    return "lightGray";

  default:
    std::cerr << "colorName(): unknown color!\n";
    return "black";
  }  

}



void PixmapDrawer::setPenColorAndStyle(QPainter *painter, MB_DrawingColor color,
				       int style) {

  if (style == MB_DASH_LINE) {
    painter->setPen(Qt::DashLine);
  }
  else {
    painter->setPen(Qt::SolidLine);    
  }
  painter->setPen(colorName(color));
  
}
