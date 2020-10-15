#include <getopt.h>
#include <unistd.h>
#include <limits>
#include <stdlib.h>
#include <QtDebug>
#include <QQmlContext>
#include <QtDataVisualization/Q3DSurface>
#include <QtDataVisualization/QSurface3DSeries>
#include <QQuickItem>
#include <QCoreApplication>
#include <QValue3DAxis>
#include <QValue3DAxisFormatter>
#include <gmt/gmt.h>

#include "BackEnd.h"
#include "TopographicSeries.h"

BackEnd *BackEnd::m_instance = nullptr;

bool BackEnd::registerSingleton(int argc, char **argv, QQmlEngine *qmlEngine) {

    if (! m_instance) {
        m_instance = new BackEnd(qmlEngine);
    }
    bool error = false;
    for (int i = 1; i < argc; i++) {
        if ((!strcmp(argv[i], "-I") && i < argc-1) ||
                (i == argc -1 && argv[i][0] != '-')) {
            char *filename;
            if (i == argc-1) {
                // Last argument is grid file
                filename = argv[i];
             }
            else {
                // Argument following '-I' is grid file
                filename = argv[++i];
            }

            char *fullPath = realpath(argv[i], nullptr);
            if (!fullPath) {
                fprintf(stderr, "Grid file \"%s\" not found\n", filename);
                error = true;
                break;
            }

            QString urlstring("file://" + QString(fullPath));
            QUrl qUrl(urlstring);
            qDebug() << "registerSingleton(): urlstring - " << urlstring
                     << ", qUrl - " << qUrl;

            m_instance->setGridFile(qUrl);
            free((void *)fullPath);
        }
        else {
            fprintf(stderr, "Unknown/incomplete option: %s\n", argv[i]);
            error = true;
        }
    }
    if (error) {
        delete m_instance;
        m_instance = nullptr;
        fprintf(stderr, "usage: %s [-I gridfile]\n", argv[0]);
        return false;
    }
    QQmlContext *rootContext = qmlEngine->rootContext();
    rootContext->setContextProperty("BackEnd", m_instance);
    return true;
}


BackEnd::BackEnd(QObject *parent) : QObject(parent)
{
   // qDebug() << "*** BackEnd constructor!";
    m_gridFile = nullptr;
    m_topographicSeries = nullptr;
    m_surface = nullptr;
}

bool BackEnd::getQmlItems() {

    QObject *object =
            g_rootWindow->findChild<QObject *>("surface3D");

    if (!object) {
        qCritical() << "Couldn't find \"surface3D\" object in GUI";
        return false;
    }
    qDebug() << "Found \"surface3D\" in GUI";
    m_surface = (DeclarativeSurface *)object;

    object = g_rootWindow->findChild<QObject *>("selectedFile");

    if (!object) {
        qCritical() << "Couldn't find \"selectedFile\" object in GUI";
        return false;      
    }

    m_selectedFileText = object;
    
    return true;
}

