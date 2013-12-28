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

#ifndef NST_BOARDS_VRC_H
#define NST_BOARDS_VRC_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include "NstBrdVrc4.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			class NST_NO_VTABLE Vrc6 : public Mapper
			{
			public:

				class Sound : public Apu::Channel
				{
				public:
		
					Sound(Cpu&,bool=true);
					~Sound();
		
					void WriteSquareReg0 (uint,uint);
					void WriteSquareReg1 (uint,uint);
					void WriteSquareReg2 (uint,uint);
					void WriteSawReg0    (uint);
					void WriteSawReg1    (uint);
					void WriteSawReg2    (uint);
		
					void SaveState(State::Saver&) const;
					void LoadState(State::Loader&);
		
				protected:

					void Reset();
					void UpdateContext(uint,const u8 (&w)[MAX_CHANNELS]);
					Sample GetSample();

				private:
				
					class NST_NO_VTABLE BaseChannel
					{
					protected:
		
						void Reset();
		
						ibool enabled;
						uint  waveLength;
						ibool active;
						iword timer;
						Cycle frequency;
						uint  step;
					};
		
					class Square : BaseChannel
					{
					public:
		
						Square();
		
						void Reset();
		
						NST_FORCE_INLINE dword GetSample(Cycle);		
						NST_FORCE_INLINE void WriteReg0(uint);
						NST_FORCE_INLINE void WriteReg1(uint,uint);
						NST_FORCE_INLINE void WriteReg2(uint,uint);
		
						void SaveState(State::Saver&) const;
						void LoadState(State::Loader&,uint);
		
						void UpdateContext(uint);
		
					private:
		
						bool CanOutput() const;
		
						enum
						{
							VOLUME = Apu::OUTPUT_MUL * 2,
							MIN_FRQ = 0x04
						};
		
						enum
						{
							REG0_VOLUME          = b00001111,
							REG0_DUTY            = b01110000,
							REG0_DIGITIZED       = b10000000,
							REG1_WAVELENGTH_LOW  = b11111111,
							REG2_WAVELENGTH_HIGH = b00001111,
							REG2_ENABLE          = b10000000,
							REG0_DUTY_SHIFT      = 4
						};
		
						uint duty;
						dword volume;
						ibool digitized;
					};
		
					class Saw : BaseChannel
					{
					public:
		
						Saw();
		
						void Reset();
		
						NST_FORCE_INLINE dword GetSample(Cycle);
						NST_FORCE_INLINE void WriteReg0(uint);
						NST_FORCE_INLINE void WriteReg1(uint,uint);
						NST_FORCE_INLINE void WriteReg2(uint,uint);
		
						void SaveState(State::Saver&) const;
						void LoadState(State::Loader&,uint);
		
						void UpdateContext(uint);
		
					private:
		
						bool CanOutput() const;
		
						enum
						{
							VOLUME = Apu::OUTPUT_MUL * 2,
							MIN_FRQ = 0x4,
							FRQ_SHIFT = 1
						};
		
						enum
						{
							REG0_PHASE           = b00111111,
							REG1_WAVELENGTH_LOW  = b11111111,
							REG2_WAVELENGTH_HIGH = b00001111,
							REG2_ENABLE          = b10000000
						};
		
						uint phase;
						dword amp;
					};
		
					Apu& apu;
					Square square[2];
					Saw saw;
					Apu::DcBlocker dcBlocker;
					const ibool hooked;
				};
		
			protected:
		
				enum Type
				{
					TYPE_NORMAL,
					TYPE_SWAP_A0_A1
				};
		
				Vrc6(Context&,Type);
				
			private:
		
				void SubReset(bool);
				void BaseSave(State::Saver&) const;
				void BaseLoad(State::Loader&,dword);
				void VSync();
		
				NES_DECL_POKE( 9000 )
				NES_DECL_POKE( 9001 )
				NES_DECL_POKE( 9002 )
				NES_DECL_POKE( A000 )
				NES_DECL_POKE( A001 )
				NES_DECL_POKE( A002 )
				NES_DECL_POKE( B000 )
				NES_DECL_POKE( B001 )
				NES_DECL_POKE( B002 )
				NES_DECL_POKE( B003 )
				NES_DECL_POKE( F000 )
				NES_DECL_POKE( F001 )
				NES_DECL_POKE( F002 )

				Vrc4::Irq irq;
				Sound sound;
				const Type type;
			};
        }
	}
}

#endif
