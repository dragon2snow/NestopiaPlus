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

#ifndef NST_MEMORY_H
#define NST_MEMORY_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include "NstState.hpp"
#include "NstRam.hpp"

namespace Nes
{
	namespace Core
	{
		namespace State
		{
			class Saver;
			class Loader;
		}

		class BaseMemory
		{
		protected:

			template<uint W> struct GetBlockShift
			{
				enum { VALUE = 1 + GetBlockShift<W / 2>::VALUE };
			};

			template<uint N> struct Pages
			{
				u8* mem[N];
				u8 ref[N];
			};

			template<uint OFFSET,uint COUNT,uint SIZE,uint I=COUNT>
			struct Unroller
			{
				template<typename Pages>
				static NST_FORCE_INLINE void SwapBank
				(
					Pages* const NST_RESTRICT pages,
					u8* const NST_RESTRICT mem,
					const dword mask,
					const dword bank,
					const uint offset=0,
					const uint source=0
				)
				{
					pages->mem[OFFSET+COUNT-I+offset] = mem + (bank & mask);
					pages->ref[OFFSET+COUNT-I+offset] = source;

					Unroller<OFFSET,COUNT,SIZE,I-1>::SwapBank( pages, mem, mask, bank + SIZE, offset, source );
				}

				template<typename Pages>
				static NST_FORCE_INLINE void SwapBanks
				(
					Pages* const NST_RESTRICT pages,
					u8* const NST_RESTRICT mem,
					const dword mask,
					const dword bank0,
					const dword bank1,
					const uint offset=0
				)
				{
					Unroller<OFFSET+COUNT*0,COUNT,SIZE,I>::SwapBank( pages, mem, mask, bank0, offset );
					Unroller<OFFSET+COUNT*1,COUNT,SIZE,I>::SwapBank( pages, mem, mask, bank1, offset );
				}

				template<typename Pages>
				static NST_FORCE_INLINE void SwapBanks
				(
					Pages* const NST_RESTRICT pages,
					u8* const NST_RESTRICT mem,
					const dword mask,
					const dword bank0,
					const dword bank1,
					const dword bank2,
					const dword bank3,
					const uint offset=0
				)
				{
					Unroller<OFFSET+COUNT*0,COUNT,SIZE,I>::SwapBank( pages, mem, mask, bank0, offset );
					Unroller<OFFSET+COUNT*1,COUNT,SIZE,I>::SwapBank( pages, mem, mask, bank1, offset );
					Unroller<OFFSET+COUNT*2,COUNT,SIZE,I>::SwapBank( pages, mem, mask, bank2, offset );
					Unroller<OFFSET+COUNT*3,COUNT,SIZE,I>::SwapBank( pages, mem, mask, bank3, offset );
				}

				template<typename Pages>
				static NST_FORCE_INLINE void SwapBanks
				(
					Pages* const NST_RESTRICT pages,
					u8* const NST_RESTRICT mem,
					const dword mask,
					const dword bank0,
					const dword bank1,
					const dword bank2,
					const dword bank3,
					const dword bank4,
					const dword bank5,
					const dword bank6,
					const dword bank7,
					const uint offset=0
				)
				{
					Unroller<OFFSET+COUNT*0,COUNT,SIZE,I>::SwapBank( pages, mem, mask, bank0, offset );
					Unroller<OFFSET+COUNT*1,COUNT,SIZE,I>::SwapBank( pages, mem, mask, bank1, offset );
					Unroller<OFFSET+COUNT*2,COUNT,SIZE,I>::SwapBank( pages, mem, mask, bank2, offset );
					Unroller<OFFSET+COUNT*3,COUNT,SIZE,I>::SwapBank( pages, mem, mask, bank3, offset );
					Unroller<OFFSET+COUNT*4,COUNT,SIZE,I>::SwapBank( pages, mem, mask, bank4, offset );
					Unroller<OFFSET+COUNT*5,COUNT,SIZE,I>::SwapBank( pages, mem, mask, bank5, offset );
					Unroller<OFFSET+COUNT*6,COUNT,SIZE,I>::SwapBank( pages, mem, mask, bank6, offset );
					Unroller<OFFSET+COUNT*7,COUNT,SIZE,I>::SwapBank( pages, mem, mask, bank7, offset );
				}
			};
		};

