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

#ifndef NST_SIGNEDARITHMETIC_H
#define NST_SIGNEDARITHMETIC_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		template<typename T,bool>
		struct SignShifter
		{
			static T Left(T value,uint count)
			{
				return value << count;
			}

			static T Right(T value,uint count)
			{
				return value >> count;
			}
		};

		template<typename T>
		struct SignShifter<T,false>
		{
			static T Left(T value,uint count)
			{
				return (value >= 0 ? +T(ulong(value) << count) : -T(ulong(-value) << count));
			}

			static T Right(T value,uint count)
			{
				return (value >= 0 ? +T(ulong(value) >> count) : -T(ulong(-value) >> count));
			}
		};

		template<typename T>
		inline T sign_shl(T value,uint count)
		{
			enum {SUPPORTED = (-7 << 1 == -14)};
			return SignShifter<T,SUPPORTED>::Left( value, count );
		}

		template<typename T>
		inline T sign_shr(T value,uint count)
		{
			enum {SUPPORTED = ((-7 >> 1 == -4) || (-7 >> 1 == -3))};
			return SignShifter<T,SUPPORTED>::Right( value, count );
		}

		template<typename U,bool>
		struct SignCaster
		{
			template<typename T>
			static T Cast(T t)
			{
				return static_cast<T>(static_cast<U>(t));
			}
		};

		template<typename U>
		struct SignCaster<U,false>
		{
			enum {SIGN_SHIFT = sizeof(U) * CHAR_BIT - 1};

			template<typename T>
			static T Cast(T t)
			{
				return t | ((t & (1UL << SIGN_SHIFT)) ? ~T(0) << SIGN_SHIFT : T(0));
			}
		};

		template<typename U,typename T>
		inline T sign_cast(const T t)
		{
			enum {SUPPORTED = T(U(-1)) == ~T(0) };
			return SignCaster<U,SUPPORTED>::Cast( t );
		}
	}
}

#endif
