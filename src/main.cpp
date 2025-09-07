#include <ostd/IOHandlers.hpp>

ostd::ConsoleOutputHandler out;

int main(int argc, char** argv)
{
	out.fg(ostd::ConsoleColors::Blue).p("Hello").fg(ostd::ConsoleColors::Red).p(" World").fg(ostd::ConsoleColors::Yellow).p("!!!").nl().reset();
	return 0;
}