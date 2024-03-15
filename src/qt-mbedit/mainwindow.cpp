#include <iostream>
#include <QFileDialog>
#include <QDebug>
#include <QDir>
#include "mainwindow.h"
#include "./ui_mainwindow.h"

using namespace mb_system;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    canvas_ = new QPixmap();
    painter_ = new QPainter(canvas_);
}

MainWindow::~MainWindow()
{
  // Free unneeded memory
    delete ui;
}



void MainWindow::on_xtrackWidthSlider_sliderMoved(int position)
{
    std::cerr << "xtrackWidth = " << position << "\n";
}


void MainWindow::on_nPingsShowSlider_sliderMoved(int position)
{
    std::cerr << "nPingsShown = " << position << "\n";
}


void MainWindow::on_vertExaggSlider_sliderMoved(int position)
{
    std::cerr << "vertExagg = " << position << "\n";
}


void MainWindow::on_actionOpen_swath_file_triggered()
{
  std::cerr << "select swath file\n";
  QString fileName = QFileDialog::getOpenFileName(this,
						  tr("Open swath file"),
						  QDir::homePath(), tr("swath files (*.m*)"));

  qDebug() << "open swath file " << fileName;
  std::string utf8_text = fileName.toUtf8().constData();
  std::cerr << "utf8_text: " << utf8_text << "\n";
  char *fname = (char *)utf8_text.c_str();
  std::cerr << "fname: " << fname << "\n";
  if (!grid_.readDatafile(fname)) {
    std::cerr << "Error reading file " << fname << std::endl;
    return;
  }

    // Draw swath data to pixmap
    plot(painter_, &grid_);
      
    // Add pixmap to UI label
    qDebug() << "Draw on GUI\n";
    ui->swathCanvas->setPixmap(*canvas_);
}


bool MainWindow::plot(QPainter *painter, SwathGridData *grid) {
  qDebug() << "plot()";
  return true;
}


