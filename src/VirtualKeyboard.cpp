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

#include "VirtualKeyboard.hpp"
#include "VirtualPiano.hpp"
#include "Renderer.hpp"
#include "Window.hpp"


VirtualKeyboard::VirtualKeyboard(VirtualPiano& vpiano) : m_vpiano(vpiano)
{
}

void VirtualKeyboard::init(void)
{
}


void VirtualKeyboard::loadFromStyleJSON(ostd::JsonFile& styleJson)
{
	m_pianoKeys.clear();
	for (int midiNote = 21; midiNote <= 108; ++midiNote)
	{
		PianoKey pk;
		pk.noteInfo = ostd::MidiParser::getNoteInfo(midiNote);
		pk.pressed = false;

		uint32_t tileIndex = styleJson.get_int("particles.tileIndex");
		pk.particles = ParticleFactory::basicFireEmitter({ &m_vpiano.vPianoRes().partTexRef, tileIndex }, { 0, 0 }, 0);
		bool useTileArray = styleJson.get_bool("emitter.useTileArray");
		pk.particles.useTileArray(useTileArray);
		pk.particles.setMaxParticleCount(styleJson.get_int("emitter.maxParticles"));
		if (useTileArray && m_vpiano.vPianoRes().partTiles.size() > 0)
			pk.particles.addTilesToArray(m_vpiano.vPianoRes().partTiles);
		pk.particles.setEmissionRect(styleJson.get_rect("emitter.emissionRect"));

		auto& partInfo = pk.particles.getDefaultParticleInfo();
		bool useAutoColorRamp = styleJson.get_bool("particles.automaticColorRamp.use");
		bool useColorRamp = styleJson.get_bool("particles.useColorRamp");
		if (useAutoColorRamp && useColorRamp)
		{
			int32_t colorCount = styleJson.get_int("particles.automaticColorRamp.colorCount");
			ParticleFactory::createColorGradient(partInfo, styleJson.get_color("particles.color"), colorCount);
		}
		else if (useColorRamp)
		{
			auto colArr = styleJson.get_color_array("particles.colorRamp");
			if (colArr.size() > 0)
			{
				size_t count = colArr.size();
				if (count > 1)
				{
				    float percentPerSegment = 1.0f / static_cast<float>(count - 1);
				    for (size_t i = 0; i < count - 1; ++i)
				        partInfo.addColorToGradient(colArr[i], colArr[i + 1], percentPerSegment);
				}
				else
					partInfo.addColorToGradient(colArr[0], colArr[0], 1.0f);
			}
		}
		partInfo.size = styleJson.get_vec2("particles.size");
		partInfo.speed = styleJson.get_float("particles.speed");
		partInfo.randomVelocity = styleJson.get_vec2("particles.randomVelocity");
		partInfo.randomDirection = styleJson.get_float("particles.randomDirection");
		partInfo.randomDamping = styleJson.get_bool("particles.randomDamping");
		partInfo.damping = styleJson.get_vec2("particles.damping");
		partInfo.fadeIn = styleJson.get_bool("particles.fadeIn");
		partInfo.lifeSpan = styleJson.get_float("particles.lifeSpan");
		partInfo.color = styleJson.get_color("particles.color");
		partInfo.allDirections = styleJson.get_bool("particles.allDirections");
		partInfo.randomLifeSpan = styleJson.get_float("particles.randomLifeSpan");
		partInfo.randomSpeed = styleJson.get_float("particles.randomSpeed");
		partInfo.randomAlpha = styleJson.get_float("particles.randomAlpha");
		partInfo.randomSize = styleJson.get_vec2("particles.randomSize");
		partInfo.angle = styleJson.get_float("particles.angle");

		m_pianoKeys.push_back(pk);
	}
}

