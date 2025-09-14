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
		inline Window(void) : m_vpiano(*this) {  }
		void onInitialize(void) override;
		void handleSignal(ostd::tSignal& signal) override;
		void onRender(void) override;
		void onFixedUpdate(void) override;
		void onUpdate(void) override;
		void enableFullscreen(bool enable = true);

	private:
		ostd::Vec2 m_windowSizeBeforeFullscreen { 0.0f, 0.0f };
		ostd::Vec2 m_windowPositionBeforeFullscreen { 0.0f, 0.0f };
		bool m_isFullscreen { false };
		VirtualPiano m_vpiano;
};