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

#include <new>
#include <algorithm>
#include "NstStream.hpp"
#include "NstImageDatabase.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		NST_COMPILE_ASSERT
		(
			Api::Cartridge::MIRROR_HORIZONTAL == 0 &&
			Api::Cartridge::MIRROR_VERTICAL   == 1 &&
			Api::Cartridge::MIRROR_FOURSCREEN == 2 &&
			Api::Cartridge::MIRROR_ZERO       == 3 &&
			Api::Cartridge::MIRROR_ONE        == 4 &&
			Api::Cartridge::MIRROR_CONTROLLED == 5
		);

		const dword ImageDatabase::ramSizes[16] =
		{
			0,
			128,
			256,
			512,
			SIZE_1K,
			SIZE_2K,
			SIZE_4K,
			SIZE_8K,
			SIZE_16K,
			SIZE_32K,
			SIZE_64K,
			SIZE_128K,
			SIZE_256K,
			SIZE_512K,
			SIZE_1024K
		};

		ImageDatabase::ImageDatabase()
		:
		enabled    (true),
		numEntries (0),
		entries    (NULL)
		{
		}

		ImageDatabase::~ImageDatabase()
		{
			Unload();
		}

		Result ImageDatabase::Load(StdStream input)
		{
			Unload();

			Result result;

			try
			{
				Stream::In stream( input );

				numEntries = stream.Read32();
				NST_VERIFY( numEntries <= 0xFFFF );

				if (!numEntries)
					throw RESULT_ERR_CORRUPT_FILE;

				numEntries &= 0xFFFF;

				Entry* NST_RESTRICT it = new Entry [numEntries];
				entries = it;

				for (Ref const end = it + numEntries; it != end; ++it)
				{
					byte data[14];
					stream.Read( data );

					it->crc       = data[0] | uint(data[1]) << 8 | dword(data[2]) << 16 | dword(data[3]) << 24;
					it->prgSize   = data[4];
					it->prgSkip   = data[5];
					it->chrSize   = data[6];
					it->chrSkip   = data[7];
					it->wrkSize   = data[8];
					it->mapper    = data[9];
					it->attribute = data[10];
					it->input     = data[11];
					it->flags     = data[12] | uint(data[13]) << 8;
				}

				return RESULT_OK;
			}
			catch (Result r)
			{
				result = r;
			}
			catch (const std::bad_alloc&)
			{
				result = RESULT_ERR_OUT_OF_MEMORY;
			}
			catch (...)
			{
				result = RESULT_ERR_GENERIC;
			}

			Unload();

			return result;
		}

		void ImageDatabase::Unload()
		{
			numEntries = 0;
			delete [] entries;
			entries = NULL;
		}

		ImageDatabase::Handle ImageDatabase::Search(const dword crc) const
		{
			if (Ref entry = entries)
			{
				Ref const end = entry + numEntries;
				entry = std::lower_bound( entry, end, crc );

				if (entry != end && entry->crc == crc)
					return entry;
			}

			return NULL;
		}

		System ImageDatabase::GetSystem(Handle h) const
		{
			const uint flags = static_cast<Ref>(h)->flags;

			if (flags & Entry::FLAGS_VS)
			{
				return SYSTEM_VS;
			}
			else if (flags & Entry::FLAGS_P10)
			{
				return SYSTEM_PC10;
			}
			else
			{
				return SYSTEM_HOME;
			}
		}

		Region ImageDatabase::GetRegion(Handle h) const
		{
			switch (uint(static_cast<Ref>(h)->flags) & (Entry::FLAGS_NTSC|Entry::FLAGS_PAL))
			{
				case (Entry::FLAGS_NTSC|Entry::FLAGS_PAL):
					return REGION_BOTH;

				case Entry::FLAGS_PAL:
					return REGION_PAL;

				default:
					return REGION_NTSC;
			}
		}

		Api::Cartridge::Condition ImageDatabase::GetCondition(Handle h) const
		{
			if (uint(static_cast<Ref>(h)->flags) & Entry::FLAGS_BAD)
			{
				return Api::Cartridge::DUMP_BAD;
			}
			else if ((uint(static_cast<Ref>(h)->prgSkip) | uint(static_cast<Ref>(h)->chrSkip)) && !(uint(static_cast<Ref>(h)->flags) & Entry::FLAGS_P10))
			{
				return Api::Cartridge::DUMP_REPAIRABLE;
			}
			else
			{
				return Api::Cartridge::DUMP_OK;
			}
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
