#include <cstdlib>

#include <SFML\Graphics\RenderWindow.hpp>
#include <SFML\Window\Event.hpp>

#include "NESEmulationConstants.h"
#include "NESEmulator.h"


int main(int argc, char* argv[])
{
	// Init the window.
	sf::RenderWindow window(
		sf::VideoMode(NES_EMU_DEFAULT_WINDOW_WIDTH, NES_EMU_DEFAULT_WINDOW_HEIGHT),
		"SD5 NES"
		);
	
	// @TODO DEBUG!!!!!
	//NESGamePak g;
	//g.LoadROM("smb.nes");
	//NESPPUMemory pm;
	//std::array<NESMemPatternTable, 2> pt;
	//for (int i = 0; i < 0x1000; ++i)
	//{
	//	pt[0].Write8(i, g.GetCharacterROMBanks()[0].Read8(i + 0x0000));
	//	pt[1].Write8(i, g.GetCharacterROMBanks()[0].Read8(i + 0x1000));
	//}
	//pm.patternTables = pt;
	//NESPPUMemoryMapper pmm(pm);
	//NESPPU p(pmm);

	//int a = 6;
	//sf::Image im;
	//sf::Texture tex;
	//sf::Sprite spr;

	NESEmulator emu(window);
	emu.LoadROM("07.nes");

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

			// Change pallette debug
			//case sf::Event::KeyPressed:
			//	if (event.key.code == sf::Keyboard::Up)
			//		a = (a >= (int)NES_PPU_PALETTE_COLORS.size() - 4 ? 0 : a + 1);
			//	else if (event.key.code == sf::Keyboard::Down)
			//		a = (a <= 0 ? (int)NES_PPU_PALETTE_COLORS.size() - 4 : a - 1);
			//	break;
			}
		}

		// @TODO DEBUG!!
		window.clear();
		emu.Frame();
		/*p.DebugDrawPatterns(im, a);*/
		//tex.loadFromImage(im);
		//spr.setTexture(tex);
		//spr.setScale(2.4f, 2.4f);
		//window.draw(spr);
		window.display();
	}

	return EXIT_SUCCESS;
}