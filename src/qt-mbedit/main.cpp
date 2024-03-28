#include "mainwindow.h"

extern "C" {
#  include "mbedit_prog.h"
}

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    
    // Initialize mbedit.
    // Function pointers reference functions that
    // draw to QPixmap canvas and that interact with Qt GUI
    int inputSpecified = 0;

    int canvasSize[4] = {0, 0, 0, 0};
    w.canvasSize(&canvasSize[1], &canvasSize[3]);
    
    mbedit_set_scaling(canvasSize, 0);
    mbedit_init(argc, argv, &inputSpecified,
		nullptr,
		&MainWindow::drawLine,
		&MainWindow::drawRect,
		&MainWindow::fillRect,
		&MainWindow::drawString,
		&MainWindow::justifyString,
		&MainWindow::parseDataList,
		&MainWindow::showError,
		&MainWindow::showMessage,
		&MainWindow::hideMessage,
		&MainWindow::enableFileButton,
		&MainWindow::disableFileButton,
		&MainWindow::enableNextButton,
		&MainWindow::disableNextButton,
		&MainWindow::resetScaleX);
    
    w.show();
    return a.exec();
}
