#pragma once

#include <ogfx/WindowBase.hpp>
#include <ogfx/BasicRenderer.hpp>
#include <ostd/Signals.hpp>
#include <ostd/Utils.hpp>
#include <deque>
#include "MidiParser.hpp"

class Window : public ogfx::WindowBase
{
	public: struct PianoKey
	{
		MidiParser::NoteInfo noteInfo;
		bool pressed { false };
	};
	public: struct VirtualPianoData
	{
		float whiteKeyWidth { 0.0f };
		float whiteKeyHeight { 0.0f };
		float blackKeyWidth { 0.0f };
		float blackKeyHeight { 0.0f };
		float blackKeyOffset { 0.0f };
		ostd::Color whiteKeyColor = { 0, 0, 0 };
		ostd::Color whiteKeyPressedColor = { 0, 0, 0 };
		ostd::Color blackKeyColor = { 0, 0, 0 };
		ostd::Color blackKeyPressedColor = { 0, 0, 0 };

		static VirtualPianoData defaults(void)
		{
			VirtualPianoData vpd;
			vpd.whiteKeyWidth = 40;
			vpd.whiteKeyHeight = vpd.whiteKeyWidth * 6.5;
			vpd.blackKeyWidth = 22;
			vpd.blackKeyHeight = vpd.blackKeyWidth * 7.5;
			vpd.blackKeyOffset = 4;
			vpd.whiteKeyColor = { 245, 245, 245 };
			vpd.whiteKeyPressedColor = { 120, 120, 210 };
			vpd.blackKeyColor = { 0, 0, 0 };
			vpd.blackKeyPressedColor = { 20, 20, 90 };
			return vpd;
		}
	};

	public:
		inline Window(void) {  }
		void onInitialize(void) override;
		void handleSignal(ostd::tSignal& signal) override;
		void onRender(void) override;
		void onFixedUpdate(void) override;
		void onUpdate(void) override;

		void play(void);
		double getCurrentTIme_ns(void);
		double getPlayTime_s(void);

		void drawVirtualPiano(const VirtualPianoData& vpd);
		bool loadMidiFile(const ostd::String& filePath);
		
	private:
		ogfx::BasicRenderer2D m_gfx;
		std::vector<PianoKey> m_pianoKeys;
		VirtualPianoData m_vPianoData;
		bool m_playing;
		double m_startTimeOffset_ns { 0.0 };
		std::vector<MidiParser::NoteEvent> m_midiNotes;
		std::deque<MidiParser::NoteEvent> m_activeNotes;
		int32_t m_nextNoteIndex { 0 };
};