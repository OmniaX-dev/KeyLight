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

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <functional>
#include <optional>
#include <ostd/BaseObject.hpp>
#include <ostd/Signals.hpp>
#include <ostd/Utils.hpp>
#include <deque>
#include "MidiParser.hpp"
#include <ostd/Geometry.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#include "VPianoDataStructures.hpp"
#include "ffmpeg_helper.hpp"

class Window;
class VirtualPiano
{

	public:
		// Core functionality
		inline VirtualPiano(Window& parentWindow) : m_sigListener(*this), m_parentWindow(parentWindow), m_videoRenderState(*this) {  }
		void init(void);
		void onWindowResized(uint32_t width, uint32_t height);

		// Playback functionality
		void play(void);
		void pause(void);
		void stop(void);
		double getPlayTime_s(void);

		// Audio functionality
		bool loadMidiFile(const ostd::String& filePath);
		bool loadAudioFile(const ostd::String& filePath);
		float scanMusicStartPoint(const ostd::String& filePath, float thresholdPercent = 0.02f, float minDuration = 0.05f);

		// Update and Render
		void update(void);
		void calculateFallingNotes(double currentTime);
		void updateVisualization(double currentTime);
		void render(std::optional<std::reference_wrapper<sf::RenderTarget>> target = std::nullopt);
		void renderVirtualKeyboard(std::optional<std::reference_wrapper<sf::RenderTarget>> target = std::nullopt);
		void drawFallingNote(const FallingNoteGraphicsData& noteData);
		void drawFallingNoteOutline(const FallingNoteGraphicsData& noteData);
		void drawFallingNoteGlow(const FallingNoteGraphicsData& noteData);

		// File rendering
		bool configImageSequenceRender(const ostd::String& folderPath, const ostd::UI16Point& resolution, uint8_t fps);
		bool configFFMPEGVideoRender(const ostd::String& filePath, const ostd::UI16Point& resolution, uint8_t fps, const FFMPEG::tProfile& profile);

		// Getters and Setters
		inline VirtualPianoData& vPianoData(void) { return m_vPianoData; }
		inline sf::Music& getAudioFile(void) { return m_audioFile; }
		inline float getAutoSoundStart(void) { return m_autoSoundStart; }
		inline bool hasAudioFile(void) { return m_hasAudioFile; }
		inline bool isPlaying(void) { return m_playing; }
		inline bool isRenderingToFile(void) { return m_isRenderingToFile; }
		inline VideoRenderState& getVideoRenderState(void) { return m_videoRenderState; }

	private:
		void __render_frame(std::optional<std::reference_wrapper<sf::RenderTarget>> target = std::nullopt);
		void __preallocate_file_names_for_rendering(uint32_t frameCount, const ostd::String& baseFileName, const ostd::String& basePath, ImageType imageType, const uint16_t marginFrames = 200);
		FILE* __open_ffmpeg_pipe(const ostd::String& filePath, const ostd::UI16Point& resolution, uint8_t fps, const FFMPEG::tProfile& profile);
		void __save_frame_to_file(const sf::RenderTexture& rt, const ostd::String& basePath, int frameIndex);
		void __stream_frame_to_ffmpeg(void);
		void __render_next_output_frame(void);
		void __finish_output_render(void);

	private:
		Window& m_parentWindow;
		VirtualPianoData m_vPianoData;
		sf::Music m_audioFile;
		ostd::String m_audioFilePath;

		std::vector<PianoKey> m_pianoKeys;
		std::vector<MidiParser::NoteEvent> m_midiNotes;
		std::deque<MidiParser::NoteEvent> m_activeFallingNotes;
		std::vector<FallingNoteGraphicsData> m_fallingNoteGfx_w;
		std::vector<FallingNoteGraphicsData> m_fallingNoteGfx_b;

		std::vector<ostd::String> m_renderFileNames;

		bool m_playing { false };
		bool m_paused { false };
		bool m_isRenderingToFile { false };
		bool m_firstNotePlayed { false };
		bool m_hasAudioFile { false };

		int32_t m_nextFallingNoteIndex { 0 };
		float m_autoSoundStart { 0.0f };

		double m_fallingTime_s { 4.5 };
		double m_startTimeOffset_ns { 0.0 };

		double m_firstNoteStartTime { 0.0 };
		double m_lastNoteEndTime { 0.0 };

		sf::RenderTexture m_glowBuffer;
		sf::RenderTexture m_blurBuff1;
		sf::RenderTexture m_blurBuff2;
		sf::View m_glowView;

		SignalListener m_sigListener;
		VideoRenderState m_videoRenderState;

		ostd::Color m_clearColor { 20, 20, 20 };

	public:
		sf::Shader noteShader;
		sf::Shader blurShader;
		sf::Shader flipShader;
		sf::Texture noteTexture;

	public:
		inline static const uint64_t NoteOnSignal = ostd::SignalHandler::newCustomSignal(5000);
		inline static const uint64_t NoteOffSignal = ostd::SignalHandler::newCustomSignal(5001);
		inline static const uint64_t MidiStartSignal = ostd::SignalHandler::newCustomSignal(5002);
		inline static const uint64_t MidiEndSignal = ostd::SignalHandler::newCustomSignal(5003);

		friend class SignalListener;

};
