/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Data Visualization module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <sys/stat.h>
#include <unistd.h>
#include <algorithm>
#include <cmath>
#include "TopographicSeries.h"

using namespace QtDataVisualization;

//! [0]
// Value used to encode height data as RGB value on PNG file
const float packingFactor = 11983.0f;
//! [0]

const float darkRedPos = 1.0f;
const float redPos = 0.8f;
const float yellowPos = 0.6f;
const float greenPos = 0.4f;
const float darkGreenPos = 0.2f;

TopographicSeries::TopographicSeries()
{
  resetDataLimits();

    setDrawMode(QSurface3DSeries::DrawSurface);
    setFlatShadingEnabled(true);
    toggleColorMap(true);
    // setBaseColor(Qt::white);
}

TopographicSeries::~TopographicSeries()
{
    // Destructor
}



void TopographicSeries::setTopography(void *gmtApi, GMT_GRID *grid)
{
  // Reset min/max latitude, longitude, height
  resetDataLimits();

  const int nRows = grid->header->n_rows;         // Number of latitudes
  const int nColumns = grid->header->n_columns;   // Number of longitudes

  // Latitudes
  double *latit = grid->y;

  // Longitudes
  double *longit = grid->x;

  // This holds the data
  QSurfaceDataArray *dataArray = new QSurfaceDataArray();

  // Reserve space for rows (latitudes)
  dataArray->reserve(nRows);

  for (int row = 0; row < nRows; row++) {
    QSurfaceDataRow *newRow = new QSurfaceDataRow(nColumns);
    for (int col = 0; col < nColumns; col++) {

      int index = GMT_Get_Index(gmtApi, grid->header, row, col);
      float dataValue = (float )grid->data[index];
      if (std::isnan(dataValue)) {
	fprintf(stderr, "setTopography(): NAN at row %d col %d\n", row, col);
      }

      (*newRow)[col].setPosition(QVector3D(longit[col], dataValue, latit[row]));

      // Check longitude range
      m_minLongit = std::min<float>(m_minLongit, longit[col]);
      m_maxLongit = std::max<float>(m_maxLongit, longit[col]);

      // Check data value range
      m_minHeight = std::min<float>(m_minHeight, dataValue);
      m_maxHeight = std::max<float>(m_maxHeight, dataValue);

    }
    *dataArray << newRow;

    // Check latitude range
    m_minLatit = std::min<float>(m_minLatit, latit[row]);
    m_maxLatit = std::max<float>(m_maxLatit, latit[row]);


    // delete newRow;  // Delete here causes crash in addSeries()
  }

  // Set Surface3DSeries data
  dataProxy()->resetArray(dataArray);
  // delete dataArray; // Delete here causes crash in addSeries()

}


void TopographicSeries::setTopography(void *gmtApi, GMT_GRID *grid, float width, float height)
{
  // Reset min/max latitude, longitude, height
  resetDataLimits();

  const int nRows = 4;         // Number of latitudes
  const int nColumns = 4;   // Number of longitudes

  // Latitudes
  float latit[nRows] = {10., 11., 12, 13 };

  // Longitudes
  float longit[nColumns] = {20., 21., 22., 23.};


  // Data in row-major order
  float data[nRows * nColumns] = {4.75, 3.00, 1.24, 2.53,
		    2.55, 2.03, 3.46, 5.12,
            1.37, 10., 14., 4,
		    4.34, 3.54, 1.65, 2.67};

  /* ***
  // Data in row-major order
  float data[nRows * nColumns] = {5., 5., 5., 5.,
				  5., 5., 5., 5.,
				  5., 5., 5., 5.,
				  5., 5., 5., 5.};

*** */

  //  float latHeight = latit[nRows-1] - latit[0];
  // float lonWidth = longit[nColumns-1] - longit[0];

  // This holds the data
  QSurfaceDataArray *dataArray = new QSurfaceDataArray();

  // Reserve space for rows (latitudes)
  dataArray->reserve(nRows);

  for (int row = 0; row < nRows; row++) {
    QSurfaceDataRow *newRow = new QSurfaceDataRow(nColumns);
    for (int col = 0; col < nColumns; col++) {
      int index = getRowMajorIndex(row, col, nColumns);
      // int index = getColMajorIndex(row, col, nRows);
      float dataValue = data[index];

      qDebug() << "lon lat value: " << longit[col] << " " << latit[row] << " " << dataValue;
      //      (*newRow)[col].setPosition(QVector3D(longit[col], latit[row], dataValue));
      (*newRow)[col].setPosition(QVector3D(longit[col], dataValue, latit[row]));

      // Set data ranges
      m_minLatit = std::min<float>(m_minLatit, latit[row]);
      m_maxLatit = std::max<float>(m_minLatit, latit[row]);

      m_minLongit = std::min<float>(m_minLongit, longit[col]);
      m_maxLongit = std::max<float>(m_maxLongit, longit[col]);

      m_minHeight = std::min<float>(m_minHeight, dataValue);
      m_maxHeight = std::max<float>(m_maxHeight, dataValue);

    }
    *dataArray << newRow;

    // delete newRow;  // Delete here causes crash in addSeries()
  }

  // Set Surface3DSeries data
  dataProxy()->resetArray(dataArray);
  // delete dataArray; // Delete here causes crash in addSeries()

  /* **
        ListElement{ longitude: "20"; latitude: "10"; pop_density: "4.75"; }
        ListElement{ longitude: "21"; latitude: "10"; pop_density: "3.00"; }
        ListElement{ longitude: "22"; latitude: "10"; pop_density: "1.24"; }
        ListElement{ longitude: "23"; latitude: "10"; pop_density: "2.53"; }
        ListElement{ longitude: "20"; latitude: "11"; pop_density: "2.55"; }
        ListElement{ longitude: "21"; latitude: "11"; pop_density: "2.03"; }
        ListElement{ longitude: "22"; latitude: "11"; pop_density: "3.46"; }
        ListElement{ longitude: "23"; latitude: "11"; pop_density: "5.12"; }
        ListElement{ longitude: "20"; latitude: "12"; pop_density: "1.37"; }
        ListElement{ longitude: "21"; latitude: "12"; pop_density: "2.98"; }
        ListElement{ longitude: "22"; latitude: "12"; pop_density: "3.33"; }
        ListElement{ longitude: "23"; latitude: "12"; pop_density: "3.23"; }
        ListElement{ longitude: "20"; latitude: "13"; pop_density: "4.34"; }
        ListElement{ longitude: "21"; latitude: "13"; pop_density: "3.54"; }
        ListElement{ longitude: "22"; latitude: "13"; pop_density: "1.65"; }
        ListElement{ longitude: "23"; latitude: "13"; pop_density: "2.67"; }
	** */


}

