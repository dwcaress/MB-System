#include <iostream>
#include <QTextStream>
#include <QtGlobal>
#include "CPixmapDrawer.h"
#include "PixmapDrawer.h"

QPainter *mb_system::CPixmapDrawer::painter_ = nullptr;
QFontMetrics *mb_system::CPixmapDrawer::fontMetrics_ = nullptr;
QString mb_system::CPixmapDrawer::textBuf_;

using namespace mb_system;


CPixmapDrawer::CPixmapDrawer(QPainter *painter) {

  if (!painter) {
    qWarning("CPixmapDrawer::CPixmapDrawer(): null painter pointer");
  }
  
  painter_ = painter;
}


void CPixmapDrawer::drawLine(void *dummy,
			    int x1, int y1, int x2, int y2,
			    DrawingColor color, int style) {

  setPenColorAndStyle(color, style);
  
  painter_->drawLine(x1, y1, x2, y2);
}



void CPixmapDrawer::drawRect(void *dummy,
			  int x, int y, int width, int height,
			  DrawingColor color, int style) {

  setPenColorAndStyle(color, style);

  painter_->drawRect(x, y, width, height);
}


void CPixmapDrawer::drawString(void *dummy, int x, int y, char *string,
			    DrawingColor color, int style) {

  QString textBuf;
  QTextStream(&textBuf) << string;
  setPenColorAndStyle(color, style);
  painter_->drawText(x, y, textBuf);
}


void CPixmapDrawer::fillRect(void *dummy,
			  int x, int y, int width, int height,
			  DrawingColor color, int style) {

  setPenColorAndStyle(color, style);

  // Set fill color
  painter_->fillRect(x, y, width, height, PixmapDrawer::colorName(color));
}


void CPixmapDrawer::justifyString(void *dummy, char *string,
			       int *width, int *ascent, int *descent) {

  if (!fontMetrics_) {
    fontMetrics_ = new QFontMetrics(painter_->font());
  }
  
  *width = fontMetrics_->boundingRect(string).width();  
  *ascent = fontMetrics_->ascent();
  *descent = fontMetrics_->descent();
}


void CPixmapDrawer::setPenColorAndStyle(DrawingColor color, int style) {

  if (style == DASH_LINE) {
    painter_->setPen(Qt::DashLine);
  }
  else {
    painter_->setPen(Qt::SolidLine);    
  }
  painter_->setPen(PixmapDrawer::colorName(color));
  
}
