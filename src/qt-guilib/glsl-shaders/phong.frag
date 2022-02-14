#version 330
/*
Adapted from phong shader demo at http://www.cs.toronto.edu/~jacobson/phong-demo/
*/
/// precision MEDIUMP float;  // Generates a syntax error on non-embedded OpenGL
varying vec3 normalInterp;  // Surface normal
varying vec3 vertPos;       // Vertex position
uniform int mode;   // Rendering mode
uniform float u_Ka;   // Ambient reflection coefficient
uniform float u_Kd;   // Diffuse reflection coefficient
uniform float u_Ks;   // Specular reflection coefficient
uniform float u_shininess; // Shininess

// Material color
uniform vec3 u_ambientColor;
/// uniform vec3 u_diffuseColor;
uniform vec3 u_specularColor;
uniform vec3 u_lightPos; // Light position

varying vec4 diffuseColor;

void main() {
  vec3 N = normalize(normalInterp);
  vec3 L = normalize(u_lightPos - vertPos);

  // Lambert's cosine law
  float lambertian = max(dot(N, L), 0.0);
  float specular = 0.0;
  if(lambertian > 0.0) {
    vec3 R = reflect(-L, N);      // Reflected light vector
    vec3 V = normalize(-vertPos); // Vector to viewer
    // Compute the specular term
    float specAngle = max(dot(R, V), 0.0);
    specular = pow(specAngle, u_shininess);
  }

  vec3 dColor = vec3(diffuseColor.x, diffuseColor.y, diffuseColor.z);
  gl_FragColor = vec4(u_Ka * u_ambientColor + u_Kd * lambertian * dColor +
       u_Ks * specular * u_specularColor, 1.0);

  // only ambient
  if(mode == 2) gl_FragColor = vec4(u_Ka * u_ambientColor, 1.0);
  // only diffuse
  if(mode == 3) gl_FragColor = vec4(u_Kd * lambertian * dColor, 1.0);
  // only specular
  if(mode == 4) gl_FragColor = vec4(u_Ks * specular * u_specularColor, 1.0);
}
