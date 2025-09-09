#include "SFMLWindow.hpp"
#include <ostd/Logger.hpp>
#include <ogfx/SDLInclude.hpp>

WindowBase::~WindowBase(void)
{
	onDestroy();
}

void WindowBase::initialize(int32_t width, int32_t height, const ostd::String& windowTitle)
{
	if (m_initialized) return;
	m_windowWidth = width;
	m_windowHeight = height;
	m_title = windowTitle;
	m_window.create(sf::VideoMode({ static_cast<uint32_t>(width), static_cast<uint32_t>(height) }), windowTitle.cpp_str());
	m_initialized = true;
	m_running = true;

	if (SDL_Init(SDL_INIT_VIDEO) != 0)	
	{
		printf( "SDL could not initialize! Error: %s\n", SDL_GetError() );
		exit(1);
	}
	// int imgFlags = IMG_INIT_PNG;
	// if (!(IMG_Init(imgFlags) & imgFlags))
	// {
	// 	printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
	// 	exit(2);
	// }
	m_sdl_window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_windowWidth, m_windowHeight, SDL_WINDOW_HIDDEN);
		

	setTypeName("dragon::WindowBase");
	enableSignals(true);
	validate();

	onInitialize();
}

void WindowBase::update(void)
{
	if (!m_initialized) return;
	int64_t start = m_clock.getElapsedTime().asMicroseconds();
	handleEvents();
	if (m_refreshScreen)
		m_window.clear({ m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a });
	onUpdate();
	onRender();
	m_window.display();
	int64_t end = m_clock.getElapsedTime().asMicroseconds();
	double elapsed = (end - start) / (double)1'000'000.0;
	m_redrawAccumulator += elapsed;
	if (m_redrawAccumulator >= 0.2f)
	{
		onFixedUpdate();
		m_redrawAccumulator = 0.0f;
	}
	end = m_clock.getElapsedTime().asMicroseconds();
	elapsed = (end - start) / (double)1'000'000.0;
	m_timeAccumulator += elapsed;
	if (m_timeAccumulator >= 0.5f)
	{
		onSlowUpdate();
		m_fps = (int32_t)(1.0f / elapsed);
		m_timeAccumulator = 0.0f;
	}
}

void WindowBase::setSize(int32_t width, int32_t height)
{
	if (!isInitialized()) return;
	m_window.setSize({ static_cast<uint32_t>(width), static_cast<uint32_t>(height) });
	ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::WindowResized, ostd::tSignalPriority::RealTime);
}

void WindowBase::syncWindowSize(void)
{
	m_windowWidth = m_window.getSize().x;
	m_windowHeight = m_window.getSize().y;
}

void WindowBase::setTitle(const ostd::String& title)
{
	if (!isInitialized()) return;
	m_title = title;
	m_window.setTitle(m_title.cpp_str());
}

void WindowBase::handleEvents(void)
{
	if (!m_initialized) return;
	auto l_getMouseState = [this](void) -> MouseEventData {
		int32_t mx = 0, my = 0;
		sf::Vector2i pos = sf::Mouse::getPosition(m_window);
		bool leftPressed   = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
		bool rightPressed  = sf::Mouse::isButtonPressed(sf::Mouse::Button::Right);
		bool middlePressed  = sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle);
		MouseEventData::eButton button = MouseEventData::eButton::_None;
		if (middlePressed) button = MouseEventData::eButton::Middle;
		if (leftPressed) button = MouseEventData::eButton::Left;
		if (rightPressed) button = MouseEventData::eButton::Right;
		MouseEventData mmd(*this, mx, my, button);
		return mmd;
	};
	while (const std::optional event = m_window.pollEvent())
	{
		// Close window: exit
		if (event->is<sf::Event::Closed>())
		{
			m_running = false;
			ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::WindowClosed, ostd::tSignalPriority::Normal, *this);
		}
		else if (event->is<sf::Event::Resized>())
		{
			const auto* resized = event->getIf<sf::Event::Resized>();
			WindowResizedData wrd(*this, m_windowWidth, m_windowHeight, 0, 0);
			// OX_INFO("OLD SIZE: %d,%d", m_windowWidth, m_windowHeight);
			m_windowWidth = resized->size.x;
			m_windowHeight = resized->size.y;
			// OX_INFO("NEW SIZE: %d,%d\n", m_windowWidth, m_windowHeight);
			wrd.new_width = m_windowWidth;
			wrd.new_height = m_windowHeight;
			ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::WindowResized, ostd::tSignalPriority::RealTime, wrd);
		}
		else if (event->is<sf::Event::MouseMoved>())
		{
			MouseEventData mmd = l_getMouseState();
			if (isMouseDragEventEnabled() && mmd.button != MouseEventData::eButton::_None)
				ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::MouseDragged, ostd::tSignalPriority::RealTime, mmd);
			else
				ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::MouseMoved, ostd::tSignalPriority::RealTime, mmd);
		}
		else if (event->is<sf::Event::MouseButtonPressed>())
		{
			MouseEventData mmd = l_getMouseState();
			ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::MousePressed, ostd::tSignalPriority::RealTime, mmd);
		}
		else if (event->is<sf::Event::MouseButtonReleased>())
		{
			MouseEventData mmd = l_getMouseState();
			ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::MouseReleased, ostd::tSignalPriority::RealTime, mmd);
		}
		else if (event->is<sf::Event::KeyPressed>())
		{
			const auto* keyPressed = event->getIf<sf::Event::KeyPressed>();
			KeyEventData ked(*this, (int32_t)keyPressed->code, 0, KeyEventData::eKeyEvent::Pressed);
			ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::KeyPressed, ostd::tSignalPriority::RealTime, ked);
		}
		else if (event->is<sf::Event::KeyReleased>())
		{
			const auto* keyReleased = event->getIf<sf::Event::KeyReleased>();
			KeyEventData ked(*this, (int32_t)keyReleased->code, 0, KeyEventData::eKeyEvent::Released);
			ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::KeyReleased, ostd::tSignalPriority::RealTime, ked);
		}
		else if (event->is<sf::Event::TextEntered>())
		{
			const auto* textEntered = event->getIf<sf::Event::TextEntered>();
			char32_t codepoint = textEntered->unicode;
			KeyEventData ked(*this, 0, static_cast<char>(codepoint), KeyEventData::eKeyEvent::Text);
			ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::TextEntered, ostd::tSignalPriority::RealTime, ked);
		}
	}
}