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

#pragma comment(lib,"shlwapi")

#include "NstCollectionVector.hpp"
#include "NstSystemRegistry.hpp"
#include <Windows.h>
#include <Shlwapi.h>
#include <ShlObj.h>

namespace Nestopia
{
	using System::Registry;

	struct Registry::Key::AutoKey
	{
		HKEY handle;

		AutoKey()
		: handle(NULL) {}

		~AutoKey()
		{
			if (handle)
				::RegCloseKey( handle );
		}
	};

	void Registry::UpdateAssociations()
	{
		::SHChangeNotify( SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL );	
	}

	ibool Registry::Key::operator << (const String::Generic dataName) const
	{
		NST_ASSERT( dataName.IsNullTerminated() );

		long result;

		AutoKey keys[MAX_STACK+1];	
		keys[0].handle = HKEY_CLASSES_ROOT;

		for (uint i=0; i < count; ++i)
		{
			result = ::RegCreateKeyEx
			( 
				keys[i].handle, 
				stack[i], 
				0, 
				NULL, 
				REG_OPTION_NON_VOLATILE, 
				i+1 == count ? KEY_SET_VALUE : KEY_WRITE, 
				NULL, 
				&keys[i+1].handle, 
				NULL 
			);

			if (result != ERROR_SUCCESS)
				return FALSE;
		}

		result = ::RegSetValueEx
		( 
			keys[count].handle, 
			NULL, 
			0, 
			REG_SZ, 
			reinterpret_cast<const BYTE*>(static_cast<cstring>(dataName)),
			dataName.Size() + 1 
		);

		if (result != ERROR_SUCCESS)
			return FALSE;

		return TRUE;
	}

	ibool Registry::Key::operator >> (String::Heap& string) const
	{
		string.Clear();

		AutoKey keys[MAX_STACK+1];	
		keys[0].handle = HKEY_CLASSES_ROOT;

		for (uint i=0; i < count; ++i)
		{
			const long result = ::RegOpenKeyEx
			( 
				keys[i].handle, 
				stack[i], 
				0, 
				KEY_QUERY_VALUE, 
				&keys[i+1].handle
			);

			if (result != ERROR_SUCCESS)
				return FALSE;
		}

		DWORD type = 0, size = 0;

		if (::RegQueryValueEx( keys[count].handle, NULL, NULL, &type, NULL, &size ) != ERROR_SUCCESS || type != REG_SZ || size <= 1)
			return FALSE;

		string.Resize( size-1 );

		if (::RegQueryValueEx( keys[count].handle, NULL, NULL, NULL, reinterpret_cast<BYTE*>(&string.Front()), &size ) != ERROR_SUCCESS)
		{
			string.Clear();
			return FALSE;
		}

		return TRUE;
	}

	void Registry::Key::Delete() const
	{
		long result;

		AutoKey keys[MAX_STACK+1];	
		keys[0].handle = HKEY_CLASSES_ROOT;

		const uint last = count - 1;

		for (uint i=0; i < last; ++i)
		{
			result = ::RegOpenKeyEx
			( 
				keys[i].handle, 
				stack[i], 
				0, 
				KEY_WRITE, 
				&keys[i+1].handle
			);

			if (result != ERROR_SUCCESS)
				return;
		}

		::SHDeleteKey( keys[last].handle, stack[last] );
	}

	void Registry::Key::Delete(const String::Generic dataName) const
	{
		NST_ASSERT( dataName.IsNullTerminated() );

		long result;

		AutoKey keys[MAX_STACK+1];	
		keys[0].handle = HKEY_CLASSES_ROOT;

		for (uint i=0; i < count; ++i)
		{
			result = ::RegOpenKeyEx
			( 
				keys[i].handle, 
				stack[i], 
				0, 
				KEY_WRITE|KEY_QUERY_VALUE, 
				&keys[i+1].handle
			);

			if (result != ERROR_SUCCESS)
				return;
		}

		enum {PAD_TO_CHECK_REAL_LENGTH = 1};

		Collection::Vector<char> storedData( dataName.Size() + 1 + PAD_TO_CHECK_REAL_LENGTH );

		for (uint i=0; ; ++i)
		{
			DWORD storedType;
			char storedValue[260+1];
			DWORD storedValueSize = 260+1;
			DWORD storedDataSize = storedData.Size();

			result = ::RegEnumValue
			( 
				keys[count].handle, 
				i, 
				storedValue, 
				&storedValueSize, 
				NULL, 
				&storedType, 
				reinterpret_cast<BYTE*>( storedData.Begin() ), 
				&storedDataSize 
			);

			if (result != ERROR_SUCCESS)
				break;

			if 
			(
				storedType == REG_SZ && 
				storedDataSize == storedData.Size() - PAD_TO_CHECK_REAL_LENGTH &&
				std::memcmp( storedData, dataName, storedDataSize ) == 0
			)
			{
				if (storedValueSize)
				{
					// not default, delete value only
					::RegDeleteValue( keys[count].handle, storedValue );
				}
				else
				{
					// selected as default, delete whole key
					::SHDeleteKey( keys[count-1].handle, stack[count-1] );
				}
				break;
			}
		}
	}
}
