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

#include <Windows.h>
#include <CommCtrl.h>
#include "../paradox/PdxFile.h"
#include "resource/resource.h"
#include "NstGameGenieManager.h"
#include "NstApplication.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT GAMEGENIEMANAGER::Create(PDXFILE* const file)
{
	hDlg = NULL;
	hList = NULL;

	if (file)
	{
		CODE code;

		const UINT count = file->Read<U32>();

		for (UINT i=0; i < count; ++i)
		{
			code.code = file->Read<U32>();

			file->Text().Read( code.comment );

			const NES::IO::GAMEGENIE::STATE state = 
			(
		     	file->Read<U8>() ? NES::IO::GAMEGENIE::ENABLE : NES::IO::GAMEGENIE::DISABLE
			);
			
			if (PDX_SUCCEEDED(NesSet( code.code, state )))
				codes.Insert( code );
		}
	}

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT GAMEGENIEMANAGER::Destroy(PDXFILE* const file)
{
	if (file)
	{
		file->Write( U32(codes.Size()) );

		for (CODES::CONSTITERATOR iterator = codes.Begin(); iterator != codes.End(); ++iterator)
		{
			const CODE& code = *iterator;
			file->Write( U32(code.code) );
			file->Text().Write( code.comment );
			file->Write( U8(NesIsEnabled( code.code ) ? 1 : 0) );
		}
	}

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
			hList = GetDlgItem( h, IDC_GAMEGENIE_LIST );
			UpdateDialog();
     		return TRUE;

		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
     			case IDC_GAMEGENIE_ADD:       CreateCodeDialog();   return TRUE;
				case IDC_GAMEGENIE_REMOVE:    RemoveCode();         return TRUE;
     			case IDC_GAMEGENIE_CLEAR_ALL: ClearCodes();         return TRUE;
				case IDC_GAMEGENIE_IMPORT:    ImportCodes();        return TRUE;
				case IDC_GAMEGENIE_EXPORT:    ExportCodes();        return TRUE;
				case IDC_GAMEGENIE_OK:        EndDialog( hDlg, 0 ); return TRUE;
			}
			return FALSE;

     	case WM_CLOSE:

     		EndDialog( hDlg, 0 );
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

			CheckRadioButton
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
				case IDC_GAMEGENIE_ADDCODE_CANCEL:   EndDialog( hDlg, 0 ); return TRUE;
				case IDC_GAMEGENIE_ADDCODE_VALIDATE: PDX_ASSERT( ggm ); ggm->ValidateCode( hDlg ); return TRUE;
			}
			return FALSE;

		case WM_CLOSE:

			PDX_ASSERT( ggm );
			ggm = NULL;
			EndDialog( hDlg, 0 );
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

	characters.Buffer().Resize( 10 );
	characters.Buffer().Resize( GetDlgItemText( hDlg, IDC_GAMEGENIE_ADDCODE_CHARACTERS, characters.Begin(), characters.Length() ) + 1);

	CODE code;

	code.comment.Buffer().Resize( 512 );
	code.comment.Buffer().Resize( GetDlgItemText( hDlg, IDC_GAMEGENIE_ADDCODE_COMMENT, code.comment.Begin(), code.comment.Length() ) + 1);

	if (PDX_SUCCEEDED(NesDecode( characters, code.code )) && codes.Find( code ) == codes.End() && AddCode( code, TRUE ))
		codes.Insert( code );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GAMEGENIEMANAGER::ValidateCode(HWND hDlg) const
{
	CHAR buffer[32];
	ULONG packed;

	if (IsDlgButtonChecked( hDlg, IDC_GAMEGENIE_ADDCODE_DECODED) == BST_CHECKED)
	{
		if (GetDlgItemText( hDlg, IDC_GAMEGENIE_ADDCODE_ADDRESS, buffer, 32-1 ))
		{
			const UINT address = strtoul( buffer, NULL, 16 );

			if (GetDlgItemText( hDlg, IDC_GAMEGENIE_ADDCODE_VALUE, buffer, 32-1 ))
			{
				const UINT value      = strtoul( buffer, NULL, 16 );
				const BOOL UseCompare = GetDlgItemText( hDlg, IDC_GAMEGENIE_ADDCODE_COMPARE, buffer, 32-1 );				
				const UINT compare    = UseCompare ? strtoul( buffer, NULL, 16 ) : 0x00;

				if (PDX_SUCCEEDED(NesPack( address, value, compare, UseCompare, packed )))
				{
					PDXSTRING characters;

					if (PDX_SUCCEEDED(NesEncode( packed, characters )))
					{
						SetDlgItemText( hDlg, IDC_GAMEGENIE_ADDCODE_CHARACTERS, characters );
						SetDlgItemText( hDlg, IDC_GAMEGENIE_ADDCODE_RESULT, "VALID" );
						return;
					}
				}
			}
		}
	}
	else
	{
		const UINT length = GetDlgItemText( hDlg, IDC_GAMEGENIE_ADDCODE_CHARACTERS, buffer, 32-1 );

		if (length && PDX_SUCCEEDED(NesDecode( buffer, packed )))
		{
			UINT address, value, compare;
			BOOL UseCompare;

			if (PDX_SUCCEEDED(NesUnpack( packed, address, value, compare, UseCompare )))
			{
				SetDlgItemText( hDlg, IDC_GAMEGENIE_ADDCODE_ADDRESS, _ultoa( address, buffer, 16 ) );
				SetDlgItemText( hDlg, IDC_GAMEGENIE_ADDCODE_VALUE,   _ultoa( value,   buffer, 16 ) );
				SetDlgItemText( hDlg, IDC_GAMEGENIE_ADDCODE_COMPARE, UseCompare ? _ultoa( compare, buffer, 16 ) : "" );
				SetDlgItemText( hDlg, IDC_GAMEGENIE_ADDCODE_RESULT, "VALID" );
				return;
			}
		}
	}

	SetDlgItemText( hDlg, IDC_GAMEGENIE_ADDCODE_RESULT, "INVALID" );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GAMEGENIEMANAGER::UpdateDialog()
{
	PDX_ASSERT( hList );
	ListView_SetExtendedListViewStyle( hList, LVS_EX_CHECKBOXES|LVS_EX_FULLROWSELECT );
	UpdateColumns();
	UpdateCodes();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GAMEGENIEMANAGER::CloseDialog()
{
	PDX_ASSERT( hList );

	const INT num = ListView_GetItemCount( hList );

	for (INT i=0; i < num; ++i)
	{
		CHAR buffer[16];
		ListView_GetItemText( hList, i, 0, buffer, 16-1 );

		ULONG packed;

		if (PDX_SUCCEEDED(NesDecode( buffer, packed )))
		{
			NesEnable
			( 
		    	packed, 
				ListView_GetCheckState( hList, i ) ? NES::IO::GAMEGENIE::ENABLE : NES::IO::GAMEGENIE::DISABLE 
			);
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
     	application.GetHInstance(), 
		MAKEINTRESOURCE(IDD_GAMEGENIE_ADDCODE), 
		hDlg, 
		StaticCodeDialogProc,
		PDX_CAST(LPARAM,this)
	); 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL GAMEGENIEMANAGER::AddCode(CODE& code,const BOOL ForceEnable)
{
	PDXSTRING text;
	
	if (PDX_FAILED(NesEncode( code.code, text )))
		return FALSE;

	LVITEM item;
	PDXMemZero( item );

	item.mask       = LVIF_TEXT;
	item.pszText    = text.Begin();
	item.cchTextMax = text.Length();
	item.iItem      = INT_MAX;

	PDX_ASSERT( hList );

	INT slot;

	if ((slot = ListView_InsertItem( hList, &item )) == -1)
		return FALSE;
	
	ListView_SetCheckState
	( 
    	hList, 
		slot, 
		(ForceEnable ? TRUE : NesIsEnabled( code.code )) 
	);

	if (PDX_FAILED(NesSet( code.code, ForceEnable ? NES::IO::GAMEGENIE::ENABLE : NES::IO::GAMEGENIE::NOCHANGE )))
	{
		ListView_DeleteItem( hList, &item );
		return FALSE;
	}

	UINT address;
	UINT data;
	UINT compare;
	BOOL UseCompare;

	const PDXRESULT result = NesUnpack( code.code, address, data, compare, UseCompare );
	PDX_ASSERT( PDX_SUCCEEDED(result) );

	text.Set( address, PDXSTRING::HEX );
	ListView_SetItemText( hList, slot, 1, text.Begin() );

	text.Set( data, PDXSTRING::HEX );
	ListView_SetItemText( hList, slot, 2, text.Begin() );

	if (UseCompare)
	{
    	text.Set( compare, PDXSTRING::HEX );
	}
	else
	{
		text.Clear();
	}

	ListView_SetItemText( hList, slot, 3, text.Begin() );
	ListView_SetItemText( hList, slot, 4, code.comment.Begin() );

	return TRUE;
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

		ListView_GetItemText( hList, slot, 0, buffer, 16-1 );
		ListView_DeleteItem( hList, slot );

		ULONG packed;

		if (PDX_SUCCEEDED(NesDecode( buffer, packed )))
		{
			codes.Erase( packed );
			NesEnable( packed, NES::IO::GAMEGENIE::DISABLE );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GAMEGENIEMANAGER::ClearCodes()
{
	NesDestroy();
	ListView_DeleteAllItems( hList );
	codes.Destroy();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GAMEGENIEMANAGER::UpdateCodes()
{
	PDX_ASSERT( hList && ListView_GetItemCount(hList) == 0 );

	for (CODES::ITERATOR iterator = codes.Begin(); iterator != codes.End(); )
	{
		if (AddCode(*iterator))
		{
			++iterator;
		}
		else
		{
			CODES::ITERATOR kill( iterator++ );
			codes.Erase( *kill );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL GAMEGENIEMANAGER::NesIsEnabled(const ULONG packed) const
{
	NES::IO::GAMEGENIE::CONTEXT context;
	
	context.op     = NES::IO::GAMEGENIE::GETSTATE;
	context.packed = packed;

	nes->GetGameGenieContext( context );

	return context.state == NES::IO::GAMEGENIE::ENABLE ? TRUE : FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT GAMEGENIEMANAGER::NesEncode(const ULONG packed,PDXSTRING& characters) const
{
	NES::IO::GAMEGENIE::CONTEXT context;
	
	context.op     = NES::IO::GAMEGENIE::ENCODE;
	context.packed = packed;
	
	PDX_TRY(nes->GetGameGenieContext( context ));

	characters = context.characters;

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT GAMEGENIEMANAGER::NesDecode(const CHAR* const characters,ULONG& packed) const
{
	NES::IO::GAMEGENIE::CONTEXT context;
	
	context.op         = NES::IO::GAMEGENIE::DECODE;
	context.characters = characters;
	
	PDX_TRY(nes->GetGameGenieContext( context ));
	
	packed = context.packed;
	
	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT GAMEGENIEMANAGER::NesPack(const UINT address,const UINT value,const UINT compare,const BOOL UseCompare,ULONG& packed) const
{
	NES::IO::GAMEGENIE::CONTEXT context;
	
	context.op         = NES::IO::GAMEGENIE::PACK;
	context.address    = address;
	context.value      = value;
	context.compare    = compare;
	context.UseCompare = UseCompare;
	
	PDX_TRY(nes->GetGameGenieContext( context ));

	packed = context.packed;
	
	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT GAMEGENIEMANAGER::NesUnpack(const ULONG packed,UINT& address,UINT& value,UINT& compare,BOOL& UseCompare) const
{
	NES::IO::GAMEGENIE::CONTEXT context;
	
	context.op     = NES::IO::GAMEGENIE::UNPACK;
	context.packed = packed;
	
	PDX_TRY(nes->GetGameGenieContext( context ));

	address    = context.address;
	value      = context.value;
	compare    = context.compare;
	UseCompare = context.UseCompare;

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT GAMEGENIEMANAGER::NesEnable(const ULONG packed,const NES::IO::GAMEGENIE::STATE state)
{
	NES::IO::GAMEGENIE::CONTEXT context;
	
	context.op     = NES::IO::GAMEGENIE::SETSTATE;
	context.packed = packed;
	context.state  = state;

	return nes->SetGameGenieContext( context );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT GAMEGENIEMANAGER::NesSet(const ULONG packed,const NES::IO::GAMEGENIE::STATE state)
{
	NES::IO::GAMEGENIE::CONTEXT context;
	
	context.op     = NES::IO::GAMEGENIE::ADD;
	context.packed = packed;
	context.state  = state;
	
	return nes->SetGameGenieContext( context );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT GAMEGENIEMANAGER::NesDestroy()
{
	NES::IO::GAMEGENIE::CONTEXT context;
	
	context.op = NES::IO::GAMEGENIE::DESTROYALL;
	
	return nes->SetGameGenieContext( context );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GAMEGENIEMANAGER::UpdateColumns()
{
	PDX_ASSERT( hList );

	LV_COLUMN column;
	PDXMemZero( column );

	column.mask     = LVCF_FMT|LVCF_TEXT|LVCF_SUBITEM|LVCF_WIDTH;
	column.fmt      = LVCFMT_LEFT;
	column.cx       = 70;
	column.iSubItem = -1;

	column.pszText = TEXT( "Code"    ); column.cchTextMax = strlen(column.pszText); ListView_InsertColumn( hList, 0, &column );
	column.pszText = TEXT( "Address" ); column.cchTextMax = strlen(column.pszText); ListView_InsertColumn( hList, 1, &column );
	column.pszText = TEXT( "Data"    ); column.cchTextMax = strlen(column.pszText); ListView_InsertColumn( hList, 2, &column );
	column.pszText = TEXT( "Compare" ); column.cchTextMax = strlen(column.pszText); ListView_InsertColumn( hList, 3, &column );

	column.pszText = TEXT( "Comment" ); 
	column.cx = 172;
	column.cchTextMax = strlen(column.pszText); 
	ListView_InsertColumn( hList, 4, &column );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GAMEGENIEMANAGER::ExportCodes()
{
	PDXSTRING name;
	name.Resize( NST_MAX_PATH );

	OPENFILENAME ofn;
	PDXMemZero( ofn );

	ofn.lStructSize     = sizeof(ofn);
	ofn.hwndOwner       = hWnd;
	ofn.hInstance       = application.GetHInstance();
	ofn.lpstrFilter     = NULL;
	ofn.nFilterIndex    = 1;
	ofn.lpstrInitialDir	= NULL;
	ofn.lpstrFile       = name.Begin();
	ofn.lpstrTitle      = "Export Game Genie codes";
	ofn.nMaxFile        = NST_MAX_PATH;
	ofn.Flags           = OFN_HIDEREADONLY|OFN_PATHMUSTEXIST;

	if (GetSaveFileName(&ofn))
	{
		name.Validate();

		PDXFILE file;
		
		if (PDX_FAILED(file.Open(name,PDXFILE::OUTPUT)))
			return;

		CHAR buffer[512];

		const INT num = ListView_GetItemCount( hList );

		for (INT i=0; i < num; ++i)
		{
			ListView_GetItemText( hList, i, 0, buffer, 14 );			
			const UINT length = strlen(buffer);
			file.Write( buffer, buffer + length );
			memset( buffer, ' ', 10 );
			file.Write( buffer + length, buffer + 10 );
			ListView_GetItemText( hList, i, 4, buffer, 512-1 );
			file.Write( buffer, buffer + strlen(buffer) );
			file.Write( '\r' );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GAMEGENIEMANAGER::ImportCodes()
{
	PDXSTRING name;
	name.Resize( NST_MAX_PATH );

	OPENFILENAME ofn;
	PDXMemZero( ofn );

	ofn.lStructSize     = sizeof(ofn);
	ofn.hwndOwner       = hWnd;
	ofn.hInstance       = application.GetHInstance();
	ofn.lpstrFilter     = NULL;
	ofn.nFilterIndex    = 1;
	ofn.lpstrInitialDir	= NULL;
	ofn.lpstrFile       = name.Begin();
	ofn.lpstrTitle      = "Import Game Genie codes";
	ofn.nMaxFile        = NST_MAX_PATH;
	ofn.Flags           = OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;

	if (GetOpenFileName(&ofn))
	{
		name.Validate();

		PDXFILE file;

		if (PDX_FAILED(file.Open(name,PDXFILE::INPUT)))
			return;

		ClearCodes();

		CHAR buffer[512];
		CODE code;

		while (!file.Eof())
		{
			static const CHAR ValidCharacter[32] =
			{
				'A','a','P','p','Z','z','L','l',
				'G','g','I','i','T','t','Y','y',
				'E','e','O','o','X','x','U','u',
				'K','k','S','s','V','v','N','n'
			};

			CHAR character = file.Peek<CHAR>();
			BOOL god = FALSE;

			for (UINT i=0; i < 32; ++i)
			{
				if (character == ValidCharacter[i])
				{
					god = TRUE;
					break;
				}
			}

			if (god == FALSE)
				goto hell;

			UINT length;

			for (length=0; !file.Eof() && (length < 512) && (character = file.Read<CHAR>()) != ' '; ++length)
				buffer[length] = character;

			if (file.Eof())
				goto hell;

			buffer[length] = '\0';

			if (PDX_FAILED(NesDecode( buffer, code.code )))
				goto hell;

			while (!file.Eof() && file.Read<CHAR>() == ' ');

			file.Seek(PDXFILE::CURRENT,-1);

			for (length=0; !file.Eof() && (length < 512) && (character = file.Read<CHAR>()) != '\r'; ++length)
				buffer[length] = character;
						
			buffer[length] = '\0';

			code.comment = buffer;

			if (PDX_SUCCEEDED(NesSet( code.code, NES::IO::GAMEGENIE::ENABLE )))
				codes.Insert( code );
		}

		UpdateCodes();
	}

	return;

hell:

	application.OnWarning( "File is corrupt!" );
}
