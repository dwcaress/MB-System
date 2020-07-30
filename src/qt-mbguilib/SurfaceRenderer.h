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

#ifndef SURFACERENDERER_H
#define SURFACERENDERER_H

#include <QObject>

#include <QScopedPointer>
#include <QSize>
#include <QOpenGLShaderProgram>

#include "Surface.h"

class QOpenGLBuffer;
class QOpenGLVertexArrayObject;

namespace mb_system {
  
  /**
     SurfaceRenderer invokes OpenGL api functions to initialize drawing and buffers,
     set up shaders, load and paint 3D surface data.
     Based on D'Angelo's MeshRenderer at
     https://www.kdab.com/integrate-opengl-code-qt-quick-2-applications-part-2/
  */
  class SurfaceRenderer : public QObject
  {
    Q_OBJECT
  public:
    explicit SurfaceRenderer(QObject *parent = 0);
    virtual ~SurfaceRenderer();

    enum CoordinateMirroring {
			      DoNotMirrorCoordinates,
			      MirrorYCoordinate
    };

    /// Create and fill buffers with surface data
    void initialize(Surface *surface,
		    CoordinateMirroring cm = DoNotMirrorCoordinates);

    /// Draw the surface
    void render();

    /// Destroy/free buffers, reset shader program
    void invalidate();

    /// Set view parameters - azimuth, elevation, distance, etc - in "local" coordinate frame
    /// (e.g. UTM meters)
    void setView(float azimuthDeg, float elevationDeg, float distance, float xOffset, float yOffset);

    /// Return pointer to surface
    Surface *surface() {
      return surface_;
    }
  
  protected:

    /// Surface to be rendered
    Surface *surface_;

    /// Vertex buffer object holds surface vertex positions and colors
    QScopedPointer<QOpenGLBuffer> positionColorBuffer_;

    /// Vertex buffer object holds surface normal vectors
    QScopedPointer<QOpenGLBuffer> normalBuffer_;

    /// Index buffer object holds indices for triangle strips
    QScopedPointer<QOpenGLBuffer> indicesBuffer_;

    /// Shaders to render surface
    QScopedPointer<QOpenGLShaderProgram> shaderProgram_;

    /// Vertex Array Object, holds all information to render surface
    QScopedPointer<QOpenGLVertexArrayObject> vao_;

    int indicesCount_;

    CoordinateMirroring coordinateMirroring_;

    /// Vertical exaggeration 
    float verticalExagg_;
  
    /// Viewing azimuth, elevation, and distance 
    float azimuthDeg_;
    float elevationDeg_;
    float distance_;

    /// X and Y offsets from target center
    int xOffset_;
    int yOffset_;
  
    /// Vertical FOV angle for perspective projection
    float verticalFovDeg_;
  
    /// Indicates if renderer has been initialized with surface data
    bool initialized_;

    /// Helper function - set uniform value
    inline bool setUniformScalarValue(QScopedPointer<QOpenGLShaderProgram>&shader,
				      const char *name,
				      float value) {
      if (shader->uniformLocation(name) == -1) {
	return false;
      }
      shader->setUniformValue(name, value);
      return true;

    }

    /// Helper function; set shader uniform variable 'name' to value.
    /// Return false if no uniform variable with name exists in shader, else return true.
    template <typename valueType>
    inline bool setUniformValue(QScopedPointer<QOpenGLShaderProgram>&shader,
				const char *name,
				valueType value) {
      if (shader->uniformLocation(name) == -1) {
	return false;
      }
      shader->setUniformValue(name, value);
      return true;
    }

  
    // Names connect to GLSL attributes and variables
    static const char *ShaderName;
    static const char *VertexAttrName;
    static const char *ColorAttrName;
    static const char *NormalAttrName;
    static const char *LightPosName;
    static const char *ModelMatrixName;
    static const char *ViewMatrixName;
    static const char *ProjectionMatrixName;
    static const char *AmbientReflectionName;
    static const char *DiffuseReflectionName;
    static const char *SpecularReflectionName;
    static const char *ShininessName;
    static const char *AmbientColorName;
    static const char *DiffuseColorName;
    static const char *SpecularColorName;          
  
  };
}

#endif // SURFACERENDERER_H


