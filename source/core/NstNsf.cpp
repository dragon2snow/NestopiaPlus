////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2007 Martin Freij
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

#include <cstring>
#include "NstLog.hpp"
#include "NstFds.hpp"
#include "NstMapper.hpp"
#include "board/NstBrdVrc6.hpp"
#include "board/NstBrdVrc7.hpp"
#include "board/NstBrdN106.hpp"
#include "board/NstBrdFme7.hpp"
#include "board/NstBrdMmc5.hpp"
#include "api/NstApiNsf.hpp"
#include "NstNsf.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		class Nsf::Chips : Apu::Channel
		{
			struct Mmc5 : Boards::Mmc5::Sound
			{
				byte exRam[SIZE_1K];

				explicit Mmc5(Cpu& c)
				: Sound(c,false) {}

				using Boards::Mmc5::Sound::Reset;
				using Boards::Mmc5::Sound::UpdateContext;
				using Boards::Mmc5::Sound::GetSample;
				using Boards::Mmc5::Sound::Clock;
			};

			struct Fds : Core::Fds::Sound
			{
				byte ram[SIZE_8K+SIZE_32K];

				explicit Fds(Cpu& c)
				: Sound(c,false) {}

				void Reset()
				{
					std::memset( ram, 0x00, sizeof(ram) );
					Sound::Reset();
				}

				using Core::Fds::Sound::UpdateContext;
				using Core::Fds::Sound::GetSample;
				using Core::Fds::Sound::Clock;
			};

			struct N106 : Boards::N106::Sound
			{
				explicit N106(Cpu& c)
				: Sound(c,false) {}

				using Boards::N106::Sound::Reset;
				using Boards::N106::Sound::UpdateContext;
				using Boards::N106::Sound::GetSample;
			};

			struct Vrc6 : Boards::Vrc6::Sound
			{
				explicit Vrc6(Cpu& c)
				: Sound(c,false) {}

				using Boards::Vrc6::Sound::Reset;
				using Boards::Vrc6::Sound::UpdateContext;
				using Boards::Vrc6::Sound::GetSample;
			};

			struct Vrc7 : Boards::Vrc7::Sound
			{
				explicit Vrc7(Cpu& c)
				: Sound(c,false) {}

				using Boards::Vrc7::Sound::Reset;
				using Boards::Vrc7::Sound::UpdateContext;
				using Boards::Vrc7::Sound::GetSample;
			};

			struct S5B : Boards::Fme7::Sound
			{
				explicit S5B(Cpu& c)
				: Sound(c,false) {}

				using Boards::Fme7::Sound::Reset;
				using Boards::Fme7::Sound::UpdateContext;
				using Boards::Fme7::Sound::GetSample;
			};

			template<typename T>
			struct Chip : Pointer<T>
			{
				Chip(Cpu& cpu,uint t)
				: Pointer<T>(t ? new T(cpu) : NULL) {}
			};

			void Reset();
			void UpdateContext(uint,const byte (&w)[MAX_CHANNELS]);
			Sample GetSample();
			Cycle Clock();

			Apu& apu;
			Cycle clock[3];

		public:

			Chips(uint,Cpu&);
			~Chips();

			Chip<Mmc5> mmc5;
			Chip<Vrc6> vrc6;
			Chip<Vrc7> vrc7;
			Chip<Fds>  fds;
			Chip<S5B>  s5b;
			Chip<N106> n106;
		};

		Nsf::Chips::Chips(const uint types,Cpu& cpu)
		:
		apu  ( cpu.GetApu() ),
		mmc5 ( cpu, types & Api::Nsf::CHIP_MMC5 ),
		vrc6 ( cpu, types & Api::Nsf::CHIP_VRC6 ),
		vrc7 ( cpu, types & Api::Nsf::CHIP_VRC7 ),
		fds  ( cpu, types & Api::Nsf::CHIP_FDS  ),
		s5b  ( cpu, types & Api::Nsf::CHIP_S5B  ),
		n106 ( cpu, types & Api::Nsf::CHIP_N106 )
		{
			if (types & Api::Nsf::CHIP_ALL)
			{
				Log log;

				if ( mmc5 ) log << "Nsf: MMC5 sound chip present" NST_LINEBREAK;
				if ( vrc6 ) log << "Nsf: VRC6 sound chip present" NST_LINEBREAK;
				if ( vrc7 ) log << "Nsf: VRC7 sound chip present" NST_LINEBREAK;
				if ( fds  ) log << "Nsf: FDS sound chip present" NST_LINEBREAK;
				if ( s5b  ) log << "Nsf: Sunsoft5B sound chip present" NST_LINEBREAK;
				if ( n106 ) log << "Nsf: N106 sound chip present" NST_LINEBREAK;
			}

			apu.HookChannel( this );
		}

		Nsf::Chips::~Chips()
		{
			apu.ReleaseChannel();
		}

		void Nsf::Chips::Reset()
		{
			clock[0] = Cpu::CYCLE_MAX;
			clock[1] = Cpu::CYCLE_MAX;
			clock[2] = Cpu::CYCLE_MAX;

			if ( mmc5 ) mmc5->Reset();
			if ( vrc6 ) vrc6->Reset();
			if ( vrc7 ) vrc7->Reset();
			if ( fds  ) fds->Reset();
			if ( s5b  ) s5b->Reset();
			if ( n106 ) n106->Reset();
		}

		void Nsf::Chips::UpdateContext(uint,const byte (&w)[MAX_CHANNELS])
		{
			clock[0] = Cpu::CYCLE_MAX;
			clock[1] = Cpu::CYCLE_MAX;
			clock[2] = Cpu::CYCLE_MAX;

			if ( mmc5 ) mmc5->SetContext( rate, fixed, mode, w );
			if ( vrc6 ) vrc6->SetContext( rate, fixed, mode, w );
			if ( vrc7 ) vrc7->SetContext( rate, fixed, mode, w );
			if ( fds  ) fds->SetContext( rate, fixed, mode, w );
			if ( s5b  ) s5b->SetContext( rate, fixed, mode, w );
			if ( n106 ) n106->SetContext( rate, fixed, mode, w );
		}

		Cycle Nsf::Chips::Clock()
		{
			if (!mmc5 && !fds)
				return 0;

			if (!fds)
				return mmc5->Clock();

			if (!mmc5)
				return fds->Clock();

			if (clock[0] == clock[2])
				clock[0] = mmc5->Clock();
			else
				clock[1] = fds->Clock();

			clock[2] = NST_MIN(clock[0],clock[1]);

			return clock[2];
		}

		Apu::Sample Nsf::Chips::GetSample()
		{
			return
			(
				(mmc5 ? mmc5->GetSample() : 0) +
				(vrc6 ? vrc6->GetSample() : 0) +
				(vrc7 ? vrc7->GetSample() : 0) +
				(fds  ? fds->GetSample()  : 0) +
				(s5b  ? s5b->GetSample()  : 0) +
				(n106 ? n106->GetSample() : 0)
			);
		}

		Nsf::Nsf(Context& context)
		:
		Image    (SOUND),
		cpu      (context.cpu),
		chips    (NULL),
		tuneMode (Api::Nsf::TUNE_MODE_NTSC)
		{
			Stream::In stream( context.stream );

			{
				byte data[5+1+2+6];
				stream.Read( data );

				if
				(
					data[0] != Ascii<'N'>::V ||
					data[1] != Ascii<'E'>::V ||
					data[2] != Ascii<'S'>::V ||
					data[3] != Ascii<'M'>::V ||
					data[4] != 0x1A
				)
					throw RESULT_ERR_INVALID_FILE;

				if (!data[6] || data[9] < 0x60 || data[11] < 0x60 || data[13] < 0x60)
					throw RESULT_ERR_CORRUPT_FILE;

				songs.count = data[6];
				songs.start = data[7] >= 1 && data[7] <= data[6] ? data[7] - 1 : 0;

				addressing.load = data[8]  | uint( data[9]  ) << 8;
				addressing.init = data[10] | uint( data[11] ) << 8;
				addressing.play = data[12] | uint( data[13] ) << 8;

				Log() << "Nsf: version " << data[5];
			}

			stream.Read( songs.info.name, 32 );
			stream.Read( songs.info.artist, 32 );
			stream.Read( songs.info.maker, 32 );

			songs.info.name[31] = '\0';
			songs.info.artist[31] = '\0';
			songs.info.maker[31] = '\0';

			if (*songs.info.name)
				Log() << NST_LINEBREAK "Nsf: name: " << songs.info.name;

			if (*songs.info.artist)
				Log() << NST_LINEBREAK "Nsf: artist: " << songs.info.artist;

			if (*songs.info.maker)
				Log() << NST_LINEBREAK "Nsf: maker: " << songs.info.maker;

			speed.ntsc = stream.Read16();
			stream.Read( banks );

			addressing.bankSwitched = 0 !=
			(
				uint( banks[0] ) |
				uint( banks[1] ) |
				uint( banks[2] ) |
				uint( banks[3] ) |
				uint( banks[4] ) |
				uint( banks[5] ) |
				uint( banks[6] ) |
				uint( banks[7] )
			);

			speed.pal = stream.Read16();
			songs.current = songs.start;

			Log() << NST_LINEBREAK "Nsf: starting song "
                  << (songs.start+1U)
                  << " of "
                  << songs.count;

			switch (stream.Read8() & 0x3)
			{
				case 0x0: tuneMode = Api::Nsf::TUNE_MODE_NTSC; Log::Flush( NST_LINEBREAK "Nsf: NTSC mode"     NST_LINEBREAK ); break;
				case 0x1: tuneMode = Api::Nsf::TUNE_MODE_PAL;  Log::Flush( NST_LINEBREAK "Nsf: PAL mode"      NST_LINEBREAK ); break;
				default:  tuneMode = Api::Nsf::TUNE_MODE_BOTH; Log::Flush( NST_LINEBREAK "Nsf: PAL/NTSC mode" NST_LINEBREAK ); break;
			}

			uint types = stream.Read8();

			if (!(types & Api::Nsf::CHIP_FDS) && addressing.load < 0x8000)
				throw RESULT_ERR_CORRUPT_FILE;

			dword length = 0;

			while (length < SIZE_4096K && stream.SafeRead8() <= 0xFF)
				++length;

			if (length <= HEADER_RESERVED_LENGTH)
				throw RESULT_ERR_CORRUPT_FILE;

			length -= HEADER_RESERVED_LENGTH;
			stream.Seek( -idword(length) );

			{
				const uint offset = addressing.load & 0xFFFU;

				prg.Source().Set( offset + length, true, false ).Fill( JAM );
				stream.Read( prg.Source().Mem() + offset, length );
			}

			Log() << "Nsf: "
                  << (length / SIZE_1K)
                  << (addressing.bankSwitched ? "k bank-switched " : "k flat ")
                  << ((types & Api::Nsf::CHIP_FDS) ? "PRG-RAM" : "PRG-ROM")
                  << NST_LINEBREAK "Nsf: load address - " << Log::Hex( 16, addressing.load )
                  << NST_LINEBREAK "Nsf: init address - " << Log::Hex( 16, addressing.init )
                  << NST_LINEBREAK "Nsf: play address - " << Log::Hex( 16, addressing.play )
                  << NST_LINEBREAK;

			if (types & Api::Nsf::CHIP_ALL)
				chips = new Chips (types,cpu);
		}

		Nsf::~Nsf()
		{
			delete chips;
		}

		void Nsf::SetMode(const Mode mode)
		{
			routine.nmi = Routine::NMI;

			cpu.SetupFrame
			(
				mode == MODE_NTSC ? Apu::FRAME_CLOCK_NTSC * 2UL * Cpu::MC_DIV_NTSC :
									Apu::FRAME_CLOCK_PAL * 2UL * Cpu::MC_DIV_PAL
			);
		}

		Mode Nsf::GetMode() const
		{
			return tuneMode == Api::Nsf::TUNE_MODE_PAL ? MODE_PAL : MODE_NTSC;
		}

		uint Nsf::GetChips() const
		{
			uint types = 0;

			if (chips)
			{
				if ( chips->vrc6 ) types |= Api::Nsf::CHIP_VRC6;
				if ( chips->vrc7 ) types |= Api::Nsf::CHIP_VRC7;
				if ( chips->fds  ) types |= Api::Nsf::CHIP_FDS;
				if ( chips->mmc5 ) types |= Api::Nsf::CHIP_MMC5;
				if ( chips->n106 ) types |= Api::Nsf::CHIP_N106;
				if ( chips->s5b  ) types |= Api::Nsf::CHIP_S5B;
			}

			return types;
		}

		void Nsf::Reset(bool)
		{
			cpu.Map( 0x38EC ).Set( this, &Nsf::Peek_38EC, &Nsf::Poke_Nop );
			cpu.Map( 0x38ED ).Set( this, &Nsf::Peek_38ED, &Nsf::Poke_Nop );
			cpu.Map( 0x38EE ).Set( this, &Nsf::Peek_38EE, &Nsf::Poke_Nop );
			cpu.Map( 0x38EF ).Set( this, &Nsf::Peek_38EF, &Nsf::Poke_Nop );
			cpu.Map( 0x38F0 ).Set( this, &Nsf::Peek_38F0, &Nsf::Poke_Nop );
			cpu.Map( 0x38F1 ).Set( this, &Nsf::Peek_38F1, &Nsf::Poke_Nop );
			cpu.Map( 0x38F2 ).Set( this, &Nsf::Peek_38F2, &Nsf::Poke_Nop );
			cpu.Map( 0x38F3 ).Set( this, &Nsf::Peek_38F3, &Nsf::Poke_Nop );
			cpu.Map( 0x38F4 ).Set( this, &Nsf::Peek_38F4, &Nsf::Poke_Nop );
			cpu.Map( 0x38F5 ).Set( this, &Nsf::Peek_38F5, &Nsf::Poke_Nop );
			cpu.Map( 0x38F6 ).Set( this, &Nsf::Peek_38F6, &Nsf::Poke_Nop );
			cpu.Map( 0x38F7 ).Set( this, &Nsf::Peek_38F7, &Nsf::Poke_Nop );
			cpu.Map( 0x38F8 ).Set( this, &Nsf::Peek_38F8, &Nsf::Poke_Nop );
			cpu.Map( 0x38F9 ).Set( this, &Nsf::Peek_38F9, &Nsf::Poke_Nop );
			cpu.Map( 0x38FA ).Set( this, &Nsf::Peek_38FA, &Nsf::Poke_Nop );
			cpu.Map( 0x38FB ).Set( this, &Nsf::Peek_38FB, &Nsf::Poke_Nop );
			cpu.Map( 0x38FC ).Set( this, &Nsf::Peek_38FC, &Nsf::Poke_Nop );
			cpu.Map( 0x38FD ).Set( this, &Nsf::Peek_38FD, &Nsf::Poke_Nop );
			cpu.Map( 0x38FE ).Set( this, &Nsf::Peek_38FE, &Nsf::Poke_Nop );
			cpu.Map( 0x38FF ).Set( this, &Nsf::Peek_38FF, &Nsf::Poke_Nop );

			cpu.Map( 0x4017 ).Set( this, &Nsf::Peek_Nop, &Nsf::Poke_4017 );

			const bool fds = chips && chips->fds;

			if (addressing.bankSwitched)
			{
				if (fds)
				{
					cpu.Map( 0x5FF6 ).Set( this, &Nsf::Peek_Nop, &Nsf::Poke_Fds_5FF6 );
					cpu.Map( 0x5FF7 ).Set( this, &Nsf::Peek_Nop, &Nsf::Poke_Fds_5FF7 );
				}

				cpu.Map( 0x5FF8 ).Set( this, &Nsf::Peek_Nop, fds ? &Nsf::Poke_Fds_5FF8 : &Nsf::Poke_5FF8 );
				cpu.Map( 0x5FF9 ).Set( this, &Nsf::Peek_Nop, fds ? &Nsf::Poke_Fds_5FF9 : &Nsf::Poke_5FF9 );
				cpu.Map( 0x5FFA ).Set( this, &Nsf::Peek_Nop, fds ? &Nsf::Poke_Fds_5FFA : &Nsf::Poke_5FFA );
				cpu.Map( 0x5FFB ).Set( this, &Nsf::Peek_Nop, fds ? &Nsf::Poke_Fds_5FFB : &Nsf::Poke_5FFB );
				cpu.Map( 0x5FFC ).Set( this, &Nsf::Peek_Nop, fds ? &Nsf::Poke_Fds_5FFC : &Nsf::Poke_5FFC );
				cpu.Map( 0x5FFD ).Set( this, &Nsf::Peek_Nop, fds ? &Nsf::Poke_Fds_5FFD : &Nsf::Poke_5FFD );
				cpu.Map( 0x5FFE ).Set( this, &Nsf::Peek_Nop, fds ? &Nsf::Poke_Fds_5FFE : &Nsf::Poke_5FFE );
				cpu.Map( 0x5FFF ).Set( this, &Nsf::Peek_Nop, fds ? &Nsf::Poke_Fds_5FFF : &Nsf::Poke_5FFF );
			}
			else if (!fds)
			{
				for (dword i=0x8000, j=0; i < 0x10000; j += (i >= (addressing.load & 0xF000U)), i += 0x1000)
					prg.SwapBank<SIZE_4K>( i-0x8000, j );
			}

			if (fds)
			{
				cpu.Map( 0x6000, 0xFFFF ).Set( this, &Nsf::Peek_Fds, &Nsf::Poke_Fds );
			}
			else
			{
				cpu.Map( 0x6000, 0x7FFF ).Set( this, &Nsf::Peek_Ram,   &Nsf::Poke_Ram );
				cpu.Map( 0x8000, 0x8FFF ).Set( this, &Nsf::Peek_Prg_8, &Nsf::Poke_Nop );
				cpu.Map( 0x9000, 0x9FFF ).Set( this, &Nsf::Peek_Prg_9, &Nsf::Poke_Nop );
				cpu.Map( 0xA000, 0xAFFF ).Set( this, &Nsf::Peek_Prg_A, &Nsf::Poke_Nop );
				cpu.Map( 0xB000, 0xBFFF ).Set( this, &Nsf::Peek_Prg_B, &Nsf::Poke_Nop );
				cpu.Map( 0xC000, 0xCFFF ).Set( this, &Nsf::Peek_Prg_C, &Nsf::Poke_Nop );
				cpu.Map( 0xD000, 0xDFFF ).Set( this, &Nsf::Peek_Prg_D, &Nsf::Poke_Nop );
				cpu.Map( 0xE000, 0xEFFF ).Set( this, &Nsf::Peek_Prg_E, &Nsf::Poke_Nop );
				cpu.Map( 0xF000, 0xFFFF ).Set( this, &Nsf::Peek_Prg_F, &Nsf::Poke_Nop );
			}

			if (chips)
			{
				if (chips->mmc5)
					cpu.Map( 0x5C00, 0x5FF5 ).Set( this, &Nsf::Peek_ExRam, &Nsf::Poke_ExRam );

				if (chips->vrc6)
				{
					cpu.Map( 0x9000 ).Set( &Nsf::Poke_Vrc6_9000 );
					cpu.Map( 0x9001 ).Set( &Nsf::Poke_Vrc6_9001 );
					cpu.Map( 0x9002 ).Set( &Nsf::Poke_Vrc6_9002 );
					cpu.Map( 0xA000 ).Set( &Nsf::Poke_Vrc6_A000 );
					cpu.Map( 0xA001 ).Set( &Nsf::Poke_Vrc6_A001 );
					cpu.Map( 0xA002 ).Set( &Nsf::Poke_Vrc6_A002 );
					cpu.Map( 0xB000 ).Set( &Nsf::Poke_Vrc6_B000 );
					cpu.Map( 0xB001 ).Set( &Nsf::Poke_Vrc6_B001 );
					cpu.Map( 0xB002 ).Set( &Nsf::Poke_Vrc6_B002 );
				}

				if (chips->vrc7)
				{
					cpu.Map( 0x9010 ).Set( &Nsf::Poke_Vrc7_9010 );
					cpu.Map( 0x9030 ).Set( &Nsf::Poke_Vrc7_9030 );
				}

				if (chips->n106)
				{
					cpu.Map( 0x4800 ).Set( this, &Nsf::Peek_N106_48, &Nsf::Poke_N106_48 );
					cpu.Map( 0xF800 ).Set( &Nsf::Poke_N106_F8 );
				}

				if (chips->s5b)
				{
					cpu.Map( 0xC000 ).Set( &Nsf::Poke_S5B_C );
					cpu.Map( 0xE000 ).Set( &Nsf::Poke_S5B_E );
				}
			}

			cpu.Map( 0xFFFA ).Set( &Nsf::Peek_FFFA );
			cpu.Map( 0xFFFB ).Set( &Nsf::Peek_FFFB );
			cpu.Map( 0xFFFC ).Set( &Nsf::Peek_FFFC );
			cpu.Map( 0xFFFD ).Set( &Nsf::Peek_FFFD );

			routine.reset = Routine::RESET;

			SetMode( cpu.GetMode() );
		}

		Result Nsf::SelectSong(const uint song)
		{
			if (song < songs.count)
			{
				if (songs.current != song)
				{
					songs.current = song;

					if (routine.playing)
					{
						routine.nmi = Routine::NMI;
						cpu.GetApu().ClearBuffers();
					}

					return RESULT_OK;
				}

				return RESULT_NOP;
			}

			return RESULT_ERR_INVALID_PARAM;
		}

		Result Nsf::PlaySong()
		{
			if (!routine.playing)
			{
				routine.nmi = Routine::NMI;
				routine.playing = true;
				return RESULT_OK;
			}

			return RESULT_NOP;
		}

		Result Nsf::StopSong()
		{
			if (routine.playing)
			{
				routine.playing = false;
				routine.nmi = Routine::NMI;
				cpu.GetApu().ClearBuffers();
				return RESULT_OK;
			}

			return RESULT_NOP;
		}

		void Nsf::InitSong()
		{
			std::memset( ram, 0x00, SIZE_8K );

			if (chips && chips->mmc5)
				std::memset( chips->mmc5->exRam, 0x00, SIZE_1K );

			const bool fds = chips && chips->fds;

			if (addressing.bankSwitched)
			{
				if (fds)
				{
					for (uint i=0; i < 2; ++i)
						cpu.Poke( 0x5FF6+i, banks[6+i] );
				}

				for (uint i=0; i < 8; ++i)
					cpu.Poke( 0x5FF8+i, banks[i] );
			}
			else if (fds)
			{
				for (dword i=0x6000, j=0; i < 0x10000; j += (i >= (addressing.load & 0xF000U)), i += 0x1000)
					std::memcpy( chips->fds->ram + (i-0x6000), prg.Source().Mem(j * 0x1000), 0x1000 );
			}

			if (fds)
			{
				cpu.Poke( 0x4089, 0x80 );
				cpu.Poke( 0x408A, 0xE8 );
			}

			cpu.GetApu().ClearBuffers();
			std::memset( cpu.SystemRam(), 0x00, Cpu::RAM_SIZE );

			for (uint i=0x4000; i <= 0x4013; ++i)
				cpu.Poke( i, 0x00 );

			cpu.Poke( 0x4015, 0x0F );
			cpu.Poke( 0x4017, 0xC0 );
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		void Nsf::BeginFrame()
		{
			routine.jmp = (routine.playing ? 0xFA : 0xFD);

			if (routine.nmi)
				cpu.DoNMI(0);
		}

		inline uint Nsf::FetchLast(uint offset) const
		{
			NST_ASSERT( offset <= 0xFFF );
			return offset[chips && chips->fds ? chips->fds->ram + (sizeof(array(chips->fds->ram))-SIZE_4K) : prg[7]];
		}

		NES_PEEK(Nsf,FFFA)
		{
			return routine.nmi ? routine.nmi &= Routine::NMI_B, routine.playing ? 0xEC : 0xFD : FetchLast(0xFFA);
		}

		NES_PEEK(Nsf,FFFB)
		{
			return routine.nmi ? routine.nmi &= Routine::NMI_A, 0x38 : FetchLast(0xFFB);
		}

		NES_PEEK(Nsf,FFFC)
		{
			return routine.reset ? routine.reset &= Routine::RESET_B, 0xFD : FetchLast(0xFFC);
		}

		NES_PEEK(Nsf,FFFD)
		{
			return routine.reset ? routine.reset &= Routine::RESET_A, 0x38 : FetchLast(0xFFD);
		}

		NES_PEEK(Nsf,38EC)
		{
			NST_VERIFY( routine.playing );

			InitSong();
			return LDA;
		}

		NES_PEEK(Nsf,38ED)
		{
			NST_VERIFY( routine.playing );
			return songs.current;
		}

		NES_PEEK(Nsf,38EE)
		{
			NST_VERIFY( routine.playing );
			return LDX;
		}

		NES_PEEK(Nsf,38EF)
		{
			NST_VERIFY( routine.playing );
			return 0xFC;
		}

		NES_PEEK(Nsf,38F0)
		{
			NST_VERIFY( routine.playing );
			return TXS;
		}

		NES_PEEK(Nsf,38F1)
		{
			NST_VERIFY( routine.playing );
			return LDX;
		}

		NES_PEEK(Nsf,38F2)
		{
			NST_VERIFY( routine.playing );
			return cpu.GetMode() == MODE_PAL;
		}

		NES_PEEK(Nsf,38F3)
		{
			NST_VERIFY( routine.playing );
			return JSR;
		}

		NES_PEEK(Nsf,38F4)
		{
			NST_VERIFY( routine.playing );
			return addressing.init & 0xFFU;
		}

		NES_PEEK(Nsf,38F5)
		{
			NST_VERIFY( routine.playing );
			return addressing.init >> 8;
		}

		NES_PEEK(Nsf,38F6)
		{
			NST_VERIFY( routine.playing );
			return SEI;
		}

		NES_PEEK(Nsf,38F7)
		{
			NST_VERIFY( routine.playing );
			routine.jmp = 0xFD;
			return JMP;
		}

		NES_PEEK(Nsf,38F8)
		{
			NST_VERIFY( routine.playing );
			return 0xFD;
		}

		NES_PEEK(Nsf,38F9)
		{
			NST_VERIFY( routine.playing );
			return 0x38;
		}

		NES_PEEK(Nsf,38FA)
		{
			NST_VERIFY( routine.playing );
			routine.jmp = 0xFD;
			return JSR;
		}

		NES_PEEK(Nsf,38FB)
		{
			NST_VERIFY( routine.playing );
			return addressing.play & 0xFFU;
		}

		NES_PEEK(Nsf,38FC)
		{
			NST_VERIFY( routine.playing );
			return addressing.play >> 8;
		}

		NES_PEEK(Nsf,38FD)
		{
			return JMP;
		}

		NES_PEEK(Nsf,38FE)
		{
			return routine.jmp;
		}

		NES_PEEK(Nsf,38FF)
		{
			return 0x38;
		}

		NES_POKE(Nsf,4017)
		{
			cpu.GetApu().Poke_4017( data );
		}

		NES_POKE(Nsf,5FF8) { prg.SwapBank<SIZE_4K,0x0000>( data ); }
		NES_POKE(Nsf,5FF9) { prg.SwapBank<SIZE_4K,0x1000>( data ); }
		NES_POKE(Nsf,5FFA) { prg.SwapBank<SIZE_4K,0x2000>( data ); }
		NES_POKE(Nsf,5FFB) { prg.SwapBank<SIZE_4K,0x3000>( data ); }
		NES_POKE(Nsf,5FFC) { prg.SwapBank<SIZE_4K,0x4000>( data ); }
		NES_POKE(Nsf,5FFD) { prg.SwapBank<SIZE_4K,0x5000>( data ); }
		NES_POKE(Nsf,5FFE) { prg.SwapBank<SIZE_4K,0x6000>( data ); }
		NES_POKE(Nsf,5FFF) { prg.SwapBank<SIZE_4K,0x7000>( data ); }

		NES_POKE(Nsf,Fds_5FF6) { std::memcpy( chips->fds->ram + (SIZE_4K*0U), prg.Source().Mem(data * dword(SIZE_4K)), SIZE_4K ); }
		NES_POKE(Nsf,Fds_5FF7) { std::memcpy( chips->fds->ram + (SIZE_4K*1U), prg.Source().Mem(data * dword(SIZE_4K)), SIZE_4K ); }
		NES_POKE(Nsf,Fds_5FF8) { std::memcpy( chips->fds->ram + (SIZE_4K*2U), prg.Source().Mem(data * dword(SIZE_4K)), SIZE_4K ); }
		NES_POKE(Nsf,Fds_5FF9) { std::memcpy( chips->fds->ram + (SIZE_4K*3U), prg.Source().Mem(data * dword(SIZE_4K)), SIZE_4K ); }
		NES_POKE(Nsf,Fds_5FFA) { std::memcpy( chips->fds->ram + (SIZE_4K*4U), prg.Source().Mem(data * dword(SIZE_4K)), SIZE_4K ); }
		NES_POKE(Nsf,Fds_5FFB) { std::memcpy( chips->fds->ram + (SIZE_4K*5U), prg.Source().Mem(data * dword(SIZE_4K)), SIZE_4K ); }
		NES_POKE(Nsf,Fds_5FFC) { std::memcpy( chips->fds->ram + (SIZE_4K*6U), prg.Source().Mem(data * dword(SIZE_4K)), SIZE_4K ); }
		NES_POKE(Nsf,Fds_5FFD) { std::memcpy( chips->fds->ram + (SIZE_4K*7U), prg.Source().Mem(data * dword(SIZE_4K)), SIZE_4K ); }
		NES_POKE(Nsf,Fds_5FFE) { std::memcpy( chips->fds->ram + (SIZE_4K*8U), prg.Source().Mem(data * dword(SIZE_4K)), SIZE_4K ); }
		NES_POKE(Nsf,Fds_5FFF) { std::memcpy( chips->fds->ram + (SIZE_4K*9U), prg.Source().Mem(data * dword(SIZE_4K)), SIZE_4K ); }

		NES_PEEK(Nsf,Prg_8) { return prg[0][address - 0x8000]; }
		NES_PEEK(Nsf,Prg_9) { return prg[1][address - 0x9000]; }
		NES_PEEK(Nsf,Prg_A) { return prg[2][address - 0xA000]; }
		NES_PEEK(Nsf,Prg_B) { return prg[3][address - 0xB000]; }
		NES_PEEK(Nsf,Prg_C) { return prg[4][address - 0xC000]; }
		NES_PEEK(Nsf,Prg_D) { return prg[5][address - 0xD000]; }
		NES_PEEK(Nsf,Prg_E) { return prg[6][address - 0xE000]; }
		NES_PEEK(Nsf,Prg_F) { return prg[7][address - 0xF000]; }

		NES_POKE(Nsf,Vrc6_9000) { chips->vrc6->WriteSquareReg0( 0, data ); }
		NES_POKE(Nsf,Vrc6_9001) { chips->vrc6->WriteSquareReg1( 0, data ); }
		NES_POKE(Nsf,Vrc6_9002) { chips->vrc6->WriteSquareReg2( 0, data ); }
		NES_POKE(Nsf,Vrc6_A000) { chips->vrc6->WriteSquareReg0( 1, data ); }
		NES_POKE(Nsf,Vrc6_A001) { chips->vrc6->WriteSquareReg1( 1, data ); }
		NES_POKE(Nsf,Vrc6_A002) { chips->vrc6->WriteSquareReg2( 1, data ); }
		NES_POKE(Nsf,Vrc6_B000) { chips->vrc6->WriteSawReg0( data );       }
		NES_POKE(Nsf,Vrc6_B001) { chips->vrc6->WriteSawReg1( data );       }
		NES_POKE(Nsf,Vrc6_B002) { chips->vrc6->WriteSawReg2( data );       }

		NES_POKE(Nsf,Vrc7_9010) { chips->vrc7->WriteReg0( data ); }
		NES_POKE(Nsf,Vrc7_9030) { chips->vrc7->WriteReg1( data ); }

		NES_POKE(Nsf,S5B_C) { chips->s5b->Poke_C000( data ); }
		NES_POKE(Nsf,S5B_E) { chips->s5b->Poke_E000( data ); }

		NES_PEEK(Nsf,N106_48) { return chips->n106->Peek_4800(); }
		NES_POKE(Nsf,N106_48) { chips->n106->Poke_4800( data );  }
		NES_POKE(Nsf,N106_F8) { chips->n106->Poke_F800( data );  }

		NES_PEEK(Nsf,Ram) { return ram[address - 0x6000]; }
		NES_POKE(Nsf,Ram) { ram[address - 0x6000] = data; }

		NES_PEEK(Nsf,ExRam) { return chips->mmc5->exRam[address - 0x5C00]; }
		NES_POKE(Nsf,ExRam) { chips->mmc5->exRam[address - 0x5C00] = data; }

		NES_PEEK(Nsf,Fds) { return chips->fds->ram[address - 0x6000]; }
		NES_POKE(Nsf,Fds) { chips->fds->ram[address - 0x6000] = data; }

		NES_PEEK(Nsf,Nop) { return address >> 8; }
		NES_POKE(Nsf,Nop) {}
	}
}
