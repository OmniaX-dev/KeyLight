#include "VirtualPiano.hpp"
#include "Common.hpp"
#include <ostd/Logger.hpp>
#include "Window.hpp"

VirtualPiano::VirtualPianoData::VirtualPianoData(void)
{
	whiteKeyWidth = 40;
	whiteKeyHeight = whiteKeyWidth * 8;
	blackKeyWidth = 22;
	blackKeyHeight = blackKeyWidth * 9;
	blackKeyOffset = 4;
	whiteKeyColor = { 245, 245, 245 };
	whiteKeyPressedColor = { 120, 120, 210 };
	blackKeyColor = { 0, 0, 0 };
	blackKeyPressedColor = { 20, 20, 90 };
	virtualPiano_x =  0.0f;
	pixelsPerSecond  = 250;
	virtualPiano_y = VirtualPianoData::base_height - whiteKeyHeight;
	scale_x = 1.0f;
	scale_y = 1.0f;
	recalculateKeyOffsets();
}

void VirtualPiano::VirtualPianoData::recalculateKeyOffsets(void)
{
	_keyOffsets.clear();
	int whiteKeyCount = 0;
	for (int midiNote = 21; midiNote <= 108; ++midiNote)
	{
		int noteInOctave = midiNote % 12;
		int keyIndex = midiNote - 21;
		if (MidiParser::NoteInfo::isWhiteKey(noteInOctave))
		{
			float x = vpx() + (whiteKeyCount * whiteKey_w());
			_keyOffsets[keyIndex] = x;
			whiteKeyCount++;
		}
		else // black keys
		{
			float x = vpx() + ((whiteKeyCount - 1) * whiteKey_w() + (whiteKey_w() - blackKey_w() / 2.0f)) - blackKey_offset();
			_keyOffsets[keyIndex] = x;
		}
	}
}

void VirtualPiano::VirtualPianoData::updateScale(int32_t width, int32_t height)
{
	scale_x = (float)width / (float)base_width;
	scale_y = (float)height / (float)base_height;
}



void VirtualPiano::init(void)
{	
	m_playing = false;
	m_paused = false;
	m_firstNotePlayed = false;

	for (int midiNote = 21; midiNote <= 108; ++midiNote)
	{
		PianoKey pk;
		pk.noteInfo = MidiParser::getNoteInfo(midiNote);
		pk.pressed = false;
		m_pianoKeys.push_back(pk);
	}
	if (!noteShader.loadFromFile("shaders/note.frag", sf::Shader::Type::Fragment))
		OX_ERROR("Failed to load shader");
}

void VirtualPiano::play(void)
{
	if (m_paused)
	{
		m_playing = true;
		m_paused = false;
		return;
	}
	stop();
	m_playing = true;
}

void VirtualPiano::pause(void)
{
	m_audioFile.pause();
	m_playing = false;
	m_paused = true;
}

void VirtualPiano::stop(void)
{
	m_audioFile.stop();
	m_playing = false;
	m_paused = false;
	m_firstNotePlayed = false;
	m_startTimeOffset_ns = Common::getCurrentTIme_ns();
	m_nextFallingNoteIndex = 0;
	m_activeFallingNotes.clear();
	for (auto& pk : m_pianoKeys)
		pk.pressed = false;
}

bool VirtualPiano::loadMidiFile(const ostd::String& filePath)
{
	try
	{
		m_midiNotes.clear();
		m_midiNotes = MidiParser::parseFile(filePath);
		for (auto& note : m_midiNotes)
		{
			note.startTime += m_fallingTime_s;
			note.endTime   += m_fallingTime_s;
		}
		std::sort(m_midiNotes.begin(), m_midiNotes.end());
		OX_DEBUG("loaded <%s>: total notes parsed: %d", filePath.c_str(), m_midiNotes.size());
		return true;
    }
    catch (const std::exception& ex) {
		OX_ERROR(ex.what());
        return false;
    }
}

bool VirtualPiano::loadAudioFile(const ostd::String& filePath)
{
	m_hasAudioFile = false;
	if (!m_audioFile.openFromFile(filePath.cpp_str()))
	{
		OX_ERROR("Failed to open audio file: %s", filePath.c_str());
		return false;
	}
	OX_DEBUG("loaded <%s>", filePath.c_str());
	m_hasAudioFile = true;
	m_autoSoundStart = scanMusicStartPoint(filePath);
	return true;
}

