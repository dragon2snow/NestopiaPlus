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

	enum HOOK
	{
		HOOK_RENDERER,
		HOOK_HSYNC
	};

	template<class OBJECT,class MEMBER>
	VOID AddHook(OBJECT,MEMBER,HOOK);

	ULONG GetCycles() const;

	PORT& GetPort(const UINT);
	const PORT& GetPort(const UINT) const;

	UINT Peek(UINT);
	VOID Poke(UINT,const UINT);

	INT  GetScanline()         const;
	UINT GetVRamAddress()      const;
	BOOL IsBgEnabled()         const;
	BOOL IsSpEnabled()         const;
	BOOL IsEnabled()           const;
	UINT GetBgPatternAddress() const;

	U8* GetCiRam(const UINT=0x00);

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
		MAX_SPRITES = 32,
		MARGIN = 8
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
		CTRL1_BG_SP_ENABLED  = CTRL1_BG_ENABLED|CTRL1_SP_ENABLED,
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

	enum
	{
		PHASE_HDUMMY         = 1,
		PHASE_HDUMMY_COUNT   = 32,
		PHASE_HACTIVE        = 8,
		PHASE_HACTIVE_COUNT  = 32,
		PHASE_HACTIVE_END    = 22,
		PHASE_HBLANK_0       = 23,
		PHASE_HBLANK_0_COUNT = 8,
		PHASE_HBLANK_END     = 38,
		PHASE_VBLANK         = 39
	};

	VOID UpdateTmpVariables();
	VOID UpdateLazy();
	VOID WritePalRam(const UINT);

	PDX_NO_INLINE VOID DoDma(UINT);
	PDX_NO_INLINE VOID SkipLine();

	VOID NextTile();
	VOID NextLine();

	VOID vBlankBegin();
	VOID vBlankEnd();	
	VOID FetchGarbageName();
	VOID FetchGarbageAttribute();
	VOID FetchGarbagePattern0();
	VOID FetchGarbagePattern1();
	VOID hDummyEnd();
	VOID RenderPixel();
	VOID FetchBgName();
	VOID FetchBgAttribute();
	VOID FetchBgPattern0();
	VOID FetchBgPattern1();
	VOID hActiveEnd();
	VOID EvaluateSp();
	VOID FetchSpPattern0();
	VOID FetchSpPattern1();
	VOID PrefetchBg0Name();
	VOID PrefetchBg1Name();
	VOID PrefetchBgAttribute();
	VOID PrefetchBgPattern0(); 
	VOID PrefetchBgPattern1();
	VOID hBlankEnd();
	VOID FrameEnd();

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

	struct PPUCYCLES
	{
		PPUCYCLES();

		enum {WARM_UP = 2};

		UINT WarmUp;
		ULONG vint;
		ULONG frame;
		ULONG fetch;
		ULONG count;

		union
		{
			ULONG pixel;
			ULONG rest;
		};

		ULONG delay;
	};

	IO::GFX* screen;
	CPU& cpu;

	BOOL pal;
	UINT EvenFrame;
	UINT OamAddress;
	UINT OamLatch;
	UINT ctrl0;
	UINT latch;
	UINT vRamLatch;
	BOOL FlipFlop;
	UINT ReadLatch;
	UINT AddressIncrease;

	struct HOOKS
	{
		VOID Reset()
		{
			renderer.Reset();
			hSync.Reset();
		}

		struct OBJECT {};

		class RENDERER
		{
		public:

			RENDERER()
			: object(NULL), member(NULL) {}

			typedef VOID (OBJECT::*MEMBER)(U8* const);

			inline VOID Execute(U8* const data)
			{ return (*object.*member)( data ); }

			template<class A,class B> VOID Set(A a,B b)
			{
				PDX_ASSERT( a && b );
				object = PDX_CAST_REF( OBJECT*, a );
				member = PDX_CAST_REF( MEMBER,  b ); 
			}

			inline operator BOOL() const
			{ return object != NULL; }

			VOID Reset()
			{
				object = NULL;
				member = NULL;
			}

		private:

			OBJECT* object;
			MEMBER member;
		};

		class HSYNC
		{
		public:

			HSYNC()
			: object(NULL), member(NULL) {}

			typedef VOID (OBJECT::*MEMBER)();

			inline VOID Execute()
			{ return (*object.*member)(); }

			template<class A,class B> VOID Set(A a,B b)
			{
				PDX_ASSERT( a && b );
				object = PDX_CAST_REF( OBJECT*, a );
				member = PDX_CAST_REF( MEMBER,  b ); 
			}

			inline operator BOOL() const
			{ return object != NULL; }

			VOID Reset()
			{
				object = NULL;
				member = NULL;
			}

		private:

			OBJECT* object;
			MEMBER member;
		};

		HSYNC hSync;
		RENDERER renderer;
	};

	HOOKS hooks;

	UINT ctrl1;

	PPU_MAP vRam;

	PPUCYCLES cycles;

	const PROCESS* PDX_RESTRICT process;
	const PROCESS* phase;

	UINT PhaseCount;
	UINT vRamAddress;
	UINT SpIndex;
	INT  scanline;
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
	BOOL CanSkipLine;

	struct OUTPUT
	{
		UINT clipping;
		UINT monochrome;
		IO::GFX::PIXEL* PDX_RESTRICT pixels;
		const IO::GFX::PIXEL* ClipOffset;
	};

	OUTPUT output;

	struct BG
	{
		UINT index;
		UINT offset;
		UINT name;

		union
		{
			struct  
			{
				U8 attribute;
				U8 pattern[2];
			};

			struct  
			{
				U32 cache;
			};
		};

		union
		{
			U8 pixels[16];
			U32 u32Pixels[4];
		};
	};

	struct SP
	{
		U8* palette;
		SHORT x;
		UCHAR behind;
		UCHAR zero;
		U8 pixels[8];
	};

	struct BGCACHE
	{
		U32 cache;
		U32* pixels;
	};

	BGCACHE BgCache;
	
	BG BgBuffer;

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

	SP SpBuffer[MAX_SPRITES];

	struct SPTMP
	{
		USHORT address;
		USHORT x;
		UCHAR pattern;
		UCHAR attribute;
		UCHAR index;
	};

	SPTMP SpTmpBuffer[MAX_SPRITES];
	
	U8 SpRam[256];
	U8 cRam[n8k];

	CHIP<n4k,4> CiRam;

	VOID Log(const CHAR* const,const UINT);

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
		U8  reserved;
		U8  latch;
		U8  ReadLatch;
		U8  xFine : 3;
		U8  FlipFlop : 1;
		U8  EvenFrame : 1;
	};

   #pragma pack(pop)

	static const PROCESS processes[];

	IO::GFX::PIXEL* GarbageLine;
};

#include "NstPpu.inl"

NES_NAMESPACE_END

#endif
