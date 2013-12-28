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

#ifndef NST_ZAPPER_H
#define NST_ZAPPER_H

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_NAMESPACE_BEGIN

class ZAPPER : public MACHINE::CONTROLLER
{
public:

	ZAPPER(const UINT,PPU* const);

	VOID Poll();
	UINT Peek_4016();
	UINT Peek_4017();

	ULONG GetState() const;
	VOID SetState(const ULONG);

	CONTROLLERTYPE Type() const
	{ return CONTROLLER_ZAPPER; }

private:

	UINT Read() const;

	enum
	{
		READ = b00001000,
		FIRE = b00010000
	};

	UINT x;
	UINT y;
	UINT status;
};

NES_NAMESPACE_END

#endif
