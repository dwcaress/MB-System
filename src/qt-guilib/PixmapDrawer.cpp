#include <iostream>
#include <QTextStream>
#include <QtGlobal>
#include "PixmapDrawer.h"


QPainter *mb_system::PixmapDrawer::painter_ = nullptr;
QFontMetrics *mb_system::PixmapDrawer::fontMetrics_ = nullptr;
QString mb_system::PixmapDrawer::textBuf_;

using namespace mb_system;


PixmapDrawer::PixmapDrawer(QPainter *painter) {

  if (!painter) {
    qWarning("PixmapDrawer::PixmapDrawer(): null painter pointer");
  }
  
  painter_ = painter;
}


void PixmapDrawer::drawLine(QPainter *painter,
			    int x1, int y1, int x2, int y2,
			    DrawingColor color, int style) {

  setPenColorAndStyle(color, style);
  
  painter->drawLine(x1, y1, x2, y2);
}



void PixmapDrawer::drawRect(QPainter *painter,
			  int x, int y, int width, int height,
			  DrawingColor color, int style) {

  setPenColorAndStyle(color, style);

  painter->drawRect(x, y, width, height);
}


void PixmapDrawer::drawString(QPainter *painter, int x, int y, char *string,
			    DrawingColor color, int style) {

  QString textBuf;
  QTextStream(&textBuf) << string;
  setPenColorAndStyle(color, style);
  painter->drawText(x, y, textBuf);
}


void PixmapDrawer::fillRect(QPainter *painter,
			  int x, int y, int width, int height,
			  DrawingColor color, int style) {

  setPenColorAndStyle(color, style);

  // Set fill color
  painter->fillRect(x, y, width, height, colorName(color));
}


void PixmapDrawer::justifyString(QPainter *painter, char *string,
			       int *width, int *ascent, int *descent) {

  if (!fontMetrics_) {
    fontMetrics_ = new QFontMetrics(painter->font());
  }
  
  *width = fontMetrics_->width(string);
  *ascent = fontMetrics_->ascent();
  *descent = fontMetrics_->descent();
}

const char *PixmapDrawer::colorName(DrawingColor color) {
  switch (color) {
  case WHITE:
    return "white";

  case BLACK:
    return "black";

  case RED:
    return "red";

  case GREEN:
    return "green";
    
  case BLUE:
    return "blue";

  case ORANGE:
    return "orange";

  case PURPLE:
    return "purple";
    
  case CORAL:
    return "coral";

  case LIGHTGREY:
    return "lightGray";

  default:
    std::cerr << "colorName(): unknown color!\n";
    return "black";
  }  

}



void PixmapDrawer::setPenColorAndStyle(QPainter *painter, DrawingColor color,
				       int style) {

  if (style == DASH_LINE) {
    painter->setPen(Qt::DashLine);
  }
  else {
    painter->setPen(Qt::SolidLine);    
  }
  painter->setPen(colorName(color));
  
}
