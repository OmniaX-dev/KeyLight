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

#include <csignal>
#include <ostd/Logger.hpp>

#include "Window.hpp"
#include "ffmpeg_helper.hpp"

ostd::ConsoleOutputHandler out;

void handleSigint(int signal)
{
	if (signal == SIGINT)
	{
		ostd::SignalHandler::emitSignal(Common::SigIntSignal, ostd::tSignalPriority::RealTime);
	}
}

int main(int argc, char** argv)
{
	std::signal(SIGINT, handleSigint);

	Window window;
	window.initialize(VirtualPianoData::base_width, VirtualPianoData::base_height, "KeyLight");

	auto ffmpegPath = FFMPEG::getExecutablePath().trim();
	if (ffmpegPath != "")
		OX_DEBUG("FFMPEG found: %s", ffmpegPath.c_str());
	else
		OX_DEBUG("FFMPEG not found. Video export disabled.");

	while (window.isRunning())
	{
		window.update();
	}

	return 0;
}
