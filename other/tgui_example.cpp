bool Window::buildGUI(tgui::BackendGui& gui)
{
	try
	{
		auto tabs = tgui::Tabs::create();
		tabs->setRenderer(theme.getRenderer("Tabs"));
		tabs->setTabHeight(30);
		tabs->setPosition(70, 40);
		tabs->add("Tab - 1");
		tabs->add("Tab - 2");
		tabs->add("Tab - 3");
		gui.add(tabs);

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

		auto editBox = tgui::EditBox::create();
		editBox->setRenderer(theme.getRenderer("EditBox"));
		editBox->setSize(200, 25);
		editBox->setTextSize(18);
		editBox->setPosition(10, 270);
		editBox->setDefaultText("Click to edit text...");
		gui.add(editBox);

		auto listBox = tgui::ListBox::create();
		listBox->setRenderer(theme.getRenderer("ListBox"));
		listBox->setSize(250, 120);
		listBox->setItemHeight(24);
		listBox->setPosition(10, 340);
		listBox->addItem("Item 1");
		listBox->addItem("Item 2");
		listBox->addItem("Item 3");
		gui.add(listBox);

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
