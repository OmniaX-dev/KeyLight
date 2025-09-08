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
			vpd.virtualPiano_x =  0.0f;
			vpd.virtualPiano_y = VirtualPianoData::base_height - vpd.whiteKeyHeight;
			vpd.recalculateKeyOffsets();
			return vpd;
		}

		void scaleFromBaseWidth(int32_t new_width, int32_t new_height)
		{
			scaleByWindowSize(base_width, base_height, new_width, new_height);
		}

		void scaleByWindowSize(int32_t old_width, int32_t old_height, int32_t new_width, int32_t new_height)
		{
			float width_ratio = (float)new_width / (float)old_width;
			float height_ratio = (float)new_height / (float)old_height;

			whiteKeyWidth *= width_ratio;
			whiteKeyHeight *= height_ratio;
			blackKeyWidth *= width_ratio;
			blackKeyHeight *= height_ratio;
			blackKeyOffset *= width_ratio;

			virtualPiano_x *= width_ratio;
			virtualPiano_y *= height_ratio;

			recalculateKeyOffsets();
		}

		void recalculateKeyOffsets(void)
		{
			keyOffsets.clear();
			int whiteKeyCount = 0;
			for (int midiNote = 21; midiNote <= 108; ++midiNote)
			{
				int noteInOctave = midiNote % 12;
				int keyIndex = midiNote - 21;
				if (MidiParser::NoteInfo::isWhiteKey(noteInOctave))
				{
					float x = virtualPiano_x + (whiteKeyCount * whiteKeyWidth);
					keyOffsets[keyIndex] = x;
					whiteKeyCount++;
				}
				else // black keys
				{
					float x = virtualPiano_x + ((whiteKeyCount - 1) * whiteKeyWidth + (whiteKeyWidth - blackKeyWidth / 2.0f)) - blackKeyOffset;
					keyOffsets[keyIndex] = x;
				}
			}
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

		double m_fallingTime_s { 1.5 };
		std::deque<MidiParser::NoteEvent> m_activeFallingNotes;
		int32_t m_nextFallingNoteIndex { 0 };
};