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

#include "Gui.hpp"
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <algorithm>
#include <ostd/Logger.hpp>
#include <ostd/Signals.hpp>
#include "Common.hpp"
#include "Renderer.hpp"

Gui& Gui::init(WindowBase& window, const ostd::String& cursorFilePath, const ostd::String& appIconFilePath, const ostd::String& themeFilePath, bool visible)
{
	invalidate();
	m_window = &window;
	m_visible = visible;

	// Load theme file
	m_tguiTheme.load(themeFilePath.c_str());

	// Load cursor file
	sf::Image cursorImage;
	if (cursorImage.loadFromFile(cursorFilePath))
	{
		sf::Cursor tempCursor(cursorImage.getPixelsPtr(), cursorImage.getSize(), sf::Vector2u(0, 0));
		m_cursor = std::move(tempCursor);
		m_window->sfWindow().setMouseCursor(*m_cursor);
	}
	else return *this; //TODO: Error

	sf::Image splashScreen;
	if (splashScreen.loadFromFile("themes/ui/logo.png"))
	{
	    (void)m_splashScreenTex.loadFromImage(splashScreen);
		m_splashScreenSpr = sf::Sprite(m_splashScreenTex);
		float x = (m_window->getWindowWidth() / 2.0f) - (m_splashScreenTex.getSize().x / 2.0f);
		float y = (m_window->getWindowHeight() / 2.0f) - (m_splashScreenTex.getSize().y / 2.0f);
		m_splashScreenSpr->setPosition({ x, y });
		m_splashScreenTimer.restart();
	}
	else return *this; //TODO: Error

	// Load Icon file
	if (m_AppIcon.loadFromFile(appIconFilePath))
    {
        m_window->sfWindow().setIcon(m_AppIcon.getSize(), m_AppIcon.getPixelsPtr());
    }
    //TODO: Error

	enableSignals();
	connectSignal(ostd::tBuiltinSignals::WindowClosed);
	setTypeName("Gui");
	validate();
	__build_gui();
	return *this;
}

void Gui::handleSignal(ostd::tSignal& signal)
{
	if (signal.ID == ostd::tBuiltinSignals::WindowClosed)
	{
		m_gui.removeAllWidgets();
		static sf::Cursor defaultCursor(sf::Cursor::Type::Arrow);
		m_window->sfWindow().setMouseCursor(defaultCursor);
		m_cursor.reset();
		OX_DEBUG("Gui Cleanup complete.");
	}
}

void Gui::showFileDialog(const ostd::String& title, const Gui::FileDialogFilterList& filters, std::function<void(const std::vector<ostd::String>&, bool)> callback, bool multiselect)
{
	if (!isValid()) return;
	m_fileDialog->onFileSelect.disconnectAll();
	m_fileDialog->onCancel.disconnectAll();
	// m_fileDialog->setPath(".");
	m_fileDialog->setTitle(title.cpp_str());
	m_fileDialog->setFileTypeFilters(filters);
	if (m_fileDialog->getParent() == nullptr)
	    m_gui.add(m_fileDialog);
	m_fileDialog->setVisible(true);
	m_fileDialog->setMultiSelect(multiselect);
	m_fileDialog->onFileSelect([this, callback](const tgui::Filesystem::Path& path) {
		if (!this->isValid()) return;  //This should never happen
		auto files = this->m_fileDialog->getSelectedPaths();
		std::vector<ostd::String> _files;
		_files.reserve(files.size());
		std::transform(files.begin(), files.end(), std::back_inserter(_files), [](const std::filesystem::path& p) {
			return ostd::String(p.string());
		});
		callback(_files, false);
	});
	m_fileDialog->onCancel([this, callback](void) {
		if (!this->isValid()) return;  //This should never happen
		callback({  }, true);
	});
}

void Gui::showVideoRenderingGui(const VideoRenderState& renderState)
{
	renderState.isFinished();
}

void Gui::draw(void)
{
	if (!isValid()) return;
	if (!isVisible()) return;

	Renderer::outlineRect({ 0.0f, 0.0f, Common::scaleX(600), (float)m_window->getWindowHeight()}, { 0, 0, 0, 230 }, { 230, 230, 230, 230 }, 2);

	m_gui.draw();

	__draw_fps();
	__show_splashscreen();
}

void Gui::onEventPoll(const std::optional<sf::Event>& event)
{
	if (!isValid()) return;
	m_gui.handleEvent(*event);
}

void Gui::__build_gui(void)
{
	if (!isValid()) return;

	m_gui.setWindow(m_window->sfWindow());

	// FileDialog
	m_fileDialog = tgui::FileDialog::create();
	m_fileDialog->setRenderer(m_tguiTheme.getRenderer("FileDialog"));
	m_fileDialog->setVisible(false);
	m_fileDialog->setFileMustExist(true);
	m_gui.add(m_fileDialog);
}

void Gui::__show_splashscreen(void)
{
	if (!isValid()) return;

	if (m_showSplashScreen)
	{
		float elapsed = m_splashScreenTimer.getElapsedTime().asSeconds();
		float x = (m_window->getWindowWidth() / 2.0f) - (m_splashScreenTex.getSize().x / 2.0f);
		float y = (m_window->getWindowHeight() / 2.0f) - (m_splashScreenTex.getSize().y / 2.0f);
		m_splashScreenSpr->setPosition({ x, y });
		if (elapsed < 1.5f)
		{
        	m_window->sfWindow().draw(*m_splashScreenSpr);
		}
		else if (elapsed < 3.0f)
        {
        	float alpha = 255.f * (1.f - (elapsed - 1.5f) / 1.5f);
	        m_splashScreenSpr->setColor(sf::Color(255, 255, 255, (uint8_t)alpha));
			m_window->sfWindow().draw(*m_splashScreenSpr);
        }
		else
		{
			m_splashScreenSpr->setColor(sf::Color(255, 255, 255, 0));
			m_showSplashScreen = false;
		}
	}
}

void Gui::__draw_fps(void)
{
	if (isInvalid()) return;
	if (!m_showFPS) return;
	ostd::String fps_text = "FPS: ";
	fps_text.add(m_window->getFPS());
	int32_t fontSize = 26;
	auto stringSize = Renderer::getStringSize(fps_text, fontSize);
	Renderer::drawString(fps_text, { (float)m_window->getWindowWidth() - stringSize.x - 10 , 10 }, { 220, 170, 0 }, fontSize);
}
