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
#include "Renderer.hpp"
#include <SFML/Window/WindowEnums.hpp>
#include <ostd/Logger.hpp>
#include <vector>

void Window::onInitialize(void)
{
	enableSignals();
	connectSignal(ostd::tBuiltinSignals::KeyReleased);
	connectSignal(VirtualPiano::NoteOnSignal);
	connectSignal(VirtualPiano::NoteOffSignal);
	connectSignal(VirtualPiano::MidiStartSignal);
	connectSignal(ostd::tBuiltinSignals::WindowResized);
	connectSignal(WindowFocusLost);
	connectSignal(WindowFocusGained);
	setClearColor({ 255, 10, 10 });

	Renderer::init(*this, "themes/fonts/RobotoMono.ttf");

	m_window.setPosition({ 30, 30 });

	m_windowSizeBeforeFullscreen	 = { (float)getWindowWidth(), (float)getWindowHeight() };
	m_windowPositionBeforeFullscreen = { (float)m_window.getPosition().x, (float)m_window.getPosition().y };
	// enableFullscreen(true);
	m_vpiano.init();
	m_vpiano.loadMidiFile("res/midi/scale2.mid");
	m_vpiano.loadAudioFile("res/music/scale2.mp3");

	m_gui.init(*this, m_vpiano.getVideoRenderState(), "themes/ui/cursor.png", "themes/ui/icon.png", "themes/Dark.txt", true);
	m_gui.showFPS(true);
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
			if (!m_vpiano.isRenderingToFile())
			{
				if (!m_vpiano.isPlaying())
					m_vpiano.play();
				else
					m_vpiano.pause();
			}
		}
		else if (evtData.keyCode == (int32_t)sf::Keyboard::Key::Enter)
		{
			if (!m_vpiano.isRenderingToFile())
			{
				m_vpiano.stop();
			}
		}
		else if (evtData.keyCode == (int32_t)sf::Keyboard::Key::F9)
		{
			if (!m_vpiano.isRenderingToFile())
			{
				if (!m_vpiano.configImageSequenceRender("tmp", { 1920, 1080 }, 60))
					OX_ERROR("Unable to start image sequence render.");
				else
				{
					m_gui.showVideoRenderingGui();
				}
			}
		}
		else if (evtData.keyCode == (int32_t)sf::Keyboard::Key::F11)
		{
			if (!m_vpiano.isRenderingToFile())
			{
				enableFullscreen(!m_isFullscreen);
			}
		}
		else if (evtData.keyCode == (int32_t)sf::Keyboard::Key::F2)
		{
			if (!m_vpiano.isRenderingToFile())
			{
				m_gui.toggleVisibility();
			}
		}
		else if (evtData.keyCode == (int32_t)sf::Keyboard::Key::F3)
		{
			if (!m_vpiano.isRenderingToFile())
			{
				m_gui.showFileDialog("Test File DIalog", { { "All file types", { "*.*" } } }, [](const std::vector<ostd::String>& fileList, bool wasCanceled){

				}, true);
			}
		}
	}
	else if (signal.ID == ostd::tBuiltinSignals::WindowResized)
	{
		if (!m_vpiano.isRenderingToFile())
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
	else if (signal.ID == VirtualPiano::MidiStartSignal)
	{
		if (m_vpiano.hasAudioFile() && !m_vpiano.isRenderingToFile())
		{
			m_vpiano.getAudioFile().play();
			m_vpiano.getAudioFile().setPlayingOffset(sf::seconds(m_vpiano.getAutoSoundStart()));
		}
	}
}

void Window::onEventPoll(const std::optional<sf::Event>& event)
{
	m_gui.onEventPoll(event);
}

void Window::onRender(void)
{
	m_vpiano.render();
	m_gui.draw();
}

void Window::onFixedUpdate(void)
{
}

