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

PDXSTRING& CONFIGFILE::operator [] (const CHAR* const command)
{
	return tree[command];
}

PDXSTRING& CONFIGFILE::operator [] (const PDXSTRING& command)
{
	return tree[command];
}

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
		string << "\n";
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
     	"// Nestopia Configuration File. version 1.04\r\n"
     	"//\r\n"
		"/////////////////////////////////////////////////////////////////////////////\r\n"
		"\r\n"
	);

	file.Write( string.Begin(), string.End() - 1 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
/*
VOID CONFIGFILE::WriteCommentVideo()
{
	const PDXSTRING string
	(
     	"\r\n"
		"/////////////////////////////////////////////////////////////////////////////\r\n"
	    "//\r\n"
	    "// Video Options\r\n"
		"// -------------\r\n"
	    "//\r\n"
	    "// video device <guid>\r\n"
	    "//\r\n"
	    "// fullscreen width <width>\r\n"
		"// fullscreen height <height>\r\n"
		"// fullscreen bpp <16,32>\r\n"
	    "//\r\n"
	    "// timing <vsync,frameskip>\r\n"
	    "//\r\n"
	    "// offscreen buffer <sysmem,vidmem>\r\n"
	    "//\r\n"
	    "// graphic filter <none,scanlines,2xsai,super2xsai,supereagle>\r\n"
	    "//\r\n"
		"// palette <internal,emulated,custom>\r\n"
	    "// palette file <file>\r\n"
	    "//\r\n"
	    "// infinite sprites <yes,no>\r\n"
	    "//\r\n"
	    "// ntsc rect left <left>\r\n"
		"// ntsc rect top <top>\r\n"
		"// ntsc rect right <right>\r\n"
		"// ntsc rect bottom <bottom>\r\n"
		"//\r\n"
	    "// pal rect left <left>\r\n"
		"// pal rect top <top>\r\n"
	    "// pal rect right <right>\r\n"
	    "// pal rect bottom <bottom>\r\n"
	    "//\r\n"
	    "// brightness <0..255>\r\n"
	    "// saturation <0..255>\r\n"
	    "// hue <0..255>\r\n"
	    "//\r\n"
		"/////////////////////////////////////////////////////////////////////////////\r\n"
		"\r\n"
	);

	file.Write( string.Begin(), string.End() - 1 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID CONFIGFILE::WriteCommentSound()
{
	const PDXSTRING string
	(
     	"\r\n"
		"/////////////////////////////////////////////////////////////////////////////\r\n"
     	"//\r\n"
	    "// Sound options\r\n"
		"// -------------\r\n"
	    "//\r\n"
	    "// sound device <guid>\r\n"
	    "//\r\n"
	    "// sample rate <11025,22050,44100,48000,96000,192000>\r\n"
	    "// sample bits <8,16>\r\n"
	    "//\r\n"
	    "// sound enable <yes,no>\r\n"
	    "// sound buffers <1..10>\r\n"
	    "// sound volume <0..100>\r\n"
	    "//\r\n"
	    "// apu square 1 enable <yes,no>\r\n"
	    "// apu square 2 enable <yes,no>\r\n"
	    "// apu triangle enable <yes,no>\r\n"
	    "// apu noise enable <yes,no>\r\n"
	    "// apu dmc enable <yes,no>\r\n"
	    "// apu external enable <yes,no>\r\n"
	    "//\r\n"
		"/////////////////////////////////////////////////////////////////////////////\r\n"
		"\r\n"
	);

	file.Write( string.Begin(), string.End() - 1 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID CONFIGFILE::WriteCommentFiles()
{
	const PDXSTRING string
	(
     	"\r\n"
		"/////////////////////////////////////////////////////////////////////////////\r\n"
	    "//\r\n"
	    "// File Options\r\n"
		"// ------------\r\n"
	    "//\r\n"
	    "// image file path <path>\r\n"
	    "// battery file path <path>\r\n"
	    "// state file path <path>\r\n"
	    "// ips file path <path>\r\n"
	    "// nsp file path <path>\r\n"
	    "//\r\n"
	    "// last image file path <path>\r\n"
	    "// last state file path <path>\r\n"
	    "// last nsp file path <path>\r\n"
	    "//\r\n"
	    "// use last image file path <yes,no>\r\n"
	    "// use last state file path <yes,no>\r\n"
	    "// use last nsp file path <yes,no>\r\n"
	    "//\r\n"
	    "// battery in image file path <yes,no>\r\n"
	    "// ips in image file path <yes,no>\r\n"
	    "// nsp in image file path <yes,no>\r\n"
	    "//\r\n"
	    "// auto apply ips <yes,no>\r\n"
	    "// auto apply nsp <yes,no>\r\n"
	    "//\r\n"
	    "// fds bios <file>\r\n"
	    "//\r\n"
	    "// write protect battery <yes,no>\r\n"
	    "// write protect fds <yes,no>\r\n"
	    "//\r\n"
	    "// recent file 1..10 <file>\r\n"
	    "//\r\n"
		"/////////////////////////////////////////////////////////////////////////////\r\n"
		"\r\n"
	);

	file.Write( string.Begin(), string.End() - 1 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID CONFIGFILE::WriteCommentInput()
{
	const PDXSTRING string
	(
     	"\r\n"
		"/////////////////////////////////////////////////////////////////////////////\r\n"
	    "//\r\n"
	    "// Input Options\r\n"
		"// -------------\r\n"
	    "//\r\n"
		"// joystick device 0..x <guid>\r\n"
		"// input pad1..pad4 <output key> <input key>\r\n"
		"// input powerpad <output key> <input key>\r\n"
		"// input general <output key> <input key>\r\n"
	    "//\r\n"
		"/////////////////////////////////////////////////////////////////////////////\r\n"
		"\r\n"
	);

	file.Write( string.Begin(), string.End() - 1 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID CONFIGFILE::WriteCommentPreferences()
{
	const PDXSTRING string
	(
	    "\r\n"
		"/////////////////////////////////////////////////////////////////////////////\r\n"
	    "//\r\n"
	    "// Preferences\r\n"
		"// -----------\r\n"
	    "//\r\n"
	    "// emulate at once <yes,no>\r\n"
	    "// run in background <yes,no>\r\n"
	    "// nsf in background <yes,no>\r\n"
	    "// priority <high,default>\r\n"
	    "// startup <window,fullscreen>\r\n"
	    "// warnings <yes,no>\r\n"
	    "// power off on exit <yes,no>\r\n"
	    "// hide menu in fullscreen <yes,no>\r\n"
	    "// confirm exit <yes,no>\r\n"
	    "// save logfile <yes,no>\r\n"
	    "//\r\n"
		"/////////////////////////////////////////////////////////////////////////////\r\n"
		"\r\n"
	);

	file.Write( string.Begin(), string.End() - 1 );
}
*/
