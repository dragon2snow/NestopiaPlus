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

#ifndef NST_API_CHEATS_H
#define NST_API_CHEATS_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include "NstApi.hpp"

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4512 )
#endif

namespace Nes
{
	namespace Api
	{
		class Cheats : public Base
		{
		public:
	
			Cheats(Emulator& e)
			: Base(e) {}
	
			struct Code
			{
				u16 address;
				u8 value;
				u8 compare;
				bool useCompare;

				Code(u16 a=0,u8 v=0,u8 c=0,bool u=false)
				: address(a), value(v), compare(c), useCompare(u) {}
			};

			Result SetCode (const Code&);
			Result GetCode (dword,Code&) const;
			Result GetCode (dword,u16*,u8*,u8*,bool*) const;
			Result DeleteCode (dword);	
			dword  NumCodes () const;
			Result ClearCodes ();

			enum
			{
				RAM_SIZE = Core::SIZE_2K
			};

			typedef const u8 (&Ram)[RAM_SIZE];

			Ram GetRam() const;

			static Result NST_CALL GameGenieEncode(const Code&,char (&)[9]);
			static Result NST_CALL GameGenieDecode(const char*,Code&);
			
			static Result NST_CALL ProActionRockyEncode(const Code&,char (&)[9]);
			static Result NST_CALL ProActionRockyDecode(const char*,Code&);
		};
	}
}

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif
