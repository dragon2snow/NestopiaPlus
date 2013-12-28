////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003-2005 Martin Freij
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

#include "NstResourceGeneric.hpp"
#include "NstResourceString.hpp"
#include <Windows.h>

namespace Nestopia
{
	Resource::String::String(uint id)
	{
		HINSTANCE const hInstance = ::GetModuleHandle(NULL);

		for (;;)
		{
			const uint length = ::LoadString( hInstance, id, string, string.Capacity() + 1 );

			if (length != string.Capacity())
			{
				string.ShrinkTo( length );
				break;
			}

			string.Reserve( string.Capacity() + BLOCK_SIZE );
		}
	}
}