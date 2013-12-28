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
#include "NstMapper019.h"
#include "../sound/NstSndN106.h"
	   
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

MAPPER19::MAPPER19(CONTEXT& c)
: MAPPER(c,regs,cRam+n8k+n2k), sound(NULL) {}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

MAPPER19::~MAPPER19()
{
	delete sound;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER19::Reset()
{
	EnableIrqSync(IRQSYNC_COUNT);

	switch (pRomCrc)
	{
     	case 0x96533999UL: // Dokuganryuu Masamune
     	case 0x429FD177UL: // Famista '90
     	case 0xDD454208UL: // Hydlide 3 - Yami Kara no Houmonsha
     	case 0xB1B9E187UL: // Kaijuu Monogatari
     	case 0xAF15338FUL: // Mindseeker
		
			cpu->SetPort( 0x4800, 0x4FFF, this, Peek_480x, Poke_480x );
			cpu->SetPort( 0xF800, 0xFFFF, this, Peek_F000, Poke_F80x );

			delete sound;
			sound = NULL;
			break;
		
		default:

			cpu->SetPort( 0x4800, 0x4FFF, this, Peek_480y, Poke_480y );
			cpu->SetPort( 0xF800, 0xFFFF, this, Peek_F000, Poke_F80y );

			if (!sound)
				sound = new SNDN106(cpu);

			sound->Reset();
			break;
	}

	for (UINT i=0x5000; i <= 0x5800; ++i)
	{
		switch (i & 0xF800)
		{
			case 0x5000: cpu->SetPort( i, this, Peek_5000, Poke_5000 ); continue;
			case 0x5800: cpu->SetPort( i, this, Peek_5800, Poke_5800 ); continue;
		}
	}

	for (ULONG i=0x8000; i <= 0xFFFFU; ++i)
	{
		switch (i & 0xF800)
		{
			case 0x8000: cpu->SetPort( i, this, Peek_8000, Poke_8000 ); continue;
			case 0x8800: cpu->SetPort( i, this, Peek_8000, Poke_8800 ); continue;
			case 0x9000: cpu->SetPort( i, this, Peek_9000, Poke_9000 ); continue;
			case 0x9800: cpu->SetPort( i, this, Peek_9000, Poke_9800 ); continue;
			case 0xA000: cpu->SetPort( i, this, Peek_A000, Poke_A000 ); continue;
			case 0xA800: cpu->SetPort( i, this, Peek_A000, Poke_A800 ); continue;
			case 0xB000: cpu->SetPort( i, this, Peek_B000, Poke_B000 ); continue;
			case 0xB800: cpu->SetPort( i, this, Peek_B000, Poke_B800 ); continue;
			case 0xC000: cpu->SetPort( i, this, Peek_C000, Poke_C000 ); continue;
			case 0xC800: cpu->SetPort( i, this, Peek_C000, Poke_C800 ); continue;
			case 0xD000: cpu->SetPort( i, this, Peek_D000, Poke_D000 ); continue;
			case 0xD800: cpu->SetPort( i, this, Peek_D000, Poke_D800 ); continue;
			case 0xE000: cpu->SetPort( i, this, Peek_E000, Poke_E000 ); continue;
			case 0xE800: cpu->SetPort( i, this, Peek_E000, Poke_E800 ); continue;
			case 0xF000: cpu->SetPort( i, this, Peek_F000, Poke_F000 ); continue;
		}
	}

	ppu->SetPort( 0x0000, 0x03FF, this, Peek_vRam_0000, Poke_vRam_0000 );
	ppu->SetPort( 0x0400, 0x07FF, this, Peek_vRam_0400, Poke_vRam_0400 );
	ppu->SetPort( 0x0800, 0x0BFF, this, Peek_vRam_0800, Poke_vRam_0800 );
	ppu->SetPort( 0x0C00, 0x0FFF, this, Peek_vRam_0C00, Poke_vRam_0C00 );
	ppu->SetPort( 0x1000, 0x13FF, this, Peek_vRam_1000, Poke_vRam_1000 );
	ppu->SetPort( 0x1400, 0x17FF, this, Peek_vRam_1400, Poke_vRam_1400 );
	ppu->SetPort( 0x1800, 0x1BFF, this, Peek_vRam_1800, Poke_vRam_1800 );
	ppu->SetPort( 0x1C00, 0x1FFF, this, Peek_vRam_1C00, Poke_vRam_1C00 );
	ppu->SetPort( 0x2000, 0x23FF, this, Peek_vRam_2000, Poke_vRam_2000 );
	ppu->SetPort( 0x2400, 0x27FF, this, Peek_vRam_2400, Poke_vRam_2400 );
	ppu->SetPort( 0x2800, 0x2BFF, this, Peek_vRam_2800, Poke_vRam_2800 );
	ppu->SetPort( 0x2C00, 0x2FFF, this, Peek_vRam_2C00, Poke_vRam_2C00 );
	ppu->SetPort( 0x3000, 0x33FF, this, Peek_vRam_2000, Poke_vRam_2000 );
	ppu->SetPort( 0x3400, 0x37FF, this, Peek_vRam_2400, Poke_vRam_2400 );
	ppu->SetPort( 0x3800, 0x3BFF, this, Peek_vRam_2800, Poke_vRam_2800 );
	ppu->SetPort( 0x3C00, 0x3FFF, this, Peek_vRam_2C00, Poke_vRam_2C00 );

	if (cRom.NumBanks<n8k>())
		cRom.SwapBanks<n8k,0x0000>( cRom.NumBanks<n8k>() - 1 );

	for (UINT i=0; i < 8; ++i)
	{
		vRam[i].protect = cRom.Size() ? TRUE : FALSE;
		vRam[i].data = cRom.Size() ? cRom.GetBank(i) : cRam + (n1k * i);
	}

	for (UINT i=8; i < 12; ++i)
		vRam[i].protect = FALSE;

	if (mirroring == MIRROR_HORIZONTAL)
	{
		vRam[0x8].data = cRam + 0x2000;
		vRam[0x9].data = cRam + 0x2000;
		vRam[0xA].data = cRam + 0x2400;
		vRam[0xB].data = cRam + 0x2400;
	}
	else
	{
		vRam[0x8].data = cRam + 0x2000;
		vRam[0x9].data = cRam + 0x2400;
		vRam[0xA].data = cRam + 0x2000;
		vRam[0xB].data = cRam + 0x2400;
	}

	regs[0] = 0;
	regs[1] = 0;
	regs[2] = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT MAPPER19::LoadState(PDXFILE& file)
{
	PDX_TRY(MAPPER::LoadState(file));

	if (sound && PDX_FAILED(sound->LoadState(file)))
		return PDX_FAILURE;

	if (!file.Readable((sizeof(U8) * n8k+n2k) + (sizeof(U8) + sizeof(U16)) * 12))
		return PDX_FAILURE;

	file.Read( cRam, cRam + n8k+n2k );

	for (UINT i=0; i < 12; ++i)
	{
		vRam[i].protect = file.Read<U8>() ? TRUE : FALSE;

		const UINT offset = file.Read<U16>();

		if (vRam[i].protect)
			vRam[i].data = cRom.Ram() + PDX_MIN(cRom.Size() - n1k,offset);
		else
			vRam[i].data = cRam + PDX_MIN(n8k+n2k-n1k,offset);
	}

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT MAPPER19::SaveState(PDXFILE& file) const
{
	PDX_TRY(MAPPER::SaveState(file));

	if (sound && PDX_FAILED(sound->SaveState(file)))
		return PDX_FAILURE;

	file.Write( cRam, cRam + n8k+n2k );

	for (UINT i=0; i < 12; ++i)
	{
		file << U8(vRam[i].protect ? 1 : 0);
		file << U16(vRam[i].data - (vRam[i].protect ? cRom.Ram() : cRam));
	}

	return PDX_OK;
}
////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(MAPPER19,vRam_0000) { return vRam[0x0].data[address & 0x3FF]; }
NES_PEEK(MAPPER19,vRam_0400) { return vRam[0x1].data[address & 0x3FF]; }
NES_PEEK(MAPPER19,vRam_0800) { return vRam[0x2].data[address & 0x3FF]; }
NES_PEEK(MAPPER19,vRam_0C00) { return vRam[0x3].data[address & 0x3FF]; }
NES_PEEK(MAPPER19,vRam_1000) { return vRam[0x4].data[address & 0x3FF]; }
NES_PEEK(MAPPER19,vRam_1400) { return vRam[0x5].data[address & 0x3FF]; }
NES_PEEK(MAPPER19,vRam_1800) { return vRam[0x6].data[address & 0x3FF]; }
NES_PEEK(MAPPER19,vRam_1C00) { return vRam[0x7].data[address & 0x3FF]; }
NES_PEEK(MAPPER19,vRam_2000) { return vRam[0x8].data[address & 0x3FF]; }
NES_PEEK(MAPPER19,vRam_2400) { return vRam[0x9].data[address & 0x3FF]; }
NES_PEEK(MAPPER19,vRam_2800) { return vRam[0xA].data[address & 0x3FF]; }
NES_PEEK(MAPPER19,vRam_2C00) { return vRam[0xB].data[address & 0x3FF]; }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER19,vRam_0000) { if (!vRam[0x0].protect) vRam[0x0].data[address & 0x3FF] = data; }
NES_POKE(MAPPER19,vRam_0400) { if (!vRam[0x1].protect) vRam[0x1].data[address & 0x3FF] = data; }
NES_POKE(MAPPER19,vRam_0800) { if (!vRam[0x2].protect) vRam[0x2].data[address & 0x3FF] = data; }
NES_POKE(MAPPER19,vRam_0C00) { if (!vRam[0x3].protect) vRam[0x3].data[address & 0x3FF] = data; }
NES_POKE(MAPPER19,vRam_1000) { if (!vRam[0x4].protect) vRam[0x4].data[address & 0x3FF] = data; }
NES_POKE(MAPPER19,vRam_1400) { if (!vRam[0x5].protect) vRam[0x5].data[address & 0x3FF] = data; }
NES_POKE(MAPPER19,vRam_1800) { if (!vRam[0x6].protect) vRam[0x6].data[address & 0x3FF] = data; }
NES_POKE(MAPPER19,vRam_1C00) { if (!vRam[0x7].protect) vRam[0x7].data[address & 0x3FF] = data; }
NES_POKE(MAPPER19,vRam_2000) { if (!vRam[0x8].protect) vRam[0x8].data[address & 0x3FF] = data; }
NES_POKE(MAPPER19,vRam_2400) { if (!vRam[0x9].protect) vRam[0x9].data[address & 0x3FF] = data; }
NES_POKE(MAPPER19,vRam_2800) { if (!vRam[0xA].protect) vRam[0xA].data[address & 0x3FF] = data; }
NES_POKE(MAPPER19,vRam_2C00) { if (!vRam[0xB].protect) vRam[0xB].data[address & 0x3FF] = data; }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(MAPPER19,480x) 
{
	const UINT data = wRam[regs[2] & 0x7F];

	if (regs[2] & 0x80)
		regs[2] = ((regs[2] & 0x7F) + 1) | 0x80;

	return data;
}

NES_POKE(MAPPER19,480x) 
{
	wRam[regs[2] & 0x7F] = data;

	if (regs[2] & 0x80)
		regs[2] = ((regs[2] & 0x7F) + 1) | 0x80;
}

NES_PEEK(MAPPER19,480y) 
{
	return sound->Peek_4800();
}

NES_POKE(MAPPER19,480y) 
{
	sound->Poke_4800(data);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(MAPPER19,5000) { return (IrqCount & 0x00FF) >> 0; }
NES_PEEK(MAPPER19,5800) { return (IrqCount & 0x7F00) >> 8; }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER19,5000) 
{ 
	IrqCount = (IrqCount & 0xFF00) | data; 
	cpu->ClearIRQ();
}

NES_POKE(MAPPER19,5800) 
{
	IrqCount = (IrqCount & 0x00FF) | ((data & 0x7F) << 8);
	cpu->ClearIRQ();
	SetIrqEnable( data & 0x80 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER19,8000) { ppu->Update(); vRam[0x0].data = ((vRam[0x0].protect = (data < 0xE0 || regs[0])) ? cRom.Ram(data * n1k) : cRam + 0x0000); }
NES_POKE(MAPPER19,8800) { ppu->Update(); vRam[0x1].data = ((vRam[0x1].protect = (data < 0xE0 || regs[0])) ? cRom.Ram(data * n1k) : cRam + 0x0400); }
NES_POKE(MAPPER19,9000) { ppu->Update(); vRam[0x2].data = ((vRam[0x2].protect = (data < 0xE0 || regs[0])) ? cRom.Ram(data * n1k) : cRam + 0x0800); }
NES_POKE(MAPPER19,9800) { ppu->Update(); vRam[0x3].data = ((vRam[0x3].protect = (data < 0xE0 || regs[0])) ? cRom.Ram(data * n1k) : cRam + 0x0C00); }
NES_POKE(MAPPER19,A000) { ppu->Update(); vRam[0x4].data = ((vRam[0x4].protect = (data < 0xE0 || regs[1])) ? cRom.Ram(data * n1k) : cRam + 0x1000); }
NES_POKE(MAPPER19,A800) { ppu->Update(); vRam[0x5].data = ((vRam[0x5].protect = (data < 0xE0 || regs[1])) ? cRom.Ram(data * n1k) : cRam + 0x1400); }
NES_POKE(MAPPER19,B000) { ppu->Update(); vRam[0x6].data = ((vRam[0x6].protect = (data < 0xE0 || regs[1])) ? cRom.Ram(data * n1k) : cRam + 0x1800); }
NES_POKE(MAPPER19,B800) { ppu->Update(); vRam[0x7].data = ((vRam[0x7].protect = (data < 0xE0 || regs[1])) ? cRom.Ram(data * n1k) : cRam + 0x1C00); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER19,C000) { ppu->Update(); vRam[0x8].data = ((vRam[0x8].protect = (data < 0xE0)) ? cRom.Ram(data * n1k) : cRam + 0x2000 + (n1k * (data & 0x1))); }
NES_POKE(MAPPER19,C800) { ppu->Update(); vRam[0x9].data = ((vRam[0x9].protect = (data < 0xE0)) ? cRom.Ram(data * n1k) : cRam + 0x2000 + (n1k * (data & 0x1))); }
NES_POKE(MAPPER19,D000) { ppu->Update(); vRam[0xA].data = ((vRam[0xA].protect = (data < 0xE0)) ? cRom.Ram(data * n1k) : cRam + 0x2000 + (n1k * (data & 0x1))); }
NES_POKE(MAPPER19,D800) { ppu->Update(); vRam[0xB].data = ((vRam[0xB].protect = (data < 0xE0)) ? cRom.Ram(data * n1k) : cRam + 0x2000 + (n1k * (data & 0x1))); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER19,E000) 
{ 
	apu->Update(); 
	pRom.SwapBanks<n8k,0x0000>( data & 0x3F ); 
}

NES_POKE(MAPPER19,E800) 
{
	apu->Update(); 
	pRom.SwapBanks<n8k,0x2000>( data & 0x3F );

	regs[0] = data & 0x40;
	regs[1] = data & 0x80;
}

NES_POKE(MAPPER19,F000) 
{ 
	apu->Update(); 
	pRom.SwapBanks<n8k,0x4000>( data & 0x3F ); 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER19,F80x) 
{
	regs[2] = data;
}

NES_POKE(MAPPER19,F80y) 
{
	sound->Poke_F800(data);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER19::IrqSync(const UINT delta)
{
	if ((IrqCount += delta) >= 0x7FFF)
	{
		IrqCount = 0x7FFF;
		SetIrqEnable(FALSE);
		cpu->DoIRQ();
	}
}

NES_NAMESPACE_END
