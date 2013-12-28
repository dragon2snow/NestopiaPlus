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

#ifndef NST_BOARDS_YC_H
#define NST_BOARDS_YC_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include "../NstClock.hpp"
#include "../NstDipSwitches.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			class NST_NO_VTABLE Jy : public Mapper
			{
			protected:

				enum DefaultDipSwitch
				{
					DEFAULT_DIP_NMT_OFF,
					DEFAULT_DIP_NMT_CONTROLLED,
					DEFAULT_DIP_NMT_ON
				};

				Jy(Context&,DefaultDipSwitch,bool=false);

			private:

				class CartSwitches : public DipSwitches
				{
				public:

					CartSwitches(uint,bool);

					inline uint GetSetting() const;
					inline ibool IsPpuLatched() const;

				private:

					uint NumDips() const;
					uint NumValues(uint) const;
					cstring GetDipName(uint) const;
					cstring GetValueName(uint,uint) const;
					uint GetValue(uint) const;
					bool SetValue(uint,uint);

					uint data;
					const ibool ppuLatched;
				};

				enum
				{
					DIPSWITCH_NMT  = b00000011,
					DIPSWITCH_GAME = b11000000
				};

				void SubReset(bool);
				void BaseSave(State::Saver&) const;
				void BaseLoad(State::Loader&,dword);
				void UpdatePrg();
				void UpdateChr() const;
				void UpdateChrLatch() const;
				void UpdateExChr();
				void UpdateNmt() const;
				Device QueryDevice(DeviceType);
				void VSync();

				NES_DECL_HOOK( PpuBg )
				NES_DECL_HOOK( PpuSp )

				NES_DECL_ACCESSOR( Chr_0000 )
				NES_DECL_ACCESSOR( Chr_1000 )

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
					void Reset();

					enum
					{
						CTRL0_PRG_MODE      = b00000011,
						CTRL0_PRG_SWAP_32K  = b00000000,
						CTRL0_PRG_SWAP_16K  = b00000001,
						CTRL0_PRG_SWAP_8K   = b00000010,
						CTRL0_PRG_SWAP_8K_R = b00000011,
						CTRL0_PRG_NOT_LAST  = b00000100,
						CTRL0_CHR_MODE      = b00011000,
						CTRL0_CHR_SWAP_8K   = b00000000,
						CTRL0_CHR_SWAP_4K   = b00001000,
						CTRL0_CHR_SWAP_2K   = b00010000,
						CTRL0_CHR_SWAP_1K   = b00011000,
						CTRL0_NMT_CHR       = b00100000,
						CTRL0_NMT_CHR_ROM   = b01000000,
						CTRL0_PRG6_ENABLE   = b10000000,
						CTRL1_MIRRORING     = b00000011,
						CTRL2_NMT_USE_RAM   = b10000000,
						CTRL3_NO_EX_CHR     = b00100000,
						CTRL3_EX_CHR_0      = b00000001,
						CTRL3_EX_CHR_1      = b00011000,
						CTRL3_EX_PRG        = b00000110
					};

					NES_DECL_PEEK( 5001 )
					NES_DECL_PEEK( 5800 )
					NES_DECL_POKE( 5800 )
					NES_DECL_POKE( 5801 )
					NES_DECL_PEEK( 5801 )
					NES_DECL_POKE( 5803 )
					NES_DECL_PEEK( 5803 )

					uint mul[2];
					uint tmp;
					uint ctrl[4];
				};

				struct Banks
				{
					void Reset();

					static uint Unscramble(uint);

					struct ExChr
					{
						uint mask;
						uint bank;
					};

					uint prg[4];
					uint chr[8];
					uint nmt[4];
					ExChr exChr;
					const u8* prg6;
					uint chrLatch[2];
				};

				struct Irq
				{
					struct A12
					{
						A12(Irq&);

						void Reset(bool);
						ibool Signal();

						Irq& base;
					};

					struct M2
					{
						M2(Irq&);

						void Reset(bool);
						ibool Signal();

						Irq& base;
					};

					Irq(Cpu&,Ppu&);

					void Reset();
					ibool IsEnabled() const;
					ibool IsEnabled(uint) const;
					ibool Signal();
					inline void Update();

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
					Clock::A12<A12> a12;
					Clock::M2<M2> m2;
				};

				Regs regs;
				Banks banks;
				Irq irq;
				CartSwitches cartSwitches;
			};
		}
	}
}

#endif
