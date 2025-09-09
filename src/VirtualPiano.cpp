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

	for (int midiNote = 21; midiNote <= 108; ++midiNote)
	{
		PianoKey pk;
		pk.noteInfo = MidiParser::getNoteInfo(midiNote);
		pk.pressed = false;
		m_pianoKeys.push_back(pk);
	}
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
	m_playing = false;
	m_paused = true;
}

void VirtualPiano::stop(void)
{
	m_playing = false;
	m_paused = false;
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
			m_parentWindow.outlinedRect({ x, y, vpd.blackKey_w(), vpd.blackKey_h() }, keyColor, { 0, 0, 0 }, 1);
			// m_gfx.outlinedRect({ x, y, vpd.blackKeyWidth, vpd.blackKeyHeight }, keyColor, { 0, 0, 0 }, 1);
		}
	}
}

void VirtualPiano::renderFallingNotes(void)
{
	double currentTime = getPlayTime_s(); // in seconds

	ostd::Color fallingWhiteNoteColor = { 100, 10, 20 };
	ostd::Color fallingWhiteNoteOutlineColor = { 225, 225, 225 };
	ostd::Color fallingBlackNoteColor = { 50, 10, 30 };
	ostd::Color fallingBlackNoteOutlineColor = { 30, 30, 30 };

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
		double x = m_vPianoData.keyOffsets()[noteInfo.keyIndex];

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
		}

		// m_gfx.outlinedRect({ static_cast<float>(x), static_cast<float>(y), m_vPianoData.whiteKeyWidth, static_cast<float>(h) }, fallingWhiteNoteColor, fallingWhiteNoteOutlineColor, 1);
		m_parentWindow.outlinedRect({ static_cast<float>(x), static_cast<float>(y), m_vPianoData.whiteKey_w(), static_cast<float>(h) }, fallingWhiteNoteColor, fallingWhiteNoteOutlineColor, 1);
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
		}

		// m_gfx.outlinedRect({ static_cast<float>(x), static_cast<float>(y), m_vPianoData.blackKeyWidth, static_cast<float>(h) }, fallingBlackNoteColor, fallingBlackNoteOutlineColor, 1);
		m_parentWindow.outlinedRect({ static_cast<float>(x), static_cast<float>(y), m_vPianoData.blackKey_w(), static_cast<float>(h) }, fallingBlackNoteColor, fallingBlackNoteOutlineColor, 1);
	}
}