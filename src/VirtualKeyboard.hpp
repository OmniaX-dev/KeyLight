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

#include "VPianoData.hpp"
#include <deque>
#include "MidiParser.hpp"
#include <vector>

class VirtualKeyboard
{
	public:
		VirtualKeyboard(VirtualPiano& vpiano);
		void init(void);
		void loadFromStyleJSON(JSONManager& styleJson);
		void calculateFallingNotes(double currentTime);
		void updateVisualization(double currentTime);
		void render(std::optional<std::reference_wrapper<sf::RenderTarget>> target = std::nullopt);
		void drawFallingNote(const FallingNoteGraphicsData& noteData);
		void drawFallingNoteOutline(const FallingNoteGraphicsData& noteData);
		void drawFallingNoteGlow(const FallingNoteGraphicsData& noteData);

	private:
		VirtualPiano& m_vpiano;

		std::vector<PianoKey> m_pianoKeys;
		std::deque<MidiParser::NoteEvent> m_activeFallingNotes;
		std::vector<FallingNoteGraphicsData> m_fallingNoteGfx_w;
		std::vector<FallingNoteGraphicsData> m_fallingNoteGfx_b;
		int32_t m_nextFallingNoteIndex { 0 };

		friend class VirtualPiano;
};
