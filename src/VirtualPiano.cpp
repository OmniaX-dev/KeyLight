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
#include "VPianoData.hpp"
#include "Window.hpp"
#include "Renderer.hpp"
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Texture.hpp>
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
	sf::Vector2u winHalfSize = { winSize.x / m_vPianoData.blur.resolutionDivider, winSize.y / m_vPianoData.blur.resolutionDivider };
	m_blurBuff1 = sf::RenderTexture(winSize);
	m_blurBuff2 = sf::RenderTexture(winSize);
	m_glowBuffer = sf::RenderTexture(winSize);
	m_hollowBuff = sf::RenderTexture(winSize);
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
	m_blurBuff1 = sf::RenderTexture({ width / m_vPianoData.blur.resolutionDivider, height / m_vPianoData.blur.resolutionDivider });
	m_blurBuff1 = sf::RenderTexture({ width / m_vPianoData.blur.resolutionDivider, height / m_vPianoData.blur.resolutionDivider });
	m_glowBuffer = sf::RenderTexture({ width / m_vPianoData.blur.resolutionDivider, height / m_vPianoData.blur.resolutionDivider });
	m_hollowBuff = sf::RenderTexture({ width / m_vPianoData.blur.resolutionDivider, height / m_vPianoData.blur.resolutionDivider });

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
		if (m_firstNotePlayed)
			m_vPianoRes.audioFile.play();
		return;
	}
	stop();
	m_playing = true;
}

void VirtualPiano::pause(void)
{
	m_vPianoRes.audioFile.pause();
	m_pausedTime_ns = Common::getCurrentTIme_ns();
	m_playing = false;
	m_paused = true;
}

void VirtualPiano::stop(void)
{
	m_vPianoRes.audioFile.stop();
	m_paused = false;
	m_firstNotePlayed = false;
	m_startTimeOffset_ns = Common::getCurrentTIme_ns();
	m_pausedTime_ns = 0.0;
	m_pausedOffset_ns = 0.0;
	m_vKeyboard.m_nextFallingNoteIndex = 0;
	m_vKeyboard.m_activeFallingNotes.clear();
	for (auto& pk : m_vKeyboard.m_pianoKeys)
	{
		pk.pressed = false;
		pk.particles.reset();
		pk.particles.update();
	}
	m_vKeyboard.updateVisualization(getPlayTime_s());
	m_playing = false;
}

double VirtualPiano::getPlayTime_s(void)
{
	double playTime = Common::getCurrentTIme_ns() - m_pausedOffset_ns - m_startTimeOffset_ns;
	return playTime * 1e-9;
}




