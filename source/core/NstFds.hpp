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

#ifndef NST_FDS_H
#define NST_FDS_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include "NstImage.hpp"
#include "NstCpu.hpp"
#include "NstClock.hpp"

namespace Nes
{
	namespace Core
	{
		class Ppu;

		class Fds : public Image
		{
		public:

			Fds(Context&);
			~Fds();

			void Reset(bool);
			Result InsertDisk(uint,uint);
			Result EjectDisk();
			
			void VSync();
			
			void LoadState(State::Loader&);
			void SaveState(State::Saver&) const;
			
			class Bios
			{
				friend class Fds;

            #ifdef _DEBUG

				Bios() {}

            #endif

				struct Instance
				{
					Instance();

					NES_DECL_PEEK( Rom )
					NES_DECL_POKE( Nop )

					u8 rom[NES_8K];
					ibool loaded;
				};

				static Instance instance;

			public:

				static Result Set(StdStream);
				static Result Get(StdStream);

				static ibool IsLoaded()
				{
					return instance.loaded;
				}
			};

			class Sound : Apu::Channel
			{
			public:
		
				Sound(Cpu&);
				~Sound();
		
				void SaveState(State::Saver&) const;
				void LoadState(State::Loader&);
		
			private:
		
				Cycle Clock();
		
				bool CanOutput() const;
				inline bool CanModulate() const;
		
				void Reset();
				void ResetChannel();
				void UpdateContext(uint);
		
				NST_FORCE_INLINE uint GetModulation() const;
				Sample GetSample();
		
				NES_DECL_PEEK( Nop  )
				NES_DECL_POKE( Nop  )
				NES_DECL_PEEK( 4040 )
				NES_DECL_POKE( 4040 )
				NES_DECL_POKE( 4080 )
				NES_DECL_POKE( 4082 )
				NES_DECL_POKE( 4083 )
				NES_DECL_POKE( 4084 )
				NES_DECL_POKE( 4085 )
				NES_DECL_POKE( 4086 )
				NES_DECL_POKE( 4087 )
				NES_DECL_POKE( 4088 )
				NES_DECL_POKE( 4089 )
				NES_DECL_POKE( 408A )
				NES_DECL_PEEK( 4090 )
				NES_DECL_PEEK( 4092 )
		
				enum
				{
					REG2_WAVELENGTH_LOW      = b11111111,
					REG3_WAVELENGTH_HIGH     = b00001111,
					REG3_ENVELOPE_DISABLE    = b01000000,
					REG3_OUTPUT_DISABLE      = b10000000,
					REG5_MOD_SWEEP           = b00111111,
					REG5_MOD_NEGATE          = b01000000,
					REG6_MOD_WAVELENGTH_LOW	 = b11111111,
					REG7_MOD_WAVELENGTH_HIGH = b00001111,
					REG7_MOD_WRITE_MODE      = b10000000,
					REG8_MOD_DATA            = b00000111,
					REG9_VOLUME              = b00000011,
					REG9_WRITE_MODE          = b10000000
				};
		
				enum
				{
					STATUS_OUTPUT_ENABLED    = REG3_OUTPUT_DISABLE,
					STATUS_ENVELOPES_ENABLED = REG3_ENVELOPE_DISABLE
				};
		
				class Envelope
				{
				public:
		
					void Reset();
					void Write(uint);
		
					NST_FORCE_INLINE void Clock();

					void SaveState(State::Saver&) const;
					void LoadState(State::Loader&);
		
					enum
					{
						CTRL_COUNT   = b00111111,
						CTRL_UP      = b01000000,
						CTRL_DISABLE = b10000000
					};

				private:
				
					enum
					{
						GAIN_MAX = 0x20,
						GAIN_MIN = 0x00
					};
		
					uint ctrl;
					uint counter;
					uint gain;
		
				public:
		
					uint Gain() const
					{ 
						return gain;
					}
		
					uint Output() const
					{ 
						return NST_MIN(gain,GAIN_MAX);
					}
				};
		
				struct Modulator
				{
					enum 
					{
						TIMER_CARRY = (sizeof(dword) * CHAR_BIT) - 1,
						SIZE = 0x20
					};
		
					ibool active;
					ibool writing;
					uint pos;
					uint length;
					dword timer;
					uint sweep;
					u8 table[SIZE];

					static const u8 steps[8];
				};
		
				enum 
				{
					VOLUME,
					SWEEP
				};
		
				struct Envelopes
				{
					enum {PULSE = 8};
		
					uint counter;
					uint length;
					Envelope units[2];
				};
		
				struct Wave
				{
					enum 
					{
						DC = 0x20,
						SIZE = 0x40
					};
		
					ibool writing;
					uint length;
					dword pos;
					uint volume;
					i8 table[SIZE];
				};
		
