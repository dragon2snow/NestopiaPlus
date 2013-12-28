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

#ifndef NST_MAPPER_105_H
#define NST_MAPPER_105_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		class Mapper105 : public Boards::Mmc1
		{
		public:

			Mapper105(Context&);

			uint NumDipSwitches() const
			{ 
				return 2; 
			}

			uint NumDipSwitchValues(uint j) const
			{ 
				return j == 0 ? 16 : j == 1 ? 2 : 0; 
			}

			cstring GetDipSwitchName(uint) const;
			cstring GetDipSwitchValueName(uint,uint) const;

			int GetDipSwitchValue(uint) const;
			Result SetDipSwitchValue(uint,uint);

		private:

			void SubReset(bool);
			void SubSave(State::Saver&) const;
			void SubLoad(State::Loader&);

			void UpdatePrg();
			void UpdateTimer();

			void UpdateRegister0();
			void UpdateRegister1();
			void UpdateRegister2() {}
			void UpdateRegister3();

			void VSync();

			enum
			{
				IRQ_DISABLE = b00010000
			};

			enum 
			{
				TIME_OFFSET = 11
			};

			ibool irqEnabled;
			uint frames;
			dword seconds;
			dword time;
			uint dipValue;
			ibool displayTime;
			char text[32];
		};
	}
}

#endif
