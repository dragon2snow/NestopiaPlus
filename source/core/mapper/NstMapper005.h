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

#ifndef NST_MAPPER_5_H
#define NST_MAPPER_5_H

#include "../sound/NstSndMmc5.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
// MMC5
////////////////////////////////////////////////////////////////////////////////////////

class MAPPER5 : public MAPPER
{
public:

	MAPPER5(CONTEXT&);

	PDXRESULT LoadState(PDXFILE&);
	PDXRESULT SaveState(PDXFILE&) const;

private:

	enum PMODE
	{
		PMODE_32K,
		PMODE_16K,
		PMODE_16K_8K,
		PMODE_8K
	};

	enum CMODE
	{
		CMODE_8K,
		CMODE_4K,
		CMODE_2K,
		CMODE_1K
	};

	enum GMODE
	{
		GMODE_SPLIT,
		GMODE_EXGFX,
		GMODE_EXRAM,
		GMODE_EXRAM_WP
	};

	enum SMODE
	{
		SMODE_A,
		SMODE_B
	};

	enum
	{
		SWITCH_PROM = b10000000
	};

	VOID Reset();
	VOID hSync();
	
	BOOL DoSplit       (U8* const);
	VOID DoFillExGfx   (U8* const);
	VOID DoFillNormal  (U8* const);
	VOID DoExGfx       (U8* const);
	VOID DoNormal      (U8* const);
	VOID PpuLatch      (U8* const);

	template<UINT A,UINT B>
	VOID xRamSwapBanks();

	VOID UpdateCRom(CROM&,const UINT* const) const;

	NES_DECL_POKE( 5100 );
	NES_DECL_POKE( 5101 );
	NES_DECL_POKE( 5102 );
	NES_DECL_POKE( 5103 );
	NES_DECL_POKE( 5104 );
	NES_DECL_POKE( 5105 );
	NES_DECL_POKE( 5106 );
	NES_DECL_POKE( 5107 );
	NES_DECL_POKE( 5113 );
	NES_DECL_POKE( 5114 );
	NES_DECL_POKE( 5115 );
	NES_DECL_POKE( 5116 );
	NES_DECL_POKE( 5117 );
	NES_DECL_POKE( 5120 );
	NES_DECL_POKE( 5121 );
	NES_DECL_POKE( 5122 );
	NES_DECL_POKE( 5123 );
	NES_DECL_POKE( 5124 );
	NES_DECL_POKE( 5125 );
	NES_DECL_POKE( 5126 );
	NES_DECL_POKE( 5127 );
	NES_DECL_POKE( 5128 );
	NES_DECL_POKE( 5129 );
	NES_DECL_POKE( 512A );
	NES_DECL_POKE( 512B );
	NES_DECL_POKE( 5200 );
	NES_DECL_POKE( 5201 );
	NES_DECL_POKE( 5202 );
	NES_DECL_POKE( 5203 );
	NES_DECL_PEEK( 5204 );
	NES_DECL_POKE( 5204 );
	NES_DECL_PEEK( 5C00 );
	NES_DECL_POKE( 5C00 );
	NES_DECL_PEEK( 6000 );
	NES_DECL_POKE( 6000 );
	NES_DECL_PEEK( 8000 );
	NES_DECL_POKE( 8000 );
	NES_DECL_PEEK( A000 );
	NES_DECL_POKE( A000 );
	NES_DECL_PEEK( C000 );
	NES_DECL_POKE( C000 );
	NES_DECL_PEEK( E000 );

	template<UINT PAGE> VOID xRamSwapBanks(UINT);
	template<UINT PAGE> VOID pRomSwapBanks(UINT);

	CROM vBgRom;
	
	U8* vNameRam;
	UINT vNameMode[4];

	GMODE gMode;
	PMODE pMode;
	CMODE cMode;

	struct FILL
	{
		enum {MODE=3};

		UINT character;
		UINT attribute;
	};

	FILL fill;

	struct XRAM
	{
		typedef UINT (*BANKFIX)(const UINT);

		static UINT BankFix16 (const UINT bank) { return bank > 3 ? 8 : 0;    }
		static UINT BankFix32 (const UINT bank) { return bank > 3 ? 1 : 0;    }
		static UINT BankFix64 (const UINT bank) { return bank > 3 ? 8 : bank; }

		PDXWORD latch;
		UINT write;
		UINT enable;
		BANKFIX BankFix;
	};

	XRAM xRam;

	U8* pBanks[1+4];
	UINT cBanks[2][8];

	struct SPLIT
	{
		enum
		{
			ENABLE = b10000000,
			RIGHT  = b01000000,
			POS    = b00011111,
			Y      = b00000111,
			SCROLL = b11111000
		};

		UINT ctrl;
		UINT scroll;
		UINT offset;
		UINT address;
		UINT x;
		UINT y;
	};

	SPLIT split;

	struct IRQ
	{
		enum
		{
			ENABLE        = b10000000,
			STATUS_HIT    = b10000000,
			STATUS_ACTIVE = b01000000,
			SIGNAL        = STATUS_HIT|STATUS_ACTIVE
		};

		UINT line;
		UINT status;
		UINT clear;
		UINT scanline;
		UINT enable;
	};

	IRQ irq;

	SNDMMC5 sound;
};

NES_NAMESPACE_END

#endif
