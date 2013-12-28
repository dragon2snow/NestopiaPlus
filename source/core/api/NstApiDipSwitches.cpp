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

#include "../NstCore.hpp"
#include "../NstDipSwitches.hpp"
#include "../NstImage.hpp"
#include "NstApiEmulator.hpp"
#include "NstApiDipSwitches.hpp"

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("s", on)
#endif

namespace Nes
{
	namespace Api
	{
		Core::DipSwitches* DipSwitches::Query() const
		{
			if (emulator.image)
			{
				if (Core::Image::ExternalDevice device = emulator.image->QueryExternalDevice( Core::Image::EXT_DIP_SWITCHES ))
					return static_cast<Core::DipSwitches*>(device);
			}

			return NULL;
		}

		uint DipSwitches::NumDips() const
		{
			if (Core::DipSwitches* const dipSwitches = Query())
				return dipSwitches->NumDips();

			return 0;
		}

		uint DipSwitches::NumValues(uint dip) const
		{
			if (Core::DipSwitches* const dipSwitches = Query())
			{
				if (dipSwitches->NumDips() > dip)
					return dipSwitches->NumValues( dip );
			}

			return 0;
		}

		const char* DipSwitches::GetDipName(uint dip) const
		{
			if (Core::DipSwitches* const dipSwitches = Query())
			{
				if (dipSwitches->NumDips() > dip)
					return dipSwitches->GetDipName( dip );
			}

			return NULL;
		}

		const char* DipSwitches::GetValueName(uint dip,uint value) const
		{
			if (Core::DipSwitches* const dipSwitches = Query())
			{
				if (dipSwitches->NumDips() > dip && dipSwitches->NumValues( dip ) > value)
					return dipSwitches->GetValueName( dip, value );

			}

			return NULL;
		}

		int DipSwitches::GetValue(uint dip) const
		{
			if (Core::DipSwitches* const dipSwitches = Query())
			{
				if (dipSwitches->NumDips() > dip)
					return dipSwitches->GetValue( dip );
			}

			return INVALID;
		}

		Result DipSwitches::SetValue(uint dip,uint value)
		{
			if (Core::DipSwitches* const dipSwitches = Query())
			{
				if (dipSwitches->NumDips() > dip && dipSwitches->NumValues( dip ) > value)
					return dipSwitches->SetValue( dip, value ) ? RESULT_OK : RESULT_NOP;
			}

			return RESULT_ERR_NOT_READY;
		}
	}
}

#ifdef NST_PRAGMA_OPTIMIZE
#pragma optimize("", on)
#endif
