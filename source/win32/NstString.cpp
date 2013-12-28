////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
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

#include <cstdio>
#include "NstString.hpp"
#include "NstApplicationInstance.hpp"
#include <Shlwapi.h>

namespace Nestopia
{
	namespace String
	{
		uint Base::FromDouble(char (&b)[MAX_FLT_LENGTH],double v,uint n)
		{
			char f[] = "%.xf";
			f[2] = '0' + n;

			int l = std::sprintf( b, f, v );

			if (l > 0)
			{
				return l;
			}
			else
			{
				b[0] = '\0';
				return 0;
			}
		}

		uint Base::FromDouble(wchar_t (&b)[MAX_FLT_LENGTH],double v,uint n)
		{
			wchar_t f[] = L"%.xf";
			f[2] = '0' + n;

			int l = std::swprintf( b, f, v );

			if (l > 0)
			{
				return l;
			}
			else
			{
				b[0] = '\0';
				return 0;
			}
		}

		// LOCALE_INVARIANT would be better but is only available in XP or greater

		int Base::Compare(const char* t,int n,const char* u,int m)
		{
			return ::CompareStringA( MAKELCID(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),SORT_DEFAULT), NORM_IGNORECASE, t, n, u, m ) - 2;
		}

		int Base::Compare(const wchar_t* t,int n,const wchar_t* u,int m)
		{
			return ::CompareStringW( MAKELCID(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),SORT_DEFAULT), NORM_IGNORECASE, t, n, u, m ) - 2;
		}

		int Base::CompareCase(const char* t,int n,const char* u,int m)
		{
			return ::CompareStringA( MAKELCID(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),SORT_DEFAULT), 0, t, n, u, m ) - 2;
		}

		int Base::CompareCase(const wchar_t* t,int n,const wchar_t* u,int m)
		{
			return ::CompareStringW( MAKELCID(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),SORT_DEFAULT), 0, t, n, u, m ) - 2;
		}

		template<>
		ibool Path<char>::Compact(char* to,const char* from,uint maxLength)
		{
			return ::PathCompactPathExA( to, from, maxLength + 1, 0 );
		}

		template<>
		ibool Path<wchar_t>::Compact(wchar_t* to,const wchar_t* from,uint maxLength)
		{
			return ::PathCompactPathExW( to, from, maxLength + 1, 0 );
		}

		template<>
		void Heap<char>::Import(cstring from)
		{
			*this = from;
		}

		template<>
		void Heap<wchar_t>::Import(cstring src)
		{
			const int n = ::MultiByteToWideChar( CP_ACP, 0, src, -1, NULL, 0 );

			if (n > 1)
			{
				const uint l = length;
				Resize( l + n-1 );
				::MultiByteToWideChar( CP_ACP, 0, src, -1, string + l, n );
			}
		}

		bool Base::Trim(char* string)
		{
			return ::StrTrimA( string, " " );
		}

		bool Base::Trim(wchar_t* string)
		{
			return ::StrTrimW( string, L" " );
		}

		template<>
		bool Path<char>::FileExists() const
		{
			if (Parent::Length())
			{
				const DWORD result = ::GetFileAttributesA( Parent::Ptr() );
				return (result != INVALID_FILE_ATTRIBUTES) && !(result & FILE_ATTRIBUTE_DIRECTORY);
			}

			return false;
		}

		template<>
		bool Path<wchar_t>::FileExists() const
		{
			if (Parent::Length())
			{
				const DWORD result = ::GetFileAttributesW( Parent::Ptr() );
				return (result != INVALID_FILE_ATTRIBUTES) && !(result & FILE_ATTRIBUTE_DIRECTORY);
			}

			return false;
		}

		template<>
		bool Path<char>::FileProtected() const
		{
			if (Parent::Length())
			{
				const DWORD result = ::GetFileAttributesA( Parent::Ptr() );
				return (result != INVALID_FILE_ATTRIBUTES) && (result & FILE_ATTRIBUTE_READONLY);
			}

			return false;
		}

		template<>
		bool Path<wchar_t>::FileProtected() const
		{
			if (Parent::Length())
			{
				const DWORD result = ::GetFileAttributesW( Parent::Ptr() );
				return (result != INVALID_FILE_ATTRIBUTES) && (result & FILE_ATTRIBUTE_READONLY);
			}

			return false;
		}

		template<>
		bool Path<char>::DirectoryExists() const
		{
			if (Parent::Length())
			{
				const DWORD result = ::GetFileAttributesA( Parent::Ptr() );
				return (result != INVALID_FILE_ATTRIBUTES) && (result & FILE_ATTRIBUTE_DIRECTORY);
			}

			return false;
		}

		template<>
		bool Path<wchar_t>::DirectoryExists() const
		{
			if (Parent::Length())
			{
				const DWORD result = ::GetFileAttributesW( Parent::Ptr() );
				return (result != INVALID_FILE_ATTRIBUTES) && (result & FILE_ATTRIBUTE_DIRECTORY);
			}

			return false;
		}
	}
}
