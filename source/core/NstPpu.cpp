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

#include "../paradox/PdxFile.h"
#include "NstTypes.h"
#include "NstCpu.h"
#include "NstPpu.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
// constructor
////////////////////////////////////////////////////////////////////////////////////////

PPU::PPU(CPU& c)
:
CiRam       (n4k),
GarbageLine (new IO::GFX::PIXEL[MARGIN + IO::GFX::WIDTH]),
cpu         (c),
pal         (FALSE)
{}

////////////////////////////////////////////////////////////////////////////////////////
// destructor
////////////////////////////////////////////////////////////////////////////////////////

PPU::~PPU()
{
	delete [] GarbageLine;
}

////////////////////////////////////////////////////////////////////////////////////////
// reset ppu
////////////////////////////////////////////////////////////////////////////////////////

VOID PPU::Reset(const BOOL hard)
{
	LogOutput("PPU: reset");

	for (UINT i=0; i < 4; ++i)
		logged[i] = false;

	if (hard)
	{
		CiRam.Clear();

		PDXMemZero( cRam,   n8k );
		PDXMemZero( SpRam,  256 );
		PDXMemZero( PalRam,	 32 );
	}

	cycles.SetMode(pal);
	cycles.WarmUp = hard ? CYCLES::WARM_UP : CYCLES::READY;

	ctrl0             = 0;
	ctrl1             = 0;
	status            = 0;	
	enabled           = FALSE;
	vRamAddress       = 0x2000;
	vRamLatch         = 0x2000;
	OamAddress        = 0x0000;
	OamLatch          = 0x00;
	DmaLatch          = 0x20;
	latch             = 0;
	ReadLatch         = 0;
	xFine             = 0;
	FlipFlop          = 0;
	SpIndex           = 0;
	MaxSprites        = 8;
	EvenFrame         = 1^1;
	ScanLine          = SCANLINE_VBLANK;
	SpTmpBufferSize   = 0;
	SpBufferSize      = 0;
	screen            = NULL;
	process           = processes;
	output.index      = 0;
	output.clipping   = 0xFF;
	output.monochrome = 0xFF;
	output.pixels     = NULL;

	UpdateTmpVariables();

	// Default to horizontal mirroring

	CiRam.SwapBanks<n1k,0x0000>(0);
	CiRam.SwapBanks<n1k,0x0400>(0);
	CiRam.SwapBanks<n1k,0x0800>(1);
	CiRam.SwapBanks<n1k,0x0C00>(1);

	vRam.SetPort( 0x0000, 0x1FFF, this, Peek_vRam_00xx, Poke_vRam_00xx );
	vRam.SetPort( 0x2000, 0x23FF, this, Peek_vRam_20xx, Poke_vRam_20xx );
	vRam.SetPort( 0x2400, 0x27FF, this, Peek_vRam_24xx, Poke_vRam_24xx );
	vRam.SetPort( 0x2800, 0x2BFF, this, Peek_vRam_28xx, Poke_vRam_28xx );
	vRam.SetPort( 0x2C00, 0x2FFF, this, Peek_vRam_2Cxx, Poke_vRam_2Cxx );
	vRam.SetPort( 0x3000, 0x33FF, this, Peek_vRam_20xx, Poke_vRam_20xx );
	vRam.SetPort( 0x3400, 0x37FF, this, Peek_vRam_24xx, Poke_vRam_24xx );
	vRam.SetPort( 0x3800, 0x3BFF, this, Peek_vRam_28xx, Poke_vRam_28xx );
	vRam.SetPort( 0x3C00, 0x3FFF, this, Peek_vRam_2Cxx, Poke_vRam_2Cxx );

	for (UINT i=0x2000; i < 0x4000; i += 0x8)
	{
		cpu.SetPort( i + 0x0, this, Peek_2xxx, Poke_2000 );
		cpu.SetPort( i + 0x1, this, Peek_2xxx, Poke_2001 );
		cpu.SetPort( i + 0x2, this, Peek_2002, Poke_2xxx );
		cpu.SetPort( i + 0x3, this, Peek_2xxx, Poke_2003 );
		cpu.SetPort( i + 0x4, this, Peek_2004, Poke_2004 );
		cpu.SetPort( i + 0x5, this, Peek_2xxx, Poke_2005 );
		cpu.SetPort( i + 0x6, this, Peek_2xxx, Poke_2006 );
		cpu.SetPort( i + 0x7, this, Peek_2007, Poke_2007 );
	}

	cpu.SetPort( 0x4014, this, Peek_4014, Poke_4014 );

	PDXMemZero( SpBuffer,    64 );
	PDXMemZero( SpTmpBuffer, 64 );
	PDXMemZero( BgBuffer        );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID PPU::UpdateTmpVariables()
{
	enabled          = ctrl1 & (CTRL1_BG_ENABLED | CTRL1_SP_ENABLED);
	AddressIncrease  = (ctrl0 & CTRL0_INC32) ? 32 : 1;
	SpHeight         = (ctrl0 & CTRL0_SP8X16) ? 16 : 8;
	BgPatternAddress = (ctrl0 & CTRL0_BG_OFFSET) ? 0x1000 : 0x0000;
	SpPatternAddress = (ctrl0 & CTRL0_SP_OFFSET) ? 0x1000 : 0x0000;
	emphasis         = ((ctrl1 & CTRL1_BG_COLOR) >> CTRL1_BG_COLOR_SHIFT) * 64;
	monochrome       = (ctrl1 & CTRL1_MONOCHROME) ? 0xF0 : 0xFF;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID PPU::CYCLES::Reset(const BOOL pal,const BOOL EvenFrame)
{
	SetMode( pal, EvenFrame );

	if (WarmUp == READY)
	{
		count = 0;
	}
	else
	{
		++WarmUp;
		count = frame;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID PPU::CYCLES::SetMode(const BOOL pal,const BOOL EvenFrame)
{
	if (pal)
	{
		fetch = NES_PPU_MCC_FETCH_PAL;
		pixel = NES_PPU_MCC_PIXEL_PAL;
		frame = EvenFrame ? NES_PPU_MCC_FRAME_0_PAL : NES_PPU_MCC_FRAME_1_PAL;
		vint  = NES_PPU_MCC_VINT_PAL;
	}
	else
	{
		fetch = NES_PPU_MCC_FETCH_NTSC;
		pixel = NES_PPU_MCC_PIXEL_NTSC;
		frame = EvenFrame ? NES_PPU_MCC_FRAME_0_NTSC : NES_PPU_MCC_FRAME_1_NTSC;
		vint  = NES_PPU_MCC_VINT_NTSC;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(PPU,vRam_00xx) { cRam[address & 0x1FFF] = data;   }
NES_POKE(PPU,vRam_20xx) { CiRam(0,address & 0x3FF) = data; }
NES_POKE(PPU,vRam_24xx) { CiRam(1,address & 0x3FF) = data; }
NES_POKE(PPU,vRam_28xx) { CiRam(2,address & 0x3FF) = data; }
NES_POKE(PPU,vRam_2Cxx) { CiRam(3,address & 0x3FF) = data; }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(PPU,vRam_00xx) { return cRam[address & 0x1FFF];  }
NES_PEEK(PPU,vRam_20xx) { return CiRam(0,address & 0x3FF); } 
NES_PEEK(PPU,vRam_24xx) { return CiRam(1,address & 0x3FF); } 
NES_PEEK(PPU,vRam_28xx) { return CiRam(2,address & 0x3FF); } 
NES_PEEK(PPU,vRam_2Cxx) { return CiRam(3,address & 0x3FF); } 

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID PPU::SetMirroring(const MIRRORING type)
{
	PDX_ASSERT(UINT(type) < 7);

	PDX_COMPILE_ASSERT
	(
	    MIRROR_HORIZONTAL == 0 &&
		MIRROR_VERTICAL   == 1 &&
		MIRROR_FOURSCREEN == 2 &&
		MIRROR_ZERO       == 3 &&
		MIRROR_ONE        == 4 &&
		MIRROR_TWO        == 5 &&
		MIRROR_THREE      == 6
	);

	Update();

	static const UCHAR select[7][4] =
	{
		{0,0,1,1}, // horizontal
		{0,1,0,1}, // vertical
		{0,1,2,3}, // four-screen
		{0,0,0,0}, // banks #1
		{1,1,1,1}, // banks #2
		{2,2,2,2}, // banks	#3
		{3,3,3,3}  // banks	#4
	};

	CiRam.SwapBanks<n1k,0x0000>( select[type][0] );
	CiRam.SwapBanks<n1k,0x0400>( select[type][1] );
	CiRam.SwapBanks<n1k,0x0800>( select[type][2] );
	CiRam.SwapBanks<n1k,0x0C00>( select[type][3] );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID PPU::SetMode(const MODE mode)
{
	pal = (mode == MODE_PAL);
	cycles.SetMode(pal,EvenFrame);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID PPU::SetContext(const IO::GFX::CONTEXT& context)
{
	MaxSprites = context.InfiniteSprites ? 64 : 8;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID PPU::GetContext(IO::GFX::CONTEXT& context) const
{
	context.InfiniteSprites = (MaxSprites == 64);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID PPU::Update()
{
	const ULONG target = PDX_MIN
	(
    	cycles.frame,
		cpu.GetCycles<CPU::CYCLE_MASTER>()
	);

	while (cycles.count < target)
		(this->**process++)();
}

////////////////////////////////////////////////////////////////////////////////////////
// write to control register 0
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(PPU,2000)
{
	Update();

	ctrl0 = latch = data;

	AddressIncrease  = (data & CTRL0_INC32) ? 32 : 1;
	SpHeight         = (data & CTRL0_SP8X16) ? 16 : 8;
	BgPatternAddress = (data & CTRL0_BG_OFFSET) ? 0x1000 : 0x0000;
	SpPatternAddress = (data & CTRL0_SP_OFFSET) ? 0x1000 : 0x0000;
	
	vRamLatch &= ~VRAM_NAME;
	vRamLatch |= (data & CTRL0_NAME_OFFSET) << 10;

	const BOOL retrigger = !cpu.IsLine(CPU::NMI) && (data & CTRL0_NMI) && (status & STATUS_VBLANK);

	cpu.SetLine(CPU::NMI,data & CTRL0_NMI);

	if (retrigger)
		cpu.DoNMI();
}

////////////////////////////////////////////////////////////////////////////////////////
// write to control register 1
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(PPU,2001)
{
	Update();

	enabled    = data & (CTRL1_BG_ENABLED | CTRL1_SP_ENABLED);
	emphasis   = ((data & CTRL1_BG_COLOR) >> CTRL1_BG_COLOR_SHIFT) * IO::GFX::PALETTE_CHUNK;
	monochrome = (data & CTRL1_MONOCHROME) ? 0xF0 : 0xFF;

	if ((data & CTRL1_BG_ENABLED) < (ctrl1 & CTRL1_BG_ENABLED))
	{
		BgBuffer.address    = 0x0000;
		BgBuffer.name       = 0x00;
		BgBuffer.attribute  = 0x00;
		BgBuffer.pattern[0] = 0x00;
		BgBuffer.pattern[1] = 0x00;
		
		PDXMemZero( BgBuffer.pixels, 16 );
	}

	ctrl1 = latch = data;
}

////////////////////////////////////////////////////////////////////////////////////////
// read the status register
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(PPU,2002)
{
	Update();

	FlipFlop = 0;

	latch = (latch & STATUS_LATCH) | status;
	status &= ~STATUS_VBLANK;

	return latch;
}

////////////////////////////////////////////////////////////////////////////////////////
// write to the sprite RAM address counter
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(PPU,2003)
{
	Update();

	OamAddress = latch = data;
	OamLatch = latch & 0x7;
}

////////////////////////////////////////////////////////////////////////////////////////
// write a byte to the current sprite RAM address
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(PPU,2004)
{
	Update();

	latch = data;

	if (OamLatch >= 0x8)
	{
		if (OamAddress >= 0x8)
			SpRam[OamAddress] = data;
	}
	else
	{
		SpRam[OamLatch] = data;
	}

	OamLatch = (OamLatch + 1) & 0xFF;
	OamAddress = (OamAddress + 1) & 0xFF;
}

////////////////////////////////////////////////////////////////////////////////////////
// read from sprite RAM
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(PPU,2004)
{
	Update();

	latch = SpRam[OamAddress];
	OamAddress = (OamAddress + 1) & 0xFF;

	return latch;
}

////////////////////////////////////////////////////////////////////////////////////////
// write a compressed byte to the scroll register
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(PPU,2005)
{
	Update();

	latch = data;

	if (FlipFlop ^= 1)
	{
       	// First write. Store the x tile and x fine. 
		
		vRamLatch &= ~VRAM_X_TILE;
		vRamLatch |= latch >> 3;
		xFine = latch & b00000111;
	}
	else
	{
		// Second write. Store the y tile and y fine.

		vRamLatch &= ~(VRAM_Y_TILE|VRAM_Y_FINE);
		vRamLatch |= (latch & b11111000) << 2;
		vRamLatch |= (latch & b00000111) << 12;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// write to the register controling the VRAM address
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(PPU,2006)
{
	Update();

	latch = data;

	if (FlipFlop ^= 1)
	{
		// First write. Store the high byte 6 bits 
		// and clear out the last two.

		vRamLatch &= ~VRAM_HIGH;
		vRamLatch |= (latch & b00111111) << 8;
	}
	else
	{
		// Second write. Store the low byte 8 bits.
		
		vRamLatch &= ~VRAM_LOW;
		vRamLatch |= latch;
		vRamAddress = vRamLatch;
		vRam[vRamAddress & 0x3FFF];
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// write a byte to VRAM
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(PPU,2007)
{
	Update();

	latch = data;

	UINT offset = vRamAddress;
	vRamAddress += AddressIncrease;

	if (offset >= 0x3F00)
	{
		if (offset < 0x4000)
		{			  
			WritePalRam( offset );
			return;
		}

		offset &= 0x3FFF;
	}

	vRam.Poke( offset, data );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID PPU::WritePalRam(const UINT address)
{
	if (!(address & 0xF))
	{
		PalRam[0x00] = 
		PalRam[0x04] = 
		PalRam[0x08] = 
		PalRam[0x0C] =			
		PalRam[0x10] = 
		PalRam[0x14] = 
		PalRam[0x18] = 
		PalRam[0x1C] = latch & b111111;
	}
	else if (address & 0x3)
	{
		PalRam[address & 0x1F] = latch & b111111;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// read a byte from VRAM
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(PPU,2007)
{
	Update();

	UINT offset = vRamAddress;
	vRamAddress += AddressIncrease;

	// first read is always a throw-away read (except palette RAM)

	if (offset >= 0x3F00)
	{
		if (offset < 0x4000)
			return latch = PalRam[offset & 0x1F];

		offset &= 0x3FFF;
	}

	latch = ReadLatch;
	ReadLatch = vRam[offset];

	return latch;
}

////////////////////////////////////////////////////////////////////////////////////////
// put any crap that gets written on the bus
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(PPU,2xxx)
{
	Update();
	Log("PPU: useless write to 0x2xxx",0);
	latch = data;
}

////////////////////////////////////////////////////////////////////////////////////////
// read what's left on the bus
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(PPU,2xxx)
{
	Update();
	Log("PPU: useless read from 0x2xxx",1);
	return latch;
}

////////////////////////////////////////////////////////////////////////////////////////
// sprite DMA
////////////////////////////////////////////////////////////////////////////////////////

NES_POKE(PPU,4014)
{
	Update();

	DmaLatch = data;

	if (ScanLine == SCANLINE_VBLANK || enabled)
	{
		const UINT offset = data << 8;
		const UINT length = offset + 256;

		UINT byte;

		for (UINT i=offset; i < length; ++i)
		{
			byte = cpu.Peek(i);

			if (OamLatch >= 0x8)
			{
				if (OamAddress >= 0x8)
					SpRam[OamAddress] = byte;
			}
			else
			{
				SpRam[OamLatch] = byte;
			}

			OamLatch = (OamLatch + 1) & 0xFF;
			OamAddress = (OamAddress + 1) & 0xFF;
		}

		latch = byte;
		cpu.AdvanceCycles( pal ? NES_PPU_MCC_SPRITE_DMA_PAL : NES_PPU_MCC_SPRITE_DMA_NTSC );
	}
	else
	{
		latch = data;
		Log("PPU: sprite DMA transfer denied",2);
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// return the current sprite DMA address
////////////////////////////////////////////////////////////////////////////////////////

NES_PEEK(PPU,4014)
{
	Update();
	Log("PPU: read from 0x4014",3);
	return DmaLatch;
}

////////////////////////////////////////////////////////////////////////////////////////
// start vertical blanking
////////////////////////////////////////////////////////////////////////////////////////

VOID PPU::BeginVBlank()
{
	OamLatch = 0x00;
	status |= STATUS_VBLANK;
	ScanLine = SCANLINE_VBLANK;
	cycles.count = cycles.vint;
}

////////////////////////////////////////////////////////////////////////////////////////
// end vertical blanking
////////////////////////////////////////////////////////////////////////////////////////

VOID PPU::EndVBlank()
{
	status &= ~(STATUS_VBLANK|STATUS_SP_ZERO_HIT|STATUS_SP_OVERFLOW);

	SpTmpBufferSize = 0;
	SpBufferSize = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// dummy name table byte fetch
////////////////////////////////////////////////////////////////////////////////////////

VOID PPU::FetchName()
{
	cycles.count += cycles.fetch;

	if (enabled)
		vRam[0x2000 + (vRamAddress & 0x0FFF)];
}

////////////////////////////////////////////////////////////////////////////////////////
// dummy attribute table fetch
////////////////////////////////////////////////////////////////////////////////////////

VOID PPU::FetchAttribute()
{
	cycles.count += cycles.fetch;

	if (enabled)
	{
		const UINT address = 
		(
	    	0x23C0 + 
			((vRamAddress & 0x0C00) >> 0) + 
			((vRamAddress & 0x0380) >> 4) + 
			((vRamAddress & 0x001F) >> 2)
		);

		vRam[address];
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// dummy pattern table byte fetch #1
////////////////////////////////////////////////////////////////////////////////////////

VOID PPU::FetchPattern0()
{
	cycles.count += cycles.fetch;

	if (enabled)
		vRam[BgPatternAddress];
}

////////////////////////////////////////////////////////////////////////////////////////
// dummy pattern table byte fetch #2
////////////////////////////////////////////////////////////////////////////////////////

VOID PPU::FetchPattern1()
{
	cycles.count += cycles.fetch;

	if (enabled)
		vRam[BgPatternAddress+8];
}

////////////////////////////////////////////////////////////////////////////////////////
// name table byte fetch
////////////////////////////////////////////////////////////////////////////////////////

VOID PPU::PreFetchBgName()
{
	cycles.count += cycles.fetch;

	if (enabled)
		BgBuffer.name = vRam[0x2000 + (vRamAddress & 0x0FFF)];
}

////////////////////////////////////////////////////////////////////////////////////////
// name table byte fetch
////////////////////////////////////////////////////////////////////////////////////////

VOID PPU::FetchBgName()
{
	if (enabled)
	{
		if ((vRamAddress & VRAM_X_TILE) == VRAM_X_TILE)
		{
			vRamAddress ^= VRAM_NAME_LOW;
			vRamAddress &= ~VRAM_X_TILE;
		}
		else
		{
			++vRamAddress;
		}

		BgBuffer.name = vRam[0x2000 + (vRamAddress & 0x0FFF)];
	}

	BgBuffer.index &= 0xF;

	if (ctrl1 & CTRL1_BG_ENABLED)
	{
		U8* const PDX_RESTRICT pixels = BgBuffer.pixels + (BgBuffer.index ^ 8);

		const UINT pattern =
		(
			((BgBuffer.pattern[0] & b01010101) << 0) |
			((BgBuffer.pattern[0] & b10101010) << 7) |
			((BgBuffer.pattern[1] & b01010101) << 1) |
			((BgBuffer.pattern[1] & b10101010) << 8) 
		);
				
		pixels[0] = ( (pattern >> 0xE) & 0x3 ) + BgBuffer.attribute;
		pixels[1] = ( (pattern >> 0x6) & 0x3 ) + BgBuffer.attribute;
		pixels[2] = ( (pattern >> 0xC) & 0x3 ) + BgBuffer.attribute;
		pixels[3] = ( (pattern >> 0x4) & 0x3 ) + BgBuffer.attribute;
		pixels[4] = ( (pattern >> 0xA) & 0x3 ) + BgBuffer.attribute;
		pixels[5] = ( (pattern >> 0x2) & 0x3 ) + BgBuffer.attribute;
		pixels[6] = ( (pattern >> 0x8) & 0x3 ) + BgBuffer.attribute;
		pixels[7] = ( (pattern >> 0x0) & 0x3 ) + BgBuffer.attribute;

		if (output.index >= 8 || (ctrl1 & CTRL1_BG_NO_CLIPPING))
		{
			output.clipping = 0xFF;
			output.monochrome = monochrome;
		}
		else
		{
			output.clipping = 0x00;
			output.monochrome = 0xFF;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// attribute table fetch with cycle adjustment
////////////////////////////////////////////////////////////////////////////////////////

VOID PPU::PreFetchBgAttribute()
{
	cycles.count += cycles.fetch;
	FetchBgAttribute();
}

////////////////////////////////////////////////////////////////////////////////////////
// attribute table fetch
////////////////////////////////////////////////////////////////////////////////////////

VOID PPU::FetchBgAttribute()
{
	if (enabled)
	{
		const UINT xTile = (vRamAddress & VRAM_X_TILE) >> 0;
		const UINT yTile = (vRamAddress & VRAM_Y_TILE) >> 5;

		const UINT address = 0x23C0 + (vRamAddress & 0x0C00) + ((yTile & 0x1C) << 1) + (xTile >> 2);
		const UINT shifter = (xTile & 0x2) + ((yTile & 0x2) << 1);

		BgBuffer.attribute = ((vRam[address] >> shifter) & 0x3) << 2;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// pattern table byte fetch #1 with cycle adjustment
////////////////////////////////////////////////////////////////////////////////////////

VOID PPU::PreFetchBgPattern0()
{
	cycles.count += cycles.fetch;
	FetchBgPattern0();
}

////////////////////////////////////////////////////////////////////////////////////////
// pattern table byte fetch #1
////////////////////////////////////////////////////////////////////////////////////////

VOID PPU::FetchBgPattern0()
{
	if (enabled)
	{
		BgBuffer.address = 0x1FFF &
		(
	    	BgPatternAddress + 
			(BgBuffer.name << 4) + 
			((vRamAddress & VRAM_Y_FINE) >> 12)
		);
		
		BgBuffer.pattern[0] = vRam[BgBuffer.address];
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// pattern table byte fetch #2 with cycle adjustment
////////////////////////////////////////////////////////////////////////////////////////

VOID PPU::PreFetchBgPattern1()
{
	cycles.count += cycles.fetch;
	FetchBgPattern1();
}

////////////////////////////////////////////////////////////////////////////////////////
// pattern table byte fetch #2
////////////////////////////////////////////////////////////////////////////////////////

VOID PPU::FetchBgPattern1()
{
	if (enabled)
		BgBuffer.pattern[1] = vRam[BgBuffer.address + 8];
}

////////////////////////////////////////////////////////////////////////////////////////
// render two dummy pixels
////////////////////////////////////////////////////////////////////////////////////////

VOID PPU::RenderDummyPixels()
{
	cycles.count += cycles.pixel + cycles.pixel;
	BgBuffer.index += 2;
}

////////////////////////////////////////////////////////////////////////////////////////
// render one pixel
////////////////////////////////////////////////////////////////////////////////////////

VOID PPU::RenderPixel()
{
	cycles.count += cycles.pixel;

	const UINT pixel = BgBuffer.pixels[(BgBuffer.index++ + xFine) & 0xF] & output.clipping;

	for (UINT i=0; i < SpBufferSize; ++i)
	{
		SP& sp = SpBuffer[i];

		if (--sp.x <= 0 && sp.x >= -7)
		{
			const UINT SpPixel = sp.pixels[-sp.x];

			if (SpPixel)
			{
				const UINT BgPixel = pixel & 0x3;

				if (sp.zero && BgPixel)
					status |= STATUS_SP_ZERO_HIT;

				output.pixels[output.index++] = emphasis + 
				(
			     	output.monochrome &
					(
				    	(sp.front || !BgPixel) ? sp.palette[SpPixel] : BgPalRam[pixel]
					)
				);
				
				for (++i; i < SpBufferSize; ++i)
					--SpBuffer[i].x;

				return;
			}
		}
	}

	output.pixels[output.index++] = emphasis +
	(
    	output.monochrome & BgPalRam[pixel]
	);
}

////////////////////////////////////////////////////////////////////////////////////////
// sprite evaluation for the next line
////////////////////////////////////////////////////////////////////////////////////////

VOID PPU::EvaluateSp()
{
	if (ctrl1 & CTRL1_SP_ENABLED)
	{
		const U8* const PDX_RESTRICT sprite = SpRam + SpIndex;

		INT y = ScanLine - sprite[SP_Y];

		if (y >= 0 && y < SpHeight && sprite[SP_Y] < 240)
		{
			if (SpTmpBufferSize == MaxSprites)
			{
				status |= STATUS_SP_OVERFLOW;
			}
			else
			{
				SPTMP& SpTmp = SpTmpBuffer[SpTmpBufferSize++];

				if (SpHeight == 8)
				{
					SpTmp.address = 
					(
						(sprite[SP_TILE] << 4) + 
						((sprite[SP_ATTRIBUTE] & SP_Y_FLIP) ? y^7 : y)
					);
				}
				else
				{
					if (y > 7)
						y += 8;

					SpTmp.address = 
					(
						((sprite[SP_TILE] & 0x01) << 0xC) + 
						((sprite[SP_TILE] & 0xFE) << 0x4) + 
						((sprite[SP_ATTRIBUTE] & SP_Y_FLIP) ? y^23 : y)
					);
				}

				SpTmp.x = sprite[SP_X] + 1;
				SpTmp.attribute = sprite[SP_ATTRIBUTE];
				SpTmp.index = SpIndex;
			}
		}
	}

	SpIndex += 4;
}

////////////////////////////////////////////////////////////////////////////////////////
// sprite tile pattern table byte fetch #1
////////////////////////////////////////////////////////////////////////////////////////

VOID PPU::FetchSpPattern0()
{
	cycles.count += cycles.fetch;

	if (enabled)
	{
		if (SpBufferSize < SpTmpBufferSize)
		{
			UINT index = SpBufferSize;

			do
			{
				SPTMP& SpTmp = SpTmpBuffer[index++];

				if (SpHeight == 8)
					SpTmp.address += SpPatternAddress;

				SpTmp.pattern = vRam[SpTmp.address];
			}
			while (MaxSprites > 8 && index >= 8 && index < SpTmpBufferSize);
		}
		else
		{
			vRam[SpPatternAddress];
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// sprite tile pattern table byte fetch #2
////////////////////////////////////////////////////////////////////////////////////////

VOID PPU::FetchSpPattern1()
{
	cycles.count += cycles.fetch;

	if (enabled)
	{
		if (SpBufferSize < SpTmpBufferSize)
		{
			do
			{
				const SPTMP& SpTmp = SpTmpBuffer[SpBufferSize];

				SP& sp = SpBuffer[SpBufferSize++];

				sp.x       = SpTmp.x;
				sp.palette = SpPalRam + ((SpTmp.attribute & SP_COLOR) << 2);
				sp.front   = !(SpTmp.attribute & SP_BEHIND);
				sp.zero    = !SpTmp.index;

				const UINT pattern1 = vRam[SpTmp.address + 8];

				const UINT pattern = 
				(
					((SpTmp.pattern & b01010101) << 0) |
					((SpTmp.pattern & b10101010) << 7) |
					((pattern1      & b01010101) << 1) |
					((pattern1      & b10101010) << 8) 
				);

				static const UCHAR shifter[2][8] =
				{
					{0xE,0x6,0xC,0x4,0xA,0x2,0x8,0x0},
					{0x0,0x8,0x2,0xA,0x4,0xC,0x6,0xE}
				};

				const UCHAR* const PDX_RESTRICT order = shifter[(SpTmp.attribute & SP_X_FLIP) ? 1 : 0];

				for (UINT i=0; i < 8; ++i)
					sp.pixels[i] = (pattern >> order[i]) & 0x3;

				if (sp.x < (8+1) && !(ctrl1 & CTRL1_SP_NO_CLIPPING))
					PDXMemZero( sp.pixels, (8+1) - sp.x );
			}
			while (MaxSprites > 8 && SpBufferSize >= 8 && SpBufferSize < SpTmpBufferSize);
		}
		else
		{
			vRam[SpPatternAddress];
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// start the pixel rendering
////////////////////////////////////////////////////////////////////////////////////////

VOID PPU::BeginHActive()
{
	PDX_ASSERT(SpIndex == 0);

	SpTmpBufferSize = 0;
	output.index = 0;

	++ScanLine;

	if (screen)
	{
		output.pixels = screen->pixels + (ScanLine * IO::GFX::WIDTH);
	}
	else
	{
		output.pixels = GarbageLine;

		if (status & STATUS_SP_ZERO_HIT)
		{
			// Now it's safe to skip one line
			cycles.count += (pal ? NES_PPU_MCC_HACTIVE_PAL : NES_PPU_MCC_HACTIVE_NTSC);
			process = processes + PROCESS_HBLANK;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// end the pixel rendering
////////////////////////////////////////////////////////////////////////////////////////

VOID PPU::EndHActive()
{
	status &= ~STATUS_SP_OVERFLOW;	
	
	SpBufferSize = 0;
	SpIndex = 0;

	if (!screen)
	{
		if (status & STATUS_SP_ZERO_HIT)
		{
			// Simulate sprite and background pattern fetches for
			// any IRQ generating hardware depending on them

			if (enabled == (CTRL1_BG_ENABLED|CTRL1_SP_ENABLED))
			{
				vRam[BgPatternAddress];
				vRam[SpPatternAddress];
			}

			cycles.count += (pal ? NES_PPU_MCC_HBLANK_PAL : NES_PPU_MCC_HBLANK_NTSC);
			process = processes + (ScanLine < 239 ? PROCESS_HACTIVE : PROCESS_END);
		}
	}

	if (enabled)
	{
		if (ScanLine == SCANLINE_VBLANK)
		{
			vRamAddress = vRamLatch;
		}
		else
		{
			vRamAddress &= ~(VRAM_X_TILE|VRAM_NAME_LOW);
			vRamAddress |= vRamLatch & (VRAM_X_TILE|VRAM_NAME_LOW);

			const UINT yFine = vRamAddress & VRAM_Y_FINE;

			if (yFine == (7U << 12))
			{
				const UINT yTile = vRamAddress & VRAM_Y_TILE;

				switch (yTile)
				{
	     			case (29U << 5):	

	    				vRamAddress ^= VRAM_NAME_HIGH;

	      			case (31U << 5): 

	     				vRamAddress &= ~(VRAM_Y_FINE|VRAM_Y_TILE);
	    				return;

	       			default:

	       				vRamAddress &= ~VRAM_Y_FINE;
	         			vRamAddress += (1U << 5);
		     			return;
				}
			}
			else
			{
				vRamAddress += (1U << 12);
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// begin a new frame
////////////////////////////////////////////////////////////////////////////////////////

VOID PPU::BeginFrame(IO::GFX* const gfx)
{ 
	screen = gfx;
	process = processes;

	cycles.Reset( pal, EvenFrame ^= 1 );

	cpu.SetFrameCycles( cycles.frame );
	cpu.DoNMI();
}

////////////////////////////////////////////////////////////////////////////////////////
// end the frame
////////////////////////////////////////////////////////////////////////////////////////

VOID PPU::EndFrame()
{ 
	const ULONG target = cycles.frame;

	while (cycles.count < target)
		(this->**process++)();
}

/////////////////////////////////////////////////////////////////////////////////////////
// end the horizontal blanking
////////////////////////////////////////////////////////////////////////////////////////

VOID PPU::EndHBlank()
{
	cycles.count += cycles.rest;

	if (ScanLine < 239)
		process = processes + PROCESS_HACTIVE;
}

////////////////////////////////////////////////////////////////////////////////////////
// end the horizontal dummy scanline blanking
////////////////////////////////////////////////////////////////////////////////////////

VOID PPU::EndHDummy()
{
	if (EvenFrame)
		cycles.count += cycles.rest;

	if (screen && PDX_FAILED(screen->Lock()))	
		screen = NULL;
}

////////////////////////////////////////////////////////////////////////////////////////
// end the frame pixel rendering process
////////////////////////////////////////////////////////////////////////////////////////

VOID PPU::EndVActive()
{
	cycles.count = cycles.frame;
	ScanLine = SCANLINE_VBLANK;

	if (screen)
	{
		screen->Unlock();
		screen = NULL;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// read from any memory location
////////////////////////////////////////////////////////////////////////////////////////

UINT PPU::Peek(UINT address)
{
	Update();
	
	address &= 0x3FFF;
	
	return (address < 0x2000) ? cRam[address] : CiRam[address & 0x1FFF];
}

////////////////////////////////////////////////////////////////////////////////////////
// write into any memory location
////////////////////////////////////////////////////////////////////////////////////////

VOID PPU::Poke(UINT address,const UINT data)
{
	Update();
	
	address &= 0x3FFF;
	
	if (address < 0x2000) 
		cRam[address] = data;
	else
		CiRam[address & 0x1FFF] = data;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID PPU::Log(const CHAR* const msg,const UINT which)
{
	if (!logged[which])
	{
		logged[which] = true;
		LogOutput( msg );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// load state file
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT PPU::LoadState(PDXFILE& file)
{
	{
		HEADER header;

		if (!file.Read(header))
			return PDX_FAILURE;

		ctrl0        = header.ctrl0;
		ctrl1        = header.ctrl1;
		status       = header.status;
		vRamAddress  = header.vRamAddress;
		vRamLatch    = header.vRamLatch;
		OamAddress   = header.OamAddress;
		DmaLatch     = header.DmaLatch;
		latch        = header.latch;
		ReadLatch    = header.ReadLatch;
		xFine        = header.xFine;
		FlipFlop     = header.FlipFlop;
		EvenFrame    = header.EvenFrame;
	}

	UpdateTmpVariables();

	if (!file.Readable(sizeof(U8) * (n8k+256+32)))
		return PDX_FAILURE;

	file.Read( cRam, cRam + n8k );
	file.Read( SpRam, SpRam + 256 );
	file.Read( PalRam, PalRam + 32 );

	return CiRam.LoadState(file);
}

////////////////////////////////////////////////////////////////////////////////////////
// save state file
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT PPU::SaveState(PDXFILE& file) const
{
	{
		HEADER header;

		header.ctrl0        = ctrl0;
		header.ctrl1        = ctrl1;
		header.status       = status;
		header.vRamAddress  = vRamAddress;
		header.vRamLatch    = vRamLatch;
		header.OamAddress   = OamAddress;
		header.DmaLatch     = DmaLatch;
		header.latch        = latch;
		header.ReadLatch    = ReadLatch;
		header.xFine        = xFine;
		header.FlipFlop     = FlipFlop ? 1 : 0;
		header.EvenFrame    = EvenFrame ? 1 : 0;

		file.Write(header);
	}

	file.Write( cRam, cRam + n8k );
	file.Write( SpRam, SpRam + 256 );
	file.Write( PalRam, PalRam + 32 );

	return CiRam.SaveState(file);
}

////////////////////////////////////////////////////////////////////////////////////////
// garbage fetches for the dummy scanline
////////////////////////////////////////////////////////////////////////////////////////

#define NES_DUMMYTILE \
			    	  \
 EvaluateSp,	      \
 FetchName,		      \
 FetchAttribute,      \
 EvaluateSp,	      \
 FetchPattern0,       \
 FetchPattern1

////////////////////////////////////////////////////////////////////////////////////////
// fetches for the first two tiles at the end of the scanline
////////////////////////////////////////////////////////////////////////////////////////

#define NES_PREFETCH  \
				      \
 PreFetchBgName,	  \
 PreFetchBgAttribute, \
 PreFetchBgPattern0,  \
 PreFetchBgPattern1,  \
 FetchBgName,     	  \
 RenderDummyPixels,	  \
 FetchBgAttribute,    \
 RenderDummyPixels,	  \
 FetchBgPattern0,     \
 RenderDummyPixels,   \
 FetchBgPattern1,     \
 RenderDummyPixels

////////////////////////////////////////////////////////////////////////////////////////
// dummy scanline
////////////////////////////////////////////////////////////////////////////////////////

#define NES_DUMMY_LINE												\
																	\
 NES_DUMMYTILE, NES_DUMMYTILE, NES_DUMMYTILE, NES_DUMMYTILE,		\
 NES_DUMMYTILE, NES_DUMMYTILE, NES_DUMMYTILE, NES_DUMMYTILE,		\
 NES_DUMMYTILE, NES_DUMMYTILE, NES_DUMMYTILE, NES_DUMMYTILE,		\
 NES_DUMMYTILE, NES_DUMMYTILE, NES_DUMMYTILE, NES_DUMMYTILE,		\
 NES_DUMMYTILE, NES_DUMMYTILE, NES_DUMMYTILE, NES_DUMMYTILE,		\
 NES_DUMMYTILE, NES_DUMMYTILE, NES_DUMMYTILE, NES_DUMMYTILE,		\
 NES_DUMMYTILE, NES_DUMMYTILE, NES_DUMMYTILE, NES_DUMMYTILE,		\
 NES_DUMMYTILE, NES_DUMMYTILE, NES_DUMMYTILE, NES_DUMMYTILE,		\
																	\
 EndHActive,														\
																	\
 FetchName, FetchName, FetchSpPattern0, FetchSpPattern1,			\
 FetchName, FetchName, FetchSpPattern0, FetchSpPattern1,			\
 FetchName, FetchName, FetchSpPattern0, FetchSpPattern1,			\
 FetchName, FetchName, FetchSpPattern0, FetchSpPattern1,			\
 FetchName, FetchName, FetchSpPattern0, FetchSpPattern1,			\
 FetchName, FetchName, FetchSpPattern0, FetchSpPattern1,			\
 FetchName, FetchName, FetchSpPattern0, FetchSpPattern1,			\
 FetchName, FetchName, FetchSpPattern0, FetchSpPattern1,			\
																	\
 NES_PREFETCH,														\
																	\
 FetchName, FetchName, EndHDummy
																					   
////////////////////////////////////////////////////////////////////////////////////////
// pixel rendering and fetches for tiles 2..32 
////////////////////////////////////////////////////////////////////////////////////////

#define NES_TILE	 \
					 \
 EvaluateSp,		 \
 FetchBgName,		 \
 RenderPixel,		 \
 RenderPixel,		 \
 FetchBgAttribute,   \
 RenderPixel,		 \
 RenderPixel,		 \
 EvaluateSp,		 \
 FetchBgPattern0,	 \
 RenderPixel,		 \
 RenderPixel,		 \
 FetchBgPattern1,	 \
 RenderPixel,		 \
 RenderPixel

////////////////////////////////////////////////////////////////////////////////////////
// active scanline
////////////////////////////////////////////////////////////////////////////////////////

#define NES_LINE												  \
																  \
 BeginHActive,													  \
																  \
 NES_TILE, NES_TILE, NES_TILE, NES_TILE,						  \
 NES_TILE, NES_TILE, NES_TILE, NES_TILE,						  \
 NES_TILE, NES_TILE, NES_TILE, NES_TILE,						  \
 NES_TILE, NES_TILE, NES_TILE, NES_TILE,						  \
 NES_TILE, NES_TILE, NES_TILE, NES_TILE,						  \
 NES_TILE, NES_TILE, NES_TILE, NES_TILE,						  \
 NES_TILE, NES_TILE, NES_TILE, NES_TILE,						  \
 NES_TILE, NES_TILE, NES_TILE, NES_TILE,						  \
																  \
 EndHActive,													  \
																  \
 FetchName, FetchName, FetchSpPattern0, FetchSpPattern1,		  \
 FetchName, FetchName, FetchSpPattern0, FetchSpPattern1,		  \
 FetchName, FetchName, FetchSpPattern0, FetchSpPattern1,		  \
 FetchName, FetchName, FetchSpPattern0, FetchSpPattern1,		  \
 FetchName, FetchName, FetchSpPattern0, FetchSpPattern1,		  \
 FetchName, FetchName, FetchSpPattern0, FetchSpPattern1,		  \
 FetchName, FetchName, FetchSpPattern0, FetchSpPattern1,		  \
 FetchName, FetchName, FetchSpPattern0, FetchSpPattern1,		  \
																  \
 NES_PREFETCH,													  \
 FetchName, FetchName, EndHBlank

////////////////////////////////////////////////////////////////////////////////////////
// all the routines gathered in one big fat LUT
////////////////////////////////////////////////////////////////////////////////////////

const PPU::PROCESS PPU::processes[] =
{
	BeginVBlank, 
	EndVBlank, 
	NES_DUMMY_LINE,
	NES_LINE,
	EndVActive
};

NES_NAMESPACE_END

