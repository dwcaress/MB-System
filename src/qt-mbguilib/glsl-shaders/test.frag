#version 330
uniform vec3 u_lightPos;       	// The position of the light in world space
  
varying vec3 vPosition;		// Interpolated position for this fragment.
varying vec4 vColor;          	// color from the vertex shader interpolated across the 
varying vec3 vNormal;         	// Interpolated normal for this fragment.
varying vec3 vFragWorldPos;     // World coordinates of fragment

// The entry point for our fragment shader.
void main()                    		
{                              
    // Will be used for attenuation.
    float distance = length(u_lightPos - vFragWorldPos);                  
	
    // Get a lighting direction vector from the light to the vertex.
    vec3 lightVector = normalize(u_lightPos - vFragWorldPos);              	

    // Calculate the dot product of the light vector and vertex normal. If the normal and light vector are
    // pointing in the same direction then it will get max illumination.
    float diffuse;

    if (gl_FrontFacing) {
        diffuse = max(dot(vNormal, lightVector), 0.0);
    } else {
    	diffuse = max(dot(-vNormal, lightVector), 0.0);
    }               	  		  													  
    // Add attenuation. 
    diffuse = diffuse * (1.0 / (1.0 + (0.10 * distance)));
    
    // Add ambient lighting
    // diffuse = diffuse + 0.3;
    diffuse = diffuse + 1.0;

    // Multiply the color by the diffuse illumination level to get final output color.
    gl_FragColor = (vColor * diffuse);                                  		
}                                                                     	

