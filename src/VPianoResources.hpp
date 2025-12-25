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

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <ostd/Geometry.hpp>
#include <ostd/String.hpp>
#include <any>
#include "Particles.hpp"
#include "MidiParser.hpp"

class VirtualPiano;
class VPianoResources
{
	public:
		VPianoResources(VirtualPiano& vpiano);
		bool loadShaders(void);
		bool loadBackgroundImage(const ostd::String& filePath);
		bool loadParticleTexture(const ostd::String& filePath, const std::vector<ostd::Rectangle>& tiles);
		bool loadNoteTexture(const ostd::String& filePath);
		bool loadAudioFile(const ostd::String& filePath);
		bool loadMidiFile(const ostd::String& filePath);

		float scanMusicStartPoint(const ostd::String& filePath, float thresholdPercent = 0.02f, float minDuration = 0.05f);

		inline sf::Music& getAudioFile(void) { return audioFile; }
		inline float getAutoSoundStart(void) { return autoSoundStart; }
		inline bool hasAudioFile(void) { return _hasAudioFile; }

	public:
		VirtualPiano& vpiano;
		sf::Music audioFile;
		ostd::String audioFilePath { "" };
		bool _hasAudioFile { false };
		float autoSoundStart { 0.0f };

		sf::Texture backgroundTex;
		std::optional<sf::Sprite> backgroundSpr;
		ostd::Vec2 backgroundOriginalSize { 0, 0 };

		std::any partTex;
		TextureRef partTexRef;
		std::vector<TextureRef::TextureAtlasIndex> partTiles;

		std::vector<MidiParser::NoteEvent> midiNotes;
		double firstNoteStartTime { 0.0 };
		double lastNoteEndTime { 0.0 };

		sf::Shader noteShader;
		sf::Shader blurShader;
		sf::Shader flipShader;
		sf::Shader particleShader;
		sf::Texture noteTexture;

};