void BackEnd::setGridFile(QUrl fileURL) {

    qDebug() << "*** setGridFile() - " << fileURL;

    bool found = false;
    for (int i = 0; i < 5000; i++) {
        if (!getQmlItems()) {
            qCritical() << "Could not find QML items";
            usleep(1000);
        }
        else {
            found = true;
            break;
        }
    }
    if (!found) {
        return;
    }

    if (m_gridFile) {
      free((void *)m_gridFile);
    }
    m_gridFile = strdup(fileURL.toLocalFile().toLatin1().data());
    
    if (m_topographicSeries) {
      // Need to remove series from surface before deleting the series
        m_surface->removeSeries(m_topographicSeries);
      
        delete m_topographicSeries;
    }
    m_topographicSeries = new TopographicSeries();

    
    void *gmtApi;
    GMT_GRID *gmtGrid =
      TopographicSeries::readGridFile(m_gridFile,
				      &gmtApi);
    if (!gmtGrid) {
        qCritical() << "Unable to open grid file " << fileURL;
        return;
    }
    qDebug() << "Opened " << fileURL;

    qDebug() << "sizeof(QVector3D) " << sizeof(QVector3D);
    size_t bytesNeeded = gmtGrid->header->n_rows * gmtGrid->header->n_columns *
      sizeof(QVector3D);
    
    qDebug() << "total " << gmtGrid->header->n_rows * gmtGrid->header->n_columns
	     << " points, " << bytesNeeded << " bytes";

    // Load data into series
    qDebug() << "call setTopography()";
    m_topographicSeries->setTopography(gmtApi, gmtGrid);
    qDebug() << "returned from setTopography()";
    
    m_topographicSeries->setItemLabelFormat(QStringLiteral("@yLabel m"));
    qDebug() << "surface3D width: " << m_surface->width();
    qDebug() << "surface3D height: " << m_surface->height();

    qDebug() << "get series list";
    QQmlListProperty<QSurface3DSeries> seriesList;

    seriesList = m_surface->seriesList();
    qDebug() << "before clear - found " << m_surface->countSeriesFunc(&seriesList) << " series";
    m_surface->clearSeriesFunc(&seriesList);
    int n = m_surface->countSeriesFunc(&seriesList);
    qDebug() << "after clear - found " << n << " series";

    // Set series axes ranges
    double min = 0, max = 50;

    m_topographicSeries->longitRange(&min, &max);
    qDebug() << "X (longit) axis min: " << min << ", max: " << max;
    QValue3DAxis *axis = m_surface->axisX();
    qDebug() << "current X (longit) min: " << axis->min() << ", max: " << axis->max();
    axis->setRange(min, max);
    if (strstr(gmtGrid->header->x_units, "meters")) {
      axis->setLabelFormat(QStringLiteral("%.0f"));
    }
    axis->setTitle(gmtGrid->header->x_units);
    axis->setTitleVisible(true);

    m_topographicSeries->heightRange(&min, &max);
    qDebug() << "Y (height) axis min: " << min << ", max: " << max;
    axis = m_surface->axisY();
    qDebug() << "current Y (height) min: " << axis->min() << ", max: " << axis->max();
    axis->setRange(min, max);
    axis->setLabelFormat(QStringLiteral("%.0f"));
    axis->setTitle(gmtGrid->header->z_units); // note qt transposes z and y
    axis->setTitleVisible(true);

    m_topographicSeries->latitRange(&min, &max);
    qDebug() << "Z (latit) axis min: " << min << ", max: " << max;
    axis = m_surface->axisZ();
    qDebug() << "current Z (latit) min: " << axis->min() << ", max: " << axis->max();
    axis->setRange(min, max);
    if (strstr(gmtGrid->header->y_units, "meters")) {
      axis->setLabelFormat(QStringLiteral("%.0f"));
    }
    axis->setTitle(gmtGrid->header->y_units);  // note qt transposes z and y
    axis->setTitleVisible(true);

    qDebug() << "flat shading supported? " << m_topographicSeries->isFlatShadingSupported();
    m_topographicSeries->setFlatShadingEnabled(true);
    qDebug() << "flat shading enabled? " << m_topographicSeries->isFlatShadingEnabled();

    qDebug() << "addSeries()";
    m_surface->addSeries(m_topographicSeries);
    qDebug() << "returned from addSeries()";

    // Freeing data array causes segfault
    // m_topographicSeries->freeDataArray();
    
    qDebug() << "after adding topo series, found " << m_surface->countSeriesFunc(&seriesList)
            << " series";


    m_selectedFileText->setProperty("text", fileURL.toLocalFile());
}

void BackEnd::test() {
  qDebug() << "*** BackEnd::test()";
}


bool BackEnd::getOptions(int argc, char **argv) {

  const char *shortOptions = "I:";
  const option longOptions[] = {
				{"grid", required_argument, nullptr, 'I'},
				{nullptr, no_argument,nullptr, 0}
  };

  bool error = false;
  int opt;
  char *endptr;

  while ((opt = getopt_long(argc, argv, shortOptions, longOptions, nullptr)) != -1) {
    switch (opt) {
    case 'I':
      m_gridFile = strdup((const char *)&endptr);
      break;

    default:
      fprintf(stderr, "Unknown option: %c\n", opt);
      error = true;
    }
  }

  if (error) {
    printUsage();
    return false;
  }
  return true;
}


void BackEnd::printUsage() {
    fprintf(stderr, "usage goes here\n");
}
