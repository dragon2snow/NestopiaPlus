////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003-2006 Martin Freij
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

#pragma comment(lib,"version")

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include "NstResourceVersion.hpp"

namespace Nestopia
{
	using Resource::Version;

	Version::Version(tstring const path)
	{
		NST_ASSERT( path );

		char buffer[] = "xx.xx";

		if (uint size = ::GetFileVersionInfoSize( path, 0 ))
		{
			void* const data = operator new (size);

			if (::GetFileVersionInfo( path, 0, size, data ))
			{
				tchar type[] = _T("\\");
				void* ptr;

				if (::VerQueryValue( data, type, &ptr, &size ) && size == sizeof(info))
				{
					info = *static_cast<const VS_FIXEDFILEINFO*>(ptr);

					char* string = buffer;

					if (HIWORD(info.dwFileVersionMS))
						*string++ = (char) ('0' + HIWORD(info.dwFileVersionMS));

					string[0] = (char) ('0' + LOWORD(info.dwFileVersionMS));
					string[1] = '.';
					string[2] = (char) ('0' + HIWORD(info.dwFileVersionLS));
					string[3] = (char) ('0' + LOWORD(info.dwFileVersionLS));
					string[4] = '\0';
				}
			}

			operator delete (data);
		}

		Heap::operator = (buffer);
	}
}
