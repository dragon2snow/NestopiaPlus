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

#ifndef NST_BOARDS_MMC1_H
#define NST_BOARDS_MMC1_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			class NST_NO_VTABLE Mmc1 : public Mapper
			{
			protected:
		
				Mmc1(Context&);

				void SubReset(bool);
				void UpdatePrg();
				void UpdateChr() const;
				void UpdateMirroring() const;
				void VSync();

			private:

				void BaseSave(State::Saver&) const;
				void BaseLoad(State::Loader&,dword);

				virtual void UpdateRegister0();
				virtual void UpdateRegister1();
				virtual void UpdateRegister2();
				virtual void UpdateRegister3();

				void ClearRegisters();
		
				static inline bool IsSoRom(dword);
				inline bool IsWRamEnabled() const;

				NES_DECL_PEEK( wRam )
				NES_DECL_POKE( wRam )
				NES_DECL_PEEK( wRam_SoRom )
				NES_DECL_POKE( wRam_SoRom )
				NES_DECL_POKE( Prg )
						
				struct Serial
				{
					enum
					{
						RESET = b10000000
					};
		
					uint buffer;
					uint shifter;
					idword time;
				};
				
				Serial serial;

			protected:

				enum
				{
					CTRL,
					CHR0,
					PRG1 = CHR0,
					CHR1,
					PRG0 
				};

				enum
				{
					CTRL_MIRRORING     = b00000011,
					CTRL_PRG_SWAP_LOW  = b00000100,
					CTRL_PRG_SWAP_16K  = b00001000,
					CTRL_CHR_SWAP_4K   = b00010000,
					CTRL_RESET         = CTRL_PRG_SWAP_LOW|CTRL_PRG_SWAP_16K,
					PRG1_256K_BANK     = b00010000,
					PRG0_BANK          = b00001111,
					PRG0_WRAM_DISABLED = b00010000
				};

				uint regs[4];

			private:

				const ibool soRom;
			};
		}
	}
}

#endif
