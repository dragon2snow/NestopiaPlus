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

#pragma once

#ifndef NST_PPU_H
#define NST_PPU_H

#include "NstChip.h"

class PDXFILE;

NES_NAMESPACE_BEGIN

class CPU;

////////////////////////////////////////////////////////////////////////////////////////
// PPU class
////////////////////////////////////////////////////////////////////////////////////////

class PPU
{
public:

	typedef PPU_PORT PORT;

	PPU(CPU&);
	~PPU();

	PDX_NO_INLINE VOID Reset(const BOOL=FALSE);	

	VOID SetMirroring(const MIRRORING);
	VOID SetMirroring(const UINT,const UINT,const UINT,const UINT);
	
	VOID BeginFrame(IO::GFX* const);	
	VOID EndFrame();

	VOID Update();

	VOID SetMode(const MODE);

	PDXRESULT LoadState(PDXFILE&);
	PDXRESULT SaveState(PDXFILE&) const;

	VOID SetContext(const IO::GFX::CONTEXT&);
	VOID GetContext(IO::GFX::CONTEXT&) const;

	template<class OBJECT,class READER,class WRITER>
	VOID SetPort(const UINT,OBJECT*,READER,WRITER);

	template<class OBJECT,class READER,class WRITER>
	VOID SetPort(const UINT,const UINT,OBJECT*,READER,WRITER);

	ULONG GetCycles() const;

	PORT& GetPort(const UINT);
	const PORT& GetPort(const UINT) const;

	UINT Peek(UINT);
	VOID Poke(UINT,const UINT);

	INT GetScanLine() const;
	UINT GetVRamAddress() const;

	BOOL IsBgEnabled() const;
	BOOL IsSpEnabled() const;

	NES_DECL_PEEK( cRam   );
	NES_DECL_POKE( cRam   );
	NES_DECL_PEEK( PalRam );
	NES_DECL_POKE( PalRam );
	NES_DECL_PEEK( CiRam  );
	NES_DECL_POKE( CiRam  );

	enum
	{
		SCANLINE_VBLANK = -1
	};

private:

	enum
	{
		MARGIN = 8
	};

	enum
	{
		PROCESS_HACTIVE = 242,
		PROCESS_HBLANK  = 691,
		PROCESS_END     = 739
	};

	enum
	{
		CTRL0_NAME_OFFSET = b00000011,
		CTRL0_INC32       = b00000100,
		CTRL0_SP_OFFSET   = b00001000,
		CTRL0_BG_OFFSET   = b00010000,
		CTRL0_SP8X16      = b00100000,
		CTRL0_NMI         = b10000000
	};

	enum
	{
		CTRL1_MONOCHROME     = b00000001,
		CTRL1_BG_NO_CLIPPING = b00000010,
		CTRL1_SP_NO_CLIPPING = b00000100,
		CTRL1_BG_ENABLED     = b00001000,
		CTRL1_SP_ENABLED     = b00010000,
		CTRL1_BG_COLOR       = b11100000,
		CTRL1_BG_COLOR_R     = b00100000,
		CTRL1_BG_COLOR_G     = b01000000,
		CTRL1_BG_COLOR_B     = b10000000,
		CTRL1_BG_COLOR_SHIFT = 5
	};

	enum
	{
		STATUS_LATCH       = b00011111,
		STATUS_SP_OVERFLOW = b00100000,
		STATUS_SP_ZERO_HIT = b01000000,
		STATUS_VBLANK      = b10000000
	};

	enum							 
	{
		VRAM_X_TILE    = b16( 00000000, 00011111 ),
		VRAM_Y_TILE    = b16( 00000011, 11100000 ),
		VRAM_Y_FINE    = b16( 01110000, 00000000 ),
		VRAM_LOW       = b16( 00000000, 11111111 ),
		VRAM_HIGH      = b16( 11111111, 00000000 ),
		VRAM_NAME      = b16( 00001100, 00000000 ),
		VRAM_NAME_LOW  = b16( 00000100, 00000000 ),
		VRAM_NAME_HIGH = b16( 00001000, 00000000 )
	};

	enum
	{
		SP_Y,
		SP_TILE,
		SP_ATTRIBUTE,
		SP_X
	};

	enum
	{
		SP_COLOR  = b00000011,
		SP_BEHIND = b00100000,
		SP_X_FLIP = b01000000,
		SP_Y_FLIP = b10000000
	};

	VOID Log(const CHAR* const,const UINT);
	VOID UpdateTmpVariables();
	VOID WritePalRam(const UINT);

	VOID RenderPixel();
	VOID RenderDummyPixels();

	VOID FetchName();
	VOID FetchAttribute();
	VOID FetchPattern0();
	VOID FetchPattern1();

	VOID PreFetchBgName();
	VOID PreFetchBgAttribute();
	VOID PreFetchBgPattern0();
	VOID PreFetchBgPattern1();

