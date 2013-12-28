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

#ifndef NST_MAPPER_H
#define NST_MAPPER_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include "NstState.hpp"
#include "NstMemory.hpp"
#include "NstCpu.hpp"
#include "NstPpu.hpp"

namespace Nes
{
	namespace Core
	{
		class NST_NO_VTABLE Mapper
		{
		public:

			struct Context
			{
				const uint id;

				Cpu& cpu;
				Ppu& ppu;

				Ram& prg;
				Ram& chr;
				Ram& wrk;

				const Ppu::Mirroring mirroring;
				const dword wrkBacked;
				const uint attribute;
				dword chrRam;
				bool wrkAuto;

				Context
				(
					uint i,
					Cpu& c,
					Ppu& p,
					Ram& pr,
					Ram& cr,
					Ram& wr,
					Ppu::Mirroring m,
					dword b,
					uint a
				)
				:
				id        (i),
				cpu       (c),
				ppu       (p),
				prg       (pr),
				chr       (cr),
				wrk       (wr),
				mirroring (m),
				wrkBacked (b),
				attribute (a),
				chrRam    (0),
				wrkAuto   (false)
				{}
			};

			static Mapper* Create(Context&);
			virtual ~Mapper();

			void Reset     (bool);
			void SaveState (State::Saver&) const;
			void LoadState (State::Loader&);

			virtual void Flush(bool) {}
			virtual void VSync() {}

			typedef void* Device;

			enum DeviceType
			{
				DEVICE_DIP_SWITCHES = 1,
				DEVICE_BARCODE_READER
			};

			virtual Device QueryDevice(DeviceType)
			{
				return NULL;
			}

			enum
			{
				EXT_SUPER24IN1 = 256,
				EXT_8157,
				EXT_8237,
				EXT_WS,
				EXT_DREAMTECH01,
				EXT_CC21,
				EXT_KOF97,
				EXT_64IN1NR,
				EXT_STREETHEROES,
				EXT_T262,
				EXT_FK23C,
				EXT_6035052,
				EXT_A65AS,
				EXT_EDU2000,
				NUM_EXT_MAPPERS = 14
			};

			static cstring GetBoard(uint);

		protected:

			enum
			{
				PROM_DEFAULT   = 0  << 0,
				PROM_MAX_16K   = 1  << 0,
				PROM_MAX_32K   = 2  << 0,
				PROM_MAX_64K   = 3  << 0,
				PROM_MAX_128K  = 4  << 0,
				PROM_MAX_256K  = 5  << 0,
				PROM_MAX_512K  = 6  << 0,
				PROM_MAX_1024K = 7  << 0,
				PROM_SETTINGS  = 7  << 0,
				CROM_DEFAULT   = 0  << 3,
				CROM_NONE      = 1  << 3,
				CROM_MAX_8K    = 2  << 3,
				CROM_MAX_16K   = 3  << 3,
				CROM_MAX_32K   = 4  << 3,
				CROM_MAX_64K   = 5  << 3,
				CROM_MAX_128K  = 6  << 3,
				CROM_MAX_256K  = 7  << 3,
				CROM_MAX_512K  = 8  << 3,
				CROM_MAX_1024K = 9  << 3,
				CROM_SETTINGS  = 15 << 3,
				CRAM_DEFAULT   = 0  << 7,
				CRAM_1K        = 1  << 7,
				CRAM_2K        = 2  << 7,
				CRAM_4K        = 3  << 7,
				CRAM_8K        = 4  << 7,
				CRAM_16K       = 5  << 7,
				CRAM_32K       = 6  << 7,
				CRAM_SETTINGS  = 7  << 7,
				WRAM_AUTO      = 0  << 10,
				WRAM_DEFAULT   = 1  << 10,
				WRAM_NONE      = 2  << 10,
				WRAM_1K        = 3  << 10,
				WRAM_2K        = 4  << 10,
				WRAM_4K        = 5  << 10,
				WRAM_8K        = 6  << 10,
				WRAM_16K       = 7  << 10,
				WRAM_32K       = 8  << 10,
				WRAM_40K       = 9  << 10,
				WRAM_64K       = 10 << 10,
				WRAM_SETTINGS  = 15 << 10
			};

			enum PrgMapping
			{
				PRG_SWAP_8K_0,
				PRG_SWAP_8K_1,
				PRG_SWAP_8K_2,
				PRG_SWAP_8K_3,
				PRG_SWAP_16K_0,
				PRG_SWAP_16K_1,
				PRG_SWAP_32K
			};

