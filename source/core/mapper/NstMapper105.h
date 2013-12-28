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

#ifndef NST_MAPPER_105_H
#define NST_MAPPER_105_H

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
// mapper 105
////////////////////////////////////////////////////////////////////////////////////////

class MAPPER105 : public MAPPER
{
public:

	MAPPER105(CONTEXT& c)
	: MAPPER(c,registers,&ready+1) {}

private:

	VOID Reset();
	VOID UpdateIRQ();
	VOID UpdateBanks();
	VOID UpdateMirroring();
	VOID IrqSync(const UINT);

	NES_DECL_POKE(pRom);

	enum
	{
		REG0_RESET  = b00001100,
		DATA_RESET  = b10000000,
		IRQ_DISABLE = b00010000
	};

	UINT registers[4];
	UINT latch;
	UINT count;
	UINT ready;
};

NES_NAMESPACE_END

#endif
