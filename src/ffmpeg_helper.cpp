#include "ffmpeg_helper.hpp"
#include "Common.hpp"

#include <cstdlib>
#include <cstdio>
#include <array>
#include <ostd/Logger.hpp>
#include <sstream>

ostd::String FFMPEG::runCommand(const ostd::String& cmd)
{
#ifdef _WIN32
	const std::string full = cmd + " 2>nul";
#else
    const std::string full = cmd + " 2>/dev/null";
#endif
    std::array<char, 4096> buf{};
    std::string out;
    if (FILE* pipe = POPEN(full.c_str(), "r"))
    {
        while (fgets(buf.data(), (int)buf.size(), pipe))
        	out += buf.data();
        PCLOSE(pipe);
    }
    return out;
}

bool FFMPEG::exists(void)
{
#ifdef WINDOWS_OS
    if (std::system("where ffmpeg >nul 2>nul") == 0) return true;
    if (std::filesystem::exists("./ffmpeg/ffmpeg.exe")) return true;
#else
    if (std::system("which ffmpeg >/dev/null 2>&1") == 0) return true;
#endif
    return false;
}

bool FFMPEG::isEncodeCodecAvailable(const ostd::String& codecName, bool checkEncode)
{
	if (!exists()) return false;
    const char* listCmd = checkEncode ? "ffmpeg -hide_banner -encoders"
                                      : "ffmpeg -hide_banner -decoders";
    std::string out = runCommand(listCmd);
    return __list_contains_name(out, codecName);
}

void FFMPEG::printDebugInfo(void)
{
	if (!Common::IsDebug) return;
	if (!exists())
	{
		OX_DEBUG("FFMPEG Not found.");
		return;
	}
	OX_DEBUG("FFMPEG TEST: AUDIO CODECS:");
	OX_DEBUG("  DECODE: AAC: %s", (FFMPEG::isEncodeCodecAvailable("aac", false) ? "yes" : "no"));
	OX_DEBUG("  ENCODE: AAC: %s", (FFMPEG::isEncodeCodecAvailable("aac") ? "yes" : "no"));
	OX_DEBUG("  DECODE: FLAC: %s", (FFMPEG::isEncodeCodecAvailable("flac", false) ? "yes" : "no"));
	OX_DEBUG("  ENCODE: FLAC: %s", (FFMPEG::isEncodeCodecAvailable("flac") ? "yes" : "no"));
	OX_DEBUG("  DECODE: MP3: %s", (FFMPEG::isEncodeCodecAvailable("libmp3lame", false) ? "yes" : "no"));
	OX_DEBUG("  ENCODE: MP3: %s", (FFMPEG::isEncodeCodecAvailable("libmp3lame") ? "yes" : "no"));
	OX_DEBUG("  DECODE: OPUS: %s", (FFMPEG::isEncodeCodecAvailable("libopus", false) ? "yes" : "no"));
	OX_DEBUG("  ENCODE: OPUS: %s", (FFMPEG::isEncodeCodecAvailable("libopus") ? "yes" : "no"));
	OX_DEBUG("  DECODE: PCM: %s", (FFMPEG::isEncodeCodecAvailable("pcm_s16le", false) ? "yes" : "no"));
	OX_DEBUG("  ENCODE: PCM: %s", (FFMPEG::isEncodeCodecAvailable("pcm_s16le") ? "yes" : "no"));
	OX_DEBUG("FFMPEG TEST: VIDEO CODECS:");
	OX_DEBUG("  DECODE: H264: %s", (FFMPEG::isEncodeCodecAvailable("libx264", false) ? "yes" : "no"));
	OX_DEBUG("  ENCODE: H264: %s", (FFMPEG::isEncodeCodecAvailable("libx264") ? "yes" : "no"));
	OX_DEBUG("  DECODE: H265: %s", (FFMPEG::isEncodeCodecAvailable("libx265", false) ? "yes" : "no"));
	OX_DEBUG("  ENCODE: H265: %s", (FFMPEG::isEncodeCodecAvailable("libx265") ? "yes" : "no"));
	OX_DEBUG("  DECODE: AV1: %s", (FFMPEG::isEncodeCodecAvailable("libaom-av1", false) ? "yes" : "no"));
	OX_DEBUG("  ENCODE: AV1: %s", (FFMPEG::isEncodeCodecAvailable("libaom-av1") ? "yes" : "no"));
	OX_DEBUG("  DECODE: MPEG4: %s", (FFMPEG::isEncodeCodecAvailable("mpeg4", false) ? "yes" : "no"));
	OX_DEBUG("  ENCODE: MPEG4: %s", (FFMPEG::isEncodeCodecAvailable("mpeg4") ? "yes" : "no"));
	OX_DEBUG("  DECODE: VP9: %s", (FFMPEG::isEncodeCodecAvailable("libvpx-vp9", false) ? "yes" : "no"));
	OX_DEBUG("  ENCODE: VP9: %s", (FFMPEG::isEncodeCodecAvailable("libvpx-vp9") ? "yes" : "no"));
	OX_DEBUG("  DECODE: PRORES: %s", (FFMPEG::isEncodeCodecAvailable("prores_ks", false) ? "yes" : "no"));
	OX_DEBUG("  ENCODE: PRORES: %s", (FFMPEG::isEncodeCodecAvailable("prores_ks") ? "yes" : "no"));
}

bool FFMPEG::__list_contains_name(const ostd::String& output, const ostd::String& name)
{
    std::istringstream iss(output);
    std::string line;
    bool inTable = false;
    while (std::getline(iss, line)) {
        // Table starts after a header like: "Encoders:" and a dashed separator.
        if (!inTable) {
            if (line.find("------") != std::string::npos) inTable = true;
            continue;
        }
        // Lines typically look like:
        // " V..... libx264            H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10 (codec h264)"
        // Columns: flags, whitespace, name, whitespace, description
        if (line.empty()) continue;
        // Split flags and the rest
        // Flags are the first token; then name is the next token.
        std::istringstream ls(line);
        std::string flags, candidate;
        ls >> flags >> candidate;
        if (candidate == name.cpp_str()) return true;
    }
    return false;
}
