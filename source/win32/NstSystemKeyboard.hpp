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

#ifndef NST_SYSTEM_KEYBOARD_H
#define NST_SYSTEM_KEYBOARD_H

#pragma once

#include "NstMain.hpp"

namespace Nestopia
{
	namespace System
	{
		class Keyboard
		{
		public:

			enum 
			{
				NUM_KEYS = 0x100,
				NONE = 0
			};

			static uint DikKey(cstring);
			static uint VikKey(cstring);
			
			enum Indicator
			{
				CAPS_LOCK = 0x14,
				NUM_LOCK = 0x90,
				SCROLL_LOCK = 0x91
			};

			static bool ToggleIndicator(Indicator,bool);

		private:

			enum 
			{
				VIK,
				DIK,
				NUM_SETS
			};

        #pragma pack(push,1)

			struct Key
			{
				inline ibool operator < (const Key&) const;

				cstring name;
				uchar vik;
				uchar dik;
			};

        #pragma pack(pop)

			static const Key* Locate(cstring);

			static const Key keys[];
			static const uchar table[NUM_SETS][NUM_KEYS];
			static const uchar converter[NUM_SETS][NUM_KEYS];

		public:

			static cstring VikName(uint key)
			{
				NST_ASSERT( key < NUM_KEYS );
				return keys[table[VIK][key]].name;
			}

			static cstring DikName(uint key)
			{
				NST_ASSERT( key < NUM_KEYS );
				return keys[table[DIK][key]].name;
			}

			static uint VikToDik(uint key)
			{
				NST_ASSERT( key < NUM_KEYS );
				return converter[VIK][key];
			}

			static uint DikToVik(uint key)
			{
				NST_ASSERT( key < NUM_KEYS );
				return converter[DIK][key];
			}

			static ibool DikValid(uint key)
			{
				NST_ASSERT( key < NUM_KEYS );
				return key != 0;
			}

			static ibool VikValid(uint key)
			{
				NST_ASSERT( key < NUM_KEYS );
				return key != 0;
			}
		};
	}
}

#endif
