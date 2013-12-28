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

#include "../NstMapper.hpp"
#include "../NstDipSwitches.hpp"
#include "../NstLog.hpp"
#include "NstMapper000.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		class Mapper0::CartSwitch : public DipSwitches
		{
		public:

			void Flush() const;

		private:

			Wrk& wrk;
			bool init;

			uint NumDips() const
			{
				return 1;
			}

			uint NumValues(uint) const
			{
				return 2;
			}

			cstring GetDipName(uint) const
			{
				return "Backup Switch";
			}

			cstring GetValueName(uint,uint value) const
			{
				NST_ASSERT( value < 2 );
				return value == 0 ? "Off" : "On";
			}

			uint GetValue(uint) const
			{
				return wrk.Source().Writable() ? 0 : 1;
			}

			void SetValue(uint,uint value)
			{
				NST_ASSERT( value < 2 );
				wrk.Source().SetSecurity( true, !value );
			}

		public:

			explicit CartSwitch(Wrk& w)
			: wrk(w), init(true) {}

			void Reset(bool hard)
			{
				if (init)
				{
					init = false;
				}
				else if (hard)
				{
					Flush();
				}
			}
		};

		void Mapper0::CartSwitch::Flush() const
		{
			if (wrk.Source().Writable())
			{
				wrk.Source().Fill( 0x00 );
				Log::Flush( "Mapper0: battery-switch OFF, discarding W-RAM.." NST_LINEBREAK );
			}
		}

		dword Mapper0::DetectWRam(const Context& c)
		{
			if (c.attribute != ATR_BACKUP_SWITCH)
			{
				return WRAM_DEFAULT;
			}
			else if (c.wrk.Size() == SIZE_2K)
			{
				return WRAM_2K;
			}
			else if (c.wrk.Size() == SIZE_4K)
			{
				return WRAM_4K;
			}
			else
			{
				return WRAM_8K;
			}
		}

		Mapper0::Mapper0(Context& c)
		:
		Mapper     (c,(PROM_MAX_32K|CROM_MAX_8K) | DetectWRam(c) ),
		cartSwitch (c.attribute == ATR_BACKUP_SWITCH ? new CartSwitch(wrk) : NULL)
		{}

		Mapper0::~Mapper0()
		{
			delete cartSwitch;
		}

		void Mapper0::SubReset(bool hard)
		{
			if (cartSwitch)
			{
				cartSwitch->Reset( hard );

				switch (wrk.Size())
				{
					case SIZE_8K:

						Map( 0x6000U, 0x7FFFU, &Mapper0::Peek_Wrk_6, &Mapper0::Poke_Wrk_6 );
						break;

					case SIZE_4K:

						Map( 0x6000U, 0x7000U, &Mapper0::Peek_Wrk_6, &Mapper0::Poke_Wrk_6 );
						break;

					case SIZE_2K:

						Map( 0x7000U, 0x7800U, &Mapper0::Peek_Wrk_7, &Mapper0::Poke_Wrk_7 );
						break;
				}
			}
		}

		void Mapper0::Sync(Event event,Input::Controllers*)
		{
			if (event == EVENT_POWER_OFF)
			{
				if (cartSwitch)
					cartSwitch->Flush();
			}
		}

		Mapper0::Device Mapper0::QueryDevice(DeviceType type)
		{
			if (type == DEVICE_DIP_SWITCHES && cartSwitch)
				return cartSwitch;
			else
				return Mapper::QueryDevice( type );
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_POKE_AD(Mapper0,Wrk_6)
		{
			NST_VERIFY( wrk.Writable(0) );

			if (wrk.Writable(0))
				wrk[0][address - 0x6000] = data;
		}

		NES_PEEK_A(Mapper0,Wrk_6)
		{
			return wrk[0][address - 0x6000];
		}

		NES_POKE_AD(Mapper0,Wrk_7)
		{
			NST_VERIFY( wrk.Writable(0) );

			if (wrk.Writable(0))
				wrk[0][address - 0x7000] = data;
		}

		NES_PEEK_A(Mapper0,Wrk_7)
		{
			return wrk[0][address - 0x7000];
		}
	}
}
