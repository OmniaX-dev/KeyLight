#pragma once

#include <ostd/Signals.hpp>
#include <ostd/Utils.hpp>
#include <deque>
#include "MidiParser.hpp"
#include <ostd/Geometry.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include "RoundedRectangleShape.hpp"

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
		public: enum class eEventType { NoteON = 0, NoteOFF };
		public:
			NoteEventData(PianoKey& key) : vPianoKey(key) { setTypeName("NoteEventData"); validate(); }
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

	public:
		inline VirtualPiano(Window& parentWindow) : m_parentWindow(parentWindow) {  }
		void init(void);

		void play(void);
		void pause(void);
		void stop(void);
		
		bool loadMidiFile(const ostd::String& filePath);
		bool loadAudioFile(const ostd::String& filePath);
		double getPlayTime_s(void);
		void update(void);
		void render(void);
		void onWindowResized(uint32_t width, uint32_t height);
		void renderVirtualKeyboard(void);
		void calculateFallingNotes(void);
		void drawFallingNote(const FallingNoteGraphicsData& noteData);
		void drawFallingNoteOutline(const FallingNoteGraphicsData& noteData);
		void drawFallingNoteGlow(const FallingNoteGraphicsData& noteData);

		float scanMusicStartPoint(const ostd::String& filePath, float thresholdPercent = 0.02f, float minDuration = 0.05f);

		inline VirtualPianoData& vPianoData(void) { return m_vPianoData; }
		inline sf::Music& getAudioFile(void) { return m_audioFile; }
		inline float getAutoSoundStart(void) { return m_autoSoundStart; }
		inline bool hasAudioFile(void) { return m_hasAudioFile; }
		inline bool isPlaying(void) { return m_playing; }

		static sf::VertexArray getMusicWaveForm(const ostd::String& filePath, int32_t windowHeight);

	private:
		Window& m_parentWindow;
		VirtualPianoData m_vPianoData;
		sf::Music m_audioFile;

		std::vector<PianoKey> m_pianoKeys;
		std::vector<MidiParser::NoteEvent> m_midiNotes;
		std::deque<MidiParser::NoteEvent> m_activeFallingNotes;
		std::vector<FallingNoteGraphicsData> m_fallingNoteGfx_w;
		std::vector<FallingNoteGraphicsData> m_fallingNoteGfx_b;

		bool m_playing;
		bool m_paused;
		bool m_firstNotePlayed;
		bool m_hasAudioFile;

		int32_t m_nextFallingNoteIndex { 0 };
		float m_autoSoundStart { 0.0f };

		double m_fallingTime_s { 4.5 };
		double m_startTimeOffset_ns { 0.0 };

		sf::RenderTexture m_glowBuffer;
		sf::RenderTexture m_blurBuff1;
		sf::RenderTexture m_blurBuff2;
		sf::View m_glowView;
	
	public:
		sf::Shader noteShader;
		sf::Shader blurShader;
		sf::Texture noteTexture;

	public:
		inline static const uint64_t NoteOnSignal = ostd::SignalHandler::newCustomSignal(5000);
		inline static const uint64_t NoteOffSignal = ostd::SignalHandler::newCustomSignal(5001);
		inline static const uint64_t MidiStartSignal = ostd::SignalHandler::newCustomSignal(5002);

};