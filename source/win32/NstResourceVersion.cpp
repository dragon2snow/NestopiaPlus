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

#pragma comment(lib,"Version")

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include "NstResourceVersion.hpp"

namespace Nestopia
{
	using Resource::Version;

	Version::Version(cstring path)
	: string("x.xx")
	{
		char buffer[_MAX_PATH+1];

		if (path == NULL)
		{
			path = buffer;

			if (!::GetModuleFileName( NULL, buffer, _MAX_PATH ))
				return;
		}

		if (InitInfo( path ))
			InitString( info.dwFileVersionMS, info.dwFileVersionLS );
	}

	ibool Version::InitInfo(cstring const path)
	{
		ibool result = FALSE;

		if (uint size = ::GetFileVersionInfoSize( path, 0 ))
		{
			void* const data = operator new (size);

			if (::GetFileVersionInfo( path, 0, size, data ))
			{
				char type[] = "\\";
				void* ptr;

				if (::VerQueryValue( data, type, &ptr, &size ) && size == sizeof(info))
				{
					info = *static_cast<const VS_FIXEDFILEINFO*>(ptr);
					result = TRUE;
				}
			}

			operator delete (data);
		}

		return result;
	}

	void Version::InitString(uint ms,uint ls)
	{
		string.Clear();

		if (HIWORD(ms))
			string = char('0' + HIWORD(ms));

		string << char('0' + LOWORD(ms)) << '.' << char('0' + HIWORD(ls)) << char('0' + LOWORD(ls));
	}
}
