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

#ifndef _MSC_VER

#include <climits>

#ifndef ULLONG_MAX

#include "NstTypes.hpp"

#ifndef NST_U64_DEFINED

namespace Nes
{
	void u64::Multiply(u64 multiplier)
	{
		u64 multiplicand(*this);
		hi = lo = 0;

		while (multiplicand)
		{
			if (multiplicand.lo & 0x1)
				(*this) += multiplier;

			multiplicand.lo = (multiplicand.lo >> 1) | (multiplicand.hi << 31);
			multiplicand.hi >>= 1;

			multiplier.hi = (multiplier.hi << 1) | (multiplier.lo >> 31);
			multiplier.lo = (multiplier.lo << 1);
		}
	}

	void u64::Divide(u64& dividend,const u64 divisor,const bool mod)
	{
		NST_VERIFY( divisor );

		u64 remainder(0);
		u64 quotient(0);

		if (divisor < dividend)
		{
			uint bits = 64;

			do
			{
				remainder.hi = (remainder.hi << 1) | (remainder.lo >> 31);
				remainder.lo = (remainder.lo << 1) | (dividend.hi >> 31);
				dividend.hi = (dividend.hi << 1) | (dividend.lo >> 31);
				dividend.lo = (dividend.lo << 1);
				--bits;
			}
			while (remainder < divisor);

			for (;;)
			{        
				u64 tmp(remainder);
				tmp -= divisor;

				quotient.hi = (quotient.hi << 1) | (quotient.lo >> 31);
				quotient.lo = (quotient.lo << 1);

				if (!(tmp.hi & 0x80000000UL))
				{
					quotient.lo |= 0x1;
					remainder = tmp;
				}

				if (!bits)
					break;

				--bits;

				remainder.hi = (remainder.hi << 1) | (remainder.lo >> 31);
				remainder.lo = (remainder.lo << 1) | (dividend.hi >> 31);
				dividend.hi = (dividend.hi << 1) | (dividend.lo >> 31);
				dividend.lo = (dividend.lo << 1);
			}
		}
		else if (divisor == dividend)
		{			
			quotient = 1;
		}
		else
		{
			remainder = dividend;
		}

		if (!mod)
			dividend = quotient;
		else
			dividend = remainder;
	} 
}

#endif
#endif
#endif
