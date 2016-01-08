#pragma once

#include "NESPPU.h"
#include "NESCPU.h"

#define NES_INVALID_NAME_TABLE_INDEX -1

/**
* Communication interface allowing the PPU to communicate with
* its memory and the CPU.
*/
class NESPPUEmuComm : public INESPPUCommunicationsInterface
{
public:
	NESPPUEmuComm(NESPPUMemory& mem, NESCPU& cpu, NESPPUMirroringType ntMirror);
	virtual ~NESPPUEmuComm();

	/**
	* Sets an NMI int to happen on the CPU for the next CPU tick.
	*/
	inline void PullNMI() override { cpu_.SetInterrupt(NESCPUInterruptType::NMI); }

	/**
	* Reads 8-bits from CPU memory at the specified address.
	*/
	std::array<u8, 0x100> OAMDMARead(u8 addrPage) override;

	void Write8(u16 addr, u8 val) override;
	u8 Read8(u16 addr) const override;

private:
	NESPPUMemory& mem_;
	NESPPUMirroringType ntMirror_;

	NESCPU& cpu_;

	/**
	* Gets the corrisponding nametable's index from an address that is inside the nametable.
	* Considers the nametable mirroring type that is being used.
	*/
	std::size_t GetNameTableIndex(u16 addr) const;
};

