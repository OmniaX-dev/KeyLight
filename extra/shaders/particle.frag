#version 130

uniform sampler2D u_texture;

void main()
{
    // lookup the pixel in the texture
    vec4 pixel = texture2D(u_texture, gl_TexCoord[0].xy);

    // multiply it by the color
    gl_FragColor = gl_Color * pixel;
}
