/****************************************************************************
 **
 ** Copyright (C) 2017 Klarälvdalens Datakonsult AB, a KDAB Group company.
 ** Author: Giuseppe D'Angelo
 ** Contact: info@kdab.com
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU Lesser General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU Lesser General Public License for more details.
 **
 ** You should have received a copy of the GNU Lesser General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **
 ****************************************************************************/

#ifndef CAMERA_H
#define CAMERA_H

#include <QObject>

namespace mb_system {
  /** 
      Camera holds viewpoint properties relative to viewed surface
  */
  class Camera : public QObject
  {
    Q_OBJECT

    // Connections to QML properties
    Q_PROPERTY(float azimuth READ azimuth WRITE setAzimuth NOTIFY azimuthChanged)
    Q_PROPERTY(float elevation READ elevation WRITE setElevation NOTIFY elevationChanged)
    Q_PROPERTY(float distance READ distance WRITE setDistance NOTIFY distanceChanged)
    Q_PROPERTY(float xOffset READ xOffset WRITE setXOffset NOTIFY xOffsetChanged)
    Q_PROPERTY(float yOffset READ yOffset WRITE setYOffset NOTIFY yOffsetChanged)
    Q_PROPERTY(bool forceRender READ forceRender WRITE setForceRender NOTIFY forceRenderChanged)  
  
    Q_PROPERTY(float maxDistance READ maxDistance)


  public:
    explicit Camera(QObject *parent = 0);

    /// View azimutch
    float azimuth() const;

    /// View distance
    float distance() const;

    /// View elevation
    float elevation() const;

    /// View x-offset from target center
    int xOffset() const;

    /// View y-offset from target center
    int yOffset() const;

    /// Returns value of forceRender_
    bool forceRender() const;

  
    /// Set maximum view distance
    float maxDistance() const {
      return maxDistance_;
    };

  signals:
    void azimuthChanged(float azimuth);
    void distanceChanged(float distance);
    void elevationChanged(float elevation);
    void xOffsetChanged(float offset);
    void yOffsetChanged(float offset);
    void forceRenderChanged(bool value);
					    

  public slots:
    /// Set camera azimuth
    void setAzimuth(float azimuth);
  
    /// Set camera distance
    void setDistance(float distance);
  
    /// Set camera elevation
    void setElevation(float elevation);

    /// Set maximum view distance
    void setMaxDistance(float maxDistance);

    /// Set camera x offset from target center
    void setXOffset(float offset);

    /// Set camera y offset from target center
    void setYOffset(float offset);

    /// Set value of forceRender_
    void setForceRender(bool value);
  
  private:
    float azimuth_;
    float elevation_;
    float distance_;
    float xOffset_;
    float yOffset_;
    bool forceRender_;

    /// Maximum viewing distance 
    float maxDistance_;
  };
}

#endif // CAMERA_H
