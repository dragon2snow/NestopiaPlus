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

#ifndef NST_MAPPER_18_H
#define NST_MAPPER_18_H

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
// mapper 18
////////////////////////////////////////////////////////////////////////////////////////

class MAPPER18 : public MAPPER
{
public:

	MAPPER18(CONTEXT& c)
	: MAPPER(c,buffer,buffer+11) {}

private:

	VOID Reset();

	NES_DECL_POKE( 8000 );
	NES_DECL_POKE( 8001 );
	NES_DECL_POKE( 8002 );
	NES_DECL_POKE( 8003 );
	NES_DECL_POKE( 9000 );
	NES_DECL_POKE( 9001 );
	NES_DECL_POKE( A000 );
	NES_DECL_POKE( A001 );
	NES_DECL_POKE( A002 );
	NES_DECL_POKE( A003 );
	NES_DECL_POKE( B000 );
	NES_DECL_POKE( B001 );
	NES_DECL_POKE( B002 );
	NES_DECL_POKE( B003 );
	NES_DECL_POKE( C000 );
	NES_DECL_POKE( C001 );
	NES_DECL_POKE( C002 );
	NES_DECL_POKE( C003 );
	NES_DECL_POKE( D000 );
	NES_DECL_POKE( D001 );
	NES_DECL_POKE( D002 );
	NES_DECL_POKE( D003 );
	NES_DECL_POKE( E000 );
	NES_DECL_POKE( E001 );
	NES_DECL_POKE( E002 );
	NES_DECL_POKE( E003 );
	NES_DECL_POKE( F000 );
	NES_DECL_POKE( F001 );
	NES_DECL_POKE( F002 );

	VOID IrqSync(const UINT);

	UINT buffer[11];
};

NES_NAMESPACE_END

#endif