	VOID FetchBgName();
	VOID FetchBgAttribute();
	VOID FetchBgPattern0();
	VOID FetchBgPattern1();

	VOID EvaluateSp();	
	VOID FetchSpPattern0();
	VOID FetchSpPattern1();
	
	VOID BeginHActive();
	VOID EndHActive();	
	VOID BeginVBlank();
	VOID EndVBlank();
	VOID EndHDummy();
	VOID EndHBlank();
	VOID EndVActive();
	
	VOID HSync();

	NES_DECL_POKE( 2000 );
	NES_DECL_PEEK( 2002 );
	NES_DECL_POKE( 2001 );
	NES_DECL_POKE( 2003 );
	NES_DECL_PEEK( 2004 );
	NES_DECL_POKE( 2004 );
	NES_DECL_POKE( 2005 );
	NES_DECL_POKE( 2006 );
	NES_DECL_PEEK( 2007 );
	NES_DECL_POKE( 2007 );
	NES_DECL_PEEK( 2xxx );
	NES_DECL_POKE( 2xxx );
	NES_DECL_PEEK( 4014 );
	NES_DECL_POKE( 4014 );

	NES_DECL_PEEK( vRam_00xx );  
	NES_DECL_POKE( vRam_00xx );
	NES_DECL_PEEK( vRam_20xx );  
	NES_DECL_POKE( vRam_20xx );
	NES_DECL_PEEK( vRam_24xx );  
	NES_DECL_POKE( vRam_24xx );
	NES_DECL_PEEK( vRam_28xx );  
	NES_DECL_POKE( vRam_28xx );
	NES_DECL_PEEK( vRam_2Cxx );  
	NES_DECL_POKE( vRam_2Cxx );

	typedef VOID (PPU::*PROCESS)();

	struct CYCLES
	{
		enum 
		{
			WARM_UP = 0,
			READY   = 2
		};

		VOID Reset   (const BOOL,const BOOL=TRUE);
		VOID SetMode (const BOOL,const BOOL=TRUE);

		ULONG count;
		ULONG fetch;

		union
		{
			ULONG pixel;
			ULONG rest;
		};

		ULONG frame;
		ULONG vint;
		UINT WarmUp;
	};

	IO::GFX* screen;
	CPU& cpu;

	BOOL pal;
	UINT EvenFrame;
	UINT OamAddress;
	UINT OamLatch;
	UINT DmaLatch;
	UINT ctrl0;
	UINT ctrl1;
	UINT latch;
	UINT vRamLatch;
	BOOL FlipFlop;
	UINT ReadLatch;
	UINT AddressIncrease;

	const PROCESS* process;

	PPU_MAP vRam;

	CYCLES cycles;

	BOOL enabled;
	UINT vRamAddress;
	UINT SpIndex;
	INT  ScanLine;
	UINT SpHeight;
	UINT MaxSprites;
	UINT SpTmpBufferSize;
	UINT status;
	UINT BgPatternAddress;
	UINT xFine;
	UINT SpBufferSize;
	UINT emphasis;
	UINT monochrome;
	UINT SpPatternAddress;	

	struct OUTPUT
	{
		UINT index;
		UINT clipping;
		UINT monochrome;
		IO::GFX::PIXEL* PDX_RESTRICT pixels;
	};

	OUTPUT output;

	struct BG
	{
		UINT address;
		UINT name;
		UINT attribute;
		UINT pattern[2];
		UINT index;
		U8 pixels[16];
	};

	struct SP
	{
		INT  x;
		U8*  palette;
		BOOL front;
		BOOL zero;
		U8 pixels[8];
	};

	BG BgBuffer;
	SP SpBuffer[64];

	union		
	{
		struct  
		{
			U8 PalRam[0x20];
		};

		struct  
		{
			U8 BgPalRam[0x10];
			U8 SpPalRam[0x10];
		};
	};

	struct SPTMP
	{
		UINT address;
		UINT pattern;
		UINT x;
		UINT attribute;
		UINT index;
	};

	SPTMP SpTmpBuffer[64];
	
	U8 SpRam[256];
	U8 cRam[n8k];

	CHIP<n4k,4> CiRam;

	bool logged[4];

   #pragma pack(push,1)
  
	struct HEADER
	{
		U8  ctrl0;
		U8  ctrl1;
		U8  status;
		U16 vRamAddress;
		U16 vRamLatch;
		U16 OamAddress;
		U8  DmaLatch;
		U8  latch;
		U8  ReadLatch;
		U8  xFine : 3;
		U8  FlipFlop : 1;
		U8  EvenFrame : 1;
	};

   #pragma pack(pop)

	static const PROCESS processes[];

	IO::GFX::PIXEL* const GarbageLine;
};

#include "NstPpu.inl"

NES_NAMESPACE_END

#endif
