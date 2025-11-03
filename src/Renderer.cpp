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

#include "Renderer.hpp"
#include <ostd/Logger.hpp>
#include "Common.hpp"

bool Renderer::init(WindowBase& window, const ostd::String& fontFilePath)
{
	m_window = nullptr;
	if (fontFilePath.new_trim() != "")
	{
		if (!m_font.openFromFile(fontFilePath.cpp_str()))
		{
			OX_ERROR("Invalid font file: %s", fontFilePath.c_str());
			return false;
		}
	}
	m_window = &window;
	return true;
}

void Renderer::setRenderTarget(sf::RenderTarget* target)
{
	m_target = target;
}

void Renderer::useShader(sf::Shader* shader)
{
	m_shader  = shader;
}

void Renderer::useFont(const ostd::String& fontFilePath)
{
	if (!m_font.openFromFile(fontFilePath.cpp_str()))
	{
		OX_ERROR("Invalid font file: %s", fontFilePath.c_str());
	}
}

void Renderer::useTexture(sf::Texture* texture, ostd::Rectangle textureRect)
{
	m_texture = texture;
	setTextureRect(textureRect);
}

void Renderer::setTextureRect(ostd::Rectangle textureRect)
{
	if (textureRect.x == 0 && textureRect.y == 0 && textureRect.w == 0 && textureRect.h == 0)
	{
		if (m_texture == nullptr) return;
		m_textureRect = sf::IntRect({{0, 0}, { (int)m_texture->getSize().x, (int)m_texture->getSize().x } });
		return;
	}
	m_textureRect = sf_intRect(textureRect);
}

void Renderer::clear(const ostd::Color& color)
{
	if (m_window == nullptr) return;
	sf::RenderTarget& target = (m_target == nullptr ? m_window->sfWindow() : *m_target);
	target.clear(sf_color(color));
}

ostd::Vec2 Renderer::getStringSize(const ostd::String& str,  uint32_t font_size)
{
	if (m_window == nullptr) return { 0.0f, 0.0f };
	ostd::Vec2 result; // or allocate however your API expects

    sf::Text text(m_font);
    text.setString(str.c_str());
    text.setCharacterSize(font_size);
    sf::FloatRect bounds = text.getLocalBounds();
    return { bounds.size.x, bounds.size.y };
}

void Renderer::drawString(const ostd::String& str, const ostd::Vec2& position, const ostd::Color& color, uint32_t font_size)
{
	if (m_text == nullptr)
	{
		if (m_textCreated) return;
		m_text = new sf::Text(m_font);
		m_textCreated = true;
	}
	if (m_window == nullptr) return;
	m_text->setFont(m_font);
	m_text->setCharacterSize(font_size);
	m_text->setFillColor(sf_color(color));
	m_text->setPosition({ position.x, position.y });
	m_text->setString(str.cpp_str());
	__draw_call(m_text);
}

void Renderer::drawTexture(const sf::Texture& texture, const ostd::Vec2& position, const ostd::Vec2& scale, const ostd::Color& tint)
{
	sf::Sprite spr(texture);
	spr.setPosition({ position.x, position.y });
	spr.setScale({ scale.x, scale.y });
	spr.setColor(sf_color(tint));
	__draw_call(&spr);
}

void Renderer::drawRect(const ostd::Rectangle& rect, const ostd::Color& outlineColor, int32_t outlineThickness)
{
	if (m_window == nullptr) return;
	m_rect.setSize({ rect.w, rect.h });
	m_rect.setPosition({ rect.x, rect.y });
	m_rect.setFillColor({ 0 , 0, 0, 0 });
	m_rect.setOutlineColor(sf_color(outlineColor));
	m_rect.setOutlineThickness(outlineThickness);
	__draw_call(&m_rect);
}

