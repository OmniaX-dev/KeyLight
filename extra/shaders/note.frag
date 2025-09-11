#version 130

// From SFML's default vertex shader
in vec2 v_texCoord;

uniform vec2  u_size;         // note size in pixels (width, height)
uniform vec4  u_radii;        // corner radii: (TL, TR, BR, BL) in pixels
uniform vec4  u_fillColor;    // RGBA 0..1
uniform float u_edgeSoftness; // pixels for inner edge AA (e.g., 1.0)
uniform float u_glowSize;     // pixels outward (e.g., 8.0)

// Output
out vec4 fragColor;

// Signed distance to a rounded rectangle with per-corner radii.
// p is in local space centered at (0,0). b is half-size.
// Radii are (TL, TR, BR, BL).
float sdRoundRectCorners(vec2 p, vec2 b, vec4 r)
{
    // Pick corner radius by quadrant (approximation that works well for UI shapes)
    // TL: (-x,-y), TR: (+x,-y), BR: (+x,+y), BL: (-x,+y)
    float rx = (p.x >= 0.0)
                 ? (p.y >= 0.0 ? r.z : r.y)
                 : (p.y >= 0.0 ? r.w : r.x);

    // Distance like standard rounded box but using the quadrant radius
    vec2 q = abs(p) - (b - vec2(rx));
    return length(max(q, 0.0)) - rx;
}

void main()
{
    // Map UV (0..1) into local pixels centered at (0,0)
    vec2 p = v_texCoord * u_size - 0.5 * u_size;
    fragColor = vec4(p / u_size, 0.0, 1.0); // temp debug
    vec2 halfSize = 0.5 * u_size;

    // Signed distance: < 0 inside, 0 at edge, > 0 outside
    float d = sdRoundRectCorners(p, halfSize, u_radii);

    // Inside fill with soft edge (alpha ramps within u_edgeSoftness)
    float insideAlpha = 1.0 - smoothstep(-u_edgeSoftness, 0.0, d);

    // Glow outside: alpha goes from 1 at the edge to 0 at u_glowSize
    float glowAlpha = 1.0 - smoothstep(0.0, u_glowSize, d);

    // Compose: keep glow only outside; inside uses fill color fully (with edge AA)
    vec3 color = u_fillColor.rgb;
    float alpha = max(insideAlpha, glowAlpha) * u_fillColor.a;

    fragColor = vec4(color, alpha);
}
