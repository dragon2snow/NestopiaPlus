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
#include "NstMapper114.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER114::Reset()
{
	EnableIrqSync(IRQSYNC_PPU);

	cpu->SetPort( 0x6000, 0x7FFF, this, Peek_Nop, Poke_6000 );

	for (ULONG i=0x8000; i < 0xFFFFU; ++i)
	{
		switch (i & 0xE000)
		{
     		case 0x8000: cpu->SetPort( i, this, Peek_8000, Poke_8000 ); continue;
			case 0xA000: cpu->SetPort( i, this, Peek_A000, Poke_A000 ); continue;
			case 0xC000: cpu->SetPort( i, this, Peek_C000, Poke_C000 ); continue;
		}
	}

	cpu->SetPort( 0xE002, this, Peek_E000, Poke_E002 );
	cpu->SetPort( 0xE003, this, Peek_E000, Poke_E003 );

	ctrl = 0;
	ready = FALSE;
	command = 0;

	for (UINT i=0; i < 8; ++i)
		banks[i] = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER114::UpdatePRom()
{
	apu->Update();

	if (ctrl & 0x80)
	{
		pRom.SwapBanks<n16k,0x0000>( ctrl & 0x1F );
	}
	else
	{
		pRom.SwapBanks<n8k,0x0000>( banks[4] );
		pRom.SwapBanks<n8k,0x2000>( banks[5] );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER114::UpdateCRom()
{
	ppu->Update();

	cRom.SwapBanks<n2k,0x0000>( banks[0] >> 1 );
	cRom.SwapBanks<n2k,0x0800>( banks[2] >> 1 );
	cRom.SwapBanks<n1k,0x1000>( banks[6]      );
	cRom.SwapBanks<n1k,0x1400>( banks[1]      );
	cRom.SwapBanks<n1k,0x1800>( banks[7]      );
	cRom.SwapBanks<n1k,0x1C00>( banks[3]      );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER114,6000)
{
	ctrl = data;
	UpdatePRom();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER114,8000)
{
	ppu->SetMirroring( (data & 0x1) ? MIRROR_HORIZONTAL : MIRROR_VERTICAL );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER114,A000)
{
	command = data & 0x7;
	ready = TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER114,C000)
{
	if (ready)
	{
		banks[command] = data;

		switch (command)
		{
     		case 0x0:
			case 0x1:
			case 0x2:
			case 0x3:
			case 0x6:
			case 0x7:

				UpdateCRom();
				break;

			case 0x4:
			case 0x5:

				UpdatePRom();
				break;
		}

		ready = FALSE;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER114,E002) { cpu->ClearIRQ(); }
NES_POKE(MAPPER114,E003) { SetIrqEnable(IrqCount = data); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER114::IrqSync()
{
	if (!--IrqCount)
	{
		SetIrqEnable(FALSE);
		cpu->DoIRQ();
	}
}

NES_NAMESPACE_END
