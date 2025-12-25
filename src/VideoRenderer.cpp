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

#include "VideoRenderer.hpp"
#include "VirtualPiano.hpp"
#include "Window.hpp"
#include "ffmpeg_helper.hpp"
#include <ostd/Logger.hpp>
#include "Renderer.hpp"
#include <cmath>
#include <cstdio>
#include <filesystem>

VideoRenderer::VideoRenderer(VirtualPiano& vpiano) : m_vpiano(vpiano), m_videoRenderState(vpiano)
{

}

bool VideoRenderer::configImageSequenceRender(const ostd::String& folderPath, const ostd::UI16Point& resolution, uint8_t fps)
{
	if (m_isRenderingToFile) return false;
	if (resolution.x != 1920 || resolution.y != 1080) return false; //TODO: allow for valid resolutions
	if (fps != 60) return false; //TODO: allow for valid FPS values
	if (m_vpiano.vPianoRes().lastNoteEndTime == 0.0) return false; //TODO: Error

	m_videoRenderState.reset();
	m_videoRenderState.mode = VideoRenderModes::ImageSequence;
	m_videoRenderState.resolution = resolution;
	m_videoRenderState.folderPath = folderPath;
	m_videoRenderState.targetFPS = fps;
	m_videoRenderState.lastNoteEndTime = m_vpiano.vPianoRes().lastNoteEndTime;
	m_videoRenderState.totalFrames = (int32_t)std::ceil(m_videoRenderState.lastNoteEndTime * fps);
	m_videoRenderState.oldScale = m_vpiano.vPianoData().getScale();
	m_videoRenderState.renderTarget = sf::RenderTexture({ resolution.x, resolution.y });
	m_videoRenderState.flippedRenderTarget = sf::RenderTexture({ resolution.x, resolution.y });
	m_videoRenderState.frameTime = 1.0 / (float)fps;
	m_videoRenderState.renderFPS = 1;

	__preallocate_file_names_for_rendering(m_videoRenderState.totalFrames, m_videoRenderState.baseFileName, m_videoRenderState.folderPath, m_videoRenderState.imageType);
	Common::ensureDirectory(folderPath);
	m_vpiano.vPianoData().updateScale(resolution.x, resolution.y);
	m_vpiano.onWindowResized(resolution.x, resolution.y);
	m_vpiano.getParentWindow().lockFullscreenStatus();
	m_vpiano.getParentWindow().enableResizeable(false);
	m_vpiano.stop();
	m_videoRenderState.updateFpsTimer.startCount(ostd::eTimeUnits::Milliseconds);

	m_isRenderingToFile = true;
	return true;
}

bool VideoRenderer::configFFMPEGVideoRender(const ostd::String& filePath, const ostd::UI16Point& resolution, uint8_t fps, const FFMPEG::tProfile& profile)
{
	if (m_isRenderingToFile) return false;
	if (resolution.x != 1920 || resolution.y != 1080) return false; //TODO: allow for valid resolutions
	if (fps != 60) return false; //TODO: allow for valid FPS values
	if (m_vpiano.vPianoRes().lastNoteEndTime == 0.0) return false; //TODO: Error

	m_videoRenderState.reset();
	m_videoRenderState.ffmpegProfile = profile;
	m_videoRenderState.mode = VideoRenderModes::Video;
	m_videoRenderState.resolution = resolution;
	ostd::String tmp = filePath.new_trim();
	while (tmp.startsWith("./") || tmp.startsWith(".\\"))
		tmp.substr(2).trim();
	m_videoRenderState.folderPath = tmp;
	m_videoRenderState.absolutePath = ostd::String(std::filesystem::absolute(m_videoRenderState.folderPath)).add(".").add(profile.Container);
	m_videoRenderState.targetFPS = fps;
	m_videoRenderState.lastNoteEndTime = m_vpiano.vPianoRes().lastNoteEndTime;
	m_videoRenderState.totalFrames = (int32_t)std::ceil(m_videoRenderState.lastNoteEndTime * fps);
	m_videoRenderState.oldScale = m_vpiano.vPianoData().getScale();
	m_videoRenderState.renderTarget = sf::RenderTexture({ resolution.x, resolution.y });
	m_videoRenderState.flippedRenderTarget = sf::RenderTexture({ resolution.x, resolution.y });
	m_videoRenderState.frameTime = 1.0 / (float)fps;
	m_videoRenderState.renderFPS = 1;
	m_vpiano.vPianoData().updateScale(resolution.x, resolution.y);
	m_vpiano.onWindowResized(resolution.x, resolution.y);
	m_vpiano.getParentWindow().lockFullscreenStatus();
	m_vpiano.getParentWindow().enableResizeable(false);
	m_vpiano.stop();
	m_videoRenderState.updateFpsTimer.startCount(ostd::eTimeUnits::Milliseconds);
	m_videoRenderState.ffmpegPipe = __open_ffmpeg_pipe(m_videoRenderState.folderPath, resolution, fps, profile);
	if (m_videoRenderState.ffmpegPipe == nullptr) return false;

	m_isRenderingToFile = true;
	return true;
}

