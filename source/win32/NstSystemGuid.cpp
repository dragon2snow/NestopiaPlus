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

#include "NstSystemGuid.hpp"

namespace Nestopia
{
	using System::Guid;

	Guid::Guid(const String::Generic& string)
	{
		FromString( string );
	}

	Guid::Name Guid::GetString() const
	{
		return
		(
			Name() << String::Hex( ( u32 ) Data1,    true ) << '-'
				   << String::Hex( ( u16 ) Data2,    true ) << '-'
				   << String::Hex( ( u16 ) Data3,    true ) << '-'
				   << String::Hex( ( u8  ) Data4[0], true ) 
				   << String::Hex( ( u8  ) Data4[1], true ) << '-'
				   << String::Hex( ( u8  ) Data4[2], true )
				   << String::Hex( ( u8  ) Data4[3], true ) 
				   << String::Hex( ( u8  ) Data4[4], true ) 
				   << String::Hex( ( u8  ) Data4[5], true ) 
				   << String::Hex( ( u8  ) Data4[6], true )
				   << String::Hex( ( u8  ) Data4[7], true )
		);
	}

	ulong Guid::ConvertData(const String::Generic slice)
	{
		uint value;

		if (!((String::Stack<2+8>("0x") << slice) >> value))
			throw ERR_INVALID_STRING;

		return value;
	}

	void Guid::FromString(const String::Generic text)
	{
		if (text.Size() == STRING_LENGTH)
		{
			try
			{
				if
				(
					text[8] != '-' || 
					text[13] != '-' || 
					text[18] != '-' ||
					text[23] != '-'
				)
					throw ERR_INVALID_STRING;

				Data1    = ( u32 ) ConvertData( text(  0, 8 ) );
				Data2    = ( u16 ) ConvertData( text(  9, 4 ) );
				Data3    = ( u16 ) ConvertData( text( 14, 4 ) );
				Data4[0] = ( u8  ) ConvertData( text( 19, 2 ) );
				Data4[1] = ( u8  ) ConvertData( text( 21, 2 ) );

				for (uint i=2; i < 8; ++i)
					Data4[i] = (u8) ConvertData( text( 20 + i * 2, 2 ) );
			}
			catch (Exception)
			{
				Clear();
			}
		}
		else
		{
			Clear();
		}
	}
}
