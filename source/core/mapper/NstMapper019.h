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

#ifndef NST_MAPPER_19_H
#define NST_MAPPER_19_H

NES_NAMESPACE_BEGIN

class SNDN106;

////////////////////////////////////////////////////////////////////////////////////////
// mapper 19
////////////////////////////////////////////////////////////////////////////////////////

class MAPPER19 : public MAPPER
{
public:

	MAPPER19(CONTEXT&);
	~MAPPER19();

	PDXRESULT LoadState(PDXFILE&);
	PDXRESULT SaveState(PDXFILE&) const;

private:

	VOID Reset();   
	VOID IrqSync(const UINT);

	NES_DECL_PEEK( 480x );
	NES_DECL_POKE( 480x );
	NES_DECL_PEEK( 480y );
	NES_DECL_POKE( 480y );
	NES_DECL_PEEK( 5000 );
	NES_DECL_POKE( 5000 );
	NES_DECL_PEEK( 5800 );
	NES_DECL_POKE( 5800 );
	NES_DECL_POKE( 8000 );
	NES_DECL_POKE( 8800 );
	NES_DECL_POKE( 9000 );
	NES_DECL_POKE( 9800 );
	NES_DECL_POKE( A000 );
	NES_DECL_POKE( A800 );
	NES_DECL_POKE( B000 );
	NES_DECL_POKE( B800 );
	NES_DECL_POKE( C000 );
	NES_DECL_POKE( C800 );
	NES_DECL_POKE( D000 );
	NES_DECL_POKE( D800 );
	NES_DECL_POKE( E000 );
	NES_DECL_POKE( E800 );
	NES_DECL_POKE( F000 );
	NES_DECL_POKE( F80x );
	NES_DECL_POKE( F80y );

	NES_DECL_POKE( vRam_0000 );
	NES_DECL_PEEK( vRam_0000 );
	NES_DECL_POKE( vRam_0400 );
	NES_DECL_PEEK( vRam_0400 );
	NES_DECL_POKE( vRam_0800 );
	NES_DECL_PEEK( vRam_0800 );
	NES_DECL_POKE( vRam_0C00 );
	NES_DECL_PEEK( vRam_0C00 );
	NES_DECL_POKE( vRam_1000 );
	NES_DECL_PEEK( vRam_1000 );
	NES_DECL_POKE( vRam_1400 );
	NES_DECL_PEEK( vRam_1400 );
	NES_DECL_POKE( vRam_1800 );
	NES_DECL_PEEK( vRam_1800 );
	NES_DECL_POKE( vRam_1C00 );
	NES_DECL_PEEK( vRam_1C00 );
	NES_DECL_PEEK( vRam_2000 );  
	NES_DECL_POKE( vRam_2000 );  
	NES_DECL_PEEK( vRam_2400 );  
	NES_DECL_POKE( vRam_2400 );  
	NES_DECL_PEEK( vRam_2800 );  
	NES_DECL_POKE( vRam_2800 );  
	NES_DECL_PEEK( vRam_2C00 );  
	NES_DECL_POKE( vRam_2C00 );  

	struct VRAM
	{
		U8* data;
		BOOL protect;
	};

	VRAM     vRam[12];	
	UINT     regs[3];
	U8       cRam[n8k+n2k];
	SNDN106* sound;
};

NES_NAMESPACE_END

#endif
