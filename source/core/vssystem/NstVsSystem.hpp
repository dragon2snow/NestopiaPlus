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

#ifndef NST_VSSYSTEM_H
#define NST_VSSYSTEM_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include "../api/NstApiInput.hpp"

namespace Nes
{
	namespace Core
	{
		class Ppu;
		class Cpu;

		class VsSystem
		{
		public:

			static VsSystem* Create(Cpu&,Ppu&,dword);
			static void Destroy(VsSystem*&);

			void Reset(bool);
			void BeginFrame(Input::Controllers*);
			void SaveState(State::Saver&) const;
			void LoadState(State::Loader&);

			uint NumDipSwitchValues(uint) const;
			cstring GetDipSwitchName(uint) const;
			cstring GetDipSwitchValueName(uint,uint) const;
			int GetDipSwitchValue(uint) const;
			Result SetDipSwitchValue(uint,uint);

		protected:

			struct Context;

			VsSystem(Context&);
			virtual ~VsSystem();

		private:

			virtual void Reset() {}
			virtual void SubSave(State::Saver&) const {}
			virtual void SubLoad(State::Loader&,dword) {}

			class Dip;

			class Dips
			{
				Dip* const table;
				const uint size;

			public:

				inline Dips(Dip*&,uint);
				~Dips();

				inline Dip& operator [] (uint) const;

				uint Size() const
				{
					return size;
				}
			};

			enum
			{
				DIPSWITCH_4016_MASK	 = b00000011,
				DIPSWITCH_4016_SHIFT = 3,
				DIPSWITCH_4017_MASK  = b11111100,
				DIPSWITCH_4017_SHIFT = 0,
				COIN_1               = Input::Controllers::VsSystem::COIN_1,
				COIN_2               = Input::Controllers::VsSystem::COIN_2,
				COIN                 = COIN_1|COIN_2,
				STATUS_4016_MASK     = (DIPSWITCH_4016_MASK << DIPSWITCH_4016_SHIFT) | COIN,
				STATUS_4017_MASK     = (DIPSWITCH_4017_MASK << DIPSWITCH_4017_SHIFT)
			};

			NES_DECL_PEEK( Nop  )
			NES_DECL_POKE( Nop  )
			NES_DECL_PEEK( 2002 )
			NES_DECL_POKE( 2002 )
			NES_DECL_PEEK( 2006 )
			NES_DECL_POKE( 2006 )
			NES_DECL_PEEK( 2007 )
			NES_DECL_POKE( 2007 )
			NES_DECL_PEEK( 4016 )
			NES_DECL_PEEK( 4016_Swap )
			NES_DECL_POKE( 4016 )
			NES_DECL_PEEK( 4017 )
			NES_DECL_PEEK( 4017_Swap )
			NES_DECL_POKE( 4017 )
			NES_DECL_PEEK( 4020 )
			NES_DECL_POKE( 4020 )

		protected:

			Cpu& cpu;
			Ppu& ppu;

		private:

			const u8* const securityColor;

			Io::Port p2007;
			Io::Port p4016;
			Io::Port p4017;

			uint regs[2];
			uint coin;
			Dips dips;
			const uint securityPpu;
			const ibool swapPorts;
			Io::Port p2002;

			static const u8 colorMaps[4][64];

		public:

			uint NumDipSwitches() const
			{ 
				return dips.Size(); 
			}
		};
	}
}

#endif
