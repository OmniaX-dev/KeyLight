#include "Window.hpp"
#include <ostd/Logger.hpp>

void Window::onInitialize(void)	
{
	enableSignals();
	connectSignal(ostd::tBuiltinSignals::KeyReleased);

	enableResizable(true);

	SDL_SetWindowMinimumSize(m_window, 1, 1);
	SDL_SetWindowPosition(m_window, 30, 30);
	SDL_MaximizeWindow(m_window);

	m_gfx.init(*this);
	m_gfx.setFont("res/ttf/Courier Prime.ttf");

	m_vPianoData = VirtualPianoData::defaults();
	m_vPianoData.scaleFromBaseWidth(getWindowWidth(), getWindowHeight());

	loadMidiFile("res/midi/testMidiFile3.mid");
	for (auto& note : m_midiNotes)
	{
		note.startTime += m_fallingTime_s;
		note.endTime   += m_fallingTime_s;
	}

	std::sort(m_midiNotes.begin(), m_midiNotes.end());
	
	m_playing = false;

	for (int midiNote = 21; midiNote <= 108; ++midiNote)
	{
		PianoKey pk;
		pk.noteInfo = MidiParser::getNoteInfo(midiNote);
		pk.pressed = false;
		m_pianoKeys.push_back(pk);
	}

	connectSignal(ostd::tBuiltinSignals::WindowResized);
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
	else if (signal.ID == ostd::tBuiltinSignals::WindowResized)
	{
		auto& evtData = (ogfx::WindowResizedData&)signal.userData;
		m_vPianoData.scaleByWindowSize(evtData.old_width, evtData.old_height, evtData.new_width, evtData.new_height);
	}
}

void Window::onRender(void)
{
	double currentTime = getPlayTime_s(); // in seconds
	ostd::Color fallingNoteColor = { 100, 10, 20 };
	ostd::Color fallingNoteOutlineColor = { 225, 225, 225 };

	for (auto& note : m_activeFallingNotes)
	{
		auto noteInfo = MidiParser::getNoteInfo(note.pitch);
		if (noteInfo.isBlackKey()) continue;
		
		float h = static_cast<float>(note.duration / m_fallingTime_s * (float)getWindowHeight());
		
		double totalTravelTime = m_fallingTime_s + note.duration;
		double elapsedSinceSpawn = (currentTime - (note.startTime - m_fallingTime_s));

		double progress = elapsedSinceSpawn / totalTravelTime;
		progress = std::clamp(progress, 0.0, 1.0);

		// Distance from spawn point (-h) to final point (virtualPianoY)
		float y = -h + static_cast<float>(progress) * (m_vPianoData.virtualPiano_y + h);


		// double timeUntilHit = note.startTime - currentTime;
		// double progress = (m_fallingTime_s - timeUntilHit) / m_fallingTime_s;
		// progress = std::clamp(progress, 0.0, 1.0);
		// float y = -h + static_cast<float>(progress) * (m_vPianoData.virtualPiano_y);

		float x = m_vPianoData.keyOffsets[noteInfo.keyIndex];

		m_gfx.outlinedRect({ x, y, m_vPianoData.whiteKeyWidth, h }, fallingNoteColor, fallingNoteOutlineColor, 1);
		m_gfx.drawString(noteInfo.name.new_add(noteInfo.octave), { x, y }, { 255, 255, 255 }, 24);
	}

	drawVirtualPiano(m_vPianoData);

	ostd::String fps_text = "FPS: ";
	fps_text.add(getFPS());
	m_gfx.drawString(fps_text, { 10, 10 }, { 220, 170, 0 }, 16);
}

void Window::onFixedUpdate(void)
{
}

void Window::onUpdate(void)
{

	if (m_playing)
	{
		double currentTime = getPlayTime_s(); // in seconds

		// Remove notes that have ended
		while (!m_activeNotes.empty() && m_activeNotes.front().endTime <= currentTime)
		{
			auto noteInfo = MidiParser::getNoteInfo(m_activeNotes.front().pitch);
			m_pianoKeys[noteInfo.keyIndex].pressed = false;
			m_activeNotes.pop_front();
		}

		// Add new notes that are starting now
		while (m_nextNoteIndex < m_midiNotes.size() && m_midiNotes[m_nextNoteIndex].startTime <= currentTime)
		{
			if (m_midiNotes[m_nextNoteIndex].endTime > currentTime)
			{
				auto noteInfo = MidiParser::getNoteInfo(m_midiNotes[m_nextNoteIndex].pitch);
				m_pianoKeys[noteInfo.keyIndex].pressed = true;
				m_activeNotes.push_back(m_midiNotes[m_nextNoteIndex]);
			}
			++m_nextNoteIndex;
		}



		// Remove notes that have ended
		while (!m_activeFallingNotes.empty() && currentTime > m_activeFallingNotes.front().endTime)
		{
			// auto noteInfo = MidiParser::getNoteInfo(m_activeFallingNotes.front().pitch);
			m_activeFallingNotes.pop_front();
		}

		// Add new notes that are starting now
		while (m_nextFallingNoteIndex < m_midiNotes.size() && currentTime >= m_midiNotes[m_nextFallingNoteIndex].startTime - m_fallingTime_s)
		{
			// auto noteInfo = MidiParser::getNoteInfo(m_midiNotes[m_nextFallingNoteIndex].pitch);
			m_activeFallingNotes.push_back(m_midiNotes[m_nextFallingNoteIndex]);
			++m_nextFallingNoteIndex;
		}


	}
}

void Window::play(void)
{
	m_playing = true;
	m_startTimeOffset_ns = getCurrentTIme_ns();
	m_nextNoteIndex = 0;
	m_nextFallingNoteIndex = 0;
	m_activeNotes.clear();
	m_activeFallingNotes.clear();
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
	int whiteKeyCount = 0;
	for (int midiNote = 21; midiNote <= 108; ++midiNote)
	{
		int noteInOctave = midiNote % 12;
		if (MidiParser::NoteInfo::isWhiteKey(noteInOctave)) // Draw white key
		{
			auto info = MidiParser::getNoteInfo(midiNote);
			PianoKey pk = m_pianoKeys[info.keyIndex];
			ostd::Color keyColor = (pk.pressed ? vpd.whiteKeyPressedColor : vpd.whiteKeyColor);
			float x = vpd.virtualPiano_x + (whiteKeyCount * vpd.whiteKeyWidth);
			float y = vpd.virtualPiano_y;
			m_gfx.outlinedRect({ x, y, vpd.whiteKeyWidth, vpd.whiteKeyHeight }, keyColor, { 0, 0, 0 }, 1 );
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
			float x = vpd.virtualPiano_x + ((whiteKeyCount - 1) * vpd.whiteKeyWidth + (vpd.whiteKeyWidth - vpd.blackKeyWidth / 2.0f)) - vpd.blackKeyOffset;
			float y = vpd.virtualPiano_y;
			m_gfx.outlinedRect({ x, y, vpd.blackKeyWidth, vpd.blackKeyHeight }, keyColor, { 0, 0, 0 }, 1);
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