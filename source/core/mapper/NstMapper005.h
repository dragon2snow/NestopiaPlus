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

	enum
	{
		ENABLE_IRQ       = b10000000,
		IRQSTATUS_HIT    = b10000000,
		IRQSTATUS_VBLANK = b01000000,
		SWITCH_PROM      = b10000000
	};

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
		GMODE_SPLIT_EXGFX,
		GMODE_EXRAM,
		GMODE_EXRAM_WRITEPROTECT
	};

	enum SMODE
	{
		SMODE_A,
		SMODE_B
	};

	enum
	{
		NO_WRAM = 8
	};

	VOID Reset();

	VOID BankSwitchPRom(const UINT,const UINT);
	VOID BankSwitchWRam(const UINT,UINT);
	VOID RefreshCRomBanks(const SMODE);

	VOID IrqSync();

	NES_DECL_PEEK( 5204 );
	NES_DECL_PEEK( 5C00 );

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
	NES_DECL_POKE( 5204 );
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

	NES_DECL_PEEK( vRam_Name );
	NES_DECL_POKE( vRam_Name );
	NES_DECL_PEEK( vRam_Attr );

	CHIP<n4k,4> vNameRam;

	U8* const Name1;
	U8* const Name2;
	U8* const ExRam;
	U8* const FillRam;

	GMODE gMode;
	PMODE pMode;
	CMODE cMode;

	struct PBANK
	{
		U8* data;
		UINT wRamBank;
	};

	PBANK pBanks[8];

	UINT LastCRomSwap;

	PDXWORD wRamLatch;
	BOOL wRamWriteEnable;
	UINT value[2];

	UINT cBanks[2][8];

	UINT SplitCtrl;
	UINT SplitBank;

	UINT IrqLine;
	UINT IrqStatus;

	SNDMMC5 sound;
};

NES_NAMESPACE_END

#endif
