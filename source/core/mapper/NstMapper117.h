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

#ifndef NST_MAPPER_117_H
#define NST_MAPPER_117_H

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
// mapper 117
////////////////////////////////////////////////////////////////////////////////////////

class MAPPER117 : public MAPPER
{
public:

	MAPPER117(CONTEXT& c)
	: MAPPER(c,&IrqLine,&IrqLine+1) {}

private:

	VOID Reset();
	VOID IrqSync();

	NES_DECL_POKE( 8000	);
	NES_DECL_POKE( 8001	);
	NES_DECL_POKE( 8002	);
	NES_DECL_POKE( 8003	);
	NES_DECL_POKE( A000	);
	NES_DECL_POKE( A001	);
	NES_DECL_POKE( A002	);
	NES_DECL_POKE( A003	);
	NES_DECL_POKE( A004	);
	NES_DECL_POKE( A005	);
	NES_DECL_POKE( A006	);
	NES_DECL_POKE( A007	);
	NES_DECL_POKE( C001 );
	NES_DECL_POKE( C002	);
	NES_DECL_POKE( C003	);
	NES_DECL_POKE( E000	);

	enum
	{
		IRQ_LINE_1   = 0x1,
		IRQ_LINE_2   = 0x2,
		IRQ_LINE_1_2 = 0x3
	};

	UINT IrqLine;
};

NES_NAMESPACE_END

#endif
