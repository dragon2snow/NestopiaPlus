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

#include "NstIoFile.hpp"
#include <Windows.h>

namespace Nestopia
{
	using Io::File;

	NST_COMPILE_ASSERT
	(
		File::BEGIN == FILE_BEGIN &&
		File::CURRENT == FILE_CURRENT &&
		File::END == FILE_END
	);

	File::File()
	: handle(NULL) {}

	File::File(const GenericString fileName,const uint mode)
	: handle(NULL)
	{
		Open( fileName, mode );
	}

	File::~File()
	{
		Close();
	}

	void File::Open(const GenericString n,const uint mode)
	{
		NST_ASSERT( (mode & (WRITE|READ)) && ((mode & WRITE) || !(mode & EMPTY)) && (mode & (RANDOM_ACCESS|SEQUENTIAL_ACCESS)) != (RANDOM_ACCESS|SEQUENTIAL_ACCESS) );

		Close();

		if (n.Empty())
			throw ERR_NOT_FOUND;

		handle = ::CreateFile
		( 
       		(name=n).Ptr(), 
			(
			    (mode & (READ|WRITE)) == (READ|WRITE) ? (GENERIC_READ|GENERIC_WRITE) :
				(mode & (READ|WRITE)) == (READ)       ? (GENERIC_READ) :
				(mode & (READ|WRITE)) == (WRITE)      ? (GENERIC_WRITE) :
				                                        0
			),
			(
     			(mode & (READ|WRITE)) == (WRITE) ? 0 : FILE_SHARE_READ
			),
			NULL,
			(
				(mode & (EMPTY|EXISTING)) == (EMPTY)          ? CREATE_ALWAYS :
				(mode & (EMPTY|EXISTING)) == (EMPTY|EXISTING) ? TRUNCATE_EXISTING :
				(mode & (EMPTY|EXISTING)) == (EXISTING)		  ? OPEN_EXISTING :
				(mode & (WRITE))							  ? OPEN_ALWAYS :
		                                                     	OPEN_EXISTING
			),
			(
				((mode & WRITE) ? FILE_ATTRIBUTE_NORMAL : 0) |
				(
		     		(mode & (READ|RANDOM_ACCESS))     == (READ|RANDOM_ACCESS)     ? FILE_FLAG_RANDOM_ACCESS :
		     		(mode & (READ|SEQUENTIAL_ACCESS)) == (READ|SEQUENTIAL_ACCESS) ? FILE_FLAG_SEQUENTIAL_SCAN : 0
     			)
			),
			NULL
		);

		if (handle && handle != INVALID_HANDLE_VALUE)
		{
			if (!(mode & EMPTY))
			{
				try
				{
					Size();
				}
				catch (...)
				{
					Close();
					throw;
				}
			}
		}
		else 
		{
			handle = NULL;
			name.Clear();

			switch (::GetLastError())
			{
				case ERROR_FILE_NOT_FOUND: 
				case ERROR_PATH_NOT_FOUND: throw ERR_NOT_FOUND;
				case ERROR_ALREADY_EXISTS: throw ERR_ALREADY_EXISTS;
				default:                   throw ERR_OPEN;
			}
		}
	}

	void File::Close()
	{
		name.Clear();

		if (void* const tmp = handle)
		{
			handle = NULL;
			::CloseHandle( tmp );
		}
	}

	void File::Write(const void* const data,const uint size) const
	{
		NST_ASSERT( IsOpen() && bool(data) >= bool(size) );

		if (size)
		{
			DWORD written = 0;

			if (!::WriteFile( handle, data, size, &written, NULL ) || written != size)
				throw ERR_WRITE;
		}
	}

	void File::Read(void* const data,const uint size) const
	{
		NST_ASSERT( IsOpen() && bool(data) >= bool(size) );

		if (size)
		{
			DWORD read = 0;

			if (!::ReadFile( handle, data, size, &read, NULL ) || read != size)
				throw ERR_READ;
		}
	}

