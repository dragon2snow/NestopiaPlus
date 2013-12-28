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
#include "NstIps.h"
#include "../paradox/PdxFile.h"

NES_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT IPS::Load(PDXFILE& ImageFile,PDXFILE& IpsFile)
{
	const PDXFILE::BUFFER backup( ImageFile.Buffer() );

	U8 block[5];

	if (!IpsFile.Read( block, block + 5 ))
		return PDX_FAILURE;

	static const U8 patch[5] =
	{
		0x50,0x41,0x54,0x43,0x48
	};

	if (memcmp( block, patch, sizeof(U8) * 5 ))
		return PDX_FAILURE;

	while (!IpsFile.Eof())
	{
		if (!IpsFile.Readable( sizeof(U8) * 3 ))
			goto hell;

		IpsFile.Read( block, block + 3 );

		static const U8 eof[3] =
		{
			0x45,0x4F,0x46
		};

		if (!memcmp( block, eof, sizeof(U8) * 3 ))
			return PDX_OK;

		const ULONG offset = 
		(
	     	( ULONG( block[0] ) << 16 ) |
			( ULONG( block[1] ) <<  8 ) |
			( ULONG( block[2] ) <<  0 )
		);

		if (!IpsFile.Read( block, block + 2 ))
			goto hell;

		UINT size = 
		(
     	   	( UINT( block[0] ) << 8 ) |
			( UINT( block[1] ) << 0 )
		);

		if (size)
		{
			if (offset + size > ImageFile.Size())
				goto hell;

			if (!IpsFile.Read( ImageFile.At( offset ), ImageFile.At( offset + size ) ))
				goto hell;
		}
		else
		{
			if (!IpsFile.Read( block, block + 2 ))
				goto hell;

			size =
			(
			    ( UINT( block[0] ) << 8 ) |
				( UINT( block[1] ) << 0 )
			);

			if (!size || offset + size > ImageFile.Size())
				goto hell;

			U8 data;

			if (!IpsFile.Read(data))
				goto hell;

			for (ULONG i=offset; i < offset + size; ++i)
				ImageFile.Buffer()[i] = data;
		}
	}

	return PDX_OK;

hell:

	ImageFile.Buffer() = backup;

	return PDX_FAILURE;
}

NES_NAMESPACE_END
