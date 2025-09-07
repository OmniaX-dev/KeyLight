#include <ostd/IOHandlers.hpp>

#include "Window.hpp"

ostd::ConsoleOutputHandler out;

int main(int argc, char** argv)
{
	Window window;
	window.initialize(52 * 40, 720, "KeyLight");
	window.setClearColor({ 0, 2	, 15 });
	
	while (window.isRunning())
	{
		window.update();
	}

	return 0;
}