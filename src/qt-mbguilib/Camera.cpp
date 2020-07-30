/****************************************************************************
**
** Copyright (C) 2015 Klarälvdalens Datakonsult AB, a KDAB Group company.
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
#include <QDebug>
#include "Camera.h"

using namespace mb_system;

Camera::Camera(QObject *parent)
    : QObject(parent)
    , azimuth_(0.0)
    , elevation_(180.0)
    , distance_(1000.0)
    , xOffset_(0)
    , yOffset_(0)
    , forceRender_(false)
{
}

float Camera::azimuth() const
{
    return azimuth_;
}

float Camera::distance() const
{
    return distance_;
}

float Camera::elevation() const
{
    return elevation_;
}

int Camera::xOffset() const {
  return xOffset_;
}


int Camera::yOffset() const {
  return yOffset_;
}


bool Camera::forceRender() const {
  return forceRender_;
}


void Camera::setAzimuth(float azimuth)
{
    if (azimuth_ == azimuth)
        return;

    azimuth_ = azimuth;
    qDebug() << "emit azimuthChanged(offset)";            
    emit azimuthChanged(azimuth);
}

void Camera::setDistance(float distance)
{
  // qDebug() << "Camera::setDistance() to " << distance;
  if (distance_ == distance)
        return;

    distance_ = distance;
    qDebug() << "emit distanceChanged(offset)";        
    emit distanceChanged(distance);
}

void Camera::setElevation(float elevation)
{
    if (elevation_ == elevation)
        return;

    elevation_ = elevation;
    qDebug() << "emit elevationChanged(offset)";    
    emit elevationChanged(elevation);
}


void Camera::setXOffset(float offset) {
  qDebug() << "Camera::setXOffset(): " << offset;
  if (xOffset_ == offset) {
    return;
  }
  xOffset_ = offset;
  qDebug() << "need to emit xOffsetChanged(offset)";
  emit xOffsetChanged(offset); // Needed? See NOTIFY property
}


void Camera::setYOffset(float offset) {
  if (yOffset_ == offset) {
    return;
  }
  yOffset_ = offset;
  qDebug() << "emit yOffsetChanged(offset)";  
  emit yOffsetChanged(offset);
}


void Camera::setForceRender(bool force) {
  forceRender_ = force;
  emit forceRenderChanged(force);
}


void Camera::setMaxDistance(float maxDistance) {
  qDebug() << "Camera::setMaxDistance(): " << maxDistance;
  maxDistance_ = maxDistance;

}
