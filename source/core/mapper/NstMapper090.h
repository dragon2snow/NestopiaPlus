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

#ifndef NST_MAPPER_90_H
#define NST_MAPPER_90_H

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

class MAPPER90 : public MAPPER
{
public:

	MAPPER90(CONTEXT&);
	~MAPPER90();

	PDXRESULT LoadState(PDXFILE&);
	PDXRESULT SaveState(PDXFILE&) const;

private:

	VOID Reset();

	NES_DECL_PEEK( 5000 );
	NES_DECL_PEEK( 5001 );
	NES_DECL_PEEK( 5800 );
	NES_DECL_PEEK( 5801 );
	NES_DECL_PEEK( 5803 );

	NES_DECL_POKE( 5800 );
	NES_DECL_POKE( 5801 );
	NES_DECL_POKE( 5803 );
	NES_DECL_POKE( 8000 );
	NES_DECL_POKE( 8001 );
	NES_DECL_POKE( 8002 );
	NES_DECL_POKE( 8003 );
	NES_DECL_POKE( 9000 );
	NES_DECL_POKE( 9001 );
	NES_DECL_POKE( 9002 );
	NES_DECL_POKE( 9003 );
	NES_DECL_POKE( 9004 );
	NES_DECL_POKE( 9005 );
	NES_DECL_POKE( 9006 );
	NES_DECL_POKE( 9007 );
	NES_DECL_POKE( A000 );
	NES_DECL_POKE( A001 );
	NES_DECL_POKE( A002 );
	NES_DECL_POKE( A003 );
	NES_DECL_POKE( A004 );
	NES_DECL_POKE( A005 );
	NES_DECL_POKE( A006 );
	NES_DECL_POKE( A007 );
	NES_DECL_POKE( B000 );
	NES_DECL_POKE( B001 );
	NES_DECL_POKE( B002 );
	NES_DECL_POKE( B003 );
	NES_DECL_POKE( B004 );
	NES_DECL_POKE( B005 );
	NES_DECL_POKE( B006 );
	NES_DECL_POKE( B007 );
	NES_DECL_POKE( C002 );
	NES_DECL_POKE( C003 );
	NES_DECL_POKE( C005 );
	NES_DECL_POKE( D000 );
	NES_DECL_POKE( D001 );

	NES_DECL_PEEK( CRom  );
	NES_DECL_PEEK( CiRam );
	NES_DECL_PEEK( CiRom );
	NES_DECL_PEEK( ExRom );
	NES_DECL_POKE( CiRam );

	VOID IrqSync(const UINT);

	VOID UpdatePRom();
	VOID UpdateCRom();
	VOID UpdateCiRom();

	enum
	{
		PROM_BANK_MODE   = b00000011,
		PROM_BANK_MODE_0 = b00000000,
		PROM_BANK_MODE_1 = b00000001,
		PROM_BANK_MODE_2 = b00000010,
		PROM_BANK_MODE_3 = b00000011,
		PROM_BANK_LAST   = b00000100,
		CROM_BANK_MODE   = b00011000,
		CROM_BANK_MODE_0 = b00000000,
		CROM_BANK_MODE_1 = b00001000,
		CROM_BANK_MODE_2 = b00010000,
		CROM_BANK_MODE_3 = b00011000,
		SELECT_CIROM     = b00100000,
		EXROM_BANK_SWAP  = b10000000,
		SELECT_MIRRORING = b00000011
	};

	typedef CHIP<n4k,4> CIROM;
	typedef CHIP<n4k,4> CIRAM;
	typedef CHIP<n8k,1> EXROM;

	UINT status;
	UINT mirror;
	UINT AddressLatch;
	UINT mul[2];
	UINT latch;

	UINT pRomBanks[4];
	PDXWORD cRomBanks[8];
	PDXWORD CiRomBanks[4];

	CIROM* CiRom;
	CIRAM  CiRam;
	EXROM* ExRom;
};

NES_NAMESPACE_END

#endif
