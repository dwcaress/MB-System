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

#ifndef TOPOGRAPHICSERIES_H
#define TOPOGRAPHICSERIES_H

#include <limits>
#include <QtDataVisualization/QSurface3DSeries>

#include <gmt/gmt.h>

using namespace QtDataVisualization;

/* **
   This class holds topographic/bathymetric data for Surface3D QML object
** */
class TopographicSeries : public QSurface3DSeries
{
    Q_OBJECT
public:

    explicit TopographicSeries();
    ~TopographicSeries();

  /// Load topographic data into series
  void setTopography(void *gmtApi, GMT_GRID *grid);

  /// Read grid from GMT file; return pointer to GMT_GRID if
  /// successful, else return nullptr
  static GMT_GRID *readGridFile(const char *filename, void **gmtApi);
    
  /// Toggle between color map and solid color
  void toggleColorMap(bool on);

  /// Get dataset latitude range
  void latitRange(double *min, double *max) {
      *min = m_minLatit;
      *max = m_maxLatit;
  }

  /// Get dataset longitude range
  void longitRange(double *min, double *max) {
      *min = m_minLongit;
      *max = m_maxLongit;
  }

  /// Get dataset height range
  void heightRange(double *min, double *max) {
      *min = m_minHeight;
      *max = m_maxHeight;
  }

  /// Reset min/max latitude, longitude, height
  void resetDataLimits() {
    m_minHeight = std::numeric_limits<double>::max();
    m_maxHeight = std::numeric_limits<double>::lowest();
    m_minLatit = std::numeric_limits<double>::max();
    m_maxLatit = std::numeric_limits<double>::lowest();
    m_minLongit = std::numeric_limits<double>::max();
    m_maxLongit = std::numeric_limits<double>::lowest();
  }


public Q_SLOTS:

  protected:

  double m_minLatit;
  double m_maxLatit;
  double m_minLongit;
  double m_maxLongit;
  double m_minHeight;
  double m_maxHeight;

  QSurfaceDataArray *m_dataArray;
  
};

#endif // TOPOGRAPHICSERIES_H
