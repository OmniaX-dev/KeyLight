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

#include "SFMLWindow.hpp"
#include <SFML/Window/Mouse.hpp>
#include <ostd/Logger.hpp>

WindowBase::~WindowBase(void)
{
	onDestroy();
}

void WindowBase::initialize(int32_t width, int32_t height, const ostd::String& windowTitle)
{
	if (m_initialized) return;
	m_windowWidth = width;
	m_windowHeight = height;
	m_title = windowTitle;
	m_window.create(sf::VideoMode({ static_cast<uint32_t>(width), static_cast<uint32_t>(height) }), windowTitle.cpp_str());
	m_initialized = true;
	m_running = true;

	m_fixedUpdateTImer.create(60.0, [this](double frameTime_s){
		this->onFixedUpdate(frameTime_s);
	});

	m_fpsUpdateTimer.create(1.0, [this](double frameTime_s){
		if (this->m_frameCount == 0) return;
		if (this->m_frameTimeAcc == 0) return;
		this->m_fps = (int32_t)(1.0f / (this->m_frameTimeAcc / static_cast<double>(this->m_frameCount)));
		this->m_frameTimeAcc = 0;
		this->m_frameCount = 0;
	});

	setTypeName("WindowBase");
	enableSignals(true);
	validate();

	onInitialize();
}

void WindowBase::close(void)
{
	m_running = false;
	onClose();
	ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::WindowClosed, ostd::tSignalPriority::RealTime, *this);
}

void WindowBase::update(void)
{
	if (!m_initialized) return;
	__handle_events();
	m_window.clear({ m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a });
	m_fixedUpdateTImer.update();
	onUpdate();
	onRender();
	m_window.display();
	m_frameTimeAcc += m_fpsUpdateClock.restart().asSeconds();
	m_frameCount++;
	m_fpsUpdateTimer.update();
}

void WindowBase::setSize(int32_t width, int32_t height)
{
	if (!isInitialized()) return;
	m_window.setSize({ static_cast<uint32_t>(width), static_cast<uint32_t>(height) });
	syncWindowSize();
}

void WindowBase::syncWindowSize(void)
{
	m_windowWidth = m_window.getSize().x;
	m_windowHeight = m_window.getSize().y;
}

void WindowBase::setTitle(const ostd::String& title)
{
	if (!isInitialized()) return;
	m_title = title;
	m_window.setTitle(m_title.cpp_str());
}

void WindowBase::__handle_events(void)
{
	if (!isInitialized()) return;
	auto l_getMouseState = [this](void) -> MouseEventData {
		sf::Vector2i pos = sf::Mouse::getPosition(m_window);
		bool leftPressed   = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
		bool rightPressed  = sf::Mouse::isButtonPressed(sf::Mouse::Button::Right);
		bool middlePressed  = sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle);
		MouseEventData::eButton button = MouseEventData::eButton::_None;
		if (middlePressed) button = MouseEventData::eButton::Middle;
		if (leftPressed) button = MouseEventData::eButton::Left;
		if (rightPressed) button = MouseEventData::eButton::Right;
		MouseEventData mmd(*this, pos.x, pos.y, button);
		return mmd;
	};
	while (const std::optional event = m_window.pollEvent())
	{
		onEventPoll(event);
		if (event->is<sf::Event::Closed>())
		{
			m_running = false;
			onClose();
			ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::WindowClosed, ostd::tSignalPriority::RealTime, *this);
		}
		else if (event->is<sf::Event::Resized>())
		{
			const auto* resized = event->getIf<sf::Event::Resized>();
			WindowResizedData wrd(*this, m_windowWidth, m_windowHeight, 0, 0);
			m_windowWidth = resized->size.x;
			m_windowHeight = resized->size.y;
			wrd.new_width = m_windowWidth;
			wrd.new_height = m_windowHeight;
			ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::WindowResized, ostd::tSignalPriority::RealTime, wrd);
		}
		else if (event->is<sf::Event::FocusLost>())
		{
			ostd::SignalHandler::emitSignal(WindowFocusLost, ostd::tSignalPriority::RealTime, *this);
		}
		else if (event->is<sf::Event::FocusGained>())
		{
			ostd::SignalHandler::emitSignal(WindowFocusGained, ostd::tSignalPriority::RealTime, *this);
		}
		else if (event->is<sf::Event::MouseMoved>())
		{
			MouseEventData mmd = l_getMouseState();
			if (isMouseDragEventEnabled() && mmd.button != MouseEventData::eButton::_None)
				ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::MouseDragged, ostd::tSignalPriority::RealTime, mmd);
			else
				ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::MouseMoved, ostd::tSignalPriority::RealTime, mmd);
		}
		else if (event->is<sf::Event::MouseButtonPressed>())
		{
			MouseEventData mmd = l_getMouseState();
			ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::MousePressed, ostd::tSignalPriority::RealTime, mmd);
		}
		else if (event->is<sf::Event::MouseButtonReleased>())
		{
			MouseEventData mmd = l_getMouseState();
			const auto* released = event->getIf<sf::Event::MouseButtonReleased>();
			if (released->button == sf::Mouse::Button::Left)
				mmd.button = MouseEventData::eButton::Left;
			else if (released->button == sf::Mouse::Button::Right)
				mmd.button = MouseEventData::eButton::Right;
			else if (released->button == sf::Mouse::Button::Middle)
				mmd.button = MouseEventData::eButton::Middle;
			ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::MouseReleased, ostd::tSignalPriority::RealTime, mmd);
		}
		else if (event->is<sf::Event::KeyPressed>())
		{
			const auto* keyPressed = event->getIf<sf::Event::KeyPressed>();
			KeyEventData ked(*this, (int32_t)keyPressed->code, 0, KeyEventData::eKeyEvent::Pressed);
			ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::KeyPressed, ostd::tSignalPriority::RealTime, ked);
		}
		else if (event->is<sf::Event::KeyReleased>())
		{
			const auto* keyReleased = event->getIf<sf::Event::KeyReleased>();
			KeyEventData ked(*this, (int32_t)keyReleased->code, 0, KeyEventData::eKeyEvent::Released);
			ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::KeyReleased, ostd::tSignalPriority::RealTime, ked);
		}
		else if (event->is<sf::Event::TextEntered>())
		{
			const auto* textEntered = event->getIf<sf::Event::TextEntered>();
			char32_t codepoint = textEntered->unicode;
			KeyEventData ked(*this, 0, static_cast<char>(codepoint), KeyEventData::eKeyEvent::Text);
			ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::TextEntered, ostd::tSignalPriority::RealTime, ked);
		}
	}
}

void WindowBase::__update_local_window_size(uint32_t width, uint32_t height)
{
	m_windowWidth = width;
	m_windowHeight = height;
}
