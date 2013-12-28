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

#ifndef NST_BOARDS_FME07_H
#define NST_BOARDS_FME07_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			class NST_NO_VTABLE Fme07 : public Mapper
			{
			public:

				class Sound : Apu::Channel
				{
				public:
		
					Sound(Cpu&);
					~Sound();
		
					void Poke_E000(uint);
		
					void LoadState(State::Loader&);
					void SaveState(State::Saver&) const;
		
				private:
		
					enum
					{
						NUM_SQUARES = 3
					};

					void Reset();
					void UpdateContext(uint);
					Sample GetSample();
					bool CanOutput() const;
		
					class Square
					{
					public:
		
						void Reset(uint);
						void UpdateContext(uint);
		
						void WriteReg0(uint,uint);
						void WriteReg1(uint,uint);
						void WriteReg2(uint);
						inline void WriteReg3(uint);
		
						NST_FORCE_INLINE Sample GetSample(Cycle);
		
						void SaveState(State::Saver&) const;
						void LoadState(State::Loader&,uint);
		
					private:
		
						void UpdateFrequency(uint);
		
						inline bool CanOutput() const;
		
						enum
						{
							REG0_WAVELENGTH_LOW  = b11111111,
							REG1_WAVELENGTH_HIGH = b00001111,
							REG2_DISABLE         = b00000001,
							REG3_VOLTAGE         = b00001111,
							REG3_ENVELOPE        = b00010000
						};
		
						enum 
						{
							FRQ_SHIFT = 4,
							MAX_WAVELENGTH = REG0_WAVELENGTH_LOW | (uint(REG1_WAVELENGTH_HIGH) << 8)
						};
		
						ibool  enabled;
						ibool  active;
						uint   waveLength;
						Cycle  frequency;
						idword timer;
						int    voltage;
						uint   dc;
		
						static const u16 voltages[16]; 

					public:
		
						ibool IsActive() const
						{ 
							return active; 
						}
					};
		
					Apu& apu;
					uint regSelect;
					Square squares[NUM_SQUARES];

				public:

					void Poke_C000(uint data)
					{ 
						regSelect = data;
					}
				};
		
			protected:
		
				Fme07(Context&);
				~Fme07();
		
			private:
		
				class BarcodeWorld;

				void SubReset(bool);
				void BaseSave(State::Saver&) const;
				void BaseLoad(State::Loader&,dword);
				Device QueryDevice(DeviceType);
				void VSync();
		
				NES_DECL_POKE( 8000  )
				NES_DECL_POKE( A000  )
				NES_DECL_POKE( C000  )
				NES_DECL_POKE( E000  )
		
				struct Irq
				{
					void Reset(bool);
					ibool Signal();
		
					uint count;
					ibool enabled;
				};
		
				uint command;
				Clock::M2<Irq> irq;
				Sound sound;
				BarcodeWorld* const barcodeWorld;
			};
		}
	}
}

#endif
