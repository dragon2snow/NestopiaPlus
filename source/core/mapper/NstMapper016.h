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

#ifndef NST_MAPPER_16_H
#define NST_MAPPER_16_H

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
// mapper 16
////////////////////////////////////////////////////////////////////////////////////////

class MAPPER16 : public MAPPER
{
public:

	MAPPER16(CONTEXT& c)
	: MAPPER(c) {}

protected:

	VOID Reset();

private:

	VOID IrqSync(const UINT);

	NES_DECL_PEEK( pRom );
	NES_DECL_POKE( x000 );
	NES_DECL_POKE( x001 );
	NES_DECL_POKE( x002 );
	NES_DECL_POKE( x003 );
	NES_DECL_POKE( x004 );
	NES_DECL_POKE( x005 );
	NES_DECL_POKE( x006 );
	NES_DECL_POKE( x007 );
	NES_DECL_POKE( x008 );
	NES_DECL_POKE( x009 );
	NES_DECL_POKE( x00A );
	NES_DECL_POKE( x00B );
	NES_DECL_POKE( x00C );
	NES_DECL_POKE( x00D );
};

NES_NAMESPACE_END

#endif
