#include "Window.hpp"
#include "Renderer.hpp"
#include <ostd/Logger.hpp>

void Window::onInitialize(void)
{
	enableSignals();
	connectSignal(ostd::tBuiltinSignals::KeyReleased);
	connectSignal(VirtualPiano::NoteOnSignal);
	connectSignal(VirtualPiano::NoteOffSignal);
	connectSignal(VirtualPiano::MidiStartSignal);
	connectSignal(ostd::tBuiltinSignals::WindowResized);
	connectSignal(WindowFocusLost);
	connectSignal(WindowFocusGained);
	setClearColor({ 255, 10, 10 });

	Renderer::init(*this, "themes/fonts/Courier Prime.ttf");

	m_window.setPosition({ 30, 30 });

	m_windowSizeBeforeFullscreen	 = { (float)getWindowWidth(), (float)getWindowHeight() };
	m_windowPositionBeforeFullscreen = { (float)m_window.getPosition().x, (float)m_window.getPosition().y };
	// enableFullscreen(true);
	m_vpiano.init();
	m_vpiano.loadMidiFile("res/midi/rach.mid");
	m_vpiano.loadAudioFile("res/music/rach.mp3");

	sf::Image cursorImage;
	if (cursorImage.loadFromFile("themes/ui/cursor.png"))
	{
		sf::Cursor tempCursor(cursorImage.getPixelsPtr(), cursorImage.getSize(), sf::Vector2u(0, 0));
		m_cursor = std::move(tempCursor);
		m_window.setMouseCursor(*m_cursor);
	}
	if (m_icon.loadFromFile("themes/ui/icon.png"))
    {
        m_window.setIcon(m_icon.getSize(), m_icon.getPixelsPtr());
    } //TODO: Error
	m_gui.setWindow(m_window);
	buildGUI(m_gui);
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
			if (!m_vpiano.isPlaying())
				m_vpiano.play();
			else
				m_vpiano.pause();
		}
		else if (evtData.keyCode == (int32_t)sf::Keyboard::Key::Enter)
		{
			m_vpiano.stop();
		}
		else if (evtData.keyCode == (int32_t)sf::Keyboard::Key::F11)
		{
			enableFullscreen(!m_isFullscreen);
		}
		else if (evtData.keyCode == (int32_t)sf::Keyboard::Key::F2)
		{
			m_drawGui = !m_drawGui;
		}
	}
	else if (signal.ID == ostd::tBuiltinSignals::WindowResized)
	{
		auto&	 evtData = (WindowResizedData&)signal.userData;
		sf::View view	 = m_window.getView();
		view.setSize({ static_cast<float>(evtData.new_width), static_cast<float>(evtData.new_height) });
		view.setCenter({ evtData.new_width / 2.f, evtData.new_height / 2.f });
		m_window.setView(view);
		m_vpiano.vPianoData().updateScale(evtData.new_width, evtData.new_height);
		m_vpiano.onWindowResized((uint32_t)evtData.new_width, (uint32_t)evtData.new_height);
		__update_local_window_size((uint32_t)evtData.new_width, (uint32_t)evtData.new_height);
	}
	else if (signal.ID == VirtualPiano::MidiStartSignal)
	{
		if (m_vpiano.hasAudioFile())
		{
			m_vpiano.getAudioFile().play();
			m_vpiano.getAudioFile().setPlayingOffset(sf::seconds(m_vpiano.getAutoSoundStart()));
		}
	}
	else if (signal.ID == WindowFocusLost)
	{
		// if (OnWindows)
		// {
		//     HWND hwnd = m_window.getNativeHandle();
		//     SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_SHOWWINDOW);
		//     lastFocusChange = std::chrono::steady_clock::now();
		//     std::cout << "LOST\n";
		// }
	}
	else if (signal.ID == WindowFocusGained)
	{
		// if (OnWindows)
		// {
		//         std::cout << "GAINED\n";
		//     if (std::chrono::steady_clock::now() - lastFocusChange > std::chrono::milliseconds(100))
		//     {
		//         HWND hwnd = m_window.getNativeHandle();
		//         SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_SHOWWINDOW);
		//         lastFocusChange = std::chrono::steady_clock::now();
		//     }
		// }
	}
}

