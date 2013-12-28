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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include "../paradox/PdxString.h"
#include "NstUI.h"
#include "NstLogFileManager.h"
#include "NstCmdLine.h"
#include "NstConfigFile.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL CMDLINEPARSER::Parse
(
    const CHAR* const LineString,
	const TSIZE LineLength,
	CONFIGFILE* const cfg,
	const UINT MsgIdError,
	const BOOL LogCommands
)
{
	if (!LineString || !LineLength)
		return FALSE;

	StartupFile.Clear();

	COMMANDS commands;

	CmdLine = LineString;
	CmdEnd = LineString + LineLength;

	try
	{
		if (!SkipSpace())
			return FALSE;

		if (*CmdLine == '\"')
		{
			++CmdLine;
			ParseValue( StartupFile, '\"' );
		}

		if (!cfg)
			return FALSE;

		while (CmdLine + 1 < CmdEnd)
		{
			if (CmdLine[0] == '/' && CmdLine[1] == '/')
			{
				if (!SkipComment())
					break;
			}
			else if (*CmdLine == '-')
			{
				++CmdLine;

				if (!SkipSpace())
					throw 1;

				commands.Grow();
				ParseValue( commands.Back().First(), ':' );
				
				++CmdLine;

				if (!SkipSpace())
					break;

				if (*CmdLine == '\"')
				{
					++CmdLine;
					ParseValue( commands.Back().Second(), '\"' );
					++CmdLine;
				}
				else
				{
					ParseLine( commands.Back().Second() );
				}
			}
			else
			{
				++CmdLine;
			}
		}

		if (commands.Size())
		{
			PDXSTRING msg;

			if (LogCommands)
				msg = "CMDLINE: overriding command: - ";

			for (TSIZE i=0; i < commands.Size(); ++i)
			{
				if (LogCommands)
				{
					msg.Resize(31);
					LOGFILE::Output( msg << commands[i].First() << " : " << commands[i].Second() );
				}

				(*cfg)[commands[i].First()] = commands[i].Second();
			}

			return TRUE;
		}
	}
	catch (...)
	{
		if (MsgIdError)
		{
			UI::MsgWarning( MsgIdError );
			LOGFILE::Output("CMDLINE: parse error!");
		}
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID CMDLINEPARSER::ParseValue(PDXSTRING& dst,const CHAR stop)
{
	while (CmdLine < CmdEnd && *CmdLine != stop)
	{
		if 
		(
     		*CmdLine != '\0' &&
       		*CmdLine != '\a' && 
       		*CmdLine != '\r' &&
     		*CmdLine != '\v' &&
         	*CmdLine != '\f' &&
			*CmdLine != '\n'
		)
			dst.Buffer().InsertBack( *CmdLine );

		++CmdLine;
	}

	if (CmdLine >= CmdEnd)
		throw 1;

	if (dst.Buffer().Size())
	{
		dst.Buffer().InsertBack('\0');
		dst.RemoveSpaces();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID CMDLINEPARSER::ParseLine(PDXSTRING& dst)
{
	while (CmdLine < CmdEnd)
	{
		if 
		(
	     	(*CmdLine == '\r') ||
			(*CmdLine == '\n') ||
			(*CmdLine == '-') ||
			(*CmdLine == '/' && CmdLine + 1 < CmdEnd && CmdLine[1] == '/')
		)
			break;

		if 
		(
    		*CmdLine != '\0' &&
     		*CmdLine != '\a' && 
    		*CmdLine != '\v' &&
       		*CmdLine != '\f'
		)
			dst.Buffer().InsertBack( *CmdLine );

		++CmdLine;
	}

	if (dst.Buffer().Size())
	{
		dst.Buffer().InsertBack('\0');
		dst.RemoveSpaces();
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL CMDLINEPARSER::SkipSpace()
{
	while (CmdLine < CmdEnd)
	{
		if (*CmdLine != ' ')
			return TRUE;

		++CmdLine;
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL CMDLINEPARSER::SkipComment()
{
	for (CmdLine += 2; CmdLine < CmdEnd && *CmdLine != '\r' && *CmdLine != '\n'; ++CmdLine);

	if (++CmdLine >= CmdEnd)
		return FALSE;

	if (*CmdLine == '\r' || *CmdLine == '\n') 
		++CmdLine;

	return TRUE;
}