double VirtualPiano::getPlayTime_s(void)
{
	double playTime = Common::getCurrentTIme_ns() - m_startTimeOffset_ns;
	return playTime * 1e-9;
}

void VirtualPiano::update(void)
{
	if (m_playing)
	{
		double currentTime = getPlayTime_s(); // in seconds

		// Remove notes that have ended
		while (!m_activeFallingNotes.empty() && currentTime > (m_activeFallingNotes.front().endTime + 1))
		{
			m_activeFallingNotes.pop_front();
		}

		// Add new notes that are starting now
		while (m_nextFallingNoteIndex < m_midiNotes.size() && currentTime >= m_midiNotes[m_nextFallingNoteIndex].startTime - m_fallingTime_s)
		{
			m_activeFallingNotes.push_back(m_midiNotes[m_nextFallingNoteIndex]);
			++m_nextFallingNoteIndex;
		}
	}
}

void VirtualPiano::renderVirtualKeyboard(void)
{
	auto& vpd = m_vPianoData;
	int whiteKeyCount = 0;
	for (int midiNote = 21; midiNote <= 108; ++midiNote)
	{
		int noteInOctave = midiNote % 12;
		if (MidiParser::NoteInfo::isWhiteKey(noteInOctave)) // Draw white key
		{
			auto info = MidiParser::getNoteInfo(midiNote);
			PianoKey pk = m_pianoKeys[info.keyIndex];
			ostd::Color keyColor = (pk.pressed ? vpd.whiteKeyPressedColor : vpd.whiteKeyColor);
			float x = vpd.vpx() + (whiteKeyCount * vpd.whiteKey_w());
			float y = vpd.vpy();
			m_parentWindow.outlinedRect({ x, y, vpd.whiteKey_w(), vpd.whiteKey_h() }, keyColor, { 0, 0, 0 }, 1 );
			// m_gfx.outlinedRect({ x, y, vpd.whiteKeyWidth, vpd.whiteKeyHeight }, keyColor, { 0, 0, 0 }, 1 );
			whiteKeyCount++;
		}
	}
	m_parentWindow.outlinedRect({ vpd.vpx(), vpd.vpy() - 2, (float)m_parentWindow.getWindowWidth(), 2 }, { 60, 10, 10 }, { 60, 0, 0 }, 1);
	m_parentWindow.outlinedRect({ vpd.vpx(), vpd.vpy(), (float)m_parentWindow.getWindowWidth(), 5 }, { 160, 10, 10 }, { 210, 0, 0 }, 1);
	whiteKeyCount = 0;
	for (int midiNote = 21; midiNote <= 108; ++midiNote)
	{
		int noteInOctave = midiNote % 12;
		if (MidiParser::NoteInfo::isWhiteKey(noteInOctave))
		{
			whiteKeyCount++;
		}
		else // Draw black key
		{
			auto info = MidiParser::getNoteInfo(midiNote);
			PianoKey pk = m_pianoKeys[info.keyIndex];
			ostd::Color keyColor = (pk.pressed ? vpd.blackKeyPressedColor : vpd.blackKeyColor);
			float x = vpd.vpx() + ((whiteKeyCount - 1) * vpd.whiteKey_w() + (vpd.whiteKey_w() - vpd.blackKey_w() / 2.0f)) - vpd.blackKey_offset();
			float y = vpd.vpy();
			m_parentWindow.outlinedRoundedRect({ x, y, vpd.blackKey_w(), vpd.blackKey_h() }, keyColor, { 0, 0, 0 }, { 0, 0, 5, 5 }, 1);
		}
	}
}

