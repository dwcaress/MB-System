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
    , m_azimuth(0.0)
    , m_elevation(180.0)
    , m_distance(1000.0)
    , m_xOffset(0)
    , m_yOffset(0)
    , m_forceRender(false)
{
}

float Camera::azimuth() const
{
    return m_azimuth;
}

float Camera::distance() const
{
    return m_distance;
}

float Camera::elevation() const
{
    return m_elevation;
}

int Camera::xOffset() const {
  return m_xOffset;
}


int Camera::yOffset() const {
  return m_yOffset;
}


bool Camera::forceRender() const {
  return m_forceRender;
}


void Camera::setAzimuth(float azimuth)
{
    if (m_azimuth == azimuth)
        return;

    m_azimuth = azimuth;
    qDebug() << "emit azimuthChanged(offset)";            
    emit azimuthChanged(azimuth);
}

void Camera::setDistance(float distance)
{
  // qDebug() << "Camera::setDistance() to " << distance;
  if (m_distance == distance)
        return;

    m_distance = distance;
    qDebug() << "emit distanceChanged(offset)";        
    emit distanceChanged(distance);
}

void Camera::setElevation(float elevation)
{
    if (m_elevation == elevation)
        return;

    m_elevation = elevation;
    qDebug() << "emit elevationChanged(offset)";    
    emit elevationChanged(elevation);
}


void Camera::setXOffset(float offset) {
  qDebug() << "Camera::setXOffset(): " << offset;
  if (m_xOffset == offset) {
    return;
  }
  m_xOffset = offset;
  qDebug() << "need to emit xOffsetChanged(offset)";
  emit xOffsetChanged(offset); // Needed? See NOTIFY property
}


void Camera::setYOffset(float offset) {
  if (m_yOffset == offset) {
    return;
  }
  m_yOffset = offset;
  qDebug() << "emit yOffsetChanged(offset)";  
  emit yOffsetChanged(offset);
}


void Camera::setForceRender(bool force) {
  m_forceRender = force;
  emit forceRenderChanged(force);
}



void Camera::setMaxDistance(float maxDistance) {
  qDebug() << "Camera::setMaxDistance(): " << maxDistance;
  m_maxDistance = maxDistance;

}
