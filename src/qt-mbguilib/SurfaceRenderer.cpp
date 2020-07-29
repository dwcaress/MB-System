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

#include "SurfaceRenderer.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions>

#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>

#include <QMatrix4x4>

#include <QDebug>

#include <cmath>
#include <QtMath>

using namespace mb_system;

const char * SurfaceRenderer::ShaderName = "phong";
const char * SurfaceRenderer::VertexAttrName = "vertexPosition";
const char * SurfaceRenderer::NormalAttrName = "vertexNormal";
const char * SurfaceRenderer::ColorAttrName = "vertexColor";
const char * SurfaceRenderer::LightPosName = "u_lightPos";
const char * SurfaceRenderer::ModelMatrixName = "u_modelMatrix";
const char * SurfaceRenderer::ViewMatrixName = "u_viewMatrix";
const char * SurfaceRenderer::ProjectionMatrixName = "u_projectionMatrix";
const char * SurfaceRenderer::AmbientReflectionName = "u_Ka";
const char * SurfaceRenderer::DiffuseReflectionName = "u_Kd";
const char * SurfaceRenderer::SpecularReflectionName = "u_Ks";
const char * SurfaceRenderer::ShininessName = "u_shininess";
const char * SurfaceRenderer::AmbientColorName = "u_ambientColor";
const char * SurfaceRenderer::DiffuseColorName = "u_diffuseColor";
const char * SurfaceRenderer::SpecularColorName = "u_specularColor";

SurfaceRenderer::SurfaceRenderer(QObject *parent)
  : QObject(parent)
  , m_surface(nullptr)
  , m_positionColorBuffer(new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer))
  , m_normalBuffer(new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer))
  , m_indicesBuffer(new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer))
  , m_shaderProgram()
  , m_vao(new QOpenGLVertexArrayObject)
  , m_indicesCount(0)
  , m_coordinateMirroring(DoNotMirrorCoordinates)
  , m_verticalExagg(1.)
  , m_azimuthDeg(0.0)
  , m_elevationDeg(180.0)
  , m_distance(500.0)
  , m_xOffset(0)
  , m_yOffset(0)
  , m_verticalFovDeg(30.)
  , m_initialized(false)
{
  qDebug() << "****SurfaceRenderer::SurfaceRenderer()";
  qDebug() << "Using shader " << ShaderName;
}

SurfaceRenderer::~SurfaceRenderer()
{
  invalidate();
  if (m_surface) {
    delete m_surface;
  }
}


