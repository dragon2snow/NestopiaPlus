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

#include "../NstTypes.h"
#include "../NstMachine.h"
#include "../NstMap.h"
#include "../NstPpu.h"
#include "NstZapper.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

ZAPPER::ZAPPER(const UINT i,PPU* const p)
: 
CONTROLLER (i,p),
x		   (0),
y          (0),
status     (0)
{}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID ZAPPER::Poll()
{
	if (input)
	{
		x = input->zapper.x;
		y = input->zapper.y;
		status = input->zapper.fire ? (FIRE|READ) : READ;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

UINT ZAPPER::Peek_4016()
{
	return port == 0 ? Read() : 0x00;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

UINT ZAPPER::Peek_4017()
{
	return port == 1 ? Read() : 0x00;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline UINT ZAPPER::Read() const
{
	UINT state = status;

	if (gfx)
	{
		PDX_ASSERT(ppu);

		ppu->Update();

		const UINT pixel = gfx->GetPixel(x,y);

		if (pixel == 32 || pixel == 48)
			state &= 0xF7;
	}

	return state;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

ULONG ZAPPER::GetState() const
{
	return 
	(
     	(status   <<  0) | 
		(ULONG(x) <<  8) | 
		(ULONG(y) << 20)
	);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID ZAPPER::SetState(const ULONG data)
{
	status = (data & 0x000000FFUL) >> 0;
	x      = (data & 0x000FFF00UL) >> 8;
	y      = (data & 0xFFF00000UL) >> 20;
}

NES_NAMESPACE_END