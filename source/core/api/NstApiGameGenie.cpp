////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003-2005 Martin Freij
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
#include "NstApiEmulator.hpp"
#include "NstApiGameGenie.hpp"
#include "../NstGameGenie.hpp"

namespace Nes
{
	namespace Api
	{
		Result GameGenie::Encode( ulong packed,char (&characters)[9])
		{ 
			return Core::GameGenie::Encode( packed, characters ); 
		}
	
		Result GameGenie::Decode(cstring const characters,ulong& packed)
		{ 
			return Core::GameGenie::Decode( characters, packed ); 
		}
	
		Result GameGenie::Pack(const uint address,const uint data,const uint compare,const bool useCompare,ulong& packed)
		{ 
			return Core::GameGenie::Pack( address, data, compare, useCompare, packed ); 
		}
	
		Result GameGenie::Unpack(const ulong packed,uint& address,uint& data,uint& compare,bool& useCompare)
		{ 
			return Core::GameGenie::Unpack( packed, address, data, compare, useCompare ); 
		}
	
		Result GameGenie::AddCode(ulong packed)
		{ 
			return emulator.gameGenie.AddCode( packed );
		}
	
		Result GameGenie::DeleteCode(ulong packed)
		{ 
			return emulator.gameGenie.DeleteCode( packed ); 
		}
	
		uint GameGenie::NumCodes() const
		{ 
			return emulator.gameGenie.NumCodes(); 
		}
	
		ulong GameGenie::GetCode(uint index) const
		{ 
			return emulator.gameGenie.GetCode( index ); 
		}
	
		Result GameGenie::ClearCodes()
		{
			if (emulator.gameGenie.NumCodes())
			{
				emulator.gameGenie.ClearCodes();
				return RESULT_OK; 
			}
	
			return RESULT_NOP;
		}
	}
}
