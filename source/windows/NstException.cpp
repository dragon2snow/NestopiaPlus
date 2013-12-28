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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include "NstException.h"
#include "../paradox/PdxString.h"

CHAR EXCEPTION::msg[EXCEPTION::MSG_LENGTH];

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

EXCEPTION::EXCEPTION(const UINT ResourceID)
{
	msg[0] = '\0';

	if (!::LoadString( ::GetModuleHandle(NULL), ResourceID, msg, MSG_LENGTH-1 ))
		msg[0] = '\0';
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

EXCEPTION::EXCEPTION(const CHAR* const string)
{
	msg[0] = '\0';

	if (string)
		strcpy( msg, string );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

EXCEPTION::EXCEPTION(const PDXSTRING& string)
{
	strcpy( msg, string.String() );
}
