#ifndef TopoProfileItem_H
#define TopoProfileItem_H
#include <vector>
#include <array>
#include <QObject>
#include <QPixmap>
#include <QPainter>
#include "PixmapImage.h"

namespace mb_system {

  /**
     Display two-dimenional vertical 'slice' of TopoData
   */
  class TopoProfileItem : public QObject {
    Q_OBJECT

  public:
    TopoProfileItem(QObject *uiRoot, const char *itemName);

    /// Draw profile
    bool draw(std::vector<std::array<double, 2>> *profile);
    
  protected:

    bool initialize(QObject *uiRoot, const char *itemName);
    
    /// Vertical exaggeration
    float verticalExagg_;
    
  };
}

#endif
