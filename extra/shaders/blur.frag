#version 130
uniform sampler2D texture;
uniform vec2 direction; // (1,0) for horizontal, (0,1) for vertical
uniform float resolution; // texture width or height depending on direction
uniform float radius;     // blur radius (e.g. 5.0)

void main() {
    vec2 uv = gl_TexCoord[0].xy;
    float weights[5] = float[](0.227, 0.194, 0.121, 0.054, 0.016);
	vec4 sum = texture2D(texture, uv) * weights[0];
	for (int i = 1; i < 5; ++i) {
		sum += texture2D(texture, uv + direction * float(i) * 15.0 / resolution) * weights[i];
		sum += texture2D(texture, uv - direction * float(i) * 15.0 / resolution) * weights[i];
	}
	gl_FragColor = sum * 1.25;

}