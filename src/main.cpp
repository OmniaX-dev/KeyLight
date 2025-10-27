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

#include <ostd/IOHandlers.hpp>

#include "Window.hpp"

ostd::ConsoleOutputHandler out;

int main(int argc, char** argv)
{
	Window window;
	window.initialize(VirtualPiano::VirtualPianoData::base_width, VirtualPiano::VirtualPianoData::base_height, "KeyLight");
	window.setClearColor({ 0, 2	, 15 });

	while (window.isRunning())
	{
		window.update();
	}

	return 0;
}