		template<>
		struct BaseMemory::GetBlockShift<1U>
		{
			enum { VALUE = 0 };
		};

		template<>
		struct BaseMemory::Pages<1U>
		{
			u8* mem[1];
			u32 ref[1];
		};

		template<>
		struct BaseMemory::Pages<2U>
		{
			u8* mem[2];
			u16 ref[2];
		};

		template<uint OFFSET,uint COUNT,uint SIZE>
		struct BaseMemory::Unroller<OFFSET,COUNT,SIZE,0U>
		{
			template<typename Pages>
			static NST_FORCE_INLINE void SwapBank(Pages*,u8*,dword,dword,uint,uint) {}
		};

		template<dword SPACE,uint U,uint V=1>
		class Memory : BaseMemory
		{
		public:

			enum
			{
				NUM_SOURCES = V
			};

		private:

			NST_COMPILE_ASSERT
			(
				((SPACE & (SPACE-1)) == 0) &&
				((U & (U-1)) == 0) &&
				(SPACE % U == 0) &&
				(V == 1 || V == 2)
			);

			enum
			{
				MEM_PAGE_SIZE = U,
				MEM_PAGE_SHIFT = GetBlockShift<MEM_PAGE_SIZE>::VALUE,
				MEM_PAGE_MASK = MEM_PAGE_SIZE - 1,
				MEM_NUM_PAGES = SPACE / U
			};

			typedef BaseMemory::Pages<MEM_NUM_PAGES> Pages;

			Pages pages;
			Ram sources[NUM_SOURCES];

		public:

			ibool Readable(uint page) const
			{
				return sources[pages.ref[page]].Readable();
			}

			ibool Writable(uint page) const
			{
				return sources[pages.ref[page]].Writable();
			}

			const u8& Peek(uint address) const
			{
				return pages.mem[address >> MEM_PAGE_SHIFT][address & MEM_PAGE_MASK];
			}

			u8* operator [] (uint page)
			{
				return pages.mem[page];
			}

			const u8* operator [] (uint page) const
			{
				return pages.mem[page];
			}

			void Poke(uint address,uint data)
			{
				const uint page = address >> MEM_PAGE_SHIFT;
				NST_VERIFY( Writable( page ) );

				if (Writable( page ))
					pages.mem[page][address & MEM_PAGE_MASK] = data;
			}

			template<uint SIZE,uint ADDRESS>
			void SwapBank(dword);

			template<uint SIZE,uint ADDRESS>
			void SwapBanks(dword,dword);

			template<uint SIZE,uint ADDRESS>
			void SwapBanks(dword,dword,dword,dword);

			template<uint SIZE,uint ADDRESS>
			void SwapBanks(dword,dword,dword,dword,dword,dword,dword,dword);

			template<uint SIZE>
			void SwapBank(uint,dword);

			template<uint SIZE>
			void SwapBanks(uint,dword,dword);

			template<uint SIZE>
			void SwapBanks(uint,dword,dword,dword,dword);

			template<uint SIZE>
			void SwapBanks(uint,dword,dword,dword,dword,dword,dword,dword,dword);

			void SaveState(State::Saver&,uint) const;
			void LoadState(State::Loader&,uint);

			class SourceProxy;
			friend class SourceProxy;

			class SourceProxy
			{
				const uint source;
				Memory& ref;

			public:

				SourceProxy(uint s,Memory& r)
				: source(s), ref(r)
				{
					NST_ASSERT( s < NUM_SOURCES );
				}

				template<uint SIZE,uint ADDRESS>
				void SwapBank(dword) const;

				template<uint SIZE>
				void SwapBank(uint,dword) const;

