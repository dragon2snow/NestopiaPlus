////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003 Martin Freij
//
// This file is part of Nestopia.
// 
// Nestopia is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// Nestopia is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Nestopia; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef NST_MAPPER_1_H
#define NST_MAPPER_1_H

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
// MMC1
////////////////////////////////////////////////////////////////////////////////////////

class MAPPER1 : public MAPPER
{
public:

	MAPPER1(CONTEXT& c)
	: MAPPER(c,registers,&ResetCycles+1) {}

protected:

	VOID Reset();

private:

	NES_DECL_POKE( pRom );
	NES_DECL_PEEK( wRam );
	NES_DECL_POKE( wRam );

	VOID SetBanks();

	VOID ProcessRegister0();
	VOID ProcessRegister1();
	VOID ProcessRegister2();
	VOID ProcessRegister3();

	VOID EndFrame()
	{ cycles = 0; }

	enum
	{
		PROM_SWAP_LOW = b00000100,
		PROM_SWAP_16K = b00001000,
		CROM_SWAP_4K  = b00010000,
		REG0_RESET    = b00001100,
		REG3_NO_WRAM  = b00010000,
		DATA_RESET    = b10000000
	};

	UINT  registers[4];
	UINT  banks[4];
	UINT  latch;
	UINT  count;
	UINT  base;
	ULONG cycles;
	UINT  last;
	ULONG ResetCycles;
};

NES_NAMESPACE_END

#endif
