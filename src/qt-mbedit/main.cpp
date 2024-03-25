#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    int nInputSpecified = 0;
    w.mbedit_init(argc, argv, &nInputSpecified);
    w.show();
    return a.exec();
}
