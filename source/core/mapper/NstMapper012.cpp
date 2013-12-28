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

#include "NstMappers.h"
#include "NstMapper004.h"
#include "NstMapper012.h"
	   
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

MAPPER12::MAPPER12(CONTEXT& c)
: MAPPER4(c,regs,regs+2) {}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER12::Reset()
{
	MAPPER4::Reset();

	regs[0] = regs[1] = 0;

	if (!cRom.Size())
	{
		EnableCartridgeCRam();
		cpu.SetPort( 0x4100, 0x7FFF, this, Peek_4100, Poke_Nop );
	}
	else
	{
		cpu.SetPort( 0x4100, 0x5FFF, this, Peek_4100, Poke_4100 );
		cpu.SetPort( 0x6000, 0x7FFF, this, Peek_4100, Poke_Nop );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER12,4100)
{
	regs[0] = (data & 0x01) << 8;
	regs[1] = (data & 0x10) << 4;

	UpdateCRom();
}

NES_PEEK(MAPPER12,4100)
{
	return 0x01;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER12::UpdateCRom()
{
	ppu.Update();

	if (IsCRam)
	{
		UpdateCRam();
	}
	else
	{
		if (command & SWAP_CROM_BANKS) 
		{
			cRom.SwapBanks<n1k,0x0000>( regs[0] + cRomBanks[2] ); 
			cRom.SwapBanks<n1k,0x0400>( regs[0] + cRomBanks[3] ); 
			cRom.SwapBanks<n1k,0x0800>( regs[0] + cRomBanks[4] ); 
			cRom.SwapBanks<n1k,0x0C00>( regs[0] + cRomBanks[5] ); 
			cRom.SwapBanks<n2k,0x1000>( regs[1] + cRomBanks[0] ); 
			cRom.SwapBanks<n2k,0x1800>( regs[1] + cRomBanks[1] ); 
		}
		else
		{
			cRom.SwapBanks<n2k,0x0000>( regs[0] + cRomBanks[0] ); 
			cRom.SwapBanks<n2k,0x0800>( regs[0] + cRomBanks[1] ); 
			cRom.SwapBanks<n1k,0x1000>( regs[1] + cRomBanks[2] ); 
			cRom.SwapBanks<n1k,0x1400>( regs[1] + cRomBanks[3] ); 
			cRom.SwapBanks<n1k,0x1800>( regs[1] + cRomBanks[4] ); 
			cRom.SwapBanks<n1k,0x1C00>( regs[1] + cRomBanks[5] ); 
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER12::UpdateCRam()
{
	if (command & SWAP_CROM_BANKS)
	{
		cRom.SwapBanks<n1k,0x0000>( cRomBanks[2] & 0x7 ); 
		cRom.SwapBanks<n1k,0x0400>( cRomBanks[3] & 0x7 ); 
		cRom.SwapBanks<n1k,0x0800>( cRomBanks[4] & 0x7 ); 
		cRom.SwapBanks<n1k,0x0C00>( cRomBanks[5] & 0x7 ); 
		cRom.SwapBanks<n2k,0x1000>( cRomBanks[0] & 0x3 ); 
		cRom.SwapBanks<n2k,0x1800>( cRomBanks[1] & 0x3 ); 
	}
	else
	{
		cRom.SwapBanks<n2k,0x0000>( cRomBanks[0] & 0x3 ); 
		cRom.SwapBanks<n2k,0x0800>( cRomBanks[1] & 0x3 ); 
		cRom.SwapBanks<n1k,0x1000>( cRomBanks[2] & 0x7 ); 
		cRom.SwapBanks<n1k,0x1400>( cRomBanks[3] & 0x7 ); 
		cRom.SwapBanks<n1k,0x1800>( cRomBanks[4] & 0x7 ); 
		cRom.SwapBanks<n1k,0x1C00>( cRomBanks[5] & 0x7 ); 
	}	
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER12::IrqSync()
{
	IrqReset = 0;

	if ((IrqCount-- <= 0) && (IrqCount = IrqLatch))
		cpu.DoIRQ();
}

NES_NAMESPACE_END
