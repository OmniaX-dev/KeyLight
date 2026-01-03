

uniform sampler2D u_texture;
uniform vec4 u_color;

void main()
{
    // lookup the pixel in the texture
    vec4 pixel = texture2D(u_texture, gl_TexCoord[0].xy);

    // multiply it by the color
    gl_FragColor = u_color * pixel;
}


