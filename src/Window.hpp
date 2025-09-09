#pragma once

#include "SFMLWindow.hpp"
// #include <ogfx/BasicRenderer.hpp>
#include <ostd/Signals.hpp>
#include <ostd/Utils.hpp>
#include <deque>
#include "MidiParser.hpp"
#include "VirtualPiano.hpp"

class Window : public WindowBase
{
	public:

	public:
		inline Window(void) : m_vpiano(*this) {  }
		void onInitialize(void) override;
		void handleSignal(ostd::tSignal& signal) override;
		void onRender(void) override;
		void onFixedUpdate(void) override;
		void onUpdate(void) override;
		void enableFullscreen(bool enable = true);

		void outlinedRect(const ostd::Rectangle& rect, const ostd::Color& fillColor, const ostd::Color& outlineColor, int32_t outlineThickness = -1);

		
	private:
		ostd::Vec2 m_windowSizeBeforeFullscreen { 0.0f, 0.0f };
		bool m_isFullscreen { false };
		VirtualPiano m_vpiano;
		sf::RectangleShape m_sf_rect;
};