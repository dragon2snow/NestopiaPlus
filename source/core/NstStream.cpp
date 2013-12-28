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

#include <string>
#include <iostream> 
#include "NstStream.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Stream
		{
			static void SeekG(StdStream stream,long offset,std::istream::seekdir dir)
			{
				if (static_cast<std::istream*>(stream)->seekg( offset, dir ).fail())
					throw RESULT_ERR_CORRUPT_FILE;
			}

			static void SeekP(StdStream stream,long offset,std::ostream::seekdir dir)
			{
				if (static_cast<std::ostream*>(stream)->seekp( offset, dir ).fail())
					throw RESULT_ERR_CORRUPT_FILE;
			}

			int In::Peek()
			{
				return static_cast<std::istream*>(stream)->peek();
			}

			void In::SetPos(ulong pos)
			{
				SeekG( stream, pos, std::ios::beg );
			}

			ulong In::GetPos()
			{
				return static_cast<std::istream*>(stream)->tellg();
			}

			void In::Seek(long distance)
			{
				SeekG( stream, distance, std::ios::cur );
			}

			void In::Read(void* data,ulong size)
			{
				NST_ASSERT( data && size );

				if (static_cast<std::istream*>(stream)->read( static_cast<char*>(data), size ).fail())
					throw RESULT_ERR_CORRUPT_FILE;
			}

			uint In::Read8()
			{
				u8 data;
				Read( &data, 1 );
				return data;
			}

			uint In::Read16()
			{
				u8 data[2];
				Read( data, 2 );
				return data[0] | (data[1] << 8);
			}

			dword In::Read32()
			{
				u8 data[4];
				Read( data, 4 );
				return data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
			}

			uint In::Peek8()
			{
				return (uint) Peek();
			}

			uint In::Peek16()
			{
				const uint data = Read16();
				Seek( -2 );
				return data;
			}

			dword In::Peek32()
			{
				const dword data = Read32();
				Seek( -4 );
				return data;
			}

			void In::Validate(u8 check)
			{
				if (Read8() != check)
					throw RESULT_ERR_CORRUPT_FILE;
			}
			
			void In::Validate(u16 check)
			{
				if (Read16() != check)
					throw RESULT_ERR_CORRUPT_FILE;
			}

			void In::Validate(u32 check)
			{
				if (Read32() != check)
					throw RESULT_ERR_CORRUPT_FILE;
			}

			bool In::Eof()
			{
				return 
				(
			       	(static_cast<std::istream*>(stream)->rdstate() & std::istream::eofbit) || 
					Peek() == std::char_traits<char>::eof()
				);
			}

			ulong In::Length()
			{
				const ulong current = GetPos();

				SeekG( stream, 0, std::ios::end );

				const ulong length = GetPos() - current;
				
				SetPos( current );

				return length;
			}

			ulong In::ReadString(void* string)
			{
				if (std::getline( *static_cast<std::istream*>(stream), *static_cast<std::string*>(string), '\0' ).fail())
					throw RESULT_ERR_CORRUPT_FILE;

				return static_cast<std::string*>(string)->length();
			}

			void Out::Write(const void* data,ulong size)
			{
				NST_VERIFY( data && size );

				if (static_cast<std::ostream*>(stream)->write( static_cast<cstring>(data), size ).fail())
					throw RESULT_ERR_CORRUPT_FILE;
			}

			void Out::Write8(const uint data)
			{
				const u8 d = data;
				Write( &d, 1 );
			}

			void Out::Write16(const uint data)
			{
				const u8 d[2] = 
				{
					data & 0xFF, 
					data >> 8
				};

				Write( d, 2 );
			}

			void Out::Write32(const dword data)
			{
				const u8 d[4] = 
				{
					data & 0xFF, 
					(data >>  8) & 0xFF,
					(data >> 16) & 0xFF,
					data >> 24
				};

				Write( d, 4 );
			}

			void Out::Seek(long distance)
			{
				SeekP( stream, distance, std::ios::cur );
			}

			void Out::SetPos(ulong pos)
			{
				SeekP( stream, pos, std::ios::beg );
			}

			ulong Out::GetPos()
			{
				return static_cast<std::ostream*>(stream)->tellp();
			}

			ulong Out::Length()
			{
				const ulong current = GetPos();

				SeekP( stream, 0, std::ios::end );

				const ulong length = GetPos() - current;

				SetPos( current );

				return length;
			}
		}
	}
}
