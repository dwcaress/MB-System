#ifndef GuiNames_H
#define GuiNames_H

#include <QObject>
#include <QString>
#include <QDebug>



class GuiNames : public QObject { 

  Q_OBJECT

public:
  
  enum class PlotOption {
    Depth, Speed, Heading
  };

  Q_ENUM(PlotOption)

  public:
  
  Q_INVOKABLE QString objectName(PlotOption option) {
    qDebug() << "objectName() option=" << option;
    
    switch (option) {
    case PlotOption::Depth:
      return "depth";

    case PlotOption::Speed:
      return "speed";

    case PlotOption::Heading:
      return "heading";

    default:
      qDebug() << option << ": unknown PlotOption!";
      return "UNKNOWN";
    }

  }
};


#endif
 
