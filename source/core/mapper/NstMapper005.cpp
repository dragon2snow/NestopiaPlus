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
#include "NstMapper005.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
// constructor
////////////////////////////////////////////////////////////////////////////////////////

MAPPER5::MAPPER5(CONTEXT& c)
: 
MAPPER   (c,&gMode,&IrqStatus+1), 
vNameRam (n4k), 
Name1    (vNameRam.Ram( 0x000 )),
Name2    (vNameRam.Ram( 0x400 )),
ExRam    (vNameRam.Ram( 0x800 )),
FillRam  (vNameRam.Ram( 0xC00 )),
sound    (*c.cpu)
{}

////////////////////////////////////////////////////////////////////////////////////////
// reset
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER5::Reset()
{
	EnableIrqSync(IRQSYNC_PPU_ALWAYS);

	cpu.SetPort( 0x5100, this, Peek_Nop,  Poke_5100 );
	cpu.SetPort( 0x5101, this, Peek_Nop,  Poke_5101 );
	cpu.SetPort( 0x5102, this, Peek_Nop,  Poke_5102 );
	cpu.SetPort( 0x5103, this, Peek_Nop,  Poke_5103 );
	cpu.SetPort( 0x5104, this, Peek_Nop,  Poke_5104 );
	cpu.SetPort( 0x5105, this, Peek_Nop,  Poke_5105 );
	cpu.SetPort( 0x5106, this, Peek_Nop,  Poke_5106 );
	cpu.SetPort( 0x5107, this, Peek_Nop,  Poke_5107 );
	cpu.SetPort( 0x5113, this, Peek_Nop,  Poke_5113 );
	cpu.SetPort( 0x5114, this, Peek_Nop,  Poke_5114 );
	cpu.SetPort( 0x5115, this, Peek_Nop,  Poke_5115 );
	cpu.SetPort( 0x5116, this, Peek_Nop,  Poke_5116 );
	cpu.SetPort( 0x5117, this, Peek_Nop,  Poke_5117 );
	cpu.SetPort( 0x5120, this, Peek_Nop,  Poke_5120 );
	cpu.SetPort( 0x5121, this, Peek_Nop,  Poke_5121 );
	cpu.SetPort( 0x5122, this, Peek_Nop,  Poke_5122 );
	cpu.SetPort( 0x5123, this, Peek_Nop,  Poke_5123 );
	cpu.SetPort( 0x5124, this, Peek_Nop,  Poke_5124 );
	cpu.SetPort( 0x5125, this, Peek_Nop,  Poke_5125 );
	cpu.SetPort( 0x5126, this, Peek_Nop,  Poke_5126 );
	cpu.SetPort( 0x5127, this, Peek_Nop,  Poke_5127 );
	cpu.SetPort( 0x5128, this, Peek_Nop,  Poke_5128 );
	cpu.SetPort( 0x5129, this, Peek_Nop,  Poke_5129 );
	cpu.SetPort( 0x512A, this, Peek_Nop,  Poke_512A );
	cpu.SetPort( 0x512B, this, Peek_Nop,  Poke_512B );
	cpu.SetPort( 0x5200, this, Peek_Nop,  Poke_5200 );
	cpu.SetPort( 0x5201, this, Peek_Nop,  Poke_5201 );
	cpu.SetPort( 0x5202, this, Peek_Nop,  Poke_5202 );
	cpu.SetPort( 0x5203, this, Peek_Nop,  Poke_5203 );
	cpu.SetPort( 0x5204, this, Peek_5204, Poke_5204 );

	cpu.SetPort( 0x5C00, 0x5FFF, this, Peek_5C00, Poke_5C00 );

	cpu.SetPort( 0x6000, 0x7FFF, this, Peek_6000, Poke_6000 );
	cpu.SetPort( 0x8000, 0x9FFF, this, Peek_8000, Poke_8000 );
	cpu.SetPort( 0xA000, 0xBFFF, this, Peek_A000, Poke_A000 );
	cpu.SetPort( 0xC000, 0xDFFF, this, Peek_C000, Poke_C000 );
	cpu.SetPort( 0xE000, 0xFFFF, this, Peek_E000, Poke_Nop  );

	ppu.SetPort( 0x2000, 0x23BF, this, Peek_vRam_Name, Poke_vRam_Name );
	ppu.SetPort( 0x23C0, 0x23FF, this, Peek_vRam_Attr, Poke_vRam_Name );
	ppu.SetPort( 0x2400, 0x27BF, this, Peek_vRam_Name, Poke_vRam_Name );
	ppu.SetPort( 0x27C0, 0x27FF, this, Peek_vRam_Attr, Poke_vRam_Name );
	ppu.SetPort( 0x2800, 0x2BBF, this, Peek_vRam_Name, Poke_vRam_Name );
	ppu.SetPort( 0x2BC0, 0x2BFF, this, Peek_vRam_Attr, Poke_vRam_Name );
	ppu.SetPort( 0x2C00, 0x2FBF, this, Peek_vRam_Name, Poke_vRam_Name );
	ppu.SetPort( 0x2FC0, 0x2FFF, this, Peek_vRam_Attr, Poke_vRam_Name );
	ppu.SetPort( 0x3000, 0x33BF, this, Peek_vRam_Name, Poke_vRam_Name );
	ppu.SetPort( 0x33C0, 0x33FF, this, Peek_vRam_Attr, Poke_vRam_Name );
	ppu.SetPort( 0x3400, 0x37BF, this, Peek_vRam_Name, Poke_vRam_Name );
	ppu.SetPort( 0x37C0, 0x37FF, this, Peek_vRam_Attr, Poke_vRam_Name );
	ppu.SetPort( 0x3800, 0x3BBF, this, Peek_vRam_Name, Poke_vRam_Name );
	ppu.SetPort( 0x3BC0, 0x3BFF, this, Peek_vRam_Attr, Poke_vRam_Name );
	ppu.SetPort( 0x3C00, 0x3FBF, this, Peek_vRam_Name, Poke_vRam_Name );
	ppu.SetPort( 0x3FC0, 0x3FFF, this, Peek_vRam_Attr, Poke_vRam_Name );

	cBanks[SMODE_A][0] = 0;
	cBanks[SMODE_A][1] = 1;
	cBanks[SMODE_A][2] = 2;
	cBanks[SMODE_A][3] = 3;
	cBanks[SMODE_A][4] = 4;
	cBanks[SMODE_A][5] = 5;
	cBanks[SMODE_A][6] = 6;
	cBanks[SMODE_A][7] = 7;

	cBanks[SMODE_B][0] = 4;
	cBanks[SMODE_B][1] = 5;
	cBanks[SMODE_B][2] = 6;
	cBanks[SMODE_B][3] = 7;
	cBanks[SMODE_B][4] = 4;
	cBanks[SMODE_B][5] = 5;
	cBanks[SMODE_B][6] = 6;
	cBanks[SMODE_B][7] = 7;

	gMode = GMODE_SPLIT;
	pMode = PMODE_8K;
	cMode = CMODE_1K;

	LastCRomSwap = 0;

	SplitCtrl = 0;
	SplitBank = 0;

	IrqStatus = 0;
	IrqLine   = 0;

	wRamLatch.d = 0x0102;
	wRamWriteEnable = TRUE;

	const UINT LastBank = pRom.NumBanks<n8k>() - 1;

	BankSwitchWRam( 3, 0        );
	BankSwitchPRom( 4, LastBank );
	BankSwitchPRom( 5, LastBank );
	BankSwitchPRom( 6, LastBank );
	BankSwitchPRom( 7, LastBank );

	sound.Reset();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT MAPPER5::LoadState(PDXFILE& file)
{
	PDX_TRY(MAPPER::LoadState( file ));
	PDX_TRY(sound.LoadState( file ));
	return vNameRam.LoadState( file );
}

PDXRESULT MAPPER5::SaveState(PDXFILE& file) const
{
	PDX_TRY(MAPPER::SaveState( file ));
	PDX_TRY(sound.SaveState( file ));
	return vNameRam.SaveState( file );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(MAPPER5,vRam_Name)
{
	if (IsA13High())
	{
		if (gMode == GMODE_SPLIT_EXGFX)
		{
			LastCRomSwap = 0xFF;

			const UINT data = ExRam[address & 0x3FF];

			const UINT bank = data & 0x3F;
			cRom.SwapBanks<n4k,0x0000>( bank );
			cRom.SwapBanks<n4k,0x1000>( bank );

			const UINT offset = 0x3C0 | ((address & 0x1C) >> 2) | ((address & 0x380) >> 4);
			Name1[offset] = Name2[offset] = ((data & 0xC0) >> 6) * 0x55;
		}
		else
		{
			if (LastCRomSwap != SMODE_B)
				RefreshCRomBanks( SMODE_B );
		}
	}
	else
	{
		if (LastCRomSwap != SMODE_A)
			RefreshCRomBanks( SMODE_A );
	}

	return vNameRam[address & 0x0FFF];
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER5,vRam_Name)
{
	vNameRam[address & 0xFFF] = data;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(MAPPER5,vRam_Attr)
{
	return vNameRam[address & 0x0FFF];
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER5::BankSwitchPRom(const UINT page,UINT bank)
{
	pBanks[page].wRamBank = NO_WRAM;
	pBanks[page].data = pRom.Ram() + ( (bank * 0x2000) & pRom.Mask() );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER5::BankSwitchWRam(const UINT page,UINT bank)
{
	if (bank != NO_WRAM)
	{
		if (bank > 3)
		{
			switch (wRam.Size())
			{
     			case 0x02000UL: 
				case 0x08000UL: bank = 8; break;
     			case 0x04000UL:	bank = 1; break;
     			case 0x10000UL: bank = 4; break;
			}
		}
		else
		{
			switch (wRam.Size())
			{
    			case 0x02000: 
     			case 0x04000:	
					
					bank = 0; 
					break;
			}
		}
	}

	if ((pBanks[page].wRamBank = bank) != NO_WRAM)
		pBanks[page].data = wRam.At( bank * 0x2000 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER5::RefreshCRomBanks(const SMODE sMode)
{
	LastCRomSwap = sMode;

	switch (cMode)
	{
		case CMODE_8K: 
	
			cRom.SwapBanks<n8k,0x0000>( cBanks[sMode][7] ); 
			return;							
	
		case CMODE_4K:						
	
			cRom.SwapBanks<n4k,0x0000>( cBanks[sMode][3] ); 
			cRom.SwapBanks<n4k,0x1000>( cBanks[sMode][7] ); 
			return;							
	
		case CMODE_2K:						
	
			cRom.SwapBanks<n2k,0x0000>( cBanks[sMode][1] ); 
			cRom.SwapBanks<n2k,0x0800>( cBanks[sMode][3] ); 
			cRom.SwapBanks<n2k,0x1000>( cBanks[sMode][5] ); 
			cRom.SwapBanks<n2k,0x1800>( cBanks[sMode][7] ); 
			return;							
	
		case CMODE_1K:						
	
			cRom.SwapBanks<n1k,0x0000>( cBanks[sMode][0] ); 
			cRom.SwapBanks<n1k,0x0400>( cBanks[sMode][1] ); 
			cRom.SwapBanks<n1k,0x0800>( cBanks[sMode][2] ); 
			cRom.SwapBanks<n1k,0x0C00>( cBanks[sMode][3] ); 
			cRom.SwapBanks<n1k,0x1000>( cBanks[sMode][4] ); 
			cRom.SwapBanks<n1k,0x1400>( cBanks[sMode][5] ); 
			cRom.SwapBanks<n1k,0x1800>( cBanks[sMode][6] ); 
			cRom.SwapBanks<n1k,0x1C00>( cBanks[sMode][7] ); 
			return;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER5,5100) { pMode = PMODE(data & 0x3); }
NES_POKE(MAPPER5,5101) { cMode = CMODE(data & 0x3); LastCRomSwap = 0xFF; }
NES_POKE(MAPPER5,5102) { wRamLatch.b.l = data & 0x3; wRamWriteEnable = wRamLatch.d == 0x0102; }
NES_POKE(MAPPER5,5103) { wRamLatch.b.h = data & 0x3; wRamWriteEnable = wRamLatch.d == 0x0102; }
NES_POKE(MAPPER5,5104) { gMode = GMODE(data & 0x3); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER5,5105) 
{
	vNameRam.SwapBanks<n1k,0x0000>( (data >> 0) & 0x3 );
	vNameRam.SwapBanks<n1k,0x0400>( (data >> 2) & 0x3 );
	vNameRam.SwapBanks<n1k,0x0800>( (data >> 4) & 0x3 );
	vNameRam.SwapBanks<n1k,0x0C00>( (data >> 6) & 0x3 );
}

////////////////////////////////////////////////////////////////////////////////////////
//				   
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER5,5106) 
{
	PDX::Fill
	( 
     	FillRam, 
		FillRam + 0x3C0, 
		data 
	);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER5,5107) 
{
	PDX::Fill
	( 
     	FillRam + 0x3C0, 
		FillRam + 0x400,
		(
     		((data & 0x3) << 0) | 
     		((data & 0x3) << 2) | 
     		((data & 0x3) << 4) | 
       		((data & 0x3) << 6)
		)
	);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER5,5113) 
{
	BankSwitchWRam( 3, data & 0x7 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER5,5114)
{
	apu.Update();

	if (pMode == 3)
	{
		if (data & SWITCH_PROM) BankSwitchPRom( 4, data & 0x7F );
		else                    BankSwitchWRam( 4, data & 0x07 );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER5,5115)
{
	apu.Update();

	switch (pMode)
	{
		case 1:
		case 2: 

			if (data & SWITCH_PROM)
			{
				const UINT bank = data & b01111110;

				BankSwitchPRom( 4, bank + 0 );
				BankSwitchPRom( 5, bank + 1 );
			}
			else
			{
				const UINT bank = data & b00000110;

				BankSwitchWRam( 4, bank + 0 ); 
				BankSwitchWRam( 5, bank + 1 ); 
			}
			return;

		case 3: 

			if (data & SWITCH_PROM) BankSwitchPRom( 5, data & 0x7F );
			else                    BankSwitchWRam( 5, data & 0x07 ); 
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER5,5116)
{
	apu.Update();

	switch (pMode)
	{
		case 2: 
		case 3:	
	
			if (data & SWITCH_PROM) BankSwitchPRom( 6, data & 0x7F );
			else                    BankSwitchWRam( 6, data & 0x07 ); 
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER5,5117)
{
	apu.Update();

	switch (pMode)
	{
       	case 0:	
		{
			const UINT bank = data & b01111100;

			BankSwitchPRom( 4, bank + 0 );
			BankSwitchPRom( 5, bank + 1 );
			BankSwitchPRom( 6, bank + 2 );
			BankSwitchPRom( 7, bank + 3 );
			return;
		}

     	case 1: 
		{
			const UINT bank = data & b01111110;

			BankSwitchPRom( 6, bank + 0 ); 
			BankSwitchPRom( 7, bank + 1 ); 
			return;
		}

    	case 2: 
     	case 3: 

			BankSwitchPRom( 7, data & 0x7F ); 
     		return;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER5,5120) { ppu.Update(); cBanks[SMODE_A][0] = data; RefreshCRomBanks(SMODE_A); }
NES_POKE(MAPPER5,5121) { ppu.Update(); cBanks[SMODE_A][1] = data; RefreshCRomBanks(SMODE_A); }
NES_POKE(MAPPER5,5122) { ppu.Update(); cBanks[SMODE_A][2] = data; RefreshCRomBanks(SMODE_A); }
NES_POKE(MAPPER5,5123) { ppu.Update(); cBanks[SMODE_A][3] = data; RefreshCRomBanks(SMODE_A); }
NES_POKE(MAPPER5,5124) { ppu.Update(); cBanks[SMODE_A][4] = data; RefreshCRomBanks(SMODE_A); }
NES_POKE(MAPPER5,5125) { ppu.Update(); cBanks[SMODE_A][5] = data; RefreshCRomBanks(SMODE_A); }
NES_POKE(MAPPER5,5126) { ppu.Update(); cBanks[SMODE_A][6] = data; RefreshCRomBanks(SMODE_A); }
NES_POKE(MAPPER5,5127) { ppu.Update(); cBanks[SMODE_A][7] = data; RefreshCRomBanks(SMODE_A); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER5,5128) { ppu.Update(); cBanks[SMODE_B][0] = data; cBanks[SMODE_B][4] = data; RefreshCRomBanks(SMODE_B); }
NES_POKE(MAPPER5,5129) { ppu.Update(); cBanks[SMODE_B][1] = data; cBanks[SMODE_B][5] = data; RefreshCRomBanks(SMODE_B); }
NES_POKE(MAPPER5,512A) { ppu.Update(); cBanks[SMODE_B][2] = data; cBanks[SMODE_B][6] = data; RefreshCRomBanks(SMODE_B); }
NES_POKE(MAPPER5,512B) { ppu.Update(); cBanks[SMODE_B][3] = data; cBanks[SMODE_B][7] = data; RefreshCRomBanks(SMODE_B); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER5,5200) { SplitCtrl = data;        }
NES_POKE(MAPPER5,5201) {                          }
NES_POKE(MAPPER5,5202) { SplitBank = data & 0x3F; }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER5,5203) 
{ 
	cpu.ClearIRQ();
	IrqLine = data;           
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(MAPPER5,5204)
{
	cpu.ClearIRQ();
	const UINT status = IrqStatus;
	IrqStatus &= ~IRQSTATUS_HIT;
	return status;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER5,5204) 
{ 
	cpu.ClearIRQ();
	SetIrqEnable(data & ENABLE_IRQ);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(MAPPER5,5C00)
{
	return (gMode >= GMODE_EXRAM) ? ExRam[ address & 0x3FF ] : cpu.GetCache();
}

NES_POKE(MAPPER5,5C00) 
{
	if (gMode != GMODE_EXRAM_WRITEPROTECT)
		ExRam[ address & 0x3FF ] = data;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER5,6000)
{
	if (wRamWriteEnable && pBanks[3].wRamBank != NO_WRAM)
		wRam[pBanks[3].wRamBank * 0x2000 + (address & 0x1FFF)] = data;
}

NES_POKE(MAPPER5,8000) 
{
	if (wRamWriteEnable && pBanks[4].wRamBank != NO_WRAM)
		wRam[pBanks[4].wRamBank * 0x2000 + (address & 0x1FFF)] = data;
}

NES_POKE(MAPPER5,A000) 
{
	if (wRamWriteEnable && pBanks[5].wRamBank != NO_WRAM)
		wRam[pBanks[5].wRamBank * 0x2000 + (address & 0x1FFF)] = data;
}

NES_POKE(MAPPER5,C000) 
{
	if (wRamWriteEnable && pBanks[6].wRamBank != NO_WRAM)
		wRam[pBanks[6].wRamBank * 0x2000 + (address & 0x1FFF)] = data;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(MAPPER5,6000) { return pBanks[3].data[address & 0x1FFF]; }
NES_PEEK(MAPPER5,8000) { return pBanks[4].data[address & 0x1FFF]; }
NES_PEEK(MAPPER5,A000) { return pBanks[5].data[address & 0x1FFF]; }
NES_PEEK(MAPPER5,C000) { return pBanks[6].data[address & 0x1FFF]; }
NES_PEEK(MAPPER5,E000) { return pBanks[7].data[address & 0x1FFF]; }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER5::IrqSync()
{
	if (IsA13Low())
	{
		RefreshCRomBanks(SMODE_A);

		const INT ScanLine = ppu.GetScanLine();
		
		if (ScanLine == PPU::SCANLINE_VBLANK)
		{
			IrqStatus |= IRQSTATUS_VBLANK;
		}
		else if (ScanLine + 1 == IrqLine) 
		{
			IrqStatus |= IRQSTATUS_HIT;
		
			if (IsIrqEnabled())
				cpu.DoIRQ();
		}
	}
	else
	{
		RefreshCRomBanks(SMODE_B);
	}
}

NES_NAMESPACE_END