			enum ChrMapping
			{
				CHR_SWAP_1K_0,
				CHR_SWAP_1K_1,
				CHR_SWAP_1K_2,
				CHR_SWAP_1K_3,
				CHR_SWAP_1K_4,
				CHR_SWAP_1K_5,
				CHR_SWAP_1K_6,
				CHR_SWAP_1K_7,
				CHR_SWAP_2K_0,
				CHR_SWAP_2K_1,
				CHR_SWAP_2K_2,
				CHR_SWAP_2K_3,
				CHR_SWAP_4K_0,
				CHR_SWAP_4K_1,
				CHR_SWAP_8K
			};

			enum NmtMapping
			{
				NMT_SWAP_HV,
				NMT_SWAP_VH,
				NMT_SWAP_VH01,
				NMT_SWAP_HV01
			};

			enum WrkMapping
			{
				WRK_PEEK,
				WRK_POKE,
				WRK_PEEK_POKE,
				WRK_SAFE_PEEK,
				WRK_SAFE_POKE,
				WRK_SAFE_PEEK_POKE
			};

			enum NopMapping
			{
				NOP_PEEK,
				NOP_POKE,
				NOP_PEEK_POKE
			};

			Mapper(Context&,uint=0);

			typedef Memory<SIZE_32K,SIZE_8K,2> Prg;
			typedef Ppu::ChrMem Chr;
			typedef Ppu::NmtMem Nmt;

			struct Wrk : Memory<SIZE_8K,SIZE_8K,2>
			{
				bool HasRam() const
				{
					return Source(0).Mem() != Source(1).Mem();
				}

				dword RamSize() const
				{
					return HasRam() ? Source(0).Size() : 0;
				}
			};

			Prg prg;
			Cpu& cpu;
			Ppu& ppu;
			Chr& chr;
			Nmt& nmt;
			Wrk wrk;
			const u16 id;
			const u16 mirroring;

			template<typename T>
			void Map(uint first,uint last,T t) const
			{
				cpu.Map( first, last ).Set( t );
			}

			template<typename T,typename U>
			void Map(uint first,uint last,T t,U u) const
			{
				cpu.Map( first, last ).Set( t, u );
			}

			template<typename T>
			void Map(uint address,T t) const
			{
				Map( address, address, t );
			}

			template<typename T,typename U>
			void Map(uint address,T t,U u) const
			{
				cpu.Map( address ).Set( t, u );
			}

			void Map(WrkMapping) const;

		private:

			NES_DECL_PEEK( Prg_8 )
			NES_DECL_PEEK( Prg_A )
			NES_DECL_PEEK( Prg_C )
			NES_DECL_PEEK( Prg_E )

			NES_DECL_POKE( Prg_8k_0  )
			NES_DECL_POKE( Prg_8k_1  )
			NES_DECL_POKE( Prg_8k_2  )
			NES_DECL_POKE( Prg_8k_3  )
			NES_DECL_POKE( Prg_16k_0 )
			NES_DECL_POKE( Prg_16k_1 )
			NES_DECL_POKE( Prg_32k   )

			NES_DECL_POKE( Chr_1k_0 )
			NES_DECL_POKE( Chr_1k_1 )
			NES_DECL_POKE( Chr_1k_2 )
			NES_DECL_POKE( Chr_1k_3 )
			NES_DECL_POKE( Chr_1k_4 )
			NES_DECL_POKE( Chr_1k_5 )
			NES_DECL_POKE( Chr_1k_6 )
			NES_DECL_POKE( Chr_1k_7 )
			NES_DECL_POKE( Chr_2k_0 )
			NES_DECL_POKE( Chr_2k_1 )
			NES_DECL_POKE( Chr_2k_2 )
			NES_DECL_POKE( Chr_2k_3 )
			NES_DECL_POKE( Chr_4k_0 )
			NES_DECL_POKE( Chr_4k_1 )
			NES_DECL_POKE( Chr_8k   )

			NES_DECL_PEEK( Wrk_6 )
			NES_DECL_POKE( Wrk_6 )
			NES_DECL_PEEK( Wrk_Safe_6 )
			NES_DECL_POKE( Wrk_Safe_6 )

			NES_DECL_POKE( Nmt_Hv )
			NES_DECL_POKE( Nmt_Vh )
			NES_DECL_POKE( Nmt_Vh01 )
			NES_DECL_POKE( Nmt_Hv01 )

			NES_DECL_PEEK( Nop )
			NES_DECL_POKE( Nop )

