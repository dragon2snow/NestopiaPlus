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

#ifndef NST_IO_IPS_H
#define NST_IO_IPS_H

#pragma once

#include "resource/resource.h"
#include "NstObjectRaw.hpp"
#include "NstCollectionVector.hpp"

namespace Nestopia
{
	namespace Io
	{
		class Ips : Sealed
		{
		public:

			enum
			{
				MAX_LENGTH = 0xFFFFFF
			};

			enum Exception
			{
				ERR_CORRUPT = IDS_FILE_ERR_CORRUPT,
				ERR_EMPTY = IDS_FILE_ERR_EMPTY
			};

			typedef const Object::Raw TargetData;
			typedef const Object::ConstRaw SourceData;
			typedef Collection::Vector<u8> PatchData;

			static void Create(SourceData,SourceData,PatchData&);

			void Parse (SourceData);
			void Patch (TargetData) const;

		private:

			enum
			{
				DATA_ID1    = 0x504154,
				DATA_ID2    = 0x4348,
				DATA_EOF    = 0x454F46,
				MIN_EQUAL   = 5,
				MIN_BEG_RUN = 9,
				MIN_MID_RUN = 13,
				MIN_END_RUN = 9,
				MAX_BLOCK   = 0xFFFF
			};

			struct Block
			{
				uint offset;
				uint length;
				u8* data;
				uint fill;
			};

			struct Blocks : Collection::Vector<Block>
			{
				~Blocks();
			};

			Blocks blocks;

		public:

			ibool Loaded() const
			{
				return blocks.Size() > 0;
			}

			void Reset()
			{
				blocks.Clear();
			}
		};
	}
}

#endif
