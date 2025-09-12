#pragma once

#include <ostd/String.hpp>
#include <ostd/Defines.hpp>

class MidiParser
{
    public: class NoteEvent
    {
        public:
            int pitch;          // MIDI note number (0-127)
            double startTime;   // Start time in seconds
            double endTime;   // Start time in seconds
            double duration;    // Duration in seconds
            int velocity;       // Attack velocity (1-127)
            int channel;        // MIDI channel (0-15)

            bool hit;

        public:
            bool operator<(const NoteEvent& other) const
            {
                return startTime < other.startTime;
            }

            ostd::String toString(void) const
            {
                ostd::String str = "Pitch: ";
                str.add(pitch);
                str.add("\nstartTime: ").add(startTime);
                str.add("\nendTime: ").add(endTime);
                str.add("\nduration: ").add(duration);
                str.add("\nvelocity: ").add(velocity);
                str.add("\nchannel: ").add(channel);
                str.add("\nhit: ").add(STR_BOOL(hit));
                return str;
            }
    };
	public: class NoteInfo
	{
        public:
            ostd::String name;  // e.g., "A", "C#", "F"
            int octave;        // e.g., 4 for C4
            int noteInOctave;    // 0-11
            int keyIndex;      // 0-based index for 88-key piano (A0=0), -1 if out of range

        public:
            bool isWhiteKey(void) const
            {
                return noteInOctave == 0 || noteInOctave == 2 || noteInOctave == 4 ||
                noteInOctave == 5 || noteInOctave == 7 || noteInOctave == 9 ||
                noteInOctave == 11;
            }

            bool isBlackKey(void) const { return !isWhiteKey(); }

            static bool isWhiteKey(int32_t noteInOctave)
            {
                return noteInOctave == 0 || noteInOctave == 2 || noteInOctave == 4 ||
                noteInOctave == 5 || noteInOctave == 7 || noteInOctave == 9 ||
                noteInOctave == 11;
            }

            ostd::String toString(void) const
            {
                ostd::String str = "NOTE INFO: ";
                str.add(name).add(octave);
                str.add("  -  noteInOctave: ").add(noteInOctave);
                str.add("  -  keyIndex: ").add(keyIndex);
                str.add("\n");
                return str;
            }

            static bool isBlackKey(int32_t noteInOctave) { return !NoteInfo::isWhiteKey(noteInOctave); }
	};

    public:
		// Parses a single-track MIDI file and returns a vector of NoteEvents
        static std::vector<NoteEvent> parseFile(const ostd::String& filePath);
		// Convert MIDI pitch to NoteInfo
		static NoteInfo getNoteInfo(int midiPitch);
};