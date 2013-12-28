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

#ifndef NST_MAPPER_4_H
#define NST_MAPPER_4_H

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
// MMC3
////////////////////////////////////////////////////////////////////////////////////////

class MAPPER4 : public MAPPER
{
public:

	MAPPER4(CONTEXT& c,VOID* const begin=NULL,const VOID* const end=NULL)
	: MAPPER(c,&command,cRomBanks+8,begin,end) {}

protected:

	VOID Reset();

	NES_DECL_POKE( 8000 );
	NES_DECL_POKE( 8001 );
	NES_DECL_POKE( A000 );
	NES_DECL_POKE( A001 );
	NES_DECL_POKE( C000 );
	NES_DECL_POKE( C001 );
	NES_DECL_POKE( E000 );
	NES_DECL_POKE( E001 );
	NES_DECL_PEEK( wRam );
	NES_DECL_POKE( wRam );
	NES_DECL_PEEK( cRam );
	NES_DECL_POKE( cRam );

	VOID IrqSync();

	virtual VOID UpdatePRom();
	virtual VOID UpdateCRom();

	enum
	{
		COMMAND_INDEX   = b00000111,
		SWAP_PROM_BANKS = b01000000,
		SWAP_CROM_BANKS = b10000000,
		WRAM_ENABLE     = b10000000
	};

	UINT command;
	UINT IrqReset;
	BOOL latched;
	UINT wRamEnable;
	UINT pRomBanks[4];
	UINT cRomBanks[8];
};

NES_NAMESPACE_END

#endif
