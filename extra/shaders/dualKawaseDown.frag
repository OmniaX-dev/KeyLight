#version 130
// precision highp float;

uniform vec2 resolution; /* Resolution Reciprocal */
uniform float offset; /* Offset multiplier for blur strength */
uniform float bloomStrength; /* bloom strength */

uniform sampler2D texture;

void main() {
    /* Dual Kawase downsample: sample center + 4 diagonal corners */
    vec2 uv = gl_TexCoord[0].xy;
    vec2 halfpixel = resolution * 0.5;
    vec2 o = halfpixel * offset;

    /* Sample center with 4x weight */
    vec4 color = texture2D(texture, uv) * 4.0;

    /* Sample 4 diagonal corners with 1x weight each */
    color += texture2D(texture, uv + vec2(-o.x, -o.y)); /* bottom-left */
    color += texture2D(texture, uv + vec2(o.x, -o.y)); /* bottom-right   */
    color += texture2D(texture, uv + vec2(-o.x, o.y)); /* top-left */
    color += texture2D(texture, uv + vec2(o.x, o.y)); /* top-right */

    /* Apply bloom strength and normalize by total weight (8) */
    gl_FragColor = (color / 8.0) * bloomStrength;
}
