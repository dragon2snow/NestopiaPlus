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
#include "NstPowerPad.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

POWERPAD::POWERPAD(const UINT p)
: CONTROLLER(p) { Reset(); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID POWERPAD::Reset()
{
	count = 0;
	strobe = 0;
	state[0] = 0;
	state[1] = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID POWERPAD::Poll()
{
	state[0] = 0;
	state[1] = 0;

	if (input)
	{
		static const U8 bits[2][IO::INPUT::POWERPAD::NUM_SIDE_A_BUTTONS] =
		{
			{0x02,0x01,0x00,0x00,0x04,0x10,0x80,0x00,0x08,0x20,0x40,0x00},
			{0x00,0x00,0x02,0x01,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x04}
		};

		for (UINT i=0; i < IO::INPUT::POWERPAD::NUM_SIDE_A_BUTTONS; ++i)
		{
			if (input->PowerPad.SideA[i])
			{
				state[0] |= bits[0][i];
				state[1] |= bits[1][i];
			}
		}
		
		static const U8 table[IO::INPUT::POWERPAD::NUM_SIDE_B_BUTTONS] = {2,1,7,6,5,4,10,9};

		for (UINT i=0; i < IO::INPUT::POWERPAD::NUM_SIDE_B_BUTTONS; ++i)
		{
			if (input->PowerPad.SideB[i])
			{
				state[0] |= bits[0][table[i]];
				state[1] |= bits[1][table[i]];
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

UINT POWERPAD::Peek_4017() 
{ 
	return port == 1 ? Read() : 0x00; 
}

UINT POWERPAD::Peek_4016() 
{ 
	return port == 0 ? Read() : 0x00; 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

UINT POWERPAD::Read()
{
	const UINT data = 
	(
     	((count < 8) ? ((state[0] >> count) & 0x1) << 3 : 0x80) |
		((count < 4) ? ((state[1] >> count) & 0x1) << 4 : 0x10)
	);

	count = PDX_MIN(count+1,STREAM_LENGTH);

	return data;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID POWERPAD::Poke_4016(const UINT data)
{
	if (SetStrobe(data))
		count = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

ULONG POWERPAD::GetState() const
{
	return 
	(
     	(count           <<  0) | 
		(ULONG(state[0]) <<  4) | 
		(ULONG(state[1]) << 16) |
		(strobe ? 0x80000000UL : 0)
	);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID POWERPAD::SetState(const ULONG data)
{
	count    = (data & 0x0000000FUL) >> 0;
	state[0] = (data & 0x0000FFF0UL) >> 4;
	state[1] = (data & 0x0FFF0000UL) >> 16;
	strobe   = (data & 0x80000000UL) >> 31;
}

NES_NAMESPACE_END
