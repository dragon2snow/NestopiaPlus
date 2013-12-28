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

#include <algorithm>
#include "NstIoFile.hpp"
#include "NstResourceString.hpp"
#include "NstWindowParam.hpp"
#include "NstWindowUser.hpp"
#include "NstDialogCheats.hpp"
#include "NstManagerPaths.hpp"
#include "NstApplicationConfiguration.hpp"
#include "NstIoNsp.hpp"
#include "../core/api/NstApiGameGenie.hpp"
#include <CommCtrl.h>

namespace Nestopia
{
	using namespace Window;

	NST_COMPILE_ASSERT
	(
		IDC_GAMEGENIE_STATIC_CODES  == IDC_GAMEGENIE_TEMP_CODES - 1 &&
		IDC_GAMEGENIE_STATIC_ADD    == IDC_GAMEGENIE_TEMP_ADD - 1 &&
		IDC_GAMEGENIE_STATIC_REMOVE == IDC_GAMEGENIE_TEMP_REMOVE - 1 &&   
		IDC_GAMEGENIE_STATIC_IMPORT == IDC_GAMEGENIE_TEMP_IMPORT - 1 &&  
		IDC_GAMEGENIE_STATIC_EXPORT == IDC_GAMEGENIE_TEMP_EXPORT - 1 &&   
		IDC_GAMEGENIE_STATIC_CLEAR  == IDC_GAMEGENIE_TEMP_CLEAR - 1    
	);

	Cheats::List::Code::Code(ulong p)
	: packed(p), enabled(TRUE) {}

	void Cheats::List::Code::CheckDesc()
	{
		if (desc.Size())
		{
			desc.Remove( '\"' );
			desc.Trim();
		}
	}

	Cheats::List::Codes::~Codes()
	{
		Clear();
	}

	void Cheats::List::Codes::Clear()
	{
		for (Iterator it=Begin(); it != End(); ++it)
			it->desc.Heap::~Heap();

		Collection::Vector<Code>::Clear();
	}

	void Cheats::List::Codes::Load(const Configuration& cfg)
	{
		String::Stack<32> index("gamegenie ");

		for (;;)
		{
			index(10) = Size() + 1;
			const String::Heap& string = cfg[index];

			if (string.Empty() || string.Size() == MAX_CODES)
				break;

			ulong packed;

			if (NES_SUCCEEDED(Nes::GameGenie::Decode( string, packed )) && Add( packed ))
			{
				Code& code = Back();

				code.enabled = 
				(
					string.Size() >= 6 + 3 && string[6] == ' ' ? string( 7, 2 ) == "on" :
		     		string.Size() >= 8 + 3 && string[8] == ' ' && string( 9, 2 ) == "on"
				);

				if (string.Back() == '\"')
				{
					code.desc = string.AfterFirstOf( '\"' );
					code.desc.Shrink();
					code.desc.Trim();
				}
			}
		}
	}

	void Cheats::List::Codes::Save(Configuration& cfg) const
	{
		String::Stack<32> index("gamegenie ");

		for (ConstIterator it=Begin(); it != End(); ++it)
		{
			index(10) = (uint) (1 + it - Begin());
			String::Heap& string = cfg[index].GetString();

			char characters[9];
			Nes::GameGenie::Encode( it->packed, characters );

			string.Reserve( 8 + 5 + 2 + it->desc.Size() );
			string << cstring(characters) << (it->enabled ? " on " : " off ");

			if (it->desc.Size())
				string << '\"' << it->desc << '\"';
		}
	}

	uint Cheats::List::Codes::Save(Io::Nsp::Context& context) const
	{
		const uint oldSize = context.cheats.size();
		context.cheats.reserve( oldSize + Size() );

		for (ConstIterator it=Begin(); it != End(); ++it)
		{
			if (std::find( context.cheats.begin(), context.cheats.begin() + oldSize, it->packed ) == context.cheats.begin() + oldSize)
			{
				context.cheats.push_back( Io::Nsp::Context::Cheat() );
				Io::Nsp::Context::Cheat& cheat = context.cheats.back();

				cheat.code = it->packed;
				cheat.enabled = it->enabled;
				cheat.desc = it->desc;
			}
		}

		return context.cheats.size() - oldSize;
	}

	ibool Cheats::List::Codes::Add(ulong packed)
	{
		NST_ASSERT( Size() < MAX_CODES );

		if (!Find( packed ))
		{
			PushBack( Code(packed) );
			return TRUE;
		}

		return FALSE;
	}
  
