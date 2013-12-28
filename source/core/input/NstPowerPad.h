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

#ifndef NST_POWERPAD_H
#define NST_POWERPAD_H

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_NAMESPACE_BEGIN

class POWERPAD : public MACHINE::CONTROLLER
{
public:

	POWERPAD(const UINT);

	VOID Reset();
	VOID Poll();
	VOID Poke_4016(const UINT);
	UINT Peek_4017();
	UINT Peek_4016();

	ULONG GetState() const;
	VOID SetState(const ULONG);

	CONTROLLERTYPE Type() const
	{ return CONTROLLER_POWERPAD; }

private:

	UINT Read();

	enum
	{
		STREAM_LENGTH = 8
	};

	UINT state[2];
	UINT count;
};

NES_NAMESPACE_END

#endif