void VirtualPiano::renderFallingNotes(void)
{
	double currentTime = getPlayTime_s(); // in seconds

	ostd::Color fallingWhiteNoteColor = { 60, 160, 255 };
	ostd::Color fallingWhiteNoteOutlineColor = { 30, 80, 225 };
	ostd::Color fallingBlackNoteColor = { 30, 80, 150 };
	ostd::Color fallingBlackNoteOutlineColor = { 15, 40, 100 };

	float shrinkWhiteKey = 16;

	for (auto& note : m_activeFallingNotes)
	{
		auto noteInfo = MidiParser::getNoteInfo(note.pitch);
		if (noteInfo.isBlackKey()) continue;

		double h = note.duration * m_vPianoData.pps();
		double totalTravelTime = m_fallingTime_s + note.duration;
		double elapsedSinceSpawn = (currentTime - (note.startTime - m_fallingTime_s));
		double progress = elapsedSinceSpawn / totalTravelTime;
		progress = std::clamp(progress, 0.0, 1.0);
		double y = -h + static_cast<double>(progress) * (m_vPianoData.vpy() + h);
		double x = m_vPianoData.keyOffsets()[noteInfo.keyIndex] + (shrinkWhiteKey / 2.0f);

		if (y >= m_vPianoData.vpy())
		{
			auto& key = m_pianoKeys[noteInfo.keyIndex];
			key.pressed = false;
			NoteEventData ned(key);
			ned.eventType = NoteEventData::eEventType::NoteOFF;
			ned.note = note;
			ostd::SignalHandler::emitSignal(NoteOffSignal, ostd::tSignalPriority::RealTime, ned);
		}
		else if (y + h >= m_vPianoData.vpy())
		{
			auto& key = m_pianoKeys[noteInfo.keyIndex];
			key.pressed = true;
			NoteEventData ned(key);
			ned.eventType = NoteEventData::eEventType::NoteON;
			ned.note = note;
			ostd::SignalHandler::emitSignal(NoteOnSignal, ostd::tSignalPriority::RealTime, ned);
			if (!m_firstNotePlayed)
			{
				ostd::SignalHandler::emitSignal(MidiStartSignal, ostd::tSignalPriority::RealTime, ned);
				m_firstNotePlayed = true;
			}
		}
		// constexpr float minAlpha = 30.f;
    	// constexpr float maxAlpha = 255.f;
		// float alpha = minAlpha + (note.velocity / 127.f) * (maxAlpha - minAlpha);
		// fallingWhiteNoteColor.a = (uint8_t)alpha;
		// fallingWhiteNoteOutlineColor.a = (uint8_t)alpha;



		ostd::Color glowColor { 200, 80, 120, 255 }; 

		noteShader.setUniform("u_size", sf::Vector2f(m_vPianoData.whiteKey_w() - shrinkWhiteKey, h));
		noteShader.setUniform("u_radius", 10);
		noteShader.setUniform("u_fillColor", sf::Glsl::Vec4(
			glowColor.r / 255.f,
			glowColor.g / 255.f,
			glowColor.b / 255.f,
			glowColor.a / 255.f
		));
		noteShader.setUniform("u_glowSize", 8.f); // glow thickness in px
		noteShader.setUniform("u_position", sf::Vector2f((float)x, (float)y));

		m_parentWindow.outlinedRoundedRect({ static_cast<float>(x), static_cast<float>(y), m_vPianoData.whiteKey_w() - shrinkWhiteKey, static_cast<float>(h) }, fallingWhiteNoteColor, fallingWhiteNoteOutlineColor, { 10, 10, 10, 10 }, 1);
	}

	for (auto& note : m_activeFallingNotes)
	{
		auto noteInfo = MidiParser::getNoteInfo(note.pitch);
		if (noteInfo.isWhiteKey()) continue;
		
		double h = note.duration * m_vPianoData.pps();
		double totalTravelTime = m_fallingTime_s + note.duration;
		double elapsedSinceSpawn = (currentTime - (note.startTime - m_fallingTime_s));
		double progress = elapsedSinceSpawn / totalTravelTime;
		progress = std::clamp(progress, 0.0, 1.0);
		float y = -h + static_cast<float>(progress) * (m_vPianoData.vpy() + h);
		float x = m_vPianoData.keyOffsets()[noteInfo.keyIndex];

		if (y >= m_vPianoData.vpy())
		{
			auto& key = m_pianoKeys[noteInfo.keyIndex];
			key.pressed = false;
			NoteEventData ned(key);
			ned.eventType = NoteEventData::eEventType::NoteOFF;
			ned.note = note;
			ostd::SignalHandler::emitSignal(NoteOffSignal, ostd::tSignalPriority::RealTime, ned);
		}
		else if (y + h >= m_vPianoData.vpy())
		{
			auto& key = m_pianoKeys[noteInfo.keyIndex];
			key.pressed = true;
			NoteEventData ned(key);
			ned.eventType = NoteEventData::eEventType::NoteON;
			ned.note = note;
			ostd::SignalHandler::emitSignal(NoteOnSignal, ostd::tSignalPriority::RealTime, ned);
			if (!m_firstNotePlayed)
			{
				ostd::SignalHandler::emitSignal(MidiStartSignal, ostd::tSignalPriority::RealTime, ned);
				m_firstNotePlayed = true;
			}
		}
		// constexpr float minAlpha = 30.f;
    	// constexpr float maxAlpha = 255.f;
		// float alpha = minAlpha + (note.velocity / 127.f) * (maxAlpha - minAlpha);
		// fallingBlackNoteColor.a = (uint8_t)alpha;
		// fallingBlackNoteOutlineColor.a = (uint8_t)alpha;



		ostd::Color glowColor { 200, 80, 120 }; 

		noteShader.setUniform("u_size", sf::Vector2f(m_vPianoData.blackKey_w(), h));
		noteShader.setUniform("u_radius", 10);
		noteShader.setUniform("u_fillColor", sf::Glsl::Vec4(
			glowColor.r / 255.f,
			glowColor.g / 255.f,
			glowColor.b / 255.f,
			glowColor.a / 255.f
		));
		noteShader.setUniform("u_glowSize", 8.f); // glow thickness in px
		noteShader.setUniform("u_position", sf::Vector2f((float)x, (float)y));
		m_parentWindow.outlinedRoundedRect({ static_cast<float>(x), static_cast<float>(y), m_vPianoData.blackKey_w(), static_cast<float>(h) }, fallingBlackNoteColor, fallingBlackNoteOutlineColor, { 10, 10, 10, 10 }, 1);
	}
}

