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

#ifndef NST_CHIP_H
#define NST_CHIP_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include "NstState.hpp"

namespace Nes
{
	namespace Core
	{
		namespace State
		{
			class Saver;
			class Loader;
		}

		struct BaseMemory
		{
			struct Linear
			{
				Linear();
				~Linear();

				void Set(dword);
				void Set(u8*,dword);
				void Destroy();
				void Fill(int);

				static dword GetPower2(dword);

				u8* mem;
				dword mask;
				dword size;
				u8 writable;
				u8 readable;
				u8 internal;
				const u8 pad;
			};

			template<uint W> struct GetBlockShift
			{
				enum { VALUE = 1 + GetBlockShift<W / 2>::VALUE };
			};

			template<uint W> struct GetBlockMask
			{
				enum { VALUE = (1U << (W-1)) | GetBlockMask<W-1>::VALUE };
			};
		};

		template<> struct BaseMemory::GetBlockShift<1U>
		{
			enum { VALUE = 0 };
		};

		template<> struct BaseMemory::GetBlockMask<0U>
		{
			enum { VALUE = 1 };
		};

		template<dword SPACE,uint U,uint V=1>
		class Memory : public BaseMemory
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

			struct Pages
			{
				u8* mem[MEM_NUM_PAGES];
				uchar ref[MEM_NUM_PAGES];
			};

			Pages pages;
			BaseMemory::Linear sources[NUM_SOURCES];

		#ifndef NDEBUG

			Memory(const Memory&) {}
			void operator = (const Memory&) {}

		#endif

		public:

			ibool IsReadable(uint page) const
			{
				return sources[pages.ref[page]].readable;
			}

			ibool IsWritable(uint page) const
			{
				return sources[pages.ref[page]].writable;
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
				NST_VERIFY( IsWritable( page ) );

				if (IsWritable( page ))
					pages.mem[page][address & MEM_PAGE_MASK] = data;
			}

			template<uint SIZE,uint ADDRESS>
			void SwapBank(dword);

			template<uint SIZE,uint ADDRESS>
			void SwapBanks(dword,dword);

			template<uint SIZE,uint ADDRESS>
			void SwapBanks(dword,dword,dword,dword);

			template<uint SIZE>
			void SwapBank(uint,dword);

			template<uint SIZE>
			void SwapBanks(uint,dword,dword);

			template<uint SIZE>
			void SwapBanks(uint,dword,dword,dword,dword);

			void SaveState(State::Saver&,uint) const;
			void LoadState(State::Loader&,uint);

			class SourceProxy;
			class ConstSourceProxy;

			friend class SourceProxy;
			friend class ConstSourceProxy;

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
					ref.sources[source].readable = read;
				}

				void WriteEnable(bool write) const
				{
					ref.sources[source].writable = write;
				}

				void SetSecurity(bool read,bool write) const
				{
					ref.sources[source].readable = read;
					ref.sources[source].writable = write;
				}

				ibool IsReadable() const
				{
					return ref.sources[source].readable;
				}

				ibool IsWritable() const
				{
					return ref.sources[source].writable;
				}

				const SourceProxy& Set(dword size,bool read,bool write) const
				{
					ref.sources[source].Set( size );
					ref.sources[source].readable = read;
					ref.sources[source].writable = write;
					return *this;
				}

				const SourceProxy& Set(u8* mem,dword size,bool read,bool write) const
				{
					ref.sources[source].Set( mem, size );
					ref.sources[source].readable = read;
					ref.sources[source].writable = write;
					return *this;
				}

				void Remove() const
				{
					ref.sources[source].Destroy();
				}

				void Fill(int value) const
				{
					ref.sources[source].Fill( value );
				}

				void Clear() const
				{
					ref.sources[source].Fill( 0x00 );
				}

				u8* Mem() const
				{
					return ref.sources[source].mem;
				}

				u8* Mem(dword offset) const
				{
					return ref.sources[source].mem + (offset & ref.sources[source].mask);
				}

				dword Size() const
				{
					return ref.sources[source].size;
				}

				ibool Internal() const
				{
					return ref.sources[source].internal;
				}