	Cheats::List::ListView::ListView(List* const list,Dialog& parent,const uint index)
	: notificationHandler( IDC_GAMEGENIE_STATIC_CODES + index, parent.Messages() )
	{
		NST_ASSERT( index <= 1 );

		static const Control::NotificationHandler::Entry<List> notifications[] =
		{
			{ LVN_KEYDOWN,     &List::OnKeyDown	    },
			{ LVN_ITEMCHANGED, &List::OnItemChanged },
			{ LVN_INSERTITEM,  &List::OnInsertItem  },
			{ LVN_DELETEITEM,  &List::OnDeleteItem  }
		};

		notificationHandler.Add( list, notifications );

		static const MsgHandler::Entry<List> commands[2][5] =
		{
			{
				{ IDC_GAMEGENIE_STATIC_ADD,    &List::OnCmdAdd    },
				{ IDC_GAMEGENIE_STATIC_REMOVE, &List::OnCmdRemove },
				{ IDC_GAMEGENIE_STATIC_EXPORT, &List::OnCmdExport },
				{ IDC_GAMEGENIE_STATIC_IMPORT, &List::OnCmdImport },
				{ IDC_GAMEGENIE_STATIC_CLEAR,  &List::OnCmdClear  }
			},
			{
				{ IDC_GAMEGENIE_TEMP_ADD,    &List::OnCmdAdd    },
				{ IDC_GAMEGENIE_TEMP_REMOVE, &List::OnCmdRemove },
				{ IDC_GAMEGENIE_TEMP_EXPORT, &List::OnCmdExport },
				{ IDC_GAMEGENIE_TEMP_IMPORT, &List::OnCmdImport },
				{ IDC_GAMEGENIE_TEMP_CLEAR,  &List::OnCmdClear  }
			}
		};

		parent.Commands().Add( list, commands[index] );
	}

	void Cheats::List::ListView::Init(const Control::ListView listView)
	{
		static_cast<Control::ListView&>(*this) = listView;
		StyleEx() = LVS_EX_FULLROWSELECT|LVS_EX_CHECKBOXES;
	}

	Cheats::List::List(CodeDialog& cd,const Managers::Paths& p)
	: 
	codeDialog (cd),
	paths (p),
	listView (NULL)
	{}

	Cheats::List::~List()
	{
		delete listView;
	}

	void Cheats::List::AddToDialog(const Code& code) const
	{
		NST_ASSERT( listView && listView->GetHandle() );

		uint a,v,c; bool u;
		Nes::GameGenie::Unpack( code.packed, a, v, c, u );

		const String::Hex address( (u16) a );
		const String::Hex value( (u8) v );
		const String::Stack<8> compare( u ? String::Hex( (u8) c ) : "-" );

		char characters[8+1];
		Nes::GameGenie::Encode( code.packed, characters );

		const int index = listView->Add( cstring(characters), listView->Size(), code.enabled );

		cstring const table[] =
		{
			address, value, compare, code.desc
		};

		for (uint i=0; i < NST_COUNT(table); ++i)
			(*listView)[index].Text(i+1) << table[i];

		listView->Columns().Align();
	}

	ibool Cheats::List::Add(ulong packed,const Generic::Stream stream)
	{
		if (codes.Add( packed ))
		{
			Code& code = codes.Back();

			stream >> code.desc;
			code.CheckDesc();

			AddToDialog( code );			
			return TRUE;
		}

		return FALSE;
	}

	void Cheats::List::Load(const Io::Nsp::Context& context)
	{
		for (Io::Nsp::Context::Cheats::const_iterator it=context.cheats.begin(); it != context.cheats.end(); ++it)
		{
			if (codes.Size() == MAX_CODES)
				break;

			if (codes.Add( it->code ))
			{
				Code& code = codes.Back();

				code.enabled = it->enabled;
				code.desc = it->desc;
				code.CheckDesc();

				if (listView && listView->GetHandle())
					AddToDialog( code );
			}
		}
	}

	void Cheats::List::Import(const String::Generic path)  
	{
		Managers::Paths::File file;

		if (paths.Load( file, Managers::Paths::File::SCRIPT|Managers::Paths::File::ARCHIVE, path ))
		{
			Io::Nsp::Context context;

			try
			{
				Io::Nsp::File().Load( file.data, context );
			}
			catch (...)
			{
				User::Fail( IDS_FILE_ERR_INVALID );
				return;
			}

			Load( context );
		}
	}

