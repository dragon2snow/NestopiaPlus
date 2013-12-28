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
#include "../sound/NstSndVrc6.h"
#include "NstMapper024.h"
			 
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

MAPPER24::MAPPER24(CONTEXT& context)
: MAPPER(context), vrc6(new SNDVRC6(*context.cpu)) {}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

MAPPER24::~MAPPER24()
{
	delete vrc6;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER24::Reset()
{
	EnableIrqSync(IRQSYNC_COUNT);

	for (ULONG i=0x8000; i <= 0xFFFF; ++i)
	{
		switch (i & 0xF003)
		{
     		case 0x8000: cpu.SetPort( i, this, Peek_8000, Poke_8000 ); continue;
			case 0x9000: cpu.SetPort( i, this, Peek_9000, Poke_9000 ); continue;
			case 0x9001: cpu.SetPort( i, this, Peek_9000, Poke_9001 ); continue;
			case 0x9002: cpu.SetPort( i, this, Peek_9000, Poke_9002 ); continue;
			case 0xA000: cpu.SetPort( i, this, Peek_A000, Poke_A000 ); continue;
			case 0xA001: cpu.SetPort( i, this, Peek_A000, Poke_A001 ); continue;
			case 0xA002: cpu.SetPort( i, this, Peek_A000, Poke_A002 ); continue;
			case 0xB000: cpu.SetPort( i, this, Peek_B000, Poke_B000 ); continue;
			case 0xB001: cpu.SetPort( i, this, Peek_B000, Poke_B001 ); continue;
			case 0xB002: cpu.SetPort( i, this, Peek_B000, Poke_B002 ); continue;
			case 0xB003: cpu.SetPort( i, this, Peek_B000, Poke_B003 ); continue;
			case 0xC000: cpu.SetPort( i, this, Peek_C000, Poke_C000 ); continue;
			case 0xD000: cpu.SetPort( i, this, Peek_D000, Poke_D000 ); continue;
			case 0xD001: cpu.SetPort( i, this, Peek_D000, Poke_D001 ); continue;
			case 0xD002: cpu.SetPort( i, this, Peek_D000, Poke_D002 ); continue;
			case 0xD003: cpu.SetPort( i, this, Peek_D000, Poke_D003 ); continue;
			case 0xE000: cpu.SetPort( i, this, Peek_E000, Poke_E000 ); continue;
			case 0xE001: cpu.SetPort( i, this, Peek_E000, Poke_E001 ); continue;
			case 0xE002: cpu.SetPort( i, this, Peek_E000, Poke_E002 ); continue;
			case 0xE003: cpu.SetPort( i, this, Peek_E000, Poke_E003 ); continue;
			case 0xF000: cpu.SetPort( i, this, Peek_F000, Poke_F000 ); continue;
			case 0xF001: cpu.SetPort( i, this, Peek_F000, Poke_F001 ); continue;
			case 0xF002: cpu.SetPort( i, this, Peek_F000, Poke_F002 ); continue;
		}																		
	}

	vrc6->Reset();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT MAPPER24::LoadState(PDXFILE& file)
{
	PDX_TRY(MAPPER::LoadState(file));
	return vrc6->LoadState(file);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT MAPPER24::SaveState(PDXFILE& file) const
{
	PDX_TRY(MAPPER::SaveState(file));
	return vrc6->SaveState(file);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER24,8000) 
{
	apu.Update(); 
	pRom.SwapBanks<n16k,0x0000>(data);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER24,9000) { vrc6->WriteSquare1Reg0  ( data ); }
NES_POKE(MAPPER24,9001) { vrc6->WriteSquare1Reg1  ( data ); }
NES_POKE(MAPPER24,9002) { vrc6->WriteSquare1Reg2  ( data ); }
NES_POKE(MAPPER24,A000) { vrc6->WriteSquare2Reg0  ( data ); }
NES_POKE(MAPPER24,A001) { vrc6->WriteSquare2Reg1  ( data ); }
NES_POKE(MAPPER24,A002) { vrc6->WriteSquare2Reg2  ( data ); }
NES_POKE(MAPPER24,B000) { vrc6->WriteSawToothReg0 ( data ); }
NES_POKE(MAPPER24,B001) { vrc6->WriteSawToothReg1 ( data ); }
NES_POKE(MAPPER24,B002) { vrc6->WriteSawToothReg2 ( data ); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER24,B003) 
{
	switch (data & 0xF)
	{
       	case 0x00: ppu.SetMirroring( 0,1,0,1 ); return;
		case 0x04: ppu.SetMirroring( 0,0,1,1 ); return;
		case 0x08: ppu.SetMirroring( 0,0,0,0 ); return;
		case 0x0C: ppu.SetMirroring( 1,1,1,1 ); return;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER24,C000) 
{ 
	apu.Update(); 
	pRom.SwapBanks<n8k,0x4000>( data ); 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER24,D000) { ppu.Update(); cRom.SwapBanks<n1k,0x0000>( data ); }
NES_POKE(MAPPER24,D001) { ppu.Update(); cRom.SwapBanks<n1k,0x0400>( data ); }
NES_POKE(MAPPER24,D002) { ppu.Update(); cRom.SwapBanks<n1k,0x0800>( data ); }
NES_POKE(MAPPER24,D003) { ppu.Update(); cRom.SwapBanks<n1k,0x0C00>( data ); }
NES_POKE(MAPPER24,E000) { ppu.Update(); cRom.SwapBanks<n1k,0x1000>( data ); }
NES_POKE(MAPPER24,E001) { ppu.Update(); cRom.SwapBanks<n1k,0x1400>( data ); }
NES_POKE(MAPPER24,E002) { ppu.Update(); cRom.SwapBanks<n1k,0x1800>( data ); }
NES_POKE(MAPPER24,E003) { ppu.Update(); cRom.SwapBanks<n1k,0x1C00>( data ); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER24,F000) 
{ 
	IrqLatch = data; 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER24,F001) 
{
	IrqTmp = data & 0x1;
	SetIrqEnable(data & 0x2);

	if (IsIrqEnabled())
		IrqCount = (0x100 - IrqLatch) * 114;

	cpu.ClearIRQ();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER24,F002) 
{
	SetIrqEnable(IrqTmp);
	cpu.ClearIRQ();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER24::IrqSync(const UINT delta)
{
	IrqCount -= delta;

	if (IrqCount <= 0)
	{
		IrqCount = (0x100 - IrqLatch) * 114;
		cpu.DoIRQ();
	}
}

NES_NAMESPACE_END