void Window::onEventPoll(const std::optional<sf::Event>& event)
{
	m_gui.handleEvent(*event);
}

void Window::onRender(void)
{
	m_window.clear({ 10, 10, 30 });
	m_vpiano.render();
	ostd::String fps_text = "FPS: ";
	fps_text.add(getFPS());
	Renderer::drawString(fps_text, { 10, 10 }, { 220, 170, 0 }, 24);
	m_gui.draw();
}

void Window::onFixedUpdate(void)
{
}

void Window::onUpdate(void)
{
	m_vpiano.update();
}

void Window::enableFullscreen(bool enable)
{
	auto old_size	  = m_window.getSize();
	auto old_position = m_window.getPosition();
	if (enable)
	{
		m_windowSizeBeforeFullscreen	 = { (float)old_size.x, (float)old_size.y };
		m_windowPositionBeforeFullscreen = { (float)old_position.x, (float)old_position.y };
		m_window.create(sf::VideoMode::getDesktopMode(), getTitle().cpp_str(), sf::Style::None, sf::State::Fullscreen);
		// auto mode = sf::VideoMode::getDesktopMode();
		// m_window.create(sf::VideoMode({ mode.size.x - 1, mode.size.y - 1 }), getTitle().cpp_str(), sf::Style::None, sf::State::Fullscreen);

		// m_window.setPosition({ 1, 1 });
		m_window.setPosition({ 0, 0 });
		m_isFullscreen = true;
		// m_window.setMouseCursorVisible(true);
		// sf::Cursor cursor(sf::Cursor::Type::Arrow);
		// m_window.setMouseCursor(cursor);
	}
	else
	{
		m_window.create(sf::VideoMode({ (uint32_t)m_windowSizeBeforeFullscreen.x, (uint32_t)m_windowSizeBeforeFullscreen.y }), getTitle().cpp_str(), sf::Style::Default);
		m_window.setPosition({ (int32_t)m_windowPositionBeforeFullscreen.x, (int32_t)m_windowPositionBeforeFullscreen.y });
		m_isFullscreen = false;
	}
	m_window.setMouseCursor(*m_cursor);
	auto			  new_size = m_window.getSize();
	WindowResizedData wrd(*this, old_size.x, old_size.y, new_size.x, new_size.y);
	ostd::SignalHandler::emitSignal(ostd::tBuiltinSignals::WindowResized, ostd::tSignalPriority::RealTime, wrd);
}

void Window::drawGUI(void)
{
}

void Window::onFileSelected(const tgui::String& str)
{
	std::cout << "BOOM! " << str << "\n";
}

