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
#include "NstMapper119.h"
		
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER119::Reset()
{
	MAPPER4::Reset();

	ppu.SetPort( 0x0000, 0x03FF, this, Peek_0000, Poke_0000 );
	ppu.SetPort( 0x0400, 0x07FF, this, Peek_0400, Poke_0400 );
	ppu.SetPort( 0x0800, 0x0BFF, this, Peek_0800, Poke_0800 );
	ppu.SetPort( 0x0C00, 0x0FFF, this, Peek_0C00, Poke_0C00 );
	ppu.SetPort( 0x1000, 0x13FF, this, Peek_1000, Poke_1000 );
	ppu.SetPort( 0x1400, 0x17FF, this, Peek_1400, Poke_1400 );
	ppu.SetPort( 0x1800, 0x1BFF, this, Peek_1800, Poke_1800 );
	ppu.SetPort( 0x1C00, 0x1FFF, this, Peek_1C00, Poke_1C00 );

	PDXMemZero( SelectCRam, 8 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER119::UpdateCRom()
{
	ppu.Update();

	if (command & SWAP_CROM_BANKS)
	{
		if (SelectCRam[0] = (cRomBanks[2] & 0x40)) cRam.SwapBanks<n1k,0x0000>(((cRomBanks[2] << 0) + 0) & 0x7); else cRom.SwapBanks<n1k,0x0000>((cRomBanks[2] << 0) + 0);
		if (SelectCRam[1] = (cRomBanks[3] & 0x40)) cRam.SwapBanks<n1k,0x0400>(((cRomBanks[3] << 0) + 0) & 0x7); else cRom.SwapBanks<n1k,0x0400>((cRomBanks[3] << 0) + 0);
		if (SelectCRam[2] = (cRomBanks[4] & 0x40)) cRam.SwapBanks<n1k,0x0800>(((cRomBanks[4] << 0) + 0) & 0x7); else cRom.SwapBanks<n1k,0x0800>((cRomBanks[4] << 0) + 0);
		if (SelectCRam[3] = (cRomBanks[5] & 0x40)) cRam.SwapBanks<n1k,0x0C00>(((cRomBanks[5] << 0) + 0) & 0x7); else cRom.SwapBanks<n1k,0x0C00>((cRomBanks[5] << 0) + 0);
		if (SelectCRam[4] = (cRomBanks[0] & 0x20)) cRam.SwapBanks<n1k,0x1000>(((cRomBanks[0] << 1) + 0) & 0x7); else cRom.SwapBanks<n1k,0x1000>((cRomBanks[0] << 1) + 0);
		if (SelectCRam[5] = (cRomBanks[0] & 0x20)) cRam.SwapBanks<n1k,0x1400>(((cRomBanks[0] << 1) + 1) & 0x7); else cRom.SwapBanks<n1k,0x1400>((cRomBanks[0] << 1) + 1);
		if (SelectCRam[6] = (cRomBanks[1] & 0x20)) cRam.SwapBanks<n1k,0x1800>(((cRomBanks[1] << 1) + 0) & 0x7); else cRom.SwapBanks<n1k,0x1800>((cRomBanks[1] << 1) + 0);
		if (SelectCRam[7] = (cRomBanks[1] & 0x20)) cRam.SwapBanks<n1k,0x1C00>(((cRomBanks[1] << 1) + 1) & 0x7); else cRom.SwapBanks<n1k,0x1C00>((cRomBanks[1] << 1) + 1);
	}
	else
	{
		if (SelectCRam[0] = (cRomBanks[0] & 0x20)) cRam.SwapBanks<n1k,0x0000>(((cRomBanks[0] << 1) + 0) & 0x7); else cRom.SwapBanks<n1k,0x0000>((cRomBanks[0] << 1) + 0); 
		if (SelectCRam[1] = (cRomBanks[0] & 0x20)) cRam.SwapBanks<n1k,0x0400>(((cRomBanks[0] << 1) + 1) & 0x7); else cRom.SwapBanks<n1k,0x0400>((cRomBanks[0] << 1) + 1); 
		if (SelectCRam[2] = (cRomBanks[1] & 0x20)) cRam.SwapBanks<n1k,0x0800>(((cRomBanks[1] << 1) + 0) & 0x7); else cRom.SwapBanks<n1k,0x0800>((cRomBanks[1] << 1) + 0); 
		if (SelectCRam[3] = (cRomBanks[1] & 0x20)) cRam.SwapBanks<n1k,0x0C00>(((cRomBanks[1] << 1) + 1) & 0x7); else cRom.SwapBanks<n1k,0x0C00>((cRomBanks[1] << 1) + 1); 
		if (SelectCRam[4] = (cRomBanks[2] & 0x40)) cRam.SwapBanks<n1k,0x1000>(((cRomBanks[2] << 0) + 0) & 0x7); else cRom.SwapBanks<n1k,0x1000>((cRomBanks[2] << 0) + 0); 
		if (SelectCRam[5] = (cRomBanks[3] & 0x40)) cRam.SwapBanks<n1k,0x1400>(((cRomBanks[3] << 0) + 0) & 0x7); else cRom.SwapBanks<n1k,0x1400>((cRomBanks[3] << 0) + 0); 
		if (SelectCRam[6] = (cRomBanks[4] & 0x40)) cRam.SwapBanks<n1k,0x1800>(((cRomBanks[4] << 0) + 0) & 0x7); else cRom.SwapBanks<n1k,0x1800>((cRomBanks[4] << 0) + 0); 
		if (SelectCRam[7] = (cRomBanks[5] & 0x40)) cRam.SwapBanks<n1k,0x1C00>(((cRomBanks[5] << 0) + 0) & 0x7); else cRom.SwapBanks<n1k,0x1C00>((cRomBanks[5] << 0) + 0); 
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT MAPPER119::LoadState(PDXFILE& file)
{
	PDX_TRY(MAPPER4::LoadState(file));
	return cRam.LoadState(file);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT MAPPER119::SaveState(PDXFILE& file) const
{
	PDX_TRY(MAPPER4::SaveState(file));
	return cRam.SaveState(file);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(MAPPER119,0000) { TriggerA13(address); const UINT offset = address & 0x3FF; return SelectCRam[0] ? cRam(0,offset) : cRom(0,offset); }
NES_PEEK(MAPPER119,0400) { TriggerA13(address); const UINT offset = address & 0x3FF; return SelectCRam[1] ? cRam(1,offset) : cRom(1,offset); }
NES_PEEK(MAPPER119,0800) { TriggerA13(address); const UINT offset = address & 0x3FF; return SelectCRam[2] ? cRam(2,offset) : cRom(2,offset); }
NES_PEEK(MAPPER119,0C00) { TriggerA13(address); const UINT offset = address & 0x3FF; return SelectCRam[3] ? cRam(3,offset) : cRom(3,offset); }
NES_PEEK(MAPPER119,1000) { TriggerA13(address); const UINT offset = address & 0x3FF; return SelectCRam[4] ? cRam(4,offset) : cRom(4,offset); }
NES_PEEK(MAPPER119,1400) { TriggerA13(address); const UINT offset = address & 0x3FF; return SelectCRam[5] ? cRam(5,offset) : cRom(5,offset); }
NES_PEEK(MAPPER119,1800) { TriggerA13(address); const UINT offset = address & 0x3FF; return SelectCRam[6] ? cRam(6,offset) : cRom(6,offset); }
NES_PEEK(MAPPER119,1C00) { TriggerA13(address); const UINT offset = address & 0x3FF; return SelectCRam[7] ? cRam(7,offset) : cRom(7,offset); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER119,0000) { if (SelectCRam[0]) cRam(0,address & 0x3FF) = data; }
NES_POKE(MAPPER119,0400) { if (SelectCRam[1]) cRam(1,address & 0x3FF) = data; }
NES_POKE(MAPPER119,0800) { if (SelectCRam[2]) cRam(2,address & 0x3FF) = data; }
NES_POKE(MAPPER119,0C00) { if (SelectCRam[3]) cRam(3,address & 0x3FF) = data; }
NES_POKE(MAPPER119,1000) { if (SelectCRam[4]) cRam(4,address & 0x3FF) = data; }
NES_POKE(MAPPER119,1400) { if (SelectCRam[5]) cRam(5,address & 0x3FF) = data; }
NES_POKE(MAPPER119,1800) { if (SelectCRam[6]) cRam(6,address & 0x3FF) = data; }
NES_POKE(MAPPER119,1C00) { if (SelectCRam[7]) cRam(7,address & 0x3FF) = data; }

NES_NAMESPACE_END
