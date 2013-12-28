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
#include "NstMapper068.h"
		 
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

MAPPER68::MAPPER68(CONTEXT& c)
: 
MAPPER (c,&status,CiRomBanks+2),
CiRam  (n2k),
CiRom  (NULL)
{}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

MAPPER68::~MAPPER68() 
{
	delete CiRom;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER68::Reset()
{
	cpu.SetPort( 0x8000, 0x8FFF, this, Peek_8000, Poke_8000 );
	cpu.SetPort( 0x9000, 0x9FFF, this, Peek_9000, Poke_9000 );
	cpu.SetPort( 0xA000, 0xAFFF, this, Peek_A000, Poke_A000 );
	cpu.SetPort( 0xB000, 0xBFFF, this, Peek_B000, Poke_B000 );
	cpu.SetPort( 0xC000, 0xCFFF, this, Peek_C000, Poke_C000 );
	cpu.SetPort( 0xD000, 0xDFFF, this, Peek_D000, Poke_D000 );
	cpu.SetPort( 0xE000, 0xEFFF, this, Peek_E000, Poke_E000 );
	cpu.SetPort( 0xF000, 0xFFFF, this, Peek_F000, Poke_F000 );

	delete CiRom; 
	CiRom = new CIROM( cRom.Ram(), cRom.Size() );

	ppu.SetPort( 0x2000, 0x2FFF, this, Peek_CiRam, Poke_CiRam );

	status = 0;
	CiRomBanks[0] = 0;
	CiRomBanks[1] = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT MAPPER68::LoadState(PDXFILE& file)
{
	PDX_TRY(MAPPER::LoadState(file));
	PDX_TRY(CiRam.LoadState(file));
	return CiRom->LoadState(file);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT MAPPER68::SaveState(PDXFILE& file) const
{
	PDX_TRY(MAPPER::SaveState(file));
	PDX_TRY(CiRam.SaveState(file));
	return CiRom->SaveState(file);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER68,8000) { ppu.Update(); cRom.SwapBanks<n2k,0x0000>(data); }
NES_POKE(MAPPER68,9000) { ppu.Update(); cRom.SwapBanks<n2k,0x0800>(data); }
NES_POKE(MAPPER68,A000) { ppu.Update(); cRom.SwapBanks<n2k,0x1000>(data); }
NES_POKE(MAPPER68,B000) { ppu.Update(); cRom.SwapBanks<n2k,0x1800>(data); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER68,C000) { CiRomBanks[0] = data; UpdateMirroring(); }
NES_POKE(MAPPER68,D000) { CiRomBanks[1] = data; UpdateMirroring(); }
NES_POKE(MAPPER68,E000) { status = data;        UpdateMirroring(); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER68,F000) 
{ 
	apu.Update(); 
	pRom.SwapBanks<n16k,0x0000>(data); 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(MAPPER68,CiRam) 
{ 
	return CiRam[address - 0x2000];    
}

NES_POKE(MAPPER68,CiRam) 
{ 
	CiRam[address - 0x2000] = data; 
}

NES_PEEK(MAPPER68,CiRom) 
{ 
	return (*CiRom)[address - 0x2000]; 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER68::UpdateMirroring()
{
	static const UCHAR select[4][4] =
	{
		{0,1,0,1},
		{0,0,1,1},
		{0,0,0,0},
		{1,1,1,1}
	};

	const UCHAR* const index = select[status & SELECT_MIRRORING];

	if (status & SELECT_CIROM)
	{
		CiRom->SwapBanks<n1k,0x0000>( BANK_OFFSET + CiRomBanks[index[0]] );
		CiRom->SwapBanks<n1k,0x0400>( BANK_OFFSET + CiRomBanks[index[1]] );
		CiRom->SwapBanks<n1k,0x0800>( BANK_OFFSET + CiRomBanks[index[2]] );
		CiRom->SwapBanks<n1k,0x0C00>( BANK_OFFSET + CiRomBanks[index[3]] );

		ppu.SetPort( 0x2000, 0x2FFF, this, Peek_CiRom, Poke_Nop );
	}
	else
	{
		CiRam.SwapBanks<n1k,0x0000>( index[0] );
		CiRam.SwapBanks<n1k,0x0400>( index[1] );
		CiRam.SwapBanks<n1k,0x0800>( index[2] );
		CiRam.SwapBanks<n1k,0x0C00>( index[3] );

		ppu.SetPort( 0x2000, 0x2FFF, this, Peek_CiRam, Poke_Nop );
	}
}

NES_NAMESPACE_END
