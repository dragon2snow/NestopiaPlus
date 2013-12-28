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

#ifndef NST_MAPPER_32_H
#define NST_MAPPER_32_H

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
// IREM G101
////////////////////////////////////////////////////////////////////////////////////////

class MAPPER32 : public MAPPER
{
public:

	MAPPER32(CONTEXT& c)
	: MAPPER(c,&pRomOffset,&pRomOffset+1) {}

private:

	VOID Reset();

	NES_DECL_POKE( 8000 );
	NES_DECL_POKE( 9000 );
	NES_DECL_POKE( A000 );
	NES_DECL_POKE( B000 );
	NES_DECL_POKE( B001 );
	NES_DECL_POKE( B002 );
	NES_DECL_POKE( B003 );
	NES_DECL_POKE( B004 );
	NES_DECL_POKE( B005 );
	NES_DECL_POKE( B006 );
	NES_DECL_POKE( B007 );

	UINT pRomOffset;
};

NES_NAMESPACE_END

#endif
