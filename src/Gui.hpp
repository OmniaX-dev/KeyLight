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

#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Clock.hpp>
#include <TGUI/Loading/Theme.hpp>
#include <TGUI/Widgets/Label.hpp>
#include <TGUI/Widgets/ProgressBar.hpp>
#include <functional>
#include <optional>
#include <ostd/BaseObject.hpp>
#include <ostd/Geometry.hpp>
#include <ostd/String.hpp>

#include <TGUI/Widgets/FileDialog.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <ostd/Utils.hpp>

#include "SFMLWindow.hpp"
#include "VPianoDataStructures.hpp"

class Gui : public ostd::BaseObject
{
	public:
		using FileDialogFilterList = std::vector<std::pair<tgui::String, std::vector<tgui::String>>>;

	public:
		inline Gui(void) { invalidate(); }
		inline Gui(WindowBase& window, VideoRenderState& videoRenderState, const ostd::String& cursorFilePath, const ostd::String& appIconFilePath, const ostd::String& themeFilePath, bool visible = true) { init(window, videoRenderState, cursorFilePath, appIconFilePath, themeFilePath, visible); }
		Gui& init(WindowBase& window, VideoRenderState& videoRenderState, const ostd::String& cursorFilePath, const ostd::String& appIconFilePath, const ostd::String& themeFilePath, bool visible = true);
		void handleSignal(ostd::tSignal& signal) override;
		void showFileDialog(const ostd::String& title, const FileDialogFilterList& filters, std::function<void(const std::vector<ostd::String>&, bool)> callback, bool multiselect = false);
		void showVideoRenderingGui(void);
		void draw(void);

		inline bool isVisible(void) const { return m_visible; }
		inline void show(bool _show = true) { m_visible = _show; }
		inline void toggleVisibility(void) { m_visible = !m_visible; }
		inline void showFPS(bool show = true) { m_showFPS = show; }

		void onEventPoll(const std::optional<sf::Event>& event);

	private:
		ostd::Rectangle __get_center_bounds(const ostd::Vec2& size);
		void __update_widgets_positions(void);
		void __build_gui(void);
		void __show_splashscreen(void);
		void __draw_fps(void);
		void __draw_sidebar(void);
		void __draw_videoRenderGui(void);

	private:
		WindowBase* m_window { nullptr };
		tgui::Gui m_gui;
		tgui::Theme m_tguiTheme;
		std::optional<sf::Cursor> m_cursor;
		sf::Image m_AppIcon;
		bool m_visible { true };
		bool m_showFPS { false };
		bool m_isRenderingVideo { false };

		// Splash screen
		sf::Texture m_splashScreenTex;
		std::optional<sf::Sprite> m_splashScreenSpr;
		sf::Clock m_splashScreenTimer;
		bool m_showSplashScreen { true };

		// File dialog
		tgui::FileDialog::Ptr m_fileDialog { nullptr };

		// Video rendering gui
		tgui::ProgressBar::Ptr m_renderingProgressBar { nullptr };
		ostd::Vec2 m_renderingGuiSize { 900, 490 };
		ostd::Vec2 m_renderingGuiPosition { 0, 0 };
		ostd::Vec2 m_renderingProgressBarSize { 800, 50 };
		float m_renderingProgressBarPadding { 80 };
		const VideoRenderState* m_videoRenderState { nullptr };

};
