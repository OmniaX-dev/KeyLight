#version 130

uniform vec2 u_resolution;     // screen size
uniform vec2 u_center;         // center of the shape
uniform vec2 u_size;           // half-size of the rectangle
uniform float u_radius;        // corner radius
uniform float u_glowWidth;     // glow thickness
uniform vec4 u_glowColor;      // glow color

float sdRoundedRect(vec2 p, vec2 size, float radius)
{
    vec2 q = abs(p) - size + radius;
    return length(max(q, 0.0)) - radius;
}

void main()
{
    vec2 fragCoord = gl_FragCoord.xy;
    vec2 localPos = fragCoord - u_center;

    float dist = sdRoundedRect(localPos, u_size, u_radius);

    // Glow intensity: fades from edge outward
    float glow = smoothstep(u_glowWidth, 0.0, dist);

    gl_FragColor = u_glowColor * glow;
	gl_FragColor = vec4(gl_FragCoord.xy / u_resolution, 0.0, 1.0);

}