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

#ifndef NST_BOARDS_MMC5_H
#define NST_BOARDS_MMC5_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			class NST_NO_VTABLE Mmc5 : public Mapper
			{
			public:

				class Sound : public Apu::Channel
				{
				public:
			
					Sound(Cpu&,bool=true);
					~Sound();
			
					void SaveState(State::Saver&) const;
					void LoadState(State::Loader&);
			
				protected:

					void Reset();
					void UpdateContext(uint,const u8 (&w)[MAX_CHANNELS]);
					Cycle Clock();
					Sample GetSample();

				private:
						
					NES_DECL_POKE( 5000 )
					NES_DECL_POKE( 5002 )
					NES_DECL_POKE( 5003 )
					NES_DECL_POKE( 5004 )
					NES_DECL_POKE( 5006 )
					NES_DECL_POKE( 5007 )
					NES_DECL_POKE( 5011 )
					NES_DECL_POKE( 5010 )
					NES_DECL_PEEK( 5015 )
					NES_DECL_POKE( 5015 )
					NES_DECL_PEEK( 5205 )
					NES_DECL_POKE( 5205 )
					NES_DECL_PEEK( 5206 )
					NES_DECL_POKE( 5206 )
					NES_DECL_PEEK( Nop  )
			
					enum 
					{
						NUM_SQUARES = 2
					};
			
					class Square
					{
					public:
			
						Square();

						void Reset();		
								
						NST_FORCE_INLINE dword GetSample(Cycle);
			
						NST_FORCE_INLINE void WriteReg0(uint);
						NST_FORCE_INLINE void WriteReg1(uint,uint);
						NST_FORCE_INLINE void WriteReg2(uint,uint);
			
						NST_FORCE_INLINE void Enable(uint);
						NST_FORCE_INLINE void ClockQuarter();
						NST_FORCE_INLINE void ClockHalf();
			
						void UpdateContext(uint);
			
						void SaveState(State::Saver&) const;
						void LoadState(State::Loader&,dword);
			
					private:
			
						inline bool CanOutput() const;
			
						enum
						{
							MIN_FRQ              = 0x4,
							REG1_WAVELENGTH_LOW  = b11111111,
							REG2_WAVELENGTH_HIGH = b00000111,
							DUTY_SHIFT           = 6
						};
			
						uint  waveLength;
						ibool active;
						Cycle frequency;
						iword timer;
						uint  step;
						uint  duty;
			
						Apu::LengthCounter lengthCounter;
						Apu::Envelope envelope;
			
					public:
			
						uint GetLengthCounter() const
						{ 
							return lengthCounter.GetCount();
						}
					};
			
					class Pcm
					{
					public:

						Pcm();

						void Reset();
					
						NST_FORCE_INLINE void WriteReg0(uint);
						NST_FORCE_INLINE void WriteReg1(uint);
			
						void SaveState(State::Saver&) const;
						void LoadState(State::Loader&);
			
					private:
			
						enum
						{
							VOLUME = Apu::OUTPUT_MUL / 4,
							PCM_DISABLE = 0x1
						};
			
						ibool enabled;
						Sample sample;
						Sample amp;
			
					public:
			
						Sample GetSample()
						{ 
							return sample; 
						}
					};
			
					uint value[2];
					Cpu& cpu;
					uint halfClock;
					Square square[NUM_SQUARES];
					Pcm pcm;
					Apu::DcBlocker dcBlocker;
					const ibool hooked;
				};

			protected:
	
				Mmc5(Context&);
	
				void SubReset(bool);
	
			private:
	
				static uint DetectWRam(dword,dword);

				void VBlank();
				void HDummy();
				void HActive0();
				void HActiveX();	
				void VSync();
				void BaseSave(State::Saver&) const;
				void BaseLoad(State::Loader&,dword);

				template<uint ADDRESS>
				void SwapPrg8Ex(uint);

				void UpdatePrg();
				void UpdateChrA() const;
				void UpdateChrB() const;
				void UpdateRenderMethod();
				
				bool ClockSpliter();

				inline void Update();

				template<uint>
				inline uint FetchByte(uint) const;

				uint GetExtPattern(uint) const;
				uint GetSpliterAttribute() const;
				uint GetSpliterPattern(uint) const;
	
				NES_DECL_HOOK( CpuUpdate )
				NES_DECL_HOOK( PpuBgMode )
				NES_DECL_HOOK( PpuSpMode )

				template<uint NT> NES_DECL_ACCESSOR( Nt         )
				template<uint NT> NES_DECL_ACCESSOR( NtExt      )
				template<uint NT> NES_DECL_ACCESSOR( NtSplit    )
				template<uint NT> NES_DECL_ACCESSOR( NtExtSplit )
				template<uint AT> NES_DECL_ACCESSOR( AtSplit    )

				NES_DECL_ACCESSOR( CRom         )
				NES_DECL_ACCESSOR( CRomExt      )
				NES_DECL_ACCESSOR( CRomSplit    )
				NES_DECL_ACCESSOR( CRomExtSplit )

				NES_DECL_POKE( 2001 )
				NES_DECL_PEEK( 2001 )

				NES_DECL_POKE( 5100 )
				NES_DECL_POKE( 5101 )
				NES_DECL_POKE( 5102 )
				NES_DECL_POKE( 5103 )
				NES_DECL_POKE( 5104 )
				NES_DECL_POKE( 5105 )
				NES_DECL_POKE( 5106 )
				NES_DECL_POKE( 5107 )
				NES_DECL_POKE( 5113 )
				NES_DECL_POKE( 5114 )
				NES_DECL_POKE( 5120 )
				NES_DECL_POKE( 5128 )
				NES_DECL_POKE( 5130 )
				NES_DECL_POKE( 5200 )
				NES_DECL_POKE( 5201 )
				NES_DECL_POKE( 5202 )
				NES_DECL_POKE( 5203 )
				NES_DECL_PEEK( 5204 )
				NES_DECL_POKE( 5204 )
				NES_DECL_PEEK( 5C00 )
				NES_DECL_POKE( 5C00 )
				NES_DECL_PEEK( 6000 )
				NES_DECL_POKE( 6000 )
				NES_DECL_PEEK( 8000 )
				NES_DECL_POKE( 8000 )
				NES_DECL_PEEK( A000 )
				NES_DECL_POKE( A000 )
				NES_DECL_PEEK( C000 )
				NES_DECL_POKE( C000 )
	
				struct Flow
				{
					void Reset();

					typedef void (Mmc5::*Phase)();

					Cycle cycles;
					Phase phase;
					uint scanline;

					static const Cycle vSync[2];
					static const Cycle hDummy[2][2];
					static const Cycle hSync[2];
				};

				struct Irq 
				{
					void Reset();

					enum
					{			  
						ENABLED    = 0x01,
						FRAME      = 0x40,
						HIT        = 0x80,
						SIGNAL_HIT = ENABLED|HIT
					};

					uint state;
					uint count;
					uint target;
				};

				struct Regs
				{
					void Reset();

					enum
					{
						PRG_MODE            = b00000011,
						PRG_MODE_32K	    = b00000000,
						PRG_MODE_16K	    = b00000001,
						PRG_MODE_16K_8K	    = b00000010,
						PRG_MODE_8K		    = b00000011,
						PRG_ROM_SELECT      = b10000000,
						PRG_ROM_BANK        = b01111111,
						PRG_RAM_BANK        = b00000111,
						CHR_MODE            = b00000011,
						CHR_MODE_8K         = b00000000,
						CHR_MODE_4K         = b00000001,
						CHR_MODE_2K         = b00000010,
						CHR_MODE_1K         = b00000011,
						CHR_HIGH			= b00000011,
						WRK_WRITABLE_A      = b00000010,
						WRK_WRITABLE_B      = b00000001,
						NMT_MODE            = b00000011,
						NMT_CIRAM_0         = b00000000,
						NMT_CIRAM_1         = b00000001,
						NMT_EXRAM           = b00000010,
						NMT_FILL			= b00000011,
						EXRAM_MODE          = b00000011,
						EXRAM_MODE_PPU_NT   = b00000000,
						EXRAM_MODE_PPU_EXT  = b00000001,
						EXRAM_MODE_CPU_RAM  = b00000010,
						EXRAM_MODE_CPU_ROM  = b00000011,
						EXRAM_EXT_CHR_BANK	= b00111111,
						PPU_CTRL0_SP8X16	= b00100000,
						PPU_CTRL1_ENABLED	= b00011000
					};
		
					uint prgMode;
					uint chrMode;
					uint exRamMode;
				};

				struct Banks
				{
					Banks(uint);

					void Reset();

					enum
					{
						READABLE_6  = 0x004,
						READABLE_8  = 0x008,
						READABLE_A  = 0x010,
						READABLE_C  = 0x020,
						WRITABLE_6  = 0x040,
						WRITABLE_8  = 0x080,
						WRITABLE_A  = 0x100,
						WRITABLE_C  = 0x200,
						CAN_WRITE_6 = READABLE_6|WRITABLE_6|Regs::WRK_WRITABLE_A|Regs::WRK_WRITABLE_B,
						CAN_WRITE_8 = READABLE_8|WRITABLE_8|Regs::WRK_WRITABLE_A|Regs::WRK_WRITABLE_B,
						CAN_WRITE_A = READABLE_A|WRITABLE_A|Regs::WRK_WRITABLE_A|Regs::WRK_WRITABLE_B,
						CAN_WRITE_C = READABLE_C|WRITABLE_C|Regs::WRK_WRITABLE_A|Regs::WRK_WRITABLE_B
					};

					enum FetchMode
					{
						FETCH_MODE_NONE,
						FETCH_MODE_BG,
						FETCH_MODE_SP
					};

					enum LastChr
					{
						LAST_CHR_A = 0,
						LAST_CHR_B = 1
					};

					class Wrk
					{					
						u8 banks[8];

					public:

						enum
						{
							INVALID = 8
						};

						Wrk(dword);

						inline uint operator [] (uint) const;
					};

					uint nmt;
					u16 chrA[8];
					u16 chrB[4];
					dword chrHigh;
					LastChr lastChr;
					FetchMode fetchMode;
					uint security;
					u8 prg[4];
					const Wrk wrk;
				};
	
				struct Filler
				{
					void Reset();

					uint tile;
					uint attribute;

					static const u8 squared[4];
				};

				struct Spliter
				{
					void Reset();

					enum
					{
						CTRL_START      = b00011111,
						CTRL_RIGHT_SIDE = b01000000,
						CTRL_ENABLED    = b10000000
					};

					uint ctrl;
					uint tile;
					ibool inside;
					uint yStart;
					uint chrBank;
					uint x;
					uint y;
				};
	
				struct ExRam
				{
					void Reset(bool);

					uint tile;
					u8 mem[SIZE_1K];
				};
		
				Flow flow;
				Irq irq;
				Regs regs;
				Banks banks;
				const u8* ciRam[2];
				Filler filler;
				Spliter spliter;
				Io::Port p2001;
				ExRam exRam;
				Sound sound;

				static const Io::Accessor::Type<Mmc5>::Definition chrMethods[8];
				static const Io::Accessor::Type<Mmc5>::Definition nmtMethods[8][4][2];
			};
		}
	}
}

#endif
