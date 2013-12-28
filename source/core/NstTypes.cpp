////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003 Martin Freij
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

#include "NstTypes.h"
#include "NstMachine.h"
#include "NstGameGenie.h"

NES_NAMESPACE_BEGIN

typedef GAMEGENIE GG;

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

IO::GAMEGENIE::GAMEGENIE(MACHINE& machine)
: handler( machine.QueryInterface(MACHINE::INTERFACE_GAMEGENIE) ) {}

PDXRESULT IO::GAMEGENIE::Encode(const ULONG packed,PDXSTRING& characters)
{ 
	return GG::Encode( packed, characters ); 
}

PDXRESULT IO::GAMEGENIE::Decode(const CHAR* const characters,ULONG& packed)
{ 
	return GG::Decode( characters, packed ); 
}

PDXRESULT IO::GAMEGENIE::Pack(const UINT address,const UINT data,const UINT compare,const BOOL UseCompare,ULONG& packed)
{ 
	return GG::Pack( address, data, compare, UseCompare, packed ); 
}

PDXRESULT IO::GAMEGENIE::Unpack(const ULONG packed,UINT& address,UINT& data,UINT& compare,BOOL& UseCompare)
{ 
	return GG::Unpack( packed, address, data, compare, UseCompare ); 
}

PDXRESULT IO::GAMEGENIE::AddCode(const ULONG packed)
{ 
	return PDX_CAST(GG* const,handler)->AddCode( packed ); 
}

PDXRESULT IO::GAMEGENIE::DeleteCode(const ULONG packed)
{ 
	return PDX_CAST(GG* const,handler)->DeleteCode( packed ); 
}

TSIZE IO::GAMEGENIE::NumCodes() const
{ 
	return PDX_CAST(GG* const,handler)->NumCodes(); 
}

ULONG IO::GAMEGENIE::GetCode(const TSIZE index) const
{ 
	return PDX_CAST(GG* const,handler)->GetCode( index ); 
}

VOID IO::GAMEGENIE::ClearCodes()
{ 
	PDX_CAST(GG* const,handler)->ClearCodes(); 
}

NES_NAMESPACE_END