	void Cheats::List::InitDialog(Dialog& parent,const uint index)
	{
		NST_ASSERT( index <= 1 );

		if (listView == NULL)
			listView = new ListView( this, parent, index );

		listView->Init( parent.ListView(index + IDC_GAMEGENIE_STATIC_CODES) );

		listView->controls[ ADD	   ] = parent.Control( index + IDC_GAMEGENIE_STATIC_ADD    );
		listView->controls[ REMOVE ] = parent.Control( index + IDC_GAMEGENIE_STATIC_REMOVE );
		listView->controls[ IMPORT ] = parent.Control( index + IDC_GAMEGENIE_STATIC_IMPORT );
		listView->controls[ EXPORT ] = parent.Control( index + IDC_GAMEGENIE_STATIC_EXPORT );
		listView->controls[ CLEAR  ] = parent.Control( index + IDC_GAMEGENIE_STATIC_CLEAR  );

		NST_COMPILE_ASSERT
		(
		    IDS_CHEAT_ADDRESS     - IDS_CHEAT_CODE == 1 &&
			IDS_CHEAT_VALUE       - IDS_CHEAT_CODE == 2 &&
			IDS_CHEAT_COMPARE     - IDS_CHEAT_CODE == 3 &&
			IDS_CHEAT_DESCRIPTION - IDS_CHEAT_CODE == 4
		);

		listView->Columns().Clear();

		for (uint i=0; i < 5; ++i)
			listView->Columns().Insert( i, Resource::String(IDS_CHEAT_CODE+i) );

		if (codes.Size())
		{
			for (Codes::ConstIterator it=codes.Begin(); it != codes.End(); ++it)
				AddToDialog( *it );
		}
		else
		{
			listView->controls[ CLEAR ].Disable();
			listView->controls[ EXPORT ].Disable();
		}

		listView->Columns().Align();
		
		listView->controls[REMOVE].Disable();
	}

	void Cheats::List::OnKeyDown(const NMHDR& nmhdr)
	{
		switch (reinterpret_cast<const NMLVKEYDOWN&>(nmhdr).wVKey)
		{
			case VK_INSERT: 
	
				if (codes.Size() < MAX_CODES)
					codeDialog.Open( listView->GetHandle() );
	
				break;
	
			case VK_DELETE: 
	
				listView->Selection().Delete();
				break;
		}
	}

	void Cheats::List::OnItemChanged(const NMHDR& nmhdr)
	{
		const NMLISTVIEW& nm = reinterpret_cast<const NMLISTVIEW&>(nmhdr);

		if ((nm.uOldState ^ nm.uNewState) & LVIS_SELECTED)
			listView->controls[REMOVE].Enable( nm.uNewState & LVIS_SELECTED );

		// As documented on MSDN the image index for the checked box is 2 (unchecked is 1)

		if ((nm.uOldState ^ nm.uNewState) & LVIS_STATEIMAGEMASK)
			codes[nm.lParam].enabled = (nm.uNewState & LVIS_STATEIMAGEMASK) == INDEXTOSTATEIMAGEMASK( 2 );
	}

	void Cheats::List::OnInsertItem(const NMHDR&)
	{
		if (codes.Size() == 1)
		{
			listView->controls[ CLEAR ].Enable();
			listView->controls[ EXPORT ].Enable();
		}
	}

	void Cheats::List::OnDeleteItem(const NMHDR& nmhdr)
	{
		Code* const code = codes + reinterpret_cast<const NMLISTVIEW&>(nmhdr).lParam;

		code->desc.Heap::~Heap();
		codes.Erase( code );

		if (codes.Empty())
		{
			listView->controls[ CLEAR ].Disable();
			listView->controls[ EXPORT ].Disable();
			listView->controls[ REMOVE ].Disable();
		}
	}

	ibool Cheats::List::OnCmdAdd(Param& param) 
	{
		if (param.Button().IsClicked() && codes.Size() < MAX_CODES)
			codeDialog.Open( listView->GetHandle() );

		return TRUE;
	}

	ibool Cheats::List::OnCmdRemove(Param& param)  
	{
		if (param.Button().IsClicked())
		{
			NST_VERIFY( codes.Size() );
			listView->Selection().Delete();
		}

		return TRUE;
	}

	ibool Cheats::List::OnCmdExport(Param& param)  
	{
		if (param.Button().IsClicked())
		{
			NST_VERIFY( codes.Size() );

			const Managers::Paths::TmpPath fileName
			(
				paths.BrowseSave( Managers::Paths::File::SCRIPT )
			);

			if (fileName.Size())
			{
				Io::Nsp::Context context;

				if (Io::File::FileExist( fileName ) && User::Confirm( IDS_GAMEGENIE_EXPORTEXISTING ))
				{
					try
					{
						Io::Nsp::File::Buffer buffer;
						Io::File( fileName, Io::File::COLLECT ).Stream() >> buffer;
						Io::Nsp::File().Load( buffer, context );
					}
					catch (...)
					{
						User::Warn( IDS_GAMEGENIE_EXPORTEXISTING_ERROR );
						context.Reset();
					}
				}

				if (Save( context ))
				{
					try
					{
						Io::Nsp::File::Buffer buffer;
						Io::Nsp::File().Save( buffer, context );
						Io::File( fileName, Io::File::DUMP ).Stream() << buffer;
					}
					catch (...)
					{
						User::Fail( IDS_FILE_ERR_INVALID );
					}
				}
			}
		}

		return TRUE;
	}

