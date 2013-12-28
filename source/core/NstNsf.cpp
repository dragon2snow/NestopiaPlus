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

#include <cstring>
#include "NstStream.hpp"
#include "NstLog.hpp"
#include "NstFds.hpp"
#include "NstMapper.hpp"
#include "board/NstBrdVrc6.hpp"
#include "board/NstBrdN106.hpp"
#include "board/NstBrdFme7.hpp"
#include "board/NstBrdMmc5.hpp"
#include "NstNsf.hpp"

namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif

		struct Nsf::Chips
		{
			Chips(uint,Cpu&,Prg&);
			~Chips();

			struct Mmc5 : Boards::Mmc5::Sound
			{
				u8 exRam[NES_1K];

				Mmc5(Cpu& c)
				: Sound(c) {}
			};

			struct Fds : Core::Fds::Sound
			{
				Memory<NES_8K,NES_4K> prl;

				Fds(Cpu& c,Prg& p)
				: Sound(c), prl( p.Source().Mem(), p.Source().Size(), true, true )
				{
					p.Source().WriteEnable( true );
				}
			};

			typedef Boards::N106::Sound N106;
			typedef Boards::Vrc6::Sound Vrc6;
			typedef Boards::Fme07::Sound Fme07;

			Mmc5* const mmc5;
			Vrc6* const vrc6;
			Fds* const fds;
			Fme07* const fme07;
			N106* const n106;
		};

		Nsf::Chips::Chips(const uint types,Cpu& cpu,Prg& prg)
		:
		mmc5  ( ( types & CHIP_MMC5  ) ? new Mmc5  (cpu)     : NULL ),
		vrc6  ( ( types & CHIP_VRC6  ) ? new Vrc6  (cpu)     : NULL ),
		fds   ( ( types & CHIP_FDS   ) ? new Fds   (cpu,prg) : NULL ),
		fme07 ( ( types & CHIP_FME07 ) ? new Fme07 (cpu)     : NULL ),
		n106  ( ( types & CHIP_N106  ) ? new N106  (cpu)     : NULL )
		{
			Log log;

			if (types & (CHIP_MMC5|CHIP_VRC6|CHIP_FDS|CHIP_FME07|CHIP_N106))
			{
				if ( mmc5  ) log << "Nsf: MMC5 sound chip present" NST_LINEBREAK;
				if ( vrc6  ) log << "Nsf: VRC6 sound chip present" NST_LINEBREAK;
				if ( fds   ) log << "Nsf: FDS sound chip present" NST_LINEBREAK;
				if ( fme07 ) log << "Nsf: FME-07 sound chip present" NST_LINEBREAK;
				if ( n106  ) log << "Nsf: N106 sound chip present" NST_LINEBREAK;
			}
		}

		Nsf::Chips::~Chips()
		{
			delete n106;
			delete fme07;
			delete fds;
			delete vrc6;
			delete mmc5;
		}
	
		Nsf::Nsf(Context& context)
		: 
		Image    (SOUND),
		cpu      (context.cpu),
		chips    (NULL),
		tuneMode (TUNE_MODE_NTSC)
		{
			Stream::In stream( context.stream );
	
			{
				u8 data[5+1+2+6];
				stream.Read( data );

				if (data[0] != 0x4E || data[1] != 0x45 || data[2] != 0x53 || data[3] != 0x4D || data[4] != 0x1A)
					throw RESULT_ERR_INVALID_FILE;

				if (!data[6] || data[9] < 0x60 || data[11] < 0x60 || data[13] < 0x60)
					throw RESULT_ERR_CORRUPT_FILE;

				songs.count = data[6];
				songs.start = data[7] >= 1 && data[7] <= data[6] ? data[7] - 1 : 0;

				addressing.load = data[8]  | (data[9] << 8);
				addressing.init = data[10] | (data[11] << 8);
				addressing.play = data[12] | (data[13] << 8);

				Log() << "Nsf: version " << data[5];
			}
	
			{
				char* const strings[3] = 
				{
					songs.info.name,
					songs.info.artist,
					songs.info.maker
				};
	
				for (uint i=0; i < 3; ++i)
				{
					stream.Read( strings[i], 32 );
					strings[i][31] = '\0';
				}
	
				Log() << NST_LINEBREAK "Nsf: name: "   << strings[0]
					  << NST_LINEBREAK "Nsf: artist: " << strings[1]
					  << NST_LINEBREAK "Nsf: maker: "  << strings[2]
					  << NST_LINEBREAK;
			}
	
			speed.ntsc = stream.Read16();	
			stream.Read( banks );

			addressing.bankSwitched = (banks[0] | banks[1] | banks[2] | banks[3] | banks[4] | banks[5] | banks[6] | banks[7]) != 0;

			speed.pal = stream.Read16();
			songs.current = songs.start;
	
			Log() << "Nsf: starting song " 
				  << (songs.start+1)
				  << " of " 
				  << songs.count;
	
			cstring string;
	
			switch (stream.Read8() & 0x3)
			{
         		case 0x0: tuneMode = TUNE_MODE_NTSC; string = NST_LINEBREAK "Nsf: NTSC mode" NST_LINEBREAK; break;
				case 0x1: tuneMode = TUNE_MODE_PAL;  string = NST_LINEBREAK "Nsf: PAL mode" NST_LINEBREAK; break;
				default:  tuneMode = TUNE_MODE_BOTH; string = NST_LINEBREAK "Nsf: PAL/NTSC mode" NST_LINEBREAK; break;
			}
	
			Log() << string;
	
			uint type = stream.Read8();
	
			if (!(type & CHIP_FDS) && addressing.load < 0x8000U)
				throw RESULT_ERR_CORRUPT_FILE;

			stream.Seek( HEADER_RESERVED_LENGTH );
			ulong length = stream.Length();
	
			if (length > NES_1024K) // unrealistic
			{
				length = NES_1024K;
			}
			else if (!length)
			{
				throw RESULT_ERR_CORRUPT_FILE;
			}
	
			{
				const uint offset = addressing.load & 0xFFF;
	
				prg.Source().Set( offset + length, true, false ).Fill( JAM );
				stream.Read( prg.Source().Mem() + offset, length );
			}
	
			if (type & (CHIP_VRC6|CHIP_FDS|CHIP_MMC5|CHIP_N106|CHIP_FME07))
			{
				// TODO: add support for more than one sound chip at the same time

				     if (type & CHIP_VRC6  ) type &= CHIP_VRC6;
				else if (type & CHIP_FDS   ) type &= CHIP_FDS;
				else if (type & CHIP_MMC5  ) type &= CHIP_MMC5;
				else if (type & CHIP_N106  ) type &= CHIP_N106;
				else if (type & CHIP_FME07 ) type &= CHIP_FME07;

				chips = new Chips (type,cpu,prg);
			}
			else
			{
				Log() << "Nsf: no external sound chip present" NST_LINEBREAK;
			}

			Log() << "Nsf: " << (length / NES_1K) << (addressing.bankSwitched ? "k bank switched PRG-MEM" : "k flat PRG-MEM")
				  << NST_LINEBREAK "Nsf: load address - " << Log::Hex( (u16) addressing.load )
				  << NST_LINEBREAK "Nsf: init address - " << Log::Hex( (u16) addressing.init )
				  << NST_LINEBREAK "Nsf: play address - " << Log::Hex( (u16) addressing.play ) 
				  << NST_LINEBREAK; 
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
		    	mode == MODE_NTSC ? Cycle(Apu::FRAME_CLOCK_NTSC) * 2 * Cpu::MC_DIV_NTSC : 
			                        Cycle(Apu::FRAME_CLOCK_PAL) * 2 * Cpu::MC_DIV_PAL
			);
		}
	
		Mode Nsf::GetMode() const 
		{
			return tuneMode == TUNE_MODE_PAL ? MODE_PAL : MODE_NTSC;
		}
	
		uint Nsf::GetChips() const
		{
			uint type = 0;

			if (chips)
			{
				if ( chips->vrc6  ) type |= CHIP_VRC6;
				if ( chips->fds   ) type |= CHIP_FDS;
				if ( chips->mmc5  ) type |= CHIP_MMC5;
				if ( chips->n106  ) type |= CHIP_N106;
				if ( chips->fme07 ) type |= CHIP_FME07;
			}								

			return type;
		}
	
		Result Nsf::Reset(bool,bool)
		{	
			cpu.Map( 0x38F0U ).Set( this, &Nsf::Peek_38F0, &Nsf::Poke_Nop );
			cpu.Map( 0x38F1U ).Set( this, &Nsf::Peek_38F1, &Nsf::Poke_Nop );
			cpu.Map( 0x38F2U ).Set( this, &Nsf::Peek_38F2, &Nsf::Poke_Nop );
			cpu.Map( 0x38F3U ).Set( this, &Nsf::Peek_38F3, &Nsf::Poke_Nop );
			cpu.Map( 0x38F4U ).Set( this, &Nsf::Peek_38F4, &Nsf::Poke_Nop );
			cpu.Map( 0x38F5U ).Set( this, &Nsf::Peek_38F5, &Nsf::Poke_Nop );
			cpu.Map( 0x38F6U ).Set( this, &Nsf::Peek_38F6, &Nsf::Poke_Nop );
			cpu.Map( 0x38F7U ).Set( this, &Nsf::Peek_38F7, &Nsf::Poke_Nop );
			cpu.Map( 0x38F8U ).Set( this, &Nsf::Peek_38F8, &Nsf::Poke_Nop );
			cpu.Map( 0x38F9U ).Set( this, &Nsf::Peek_38F9, &Nsf::Poke_Nop );
			cpu.Map( 0x38FAU ).Set( this, &Nsf::Peek_38FA, &Nsf::Poke_Nop );
			cpu.Map( 0x38FBU ).Set( this, &Nsf::Peek_38FB, &Nsf::Poke_Nop );
			cpu.Map( 0x38FCU ).Set( this, &Nsf::Peek_38FC, &Nsf::Poke_Nop );
			cpu.Map( 0x38FDU ).Set( this, &Nsf::Peek_38FD, &Nsf::Poke_Nop );
			cpu.Map( 0x38FEU ).Set( this, &Nsf::Peek_38FE, &Nsf::Poke_Nop );
			cpu.Map( 0x38FFU ).Set( this, &Nsf::Peek_38FF, &Nsf::Poke_Nop );

			cpu.Map( 0x4017U ).Set( this, &Nsf::Peek_Nop, &Nsf::Poke_4017 );

			if (addressing.bankSwitched)
			{
				if (chips && chips->fds)
				{
					cpu.Map( 0x5FF6U ).Set( this, &Nsf::Peek_Nop, &Nsf::Poke_5FF6 );
					cpu.Map( 0x5FF7U ).Set( this, &Nsf::Peek_Nop, &Nsf::Poke_5FF7 );
				}

				cpu.Map( 0x5FF8U ).Set( this, &Nsf::Peek_Nop, &Nsf::Poke_5FF8 );
				cpu.Map( 0x5FF9U ).Set( this, &Nsf::Peek_Nop, &Nsf::Poke_5FF9 );
				cpu.Map( 0x5FFAU ).Set( this, &Nsf::Peek_Nop, &Nsf::Poke_5FFA );
				cpu.Map( 0x5FFBU ).Set( this, &Nsf::Peek_Nop, &Nsf::Poke_5FFB );
				cpu.Map( 0x5FFCU ).Set( this, &Nsf::Peek_Nop, &Nsf::Poke_5FFC );
				cpu.Map( 0x5FFDU ).Set( this, &Nsf::Peek_Nop, &Nsf::Poke_5FFD );
				cpu.Map( 0x5FFEU ).Set( this, &Nsf::Peek_Nop, &Nsf::Poke_5FFE );
				cpu.Map( 0x5FFFU ).Set( this, &Nsf::Peek_Nop, &Nsf::Poke_5FFF );
			}
			else for (dword i=0x6000U, j=0; i < 0x10000UL; j += (i >= (addressing.load & 0xF000U)), i += 0x1000U)
			{
				if (i < 0x8000U)
				{
					if (chips && chips->fds)
						chips->fds->prl.SwapBank<NES_4K>( i-0x6000U, j );
				}
				else
				{
					prg.SwapBank<NES_4K>( i-0x8000U, j );
				}
			}

			const bool fds = chips && chips->fds;

			cpu.Map( 0x6000U, 0x6FFFU ).Set( this, fds ? &Nsf::Peek_Prg_6 : &Nsf::Peek_Ram, fds ? &Nsf::Poke_Prg_6 : &Nsf::Poke_Ram );
			cpu.Map( 0x7000U, 0x7FFFU ).Set( this, fds ? &Nsf::Peek_Prg_7 : &Nsf::Peek_Ram, fds ? &Nsf::Poke_Prg_7 : &Nsf::Poke_Ram );
			
			cpu.Map( 0x8000U, 0x8FFFU ).Set( this, &Nsf::Peek_Prg_8, fds ? &Nsf::Poke_Prg_8 : &Nsf::Poke_Nop );
			cpu.Map( 0x9000U, 0x9FFFU ).Set( this, &Nsf::Peek_Prg_9, fds ? &Nsf::Poke_Prg_9 : &Nsf::Poke_Nop );
			cpu.Map( 0xA000U, 0xAFFFU ).Set( this, &Nsf::Peek_Prg_A, fds ? &Nsf::Poke_Prg_A : &Nsf::Poke_Nop );
			cpu.Map( 0xB000U, 0xBFFFU ).Set( this, &Nsf::Peek_Prg_B, fds ? &Nsf::Poke_Prg_B : &Nsf::Poke_Nop );
			cpu.Map( 0xC000U, 0xCFFFU ).Set( this, &Nsf::Peek_Prg_C, fds ? &Nsf::Poke_Prg_C : &Nsf::Poke_Nop );
			cpu.Map( 0xD000U, 0xDFFFU ).Set( this, &Nsf::Peek_Prg_D, fds ? &Nsf::Poke_Prg_D : &Nsf::Poke_Nop );
			cpu.Map( 0xE000U, 0xEFFFU ).Set( this, &Nsf::Peek_Prg_E, fds ? &Nsf::Poke_Prg_E : &Nsf::Poke_Nop );
			cpu.Map( 0xF000U, 0xFFFFU ).Set( this, &Nsf::Peek_Prg_F, fds ? &Nsf::Poke_Prg_F : &Nsf::Poke_Nop );

			if (chips && chips->mmc5)
				cpu.Map( 0x5C00U, 0x5FF5U ).Set( this, &Nsf::Peek_ExRam, &Nsf::Poke_ExRam );

			if (chips && chips->vrc6)
			{
				cpu.Map( 0x9000U ).Set( &Nsf::Poke_Vrc6_9000 );
				cpu.Map( 0x9001U ).Set( &Nsf::Poke_Vrc6_9001 );
				cpu.Map( 0x9002U ).Set( &Nsf::Poke_Vrc6_9002 );
				cpu.Map( 0xA000U ).Set( &Nsf::Poke_Vrc6_A000 );
				cpu.Map( 0xA001U ).Set( &Nsf::Poke_Vrc6_A001 );
				cpu.Map( 0xA002U ).Set( &Nsf::Poke_Vrc6_A002 );
				cpu.Map( 0xB000U ).Set( &Nsf::Poke_Vrc6_B000 );
				cpu.Map( 0xB001U ).Set( &Nsf::Poke_Vrc6_B001 );
				cpu.Map( 0xB002U ).Set( &Nsf::Poke_Vrc6_B002 );
			}

			if (chips && chips->n106)
			{
				cpu.Map( 0x4800U ).Set( this, &Nsf::Peek_N106_48, &Nsf::Poke_N106_48 );
				cpu.Map( 0xF800U ).Set( &Nsf::Poke_N106_F8 );
			}

			if (chips && chips->fme07)
			{
				cpu.Map( 0xC000U ).Set( &Nsf::Poke_Fme07_C );
				cpu.Map( 0xE000U ).Set( &Nsf::Poke_Fme07_E );
			}

			cpu.Map( 0xFFFAU ).Set( &Nsf::Peek_FFFA );
			cpu.Map( 0xFFFBU ).Set( &Nsf::Peek_FFFB );
			cpu.Map( 0xFFFCU ).Set( &Nsf::Peek_FFFC );
			cpu.Map( 0xFFFDU ).Set( &Nsf::Peek_FFFD );

			routine.reset = Routine::RESET;

			SetMode( cpu.GetMode() );

			return RESULT_OK;
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
			std::memset( ram, 0x00, NES_8K );
	
			if (chips && chips->mmc5)
				std::memset( chips->mmc5->exRam, 0x00, NES_1K );

			if (addressing.bankSwitched)
			{
				prg.SwapBank<NES_4K,0x0000U>( banks[0] );
				prg.SwapBank<NES_4K,0x1000U>( banks[1] );
				prg.SwapBank<NES_4K,0x2000U>( banks[2] );
				prg.SwapBank<NES_4K,0x3000U>( banks[3] );
				prg.SwapBank<NES_4K,0x4000U>( banks[4] );
				prg.SwapBank<NES_4K,0x5000U>( banks[5] );
				prg.SwapBank<NES_4K,0x6000U>( banks[6] );
				prg.SwapBank<NES_4K,0x7000U>( banks[7] );

				if (chips && chips->fds)
				{
					chips->fds->prl.SwapBank<NES_4K,0x0000U>( banks[6] );
					chips->fds->prl.SwapBank<NES_4K,0x1000U>( banks[7] );
				}
			}
	
			if (chips && chips->fds)
			{
				cpu.Poke( 0x4089, 0x80 );
				cpu.Poke( 0x408A, 0xE8 );
			}

			cpu.GetApu().ClearBuffers();
			cpu.ClearSystemRam();
	
			for (uint i=0x4000; i <= 0x4013; ++i)
				cpu.Poke( i, 0x00 );

			cpu.Poke( 0x4015, 0x0F );
			cpu.Poke( 0x4017, 0xC0 );
		}

        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif

		void Nsf::BeginFrame(Input::Controllers*)
		{
			routine.jmp = (routine.playing ? 0xFA : 0xFD);

			if (routine.nmi)
				cpu.DoNMI(0);
		}

		NES_PEEK(Nsf,FFFA)
		{
			return routine.nmi ? routine.nmi &= Routine::NMI_B, routine.playing ? 0xF0 : 0xFD : prg[7][0xFFA];
		}

		NES_PEEK(Nsf,FFFB)
		{
			return routine.nmi ? routine.nmi &= Routine::NMI_A, 0x38 : prg[7][0xFFB];
		}

		NES_PEEK(Nsf,FFFC)
		{
			return routine.reset ? routine.reset &= Routine::RESET_B, 0xFD : prg[7][0xFFC];
		}

		NES_PEEK(Nsf,FFFD)
		{
			return routine.reset ? routine.reset &= Routine::RESET_A, 0x38 : prg[7][0xFFD];
		}

		NES_PEEK(Nsf,38F0) 
		{ 
			NST_VERIFY( routine.playing );

			InitSong();
			return LDA;
		}

		NES_PEEK(Nsf,38F1) 
		{ 
			NST_VERIFY( routine.playing );
			return songs.current;
		}

		NES_PEEK(Nsf,38F2) 
		{ 
			NST_VERIFY( routine.playing );
			return LDX;
		}

		NES_PEEK(Nsf,38F3) 
		{ 
			NST_VERIFY( routine.playing );
			return cpu.GetMode() == MODE_PAL;
		}

		NES_PEEK(Nsf,38F4) 
		{ 
			NST_VERIFY( routine.playing );
			return JSR;
		}

		NES_PEEK(Nsf,38F5) 
		{ 
			NST_VERIFY( routine.playing );
			return addressing.init & 0xFF;
		}

		NES_PEEK(Nsf,38F6) 
		{ 
			NST_VERIFY( routine.playing );
			return addressing.init >> 8;
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
			return addressing.play & 0xFF; 
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

		NES_POKE(Nsf,5FF6) { chips->fds->prl.SwapBank<NES_4K,0x0000U>( data ); } 
		NES_POKE(Nsf,5FF7) { chips->fds->prl.SwapBank<NES_4K,0x1000U>( data ); }

		NES_POKE(Nsf,5FF8) { prg.SwapBank<NES_4K,0x0000U>( data ); }
		NES_POKE(Nsf,5FF9) { prg.SwapBank<NES_4K,0x1000U>( data ); }
		NES_POKE(Nsf,5FFA) { prg.SwapBank<NES_4K,0x2000U>( data ); }
		NES_POKE(Nsf,5FFB) { prg.SwapBank<NES_4K,0x3000U>( data ); }
		NES_POKE(Nsf,5FFC) { prg.SwapBank<NES_4K,0x4000U>( data ); }
		NES_POKE(Nsf,5FFD) { prg.SwapBank<NES_4K,0x5000U>( data ); }
		NES_POKE(Nsf,5FFE) { prg.SwapBank<NES_4K,0x6000U>( data ); }
		NES_POKE(Nsf,5FFF) { prg.SwapBank<NES_4K,0x7000U>( data ); }

		NES_PEEK(Nsf,Prg_8) { return prg[0][address - 0x8000U]; }
		NES_PEEK(Nsf,Prg_9) { return prg[1][address - 0x9000U]; }
		NES_PEEK(Nsf,Prg_A) { return prg[2][address - 0xA000U]; }
		NES_PEEK(Nsf,Prg_B) { return prg[3][address - 0xB000U]; }
		NES_PEEK(Nsf,Prg_C) { return prg[4][address - 0xC000U]; }
		NES_PEEK(Nsf,Prg_D) { return prg[5][address - 0xD000U]; }
		NES_PEEK(Nsf,Prg_E) { return prg[6][address - 0xE000U]; }
		NES_PEEK(Nsf,Prg_F) { return prg[7][address - 0xF000U]; }

		NES_POKE(Nsf,Prg_8) { prg[0][address - 0x8000U] = data; }
		NES_POKE(Nsf,Prg_9) { prg[1][address - 0x9000U] = data; }
		NES_POKE(Nsf,Prg_A) { prg[2][address - 0xA000U] = data; }
		NES_POKE(Nsf,Prg_B) { prg[3][address - 0xB000U] = data; }
		NES_POKE(Nsf,Prg_C) { prg[4][address - 0xC000U] = data; }
		NES_POKE(Nsf,Prg_D) { prg[5][address - 0xD000U] = data; }
		NES_POKE(Nsf,Prg_E) { prg[6][address - 0xE000U] = data; }
		NES_POKE(Nsf,Prg_F) { prg[7][address - 0xF000U] = data; }

		NES_POKE(Nsf,Vrc6_9000) { chips->vrc6->WriteSquareReg0( 0, data ); }
		NES_POKE(Nsf,Vrc6_9001) { chips->vrc6->WriteSquareReg1( 0, data ); }
		NES_POKE(Nsf,Vrc6_9002) { chips->vrc6->WriteSquareReg2( 0, data ); }
		NES_POKE(Nsf,Vrc6_A000) { chips->vrc6->WriteSquareReg0( 1, data ); }
		NES_POKE(Nsf,Vrc6_A001) { chips->vrc6->WriteSquareReg1( 1, data ); }
		NES_POKE(Nsf,Vrc6_A002) { chips->vrc6->WriteSquareReg2( 1, data ); }
		NES_POKE(Nsf,Vrc6_B000) { chips->vrc6->WriteSawReg0( data );       }
		NES_POKE(Nsf,Vrc6_B001) { chips->vrc6->WriteSawReg1( data );       }
		NES_POKE(Nsf,Vrc6_B002) { chips->vrc6->WriteSawReg2( data );       }

		NES_POKE(Nsf,Fme07_C) { chips->fme07->Poke_C000( data ); }
		NES_POKE(Nsf,Fme07_E) { chips->fme07->Poke_E000( data ); }

		NES_PEEK(Nsf,N106_48) { return chips->n106->Peek_4800(); }
		NES_POKE(Nsf,N106_48) { chips->n106->Poke_4800( data );  }
		NES_POKE(Nsf,N106_F8) { chips->n106->Poke_F800( data );  }

		NES_PEEK(Nsf,Prg_6) { return chips->fds->prl[0][address - 0x6000U]; }
		NES_PEEK(Nsf,Prg_7) { return chips->fds->prl[1][address - 0x7000U]; }
		NES_POKE(Nsf,Prg_6) { chips->fds->prl[0][address - 0x6000U] = data; }
		NES_POKE(Nsf,Prg_7) { chips->fds->prl[1][address - 0x7000U] = data; }

		NES_PEEK(Nsf,Ram) { return ram[address - 0x6000U]; }
		NES_POKE(Nsf,Ram) { ram[address - 0x6000U] = data; }

		NES_PEEK(Nsf,ExRam) { return chips->mmc5->exRam[address - 0x5C00U]; }
		NES_POKE(Nsf,ExRam) { chips->mmc5->exRam[address - 0x5C00U] = data; }

		NES_PEEK(Nsf,Nop) { return address >> 8; }
		NES_POKE(Nsf,Nop) {}
	}
}
