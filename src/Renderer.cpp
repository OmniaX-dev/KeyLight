#include "Renderer.hpp"
#include <ostd/Logger.hpp>
#include "Common.hpp"

bool Renderer::init(WindowBase& window, const ostd::String& fontFilePath = "")
{
	m_window = &window;
	if (fontFilePath.new_trim() != "")
	{
		if (!m_font.openFromFile(fontFilePath.cpp_str()))
		{
			OX_ERROR("Invalid font file: %s", fontFilePath.c_str());
			return false;
		}
	}
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
}

void Renderer::setTextureRect(ostd::Rectangle textureRect)
{
	if (textureRect.x == 0 && textureRect.y == 0 && textureRect.w == 0 && textureRect.h)
	{
		if (m_texture == nullptr) return;
		m_textureRect = sf_intRect(textureRect);
	}
		m_textureRect = sf_intRect(textureRect);
}

void Renderer::drawString(const ostd::String& str, const ostd::Vec2& position, const ostd::Color& color, uint32_t font_size)
{
	if (m_text == nullptr) return;
	if (m_window == nullptr) return;
	m_text->setFont(m_font);
	m_text->setCharacterSize(font_size);
	m_text->setFillColor(sf_color(color));
	m_text->setPosition({ position.x, position.y });
	m_text->setString(str.cpp_str());
	__draw_call(m_text);
}

void Renderer::drawRect(const ostd::Rectangle& rect, const ostd::Color& outlineColor, int32_t outlineThickness = -1)
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
	__draw_call(&m_rect);
}

void Renderer::outlineRect(const ostd::Rectangle& rect, const ostd::Color& fillColor, const ostd::Color& outlineColor, int32_t outlineThickness = -1)
{
	if (m_window == nullptr) return;
	m_rect.setSize({ rect.w, rect.h });
	m_rect.setPosition({ rect.x, rect.y });
	m_rect.setFillColor(sf_color(fillColor));
	m_rect.setOutlineColor(sf_color(outlineColor));
	m_rect.setOutlineThickness(outlineThickness);
	__draw_call(&m_rect);
}

void Renderer::drawRoundedRect(const ostd::Rectangle& rect, const ostd::Color& outlineColor, const ostd::Rectangle& radius, int32_t outlineThickness = -1)
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

void Renderer::fillRoundedRect(const ostd::Rectangle& rect, const ostd::Color& fillColor, const ostd::Rectangle& radius, int32_t outlineThickness = -1)
{
	if (m_window == nullptr) return;
	m_roundedRect = { {rect.w, rect.h}, radius.x, radius.y, radius.w, radius.h };
	m_roundedRect.setPosition({ rect.x, rect.y });
	m_roundedRect.setCornerPointCount(m_roundedRectCornerResolution);
	m_roundedRect.setFillColor(sf_color(fillColor));
	m_roundedRect.setOutlineColor({ 0, 0, 0, 0 });
	m_roundedRect.setOutlineThickness(0);
}

void Renderer::outlineRoundedRect(const ostd::Rectangle& rect, const ostd::Color& fillColor, const ostd::Color& outlineColor, const ostd::Rectangle& radius, int32_t outlineThickness = -1)
{
	if (m_window == nullptr) return;
	m_roundedRect = { {rect.w, rect.h}, radius.x, radius.y, radius.w, radius.h };
	m_roundedRect.setPosition({ rect.x, rect.y });
	m_roundedRect.setCornerPointCount(m_roundedRectCornerResolution);
	m_roundedRect.setFillColor(sf_color(fillColor));
	m_roundedRect.setOutlineColor(sf_color(outlineColor));
	m_roundedRect.setOutlineThickness(outlineThickness);
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