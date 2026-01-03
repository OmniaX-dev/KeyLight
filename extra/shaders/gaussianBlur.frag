uniform sampler2D texture;
uniform vec2 direction;
uniform float resolution;
uniform float radius;
uniform float bloomIntensity;

void main()
{
    vec2 uv = gl_TexCoord[0].xy;
    vec2 texel = direction / resolution;

    // macOS-safe weights
    float w0 = 0.227027;
    float w1 = 0.1945946;
    float w2 = 0.1216216;
    float w3 = 0.054054;
    float w4 = 0.016216;

    vec4 color = texture2D(texture, uv) * w0;

    float off;
    vec2 offset;

    off = 1.0 * radius;
    offset = texel * off;
    color += texture2D(texture, clamp(uv + offset, 0.0, 1.0)) * w1;
    color += texture2D(texture, clamp(uv - offset, 0.0, 1.0)) * w1;

    off = 2.0 * radius;
    offset = texel * off;
    color += texture2D(texture, clamp(uv + offset, 0.0, 1.0)) * w2;
    color += texture2D(texture, clamp(uv - offset, 0.0, 1.0)) * w2;

    off = 3.0 * radius;
    offset = texel * off;
    color += texture2D(texture, clamp(uv + offset, 0.0, 1.0)) * w3;
    color += texture2D(texture, clamp(uv - offset, 0.0, 1.0)) * w3;

    off = 4.0 * radius;
    offset = texel * off;
    color += texture2D(texture, clamp(uv + offset, 0.0, 1.0)) * w4;
    color += texture2D(texture, clamp(uv - offset, 0.0, 1.0)) * w4;

    gl_FragColor = color * bloomIntensity;
}