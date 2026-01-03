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

#include "Window.hpp"
#include "Common.hpp"
#include "Renderer.hpp"
#include "VPianoData.hpp"
#include "ffmpeg_helper.hpp"
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Window/WindowEnums.hpp>
#include <TGUI/Color.hpp>
#include <ostd/Logger.hpp>

void Window::onInitialize(void)
{
	enableSignals();
	connectSignal(ostd::tBuiltinSignals::KeyReleased);
	connectSignal(ostd::tBuiltinSignals::KeyPressed);
	connectSignal(SignalListener::NoteOnSignal);
	connectSignal(SignalListener::NoteOffSignal);
	connectSignal(SignalListener::MidiStartSignal);
	connectSignal(ostd::tBuiltinSignals::WindowResized);
	connectSignal(WindowFocusLost);
	connectSignal(WindowFocusGained);

	Renderer::init(*this, "themes/fonts/RobotoMono.ttf");

	m_window.setPosition({ 30, 30 });

	m_windowSizeBeforeFullscreen	 = { (float)getWindowWidth(), (float)getWindowHeight() };
	m_windowPositionBeforeFullscreen = { (float)m_window.getPosition().x, (float)m_window.getPosition().y };
	m_vpiano.init();
	setClearColor(m_vpiano.vPianoData().backgroundColor);

	m_gui.init(*this, m_vpiano.getVideoRenderer().getVideoRenderState(), "themes/ui/cursor.png", "themes/ui/icon.png", "themes/Dark.txt", true);
	m_gui.showFPS(true);
	setSize(1280, 720);

	m_gui.toggleVisibility();

	m_vpiano.loadProjectFile("TestProject.klp");
}

void Window::handleSignal(ostd::tSignal& signal)
{
	if (signal.ID == ostd::tBuiltinSignals::KeyReleased)
	{
		auto& evtData = (KeyEventData&)signal.userData;
		if (evtData.keyCode == (int32_t)sf::Keyboard::Key::Escape)
			close();
		if (evtData.keyCode == (int32_t)sf::Keyboard::Key::Space)
		{
			if (!m_vpiano.getVideoRenderer().isRenderingToFile())
			{
				if (!m_vpiano.isPlaying())
					m_vpiano.play();
				else
					m_vpiano.pause();
			}
		}
		else if (evtData.keyCode == (int32_t)sf::Keyboard::Key::Enter)
		{
			if (!m_vpiano.getVideoRenderer().isRenderingToFile())
			{
				m_vpiano.stop();
			}
		}
		else if (evtData.keyCode == (int32_t)sf::Keyboard::Key::F9)
		{
			if (!m_vpiano.getVideoRenderer().isRenderingToFile())
			{
				// if (!m_vpiano.configImageSequenceRender("tmp", { 1920, 1080 }, 60))
				// 	OX_ERROR("Unable to start image sequence render.");
				if (!m_vpiano.getVideoRenderer().configFFMPEGVideoRender("./output", { 1920, 1080 }, 60, FFMPEG::Profiles::GeneralPurpose))
					OX_ERROR("Unable to start video render.");
				else
				{
					m_gui.showVideoRenderingGui();
				}
			}
		}
		else if (evtData.keyCode == (int32_t)sf::Keyboard::Key::F11)
		{
			if (!m_vpiano.getVideoRenderer().isRenderingToFile())
			{
				enableFullscreen(!m_isFullscreen);
			}
		}
		else if (evtData.keyCode == (int32_t)sf::Keyboard::Key::F2)
		{
			if (!m_vpiano.getVideoRenderer().isRenderingToFile())
			{
				m_gui.toggleVisibility();
			}
		}
		else if (evtData.keyCode == (int32_t)sf::Keyboard::Key::F3)
		{
			if (!m_vpiano.getVideoRenderer().isRenderingToFile())
			{
				// m_gui.showFileDialog("Test File DIalog", { { "All file types", { "*.*" } } }, [](const std::vector<ostd::String>& fileList, bool wasCanceled){

				// }, true);
				// m_gui.showColorPicker("Test Color DIalog", { 255, 0, 0 }, [](const ostd::Color& color){
				// 	std::cout << color << "\n";
				// });
			}
		}
	}
	else if (signal.ID == ostd::tBuiltinSignals::WindowResized)
	{
		if (!m_vpiano.getVideoRenderer().isRenderingToFile())
		{
			auto& evtData = (WindowResizedData&)signal.userData;
			sf::View view	 = m_window.getView();
			view.setSize({ static_cast<float>(evtData.new_width), static_cast<float>(evtData.new_height) });
			view.setCenter({ evtData.new_width / 2.f, evtData.new_height / 2.f });
			m_window.setView(view);
			m_vpiano.vPianoData().updateScale(evtData.new_width, evtData.new_height);
			m_vpiano.onWindowResized((uint32_t)evtData.new_width, (uint32_t)evtData.new_height);
			__update_local_window_size((uint32_t)evtData.new_width, (uint32_t)evtData.new_height);
		}
	}
	else if (signal.ID == SignalListener::MidiStartSignal)
	{
		if (m_vpiano.vPianoRes().hasAudioFile() && !m_vpiano.getVideoRenderer().isRenderingToFile())
		{
			m_vpiano.vPianoRes().getAudioFile().play();
			m_vpiano.vPianoRes().getAudioFile().setPlayingOffset(sf::seconds(m_vpiano.vPianoRes().getAutoSoundStart()));
		}
	}
}

