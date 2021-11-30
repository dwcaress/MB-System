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
  , surface_(nullptr)
  , positionColorBuffer_(new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer))
  , normalBuffer_(new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer))
  , indicesBuffer_(new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer))
  , shaderProgram_()
  , vao_(new QOpenGLVertexArrayObject)
  , indicesCount_(0)
  , coordinateMirroring_(DoNotMirrorCoordinates)
  , verticalExagg_(1.)
  , azimuthDeg_(0.0)
  , elevationDeg_(180.0)
  , distance_(500.0)
  , xOffset_(0)
  , yOffset_(0)
  , verticalFovDeg_(30.)
  , initialized_(false)
{
  qDebug() << "****SurfaceRenderer::SurfaceRenderer()";
  qDebug() << "Using shader " << ShaderName;
}

SurfaceRenderer::~SurfaceRenderer()
{
  invalidate();
  if (surface_) {
    delete surface_;
  }
}


void SurfaceRenderer::initialize(Surface *surface, CoordinateMirroring cm)
{
  qDebug() << "SurfaceRenderer::initialize()";
  if (!surface) {
    qInfo() << "SurfaceRenderer::initialize(): surface not yet created";
    return;
  }

  if (initialized_) {
    qInfo() << "SurfaceRenderer::initialize(): already initialized";
  }
  
  if (vao_->isCreated())
    return; // already initialized

  surface_ = surface;

  size_t totalBytes = 0;
  size_t nBytes = surface_->vertices().size() * sizeof(Vertex);
  
  qDebug() << "will allocate " << surface_->vertices().size() <<
    " vertices = " << nBytes << " bytes";

  totalBytes += nBytes;
  
  nBytes = surface_->normals().size() * sizeof(Point3D);
  
  qDebug() << "will allocate " << surface_->normals().size() <<
    " normals = " << nBytes << " bytes";  

  totalBytes += nBytes;
  
  nBytes = surface_->drawingIndices().size() * sizeof(unsigned int);
  
  qDebug() << "will allocate " << surface_->drawingIndices().size() <<
    " indices = " << nBytes << " bytes";

  totalBytes += nBytes;  
  qDebug() << "will allocate total " << totalBytes/1e9 << " GB";
  
  coordinateMirroring_ = cm;

  if (!vao_->create())
    qFatal("Unable to create VAO");

  vao_->bind();

  std::vector<Vertex> vertices = surface_->vertices();
  if (!positionColorBuffer_->create())
    qFatal("Unable to create position buffer");
  positionColorBuffer_->bind();
  positionColorBuffer_->setUsagePattern(QOpenGLBuffer::StaticDraw);

  if (verticalExagg_ != 1.) {
    // Apply vertical exaggeration
    for (unsigned int i = 0; i < vertices.size(); i++) {
      Point3D position = vertices[i].position();
      float z = position.z() * verticalExagg_;
      position.setZ(z);
      vertices[i].setPosition(position);
    }
  }

  QOpenGLFunctions *functions =
    QOpenGLContext::currentContext()->functions();
  
  qDebug() << "allocate positionColorBuffer: " << vertices.size() << " elements";
  /* ***
  positionColorBuffer_->allocate(vertices.data(),
			      vertices.size() * sizeof(Vertex));
			      *** */
  qDebug() << "allocate positionColorBuffer with glBufferData()";
  functions->glBufferData(GL_ARRAY_BUFFER,
			  vertices.size() * sizeof(Vertex),
			  vertices.data(), GL_STATIC_DRAW);
  
  const std::vector<Point3D> normals = surface_->normals();
  normalBuffer_->create();
  normalBuffer_->bind();
  normalBuffer_->setUsagePattern(QOpenGLBuffer::StaticDraw);

  qDebug() << "allocate normalBuffer: " << normals.size() << " elements";
  /* **
  normalBuffer_->allocate(normals.data(),
			   normals.size() * sizeof(QVector3D));
  ** */
      
  qDebug() << "allocate normalBuffer with glBufferData()";
  functions->glBufferData(GL_ARRAY_BUFFER,
			  normals.size() * sizeof(Point3D),
			  normals.data(), GL_STATIC_DRAW);
  
  const std::vector<unsigned int> indices = surface_->drawingIndices();
  indicesCount_ = indices.size();
  if (!indicesBuffer_->create())
    qFatal("Unable to create index buffer");

  indicesBuffer_->bind();
  indicesBuffer_->setUsagePattern(QOpenGLBuffer::StaticDraw);
  qDebug() << "allocate indices buffer: " << normals.size() << " elements";  
  indicesBuffer_->allocate(indices.data(),
			    indices.size() * sizeof(unsigned int));

  qDebug() << "Done with buffer allocation";
  shaderProgram_.reset(new QOpenGLShaderProgram);
  if (!shaderProgram_->create()) {
    qFatal("Couldn't create shader program:\n%s",
	   shaderProgram_->log().toLatin1().constData());
  }

  char vertShaderName[64];
  char fragShaderName[64];
  // May need to modify shader inclusion strategy when using cmake.
  // See https://community.khronos.org/t/glsl-shaders-and-cmake/70653/3
  sprintf(vertShaderName, ":/glsl-shaders/%s.vert", ShaderName);
  sprintf(fragShaderName, ":/glsl-shaders/%s.frag", ShaderName);
  
  qDebug() << "using vertex shader " << vertShaderName << ", fragment shader " << fragShaderName;
  
  if (!shaderProgram_->addShaderFromSourceFile(QOpenGLShader::Vertex,
						vertShaderName)) {
    qFatal("Vertex shader compilation failed:\n%s",
	   shaderProgram_->log().toLatin1().constData());
  }
  qDebug() << "vertex shader compiled ok";
  if (!shaderProgram_->addShaderFromSourceFile(QOpenGLShader::Fragment,
						fragShaderName)) {
    qFatal("Fragment shader compilation failed:\n%s",
	   shaderProgram_->log().toLatin1().constData());
  }
  qDebug() << "fragment shader compiled ok";
  
  if (!shaderProgram_->link()) {
    qFatal("Shader program link failed:\n%s",
	   shaderProgram_->log().toLatin1().constData());    
  }
  
  shaderProgram_->bind();

  positionColorBuffer_->bind();

  shaderProgram_->enableAttributeArray(VertexAttrName);
  shaderProgram_->setAttributeBuffer(VertexAttrName, GL_FLOAT,
				      Vertex::positionOffset(),
				      Vertex::PositionTupleSize,
				      Vertex::stride());

  shaderProgram_->enableAttributeArray(ColorAttrName);    
  shaderProgram_->setAttributeBuffer(ColorAttrName, GL_FLOAT,
				      Vertex::colorOffset(),
				      Vertex::ColorTupleSize,
				      Vertex::stride());    

  normalBuffer_->bind();
  shaderProgram_->enableAttributeArray(NormalAttrName);
  int offset = 0;
  int tupleSize = 3;
  int stride = 0;
  shaderProgram_->setAttributeBuffer(NormalAttrName, GL_FLOAT, offset,
				      tupleSize, stride);

  vao_->release();

  initialized_ = true;
}


