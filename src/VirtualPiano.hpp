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
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <functional>
#include <optional>
#include <ostd/BaseObject.hpp>
#include <ostd/Signals.hpp>
#include <ostd/Utils.hpp>
#include <ostd/Geometry.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include "VPianoData.hpp"
#include "VideoRenderer.hpp"
#include "JSONManager.hpp"
#include "VPianoResources.hpp"
#include "VirtualKeyboard.hpp"

class Window;
class VirtualPiano
{

	public:
		// Core functionality
		inline VirtualPiano(Window& parentWindow) : m_vPianoRes(*this), m_sigListener(*this), m_parentWindow(parentWindow), m_videoRenderer(*this), m_vKeyboard(*this) {  }
		void init(void);
		void onWindowResized(uint32_t width, uint32_t height);

		// Playback functionality
		void play(void);
		void pause(void);
		void stop(void);
		double getPlayTime_s(void);

		// Update and Render
		void update(void);
		void render(std::optional<std::reference_wrapper<sf::RenderTarget>> target = std::nullopt);
		void renderFrame(std::optional<std::reference_wrapper<sf::RenderTarget>> target = std::nullopt);

		// Getters and Setters
		inline VirtualPianoData& vPianoData(void) { return m_vPianoData; }
		inline VPianoResources& vPianoRes(void) { return m_vPianoRes; }
		inline VirtualKeyboard& vKeyboard(void) { return m_vKeyboard; }
		inline VideoRenderer& getVideoRenderer(void) { return m_videoRenderer; }
		inline bool isPlaying(void) { return m_playing; }
		inline Window& getParentWindow(void) { return m_parentWindow; }

	private:
		Window& m_parentWindow;
		JSONManager m_config;
		VirtualPianoData m_vPianoData;
		VPianoResources m_vPianoRes;
		VideoRenderer m_videoRenderer;
		SignalListener m_sigListener;
		VirtualKeyboard m_vKeyboard;

		bool m_playing { false };
		bool m_paused { false };
		bool m_firstNotePlayed { false };
		double m_startTimeOffset_ns { 0.0 };

		sf::RenderTexture m_glowBuffer;
		sf::RenderTexture m_blurBuff1;
		sf::RenderTexture m_blurBuff2;
		sf::View m_glowView;
		bool m_showBackground { true };

		friend class SignalListener;
		friend class VPianoResources;
		friend class VirtualKeyboard;
};
