#include "Midi.hpp"

#include <midifile/MidiFile.h>
#include <midifile/Options.h>

namespace ostd
{
	void MidiFile::test(const ostd::String& filePath)
	{
		smf::MidiFile midifile;
		midifile.read(filePath);
		midifile.doTimeAnalysis();
		midifile.linkNotePairs();

		int tracks = midifile.getTrackCount();
		std::cout << "TPQ: " << midifile.getTicksPerQuarterNote() << std::endl;
		if (tracks > 1) std::cout << "TRACKS: " << tracks << std::endl;
		for (int track=0; track<tracks; track++)
		{
			if (tracks > 1)
				std::cout << "\nTrack " << track << std::endl;
			std::cout << "Tick\tSeconds\tDur\tMessage" << std::endl;
			for (int event=0; event<midifile[track].size(); event++)
			{
				std::cout << std::dec << midifile[track][event].tick;
				std::cout << '\t' << std::dec << midifile[track][event].seconds;
				std::cout << '\t';
				if (midifile[track][event].isNoteOn())
					std::cout << midifile[track][event].getDurationInSeconds();
				std::cout << '\t' << std::hex;
				for (int i=0; i<midifile[track][event].size(); i++)
					std::cout << (int)midifile[track][event][i] << ' ';
				std::cout << std::endl;
			}
		}









		// midi.doTimeAnalysis();       // Compute seconds for each event
		// midi.linkNotePairs();        // Link Note On/Off for durations

		// int tracks = midi.getTrackCount();
		// for (int t = 0; t < tracks; t++)
		// {
		// 	for (int e = 0; e < midi[t].size(); e++)
		// 	{
		// 		auto& ev = midi[t][e];
		// 		if (ev.isNoteOn())
		// 		{
		// 			int pitch = ev.getKeyNumber(); // MIDI note number
		// 			double start = ev.seconds;    // Start time in seconds
		// 			double duration = ev.getDurationInSeconds();
		// 			int velocity = ev.getVelocity();

		// 			std::cout << "Note " << pitch
		// 					  << " start: " << start
		// 					  << " dur: " << duration
		// 					  << " vel: " << velocity << "\n";
		// 		}
		// 	}
		// }

	}
}