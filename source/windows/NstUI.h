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

#pragma once

#ifndef NST_UI_H
#define NST_UI_H

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

namespace UI
{
	PDXRESULT MsgError (const UINT);
	PDXRESULT MsgError (const PDXSTRING&);
	PDXRESULT MsgError (const CHAR* const); 
	
	PDXRESULT MsgWarning (const UINT);
	PDXRESULT MsgWarning (const PDXSTRING&);
	PDXRESULT MsgWarning (const CHAR* const); 
	
	BOOL MsgQuestion (const UINT,const UINT);
	BOOL MsgQuestion (const UINT,const PDXSTRING&);
	BOOL MsgQuestion (const CHAR* const,const CHAR* const); 
	
	VOID MsgOutput (const UINT);
	VOID MsgOutput (const PDXSTRING&);
	VOID MsgOutput (const CHAR* const);
	
	BOOL MsgInput (const UINT,const UINT,PDXSTRING&);
	BOOL MsgInput (const PDXSTRING&,const PDXSTRING&,PDXSTRING&);
	BOOL MsgInput (const CHAR* const,const CHAR* const,PDXSTRING&);

	VOID LogOutput (const PDXSTRING& msg);
}

#endif
