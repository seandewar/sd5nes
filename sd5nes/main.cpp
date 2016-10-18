#include <cstdlib>
#include <iostream>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/Font.hpp>

#include "NESEmulationConstants.h"
#include "NESEmulator.h"


int main(int argc, char* argv[])
{
    // Input ROM path from command-line or from stdin if
    // no args given
    std::string romPath;
    if (argc > 1) {
        romPath = std::string(argv[1]);
    }
    else {
        std::cout << "Input ROM path: ";
        std::cin >> romPath;
    }

	// Init the window.
	sf::RenderWindow window(sf::VideoMode(NES_EMU_DEFAULT_WINDOW_WIDTH, NES_EMU_DEFAULT_WINDOW_HEIGHT), "SD5 NES");
	window.setFramerateLimit(60);

	// @TODO: DEBUG!
	sf::Font font;
	font.loadFromFile("font.ttf");

	NESEmulator emu(window, font);

	NESStandardController controller;
    controller.SetUpDownOrLeftRightAllowed(true);
	emu.AddController(NESControllerPort::CONTROLLER_1, controller);

	emu.LoadROM(romPath);

	// Main loop.
	while (window.isOpen())
	{
		// Handle events
		sf::Event event;
		while (window.pollEvent(event))
		{
			switch (event.type)
			{
			// Window closed.
			case sf::Event::Closed:
				window.close();
				break;
			}
		}

		// Update controller state if window is in focus, otherwise reset it.
		if (!window.hasFocus())
			controller.ResetButtonStates();
		else
		{
			controller.SetButtonState(NESControllerButton::A, sf::Keyboard::isKeyPressed(sf::Keyboard::X));
			controller.SetButtonState(NESControllerButton::B, sf::Keyboard::isKeyPressed(sf::Keyboard::Z));
			controller.SetButtonState(NESControllerButton::SELECT, sf::Keyboard::isKeyPressed(sf::Keyboard::RShift));
			controller.SetButtonState(NESControllerButton::START, sf::Keyboard::isKeyPressed(sf::Keyboard::Return));
			controller.SetButtonState(NESControllerButton::UP, sf::Keyboard::isKeyPressed(sf::Keyboard::Up));
			controller.SetButtonState(NESControllerButton::DOWN, sf::Keyboard::isKeyPressed(sf::Keyboard::Down));
			controller.SetButtonState(NESControllerButton::LEFT, sf::Keyboard::isKeyPressed(sf::Keyboard::Left));
			controller.SetButtonState(NESControllerButton::RIGHT, sf::Keyboard::isKeyPressed(sf::Keyboard::Right));
		}

		window.clear();
		emu.Frame();
		window.display();
	}

	return EXIT_SUCCESS;
}
