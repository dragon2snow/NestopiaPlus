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
#include "NstMapper069.h"
		   
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

MAPPER69::MAPPER69(CONTEXT& c)
: 
MAPPER (c,&UseWRam,&command+1),
sound  (*c.cpu)
{}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER69::Reset()
{
	EnableIrqSync(IRQSYNC_COUNT);

	UseWRam = TRUE;
	command = 0x0;

	cpu.SetPort( 0x6000, 0x7FFF, this, Peek_6000, Poke_6000 );
	cpu.SetPort( 0x8000, 0x9FFF, this, Peek_8000, Poke_8000 );
	cpu.SetPort( 0xA000, 0xBFFF, this, Peek_A000, Poke_A000 );
	cpu.SetPort( 0xC000, 0xDFFF, this, Peek_C000, Poke_C000 );
	cpu.SetPort( 0xE000, 0xFFFF, this, Peek_E000, Poke_E000 );

	sound.Reset();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT MAPPER69::LoadState(PDXFILE& file)
{
	PDX_TRY(MAPPER::LoadState(file));
	return sound.LoadState(file);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT MAPPER69::SaveState(PDXFILE& file) const
{
	PDX_TRY(MAPPER::SaveState(file));
	return sound.SaveState(file);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(MAPPER69,6000)
{
	return UseWRam ? wRam[address - 0x6000] : *pRom.Ram(pRomOffset + (address & 0x1FFF));
}

NES_POKE(MAPPER69,6000)
{
	if (UseWRam)
		wRam[address - 0x6000] = data;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER69,8000) 
{ 
	command = data & 0xF;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER69,A000) 
{ 
	apu.Update(); 
	ppu.Update();

	switch (command)
	{
		case 0x0: cRom.SwapBanks<n1k,0x0000>(data); return;
		case 0x1: cRom.SwapBanks<n1k,0x0400>(data); return; 
		case 0x2: cRom.SwapBanks<n1k,0x0800>(data); return; 
		case 0x3: cRom.SwapBanks<n1k,0x0C00>(data); return; 
		case 0x4: cRom.SwapBanks<n1k,0x1000>(data); return; 
		case 0x5: cRom.SwapBanks<n1k,0x1400>(data); return; 
		case 0x6: cRom.SwapBanks<n1k,0x1800>(data); return; 
		case 0x7: cRom.SwapBanks<n1k,0x1C00>(data); return; 

		case 0x8:

			if (data & 0x40)
			{
				if (data & 0x80)
					UseWRam = TRUE;
			}
			else
			{
				UseWRam = FALSE;
				pRomOffset = (data * n8k) & pRom.Mask();
			}
			return;

		case 0x9: pRom.SwapBanks<n8k,0x0000>(data);	return;
		case 0xA: pRom.SwapBanks<n8k,0x2000>(data);	return;
		case 0xB: pRom.SwapBanks<n8k,0x4000>(data);	return;

		case 0xC:
		{
			const UCHAR select[4][4] =
			{
				{0,1,0,1},
				{0,0,1,1},
				{0,0,0,0},
				{1,1,1,1}
			};

			const UCHAR* const index = select[data & 0x3];

			ppu.SetMirroring
			(
				index[0],
				index[1],
				index[2],
				index[3]
			);

			return;
		}

		case 0xD: SetIrqEnable(data); return;
		case 0xE: IrqCount = (IrqCount & 0xFF00) | (data << 0); return;
		case 0xF: IrqCount = (IrqCount & 0x00FF) | (data << 8); return;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER69,C000) { sound.Poke_C000( data ); }
NES_POKE(MAPPER69,E000) { sound.Poke_E000( data ); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER69::IrqSync(const UINT delta)
{
	IrqCount -= delta;

	if (IrqCount <= 0)
	{
		IrqCount = 0xFFFF;
		SetIrqEnable(FALSE);
		cpu.TryIRQ();
	}
}

NES_NAMESPACE_END
