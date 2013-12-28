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

#ifndef NST_FDS_H
#define NST_FDS_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include "NstImage.hpp"
#include "NstCpu.hpp"
#include "NstClock.hpp"
#include "NstChecksumMd5.hpp"
#include "api/NstApiFds.hpp"

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
			uint GetDesiredController(uint) const;
			uint GetDesiredAdapter() const;

			Result GetDiskData(uint,Api::Fds::DiskData&);

			void LoadState(State::Loader&);
			void SaveState(State::Saver&) const;

			class Bios
			{
				friend class Fds;

				struct Instance
				{
					Instance();

					NES_DECL_PEEK( Rom )
					NES_DECL_POKE( Nop )

					u8 rom[SIZE_8K];
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
				Sample GetSample();
				Cycle Clock();

			private:

				bool CanOutput() const;
				inline bool CanModulate() const;

				void ResetChannel();

				NST_FORCE_INLINE uint GetModulation() const;

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
					REG6_MOD_WAVELENGTH_LOW  = b11111111,
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
						SIZE = 0x40
					};

					ibool writing;
					uint length;
					dword pos;
					uint volume;
					u8 table[SIZE];
				};

				Cpu& cpu;

				Wave wave;
				Envelopes envelopes;
				Modulator modulator;

				uint volume;
				dword amp;
				uint status;
				Apu::DcBlocker dcBlocker;

				static const u8 volumes[4];

				const ibool hooked;
			};

		private:

			Result Flush(bool,bool) const;

			NES_DECL_PEEK( Nop  )
			NES_DECL_POKE( Nop  )
			NES_DECL_POKE( 4023 )
			NES_DECL_POKE( 4025 )
			NES_DECL_POKE( 4026 )
			NES_DECL_PEEK( 4031 )
			NES_DECL_PEEK( 4033 )

			enum
			{
				SIDE_SIZE            = 65500U,
				MAX_SIDE_SIZE        = 68000U,
				CTRL1_NMT_HORIZONTAL = 0x08,
				OPEN_BUS             = 0x40
			};

			class Disks
			{
				struct Sides
				{
					void Save() const;

					u8 (*data)[SIDE_SIZE];
					uint count;
					const u8* header;
					dword id;
					dword crc;
					Checksum::Md5::Key checksum;
				};

				enum
				{
					HEADER_SIZE = 16,
					HEADER_RESERVED = HEADER_SIZE - (4 + 1)
				};

				static const Sides Create(StdStream);

			public:

				Disks(StdStream);
				~Disks();

				enum
				{
					EJECTED  = 0xFFF,
					MOUNTING = 180
				};

				uint current;
				uint mounting;
				ibool writeProtected;
				const Sides sides;
			};

			struct Ram
			{
				void Reset();

				NES_DECL_PEEK( Ram )
				NES_DECL_POKE( Ram )

				u8 mem[SIZE_32K];
			};

			struct Unit
			{
				Unit();

				void Reset(bool);
				ibool Signal();

				enum
				{
					STATUS_PENDING_IRQ = 0x1,
					STATUS_TRANSFERED  = 0x2
				};

				struct Timer
				{
					Timer();

					void Reset();
					void Advance(uint&);

					inline ibool Clock();

					enum
					{
						CTRL_REPEAT  = 0x1,
						CTRL_ENABLED = 0x2
					};

					uint ctrl;
					uint count;
					uint latch;
				};

				struct Drive
				{
					Drive();

					static Result Analyze(const u8*,Api::Fds::DiskData&);

					void  Reset();
					void  Mount(u8*,ibool);
					ibool Advance(uint&);

					inline ibool Clock();

					NST_FORCE_INLINE void Write(uint);

					enum
					{
						CLK_HEAD = 96400U,

						BYTES_GAP_INIT = CLK_HEAD/8UL * 398 / 1000,
						BYTES_GAP_NEXT = CLK_HEAD/8UL * 10  / 1000,

						CLK_BYTE = dword(Cpu::MC_NTSC) / (CLK_HEAD/8UL * Cpu::CLK_NTSC_DIV * Cpu::MC_DIV_NTSC),

						CLK_MOTOR  = CLK_HEAD/8UL * 100 * CLK_BYTE / 1000,
						CLK_REWIND = CLK_HEAD/8UL * 135 * CLK_BYTE / 1000,

						CTRL_ON        = 0x01,
						CTRL_STOP      = 0x02,
						CTRL_READ_MODE = 0x04,
						CTRL_CRC       = 0x10,
						CTRL_IO_MODE   = 0x40,
						CTRL_GEN_IRQ   = 0x80,

						STATUS_EJECTED   = 0x01,
						STATUS_UNREADY   = 0x02,
						STATUS_PROTECTED = 0x04,

						BLOCK_VOLUME = 1,
						BLOCK_COUNT,
						BLOCK_HEADER,
						BLOCK_DATA,

						LENGTH_HEADER  = 15,
						LENGTH_VOLUME  = 55,
						LENGTH_COUNT   = 1,
						LENGTH_UNKNOWN = 0xFFFFU
					};

					dword count;
					dword headPos;
					uint dataPos;
					uint gap;
					u8* io;
					uint ctrl;
					uint length;
					uint in;
					uint out;
					uint status;
					mutable ibool dirty;
				};

				Timer timer;
				Drive drive;
				uint status;
			};

			class Adapter : Clock::M2<Unit>
			{
				NES_DECL_PEEK( Nop  )
				NES_DECL_POKE( Nop  )
				NES_DECL_POKE( 4020 )
				NES_DECL_POKE( 4021 )
				NES_DECL_POKE( 4022 )
				NES_DECL_POKE( 4024 )
				NES_DECL_PEEK( 4030 )
				NES_DECL_PEEK( 4032 )

			public:

				Adapter(Cpu&);

				void Reset(u8*,ibool=false);

				NST_FORCE_INLINE void Write(uint);
				NST_FORCE_INLINE uint Read();

				void LoadState(State::Loader&,dword);
				void SaveState(State::Saver&) const;

				inline void  Mount(u8*,ibool=false);
				inline void  WriteProtect();
				inline ibool Dirty() const;
				inline uint  Activity() const;

				using Clock::M2<Unit>::VSync;
			};

			struct Io
			{
				Io();

				void Reset();

				enum
				{
					CTRL0_DISK_ENABLED = 0x01,
					BATTERY_CHARGED    = 0x80
				};

				uint ctrl;
				uint port;
				mutable uint led;
			};

			Disks disks;
			Adapter adapter;
			Io io;
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

			bool CanChangeDiskSide() const
			{
				return disks.current != Disks::EJECTED && (disks.current % 2 || disks.current+1 < disks.sides.count);
			}

			bool HasHeader() const
			{
				return disks.sides.header != NULL;
			}
		};
	}
}

#endif
