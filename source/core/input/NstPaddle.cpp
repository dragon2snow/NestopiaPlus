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
#include "NstPaddle.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PADDLE::PADDLE(const UINT p)
: 
CONTROLLER (p),
fire       (0),
state      (0),
count      (0)
{}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID PADDLE::Poll()
{
	polled = TRUE;
	count = 0;
	state = 0;
	fire = 0;

	if (input)
	{
		const UINT x = PDX_CLAMP( input->paddle.x, 32, 176 );
		const UINT p = 0xFF - ((82 + 172 * (x-32) / 144) & 0xFF);

		state  = ( p & 0x01 ) << 7;
		state |= ( p & 0x02 ) << 5;
		state |= ( p & 0x04 ) << 3;
		state |= ( p & 0x08 ) << 1;
		state |= ( p & 0x10 ) >> 1;
		state |= ( p & 0x20 ) >> 3;
		state |= ( p & 0x40 ) >> 5;
		state |= ( p & 0x80 ) >> 7;

		if (input->paddle.fire)
			fire = FIRE;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

UINT PADDLE::Peek_4016()
{
	if (!polled)
		PADDLE::Poll();
		 
	return fire;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

UINT PADDLE::Peek_4017()
{
	if (!polled)
		PADDLE::Poll();

	const UINT data = ((state >> count) & 0x1) << 1;
	count = PDX_MIN(count+1,STREAM_LENGTH);

	return data;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

ULONG PADDLE::GetState() const
{
	return 
	(
       	(count       <<  0) | 
		(state       <<  8) | 
		(ULONG(fire) << 16)
	);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID PADDLE::SetState(const ULONG data)
{
	count = (data & 0x0000FFUL) >> 0;
	state = (data & 0x00FF00UL) >> 8;
	fire  = (data & 0xFF0000UL) >> 16;
}

NES_NAMESPACE_END
