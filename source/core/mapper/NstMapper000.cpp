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

#include "../NstMapper.hpp"
#include "../NstDipSwitches.hpp"
#include "../NstLog.hpp"
#include "NstMapper000.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		class Mapper0::CartSwitch : public DipSwitches
		{
			Wrk& wrk;

			uint NumDips() const
			{
				return 1;
			}

			uint NumValues(uint dip) const
			{
				NST_ASSERT( dip == 0 );
				return 2;
			}

			cstring GetDipName(uint dip) const
			{
				NST_ASSERT( dip == 0 );
				return "Backup Switch";
			}

			cstring GetValueName(uint dip,uint value) const
			{
				NST_ASSERT( dip == 0 && value < 2 );
				return value == 0 ? "Off" : "On";
			}

			uint GetValue(uint dip) const
			{
				NST_ASSERT( dip == 0 );
				return wrk.Source().Writable() ? 0 : 1;
			}

			bool SetValue(uint dip,uint value)
			{
				NST_ASSERT( dip == 0 && value < 2 );

				if (bool(wrk.Source().Writable()) != !value)
				{
					wrk.Source().SetSecurity( true, !value );
					return true;
				}

				return false;
			}

		public:

			CartSwitch(Wrk& w)
			: wrk(w) {}

			void Flush(bool power) const
			{
				if (wrk.Source().Writable() && !power)
				{
					wrk.Source().Fill( 0x00 );
					Log::Flush( "Mapper0: battery-switch OFF, discarding data!" NST_LINEBREAK );
				}
			}

			static bool IsBasic(const dword crc)
			{
				switch (crc)
				{
					case 0x868FCD89UL: // Family BASIC v1.0
					case 0xF9DEF527UL: // Family BASIC v2.0a
					case 0xDE34526EUL: // Family BASIC v2.1a
					case 0x3AAEED3FUL: // Family BASIC v3.0
					case 0xF050B611UL: // -||-
					case 0xDA03D908UL: // Playbox BASIC v1.0
					case 0x2D6B7E5AUL: // Playbox BASIC Prototype
						return true;
				}

				return false;
			}
		};

		Mapper0::Mapper0(Context& c)
		:
		Mapper     (c,CartSwitch::IsBasic(c.prgCrc) ? WRAM_8K : 0),
		cartSwitch (CartSwitch::IsBasic(c.prgCrc) ? new CartSwitch(wrk) : NULL)
		{}

		Mapper0::~Mapper0()
		{
			delete cartSwitch;
		}

		void Mapper0::SubReset(bool)
		{
			if (cartSwitch)
				Map( WRK_PEEK_POKE_BUS );
		}

		void Mapper0::Flush(bool power)
		{
			if (cartSwitch)
				cartSwitch->Flush( power );
		}

		Mapper0::Device Mapper0::QueryDevice(DeviceType type)
		{
			if (type == DEVICE_DIP_SWITCHES && cartSwitch)
				return cartSwitch;
			else
				return Mapper::QueryDevice( type );
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
