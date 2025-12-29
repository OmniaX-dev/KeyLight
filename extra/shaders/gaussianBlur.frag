#version 130

uniform sampler2D texture;
uniform vec2 direction; // (1,0) = horizontal, (0,1) = vertical
uniform float resolution; // width for horizontal, height for vertical
uniform float radius = 4.0; // blur strength
uniform float bloomIntensity = 1.0;

const float weights[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main() {
    vec2 uv = gl_TexCoord[0].xy;
    vec2 texel = direction / resolution;

    vec4 color = texture2D(texture, uv) * weights[0];

    for (int i = 1; i < 5; ++i) {
        float off = float(i) * radius;
        vec2 offset = texel * off;

        color += texture2D(texture, clamp(uv + offset, 0.0, 1.0)) * weights[i];
        color += texture2D(texture, clamp(uv - offset, 0.0, 1.0)) * weights[i];
    }

    gl_FragColor = color;
    gl_FragColor *= bloomIntensity;
}
