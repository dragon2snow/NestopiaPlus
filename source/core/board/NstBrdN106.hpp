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

#ifndef NST_BOARDS_N106_H
#define NST_BOARDS_N106_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			class NST_NO_VTABLE N106 : public Mapper
			{
			public:
	
				class Sound : Apu::Channel
				{
				public:
	
					Sound(Cpu&);
					~Sound();
	
					void Poke_4800(uint);
					uint Peek_4800();
					void Poke_F800(uint);
	
					void SaveState(State::Saver&) const;
					void LoadState(State::Loader&);
	
				private:
	
					void Reset();
					inline void SetChannelState(uint);
	
					inline void WriteWave(uint);
					inline dword FetchFrequency(uint) const;
	
					void UpdateContext(Cycle);
					Sample GetSample();
	
					enum
					{
						NUM_CHANNELS     = 8,
						EXRAM_INC        = b10000000,
						REG_WAVELENGTH   = b00011100,
						REG_ENABLE_SHIFT = 5,
						REG_VOLUME       = b00001111,
						PHASE_SHIFT      = 18,
						SPEED_SHIFT      = 20
					};
	
					class BaseChannel
					{
					public:
	
						void Reset();
	
						inline Sample GetSample(Cycle,Cycle,const i8 (&)[0x100]);
	
						inline void SetFrequency  (uint);
						inline void SetWaveLength (uint);
						inline void SetWaveOffset (uint);
						inline void SetVolume     (uint);
	
						inline void Validate();
	
					private:
	
						enum
						{
							VOLUME = Apu::OUTPUT_MUL / 16
						};
	
						inline bool CanOutput() const;
	
						ibool enabled;
						ibool active;
						Cycle timer;
						Cycle frequency;
						Cycle phase;
						dword waveLength;
						uint  waveOffset;
						uint  volume;
					};
	
					Apu& apu;
	
					Cycle frequency;

					uint exAddress;
					uint exIncrease;
					uint startChannel;
	
					i8 wave[0x100];
					u8 exRam[0x80];
	
					BaseChannel channels[NUM_CHANNELS];
				};

			protected:
	
				enum Type
				{
					TYPE_PLAIN,
					TYPE_ADD_ONS
				};
	
				N106(Context&,Type);
				~N106();
	
			private:
	
				void SubReset(bool);
				void BaseSave(State::Saver&) const;
				void BaseLoad(State::Loader&,dword);
				void SwapChr(uint,uint,uint) const;
				void SwapNmt(uint,uint) const;
				void VSync();
	
				NES_DECL_PEEK( 4800 )
				NES_DECL_POKE( 4800 )
				NES_DECL_PEEK( 5000 )
				NES_DECL_POKE( 5000 )
				NES_DECL_PEEK( 5800 )
				NES_DECL_POKE( 5800 )
				NES_DECL_POKE( 8000 )
				NES_DECL_POKE( 8800 )
				NES_DECL_POKE( 9000 )
				NES_DECL_POKE( 9800 )
				NES_DECL_POKE( A000 )
				NES_DECL_POKE( A800 )
				NES_DECL_POKE( B000 )
				NES_DECL_POKE( B800 )
				NES_DECL_POKE( C000 )
				NES_DECL_POKE( C800 )
				NES_DECL_POKE( D000 )
				NES_DECL_POKE( D800 )
				NES_DECL_POKE( E000 )
				NES_DECL_POKE( E800 )
				NES_DECL_POKE( F000 )
				NES_DECL_POKE( F800 )
	
				uint reg;
	
  				struct Chips;
				Chips* const chips;
			};
		}
	}
}

#endif