float VirtualPiano::scanMusicStartPoint(const ostd::String& filePath, float thresholdPercent, float minDuration)
{
	sf::SoundBuffer buffer;
	if (!buffer.loadFromFile(filePath))
	{
		OX_ERROR("Invalid audio file: %s", filePath.c_str());
		return 0.0f;
	}
	const int16_t* samples = buffer.getSamples();
    std::size_t sampleCount  = buffer.getSampleCount();
    unsigned int channels    = buffer.getChannelCount();
    unsigned int sampleRate  = buffer.getSampleRate();

    // Convert percentage to raw PCM units
    const float fullScale = 32767.f;
    const float threshold = thresholdPercent * fullScale;
	const std::size_t hop = 256; 

    // Number of samples in the sustain window (per channel)
    std::size_t windowSamples = static_cast<std::size_t>(minDuration * sampleRate);

    for (std::size_t i = 0; i + windowSamples * channels < sampleCount; i += hop * channels)
    {
        // Compute RMS over the window
        double sumSquares = 0.0;
        for (std::size_t j = 0; j < windowSamples; ++j)
        {
            for (unsigned int c = 0; c < channels; ++c)
            {
                float s = static_cast<float>(samples[(i + j * channels) + c]);
                sumSquares += s * s;
            }
        }

        double meanSquare = sumSquares / (windowSamples * channels);
        double rms = std::sqrt(meanSquare);

        if (rms > threshold)
        {
            // Found the first sustained sound
            return static_cast<float>(i / channels) / sampleRate;
        }
    }

    return 0.f; // No sound found above threshold
}

sf::VertexArray VirtualPiano::getMusicWaveForm(const ostd::String& filePath, int32_t windowHeight)
{
	sf::SoundBuffer buffer;
	if (!buffer.loadFromFile(filePath.cpp_str()))
	{
		OX_ERROR("Error while trying to load audio file: %s", filePath.c_str());
	}
	const int16_t* samples = buffer.getSamples();
	std::size_t sampleCount = buffer.getSampleCount();
	unsigned int channels = buffer.getChannelCount();
	std::vector<float> amplitudes;
	amplitudes.reserve(1000);
	std::size_t samplesPerPixel = sampleCount / channels / 1000;
	for (std::size_t i = 0; i < sampleCount; i += samplesPerPixel * channels)
	{
		long sum = 0;
		for (unsigned int c = 0; c < channels; ++c)
			sum += std::abs(samples[i + c]);
		amplitudes.push_back(static_cast<float>(sum) / channels / 32768.f);
	}
	sf::VertexArray waveform(sf::PrimitiveType::LineStrip, amplitudes.size());
	for (std::size_t x = 0; x < amplitudes.size(); ++x) {
		float y = (1.f - amplitudes[x]) * windowHeight / 2.f;
		waveform[x].position = sf::Vector2f(static_cast<float>(x), y);
		waveform[x].color = sf::Color::White;
	}
	return waveform;
}