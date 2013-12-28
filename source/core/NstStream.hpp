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

#ifndef NST_STREAM_H
#define NST_STREAM_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include "NstCore.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Stream
		{
			class In
			{
				StdStream const stream;

				int Peek();

			public:

				explicit In(StdStream s)
				: stream(s)	
				{
					NST_ASSERT( stream );
				}

				void  SetPos(ulong);
				ulong GetPos();
				void  Read(void*,ulong);
				uint  Read8();
				uint  Read16();
				dword Read32();
				uint  Peek8();
				uint  Peek16();
				dword Peek32();
				void  Seek(long);
				void  Validate(u8);
				void  Validate(u16);
				void  Validate(u32);
				bool  Eof();
				ulong Length();
				ulong ReadString(void*);

				template<size_t N>
				void Read(u8 (&data)[N])
				{
					Read( data, N );
				}

				StdStream GetStdStream() const
				{
					return stream;
				}
			};

			class Out
			{
				StdStream const stream;

			public:

				explicit Out(StdStream s)
				: stream(s)	
				{
					NST_ASSERT( stream );
				}

				void  Write(const void*,ulong);
				void  Write8(uint);
				void  Write16(uint);
				void  Write32(dword);
				void  Seek(long);
				void  SetPos(ulong);
				ulong GetPos();
				ulong Length();

				StdStream GetStdStream() const
				{
					return stream;
				}
			};
		}
	}
}

#endif