void Window::onEventPoll(const std::optional<sf::Event>& event)
{
	m_gui.onEventPoll(event);
}

void Window::onRender(void)
{
	Common::deltaTime = 1.0 / 60.0; //m_frameClock.restart().asSeconds();
	m_vpiano.render();
	m_gui.draw();
}

void Window::onFixedUpdate(double frameTime_s)
{
	m_vpiano.update();
}

void Window::onUpdate(void)
{
}

void Window::enableFullscreen(bool enable)
{
	if (m_lockFullscreenStatus) return;
	auto old_size	  = m_window.getSize();
	auto old_position = m_window.getPosition();
	if (enable)
	{
		m_windowSizeBeforeFullscreen	 = { (float)old_size.x, (float)old_size.y };
		m_windowPositionBeforeFullscreen = { (float)old_position.x, (float)old_position.y };
		m_window.create(sf::VideoMode::getDesktopMode(), getTitle().cpp_str(), sf::Style::None, sf::State::Fullscreen);
		// auto mode = sf::VideoMode::getDesktopMode();
		// m_window.create(sf::VideoMode({ mode.size.x - 1, mode.size.y - 1 }), getTitle().cpp_str(), sf::Style::None, sf::State::Fullscreen);

		// m_window.setPosition({ 1, 1 });
		m_window.setPosition({ 0, 0 });
		m_isFullscreen = true;
		// m_window.setMouseCursorVisible(true);
		// sf::Cursor cursor(sf::Cursor::Type::Arrow);
		// m_window.setMouseCursor(cursor);
	}
	else
	{
		m_window.create(sf::VideoMode({ (uint32_t)m_windowSizeBeforeFullscreen.x, (uint32_t)m_windowSizeBeforeFullscreen.y }), getTitle().cpp_str(), sf::Style::Default);
		m_window.setPosition({ (int32_t)m_windowPositionBeforeFullscreen.x, (int32_t)m_windowPositionBeforeFullscreen.y });
		m_isFullscreen = false;
	}
	// m_window.setMouseCursor(*m_cursor);
	auto new_size = m_window.getSize();
	WindowResizedData wrd(*this, old_size.x, old_size.y, new_size.x, new_size.y);
	ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::WindowResized, ostd::tSignalPriority::RealTime, wrd);
}

void Window::enableResizeable(bool enable)
{
	if (m_isFullscreen) return;
	if (enable && m_isResizeable) return;
	if (!enable && !m_isResizeable) return;
	auto position = m_window.getPosition();
	if (enable)
	{
		m_window.create(sf::VideoMode({ (uint32_t)getWindowWidth(), (uint32_t)getWindowHeight() }), getTitle().cpp_str(), sf::Style::Default, sf::State::Windowed);
		m_window.setPosition(position);
		m_isResizeable = true;
	}
	else
	{
		m_window.create(sf::VideoMode({ (uint32_t)getWindowWidth(), (uint32_t)getWindowHeight() }), getTitle().cpp_str(), sf::Style::Close, sf::State::Windowed);
		m_window.setPosition(position);
		m_isResizeable = false;
	}
}
