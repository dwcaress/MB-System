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

#define MAX_QVECTOR_BYTES 2147483647

using namespace QtDataVisualization;

const float darkRedPos = 1.0f;
const float redPos = 0.8f;
const float yellowPos = 0.6f;
const float greenPos = 0.4f;
const float darkGreenPos = 0.2f;

TopographicSeries::TopographicSeries() :
  m_dataArray(nullptr)
{
  resetDataLimits();

    setDrawMode(QSurface3DSeries::DrawSurface);
    setFlatShadingEnabled(true);
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
  const int nCols = grid->header->n_columns;   // Number of longitudes

  size_t bytesNeeded = nRows * nCols * sizeof(QVector3D);
  qDebug() << "need " << bytesNeeded << " bytes";
  qDebug() << "max QVector size: " << MAX_QVECTOR_BYTES;

  // Might need to subsample grid data to stay within QVector limits
  int subInterval = 1;      // subsample interval
  while ((nRows/subInterval + nRows % subInterval) *
	 (nCols/subInterval + nCols % subInterval) *
	 sizeof(QVector3D) > MAX_QVECTOR_BYTES/16) {
    // Dataset too big for QVector; increase subsample interval
    subInterval++;
  }


  qDebug() << "**** subInterval: " << subInterval;
  
  int nSubRows =
    nRows / subInterval + nRows % subInterval;  // number of subsampled rows

  int nSubCols =
    nCols / subInterval + nCols % subInterval;  // number of subsampled cols
  
  // Latitudes
  double *latit = grid->y;

  // Longitudes
  double *longit = grid->x;

  // Free data array before allocating a new one
  if (m_dataArray) {
    delete m_dataArray;
  }

  // This holds the data
  m_dataArray = new QSurfaceDataArray();

  /// TEST TEST TEST
  nSubRows = nRows;
  nSubCols = nCols;
  
  // Reserve space for rows (latitudes)  
  m_dataArray->reserve(nSubRows);
  int nPoints = 0;
  for (int row = 0; row < nRows; row += subInterval) {
    QSurfaceDataRow *newRow = new QSurfaceDataRow(nSubCols);
    int subCol = 0;   // sub-sampled column
    for (int col = 0; col < nCols; col += subInterval) {

      int index = GMT_Get_Index(gmtApi, grid->header, row, col);
      float dataValue = (float )grid->data[index];

      (*newRow)[subCol++].setPosition(QVector3D(longit[col], dataValue, latit[row]));
      nPoints++;
      
      // Check longitude range
      m_minLongit = std::min<double>(m_minLongit, longit[col]);
      m_maxLongit = std::max<double>(m_maxLongit, longit[col]);

      // Check data value range
      m_minHeight = std::min<double>(m_minHeight, dataValue);
      m_maxHeight = std::max<double>(m_maxHeight, dataValue);

    }
    *m_dataArray << newRow;

    // Check latitude range
    m_minLatit = std::min<double>(m_minLatit, latit[row]);
    m_maxLatit = std::max<double>(m_maxLatit, latit[row]);


    // delete newRow;  // Delete here causes crash in addSeries()
  }
  
  qDebug() << "**** nPoints: " << nPoints;

  // Set Surface3DSeries data
  dataProxy()->resetArray(m_dataArray);
  // delete dataArray; // Delete here causes crash in addSeries()


}



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



