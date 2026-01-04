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

#include <ostd/Json.hpp>
#include <ostd/Midi.hpp>
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
#include "Particles.hpp"
#include "ffmpeg_helper.hpp"
#include <boost/process/v1.hpp>

namespace bp = boost::process::v1;

class VirtualPiano;

enum class VideoRenderModes { ImageSequence = 0, Video };
enum class ImageType { PNG = 0, BMP = 1, JPG = 2 };

struct PianoKey
{
	ostd::MidiParser::NoteInfo noteInfo;
	ParticleEmitter particles;
	bool pressed { false };
	ostd::Vec2 pressedForce { 0.0f, 0.0f };
};
struct VirtualPianoData
{
	public: enum class eBlurType { Gaussian, Kawase };
	public: struct BlurData
	{
		uint8_t passes { 8 };
		float startRadius { 1.0f };
		float increment { 1.0f };
		float threshold { 0.1f };
		uint8_t resolutionDivider { 1 };
		float bloomIntensity { 1.0f };
		eBlurType type { eBlurType::Gaussian };
	};

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

		double fallingTime_s { 0 };
		float pressedVelocityMultiplier { 0.0f };

		int32_t fallingWhiteNoteOutlineWidth { 0 };
		float fallingWhiteNoteBorderRadius { 0 };

		int32_t fallingBlackNoteOutlineWidth { 0 };
		float fallingBlackNoteBorderRadius { 0 };

		ostd::Vec2 texCoordsPos { 0, 0 };
		ostd::Vec2 texCoordsScale { 1, 1 };

		ostd::Color backgroundColor { 0, 0, 0 };

		ostd::Color whiteKeyColor { 0, 0, 0 };
		ostd::Color whiteKeyPressedColor { 0, 0, 0 };
		ostd::Color whiteKeySplitColor = { 0, 0, 0 };
		ostd::Color blackKeyColor { 0, 0, 0 };
		ostd::Color blackKeyPressedColor { 0, 0, 0 };
		ostd::Color blackKeySplitColor = { 0, 0, 0 };

		ostd::Color fallingWhiteNoteColor { 0, 0, 0 };
		ostd::Color fallingWhiteNoteOutlineColor { 0, 0, 0 };
		ostd::Color fallingWhiteNoteGlowColor { 0, 0, 0 };
		ostd::Color fallingBlackNoteColor { 0, 0, 0 };
		ostd::Color fallingBlackNoteOutlineColor { 0, 0, 0 };
		ostd::Color fallingBlackNoteGlowColor { 0, 0, 0 };

		ostd::Color pianoLineColor1 = { 0, 0, 0 };
		ostd::Color pianoLineColor2 = { 0, 0, 0 };

		bool usePerNoteColors { false };
		ostd::Color perNoteColors[36] = {
			{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
			{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
			{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
			{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },

			{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
			{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
			{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
			{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },

			{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
			{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
			{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 },
			{ 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }
		};

		BlurData blur;

		enum class eNoteColor {
			C_Main = 0, Csharp_Main, D_Main, Dsharp_Main,
			E_Main, F_Main, Fsharp_Main, G_Main,
			Gsharp_Main, A_Main, Asharp_Main, B_Main,

			C_Outline, Csharp_Outline, D_Outline, Dsharp_Outline,
			E_Outline, F_Outline, Fsharp_Outline, G_Outline,
			Gsharp_Outline, A_Outline, Asharp_Outline, B_Outline,

			C_Glow, Csharp_Glow, D_Glow, Dsharp_Glow,
			E_Glow, F_Glow, Fsharp_Glow, G_Glow,
			Gsharp_Glow, A_Glow, Asharp_Glow, B_Glow
		};


	public:
		VirtualPianoData(void);
		void loadFromStyleJSON(ostd::JsonFile& styleJson);
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
		ostd::MidiParser::NoteEvent note;
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

		inline static const uint64_t NoteOnSignal = ostd::SignalHandler::newCustomSignal(5000);
		inline static const uint64_t NoteOffSignal = ostd::SignalHandler::newCustomSignal(5001);
		inline static const uint64_t MidiStartSignal = ostd::SignalHandler::newCustomSignal(5002);
		inline static const uint64_t MidiEndSignal = ostd::SignalHandler::newCustomSignal(5003);
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