void Window::onUpdate(void)
{
	m_vpiano.update();
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

/*
bool Window::buildGUI(tgui::BackendGui& gui)
{
	try
	{
		auto tabs = tgui::Tabs::create();
		tabs->setRenderer(theme.getRenderer("Tabs"));
		tabs->setTabHeight(30);
		tabs->setPosition(70, 40);
		tabs->add("Tab - 1");
		tabs->add("Tab - 2");
		tabs->add("Tab - 3");
		gui.add(tabs);

		auto radioButton = tgui::RadioButton::create();
		radioButton->setRenderer(theme.getRenderer("RadioButton"));
		radioButton->setPosition(20, 140);
		radioButton->setText("Yep!");
		radioButton->setSize(25, 25);
		gui.add(radioButton);

		radioButton = tgui::RadioButton::create();
		radioButton->setRenderer(theme.getRenderer("RadioButton"));
		radioButton->setPosition(20, 170);
		radioButton->setText("Nope!");
		radioButton->setSize(25, 25);
		gui.add(radioButton);

		radioButton = tgui::RadioButton::create();
		radioButton->setRenderer(theme.getRenderer("RadioButton"));
		radioButton->setPosition(20, 200);
		radioButton->setText("Don't know!");
		radioButton->setSize(25, 25);
		gui.add(radioButton);

		auto editBox = tgui::EditBox::create();
		editBox->setRenderer(theme.getRenderer("EditBox"));
		editBox->setSize(200, 25);
		editBox->setTextSize(18);
		editBox->setPosition(10, 270);
		editBox->setDefaultText("Click to edit text...");
		gui.add(editBox);

		auto listBox = tgui::ListBox::create();
		listBox->setRenderer(theme.getRenderer("ListBox"));
		listBox->setSize(250, 120);
		listBox->setItemHeight(24);
		listBox->setPosition(10, 340);
		listBox->addItem("Item 1");
		listBox->addItem("Item 2");
		listBox->addItem("Item 3");
		gui.add(listBox);

		auto slider = tgui::Slider::create();
		slider->setRenderer(theme.getRenderer("Slider"));
		slider->setPosition(10, 560);
		slider->setSize(200, 18);
		slider->setValue(4);
		gui.add(slider);

		auto scrollbar = tgui::Scrollbar::create();
		scrollbar->setRenderer(theme.getRenderer("Scrollbar"));
		scrollbar->setPosition(380, 40);
		scrollbar->setSize(18, 540);
		scrollbar->setMaximum(100);
		scrollbar->setViewportSize(70);
		gui.add(scrollbar);

		auto comboBox = tgui::ComboBox::create();
		comboBox->setRenderer(theme.getRenderer("ComboBox"));
		comboBox->setSize(120, 21);
		comboBox->setPosition(420, 40);
		comboBox->addItem("Item 1");
		comboBox->addItem("Item 2");
		comboBox->addItem("Item 3");
		comboBox->setSelectedItem("Item 2");
		gui.add(comboBox);

		auto child = tgui::ChildWindow::create();
		child->setRenderer(theme.getRenderer("ChildWindow"));
		child->setClientSize({ 250, 120 });
		child->setPosition(420, 80);
		child->setTitle("Child window");
		gui.add(child);

		auto button = tgui::Button::create();
		button->setRenderer(theme.getRenderer("Button"));
		button->setPosition(75, 70);
		button->setText("OK");
		button->setSize(100, 30);
		button->onPress([=] { child->setVisible(false); });
		button->onPress([this, fileDialog]() {
			fileDialog->setVisible(true);
		});
		child->add(button);

		auto checkbox = tgui::CheckBox::create();
		checkbox->setRenderer(theme.getRenderer("CheckBox"));
		checkbox->setPosition(420, 240);
		checkbox->setText("Ok, I got it");
		checkbox->setSize(25, 25);
		gui.add(checkbox);

		checkbox = tgui::CheckBox::create();
		checkbox->setRenderer(theme.getRenderer("CheckBox"));
		checkbox->setPosition(570, 240);
		checkbox->setText("No, I didn't");
		checkbox->setSize(25, 25);
		gui.add(checkbox);

		auto chatbox = tgui::ChatBox::create();
		chatbox->setRenderer(theme.getRenderer("ChatBox"));
		chatbox->setSize(300, 100);
		chatbox->setTextSize(18);
		chatbox->setPosition(420, 310);
		chatbox->setLinesStartFromTop();
		chatbox->addLine("texus: Hey, this is TGUI!", tgui::Color::Green);
		chatbox->addLine("Me: Looks awesome! ;)", tgui::Color::Yellow);
		chatbox->addLine("texus: Thanks! :)", tgui::Color::Green);
		chatbox->addLine("Me: The widgets rock ^^", tgui::Color::Yellow);
		gui.add(chatbox);

		button = tgui::Button::create();
		button->setRenderer(theme.getRenderer("Button"));
		button->setPosition(gui.getView().getSize().x - 115.f, gui.getView().getSize().y - 50.f);
		button->setText("Exit");
		button->setSize(100, 40);
		gui.add(button);
	}
	catch (const tgui::Exception& e)
	{
		std::cerr << "TGUI Exception: " << e.what() << std::endl;
		return false;
	}

	return true;
}
*/
