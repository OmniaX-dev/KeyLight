#include "Window.hpp"
#include <ostd/Logger.hpp>

#define sf_color(ostd_color) sf::Color { ostd_color.r, ostd_color.g, ostd_color.b, ostd_color.a }

Window::VirtualPianoData Window::VirtualPianoData::defaults(void)
{
	VirtualPianoData vpd;
	vpd.whiteKeyWidth = 40;
	vpd.whiteKeyHeight = vpd.whiteKeyWidth * 8;
	vpd.blackKeyWidth = 22;
	vpd.blackKeyHeight = vpd.blackKeyWidth * 9;
	vpd.blackKeyOffset = 4;
	vpd.whiteKeyColor = { 245, 245, 245 };
	vpd.whiteKeyPressedColor = { 120, 120, 210 };
	vpd.blackKeyColor = { 0, 0, 0 };
	vpd.blackKeyPressedColor = { 20, 20, 90 };
	vpd.virtualPiano_x =  0.0f;
	vpd.virtualPiano_y = VirtualPianoData::base_height - vpd.whiteKeyHeight;
	vpd.recalculateKeyOffsets();
	return vpd;
}

void Window::VirtualPianoData::scaleFromBaseWidth(int32_t new_width, int32_t new_height)
{
	scaleByWindowSize(base_width, base_height, new_width, new_height);
}

void Window::VirtualPianoData::scaleByWindowSize(int32_t old_width, int32_t old_height, int32_t new_width, int32_t new_height)
{
	float width_ratio = (float)new_width / (float)old_width;
	float height_ratio = (float)new_height / (float)old_height;

	whiteKeyWidth *= width_ratio;
	whiteKeyHeight *= height_ratio;
	blackKeyWidth *= width_ratio;
	blackKeyHeight *= height_ratio;
	blackKeyOffset *= width_ratio;

	virtualPiano_x *= width_ratio;
	virtualPiano_y *= height_ratio;

	recalculateKeyOffsets();
}

void Window::VirtualPianoData::recalculateKeyOffsets(void)
{
	keyOffsets.clear();
	int whiteKeyCount = 0;
	for (int midiNote = 21; midiNote <= 108; ++midiNote)
	{
		int noteInOctave = midiNote % 12;
		int keyIndex = midiNote - 21;
		if (MidiParser::NoteInfo::isWhiteKey(noteInOctave))
		{
			float x = virtualPiano_x + (whiteKeyCount * whiteKeyWidth);
			keyOffsets[keyIndex] = x;
			whiteKeyCount++;
		}
		else // black keys
		{
			float x = virtualPiano_x + ((whiteKeyCount - 1) * whiteKeyWidth + (whiteKeyWidth - blackKeyWidth / 2.0f)) - blackKeyOffset;
			keyOffsets[keyIndex] = x;
		}
	}
}




void Window::onInitialize(void)	
{
	enableSignals();
	connectSignal(ostd::tBuiltinSignals::KeyReleased);
	connectSignal(NoteOnSignal);
	connectSignal(NoteOffSignal);
	sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
	m_window.create(desktop, getTitle().cpp_str(), sf::Style::Default);
	m_window.setPosition({ 30, 30 });

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
		auto& evtData = (KeyEventData&)signal.userData;
		if (evtData.keyCode == (int32_t)sf::Keyboard::Key::Escape)
			close();
		if (evtData.keyCode == (int32_t)sf::Keyboard::Key::Space)
		{
			if (!m_playing)
				play();
		}
	}
	else if (signal.ID == ostd::tBuiltinSignals::WindowResized)
	{
		auto& evtData = (WindowResizedData&)signal.userData;
		m_vPianoData.scaleByWindowSize(evtData.old_width, evtData.old_height, evtData.new_width, evtData.new_height);
	}
}

