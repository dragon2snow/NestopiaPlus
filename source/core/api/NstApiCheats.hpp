////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2007 Martin Freij
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

#ifndef NST_API_CHEATS_H
#define NST_API_CHEATS_H

#include "NstApi.hpp"

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#if NST_ICC >= 810
#pragma warning( push )
#pragma warning( disable : 444 )
#elif NST_MSVC >= 1200
#pragma warning( push )
#pragma warning( disable : 4512 )
#endif

namespace Nes
{
	namespace Api
	{
		class Cheats : public Base
		{
			struct Lut;

		public:

			template<typename T>
			Cheats(T& e)
			: Base(e) {}

			struct Code
			{
				ushort address;
				uchar value;
				uchar compare;
				bool useCompare;

				explicit Code(ushort a=0,uchar v=0,uchar c=0,bool u=false)
				: address(a), value(v), compare(c), useCompare(u) {}
			};

			Result SetCode (const Code&) throw();
			Result GetCode (ulong,Code&) const throw();
			Result GetCode (ulong,ushort*,uchar*,uchar*,bool*) const throw();
			Result DeleteCode (ulong) throw();
			ulong  NumCodes () const throw();
			Result ClearCodes () throw();

			enum
			{
				RAM_SIZE = 0x800
			};

			typedef const uchar (&Ram)[RAM_SIZE];

			Ram GetRam() const throw();

			static Result NST_CALL GameGenieEncode(const Code&,char (&)[9]) throw();
			static Result NST_CALL GameGenieDecode(const char*,Code&) throw();

			static Result NST_CALL ProActionRockyEncode(const Code&,char (&)[9]) throw();
			static Result NST_CALL ProActionRockyDecode(const char*,Code&) throw();
		};
	}
}

#if NST_MSVC >= 1200 || NST_ICC >= 810
#pragma warning( pop )
#endif

#endif
