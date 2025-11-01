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
#include <cmath>
#include <ostd/Geometry.hpp>
#include <ostd/Logger.hpp>
#include <ostd/Signals.hpp>
#include <ostd/Utils.hpp>
#include "Window.hpp"
#include "Renderer.hpp"

VirtualPiano::VirtualPianoData::VirtualPianoData(void)
{
	whiteKeyWidth = 40;
	whiteKeyHeight = whiteKeyWidth * 8;
	blackKeyWidth = 22;
	blackKeyHeight = blackKeyWidth * 9;
	blackKeyOffset = 4;
	whiteKeyColor = { 245, 245, 245 };
	whiteKeyPressedColor = { 120, 120, 210 };
	blackKeyColor = { 0, 0, 0 };
	blackKeyPressedColor = { 20, 20, 90 };
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

	whiteKeyShrinkFactor = 8;
	blackKeyShrinkFactor = 0;
}

void VirtualPiano::VirtualPianoData::recalculateKeyOffsets(void)
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

void VirtualPiano::VirtualPianoData::updateScale(int32_t width, int32_t height)
{
	scale_x = (float)width / (float)base_width;
	scale_y = (float)height / (float)base_height;
	Common::guiScaleX = scale_x;
	Common::guiScaleY = scale_y;
}


void VirtualPiano::SignalListener::handleSignal(ostd::tSignal& signal)
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



void VirtualPiano::init(void)
{
	m_playing = false;
	m_paused = false;
	m_firstNotePlayed = false;

	for (int midiNote = 21; midiNote <= 108; ++midiNote)
	{
		PianoKey pk;
		pk.noteInfo = MidiParser::getNoteInfo(midiNote);
		pk.pressed = false;
		m_pianoKeys.push_back(pk);
	}
	if (!noteShader.loadFromFile("shaders/note.vert", "shaders/note.frag"))
		OX_ERROR("Failed to load shader");
	if (!blurShader.loadFromFile("shaders/note.vert", "shaders/blur.frag"))
		OX_ERROR("Failed to load shader");
	if (!flipShader.loadFromFile("shaders/flip.vert", "shaders/flip.frag"))
		OX_ERROR("Failed to load shader");
	if (!noteTexture.loadFromFile("res/tex/note.jpg"))
		OX_ERROR("Failed to load texture");
	noteTexture.setRepeated(true);

	sf::Vector2u winSize = { m_parentWindow.sfWindow().getSize().x, m_parentWindow.sfWindow().getSize().y };
	sf::Vector2u winHalfSize = { winSize.x / 2, winSize.y / 2 };
	m_blurBuff1 = sf::RenderTexture(winHalfSize);
	m_blurBuff2 = sf::RenderTexture(winHalfSize);
	m_glowBuffer = sf::RenderTexture(winHalfSize);

	m_glowView.setSize({ (float)winSize.x, (float)winSize.y });
	m_glowView.setCenter({ winSize.x / 2.f, winSize.y / 2.f });
	m_glowBuffer.setView(m_glowView);
}

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
		pk.pressed = false;
	update();
	m_playing = false;
}

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
		}
		std::sort(m_midiNotes.begin(), m_midiNotes.end());
		OX_DEBUG("loaded <%s>: total notes parsed: %d", filePath.c_str(), m_midiNotes.size());
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
	return true;
}

double VirtualPiano::getPlayTime_s(void)
{
	double playTime = Common::getCurrentTIme_ns() - m_startTimeOffset_ns;
	return playTime * 1e-9;
}

void VirtualPiano::update(void)
{
	if (m_playing)
	{
		updateVisualization(getPlayTime_s());
	}
}

