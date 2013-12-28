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

#include <new>
#include <algorithm>
#include "NstStream.hpp"
#include "NstImageDatabase.hpp"

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("s", on)
#endif

namespace Nes
{
	namespace Core
	{
		NST_COMPILE_ASSERT
		(
			Api::Cartridge::MIRROR_HORIZONTAL == 0 &&
			Api::Cartridge::MIRROR_VERTICAL   == 1 &&
			Api::Cartridge::MIRROR_FOURSCREEN == 2 &&
			Api::Cartridge::MIRROR_ZERO       == 3 &&
			Api::Cartridge::MIRROR_ONE        == 4 &&
			Api::Cartridge::MIRROR_CONTROLLED == 5
		);

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
			Stream::In stream( input );

			try
			{
				numEntries = stream.Read32() & 0xFFFF;

				if (!numEntries)
					throw RESULT_ERR_CORRUPT_FILE;

				Entry* NST_RESTRICT it = new Entry [numEntries];
				entries = it;

				for (Ref const end = it + numEntries; it != end; ++it)
				{
					u8 data[12];
					stream.Read( data );

					it->crc     = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
					it->prgSize = data[4];
					it->prgSkip = data[5];
					it->chrSize = data[6];
					it->chrSkip = data[7];
					it->wrkSize = data[8];
					it->mapper  = data[9];
					it->flags   = data[10] | (data[11] << 8);
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

		ImageDatabase::Handle ImageDatabase::GetHandle(const dword crc) const
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

		Api::Cartridge::System ImageDatabase::GetSystem(Handle h) const
		{
			const uint flags = static_cast<Ref>(h)->flags;

			Api::Cartridge::System system;

			if (flags & Entry::FLAGS_P10)
			{
				system = Api::Cartridge::SYSTEM_PC10;
			}
			else if (flags & Entry::FLAGS_VS)
			{
				system = Api::Cartridge::SYSTEM_VS;
			}
			else if (flags & Entry::FLAGS_PAL)
			{
				system = (flags & Entry::FLAGS_NTSC) ? Api::Cartridge::SYSTEM_NTSC_PAL : Api::Cartridge::SYSTEM_PAL;
			}
			else
			{
				system = Api::Cartridge::SYSTEM_NTSC;
			}

			return system;
		}
	}
}

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("", on)
#endif
