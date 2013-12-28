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

#ifndef NST_MAPPER_83_H
#define NST_MAPPER_83_H

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
// Mapper 83
////////////////////////////////////////////////////////////////////////////////////////

class MAPPER83 : public MAPPER
{
public:

	MAPPER83(CONTEXT& c)
	: MAPPER(c,regs,&cRomBank+1) {}

protected:

	VOID Reset();
	VOID IrqSync(const UINT);

	NES_DECL_PEEK( 5100   );
	NES_DECL_POKE( 5101   );
	NES_DECL_POKE( 8000   );
	NES_DECL_POKE( 8100   );
	NES_DECL_POKE( 8200   );
	NES_DECL_POKE( 8201   );
	NES_DECL_POKE( 8300   );
	NES_DECL_POKE( 8301   );
	NES_DECL_POKE( 8302   );
	NES_DECL_POKE( 8310_1 );
	NES_DECL_POKE( 8310_2 );
	NES_DECL_POKE( 8311_1 );
	NES_DECL_POKE( 8311_2 );
	NES_DECL_POKE( 8312   );
	NES_DECL_POKE( 8313   );
	NES_DECL_POKE( 8314   );
	NES_DECL_POKE( 8315   );
	NES_DECL_POKE( 8316_1 );
	NES_DECL_POKE( 8316_2 );
	NES_DECL_POKE( 8317_1 );
	NES_DECL_POKE( 8317_2 );
	NES_DECL_POKE( 8318   );

	UINT regs[3];
	UINT cRomBank;
};

NES_NAMESPACE_END

#endif
