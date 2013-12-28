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

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif

#include <Windows.h>
#include "NstApplication.h"
#include "NstSoundManager.h"
#include "NstGraphicManager.h"
#include "NstUserInputManager.h"
#include "NstLogFileManager.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

NES_NAMESPACE_BEGIN

PDXRESULT MsgError(const CHAR* const msg) 
{ return ::UI::MsgError( msg ); }

PDXRESULT MsgWarning(const CHAR* const msg) 
{ return ::UI::MsgWarning( msg ); }

BOOL MsgQuestion(const CHAR* const title,const CHAR* const msg)
{ return ::UI::MsgQuestion( title, msg ); }

BOOL MsgInput(const CHAR* const title,const CHAR* const msg,PDXSTRING& res) 
{ return ::UI::MsgInput( title, msg, res ); }

VOID MsgOutput(const CHAR* const msg)
{ ::UI::MsgOutput( msg ); }

VOID LogOutput(const PDXSTRING& msg)
{ ::LOGFILE::Output( msg ); }

NES_NAMESPACE_END

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

namespace UI {

PDXRESULT MsgError(const CHAR* const msg) 
{ 
	const BOOL IsInstanced = APPLICATION::IsInstanced();

	if (IsInstanced)
	{
		application.GetSoundManager().Clear();
		application.GetGraphicManager().EnableGDI( TRUE );
	}

	::MessageBox
	( 
     	IsInstanced ? application.GetHWnd() : NULL, 
		msg, 
		UTILITIES::IdToString(IDS_APP_ERROR).String(), 
		MB_OK|MB_ICONSTOP
	);

	if (IsInstanced)
		application.GetGraphicManager().EnableGDI( FALSE );	

	return PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT MsgWarning(const CHAR* const msg) 
{ 
	const BOOL IsInstanced = APPLICATION::IsInstanced();

	if (IsInstanced)
	{
		application.GetSoundManager().Clear();
		application.GetGraphicManager().EnableGDI( TRUE );
	}

	::MessageBox
	( 
     	IsInstanced ? application.GetHWnd() : NULL, 
		msg, 
		UTILITIES::IdToString(IDS_APP_WARNING).String(), 
		MB_OK|MB_ICONEXCLAMATION 
	);

	if (IsInstanced)
		application.GetGraphicManager().EnableGDI( FALSE );	

	return PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT MsgInfo(const CHAR* const msg) 
{ 
	const BOOL IsInstanced = APPLICATION::IsInstanced();

	if (IsInstanced)
	{
		application.GetSoundManager().Clear();
		application.GetGraphicManager().EnableGDI( TRUE );
	}

	::MessageBox
	( 
     	IsInstanced ? application.GetHWnd() : NULL, 
		msg, 
		UTILITIES::IdToString(IDS_APP_MESSAGE).String(), 
		MB_OK|MB_ICONINFORMATION 
	);

	if (IsInstanced)
		application.GetGraphicManager().EnableGDI( FALSE );	

	return PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL MsgQuestion(const CHAR* const title,const CHAR* const msg) 
{ 
	const BOOL IsInstanced = APPLICATION::IsInstanced();

	if (IsInstanced)
	{
		application.GetSoundManager().Clear();
		application.GetGraphicManager().EnableGDI( TRUE );
	}

	const BOOL yes = 
	(
     	::MessageBox
     	( 
	       	IsInstanced ? application.GetHWnd() : NULL, 
     		msg, 
      		title, 
     		MB_YESNO | MB_ICONQUESTION 
     	) == IDYES
	);

	if (IsInstanced)
		application.GetGraphicManager().EnableGDI( FALSE );	

	return yes;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MsgOutput(const CHAR* const msg)
{ 
	PDX_ASSERT( APPLICATION::IsInstanced() );

	if (APPLICATION::IsInstanced())
		application.StartScreenMsg( 1500, msg ); 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL MsgInput(const CHAR* const title,const CHAR* const msg,PDXSTRING& input)
{ 
	PDX_ASSERT( APPLICATION::IsInstanced() );

	if (APPLICATION::IsInstanced())
		return application.GetUserInputManager().Start( title, msg, input );

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT MsgError   (const UINT id) { return MsgError   ( UTILITIES::IdToString( id ).String() ); }
PDXRESULT MsgWarning (const UINT id) { return MsgWarning ( UTILITIES::IdToString( id ).String() ); }
PDXRESULT MsgInfo    (const UINT id) { return MsgInfo    ( UTILITIES::IdToString( id ).String() ); }

PDXRESULT MsgError   (const PDXSTRING& msg) { return MsgError   ( msg.String() ); }
PDXRESULT MsgWarning (const PDXSTRING& msg) { return MsgWarning ( msg.String() ); }
PDXRESULT MsgInfo    (const PDXSTRING& msg) { return MsgInfo    ( msg.String() ); }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL MsgQuestion(const UINT IdTitle,const UINT IdMsg)
{
	return MsgQuestion
	( 
     	UTILITIES::IdToString( IdTitle ).String(), 
		UTILITIES::IdToString( IdMsg ).String()
	);
}

BOOL MsgQuestion(const PDXSTRING& title,const PDXSTRING& msg)
{
	return MsgQuestion( title.String(), msg.String() );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID MsgOutput(const UINT id)
{
	MsgOutput( UTILITIES::IdToString( id ).String() );
}

VOID MsgOutput(const PDXSTRING& msg)
{
	MsgOutput( msg.String() );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL MsgInput(const UINT IdTitle,const UINT IdMsg,PDXSTRING& res)
{
	return MsgInput
	( 
		UTILITIES::IdToString( IdTitle ).String(), 
		UTILITIES::IdToString( IdMsg ).String(),
		res
	);
}

BOOL MsgInput(const PDXSTRING& title,const PDXSTRING& msg,PDXSTRING& res)
{
	return MsgInput( title.String(), msg.String(), res );
}

}