	uint File::ReadSome(void* const data,uint size) const
	{
		NST_ASSERT( IsOpen() && bool(data) >= bool(size) );

		if (size)
		{
			DWORD read = 0;

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

	void File::EndianSwap(void* const begin,void* const end)
	{
		for (u8 *p=static_cast<u8*>(begin), *e=static_cast<u8*>(end); p != e; p += 2)
		{
			u8 tmp = p[0];
			p[0] = p[1];
			p[1] = tmp;
		}
	}

	void File::ReadText(String::Heap<char>& string,uint length) const
	{
		const uint maxLength = Size() - Position();
		string.Resize( NST_MIN(length,maxLength) );

		if (string.Length())
			Read( string.Ptr(), string.Length() );
	}

	void File::ReadText(String::Heap<wchar_t>& string,uint length) const
	{
		NST_COMPILE_ASSERT( sizeof(wchar_t) == sizeof(u16) );

		const uint maxLength = Size() - Position();
		length = NST_MIN(length,maxLength);
		const uint utf = (length >= 2 ? Peek<u16>() : 0);

		if (utf == UTF16_LE || utf == UTF16_BE)
		{
			length = (length-2) / 2;
			string.Resize( length );
			Seek( CURRENT, 2 );

			if (length)
			{
				Read( string.Ptr(), length * 2 );

				if (utf == UTF16_BE)
					EndianSwap( string.Ptr(), string.Ptr() + length );
			}
		}
		else
		{
			String::Heap<char> tmp;
			ReadText( tmp );
			string = tmp;
		}
	}

	void File::WriteText(cstring const string,const uint length,ibool) const
	{
		Write( string, length );
	}

	void File::WriteText(wstring const string,const uint length,ibool forceUnicode) const
	{
		NST_COMPILE_ASSERT( sizeof(wchar_t) == sizeof(u16) );

		if (length)
		{
			if (forceUnicode || String::Generic<wchar_t>(string,length).Wide())
			{
				const u16 utf = UTF16_LE;				
				Write( &utf, sizeof(u16) );
				Write( string, length * sizeof(wchar_t) );
			}
			else
			{
				const String::Heap<char> tmp( string, length );
				Write( tmp.Ptr(), tmp.Length() );
			}
		}
	}

	void File::ParseText(const void* src,uint size,String::Heap<char>& dst)
	{
		if (size)
			dst.Assign( static_cast<const char*>(src), size );
		else
			dst.Clear();
	}

	void File::ParseText(const void* src,uint size,String::Heap<wchar_t>& dst)
	{
		const uint utf = (size >= 2 ? *static_cast<const u16*>(src) : 0);

		if (utf == UTF16_LE || utf == UTF16_BE)
		{
			size = (size-2) / 2;
			dst.Resize( size );

			if (size)
			{
				dst.Assign( static_cast<const wchar_t*>(src) + 1, size );

				if (utf == UTF16_BE)
					EndianSwap( dst.Ptr(), dst.Ptr() + size );
			}
			else
			{
				dst.Clear();
			}
		}
		else if (size)
		{
			dst.Assign( static_cast<const char*>(src), size );
		}		
		else
		{
			dst.Clear();
		}
	}

	ibool File::FileExist(tstring const path)
	{
		NST_ASSERT( path );

		if (*path)
		{
			const DWORD result = ::GetFileAttributes( path );
			return (result != INVALID_FILE_ATTRIBUTES) && !(result & FILE_ATTRIBUTE_DIRECTORY);
		}

		return FALSE;
	}

	ibool File::FileProtected(tstring const path)
	{
		NST_ASSERT( path );

		if (*path)
		{
			const DWORD result = ::GetFileAttributes( path );
			return (result != INVALID_FILE_ATTRIBUTES) && (result & FILE_ATTRIBUTE_READONLY);
		}

		return FALSE;
	}

	ibool File::DirExist(tstring const path)
	{
		NST_ASSERT( path );
		return *path && ::GetFileAttributes( path ) != INVALID_FILE_ATTRIBUTES;
	}

	ibool File::Delete(tstring const file)
	{
		NST_ASSERT( file && *file );
		return ::DeleteFile( file );
	}
}
