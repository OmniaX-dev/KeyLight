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

#include "ffmpeg_helper.hpp"
#include "Common.hpp"

#include <cstdlib>
#include <cstdio>
#include <array>
#include <ostd/Logger.hpp>
#include <sstream>

#ifdef WINDOWS_OS
	#include <windows.h>
    #define POPEN  _popen
    #define PCLOSE _pclose
    #include <shlwapi.h>
    #pragma comment(lib, "shlwapi.lib")
#else
    #define POPEN  popen
    #define PCLOSE pclose
    #include <unistd.h>
    #include <sys/stat.h>
#endif


static bool is_valid_ffmpeg(const ostd::String& path)
{
    if (path.new_trim().len() == 0) return false;

    // 1. File must exist and be executable
    std::filesystem::path fs = path.cpp_str();
    if (!std::filesystem::exists(fs)) return false;

#ifdef _WIN32
    // On Windows any file can be "executed" via CreateProcess
    (void)0; // nothing extra
#else
    if (access(path.c_str(), X_OK) != 0) return false;
#endif

    // 2. Run "ffmpeg -version" – it must produce *some* output and exit 0
    ostd::String cmd = path + " -version";
    FILE* pipe = POPEN(cmd.c_str(), "r");
    if (!pipe) return false;

    std::array<char, 256> buf{};
    bool got_output = false;
    while (fgets(buf.data(), (int)buf.size(), pipe)) {
        got_output = true;
        break;                 // we only need to know it printed something
    }
    int rc = PCLOSE(pipe);
    return got_output && (rc == 0);
}

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

#ifdef _WIN32
inline std::string utf16_to_utf8(const std::wstring& w)
{
    if (w.empty())
        return {};

    int size_needed = WideCharToMultiByte(
        CP_UTF8, 0,
        w.data(), static_cast<int>(w.size()),
        nullptr, 0,
        nullptr, nullptr
    );

    std::string result(size_needed, 0);

    WideCharToMultiByte(
        CP_UTF8, 0,
        w.data(), static_cast<int>(w.size()),
        result.data(), size_needed,
        nullptr, nullptr
    );

    return result;
}
#endif

// ---------------------------------------------------------------------
// Main function
// ---------------------------------------------------------------------
ostd::String FFMPEG::getExecutablePath()
{
    // -----------------------------------------------------------------
    // 1. Build the candidate list
    // -----------------------------------------------------------------
    std::vector<ostd::String> candidates;

#ifdef _WIN32
    // ---- Windows system locations ------------------------------------------------
    candidates.emplace_back("C:\\ffmpeg\\bin\\ffmpeg.exe");
    candidates.emplace_back("C:\\Program Files\\ffmpeg\\bin\\ffmpeg.exe");

    // Winget installs under %LOCALAPPDATA%\Microsoft\WinGet\Packages\...
    // We cannot enumerate all versions, but the most common pattern is:
    const char* local = std::getenv("LOCALAPPDATA");
    if (local)
    {
        std::filesystem::path winget(std::string(local) + "\\Microsoft\\WinGet\\Packages");
        if (std::filesystem::exists(winget))
        {
            for (const auto& entry : std::filesystem::directory_iterator(winget))
        	{
                if (!entry.is_directory()) continue;
                std::filesystem::path exe = entry.path() / "ffmpeg" / "ffmpeg.exe";
                if (std::filesystem::exists(exe))
                    candidates.emplace_back(exe.string().c_str());
            }
        }
    }

    // ---- Relative to current executable (KeyLight) -------------------------------
    wchar_t modulePath[MAX_PATH] = {};
    if (GetModuleFileNameW(nullptr, modulePath, MAX_PATH)) {
        PathRemoveFileSpecW(modulePath);               // strip exe name
        std::wstring wlocal = std::wstring(modulePath) + L"\\ffmpeg\\ffmpeg.exe";
        candidates.emplace_back(ostd::String(utf16_to_utf8(wlocal));
    }

#else
    // ---- Linux / *nix ------------------------------------------------------------
    candidates.emplace_back("/usr/bin/ffmpeg");
    candidates.emplace_back("/usr/local/bin/ffmpeg");
    candidates.emplace_back("/opt/ffmpeg/bin/ffmpeg");
    candidates.emplace_back("./ffmpeg/ffmpeg");
#endif

    // -----------------------------------------------------------------
    // 2. Test each candidate
    // -----------------------------------------------------------------
    for (const auto& p : candidates)
    {
        if (is_valid_ffmpeg(p))
            return p;
    }

    // -----------------------------------------------------------------
    // 3. Fallback: ask the OS (PATH)
    // -----------------------------------------------------------------
    if (exists())
    {                     // uses `where` / `which` internally
        // Run the same command but capture the *path*
#ifdef _WIN32
        ostd::String out = runCommand("where ffmpeg");
#else
        ostd::String out = runCommand("which ffmpeg");
#endif
        std::istringstream iss(out.cpp_str());
        std::string line;
        while (std::getline(iss, line)) {
            ostd::String candidate(line.c_str());
            if (is_valid_ffmpeg(candidate))
                return candidate;
        }
    }

    return ostd::String();   // not found
}
