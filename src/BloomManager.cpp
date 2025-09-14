#include "BloomManager.hpp"

const std::string BloomManager::blurShaderCode = R"(
    #version 130
    uniform sampler2D texture;
    uniform vec2 direction;

    void main() {
        vec2 uv = gl_TexCoord[0].xy;
        vec4 sum = vec4(0.0);
        float weights[5] = float[](0.227, 0.194, 0.121, 0.054, 0.016);

        for (int i = -4; i <= 4; ++i) {
            sum += texture2D(texture, uv + direction * float(i) / 512.0) * weights[abs(i)];
        }

        gl_FragColor = sum;
    }
)";

const std::string BloomManager::bloomShaderCode = R"(
    #version 130
    uniform sampler2D texture;
    uniform float threshold;

    void main() {
        vec2 uv = gl_TexCoord[0].xy;
        vec4 color = texture2D(texture, uv);
        float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
        gl_FragColor = (brightness > threshold) ? color : vec4(0.0);
    }
)";

BloomManager::BloomManager(unsigned int width, unsigned int height) {
    glowBuffer = sf::RenderTexture({ width, height });
    bloomExtract = sf::RenderTexture({ width, height });
    blurBuffer1 = sf::RenderTexture({ width, height });
    blurBuffer2 = sf::RenderTexture({ width, height });

    blurShader.loadFromMemory(blurShaderCode, sf::Shader::Type::Fragment);
    bloomShader.loadFromMemory(bloomShaderCode, sf::Shader::Type::Fragment);

    // sprite.setTexture(glowBuffer.getTexture());
}

void BloomManager::beginGlowPass() {
    glowBuffer.clear(sf::Color::Black);
}

void BloomManager::drawGlow(const sf::Drawable& drawable, const sf::RenderStates& states) {
    glowBuffer.draw(drawable, states);
}

void BloomManager::endGlowPass() {
    glowBuffer.display();
}

void BloomManager::applyBloom(float threshold) {
    sf::Sprite sprite(glowBuffer.getTexture());
    // Extract bright areas
    bloomExtract.clear(sf::Color::Black);
    bloomShader.setUniform("texture", glowBuffer.getTexture());
    bloomShader.setUniform("threshold", threshold);
    // sprite.setTexture(glowBuffer.getTexture());
    bloomExtract.draw(sprite, &bloomShader);
    bloomExtract.display();

    // Horizontal blur
    blurBuffer1.clear();
    blurShader.setUniform("texture", bloomExtract.getTexture());
    blurShader.setUniform("direction", sf::Glsl::Vec2(1.f, 0.f));
    sprite.setTexture(bloomExtract.getTexture());
    blurBuffer1.draw(sprite, &blurShader);
    blurBuffer1.display();

    // Vertical blur
    blurBuffer2.clear();
    blurShader.setUniform("texture", blurBuffer1.getTexture());
    blurShader.setUniform("direction", sf::Glsl::Vec2(0.f, 1.f));
    sprite.setTexture(blurBuffer1.getTexture());
    blurBuffer2.draw(sprite, &blurShader);
    blurBuffer2.display();
}

void BloomManager::drawScene(sf::RenderTarget& target, const sf::Drawable& scene) {
    target.draw(scene); // base scene

    sf::Sprite sprite(blurBuffer2.getTexture());
    sf::RenderStates additive;
    additive.blendMode = sf::BlendAdd;
    // sprite.setTexture(blurBuffer2.getTexture());
    target.draw(sprite, additive); // blurred glow
}