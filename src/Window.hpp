#pragma once

// #include <ogfx/WindowBase.hpp>
#include "SFMLWindow.hpp"
// #include <ogfx/BasicRenderer.hpp>
#include <ostd/Signals.hpp>
#include <ostd/Utils.hpp>
#include <deque>
#include "MidiParser.hpp"

class Window : public WindowBase
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
		
		public:
			inline static constexpr int32_t base_width = 2080;
			inline static constexpr int32_t base_height = 1400;
		
			ostd::Color whiteKeyColor = { 0, 0, 0 };
			ostd::Color whiteKeyPressedColor = { 0, 0, 0 };
			ostd::Color blackKeyColor = { 0, 0, 0 };
			ostd::Color blackKeyPressedColor = { 0, 0, 0 };

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

	public:
		inline Window(void) {  }
		void onInitialize(void) override;
		void handleSignal(ostd::tSignal& signal) override;
		void onRender(void) override;
		void onFixedUpdate(void) override;
		void onUpdate(void) override;
		void enableFullscreen(bool enable = true);

		void outlinedRect(const ostd::Rectangle& rect, const ostd::Color& fillColor, const ostd::Color& outlineColor, int32_t outlineThickness = -1);

		void play(void);
		double getCurrentTIme_ns(void);
		double getPlayTime_s(void);

		void drawVirtualPiano(const VirtualPianoData& vpd);
		bool loadMidiFile(const ostd::String& filePath);
		
	private:
		inline static const uint64_t NoteOnSignal = ostd::SignalHandler::newCustomSignal(5000);
		inline static const uint64_t NoteOffSignal = ostd::SignalHandler::newCustomSignal(5001);

		// ogfx::BasicRenderer2D m_gfx;
		std::vector<PianoKey> m_pianoKeys;
		VirtualPianoData m_vPianoData;
		bool m_playing;
		double m_startTimeOffset_ns { 0.0 };
		std::vector<MidiParser::NoteEvent> m_midiNotes;
		ostd::Vec2 m_windowSizeBeforeFullscreen { 0.0f, 0.0f };
		bool m_isFullscreen { false };

		double m_fallingTime_s { 4.5 };
		std::deque<MidiParser::NoteEvent> m_activeFallingNotes;
		int32_t m_nextFallingNoteIndex { 0 };

		sf::RectangleShape m_sf_rect;
};