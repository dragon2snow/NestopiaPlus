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

#ifndef NST_BOARDS_BTLTEK2A_H
#define NST_BOARDS_BTLTEK2A_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include "../NstClock.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			class NST_NO_VTABLE BtlTek2A : public Mapper
			{
			public:
		
				void SaveState(State::Saver&) const;
				void LoadState(State::Loader&);
		
			protected:
		
				enum DefaultDipSwitch
				{
					DEFAULT_DIPSWITCH_DETECT,
					DEFAULT_DIPSWITCH_OFF,
					DEFAULT_DIPSWITCH_EXT_MIRRORING
				};
		
				BtlTek2A(Context&,DefaultDipSwitch);
				
			private:
		
				enum
				{
					DIPSWITCH_MIRROR = b00000001,
					DIPSWITCH_GAME   = b11000000
				};
		
				static uint GetDefaultDipSwitch(dword);
		
				void SubReset(bool);
				void UpdatePrg();
				void UpdateChr() const;
				void UpdateName() const;
				void VSync();
		
				NES_DECL_PEEK( 5000 )
				NES_DECL_PEEK( 6000 )
				NES_DECL_POKE( 8000 )
				NES_DECL_POKE( 9000 )
				NES_DECL_POKE( A000 )
				NES_DECL_POKE( B000 )
				NES_DECL_POKE( B004 )
				NES_DECL_POKE( C000 )
				NES_DECL_POKE( C001 )
				NES_DECL_POKE( C002 )
				NES_DECL_POKE( C003 )
				NES_DECL_POKE( C004 )
				NES_DECL_POKE( C005 )
				NES_DECL_POKE( C006 )
				NES_DECL_POKE( D000 )
				NES_DECL_POKE( D001 )
				NES_DECL_POKE( D002 )
				NES_DECL_POKE( D003 )
						   
				struct Regs
				{
					Regs();
		
					enum
					{
						CTRL0_PRG_MODE           = b00000011,
						CTRL0_PRG_SWAP_32K       = b00000000,
						CTRL0_PRG_SWAP_16K       = b00000001,
						CTRL0_PRG_SWAP_8K        = b00000010,
						CTRL0_PRG_SWAP_8K_R      = b00000011,
						CTRL0_PRG_NOT_LAST       = b00000100,
						CTRL0_CHR_MODE           = b00011000,
						CTRL0_CHR_SWAP_8K        = b00000000,
						CTRL0_CHR_SWAP_4K        = b00001000,
						CTRL0_CHR_SWAP_2K        = b00010000,
						CTRL0_CHR_SWAP_1K        = b00011000,
						CTRL0_CHR_NAMETABLES     = b00100000,
						CTRL0_CHR_ROM_NAMETABLES = b01000000,
						CTRL0_PRG6_ENABLE        = b10000000,
						CTRL1_MIRRORING          = b00000011,
						CTRL2_NAME_USE_RAM       = b10000000,
						CTRL3_NO_SECONDARY       = b00100000,
						CTRL3_SECONDARY          = b00011111
					};
		
					void Reset();
		
					NES_DECL_PEEK( 5001 )
					NES_DECL_PEEK( 5800 )
					NES_DECL_POKE( 5800 )
					NES_DECL_POKE( 5801 )
					NES_DECL_PEEK( 5801 )
					NES_DECL_POKE( 5803 )
					NES_DECL_PEEK( 5803 )
		
					uint mul[2];
					uint tmp;
					uint banking;
					uint mirroring;
					uint name;
				};
		
				struct Banks
				{
					Banks();
		
					void Reset();
		
					static uint Unscramble(uint);
		
					uint prg[4];
					uint chr[8];
					uint name[4];
					uint secondary[2];
		
					const u8* prg6;
				};
		
				struct Irq
				{
					void Reset(bool);
					ibool IsEnabled() const;
					inline ibool Signal();
		
					enum
					{
						CYCLE_DELAY = 6
					};
		
					enum
					{
						TOGGLE            = b00000001,
						MODE_SOURCE       = b00000011,
						MODE_M2           = b00000000,
						MODE_PPU_A12      = b00000001,
						MODE_PPU_READ     = b00000010,
						MODE_CPU_WRITE    = b00000011,
						MODE_SCALE_3BIT   = b00000100,
						MODE_SCALE_ADJUST = b00001000,
						MODE_COUNT_ENABLE = b11000000,      
						MODE_COUNT_UP     = b01000000,
						MODE_COUNT_DOWN   = b10000000
					};
		
					uint enabled;
					uint mode;
					uint prescaler;
					uint scale;
					uint count;
					uint flip;
				};
		
				Regs regs;
				Banks banks;
				Clock::A12<Irq> irq;
				uint dipswitch;
				
			public:
		
				uint NumDipSwitches() const
				{ 
					return 2; 
				}
		
				uint NumDipSwitchValues(uint i) const
				{ 
					return i == 0 ? 4 : i == 1 ? 2 : 0; 
				}
		
				cstring GetDipSwitchName(uint) const;
				cstring GetDipSwitchValueName(uint,uint) const;
		
				int GetDipSwitchValue(uint) const;
				Result SetDipSwitchValue(uint,uint);
			};
		}
	}
}

#endif