void SurfaceRenderer::initialize(Surface *surface, CoordinateMirroring cm)
{
  qDebug() << "SurfaceRenderer::initialize()";
  if (!surface) {
    qInfo() << "SurfaceRenderer::initialize(): surface not yet created";
    return;
  }

  if (m_initialized) {
    qInfo() << "SurfaceRenderer::initialize(): already initialized";
  }
  
  if (m_vao->isCreated())
    return; // already initialized

  m_surface = surface;

  size_t totalBytes = 0;
  size_t nBytes = m_surface->vertices().size() * sizeof(Vertex);
  
  qDebug() << "will allocate " << m_surface->vertices().size() <<
    " vertices = " << nBytes << " bytes";

  totalBytes += nBytes;
  
  nBytes = m_surface->normals().size() * sizeof(Point3D);
  
  qDebug() << "will allocate " << m_surface->normals().size() <<
    " normals = " << nBytes << " bytes";  

  totalBytes += nBytes;
  
  nBytes = m_surface->drawingIndices().size() * sizeof(unsigned int);
  
  qDebug() << "will allocate " << m_surface->drawingIndices().size() <<
    " indices = " << nBytes << " bytes";

  totalBytes += nBytes;  
  qDebug() << "will allocate total " << totalBytes/1e9 << " GB";
  
  m_coordinateMirroring = cm;

  if (!m_vao->create())
    qFatal("Unable to create VAO");

  m_vao->bind();

  std::vector<Vertex> vertices = m_surface->vertices();
  if (!m_positionColorBuffer->create())
    qFatal("Unable to create position buffer");
  m_positionColorBuffer->bind();
  m_positionColorBuffer->setUsagePattern(QOpenGLBuffer::StaticDraw);

  if (m_verticalExagg != 1.) {
    // Apply vertical exaggeration
    for (unsigned int i = 0; i < vertices.size(); i++) {
      Point3D position = vertices[i].position();
      float z = position.z() * m_verticalExagg;
      position.setZ(z);
      vertices[i].setPosition(position);
    }
  }

  QOpenGLFunctions *functions =
    QOpenGLContext::currentContext()->functions();
  
  qDebug() << "allocate positionColorBuffer: " << vertices.size() << " elements";
  /* ***
  m_positionColorBuffer->allocate(vertices.data(),
			      vertices.size() * sizeof(Vertex));
			      *** */
  qDebug() << "allocate positionColorBuffer with glBufferData()";
  functions->glBufferData(GL_ARRAY_BUFFER,
			  vertices.size() * sizeof(Vertex),
			  vertices.data(), GL_STATIC_DRAW);
  
  const std::vector<Point3D> normals = m_surface->normals();
  m_normalBuffer->create();
  m_normalBuffer->bind();
  m_normalBuffer->setUsagePattern(QOpenGLBuffer::StaticDraw);

  qDebug() << "allocate normalBuffer: " << normals.size() << " elements";
  /* **
  m_normalBuffer->allocate(normals.data(),
			   normals.size() * sizeof(QVector3D));
  ** */
      
  qDebug() << "allocate normalBuffer with glBufferData()";
  functions->glBufferData(GL_ARRAY_BUFFER,
			  normals.size() * sizeof(Point3D),
			  normals.data(), GL_STATIC_DRAW);
  
  const std::vector<unsigned int> indices = m_surface->drawingIndices();
  m_indicesCount = indices.size();
  if (!m_indicesBuffer->create())
    qFatal("Unable to create index buffer");

  m_indicesBuffer->bind();
  m_indicesBuffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
  qDebug() << "allocate indices buffer: " << normals.size() << " elements";  
  m_indicesBuffer->allocate(indices.data(),
			    indices.size() * sizeof(unsigned int));

  qDebug() << "Done with buffer allocation";
  m_shaderProgram.reset(new QOpenGLShaderProgram);
  if (!m_shaderProgram->create()) {
    qFatal("Couldn't create shader program:\n%s",
	   m_shaderProgram->log().toLatin1().constData());
  }

  char vertShaderName[64];
  char fragShaderName[64];
  sprintf(vertShaderName, ":/glsl-shaders/%s.vert", ShaderName);
  sprintf(fragShaderName, ":/glsl-shaders/%s.frag", ShaderName);
  
  qDebug() << "using vertex shader " << vertShaderName << ", fragment shader " << fragShaderName;
  
  if (!m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex,
						vertShaderName)) {
    qFatal("Vertex shader compilation failed:\n%s",
	   m_shaderProgram->log().toLatin1().constData());
  }
  qDebug() << "vertex shader compiled ok";
  if (!m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment,
						fragShaderName)) {
    qFatal("Fragment shader compilation failed:\n%s",
	   m_shaderProgram->log().toLatin1().constData());
  }
  qDebug() << "fragment shader compiled ok";
  
  if (!m_shaderProgram->link()) {
    qFatal("Shader program link failed:\n%s",
	   m_shaderProgram->log().toLatin1().constData());    
  }
  
  m_shaderProgram->bind();

  m_positionColorBuffer->bind();

  m_shaderProgram->enableAttributeArray(VertexAttrName);
  m_shaderProgram->setAttributeBuffer(VertexAttrName, GL_FLOAT,
				      Vertex::positionOffset(),
				      Vertex::PositionTupleSize,
				      Vertex::stride());

  m_shaderProgram->enableAttributeArray(ColorAttrName);    
  m_shaderProgram->setAttributeBuffer(ColorAttrName, GL_FLOAT,
				      Vertex::colorOffset(),
				      Vertex::ColorTupleSize,
				      Vertex::stride());    

  m_normalBuffer->bind();
  m_shaderProgram->enableAttributeArray(NormalAttrName);
  int offset = 0;
  int tupleSize = 3;
  int stride = 0;
  m_shaderProgram->setAttributeBuffer(NormalAttrName, GL_FLOAT, offset,
				      tupleSize, stride);

  m_vao->release();

  m_initialized = true;
}


