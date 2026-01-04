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

#include "VPianoData.hpp"
#include "VideoRenderer.hpp"
#include <ostd/Json.hpp>
#include "VPianoResources.hpp"
#include "VirtualKeyboard.hpp"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderTexture.hpp>


class Window;
class VirtualPiano
{

	public:
		// Core functionality
		inline VirtualPiano(Window& parentWindow) : m_vPianoRes(*this), m_sigListener(*this), m_parentWindow(parentWindow), m_videoRenderer(*this), m_vKeyboard(*this) {  }
		void init(void);
		void loadProjectFile(const ostd::String& filePath);
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
		inline sf::RenderTexture& __apply_blur(uint8_t passes = 6, float intensity = 1.0f, float start_offset = 1.0f, float increment = 1.0f, float threshold = 0.1f);
		inline sf::RenderTexture& __apply_kawase_blur(uint8_t passes = 6, float intensity = 1.0f, float start_offset = 1.0f, float increment = 1.0f, float threshold = 0.1f);
		inline sf::RenderTexture& __apply_gaussian_blur(uint8_t passes = 6, float intensity = 1.0, float start_radius = 1.0f, float increment = 1.0f, float threshold = 0.1f);

	private:
		Window& m_parentWindow;
		ostd::JsonFile m_configJson;
		ostd::JsonFile m_projJson;
		ostd::JsonFile m_styleJson;
		ostd::JsonFile m_partJson;
		VirtualPianoData m_vPianoData;
		VPianoResources m_vPianoRes;
		VideoRenderer m_videoRenderer;
		SignalListener m_sigListener;
		VirtualKeyboard m_vKeyboard;

		bool m_playing { false };
		bool m_paused { false };
		bool m_firstNotePlayed { false };
		double m_startTimeOffset_ns { 0.0 };
		double m_pausedOffset_ns { 0.0 };
		double m_pausedTime_ns { 0.0 };
		uint16_t m_partPerFrame { 10 }
;
		sf::RenderTexture m_glowBuffer;
		sf::RenderTexture m_blurBuff1;
		sf::RenderTexture m_blurBuff2;
		sf::RenderTexture m_hollowBuff;
		sf::View m_glowView;
		bool m_showBackground { true };

		friend class SignalListener;
		friend class VPianoResources;
		friend class VirtualKeyboard;
};
