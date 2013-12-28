////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
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

#ifndef NST_IMAGEDATABASE_H
#define NST_IMAGEDATABASE_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

#include "api/NstApiCartridge.hpp"

namespace Nes
{
	namespace Core
	{
		class ImageDatabase
		{
		public:

			ImageDatabase();
			~ImageDatabase();

			Result Load(StdStream);
			void Unload();

			typedef const void* Handle;

			Handle GetHandle(dword) const;
			Api::Cartridge::System GetSystem(Handle) const;

		private:

            #ifdef _MSC_VER
            #pragma pack(push,1)
            #endif

			struct Entry
			{
				enum
				{
					FLAGS_PAL       = 0x001,
					FLAGS_NTSC      = 0x002,
					FLAGS_VS        = 0x004,
					FLAGS_P10       = 0x008,
					FLAGS_MIRRORING = 0x070,
					FLAGS_BATTERY   = 0x080,
					FLAGS_TRAINER   = 0x100,
					FLAGS_BAD       = 0x200,
					FLAGS_MIRRORING_SHIFT = 4
				};

				u32 crc;
				u32 pRomCrc;
				u8 pRomSize;
				u8 cRomSize;
				u8 wRamSize;
				u8 mapper;
				u16 flags;

				operator u32 () const
				{ 
					return crc; 
				}
			};

            #ifdef _MSC_VER
            #pragma pack(pop)
            #endif

			typedef const Entry* Ref;

			ibool enabled;
			dword numEntries;
			Entry* entries;

		public:

			void Enable(bool state=true)
			{
				enabled = state;
			}

			ibool Enabled() const
			{
				return enabled;
			}

			Api::Cartridge::Mirroring GetMirroring(Handle h) const 
			{ 
				return (Api::Cartridge::Mirroring) ((static_cast<Ref>(h)->flags & Entry::FLAGS_MIRRORING) >> Entry::FLAGS_MIRRORING_SHIFT);
			}

			dword Crc        (Handle h) const { return static_cast<Ref>(h)->crc;                          }
			dword pRomCrc    (Handle h) const { return static_cast<Ref>(h)->pRomCrc;                      }
			dword pRomSize   (Handle h) const { return static_cast<Ref>(h)->pRomSize * SIZE_16K;          }
			dword cRomSize   (Handle h) const { return static_cast<Ref>(h)->cRomSize * SIZE_8K;           }
			dword wRamSize   (Handle h) const { return static_cast<Ref>(h)->wRamSize * SIZE_8K;           }	
			uint  Mapper     (Handle h) const { return static_cast<Ref>(h)->mapper;                       }
			ibool HasBattery (Handle h) const { return static_cast<Ref>(h)->flags & Entry::FLAGS_BATTERY; }
			ibool HasTrainer (Handle h) const { return static_cast<Ref>(h)->flags & Entry::FLAGS_TRAINER; }
			ibool IsBad      (Handle h) const { return static_cast<Ref>(h)->flags & Entry::FLAGS_BAD;     }
		};
	}
}

#endif