				Cpu& cpu;
		
				Wave wave;
				Envelopes envelopes;
				Modulator modulator;
		
				uint volume;
				uint status;
		
				static const u8 volumes[4];
			};

		private:

			NES_DECL_PEEK( Nop  )
			NES_DECL_POKE( Nop  )
			NES_DECL_POKE( 4020 )
			NES_DECL_POKE( 4021 )
			NES_DECL_POKE( 4022 )
			NES_DECL_POKE( 4023 )
			NES_DECL_POKE( 4024 )
			NES_DECL_POKE( 4025 )
			NES_DECL_POKE( 4026 )
			NES_DECL_PEEK( 4030 )
			NES_DECL_PEEK( 4031 )
			NES_DECL_PEEK( 4032 )
			NES_DECL_PEEK( 4033 )

			enum
			{
				OPEN_BUS = 0x40
			};

			class Disks
			{
			public:

				enum
				{
					SIDE_SIZE = 65500U,
					EJECTED   = 0xFFF,
					MOUNTING  =	120
				};

			private:

				enum
				{
					HEADER_SIZE = 16,
					HEADER_RESERVED = HEADER_SIZE - (4 + 1)
				};

				struct Sides
				{
					void Save() const;

					u8 (*data)[SIDE_SIZE];
					uint count;
					const u8* header;
					dword crc;
				};

				static const Sides Create(StdStream);

			public:

				Disks(StdStream);
				~Disks();

				u8* data;
				uint current;
				uint mounting;
				const Sides sides;
			};

			struct Regs
			{
				void Reset();

				enum
				{
					CTRL0_DISK_ENABLED = 0x01
				};

				enum
				{
					CTRL1_MOTOR                = 0x01,
					CTRL1_TRANSFER_RESET       = 0x02,
					CTRL1_READ_MODE            = 0x04,
					CTRL1_MIRRORING_HORIZONTAL = 0x08,
					CTRL1_CRC                  = 0x10,
					CTRL1_DRIVE_READY          = 0x40,
					CTRL1_DISK_IRQ_ENABLED     = 0x80
				};

				enum
				{
					STATUS_DISK_EJECTED    = 0x01,
					STATUS_DRIVE_NOT_READY = 0x02,
					STATUS_DISK_PROTECTED  = 0x04,
					STATUS_LATCH           = 0x40
				};

				uint ctrl0;
				uint ctrl1;
				uint data;
			};

			struct Io
			{
				void Reset();

				enum
				{
					BATTERY_CHARGED = 0x80
				};

				uint pos;
				uint skip;
				uint port;
				uint led;
			};

			struct Ram
			{
				void Reset();

				NES_DECL_PEEK( Ram )
				NES_DECL_POKE( Ram )

				u8 mem[NES_32K];
			};

			struct Irq
			{
				void Reset(bool);
				ibool Signal();

				enum
				{
					CTRL_REPEAT  = 0x1,
					CTRL_ENABLED = 0x2
				};

				enum Flag
				{
					PENDING_CTRL  = 0x1,
					PENDING_DRIVE = 0x2
				};

				struct Drive
				{
					enum
					{
						FAST = 150,
						SLOW = 200
					};

					uint count;
					uint notify;
				};

				uint ctrl;
				uint count;
				uint latch;
				uint status;
				Drive drive;
			};

			struct IrqClock : Clock::M2<Irq>
			{
				IrqClock(Cpu&);

				NES_DECL_POKE( 4020 ) 
				NES_DECL_POKE( 4021 ) 
				NES_DECL_POKE( 4022 )
				NES_DECL_PEEK( 4030 )
				NES_DECL_POKE( Nop  )
				NES_DECL_PEEK( Nop  )

				void Clear(Irq::Flag);
			};

			Regs regs;
			Io io;
			Disks disks;
			IrqClock irq;
			Cpu& cpu;
			Ppu& ppu;
			Ram ram;
			Sound sound;

		public:

			Mode GetMode() const
			{
				return MODE_NTSC;
			}

			bool IsAnyDiskInserted() const
			{
				return disks.current != Disks::EJECTED;
			}

			int CurrentDisk() const
			{
				return disks.current != Disks::EJECTED ? int(disks.current / 2) : -1;
			}

			int CurrentDiskSide() const
			{
				return disks.current != Disks::EJECTED ? int(disks.current % 2) : -1;
			}

			uint NumSides() const
			{
				return disks.sides.count;
			}

			uint NumDisks() const
			{
				return (disks.sides.count / 2) + (disks.sides.count % 2);
			}

			dword GetPrgCrc() const
			{
				return disks.sides.crc;
			}

			bool HasHeader() const
			{
				return disks.sides.header != NULL;
			}
		};
	}
}

#endif