void VirtualKeyboard::calculateFallingNotes(double currentTime)
{
	auto l_calcPressedVelocity = [this](int32_t midiVelocity) -> ostd::Vec2 {
		return { 0.0f, -((float)(midiVelocity / 128.0f)) * (float)m_vpiano.vPianoData().pressedVelocityMultiplier };
	};

	m_fallingNoteGfx_w.clear();
	m_fallingNoteGfx_b.clear();
	for (auto& note : m_activeFallingNotes)
	{
		auto noteInfo = ostd::MidiParser::getNoteInfo(note.pitch);
		if (noteInfo.isBlackKey()) continue;

		double h = note.duration * m_vpiano.vPianoData().pps();
		double totalTravelTime = m_vpiano.vPianoData().fallingTime_s + note.duration;
		double elapsedSinceSpawn = (currentTime - (note.startTime - m_vpiano.vPianoData().fallingTime_s));
		double progress = elapsedSinceSpawn / totalTravelTime;
		progress = std::clamp(progress, 0.0, 1.0);
		double y = -h + static_cast<double>(progress) * (m_vpiano.vPianoData().vpy() + h);
		double x = m_vpiano.vPianoData().keyOffsets()[noteInfo.keyIndex] + (m_vpiano.vPianoData().whiteKey_shrink() / 2.0f);

		if (y >= m_vpiano.vPianoData().vpy())
		{
			auto& key = m_pianoKeys[noteInfo.keyIndex];
			key.pressed = false;
			key.pressedForce = { 0.0f, 0.0f };
			NoteEventData ned(key);
			ned.eventType = NoteEventData::eEventType::NoteOFF;
			ned.note = note;
			ostd::SignalHandler::emitSignal(SignalListener::NoteOffSignal, ostd::tSignalPriority::RealTime, ned);
		}
		else if (y + h >= m_vpiano.vPianoData().vpy())
		{
			auto& key = m_pianoKeys[noteInfo.keyIndex];
			key.pressed = true;
			key.pressedForce = l_calcPressedVelocity(note.velocity);
			NoteEventData ned(key);
			ned.eventType = NoteEventData::eEventType::NoteON;
			ned.note = note;
			ostd::SignalHandler::emitSignal(SignalListener::NoteOnSignal, ostd::tSignalPriority::RealTime, ned);
			if (!m_vpiano.m_firstNotePlayed)
			{
				ostd::SignalHandler::emitSignal(SignalListener::MidiStartSignal, ostd::tSignalPriority::RealTime, ned);
				m_vpiano.m_firstNotePlayed = true;
			}
		}
		ostd::Color noteColor = m_vpiano.vPianoData().fallingWhiteNoteColor;
		ostd::Color outlineColor = m_vpiano.vPianoData().fallingWhiteNoteOutlineColor;
		ostd::Color glowColor = m_vpiano.vPianoData().fallingWhiteNoteGlowColor;
		if (m_vpiano.vPianoData().usePerNoteColors)
		{
			noteColor = m_vpiano.vPianoData().perNoteColors[noteInfo.noteInOctave];
			outlineColor = m_vpiano.vPianoData().perNoteColors[noteInfo.noteInOctave + 12];
			glowColor = m_vpiano.vPianoData().perNoteColors[noteInfo.noteInOctave + 24];
		}
		m_fallingNoteGfx_w.push_back(FallingNoteGraphicsData {
			{ static_cast<float>(x), static_cast<float>(y), m_vpiano.vPianoData().whiteKey_w() - m_vpiano.vPianoData().whiteKey_shrink(), static_cast<float>(h) },
			noteColor,
			outlineColor,
			glowColor,
			&m_vpiano.vPianoRes().noteTexture,
			-m_vpiano.vPianoData().fallingWhiteNoteOutlineWidth,
			m_vpiano.vPianoData().fallingWhiteNoteBorderRadius
		});
	}

	for (auto& note : m_activeFallingNotes)
	{
		auto noteInfo = ostd::MidiParser::getNoteInfo(note.pitch);
		if (noteInfo.isWhiteKey()) continue;

		double h = note.duration * m_vpiano.vPianoData().pps();
		double totalTravelTime = m_vpiano.vPianoData().fallingTime_s + note.duration;
		double elapsedSinceSpawn = (currentTime - (note.startTime - m_vpiano.vPianoData().fallingTime_s));
		double progress = elapsedSinceSpawn / totalTravelTime;
		progress = std::clamp(progress, 0.0, 1.0);
		float y = -h + static_cast<float>(progress) * (m_vpiano.vPianoData().vpy() + h);
		float x = m_vpiano.vPianoData().keyOffsets()[noteInfo.keyIndex] + (m_vpiano.vPianoData().blackKey_shrink() / 2.0f);

		if (y >= m_vpiano.vPianoData().vpy())
		{
			auto& key = m_pianoKeys[noteInfo.keyIndex];
			key.pressed = false;
			key.pressedForce = { 0.0f, 0.0f };
			NoteEventData ned(key);
			ned.eventType = NoteEventData::eEventType::NoteOFF;
			ned.note = note;
			ostd::SignalHandler::emitSignal(SignalListener::NoteOffSignal, ostd::tSignalPriority::RealTime, ned);
		}
		else if (y + h >= m_vpiano.vPianoData().vpy())
		{
			auto& key = m_pianoKeys[noteInfo.keyIndex];
			key.pressed = true;
			key.pressedForce = l_calcPressedVelocity(note.velocity);
			NoteEventData ned(key);
			ned.eventType = NoteEventData::eEventType::NoteON;
			ned.note = note;
			ostd::SignalHandler::emitSignal(SignalListener::NoteOnSignal, ostd::tSignalPriority::RealTime, ned);
			if (!m_vpiano.m_firstNotePlayed)
			{
				ostd::SignalHandler::emitSignal(SignalListener::MidiStartSignal, ostd::tSignalPriority::RealTime, ned);
				m_vpiano.m_firstNotePlayed = true;
			}
		}
		ostd::Color noteColor = m_vpiano.vPianoData().fallingBlackNoteColor;
		ostd::Color outlineColor = m_vpiano.vPianoData().fallingBlackNoteOutlineColor;
		ostd::Color glowColor = m_vpiano.vPianoData().fallingBlackNoteGlowColor;
		if (m_vpiano.vPianoData().usePerNoteColors)
		{
			noteColor = m_vpiano.vPianoData().perNoteColors[noteInfo.noteInOctave];
			outlineColor = m_vpiano.vPianoData().perNoteColors[noteInfo.noteInOctave + 12];
			glowColor = m_vpiano.vPianoData().perNoteColors[noteInfo.noteInOctave + 24];
		}
		m_fallingNoteGfx_b.push_back(FallingNoteGraphicsData {
			{ static_cast<float>(x), static_cast<float>(y), m_vpiano.vPianoData().blackKey_w() - m_vpiano.vPianoData().blackKey_shrink(), static_cast<float>(h) },
			noteColor,
			outlineColor,
			glowColor,
			&m_vpiano.vPianoRes().noteTexture,
			-m_vpiano.vPianoData().fallingBlackNoteOutlineWidth,
			m_vpiano.vPianoData().fallingBlackNoteBorderRadius
		});
	}
}

