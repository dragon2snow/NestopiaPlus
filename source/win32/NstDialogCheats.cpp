////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003-2006 Martin Freij
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
#include "NstApplicationInstance.hpp"
#include "NstApplicationConfiguration.hpp"
#include "NstIoNsp.hpp"
#include <CommCtrl.h>

namespace Nestopia
{
	NST_COMPILE_ASSERT
	(
		IDC_CHEATS_STATIC_CODES  == IDC_CHEATS_TEMP_CODES  - 1 &&
		IDC_CHEATS_STATIC_ADD    == IDC_CHEATS_TEMP_ADD    - 1 &&
		IDC_CHEATS_STATIC_REMOVE == IDC_CHEATS_TEMP_REMOVE - 1 &&   
		IDC_CHEATS_STATIC_IMPORT == IDC_CHEATS_TEMP_IMPORT - 1 &&  
		IDC_CHEATS_STATIC_EXPORT == IDC_CHEATS_TEMP_EXPORT - 1 &&   
		IDC_CHEATS_STATIC_CLEAR  == IDC_CHEATS_TEMP_CLEAR  - 1    
	);

	NST_COMPILE_ASSERT
	(
		IDC_CHEATS_ADDCODE_SEARCH_B			  - IDC_CHEATS_ADDCODE_SEARCH_A == 1 &&
		IDC_CHEATS_ADDCODE_SEARCH_LIST		  - IDC_CHEATS_ADDCODE_SEARCH_A == 2 &&
		IDC_CHEATS_ADDCODE_SEARCH_NONE		  - IDC_CHEATS_ADDCODE_SEARCH_A == 3 &&
		IDC_CHEATS_ADDCODE_SEARCH_R0_A_R1_B	  - IDC_CHEATS_ADDCODE_SEARCH_A == 4 &&
		IDC_CHEATS_ADDCODE_SEARCH_R0_A_R0R1_B - IDC_CHEATS_ADDCODE_SEARCH_A == 5 &&
		IDC_CHEATS_ADDCODE_SEARCH_R0R1_B	  - IDC_CHEATS_ADDCODE_SEARCH_A == 6 &&
		IDC_CHEATS_ADDCODE_SEARCH_R0_L_R1	  - IDC_CHEATS_ADDCODE_SEARCH_A == 7 &&
		IDC_CHEATS_ADDCODE_SEARCH_R0_G_R1	  - IDC_CHEATS_ADDCODE_SEARCH_A == 8 &&
		IDC_CHEATS_ADDCODE_SEARCH_R0_N_R1	  - IDC_CHEATS_ADDCODE_SEARCH_A == 9 &&
		IDC_CHEATS_ADDCODE_SEARCH_RESET		  - IDC_CHEATS_ADDCODE_SEARCH_A == 10
	);

	NST_COMPILE_ASSERT
	(
		IDS_CHEAT_ADDRESS     == IDS_CHEAT_CODE + 1 &&
		IDS_CHEAT_VALUE       == IDS_CHEAT_CODE + 2 &&
		IDS_CHEAT_COMPARE     == IDS_CHEAT_CODE + 3 &&
		IDS_CHEAT_DESCRIPTION == IDS_CHEAT_CODE + 4
	);

	using namespace Window;

	Cheats::List::Code::Code(const Mem& m)
	: mem(m) {}

	Cheats::Searcher::Searcher()
	: hex(FALSE), filter(NO_FILTER), a(0x00), b(0x00) {}

	inline bool Cheats::List::Code::operator == (uint address) const
	{
		return mem.address == address;
	}