void VideoRenderer::renderNextOutputFrame(void)
{
	if (!m_isRenderingToFile) return;
	m_vpiano.vKeyboard().updateVisualization(m_videoRenderState.currentTime);
	m_vpiano.renderFrame(m_videoRenderState.renderTarget);
	m_videoRenderState.framTimeTimer.startCount(ostd::eTimeUnits::Milliseconds);
	Renderer::setRenderTarget(&m_videoRenderState.flippedRenderTarget);
	Renderer::useShader(&m_vpiano.vPianoRes().flipShader);
	m_vpiano.vPianoRes().flipShader.setUniform("texture", m_videoRenderState.renderTarget.getTexture());
	Renderer::drawTexture(m_videoRenderState.renderTarget.getTexture());
	Renderer::useShader(nullptr);
	if (m_videoRenderState.mode == VideoRenderModes::ImageSequence)
		__save_frame_to_file(m_videoRenderState.flippedRenderTarget, m_videoRenderState.folderPath, ++m_videoRenderState.frameIndex);
	else if (m_videoRenderState.mode == VideoRenderModes::Video)
	{
		__stream_frame_to_ffmpeg();
		++m_videoRenderState.frameIndex;
	}
	double _frame_render_time = (double)m_videoRenderState.framTimeTimer.endCount();
	if (m_videoRenderState.updateFpsTimer.read() > 1000 == 0)
	{
		m_videoRenderState.renderFPS = (int32_t)std::round(1000.0 / _frame_render_time);
		m_videoRenderState.updateFpsTimer.endCount();
		m_videoRenderState.updateFpsTimer.startCount(ostd::eTimeUnits::Milliseconds);
	}
	m_videoRenderState.percentage = Common::percentage(m_videoRenderState.frameIndex, m_videoRenderState.totalFrames + m_videoRenderState.extraFrames);

	m_videoRenderState.currentTime += m_videoRenderState.frameTime;
	Renderer::setRenderTarget(nullptr);
}

void VideoRenderer::finishOutputRender(void)
{
	if (!m_isRenderingToFile) return;
	if (!m_videoRenderState.isFinished()) return;
	m_vpiano.vPianoData().updateScale(m_vpiano.getParentWindow().getWindowWidth(), m_vpiano.getParentWindow().getWindowHeight());
	m_vpiano.onWindowResized(m_vpiano.getParentWindow().getWindowWidth(), m_vpiano.getParentWindow().getWindowHeight());
	m_vpiano.getParentWindow().lockFullscreenStatus(false);
	m_vpiano.getParentWindow().enableResizeable(true);
	m_isRenderingToFile = false;
	if (m_videoRenderState.mode == VideoRenderModes::Video)
	{
	    if (m_videoRenderState.ffmpegPipe)
		{
			fflush(m_videoRenderState.ffmpegPipe);
			fclose(m_videoRenderState.ffmpegPipe);
			m_videoRenderState.ffmpegPipe = nullptr;
	    }

	    if (m_videoRenderState.ffmpeg_child.running())
		{
			m_videoRenderState.ffmpeg_child.wait();
			int exit_code = m_videoRenderState.ffmpeg_child.exit_code();
	        if (exit_code == 0)
	            OX_DEBUG("Video encoded successfully!");
	        else
	            OX_ERROR("FFmpeg failed with exit code: %d", exit_code);
	    }
	}
}