bool Window::buildGUI(tgui::BackendGui& gui)
{
	try
	{
		gui.setRelativeView({ 0, 0, 0.5f, 0.5f });
		tgui::Theme theme { "themes/Dark.txt" };

		auto fileDialog = tgui::FileDialog::create();
		fileDialog->setTitle("Open File");
		fileDialog->setFileMustExist(true);
		const std::vector<std::pair<tgui::String, std::vector<tgui::String>>> filters {
			{ "Image files", { "*.png", "*.jpg" } }
		};
		fileDialog->setFileTypeFilters(filters);
		fileDialog->setRenderer(theme.getRenderer("FileDialog"));

		fileDialog->onFileSelect([&](const std::filesystem::path& path) {
			std::cout << "Selected file: " << path << '\n';
		});

		gui.add(fileDialog);
		fileDialog->setVisible(false);
		fileDialog->onFileSelect([this](const tgui::Filesystem::Path& path) {
			onFileSelected(path.asString());  // Convert Path to String if needed
		});


		auto tabs = tgui::Tabs::create();
		tabs->setRenderer(theme.getRenderer("Tabs"));
		tabs->setTabHeight(30);
		tabs->setPosition(70, 40);
		tabs->add("Tab - 1");
		tabs->add("Tab - 2");
		tabs->add("Tab - 3");
		gui.add(tabs);

		// auto menu = tgui::MenuBar::create();
		// menu->setRenderer(theme.getRenderer("MenuBar"));
		// menu->setHeight(22.f);
		// menu->addMenu("File");
		// menu->addMenuItem("Load");
		// menu->addMenuItem("Save");
		// menu->addMenuItem("Exit");
		// menu->addMenu("Edit");
		// menu->addMenuItem("Copy");
		// menu->addMenuItem("Paste");
		// menu->addMenu("Help");
		// menu->addMenuItem("About");
		// gui.add(menu);

		// auto label = tgui::Label::create();
		// label->setRenderer(theme.getRenderer("Label"));
		// label->setText("This is a label.\nAnd these are radio buttons:");
		// label->setPosition(10, 90);
		// label->setTextSize(18);
		// gui.add(label);

		auto radioButton = tgui::RadioButton::create();
		radioButton->setRenderer(theme.getRenderer("RadioButton"));
		radioButton->setPosition(20, 140);
		radioButton->setText("Yep!");
		radioButton->setSize(25, 25);
		gui.add(radioButton);

		radioButton = tgui::RadioButton::create();
		radioButton->setRenderer(theme.getRenderer("RadioButton"));
		radioButton->setPosition(20, 170);
		radioButton->setText("Nope!");
		radioButton->setSize(25, 25);
		gui.add(radioButton);

		radioButton = tgui::RadioButton::create();
		radioButton->setRenderer(theme.getRenderer("RadioButton"));
		radioButton->setPosition(20, 200);
		radioButton->setText("Don't know!");
		radioButton->setSize(25, 25);
		gui.add(radioButton);

		// label = tgui::Label::create();
		// label->setRenderer(theme.getRenderer("Label"));
		// label->setText("We've got some edit boxes:");
		// label->setPosition(10, 240);
		// label->setTextSize(18);
		// gui.add(label);

		auto editBox = tgui::EditBox::create();
		editBox->setRenderer(theme.getRenderer("EditBox"));
		editBox->setSize(200, 25);
		editBox->setTextSize(18);
		editBox->setPosition(10, 270);
		editBox->setDefaultText("Click to edit text...");
		gui.add(editBox);

		// label = tgui::Label::create();
		// label->setRenderer(theme.getRenderer("Label"));
		// label->setText("And some list boxes too...");
		// label->setPosition(10, 310);
		// label->setTextSize(18);
		// gui.add(label);

		auto listBox = tgui::ListBox::create();
		listBox->setRenderer(theme.getRenderer("ListBox"));
		listBox->setSize(250, 120);
		listBox->setItemHeight(24);
		listBox->setPosition(10, 340);
		listBox->addItem("Item 1");
		listBox->addItem("Item 2");
		listBox->addItem("Item 3");
		gui.add(listBox);

		// label = tgui::Label::create();
		// label->setRenderer(theme.getRenderer("Label"));
		// label->setText("It's the progress bar below");
		// label->setPosition(10, 470);
		// label->setTextSize(18);
		// gui.add(label);

		auto progressBar = tgui::ProgressBar::create();
		progressBar->setRenderer(theme.getRenderer("ProgressBar"));
		progressBar->setPosition(10, 500);
		progressBar->setSize(200, 20);
		progressBar->setValue(50);
		gui.add(progressBar);

		// label = tgui::Label::create();
		// label->setRenderer(theme.getRenderer("Label"));
		// label->setText(tgui::String::fromNumber(progressBar->getValue()) + "%");
		// label->setPosition(220, 500);
		// label->setTextSize(18);
		// gui.add(label);

		// label = tgui::Label::create();
		// label->setRenderer(theme.getRenderer("Label"));
		// label->setText("That's the slider");
		// label->setPosition(10, 530);
		// label->setTextSize(18);
		// gui.add(label);

		auto slider = tgui::Slider::create();
		slider->setRenderer(theme.getRenderer("Slider"));
		slider->setPosition(10, 560);
		slider->setSize(200, 18);
		slider->setValue(4);
		gui.add(slider);

		auto scrollbar = tgui::Scrollbar::create();
		scrollbar->setRenderer(theme.getRenderer("Scrollbar"));
		scrollbar->setPosition(380, 40);
		scrollbar->setSize(18, 540);
		scrollbar->setMaximum(100);
		scrollbar->setViewportSize(70);
		gui.add(scrollbar);

		auto comboBox = tgui::ComboBox::create();
		comboBox->setRenderer(theme.getRenderer("ComboBox"));
		comboBox->setSize(120, 21);
		comboBox->setPosition(420, 40);
		comboBox->addItem("Item 1");
		comboBox->addItem("Item 2");
		comboBox->addItem("Item 3");
		comboBox->setSelectedItem("Item 2");
		gui.add(comboBox);

		auto child = tgui::ChildWindow::create();
		child->setRenderer(theme.getRenderer("ChildWindow"));
		child->setClientSize({ 250, 120 });
		child->setPosition(420, 80);
		child->setTitle("Child window");
		gui.add(child);

		// label = tgui::Label::create();
		// label->setRenderer(theme.getRenderer("Label"));
		// label->setText("Hi! I'm a child window.");
		// label->setPosition(30, 30);
		// label->setTextSize(15);
		// child->add(label);

		auto button = tgui::Button::create();
		button->setRenderer(theme.getRenderer("Button"));
		button->setPosition(75, 70);
		button->setText("OK");
		button->setSize(100, 30);
		button->onPress([=] { child->setVisible(false); });
		button->onPress([this, fileDialog]() {
			fileDialog->setVisible(true);
		});
		child->add(button);

		auto checkbox = tgui::CheckBox::create();
		checkbox->setRenderer(theme.getRenderer("CheckBox"));
		checkbox->setPosition(420, 240);
		checkbox->setText("Ok, I got it");
		checkbox->setSize(25, 25);
		gui.add(checkbox);

		checkbox = tgui::CheckBox::create();
		checkbox->setRenderer(theme.getRenderer("CheckBox"));
		checkbox->setPosition(570, 240);
		checkbox->setText("No, I didn't");
		checkbox->setSize(25, 25);
		gui.add(checkbox);

		// label = tgui::Label::create();
		// label->setRenderer(theme.getRenderer("Label"));
		// label->setText("Chatbox");
		// label->setPosition(420, 280);
		// label->setTextSize(18);
		// gui.add(label);

		auto chatbox = tgui::ChatBox::create();
		chatbox->setRenderer(theme.getRenderer("ChatBox"));
		chatbox->setSize(300, 100);
		chatbox->setTextSize(18);
		chatbox->setPosition(420, 310);
		chatbox->setLinesStartFromTop();
		chatbox->addLine("texus: Hey, this is TGUI!", tgui::Color::Green);
		chatbox->addLine("Me: Looks awesome! ;)", tgui::Color::Yellow);
		chatbox->addLine("texus: Thanks! :)", tgui::Color::Green);
		chatbox->addLine("Me: The widgets rock ^^", tgui::Color::Yellow);
		gui.add(chatbox);

		button = tgui::Button::create();
		button->setRenderer(theme.getRenderer("Button"));
		button->setPosition(gui.getView().getSize().x - 115.f, gui.getView().getSize().y - 50.f);
		button->setText("Exit");
		button->setSize(100, 40);
		gui.add(button);
	}
	catch (const tgui::Exception& e)
	{
		std::cerr << "TGUI Exception: " << e.what() << std::endl;
		return false;
	}

	return true;
}
