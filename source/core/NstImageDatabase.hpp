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

#ifndef NST_IMAGEDATABASE_H
#define NST_IMAGEDATABASE_H

#include "api/NstApiCartridge.hpp"

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		class ImageDatabase
		{
			typedef Api::Cartridge Cart;

		public:

			ImageDatabase();
			~ImageDatabase();

			Result Load(StdStream);
			void Unload();

			typedef const void* Handle;

			Handle Search(dword) const;

			Cart::System GetSystem(Handle) const;
			Cart::Region GetRegion(Handle) const;
			Cart::Condition GetCondition(Handle) const;

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
				INPUT_POWERGLOVE,

				INPUT_EX_TURBOFILE = 1
			};

		private:

			#if NST_MSVC
			#pragma pack(push,1)
			#endif

			struct Entry
			{
				enum
				{
					FLAGS_PAL       = 0x0001,
					FLAGS_NTSC      = 0x0002,
					FLAGS_VS        = 0x0004,
					FLAGS_P10       = 0x0008,
					FLAGS_MIRRORING = 0x0070,
					FLAGS_TRAINER   = 0x0100,
					FLAGS_BAD       = 0x0200,
					FLAGS_PRG_HI    = 0x0400,
					FLAGS_ENCRYPTED = 0x0800,

					FLAGS_MIRRORING_SHIFT = 4,
					FLAGS_PRG_HI_SHIFT = 2,

					INPUT_BITS = 0x1F,
					INPUT_EX_SHIFT = 5
				};

				dword crc;
				byte prgSize;
				byte prgSkip;
				byte chrSize;
				byte chrSkip;
				byte wrkSize;
				byte mapper;
				byte attribute;
				byte input;
				word flags;

				operator dword () const
				{
					return crc;
				}
			};

			#if NST_MSVC
			#pragma pack(pop)
			#endif

			typedef const Entry* Ref;

			ibool enabled;
			dword numEntries;
			Ref entries;

			static const dword ramSizes[16];

		public:

			void Enable(bool state=true)
			{
				enabled = state;
			}

			bool Enabled() const
			{
				return enabled;
			}

			dword PrgRom(Handle h) const
			{
				return (static_cast<Ref>(h)->prgSize | ((static_cast<Ref>(h)->flags & uint(Entry::FLAGS_PRG_HI)) >> Entry::FLAGS_PRG_HI_SHIFT)) * dword(SIZE_16K);
			}

			Api::Cartridge::Mirroring GetMirroring(Handle h) const
			{
				return static_cast<Cart::Mirroring>(((static_cast<Ref>(h)->flags & uint(Entry::FLAGS_MIRRORING)) >> Entry::FLAGS_MIRRORING_SHIFT));
			}

			dword Crc          (Handle h) const { return static_cast<Ref>(h)->crc;                                  }
			dword PrgRomSkip   (Handle h) const { return static_cast<Ref>(h)->prgSkip * dword(SIZE_16K);            }
			dword ChrRom       (Handle h) const { return static_cast<Ref>(h)->chrSize * dword(SIZE_8K);             }
			dword ChrRomSkip   (Handle h) const { return static_cast<Ref>(h)->chrSkip * dword(SIZE_8K);             }
			dword ChrRam       (Handle h) const { return static_cast<Ref>(h)->chrSize ? 0 : SIZE_8K;                }
			dword ChrRamBacked (Handle)   const { return 0;                                                         }
			dword WrkRam       (Handle h) const { return ramSizes[static_cast<Ref>(h)->wrkSize >> 4];               }
			dword WrkRamBacked (Handle h) const { return ramSizes[static_cast<Ref>(h)->wrkSize & 0xFU];             }
			uint  Mapper       (Handle h) const { return static_cast<Ref>(h)->mapper;                               }
			uint  Attribute    (Handle h) const { return static_cast<Ref>(h)->attribute;                            }
			uint  Input        (Handle h) const { return static_cast<Ref>(h)->input & uint(Entry::INPUT_BITS);      }
			uint  InputEx      (Handle h) const { return static_cast<Ref>(h)->input >> Entry::INPUT_EX_SHIFT;       }
			bool  Trainer      (Handle h) const { return static_cast<Ref>(h)->flags & uint(Entry::FLAGS_TRAINER);   }
			bool  Encrypted    (Handle h) const { return static_cast<Ref>(h)->flags & uint(Entry::FLAGS_ENCRYPTED); }
		};
	}
}

#endif
