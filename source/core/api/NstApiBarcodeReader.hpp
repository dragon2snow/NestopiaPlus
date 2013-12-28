////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2008 Martin Freij
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

#ifndef NST_API_BARCODEREADER_H
#define NST_API_BARCODEREADER_H

#include "NstApi.hpp"

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

#if NST_ICC >= 810
#pragma warning( push )
#pragma warning( disable : 444 )
#elif NST_MSVC >= 1200
#pragma warning( push )
#pragma warning( disable : 4512 )
#endif

namespace Nes
{
	namespace Core
	{
		class BarcodeReader;
	}

	namespace Api
	{
		class BarcodeReader : public Base
		{
			Core::BarcodeReader* Query() const;

		public:

			template<typename T>
			BarcodeReader(T& e)
			: Base(e) {}

			enum
			{
				MIN_DIGITS = 8,
				MAX_DIGITS = 13
			};

			bool IsDigitsSupported(uint) const throw();
			bool CanTransfer() const throw();
			uint Randomize(char (&)[MAX_DIGITS+1]) const throw();
			Result Transfer(const char*,uint) throw();

			bool IsConnected() const
			{
				return Query();
			}
		};
	}
}

#if NST_MSVC >= 1200 || NST_ICC >= 810
#pragma warning( pop )
#endif

#endif
