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
#include <functional>
#include <optional>
#include <ostd/BaseObject.hpp>
#include <ostd/String.hpp>

#include <TGUI/Widgets/FileDialog.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <ostd/Utils.hpp>

#include "SFMLWindow.hpp"

class Gui : public ostd::BaseObject
{
	public:
		using FileDialogFilterList = std::vector<std::pair<tgui::String, std::vector<tgui::String>>>;

	public:
		inline Gui(void) { invalidate(); }
		inline Gui(WindowBase& window, const ostd::String& cursorFilePath, const ostd::String& appIconFilePath, const ostd::String& themeFilePath, bool visible = true) { init(window, cursorFilePath, appIconFilePath, themeFilePath, visible); }
		Gui& init(WindowBase& window, const ostd::String& cursorFilePath, const ostd::String& appIconFilePath, const ostd::String& themeFilePath, bool visible = true);
		void handleSignal(ostd::tSignal& signal) override;
		void showFileDialog(const ostd::String& title, const FileDialogFilterList& filters, std::function<void(const std::vector<ostd::String>&, bool)> callback, bool multiselect = false);
		void draw(void);

		inline bool isVisible(void) const { return m_visible; }
		inline void show(bool _show = true) { m_visible = _show; }
		inline void toggleVisibility(void) { m_visible = !m_visible; }
		inline void showFPS(bool show = true) { m_showFPS = show; }

		void onEventPoll(const std::optional<sf::Event>& event);

	private:
		void __build_gui(void);
		void __show_splashscreen(void);
		void __draw_fps(void);

	private:
		tgui::Gui m_gui;
		std::optional<sf::Cursor> m_cursor;
		sf::Image m_AppIcon;
		WindowBase* m_window { nullptr };
		bool m_visible { true };
		bool m_showFPS { false };

		sf::Texture m_splashScreenTex;
		std::optional<sf::Sprite> m_splashScreenSpr;
		sf::Clock m_splashScreenTimer;
		bool m_showSplashScreen { true };

		tgui::FileDialog::Ptr m_fileDialog { nullptr };
		tgui::Theme m_tguiTheme;
};
