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
	//emu.LoadROM("roms//instr_test-v4//rom_singles//07-abs_xy.nes"); // PASS
	//emu.LoadROM("roms//nestest.nes");
	//emu.LoadROM("roms//smb_jp_usa.nes");
	//emu.LoadROM("roms//palette.nes");
	//emu.LoadROM("roms//registers.nes");
	//emu.LoadROM("roms//blargg_ppu_tests_2005.09.15b//palette_ram.nes"); // PASS
	//emu.LoadROM("roms//02.alignment.nes");
	//emu.LoadROM("roms//blargg_ppu_tests_2005.09.15b//vbl_clear_time.nes");
	//emu.LoadROM("roms//blargg_ppu_tests_2005.09.15b//vram_access.nes"); // PASS
	//emu.LoadROM("roms//blargg_ppu_tests_2005.09.15b//sprite_ram.nes"); // PASS
	//emu.LoadROM("roms//ppu_open_bus.nes");
	//emu.LoadROM("roms//scanline.nes");
	//emu.LoadROM("roms//sprite_overflow_tests//1.Basics.nes"); // PASS
	//emu.LoadROM("roms//sprite_overflow_tests//2.Details.nes"); // PASS
	//emu.LoadROM("roms//sprite_overflow_tests//3.Timing.nes");
	//emu.LoadROM("roms//sprite_overflow_tests//4.Obscure.nes"); // PASS
	//emu.LoadROM("roms//sprite_overflow_tests//5.Emulator.nes"); // PASS
	emu.LoadROM("roms//sprite_hit_tests_2005.10.05//01.basics.nes");

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