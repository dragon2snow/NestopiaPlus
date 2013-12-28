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
#include <CommCtrl.h>
#include "../NstNes.h"
#include "../core/NstNsp.h"
#include "NstGameGenieManager.h"
#include "NstFileManager.h"
#include "NstApplication.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

GAMEGENIEMANAGER::GAMEGENIEMANAGER()
: 
MANAGER   (IDD_GAMEGENIE),
hDlg      (NULL),
hList  	  (NULL),
GameGenie (nes) 
{}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GAMEGENIEMANAGER::Create(CONFIGFILE* const ConfigFile)
{
	PDX_ASSERT(!hDlg && !hList);

	if (!ConfigFile)
		return;

	CONFIGFILE& file = *ConfigFile;

	const ULONG NumCodes = file["game genie number of codes"].ToUlong();

	if (NumCodes > USHORT_MAX)
	{
		UI::MsgWarning(IDS_GAMEGENIE_TOO_MANY_CODES);
		return;
	}

	if (!NumCodes)
		return;

	PDXSTRING characters;
	PDXSTRING comment;

	PDXSTRING index("game genie code ");

	for (UINT i=0; i < NumCodes; ++i)
	{
		index.Resize(16);
		index << i;

		const PDXSTRING& string = file[index];

		if (string.Length() >= 6)
		{
			characters = string;

			for (TSIZE j=0; j < string.Length(); ++j)
			{
				if (string[j] == ' ')
				{
					if (j != 6 && j != 8)
						break;

					characters.Resize(j);

					PDXSTRING::CONSTITERATOR offset = string.At(j+1);

					while (*offset == ' ')
						++offset;

					const BOOL enabled = !
					(
						(offset[0] == 'O' || offset[0] == 'o') &&
						(offset[1] == 'F' || offset[1] == 'f') &&
						(offset[2] == 'F' || offset[2] == 'f') &&
						(offset[3] == ' ' || offset[3] == '\0')
					);

					while (*offset != ' ' && *offset != '\0')
						++offset;

					while (*offset == ' ')
						++offset;

					comment.Clear();

					if (*offset != '\0')
					{
						comment = offset;
						comment.RemoveQuotes();
						comment.RemoveSpaces();
					}

					AddCode( characters, enabled, &comment );
					break;
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GAMEGENIEMANAGER::Destroy(CONFIGFILE* const ConfigFile)
{
	if (!ConfigFile)
		return;
	
	CONFIGFILE& file = *ConfigFile;

	PDXSTRING option("game genie code ");
	PDXSTRING string;

	ULONG NumCodes = 0;

	for (CODES::CONSTITERATOR i(codes.Begin()); i != codes.End(); ++i)
	{
		string.Clear();

		if (PDX_SUCCEEDED(GameGenie.Encode( (*i).packed, string )))
		{
			string << ((*i).enabled ? " on" : " off");

			const PDXSTRING& comment = (*i).comment;

			if (comment.Length())
			{
				string << " \"";

				for (TSIZE i=0; i < comment.Length(); ++i)
				{
					if (comment[i] != '\"') 
						string.InsertBack( comment[i] );
				}

				string.InsertBack('\"');
			}

			option.Resize(16);
			option += NumCodes++;

			file[option] = string;
		}
	}

	file["game genie number of codes"] = NumCodes;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL GAMEGENIEMANAGER::GAMEGENIECLOSE::DialogProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM)
{
	if (uMsg == WM_COMMAND)
	{
		switch (LOWORD(wParam))
		{
     		case IDC_GAMEGENIECLOSE_KEEP:

				::EndDialog( hDlg, 0 );
				return TRUE;

			case IDC_GAMEGENIECLOSE_DISABLE:

				ggm.SetCodeStates( FALSE );
				ggm.GameGenie.ClearCodes();
				::EndDialog( hDlg, 0 );
				return TRUE;

			case IDC_GAMEGENIECLOSE_REMOVE:

				ggm.ClearCodes();
				ggm.GameGenie.ClearCodes();
				::EndDialog( hDlg, 0 );
				return TRUE;
		}
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GAMEGENIEMANAGER::SetCodeStates(const BOOL enabled)
{
	for (CODES::ITERATOR i(codes.Begin()); i != codes.End(); ++i)
		(*i).enabled = enabled;

	if (hList)
	{
		const INT num = ListView_GetItemCount( hList );

		for (INT i=0; i < num; ++i)
			ListView_SetCheckState( hList, i, enabled );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT GAMEGENIEMANAGER::ClearCodes(const BOOL AskFirst)
{
	if (AskFirst)
	{
		for (CODES::CONSTITERATOR i(codes.Begin()); i != codes.End(); ++i)
		{
			if ((*i).enabled)
			{
				GAMEGENIECLOSE ggc( *this );
				ggc.StartDialog();
				break;
			}
		}
	}
	else
	{
		GameGenie.ClearCodes();
		ClearCodes();
	}

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT GAMEGENIEMANAGER::GetCode(const TSIZE index,PDXSTRING& characters,BOOL& enabled,PDXSTRING* const comment) const
{
	if (index >= codes.Size())
		return PDX_FAILURE;

	CODES::CONSTITERATOR i( codes.At(index) );
	PDX_TRY(GameGenie.Encode( (*i).packed, characters ));

	enabled = (*i).enabled;

	if (comment)
		*comment = (*i).comment;

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT GAMEGENIEMANAGER::AddCode(const PDXSTRING& characters,const BOOL enabled,const PDXSTRING* const comment)
{
	ULONG packed;
	PDX_TRY(GameGenie.Decode( characters.String(), packed ));

	CODE code( packed, enabled, comment );

	if (codes.SafeInsert( code ).Second() && hList && !AddToList( code ))
	{
		codes.Erase( packed );
		return PDX_FAILURE;
	}

	if (enabled && !hList)
		GameGenie.AddCode( packed );

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL GAMEGENIEMANAGER::DialogProc(HWND h,UINT uMsg,WPARAM wParam,LPARAM)
{
	switch (uMsg) 
	{
     	case WM_INITDIALOG:

			hDlg = h;
			hList = ::GetDlgItem( h, IDC_GAMEGENIE_LIST );
			InitDialog();
     		return TRUE;

		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
     			case IDC_GAMEGENIE_ADD:       CreateCodeDialog();     return TRUE;
				case IDC_GAMEGENIE_REMOVE:    RemoveCode();           return TRUE;
     			case IDC_GAMEGENIE_CLEAR_ALL: ClearCodes();           return TRUE;
				case IDC_GAMEGENIE_IMPORT:    ImportCodes();          return TRUE;
				case IDC_GAMEGENIE_EXPORT:    ExportCodes();          return TRUE;
				case IDC_GAMEGENIE_OK:        ::EndDialog( hDlg, 0 ); return TRUE;
			}
			return FALSE;

     	case WM_CLOSE:

     		::EndDialog( hDlg, 0 );
     		return TRUE;

    	case WM_DESTROY:

			CloseDialog();
			hList = NULL;
    		return TRUE;
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK GAMEGENIEMANAGER::StaticCodeDialogProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	static GAMEGENIEMANAGER* ggm = NULL;

	switch (uMsg)
	{
     	case WM_INITDIALOG:

			PDX_ASSERT( !ggm );
			ggm = PDX_CAST(GAMEGENIEMANAGER*,lParam);

			::CheckRadioButton
			( 
    			hDlg, 
        		IDC_GAMEGENIE_ADDCODE_ENCODED, 
        		IDC_GAMEGENIE_ADDCODE_DECODED, 
        		IDC_GAMEGENIE_ADDCODE_ENCODED 
       		);
			return TRUE;
		
		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
     			case IDC_GAMEGENIE_ADDCODE_SUBMIT:   PDX_ASSERT( ggm ); ggm->SubmitCode( hDlg );
				case IDC_GAMEGENIE_ADDCODE_CANCEL:   ::EndDialog( hDlg, 0 ); return TRUE;
				case IDC_GAMEGENIE_ADDCODE_VALIDATE: PDX_ASSERT( ggm ); ggm->ValidateCode( hDlg ); return TRUE;
			}
			return FALSE;

		case WM_CLOSE:

			::EndDialog( hDlg, 0 );
			return TRUE;

		case WM_DESTROY:

			PDX_ASSERT( ggm );
			ggm = NULL;
			return TRUE;
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GAMEGENIEMANAGER::SubmitCode(HWND hDlg)
{
	PDXSTRING characters;

	if (!MANAGER::GetDlgItemText( hDlg, IDC_GAMEGENIE_ADDCODE_CHARACTERS, characters, 16 ))
	{
		ValidateCode( hDlg );

		if (!MANAGER::GetDlgItemText( hDlg, IDC_GAMEGENIE_ADDCODE_CHARACTERS, characters, 16 ))
			return;
	}

	if (characters.Length())
	{
		PDXSTRING comment;
		MANAGER::GetDlgItemText( hDlg, IDC_GAMEGENIE_ADDCODE_COMMENT, comment );

		if (PDX_FAILED(AddCode( characters, TRUE, &comment )))
			UI::MsgWarning( IDS_GAMEGENIE_INVALID );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GAMEGENIEMANAGER::ValidateCode(HWND hDlg) const
{
	CHAR buffer[32];
	ULONG packed;

	if (::IsDlgButtonChecked( hDlg, IDC_GAMEGENIE_ADDCODE_DECODED) == BST_CHECKED)
	{
		if (::GetDlgItemText( hDlg, IDC_GAMEGENIE_ADDCODE_ADDRESS, buffer, 32-1 ))
		{
			const UINT address = strtoul( buffer, NULL, 16 );

			if (::GetDlgItemText( hDlg, IDC_GAMEGENIE_ADDCODE_VALUE, buffer, 32-1 ))
			{
				const UINT value      = strtoul( buffer, NULL, 16 );
				const BOOL UseCompare = ::GetDlgItemText( hDlg, IDC_GAMEGENIE_ADDCODE_COMPARE, buffer, 32-1 );				
				const UINT compare    = UseCompare ? strtoul( buffer, NULL, 16 ) : 0x00;

				if (PDX_SUCCEEDED(GameGenie.Pack( address, value, compare, UseCompare, packed )))
				{
					PDXSTRING characters;

					if (PDX_SUCCEEDED(GameGenie.Encode( packed, characters )))
					{
						::SetDlgItemText( hDlg, IDC_GAMEGENIE_ADDCODE_CHARACTERS, characters.String() );
						::SetDlgItemText( hDlg, IDC_GAMEGENIE_ADDCODE_RESULT, "VALID" );
						return;
					}
				}
			}
		}
	}
	else
	{
		const UINT length = ::GetDlgItemText( hDlg, IDC_GAMEGENIE_ADDCODE_CHARACTERS, buffer, 32-1 );

		if (length && PDX_SUCCEEDED(GameGenie.Decode( buffer, packed )))
		{
			UINT address, value, compare;
			BOOL UseCompare;

			if (PDX_SUCCEEDED(GameGenie.Unpack( packed, address, value, compare, UseCompare )))
			{
				::SetDlgItemText( hDlg, IDC_GAMEGENIE_ADDCODE_ADDRESS, _ultoa( address, buffer, 16 ) );
				::SetDlgItemText( hDlg, IDC_GAMEGENIE_ADDCODE_VALUE,   _ultoa( value,   buffer, 16 ) );
				::SetDlgItemText( hDlg, IDC_GAMEGENIE_ADDCODE_COMPARE, UseCompare ? _ultoa( compare, buffer, 16 ) : "" );
				::SetDlgItemText( hDlg, IDC_GAMEGENIE_ADDCODE_RESULT, "VALID" );
				return;
			}
		}
	}

	::SetDlgItemText( hDlg, IDC_GAMEGENIE_ADDCODE_RESULT, "INVALID" );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GAMEGENIEMANAGER::InitDialog()
{
	PDX_ASSERT( hList );

	ListView_SetExtendedListViewStyle( hList, LVS_EX_CHECKBOXES|LVS_EX_FULLROWSELECT );

	LV_COLUMN column;
	PDXMemZero( column );

	column.mask     = LVCF_FMT|LVCF_TEXT|LVCF_SUBITEM|LVCF_WIDTH;
	column.fmt      = LVCFMT_LEFT;
	column.cx       = 70;
	column.iSubItem = -1;

	CHAR* const strings[] = {"Code","Address","Data","Compare"};

	for (UINT i=0; i < 4; ++i)
	{
		column.pszText = strings[i];
		column.cchTextMax = strlen(strings[i]);
		ListView_InsertColumn( hList, i, &column );
	}

	column.pszText = "Comment"; 
	column.cx = 172;
	column.cchTextMax = strlen(column.pszText); 
	ListView_InsertColumn( hList, 4, &column );

	::EnableWindow( ::GetDlgItem( hDlg, IDC_GAMEGENIE_EXPORT    ), FALSE );
	::EnableWindow( ::GetDlgItem( hDlg, IDC_GAMEGENIE_CLEAR_ALL ), FALSE );
	::EnableWindow( ::GetDlgItem( hDlg, IDC_GAMEGENIE_REMOVE    ), FALSE );

	for (CODES::ITERATOR i(codes.Begin()); i != codes.End(); )
	{
		if (AddToList( *i ))
		{
			++i;
		}
		else
		{
			CODES::ITERATOR kill( i++ );
			codes.Erase( *kill );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL GAMEGENIEMANAGER::AddToList(CODE& code)
{
	PDXSTRING string;

	if (PDX_FAILED(GameGenie.Encode( code.packed, string )))
		return FALSE;

	LVITEM item;
	PDXMemZero( item );

	item.mask       = LVIF_TEXT;
	item.pszText    = string.Begin();
	item.cchTextMax = string.Length();
	item.iItem      = INT_MAX;

	PDX_ASSERT( hList );

	INT slot;

	if ((slot = ListView_InsertItem( hList, &item )) == -1)
		return FALSE;

	ListView_SetCheckState( hList, slot, code.enabled );

	UINT address;
	UINT data;
	UINT compare;
	BOOL UseCompare;

	const PDXRESULT result = GameGenie.Unpack( code.packed, address, data, compare, UseCompare );
	PDX_ASSERT( PDX_SUCCEEDED(result) );

	string.Set( address, PDXSTRING::HEX );
	ListView_SetItemText( hList, slot, 1, string.Begin() );

	string.Set( data, PDXSTRING::HEX );
	ListView_SetItemText( hList, slot, 2, string.Begin() );

	if (UseCompare)
		string.Set( compare, PDXSTRING::HEX );
	else
		string.Clear();

	ListView_SetItemText( hList, slot, 3, string.Begin() );
	ListView_SetItemText( hList, slot, 4, code.comment.Begin() );

	::EnableWindow( ::GetDlgItem( hDlg, IDC_GAMEGENIE_EXPORT    ), TRUE );
	::EnableWindow( ::GetDlgItem( hDlg, IDC_GAMEGENIE_CLEAR_ALL ), TRUE );
	::EnableWindow( ::GetDlgItem( hDlg, IDC_GAMEGENIE_REMOVE    ), TRUE );

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GAMEGENIEMANAGER::CloseDialog()
{
  	GameGenie.ClearCodes();

	PDX_ASSERT( hList );
	const INT num = ListView_GetItemCount( hList );

	for (INT i=0; i < num; ++i)
	{
		const BOOL enabled = ListView_GetCheckState( hList, i );

		CHAR buffer[16];
		buffer[0] = '\0';

		ListView_GetItemText( hList, i, 0, buffer, 16-1 );

		ULONG packed;

		if (PDX_SUCCEEDED(GameGenie.Decode( buffer, packed )))
		{
			(*codes.Find( packed )).enabled = enabled;

			if (enabled)
				GameGenie.AddCode( packed );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GAMEGENIEMANAGER::CreateCodeDialog()
{
	DialogBoxParam
	( 
     	::GetModuleHandle(NULL), 
		MAKEINTRESOURCE(IDD_GAMEGENIE_ADDCODE), 
		hDlg, 
		StaticCodeDialogProc,
		PDX_CAST(LPARAM,this)
	); 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GAMEGENIEMANAGER::RemoveCode()
{
	PDX_ASSERT( hList );

	const INT slot = ListView_GetSelectionMark( hList );

	if (slot > -1)
	{
		CHAR buffer[16];

		buffer[0] = '\0';
		ListView_GetItemText( hList, slot, 0, buffer, 16-1 );
		ListView_DeleteItem( hList, slot );

		const BOOL enable = (ListView_GetItemCount( hList ) > 0);

		::EnableWindow( ::GetDlgItem( hDlg, IDC_GAMEGENIE_EXPORT    ), enable );
		::EnableWindow( ::GetDlgItem( hDlg, IDC_GAMEGENIE_CLEAR_ALL ), enable );
		::EnableWindow( ::GetDlgItem( hDlg, IDC_GAMEGENIE_REMOVE    ), enable );

		ULONG packed;

		if (PDX_SUCCEEDED(GameGenie.Decode( buffer, packed )))
			codes.Erase( packed );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GAMEGENIEMANAGER::ClearCodes()
{
	if (hList)
	{
		ListView_DeleteAllItems( hList );

		PDX_ASSERT( hDlg );
		::EnableWindow( ::GetDlgItem( hDlg, IDC_GAMEGENIE_EXPORT    ), FALSE );
		::EnableWindow( ::GetDlgItem( hDlg, IDC_GAMEGENIE_CLEAR_ALL ), FALSE );
		::EnableWindow( ::GetDlgItem( hDlg, IDC_GAMEGENIE_REMOVE    ), FALSE );
	}

	codes.Destroy();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GAMEGENIEMANAGER::ExportCodes()
{
	PDXSTRING filename;

	const BOOL succeeded = UTILITIES::BrowseSaveFile
	(
	    filename,
		hDlg,
		IDS_FILE_SAVE_GAMEGENIE,
   		"Nestopia Script Files (*.nsp)\0"
   		"*.nsp\0"
   		"All files (*.*)\0"
   		"*.*\0",
		application.GetFileManager().GetNspPath().String(),
		"nsp"
	);

	if (!succeeded)
		return;

	PDXFILE file( filename, PDXFILE::APPEND );
		
	if (!file.IsOpen())
	{
		UI::MsgWarning( IDS_FILE_CREATE_FAILED );
		return;
	}

	NES::IO::NSP::CONTEXT context;
	NES::NSP NspFile;

	if (file.Size())
	{
		file.Seek( PDXFILE::BEGIN );

		FILEEXISTDIALOG FileExistDialog;
		FileExistDialog.StartDialog();

		switch (FileExistDialog.Choice())
		{
   			case IDC_FILEEXIST_CANCEL:

				file.Abort();
				return;

			case IDC_FILEEXIST_APPEND:

				if (PDX_SUCCEEDED(NspFile.Load( file )))
				{
					NspFile.GetContext( context );

					if (context.GenieCodes.Size())
					{
						const BOOL keep = UI::MsgQuestion
						(
							IDS_GAMEGENIE_FILE_KEEP_CODES_1_2,
							IDS_GAMEGENIE_FILE_KEEP_CODES_2_2
						);

						if (!keep)
							context.GenieCodes.Clear();
					}
				}
				else
				{
					file.Abort();
					UI::MsgWarning( IDS_FILE_PARSE_ERROR );
					return;
				}
		}
	}

	CHAR buffer[512];

	const INT num = ListView_GetItemCount( hList );

	for (INT i=0; i < num; ++i)
	{
		buffer[0] = '\0';
		ListView_GetItemText( hList, i, 0, buffer, 16-1 );			
	
		if (buffer[0] != '\0')
		{
			context.GenieCodes.Grow();
			context.GenieCodes.Back().code = buffer;
			context.GenieCodes.Back().enabled = ListView_GetCheckState( hList, i ); 

			buffer[0] = '\0';
			ListView_GetItemText( hList, i, 4, buffer, 512-1 );

			if (buffer[0] != '\0')
				context.GenieCodes.Back().comment = buffer;
		}
	}

	NspFile.SetContext( context );
	file.Seek( PDXFILE::BEGIN );

	if (PDX_FAILED(NspFile.Save( file, NES::NSP::SAVE_ONLY_GAMEGENIE )))
		UI::MsgWarning( IDS_FILE_SAVE_FAILED );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GAMEGENIEMANAGER::ImportCodes()
{
	PDXSTRING filename;

	const BOOL succeeded = UTILITIES::BrowseOpenFile
	(
	    filename,
		hDlg,
		IDS_FILE_LOAD_GAMEGENIE,
   		"Nestopia Script Files (*.nsp)\0"
   		"*.nsp\0"
   		"All files (*.*)\0"
   		"*.*\0",
		application.GetFileManager().GetNspPath().String()
	);

	if (!succeeded)
		return;

	NES::NSP NspFile;

	{
		PDXFILE file( filename, PDXFILE::INPUT );

		if (!file.IsOpen())
		{
			UI::MsgWarning( IDS_FILE_OPEN_ERROR );
			return;
		}

		if (PDX_FAILED(NspFile.Load( file )))
		{
			UI::MsgWarning( IDS_FILE_PARSE_ERROR );			
			return;
		}
	}
			
	NES::NSP::CONTEXT context;
	NspFile.GetContext( context );

	if (!context.GenieCodes.Size())
		return;

	if (codes.Size())
	{
		const BOOL keep = UI::MsgQuestion
		(
			IDS_GAMEGENIE_IMPORT_KEEP_1_2,
			IDS_GAMEGENIE_IMPORT_KEEP_2_2
		);

		if (!keep)
			ClearCodes();
	}

	for (UINT i=0; i < context.GenieCodes.Size(); ++i)
	{
		AddCode
		( 
			context.GenieCodes[i].code, 
			context.GenieCodes[i].enabled, 
			&context.GenieCodes[i].comment 
		);
	}
}
