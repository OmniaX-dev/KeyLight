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

#include "VirtualPiano.hpp"
#include "Common.hpp"
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <optional>
#include <ostd/Color.hpp>
#include <ostd/Geometry.hpp>
#include <ostd/Logger.hpp>
#include <ostd/Signals.hpp>
#include <ostd/String.hpp>
#include <ostd/Utils.hpp>
#include "MidiParser.hpp"
#include "Particles.hpp"
#include "VPianoDataStructures.hpp"
#include "Window.hpp"
#include "Renderer.hpp"
#include "ffmpeg_helper.hpp"
#include <ostd/Random.hpp>


// VirtualPianoData
VirtualPianoData::VirtualPianoData(void)
{
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




// Core functionality
void VirtualPiano::init(void)
{
	m_playing = false;
	m_paused = false;
	m_firstNotePlayed = false;
	m_config.init("settings.json", &Common::DefaultSettingsJSON);
	std::cout << m_config.get_string("settings.ffmpegPath") << "\n";

	__load_resources();

	for (int midiNote = 21; midiNote <= 108; ++midiNote)
	{
		PianoKey pk;
		pk.noteInfo = MidiParser::getNoteInfo(midiNote);
		pk.pressed = false;

		pk.particles = ParticleFactory::basicFireEmitter({ &m_partTexRef, m_partTiles[0] }, { 0, 0 }, 0);
		pk.particles.useTileArray(true);
		// pk.particles.getDefaultParticleInfo().speed = 140;
		pk.particles.setMaxParticleCount(2500);
		// pk.particles.getDefaultParticleInfo().lifeSpan = 1200;
		pk.particles.addTilesToArray(m_partTiles);

		auto& partInfo = pk.particles.getDefaultParticleInfo();
		ParticleFactory::createColorGradient(partInfo, ostd::Color("#FF3D6AFF"), 10);
		// partInfo.colorRamp.reset();
		// partInfo.colorRamp.m_colors.clear();
		// partInfo.addColorToGradient(ostd::Color("#001A0CFF"),   ostd::Color("#004721FF"),  0.15f);
		// partInfo.addColorToGradient(ostd::Color("#004721FF"),   ostd::Color("#007537FF"),  0.15f);
		// partInfo.addColorToGradient(ostd::Color("#007537FF"),   ostd::Color("#00A34CFF"),  0.15f);
		// partInfo.addColorToGradient(ostd::Color("#00A34CFF"),   ostd::Color("#00D162FF"),  0.15f);
		// partInfo.addColorToGradient(ostd::Color("#00D162FF"),   ostd::Color("#00FF77FF"),  0.15f);
		// partInfo.addColorToGradient(ostd::Color("#00FF77FF"),   ostd::Color("#2EFF8FFF"),  0.15f);
		// partInfo.addColorToGradient(ostd::Color("#2EFF8FFF"),   ostd::Color("#5CFFA8FF"),  0.1f);
		partInfo.size = { 12.0f, 12.0f };
		partInfo.speed = 2.6f;
		partInfo.randomVelocity = { 0.25f, 0.7f };
		partInfo.randomDirection = 0.15f;
		partInfo.randomDamping = true;
		partInfo.damping = { 0.0f, 0.2f };
		partInfo.fadeIn = false;
		partInfo.lifeSpan = 250;


		m_pianoKeys.push_back(pk);
	}

	sf::Vector2u winSize = { m_parentWindow.sfWindow().getSize().x, m_parentWindow.sfWindow().getSize().y };
	sf::Vector2u winHalfSize = { winSize.x / 2, winSize.y / 2 };
	m_blurBuff1 = sf::RenderTexture(winHalfSize);
	m_blurBuff2 = sf::RenderTexture(winHalfSize);
	m_glowBuffer = sf::RenderTexture(winHalfSize);

	m_glowView.setSize({ (float)winSize.x, (float)winSize.y });
	m_glowView.setCenter({ winSize.x / 2.f, winSize.y / 2.f });
	m_glowBuffer.setView(m_glowView);

	// m_vPianoData.whiteKeyColor = { 0, 0, 0 };
	// m_vPianoData.whiteKeyPressedColor = { 80, 80, 80 };
	// m_vPianoData.whiteKeySplitColor = { 20, 20, 20 };
	// m_vPianoData.blackKeyColor = { 20, 20, 20 };
	// m_vPianoData.blackKeyPressedColor = { 80, 80, 80 };
	// m_vPianoData.blackKeySplitColor = { 40, 40, 40 };
	// m_vPianoData.pianoLineColor1 = { 20, 2, 2 };
	// m_vPianoData.pianoLineColor2 = { 40, 2, 2 };
	// m_vPianoData.backgroundColor = { 5, 5, 20 };
	//

	m_vPianoData.fallingWhiteNoteColor = { "#FFB0F7FF" };
	m_vPianoData.fallingWhiteNoteOutlineColor = { "#FFB0F7FF" };
	m_vPianoData.fallingWhiteNoteGlowColor = { "#FFB0F7FF" };
	m_vPianoData.fallingBlackNoteColor = { "#63132EFF" };
	m_vPianoData.fallingBlackNoteOutlineColor = { "#63132EFF" };
	m_vPianoData.fallingBlackNoteGlowColor = { "#63132EFF" };
	m_vPianoData.backgroundColor = { "#0A0205FF" };
	m_vPianoData.whiteKeyPressedColor = { "#FF7ACEFF" };
	m_vPianoData.blackKeyPressedColor = { "#873568FF" };
	m_vPianoData.whiteKeyColor = { 200, 200, 200 };
	m_vPianoData.blackKeyColor = { 20, 20, 20 };

	m_showBackground = false;

	// m_vPianoData.usePerNoteColors = true;
	// for (int32_t i = 0; i < 12; i++)
	// {
	// 	if (i == (int)VirtualPianoData::eNoteColor::Asharp_Main ||
	// 		i == (int)VirtualPianoData::eNoteColor::Csharp_Main ||
	// 		i == (int)VirtualPianoData::eNoteColor::Dsharp_Main ||
	// 		i == (int)VirtualPianoData::eNoteColor::Fsharp_Main ||
	// 		i == (int)VirtualPianoData::eNoteColor::Gsharp_Main)
	// 	{
	// 		m_vPianoData.perNoteColors[i] = { 190, 140, 80 };
	// 		m_vPianoData.perNoteColors[i + 12] = { 190, 140, 80 };
	// 		m_vPianoData.perNoteColors[i + 24] = { 190, 140, 80 };
	// 	}
	// 	else
	// 	{
	// 		m_vPianoData.perNoteColors[i] = { 255, 230, 150 };
	// 		m_vPianoData.perNoteColors[i + 12] = { 255, 230, 150 };
	// 		m_vPianoData.perNoteColors[i + 24] = { 255, 230, 150 };
	// 	}
	// }
	m_snow = ParticleFactory::basicSnowEmitter({ &m_partTexRef, m_partTiles[0] }, { (float)winSize.x, (float)winSize.y }, 000);
	// m_snow.getDefaultParticleInfo().size = { 16, 16 };
}

void VirtualPiano::onWindowResized(uint32_t width, uint32_t height)
{
	m_blurBuff1 = sf::RenderTexture({ width / 2, height / 2 });
	m_blurBuff2 = sf::RenderTexture({ width / 2, height / 2 });
	m_glowBuffer = sf::RenderTexture({ width / 2, height / 2 });

	m_glowView.setSize({ (float)width, (float)height });
	m_glowView.setCenter({ width / 2.f, height / 2.f });
	m_glowBuffer.setView(m_glowView);

	if (m_showBackground)
	{
		ostd::Vec2 scale = { m_backgroundOriginalSize.x / (float)width, m_backgroundOriginalSize.y / (float)height };
		scale = { 1.0f / scale.x, 1.0f / scale.y };
		m_backgroundSpr->setScale({ scale.x, scale.y });
	}

	// m_snow.setEmissionRect(ostd::Rectangle(0, 0, (float)width, 1.0f));
	// m_snow.setWorkingRectangle({ 0, 0, (float)width, (float)height });
}

void VirtualPiano::onSignal(ostd::tSignal& signal)
{
	return;
}




// Playback functionality
void VirtualPiano::play(void)
{
	if (m_paused)
	{
		m_playing = true;
		m_paused = false;
		return;
	}
	stop();
	m_playing = true;
}

void VirtualPiano::pause(void)
{
	m_audioFile.pause();
	m_playing = false;
	m_paused = true;
}

void VirtualPiano::stop(void)
{
	m_audioFile.stop();
	m_paused = false;
	m_firstNotePlayed = false;
	m_startTimeOffset_ns = Common::getCurrentTIme_ns();
	m_nextFallingNoteIndex = 0;
	m_activeFallingNotes.clear();
	for (auto& pk : m_pianoKeys)
	{
		pk.pressed = false;
	}
	update();
	m_playing = false;
}

double VirtualPiano::getPlayTime_s(void)
{
	double playTime = Common::getCurrentTIme_ns() - m_startTimeOffset_ns;
	return playTime * 1e-9;
}




// Audio functionality
bool VirtualPiano::loadMidiFile(const ostd::String& filePath)
{
	try
	{
		m_midiNotes.clear();
		m_midiNotes = MidiParser::parseFile(filePath);
		for (auto& note : m_midiNotes)
		{
			note.startTime += m_fallingTime_s;
			note.endTime   += m_fallingTime_s;

			if (note.first)
				m_firstNoteStartTime = note.startTime;
			else if (note.last)
				m_lastNoteEndTime = note.startTime;
		}
		std::sort(m_midiNotes.begin(), m_midiNotes.end());
		OX_DEBUG("loaded <%s>: total notes parsed: %f", filePath.c_str(), m_midiNotes.size());
		OX_DEBUG("  First note start time: %f seconds.", m_firstNoteStartTime);
		OX_DEBUG("  Last note end time: %f seconds.", m_lastNoteEndTime);
		return true;
    }
    catch (const std::exception& ex) {
		OX_ERROR(ex.what());
        return false;
    }
}

bool VirtualPiano::loadAudioFile(const ostd::String& filePath)
{
	m_hasAudioFile = false;
	if (!m_audioFile.openFromFile(filePath.cpp_str()))
	{
		OX_ERROR("Failed to open audio file: %s", filePath.c_str());
		return false;
	}
	OX_DEBUG("loaded <%s>", filePath.c_str());
	m_hasAudioFile = true;
	m_autoSoundStart = scanMusicStartPoint(filePath, 0.005f);
	m_audioFilePath = filePath;
	OX_DEBUG("  Auto sound start: %f.", m_autoSoundStart);
	OX_DEBUG("  Delta time: %f.", (m_firstNoteStartTime - m_autoSoundStart));
	return true;
}

float VirtualPiano::scanMusicStartPoint(const ostd::String& filePath, float thresholdPercent, float minDuration)
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




// Update and Render
void VirtualPiano::update(void)
{
	if (m_playing)
	{
		updateVisualization(getPlayTime_s());
	}
	if (m_playing || m_isRenderingToFile)
	{
		for (auto& pk : m_pianoKeys)
		{
			pk.particles.update(pk.pressedForce);
			if (pk.pressed)
			{
				pk.particles.emit(70);//ostd::Random::geti32(5, 20));
			}
		}
	}
	if (m_windCounter++ > 120)
	{
		m_windCounter = 0;
		m_wind.x = ostd::Random::getf32(0.0f, 0.7f);
	}
	// m_snow.update(m_wind / 5.0f);
	// m_snow.emit(ostd::Random::geti32(0, 2));
}

void VirtualPiano::fastUpdate(void)
{
	// if (m_playing || m_isRenderingToFile)
	// {
	// 	for (auto& pk : m_pianoKeys)
	// 	{
	// 		pk.particles.update(m_wind + pk.pressedForce);
	// 	}
	// }
	// if (m_windCounter++ > 1000)
	// {
	// 	m_windCounter = 0;
	// 	m_wind.x = ostd::Random::getf32(0.0f, 0.7f);
	// }
	// m_snow.update(m_wind / 5.0f);
}

void VirtualPiano::calculateFallingNotes(double currentTime)
{
	auto l_calcPressedVelocity = [this](int32_t midiVelocity) -> ostd::Vec2 {
		return { 0.0f, -((float)(midiVelocity / 128.0f)) * (float)m_vPianoData.pressedVelocityMultiplier };
	};

	m_fallingNoteGfx_w.clear();
	m_fallingNoteGfx_b.clear();
	for (auto& note : m_activeFallingNotes)
	{
		auto noteInfo = MidiParser::getNoteInfo(note.pitch);
		if (noteInfo.isBlackKey()) continue;

		double h = note.duration * m_vPianoData.pps();
		double totalTravelTime = m_fallingTime_s + note.duration;
		double elapsedSinceSpawn = (currentTime - (note.startTime - m_fallingTime_s));
		double progress = elapsedSinceSpawn / totalTravelTime;
		progress = std::clamp(progress, 0.0, 1.0);
		double y = -h + static_cast<double>(progress) * (m_vPianoData.vpy() + h);
		double x = m_vPianoData.keyOffsets()[noteInfo.keyIndex] + (m_vPianoData.whiteKey_shrink() / 2.0f);

		if (y >= m_vPianoData.vpy())
		{
			auto& key = m_pianoKeys[noteInfo.keyIndex];
			key.pressed = false;
			key.pressedForce = { 0.0f, 0.0f };
			NoteEventData ned(key);
			ned.eventType = NoteEventData::eEventType::NoteOFF;
			ned.note = note;
			ostd::SignalHandler::emitSignal(NoteOffSignal, ostd::tSignalPriority::RealTime, ned);
		}
		else if (y + h >= m_vPianoData.vpy())
		{
			auto& key = m_pianoKeys[noteInfo.keyIndex];
			key.pressed = true;
			key.pressedForce = l_calcPressedVelocity(note.velocity);
			NoteEventData ned(key);
			ned.eventType = NoteEventData::eEventType::NoteON;
			ned.note = note;
			ostd::SignalHandler::emitSignal(NoteOnSignal, ostd::tSignalPriority::RealTime, ned);
			if (!m_firstNotePlayed)
			{
				ostd::SignalHandler::emitSignal(MidiStartSignal, ostd::tSignalPriority::RealTime, ned);
				m_firstNotePlayed = true;
			}
		}
		ostd::Color noteColor = m_vPianoData.fallingWhiteNoteColor;
		ostd::Color outlineColor = m_vPianoData.fallingWhiteNoteOutlineColor;
		ostd::Color glowColor = m_vPianoData.fallingWhiteNoteGlowColor;
		if (m_vPianoData.usePerNoteColors)
		{
			noteColor = m_vPianoData.perNoteColors[noteInfo.noteInOctave];
			outlineColor = m_vPianoData.perNoteColors[noteInfo.noteInOctave + 12];
			glowColor = m_vPianoData.perNoteColors[noteInfo.noteInOctave + 24];
		}
		m_fallingNoteGfx_w.push_back(FallingNoteGraphicsData {
			{ static_cast<float>(x), static_cast<float>(y), m_vPianoData.whiteKey_w() - m_vPianoData.whiteKey_shrink(), static_cast<float>(h) },
			noteColor,
			outlineColor,
			glowColor,
			&noteTexture,
			-m_vPianoData.fallingWhiteNoteOutlineWidth,
			m_vPianoData.fallingWhiteNoteBorderRadius
		});
	}

	for (auto& note : m_activeFallingNotes)
	{
		auto noteInfo = MidiParser::getNoteInfo(note.pitch);
		if (noteInfo.isWhiteKey()) continue;

		double h = note.duration * m_vPianoData.pps();
		double totalTravelTime = m_fallingTime_s + note.duration;
		double elapsedSinceSpawn = (currentTime - (note.startTime - m_fallingTime_s));
		double progress = elapsedSinceSpawn / totalTravelTime;
		progress = std::clamp(progress, 0.0, 1.0);
		float y = -h + static_cast<float>(progress) * (m_vPianoData.vpy() + h);
		float x = m_vPianoData.keyOffsets()[noteInfo.keyIndex] + (m_vPianoData.blackKey_shrink() / 2.0f);

		if (y >= m_vPianoData.vpy())
		{
			auto& key = m_pianoKeys[noteInfo.keyIndex];
			key.pressed = false;
			key.pressedForce = { 0.0f, 0.0f };
			NoteEventData ned(key);
			ned.eventType = NoteEventData::eEventType::NoteOFF;
			ned.note = note;
			ostd::SignalHandler::emitSignal(NoteOffSignal, ostd::tSignalPriority::RealTime, ned);
		}
		else if (y + h >= m_vPianoData.vpy())
		{
			auto& key = m_pianoKeys[noteInfo.keyIndex];
			key.pressed = true;
			key.pressedForce = l_calcPressedVelocity(note.velocity);
			NoteEventData ned(key);
			ned.eventType = NoteEventData::eEventType::NoteON;
			ned.note = note;
			ostd::SignalHandler::emitSignal(NoteOnSignal, ostd::tSignalPriority::RealTime, ned);
			if (!m_firstNotePlayed)
			{
				ostd::SignalHandler::emitSignal(MidiStartSignal, ostd::tSignalPriority::RealTime, ned);
				m_firstNotePlayed = true;
			}
		}
		ostd::Color noteColor = m_vPianoData.fallingBlackNoteColor;
		ostd::Color outlineColor = m_vPianoData.fallingBlackNoteOutlineColor;
		ostd::Color glowColor = m_vPianoData.fallingBlackNoteGlowColor;
		if (m_vPianoData.usePerNoteColors)
		{
			noteColor = m_vPianoData.perNoteColors[noteInfo.noteInOctave];
			outlineColor = m_vPianoData.perNoteColors[noteInfo.noteInOctave + 12];
			glowColor = m_vPianoData.perNoteColors[noteInfo.noteInOctave + 24];
		}
		m_fallingNoteGfx_b.push_back(FallingNoteGraphicsData {
			{ static_cast<float>(x), static_cast<float>(y), m_vPianoData.blackKey_w() - m_vPianoData.blackKey_shrink(), static_cast<float>(h) },
			noteColor,
			outlineColor,
			glowColor,
			&noteTexture,
			-m_vPianoData.fallingBlackNoteOutlineWidth,
			m_vPianoData.fallingBlackNoteBorderRadius
		});
	}
}

void VirtualPiano::updateVisualization(double currentTime)
{
	// Remove notes that have ended
	while (!m_activeFallingNotes.empty() && currentTime > (m_activeFallingNotes.front().endTime + 0.05))
	{
		auto note = m_activeFallingNotes.front();
		auto info = MidiParser::getNoteInfo(note.pitch);
		m_pianoKeys[info.keyIndex].pressed = false;
		m_pianoKeys[info.keyIndex].pressedForce = { 0.0f, 0.0f };
		if (note.last)
		{
			auto& key = m_pianoKeys[info.keyIndex];
			NoteEventData ned(key);
			ned.eventType = NoteEventData::eEventType::MidiEnd;
			ned.note = note;
			ostd::SignalHandler::emitSignal(MidiEndSignal, ostd::tSignalPriority::RealTime, ned);
		}
		m_activeFallingNotes.pop_front();
	}

	// Add new notes that are starting now
	while (m_nextFallingNoteIndex < m_midiNotes.size() && currentTime >= m_midiNotes[m_nextFallingNoteIndex].startTime - m_fallingTime_s)
	{
		// auto info = MidiParser::getNoteInfo(m_midiNotes[m_nextFallingNoteIndex].pitch);
		m_activeFallingNotes.push_back(m_midiNotes[m_nextFallingNoteIndex]);
		++m_nextFallingNoteIndex;
	}
	calculateFallingNotes(currentTime);
}

void VirtualPiano::render(std::optional<std::reference_wrapper<sf::RenderTarget>> target)
{
	if (m_isRenderingToFile)
	{
		ostd::Vec2 render_scale = m_vPianoData.getScale();
		m_vPianoData.setScale(m_videoRenderState.oldScale);
		Renderer::clear(m_vPianoData.backgroundColor);
		renderVirtualKeyboard(std::nullopt);
		m_vPianoData.setScale(render_scale);
		if (m_videoRenderState.mode == VideoRenderModes::ImageSequence)
		{
			__render_next_output_frame();
			if (m_videoRenderState.isFinished())
				__finish_output_render();
		}
		else if (m_videoRenderState.mode == VideoRenderModes::Video)
		{
			__render_next_output_frame();
			if (m_videoRenderState.isFinished())
				__finish_output_render();
		}
	}
	else
	{
		__render_frame(std::nullopt);
	}
}

void VirtualPiano::renderVirtualKeyboard(std::optional<std::reference_wrapper<sf::RenderTarget>> target)
{
	sf::RenderTarget*  __target = nullptr;
	if (target)
		__target = &target->get();
	Renderer::setRenderTarget(__target);
	for (auto& pk : m_pianoKeys)
	{
		auto& tex = std::any_cast<sf::Texture&>(m_partTex);
		Renderer::useShader(&particleShader);
		particleShader.setUniform("u_texture", tex);
		Renderer::useTexture(&tex);
		Renderer::drawParticleSysten(pk.particles);
	}
	Renderer::useShader(nullptr);
	Renderer::useTexture(nullptr);
	auto& vpd = m_vPianoData;
	int whiteKeyCount = 0;
	for (int midiNote = 21; midiNote <= 108; ++midiNote)
	{
		int noteInOctave = midiNote % 12;
		if (MidiParser::NoteInfo::isWhiteKey(noteInOctave)) // Draw white key
		{
			auto info = MidiParser::getNoteInfo(midiNote);
			PianoKey& pk = m_pianoKeys[info.keyIndex];
			ostd::Color keyColor = (pk.pressed ? vpd.whiteKeyPressedColor : vpd.whiteKeyColor);
			float x = vpd.vpx() + (whiteKeyCount * vpd.whiteKey_w());
			float y = vpd.vpy();
			pk.particles.setEmissionRect({ x + (vpd.whiteKey_w() / 2.0f) - (vpd.whiteKey_w() / 8.0f), y - 2.0f, vpd.whiteKey_w() / 4.0f, 2.0f });
			Renderer::outlineRect({ x, y, vpd.whiteKey_w(), vpd.whiteKey_h() }, keyColor, vpd.whiteKeySplitColor, 1 );
			whiteKeyCount++;
		}
	}
	Renderer::outlineRect({ vpd.vpx(), vpd.vpy() - 2, (float)m_parentWindow.getWindowWidth(), 2 }, vpd.pianoLineColor1, vpd.pianoLineColor1, 1);
	Renderer::outlineRect({ vpd.vpx(), vpd.vpy(), (float)m_parentWindow.getWindowWidth(), 5 }, vpd.pianoLineColor2, vpd.pianoLineColor2, 1);
	whiteKeyCount = 0;
	for (int midiNote = 21; midiNote <= 108; ++midiNote)
	{
		int noteInOctave = midiNote % 12;
		if (MidiParser::NoteInfo::isWhiteKey(noteInOctave))
		{
			whiteKeyCount++;
		}
		else // Draw black key
		{
			auto info = MidiParser::getNoteInfo(midiNote);
			PianoKey& pk = m_pianoKeys[info.keyIndex];
			ostd::Color keyColor = (pk.pressed ? vpd.blackKeyPressedColor : vpd.blackKeyColor);
			float x = vpd.vpx() + ((whiteKeyCount - 1) * vpd.whiteKey_w() + (vpd.whiteKey_w() - vpd.blackKey_w() / 2.0f)) - vpd.blackKey_offset();
			float y = vpd.vpy();
			pk.particles.setEmissionRect({ x + (vpd.blackKey_w() / 2.0f) - (vpd.blackKey_w() / 8.0f), y - 2.0f, vpd.blackKey_w() / 4.0f, 2.0f });
			Renderer::outlineRoundedRect({ x, y, vpd.blackKey_w(), vpd.blackKey_h() }, keyColor, vpd.blackKeySplitColor, { 0, 0, 5, 5 }, 1);
		}
	}


	// DEBUG
	// whiteKeyCount = 0;
	// for (int midiNote = 21; midiNote <= 108; ++midiNote)
	// {
	// 	int noteInOctave = midiNote % 12;
	// 	if (MidiParser::NoteInfo::isWhiteKey(noteInOctave))
	// 	{
	// 		float x = vpd.vpx() + (whiteKeyCount * vpd.whiteKey_w());
	// 		float y = vpd.vpy();
	// 		Renderer::fillRect({ x + (vpd.whiteKey_w() / 2.0f) - (vpd.whiteKey_w() / 8.0f), y - 2.0f, vpd.whiteKey_w() / 4.0f, 2.0f }, { 255, 255, 0, 100 });
	// 		whiteKeyCount++;
	// 	}
	// }
	// whiteKeyCount = 0;
	// for (int midiNote = 21; midiNote <= 108; ++midiNote)
	// {
	// 	int noteInOctave = midiNote % 12;
	// 	if (MidiParser::NoteInfo::isWhiteKey(noteInOctave))
	// 	{
	// 		whiteKeyCount++;
	// 	}
	// 	else // Draw black key
	// 	{
	// 		float x = vpd.vpx() + ((whiteKeyCount - 1) * vpd.whiteKey_w() + (vpd.whiteKey_w() - vpd.blackKey_w() / 2.0f)) - vpd.blackKey_offset();
	// 		float y = vpd.vpy();
	// 		Renderer::fillRect({ x + (vpd.blackKey_w() / 2.0f) - (vpd.blackKey_w() / 8.0f), y - 2.0f, vpd.blackKey_w() / 4.0f, 2.0f }, { 0, 0, 255, 100 });
	// 	}
	// }
}

void VirtualPiano::drawFallingNote(const FallingNoteGraphicsData& noteData)
{
	Renderer::useTexture(noteData.texture);
	Renderer::setTextureRect({ m_vPianoData.texCoordsPos.x, m_vPianoData.texCoordsPos.y, (float)noteData.texture->getSize().x * m_vPianoData.texCoordsScale.x, (float)noteData.texture->getSize().y * m_vPianoData.texCoordsScale.y });
	Renderer::useShader(&noteShader);
	Renderer::outlineRoundedRect(noteData.rect, noteData.fillColor, noteData.outlineColor, { noteData.cornerRadius, noteData.cornerRadius, noteData.cornerRadius, noteData.cornerRadius }, noteData.outlineThickness);
}

void VirtualPiano::drawFallingNoteGlow(const FallingNoteGraphicsData& noteData)
{
	Renderer::useTexture(nullptr);
	Renderer::useShader(nullptr);
	auto margins = m_vPianoData.getGlowMargins();
	ostd::Rectangle bounds = {
		noteData.rect.x - margins.x,
		noteData.rect.y - margins.y,
		noteData.rect.w + margins.x + margins.w,
		noteData.rect.h + margins.y + margins.h
	};
	Renderer::fillRoundedRect(bounds, noteData.glowColor, { noteData.cornerRadius, noteData.cornerRadius, noteData.cornerRadius, noteData.cornerRadius });
}

void VirtualPiano::drawFallingNoteOutline(const FallingNoteGraphicsData& noteData)
{
	Renderer::useTexture(nullptr);
	Renderer::useShader(nullptr);
	Renderer::drawRoundedRect(noteData.rect, noteData.outlineColor, { noteData.cornerRadius, noteData.cornerRadius, noteData.cornerRadius, noteData.cornerRadius }, noteData.outlineThickness);
}




// File rendering
bool VirtualPiano::configImageSequenceRender(const ostd::String& folderPath, const ostd::UI16Point& resolution, uint8_t fps)
{
	if (m_isRenderingToFile) return false;
	if (resolution.x != 1920 || resolution.y != 1080) return false; //TODO: allow for valid resolutions
	if (fps != 60) return false; //TODO: allow for valid FPS values
	if (m_lastNoteEndTime == 0.0) return false; //TODO: Error

	m_videoRenderState.reset();
	m_videoRenderState.mode = VideoRenderModes::ImageSequence;
	m_videoRenderState.resolution = resolution;
	m_videoRenderState.folderPath = folderPath;
	m_videoRenderState.targetFPS = fps;
	m_videoRenderState.lastNoteEndTime = m_lastNoteEndTime;
	m_videoRenderState.totalFrames = (int32_t)std::ceil(m_videoRenderState.lastNoteEndTime * fps);
	m_videoRenderState.oldScale = m_vPianoData.getScale();
	m_videoRenderState.renderTarget = sf::RenderTexture({ resolution.x, resolution.y });
	m_videoRenderState.flippedRenderTarget = sf::RenderTexture({ resolution.x, resolution.y });
	m_videoRenderState.frameTime = 1.0 / (float)fps;
	m_videoRenderState.renderFPS = 1;

	__preallocate_file_names_for_rendering(m_videoRenderState.totalFrames, m_videoRenderState.baseFileName, m_videoRenderState.folderPath, m_videoRenderState.imageType);
	Common::ensureDirectory(folderPath);
	m_vPianoData.updateScale(resolution.x, resolution.y);
	onWindowResized(resolution.x, resolution.y);
	m_parentWindow.lockFullscreenStatus();
	m_parentWindow.enableResizeable(false);
	stop();
	m_videoRenderState.updateFpsTimer.startCount(ostd::eTimeUnits::Milliseconds);

	m_isRenderingToFile = true;
	return true;
}

bool VirtualPiano::configFFMPEGVideoRender(const ostd::String& filePath, const ostd::UI16Point& resolution, uint8_t fps, const FFMPEG::tProfile& profile)
{
	if (m_isRenderingToFile) return false;
	if (resolution.x != 1920 || resolution.y != 1080) return false; //TODO: allow for valid resolutions
	if (fps != 60) return false; //TODO: allow for valid FPS values
	if (m_lastNoteEndTime == 0.0) return false; //TODO: Error

	m_videoRenderState.reset();
	m_videoRenderState.ffmpegProfile = profile;
	m_videoRenderState.mode = VideoRenderModes::Video;
	m_videoRenderState.resolution = resolution;
	ostd::String tmp = filePath.new_trim();
	while (tmp.startsWith("./") || tmp.startsWith(".\\"))
		tmp.substr(2).trim();
	m_videoRenderState.folderPath = tmp;
	m_videoRenderState.absolutePath = ostd::String(std::filesystem::absolute(m_videoRenderState.folderPath)).add(".").add(profile.Container);
	m_videoRenderState.targetFPS = fps;
	m_videoRenderState.lastNoteEndTime = m_lastNoteEndTime;
	m_videoRenderState.totalFrames = (int32_t)std::ceil(m_videoRenderState.lastNoteEndTime * fps);
	m_videoRenderState.oldScale = m_vPianoData.getScale();
	m_videoRenderState.renderTarget = sf::RenderTexture({ resolution.x, resolution.y });
	m_videoRenderState.flippedRenderTarget = sf::RenderTexture({ resolution.x, resolution.y });
	m_videoRenderState.frameTime = 1.0 / (float)fps;
	m_videoRenderState.renderFPS = 1;
	m_vPianoData.updateScale(resolution.x, resolution.y);
	onWindowResized(resolution.x, resolution.y);
	m_parentWindow.lockFullscreenStatus();
	m_parentWindow.enableResizeable(false);
	stop();
	m_videoRenderState.updateFpsTimer.startCount(ostd::eTimeUnits::Milliseconds);
	m_videoRenderState.ffmpegPipe = __open_ffmpeg_pipe(m_videoRenderState.folderPath, resolution, fps, profile);
	if (m_videoRenderState.ffmpegPipe == nullptr) return false;

	m_isRenderingToFile = true;
	return true;
}




// Private methods
void VirtualPiano::__render_frame(std::optional<std::reference_wrapper<sf::RenderTarget>> target)
{
	sf::RenderTarget*  __target = nullptr;
	if (target)
		__target = &target->get();
	Renderer::setRenderTarget(&m_glowBuffer);
	Renderer::clear({ 0, 0, 0, 0 });
	for (const auto& note : m_fallingNoteGfx_w)
	{
		drawFallingNoteGlow(note);
	}
	for (auto& note : m_fallingNoteGfx_b)
	{
		drawFallingNoteGlow(note);
	}
	m_glowBuffer.display();
	Renderer::setRenderTarget(&m_blurBuff1);
	Renderer::clear({ 0, 0, 0, 0 });
	Renderer::useShader(&blurShader);
	blurShader.setUniform("texture", m_glowBuffer.getTexture());
	blurShader.setUniform("direction", sf::Glsl::Vec2(1.f, 0.f));
	blurShader.setUniform("resolution", (float)m_glowBuffer.getSize().x);
	blurShader.setUniform("spread", 2.5f);
	blurShader.setUniform("intensity", 1.0f);
	Renderer::drawTexture(m_glowBuffer.getTexture());
	m_blurBuff1.display();

	Renderer::setRenderTarget(&m_blurBuff2);
	Renderer::clear({ 0, 0, 0, 0 });
	blurShader.setUniform("texture", m_blurBuff1.getTexture());
	blurShader.setUniform("direction", sf::Glsl::Vec2(0.f, 1.f));
	blurShader.setUniform("resolution", (float)m_blurBuff2.getSize().y);
	Renderer::drawTexture(m_blurBuff1.getTexture());
	m_blurBuff2.display();

	Renderer::setRenderTarget(__target);
	Renderer::useTexture(nullptr);
	Renderer::useShader(nullptr);
	Renderer::clear(m_vPianoData.backgroundColor);
	if (m_showBackground)
	{
		Renderer::drawSprite(*m_backgroundSpr);
	}
	Renderer::useShader(&particleShader);
	auto& tex = std::any_cast<sf::Texture&>(m_partTex);
	particleShader.setUniform("u_texture", tex);
	Renderer::useTexture(&tex);
	// Renderer::drawParticleSysten(m_snow);
	Renderer::useShader(nullptr);
	Renderer::useTexture(nullptr);
	Renderer::drawTexture(m_blurBuff2.getTexture(), { 0, 0 }, 2);

	for (const auto& note : m_fallingNoteGfx_w)
	{
		noteShader.setUniform("u_texture", noteTexture);
		noteShader.setUniform("u_color", color_to_glsl(note.fillColor));
		drawFallingNote(note);
		drawFallingNoteOutline(note);
	}
	for (auto& note : m_fallingNoteGfx_b)
	{
		noteShader.setUniform("u_texture", noteTexture);
		noteShader.setUniform("u_color", color_to_glsl(note.fillColor));
		drawFallingNote(note);
		drawFallingNoteOutline(note);
	}

	renderVirtualKeyboard(target);
}

void VirtualPiano::__preallocate_file_names_for_rendering(uint32_t frameCount, const ostd::String& baseFileName, const ostd::String& basePath, ImageType imageType, const uint16_t marginFrames)
{
	ostd::String extension = "png";
	if (imageType == ImageType::BMP) extension = "bmp";
	if (imageType == ImageType::JPG) extension = "jpg";
	m_renderFileNames.clear();
	char filename[256];
	for (int32_t i = 0; i < frameCount + marginFrames; i++)
	{
    	std::snprintf(filename, sizeof(filename), "%s/%s%06d.%s", basePath.c_str(), baseFileName.c_str(), i, extension.c_str());
    	m_renderFileNames.push_back(filename);
	}
}

FILE* VirtualPiano::__open_ffmpeg_pipe(const ostd::String& filePath, const ostd::UI16Point& resolution, uint8_t fps, const FFMPEG::tProfile& profile)
{
	if (!FFMPEG::exists()) return nullptr; // TODO: Error
	if (!FFMPEG::isEncodeCodecAvailable(profile.VideoCodec)) return nullptr; //TODO: Error
	if (!FFMPEG::isEncodeCodecAvailable(profile.AudioCodec)) return nullptr; //TODO: Error

	m_videoRenderState.subProcArgs.push_back("-f");
	m_videoRenderState.subProcArgs.push_back("rawvideo");
	m_videoRenderState.subProcArgs.push_back("-pix_fmt");
	m_videoRenderState.subProcArgs.push_back("rgba");
	m_videoRenderState.subProcArgs.push_back("-s");
	m_videoRenderState.subProcArgs.push_back(ostd::String("").add(resolution.x).add("x").add(resolution.y));
	m_videoRenderState.subProcArgs.push_back("-r");
	m_videoRenderState.subProcArgs.push_back(ostd::String("").add(fps));
	// m_videoRenderState.subProcArgs.push_back("-re");
	m_videoRenderState.subProcArgs.push_back("-i");
	m_videoRenderState.subProcArgs.push_back("-");
	if (m_hasAudioFile)
	{
		if (m_firstNoteStartTime > m_autoSoundStart)
		{
			// m_videoRenderState.subProcArgs.push_back("-itsoffset");
			// m_videoRenderState.subProcArgs.push_back(ostd::String("").add((m_firstNoteStartTime - m_autoSoundStart)));
			m_videoRenderState.subProcArgs.push_back("-f");
			m_videoRenderState.subProcArgs.push_back("lavfi");
			m_videoRenderState.subProcArgs.push_back("-i");
			m_videoRenderState.subProcArgs.push_back(ostd::String("anullsrc=channel_layout=stereo:sample_rate=44100:duration=").add((m_firstNoteStartTime - m_autoSoundStart)));
			m_videoRenderState.subProcArgs.push_back("-i");
			m_videoRenderState.subProcArgs.push_back(ostd::String("").add(m_audioFilePath).add(""));
			m_videoRenderState.subProcArgs.push_back("-filter_complex");
			// m_videoRenderState.subProcArgs.push_back("[1:a][2:a]concat=n=2:v=0:a=1[aout]");
			m_videoRenderState.subProcArgs.push_back(
			    "[1:a]aformat=sample_fmts=s16:channel_layouts=stereo:sample_rates=44100[sil];"
			    "[2:a]aformat=sample_fmts=s16:channel_layouts=stereo:sample_rates=44100[aud];"
			    "[sil][aud]concat=n=2:v=0:a=1[aout]"
			);
			m_videoRenderState.subProcArgs.push_back("-map");
			m_videoRenderState.subProcArgs.push_back("0:v");
			m_videoRenderState.subProcArgs.push_back("-map");
			m_videoRenderState.subProcArgs.push_back("[aout]");
		}
		else if (m_autoSoundStart > m_firstNoteStartTime)
		{
			m_videoRenderState.subProcArgs.push_back("-ss");
			m_videoRenderState.subProcArgs.push_back(ostd::String("").add((m_autoSoundStart - m_firstNoteStartTime)));
			m_videoRenderState.subProcArgs.push_back("-i");
			m_videoRenderState.subProcArgs.push_back(ostd::String("").add(m_audioFilePath).add(""));
		}
		else
		{
			m_videoRenderState.subProcArgs.push_back("-i");
			m_videoRenderState.subProcArgs.push_back(ostd::String("").add(m_audioFilePath).add(""));
		}
	}
	m_videoRenderState.subProcArgs.push_back("-c:v");
	m_videoRenderState.subProcArgs.push_back(ostd::String("").add(profile.VideoCodec));
	m_videoRenderState.subProcArgs.push_back("-preset");
	m_videoRenderState.subProcArgs.push_back(ostd::String("").add(profile.Preset));
	m_videoRenderState.subProcArgs.push_back("-crf");
	m_videoRenderState.subProcArgs.push_back(ostd::String("").add(profile.Quality));
	m_videoRenderState.subProcArgs.push_back("-c:a");
	m_videoRenderState.subProcArgs.push_back(ostd::String("").add(profile.AudioCodec));
	m_videoRenderState.subProcArgs.push_back("-b:a");
	m_videoRenderState.subProcArgs.push_back("192k");
	m_videoRenderState.subProcArgs.push_back("-y");
	// m_videoRenderState.subProcArgs.push_back("-t");
	// m_videoRenderState.subProcArgs.push_back("10");
	m_videoRenderState.subProcArgs.push_back("-loglevel");
	m_videoRenderState.subProcArgs.push_back("error");
	m_videoRenderState.subProcArgs.push_back("-shortest");
	m_videoRenderState.subProcArgs.push_back("-movflags");
	m_videoRenderState.subProcArgs.push_back("+faststart");
	m_videoRenderState.subProcArgs.push_back(ostd::String("").add(filePath).add(".").add(profile.Container).add(""));

	ostd::String ffmpeg_executable = FFMPEG::getExecutablePath();
	ostd::String cmd = ffmpeg_executable.new_add(" ");
	for (const auto& str : m_videoRenderState.subProcArgs)
		cmd.add(str).add(" ");
	OX_DEBUG("FFMPEG Command:\n%s", cmd.c_str());

	// LAUNCH FFMPEG
    try
    {
        m_videoRenderState.ffmpeg_child = bp::child(
            bp::exe = ffmpeg_executable.cpp_str(),
            bp::args = m_videoRenderState.subProcArgs,
            bp::std_in < m_videoRenderState.ffmpeg_stdin,
            bp::std_out > bp::null,
            bp::std_err > stderr  // ← SEE FFMPEG ERRORS IN CONSOLE
        );
    }
    catch (const std::exception& e)
    {
        OX_ERROR("Boost.Process v1 failed: %s", e.what());
        return nullptr;
    }
    // Convert opstream to FILE* for your fwrite()
    int fd = m_videoRenderState.ffmpeg_stdin.pipe().native_sink();
    FILE* pipe_file = fdopen(fd, "wb");
    if (!pipe_file)
    {
        OX_ERROR("fdopen failed");
        m_videoRenderState.ffmpeg_child.terminate();
        return nullptr;
    }
    return pipe_file;
}

void VirtualPiano::__save_frame_to_file(const sf::RenderTexture& rt, const ostd::String& basePath, int frameIndex)
{
	if (!m_isRenderingToFile) return;
	if (m_videoRenderState.mode != VideoRenderModes::ImageSequence) return;
    sf::Image img = rt.getTexture().copyToImage();
    if (!img.saveToFile(m_renderFileNames[frameIndex]))
    {
        OX_ERROR("Failed to save frame %d to %s", frameIndex, m_renderFileNames[frameIndex].c_str());
    }
}

void VirtualPiano::__stream_frame_to_ffmpeg(void)
{
	if (!m_isRenderingToFile) return;
	if (m_videoRenderState.mode != VideoRenderModes::Video) return;
	sf::Image frame = m_videoRenderState.flippedRenderTarget.getTexture().copyToImage();
    const uint8_t* pixels = frame.getPixelsPtr();
    std::size_t dataSize = m_videoRenderState.flippedRenderTarget.getSize().x * m_videoRenderState.flippedRenderTarget.getSize().y * 4;
    if (!m_videoRenderState.ffmpeg_child.running())
    {
        OX_ERROR("FFmpeg not running");
        return;
    }
    if (fwrite(pixels, 1, dataSize, m_videoRenderState.ffmpegPipe) != dataSize)
    {
        OX_ERROR("fwrite failed: %s", strerror(errno));
        return;
    }
    fflush(m_videoRenderState.ffmpegPipe);
}

void VirtualPiano::__render_next_output_frame(void)
{
	if (!m_isRenderingToFile) return;
	updateVisualization(m_videoRenderState.currentTime);
	__render_frame(m_videoRenderState.renderTarget);
	m_videoRenderState.framTimeTimer.startCount(ostd::eTimeUnits::Milliseconds);
	Renderer::setRenderTarget(&m_videoRenderState.flippedRenderTarget);
	Renderer::useShader(&flipShader);
	flipShader.setUniform("texture", m_videoRenderState.renderTarget.getTexture());
	Renderer::drawTexture(m_videoRenderState.renderTarget.getTexture());
	Renderer::useShader(nullptr);
	if (m_videoRenderState.mode == VideoRenderModes::ImageSequence)
		__save_frame_to_file(m_videoRenderState.flippedRenderTarget, m_videoRenderState.folderPath, ++m_videoRenderState.frameIndex);
	else if (m_videoRenderState.mode == VideoRenderModes::Video)
	{
		__stream_frame_to_ffmpeg();
		++m_videoRenderState.frameIndex;
	}
	double _frame_render_time = (double)m_videoRenderState.framTimeTimer.endCount();
	if (m_videoRenderState.updateFpsTimer.read() > 1000 == 0)
	{
		m_videoRenderState.renderFPS = (int32_t)std::round(1000.0 / _frame_render_time);
		m_videoRenderState.updateFpsTimer.endCount();
		m_videoRenderState.updateFpsTimer.startCount(ostd::eTimeUnits::Milliseconds);
	}
	m_videoRenderState.percentage = Common::percentage(m_videoRenderState.frameIndex, m_videoRenderState.totalFrames + m_videoRenderState.extraFrames);

	m_videoRenderState.currentTime += m_videoRenderState.frameTime;
	Renderer::setRenderTarget(nullptr);
}

void VirtualPiano::__finish_output_render(void)
{
	if (!m_isRenderingToFile) return;
	if (!m_videoRenderState.isFinished()) return;
	m_vPianoData.updateScale(m_parentWindow.getWindowWidth(), m_parentWindow.getWindowHeight());
	onWindowResized(m_parentWindow.getWindowWidth(), m_parentWindow.getWindowHeight());
	m_parentWindow.lockFullscreenStatus(false);
	m_parentWindow.enableResizeable(true);
	m_isRenderingToFile = false;
	if (m_videoRenderState.mode == VideoRenderModes::Video)
	{
	    if (m_videoRenderState.ffmpegPipe)
		{
			fflush(m_videoRenderState.ffmpegPipe);
			fclose(m_videoRenderState.ffmpegPipe);
			m_videoRenderState.ffmpegPipe = nullptr;
	    }

	    if (m_videoRenderState.ffmpeg_child.running())
		{
			m_videoRenderState.ffmpeg_child.wait();
			int exit_code = m_videoRenderState.ffmpeg_child.exit_code();
	        if (exit_code == 0)
	            OX_DEBUG("Video encoded successfully!");
	        else
	            OX_ERROR("FFmpeg failed with exit code: %d", exit_code);
	    }
	}
}

void VirtualPiano::__load_resources(void)
{
	if (!noteShader.loadFromFile("shaders/basic.vert", "shaders/note.frag"))
		OX_ERROR("Failed to load shader");
	if (!blurShader.loadFromFile("shaders/basic.vert", "shaders/blur.frag"))
		OX_ERROR("Failed to load shader");
	if (!flipShader.loadFromFile("shaders/basic.vert", "shaders/flip.frag"))
		OX_ERROR("Failed to load shader");
	if (!particleShader.loadFromFile("shaders/basic.vert", "shaders/particle.frag"))
		OX_ERROR("Failed to load shader");

	if (!noteTexture.loadFromFile("res/tex/note.png"))
		OX_ERROR("Failed to load texture");
	noteTexture.setRepeated(true);

	m_partTex = sf::Texture("res/tex/simpleParticle.png");
	sf::Texture& tex = std::any_cast<sf::Texture&>(m_partTex);
	if (!m_partTex.has_value())
		OX_ERROR("Unable To load texture: %s", "res/tex/simpleParticle.png");

	m_partTexRef.attachTexture(m_partTex, tex.getSize().x, tex.getSize().y);
	m_partTiles.push_back(m_partTexRef.addTileInfo(0, 0, 32, 32));
	m_partTiles.push_back(m_partTexRef.addTileInfo(32, 0, 32, 32));
	m_partTiles.push_back(m_partTexRef.addTileInfo(64, 0, 32, 32));
	m_partTiles.push_back(m_partTexRef.addTileInfo(96, 0, 32, 32));
	m_partTiles.push_back(m_partTexRef.addTileInfo(128, 0, 32, 32));
	m_partTiles.push_back(m_partTexRef.addTileInfo(160, 0, 32, 32));

	ostd::String backgroundPath = "res/background.png";
	sf::Image background;
	if (!background.loadFromFile(backgroundPath))
		OX_WARN("Unable to load background image.");
	else
	{
		(void)m_backgroundTex.loadFromImage(background);
		m_backgroundOriginalSize = { (float)m_backgroundTex.getSize().x, (float)m_backgroundTex.getSize().y };
		m_backgroundSpr = sf::Sprite(m_backgroundTex);
		m_backgroundSpr->setPosition({ 0, 0 });
		ostd::Vec2 scale = { m_backgroundOriginalSize.x / (float)m_parentWindow.getWindowWidth(), m_backgroundOriginalSize.y / (float)m_parentWindow.getWindowHeight() };
		scale = { 1.0f / scale.x, 1.0f / scale.y };
		m_backgroundSpr->setScale({ scale.x, scale.y });
		OX_DEBUG("Loaded background image: <%s>", backgroundPath.c_str());
	}
}
