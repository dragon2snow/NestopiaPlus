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

#ifndef NST_LOG_H
#define NST_LOG_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include "NstCore.hpp"

namespace Nes
{
	namespace Core
	{
		class Log
		{
		public:

			Log();
			~Log();

			class Hex
			{
				friend class Log;

				const dword value;
				cstring const format;

			public:

				Hex(u32 v) : value(v), format("%08X") {}
				Hex(u16 v) : value(v), format("%04X") {}
				Hex(u8  v) : value(v), format("%02X") {}
			};

			Log& operator << (char);
			Log& operator << (cstring);
			Log& operator << (const Hex&);
			Log& operator << (long);
			Log& operator << (ulong);

		private:

			void Append(cstring,size_t);

			struct Object;
			Object& object;

		public:

			Log& operator << (schar  i) { return operator << ( (long)  i ); }
			Log& operator << (uchar  i) { return operator << ( (ulong) i ); }
			Log& operator << (short  i) { return operator << ( (long)  i ); }
			Log& operator << (ushort i) { return operator << ( (ulong) i ); }
			Log& operator << (int    i) { return operator << ( (long)  i ); }
			Log& operator << (uint   i) { return operator << ( (ulong) i ); }

			static void Flush(cstring,dword);

			template<size_t N>
			Log& operator << (const char (&c)[N])
			{
				NST_COMPILE_ASSERT( N );
				Append( c, N-1 );
				return *this;
			}

			template<size_t N>
			static void Flush(const char (&c)[N])
			{
				NST_COMPILE_ASSERT( N );
				Flush( c, N-1 );
			}
		};
	}
}

#endif