void Window::onRender(void)
{
	double currentTime = getPlayTime_s(); // in seconds

	ostd::Color fallingWhiteNoteColor = { 100, 10, 20 };
	ostd::Color fallingWhiteNoteOutlineColor = { 225, 225, 225 };
	ostd::Color fallingBlackNoteColor = { 50, 10, 30 };
	ostd::Color fallingBlackNoteOutlineColor = { 30, 30, 30 };

	const double pixelsPerSecond = 250.0;

	for (auto& note : m_activeFallingNotes)
	{
		auto noteInfo = MidiParser::getNoteInfo(note.pitch);
		if (noteInfo.isBlackKey()) continue;

		double h = note.duration * pixelsPerSecond;
		double totalTravelTime = m_fallingTime_s + note.duration;
		double elapsedSinceSpawn = (currentTime - (note.startTime - m_fallingTime_s));
		double progress = elapsedSinceSpawn / totalTravelTime;
		progress = std::clamp(progress, 0.0, 1.0);
		double y = -h + static_cast<double>(progress) * (m_vPianoData.virtualPiano_y + h);
		double x = m_vPianoData.keyOffsets[noteInfo.keyIndex];

		if (y >= m_vPianoData.virtualPiano_y)
		{
			auto& key = m_pianoKeys[noteInfo.keyIndex];
			key.pressed = false;
			NoteEventData ned(key);
			ned.eventType = NoteEventData::eEventType::NoteOFF;
			ned.note = note;
			ostd::SignalHandler::emitSignal(NoteOffSignal, ostd::tSignalPriority::RealTime, ned);
		}
		else if (y + h >= m_vPianoData.virtualPiano_y)
		{
			auto& key = m_pianoKeys[noteInfo.keyIndex];
			key.pressed = true;
			NoteEventData ned(key);
			ned.eventType = NoteEventData::eEventType::NoteON;
			ned.note = note;
			ostd::SignalHandler::emitSignal(NoteOnSignal, ostd::tSignalPriority::RealTime, ned);
		}

		// m_gfx.outlinedRect({ static_cast<float>(x), static_cast<float>(y), m_vPianoData.whiteKeyWidth, static_cast<float>(h) }, fallingWhiteNoteColor, fallingWhiteNoteOutlineColor, 1);
		outlinedRect({ static_cast<float>(x), static_cast<float>(y), m_vPianoData.whiteKeyWidth, static_cast<float>(h) }, fallingWhiteNoteColor, fallingWhiteNoteOutlineColor, 1);
	}

	for (auto& note : m_activeFallingNotes)
	{
		auto noteInfo = MidiParser::getNoteInfo(note.pitch);
		if (noteInfo.isWhiteKey()) continue;
		
		double h = note.duration * pixelsPerSecond;
		double totalTravelTime = m_fallingTime_s + note.duration;
		double elapsedSinceSpawn = (currentTime - (note.startTime - m_fallingTime_s));
		double progress = elapsedSinceSpawn / totalTravelTime;
		progress = std::clamp(progress, 0.0, 1.0);
		float y = -h + static_cast<float>(progress) * (m_vPianoData.virtualPiano_y + h);
		float x = m_vPianoData.keyOffsets[noteInfo.keyIndex];

		if (y >= m_vPianoData.virtualPiano_y)
		{
			auto& key = m_pianoKeys[noteInfo.keyIndex];
			key.pressed = false;
			NoteEventData ned(key);
			ned.eventType = NoteEventData::eEventType::NoteOFF;
			ned.note = note;
			ostd::SignalHandler::emitSignal(NoteOffSignal, ostd::tSignalPriority::RealTime, ned);
		}
		else if (y + h >= m_vPianoData.virtualPiano_y)
		{
			auto& key = m_pianoKeys[noteInfo.keyIndex];
			key.pressed = true;
			NoteEventData ned(key);
			ned.eventType = NoteEventData::eEventType::NoteON;
			ned.note = note;
			ostd::SignalHandler::emitSignal(NoteOnSignal, ostd::tSignalPriority::RealTime, ned);
		}

		// m_gfx.outlinedRect({ static_cast<float>(x), static_cast<float>(y), m_vPianoData.blackKeyWidth, static_cast<float>(h) }, fallingBlackNoteColor, fallingBlackNoteOutlineColor, 1);
		outlinedRect({ static_cast<float>(x), static_cast<float>(y), m_vPianoData.blackKeyWidth, static_cast<float>(h) }, fallingBlackNoteColor, fallingBlackNoteOutlineColor, 1);
	}

	drawVirtualPiano(m_vPianoData);

	ostd::String fps_text = "FPS: ";
	fps_text.add(getFPS());
	// m_gfx.drawString(fps_text, { 10, 10 }, { 220, 170, 0 }, 16);
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

void Window::outlinedRect(const ostd::Rectangle& rect, const ostd::Color& fillColor, const ostd::Color& outlineColor, int32_t outlineThickness)
{
	m_sf_rect.setSize({ rect.w, rect.h });
	m_sf_rect.setPosition({ rect.x, rect.y });
	m_sf_rect.setFillColor(sf_color(fillColor));
	m_sf_rect.setOutlineColor(sf_color(outlineColor));
	m_sf_rect.setOutlineThickness(outlineThickness);
	m_window.draw(m_sf_rect);
}

void Window::play(void)
{
	m_playing = true;
	m_startTimeOffset_ns = getCurrentTIme_ns();
	m_nextFallingNoteIndex = 0;
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
			outlinedRect({ x, y, vpd.whiteKeyWidth, vpd.whiteKeyHeight }, keyColor, { 0, 0, 0 }, 1 );
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
			float x = vpd.virtualPiano_x + ((whiteKeyCount - 1) * vpd.whiteKeyWidth + (vpd.whiteKeyWidth - vpd.blackKeyWidth / 2.0f)) - vpd.blackKeyOffset;
			float y = vpd.virtualPiano_y;
			outlinedRect({ x, y, vpd.blackKeyWidth, vpd.blackKeyHeight }, keyColor, { 0, 0, 0 }, 1);
			// m_gfx.outlinedRect({ x, y, vpd.blackKeyWidth, vpd.blackKeyHeight }, keyColor, { 0, 0, 0 }, 1);
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