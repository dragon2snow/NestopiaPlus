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

#ifndef NST_VSSYSTEM_H
#define NST_VSSYSTEM_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include "../api/NstApiInput.hpp"
#include "../NstDipSwitches.hpp"

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
			void SaveState(State::Saver&) const;
			void LoadState(State::Loader&);

		protected:

			struct Context;

			VsSystem(Context&);
			virtual ~VsSystem();

		private:

			virtual void Reset() {}
			virtual void SubSave(State::Saver&) const {}
			virtual void SubLoad(State::Loader&,dword) {}

			class Dip;

			class VsDipSwitches : public DipSwitches
			{
			public:

				VsDipSwitches(Dip*&,uint);
				~VsDipSwitches();

				inline uint Reg(uint) const;
				inline void Reset();
				
				void BeginFrame(Input::Controllers*);

			private:

				uint NumDips() const;
				uint NumValues(uint) const;
				cstring GetDipName(uint) const;
				cstring GetValueName(uint,uint) const;
				uint GetValue(uint) const;
				bool SetValue(uint,uint);

				Dip* const table;
				const uint size;
				uint regs[2];
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

			uint coin;
			VsDipSwitches dips;
			const uint securityPpu;
			const ibool swapPorts;
			Io::Port p2002;

			static const u8 colorMaps[4][64];

		public:

			void BeginFrame(Input::Controllers* input)
			{
				dips.BeginFrame( input );
			}

			DipSwitches& GetDipSwiches()
			{
				return dips;
			}
		};
	}
}

#endif
