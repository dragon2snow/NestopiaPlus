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

#ifndef NST_IO_LOG_H
#define NST_IO_LOG_H

#pragma once

#include "NstString.hpp"

namespace Nestopia
{
	namespace Managers
	{
		class Logfile;
	}

	namespace Io
	{
		class Log : String::Smart<_MAX_PATH>
		{
			friend class Managers::Logfile;

			Log(const Log&);
			void operator = (const Log&);

			struct Callbacker
			{
				inline Callbacker();

				void* data;
				void (NST_CALL *code)(void*,cstring,uint);
			};

			static Callbacker callbacker;

		public:

			Log() {}
			~Log();

			void Flush();

			template<typename T>
			Log& operator << (const T& t)
			{
				String::Smart<_MAX_PATH>::operator << (t);
				return *this;
			}
		};
	}
}

#endif
