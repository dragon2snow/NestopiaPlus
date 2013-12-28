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

			Handle Search(dword) const;

			dword WrkRam       (Handle) const;
			dword WrkRamBacked (Handle) const;
			dword ChrRam       (Handle) const;

			Api::Cartridge::Condition Condition (Handle) const;

			System GetSystem(Handle) const;
			Region GetRegion(Handle) const;

			enum
			{
				INPUT_LIGHTGUN = 1,
				INPUT_LIGHTGUN_VS,
				INPUT_POWERPAD,
				INPUT_FAMILYTRAINER,
				INPUT_PADDLE_NES,
				INPUT_PADDLE_FAMICOM,
				INPUT_ADAPTER_NES,
				INPUT_ADAPTER_FAMICOM,
				INPUT_SUBORKEYBOARD,
				INPUT_FAMILYKEYBOARD,
				INPUT_PARTYTAP,
				INPUT_CRAZYCLIMBER,
				INPUT_EXCITINGBOXING,
				INPUT_HYPERSHOT,
				INPUT_POKKUNMOGURAA,
				INPUT_OEKAKIDS,
				INPUT_MAHJONG,
				INPUT_TOPRIDER,
				INPUT_PAD_SWAP,
				INPUT_ROB,

				INPUT_EX_TURBOFILE = 1
			};

		private:

			#ifdef _MSC_VER
			#pragma pack(push,1)
			#endif

			struct Entry
			{
				enum
				{
					FLAGS_PAL       = 0x0001U,
					FLAGS_NTSC      = 0x0002U,
					FLAGS_VS        = 0x0004U,
					FLAGS_P10       = 0x0008U,
					FLAGS_MIRRORING = 0x0070U,
					FLAGS_TRAINER   = 0x0100U,
					FLAGS_BAD       = 0x0200U,
					FLAGS_ENCRYPTED = 0x0800U,

					FLAGS_MIRRORING_SHIFT = 4,

					INPUT_BITS = 0x1F,
					INPUT_EX_SHIFT = 5
				};

				u32 crc;
				u8 prgSize;
				u8 prgSkip;
				u8 chrSize;
				u8 chrSkip;
				u8 wrkSize;
				u8 mapper;
				u8 attribute;
				u8 input;
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
			Ref entries;

			static const u32 ramLut[16];

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

			dword Crc          (Handle h) const { return static_cast<Ref>(h)->crc;                            }
			dword PrgRom       (Handle h) const { return static_cast<Ref>(h)->prgSize * SIZE_16K;             }
			dword PrgRomSkip   (Handle h) const { return static_cast<Ref>(h)->prgSkip * SIZE_16K;             }
			dword ChrRom       (Handle h) const { return static_cast<Ref>(h)->chrSize * SIZE_8K;              }
			dword ChrRomSkip   (Handle h) const { return static_cast<Ref>(h)->chrSkip * SIZE_8K;              }
			dword ChrRamBacked (Handle)   const { return 0;                                                   }
			uint  Mapper       (Handle h) const { return static_cast<Ref>(h)->mapper;                         }
			uint  Attribute    (Handle h) const { return static_cast<Ref>(h)->attribute;                      }
			uint  Input        (Handle h) const { return static_cast<Ref>(h)->input & Entry::INPUT_BITS;      }
			uint  InputEx      (Handle h) const { return static_cast<Ref>(h)->input >> Entry::INPUT_EX_SHIFT; }
			ibool Trainer      (Handle h) const { return static_cast<Ref>(h)->flags & Entry::FLAGS_TRAINER;   }
			ibool Encrypted    (Handle h) const { return static_cast<Ref>(h)->flags & Entry::FLAGS_ENCRYPTED; }
		};
	}
}

#endif
