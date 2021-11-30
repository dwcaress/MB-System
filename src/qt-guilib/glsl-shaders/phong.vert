#version 330

attribute vec3 vertexPosition;
attribute vec4 vertexColor;
attribute vec3 vertexNormal;

// Transformation matrices
uniform mat4 u_modelMatrix;
uniform mat4 u_viewMatrix;
uniform mat4 u_projectionMatrix;

out vec3 normalInterp;
out vec3 vNormal;
out vec4 diffuseColor;
out vec3 vFragWorldPos;
out vec3 vertPos;

void main()
{
  // Apply transformation matrix to position
  gl_Position = u_projectionMatrix * u_viewMatrix * u_modelMatrix * vec4(vertexPosition, 1.0);

  // Output
  vertPos = vec3(gl_Position);
  normalInterp = vec3(vertexNormal);
  diffuseColor = vertexColor;
  vFragWorldPos = vec3(u_modelMatrix * vec4(vertexPosition, 1.0));
}
