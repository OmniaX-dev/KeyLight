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
		inline static constexpr int32_t base_width = 2080;
		inline static constexpr int32_t base_height = 1400;

		float virtualPiano_x { 0.0f };
		float virtualPiano_y { 0.0f };

		float whiteKeyWidth { 0.0f };
		float whiteKeyHeight { 0.0f };
		float blackKeyWidth { 0.0f };
		float blackKeyHeight { 0.0f };
		float blackKeyOffset { 0.0f };
		ostd::Color whiteKeyColor = { 0, 0, 0 };
		ostd::Color whiteKeyPressedColor = { 0, 0, 0 };
		ostd::Color blackKeyColor = { 0, 0, 0 };
		ostd::Color blackKeyPressedColor = { 0, 0, 0 };

		std::unordered_map<int32_t, float> keyOffsets;

		static VirtualPianoData defaults(void);
		void scaleFromBaseWidth(int32_t new_width, int32_t new_height);
		void scaleByWindowSize(int32_t old_width, int32_t old_height, int32_t new_width, int32_t new_height);
		void recalculateKeyOffsets(void);
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
		// std::deque<MidiParser::NoteEvent> m_activeNotes;
		// int32_t m_nextNoteIndex { 0 };

		double m_fallingTime_s { 4.5 };
		std::deque<MidiParser::NoteEvent> m_activeFallingNotes;
		int32_t m_nextFallingNoteIndex { 0 };

		sf::RectangleShape m_sf_rect;
};