				bool Empty() const
				{
					return ref.sources[source].size == 0;
				}
			};

			class ConstSourceProxy
			{
				const BaseMemory::Linear& ref;

			public:

				ConstSourceProxy(const BaseMemory::Linear& r)
				: ref(r) {}

				const u8* Mem() const
				{
					return ref.mem;
				}

				const u8* Mem(dword offset) const
				{
					return ref.mem + (offset & ref.mask);
				}

				dword Size() const
				{
					return ref.size;
				}

				ibool Internal() const
				{
					return ref.internal;
				}

				bool Empty() const
				{
					return ref.size == 0;
				}

				ibool IsReadable() const
				{
					return ref.readable;
				}

				ibool IsWritable() const
				{
					return ref.writable;
				}
			};

		public:

			const SourceProxy Source(uint i=0)
			{
				NST_ASSERT( i < NUM_SOURCES );
				return SourceProxy( i, *this );
			}

			const ConstSourceProxy Source(uint i=0) const
			{
				NST_ASSERT( i < NUM_SOURCES );
				return ConstSourceProxy( sources[i] );
			}

			Memory()
			{
			}

			Memory(dword size,bool read,bool write)
			{
				Source().Set( size, read, write );
			}

			Memory(u8* mem,dword size,bool read,bool write)
			{
				Source().Set( mem, size, read, write );
			}

			template<uint SIZE,uint ADDRESS>
			dword GetBank() const
			{
				NST_COMPILE_ASSERT( SIZE && (SIZE % MEM_PAGE_SIZE == 0) && (SPACE >= ADDRESS + SIZE) );

				enum {MEM_PAGE = ADDRESS >> MEM_PAGE_SHIFT};
				return dword(pages.mem[MEM_PAGE] - sources[pages.ref[MEM_PAGE]].mem) >> GetBlockShift<SIZE>::VALUE;
			}

			template<uint SIZE>
			dword GetBank(uint address) const
			{
				NST_COMPILE_ASSERT( SIZE && (SIZE % MEM_PAGE_SIZE == 0) );
				NST_ASSERT( SPACE >= address + SIZE );

				address >>= MEM_PAGE_SHIFT;
				return dword(pages.mem[address] - sources[pages.ref[address]].mem) >> GetBlockShift<SIZE>::VALUE;
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
				MEM_PAGE_END   = MEM_PAGE_BEGIN + (SIZE / MEM_PAGE_SIZE)
			};

			bank <<= MEM_OFFSET;

			for (uint i=MEM_PAGE_BEGIN; i < MEM_PAGE_END; ++i, bank += MEM_PAGE_SIZE)
			{
				pages.mem[i] = sources[0].mem + (bank & sources[0].mask);
				pages.ref[i] = 0;
			}
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

			bank <<= MEM_OFFSET;
			address >>= MEM_PAGE_SHIFT;

			for (uint i=0; i < MEM_PAGE_COUNT; ++i, bank += MEM_PAGE_SIZE)
			{
				pages.mem[address+i] = sources[0].mem + (bank & sources[0].mask);
				pages.ref[address+i] = 0;
			}
		}

		template<dword SPACE,uint U,uint V> template<uint SIZE,uint ADDRESS>
		void Memory<SPACE,U,V>::SwapBanks(dword bank0,dword bank1)
		{
			NST_COMPILE_ASSERT( (SPACE >= ADDRESS + SIZE * 2) && SIZE && (SIZE % MEM_PAGE_SIZE == 0) );

			enum
			{
				MEM_OFFSET = GetBlockShift<SIZE>::VALUE,
				MEM_PAGE_BEGIN = ADDRESS / MEM_PAGE_SIZE,
				MEM_PAGE_END = MEM_PAGE_BEGIN + (SIZE / MEM_PAGE_SIZE),
				BANK_0 = SIZE / MEM_PAGE_SIZE * 0,
				BANK_1 = SIZE / MEM_PAGE_SIZE * 1
			};

			bank0 <<= MEM_OFFSET;
			bank1 <<= MEM_OFFSET;

			for (uint i=MEM_PAGE_BEGIN; i < MEM_PAGE_END; ++i)
			{
				pages.mem[BANK_0 + i] = sources[0].mem + (bank0 & sources[0].mask); bank0 += MEM_PAGE_SIZE;
				pages.ref[BANK_0 + i] = 0;
				pages.mem[BANK_1 + i] = sources[0].mem + (bank1 & sources[0].mask); bank1 += MEM_PAGE_SIZE;
				pages.ref[BANK_1 + i] = 0;
			}
		}

		template<dword SPACE,uint U,uint V> template<uint SIZE,uint ADDRESS>
		void Memory<SPACE,U,V>::SwapBanks(dword bank0,dword bank1,dword bank2,dword bank3)
		{
			NST_COMPILE_ASSERT( (SPACE >= ADDRESS + SIZE * 4) && SIZE && (SIZE % MEM_PAGE_SIZE == 0) );

			enum
			{
				MEM_OFFSET = GetBlockShift<SIZE>::VALUE,
				MEM_PAGE_BEGIN = ADDRESS / MEM_PAGE_SIZE,
				MEM_PAGE_END = MEM_PAGE_BEGIN + (SIZE / MEM_PAGE_SIZE),
				BANK_0 = SIZE / MEM_PAGE_SIZE * 0,
				BANK_1 = SIZE / MEM_PAGE_SIZE * 1,
				BANK_2 = SIZE / MEM_PAGE_SIZE * 2,
				BANK_3 = SIZE / MEM_PAGE_SIZE * 3
			};

			bank0 <<= MEM_OFFSET;
			bank1 <<= MEM_OFFSET;
			bank2 <<= MEM_OFFSET;
			bank3 <<= MEM_OFFSET;

			for (uint i=MEM_PAGE_BEGIN; i < MEM_PAGE_END; ++i)
			{
				pages.mem[BANK_0 + i] = sources[0].mem + (bank0 & sources[0].mask); bank0 += MEM_PAGE_SIZE;
				pages.ref[BANK_0 + i] = 0;
				pages.mem[BANK_1 + i] = sources[0].mem + (bank1 & sources[0].mask); bank1 += MEM_PAGE_SIZE;
				pages.ref[BANK_1 + i] = 0;
				pages.mem[BANK_2 + i] = sources[0].mem + (bank2 & sources[0].mask); bank2 += MEM_PAGE_SIZE;
				pages.ref[BANK_2 + i] = 0;
				pages.mem[BANK_3 + i] = sources[0].mem + (bank3 & sources[0].mask); bank3 += MEM_PAGE_SIZE;
				pages.ref[BANK_3 + i] = 0;
			}
		}

		template<dword SPACE,uint U,uint V> template<uint SIZE>
		void Memory<SPACE,U,V>::SwapBanks(uint address,dword bank0,dword bank1)
		{
			NST_COMPILE_ASSERT( SIZE && (SIZE % MEM_PAGE_SIZE == 0) );
			NST_ASSERT( SPACE >= address + SIZE * 2 );

			enum
			{
				MEM_OFFSET = GetBlockShift<SIZE>::VALUE,
				MEM_PAGE_COUNT = SIZE / MEM_PAGE_SIZE,
				BANK_0 = SIZE / MEM_PAGE_SIZE * 0,
				BANK_1 = SIZE / MEM_PAGE_SIZE * 1
			};

			bank0 <<= MEM_OFFSET;
			bank1 <<= MEM_OFFSET;
			address >>= MEM_PAGE_SHIFT;

			for (uint i=0; i < MEM_PAGE_COUNT; ++i)
			{
				pages.mem[BANK_0 + address + i] = sources[0].mem + (bank0 & sources[0].mask); bank0 += MEM_PAGE_SIZE;
				pages.ref[BANK_0 + address + i] = 0;
				pages.mem[BANK_1 + address + i] = sources[0].mem + (bank1 & sources[0].mask); bank1 += MEM_PAGE_SIZE;
				pages.ref[BANK_1 + address + i] = 0;
			}
		}

		template<dword SPACE,uint U,uint V> template<uint SIZE>
		void Memory<SPACE,U,V>::SwapBanks(uint address,dword bank0,dword bank1,dword bank2,dword bank3)
		{
			NST_COMPILE_ASSERT( SIZE && (SIZE % MEM_PAGE_SIZE == 0) );
			NST_ASSERT( SPACE >= address + SIZE * 4 );

			enum
			{
				MEM_OFFSET = GetBlockShift<SIZE>::VALUE,
				MEM_PAGE_COUNT = SIZE / MEM_PAGE_SIZE,
				BANK_0 = SIZE / MEM_PAGE_SIZE * 0,
				BANK_1 = SIZE / MEM_PAGE_SIZE * 1,
				BANK_2 = SIZE / MEM_PAGE_SIZE * 2,
				BANK_3 = SIZE / MEM_PAGE_SIZE * 3
			};

			bank0 <<= MEM_OFFSET;
			bank1 <<= MEM_OFFSET;
			bank2 <<= MEM_OFFSET;
			bank3 <<= MEM_OFFSET;
			address >>= MEM_PAGE_SHIFT;

			for (uint i=0; i < MEM_PAGE_COUNT; ++i)
			{
				pages.mem[BANK_0 + address + i] = sources[0].mem + (bank0 & sources[0].mask); bank0 += MEM_PAGE_SIZE;
				pages.ref[BANK_0 + address + i] = 0;
				pages.mem[BANK_1 + address + i] = sources[0].mem + (bank1 & sources[0].mask); bank1 += MEM_PAGE_SIZE;
				pages.ref[BANK_1 + address + i] = 0;
				pages.mem[BANK_2 + address + i] = sources[0].mem + (bank2 & sources[0].mask); bank2 += MEM_PAGE_SIZE;
				pages.ref[BANK_2 + address + i] = 0;
				pages.mem[BANK_3 + address + i] = sources[0].mem + (bank3 & sources[0].mask); bank3 += MEM_PAGE_SIZE;
				pages.ref[BANK_3 + address + i] = 0;
			}
		}

		template<dword SPACE,uint U,uint V> template<uint SIZE,uint ADDRESS>
		void Memory<SPACE,U,V>::SourceProxy::SwapBank(dword bank) const
		{
			NST_COMPILE_ASSERT( (SPACE >= ADDRESS + SIZE) && SIZE && (SIZE % MEM_PAGE_SIZE == 0) );

			enum
			{
				MEM_OFFSET = GetBlockShift<SIZE>::VALUE,
				MEM_PAGE_COUNT = SIZE / MEM_PAGE_SIZE,
				MEM_PAGE_BEGIN = ADDRESS / MEM_PAGE_SIZE,
				MEM_PAGE_END   = MEM_PAGE_BEGIN + MEM_PAGE_COUNT,
				WRITE_MASK = uint(GetBlockMask<MEM_PAGE_COUNT>::VALUE) << MEM_PAGE_BEGIN
			};

			bank <<= MEM_OFFSET;

			for (uint i=MEM_PAGE_BEGIN; i < MEM_PAGE_END; ++i, bank += MEM_PAGE_SIZE)
			{
				ref.pages.mem[i] = ref.sources[source].mem + (bank & ref.sources[source].mask);
				ref.pages.ref[i] = source;
			}
		}

		template<dword SPACE,uint U,uint V> template<uint SIZE>
		void Memory<SPACE,U,V>::SourceProxy::SwapBank(uint address,dword bank) const
		{
			NST_COMPILE_ASSERT( SIZE && (SIZE % MEM_PAGE_SIZE == 0) );
			NST_ASSERT( SPACE >= address + SIZE );

			enum
			{
				MEM_OFFSET = GetBlockShift<SIZE>::VALUE,
				MEM_PAGE_COUNT = SIZE / MEM_PAGE_SIZE,
				WRITE_MASK = GetBlockMask<MEM_PAGE_COUNT>::VALUE
			};

			address >>= MEM_PAGE_SHIFT;
			bank <<= MEM_OFFSET;

			for (uint i=0; i < MEM_PAGE_COUNT; ++i, bank += MEM_PAGE_SIZE)
			{
				ref.pages.mem[address+i] = ref.sources[source].mem + (bank & ref.sources[source].mask);
				ref.pages.ref[address+i] = source;
			}
		}

		template<dword SPACE,uint U,uint V>
		void Memory<SPACE,U,V>::SaveState(State::Saver& state,const uint sourceMask) const
		{
			{
				u8 data[NUM_SOURCES];

				for (uint i=0; i < NUM_SOURCES; ++i)
					data[i] = (sources[i].readable ? 0x1 : 0x0) | (sources[i].writable ? 0x2 : 0x0);

				state.Begin('A','C','C','\0').Write( data ).End();
			}

			for (uint i=0; i < NUM_SOURCES; ++i)
			{
				if (sourceMask & (1U << i))
					state.Begin('R','M','0'+i,'\0').Compress( sources[i].mem, sources[i].size ).End();
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
							sources[i].readable = (data[i] & 0x1) >> 0;
							sources[i].writable = (data[i] & 0x2) >> 1;
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
									state.Uncompress( sources[i].mem, sources[i].size );

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
