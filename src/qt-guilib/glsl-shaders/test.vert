// #version 330
// attribute vec3 a_Position;
// attribute vec3 a_Normal;
// attribute vec4 a_Color;

attribute vec3 vertexPosition;
attribute vec4 vertexColor;
attribute vec3 vertexNormal;

// Transformation matrices
uniform mat4 u_modelMatrix;
uniform mat4 u_viewMatrix;
uniform mat4 u_projectionMatrix;

out vec3 vPosition;
out vec3 vNormal;
out vec4 vColor;
out vec3 vFragWorldPos;

void main()
{
  // Apply transformation matrix to position
  gl_Position = u_projectionMatrix * u_viewMatrix * u_modelMatrix * vec4(vertexPosition, 1.0);

  // Output
  vPosition = vec3(gl_Position);
  vNormal = vec3(vertexNormal);
  vColor = vertexColor;
  vFragWorldPos = vec3(u_modelMatrix * vec4(vertexPosition, 1.0));
}
