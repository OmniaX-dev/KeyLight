#include "Window.hpp"
#include <ostd/Logger.hpp>

void Window::onInitialize(void)	
{ 
	enableSignals();
	connectSignal(ostd::tBuiltinSignals::KeyReleased);

	m_gfx.init(*this);
	//m_gfx.setFont("res/ttf/Courier Prime.ttf");

	m_vPianoData = VirtualPianoData::defaults();

	loadMidiFile("testMidiFile4.mid");
	std::sort(m_midiNotes.begin(), m_midiNotes.end());
	m_playing = false;

	for (int midiNote = 21; midiNote <= 108; ++midiNote)
	{
		PianoKey pk;
		pk.noteInfo = MidiParser::getNoteInfo(midiNote);
		pk.pressed = false;
		m_pianoKeys.push_back(pk);
	}
}
	
void Window::handleSignal(ostd::tSignal& signal)
{
	if (signal.ID == ostd::tBuiltinSignals::KeyReleased)
	{
		auto& evtData = (ogfx::KeyEventData&)signal.userData;
		if (evtData.keyCode == SDLK_ESCAPE)
			close();
		if (evtData.keyCode == SDLK_SPACE)
		{
			if (!m_playing)
				play();
		}
	}
}

void Window::onRender(void)
{
	drawVirtualPiano(m_vPianoData);
}

void Window::onFixedUpdate(void)
{
	std::cout << (int)getFPS() << "\n";
}

void Window::onUpdate(void)
{

	if (m_playing)
	{
		double currentTime = getPlayTime_s(); // in seconds

		// Remove notes that have ended
		while (!m_activeNotes.empty() && m_activeNotes.front().endTime <= currentTime) {
			auto noteInfo = MidiParser::getNoteInfo(m_activeNotes.front().pitch);
			m_pianoKeys[noteInfo.keyIndex].pressed = false;
			m_activeNotes.pop_front();
		}

		// Add new notes that are starting now
		while (m_nextNoteIndex < m_midiNotes.size() && m_midiNotes[m_nextNoteIndex].startTime <= currentTime) {
			if (m_midiNotes[m_nextNoteIndex].endTime > currentTime) {
				auto noteInfo = MidiParser::getNoteInfo(m_midiNotes[m_nextNoteIndex].pitch);
				m_pianoKeys[noteInfo.keyIndex].pressed = true;
				m_activeNotes.push_back(m_midiNotes[m_nextNoteIndex]);
			}
			++m_nextNoteIndex;
		}
	}
}

void Window::play(void)
{
	if (m_playing) return;
	m_playing = true;
	m_startTimeOffset_ns = getCurrentTIme_ns();
	m_nextNoteIndex = 0;
	m_activeNotes.clear();
}

double Window::getCurrentTIme_ns(void)
{
	return std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

double Window::getPlayTime_s(void)
{
	double playTime = getCurrentTIme_ns() - m_startTimeOffset_ns;
	return playTime * 1e-9;
}

void Window::drawVirtualPiano(const VirtualPianoData& vpd)
{
	auto isWhiteKey = [](int noteInOctave) {
		return noteInOctave == 0 || noteInOctave == 2 || noteInOctave == 4 ||
			noteInOctave == 5 || noteInOctave == 7 || noteInOctave == 9 ||
			noteInOctave == 11;
	};

	// Black key positions in the 12-note scale (A0 to G#)
	const int blackKeyOffsets[5] = { 1, 4, 6, 9, 11 };

	int whiteKeyCount = 0;
	for (int midiNote = 21; midiNote <= 108; ++midiNote)
	{
		int noteInOctave = midiNote % 12;
		if (isWhiteKey(noteInOctave)) // Draw white key
		{
			auto info = MidiParser::getNoteInfo(midiNote);
			PianoKey pk = m_pianoKeys[info.keyIndex];
			ostd::Color keyColor = (pk.pressed ? vpd.whiteKeyPressedColor : vpd.whiteKeyColor);
			m_gfx.outlinedRect({ (whiteKeyCount * vpd.whiteKeyWidth), (float)getWindowHeight() - vpd.whiteKeyHeight, vpd.whiteKeyWidth, vpd.whiteKeyHeight }, keyColor, { 0, 0, 0 }, 1 );
			whiteKeyCount++;
		}
	}
	whiteKeyCount = 0;
	for (int midiNote = 21; midiNote <= 108; ++midiNote)
	{
		int noteInOctave = midiNote % 12;
		if (isWhiteKey(noteInOctave))
		{
			whiteKeyCount++;
		}
		else // Draw black key
		{
			auto info = MidiParser::getNoteInfo(midiNote);
			PianoKey pk = m_pianoKeys[info.keyIndex];
			ostd::Color keyColor = (pk.pressed ? vpd.blackKeyPressedColor : vpd.blackKeyColor);
			float x = (whiteKeyCount - 1) * vpd.whiteKeyWidth + (vpd.whiteKeyWidth - vpd.blackKeyWidth / 2.0f);
			float y = (float)getWindowHeight() - vpd.whiteKeyHeight;
			m_gfx.outlinedRect({ x - vpd.blackKeyOffset, y, vpd.blackKeyWidth, vpd.blackKeyHeight }, keyColor, { 0, 0, 0 }, 1);
		}
	}
}

bool Window::loadMidiFile(const ostd::String& filePath)
{
	try
	{
		m_midiNotes = MidiParser::parseFile(filePath);
		OX_DEBUG("loaded <%s>: total notes parsed: %d", filePath.c_str(), m_midiNotes.size());
		return true;
    }
    catch (const std::exception& ex) {
		OX_ERROR(ex.what());
        return false;
    }
}