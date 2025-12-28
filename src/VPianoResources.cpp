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

#include "VPianoResources.hpp"
#include <ostd/Logger.hpp>
#include "VirtualPiano.hpp"
#include "Window.hpp"


VPianoResources::VPianoResources(VirtualPiano& vpiano) : vpiano(vpiano)
{
}

void VPianoResources::loadStyleFromJson(JSONManager& style, JSONManager& particles)
{

}

bool VPianoResources::loadShaders(void)
{
	if (!noteShader.loadFromFile("shaders/basic.vert", "shaders/note.frag"))
	{
		OX_ERROR("Failed to load shader");
		return false;
	}
	if (!thresholdShader.loadFromFile("shaders/basic.vert", "shaders/threshold.frag"))
	{
		OX_ERROR("Failed to load shader");
		return false;
	}
	if (!kawaseBlurShader.loadFromFile("shaders/basic.vert", "shaders/kawase_blur.frag"))
	{
		OX_ERROR("Failed to load shader");
		return false;
	}
	if (!gaussianBlurShader.loadFromFile("shaders/basic.vert", "shaders/gaussian_blur.frag"))
	{
		OX_ERROR("Failed to load shader");
		return false;
	}
	if (!flipShader.loadFromFile("shaders/basic.vert", "shaders/flip.frag"))
	{
		OX_ERROR("Failed to load shader");
		return false;
	}
	if (!particleShader.loadFromFile("shaders/basic.vert", "shaders/particle.frag"))
	{
		OX_ERROR("Failed to load shader");
		return false;
	}
	return true;
}

bool VPianoResources::loadBackgroundImage(const ostd::String& filePath)
{
	sf::Image background;
	if (!background.loadFromFile(filePath))
	{
		OX_WARN("Unable to load background image.");
		return false;
	}
	(void)backgroundTex.loadFromImage(background);
	backgroundOriginalSize = { (float)backgroundTex.getSize().x, (float)backgroundTex.getSize().y };
	backgroundSpr = sf::Sprite(backgroundTex);
	backgroundSpr->setPosition({ 0, 0 });
	ostd::Vec2 scale = { backgroundOriginalSize.x / (float)vpiano.m_parentWindow.getWindowWidth(), backgroundOriginalSize.y / (float)vpiano.m_parentWindow.getWindowHeight() };
	scale = { 1.0f / scale.x, 1.0f / scale.y };
	backgroundSpr->setScale({ scale.x, scale.y });
	OX_DEBUG("Loaded background image: <%s>", filePath.c_str());
	return true;
}

bool VPianoResources::loadParticleTexture(const ostd::String& filePath, const std::vector<ostd::Rectangle>& tiles)
{
	partTex = sf::Texture(filePath);
	sf::Texture& tex = std::any_cast<sf::Texture&>(partTex);
	if (!partTex.has_value())
	{
		OX_ERROR("Unable To load texture: %s", filePath.c_str());
		return false;
	}

	partTexRef.attachTexture(partTex, tex.getSize().x, tex.getSize().y);
	partTiles.clear();
	if (tiles.size() > 0)
	{
		for (auto& rect : tiles)
			partTiles.push_back(partTexRef.addTileInfo((uint32_t)rect.x, (uint32_t)rect.y, (uint32_t)rect.w, (uint32_t)rect.h));
	}

	return true;
}

bool VPianoResources::loadNoteTexture(const ostd::String& filePath)
{
	if (!noteTexture.loadFromFile(filePath))
	{
		OX_ERROR("Failed to load texture: %s", filePath.c_str());
		return false;
	}
	noteTexture.setRepeated(true);
	return true;
}

bool VPianoResources::loadAudioFile(const ostd::String& filePath)
{
	_hasAudioFile = false;
	if (!audioFile.openFromFile(filePath.cpp_str()))
	{
		OX_ERROR("Failed to open audio file: %s", filePath.c_str());
		return false;
	}
	OX_DEBUG("loaded <%s>", filePath.c_str());
	_hasAudioFile = true;
	autoSoundStart = scanMusicStartPoint(filePath, 0.005f);
	audioFilePath = filePath;
	OX_DEBUG("  Auto sound start: %f.", autoSoundStart);
	OX_DEBUG("  Delta time: %f.", (firstNoteStartTime - autoSoundStart));
	return true;
}

bool VPianoResources::loadMidiFile(const ostd::String& filePath)
{
	try
	{
		midiNotes.clear();
		midiNotes = MidiParser::parseFile(filePath);
		for (auto& note : midiNotes)
		{
			note.startTime += vpiano.m_vPianoData.fallingTime_s;
			note.endTime   += vpiano.m_vPianoData.fallingTime_s;

			if (note.first)
				firstNoteStartTime = note.startTime;
			else if (note.last)
				lastNoteEndTime = note.startTime;
		}
		std::sort(midiNotes.begin(), midiNotes.end());
		OX_DEBUG("loaded <%s>: total notes parsed: %f", filePath.c_str(), midiNotes.size());
		OX_DEBUG("  First note start time: %f seconds.", firstNoteStartTime);
		OX_DEBUG("  Last note end time: %f seconds.", lastNoteEndTime);
		return true;
    }
    catch (const std::exception& ex)
    {
		OX_ERROR(ex.what());
        return false;
    }
}

float VPianoResources::scanMusicStartPoint(const ostd::String& filePath, float thresholdPercent, float minDuration)
{
	sf::SoundBuffer buffer;
	if (!buffer.loadFromFile(filePath))
	{
		OX_ERROR("Invalid audio file: %s", filePath.c_str());
		return 0.0f;
	}
	const int16_t* samples = buffer.getSamples();
    std::size_t sampleCount  = buffer.getSampleCount();
    unsigned int channels    = buffer.getChannelCount();
    unsigned int sampleRate  = buffer.getSampleRate();

    // Convert percentage to raw PCM units
    const float fullScale = 32767.f;
    const float threshold = thresholdPercent * fullScale;
	const std::size_t hop = 256;

    // Number of samples in the sustain window (per channel)
    std::size_t windowSamples = static_cast<std::size_t>(minDuration * sampleRate);

    for (std::size_t i = 0; i + windowSamples * channels < sampleCount; i += hop * channels)
    {
        // Compute RMS over the window
        double sumSquares = 0.0;
        for (std::size_t j = 0; j < windowSamples; ++j)
        {
            for (unsigned int c = 0; c < channels; ++c)
            {
                float s = static_cast<float>(samples[(i + j * channels) + c]);
                sumSquares += s * s;
            }
        }

        double meanSquare = sumSquares / (windowSamples * channels);
        double rms = std::sqrt(meanSquare);

        if (rms > threshold)
        {
            // Found the first sustained sound
            return static_cast<float>((float)i / channels) / sampleRate;
        }
    }

    return 0.f; // No sound found above threshold
}
