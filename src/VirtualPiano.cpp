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
#include "Window.hpp"
#include "Renderer.hpp"
#include <ostd/Logger.hpp>

// Core functionality
void VirtualPiano::init(void)
{
	m_playing = false;
	m_paused = false;
	m_firstNotePlayed = false;
	m_vPianoRes.loadShaders();
	m_configJson.init("settings.json", true, &Common::DefaultSettingsJSON);
	m_vKeyboard.init();

	sf::Vector2u winSize = { m_parentWindow.sfWindow().getSize().x, m_parentWindow.sfWindow().getSize().y };
	sf::Vector2u winHalfSize = { winSize.x / 2, winSize.y / 2 };
	m_blurBuff1 = sf::RenderTexture(winHalfSize);
	m_blurBuff2 = sf::RenderTexture(winHalfSize);
	m_glowBuffer = sf::RenderTexture(winHalfSize);
	m_glowView.setSize({ (float)winSize.x, (float)winSize.y });
	m_glowView.setCenter({ winSize.x / 2.f, winSize.y / 2.f });
	m_glowBuffer.setView(m_glowView);
}

void VirtualPiano::loadProjectFile(const ostd::String& filePath)
{
	if (!m_projJson.init(filePath, false))
	{
		OX_ERROR("Invalid project file: %s", filePath.c_str());
		return;
	}

	enum class eDefaultPathType { Texture, Music, Style, Particle };
	auto resolveFilePath_l = [](const ostd::String& _path, eDefaultPathType pathType) -> ostd::String {
		ostd::String path = _path.new_trim();
		if (!path.startsWith("@/"))
			return path;
		path.substr(1);
		switch (pathType)
		{
			case eDefaultPathType::Music:
				path = "./music" + path;
			break;
			case eDefaultPathType::Particle:
				path = "./styles/particles" + path;
			break;
			case eDefaultPathType::Style:
				path = "./styles" + path;
			break;
			case eDefaultPathType::Texture:
				path = "./texture" + path;
			break;
			default: return path;
		}
		return path;
	};

	ostd::String tmp = resolveFilePath_l(m_projJson.get_string("project.graphics.styleFile"), eDefaultPathType::Style);
	if (!m_styleJson.init(tmp, false))
	{
		OX_ERROR("Invalid style file: %s", tmp.c_str());
		return;
	}
	tmp = resolveFilePath_l(m_projJson.get_string("project.particles.configFile"), eDefaultPathType::Particle);
	if (!m_partJson.init(tmp, false))
	{
		OX_ERROR("Invalid particle config file: %s", tmp.c_str());
		return;
	}
	m_showBackground = m_projJson.get_bool("project.useBackgroundImage");
	m_vPianoRes.loadBackgroundImage(resolveFilePath_l(m_projJson.get_string("project.graphics.backgroundImageFile"), eDefaultPathType::Texture));
	m_vPianoRes.loadParticleTexture(resolveFilePath_l(m_projJson.get_string("project.particles.texture.file"), eDefaultPathType::Texture), m_projJson.get_rect_array("project.particles.texture.tiles"));
	m_vPianoRes.loadNoteTexture(resolveFilePath_l(m_projJson.get_string("project.graphics.noteTextureFile"), eDefaultPathType::Texture));
	m_vPianoRes.loadMidiFile(resolveFilePath_l(m_projJson.get_string("project.audio.midiFile"), eDefaultPathType::Music));
	m_vPianoRes.loadAudioFile(resolveFilePath_l(m_projJson.get_string("project.audio.audioFile"), eDefaultPathType::Music));
	m_partPerFrame = m_projJson.get_int("project.particles.emitPerFrame");
	m_vPianoData.pressedVelocityMultiplier = m_projJson.get_float("project.particles.pressedVelocityMultiplier");

	m_vPianoData.loadFromStyleJSON(m_styleJson);
	m_vKeyboard.loadFromStyleJSON(m_partJson);
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
		ostd::Vec2 scale = { m_vPianoRes.backgroundOriginalSize.x / (float)width, m_vPianoRes.backgroundOriginalSize.y / (float)height };
		scale = { 1.0f / scale.x, 1.0f / scale.y };
		m_vPianoRes.backgroundSpr->setScale({ scale.x, scale.y });
	}
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
	m_vPianoRes.audioFile.pause();
	m_playing = false;
	m_paused = true;
}

