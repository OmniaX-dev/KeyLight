#pragma once

#include "MidiParser.hpp"
#include <SFML/Graphics/RenderTexture.hpp>
#include <boost/process/v1/pipe.hpp>
#include <ostd/String.hpp>
#include <unordered_map>
#include <ostd/Geometry.hpp>
#include <ostd/Color.hpp>
#include <ostd/Utils.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#include "Common.hpp"
#include "ffmpeg_helper.hpp"
#include <boost/process/v1.hpp>

namespace bp = boost::process::v1;

class VirtualPiano;

enum class VideoRenderModes { ImageSequence = 0, Video };
enum class ImageType { PNG = 0, BMP = 1, JPG = 2 };

struct PianoKey
{
	MidiParser::NoteInfo noteInfo;
	bool pressed { false };
};
struct VirtualPianoData
{
	private:
		float pixelsPerSecond { 0 };
		float virtualPiano_x { 0.0f };
		float virtualPiano_y { 0.0f };
		float whiteKeyWidth { 0.0f };
		float whiteKeyHeight { 0.0f };
		float blackKeyWidth { 0.0f };
		float blackKeyHeight { 0.0f };
		float blackKeyOffset { 0.0f };
		std::unordered_map<int32_t, float> _keyOffsets;

		float scale_x { 0.0f };
		float scale_y { 0.0f };

		float whiteKeyShrinkFactor { 0 };
		float blackKeyShrinkFactor { 0 };

		ostd::Rectangle glowMargins { 0, 0, 0, 0 };

	public:
		inline static constexpr int32_t base_width { 2080 };
		inline static constexpr int32_t base_height { 1400 };

		ostd::Color whiteKeyColor { 0, 0, 0 };
		ostd::Color whiteKeyPressedColor { 0, 0, 0 };
		ostd::Color blackKeyColor { 0, 0, 0 };
		ostd::Color blackKeyPressedColor { 0, 0, 0 };

		ostd::Color fallingWhiteNoteColor { 0, 0, 0 };
		ostd::Color fallingWhiteNoteOutlineColor { 0, 0, 0 };
		ostd::Color fallingWhiteNoteGlowColor { 0, 0, 0 };
		ostd::Color fallingBlackNoteColor { 0, 0, 0 };
		ostd::Color fallingBlackNoteOutlineColor { 0, 0, 0 };
		ostd::Color fallingBlackNoteGlowColor { 0, 0, 0 };


	public:
		VirtualPianoData(void);
		void recalculateKeyOffsets(void);
		void updateScale(int32_t width, int32_t height);

		inline void setScale(const ostd::Vec2& scale) { scale_x = scale.x; scale_y = scale.y; }
		inline ostd::Vec2 getScale(void) const { return { scale_x, scale_y }; }
		inline float pps(void) const { return pixelsPerSecond * scale_y; }
		inline float vpx(void) const { return virtualPiano_x * scale_x; }
		inline float vpy(void) const { return virtualPiano_y * scale_y; }
		inline float whiteKey_w(void) const { return whiteKeyWidth * scale_x; }
		inline float whiteKey_h(void) const { return whiteKeyHeight * scale_y; }
		inline float blackKey_w(void) const { return blackKeyWidth * scale_x; }
		inline float blackKey_h(void) const { return blackKeyHeight * scale_y; }
		inline float blackKey_offset(void) const { return blackKeyOffset * scale_x; }
		inline float whiteKey_shrink(void) const { return whiteKeyShrinkFactor * scale_x; }
		inline float blackKey_shrink(void) const { return blackKeyShrinkFactor * scale_x; }
		inline ostd::Rectangle getGlowMargins(void) const { return { glowMargins.x * scale_x, glowMargins.y * scale_y,
																	 glowMargins.w * scale_x, glowMargins.h * scale_y }; }
		inline std::unordered_map<int32_t, float>& keyOffsets(void) { recalculateKeyOffsets(); return _keyOffsets;}
};
class NoteEventData : public ostd::BaseObject
{
	public: enum class eEventType { NoteON = 0, NoteOFF, MidiEnd };
	public:
		NoteEventData(PianoKey& key) : vPianoKey(key) { setTypeName("VirtualPiano::NoteEventData"); validate(); }
		PianoKey& vPianoKey;
		MidiParser::NoteEvent note;
		eEventType eventType;
};
struct FallingNoteGraphicsData
{
	ostd::Rectangle rect;
	ostd::Color fillColor;
	ostd::Color outlineColor;
	ostd::Color glowColor;
	sf::Texture* texture;
	int32_t outlineThickness;
	float cornerRadius;
};
class SignalListener : public ostd::BaseObject
{
	public:
		inline SignalListener(VirtualPiano& _parent) : parent(_parent)
		{
			setTypeName("VirtualPiano::SignalListener");
			connectSignal(Common::SigIntSignal);
			validate();
		}
		void handleSignal(ostd::tSignal& signal) override;

	public:
		VirtualPiano& parent;
};
struct VideoRenderState
{
	inline VideoRenderState(VirtualPiano& parent) : virtualPiano(parent) {  }
	inline void reset(void)
	{
		mode = VideoRenderModes::ImageSequence;
		imageType = ImageType::PNG;
		ffmpegProfile = FFMPEG::Profiles::GeneralPurpose;
		ffmpegPipe = nullptr;
		subProcArgs.clear();

		lastNoteEndTime = 0.0;
		totalFrames = 0;
		targetFPS = 60;
		frameTime = 0.0;
		extraFrames = 120;

		baseFileName = "";
		folderPath = "";

		oldScale = { 0.0f, 0.0f };
		resolution = { 0, 0 };

		renderTarget = sf::RenderTexture();
		flippedRenderTarget = sf::RenderTexture();

		frameIndex = 0;
		renderFPS = 0;
		percentage = 0;
		currentTime = 0.0;
	}

	VirtualPiano& virtualPiano;
	VideoRenderModes mode { VideoRenderModes::ImageSequence };
	ImageType imageType { ImageType::PNG };
	FFMPEG::tProfile ffmpegProfile;
	FILE* ffmpegPipe { nullptr };
	std::vector<std::string> subProcArgs;
	bp::child ffmpeg_child;
	bp::opstream ffmpeg_stdin;

	double lastNoteEndTime { 0.0 };
	int32_t totalFrames { 0 };
	uint8_t targetFPS { 60 };
	double frameTime { 0.0 };
	int32_t extraFrames { 120 };
	ostd::Timer framTimeTimer;
	ostd::Timer updateFpsTimer;

	ostd::String baseFileName { "" };
	ostd::String folderPath { "" };
	ostd::String absolutePath { "" };

	ostd::Vec2 oldScale { 0.0f, 0.0f };
	ostd::UI16Point resolution { 0, 0 };

	sf::RenderTexture renderTarget;
	sf::RenderTexture flippedRenderTarget;

	int32_t frameIndex { 0 };
	int32_t renderFPS { 0 };
	int32_t percentage { 0 };
	double currentTime { 0.0 };

	inline bool isFinished(void) const { return frameIndex > (totalFrames + extraFrames); }
};
