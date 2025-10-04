
#version 130
uniform sampler2D texture;
uniform vec2 direction;   // (1,0) for horizontal, (0,1) for vertical
uniform float resolution; // texture width or height depending on direction
uniform float spread;     // spacing between taps (1–3 is typical)
uniform float intensity;  // brightness boost

void main() {
    vec2 uv = gl_TexCoord[0].xy;
    float weights[5] = float[](0.227, 0.194, 0.121, 0.054, 0.016);
    vec4 sum = texture2D(texture, uv) * weights[0];

    for (int i = 1; i < 5; ++i) {
        vec2 offset = direction * float(i) * spread / resolution;
        sum += texture2D(texture, uv + offset) * weights[i];
        sum += texture2D(texture, uv - offset) * weights[i];
    }

    gl_FragColor = sum * intensity;
}