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
#include <TGUI/Color.hpp>
#include <TGUI/Widgets/ColorPicker.hpp>
#include <algorithm>
#include <ostd/Color.hpp>
#include <ostd/Geometry.hpp>
#include <ostd/Logger.hpp>
#include <ostd/Signals.hpp>
#include <ostd/String.hpp>
#include <ostd/Utils.hpp>
#include "Common.hpp"
#include "Renderer.hpp"
#include "SFMLWindow.hpp"
#include "VPianoData.hpp"
#include "VirtualPiano.hpp"



Gui::ColorPickerBlock& Gui::ColorPickerBlock::create(const ostd::Color& color, const ostd::String& title)
{
	m_color = color;
	m_title = title;

	enableSignals(true);
	connectSignal(ostd::tBuiltinSignals::MouseReleased);
	connectSignal(ostd::tBuiltinSignals::MouseMoved);
	setTypeName("Gui::ColorPickerBlock");
	validate();
	return *this;
}

void Gui::ColorPickerBlock::handleSignal(ostd::tSignal& signal)
{
	auto l_contains = [this](MouseEventData& med) -> bool {
		return this->contains((float)med.position_x, (float)med.position_y, true);
	};

	if (signal.ID == ostd::tBuiltinSignals::MouseReleased)
	{
		MouseEventData& med = (MouseEventData&)(signal.userData);
		if (l_contains(med) && med.button == MouseEventData::eButton::Left)
		{
			m_parent.showColorPicker(m_title, m_color, { (float)med.position_x, (float)med.position_y }, [this](const ostd::Color& color){
				this->m_color = color;
			});
		}
	}
	else if (signal.ID == ostd::tBuiltinSignals::MouseMoved)
	{
		MouseEventData& med = (MouseEventData&)(signal.userData);
		if (l_contains(med))
		{
			m_isMouseInside = true;
		}
		else
		{
			m_isMouseInside = false;
		}
	}
}

void Gui::ColorPickerBlock::render(void)
{
	Renderer::outlineRoundedRect(*this, m_color, (m_isMouseInside ? m_borderColor_hover : m_borderColor), { 10, 10, 10, 10 }, 2);
}




Gui& Gui::init(WindowBase& window, VideoRenderState& videoRenderState, const ostd::String& cursorFilePath, const ostd::String& appIconFilePath, const ostd::String& themeFilePath, bool visible)
{
	invalidate();
	m_window = &window;
	m_videoRenderState = &videoRenderState;
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

    m_showSplashScreen = false; //TODO: remove

    m_labelUpdateTimer.startCount(ostd::eTimeUnits::Milliseconds);

	enableSignals();
	connectSignal(ostd::tBuiltinSignals::WindowClosed);
	connectSignal(ostd::tBuiltinSignals::WindowResized);
	setTypeName("Gui");
	validate();
	__build_gui();
	return *this;
}