// Update and Render
void VirtualPiano::update(void)
{
	if (m_paused)
	{
		m_pausedOffset_ns += (Common::getCurrentTIme_ns() - m_pausedTime_ns);
		m_pausedTime_ns = Common::getCurrentTIme_ns();
	}
	if (m_playing)
	{
		m_vKeyboard.updateVisualization(getPlayTime_s());
	}
	if (m_playing || m_videoRenderer.isRenderingToFile())
	{
		for (auto& pk : m_vKeyboard.m_pianoKeys)
		{
			pk.particles.update(pk.pressedForce);
			// if (pk.pressedForce.y != 0)
			// 	std::cout << pk.pressedForce << "\n";
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
		m_vKeyboard.renderKeyboard(std::nullopt);
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

	auto& blurBuffer = __apply_blur(m_vPianoData.blur.passes,
										 m_vPianoData.blur.bloomIntensity,
										 m_vPianoData.blur.startRadius,
										 m_vPianoData.blur.increment,
										 m_vPianoData.blur.threshold);

	m_hollowBuff.clear(sf::Color::Transparent);
    m_vKeyboard.renderHollowNoteNegative(m_hollowBuff);
    m_hollowBuff.display();

    Renderer::setRenderTarget(&blurBuffer);
    Renderer::useTexture(nullptr);
	Renderer::useShader(nullptr);
	Renderer::drawTexture(m_hollowBuff.getTexture(), { 0, 0 }, { (float)m_vPianoData.blur.resolutionDivider, (float)m_vPianoData.blur.resolutionDivider });  // Upscale

	Renderer::setRenderTarget(__target);
	Renderer::useTexture(nullptr);
	Renderer::useShader(nullptr);
	Renderer::clear(m_vPianoData.backgroundColor);
	if (m_showBackground)
		Renderer::drawSprite(*m_vPianoRes.backgroundSpr);

	sf::RenderStates glowState;
    glowState.blendMode = sf::BlendAdd;
    Renderer::useRenderStates(&glowState);
    Renderer::drawTexture(blurBuffer.getTexture(), { 0, 0 }, { (float)m_vPianoData.blur.resolutionDivider, (float)m_vPianoData.blur.resolutionDivider });  // Upscale
    Renderer::useRenderStates(nullptr);

    m_vKeyboard.renderFallingNotes(target);
    m_vKeyboard.renderKeyboard(target);
}

sf::RenderTexture& VirtualPiano::__apply_blur(uint8_t passes, float intensity, float start_offset, float increment, float threshold)
{
	switch (m_vPianoData.blur.type)
	{
		case VirtualPianoData::eBlurType::Gaussian:
			return __apply_gaussian_blur(passes, intensity, start_offset, increment, threshold);
		case VirtualPianoData::eBlurType::Kawase:
			return __apply_kawase_blur(passes, intensity, start_offset, increment, threshold);
		default: break;
	}
	OX_ERROR("Invalid blur type");
	return m_glowBuffer;
}

sf::RenderTexture& VirtualPiano::__apply_kawase_blur(uint8_t passes, float intensity, float start_offset, float increment, float threshold)
{
	m_glowBuffer.clear(sf::Color::Transparent);
    m_vKeyboard.renderFallingNotesGlow(m_glowBuffer);
    m_glowBuffer.display();

	m_blurBuff1.clear(sf::Color::Transparent);
	Renderer::setRenderTarget(&m_blurBuff1);
	Renderer::useShader(&m_vPianoRes.thresholdShader);
	m_vPianoRes.thresholdShader.setUniform("texture", m_glowBuffer.getTexture());
	m_vPianoRes.thresholdShader.setUniform("threshold", threshold);
	Renderer::drawTexture(m_glowBuffer.getTexture());
	m_blurBuff1.display();

	auto blurPass = [&](sf::RenderTexture& src, sf::RenderTexture& dst, sf::Shader& shader, float offset, float intensity) {
        shader.setUniform("texture", src.getTexture());
        shader.setUniform("resolution", sf::Vector2f({ 1.0f / dst.getSize().x, 1.0f / dst.getSize().y }));
        shader.setUniform("offset", offset);
        shader.setUniform("bloomStrength", intensity);

        dst.clear(sf::Color::Transparent);
        Renderer::setRenderTarget(&dst);
        Renderer::useShader(&shader);
        Renderer::drawTexture(src.getTexture());
        dst.display();
    };
    auto applyBlur = [&](sf::RenderTexture& buff1, sf::RenderTexture& buff2, uint8_t passes, float intensity, float increment) -> sf::RenderTexture& {
		bool toggle = true;
		float offset = start_offset;
		for (int32_t i = 0; i < passes; i++)
		{
			if (toggle) blurPass(buff1, buff2, m_vPianoRes.kawaseDownShader, offset, intensity);
			else blurPass(buff2, buff1, m_vPianoRes.kawaseDownShader, offset, intensity);
			toggle = !toggle;
			offset += increment;
		}
		auto& _buff1 = (toggle ? buff2 : buff1);
		auto& _buff2 = (toggle ? buff1 : buff2);
		toggle = true;
		offset = start_offset;
		for (int32_t i = 0; i < passes; i++)
		{
			if (toggle) blurPass(_buff1, _buff2, m_vPianoRes.kawaseUpShader, offset, intensity);
			else blurPass(_buff2, _buff1, m_vPianoRes.kawaseUpShader, offset, intensity);
			toggle = !toggle;
			offset += increment;
		}
		return (toggle ? buff2 : buff1);
    };

    auto& finalBlurBuff = applyBlur(m_blurBuff1, m_blurBuff2, passes, intensity, increment);
    Renderer::setRenderTarget(nullptr);
    Renderer::useShader(nullptr);
    Renderer::useTexture(nullptr);
    return finalBlurBuff;
}

sf::RenderTexture& VirtualPiano::__apply_gaussian_blur(uint8_t passes, float intensity, float start_radius, float increment, float threshold)
{
	m_glowBuffer.clear(sf::Color::Transparent);
    m_vKeyboard.renderFallingNotesGlow(m_glowBuffer);
    m_glowBuffer.display();

	m_blurBuff1.clear(sf::Color::Transparent);
	Renderer::setRenderTarget(&m_blurBuff1);
	Renderer::useShader(&m_vPianoRes.thresholdShader);
	m_vPianoRes.thresholdShader.setUniform("texture", m_glowBuffer.getTexture());
	m_vPianoRes.thresholdShader.setUniform("threshold", threshold);
	Renderer::drawTexture(m_glowBuffer.getTexture());
	m_blurBuff1.display();

	auto blurPass = [&](sf::RenderTexture& src, sf::RenderTexture& dst, bool horizontal, float radius, float intensity) {
        m_vPianoRes.gaussianBlurShader.setUniform("texture", src.getTexture());
        m_vPianoRes.gaussianBlurShader.setUniform("direction", horizontal ? sf::Glsl::Vec2(1.0f, 0.0f) : sf::Glsl::Vec2(0.0f, 1.0f));
        m_vPianoRes.gaussianBlurShader.setUniform("resolution", (horizontal ? (float)dst.getSize().x : (float)dst.getSize().y));
        m_vPianoRes.gaussianBlurShader.setUniform("radius", radius);
        m_vPianoRes.gaussianBlurShader.setUniform("bloomIntensity", intensity);

        dst.clear(sf::Color::Transparent);
        Renderer::setRenderTarget(&dst);
        Renderer::useShader(&m_vPianoRes.gaussianBlurShader);
        Renderer::drawTexture(src.getTexture());
        dst.display();
    };
    auto applyBlur = [&](sf::RenderTexture& buff1, sf::RenderTexture& buff2, uint8_t passes, float radius, float intensity, float increment) -> sf::RenderTexture& {
		float offset = radius;
		for (int32_t i = 0; i < passes; i++)
		{
			blurPass(buff1, buff2, true, offset, intensity);
			blurPass(buff2, buff1, false, offset, intensity);
			offset += increment;
		}
		return buff1;
    };
    auto& finalBlurBuff = applyBlur(m_blurBuff1, m_blurBuff2, passes, start_radius, intensity, increment);
    Renderer::setRenderTarget(nullptr);
    Renderer::useShader(nullptr);
    Renderer::useTexture(nullptr);
    return finalBlurBuff;
}
