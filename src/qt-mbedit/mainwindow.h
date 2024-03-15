#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <iostream>
#include <QMainWindow>
#include <QPixmap>
#include <QPainter>

#include "/home/oreilly/MB-System/src/qt-guilib/SwathGridData.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:

  
  /// Plot swath data
  bool plot(QPainter *painter, mb_system::SwathGridData *grid);

  mb_system::SwathGridData grid_;
  
  QPixmap *canvas_;
  QPainter *painter_;

		   
		      
private slots:

    void on_xtrackWidthSlider_sliderMoved(int position);

    void on_nPingsShowSlider_sliderMoved(int position);

    void on_vertExaggSlider_sliderMoved(int position);

    void on_actionOpen_swath_file_triggered();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
