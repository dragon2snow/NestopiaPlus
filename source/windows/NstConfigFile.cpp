///////////////////////////////////////////////////////////////////////////////////////
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

#include <cstdio>
#include <Windows.h>
#include "../paradox/PdxFile.h"
#include "NstConfigFile.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

#define NST_ENDL "\r\n"
#define NST_YES_OR_NO(x) ((x) ? "yes" : "no")

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

const CHAR* CONFIGFILE::FromGUID(const GUID& guid)
{
	static CHAR buffer[36+1];

	sprintf( buffer +  0, "%08X", guid.Data1    );
	sprintf( buffer +  9, "%04X", guid.Data2    );
	sprintf( buffer + 14, "%04X", guid.Data3    );
	sprintf( buffer + 19, "%02X", guid.Data4[0] );
	sprintf( buffer + 21, "%02X", guid.Data4[1] );

	for (UINT i=0; i < 8-2; ++i)
		sprintf( buffer + 24 + (i*2), "%02X", guid.Data4[i+2] );

	buffer[8] = buffer[13] = buffer[18] = buffer[23] = '-';

	return buffer;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

GUID CONFIGFILE::ToGUID(const CHAR* const string)
{
	GUID guid;

	if (string && strlen(string) == 36 && string[8] == '-' && string[13] == '-' && string[18] == '-' && string[23] == '-')
	{
		CHAR buffer[2+8+1];
		
		buffer[0] = '0';
		buffer[1] = 'X';

		memcpy( buffer + 2, string +  0, 8 ); buffer[2+8] = '\0'; guid.Data1    = strtoul( buffer, NULL, 0 );
		memcpy( buffer + 2, string +  9, 4 ); buffer[2+4] = '\0'; guid.Data2    = strtoul( buffer, NULL, 0 );
		memcpy( buffer + 2, string + 14, 4 ); buffer[2+4] = '\0'; guid.Data3    = strtoul( buffer, NULL, 0 );
		memcpy( buffer + 2, string + 19, 2 ); buffer[2+2] = '\0'; guid.Data4[0] = strtoul( buffer, NULL, 0 );
		memcpy( buffer + 2, string + 21, 2 ); buffer[2+2] = '\0'; guid.Data4[1] = strtoul( buffer, NULL, 0 );

		for (UINT i=0; i < 8-2; ++i)
		{
			memcpy( buffer + 2, string + 24 + (i*2), 2 );
			buffer[2+2] = '\0';
			guid.Data4[i+2] = strtoul( buffer, NULL, 0 );
		}
	}
	else
	{
		PDXMemZero( guid );
	}

	return guid;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT CONFIGFILE::Load(const PDXSTRING& filename)
{
	PDX_TRY(file.Open( filename, PDXFILE::INPUT ));

	PDXSTRING options[2];

	while (!file.Eof())
	{
		CHAR c;

		if (!file.Read(c))
			return PDX_OK;

		switch (c)
		{
			case '=':  

				while (!file.Eof())
				{
					if (!file.Read(c))
						return PDX_OK;

					if (c == '\n' || c == '\r')
						break;

					options[1].InsertBack(c);
				}

				ParseOptions( options );
				continue;

			case '/':

				if (file.Readable(1) && file.Peek<CHAR>() != '/')
					break;

				while (!file.Eof())
				{
					if (!file.Read(c))
						return PDX_OK;

					if (c == '\r' || c == '\n')
						break;
				}
				continue;

			case '\r':
			case '\n': 

				options[0].Clear();
				continue;
		}

		options[0].InsertBack(c);
	}

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT CONFIGFILE::Save(const PDXSTRING& filename)
{
	PDX_TRY(file.Open( filename, PDXFILE::OUTPUT ));

	WriteCommentHeader();

	PDXSTRING string;

	for (TREE::CONSTITERATOR i( tree.Begin() ); i != tree.End(); ++i)
	{
		string << (*i).First();
		string << " = ";
		string << (*i).Second();
		string << "\r\n";
	}

	file.Write( string.Begin(), string.End() - 1 );

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID CONFIGFILE::FormatString(PDXSTRING& string)
{
	INT start = 0;
	INT end = string.Size();

	for (INT i=0; i < string.Size(); ++i)
	{
		if (*string.At(i) != ' ')
			break;

		++start;
	}

	for (INT i=string.Size()-1; i >= 0; --i)
	{
		if (*string.At(i) != ' ')
			break;

		--end;
	}

	if (start > end)
	{
		string.Clear();
	}
	else if (start != 0 || end != string.Size())
	{
		const PDXSTRING tmp( string.At(start), string.At(end) );
		string = tmp;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID CONFIGFILE::ParseOptions(PDXSTRING* const options)
{
	if (options[0].Size())
	{
		FormatString( options[0] );
		
		if (options[0].Size())
		{
			FormatString( options[1] );
			tree[options[0]] = options[1];
		}
	}

	options[0].Clear();
	options[1].Clear();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID CONFIGFILE::WriteCommentHeader()
{
	const PDXSTRING string
	(
     	"/////////////////////////////////////////////////////////////////////////////\r\n"
       	"//\r\n"
     	"// Nestopia Configuration File. version 1.05\r\n"
     	"//\r\n"
		"/////////////////////////////////////////////////////////////////////////////\r\n"
		"\r\n"
	);

	file.Write( string.Begin(), string.End() );
}