void VirtualPiano::render(std::optional<std::reference_wrapper<sf::RenderTarget>> target)
{
	sf::RenderTarget*  __target = nullptr;
	if (target)
		__target = &target->get();
	Renderer::setRenderTarget(&m_glowBuffer);
	Renderer::clear({ 10, 10, 30, 0 });
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
	Renderer::clear({ 10, 10, 30, 0 });
	Renderer::useShader(&blurShader);
	blurShader.setUniform("texture", m_glowBuffer.getTexture());
	blurShader.setUniform("direction", sf::Glsl::Vec2(1.f, 0.f));
	blurShader.setUniform("resolution", (float)m_glowBuffer.getSize().x);
	blurShader.setUniform("spread", 2.5f);
	blurShader.setUniform("intensity", 1.0f);
	Renderer::drawTexture(m_glowBuffer.getTexture());
	m_blurBuff1.display();

	Renderer::setRenderTarget(&m_blurBuff2);
	Renderer::clear({ 10, 10, 30, 0 });
	blurShader.setUniform("texture", m_blurBuff1.getTexture());
	blurShader.setUniform("direction", sf::Glsl::Vec2(0.f, 1.f));
	blurShader.setUniform("resolution", (float)m_blurBuff2.getSize().y);
	Renderer::drawTexture(m_blurBuff1.getTexture());
	m_blurBuff2.display();

	Renderer::setRenderTarget(__target);
	Renderer::useTexture(nullptr);
	Renderer::useShader(nullptr);
	Renderer::clear({ 20, 20, 20 });
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

	Renderer::useShader(nullptr);
	Renderer::useTexture(nullptr);

	renderVirtualKeyboard(target);
}

void VirtualPiano::onWindowResized(uint32_t width, uint32_t height)
{
	m_blurBuff1 = sf::RenderTexture({ width / 2, height / 2 });
	m_blurBuff2 = sf::RenderTexture({ width / 2, height / 2 });
	m_glowBuffer = sf::RenderTexture({ width / 2, height / 2 });

	m_glowView.setSize({ (float)width, (float)height });
	m_glowView.setCenter({ width / 2.f, height / 2.f });
	m_glowBuffer.setView(m_glowView);
}

void VirtualPiano::renderVirtualKeyboard(std::optional<std::reference_wrapper<sf::RenderTarget>> target)
{
	sf::RenderTarget*  __target = nullptr;
	if (target)
		__target = &target->get();
	Renderer::setRenderTarget(__target);
	auto& vpd = m_vPianoData;
	int whiteKeyCount = 0;
	for (int midiNote = 21; midiNote <= 108; ++midiNote)
	{
		int noteInOctave = midiNote % 12;
		if (MidiParser::NoteInfo::isWhiteKey(noteInOctave)) // Draw white key
		{
			auto info = MidiParser::getNoteInfo(midiNote);
			PianoKey pk = m_pianoKeys[info.keyIndex];
			ostd::Color keyColor = (pk.pressed ? vpd.whiteKeyPressedColor : vpd.whiteKeyColor);
			float x = vpd.vpx() + (whiteKeyCount * vpd.whiteKey_w());
			float y = vpd.vpy();
			Renderer::outlineRect({ x, y, vpd.whiteKey_w(), vpd.whiteKey_h() }, keyColor, { 0, 0, 0 }, 1 );
			whiteKeyCount++;
		}
	}
	Renderer::outlineRect({ vpd.vpx(), vpd.vpy() - 2, (float)m_parentWindow.getWindowWidth(), 2 }, { 60, 10, 10 }, { 60, 0, 0 }, 1);
	Renderer::outlineRect({ vpd.vpx(), vpd.vpy(), (float)m_parentWindow.getWindowWidth(), 5 }, { 160, 10, 10 }, { 210, 0, 0 }, 1);
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
			PianoKey pk = m_pianoKeys[info.keyIndex];
			ostd::Color keyColor = (pk.pressed ? vpd.blackKeyPressedColor : vpd.blackKeyColor);
			float x = vpd.vpx() + ((whiteKeyCount - 1) * vpd.whiteKey_w() + (vpd.whiteKey_w() - vpd.blackKey_w() / 2.0f)) - vpd.blackKey_offset();
			float y = vpd.vpy();
			Renderer::outlineRoundedRect({ x, y, vpd.blackKey_w(), vpd.blackKey_h() }, keyColor, { 0, 0, 0 }, { 0, 0, 5, 5 }, 1);
		}
	}
}

