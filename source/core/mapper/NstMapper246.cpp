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
#include "NstMapper246.h"
		   
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER246::Reset()
{
	for (ULONG i=0x6000; i <= 0x67FF; ++i)
	{
		switch (i & 0xF007)
		{
			case 0x6000: cpu->SetPort( i, this, Peek_Nop, Poke_6000 ); continue;
			case 0x6001: cpu->SetPort( i, this, Peek_Nop, Poke_6001 ); continue;
			case 0x6002: cpu->SetPort( i, this, Peek_Nop, Poke_6002 ); continue;
			case 0x6003: cpu->SetPort( i, this, Peek_Nop, Poke_6003 ); continue;
			case 0x6004: cpu->SetPort( i, this, Peek_Nop, Poke_6004 ); continue;
			case 0x6005: cpu->SetPort( i, this, Peek_Nop, Poke_6005 ); continue;
			case 0x6006: cpu->SetPort( i, this, Peek_Nop, Poke_6006 ); continue;
			case 0x6007: cpu->SetPort( i, this, Peek_Nop, Poke_6007 ); continue;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER246,6000) { apu->Update(); pRom.SwapBanks<n8k,0x0000>(data); }
NES_POKE(MAPPER246,6001) { apu->Update(); pRom.SwapBanks<n8k,0x2000>(data); }
NES_POKE(MAPPER246,6002) { apu->Update(); pRom.SwapBanks<n8k,0x4000>(data); }
NES_POKE(MAPPER246,6003) { apu->Update(); pRom.SwapBanks<n8k,0x6000>(data); }
NES_POKE(MAPPER246,6004) { ppu->Update(); cRom.SwapBanks<n2k,0x0000>(data); }
NES_POKE(MAPPER246,6005) { ppu->Update(); cRom.SwapBanks<n2k,0x0800>(data); }
NES_POKE(MAPPER246,6006) { ppu->Update(); cRom.SwapBanks<n2k,0x1000>(data); }
NES_POKE(MAPPER246,6007) { ppu->Update(); cRom.SwapBanks<n2k,0x1800>(data); }

NES_NAMESPACE_END
