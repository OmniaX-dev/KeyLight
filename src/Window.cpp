#include "Window.hpp"
#include <ostd/Logger.hpp>
#include "Common.hpp"

void Window::onInitialize(void)	
{
	enableSignals();
	connectSignal(ostd::tBuiltinSignals::KeyReleased);
	connectSignal(VirtualPiano::NoteOnSignal);
	connectSignal(VirtualPiano::NoteOffSignal);
	connectSignal(ostd::tBuiltinSignals::WindowResized);
	m_window.setPosition({ 30, 30 });
	m_windowSizeBeforeFullscreen = { (float)getWindowWidth(), (float)getWindowHeight() };
	enableFullscreen(true);
	m_vpiano.init();
	m_vpiano.loadMidiFile("res/midi/testMidiFile3.mid");
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
}

void Window::onRender(void)
{
	m_vpiano.renderFallingNotes();
	m_vpiano.renderVirtualKeyboard();
	ostd::String fps_text = "FPS: ";
	fps_text.add(getFPS());
	// m_gfx.drawString(fps_text, { 10, 10 }, { 220, 170, 0 }, 16);
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
	if (enable)
	{
		m_windowSizeBeforeFullscreen = { (float)old_size.x, (float)old_size.y };
		m_window.create(sf::VideoMode::getDesktopMode(), getTitle().cpp_str(), sf::Style::None);
		m_isFullscreen = true;
	}
	else
	{
		m_window.create(sf::VideoMode({ (uint32_t)m_windowSizeBeforeFullscreen.x, (uint32_t)m_windowSizeBeforeFullscreen.y }), getTitle().cpp_str(), sf::Style::Default);
		m_isFullscreen = false;
	}
	auto new_size = m_window.getSize();
	WindowResizedData wrd(*this, old_size.x, old_size.y, new_size.x, new_size.y);
	ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::WindowResized, ostd::tSignalPriority::RealTime, wrd);
}

void Window::outlinedRect(const ostd::Rectangle& rect, const ostd::Color& fillColor, const ostd::Color& outlineColor, int32_t outlineThickness)
{
	m_sf_rect.setSize({ rect.w, rect.h });
	m_sf_rect.setPosition({ rect.x, rect.y });
	m_sf_rect.setFillColor(sf_color(fillColor));
	m_sf_rect.setOutlineColor(sf_color(outlineColor));
	m_sf_rect.setOutlineThickness(outlineThickness);
	m_window.draw(m_sf_rect);
}