void SurfaceRenderer::render()
{
  qDebug() << "SurfaceRenderer::render()";
  if (!m_surface) {
    qInfo() << "SurfaceRenderer::render(): surface not yet created";
    return;
  }

  if (!m_initialized) {
    qDebug() << "SurfaceRenderer::render(): call initialize()";
    initialize(m_surface);
  }

  QOpenGLFunctions *functions = QOpenGLContext::currentContext()->functions();

  functions->glClear(GL_COLOR_BUFFER_BIT);
  if (!m_shaderProgram->bind()) {
    QString msg = m_shaderProgram->log();
    qFatal("Couldn't bind program:\n%s\n", msg.toLatin1().constData());
  }

  if (m_shaderProgram->attributeLocation(VertexAttrName) == -1) {
    qFatal("Attribute %s not found in shader", VertexAttrName);
  }

  if (m_shaderProgram->attributeLocation(ColorAttrName) == -1) {
    qFatal("Attribute %s not found in shader", ColorAttrName);
  }

  if (m_shaderProgram->attributeLocation(NormalAttrName) == -1) {
    qFatal("Attribute %s not found in shader", NormalAttrName);
  }
  
  // Get map limits
  float xMin, xMax, yMin, yMax, zMin, zMax;
  m_surface->xSpan(&xMin, &xMax);
  m_surface->ySpan(&yMin, &yMax);
  m_surface->zSpan(&zMin, &zMax);
  zMin *= m_verticalExagg;
  zMax *= m_verticalExagg;

  QMatrix4x4 modelMatrix;
  // qDebug("Set model to identity matrix");
  modelMatrix.setToIdentity();
  modelMatrix.rotate(-90, 0, 1, 0);
  
  float x, y, z;
  m_surface->center(&x, &y, &z);
  // qDebug() << "m_xOffset: " << m_xOffset << ", m_yOffset: " << m_yOffset;
  x += m_xOffset;
  y += m_yOffset;
  modelMatrix.translate(-x, -y, -z * m_verticalExagg);
  
  const float azimuthRad = qDegreesToRadians(m_azimuthDeg);
  const float elevationRad = qDegreesToRadians(m_elevationDeg);

  const QVector3D eyePosition(std::cos(elevationRad) * std::cos(azimuthRad),
			      std::sin(elevationRad),
			      -std::cos(elevationRad) * std::sin(azimuthRad));
  
  QVector3D upVector = qFuzzyCompare(m_elevationDeg, 90.0f)
    ? QVector3D(-std::cos(azimuthRad), 0, std::sin(azimuthRad))
    : QVector3D(0, 1, 0);

  QMatrix4x4 viewMatrix;
  viewMatrix.setToIdentity();
  viewMatrix.lookAt(eyePosition * m_distance,
		    QVector3D(0, 0, 0),
		    upVector);

  QMatrix4x4 projectionMatrix;
  projectionMatrix.setToIdentity();
  projectionMatrix.perspective(45.f, 0.8, 0.1, 1000000.f);

  // qDebug("send modelMatrix to shader");
  if (!setUniformValue(m_shaderProgram, ModelMatrixName, modelMatrix)) {
    qFatal("variable %s not found in shader", ModelMatrixName);
  }
  
  // qDebug("send viewMatrix to shader");
  if (!setUniformValue(m_shaderProgram, ViewMatrixName, viewMatrix)) {
    qCritical() << "WARNING: variable " << ViewMatrixName << " not found in shader";
    // return;    
  }
  
  // qDebug("send projectionMatrix to shader");
  if (!setUniformValue(m_shaderProgram, ProjectionMatrixName, projectionMatrix)) {
    qFatal("variable %s not found in shader", ProjectionMatrixName);
  }


  QVector3D ambientColor(0., 0., 0.); // shadow should be black
  if (!setUniformValue(m_shaderProgram, AmbientColorName, ambientColor)) {
    qFatal("variable %s not found in shader", AmbientColorName);
  }

  QVector3D specularColor(1.0, 1.0, 1.0);  // should be white
  if (!setUniformValue(m_shaderProgram, SpecularColorName, specularColor)) {
    qFatal("variable %s not found in shader", SpecularColorName);
  }    
  
  // QVector3D lightPos(0, 0, 10000);  // This works
  QVector3D lightPos(4000, 4000, 10000);  
  if (!setUniformValue(m_shaderProgram, LightPosName, lightPos)) {
    qFatal("variable %s not found in shader", LightPosName);
  }

  // Set reflectivities
  if (!setUniformScalarValue(m_shaderProgram, AmbientReflectionName, 0.84)) {
    qFatal("variable %s not found in shader", AmbientReflectionName);
  }

  if (!setUniformScalarValue(m_shaderProgram, DiffuseReflectionName, 1.00)) {
    qFatal("variable %s not found in shader", DiffuseReflectionName);
  }

  if (!setUniformScalarValue(m_shaderProgram, SpecularReflectionName, .2)) {
    qFatal("variable %s not found in shader", SpecularReflectionName);
    return;
  }


  if (!setUniformScalarValue(m_shaderProgram, ShininessName, 1.)) {
    qFatal("variable %s not found in shader", ShininessName);
    return;
  }        
					     
  m_vao->bind();
  
  functions->glDrawElements(GL_TRIANGLES, m_indicesCount, GL_UNSIGNED_INT, 0);

  m_vao->release();

  m_shaderProgram->release();
  glFlush();
  return;
}


void SurfaceRenderer::invalidate()
{
  qDebug() << "SurfaceRenderer::invalidate()";
  
  m_positionColorBuffer->destroy();
  m_normalBuffer->destroy();
  m_indicesBuffer->destroy();
  m_shaderProgram.reset();
  m_vao->destroy();
  m_initialized = false;
}


void SurfaceRenderer::setView(float azimuthDeg, float elevationDeg, float distance,
	     float xOffset, float yOffset) {

  m_azimuthDeg = azimuthDeg;
  m_elevationDeg = elevationDeg;
  m_distance = distance;
  m_xOffset = xOffset;
  m_yOffset = yOffset;
}