void VideoRenderer::__preallocate_file_names_for_rendering(uint32_t frameCount, const ostd::String& baseFileName, const ostd::String& basePath, ImageType imageType, const uint16_t marginFrames)
{
	ostd::String extension = "png";
	if (imageType == ImageType::BMP) extension = "bmp";
	if (imageType == ImageType::JPG) extension = "jpg";
	m_renderFileNames.clear();
	char filename[256];
	for (int32_t i = 0; i < frameCount + marginFrames; i++)
	{
    	std::snprintf(filename, sizeof(filename), "%s/%s%06d.%s", basePath.c_str(), baseFileName.c_str(), i, extension.c_str());
    	m_renderFileNames.push_back(filename);
	}
}

FILE* VideoRenderer::__open_ffmpeg_pipe(const ostd::String& filePath, const ostd::UI16Point& resolution, uint8_t fps, const FFMPEG::tProfile& profile)
{
	if (!FFMPEG::exists()) return nullptr; // TODO: Error
	if (!FFMPEG::isEncodeCodecAvailable(profile.VideoCodec)) return nullptr; //TODO: Error
	if (!FFMPEG::isEncodeCodecAvailable(profile.AudioCodec)) return nullptr; //TODO: Error

	m_videoRenderState.subProcArgs.push_back("-f");
	m_videoRenderState.subProcArgs.push_back("rawvideo");
	m_videoRenderState.subProcArgs.push_back("-pix_fmt");
	m_videoRenderState.subProcArgs.push_back("rgba");
	m_videoRenderState.subProcArgs.push_back("-s");
	m_videoRenderState.subProcArgs.push_back(ostd::String("").add(resolution.x).add("x").add(resolution.y));
	m_videoRenderState.subProcArgs.push_back("-r");
	m_videoRenderState.subProcArgs.push_back(ostd::String("").add(fps));
	// m_videoRenderState.subProcArgs.push_back("-re");
	m_videoRenderState.subProcArgs.push_back("-i");
	m_videoRenderState.subProcArgs.push_back("-");
	if (m_vpiano.vPianoRes().hasAudioFile())
	{
		if (m_vpiano.vPianoRes().firstNoteStartTime > m_vpiano.vPianoRes().autoSoundStart)
		{
			// m_videoRenderState.subProcArgs.push_back("-itsoffset");
			// m_videoRenderState.subProcArgs.push_back(ostd::String("").add((m_firstNoteStartTime - m_autoSoundStart)));
			m_videoRenderState.subProcArgs.push_back("-f");
			m_videoRenderState.subProcArgs.push_back("lavfi");
			m_videoRenderState.subProcArgs.push_back("-i");
			m_videoRenderState.subProcArgs.push_back(ostd::String("anullsrc=channel_layout=stereo:sample_rate=44100:duration=").add((m_vpiano.vPianoRes().firstNoteStartTime - m_vpiano.vPianoRes().autoSoundStart)));
			m_videoRenderState.subProcArgs.push_back("-i");
			m_videoRenderState.subProcArgs.push_back(ostd::String("").add(m_vpiano.vPianoRes().audioFilePath).add(""));
			m_videoRenderState.subProcArgs.push_back("-filter_complex");
			// m_videoRenderState.subProcArgs.push_back("[1:a][2:a]concat=n=2:v=0:a=1[aout]");
			m_videoRenderState.subProcArgs.push_back(
			    "[1:a]aformat=sample_fmts=s16:channel_layouts=stereo:sample_rates=44100[sil];"
			    "[2:a]aformat=sample_fmts=s16:channel_layouts=stereo:sample_rates=44100[aud];"
			    "[sil][aud]concat=n=2:v=0:a=1[aout]"
			);
			m_videoRenderState.subProcArgs.push_back("-map");
			m_videoRenderState.subProcArgs.push_back("0:v");
			m_videoRenderState.subProcArgs.push_back("-map");
			m_videoRenderState.subProcArgs.push_back("[aout]");
		}
		else if (m_vpiano.vPianoRes().autoSoundStart > m_vpiano.vPianoRes().firstNoteStartTime)
		{
			m_videoRenderState.subProcArgs.push_back("-ss");
			m_videoRenderState.subProcArgs.push_back(ostd::String("").add((m_vpiano.vPianoRes().autoSoundStart - m_vpiano.vPianoRes().firstNoteStartTime)));
			m_videoRenderState.subProcArgs.push_back("-i");
			m_videoRenderState.subProcArgs.push_back(ostd::String("").add(m_vpiano.vPianoRes().audioFilePath).add(""));
		}
		else
		{
			m_videoRenderState.subProcArgs.push_back("-i");
			m_videoRenderState.subProcArgs.push_back(ostd::String("").add(m_vpiano.vPianoRes().audioFilePath).add(""));
		}
	}
	m_videoRenderState.subProcArgs.push_back("-c:v");
	m_videoRenderState.subProcArgs.push_back(ostd::String("").add(profile.VideoCodec));
	m_videoRenderState.subProcArgs.push_back("-preset");
	m_videoRenderState.subProcArgs.push_back(ostd::String("").add(profile.Preset));
	m_videoRenderState.subProcArgs.push_back("-crf");
	m_videoRenderState.subProcArgs.push_back(ostd::String("").add(profile.Quality));
	m_videoRenderState.subProcArgs.push_back("-c:a");
	m_videoRenderState.subProcArgs.push_back(ostd::String("").add(profile.AudioCodec));
	m_videoRenderState.subProcArgs.push_back("-b:a");
	m_videoRenderState.subProcArgs.push_back("192k");
	m_videoRenderState.subProcArgs.push_back("-y");
	// m_videoRenderState.subProcArgs.push_back("-t");
	// m_videoRenderState.subProcArgs.push_back("10");
	m_videoRenderState.subProcArgs.push_back("-loglevel");
	m_videoRenderState.subProcArgs.push_back("error");
	m_videoRenderState.subProcArgs.push_back("-shortest");
	m_videoRenderState.subProcArgs.push_back("-movflags");
	m_videoRenderState.subProcArgs.push_back("+faststart");
	m_videoRenderState.subProcArgs.push_back(ostd::String("").add(filePath).add(".").add(profile.Container).add(""));

	ostd::String ffmpeg_executable = FFMPEG::getExecutablePath();
	ostd::String cmd = ffmpeg_executable.new_add(" ");
	for (const auto& str : m_videoRenderState.subProcArgs)
		cmd.add(str).add(" ");
	OX_DEBUG("FFMPEG Command:\n%s", cmd.c_str());

	// LAUNCH FFMPEG
    try
    {
        m_videoRenderState.ffmpeg_child = bp::child(
            bp::exe = ffmpeg_executable.cpp_str(),
            bp::args = m_videoRenderState.subProcArgs,
            bp::std_in < m_videoRenderState.ffmpeg_stdin,
            bp::std_out > bp::null,
            bp::std_err > stderr  // ← SEE FFMPEG ERRORS IN CONSOLE
        );
    }
    catch (const std::exception& e)
    {
        OX_ERROR("Boost.Process v1 failed: %s", e.what());
        return nullptr;
    }
    // Convert opstream to FILE* for your fwrite()
    int fd = m_videoRenderState.ffmpeg_stdin.pipe().native_sink();
    FILE* pipe_file = fdopen(fd, "wb");
    if (!pipe_file)
    {
        OX_ERROR("fdopen failed");
        m_videoRenderState.ffmpeg_child.terminate();
        return nullptr;
    }
    return pipe_file;
}

