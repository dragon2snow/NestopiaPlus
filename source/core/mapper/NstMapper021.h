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

#ifndef NST_MAPPER_21_H
#define NST_MAPPER_21_H

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
// VRC4
////////////////////////////////////////////////////////////////////////////////////////

class MAPPER21 : public MAPPER
{
public:

	MAPPER21(CONTEXT& context,VOID* const begin=NULL,const VOID* const end=NULL)
	: MAPPER(context,&pRomSelect,cRomSelect+8,begin,end) {}

protected:

	VOID Reset();

	NES_DECL_POKE( 8000 );
	NES_DECL_POKE( 9000 );
	NES_DECL_POKE( 9002 );
	NES_DECL_POKE( A000 );
	NES_DECL_POKE( B000 );
	NES_DECL_POKE( B002 );
	NES_DECL_POKE( B004 );
	NES_DECL_POKE( B006 );
	NES_DECL_POKE( C000 );
	NES_DECL_POKE( C002 );
	NES_DECL_POKE( C004 );
	NES_DECL_POKE( C006 );
	NES_DECL_POKE( D000 );
	NES_DECL_POKE( D002 );
	NES_DECL_POKE( D004 );
	NES_DECL_POKE( D006 );
	NES_DECL_POKE( E000 );
	NES_DECL_POKE( E002 );
	NES_DECL_POKE( E004 );
	NES_DECL_POKE( E006 );
	NES_DECL_POKE( F000 );
	NES_DECL_POKE( F002 );
	NES_DECL_POKE( F003 );
	NES_DECL_POKE( F004 );

private:

	VOID IrqSync(const UINT);

	union CROMSELECT
	{
		struct  
		{
			U8 b;
		};

		struct
		{
			U8 l : 4;
			U8 h : 4;
		};
	};

	UINT pRomSelect;

	CROMSELECT cRomSelect[8];
};

NES_NAMESPACE_END

#endif