				void ReadEnable(bool read) const
				{
					ref.sources[source].ReadEnable( read );
				}

				void WriteEnable(bool write) const
				{
					ref.sources[source].WriteEnable( write );
				}

				void SetSecurity(bool read,bool write) const
				{
					ref.sources[source].ReadEnable( read );
					ref.sources[source].WriteEnable( write );
				}

				ibool Readable() const
				{
					return ref.sources[source].Readable();
				}

				ibool Writable() const
				{
					return ref.sources[source].Writable();
				}

				const SourceProxy& Set(u8* mem,dword size,bool read,bool write) const
				{
					ref.sources[source].Set( read, write, size, mem );
					return *this;
				}

				const SourceProxy& Set(dword size,bool read,bool write) const
				{
					ref.sources[source].Set( read, write, size );
					return *this;
				}

				void Remove() const
				{
					ref.sources[source].Destroy();
				}

				void Fill(uint value) const
				{
					ref.sources[source].Fill( value );
				}

				u8* Mem(dword offset=0) const
				{
					return ref.sources[source].Mem(offset);
				}

				dword Size() const
				{
					return ref.sources[source].Size();
				}

				dword Masking() const
				{
					return ref.sources[source].Masking();
				}

				ibool Internal() const
				{
					return ref.sources[source].Internal();
				}

				bool Empty() const
				{
					return ref.sources[source].Size() == 0;
				}
			};

		public:

			const SourceProxy Source(uint i=0)
			{
				NST_ASSERT( i < NUM_SOURCES );
				return SourceProxy( i, *this );
			}

			const Ram& Source(uint i=0) const
			{
				NST_ASSERT( i < NUM_SOURCES );
				return sources[i];
			}

			Memory()
			{
			}

			Memory(u8* mem,dword size,bool read,bool write)
			{
				Source().Set( mem, size, read, write );
			}

			Memory(dword size,bool read,bool write)
			{
				Source().Set( size, read, write );
			}

			template<uint SIZE,uint ADDRESS>
			dword GetBank() const
			{
				NST_COMPILE_ASSERT( SIZE && (SIZE % MEM_PAGE_SIZE == 0) && (SPACE >= ADDRESS + SIZE) );

				enum {MEM_PAGE = ADDRESS >> MEM_PAGE_SHIFT};
				return dword(pages.mem[MEM_PAGE] - sources[pages.ref[MEM_PAGE]].Mem()) >> GetBlockShift<SIZE>::VALUE;
			}

			template<uint SIZE>
			dword GetBank(uint address) const
			{
				NST_COMPILE_ASSERT( SIZE && (SIZE % MEM_PAGE_SIZE == 0) );
				NST_ASSERT( SPACE >= address + SIZE );

				address >>= MEM_PAGE_SHIFT;
				return dword(pages.mem[address] - sources[pages.ref[address]].Mem()) >> GetBlockShift<SIZE>::VALUE;
			}
		};

		template<dword SPACE,uint U,uint V> template<uint SIZE,uint ADDRESS>
		void Memory<SPACE,U,V>::SwapBank(dword bank)
		{
			NST_COMPILE_ASSERT( (SPACE >= ADDRESS + SIZE) && SIZE && (SIZE % MEM_PAGE_SIZE == 0) );

			enum
			{
				MEM_OFFSET = GetBlockShift<SIZE>::VALUE,
				MEM_PAGE_BEGIN = ADDRESS / MEM_PAGE_SIZE,
				MEM_PAGE_COUNT = SIZE / MEM_PAGE_SIZE
			};

			BaseMemory::Unroller<MEM_PAGE_BEGIN,MEM_PAGE_COUNT,MEM_PAGE_SIZE>::SwapBank
			(
				&pages,
				sources[0].Mem(),
				sources[0].Masking(),
				bank << MEM_OFFSET
			);
		}