/* **
void TopographicSeries::setTopography(void *gmtApi, GMT_GRID *grid)
{
  int imageHeight = grid->header->n_rows;
  int imageWidth = grid->header->n_columns;
  float stepX = widthLon / float(imageWidth);
  float stepZ = heightLat / float(imageHeight);

  QSurfaceDataArray *dataArray = new QSurfaceDataArray;

  dataArray->reserve(imageHeight);
  for (int row = 0; row < imageHeight; row++) {
      float z = (heightLat - float(row) * stepZ) + grid->header->wesn[2];
      QSurfaceDataRow *newRow = new QSurfaceDataRow(imageWidth);
      for (int col = 0; col < imageWidth; col++) {
          //int index = row * imageWidth + col;
          int index = GMT_Get_Index(gmtApi, grid->header, row, col);
	  float y = grid->data[index];
          // (QSurface properly ignores NaN)
          float x = float(col) * stepX + grid->header->wesn[0];

          (*newRow)[col].setPosition(QVector3D(x, y, z));
      }
      *dataArray << newRow;
  }


    dataProxy()->resetArray(dataArray);
//! [1]

}
** */


GMT_GRID *TopographicSeries::readGridFile(const char *gridFile, void **api) {
    fprintf(stderr, "readGridFile(): gridFile: %s\n", gridFile);
    // Check for file existence and readability
    struct stat fileStatus;

    if (stat(gridFile, &fileStatus) != 0
            || (fileStatus.st_mode & S_IFMT) == S_IFDIR
            || fileStatus.st_size <= 0) {
        qCritical() << "Can not read \"" << gridFile << "\"";
        return nullptr;
    }

    fprintf(stderr, "readGridFile(): create session\n");
    // Create GMT API
    *api =
            GMT_Create_Session("Topography::loadGrid()", 2U, 0U, nullptr);

    if (!*api) {
        qCritical() << "Could not get GMT API for \"" << gridFile << "\"";
        return nullptr;
    }

    fprintf(stderr, "gridFile now: %s\n", gridFile);

    GMT_GRID *grid = nullptr;
    // Try to read header and grid
    for (int nTry = 0; nTry < 100; nTry++) {
        grid = (struct GMT_GRID *)GMT_Read_Data(*api, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE,
                                                GMT_GRID_ALL, nullptr, gridFile, nullptr);
        if (grid) break;
        usleep(1000);
    }

    if (!grid) {
        qCritical() << "Unable to read GMT grid from \"" << gridFile << "\"";
        return nullptr;
    }
    return grid;
}

void TopographicSeries::toggleColorMap(bool on) {

    if (on) {
        float ratio = 1.f;

        QLinearGradient gr;
        gr.setColorAt(0.0f, Qt::black);
        gr.setColorAt(darkGreenPos * ratio, Qt::darkGreen);
        gr.setColorAt(greenPos * ratio, Qt::green);
        gr.setColorAt(yellowPos * ratio, Qt::yellow);
        gr.setColorAt(redPos * ratio, Qt::red);
        gr.setColorAt(darkRedPos * ratio, Qt::darkRed);

        setBaseGradient(gr);
        setColorStyle(Q3DTheme::ColorStyleRangeGradient);
    }
    else {
        setBaseColor(Qt::white);
    }
}