void Renderer::fillRect(const ostd::Rectangle& rect, const ostd::Color& fillColor)
{
	if (m_window == nullptr) return;
	m_rect.setSize({ rect.w, rect.h });
	m_rect.setPosition({ rect.x, rect.y });
	m_rect.setFillColor(sf_color(fillColor));
	m_rect.setOutlineColor({ 0 , 0, 0, 0 });
	m_rect.setOutlineThickness(0);
	m_rect.setTexture(nullptr);
	if (m_texture != nullptr)
	{
		m_rect.setTexture(m_texture);
		m_rect.setTextureRect(m_textureRect);
	}
	__draw_call(&m_rect);
}

void Renderer::outlineRect(const ostd::Rectangle& rect, const ostd::Color& fillColor, const ostd::Color& outlineColor, int32_t outlineThickness)
{
	if (m_window == nullptr) return;
	m_rect.setSize({ rect.w, rect.h });
	m_rect.setPosition({ rect.x, rect.y });
	m_rect.setFillColor(sf_color(fillColor));
	m_rect.setOutlineColor(sf_color(outlineColor));
	m_rect.setOutlineThickness(outlineThickness);
	m_rect.setTexture(nullptr);
	if (m_texture != nullptr)
	{
		m_rect.setTexture(m_texture);
		m_rect.setTextureRect(m_textureRect);
	}
	__draw_call(&m_rect);
}

void Renderer::drawRoundedRect(const ostd::Rectangle& rect, const ostd::Color& outlineColor, const ostd::Rectangle& radius, int32_t outlineThickness)
{
	if (m_window == nullptr) return;
	m_roundedRect = { {rect.w, rect.h}, radius.x, radius.y, radius.w, radius.h };
	m_roundedRect.setPosition({ rect.x, rect.y });
	m_roundedRect.setCornerPointCount(m_roundedRectCornerResolution);
	m_roundedRect.setFillColor({ 0, 0, 0, 0 });
	m_roundedRect.setOutlineColor(sf_color(outlineColor));
	m_roundedRect.setOutlineThickness(outlineThickness);
	__draw_call(&m_roundedRect);
}

void Renderer::fillRoundedRect(const ostd::Rectangle& rect, const ostd::Color& fillColor, const ostd::Rectangle& radius)
{
	if (m_window == nullptr) return;
	m_roundedRect = { {rect.w, rect.h}, radius.x, radius.y, radius.w, radius.h };
	m_roundedRect.setPosition({ rect.x, rect.y });
	m_roundedRect.setCornerPointCount(m_roundedRectCornerResolution);
	m_roundedRect.setFillColor(sf_color(fillColor));
	m_roundedRect.setOutlineColor({ 0, 0, 0, 0 });
	m_roundedRect.setOutlineThickness(0);
	m_roundedRect.setTexture(nullptr);
	if (m_texture != nullptr)
	{
		m_roundedRect.setTexture(m_texture);
		m_roundedRect.setTextureRect(m_textureRect);
	}
	__draw_call(&m_roundedRect);
}

void Renderer::outlineRoundedRect(const ostd::Rectangle& rect, const ostd::Color& fillColor, const ostd::Color& outlineColor, const ostd::Rectangle& radius, int32_t outlineThickness)
{
	if (m_window == nullptr) return;
	m_roundedRect = { {rect.w, rect.h}, radius.x, radius.y, radius.w, radius.h };
	m_roundedRect.setPosition({ rect.x, rect.y });
	m_roundedRect.setCornerPointCount(m_roundedRectCornerResolution);
	m_roundedRect.setFillColor(sf_color(fillColor));
	m_roundedRect.setOutlineColor(sf_color(outlineColor));
	m_roundedRect.setOutlineThickness(outlineThickness);
	m_roundedRect.setTexture(nullptr);
	if (m_texture != nullptr)
	{
		m_roundedRect.setTexture(m_texture);
		m_roundedRect.setTextureRect(m_textureRect);
	}
	__draw_call(&m_roundedRect);
}

void Renderer::__draw_call(sf::Drawable* obj)
{
	if (m_window == nullptr) return;
	if (obj == nullptr) return;
	sf::RenderTarget& target = (m_target == nullptr ? m_window->sfWindow() : *m_target);
	if (m_shader == nullptr)
		target.draw(*obj);
	else
		target.draw(*obj, m_shader);
}