		template<dword SPACE,uint U,uint V> template<uint SIZE>
		void Memory<SPACE,U,V>::SwapBank(uint address,dword bank)
		{
			NST_COMPILE_ASSERT( SIZE && (SIZE % MEM_PAGE_SIZE == 0) );
			NST_ASSERT( SPACE >= address + SIZE );

			enum
			{
				MEM_OFFSET = GetBlockShift<SIZE>::VALUE,
				MEM_PAGE_COUNT = SIZE / MEM_PAGE_SIZE
			};

			BaseMemory::Unroller<0,MEM_PAGE_COUNT,MEM_PAGE_SIZE>::SwapBank
			(
				&pages,
				sources[0].Mem(),
				sources[0].Masking(),
				bank << MEM_OFFSET,
				address >> MEM_PAGE_SHIFT
			);
		}

		template<dword SPACE,uint U,uint V> template<uint SIZE,uint ADDRESS>
		void Memory<SPACE,U,V>::SwapBanks(dword bank0,dword bank1)
		{
			NST_COMPILE_ASSERT( (SPACE >= ADDRESS + SIZE * 2) && SIZE && (SIZE % MEM_PAGE_SIZE == 0) );

			enum
			{
				MEM_OFFSET = GetBlockShift<SIZE>::VALUE,
				MEM_PAGE_BEGIN = ADDRESS / MEM_PAGE_SIZE,
				MEM_PAGE_COUNT = SIZE / MEM_PAGE_SIZE
			};

			BaseMemory::Unroller<MEM_PAGE_BEGIN,MEM_PAGE_COUNT,MEM_PAGE_SIZE>::SwapBanks
			(
				&pages,
				sources[0].Mem(),
				sources[0].Masking(),
				bank0 << MEM_OFFSET,
				bank1 << MEM_OFFSET
			);
		}

		template<dword SPACE,uint U,uint V> template<uint SIZE,uint ADDRESS>
		void Memory<SPACE,U,V>::SwapBanks(dword bank0,dword bank1,dword bank2,dword bank3)
		{
			NST_COMPILE_ASSERT( (SPACE >= ADDRESS + SIZE * 4) && SIZE && (SIZE % MEM_PAGE_SIZE == 0) );

			enum
			{
				MEM_OFFSET = GetBlockShift<SIZE>::VALUE,
				MEM_PAGE_BEGIN = ADDRESS / MEM_PAGE_SIZE,
				MEM_PAGE_COUNT = SIZE / MEM_PAGE_SIZE
			};

			BaseMemory::Unroller<MEM_PAGE_BEGIN,MEM_PAGE_COUNT,MEM_PAGE_SIZE>::SwapBanks
			(
				&pages,
				sources[0].Mem(),
				sources[0].Masking(),
				bank0 << MEM_OFFSET,
				bank1 << MEM_OFFSET,
				bank2 << MEM_OFFSET,
				bank3 << MEM_OFFSET
			);
		}

		template<dword SPACE,uint U,uint V> template<uint SIZE,uint ADDRESS>
		void Memory<SPACE,U,V>::SwapBanks(dword bank0,dword bank1,dword bank2,dword bank3,dword bank4,dword bank5,dword bank6,dword bank7)
		{
			NST_COMPILE_ASSERT( (SPACE >= ADDRESS + SIZE * 4) && SIZE && (SIZE % MEM_PAGE_SIZE == 0) );

			enum
			{
				MEM_OFFSET = GetBlockShift<SIZE>::VALUE,
				MEM_PAGE_BEGIN = ADDRESS / MEM_PAGE_SIZE,
				MEM_PAGE_COUNT = SIZE / MEM_PAGE_SIZE
			};

			BaseMemory::Unroller<MEM_PAGE_BEGIN,MEM_PAGE_COUNT,MEM_PAGE_SIZE>::SwapBanks
			(
				&pages,
				sources[0].Mem(),
				sources[0].Masking(),
				bank0 << MEM_OFFSET,
				bank1 << MEM_OFFSET,
				bank2 << MEM_OFFSET,
				bank3 << MEM_OFFSET,
				bank4 << MEM_OFFSET,
				bank5 << MEM_OFFSET,
				bank6 << MEM_OFFSET,
				bank7 << MEM_OFFSET
			);
		}

