#include <ostd/IOHandlers.hpp>

#include "Window.hpp"

ostd::ConsoleOutputHandler out;

int main(int argc, char** argv)
{
	Window window;
	window.initialize(Window::VirtualPianoData::base_width, Window::VirtualPianoData::base_height, "KeyLight");
	window.setClearColor({ 0, 2	, 15 });
	
	while (window.isRunning())
	{
		window.update();
	}

	return 0;
}