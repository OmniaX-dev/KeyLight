#pragma once

#include <SFML/Graphics.hpp>
#include "SFMLWindow.hpp"
#include "RoundedRectangleShape.hpp"

class Renderer
{
	public:
		static bool init(WindowBase& window, const ostd::String& fontFilePath = "");
		static void setRenderTarget(sf::RenderTarget* target);
		static void useShader(sf::Shader* shader);
		static void useFont(const ostd::String& fontFilePath);
		static void useTexture(sf::Texture* texture, ostd::Rectangle textureRect = { 0, 0, 0, 0 });
		static void setTextureRect(ostd::Rectangle textureRect);
		static void clear(const ostd::Color& color);

		static inline sf::RenderTarget* getRenderTarget(void) { return m_target; }
		static inline WindowBase* getWindow(void) { return m_window; }
		static inline sf::Shader* getShader(void) { return m_shader; }
		static inline sf::Texture* getTexture(void) { return m_texture; }
		static inline sf::Font& getFont(void) { return m_font; }
		static inline int32_t getRoundedRectCornerResolution(void) { return m_roundedRectCornerResolution; }

		static inline void setRoundedRectCornerResolution(int32_t res) { m_roundedRectCornerResolution = res; }

		static void drawString(const ostd::String& str, const ostd::Vec2& position, const ostd::Color& color, uint32_t font_size);
		static void drawTexture(const sf::Texture& texture, const ostd::Rectangle& bounds = { 0, 0, 0, 0 });
		
		static void drawRect(const ostd::Rectangle& rect, const ostd::Color& outlineColor, int32_t outlineThickness = -1);
		static void fillRect(const ostd::Rectangle& rect, const ostd::Color& fillColor);
		static void outlineRect(const ostd::Rectangle& rect, const ostd::Color& fillColor, const ostd::Color& outlineColor, int32_t outlineThickness = -1);
		
		static void drawRoundedRect(const ostd::Rectangle& rect, const ostd::Color& outlineColor, const ostd::Rectangle& radius, int32_t outlineThickness = -1);
		static void fillRoundedRect(const ostd::Rectangle& rect, const ostd::Color& fillColor, const ostd::Rectangle& radius);
		static void outlineRoundedRect(const ostd::Rectangle& rect, const ostd::Color& fillColor, const ostd::Color& outlineColor, const ostd::Rectangle& radius, int32_t outlineThickness = -1);
		
	private:
		static void __draw_call(sf::Drawable* obj);

	private:
		inline static sf::RenderTarget* m_target { nullptr };
		inline static WindowBase* m_window { nullptr };
		inline static sf::Shader* m_shader { nullptr };
		inline static sf::Texture* m_texture { nullptr };

		inline static sf::Font m_font;
		inline static bool m_textCreated { false };
		inline static sf::IntRect m_textureRect { };
		inline static int32_t m_roundedRectCornerResolution { 12 };

		inline static sf::RectangleShape m_rect;
		inline static RoundedRectangleShape m_roundedRect;
		inline static sf::Text*  m_text;
};