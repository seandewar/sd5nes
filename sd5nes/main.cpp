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
	//window.setFramerateLimit(90);

	NESEmulator emu(window);
	//emu.LoadROM("roms//instr_test-v4//rom_singles//07-abs_xy.nes"); // PASS
	//emu.LoadROM("roms//nestest.nes");
	//emu.LoadROM("roms//smb_jp_usa.nes");
	emu.LoadROM("roms//digdug.nes");
	//emu.LoadROM("roms//full_palette.nes");
	//emu.LoadROM("roms//registers.nes");
	//emu.LoadROM("roms//blargg_ppu_tests_2005.09.15b//palette_ram.nes"); // PASS
	//emu.LoadROM("roms//02.alignment.nes");
	//emu.LoadROM("roms//blargg_ppu_tests_2005.09.15b//vbl_clear_time.nes"); // PASS
	//emu.LoadROM("roms//blargg_ppu_tests_2005.09.15b//vram_access.nes"); // PASS
	//emu.LoadROM("roms//blargg_ppu_tests_2005.09.15b//sprite_ram.nes"); // PASS
	//emu.LoadROM("roms//ppu_open_bus.nes");
	//emu.LoadROM("roms//scanline.nes");
	//emu.LoadROM("roms//sprite_overflow_tests//1.Basics.nes"); // PASS
	//emu.LoadROM("roms//sprite_overflow_tests//2.Details.nes"); // PASS
	//emu.LoadROM("roms//sprite_overflow_tests//3.Timing.nes");
	//emu.LoadROM("roms//sprite_overflow_tests//4.Obscure.nes"); // PASS
	//emu.LoadROM("roms//sprite_overflow_tests//5.Emulator.nes"); // PASS
	//emu.LoadROM("roms//sprite_hit_tests_2005.10.05//01.basics.nes"); // PASS
	//emu.LoadROM("roms//sprite_hit_tests_2005.10.05//02.alignment.nes"); // PASS
	//emu.LoadROM("roms//sprite_hit_tests_2005.10.05//03.corners.nes"); // PASS
	//emu.LoadROM("roms//sprite_hit_tests_2005.10.05//04.flip.nes"); // PASS
	//emu.LoadROM("roms//sprite_hit_tests_2005.10.05//05.left_clip.nes"); // PASS
	//emu.LoadROM("roms//sprite_hit_tests_2005.10.05//06.right_edge.nes"); // PASS
	//emu.LoadROM("roms//sprite_hit_tests_2005.10.05//07.screen_bottom.nes");
	//emu.LoadROM("roms//sprite_hit_tests_2005.10.05//08.double_height.nes"); // PASS
	//emu.LoadROM("roms//sprite_hit_tests_2005.10.05//09.timing_basics.nes");
	//emu.LoadROM("roms//sprite_hit_tests_2005.10.05//10.timing_order.nes");
	//emu.LoadROM("roms//sprite_hit_tests_2005.10.05//11.edge_timing.nes"); // PASS
	//emu.LoadROM("roms//ppu_vbl_nmi//01-vbl_basics.nes"); // PASS
	//emu.LoadROM("roms//ppu_vbl_nmi//02-vbl_set_time.nes"); // PASS
	//emu.LoadROM("roms//ppu_vbl_nmi//03-vbl_clear_time.nes"); // PASS
	//emu.LoadROM("roms//ppu_vbl_nmi//04-nmi_control.nes");
	//emu.LoadROM("roms//ppu_vbl_nmi//05-nmi_timing.nes");
	//emu.LoadROM("roms//ppu_vbl_nmi//06-suppression.nes");
	//emu.LoadROM("roms//ppu_vbl_nmi//07-nmi_on_timing.nes");
	//emu.LoadROM("roms//ppu_vbl_nmi//08-nmi_off_timing.nes");
	//emu.LoadROM("roms//ppu_vbl_nmi//09-even_odd_frames.nes");
	//emu.LoadROM("roms//ppu_vbl_nmi//10-even_odd_timing.nes");
	//emu.LoadROM("roms//nes_instr_misc//01-abs_x_wrap.nes");
	//emu.LoadROM("roms//cpu_interrupts_v2//1-cli_latency.nes");
	//emu.LoadROM("roms//cpu_interrupts_v2//2-nmi_and_brk.nes");

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

		window.clear();
		emu.Frame();
		window.display();
	}

	return EXIT_SUCCESS;
}