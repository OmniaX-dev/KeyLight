#pragma once
#include <SFML/Graphics.hpp>

class BloomManager {
public:
    BloomManager(unsigned int width, unsigned int height);

    void beginGlowPass();
    void drawGlow(const sf::Drawable& drawable, const sf::RenderStates& states = sf::RenderStates::Default);
    void endGlowPass();

    void applyBloom(float threshold = 0.6f);
    void drawScene(sf::RenderTarget& target, const sf::Drawable& scene);

private:
    sf::RenderTexture glowBuffer;
    sf::RenderTexture bloomExtract;
    sf::RenderTexture blurBuffer1;
    sf::RenderTexture blurBuffer2;

    sf::Shader blurShader;
    sf::Shader bloomShader;
    // sf::Sprite sprite;

    static const std::string blurShaderCode;
    static const std::string bloomShaderCode;
};