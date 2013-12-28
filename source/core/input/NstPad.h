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

#ifndef NST_PAD_H
#define NST_PAD_H

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_NAMESPACE_BEGIN

class PAD : public MACHINE::CONTROLLER
{
public:

	PAD(const UINT,const UINT);

	VOID Poll();
	VOID Reset();
	UINT Peek_4016();
	UINT Peek_4017();
	VOID Poke_4016(const UINT);

	ULONG GetState() const;
	VOID SetState(const ULONG);

	virtual CONTROLLERTYPE Type() const
	{ 
		PDX_COMPILE_ASSERT
		( 
	     	( CONTROLLER_PAD2 - CONTROLLER_PAD1 ) == 1 &&
			( CONTROLLER_PAD3 - CONTROLLER_PAD1 ) == 2 &&
			( CONTROLLER_PAD4 - CONTROLLER_PAD1 ) == 3
		);

		return CONTROLLERTYPE(UINT(CONTROLLER_PAD1) + VirtualPort); 
	}

private:

	enum
	{
		STREAM_LENGTH = 20
	};

	enum
	{
		A      = b00000001,
		B      = b00000010,
		SELECT = b00000100,
		START  = b00001000,
		UP     = b00010000,
		DOWN   = b00100000,
		LEFT   = b01000000,
		RIGHT  = b10000000
	};

	ULONG state;
	UINT  count;

	const UINT VirtualPort;
};

NES_NAMESPACE_END

#endif
