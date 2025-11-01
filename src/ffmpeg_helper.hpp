#pragma once

#include <ostd/String.hpp>
#include <ostd/Defines.hpp>

#ifdef WINDOWS_OS
    #define POPEN  _popen
    #define PCLOSE _pclose
#else
    #define POPEN  popen
    #define PCLOSE pclose
#endif

class FFMPEG
{
	public: struct Quality
	{
		inline static const ostd::String Lossless = "18";
		inline static const ostd::String Default = "23";
		inline static const ostd::String Low = "28";
	};
	public: struct Preset
	{
		inline static const ostd::String UltraFast = "ultrafast";
		inline static const ostd::String Fast = "fast";
		inline static const ostd::String Medium = "medium";
		inline static const ostd::String Slow = "slow";
		inline static const ostd::String VerySlow = "veryslow";
	};
	public: struct Container
	{
		inline static const ostd::String MP4 = "mp4";
		inline static const ostd::String MKV = "mkv";
		inline static const ostd::String WebM = "webm";
		inline static const ostd::String AVI = "avi";
		inline static const ostd::String MOV = "mov";
	};
	public: struct Codecs
	{
		struct Audio
		{
			inline static const ostd::String AAC = "aac";
			inline static const ostd::String Flac = "flac";
			inline static const ostd::String MP3 = "libmp3lame";
			inline static const ostd::String OPUS = "libopus";
			inline static const ostd::String PCM = "pcm_s16le";
		};
		struct Video
		{
			inline static const ostd::String H264 = "libx264";
			inline static const ostd::String H265 = "libx265";
			inline static const ostd::String AV1 = "libaom-av1";
			inline static const ostd::String MPEG4 = "mpeg4";
			inline static const ostd::String VP9 = "libvpx-vp9";
			inline static const ostd::String PRORES = "prores_ks";
		};
	};
	public: struct tProfile
	{
		const ostd::String Container { "" };
		const ostd::String VideoCodec { "" };
		const ostd::String AudioCodec { "" };
		const ostd::String Preset { "" };
		const ostd::String Quality { "" };
	};
	public: struct Profiles
	{
		inline static const tProfile GeneralPurpose { Container::MP4, Codecs::Video::H264, Codecs::Audio::AAC, Preset::Medium, Quality::Default };
		inline static const tProfile HighQUality { Container::MKV, Codecs::Video::H265, Codecs::Audio::Flac, Preset::Slow, Quality::Lossless };
		inline static const tProfile Streaming { Container::WebM, Codecs::Video::AV1, Codecs::Audio::OPUS, Preset::Fast, Quality::Default };
		inline static const tProfile Legacy { Container::AVI, Codecs::Video::MPEG4, Codecs::Audio::MP3, Preset::Fast, Quality::Low };
		inline static const tProfile Editing { Container::MOV, Codecs::Video::PRORES, Codecs::Audio::PCM, "", "" };
	};
	public:
		static ostd::String runCommand(const ostd::String& cmd);
		static bool exists(void);
		static bool isEncodeCodecAvailable(const ostd::String& codecName, bool checkEncode = true);
		static void printDebugInfo(void);

	private:
		static bool __list_contains_name(const ostd::String& output, const ostd::String& name);
};
