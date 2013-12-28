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

#ifndef NST_MAPPER_17_H
#define NST_MAPPER_17_H

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
// mapper 17
////////////////////////////////////////////////////////////////////////////////////////

class MAPPER17 : public MAPPER
{
public:

	MAPPER17(CONTEXT& c)
	: MAPPER(c) {}

private:

	VOID Reset();

	NES_DECL_POKE( 42FE );
	NES_DECL_POKE( 42FF );
	NES_DECL_POKE( 4501 );
	NES_DECL_POKE( 4502 );
	NES_DECL_POKE( 4503 );
	NES_DECL_POKE( 4504 );
	NES_DECL_POKE( 4505 );
	NES_DECL_POKE( 4506 );
	NES_DECL_POKE( 4507 );
	NES_DECL_POKE( 4510 );
	NES_DECL_POKE( 4511 );
	NES_DECL_POKE( 4512 );
	NES_DECL_POKE( 4513 );
	NES_DECL_POKE( 4514 );
	NES_DECL_POKE( 4515 );
	NES_DECL_POKE( 4516 );
	NES_DECL_POKE( 4517 );

	VOID IrqSync(const UINT);

	ULONG IrqNum;
};

NES_NAMESPACE_END

#endif