			dword GetStateName() const;

			virtual void SubReset(bool) = 0;
			virtual void SubSave(State::Saver&) const {}
			virtual void SubLoad(State::Loader&) {}
			virtual void BaseSave(State::Saver&) const {}
			virtual void BaseLoad(State::Loader&,dword) {}

			struct Setup;
			static const Setup setup[256+NUM_EXT_MAPPERS];

		protected:

			void SetMirroringHV(uint data) { NES_CALL_POKE(Mapper,Nmt_Hv,0,data); }
			void SetMirroringVH(uint data) { NES_CALL_POKE(Mapper,Nmt_Vh,0,data); }

			void SetMirroringVH01(uint data) { NES_CALL_POKE(Mapper,Nmt_Vh01,0,data); }
			void SetMirroringHV01(uint data) { NES_CALL_POKE(Mapper,Nmt_Hv01,0,data); }
		};

		template<>
		inline void Mapper::Map(uint first,uint last,NopMapping m) const
		{
			switch (m)
			{
				case NOP_PEEK:      cpu.Map( first, last ).Set( &Mapper::Peek_Nop ); break;
				case NOP_POKE:      cpu.Map( first, last ).Set( &Mapper::Poke_Nop ); break;
				case NOP_PEEK_POKE: cpu.Map( first, last ).Set( &Mapper::Peek_Nop, &Mapper::Poke_Nop ); break;
			}
		}

		template<>
		inline void Mapper::Map(uint first,uint last,PrgMapping mapping) const
		{
			cpu.Map( first, last ).Set
			(
				mapping == PRG_SWAP_8K_0  ? &Mapper::Poke_Prg_8k_0 :
				mapping == PRG_SWAP_8K_1  ? &Mapper::Poke_Prg_8k_1 :
				mapping == PRG_SWAP_8K_2  ? &Mapper::Poke_Prg_8k_2 :
				mapping == PRG_SWAP_8K_3  ? &Mapper::Poke_Prg_8k_3 :
				mapping == PRG_SWAP_16K_0 ? &Mapper::Poke_Prg_16k_0 :
				mapping == PRG_SWAP_16K_1 ? &Mapper::Poke_Prg_16k_1 :
											&Mapper::Poke_Prg_32k
			);
		}

		template<>
		inline void Mapper::Map(uint first,uint last,ChrMapping mapping) const
		{
			cpu.Map( first, last ).Set
			(
				mapping == CHR_SWAP_1K_0 ? &Mapper::Poke_Chr_1k_0 :
				mapping == CHR_SWAP_1K_1 ? &Mapper::Poke_Chr_1k_1 :
				mapping == CHR_SWAP_1K_2 ? &Mapper::Poke_Chr_1k_2 :
				mapping == CHR_SWAP_1K_3 ? &Mapper::Poke_Chr_1k_3 :
				mapping == CHR_SWAP_1K_4 ? &Mapper::Poke_Chr_1k_4 :
				mapping == CHR_SWAP_1K_5 ? &Mapper::Poke_Chr_1k_5 :
				mapping == CHR_SWAP_1K_6 ? &Mapper::Poke_Chr_1k_6 :
				mapping == CHR_SWAP_1K_7 ? &Mapper::Poke_Chr_1k_7 :
				mapping == CHR_SWAP_2K_0 ? &Mapper::Poke_Chr_2k_0 :
				mapping == CHR_SWAP_2K_1 ? &Mapper::Poke_Chr_2k_1 :
				mapping == CHR_SWAP_2K_2 ? &Mapper::Poke_Chr_2k_2 :
				mapping == CHR_SWAP_2K_3 ? &Mapper::Poke_Chr_2k_3 :
				mapping == CHR_SWAP_4K_0 ? &Mapper::Poke_Chr_4k_0 :
				mapping == CHR_SWAP_4K_1 ? &Mapper::Poke_Chr_4k_1 :
                                           &Mapper::Poke_Chr_8k
			);
		}

		template<>
		inline void Mapper::Map(uint first,uint last,NmtMapping mapping) const
		{
			cpu.Map( first, last ).Set
			(
				mapping == NMT_SWAP_HV   ? &Mapper::Poke_Nmt_Hv :
				mapping == NMT_SWAP_VH   ? &Mapper::Poke_Nmt_Vh :
				mapping == NMT_SWAP_VH01 ? &Mapper::Poke_Nmt_Vh01 :
                                           &Mapper::Poke_Nmt_Hv01
			);
		}
	}
}

#endif
