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
: MAPPER(c), sound(*c.cpu) {}

////////////////////////////////////////////////////////////////////////////////////////
// reset
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER5::Reset()
{
	PDX_ASSERT( wRam.Size() >= n16k );

	if (wRam.Size() < n16k)
		wRam.Resize( n16k );

	cpu.RemoveEvent( PPU::Update );
	cpu.SetEvent( &ppu, PPU::Update );

	ppu.AddHook( this, PpuLatch, PPU::HOOK_RENDERER );
	ppu.AddHook( this, hSync, PPU::HOOK_HSYNC );

	vBgRom.ReAssign( cRom.Ram(), cRom.Size() );
	vNameRam = ppu.GetCiRam(0x0800);

	vNameMode[0] = 0;
	vNameMode[1] = 0;
	vNameMode[2] = 0;
	vNameMode[3] = 0;

	cpu.SetPort( 0x5100,         this, Peek_Nop,  Poke_5100 );
	cpu.SetPort( 0x5101,         this, Peek_Nop,  Poke_5101 );
	cpu.SetPort( 0x5102,         this, Peek_Nop,  Poke_5102 );
	cpu.SetPort( 0x5103,         this, Peek_Nop,  Poke_5103 );
	cpu.SetPort( 0x5104,         this, Peek_Nop,  Poke_5104 );
	cpu.SetPort( 0x5105,         this, Peek_Nop,  Poke_5105 );
	cpu.SetPort( 0x5106,         this, Peek_Nop,  Poke_5106 );
	cpu.SetPort( 0x5107,         this, Peek_Nop,  Poke_5107 );
	cpu.SetPort( 0x5113,         this, Peek_Nop,  Poke_5113 );
	cpu.SetPort( 0x5114,         this, Peek_Nop,  Poke_5114 );
	cpu.SetPort( 0x5115,         this, Peek_Nop,  Poke_5115 );
	cpu.SetPort( 0x5116,         this, Peek_Nop,  Poke_5116 );
	cpu.SetPort( 0x5117,         this, Peek_Nop,  Poke_5117 );
	cpu.SetPort( 0x5120,         this, Peek_Nop,  Poke_5120 );
	cpu.SetPort( 0x5121,         this, Peek_Nop,  Poke_5121 );
	cpu.SetPort( 0x5122,         this, Peek_Nop,  Poke_5122 );
	cpu.SetPort( 0x5123,         this, Peek_Nop,  Poke_5123 );
	cpu.SetPort( 0x5124,         this, Peek_Nop,  Poke_5124 );
	cpu.SetPort( 0x5125,         this, Peek_Nop,  Poke_5125 );
	cpu.SetPort( 0x5126,         this, Peek_Nop,  Poke_5126 );
	cpu.SetPort( 0x5127,         this, Peek_Nop,  Poke_5127 );
	cpu.SetPort( 0x5128,         this, Peek_Nop,  Poke_5128 );
	cpu.SetPort( 0x5129,         this, Peek_Nop,  Poke_5129 );
	cpu.SetPort( 0x512A,         this, Peek_Nop,  Poke_512A );
	cpu.SetPort( 0x512B,         this, Peek_Nop,  Poke_512B );
	cpu.SetPort( 0x5200,         this, Peek_Nop,  Poke_5200 );
	cpu.SetPort( 0x5201,         this, Peek_Nop,  Poke_5201 );
	cpu.SetPort( 0x5202,         this, Peek_Nop,  Poke_5202 );
	cpu.SetPort( 0x5203,         this, Peek_Nop,  Poke_5203 );
	cpu.SetPort( 0x5204,         this, Peek_5204, Poke_5204 );
	cpu.SetPort( 0x5C00, 0x5FFF, this, Peek_5C00, Poke_5C00 );
	cpu.SetPort( 0x6000, 0x7FFF, this, Peek_6000, Poke_6000 );
	cpu.SetPort( 0x8000, 0x9FFF, this, Peek_8000, Poke_8000 );
	cpu.SetPort( 0xA000, 0xBFFF, this, Peek_A000, Poke_A000 );
	cpu.SetPort( 0xC000, 0xDFFF, this, Peek_C000, Poke_C000 );
	cpu.SetPort( 0xE000, 0xFFFF, this, Peek_E000, Poke_Nop  );

	switch (wRam.Size())
	{
		case 0x08000UL: xRam.BankFix = XRAM::BankFix32; break;
		case 0x10000UL: xRam.BankFix = XRAM::BankFix64;	break;
		default:        xRam.BankFix = XRAM::BankFix16; break;
	}

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

	UpdateCRom( vBgRom, cBanks[SMODE_B] );
	UpdateCRom( cRom, cBanks[SMODE_A] );

	fill.attribute = 0;
	fill.character = 0;

	split.ctrl    = 0;
	split.scroll  = 0;
	split.offset  = 0;
	split.address = 0;
	split.x       = 0;
	split.y       = 0;

	irq.clear     = 0;
	irq.enable    = 0;
	irq.line      = 0;
	irq.scanline  = 0;
	irq.status    = 0;

	xRam.latch.d  = 0;
	xRam.write    = 0;
	xRam.enable   = 0;

	xRamSwapBanks<0>( 0 );
	pRomSwapBanks<1>( pRom.LastBank<n8k>() );
	pRomSwapBanks<2>( pRom.LastBank<n8k>() );
	pRomSwapBanks<3>( pRom.LastBank<n8k>() );
	pRomSwapBanks<4>( pRom.LastBank<n8k>() );
  
	sound.Reset();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT MAPPER5::LoadState(PDXFILE& file)
{
	PDX_TRY(MAPPER::LoadState( file ));
	PDX_TRY(vBgRom.LoadState( file, FALSE ));
	PDX_TRY(sound.LoadState( file ));

	if (!file.Readable(sizeof(U8) * 67))
		return PDX_FAILURE;

	gMode = GMODE( file.Read<U8>() );
	pMode = PMODE( file.Read<U8>() );
	cMode = CMODE( file.Read<U8>() );

	fill.character = file.Read<U8>() << 4;
	fill.attribute = file.Read<U8>() << 0;

	xRam.latch.d = file.Read<U16>();
	xRam.enable  = file.Read<U8>();
	xRam.write   = file.Read<U8>();

	split.ctrl    = file.Read<U8>();
	split.scroll  = file.Read<U8>();
	split.offset  = file.Read<U16>();
	split.address = file.Read<U16>();
	split.x       = file.Read<U8>();
	split.y       = file.Read<U8>();

	irq.line     = file.Read<U8>();
	irq.status   = file.Read<U8>();
	irq.clear    = file.Read<U8>();
	irq.scanline = file.Read<U8>();
	irq.enable   = file.Read<U8>();

	for (UINT i=0; i < 4; ++i)
		vNameMode[i] = file.Read<U8>();

	for (UINT i=0; i < 2; ++i)
		for (UINT j=0; j < 8; ++j)
			cBanks[i][j] = file.Read<U8>();

	for (UINT i=0; i < (1+4); ++i)
	{
		const BOOL UseWRAM = file.Read<U8>();
		const ULONG offset = file.Read<U32>();

		if (UseWRAM)
		{
			if (offset <= pRom.Size() - n8k)
				pBanks[i] = pRom.Ram(offset);
		}
		else
		{
			if (offset <= wRam.Size() - n8k)
				pBanks[i] = wRam.At(offset);
		}
	}

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT MAPPER5::SaveState(PDXFILE& file) const
{
	PDX_TRY(MAPPER::SaveState( file ));
	PDX_TRY(vBgRom.SaveState( file, FALSE ));
	PDX_TRY(sound.SaveState( file ));

	file.Write( U8(gMode) );
	file.Write( U8(pMode) );
	file.Write( U8(cMode) );

	file.Write( U8(fill.character >> 4) );
	file.Write( U8(fill.attribute >> 0) );

	file.Write( U16(xRam.latch.d) );
	file.Write( U8(xRam.enable)   );
	file.Write( U8(xRam.write)    );

	file.Write( U8(split.ctrl)     );
	file.Write( U8(split.scroll)   );
	file.Write( U16(split.offset)  );
	file.Write( U16(split.address) );
	file.Write( U8(split.x)        );
	file.Write( U8(split.y)        );

	file.Write( U8(irq.line)     );
	file.Write( U8(irq.status)   );
	file.Write( U8(irq.clear)    );
	file.Write( U8(irq.scanline) );
	file.Write( U8(irq.enable)   );

	for (UINT i=0; i < 4; ++i)
		file.Write( U8(vNameMode[i]) );

	for (UINT i=0; i < 2; ++i)
		for (UINT j=0; j < 8; ++j)
			file.Write( U8(cBanks[i][j]) );

	for (UINT i=0; i < (1+4); ++i)
	{
		if (pBanks[i] >= wRam.Begin() && pBanks[i] <= wRam.End())
		{
			file.Write( U8(1) );
			file.Write( U32(pBanks[i] - wRam.Begin()) );
		}
		else
		{
			file.Write( U8(0) );
			file.Write( U32(pBanks[i] - pRom.Ram()) );
		}
	}

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL MAPPER5::DoSplit(U8* const PDX_RESTRICT data)
{
	const UINT SplitTile = split.ctrl & SPLIT::POS;

	if (split.ctrl & SPLIT::RIGHT)
	{
		if (SplitTile > split.x)
			return FALSE;
	}
	else
	{
		if (SplitTile <= split.x)
			return FALSE;
	}

	const UINT name = ((split.address & 0x3E0) | (split.x & 0x1F)) & 0x3FF;				
	
	UINT attribute = vNameRam[0x3C0 + ((name & 0x380) >> 4) + ((name & 0x1C) >> 2)];

	if (name & 0x02) attribute >>= 2;
	if (name & 0x40) attribute >>= 4;

	data[0] = (attribute & 0x3) << 2;

	const UINT PatternAddress = (vNameRam[name] << 4) + split.y + split.offset;

	data[1] = *cRom.Ram(PatternAddress + 0x0);
	data[2] = *cRom.Ram(PatternAddress + 0x8);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER5::DoFillExGfx(U8* const PDX_RESTRICT pattern)
{
	const UINT vRamAddress    = ppu.GetVRamAddress();
	const UINT TileAddress    = fill.character + (vRamAddress >> 12);
	const UINT TileBank       = ((vNameRam[vRamAddress & 0x3FF] & 0x3F) << 12) & cRom.Mask();
	const UINT PatternAddress = TileAddress + TileBank;

	pattern[0] = *cRom.Ram(PatternAddress + 0x0);
	pattern[1] = *cRom.Ram(PatternAddress + 0x8);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER5::DoFillNormal(U8* const PDX_RESTRICT pattern)
{
	const UINT vRamAddress    = ppu.GetVRamAddress();
	const UINT TileAddress    = fill.character + (vRamAddress >> 12);
	const UINT PatternAddress = TileAddress + ppu.GetBgPatternAddress();

	pattern[0] = vBgRom[PatternAddress + 0x0];
	pattern[1] = vBgRom[PatternAddress + 0x8];
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER5::DoExGfx(U8* const PDX_RESTRICT data)
{
	const UINT vRamAddress    = ppu.GetVRamAddress();
	const UINT attribute      = vNameRam[vRamAddress & 0x3FF];
	const UINT TileAddress    = (ppu.Peek_CiRam(vRamAddress & 0xFFF) << 4) + (vRamAddress >> 12);
	const UINT TileBank       = ((attribute & 0x3F) << 12) & cRom.Mask();
	const UINT PatternAddress = TileAddress + TileBank; 

	data[0] = (attribute & 0xC0) >> 4;
	data[1] = *cRom.Ram(PatternAddress + 0x0);
	data[2] = *cRom.Ram(PatternAddress + 0x8);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER5::DoNormal(U8* const PDX_RESTRICT data)
{
	const UINT vRamAddress    = ppu.GetVRamAddress();
	const UINT AttrAddress    = 0x3C0 + (vRamAddress & 0xC00) + ((vRamAddress & 0x380) >> 4) + ((vRamAddress & 0x1C) >> 2);
	const UINT PatternAddress = ppu.GetBgPatternAddress() + (ppu.Peek_CiRam(vRamAddress & 0xFFF) << 4) + (vRamAddress >> 12);

	UINT attribute = ppu.Peek_CiRam(AttrAddress);

	if (vRamAddress & 0x02) attribute >>= 2;
	if (vRamAddress & 0x40) attribute >>= 4;

	data[0] = (attribute & 0x3) << 2;
	data[1] = vBgRom[PatternAddress + 0x0];
	data[2] = vBgRom[PatternAddress + 0x8];
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER5::PpuLatch(U8* const PDX_RESTRICT data)
{
	if (ppu.IsBgEnabled())
	{
		if (!(split.ctrl & SPLIT::ENABLE) || !DoSplit( data ))
		{
			const UINT offset = (ppu.GetVRamAddress() & 0x0C00) >> 10;

			if (vNameMode[offset] == FILL::MODE)
			{
				data[0] = fill.attribute;

				if (gMode == GMODE_EXGFX)
					DoFillExGfx( data+1 );
				else
					DoFillNormal( data+1 );
			}
			else 
			{
				if (gMode == GMODE_EXGFX)
					DoExGfx( data );
				else
					DoNormal( data );
			}
		}
	}

	++split.x;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER5::UpdateCRom(CROM& vRom,const UINT* const banks) const
{
	switch (cMode)
	{
       	case CMODE_8K:

			vRom.SwapBanks<n8k,0x0000>( banks[7] ); 
			break;

     	case CMODE_4K:

			vRom.SwapBanks<n4k,0x0000>( banks[3] ); 
			vRom.SwapBanks<n4k,0x1000>( banks[7] ); 
			break;

		case CMODE_2K:

			vRom.SwapBanks<n2k,0x0000>( banks[1] ); 
			vRom.SwapBanks<n2k,0x0800>( banks[3] ); 
			vRom.SwapBanks<n2k,0x1000>( banks[5] ); 
			vRom.SwapBanks<n2k,0x1800>( banks[7] ); 
			break;

		case CMODE_1K:

			vRom.SwapBanks<n1k,0x0000>( banks[0] ); 
			vRom.SwapBanks<n1k,0x0400>( banks[1] ); 
			vRom.SwapBanks<n1k,0x0800>( banks[2] ); 
			vRom.SwapBanks<n1k,0x0C00>( banks[3] ); 
			vRom.SwapBanks<n1k,0x1000>( banks[4] ); 
			vRom.SwapBanks<n1k,0x1400>( banks[5] ); 
			vRom.SwapBanks<n1k,0x1800>( banks[6] ); 
			vRom.SwapBanks<n1k,0x1C00>( banks[7] ); 
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER5,5100) { pMode = PMODE(data & 0x3); }
NES_POKE(MAPPER5,5101) { ppu.Update(); cMode = CMODE(data & 0x3); }
NES_POKE(MAPPER5,5102) { xRam.latch.b.l = data & 0x3; xRam.write = (xRam.latch.d == 0x0102 ? ~0U : 0U); }
NES_POKE(MAPPER5,5103) { xRam.latch.b.h = data & 0x3; xRam.write = (xRam.latch.d == 0x0102 ? ~0U : 0U); }
NES_POKE(MAPPER5,5104) { ppu.Update(); gMode = GMODE(data & 0x3); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER5,5105) 
{
	ppu.Update();

	ppu.SetMirroring
	(
       	vNameMode[0] = (data >> 0) & 0x3,
		vNameMode[1] = (data >> 2) & 0x3,
		vNameMode[2] = (data >> 4) & 0x3,
		vNameMode[3] = (data >> 6) & 0x3
	);
}

////////////////////////////////////////////////////////////////////////////////////////
//				   
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER5,5106)
{
	ppu.Update(); 
	fill.character = data << 4;
}

NES_POKE(MAPPER5,5107)
{
	ppu.Update(); 
	fill.attribute = (data << 2) & 0xC;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER5,5113) 
{
	apu.Update();
	xRamSwapBanks<0>( data & 0x07 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER5,5114)
{
	if (pMode == PMODE_8K)
	{
		apu.Update();

		if (data & SWITCH_PROM)
			pRomSwapBanks<1>( data & 0x7F );
		else
			xRamSwapBanks<1>( data & 0x07 );
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
   		case PMODE_16K:
		case PMODE_16K_8K: 
			
			if (data & SWITCH_PROM)
			{
				const UINT bank = data & 0x7E;
				pRomSwapBanks<1>( bank + 0 );
				pRomSwapBanks<2>( bank + 1 );
			}
			else 
			{
				const UINT bank = data & 0x06;
				xRamSwapBanks<1>( bank + 0 );
				xRamSwapBanks<2>( bank + 1 );
			}
			break;

		case PMODE_8K: 
				
			if (data & SWITCH_PROM)
				pRomSwapBanks<2>( data & 0x7F ); 
			else
				xRamSwapBanks<2>( data & 0x07 );

			break;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER5,5116)
{
	switch (pMode)
	{
     	case PMODE_16K_8K:
		case PMODE_8K:

			apu.Update();

			if (data & SWITCH_PROM)
				pRomSwapBanks<3>( data & 0x7F );
			else
				xRamSwapBanks<3>( data & 0x07 );
			
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER5,5117)
{
	if (data & SWITCH_PROM)
	{
		apu.Update();

		switch (pMode)
		{
       		case PMODE_32K:
			{
				const UINT bank = data & 0x7D;
				pRomSwapBanks<1>( bank + 0 );
				pRomSwapBanks<2>( bank + 1 );
				pRomSwapBanks<3>( bank + 2 );
				pRomSwapBanks<4>( bank + 3 );
				break;
			}

       		case PMODE_16K:
			{
				const UINT bank = data & 0x7E;
				pRomSwapBanks<3>( bank + 0 );
				pRomSwapBanks<4>( bank + 1 );
				break;
			}

	     	case PMODE_16K_8K:
       		case PMODE_8K:

       			pRomSwapBanks<4>( data & 0x7F );
     			break;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

template<UINT PAGE> 
VOID MAPPER5::pRomSwapBanks(UINT bank)
{
	xRam.enable &= ~(1 << PAGE);
	pBanks[PAGE] = pRom.Ram( bank << 13 );
}

template<UINT PAGE> 
VOID MAPPER5::xRamSwapBanks(UINT bank)
{
	bank = xRam.BankFix( bank );

	if (bank != 8)
	{
		xRam.enable |= (1 << PAGE);
		pBanks[PAGE] = wRam.At(bank << 13);
	}
	else
	{
		xRam.enable &= ~(1 << PAGE);
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER5,5120) { ppu.Update(); cBanks[SMODE_A][0] = data; UpdateCRom( cRom, cBanks[SMODE_A] ); }
NES_POKE(MAPPER5,5121) { ppu.Update(); cBanks[SMODE_A][1] = data; UpdateCRom( cRom, cBanks[SMODE_A] ); }
NES_POKE(MAPPER5,5122) { ppu.Update(); cBanks[SMODE_A][2] = data; UpdateCRom( cRom, cBanks[SMODE_A] ); }
NES_POKE(MAPPER5,5123) { ppu.Update(); cBanks[SMODE_A][3] = data; UpdateCRom( cRom, cBanks[SMODE_A] ); }
NES_POKE(MAPPER5,5124) { ppu.Update(); cBanks[SMODE_A][4] = data; UpdateCRom( cRom, cBanks[SMODE_A] ); }
NES_POKE(MAPPER5,5125) { ppu.Update(); cBanks[SMODE_A][5] = data; UpdateCRom( cRom, cBanks[SMODE_A] ); }
NES_POKE(MAPPER5,5126) { ppu.Update(); cBanks[SMODE_A][6] = data; UpdateCRom( cRom, cBanks[SMODE_A] ); }
NES_POKE(MAPPER5,5127) { ppu.Update(); cBanks[SMODE_A][7] = data; UpdateCRom( cRom, cBanks[SMODE_A] ); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER5,5128) { ppu.Update(); cBanks[SMODE_B][0] = cBanks[SMODE_B][4] = data; UpdateCRom( vBgRom, cBanks[SMODE_B] ); }
NES_POKE(MAPPER5,5129) { ppu.Update(); cBanks[SMODE_B][1] = cBanks[SMODE_B][5] = data; UpdateCRom( vBgRom, cBanks[SMODE_B] ); }
NES_POKE(MAPPER5,512A) { ppu.Update(); cBanks[SMODE_B][2] = cBanks[SMODE_B][6] = data; UpdateCRom( vBgRom, cBanks[SMODE_B] ); }
NES_POKE(MAPPER5,512B) { ppu.Update(); cBanks[SMODE_B][3] = cBanks[SMODE_B][7] = data; UpdateCRom( vBgRom, cBanks[SMODE_B] ); }
																								   
////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER5,5200) { ppu.Update(); split.ctrl = data; }
NES_POKE(MAPPER5,5201) { ppu.Update(); split.scroll = data; }
NES_POKE(MAPPER5,5202) { ppu.Update(); split.offset = ((data & 0x3F) << 12) & cRom.Mask(); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER5,5203) 
{ 
	cpu.ClearIRQ();
	irq.line = data;           
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(MAPPER5,5204)
{
	cpu.ClearIRQ();
	const UINT status = irq.status;
	irq.status &= ~IRQ::STATUS_HIT;
	return status;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER5,5204) 
{ 
	cpu.ClearIRQ();
	irq.enable = data & IRQ::ENABLE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(MAPPER5,5C00)
{
	ppu.Update();
	
	if (gMode >= GMODE_EXRAM) 
		return vNameRam[address & 0x03FF];
	
	return cpu.GetCache();
}

NES_POKE(MAPPER5,5C00) 
{
	ppu.Update();

	U8& vData = vNameRam[address & 0x03FF];

	if (gMode == GMODE_EXRAM)
	{
		vData = data;
	}
	else if (gMode != GMODE_EXRAM_WP)
	{
		vData = (irq.status & IRQ::STATUS_ACTIVE) ? data : 0x00;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(MAPPER5,6000) { if (xRam.write & xRam.enable & 0x1) pBanks[0][address & 0x1FFF] = data; }
NES_POKE(MAPPER5,8000) { if (xRam.write & xRam.enable & 0x2) pBanks[1][address & 0x1FFF] = data; }
NES_POKE(MAPPER5,A000) { if (xRam.write & xRam.enable & 0x4) pBanks[2][address & 0x1FFF] = data; }
NES_POKE(MAPPER5,C000) { if (xRam.write & xRam.enable & 0x8) pBanks[3][address & 0x1FFF] = data; }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(MAPPER5,6000) { return pBanks[0][address & 0x1FFF]; }
NES_PEEK(MAPPER5,8000) { return pBanks[1][address & 0x1FFF]; }
NES_PEEK(MAPPER5,A000) { return pBanks[2][address & 0x1FFF]; }
NES_PEEK(MAPPER5,C000) { return pBanks[3][address & 0x1FFF]; }
NES_PEEK(MAPPER5,E000) { return pBanks[4][address & 0x1FFF]; }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MAPPER5::hSync()
{
	const INT scanline = ppu.GetScanline();

	if (ppu.IsEnabled() && scanline != PPU::SCANLINE_VBLANK)
	{
		irq.clear = 0;
		irq.status |= IRQ::STATUS_ACTIVE;
		++irq.scanline;
	}

	if (irq.scanline == irq.line)
		irq.status |= IRQ::STATUS_HIT;

	if (++irq.clear > 1)
	{
		irq.scanline = 0;
		irq.status &= ~IRQ::SIGNAL;
		cpu.ClearIRQ();
	}
	else if (irq.enable && (irq.status & IRQ::SIGNAL) == IRQ::SIGNAL)
	{
		cpu.DoIRQ();
	}

	split.x = 0;

	if (scanline == 0) 
	{
		split.y = split.scroll & SPLIT::Y;
		split.address = (split.scroll & SPLIT::SCROLL) << 2;
	} 
	else if (ppu.IsEnabled()) 
	{
		if (split.y++ == SPLIT::Y) 
		{
			split.y = 0;
			
			switch (split.address & 0x03E0)
			{
     			case 0x03A0:
				case 0x03E0: 
					
					split.address &= 0x001F;
					break;

				default:

					split.address += 0x0020;
					break;
			}
		} 
	}
}

NES_NAMESPACE_END