	ibool Cheats::List::OnCmdImport(Param& param)  
	{
		if (param.Button().IsClicked())
			Import();

		return TRUE;
	}

	ibool Cheats::List::OnCmdClear(Param& param)
	{
		if (param.Button().IsClicked())
		{
			NST_VERIFY( codes.Size() );
			listView->Clear();
		}

		return TRUE;
	}

	template<typename T,typename U,typename V>
	inline Cheats::CodeDialog::CodeDialog(T* instance,const U& messages,const V& commands)
	: Dialog( IDD_GAMEGENIE_ADDCODE, instance, messages, commands ), listView(NULL) {}

	inline void Cheats::CodeDialog::Open(HWND hWnd)
	{
		listView = hWnd;
		Dialog::Open();
	}

	inline HWND Cheats::CodeDialog::GetListView() const
	{
		return listView;
	}

	struct Cheats::Handlers
	{
		static const MsgHandler::Entry<Cheats> messages[];
		static const MsgHandler::Entry<Cheats> commands[];
		static const MsgHandler::Entry<Cheats> codeMessages[];
		static const MsgHandler::Entry<Cheats> codeCommands[];
	};

	const MsgHandler::Entry<Cheats> Cheats::Handlers::messages[] =
	{
		{ WM_INITDIALOG, &Cheats::OnInitMainDialog },
		{ WM_DROPFILES,  &Cheats::OnDropFiles      }
	};

	const MsgHandler::Entry<Cheats> Cheats::Handlers::codeMessages[] =
	{
		{ WM_INITDIALOG, &Cheats::OnInitCodeDialog }
	};

	const MsgHandler::Entry<Cheats> Cheats::Handlers::commands[] =
	{
		{ IDC_GAMEGENIE_CLOSE, &Cheats::OnClose }
	};

	const MsgHandler::Entry<Cheats> Cheats::Handlers::codeCommands[] =
	{
		{ IDC_GAMEGENIE_ADDCODE_SUBMIT,   &Cheats::OnCodeCmdSubmit   },
		{ IDC_GAMEGENIE_ADDCODE_CANCEL,   &Cheats::OnCodeCmdCancel   },
		{ IDC_GAMEGENIE_ADDCODE_VALIDATE, &Cheats::OnCodeCmdValidate }
	};

	Cheats::Cheats(const Configuration& cfg,const Managers::Paths& paths)
	: 
	mainDialog    ( IDD_GAMEGENIE, this, Handlers::messages, Handlers::commands ),
	codeDialog    ( this, Handlers::codeMessages, Handlers::codeCommands ),
	staticList    ( codeDialog, paths ),
	tempList      ( codeDialog, paths )
	{
		staticList.Load( cfg );
	}

	Cheats::~Cheats()
	{
	}

	void Cheats::Save(Configuration& cfg) const
	{
		staticList.Save( cfg );
	}

	void Cheats::Save(Io::Nsp::Context& context) const
	{
		tempList.Save( context );
		staticList.Save( context );
	}

	void Cheats::Load(const Io::Nsp::Context& context)
	{
		tempList.Load( context );
	}

	uint Cheats::ClearTemporaryCodes()
	{
		uint prev = tempList.Size();
		tempList.Clear();
		return prev;
	}

	ibool Cheats::OnInitMainDialog(Param&)
	{
		staticList.InitDialog( mainDialog, 0 );
		tempList.InitDialog( mainDialog, 1 );
		return TRUE;
	}

	ibool Cheats::OnDropFiles(Param& param)
	{
		if (param.DropFiles().IsInside( staticList.GetHandle() ))
		{
			staticList.Import( param.DropFiles()[0] );
		}
		else if (param.DropFiles().IsInside( tempList.GetHandle() ))
		{
			tempList.Import( param.DropFiles()[0] );
		}

		return TRUE;
	}

	ibool Cheats::OnClose(Param& param)
	{
		if (param.Button().IsClicked())
			mainDialog.Close();

		return TRUE;
	}

