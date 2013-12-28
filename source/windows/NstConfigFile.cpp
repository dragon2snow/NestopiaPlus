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

#include "resource/resource.h"
#include "../paradox/PdxFile.h"
#include "NstConfigFile.h"
#include "NstCmdLine.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL CONFIGFILE::Load(const PDXSTRING& filename,const BOOL ShowWarnings)
{
	PDXFILE file( filename, PDXFILE::INPUT );

	if (!file.IsOpen() || file.IsEmpty())
		return FALSE;

	CMDLINEPARSER CmdLineParser;
	
	return CmdLineParser.Parse
	( 
     	file.Begin(),
		file.Size(),
		this, 
		ShowWarnings ? IDS_CFGFILE_PARSE_ERROR : 0,
		FALSE
	);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL CONFIGFILE::Save(const PDXSTRING& filename)
{
	PDXFILE file( filename, PDXFILE::OUTPUT );

	if (!file.IsOpen())
		return FALSE;

	const CHAR* header =
	(
		"/////////////////////////////////////////////////////////////////////////////\r\n"
		"//\r\n"
		"// Nestopia Configuration File. Version 1.09\r\n"
		"//\r\n"
		"/////////////////////////////////////////////////////////////////////////////\r\n"
		"\r\n"
	);

	file.Write( header, header + strlen(header) );

	const CHAR separator[] = {' ',':',' '};
	const CHAR endline[] = {'\r','\n'};

	for (TREE::CONSTITERATOR i( tree.Begin() ); i != tree.End(); ++i)
	{	
		file.Write( '-' );

		const PDXSTRING& first = (*i).First();

		file.Write( first.Begin(), first.Begin() + first.Length() );
		file.Write( separator, separator + 3 );

		const PDXSTRING& second = (*i).Second();

		file.Write( second.Begin(), second.Begin() + second.Length() );
		file.Write( endline, endline + 2 );
	}

	return TRUE;
}
