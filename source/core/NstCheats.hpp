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

#ifndef NST_CHEATS_H
#define NST_CHEATS_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include <vector>

namespace Nes
{
	namespace Core
	{
		class Cpu;

		class Cheats
		{
		public:

			Cheats(Cpu&);
			~Cheats();

			void Reset();
			void BeginFrame() const;
			void ClearCodes();

			Result GetCode (dword,u16*,u8*,u8*,bool*) const;	
			Result SetCode (u16,u8,u8,bool,bool);
			Result DeleteCode (dword);

		private:

			NES_DECL_PEEK( Wizard )
			NES_DECL_POKE( Wizard )

			struct LoCode
			{
				LoCode(u16=0,u8=0,u8=0,bool=false);

				bool operator == (const LoCode&) const;

				u16 address;
				u8 data;
				u8 compare;
				bool useCompare;
			};

			struct HiCode : LoCode
			{
				HiCode(u16=0,u8=0,u8=0,bool=false);
				inline HiCode(uint);

				inline bool operator < (const HiCode&) const;

				const Io::Port* port;
			};

			typedef std::vector<LoCode> LoCodes;
			typedef std::vector<HiCode> HiCodes;

			void Map(HiCode&);

			Cpu& cpu;
			LoCodes loCodes;
			HiCodes hiCodes;

		public:

			dword NumCodes() const
			{
				return loCodes.size() + hiCodes.size();
			}
		};
	}
}

#endif