void VirtualPiano::stop(void)
{
	m_vPianoRes.audioFile.stop();
	m_paused = false;
	m_firstNotePlayed = false;
	m_startTimeOffset_ns = Common::getCurrentTIme_ns();
	m_vKeyboard.m_nextFallingNoteIndex = 0;
	m_vKeyboard.m_activeFallingNotes.clear();
	for (auto& pk : m_vKeyboard.m_pianoKeys)
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




// Update and Render
void VirtualPiano::update(void)
{
	if (m_playing)
	{
		m_vKeyboard.updateVisualization(getPlayTime_s());
	}
	if (m_playing || m_videoRenderer.isRenderingToFile())
	{
		for (auto& pk : m_vKeyboard.m_pianoKeys)
		{
			pk.particles.update(pk.pressedForce);
			if (pk.pressedForce.y != 0)
				std::cout << pk.pressedForce << "\n";
			if (pk.pressed)
			{
				pk.particles.emit(m_partPerFrame);
			}
		}
	}
}

void VirtualPiano::render(std::optional<std::reference_wrapper<sf::RenderTarget>> target)
{
	if (m_videoRenderer.isRenderingToFile())
	{
		ostd::Vec2 render_scale = m_vPianoData.getScale();
		m_vPianoData.setScale(m_videoRenderer.m_videoRenderState.oldScale);
		Renderer::clear(m_vPianoData.backgroundColor);
		m_vKeyboard.render(std::nullopt);
		m_vPianoData.setScale(render_scale);
		if (m_videoRenderer.m_videoRenderState.mode == VideoRenderModes::ImageSequence)
		{
			m_videoRenderer.renderNextOutputFrame();
			if (m_videoRenderer.m_videoRenderState.isFinished())
				m_videoRenderer.finishOutputRender();
		}
		else if (m_videoRenderer.m_videoRenderState.mode == VideoRenderModes::Video)
		{
			m_videoRenderer.renderNextOutputFrame();
			if (m_videoRenderer.m_videoRenderState.isFinished())
				m_videoRenderer.finishOutputRender();
		}
	}
	else
	{
		renderFrame(std::nullopt);
	}
}

void VirtualPiano::renderFrame(std::optional<std::reference_wrapper<sf::RenderTarget>> target)
{
	sf::RenderTarget*  __target = nullptr;
	if (target)
		__target = &target->get();
	Renderer::setRenderTarget(&m_glowBuffer);
	Renderer::clear({ 0, 0, 0, 0 });
	for (const auto& note : m_vKeyboard.m_fallingNoteGfx_w)
	{
		m_vKeyboard.drawFallingNoteGlow(note);
	}
	for (auto& note : m_vKeyboard.m_fallingNoteGfx_b)
	{
		m_vKeyboard.drawFallingNoteGlow(note);
	}
	m_glowBuffer.display();
	Renderer::setRenderTarget(&m_blurBuff1);
	Renderer::clear({ 0, 0, 0, 0 });
	Renderer::useShader(&m_vPianoRes.blurShader);
	m_vPianoRes.blurShader.setUniform("texture", m_glowBuffer.getTexture());
	m_vPianoRes.blurShader.setUniform("direction", sf::Glsl::Vec2(1.f, 0.f));
	m_vPianoRes.blurShader.setUniform("resolution", (float)m_glowBuffer.getSize().x);
	m_vPianoRes.blurShader.setUniform("spread", 2.5f);
	m_vPianoRes.blurShader.setUniform("intensity", 1.0f);
	Renderer::drawTexture(m_glowBuffer.getTexture());
	m_blurBuff1.display();

	Renderer::setRenderTarget(&m_blurBuff2);
	Renderer::clear({ 0, 0, 0, 0 });
	m_vPianoRes.blurShader.setUniform("texture", m_blurBuff1.getTexture());
	m_vPianoRes.blurShader.setUniform("direction", sf::Glsl::Vec2(0.f, 1.f));
	m_vPianoRes.blurShader.setUniform("resolution", (float)m_blurBuff2.getSize().y);
	Renderer::drawTexture(m_blurBuff1.getTexture());
	m_blurBuff2.display();

	Renderer::setRenderTarget(__target);
	Renderer::useTexture(nullptr);
	Renderer::useShader(nullptr);
	Renderer::clear(m_vPianoData.backgroundColor);
	if (m_showBackground)
	{
		Renderer::drawSprite(*m_vPianoRes.backgroundSpr);
	}
	Renderer::useShader(&m_vPianoRes.particleShader);
	auto& tex = std::any_cast<sf::Texture&>(m_vPianoRes.partTex);
	m_vPianoRes.particleShader.setUniform("u_texture", tex);
	Renderer::useTexture(&tex);
	// Renderer::drawParticleSysten(m_snow);
	Renderer::useShader(nullptr);
	Renderer::useTexture(nullptr);
	Renderer::drawTexture(m_blurBuff2.getTexture(), { 0, 0 }, 2);

	for (const auto& note : m_vKeyboard.m_fallingNoteGfx_w)
	{
		m_vPianoRes.noteShader.setUniform("u_texture", m_vPianoRes.noteTexture);
		m_vPianoRes.noteShader.setUniform("u_color", color_to_glsl(note.fillColor));
		m_vKeyboard.drawFallingNote(note);
		m_vKeyboard.drawFallingNoteOutline(note);
	}
	for (auto& note : m_vKeyboard.m_fallingNoteGfx_b)
	{
		m_vPianoRes.noteShader.setUniform("u_texture", m_vPianoRes.noteTexture);
		m_vPianoRes.noteShader.setUniform("u_color", color_to_glsl(note.fillColor));
		m_vKeyboard.drawFallingNote(note);
		m_vKeyboard.drawFallingNoteOutline(note);
	}

	m_vKeyboard.render(target);
}
