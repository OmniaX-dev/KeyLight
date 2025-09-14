#pragma once

#include "SFMLWindow.hpp"
// #include <ogfx/BasicRenderer.hpp>
#include <ostd/Signals.hpp>
#include <ostd/Utils.hpp>
#include <deque>
#include "MidiParser.hpp"
#include "VirtualPiano.hpp"
#include "RoundedRectangleShape.hpp"

class Window : public WindowBase
{
	public:

	public:
		inline Window(void) : m_vpiano(*this) /*, m_sf_text(m_font) */ {  }
		void onInitialize(void) override;
		void handleSignal(ostd::tSignal& signal) override;
		void onRender(void) override;
		void onFixedUpdate(void) override;
		void onUpdate(void) override;
		void enableFullscreen(bool enable = true);

		// void drawString(const ostd::String& str, const ostd::Vec2& position, const ostd::Color& color, uint32_t font_size);
		// void outlinedRect(const ostd::Rectangle& rect, const ostd::Color& fillColor, const ostd::Color& outlineColor, int32_t outlineThickness = -1);
		// void outlinedRoundedRect(const ostd::Rectangle& rect, const ostd::Color& fillColor, const ostd::Color& outlineColor, const ostd::Rectangle& radius, int32_t outlineThickness = -1);
		
	private:
		ostd::Vec2 m_windowSizeBeforeFullscreen { 0.0f, 0.0f };
		ostd::Vec2 m_windowPositionBeforeFullscreen { 0.0f, 0.0f };
		bool m_isFullscreen { false };
		VirtualPiano m_vpiano;
		// sf::Font m_font;
		// sf::RectangleShape m_sf_rect;
		// RoundedRectangleShape m_sf_roundedRect;
		// sf::Text m_sf_text;
		// sf::Shader* m_currentShader { nullptr };
};