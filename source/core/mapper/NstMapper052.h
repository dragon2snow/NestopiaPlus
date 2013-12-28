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

#ifndef NST_MAPPER_52_H
#define NST_MAPPER_52_H

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
// mapper 52
////////////////////////////////////////////////////////////////////////////////////////

class MAPPER52 : public MAPPER4
{
public:

	MAPPER52(CONTEXT& c)
	: MAPPER4(c,regs,regs+2) {}

private:

	VOID Reset();   
	VOID UpdatePRom();
	VOID UpdateCRom();

	NES_DECL_PEEK( 6000 );
	NES_DECL_POKE( 6000 );

	UINT regs[2];
};

NES_NAMESPACE_END

#endif
