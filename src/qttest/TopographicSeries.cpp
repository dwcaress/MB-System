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
    // toggleColorMap(true);
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



