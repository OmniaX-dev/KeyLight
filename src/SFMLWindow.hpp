#pragma once

#include <ostd/BaseObject.hpp>
#include <ostd/Signals.hpp>
#include <ostd/IOHandlers.hpp>

#include <SFML/Graphics.hpp>
#include <ogfx/SDLInclude.hpp>

class WindowBase : public ostd::BaseObject
{
	public: enum class eCursor { Arrow = 0, IBeam };
	public:
		inline WindowBase(void) {  }
		~WindowBase(void);
		inline WindowBase(int32_t width, int32_t height, const ostd::String& windowTitle) { initialize(width, height, windowTitle); }
		void initialize(int32_t width, int32_t height, const ostd::String& windowTitle);

		void update(void);
		void setSize(int32_t width, int32_t height);
		void syncWindowSize(void);
		void setTitle(const ostd::String& title);

		inline virtual void onRender(void) {  }
		inline virtual void onUpdate(void) {  }
		inline virtual void onFixedUpdate(void) {  }
		inline virtual void onSlowUpdate(void) {  }
		inline virtual void onInitialize(void) {  }
		inline virtual void onDestroy(void) {  }

		inline bool isInitialized(void) const { return m_initialized; }
		inline bool isRunning(void) const { return m_running; }
		inline void close(void) { m_running = false; }
		inline void hide(void) { m_window.setVisible(false); }
		inline void show(void) { m_window.setVisible(true); }
		inline ostd::String getTitle(void) const { return m_title; }
		inline int32_t getFPS(void) const { return m_fps; }
		inline int32_t getWindowWidth(void) const { return m_windowWidth; }
		inline int32_t getWindowHeight(void) const { return m_windowHeight; }
		inline bool isMouseDragEventEnabled(void) const { return m_deagEventEnabled; }
		inline void enableMouseDragEvent(bool enable = true) { m_deagEventEnabled = enable; }
		inline ostd::Color getClearColor(void) const { return m_clearColor; }
		inline void setClearColor(const ostd::Color& color) { m_clearColor = color; }
	private:
		void handleEvents(void);

	protected:
		ostd::ConsoleOutputHandler out;

		sf::RenderWindow m_window;
		bool m_refreshScreen { true };

	private:
		int32_t m_windowWidth { 0 };
		int32_t m_windowHeight { 0 };
		ostd::String m_title { "" };
		int32_t m_fps { 0 };

		sf::Clock m_clock;

		ostd::Color m_clearColor { 10, 10, 10, 255 };

		float m_timeAccumulator { 0.0f };
		float m_redrawAccumulator { 0.0f };

		bool m_deagEventEnabled { false };
		bool m_running { false };
		bool m_initialized { false };
};
class WindowResizedData : public ostd::BaseObject
{
	public:
		inline WindowResizedData(WindowBase& parent, int32_t _oldx, int32_t _oldy, int32_t _newx, int32_t _newy) : parentWindow(parent), old_width(_oldx), old_height(_oldy), new_width(_newx), new_height(_newy)
		{
			setTypeName("ogfx::WindowResizedData");
			validate();
		} 
	
	public:
		int32_t new_width;
		int32_t new_height;
		int32_t old_width;
		int32_t old_height;
		WindowBase& parentWindow;
};
class MouseEventData : public ostd::BaseObject
{
	public: enum class eButton { _None = 0, Left, Middle, Right };
	public:
		inline MouseEventData(WindowBase& parent, int32_t mousex, int32_t mousey, eButton btn) : parentWindow(parent), position_x(mousex), position_y(mousey), button(btn)
		{
			setTypeName("ogfx::MouseEventData");
			validate();
		}

	public:
		int32_t position_x;
		int32_t position_y;
		eButton button;
		WindowBase& parentWindow;
};
class KeyEventData : public ostd::BaseObject
{
	public: enum class eKeyEvent { Pressed = 0, Released, Text };
	public:
		inline KeyEventData(WindowBase& parent, int32_t key, char _text, eKeyEvent evt) : parentWindow(parent), keyCode(key), text(_text), eventType(evt)
		{
			setTypeName("ogfx::KeyEventData");
			validate();
		}

	public:
		int32_t keyCode;
		char text;
		eKeyEvent eventType;
		WindowBase& parentWindow;
};