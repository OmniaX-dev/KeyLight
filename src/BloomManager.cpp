#include "BloomManager.hpp"
#include <ostd/Logger.hpp>

const std::string BloomManager::blurShaderCode = R"(
    #version 130
    uniform sampler2D texture;
    uniform vec2 direction;

    void main() {
        vec2 uv = gl_TexCoord[0].xy;
        vec4 sum = vec4(0.0);
        float weights[5] = float[](0.227, 0.194, 0.121, 0.054, 0.016);

        for (int i = -4; i <= 12; ++i) {
            sum += texture2D(texture, uv + direction * float(i) / 512.0) * weights[abs(i)];
        }

        gl_FragColor = sum;
    }
)";

const std::string BloomManager::bloomShaderCode = R"(
    #version 130

    uniform sampler2D texture;
    uniform float bloom_spread;
    uniform float bloom_intensity;

    void main() {
        vec2 tex_coord = gl_TexCoord[0].xy;
        ivec2 size = textureSize(texture, 0);

        float uv_x = tex_coord.x * size.x;
        float uv_y = tex_coord.y * size.y;

        vec4 sum = vec4(0.0);
        for (int n = 0; n < 9; ++n) {
            uv_y = (tex_coord.y * size.y) + (bloom_spread * float(n - 4));
            vec4 h_sum = vec4(0.0);
            h_sum += texelFetch(texture, ivec2(uv_x - (4.0 * bloom_spread), uv_y), 0);
            h_sum += texelFetch(texture, ivec2(uv_x - (3.0 * bloom_spread), uv_y), 0);
            h_sum += texelFetch(texture, ivec2(uv_x - (2.0 * bloom_spread), uv_y), 0);
            h_sum += texelFetch(texture, ivec2(uv_x - bloom_spread, uv_y), 0);
            h_sum += texelFetch(texture, ivec2(uv_x, uv_y), 0);
            h_sum += texelFetch(texture, ivec2(uv_x + bloom_spread, uv_y), 0);
            h_sum += texelFetch(texture, ivec2(uv_x + (2.0 * bloom_spread), uv_y), 0);
            h_sum += texelFetch(texture, ivec2(uv_x + (3.0 * bloom_spread), uv_y), 0);
            h_sum += texelFetch(texture, ivec2(uv_x + (4.0 * bloom_spread), uv_y), 0);
            sum += h_sum / 9.0;
        }

        gl_FragColor = texture(texture, tex_coord) - ((sum / 9.0) * bloom_intensity);
    }
)";

// Default constructor
BloomManager::BloomManager() {
    // Do nothing yet — init() will be called later
}

// Optional immediate init constructor
BloomManager::BloomManager(unsigned int width, unsigned int height) {
    init(width, height);
}

void BloomManager::init(unsigned int width, unsigned int height) {
    glowBuffer = sf::RenderTexture({ width, height });
    bloomExtract = sf::RenderTexture({ width, height });
    blurBuffer1 = sf::RenderTexture({ width, height });
    blurBuffer2 = sf::RenderTexture({ width, height });

    if (!blurShader.loadFromMemory(blurShaderCode, sf::Shader::Type::Fragment))
        OX_ERROR("Unable to load shader: bloom fragment");
    if (!bloomShader.loadFromMemory(bloomShaderCode, sf::Shader::Type::Fragment))
        OX_ERROR("Unable to load shader: bloom fragment");

    // sprite.setTexture(glowBuffer.getTexture());
    initialized = true;
}

void BloomManager::beginGlowPass() {
    if (!initialized) return;
    glowBuffer.clear(sf::Color::Black);
}

void BloomManager::drawGlow(const sf::Drawable& drawable, const sf::RenderStates& states) {
    if (!initialized) return;
    glowBuffer.draw(drawable, states);
}

void BloomManager::endGlowPass() {
    if (!initialized) return;
    glowBuffer.display();
}

void BloomManager::applyBloom(float threshold) {
    if (!initialized) return;
    sf::Sprite sprite(glowBuffer.getTexture());
    // Extract bright areas
    bloomExtract.clear(sf::Color::Black);
    // bloomShader.setUniform("texture", glowBuffer.getTexture());
    // bloomShader.setUniform("bloom_spread", 1);
    // bloomShader.setUniform("bloom_intensity", 2);
    sprite.setTexture(glowBuffer.getTexture(), true);
    bloomExtract.draw(sprite/*, &bloomShader*/);
    bloomExtract.display();

    // Horizontal blur
    blurBuffer1.clear();
    blurShader.setUniform("texture", bloomExtract.getTexture());
    blurShader.setUniform("direction", sf::Glsl::Vec2(1.f, 0.f));
    sprite.setTexture(bloomExtract.getTexture(), true);
    blurBuffer1.draw(sprite, &blurShader);
    blurBuffer1.display();

    // Vertical blur
    blurBuffer2.clear();
    blurShader.setUniform("texture", blurBuffer1.getTexture());
    blurShader.setUniform("direction", sf::Glsl::Vec2(0.f, 1.f));
    sprite.setTexture(blurBuffer1.getTexture(), true);
    blurBuffer2.draw(sprite, &blurShader);
    blurBuffer2.display();
}

void BloomManager::drawScene(sf::RenderTarget& target, const sf::Drawable& scene) {
    if (!initialized) return;
    target.draw(scene); // base scene

    sf::Sprite sprite(blurBuffer2.getTexture());
    sf::RenderStates additive;
    additive.blendMode = sf::BlendAdd;
    sprite.setTexture(blurBuffer2.getTexture(), true);
    target.draw(sprite, additive); // blurred glow
}

void BloomManager::updateSize(unsigned int width, unsigned int height) {
    if (!initialized) return;
    glowBuffer = sf::RenderTexture({ width, height });
    bloomExtract = sf::RenderTexture({ width, height });
    blurBuffer1 = sf::RenderTexture({ width, height });
    blurBuffer2 = sf::RenderTexture({ width, height });
}