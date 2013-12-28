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

#include "NstIoFile.hpp"
#include <Windows.h>

namespace Nestopia
{
	using Io::File;

	NST_COMPILE_ASSERT
	(
		File::BEGIN == FILE_BEGIN &&
		File::CURRENT == FILE_CURRENT &&
		File::END == FILE_END &&
		LONG_PTR(INVALID_HANDLE_VALUE) == -1
	);

	File::File()
	: handle(INVALID_HANDLE_VALUE) {}

	File::File(const String::Generic fileName,const uint mode)
	: handle(INVALID_HANDLE_VALUE)
	{
		Open( fileName, mode );
	}

	File::~File()
	{
		Close();
	}

	void File::Open(const String::Generic n,const uint mode)
	{
		NST_ASSERT( (mode & (WRITE|READ)) && ((mode & WRITE) || !(mode & EMPTY)) );

		Close();

		if (n.Empty())
			throw ERR_NOT_FOUND;

		name = n;

		handle = ::CreateFile
		( 
       		n.IsNullTerminated() ? static_cast<cstring>(name) : static_cast<cstring>(String::Path<true>(name)), 
			(
				((mode & READ)  ? GENERIC_READ  : 0) | 
				((mode & WRITE) ? GENERIC_WRITE : 0)
			),
			FILE_SHARE_READ,
			NULL,
			(
				(mode & (EMPTY|EXISTING)) == (EMPTY)          ? CREATE_ALWAYS :
				(mode & (EMPTY|EXISTING)) == (EMPTY|EXISTING) ? TRUNCATE_EXISTING :
				(mode & (EMPTY|EXISTING)) == (EXISTING)		  ? OPEN_EXISTING :
				(mode & (WRITE))							  ? OPEN_ALWAYS :
		                                                     	OPEN_EXISTING
			),
			FILE_ATTRIBUTE_NORMAL |
			(
				(mode & RANDOM_ACCESS) ? FILE_FLAG_RANDOM_ACCESS :
	    	 	(mode & SEQUENTIAL_ACCESS) ? FILE_FLAG_SEQUENTIAL_SCAN : 0
			),
			NULL
		);

		if (handle == INVALID_HANDLE_VALUE)
		{
			switch (::GetLastError())
			{
				case ERROR_FILE_NOT_FOUND: 
				case ERROR_PATH_NOT_FOUND: throw ERR_NOT_FOUND;
				case ERROR_ALREADY_EXISTS: throw ERR_ALREADY_EXISTS;
				default:                   throw ERR_OPEN;
			}
		}

		try
		{
			Size();
		}
		catch (Exception id)
		{
			Close();
			throw id;
		}
	}

	void File::Close()
	{
		name.Clear();

		if (IsOpen())
		{
			::CloseHandle( handle );
			handle = INVALID_HANDLE_VALUE;
		}
	}

	void File::Write(const void* const data,const uint size) const
	{
		NST_ASSERT( IsOpen() && bool(data) >= bool(size) );

		if (size)
		{
			DWORD written;

			if (!::WriteFile( handle, data, size, &written, NULL ) || written != size)
				throw ERR_WRITE;
		}
	}

	void File::Read(void* const data,const uint size) const
	{
		NST_ASSERT( IsOpen() && bool(data) >= bool(size) );

		if (size)
		{
			DWORD read;

			if (!::ReadFile( handle, data, size, &read, NULL ) || read != size)
				throw ERR_READ;
		}
	}

	uint File::ReadSome(void* const data,uint size) const
	{
		NST_ASSERT( IsOpen() && bool(data) >= bool(size) );

		if (size)
		{
			DWORD read;

			if (!::ReadFile( handle, data, size, &read, NULL ))
				throw ERR_READ;

			return read;
		}

		return 0;
	}

	void File::Peek(void* const data,const uint size) const
	{
		const uint pos = Position();
		Read( data, size );
		Position() = pos;
	}

	void File::Peek(const uint from,void* const data,const uint size) const
	{
		const uint pos = Position();
		Position() = from;
		Read( data, size );
		Position() = pos;
	}

	uint File::Seek(const Offset offset,const int distance) const
	{
		NST_ASSERT( IsOpen() );

		const DWORD pos = ::SetFilePointer( handle, distance, NULL, offset );

		if (pos == INVALID_SET_FILE_POINTER && ::GetLastError() != NO_ERROR)
			throw ERR_SEEK;

		return pos;
	}

	uint File::Size() const
	{
		NST_ASSERT( IsOpen() );

		DWORD words[2];

		words[1] = 0;
		words[0] = ::GetFileSize( handle, &words[1] );

		if (words[0] == INVALID_FILE_SIZE && ::GetLastError() != NO_ERROR)
			throw ERR_SEEK;

		if (words[1] || words[0] > MAX_SIZE)
			throw ERR_TOO_BIG;

		return words[0];
	}

	void File::Truncate() const
	{
		NST_ASSERT( IsOpen() );

		if (!::SetEndOfFile( handle ))
			throw ERR_SEEK;
	}

	void File::Truncate(const uint pos) const
	{
		Position() = pos;
		Truncate();
	}

	void File::Flush() const
	{
		NST_ASSERT( IsOpen() );
		::FlushFileBuffers( handle );
	}

	ibool File::FileExist(cstring const path)
	{
		NST_ASSERT( path );

		if (*path)
		{
			const DWORD result = ::GetFileAttributes( path );
			return (result != INVALID_FILE_ATTRIBUTES) && !(result & FILE_ATTRIBUTE_DIRECTORY);
		}

		return FALSE;
	}

	ibool File::FileProtected(cstring const path)
	{
		NST_ASSERT( path );

		if (*path)
		{
			const DWORD result = ::GetFileAttributes( path );
			return (result != INVALID_FILE_ATTRIBUTES) && (result & FILE_ATTRIBUTE_READONLY);
		}

		return FALSE;
	}

	ibool File::DirExist(cstring const path)
	{
		NST_ASSERT( path );
		return *path && ::GetFileAttributes( path ) != INVALID_FILE_ATTRIBUTES;
	}

	ibool File::Delete(cstring const file)
	{
		NST_ASSERT( file && *file );
		return ::DeleteFile( file );
	}
}
