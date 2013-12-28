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
#include "NstMapper105.h"
			  
NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

MAPPER105::MAPPER105(CONTEXT& c)
: 
MAPPER       (c,registers,&ready+1), 
DipValue     (0),
DisplayTimer (TRUE),
elapsed      ("Time Left: ")
{}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER105::Reset()
{
	EnableIrqSync(IRQSYNC_COUNT);

	cpu.SetPort( 0x8000, 0x9FFF, this, Peek_8000, Poke_pRom );
	cpu.SetPort( 0xA000, 0xBFFF, this, Peek_A000, Poke_pRom );
	cpu.SetPort( 0xC000, 0xDFFF, this, Peek_C000, Poke_pRom );
	cpu.SetPort( 0xE000, 0xFFFF, this, Peek_E000, Poke_pRom );

	registers[0] = REG0_RESET;
	registers[1] = 0x00;
	registers[2] = 0x00;
	registers[3] = 0x10;

	pRom.SwapBanks<n32k,0x0000>(0);

	latch = 0;
	count = 0;
	ready = 0;
	
	UpdateTimer();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER105,pRom)
{
	const UINT index = (address & 0x7FFF) >> 13;

	if (data & DATA_RESET)
	{
		latch = 0;
		count = 0;

		if (!index)
			registers[0] |= REG0_RESET;
	}
	else
	{
		latch |= (data & 0x1) << count++;

		if (count == 5)
		{
			registers[index] = latch & 0x1F;
			latch = 0;
			count = 0;
		}
	}

	UpdateMirroring();

	if (ready < 2)
	{
		++ready;
	}
	else
	{
		UpdateBanks();
		UpdateIRQ();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER105::UpdateMirroring()
{
	static const UCHAR select[4][4] =
	{
		{0,0,0,0},
		{1,1,1,1},
		{0,1,0,1},
		{0,0,1,1}
	};

	const UCHAR* const index = select[registers[0] & 0x3];

	ppu.SetMirroring
	(
	    index[0],
		index[1],
		index[2],
		index[3]
	);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER105::UpdateIRQ()
{
	const BOOL state = (registers[1] & IRQ_DISABLE) ? FALSE : TRUE;

	SetIrqEnable(state);

	if (!state)
		timer = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER105::UpdateBanks()
{
	apu.Update();

	if (registers[1] & 0x8)
	{
		const UINT mode = registers[0] & 0xC;

		switch (mode)
		{
     		case 0x0:
     		case 0x4:

     			pRom.SwapBanks<n32k,0x0000>( (0x8 + (registers[3] & 0x6)) >> 1 );
     			return;

			case 0x8:

				pRom.SwapBanks<n16k,0x0000>( 0x8 );
				pRom.SwapBanks<n16k,0x4000>( 0x8 + (registers[3] & 0x7) );
				return;

			case 0xC:

				pRom.SwapBanks<n16k,0x0000>( 0x8 + (registers[3] & 0x7) );
				pRom.SwapBanks<n16k,0x4000>( 0xF );
				return;
		}
	}
	else
	{	
		pRom.SwapBanks<n32k,0x0000>( (registers[1] & 0x6) >> 1 );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

UINT MAPPER105::NumDipSwitches() const 
{ 
	return 2; 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER105::GetDipSwitch(const UINT index,IO::DIPSWITCH::CONTEXT& context) const 
{
	PDX_ASSERT( index <= 1 );

	switch (index)
	{
     	case 0:

			context.name = "Time";

			context.settings.Resize( 16 );

			context.settings[0x0] = "5.001";
			context.settings[0x1] = "5.316";
			context.settings[0x2] = "5.629";
			context.settings[0x3] = "5.942";
			context.settings[0x4] = "6.254";
			context.settings[0x5] = "6.567";
			context.settings[0x6] = "6.880";
			context.settings[0x7] = "7.193";
			context.settings[0x8] = "7.505";
			context.settings[0x9] = "7.818";
			context.settings[0xA] = "8.131";
			context.settings[0xB] = "8.444";
			context.settings[0xC] = "8.756";
			context.settings[0xD] = "9.070";
			context.settings[0xE] = "9.318";
			context.settings[0xF] = "9.695";

			context.index = DipValue;
			return;

		case 1:

			context.name = "Display Timer";

			context.settings.Resize( 2 );

			context.settings[0x0] = "no";
			context.settings[0x1] = "yes";

			context.index = DisplayTimer ? 1 : 0;
			return;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER105::SetDipSwitch(const UINT index,const IO::DIPSWITCH::CONTEXT& context) 
{
	PDX_ASSERT( index <= 1 );
	
	switch (index)
	{
     	case 0:

			PDX_ASSERT( context.index <= 15 );
			DipValue = context.index;
			UpdateTimer();
			return;

		case 1:

			PDX_ASSERT( context.index <= 1 );
			DisplayTimer = context.index == 1;
			return;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER105::UpdateTimer()
{
	TimerEnd = cpu.IsPAL() ? NES_CPU_CLOCK_HZ_REAL_PAL : NES_CPU_CLOCK_HZ_REAL_NTSC;

	switch (DipValue)
	{
		case 0x0: TimerEnd *= 5.001 * 60; return;
		case 0x1: TimerEnd *= 5.316 * 60; return;
		case 0x2: TimerEnd *= 5.629 * 60; return;
		case 0x3: TimerEnd *= 5.942 * 60; return;
		case 0x4: TimerEnd *= 6.254 * 60; return;
		case 0x5: TimerEnd *= 6.567 * 60; return;
		case 0x6: TimerEnd *= 6.880 * 60; return;
		case 0x7: TimerEnd *= 7.193 * 60; return;
		case 0x8: TimerEnd *= 7.505 * 60; return;
		case 0x9: TimerEnd *= 7.818 * 60; return;
		case 0xA: TimerEnd *= 8.131 * 60; return;
		case 0xB: TimerEnd *= 8.444 * 60; return;
		case 0xC: TimerEnd *= 8.756 * 60; return;
		case 0xD: TimerEnd *= 9.070 * 60; return;
		case 0xE: TimerEnd *= 9.318 * 60; return;
		case 0xF: TimerEnd *= 9.695 * 60; return;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER105::IrqSync(const UINT delta)
{
	timer += delta;

	if (timer >= TimerEnd)
	{
		timer = 0;
		cpu.DoIRQ();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER105::EndFrame()
{
	if (DisplayTimer && IsIrqEnabled())
	{
		elapsed.Resize( 11 );

		const DOUBLE cycles = cpu.IsPAL() ? NES_CPU_CLOCK_HZ_REAL_PAL : NES_CPU_CLOCK_HZ_REAL_NTSC;
		const ULONG seconds = ULONG((TimerEnd - timer) / cycles);

		elapsed += seconds / 60;
		elapsed += ":";
		elapsed += seconds % 60;

		MsgOutput( elapsed.String() );
	}
}

NES_NAMESPACE_END