void VirtualKeyboard::updateVisualization(double currentTime)
{
	// Remove notes that have ended
	while (!m_activeFallingNotes.empty() && currentTime > (m_activeFallingNotes.front().endTime + 0.05))
	{
		auto note = m_activeFallingNotes.front();
		auto info = ostd::MidiParser::getNoteInfo(note.pitch);
		m_pianoKeys[info.keyIndex].pressed = false;
		m_pianoKeys[info.keyIndex].pressedForce = { 0.0f, 0.0f };
		if (note.last)
		{
			auto& key = m_pianoKeys[info.keyIndex];
			NoteEventData ned(key);
			ned.eventType = NoteEventData::eEventType::MidiEnd;
			ned.note = note;
			ostd::SignalHandler::emitSignal(SignalListener::MidiEndSignal, ostd::tSignalPriority::RealTime, ned);
		}
		m_activeFallingNotes.pop_front();
	}

	// Add new notes that are starting now
	while (m_nextFallingNoteIndex < m_vpiano.vPianoRes().midiNotes.size() && currentTime >= m_vpiano.vPianoRes().midiNotes[m_nextFallingNoteIndex].startTime - m_vpiano.vPianoData().fallingTime_s)
	{
		// auto info = ostd::MidiParser::getNoteInfo(m_midiNotes[m_nextFallingNoteIndex].pitch);
		m_activeFallingNotes.push_back(m_vpiano.vPianoRes().midiNotes[m_nextFallingNoteIndex]);
		++m_nextFallingNoteIndex;
	}
	calculateFallingNotes(currentTime);
}