void SurfaceRenderer::render()
{
  qDebug() << "SurfaceRenderer::render()";
  if (!surface_) {
    qInfo() << "SurfaceRenderer::render(): surface not yet created";
    return;
  }

  if (!initialized_) {
    qDebug() << "SurfaceRenderer::render(): call initialize()";
    initialize(surface_);
  }

  QOpenGLFunctions *functions = QOpenGLContext::currentContext()->functions();

  functions->glClear(GL_COLOR_BUFFER_BIT);
  if (!shaderProgram_->bind()) {
    QString msg = shaderProgram_->log();
    qFatal("Couldn't bind program:\n%s\n", msg.toLatin1().constData());
  }

  if (shaderProgram_->attributeLocation(VertexAttrName) == -1) {
    qFatal("Attribute %s not found in shader", VertexAttrName);
  }

  if (shaderProgram_->attributeLocation(ColorAttrName) == -1) {
    qFatal("Attribute %s not found in shader", ColorAttrName);
  }

  if (shaderProgram_->attributeLocation(NormalAttrName) == -1) {
    qFatal("Attribute %s not found in shader", NormalAttrName);
  }
  
  // Get map limits
  float xMin, xMax, yMin, yMax, zMin, zMax;
  surface_->xSpan(&xMin, &xMax);
  surface_->ySpan(&yMin, &yMax);
  surface_->zSpan(&zMin, &zMax);
  zMin *= verticalExagg_;
  zMax *= verticalExagg_;

  QMatrix4x4 modelMatrix;
  // qDebug("Set model to identity matrix");
  modelMatrix.setToIdentity();
  modelMatrix.rotate(-90, 0, 1, 0);
  
  float x, y, z;
  surface_->center(&x, &y, &z);
  // qDebug() << "xOffset_: " << xOffset_ << ", yOffset_: " << yOffset_;
  x += xOffset_;
  y += yOffset_;
  modelMatrix.translate(-x, -y, -z * verticalExagg_);
  
  const float azimuthRad = qDegreesToRadians(azimuthDeg_);
  const float elevationRad = qDegreesToRadians(elevationDeg_);

  const QVector3D eyePosition(std::cos(elevationRad) * std::cos(azimuthRad),
			      std::sin(elevationRad),
			      -std::cos(elevationRad) * std::sin(azimuthRad));
  
  QVector3D upVector = qFuzzyCompare(elevationDeg_, 90.0f)
    ? QVector3D(-std::cos(azimuthRad), 0, std::sin(azimuthRad))
    : QVector3D(0, 1, 0);

  QMatrix4x4 viewMatrix;
  viewMatrix.setToIdentity();
  viewMatrix.lookAt(eyePosition * distance_,
		    QVector3D(0, 0, 0),
		    upVector);

  QMatrix4x4 projectionMatrix;
  projectionMatrix.setToIdentity();
  projectionMatrix.perspective(45.f, 0.8, 0.1, 1000000.f);

  // qDebug("send modelMatrix to shader");
  if (!setUniformValue(shaderProgram_, ModelMatrixName, modelMatrix)) {
    qFatal("variable %s not found in shader", ModelMatrixName);
  }
  
  // qDebug("send viewMatrix to shader");
  if (!setUniformValue(shaderProgram_, ViewMatrixName, viewMatrix)) {
    qCritical() << "WARNING: variable " << ViewMatrixName << " not found in shader";
    // return;    
  }
  
  // qDebug("send projectionMatrix to shader");
  if (!setUniformValue(shaderProgram_, ProjectionMatrixName, projectionMatrix)) {
    qFatal("variable %s not found in shader", ProjectionMatrixName);
  }


  QVector3D ambientColor(0., 0., 0.); // shadow should be black
  if (!setUniformValue(shaderProgram_, AmbientColorName, ambientColor)) {
    qFatal("variable %s not found in shader", AmbientColorName);
  }

  QVector3D specularColor(1.0, 1.0, 1.0);  // should be white
  if (!setUniformValue(shaderProgram_, SpecularColorName, specularColor)) {
    qFatal("variable %s not found in shader", SpecularColorName);
  }    
  
  // QVector3D lightPos(0, 0, 10000);  // This works
  QVector3D lightPos(4000, 4000, 10000);  
  if (!setUniformValue(shaderProgram_, LightPosName, lightPos)) {
    qFatal("variable %s not found in shader", LightPosName);
  }

  // Set reflectivities
  if (!setUniformScalarValue(shaderProgram_, AmbientReflectionName, 0.84)) {
    qFatal("variable %s not found in shader", AmbientReflectionName);
  }

  if (!setUniformScalarValue(shaderProgram_, DiffuseReflectionName, 1.00)) {
    qFatal("variable %s not found in shader", DiffuseReflectionName);
  }

  if (!setUniformScalarValue(shaderProgram_, SpecularReflectionName, .2)) {
    qFatal("variable %s not found in shader", SpecularReflectionName);
    return;
  }


  if (!setUniformScalarValue(shaderProgram_, ShininessName, 1.)) {
    qFatal("variable %s not found in shader", ShininessName);
    return;
  }        
					     
  vao_->bind();
  
  functions->glDrawElements(GL_TRIANGLES, indicesCount_, GL_UNSIGNED_INT, 0);

  vao_->release();

  shaderProgram_->release();
  glFlush();
  return;
}


void SurfaceRenderer::invalidate()
{
  qDebug() << "SurfaceRenderer::invalidate()";
  
  positionColorBuffer_->destroy();
  normalBuffer_->destroy();
  indicesBuffer_->destroy();
  shaderProgram_.reset();
  vao_->destroy();
  initialized_ = false;
}


void SurfaceRenderer::setView(float azimuthDeg, float elevationDeg, float distance,
	     float xOffset, float yOffset) {

  azimuthDeg_ = azimuthDeg;
  elevationDeg_ = elevationDeg;
  distance_ = distance;
  xOffset_ = xOffset;
  yOffset_ = yOffset;
}




