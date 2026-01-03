// threshold.frag


uniform sampler2D texture;
uniform float threshold; // Only glow >80% brightness

void main() {
    vec4 color = texture2D(texture, gl_TexCoord[0].xy);
    float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
    float amount = clamp((brightness - threshold) / (1.0 - threshold), 0.0, 1.0);
    gl_FragColor = color * amount;
    gl_FragColor.a = color.a; // Keep alpha
}