	ibool Cheats::OnInitCodeDialog(Param&)
	{
		codeDialog.Edit( IDC_GAMEGENIE_ADDCODE_CHARACTERS ).Limit( 8 );
		codeDialog.Edit( IDC_GAMEGENIE_ADDCODE_ADDRESS    ).Limit( 4 );
		codeDialog.Edit( IDC_GAMEGENIE_ADDCODE_VALUE      ).Limit( 2 );
		codeDialog.Edit( IDC_GAMEGENIE_ADDCODE_COMPARE    ).Limit( 2 );
		codeDialog.Edit( IDC_GAMEGENIE_ADDCODE_DESC       ).Limit( 256 );

		codeDialog.RadioButton( IDC_GAMEGENIE_ADDCODE_ENCODED ).Check( TRUE  );
		codeDialog.RadioButton( IDC_GAMEGENIE_ADDCODE_DECODED ).Check( FALSE );

		return TRUE;
	}

	ibool Cheats::GetPackedEncodedCode(ulong& packed) const
	{
		String::Smart<16> string;

		codeDialog.Edit( IDC_GAMEGENIE_ADDCODE_CHARACTERS ) >> string;
		return NES_SUCCEEDED(Nes::GameGenie::Decode( string, packed ));
	}

	ibool Cheats::GetPackedDecodedCode(ulong& packed) const
	{
		String::Smart<16> string;

		uint address, value, compare = 0;

		if (codeDialog.Edit( IDC_GAMEGENIE_ADDCODE_ADDRESS ) >> string)
		{
			string(0) << "0x";

			if (string >> address && codeDialog.Edit( IDC_GAMEGENIE_ADDCODE_VALUE ) >> string)
			{
				string(0) << "0x";

				if (string >> value)
				{
					if (codeDialog.Edit( IDC_GAMEGENIE_ADDCODE_COMPARE ) >> string)
					{
						string(0) << "0x";

						if (!(string >> compare))
							return FALSE;
					}

					return NES_SUCCEEDED(Nes::GameGenie::Pack( address, value, compare, string.Size() > 2, packed ));
				}
			}
		}

		return FALSE;
	}

	ibool Cheats::OnCodeCmdCancel(Param& param)  
	{
		if (param.Button().IsClicked())
			codeDialog.Close();

		return TRUE;
	}

	ibool Cheats::OnCodeCmdValidate(Param& param)
	{
		if (param.Button().IsClicked())
		{
			cstring result = "INVALID";

			ulong packed;

			if (codeDialog.RadioButton( IDC_GAMEGENIE_ADDCODE_ENCODED ).IsChecked())
			{
				if (GetPackedEncodedCode( packed ))
				{
					uint address, value, compare; bool useCompare;
					Nes::GameGenie::Unpack( packed, address, value, compare, useCompare );

					codeDialog.Edit( IDC_GAMEGENIE_ADDCODE_ADDRESS ) << String::Hex( u16(address), true );
					codeDialog.Edit( IDC_GAMEGENIE_ADDCODE_VALUE   ) << String::Hex( u8(value), true );
					codeDialog.Edit( IDC_GAMEGENIE_ADDCODE_COMPARE ) << (useCompare ? String::Hex( u8(compare), true ) : "");

					result = "VALID";
				}
			}
			else
			{
				if (GetPackedDecodedCode( packed ))
				{
					char characters[9];
					Nes::GameGenie::Encode( packed, characters );

					codeDialog.Edit( IDC_GAMEGENIE_ADDCODE_CHARACTERS ) << cstring(characters);

					result = "VALID";
				}
			}

			codeDialog.Edit( IDC_GAMEGENIE_ADDCODE_RESULT ) << result;
		}

		return TRUE;
	}

	ibool Cheats::OnCodeCmdSubmit(Param& param)
	{
		if (param.Button().IsClicked())
		{
			ibool result;
			ulong packed;

			if (codeDialog.RadioButton( IDC_GAMEGENIE_ADDCODE_ENCODED ).IsChecked())
				result = GetPackedEncodedCode( packed );
			else
				result = GetPackedDecodedCode( packed );

			if (result)
			{
				List& list = codeDialog.GetListView() == staticList.GetHandle() ? staticList : tempList;

				if (list.Add( packed, codeDialog.Edit(IDC_GAMEGENIE_ADDCODE_DESC).Text() ))
					codeDialog.Close();
				else
					User::Warn( IDS_GAMEGENIE_DUPLICATE_CODE, IDS_GAMEGENIE );
			}
			else
			{
				User::Warn( IDS_GAMEGENIE_INVALID_CODE, IDS_GAMEGENIE );
			}
		}

		return TRUE;
	}
}