void Gui::handleSignal(ostd::tSignal& signal)
{
	if (!isValid()) return;
	if (signal.ID == ostd::tBuiltinSignals::WindowClosed)
	{
		m_gui.removeAllWidgets();
		static sf::Cursor defaultCursor(sf::Cursor::Type::Arrow);
		m_window->sfWindow().setMouseCursor(defaultCursor);
		m_cursor.reset();
		OX_DEBUG("Gui Cleanup complete.");
	}
	else if (signal.ID == ostd::tBuiltinSignals::WindowResized)
	{
		__update_widgets_positions();
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

void Gui::showColorPicker(const ostd::String& title, const ostd::Color& setColor, const ostd::Vec2& position, std::function<void(const ostd::Color&)> callback)
{
	if (!isValid()) return;
	m_colorPicker->onOkPress.disconnectAll();
	m_colorPicker->onUnfocus.disconnectAll();
	m_colorPicker->setTitle(title.cpp_str());
	m_colorPicker->setColor(tgui_color(setColor));
	m_colorPicker->setPosition(position.x, position.y);
	m_colorPicker->setWidgetName("m_colorPicker");
	if (m_colorPicker->getParent() == nullptr)
	    m_gui.add(m_colorPicker);
	m_colorPicker->setVisible(true);
	m_colorPicker->onOkPress([this, callback](const tgui::Color& color) {
		if (!this->isValid()) return;  //This should never happen
		callback({ color.getRed(), color.getGreen(), color.getBlue(), color.getAlpha() });
	});
	m_colorPicker->onUnfocus([this](void) {
		if (!this->isValid()) return;  //This should never happen
		this->m_colorPicker->setVisible(false);
		this->m_colorPicker->onOkPress.emit( this->m_colorPicker->get("m_colorPicker").get(), this->m_colorPicker->getColor() );
	});
	m_colorPicker->setFocused(true);
}

void Gui::showVideoRenderingGui(void)
{
	if (!m_videoRenderState->virtualPiano.getVideoRenderer().isRenderingToFile())
		return;

	m_isRenderingVideo = true;
	__update_widgets_positions();
	m_renderingProgressBar->setVisible(true);
	m_renderingProgressBar->setValue(0);
}

void Gui::draw(void)
{
	if (!isValid()) return;

	__draw_sidebar();
	__draw_videoRenderGui();
	if (isVisible()) m_gui.draw();
	__draw_fps();
	__show_splashscreen();
}

void Gui::onEventPoll(const std::optional<sf::Event>& event)
{
	if (!isValid()) return;
	m_gui.handleEvent(*event);
}

void Gui::__update_widgets_positions(void)
{
	auto bounds = __get_center_bounds(m_renderingProgressBarSize);
	bounds.y -= Common::scaleY((m_renderingGuiSize.y / 2.0f) - (m_renderingProgressBarSize.y / 2.0) - m_renderingProgressBarPadding);
	m_renderingProgressBar->setPosition(bounds.x, bounds.y);
	m_renderingProgressBar->setSize(bounds.w, bounds.h);
	m_renderingGuiPosition = __get_center_bounds(m_renderingGuiSize).topLeft();
}

ostd::Rectangle Gui::__get_center_bounds(const ostd::Vec2& size)
{
	ostd::Rectangle bounds;
	bounds.w = Common::scaleX(size.x);
	bounds.h = Common::scaleY(size.y);
	bounds.x = ((float)m_window->getWindowWidth() / 2.0f) - (bounds.w / 2.0f);
	bounds.y = ((float)m_window->getWindowHeight() / 2.0f) - (bounds.h / 2.0f);
	return bounds;
}

void Gui::__build_gui(void)
{
	if (!isValid()) return;

	m_colorPickerTest1.setBounds(20, 20, 100, 30);
	m_colorPickerTest2.setBounds(20, 60, 100, 30);

	m_gui.setWindow(m_window->sfWindow());

	// FileDialog
	m_fileDialog = tgui::FileDialog::create();
	m_fileDialog->setRenderer(m_tguiTheme.getRenderer("FileDialog"));
	m_fileDialog->setVisible(false);
	m_fileDialog->setFileMustExist(true);
	m_gui.add(m_fileDialog);

	// Color picker
	m_colorPicker = tgui::ColorPicker::create();
	m_colorPicker->setRenderer(m_tguiTheme.getRenderer("ColorPicker"));
	m_colorPicker->setVisible(false);
	m_gui.add(m_colorPicker);

	m_renderingProgressBar = tgui::ProgressBar::create();
	m_renderingProgressBar->setRenderer(m_tguiTheme.getRenderer("ProgressBar"));
	m_renderingProgressBar->setValue(0);
	m_renderingProgressBar->setVisible(false);
	m_renderingProgressBar->setMaximum(100);
	m_renderingProgressBar->setMinimum(0);
	m_renderingProgressBar->getRenderer()->setFillColor(tgui::Color(110, 20, 20));
	m_renderingProgressBar->getRenderer()->setBackgroundColor(tgui::Color(30, 5, 5));
	m_renderingProgressBar->getRenderer()->setTextColor(tgui::Color(180, 180, 180));
	m_renderingProgressBar->getRenderer()->setBorderColor(tgui::Color(130, 130, 130));
	m_renderingProgressBar->getRenderer()->setTextSize(24);
	m_renderingProgressBar->setText("");
	m_gui.add(m_renderingProgressBar);
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
	if (isInvalid() || m_isRenderingVideo) return;
	if (!m_visible && m_showFPS) return;
	ostd::String fps_text = "FPS: ";
	fps_text.add(m_window->getFPS());
	int32_t fontSize = 26;
	auto stringSize = Renderer::getStringSize(fps_text, fontSize);
	Renderer::drawString(fps_text, { (float)m_window->getWindowWidth() - stringSize.x - 10 , 10 }, { 220, 170, 0 }, fontSize);
}

void Gui::__draw_sidebar(void)
{
	if (!isVisible() || m_isRenderingVideo) return;
	Renderer::outlineRect({ 0.0f, 0.0f, Common::scaleX(600), (float)m_window->getWindowHeight()}, { 0, 0, 0, 230 }, { 230, 230, 230, 230 }, 2);

	m_colorPickerTest1.render();
	m_colorPickerTest2.render();
}

void Gui::__draw_videoRenderGui(void)
{
	if (!m_isRenderingVideo) return;
	auto padx = [](float pad, const ostd::String& str, uint32_t fontSize) -> float {
		return Common::scaleX(pad) + Renderer::getStringSize(str, fontSize).x;
	};
	auto& vrs = *m_videoRenderState;
	auto& basePos = m_renderingGuiPosition;

	ostd::Vec2 pos;
	ostd::String label = "";
	uint32_t fontSize = Common::scaleXY(35);
	ostd::Color color1 = { 210, 210, 210 }, color2 = { 120, 120, 120, 120 };
	switch (vrs.mode)
	{
		case VideoRenderModes::Video:
			label = "Rendering Video";
			break;
		case VideoRenderModes::ImageSequence:
			label = "Rendering Image Sequence";
			break;
	}
	auto guiBounds = __get_center_bounds(m_renderingGuiSize);


	Renderer::outlineRoundedRect(guiBounds, { 0, 0, 0, 230 }, color2, { 20, 20, 20, 20 }, 2);
	guiBounds.h = Common::scaleY(m_renderingProgressBarPadding - 20);
	Renderer::outlineRoundedRect(guiBounds, { 140, 20, 120, 230 }, color2, { 20, 20, 0, 0 }, 2);
	auto titleBounds = Renderer::getStringSize(label, fontSize);
	pos.x = guiBounds.x + ((guiBounds.w / 2.0f) - (titleBounds.x / 2.0f));
	pos.y = guiBounds.y + ((guiBounds.h / 2.0f) - (titleBounds.y / 2.0f)) - Common::scaleY(10);
	Renderer::drawString(label, pos, color1, fontSize);


	bool updateLabels = m_labelUpdateTimer.read() >= 500;
	ostd::String percentageStr = "";
	percentageStr.add(vrs.percentage);
	percentageStr.add(" %");
	m_renderingProgressBar->setText(percentageStr.cpp_str());
	m_renderingProgressBar->setValue(vrs.percentage);

	auto pbpos = __get_center_bounds(m_renderingProgressBarSize);
	pos = { pbpos.x, basePos.y + Common::scaleY(m_renderingProgressBarPadding + m_renderingProgressBarSize.y + 20) };
	color1 = { 200, 200, 230 };
	color2 = { 250, 210, 10 };
	fontSize = Common::scaleXY(24);

	label = "Frames: ";
	Renderer::drawString(label, pos, color1, fontSize);
	pos.x += padx(-5, label, fontSize);
	label.clr().add(vrs.frameIndex - 1).add("/").add(vrs.totalFrames + vrs.extraFrames);
	Renderer::drawString(label, pos, color2, fontSize);

	label = "FPS: ";
	pos.x += Common::scaleX(250);
	Renderer::drawString(label, pos, color1, fontSize);
	pos.x += padx(-5, label, fontSize);
	label.clr().add(vrs.renderFPS);
	if (!updateLabels)
		label = m_oldFPSLabel;
	else
		m_oldFPSLabel = label;
	Renderer::drawString(label, pos, color2, fontSize);

	float etaStrSize = 0;
	auto tmpPos = pos;
	int32_t totalSeconds = 0;
	if (vrs.renderFPS > 0)
		totalSeconds = static_cast<int32_t>(((vrs.totalFrames + vrs.extraFrames) - vrs.frameIndex) / vrs.renderFPS);
	label.clr().add(Common::secondsToFormattedString(totalSeconds));
	if (!updateLabels || totalSeconds > m_oldETASeconds)
		label = m_oldETALabel;
	else
		m_oldETALabel = label;
	etaStrSize += Renderer::getStringSize(label, fontSize).x;
	etaStrSize += Common::scaleX(2);
	pos.x = pbpos.x + pbpos.w - etaStrSize;
	Renderer::drawString(label, pos, color2, fontSize);
	label = "ETA: ";
	pos.x -= Common::scaleX(Renderer::getStringSize(label, fontSize).x - 2);
	Renderer::drawString(label, pos, color1, fontSize);

	Renderer::drawTexture(vrs.flippedRenderTarget.getTexture(), { pbpos.x, pos.y + Common::scaleY(110) }, { Common::scaleXY(0.2f), Common::scaleXY(0.2f) }, { 140, 140, 140 });
	auto tmpSize = vrs.flippedRenderTarget.getTexture().getSize();
	ostd::Vec2 previewSize = { (float)tmpSize.x * Common::scaleXY(0.2f), (float)tmpSize.y * Common::scaleXY(0.2f) };
	Renderer::drawRoundedRect({ pbpos.x, pos.y + Common::scaleY(110), previewSize.x, previewSize.y }, { 140, 20, 120, 230 }, { 5, 5, 5, 5 }, 3);

	pos = { guiBounds.x, pos.y + Common::scaleY(40) };
	Renderer::fillRect({ pos.x, pos.y, guiBounds.w, 2 }, { 120, 120, 120, 120 });

	fontSize = Common::scaleXY(22);
	label = "File: ";
	pos.x = pbpos.x;
	pos.y += Common::scaleY(20);
	Renderer::drawString(label, pos, color1, fontSize);
	pos.x += padx(-5, label, fontSize);
	label.clr().add(vrs.absolutePath);
	Renderer::drawString(label, pos, color2, fontSize);

	fontSize = Common::scaleXY(24);
	color2 = { 240, 28, 180 };
	pos = { pbpos.x + previewSize.x + Common::scaleX(10), pos.y + Common::scaleY(50) };
	label = "Resolution: ";
	Renderer::drawString(label, pos, color1, fontSize);
	pos.x += padx(-5, label, fontSize);
	label.clr().add(vrs.resolution.x).add("x").add(vrs.resolution.y);
	Renderer::drawString(label, pos, color2, fontSize);

	pos = { pbpos.x + previewSize.x + Common::scaleX(10), pos.y += Common::scaleY(40) };
	label = "Target FPS: ";
	Renderer::drawString(label, pos, color1, fontSize);
	pos.x += padx(-5, label, fontSize);
	label.clr().add(vrs.targetFPS);
	Renderer::drawString(label, pos, color2, fontSize);

	pos = { pbpos.x + previewSize.x + Common::scaleX(10), pos.y += Common::scaleY(40) };
	label = "Duration: ";
	Renderer::drawString(label, pos, color1, fontSize);
	pos.x += padx(-5, label, fontSize);
	int32_t seconds = (int32_t)std::round(vrs.lastNoteEndTime + ((double)vrs.extraFrames / (double)vrs.targetFPS));
	label.clr().add(Common::secondsToFormattedString(seconds));
	Renderer::drawString(label, pos, color2, fontSize);

	if (updateLabels)
	{
		m_labelUpdateTimer.endCount();
		m_labelUpdateTimer.startCount(ostd::eTimeUnits::Milliseconds);
		if (totalSeconds < m_oldETASeconds)
			m_oldETASeconds = totalSeconds;
	}
}
