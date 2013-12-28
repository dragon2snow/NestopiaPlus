////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2006 Martin Freij
//
// Based on: RFC 1321 compliant MD5 implementation
// Copyright (C) 2001-2003 Christophe Devine
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

#ifndef NST_CHECKSUM_MD5_H
#define NST_CHECKSUM_MD5_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include <cstring>

namespace Nes
{
	namespace Core
	{
		namespace Checksum
		{
			namespace Md5
			{
				struct Key
				{
					union
					{
						u8 digest[16];
						u32 block[4];
					};

					Key()
					{
						std::memset( digest, 0, sizeof(digest) );
					}

					bool operator == (const Key& k) const
					{
						return std::memcmp( digest, k.digest, sizeof(digest) ) == 0;
					}

					bool operator != (const Key& k) const
					{
						return !(*this == k);
					}

					bool IsNull() const
					{
						return (block[0] | block[1] | block[2] | block[3]) == 0;
					}
				};

				Key Compute(const void*,ulong);
			}
		}
	}
}

#endif
