#pragma once

#include <ostd/String.hpp>

class MidiParser
{
    public: struct NoteEvent
    {
        int pitch;          // MIDI note number (0-127)
        double startTime;   // Start time in seconds
        double endTime;   // Start time in seconds
        double duration;    // Duration in seconds
        int velocity;       // Attack velocity (1-127)
        int channel;        // MIDI channel (0-15)

        bool operator<(const NoteEvent& other) const
        {
            return startTime < other.startTime;
        }
    };
	public: struct NoteInfo
	{
		ostd::String name;  // e.g., "A", "C#", "F"
		int octave;        // e.g., 4 for C4
		int keyIndex;      // 0-based index for 88-key piano (A0=0), -1 if out of range
	};

    public:
		// Parses a single-track MIDI file and returns a vector of NoteEvents
        static std::vector<NoteEvent> parseFile(const ostd::String& filePath);
		// Convert MIDI pitch to NoteInfo
		static NoteInfo getNoteInfo(int midiPitch);
};