	void Cheats::List::Code::CheckDesc()
	{
		if (desc.Length())
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
		String::Stack<32,char> index("cheat ");

		for (uint count=1; ; ++count)
		{
			index(6) = count;
			const GenericString string( cfg[index] );

			if (string.Empty() || Size() == MAX_CODES)
				break;

			String::Stack<3,tchar> state;
			String::Stack<255,tchar> desc;

			Mem mem;

			if ((string[0] >= 'A' && string[0] <= 'Z') || (string[0] >= 'a' && string[0] <= 'z'))
			{
				String::Stack<8,tchar> characters;

				if (::_stscanf( string.Ptr(), _T("%8s %3s %255[^\0]"), characters.Ptr(), state.Ptr(), desc.Ptr() ) < 1)
					continue;
				
				if (NES_FAILED(Nes::Cheats::GameGenieDecode( String::Stack<8,char>(characters).Ptr(), mem )))
					continue;
			}
			else
			{
				int address=INT_MAX, value=INT_MAX, compare=INT_MAX;
				int count = ::_stscanf( string.Ptr(), _T("%x %x %x %3s %255[^\0]"), &address, &value, &compare, state.Ptr(), desc.Ptr() );

				if (count > 2 && compare >= 0x00 && compare <= 0xFF)
				{
					mem.useCompare = true;
					mem.compare = (u8) compare;
				}
				else if (count == 2)
				{
					mem.useCompare = false;
					mem.compare = 0x00;
					count = ::_stscanf( string.Ptr(), _T("%*s %*s %3s %255[^\0]"), state.Ptr(), desc.Ptr() );
				}
				else
				{
					continue;
				}
				
				if (address < 0x0000 || address > 0xFFFF || value < 0x00 || value > 0xFF)
					continue;

				mem.address = (u16) address;
				mem.value = (u8) value;
			}

			Code& code = Add( mem );

			state.Validate();
			code.enabled = (state != _T("off"));

			desc.Validate();

			if (desc.Length() > 2 && desc.Front() == '\"' && desc.Back() == '\"')
			{
				code.desc = desc( 1, desc.Length() - 1 );
				code.desc.Trim();
			}
		}
	}

	void Cheats::List::Codes::Save(Configuration& cfg) const
	{
		String::Stack<32,char> index("cheat ");

		for (ConstIterator it=Begin(), end=End(); it != end; ++it)
		{
			index(6) = (uint) (1 + it - Begin());
			HeapString& string = cfg[index].GetString();

			string.Reserve( 2+4 + 1 + 2+2 + 1 + 2+2 + 4 + 1+1 + it->desc.Length() );

			string << HexString( (u16) it->mem.address )
				   << ' '
				   << HexString( (u8) it->mem.value );

		   if (it->mem.useCompare)
			   string << ' ' << HexString( (u8) it->mem.compare );

			string << (it->enabled ? " on" : " off");

			if (it->desc.Length())
				string << " \"" << it->desc << '\"';
		}
	}

	uint Cheats::List::Codes::Save(Io::Nsp::Context& context) const
	{
		const uint oldSize = context.cheats.size();
		context.cheats.reserve( oldSize + Size() );

		for (ConstIterator it=Begin(), end=End(); it != end; ++it)
		{
			if (std::find( context.cheats.begin(), context.cheats.begin() + oldSize, it->mem.address ) == context.cheats.begin() + oldSize)
			{
				context.cheats.push_back( Io::Nsp::Context::Cheat() );
				Io::Nsp::Context::Cheat& cheat = context.cheats.back();

				cheat.address = it->mem.address;
				cheat.value = it->mem.value;
				cheat.compare = it->mem.compare;
				cheat.useCompare = it->mem.useCompare;
				cheat.enabled = it->enabled;
				cheat.desc = it->desc;
			}
		}
  
		return context.cheats.size() - oldSize;
	}

	Cheats::List::Code& Cheats::List::Codes::Add(const Mem& mem)
	{
		NST_ASSERT( Size() < MAX_CODES );

		if (Code* const code = Find( mem.address ))
		{
			code->mem = mem;
			return *code;
		}
		else
		{
			PushBack( Code(mem) );
			return Back();
		}
	}
  
