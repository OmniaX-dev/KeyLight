#pragma once

#include "SFMLWindow.hpp"
#include "VirtualPiano.hpp"
#include <ostd/Defines.hpp>
#include <ostd/Signals.hpp>
#include <ostd/Utils.hpp>

#include <TGUI/Backend/SFML-Graphics.hpp>
#include <TGUI/TGUI.hpp>

// #ifdef WINDOWS_OS
// 	#include  <windows.h>
// #endif

class Window : public WindowBase
{
	public:

	public:
		inline Window(void) : m_vpiano(*this) { }
		void onInitialize(void) override;
		void handleSignal(ostd::tSignal& signal) override;
		void onEventPoll(const std::optional<sf::Event>& event) override;
		void onRender(void) override;
		void onFixedUpdate(void) override;
		void onUpdate(void) override;
		void enableFullscreen(bool enable = true);
		void drawGUI(void);

		bool buildGUI(tgui::BackendGui& gui);

		void onFileSelected(const tgui::String& str);

	private:
		ostd::Vec2 m_windowSizeBeforeFullscreen { 0.0f, 0.0f };
		ostd::Vec2 m_windowPositionBeforeFullscreen { 0.0f, 0.0f };
		bool m_isFullscreen { false };
		VirtualPiano m_vpiano;

		tgui::Gui m_gui;
		std::optional<sf::Cursor> m_cursor;
		sf::Image m_icon;
		bool m_drawGui { true };

		// #ifdef WINDOWS_OS
		// 		std::chrono::steady_clock::time_point lastFocusChange;
		// 		bool isTopmost = false;
		// 	public:
		// 		inline static constexpr bool OnWindows = true;
		// #endif
};
