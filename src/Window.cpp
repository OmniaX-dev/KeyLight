#include "Window.hpp"
#include <ostd/Logger.hpp>
#include "Common.hpp"

void Window::onInitialize(void)	
{
	enableSignals();
	connectSignal(ostd::tBuiltinSignals::KeyReleased);
	connectSignal(VirtualPiano::NoteOnSignal);
	connectSignal(VirtualPiano::NoteOffSignal);
	connectSignal(VirtualPiano::MidiStartSignal);
	connectSignal(ostd::tBuiltinSignals::WindowResized);
	setClearColor({ 255, 10, 10 });
	m_window.setPosition({ 30, 30 });
	m_windowSizeBeforeFullscreen = { (float)getWindowWidth(), (float)getWindowHeight() };
	m_windowPositionBeforeFullscreen = { (float)m_window.getPosition().x, (float)m_window.getPosition().y };
	enableFullscreen(true);
	m_vpiano.init();
	m_vpiano.loadMidiFile("res/midi/noct20.mid");
	m_vpiano.loadAudioFile("res/music/noct20.mp3");
	if (!m_font.openFromFile("res/ttf/Courier Prime.ttf"))
		OX_ERROR("Invalid font file:");
}
	
void Window::handleSignal(ostd::tSignal& signal)
{
	if (signal.ID == ostd::tBuiltinSignals::KeyReleased)
	{
		auto& evtData = (KeyEventData&)signal.userData;
		if (evtData.keyCode == (int32_t)sf::Keyboard::Key::Escape)
			close();
		if (evtData.keyCode == (int32_t)sf::Keyboard::Key::Space)
		{
			if (!m_vpiano.isPlaying())
				m_vpiano.play();
			else
				m_vpiano.pause();
		}
		else if (evtData.keyCode == (int32_t)sf::Keyboard::Key::Enter)
		{
			m_vpiano.stop();
		}
		else if (evtData.keyCode == (int32_t)sf::Keyboard::Key::F11)
		{
			enableFullscreen(!m_isFullscreen);
		}
	}
	else if (signal.ID == ostd::tBuiltinSignals::WindowResized)
	{
		auto& evtData = (WindowResizedData&)signal.userData;
		sf::View view = m_window.getView();
		view.setSize({ static_cast<float>(evtData.new_width), static_cast<float>(evtData.new_height) });
		view.setCenter({ evtData.new_width / 2.f, evtData.new_height / 2.f });
		m_window.setView(view);
		m_vpiano.vPianoData().updateScale(evtData.new_width, evtData.new_height);
	}
	else if (signal.ID == VirtualPiano::MidiStartSignal)
	{
		if (m_vpiano.hasAudioFile())
		{
			m_vpiano.getAudioFile().play();
			m_vpiano.getAudioFile().setPlayingOffset(sf::seconds(m_vpiano.getAutoSoundStart()));
		}
	}
}

void Window::onRender(void)
{
	m_window.clear({ 10, 10, 10 });
	// m_currentShader = &m_vpiano.noteShader;
	m_vpiano.renderFallingNotes();
	m_currentShader = nullptr;
	m_vpiano.renderVirtualKeyboard();
	ostd::String fps_text = "FPS: ";
	fps_text.add(getFPS());
	drawString(fps_text, { 10, 10 }, { 220, 170, 0 }, 24);
}

void Window::onFixedUpdate(void)
{
}

void Window::onUpdate(void)
{
	m_vpiano.update();
}

void Window::enableFullscreen(bool enable)
{
	auto old_size = m_window.getSize();
	auto old_position = m_window.getPosition();
	if (enable)
	{
		m_windowSizeBeforeFullscreen = { (float)old_size.x, (float)old_size.y };
		m_windowPositionBeforeFullscreen = { (float)old_position.x, (float)old_position.y };
		m_window.create(sf::VideoMode::getDesktopMode(), getTitle().cpp_str(), sf::Style::None, sf::State::Fullscreen);
		m_window.setPosition({ 0, 0 });
		m_isFullscreen = true;
	}
	else
	{
		m_window.create(sf::VideoMode({ (uint32_t)m_windowSizeBeforeFullscreen.x, (uint32_t)m_windowSizeBeforeFullscreen.y }), getTitle().cpp_str(), sf::Style::Default);
		m_window.setPosition({ (int32_t)m_windowPositionBeforeFullscreen.x, (int32_t)m_windowPositionBeforeFullscreen.y });
		m_isFullscreen = false;
	}
	auto new_size = m_window.getSize();
	WindowResizedData wrd(*this, old_size.x, old_size.y, new_size.x, new_size.y);
	ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::WindowResized, ostd::tSignalPriority::RealTime, wrd);
}

void Window::drawString(const ostd::String& str, const ostd::Vec2& position, const ostd::Color& color, uint32_t font_size)
{
	m_sf_text.setFont(m_font);
	m_sf_text.setCharacterSize(font_size);
	m_sf_text.setFillColor(sf_color(color));
	m_sf_text.setPosition({ position.x, position.y });
	m_sf_text.setString(str.cpp_str());
	m_window.draw(m_sf_text);
}

void Window::outlinedRect(const ostd::Rectangle& rect, const ostd::Color& fillColor, const ostd::Color& outlineColor, int32_t outlineThickness)
{
	m_sf_rect.setSize({ rect.w, rect.h });
	m_sf_rect.setPosition({ rect.x, rect.y });
	m_sf_rect.setFillColor(sf_color(fillColor));
	m_sf_rect.setOutlineColor(sf_color(outlineColor));
	m_sf_rect.setOutlineThickness(outlineThickness);
	if (m_currentShader == nullptr)
		m_window.draw(m_sf_rect);
	else
		m_window.draw(m_sf_rect, m_currentShader);
}

void Window::outlinedRoundedRect(const ostd::Rectangle& rect, const ostd::Color& fillColor, const ostd::Color& outlineColor, const ostd::Rectangle& radius, int32_t outlineThickness)
{
	m_sf_roundedRect = { {rect.w, rect.h}, radius.x, radius.y, radius.w, radius.h };
	m_sf_roundedRect.setPosition({ rect.x, rect.y });
	m_sf_roundedRect.setCornerPointCount(12); // smoothness of corners
	m_sf_roundedRect.setFillColor(sf_color(fillColor));
	m_sf_roundedRect.setOutlineColor(sf_color(outlineColor));
	m_sf_roundedRect.setOutlineThickness(outlineThickness);
	if (m_currentShader == nullptr)
		m_window.draw(m_sf_roundedRect);
	else
		m_window.draw(m_sf_roundedRect, m_currentShader);
}