		template<dword SPACE,uint U,uint V> template<uint SIZE>
		void Memory<SPACE,U,V>::SwapBanks(uint address,dword bank0,dword bank1)
		{
			NST_COMPILE_ASSERT( SIZE && (SIZE % MEM_PAGE_SIZE == 0) );
			NST_ASSERT( SPACE >= address + SIZE * 2 );

			enum
			{
				MEM_OFFSET = GetBlockShift<SIZE>::VALUE,
				MEM_PAGE_COUNT = SIZE / MEM_PAGE_SIZE
			};

			BaseMemory::Unroller<0,MEM_PAGE_COUNT,MEM_PAGE_SIZE>::SwapBanks
			(
				&pages,
				sources[0].Mem(),
				sources[0].Masking(),
				bank0 << MEM_OFFSET,
				bank1 << MEM_OFFSET,
				address >> MEM_PAGE_SHIFT
			);
		}

		template<dword SPACE,uint U,uint V> template<uint SIZE>
		void Memory<SPACE,U,V>::SwapBanks(uint address,dword bank0,dword bank1,dword bank2,dword bank3)
		{
			NST_COMPILE_ASSERT( SIZE && (SIZE % MEM_PAGE_SIZE == 0) );
			NST_ASSERT( SPACE >= address + SIZE * 4 );

			enum
			{
				MEM_OFFSET = GetBlockShift<SIZE>::VALUE,
				MEM_PAGE_COUNT = SIZE / MEM_PAGE_SIZE
			};

			BaseMemory::Unroller<0,MEM_PAGE_COUNT,MEM_PAGE_SIZE>::SwapBanks
			(
				&pages,
				sources[0].Mem(),
				sources[0].Masking(),
				bank0 << MEM_OFFSET,
				bank1 << MEM_OFFSET,
				bank2 << MEM_OFFSET,
				bank3 << MEM_OFFSET,
				address >> MEM_PAGE_SHIFT
			);
		}

		template<dword SPACE,uint U,uint V> template<uint SIZE>
		void Memory<SPACE,U,V>::SwapBanks(uint address,dword bank0,dword bank1,dword bank2,dword bank3,dword bank4,dword bank5,dword bank6,dword bank7)
		{
			NST_COMPILE_ASSERT( SIZE && (SIZE % MEM_PAGE_SIZE == 0) );
			NST_ASSERT( SPACE >= address + SIZE * 4 );

			enum
			{
				MEM_OFFSET = GetBlockShift<SIZE>::VALUE,
				MEM_PAGE_COUNT = SIZE / MEM_PAGE_SIZE
			};

			BaseMemory::Unroller<0,MEM_PAGE_COUNT,MEM_PAGE_SIZE>::SwapBanks
			(
				&pages,
				sources[0].Mem(),
				sources[0].Masking(),
				bank0 << MEM_OFFSET,
				bank1 << MEM_OFFSET,
				bank2 << MEM_OFFSET,
				bank3 << MEM_OFFSET,
				bank4 << MEM_OFFSET,
				bank5 << MEM_OFFSET,
				bank6 << MEM_OFFSET,
				bank7 << MEM_OFFSET,
				address >> MEM_PAGE_SHIFT
			);
		}

		template<dword SPACE,uint U,uint V> template<uint SIZE,uint ADDRESS>
		void Memory<SPACE,U,V>::SourceProxy::SwapBank(dword bank) const
		{
			NST_COMPILE_ASSERT( (SPACE >= ADDRESS + SIZE) && SIZE && (SIZE % MEM_PAGE_SIZE == 0) );

			enum
			{
				MEM_OFFSET = GetBlockShift<SIZE>::VALUE,
				MEM_PAGE_BEGIN = ADDRESS / MEM_PAGE_SIZE,
				MEM_PAGE_COUNT = SIZE / MEM_PAGE_SIZE
			};

			BaseMemory::Unroller<MEM_PAGE_BEGIN,MEM_PAGE_COUNT,MEM_PAGE_SIZE>::SwapBank
			(
				&ref.pages,
				ref.sources[source].Mem(),
				ref.sources[source].Masking(),
				bank << MEM_OFFSET,
				0,
				source
			);
		}

