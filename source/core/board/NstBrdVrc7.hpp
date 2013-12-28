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

#ifndef NST_BOARDS_VRC7_H
#define NST_BOARDS_VRC7_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			class NST_NO_VTABLE Vrc7 : public Mapper
			{
			public:

				class Sound : public Apu::Channel
				{
				public:

					Sound(Cpu&,bool=true);
					~Sound();

					void WriteReg0(uint);
					void WriteReg1(uint);

					void SaveState(State::Saver&) const;
					void LoadState(State::Loader&);

				protected:

					void Reset();
					void UpdateContext(uint,const u8 (&w)[MAX_CHANNELS]);
					Sample GetSample();

				private:

					void ResetClock();

					enum
					{
						EG_PHASE_SHIFT = 15,
						EG_MUTE        = 0xFF,
						EG_END         = 0x7F,
						PG_PHASE_SHIFT = 9,
						WAVE_SIZE      = 0x200,
						WAVE_RANGE     = WAVE_SIZE-1,
						PITCH_SHIFT    = 8,
						PITCH_SIZE     = 0x100,
						PITCH_RANGE    = 0xFFFF,
						AMP_SHIFT      = 8,
						AMP_SIZE       = 0x100,
						AMP_RANGE      = 0xFFFF,
						LIN2LOG_SIZE   = 0x80,
						DB2LIN_SIZE    = 0x400,
						TL_SIZE        = 0x40,
						FEEDBACK_SHIFT = 8,
						CLOCK_DIV      = 3579545UL / 72,
						CLOCK_RATE     = (1UL << 31) / CLOCK_DIV,
						PG_PHASE_RANGE = (1UL << 18) - 1,
						EG_BEGIN       = 1UL << 22
					};

					static const dword PITCH_RATE;
					static const dword AMP_RATE;
  
					class Tables
					{
					public:

						Tables();

						inline uint GetAmp(uint) const;
						inline uint GetPitch(uint) const;
						inline uint GetSustainLevel(uint,uint,uint) const;
						inline uint GetTotalLevel(uint,uint,uint,uint) const;
						inline uint GetLog(uint) const;
						inline dword GetAttack(uint,uint) const;
						inline dword GetDecay(uint,uint) const;
						inline dword GetSustain(uint,uint) const;
						inline dword GetRelease(uint,uint) const;
						inline dword GetPhase(uint,uint,uint) const;
						inline Sample GetOutput(uint,uint,uint) const;

					private:

						u16 pitch[PITCH_SIZE];
						u8  amp[AMP_SIZE];
						u8  lin2log[LIN2LOG_SIZE];
 						u32 adr[2][16][16];
						u16 wave[2][WAVE_SIZE];
						i16 db2lin[DB2LIN_SIZE];
						u8  sl[2][8][2];
						u8  tl[16][8][TL_SIZE][4];
						u32 phase[512][8][16];
					};

					enum
					{
						NUM_OPLL_CHANNELS = 6
					};

					class OpllChannel
					{
					public:

						OpllChannel();

						void Reset();
						void Update(const Tables&);
						void SaveState(State::Saver&) const;
						void LoadState(State::Loader&,const Tables&);

						NST_FORCE_INLINE void WriteReg0 (uint,const Tables&);
						NST_FORCE_INLINE void WriteReg1 (uint,const Tables&);
						NST_FORCE_INLINE void WriteReg2 (uint,const Tables&);
						NST_FORCE_INLINE void WriteReg3 (uint);
						NST_FORCE_INLINE void WriteReg4 (uint,const Tables&);
						NST_FORCE_INLINE void WriteReg5 (uint,const Tables&);
						NST_FORCE_INLINE void WriteReg6 (uint,const Tables&);
						NST_FORCE_INLINE void WriteReg7 (uint,const Tables&);
						NST_FORCE_INLINE void WriteReg8 (uint,const Tables&);
						NST_FORCE_INLINE void WriteReg9 (uint,const Tables&);
						NST_FORCE_INLINE void WriteRegA (uint,const Tables&);

						NST_FORCE_INLINE Sample GetSample(uint,uint,const Tables&);

					private:

						void UpdatePhase        (const Tables&,uint);
						void UpdateSustainLevel (const Tables&,uint);
						void UpdateTotalLevel   (const Tables&,uint);
						void UpdateEgPhase      (const Tables&,uint);

						enum Mode
						{ 
							EG_SETTLE, 
							EG_ATTACK, 
							EG_DECAY, 
							EG_HOLD, 
							EG_SUSTAIN, 
							EG_RELEASE, 
							EG_FINISH 
						};

						enum
						{
							REG01_MULTIPLE      = b00001111,
							REG01_RATE          = b00010000,
							REG01_HOLD          = b00100000,
							REG01_USE_VIBRATO   = b01000000,
							REG01_USE_AMP		= b10000000,
							REG2_TOTAL_LEVEL    = b00111111,
							REG3_FEEDBACK       = b00000111,
							REG3_MODULATED_WAVE = b00001000,
							REG3_CARRIER_WAVE   = b00010000,
							REG45_DECAY         = b00001111,
							REG45_ATTACK        = b11110000,
							REG67_RELEASE       = b00001111,
							REG67_SUSTAIN_LEVEL = b11110000,
							REG8_FRQ_LO         = b11111111,
							REG9_FRQ_HI         = b00000001,
							REG9_BLOCK          = b00001110,
							REG9_KEY            = b00010000,
							REG9_SUSTAIN        = b00100000,
							REGA_VOLUME         = b00001111,
							REGA_INSTRUMENT	    = b11110000,
							SUSTAIN_LEVEL_MAX   = 0x100,
						};

						struct Patch
						{
							enum { CUSTOM };

							uint instrument;
							u8 tone[8];
							u8 custom[8];

							static const u8 preset[15][8];
						};

						enum
						{
							MODULATOR,
							CARRIER,
							NUM_SLOTS
						};

						struct Slot
						{
							struct Eg
							{
								Mode mode;
								dword phase;
								dword counter;
							};

							struct Pg
							{
								dword phase;
								dword counter;
							};

							Pg pg;
							Eg eg;
							uint tl;	  
							uint sl;      
							Sample output;
						};

						uint frequency;
						uint key;
						uint sustain;
						uint block;
						uint volume;
						Patch patch;
						Slot slots[NUM_SLOTS];
						Sample feedback;
					};

					Apu& apu;
					uint reg;

					dword sampleRate;
					dword samplePhase;
					dword pitchPhase;
					dword ampPhase;
					
					Sample prevSample;
					Sample nextSample;

					OpllChannel channels[NUM_OPLL_CHANNELS];
					const Tables tables;
					
					const ibool hooked;
				};

			protected:

				Vrc7(Context&);

			private:
		
				void SubReset(bool);
				void BaseSave(State::Saver&) const;
				void BaseLoad(State::Loader&,dword);
				void VSync();
		
				NES_DECL_POKE( 9010 )
				NES_DECL_POKE( 9030 )
				NES_DECL_POKE( E000 )
				NES_DECL_POKE( E008 )
				NES_DECL_POKE( F000 )
				NES_DECL_POKE( F008 )
		
				Vrc4::Irq irq;
				Sound sound;
			};
		}
	}
}

#endif
