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

#ifndef NST_NSF_H
#define NST_NSF_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include "NstChip.hpp"
#include "NstImage.hpp"

namespace Nes
{
	namespace Core
	{
		class Cpu;
		class Apu;

		class Nsf : public Image
		{
		public:

			Nsf(Context&);
			~Nsf();

			void Reset(bool);
			Result SelectSong(uint);
			Result PlaySong();
			Result StopSong();

			enum TuneMode
			{
				TUNE_MODE_NTSC,
				TUNE_MODE_PAL,
				TUNE_MODE_BOTH
			};

			enum Chip
			{
				CHIP_VRC6  = 0x01,
				CHIP_VRC7  = 0x02,
				CHIP_FDS   = 0x04,
				CHIP_MMC5  = 0x08,
				CHIP_N106  = 0x10,
				CHIP_FME07 = 0x20
			};

			void SetMode(Mode);
			Mode GetMode() const;
			void BeginFrame(Input::Controllers*);
			uint GetChips() const;

		private:

			void InitSong();

			typedef Memory<NES_32K,NES_4K> Prg;

			enum 
			{
				JAM	= 0x02,
				JMP = 0x4C,
				JSR = 0x20,
				LDA = 0xA9,
				LDX = 0xA2
			};

			NES_DECL_PEEK( 38F0 )
			NES_DECL_PEEK( 38F1 )
			NES_DECL_PEEK( 38F2 )
			NES_DECL_PEEK( 38F3 )
			NES_DECL_PEEK( 38F4 )
			NES_DECL_PEEK( 38F5 )
			NES_DECL_PEEK( 38F6 )
			NES_DECL_PEEK( 38F7 )
			NES_DECL_PEEK( 38F8 )
			NES_DECL_PEEK( 38F9 )
			NES_DECL_PEEK( 38FA )
			NES_DECL_PEEK( 38FB )
			NES_DECL_PEEK( 38FC )
			NES_DECL_PEEK( 38FD )
			NES_DECL_PEEK( 38FE )
			NES_DECL_PEEK( 38FF )

			NES_DECL_POKE( 4017 )

			NES_DECL_POKE( 5FF6 )
			NES_DECL_POKE( 5FF7 )
			NES_DECL_POKE( 5FF8 )
			NES_DECL_POKE( 5FF9 )
			NES_DECL_POKE( 5FFA )
			NES_DECL_POKE( 5FFB )
			NES_DECL_POKE( 5FFC )
			NES_DECL_POKE( 5FFD )
			NES_DECL_POKE( 5FFE )
			NES_DECL_POKE( 5FFF )

			NES_DECL_PEEK( Ram   )
			NES_DECL_POKE( Ram   )
			NES_DECL_PEEK( ExRam )
			NES_DECL_POKE( ExRam )

			NES_DECL_PEEK( Prg_6 )
			NES_DECL_PEEK( Prg_7 )
			NES_DECL_PEEK( Prg_8 )
			NES_DECL_PEEK( Prg_9 )
			NES_DECL_PEEK( Prg_A )
			NES_DECL_PEEK( Prg_B )
			NES_DECL_PEEK( Prg_C )
			NES_DECL_PEEK( Prg_D )
			NES_DECL_PEEK( Prg_E )
			NES_DECL_PEEK( Prg_F )

			NES_DECL_POKE( Prg_6 )
			NES_DECL_POKE( Prg_7 )
			NES_DECL_POKE( Prg_8 )
			NES_DECL_POKE( Prg_9 )
			NES_DECL_POKE( Prg_A )
			NES_DECL_POKE( Prg_B )
			NES_DECL_POKE( Prg_C )
			NES_DECL_POKE( Prg_D )
			NES_DECL_POKE( Prg_E )
			NES_DECL_POKE( Prg_F )

			NES_DECL_POKE( Vrc6_9000 )
			NES_DECL_POKE( Vrc6_9001 )
			NES_DECL_POKE( Vrc6_9002 )
			NES_DECL_POKE( Vrc6_A000 )
			NES_DECL_POKE( Vrc6_A001 )
			NES_DECL_POKE( Vrc6_A002 )
			NES_DECL_POKE( Vrc6_B000 )
			NES_DECL_POKE( Vrc6_B001 )
			NES_DECL_POKE( Vrc6_B002 )

			NES_DECL_PEEK( N106_48 )
			NES_DECL_POKE( N106_48 )
			NES_DECL_POKE( N106_F8 )

			NES_DECL_POKE( Fme07_C )
			NES_DECL_POKE( Fme07_E )

			NES_DECL_PEEK( FFFA )
			NES_DECL_PEEK( FFFB )
			NES_DECL_PEEK( FFFC )
			NES_DECL_PEEK( FFFD )

			NES_DECL_PEEK( Nop )
			NES_DECL_POKE( Nop )

			enum Header
			{
				HEADER_SIZE = 128,
				HEADER_RESERVED_LENGTH = 4
			};

			struct Chips;

			struct Info
			{
				char name[32];
				char artist[32];
				char maker[32];
			};

			struct Songs
			{
				uchar start;
				uchar current;
				uchar count;
				Info info;

				Songs()
				: 
				start   (0),
				current (0),
				count   (0)
				{}
			};

			struct Speed
			{
				ushort ntsc;
				ushort pal;

				Speed()
				:
				ntsc (0),
				pal  (0)
				{}
			};

			struct Addressing
			{
				ushort play;
				ushort init;
				ushort load;
				bool bankSwitched;

				Addressing()
				:
				play         (0x0000),
				init         (0x0000),
				load         (0x0000),
				bankSwitched (false)
				{}
			};

			struct Routine
			{
				enum
				{
					RESET_A = 0x1,
					RESET_B = 0x2,
					RESET   = RESET_A|RESET_B,
					NMI_A   = 0x1,
					NMI_B   = 0x2,
					NMI     = NMI_A|NMI_B
				};

				bool playing;
				uchar nmi;
				uchar reset;
				uchar jmp;

				Routine()
				:
				playing (false),
				nmi     (0),
				reset   (0),
				jmp     (0x00)
				{}
			};

			Prg        prg;
			Routine    routine;
			Cpu&       cpu;
			Chips*     chips;
			Songs      songs;
			Addressing addressing;
			Speed      speed;
			TuneMode   tuneMode;
			u8         banks[8];
			u8         ram[NES_8K];

		public:

			cstring GetName() const 
			{ 
				return songs.info.name; 
			}

			cstring GetArtist() const
			{
				return songs.info.artist; 
			}

			cstring GetMaker() const 
			{
				return songs.info.maker; 
			}

			TuneMode GetTuneMode() const
			{
				return tuneMode;
			}

			uint NumSongs() const 
			{ 
				return songs.count; 
			}

			uint CurrentSong() const 
			{ 
				return songs.current; 
			}

			uint StartingSong() const
			{
				return songs.start;
			}

			bool UsesBankSwitching() const
			{
				return addressing.bankSwitched;
			}

			bool IsPlaying() const 
			{ 
				return routine.playing; 
			}

			uint GetInitAddress() const
			{
				return addressing.init;
			}

			uint GetLoadAddress() const
			{
				return addressing.load;
			}

			uint GetPlayAddress() const
			{
				return addressing.play;
			}
		};
	}
}

#endif
