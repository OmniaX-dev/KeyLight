#version 130

uniform vec2 u_size;       // rectangle size in pixels
uniform float u_radius;    // corner radius in pixels
uniform vec4 u_fillColor;  // RGBA fill color (0–1 range)
uniform float u_glowSize;  // thickness of glow in pixels
uniform vec2 u_position; // top-left of the shape in window coords

out vec4 fragColor;

// Signed distance to a rounded rectangle
float sdRoundRect(vec2 p, vec2 b, float r) {
    vec2 q = abs(p) - b + vec2(r);
    return length(max(q, 0.0)) - r;
}

void main() {
    // Convert fragment coords to local rect space (centered at 0,0)
    vec2 halfSize = u_size * 0.5;
    vec2 p = (gl_FragCoord.xy - u_position) - (u_size * 0.5);

    // Distance from edge (negative inside, positive outside)
    float dist = sdRoundRect(p, halfSize - vec2(u_radius), u_radius);

    // Fill area
    if (dist < 0.0) {
        fragColor = u_fillColor;
    }
    else if (dist < u_glowSize) {
        // Glow region: fade alpha from fillColor outward
        float alpha = 1.0 - smoothstep(0.0, u_glowSize, dist);
        fragColor = vec4(u_fillColor.rgb, alpha * u_fillColor.a);
    }
    else {
        // Outside glow
        fragColor = vec4(0.0);
    }
}

