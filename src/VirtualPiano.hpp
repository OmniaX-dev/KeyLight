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
#include "Common.hpp"
#include "MidiParser.hpp"
#include <ostd/Geometry.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <vector>

class Window;
class VirtualPiano
{
	public: struct PianoKey
	{
		MidiParser::NoteInfo noteInfo;
		bool pressed { false };
	};
	public: struct VirtualPianoData
	{
		private:
			float pixelsPerSecond { 0 };
			float virtualPiano_x { 0.0f };
			float virtualPiano_y { 0.0f };
			float whiteKeyWidth { 0.0f };
			float whiteKeyHeight { 0.0f };
			float blackKeyWidth { 0.0f };
			float blackKeyHeight { 0.0f };
			float blackKeyOffset { 0.0f };
			std::unordered_map<int32_t, float> _keyOffsets;

			float scale_x { 0.0f };
			float scale_y { 0.0f };

			float whiteKeyShrinkFactor { 0 };
			float blackKeyShrinkFactor { 0 };

			ostd::Rectangle glowMargins { 0, 0, 0, 0 };

		public:
			inline static constexpr int32_t base_width { 2080 };
			inline static constexpr int32_t base_height { 1400 };

			ostd::Color whiteKeyColor { 0, 0, 0 };
			ostd::Color whiteKeyPressedColor { 0, 0, 0 };
			ostd::Color blackKeyColor { 0, 0, 0 };
			ostd::Color blackKeyPressedColor { 0, 0, 0 };

			ostd::Color fallingWhiteNoteColor { 0, 0, 0 };
			ostd::Color fallingWhiteNoteOutlineColor { 0, 0, 0 };
			ostd::Color fallingWhiteNoteGlowColor { 0, 0, 0 };
			ostd::Color fallingBlackNoteColor { 0, 0, 0 };
			ostd::Color fallingBlackNoteOutlineColor { 0, 0, 0 };
			ostd::Color fallingBlackNoteGlowColor { 0, 0, 0 };


		public:
			VirtualPianoData(void);
			void recalculateKeyOffsets(void);
			void updateScale(int32_t width, int32_t height);

			inline void setScale(const ostd::Vec2& scale) { scale_x = scale.x; scale_y = scale.y; }
			inline ostd::Vec2 getScale(void) const { return { scale_x, scale_y }; }
			inline float pps(void) const { return pixelsPerSecond * scale_y; }
			inline float vpx(void) const { return virtualPiano_x * scale_x; }
			inline float vpy(void) const { return virtualPiano_y * scale_y; }
			inline float whiteKey_w(void) const { return whiteKeyWidth * scale_x; }
			inline float whiteKey_h(void) const { return whiteKeyHeight * scale_y; }
			inline float blackKey_w(void) const { return blackKeyWidth * scale_x; }
			inline float blackKey_h(void) const { return blackKeyHeight * scale_y; }
			inline float blackKey_offset(void) const { return blackKeyOffset * scale_x; }
			inline float whiteKey_shrink(void) const { return whiteKeyShrinkFactor * scale_x; }
			inline float blackKey_shrink(void) const { return blackKeyShrinkFactor * scale_x; }
			inline ostd::Rectangle getGlowMargins(void) const { return { glowMargins.x * scale_x, glowMargins.y * scale_y,
																		 glowMargins.w * scale_x, glowMargins.h * scale_y }; }
			inline std::unordered_map<int32_t, float>& keyOffsets(void) { recalculateKeyOffsets(); return _keyOffsets;}
	};
	public: class NoteEventData : public ostd::BaseObject
	{
		public: enum class eEventType { NoteON = 0, NoteOFF, MidiEnd };
		public:
			NoteEventData(PianoKey& key) : vPianoKey(key) { setTypeName("VirtualPiano::NoteEventData"); validate(); }
			PianoKey& vPianoKey;
			MidiParser::NoteEvent note;
			eEventType eventType;
	};
	public: struct FallingNoteGraphicsData
	{
		ostd::Rectangle rect;
		ostd::Color fillColor;
		ostd::Color outlineColor;
		ostd::Color glowColor;
		sf::Texture* texture;
		int32_t outlineThickness;
		float cornerRadius;
	};
	public: class SignalListener : public ostd::BaseObject
	{
		public:
			inline SignalListener(VirtualPiano& _parent) : parent(_parent)
			{
				setTypeName("VirtualPiano::SignalListener");
				connectSignal(Common::SigIntSignal);
				validate();
			}
			void handleSignal(ostd::tSignal& signal) override;

		public:
			VirtualPiano& parent;
	};

	public:
		inline VirtualPiano(Window& parentWindow) : m_sigListener(*this), m_parentWindow(parentWindow) {  }
		void init(void);

		void play(void);
		void pause(void);
		void stop(void);

		bool loadMidiFile(const ostd::String& filePath);
		bool loadAudioFile(const ostd::String& filePath);
		double getPlayTime_s(void);
		void update(void);
		void render(std::optional<std::reference_wrapper<sf::RenderTarget>> target = std::nullopt);
		void onWindowResized(uint32_t width, uint32_t height);
		void renderVirtualKeyboard(std::optional<std::reference_wrapper<sf::RenderTarget>> target = std::nullopt);
		void calculateFallingNotes(double currentTime);
		void drawFallingNote(const FallingNoteGraphicsData& noteData);
		void drawFallingNoteOutline(const FallingNoteGraphicsData& noteData);
		void drawFallingNoteGlow(const FallingNoteGraphicsData& noteData);
		void updateVisualization(double currentTime);

		bool renderFramesToFile(const ostd::String& folderPath, const ostd::UI16Point& resolution, uint8_t fps);
		void saveFrame(const sf::RenderTexture& rt, const ostd::String& basePath, int frameIndex);

		float scanMusicStartPoint(const ostd::String& filePath, float thresholdPercent = 0.02f, float minDuration = 0.05f);

		inline VirtualPianoData& vPianoData(void) { return m_vPianoData; }
		inline sf::Music& getAudioFile(void) { return m_audioFile; }
		inline float getAutoSoundStart(void) { return m_autoSoundStart; }
		inline bool hasAudioFile(void) { return m_hasAudioFile; }
		inline bool isPlaying(void) { return m_playing; }
		inline bool isRenderingToFile(void) { return m_isRenderingToFile; }

		static sf::VertexArray getMusicWaveForm(const ostd::String& filePath, int32_t windowHeight);

	private:
		void __preallocate_file_names_for_rendering(uint32_t frameCount, const ostd::String& baseFileName, const ostd::String& basePath, const ostd::String& extension, const uint16_t marginFrames = 200);
		void __build_ffmpeg_command(const ostd::UI16Point& resolution, uint16_t fps);

	private:
		Window& m_parentWindow;
		VirtualPianoData m_vPianoData;
		sf::Music m_audioFile;

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
