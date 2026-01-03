
uniform sampler2D texture;

void main() {
    vec2 flippedUV = vec2(gl_TexCoord[0].x, gl_TexCoord[0].y);
    gl_FragColor = texture2D(texture, flippedUV);
}
