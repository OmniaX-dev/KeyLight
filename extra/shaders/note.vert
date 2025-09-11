#version 130

// Attributes SFML sends
in vec4 position;
in vec4 color;
in vec2 texCoords;

// Outputs to fragment shader
out vec4 v_color;
out vec2 v_texCoord;

// SFML-provided transforms
uniform mat4 u_projectionMatrix;
uniform mat4 u_modelViewMatrix;

void main()
{
    // Pass through color and texcoords
    v_color = color;
    v_texCoord = texCoords;

    // Correct transform: model-view first, then projection
    gl_Position = u_projectionMatrix * u_modelViewMatrix * position;
}