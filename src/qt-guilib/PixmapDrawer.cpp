#include <iostream>
#include <QTextStream>
#include "PixmapDrawer.h"


QPainter *mb_system::PixmapDrawer::painter_ = nullptr;
QFontMetrics *mb_system::PixmapDrawer::fontMetrics_ = nullptr;
QString mb_system::PixmapDrawer::textBuf_;

using namespace mb_system;

void PixmapDrawer::drawLine(void *dummy,
			  int x1, int y1, int x2, int y2,
			  DrawingColor color, int style) {

  setPenColorAndStyle(color, style);
  
  painter_->drawLine(x1, y1, x2, y2);
}


void PixmapDrawer::drawRect(void *dummy,
			  int x, int y, int width, int height,
			  DrawingColor color, int style) {

  setPenColorAndStyle(color, style);

  painter_->drawRect(x, y, width, height);
}


void PixmapDrawer::drawString(void *dummy, int x, int y, char *string,
			    DrawingColor color, int style) {

  QString textBuf;
  QTextStream(&textBuf) << string;
  setPenColorAndStyle(color, style);
  painter_->drawText(x, y, textBuf);
}


void PixmapDrawer::fillRect(void *dummy,
			  int x, int y, int width, int height,
			  DrawingColor color, int style) {

  setPenColorAndStyle(color, style);

  // Set fill color
  painter_->fillRect(x, y, width, height, colorName(color));
}



void PixmapDrawer::justifyString(void *dummy, char *string,
			       int *width, int *ascent, int *descent) {

  if (!fontMetrics_) {
    fontMetrics_ = new QFontMetrics(painter_->font());
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


void PixmapDrawer::setPenColorAndStyle(DrawingColor color, int style) {

  if (style == DASH_LINE) {
    painter_->setPen(Qt::DashLine);
  }
  else {
    painter_->setPen(Qt::SolidLine);    
  }
  painter_->setPen(colorName(color));
  
}
