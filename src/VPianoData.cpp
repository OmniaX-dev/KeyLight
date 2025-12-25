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

#include "VPianoData.hpp"
#include <ostd/Logger.hpp>


// VirtualPianoData
VirtualPianoData::VirtualPianoData(void)
{
	fallingTime_s = 4.5;
	whiteKeyWidth = 40;
	whiteKeyHeight = whiteKeyWidth * 8;
	blackKeyWidth = 22;
	blackKeyHeight = blackKeyWidth * 9;
	blackKeyOffset = 4;
	whiteKeyColor = { 245, 245, 245 };
	whiteKeyPressedColor = { 120, 120, 210 };
	whiteKeySplitColor = { 0, 0, 0 };
	blackKeyColor = { 0, 0, 0 };
	blackKeyPressedColor = { 20, 20, 90 };
	blackKeySplitColor = { 0, 0, 0 };
	virtualPiano_x =  0.0f;
	pixelsPerSecond  = 250;
	virtualPiano_y = VirtualPianoData::base_height - whiteKeyHeight;
	scale_x = 1.0f;
	scale_y = 1.0f;
	glowMargins = { 4, 4, 4, 4 };
	recalculateKeyOffsets();

	fallingWhiteNoteColor = { 20, 110, 170 };
	fallingWhiteNoteOutlineColor = { 50, 140, 200 };
	fallingWhiteNoteGlowColor = { 50, 140, 200 };
	fallingBlackNoteColor = { 0, 50, 75 };
	fallingBlackNoteOutlineColor = { 30, 80, 105 };
	fallingBlackNoteGlowColor = { 30, 80, 105 };

	pianoLineColor1 = { 60, 10, 10 };
	pianoLineColor2 = { 160, 10, 10 };

	usePerNoteColors = false;

	for (int32_t i = 0; i < 36; i++)
		perNoteColors[i] = { 0, 0, 0 };

	whiteKeyShrinkFactor = 8;
	blackKeyShrinkFactor = 0;

	fallingWhiteNoteOutlineWidth = 2;
	fallingWhiteNoteBorderRadius = 5;

	fallingBlackNoteOutlineWidth = 2;
	fallingBlackNoteBorderRadius = 5;

	texCoordsPos = { 0, 0 };
	texCoordsScale = { 1, 1 };

	backgroundColor = { 20, 20, 20 };

	pressedVelocityMultiplier = 8.0f;
}

void VirtualPianoData::recalculateKeyOffsets(void)
{
	_keyOffsets.clear();
	int whiteKeyCount = 0;
	for (int midiNote = 21; midiNote <= 108; ++midiNote)
	{
		int noteInOctave = midiNote % 12;
		int keyIndex = midiNote - 21;
		if (MidiParser::NoteInfo::isWhiteKey(noteInOctave))
		{
			float x = vpx() + (whiteKeyCount * whiteKey_w());
			_keyOffsets[keyIndex] = x;
			whiteKeyCount++;
		}
		else // black keys
		{
			float x = vpx() + ((whiteKeyCount - 1) * whiteKey_w() + (whiteKey_w() - blackKey_w() / 2.0f)) - blackKey_offset();
			_keyOffsets[keyIndex] = x;
		}
	}
}

void VirtualPianoData::updateScale(int32_t width, int32_t height)
{
	scale_x = (float)width / (float)base_width;
	scale_y = (float)height / (float)base_height;
	Common::guiScaleY = scale_y;
}




// SignalListener
void SignalListener::handleSignal(ostd::tSignal& signal)
{
	if (signal.ID == Common::SigIntSignal)
	{
		if (Common::IsDebug)
		{
			OX_DEBUG("Capturing SIGINT.");
		}
		Common::s_sigint_triggered = true;
	}
}