		template<dword SPACE,uint U,uint V> template<uint SIZE>
		void Memory<SPACE,U,V>::SourceProxy::SwapBank(uint address,dword bank) const
		{
			NST_COMPILE_ASSERT( SIZE && (SIZE % MEM_PAGE_SIZE == 0) );
			NST_ASSERT( SPACE >= address + SIZE );

			enum
			{
				MEM_OFFSET = GetBlockShift<SIZE>::VALUE,
				MEM_PAGE_COUNT = SIZE / MEM_PAGE_SIZE
			};

			BaseMemory::Unroller<0,MEM_PAGE_COUNT,MEM_PAGE_SIZE>::SwapBank
			(
				&ref.pages,
				ref.sources[source].Mem(),
				ref.sources[source].Masking(),
				bank << MEM_OFFSET,
				address >> MEM_PAGE_SHIFT,
				source
			);
		}

		template<dword SPACE,uint U,uint V>
		void Memory<SPACE,U,V>::SaveState(State::Saver& state,const uint sourceMask) const
		{
			{
				u8 data[NUM_SOURCES];

				for (uint i=0; i < NUM_SOURCES; ++i)
					data[i] = (sources[i].Readable() ? 0x1 : 0x0) | (sources[i].Writable() ? 0x2 : 0x0);

				state.Begin('A','C','C','\0').Write( data ).End();
			}

			for (uint i=0; i < NUM_SOURCES; ++i)
			{
				if (sourceMask & (1U << i))
					state.Begin('R','M','0'+i,'\0').Compress( sources[i].Mem(), sources[i].Size() ).End();
			}

			{
				u8 data[MEM_NUM_PAGES*3];

				for (uint i=0; i < MEM_NUM_PAGES; ++i)
				{
					const uint bank = GetBank<MEM_PAGE_SIZE>( i * MEM_PAGE_SIZE );

					data[i*3+0] = pages.ref[i];
					data[i*3+1] = bank & 0xFF;
					data[i*3+2] = bank >> 8;
				}

				state.Begin('B','N','K','\0').Write( data ).End();
			}
		}

		template<dword SPACE,uint U,uint V>
		void Memory<SPACE,U,V>::LoadState(State::Loader& state,const uint sourceMask)
		{
			while (const dword chunk = state.Begin())
			{
				switch (chunk)
				{
					case NES_STATE_CHUNK_ID('A','C','C','\0'):
					{
						const State::Loader::Data<NUM_SOURCES> data( state );

						for (uint i=0; i < NUM_SOURCES; ++i)
						{
							sources[i].ReadEnable( data[i] & 0x1 );
							sources[i].WriteEnable( data[i] & 0x2 );
						}

						break;
					}

					case NES_STATE_CHUNK_ID('B','N','K','\0'):
					{
						const State::Loader::Data<MEM_NUM_PAGES*3> data( state );

						for (uint i=0; i < MEM_NUM_PAGES; ++i)
						{
							if (data[i*3+0] < NUM_SOURCES)
							{
								Source( data[i*3+0] ).SwapBank<MEM_PAGE_SIZE>( i * MEM_PAGE_SIZE, data[i*3+1] | (data[i*3+2] << 8) );
							}
							else
							{
								throw RESULT_ERR_CORRUPT_FILE;
							}
						}

						break;
					}

					default:

						for (uint i=0; i < NUM_SOURCES; ++i)
						{
							if (chunk == NES_STATE_CHUNK_ID('R','M','0'+i,'\0'))
							{
								if (sourceMask & (1U << i))
									state.Uncompress( sources[i].Mem(), sources[i].Size() );

								break;
							}
						}
						break;
				}

				state.End();
			}
		}
	}
}

#endif
