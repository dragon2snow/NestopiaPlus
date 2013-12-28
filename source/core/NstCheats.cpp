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

#include <algorithm>
#include "NstCore.hpp"
#include "NstCpu.hpp"
#include "NstCheats.hpp"

namespace Nes
{
	namespace Core
	{
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif

		Cheats::LoCode::LoCode(u16 a,u8 d,u8 c,bool u)
		: 
		address    (a),
		data       (d),
		compare    (c),
		useCompare (u)
		{}

		bool Cheats::LoCode::operator == (const LoCode& code) const
		{
			return data == code.data && compare == code.compare && useCompare == code.useCompare;
		}

		Cheats::HiCode::HiCode(u16 a,u8 d,u8 c,bool u)
		: LoCode(a,d,c,u), port(NULL) {}

		inline Cheats::HiCode::HiCode(uint address)
		: LoCode(address,0,0,0), port(NULL) {}

		Cheats::Cheats(Cpu& c)
		: cpu(c) {}

		Cheats::~Cheats()
		{
			ClearCodes();
		}

		Result Cheats::SetCode
		(
	       	const u16 address,
			const u8 data,
			const u8 compare,
			const bool useCompare,
			const bool activate
		)
		{
			if (address < 0x2000U)
			{
				const LoCode code(address,data,compare,useCompare);

				for (LoCodes::iterator it(loCodes.begin()), end(loCodes.end()); ; ++it)
				{
					if (it == loCodes.end() || it->address > address)
					{
						loCodes.insert( it, code );
						break;
					}
					else if (it->address == address)
					{
						if (*it == code)
						{
							return RESULT_NOP;
						}
						else
						{
							*it = code;
							break;
						}
					}
				}
			}
			else
			{
				const HiCode code(address,data,compare,useCompare);

				HiCodes::iterator it(hiCodes.begin());

				for (HiCodes::const_iterator end(hiCodes.end()); ; ++it)
				{
					if (it == end || it->address > address)
					{
				    	it = hiCodes.insert( it, code );
						break;
					}
					else if (it->address == address)
					{
						if (*it == code)
						{
							return RESULT_NOP;
						}
						else
						{
							*it = code;
							break;
						}
					}
				}

				if (activate)
					Map( *it );
			}

			return RESULT_OK;
		}

		Result Cheats::DeleteCode(dword index)
		{
			if (loCodes.size() > index)
			{
				loCodes.erase( loCodes.begin() + index );
				return RESULT_OK;
			}
			else if (hiCodes.size() > (index -= loCodes.size()))
			{
				HiCodes::iterator it( hiCodes.begin() + index );
				cpu.Unlink( it->address, this, &Cheats::Peek_Wizard, &Cheats::Poke_Wizard );
				hiCodes.erase( it );
				return RESULT_OK;
			}
			else
			{
				return RESULT_ERR_INVALID_PARAM;
			}
		}

		void Cheats::Reset()
		{
			for (HiCodes::iterator it(hiCodes.begin()), end(hiCodes.end()); it != end; ++it)
				Map( *it );
		}

		void Cheats::Map(HiCode& code)
		{
			code.port = cpu.Link( code.address, Cpu::LEVEL_HIGH, this, &Cheats::Peek_Wizard, &Cheats::Poke_Wizard );
		}

		void Cheats::ClearCodes()
		{
			loCodes.clear();

			for (HiCodes::iterator it(hiCodes.begin()), end(hiCodes.end()); it != end; ++it)
				cpu.Unlink( it->address, this, &Cheats::Peek_Wizard, &Cheats::Poke_Wizard );

			hiCodes.clear();
		}

		Result Cheats::GetCode
		(
	       	dword index,
			u16* const address,
			u8* const data,
			u8* const compare,
			bool* const useCompare
		)   const
		{
			const LoCode* code;

			if (loCodes.size() > index)
			{
				code = &loCodes[index];
			}
			else if (hiCodes.size() > (index -= loCodes.size()))
			{
				code = &hiCodes[index];
			}
			else
			{
				return RESULT_ERR_INVALID_PARAM;
			}

			if (address)
				*address = code->address;

			if (data)
				*data = code->data;

			if (compare)
				*compare = code->compare;

			if (useCompare)
				*useCompare = code->useCompare;

			return RESULT_OK;
		}

        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif

        void Cheats::BeginFrame() const
		{
			for (LoCodes::const_iterator it(loCodes.begin()), end(loCodes.end()); it != end; ++it)
				cpu.PatchSystemRam( it->address, it->data, it->compare, it->useCompare );
		}

		inline bool Cheats::HiCode::operator < (const HiCode& code) const
		{
			return address < code.address;
		}
  
		NES_PEEK(Cheats,Wizard)
		{
			NST_ASSERT( address >= 0x2000U );

			const HiCode& code = *std::lower_bound( &hiCodes.front(), &hiCodes.back() + 1, HiCode(address) );

			if (code.useCompare)
			{
				const uint data = code.port->Peek( address );

				if (code.compare != data)
					return data;
			}

			return code.data;
		}

		NES_POKE(Cheats,Wizard)
		{
			NST_ASSERT( address >= 0x2000U );

			return std::lower_bound( &hiCodes.front(), &hiCodes.back() + 1, HiCode(address) )->port->Poke( address, data );
		}
	}
}
