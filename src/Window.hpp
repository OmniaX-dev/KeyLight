/*
    KeyLight - A MIDI Piano Visualizer
    Copyright (C) 2025  OmniaX-Dev

    This file is part of KeyLight.

    KeyLight is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    KeyLight is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with KeyLight.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include "SFMLWindow.hpp"
#include "VirtualPiano.hpp"
#include <SFML/Graphics/Texture.hpp>
#include <ostd/Defines.hpp>
#include <ostd/Signals.hpp>
#include <ostd/Utils.hpp>
#include "Gui.hpp"

class Window : public WindowBase
{
	public:

	public:
		inline Window(void) : m_vpiano(*this) { }
		void onInitialize(void) override;
		void handleSignal(ostd::tSignal& signal) override;
		void onEventPoll(const std::optional<sf::Event>& event) override;
		void onRender(void) override;
		void onUpdate(void) override;
		void onFixedUpdate(double frameTime_s) override;
		void enableFullscreen(bool enable = true);
		void enableResizeable(bool enable = true);

		inline bool isFullscreen(void) const { return m_lockFullscreenStatus; }
		inline void lockFullscreenStatus(bool lock = true) { m_lockFullscreenStatus = lock; }
		inline bool isFullscreenStatusLocked(void) const { return m_lockFullscreenStatus; }
		inline const VirtualPiano& getVirtualPiano(void) const { return m_vpiano; }

	private:
		ostd::Vec2 m_windowSizeBeforeFullscreen { 0.0f, 0.0f };
		ostd::Vec2 m_windowPositionBeforeFullscreen { 0.0f, 0.0f };
		bool m_isFullscreen { false };
		bool m_isResizeable { true };
		bool m_lockFullscreenStatus { false };
		VirtualPiano m_vpiano;
		Gui m_gui;
		sf::Clock m_frameClock;
};