void VirtualKeyboard::renderKeyboard(std::optional<std::reference_wrapper<sf::RenderTarget>> target)
{
	sf::RenderTarget*  __target = nullptr;
	if (target)
		__target = &target->get();
	Renderer::setRenderTarget(__target);
	for (auto& pk : m_pianoKeys)
	{
		auto& tex = std::any_cast<sf::Texture&>(m_vpiano.vPianoRes().partTex);
		Renderer::useShader(&m_vpiano.vPianoRes().particleShader);
		m_vpiano.vPianoRes().particleShader.setUniform("u_texture", tex);
		Renderer::useTexture(&tex);
		Renderer::drawParticleSysten(pk.particles);
	}
	Renderer::useShader(nullptr);
	Renderer::useTexture(nullptr);
	auto& vpd = m_vpiano.vPianoData();
	int whiteKeyCount = 0;
	for (int midiNote = 21; midiNote <= 108; ++midiNote)
	{
		int noteInOctave = midiNote % 12;
		if (ostd::MidiParser::NoteInfo::isWhiteKey(noteInOctave)) // Draw white key
		{
			auto info = ostd::MidiParser::getNoteInfo(midiNote);
			PianoKey& pk = m_pianoKeys[info.keyIndex];
			ostd::Color keyColor = (pk.pressed ? vpd.whiteKeyPressedColor : vpd.whiteKeyColor);
			float x = vpd.vpx() + (whiteKeyCount * vpd.whiteKey_w());
			float y = vpd.vpy();
			pk.particles.setEmissionRect({ x + (vpd.whiteKey_w() / 2.0f) - (vpd.whiteKey_w() / 8.0f), y - 2.0f, vpd.whiteKey_w() / 4.0f, 2.0f });
			Renderer::outlineRect({ x, y, vpd.whiteKey_w(), vpd.whiteKey_h() }, keyColor, vpd.whiteKeySplitColor, 1 );
			whiteKeyCount++;
		}
	}
	Renderer::outlineRect({ vpd.vpx(), vpd.vpy() - 2, (float)m_vpiano.getParentWindow().getWindowWidth(), 2 }, vpd.pianoLineColor1, vpd.pianoLineColor1, 1);
	Renderer::outlineRect({ vpd.vpx(), vpd.vpy(), (float)m_vpiano.getParentWindow().getWindowWidth(), 5 }, vpd.pianoLineColor2, vpd.pianoLineColor2, 1);
	whiteKeyCount = 0;
	for (int midiNote = 21; midiNote <= 108; ++midiNote)
	{
		int noteInOctave = midiNote % 12;
		if (ostd::MidiParser::NoteInfo::isWhiteKey(noteInOctave))
		{
			whiteKeyCount++;
		}
		else // Draw black key
		{
			auto info = ostd::MidiParser::getNoteInfo(midiNote);
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
	// 	if (ostd::MidiParser::NoteInfo::isWhiteKey(noteInOctave))
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
	// 	if (ostd::MidiParser::NoteInfo::isWhiteKey(noteInOctave))
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

void VirtualKeyboard::renderFallingNotes(std::optional<std::reference_wrapper<sf::RenderTarget>> target)
{
	sf::RenderTarget*  __target = nullptr;
	if (target)
		__target = &target->get();
	Renderer::setRenderTarget(__target);
	__render_falling_notes(m_fallingNoteGfx_w);
	__render_falling_notes(m_fallingNoteGfx_b);
}

void VirtualKeyboard::renderFallingNotesGlow(std::optional<std::reference_wrapper<sf::RenderTarget>> target)
{
	sf::RenderTarget*  __target = nullptr;
	if (target)
		__target = &target->get();
	Renderer::setRenderTarget(__target);
	__render_falling_notes_glow(m_fallingNoteGfx_b);
	__render_falling_notes_glow(m_fallingNoteGfx_w);
}

void VirtualKeyboard::renderHollowNoteNegative(std::optional<std::reference_wrapper<sf::RenderTarget>> target)
{
	sf::RenderTarget*  __target = nullptr;
	if (target)
		__target = &target->get();
	Renderer::setRenderTarget(__target);
	__render_hollow_note_negatives(m_fallingNoteGfx_b);
	__render_hollow_note_negatives(m_fallingNoteGfx_w);
}

void VirtualKeyboard::__render_falling_notes(const std::vector<FallingNoteGraphicsData>& noteList)
{
	for (auto& note : noteList)
	{
		m_vpiano.m_vPianoRes.noteShader.setUniform("u_texture", m_vpiano.m_vPianoRes.noteTexture);
		m_vpiano.m_vPianoRes.noteShader.setUniform("u_color", color_to_glsl(note.fillColor));
		Renderer::useShader(&m_vpiano.vPianoRes().noteShader);
		Renderer::useTexture(note.texture);
		Renderer::setTextureRect({ m_vpiano.vPianoData().texCoordsPos.x, m_vpiano.vPianoData().texCoordsPos.y, (float)note.texture->getSize().x * m_vpiano.vPianoData().texCoordsScale.x, (float)note.texture->getSize().y * m_vpiano.vPianoData().texCoordsScale.y });
		Renderer::outlineRoundedRect(note.rect, note.fillColor, note.outlineColor, { note.cornerRadius, note.cornerRadius, note.cornerRadius, note.cornerRadius }, note.outlineThickness);
		Renderer::useTexture(nullptr);
		Renderer::useShader(nullptr);
		Renderer::drawRoundedRect(note.rect, note.outlineColor, { note.cornerRadius, note.cornerRadius, note.cornerRadius, note.cornerRadius }, note.outlineThickness);
	}
}

void VirtualKeyboard::__render_falling_notes_glow(const std::vector<FallingNoteGraphicsData>& noteList)
{
	for (auto& note : noteList)
	{
		Renderer::useTexture(nullptr);
		Renderer::useShader(nullptr);
		auto margins = m_vpiano.vPianoData().getGlowMargins();
		ostd::Rectangle bounds = {
			note.rect.x - margins.x,
			note.rect.y - margins.y,
			note.rect.w + margins.x + margins.w,
			note.rect.h + margins.y + margins.h
		};
		Renderer::fillRoundedRect(bounds, note.glowColor, { note.cornerRadius, note.cornerRadius, note.cornerRadius, note.cornerRadius });
	}
}

void VirtualKeyboard::__render_hollow_note_negatives(const std::vector<FallingNoteGraphicsData>& noteList)
{
	for (auto& note : noteList)
	{
		Renderer::useTexture(nullptr);
		Renderer::useShader(nullptr);
		auto margins = m_vpiano.vPianoData().getGlowMargins();
		margins.x = -2;
		margins.y = -2;
		margins.w = -2;
		margins.h = -2;
		ostd::Rectangle bounds = {
			note.rect.x - margins.x,
			note.rect.y - margins.y,
			note.rect.w + margins.x + margins.w,
			note.rect.h + margins.y + margins.h
		};
		Renderer::fillRoundedRect(bounds, { 0, 0, 0, 255 }, { 10, 10, 10, 10 });
	}
}
