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

#include <ostd/BaseObject.hpp>
#include <ostd/IOHandlers.hpp>
#include <ostd/Signals.hpp>
#include <ostd/Time.hpp>

#include <SFML/Graphics.hpp>
#include <ogfx/SDLInclude.hpp>

class WindowBase : public ostd::BaseObject
{
	public:
		enum class eCursor { Arrow = 0,  IBeam };

	public:
		inline WindowBase(void) { }
		~WindowBase(void);
		inline WindowBase(int32_t width, int32_t height, const ostd::String& windowTitle) { initialize(width, height, windowTitle); }
		void initialize(int32_t width, int32_t height, const ostd::String& windowTitle);
		void close(void);
		void update(void);
		void setSize(int32_t width, int32_t height);
		void syncWindowSize(void);
		void setTitle(const ostd::String& title);

		inline virtual void onRender(void) { }
		inline virtual void onUpdate(void) { }
		inline virtual void onFixedUpdate(double frameTime_s) { }
		inline virtual void onInitialize(void) { }
		inline virtual void onDestroy(void) { }
		inline virtual void onClose(void) { }
		inline virtual void onEventPoll(const std::optional<sf::Event>& event) { }

		inline bool	isInitialized(void) const { return m_initialized; }
		inline bool	isRunning(void) const { return m_running; }
		inline void	hide(void) { m_window.setVisible(false); }
		inline void	show(void) { m_window.setVisible(true); }
		inline ostd::String	getTitle(void) const { return m_title; }
		inline int32_t getFPS(void) const { return m_fps; }
		inline int32_t getWindowWidth(void) const { return m_windowWidth; }
		inline int32_t getWindowHeight(void) const { return m_windowHeight; }
		inline bool	isMouseDragEventEnabled(void) const { return m_deagEventEnabled; }
		inline void	enableMouseDragEvent(bool enable = true) { m_deagEventEnabled = enable; }
		inline ostd::Color getClearColor(void) const { return m_clearColor; }
		inline void	setClearColor(const ostd::Color& color) { m_clearColor = color; }
		inline sf::RenderWindow& sfWindow(void) { return m_window; }

	protected:
		void __update_local_window_size(uint32_t width, uint32_t height);

	private:
		void __handle_events(void);


	protected:
		sf::RenderWindow m_window;

	private:
		ostd::Color m_clearColor { 0, 0, 0, 255 };

		int32_t	m_windowWidth { 0 };
		int32_t	m_windowHeight { 0 };
		ostd::String m_title { "" };
		int32_t	m_fps { 0 };


		ostd::StepTimer m_fixedUpdateTImer;
		ostd::StepTimer m_fpsUpdateTimer;
		sf::Clock m_fpsUpdateClock;
		double m_frameTimeAcc { 0.0 };
		int32_t m_frameCount { 0 };

		bool m_deagEventEnabled { false };
		bool m_running { false };
		bool m_initialized { false };

	public:
		inline static const uint64_t WindowFocusLost   = ostd::SignalHandler::newCustomSignal(6000);
		inline static const uint64_t WindowFocusGained = ostd::SignalHandler::newCustomSignal(6001);
};
class WindowResizedData : public ostd::BaseObject
{
	public:
		inline WindowResizedData(WindowBase& parent, int32_t _oldx, int32_t _oldy, int32_t _newx, int32_t _newy) :
			parentWindow(parent), old_width(_oldx), old_height(_oldy), new_width(_newx), new_height(_newy)
		{
			setTypeName("ogfx::WindowResizedData");
			validate();
		}

	public:
		int32_t	new_width;
		int32_t	new_height;
		int32_t	old_width;
		int32_t	old_height;
		WindowBase& parentWindow;
};
class MouseEventData : public ostd::BaseObject
{
	public:
		enum class eButton { _None = 0, Left, Middle, Right };

	public:
		inline MouseEventData(WindowBase& parent, int32_t mousex, int32_t mousey, eButton btn) :
			parentWindow(parent), position_x(mousex), position_y(mousey), button(btn)
		{
			setTypeName("ogfx::MouseEventData");
			validate();
		}

	public:
		int32_t	position_x;
		int32_t	position_y;
		eButton	button;
		WindowBase& parentWindow;
};
class KeyEventData : public ostd::BaseObject
{
	public:
		enum class eKeyEvent { Pressed = 0, Released, Text };

	public:
		inline KeyEventData(WindowBase& parent, int32_t key, char _text, eKeyEvent evt) :
			parentWindow(parent), keyCode(key), text(_text), eventType(evt)
		{
			setTypeName("ogfx::KeyEventData");
			validate();
		}

	public:
		int32_t		keyCode;
		char		text;
		eKeyEvent	eventType;
		WindowBase& parentWindow;
};
