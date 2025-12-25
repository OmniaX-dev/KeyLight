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

#pragma once

#include "VPianoData.hpp"

class VideoRenderer
{
	public:
		VideoRenderer(VirtualPiano& vpiano);
		bool configImageSequenceRender(const ostd::String& folderPath, const ostd::UI16Point& resolution, uint8_t fps);
		bool configFFMPEGVideoRender(const ostd::String& filePath, const ostd::UI16Point& resolution, uint8_t fps, const FFMPEG::tProfile& profile);
		void renderNextOutputFrame(void);
		void finishOutputRender(void);

		inline VideoRenderState& getVideoRenderState(void) { return m_videoRenderState; }
		inline bool isRenderingToFile(void) { return m_isRenderingToFile; }

	private:
		void __preallocate_file_names_for_rendering(uint32_t frameCount, const ostd::String& baseFileName, const ostd::String& basePath, ImageType imageType, const uint16_t marginFrames = 200);
		FILE* __open_ffmpeg_pipe(const ostd::String& filePath, const ostd::UI16Point& resolution, uint8_t fps, const FFMPEG::tProfile& profile);
		void __save_frame_to_file(const sf::RenderTexture& rt, const ostd::String& basePath, int frameIndex);
		void __stream_frame_to_ffmpeg(void);

	public:
		VirtualPiano& m_vpiano;
		VideoRenderState m_videoRenderState;
		std::vector<ostd::String> m_renderFileNames;
		bool m_isRenderingToFile { false };

};