	Cheats::List::ListView::ListView(List* const list,Dialog& parent,const uint index)
	: notificationHandler( IDC_CHEATS_STATIC_CODES + index, parent.Messages() )
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
				{ IDC_CHEATS_STATIC_ADD,    &List::OnCmdAdd    },
				{ IDC_CHEATS_STATIC_REMOVE, &List::OnCmdRemove },
				{ IDC_CHEATS_STATIC_EXPORT, &List::OnCmdExport },
				{ IDC_CHEATS_STATIC_IMPORT, &List::OnCmdImport },
				{ IDC_CHEATS_STATIC_CLEAR,  &List::OnCmdClear  }
			},
			{
				{ IDC_CHEATS_TEMP_ADD,    &List::OnCmdAdd    },
				{ IDC_CHEATS_TEMP_REMOVE, &List::OnCmdRemove },
				{ IDC_CHEATS_TEMP_EXPORT, &List::OnCmdExport },
				{ IDC_CHEATS_TEMP_IMPORT, &List::OnCmdImport },
				{ IDC_CHEATS_TEMP_CLEAR,  &List::OnCmdClear  }
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

		const HexString address( (u16) code.mem.address );
		const HexString value( (u8) code.mem.value );
		const String::Stack<8,tchar> compare( code.mem.useCompare ? HexString( (u8) code.mem.compare ).Ptr() : _T("-") );

		String::Stack<8,tchar> characters;

		if (code.mem.address >= 0x8000U)
		{
			char gg[8+1];
			Nes::Cheats::GameGenieEncode( code.mem, gg );
			characters = gg;
		}
		else
		{
			characters[0] = '-';
			characters[1] = '\0';
		}

		int index = listView->Size();

		while (index--)
		{
			if ((*listView)[index].Data() == code.mem.address)
			{
				(*listView)[index].Check( code.enabled );
				break;
			}
		}

		if (index == -1)
			index = listView->Add( characters.Ptr(), code.mem.address, code.enabled );

		tstring const table[] =
		{
			address.Ptr(), value.Ptr(), compare.Ptr(), code.desc.Ptr()
		};

		for (uint i=0; i < NST_COUNT(table); ++i)
			(*listView)[index].Text(i+1) << table[i];

		listView->Columns().Align();
	}

	void Cheats::List::Add(const Mem& mem,const Generic::Stream desc)
	{
		Code& code = codes.Add( mem );

		desc >> code.desc;
		code.CheckDesc();

		AddToDialog( code );			
	}

	void Cheats::List::Load(const Io::Nsp::Context& context)
	{
		for (Io::Nsp::Context::Cheats::const_iterator it=context.cheats.begin(); it != context.cheats.end(); ++it)
		{
			if (codes.Size() == MAX_CODES)
				break;

			Code& code = codes.Add( Mem(it->address,it->value,it->compare,it->useCompare) );

			code.enabled = it->enabled;
			code.desc = it->desc;
			code.CheckDesc();

			if (listView && listView->GetHandle())
				AddToDialog( code );
		}
	}

	void Cheats::List::Import(const GenericString path)  
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

		listView->Init( parent.ListView(index + IDC_CHEATS_STATIC_CODES) );

		listView->controls[ ADD	   ] = parent.Control( index + IDC_CHEATS_STATIC_ADD    );
		listView->controls[ REMOVE ] = parent.Control( index + IDC_CHEATS_STATIC_REMOVE );
		listView->controls[ IMPORT ] = parent.Control( index + IDC_CHEATS_STATIC_IMPORT );
		listView->controls[ EXPORT ] = parent.Control( index + IDC_CHEATS_STATIC_EXPORT );
		listView->controls[ CLEAR  ] = parent.Control( index + IDC_CHEATS_STATIC_CLEAR  );

		listView->Columns().Clear();

		for (uint i=0; i < 5; ++i)
			listView->Columns().Insert( i, Resource::String(IDS_CHEAT_CODE+i) );

		if (codes.Size())
		{
			for (Codes::ConstIterator it=codes.Begin(), end=codes.End(); it != end; ++it)
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

		if ((nm.uOldState ^ nm.uNewState) & LVIS_STATEIMAGEMASK)
		{
			const uint address = nm.lParam;

			for (Codes::Iterator it=codes.Begin(), end=codes.End(); it != end; ++it)
			{
				if (it->mem.address == address)
				{
					// As documented on MSDN the image index for the checked box is 2 (unchecked is 1)
					it->enabled = ((nm.uNewState & LVIS_STATEIMAGEMASK) == INDEXTOSTATEIMAGEMASK( 2 ));
					break;
				}
			}
		}			
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
		const uint address = reinterpret_cast<const NMLISTVIEW&>(nmhdr).lParam;

		for (Codes::Iterator it=codes.Begin(), end=codes.End(); it != end; ++it)
		{
			if (it->mem.address == address)
			{
				it->desc.Heap::~Heap();
				codes.Erase( it );
				break;
			}
		}

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

			const Path fileName( paths.BrowseSave( Managers::Paths::File::SCRIPT ) );

			if (fileName.Length())
			{
				Io::Nsp::Context context;

				if (Io::File::FileExist( fileName.Ptr() ) && User::Confirm( IDS_CHEATS_EXPORTEXISTING ))
				{
					try
					{
						Collection::Buffer buffer;
						Io::File( fileName, Io::File::COLLECT ).Stream() >> buffer;
						Io::Nsp::File().Load( buffer, context );
					}
					catch (...)
					{
						User::Warn( IDS_CHEATS_EXPORTEXISTING_ERROR );
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
	: Dialog( IDD_CHEATS_ADDCODE, instance, messages, commands ), listView(NULL) {}

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
		{ WM_INITDIALOG, &Cheats::OnInitCodeDialog    },
		{ WM_DESTROY,    &Cheats::OnDestroyCodeDialog }
	};

	const MsgHandler::Entry<Cheats> Cheats::Handlers::commands[] =
	{
		{ IDC_CHEATS_OK, &Cheats::OnClose }
	};

	const MsgHandler::Entry<Cheats> Cheats::Handlers::codeCommands[] =
	{
		{ IDC_CHEATS_ADDCODE_SUBMIT,             &Cheats::OnCodeCmdSubmit   },
		{ IDC_CHEATS_ADDCODE_CANCEL,             &Cheats::OnCodeCmdCancel   },
		{ IDC_CHEATS_ADDCODE_VALIDATE,           &Cheats::OnCodeCmdValidate },
		{ IDC_CHEATS_ADDCODE_USE_HEX,            &Cheats::OnCodeCmdHex      },
		{ IDC_CHEATS_ADDCODE_USE_RAW,            &Cheats::OnCodeCmdType     },
		{ IDC_CHEATS_ADDCODE_USE_GENIE,          &Cheats::OnCodeCmdType     },
		{ IDC_CHEATS_ADDCODE_USE_ROCKY,          &Cheats::OnCodeCmdType     },
		{ IDC_CHEATS_ADDCODE_SEARCH_NONE,		 &Cheats::OnCodeSearchType  }, 
		{ IDC_CHEATS_ADDCODE_SEARCH_R0_A_R1_B,	 &Cheats::OnCodeSearchType  }, 
		{ IDC_CHEATS_ADDCODE_SEARCH_R0_A_R0R1_B, &Cheats::OnCodeSearchType  },
		{ IDC_CHEATS_ADDCODE_SEARCH_R0R1_B,		 &Cheats::OnCodeSearchType  },
		{ IDC_CHEATS_ADDCODE_SEARCH_R0_L_R1,	 &Cheats::OnCodeSearchType  }, 
		{ IDC_CHEATS_ADDCODE_SEARCH_R0_G_R1,	 &Cheats::OnCodeSearchType  }, 
		{ IDC_CHEATS_ADDCODE_SEARCH_R0_N_R1,	 &Cheats::OnCodeSearchType  },
		{ IDC_CHEATS_ADDCODE_SEARCH_RESET,  	 &Cheats::OnCodeCmdReset    } 
	};

	Cheats::Cheats(Managers::Emulator& e,const Configuration& cfg,const Managers::Paths& paths)
	: 
	mainDialog                  ( IDD_CHEATS, this, Handlers::messages, Handlers::commands ),
	codeDialog                  ( this, Handlers::codeMessages, Handlers::codeCommands ),
	staticList                  ( codeDialog, paths ),
	tempList                    ( codeDialog, paths ),
	emulator                    ( e ),
	searcherNotificationHandler ( IDC_CHEATS_ADDCODE_SEARCH_LIST, codeDialog.Messages() )
	{
		static const Control::NotificationHandler::Entry<Cheats> notifications[] =
		{
			{ LVN_ITEMCHANGED, &Cheats::OnCodeItemChanged }
		};

		searcherNotificationHandler.Add( this, notifications );

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
		codeDialog.Edit( IDC_CHEATS_ADDCODE_ADDRESS ).Limit( 4   );
		codeDialog.Edit( IDC_CHEATS_ADDCODE_DESC    ).Limit( 256 );
		codeDialog.Edit( IDC_CHEATS_ADDCODE_GENIE   ).Limit( 8   );
		codeDialog.Edit( IDC_CHEATS_ADDCODE_ROCKY   ).Limit( 8   );

		codeDialog.RadioButton( IDC_CHEATS_ADDCODE_USE_RAW   ).Check( TRUE  );
		codeDialog.RadioButton( IDC_CHEATS_ADDCODE_USE_GENIE ).Check( FALSE );
		codeDialog.RadioButton( IDC_CHEATS_ADDCODE_USE_ROCKY ).Check( FALSE );
		
		UpdateInput();

		if (emulator.Is(Nes::Machine::GAME,Nes::Machine::ON))
		{
			Control::ListView listView( codeDialog.ListView(IDC_CHEATS_ADDCODE_SEARCH_LIST) );

			listView.StyleEx() = LVS_EX_FULLROWSELECT;

			static const tstring columns[] =
			{
				_T("Index"), _T("R0"), _T("R1")
			};

			listView.Columns().Set( columns );

			if (searcher.filter == Searcher::NO_FILTER)
			{
				searcher.filter = IDC_CHEATS_ADDCODE_SEARCH_NONE;
				std::memcpy( searcher.ram, Nes::Cheats(emulator).GetRam(), Nes::Cheats::RAM_SIZE );

				codeDialog.Control( IDC_CHEATS_ADDCODE_SEARCH_RESET ).Disable();
			}
			else if (std::memcmp( searcher.ram, Nes::Cheats(emulator).GetRam(), Nes::Cheats::RAM_SIZE ) == 0)
			{
				codeDialog.Control( IDC_CHEATS_ADDCODE_SEARCH_RESET ).Disable();
			}

			codeDialog.RadioButton( searcher.filter ).Check();
		}
		else
		{
			for (uint i=IDC_CHEATS_ADDCODE_SEARCH_A; i <= IDC_CHEATS_ADDCODE_SEARCH_RESET; ++i)
				codeDialog.Control( i ).Disable();
		}

		codeDialog.CheckBox( IDC_CHEATS_ADDCODE_USE_HEX ).Check( searcher.hex );		
		UpdateHexView( FALSE );

		return TRUE;
	}

	void Cheats::OnCodeItemChanged(const NMHDR& nmhdr)
	{
		const NMLISTVIEW& nm = reinterpret_cast<const NMLISTVIEW&>(nmhdr);

		if ((nm.uNewState & LVIS_SELECTED) > (nm.uOldState & LVIS_SELECTED))
		{
			codeDialog.Edit( IDC_CHEATS_ADDCODE_ADDRESS ) << HexString( (u16) nm.lParam, true ).Ptr();

			if (codeDialog.RadioButton( IDC_CHEATS_ADDCODE_USE_RAW ).IsUnchecked())
			{
				codeDialog.RadioButton( IDC_CHEATS_ADDCODE_USE_RAW   ).Check( TRUE  );
				codeDialog.RadioButton( IDC_CHEATS_ADDCODE_USE_GENIE ).Check( FALSE );
				codeDialog.RadioButton( IDC_CHEATS_ADDCODE_USE_ROCKY ).Check( FALSE );
				UpdateInput();
			}
		}
	}

	void Cheats::AddSearchEntry(Control::ListView list,const uint address) const
	{
		const int index = list.Add( HexString( (u16) address, true ), address );

		if (searcher.hex)
		{
			list[index].Text(1) << HexString( (u8) searcher.ram[address], true ).Ptr();
			list[index].Text(2) << HexString( (u8) Nes::Cheats(emulator).GetRam()[address], true ).Ptr();
		}
		else
		{
			list[index].Text(1) << String::Num<tchar>( searcher.ram[address] ).Ptr();
			list[index].Text(2) << String::Num<tchar>( Nes::Cheats(emulator).GetRam()[address] ).Ptr();
		}
	}

	void Cheats::UpdateHexView(ibool changed)
	{
		Mem mem;

		if (changed)
		{
			changed = GetRawCode( mem );

			if (emulator.Is( Nes::Machine::GAME, Nes::Machine::ON ))
			{
				searcher.a = (u8) GetSearchValue( IDC_CHEATS_ADDCODE_SEARCH_A );
				searcher.b = (u8) GetSearchValue( IDC_CHEATS_ADDCODE_SEARCH_B );
			}

			searcher.hex = codeDialog.CheckBox( IDC_CHEATS_ADDCODE_USE_HEX ).IsChecked();
		}

		const uint digits = searcher.hex ? 2 : 3;

		codeDialog.Edit( IDC_CHEATS_ADDCODE_VALUE   ).Limit( digits );
		codeDialog.Edit( IDC_CHEATS_ADDCODE_COMPARE ).Limit( digits );
		codeDialog.Edit( IDC_CHEATS_ADDCODE_VALUE   ).SetNumberOnly( digits == 3 );
		codeDialog.Edit( IDC_CHEATS_ADDCODE_COMPARE ).SetNumberOnly( digits == 3 );

		if (changed)
		{
			SetRawCode( mem );
		}
		else
		{
			codeDialog.Edit( IDC_CHEATS_ADDCODE_VALUE ).Clear();
			codeDialog.Edit( IDC_CHEATS_ADDCODE_COMPARE ).Clear();
		}

		if (emulator.Is( Nes::Machine::GAME, Nes::Machine::ON ))
		{
			codeDialog.Edit( IDC_CHEATS_ADDCODE_SEARCH_A ).Limit( digits );
			codeDialog.Edit( IDC_CHEATS_ADDCODE_SEARCH_B ).Limit( digits );
			codeDialog.Edit( IDC_CHEATS_ADDCODE_SEARCH_A ).SetNumberOnly( digits == 3 );
			codeDialog.Edit( IDC_CHEATS_ADDCODE_SEARCH_B ).SetNumberOnly( digits == 3 );

			SetSearchValue( IDC_CHEATS_ADDCODE_SEARCH_A, searcher.a );
			SetSearchValue( IDC_CHEATS_ADDCODE_SEARCH_B, searcher.b );

			UpdateSearchList();
		}
	}

	void Cheats::UpdateInput() const
	{
		const ibool raw = codeDialog.RadioButton( IDC_CHEATS_ADDCODE_USE_RAW ).IsChecked();
		const ibool genie = !raw && codeDialog.RadioButton( IDC_CHEATS_ADDCODE_USE_GENIE ).IsChecked();
		const ibool rocky = !raw && !genie;

		codeDialog.Control( IDC_CHEATS_ADDCODE_VALUE   ).Enable( raw   );
		codeDialog.Control( IDC_CHEATS_ADDCODE_COMPARE ).Enable( raw   );
		codeDialog.Control( IDC_CHEATS_ADDCODE_ADDRESS ).Enable( raw   );
		codeDialog.Control( IDC_CHEATS_ADDCODE_GENIE   ).Enable( genie );
		codeDialog.Control( IDC_CHEATS_ADDCODE_ROCKY   ).Enable( rocky );
	}

	void Cheats::UpdateSearchList() const
	{
		Application::Instance::Waiter wait;

		Control::ListView list( codeDialog.ListView(IDC_CHEATS_ADDCODE_SEARCH_LIST) );
		
		const u8 values[] =
		{
			(u8) GetSearchValue( IDC_CHEATS_ADDCODE_SEARCH_A ),
			(u8) GetSearchValue( IDC_CHEATS_ADDCODE_SEARCH_B )
		};

		list.Clear();
		list.Reserve( Nes::Cheats::RAM_SIZE );

		Nes::Cheats::Ram ram = Nes::Cheats(emulator).GetRam();

		switch (searcher.filter)
		{
			case IDC_CHEATS_ADDCODE_SEARCH_NONE:

				for (uint i=0; i < Nes::Cheats::RAM_SIZE; ++i)
					AddSearchEntry( list, i );

				break;

			case IDC_CHEATS_ADDCODE_SEARCH_R0_N_R1:

				for (uint i=0; i < Nes::Cheats::RAM_SIZE; ++i)
				{
					if (searcher.ram[i] != ram[i])
						AddSearchEntry( list, i );
				}
				break;

			case IDC_CHEATS_ADDCODE_SEARCH_R0_L_R1:	 

				for (uint i=0; i < Nes::Cheats::RAM_SIZE; ++i)
				{
					if (searcher.ram[i] < ram[i])
						AddSearchEntry( list, i );
				}
				break;

			case IDC_CHEATS_ADDCODE_SEARCH_R0_G_R1:	 

				for (uint i=0; i < Nes::Cheats::RAM_SIZE; ++i)
				{
					if (searcher.ram[i] > ram[i])
						AddSearchEntry( list, i );
				}
				break;

			case IDC_CHEATS_ADDCODE_SEARCH_R0_A_R1_B:

				for (uint i=0; i < Nes::Cheats::RAM_SIZE; ++i)
				{
					if (searcher.ram[i] == values[0] && ram[i] == values[1])
						AddSearchEntry( list, i );
				}
				break;

			case IDC_CHEATS_ADDCODE_SEARCH_R0_A_R0R1_B:

				for (uint i=0; i < Nes::Cheats::RAM_SIZE; ++i)
				{
					if (searcher.ram[i] == values[0] && ((searcher.ram[i] - ram[i]) & 0xFF) == values[1])
						AddSearchEntry( list, i );
				}
				break;

			case IDC_CHEATS_ADDCODE_SEARCH_R0R1_B:	 

				for (uint i=0; i < Nes::Cheats::RAM_SIZE; ++i)
				{
					if (((searcher.ram[i] - ram[i]) & 0xFF) == values[1])
						AddSearchEntry( list, i );
				}
				break;
		}

		list.Columns().Align();
	}

	ibool Cheats::GetRawCode(Mem& mem) const
	{
		HeapString string;

		if (!(codeDialog.Edit( IDC_CHEATS_ADDCODE_ADDRESS ) >> string))
			return FALSE;
		
		string.Insert( 0, "0x" );

		uint value;

		if (!(string >> value))
			return FALSE;
		
		mem.address = (u16) value;

		if (searcher.hex)
		{
			if (!(codeDialog.Edit( IDC_CHEATS_ADDCODE_VALUE ) >> string))
				return FALSE;
			
			string.Insert( 0, "0x" );

			if (!(string >> value))
				return FALSE;
			
			mem.value = (u8) value;

			if (codeDialog.Edit( IDC_CHEATS_ADDCODE_COMPARE ) >> string)
			{
				string.Insert( 0, "0x" );

				if (!(string >> value))
					return FALSE;

				mem.compare = (u8) value;
				mem.useCompare = true;
			}
			else
			{
				mem.compare = 0x00;
				mem.useCompare = false;
			}
		}
		else
		{
			if (!(codeDialog.Edit( IDC_CHEATS_ADDCODE_VALUE ) >> value) || value > 0xFF)
				return FALSE;

			mem.value = (u8) value;

			if (codeDialog.Edit( IDC_CHEATS_ADDCODE_COMPARE ) >> value)
			{
				if (value > 0xFF)
					return FALSE;

				mem.compare = (u8) value;
				mem.useCompare = true;
			}
			else
			{
				mem.compare = 0x00;
				mem.useCompare = false;
			}
		}

		return TRUE;
	}
  
	void Cheats::SetRawCode(const Mem& mem) const
	{
		codeDialog.Edit( IDC_CHEATS_ADDCODE_ADDRESS ) << HexString( (u16) mem.address, true ).Ptr();

		if (searcher.hex)
			codeDialog.Edit( IDC_CHEATS_ADDCODE_VALUE ) << HexString( (u8) mem.value, true ).Ptr();
		else
			codeDialog.Edit( IDC_CHEATS_ADDCODE_VALUE ) << (uint) mem.value;

		if (!mem.useCompare)
		{
			codeDialog.Edit( IDC_CHEATS_ADDCODE_COMPARE ).Clear();
		}
		else if (searcher.hex)
		{
			codeDialog.Edit( IDC_CHEATS_ADDCODE_COMPARE ) << HexString( (u8) mem.compare, true ).Ptr();
		}
		else
		{
			codeDialog.Edit( IDC_CHEATS_ADDCODE_COMPARE ) << (uint) mem.compare;
		}
	}

	ibool Cheats::GetGenieCode(Mem& mem) const
	{
		String::Heap<char> string;
		codeDialog.Edit( IDC_CHEATS_ADDCODE_GENIE ) >> string;
		return NES_SUCCEEDED(Nes::Cheats::GameGenieDecode( string.Ptr(), mem ));
	}

	ibool Cheats::GetRockyCode(Mem& mem) const
	{
		String::Heap<char> string;
		codeDialog.Edit( IDC_CHEATS_ADDCODE_ROCKY ) >> string;
		return NES_SUCCEEDED(Nes::Cheats::ProActionRockyDecode( string.Ptr(), mem ));
	}

	uint Cheats::GetSearchValue(const uint id) const
	{
		uint value = 0;

		if (searcher.hex)
		{
			HeapString string;

			if (codeDialog.Edit( id ) >> string)
			{
				string.Insert( 0, "0x" );

				if (!(string >> value))
					value = 0;
			}
		}
		else
		{
			if (!(codeDialog.Edit( id ) >> value) || value > 0xFF)
				value = 0;
		}

		return value;
	}

	void Cheats::SetSearchValue(const uint id,const uint value) const
	{
		if (searcher.hex)
			codeDialog.Edit( id ) << HexString( (u8) value, true ).Ptr();
		else
			codeDialog.Edit( id ) << (uint) value;
	}

	ibool Cheats::OnCodeCmdReset(Param& param)
	{
		if (param.Button().IsClicked())
		{
			codeDialog.Control( IDC_CHEATS_ADDCODE_SEARCH_RESET ).Disable();
			std::memcpy( searcher.ram, Nes::Cheats(emulator).GetRam(), Nes::Cheats::RAM_SIZE );
			UpdateSearchList();
		}

		return TRUE;
	}

	ibool Cheats::OnCodeCmdType(Param& param)
	{
		if (param.Button().IsClicked())
		{
			const uint cmd = param.Button().GetId();

			codeDialog.RadioButton( IDC_CHEATS_ADDCODE_USE_RAW   ).Check( cmd == IDC_CHEATS_ADDCODE_USE_RAW   );
			codeDialog.RadioButton( IDC_CHEATS_ADDCODE_USE_GENIE ).Check( cmd == IDC_CHEATS_ADDCODE_USE_GENIE );
			codeDialog.RadioButton( IDC_CHEATS_ADDCODE_USE_ROCKY ).Check( cmd == IDC_CHEATS_ADDCODE_USE_ROCKY );

			UpdateInput();
		}

		return TRUE;
	}

	ibool Cheats::OnCodeSearchType(Param& param)
	{
		if (param.Button().IsClicked())
		{
			const uint cmd = searcher.filter = (u16) param.Button().GetId();

			for (uint i=IDC_CHEATS_ADDCODE_SEARCH_NONE; i <= IDC_CHEATS_ADDCODE_SEARCH_R0_N_R1; ++i)
				codeDialog.RadioButton( i ).Check( cmd == i );

			UpdateSearchList();
		}

		return TRUE;
	}

	ibool Cheats::OnCodeCmdHex(Param& param)
	{
		if (param.Button().IsClicked())
			UpdateHexView( TRUE );

		return TRUE;
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
			uint id;
			Mem mem;

			if (codeDialog.RadioButton( IDC_CHEATS_ADDCODE_USE_RAW).IsChecked())
			{
				id = GetRawCode( mem ) ? IDC_CHEATS_ADDCODE_USE_RAW : 0;
			}
			else if (codeDialog.RadioButton( IDC_CHEATS_ADDCODE_USE_GENIE ).IsChecked())
			{
				id = GetGenieCode( mem ) ? IDC_CHEATS_ADDCODE_USE_GENIE : 0;
			}
			else
			{
				id = GetRockyCode( mem ) ? IDC_CHEATS_ADDCODE_USE_ROCKY : 0;
			}

			if (id)
			{
				if (id != IDC_CHEATS_ADDCODE_USE_RAW)
				{
					SetRawCode( mem );
				}

				if (id != IDC_CHEATS_ADDCODE_USE_GENIE)
				{
					if (mem.address >= 0x8000U)
					{
						char characters[9];
						Nes::Cheats::GameGenieEncode( mem, characters );
						codeDialog.Edit( IDC_CHEATS_ADDCODE_GENIE ) << characters;
					}
					else
					{
						codeDialog.Edit( IDC_CHEATS_ADDCODE_GENIE ).Clear();
					}
				}

				if (id != IDC_CHEATS_ADDCODE_USE_ROCKY)
				{
					if (mem.address >= 0x8000U && mem.useCompare)
					{
						char characters[9];
						Nes::Cheats::ProActionRockyEncode( mem, characters );
						codeDialog.Edit( IDC_CHEATS_ADDCODE_ROCKY ) << characters;
					}
					else
					{
						codeDialog.Edit( IDC_CHEATS_ADDCODE_ROCKY ).Clear();
					}
				}
			}

			codeDialog.Edit( IDC_CHEATS_ADDCODE_RESULT ) << (id ? "VALID" : "INVALID");
		}

		return TRUE;
	}

	ibool Cheats::OnCodeCmdSubmit(Param& param)
	{
		if (param.Button().IsClicked())
		{
			ibool result;
			Mem mem;

			if (codeDialog.RadioButton( IDC_CHEATS_ADDCODE_USE_RAW ).IsChecked())
			{
				result = GetRawCode( mem );
			}
			else if (codeDialog.RadioButton( IDC_CHEATS_ADDCODE_USE_GENIE ).IsChecked())
			{
				result = GetGenieCode( mem );
			}
			else
			{
				result = GetRockyCode( mem );
			}

			if (result)
			{
				List& list = codeDialog.GetListView() == staticList.GetHandle() ? staticList : tempList;

				list.Add( mem, codeDialog.Edit(IDC_CHEATS_ADDCODE_DESC).Text() );
				codeDialog.Close();
			}
			else
			{
				User::Warn( IDS_CHEATS_INVALID_CODE, IDS_CHEATS );
			}
		}

		return TRUE;
	}

	ibool Cheats::OnDestroyCodeDialog(Param&)
	{
		searcher.a = (u8) GetSearchValue( IDC_CHEATS_ADDCODE_SEARCH_A );
		searcher.b = (u8) GetSearchValue( IDC_CHEATS_ADDCODE_SEARCH_B );

		return TRUE;
	}
}