void VirtualPiano::calculateFallingNotes(double currentTime)
{
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
			NoteEventData ned(key);
			ned.eventType = NoteEventData::eEventType::NoteOFF;
			ned.note = note;
			ostd::SignalHandler::emitSignal(NoteOffSignal, ostd::tSignalPriority::RealTime, ned);
		}
		else if (y + h >= m_vPianoData.vpy())
		{
			auto& key = m_pianoKeys[noteInfo.keyIndex];
			key.pressed = true;
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
		m_fallingNoteGfx_w.push_back(FallingNoteGraphicsData {
			{ static_cast<float>(x), static_cast<float>(y), m_vPianoData.whiteKey_w() - m_vPianoData.whiteKey_shrink(), static_cast<float>(h) },
			m_vPianoData.fallingWhiteNoteColor,
			m_vPianoData.fallingWhiteNoteOutlineColor,
			m_vPianoData.fallingWhiteNoteGlowColor,
			&noteTexture,
			-3,
			10
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
			NoteEventData ned(key);
			ned.eventType = NoteEventData::eEventType::NoteOFF;
			ned.note = note;
			ostd::SignalHandler::emitSignal(NoteOffSignal, ostd::tSignalPriority::RealTime, ned);
		}
		else if (y + h >= m_vPianoData.vpy())
		{
			auto& key = m_pianoKeys[noteInfo.keyIndex];
			key.pressed = true;
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
		m_fallingNoteGfx_b.push_back(FallingNoteGraphicsData {
			{ static_cast<float>(x), static_cast<float>(y), m_vPianoData.blackKey_w() - m_vPianoData.blackKey_shrink(), static_cast<float>(h) },
			m_vPianoData.fallingBlackNoteColor,
			m_vPianoData.fallingBlackNoteOutlineColor,
			m_vPianoData.fallingBlackNoteGlowColor,
			&noteTexture,
			-3,
			10
		});
	}
}

void VirtualPiano::drawFallingNote(const FallingNoteGraphicsData& noteData)
{
	Renderer::useTexture(noteData.texture);
	Renderer::setTextureRect({ 0, 0, (float)(noteData.texture->getSize().x * 0.1), (float)noteData.texture->getSize().y * 10 });
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

void VirtualPiano::updateVisualization(double currentTime)
{
	// Remove notes that have ended
	while (!m_activeFallingNotes.empty() && currentTime > (m_activeFallingNotes.front().endTime + 0.05))
	{
		auto note = m_activeFallingNotes.front();
		auto info = MidiParser::getNoteInfo(note.pitch);
		m_pianoKeys[info.keyIndex].pressed = false;
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

void VirtualPiano::drawFallingNoteOutline(const FallingNoteGraphicsData& noteData)
{
	Renderer::useTexture(nullptr);
	Renderer::useShader(nullptr);
	Renderer::drawRoundedRect(noteData.rect, noteData.outlineColor, { noteData.cornerRadius, noteData.cornerRadius, noteData.cornerRadius, noteData.cornerRadius }, noteData.outlineThickness);
}

bool VirtualPiano::renderFramesToFile(const ostd::String& folderPath, const ostd::UI16Point& resolution, uint8_t fps)
{
	if (resolution.x != 1920 || resolution.y != 1080) return false; //TODO: allow for valid resolutions
	if (fps != 60) return false; //TODO: allow for valid FPS values

	double last_time = 0.0;
	for (const auto& note : m_midiNotes)
	{
		if (note.last)
		{
			last_time = note.endTime;
			break;
		}
	}

	if (last_time == 0.0) return false; //TODO: Error

	m_isRenderingToFile = true;

	int32_t total_frames = (int32_t)std::ceil(last_time * fps);
	__preallocate_file_names_for_rendering(total_frames, "frame_", folderPath, ".png");

	Common::ensureDirectory(folderPath);
	ostd::Vec2 current_scale = m_vPianoData.getScale();
	m_vPianoData.updateScale(resolution.x, resolution.y);

	sf::RenderTexture render_target({ resolution.x, resolution.y });
	sf::RenderTexture flipped_render_target({ resolution.x, resolution.y });
	stop();

	ostd::Timer fpsTimer;

	int32_t frameIndex = 0;
	int32_t render_fps = 0;
	int percentage = 0;
	double frame_time = 1.0 / (float)fps;
	double current_time = 0.0;
	fpsTimer.startCount(ostd::eTimeUnits::Milliseconds);
	while (true)
	{
		updateVisualization(current_time);
		render(render_target);
		Renderer::setRenderTarget(&flipped_render_target);
		Renderer::useShader(&flipShader);
		flipShader.setUniform("texture", render_target.getTexture());
		Renderer::drawTexture(render_target.getTexture());
		Renderer::useShader(nullptr);
		saveFrame(flipped_render_target, folderPath, ++frameIndex);
		if (Common::wasSIGINTTriggered())
		{
			Common::deleteDirectory(folderPath);
			m_parentWindow.close();
			break;
		}
		if ((frameIndex % fps) == 0)
		{
			double frame_render_time = ((double)fpsTimer.endCount() / (double)fps);
			fpsTimer.startCount(ostd::eTimeUnits::Milliseconds);
			render_fps = (int32_t)std::round(1000.0 / frame_render_time);
		}
		percentage = Common::percentage(current_time, last_time);
		if (frameIndex % 10 == 0)
			std::cout << (int)percentage << "% (FPS: " << (int)render_fps << ")\n";

		current_time += frame_time;

		if (current_time > last_time)
			break;
	}
	m_vPianoData.setScale(current_scale);
	m_isRenderingToFile = false;

	return true;
}

void VirtualPiano::saveFrame(const sf::RenderTexture& rt, const ostd::String& basePath, int frameIndex)
{
    sf::Image img = rt.getTexture().copyToImage();

    // char filename[256];
    // std::snprintf(filename, sizeof(filename), "%s/frame_%04d.png", basePath.c_str(), frameIndex);
    // img.flipVertically();
    if (!img.saveToFile(m_renderFileNames[frameIndex]))
    {
        OX_ERROR("Failed to save frame %d to %s", frameIndex, m_renderFileNames[frameIndex].c_str());
    }
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

sf::VertexArray VirtualPiano::getMusicWaveForm(const ostd::String& filePath, int32_t windowHeight)
{
	sf::SoundBuffer buffer;
	if (!buffer.loadFromFile(filePath.cpp_str()))
	{
		OX_ERROR("Error while trying to load audio file: %s", filePath.c_str());
	}
	const int16_t* samples = buffer.getSamples();
	std::size_t sampleCount = buffer.getSampleCount();
	unsigned int channels = buffer.getChannelCount();
	std::vector<float> amplitudes;
	amplitudes.reserve(1000);
	std::size_t samplesPerPixel = sampleCount / channels / 1000;
	for (std::size_t i = 0; i < sampleCount; i += samplesPerPixel * channels)
	{
		long sum = 0;
		for (unsigned int c = 0; c < channels; ++c)
			sum += std::abs(samples[i + c]);
		amplitudes.push_back(static_cast<float>(sum) / channels / 32768.f);
	}
	sf::VertexArray waveform(sf::PrimitiveType::LineStrip, amplitudes.size());
	for (std::size_t x = 0; x < amplitudes.size(); ++x) {
		float y = (1.f - amplitudes[x]) * windowHeight / 2.f;
		waveform[x].position = sf::Vector2f(static_cast<float>(x), y);
		waveform[x].color = sf::Color::White;
	}
	return waveform;
}

void VirtualPiano::__preallocate_file_names_for_rendering(uint32_t frameCount, const ostd::String& baseFileName, const ostd::String& basePath, const ostd::String& extension, const uint16_t marginFrames)
{
	m_renderFileNames.clear();
	char filename[256];
	for (int32_t i = 0; i < frameCount + marginFrames; i++)
	{
    	std::snprintf(filename, sizeof(filename), "%s/%s%06d.%s", basePath.c_str(), baseFileName.c_str(), i, extension.c_str());
    	m_renderFileNames.push_back(filename);
	}
}
