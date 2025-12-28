#version 130

uniform sampler2D texture;
uniform vec2 resolution;
uniform float offset = 2.0;
uniform float intensity = 1.0;

void main() {
    vec2 uv = gl_TexCoord[0].xy;
    vec2 texel = 1.0 / resolution;

    vec4 color = texture2D(texture, uv); // Center

    // Remove +0.5 — sample at integer offsets only
    color += texture2D(texture, clamp(uv + vec2(offset, offset) * texel, 0.0, 1.0));
    color += texture2D(texture, clamp(uv + vec2(offset, -offset) * texel, 0.0, 1.0));
    color += texture2D(texture, clamp(uv + vec2(-offset, offset) * texel, 0.0, 1.0));
    color += texture2D(texture, clamp(uv + vec2(-offset, -offset) * texel, 0.0, 1.0));

    gl_FragColor = color / 5.0;
    gl_FragColor *= intensity;
}