void VideoRenderer::__save_frame_to_file(const sf::RenderTexture& rt, const ostd::String& basePath, int frameIndex)
{
	if (!m_isRenderingToFile) return;
	if (m_videoRenderState.mode != VideoRenderModes::ImageSequence) return;
    sf::Image img = rt.getTexture().copyToImage();
    if (!img.saveToFile(m_renderFileNames[frameIndex]))
    {
        OX_ERROR("Failed to save frame %d to %s", frameIndex, m_renderFileNames[frameIndex].c_str());
    }
}

void VideoRenderer::__stream_frame_to_ffmpeg(void)
{
	if (!m_isRenderingToFile) return;
	if (m_videoRenderState.mode != VideoRenderModes::Video) return;
	sf::Image frame = m_videoRenderState.flippedRenderTarget.getTexture().copyToImage();
    const uint8_t* pixels = frame.getPixelsPtr();
    std::size_t dataSize = m_videoRenderState.flippedRenderTarget.getSize().x * m_videoRenderState.flippedRenderTarget.getSize().y * 4;
    if (!m_videoRenderState.ffmpeg_child.running())
    {
        OX_ERROR("FFmpeg not running");
        return;
    }
    if (fwrite(pixels, 1, dataSize, m_videoRenderState.ffmpegPipe) != dataSize)
    {
        OX_ERROR("fwrite failed: %s", strerror(errno));
        return;
    }
    fflush(m_videoRenderState.ffmpegPipe);
}
