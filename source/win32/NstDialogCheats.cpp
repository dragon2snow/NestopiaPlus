////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2008 Martin Freij
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
#include "NstIoStream.hpp"
#include "NstResourceString.hpp"
#include "NstWindowParam.hpp"
#include "NstWindowDropFiles.hpp"
#include "NstWindowUser.hpp"
#include "NstManagerPaths.hpp"
#include "NstDialogCheats.hpp"
#include "NstApplicationInstance.hpp"
#include "../core/NstXml.hpp"
#include <CommCtrl.h>

namespace Nestopia
{
	namespace Window
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
			IDC_CHEATS_ADDCODE_SEARCH_TEXT_B      - IDC_CHEATS_ADDCODE_SEARCH_TEXT_A ==  1 &&
			IDC_CHEATS_ADDCODE_SEARCH_A           - IDC_CHEATS_ADDCODE_SEARCH_TEXT_A ==  2 &&
			IDC_CHEATS_ADDCODE_SEARCH_B           - IDC_CHEATS_ADDCODE_SEARCH_TEXT_A ==  3 &&
			IDC_CHEATS_ADDCODE_SEARCH_LIST        - IDC_CHEATS_ADDCODE_SEARCH_TEXT_A ==  4 &&
			IDC_CHEATS_ADDCODE_SEARCH_NONE        - IDC_CHEATS_ADDCODE_SEARCH_TEXT_A ==  5 &&
			IDC_CHEATS_ADDCODE_SEARCH_R0_A_R1_B   - IDC_CHEATS_ADDCODE_SEARCH_TEXT_A ==  6 &&
			IDC_CHEATS_ADDCODE_SEARCH_R0_A_R0R1_B - IDC_CHEATS_ADDCODE_SEARCH_TEXT_A ==  7 &&
			IDC_CHEATS_ADDCODE_SEARCH_R0R1_B      - IDC_CHEATS_ADDCODE_SEARCH_TEXT_A ==  8 &&
			IDC_CHEATS_ADDCODE_SEARCH_R0_L_R1     - IDC_CHEATS_ADDCODE_SEARCH_TEXT_A ==  9 &&
			IDC_CHEATS_ADDCODE_SEARCH_R0_G_R1     - IDC_CHEATS_ADDCODE_SEARCH_TEXT_A == 10 &&
			IDC_CHEATS_ADDCODE_SEARCH_R0_N_R1     - IDC_CHEATS_ADDCODE_SEARCH_TEXT_A == 11 &&
			IDC_CHEATS_ADDCODE_SEARCH_RESET       - IDC_CHEATS_ADDCODE_SEARCH_TEXT_A == 12
		);

		NST_COMPILE_ASSERT
		(
			IDS_CHEAT_ADDRESS     == IDS_CHEAT_CODE + 1 &&
			IDS_CHEAT_VALUE       == IDS_CHEAT_CODE + 2 &&
			IDS_CHEAT_COMPARE     == IDS_CHEAT_CODE + 3 &&
			IDS_CHEAT_DESCRIPTION == IDS_CHEAT_CODE + 4
		);

		Cheats::List::Code::Code(const Mem& m)
		: mem(m) {}

		void Cheats::List::Code::CheckDesc()
		{
			if (desc.Length())
			{
				desc.Remove( '\"' );
				desc.Trim();
			}
		}

		Cheats::Searcher::Searcher()
		: filter(NO_FILTER), a(0x00), b(0x00), hex(false) {}

		Cheats::List::Codes::~Codes()
		{
			Clear();
		}

		void Cheats::List::Codes::Clear()
		{
			for (Iterator it(Begin()), end(End()); it != end; ++it)
				it->desc.HeapString::~HeapString();

			Collection::Vector<Code>::Clear();
		}

		void Cheats::List::Codes::Load(const Configuration& cfg)
		{
			Configuration::ConstSection cheats( cfg["cheats"] );

			for (uint i=0; i < MAX_CODES; ++i)
			{
				if (Configuration::ConstSection cheat=cheats["cheat"][i])
				{
					Mem mem;

					uint data = cheat["address"].Int();

					if (data > 0xFFFF)
						continue;

					mem.address = data;

					data = cheat["value"].Int();

					if (data > 0xFF)
						continue;

					mem.value = data;

					if (Configuration::ConstSection compare=cheat["compare"])
					{
						data = compare.Int();

						if (data > 0xFF)
							continue;

						mem.useCompare = true;
						mem.compare = data;
					}
					else
					{
						mem.useCompare = false;
					}

					Code& code = Add( mem );

					code.enabled = !cheat["enabled"].No();
					code.desc = cheat["description"].Str();
					code.CheckDesc();
				}
				else
				{
					break;
				}
			}
		}

		void Cheats::List::Codes::Save(Configuration& cfg) const
		{
			if (const uint size=Size())
			{
				Configuration::Section cheats( cfg["cheats"] );

				for (uint i=0; i < size; ++i)
				{
					Configuration::Section cheat( cheats["cheat"][i] );
					const Code& code = *At(i);

					cheat[ "enabled" ].YesNo() = code.enabled;
					cheat[ "address" ].Str() = HexString( 16, code.mem.address ).Ptr();
					cheat[ "value"   ].Str() = HexString(  8, code.mem.value   ).Ptr();

					if (code.mem.useCompare)
						cheat["compare"].Str() = HexString( 8, code.mem.compare ).Ptr();

					if (code.desc.Length())
						cheat["description"].Str() = code.desc.Ptr();
				}
			}
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
				{ LVN_KEYDOWN,     &List::OnKeyDown     },
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

			const HexString address( 16, code.mem.address );
			const HexString value( 8, code.mem.value );
			const String::Stack<8,wchar_t> compare( code.mem.useCompare ? HexString( 8, code.mem.compare ).Ptr() : L"-" );

			String::Stack<8,wchar_t> characters;

			if (code.mem.address >= 0x8000)
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

			wcstring const table[] =
			{
				address.Ptr(), value.Ptr(), compare.Ptr(), code.desc.Ptr()
			};

			for (uint i=0; i < sizeof(array(table)); ++i)
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

		bool Cheats::List::Import(Codes& codes,const Path& path)
		{
			try
			{
				typedef Nes::Core::Xml Xml;
				Xml xml;

				{
					Io::Stream::In stream( path );
					xml.Read( stream );
				}

				if (!xml.GetRoot().IsType( L"cheats" ))
					return false;

				for (Xml::Node node(xml.GetRoot().GetFirstChild()); node && codes.Size() < MAX_CODES; node=node.GetNextSibling())
				{
					if (!node.IsType( L"cheat" ))
						return false;

					Mem mem;

					if (const Xml::Node address=node.GetChild( L"address" ))
					{
						ulong v;

						if (0xFFFF < (v=std::wcstoul( address.GetValue(), NULL, 0 )))
							return false;

						mem.address = v;

						if (const Xml::Node value=node.GetChild( L"value" ))
						{
							if (0xFF < (v=std::wcstoul( value.GetValue(), NULL, 0 )))
								return false;

							mem.value = v;
						}

						if (const Xml::Node compare=node.GetChild( L"compare" ))
						{
							if (0xFF < (v=std::wcstoul( compare.GetValue(), NULL, 0 )))
								return false;

							mem.compare = v;
							mem.useCompare = true;
						}
					}
					else if (const Xml::Node genie=node.GetChild( L"genie" ))
					{
						if (NES_FAILED(Nes::Cheats::GameGenieDecode( String::Heap<char>(genie.GetValue()).Ptr(), mem )))
							return false;
					}
					else if (const Xml::Node rocky=node.GetChild( L"rocky" ))
					{
						if (NES_FAILED(Nes::Cheats::ProActionRockyDecode( String::Heap<char>(rocky.GetValue()).Ptr(), mem )))
							return false;
					}

					if (!codes.Find( mem.address ))
					{
						codes.PushBack( Code(mem) );
						Code& code = codes.Back();

						code.desc = node.GetChild( L"description" ).GetValue();
						code.enabled = !(node.GetAttribute( L"enabled" ).IsValue( L"0" ));
					}
				}
			}
			catch (...)
			{
				return false;
			}

			return true;
		}

		void Cheats::List::Import(const Path& path)
		{
			if (path.Length())
			{
				Codes imported;

				if (Import( imported, path ))
				{
					for (Codes::ConstIterator it(imported.Begin()), end(imported.End()); it != end && codes.Size() < MAX_CODES; ++it)
					{
						Code& code = codes.Add( it->mem );

						code.enabled = it->enabled;
						code.desc = it->desc;
						code.CheckDesc();

						if (listView && listView->GetHandle())
							AddToDialog( code );
					}
				}
				else
				{
					User::Fail( IDS_FILE_ERR_INVALID );
				}
			}
		}

		void Cheats::List::Export(const Path& path) const
		{
			NST_VERIFY( codes.Size() );

			if (path.Length() && codes.Size())
			{
				Codes imported;

				for (uint i=0, n=codes.Size(); i < n; ++i)
				{
					Code& code = imported.Add( codes[i].mem );
					code.enabled = codes[i].enabled;
					code.desc = codes[i].desc;
				}

				if (path.FileExists() && User::Confirm( IDS_CHEATS_EXPORTEXISTING ))
				{
					if (!Import( imported, path ))
					{
						imported.Destroy();
						User::Warn( IDS_CHEATS_EXPORTEXISTING_ERROR );
					}
				}

				try
				{
					typedef Nes::Core::Xml Xml;

					Xml xml;
					Xml::Node root( xml.GetRoot() );

					root = xml.Create( L"cheats" );
					root.AddAttribute( L"version", L"1.0" );

					for (Codes::ConstIterator it(imported.Begin()), end(imported.End()); it != end; ++it)
					{
						Xml::Node node( root.AddChild( L"cheat" ) );
						node.AddAttribute( L"enabled", it->enabled ? L"1" : L"0" );

						char buffer[9];

						if (NES_SUCCEEDED(Nes::Cheats::GameGenieEncode( it->mem, buffer )))
							node.AddChild( L"genie", HeapString(buffer).Ptr() );

						if (NES_SUCCEEDED(Nes::Cheats::ProActionRockyEncode( it->mem, buffer )))
							node.AddChild( L"rocky", HeapString(buffer).Ptr() );

						node.AddChild( L"address", HexString( 16, it->mem.address ).Ptr() );
						node.AddChild( L"value",   HexString( 8,  it->mem.value   ).Ptr() );

						if (it->mem.useCompare)
							node.AddChild( L"compare", HexString( 8, it->mem.compare ).Ptr() );

						if (it->desc.Length())
							node.AddChild( L"description", it->desc.Ptr() );
					}

					Io::Stream::Out stream( path );
					xml.Write( root, stream );
				}
				catch (...)
				{
					User::Fail( IDS_FILE_ERR_INVALID );
				}
			}
		}

		void Cheats::List::InitDialog(Dialog& parent,const uint index)
		{
			NST_ASSERT( index <= 1 );

			if (listView == NULL)
				listView = new ListView( this, parent, index );

			listView->Init( parent.ListView(index + IDC_CHEATS_STATIC_CODES) );

			listView->controls[ ADD    ] = parent.Control( index + IDC_CHEATS_STATIC_ADD    );
			listView->controls[ REMOVE ] = parent.Control( index + IDC_CHEATS_STATIC_REMOVE );
			listView->controls[ IMPORT ] = parent.Control( index + IDC_CHEATS_STATIC_IMPORT );
			listView->controls[ EXPORT ] = parent.Control( index + IDC_CHEATS_STATIC_EXPORT );
			listView->controls[ CLEAR  ] = parent.Control( index + IDC_CHEATS_STATIC_CLEAR  );

			listView->Columns().Clear();

			for (uint i=0; i < 5; ++i)
				listView->Columns().Insert( i, Resource::String(IDS_CHEAT_CODE+i) );

			if (codes.Size())
			{
				for (Codes::ConstIterator it(codes.Begin()), end(codes.End()); it != end; ++it)
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

				for (Codes::Iterator it(codes.Begin()), end(codes.End()); it != end; ++it)
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

			for (Codes::Iterator it(codes.Begin()), end(codes.End()); it != end; ++it)
			{
				if (it->mem.address == address)
				{
					it->desc.HeapString::~HeapString();
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
			if (param.Button().Clicked() && codes.Size() < MAX_CODES)
				codeDialog.Open( listView->GetHandle() );

			return true;
		}

		ibool Cheats::List::OnCmdRemove(Param& param)
		{
			if (param.Button().Clicked())
			{
				NST_VERIFY( codes.Size() );
				listView->Selection().Delete();
			}

			return true;
		}

		ibool Cheats::List::OnCmdExport(Param& param)
		{
			if (param.Button().Clicked())
			{
				Path path( paths.BrowseSave(Managers::Paths::File::XML) );
				paths.FixFile( Managers::Paths::File::XML, path );
				Export( path );
			}

			return true;
		}

		ibool Cheats::List::OnCmdImport(Param& param)
		{
			if (param.Button().Clicked())
				Import( paths.BrowseLoad(Managers::Paths::File::XML) );

			return true;
		}

		ibool Cheats::List::OnCmdClear(Param& param)
		{
			if (param.Button().Clicked())
			{
				NST_VERIFY( codes.Size() );
				listView->Clear();
			}

			return true;
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

		const MsgHandler::Entry<Cheats> Cheats::Handlers::codeCommands[] =
		{
			{ IDC_CHEATS_ADDCODE_SUBMIT,             &Cheats::OnCodeCmdSubmit   },
			{ IDC_CHEATS_ADDCODE_VALIDATE,           &Cheats::OnCodeCmdValidate },
			{ IDC_CHEATS_ADDCODE_USE_HEX,            &Cheats::OnCodeCmdHex      },
			{ IDC_CHEATS_ADDCODE_USE_RAW,            &Cheats::OnCodeCmdType     },
			{ IDC_CHEATS_ADDCODE_USE_GENIE,          &Cheats::OnCodeCmdType     },
			{ IDC_CHEATS_ADDCODE_USE_ROCKY,          &Cheats::OnCodeCmdType     },
			{ IDC_CHEATS_ADDCODE_SEARCH_NONE,        &Cheats::OnCodeSearchType  },
			{ IDC_CHEATS_ADDCODE_SEARCH_R0_A_R1_B,   &Cheats::OnCodeSearchType  },
			{ IDC_CHEATS_ADDCODE_SEARCH_R0_A_R0R1_B, &Cheats::OnCodeSearchType  },
			{ IDC_CHEATS_ADDCODE_SEARCH_R0R1_B,      &Cheats::OnCodeSearchType  },
			{ IDC_CHEATS_ADDCODE_SEARCH_R0_L_R1,     &Cheats::OnCodeSearchType  },
			{ IDC_CHEATS_ADDCODE_SEARCH_R0_G_R1,     &Cheats::OnCodeSearchType  },
			{ IDC_CHEATS_ADDCODE_SEARCH_R0_N_R1,     &Cheats::OnCodeSearchType  },
			{ IDC_CHEATS_ADDCODE_SEARCH_RESET,       &Cheats::OnCodeCmdReset    }
		};

		Cheats::Cheats(Managers::Emulator& e,const Configuration& cfg,const Managers::Paths& paths)
		:
		mainDialog                  ( IDD_CHEATS, this, Handlers::messages ),
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
			return true;
		}

		ibool Cheats::OnDropFiles(Param& param)
		{
			DropFiles dropFiles( param );

			if (dropFiles.Size())
			{
				if (dropFiles.Inside( staticList.GetHandle() ))
				{
					staticList.Import( dropFiles[0] );
				}
				else if (dropFiles.Inside( tempList.GetHandle() ))
				{
					tempList.Import( dropFiles[0] );
				}
			}

			return true;
		}

		ibool Cheats::OnInitCodeDialog(Param&)
		{
			codeDialog.Edit( IDC_CHEATS_ADDCODE_ADDRESS ).Limit( 4   );
			codeDialog.Edit( IDC_CHEATS_ADDCODE_DESC    ).Limit( 256 );
			codeDialog.Edit( IDC_CHEATS_ADDCODE_GENIE   ).Limit( 8   );
			codeDialog.Edit( IDC_CHEATS_ADDCODE_ROCKY   ).Limit( 8   );

			codeDialog.RadioButton( IDC_CHEATS_ADDCODE_USE_RAW   ).Check( true  );
			codeDialog.RadioButton( IDC_CHEATS_ADDCODE_USE_GENIE ).Check( false );
			codeDialog.RadioButton( IDC_CHEATS_ADDCODE_USE_ROCKY ).Check( false );

			UpdateInput();

			if (emulator.IsGameOn())
			{
				Control::ListView listView( codeDialog.ListView(IDC_CHEATS_ADDCODE_SEARCH_LIST) );

				listView.StyleEx() = LVS_EX_FULLROWSELECT;

				static wcstring const columns[] =
				{
					L"Index", L"R0", L"R1"
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
				for (uint i=IDC_CHEATS_ADDCODE_SEARCH_TEXT_A; i <= IDC_CHEATS_ADDCODE_SEARCH_RESET; ++i)
					codeDialog.Control( i ).Disable();
			}

			codeDialog.CheckBox( IDC_CHEATS_ADDCODE_USE_HEX ).Check( searcher.hex );
			UpdateHexView( false );

			return true;
		}

		void Cheats::OnCodeItemChanged(const NMHDR& nmhdr)
		{
			const NMLISTVIEW& nm = reinterpret_cast<const NMLISTVIEW&>(nmhdr);

			if ((nm.uNewState & LVIS_SELECTED) > (nm.uOldState & LVIS_SELECTED))
			{
				codeDialog.Edit( IDC_CHEATS_ADDCODE_ADDRESS ) << HexString( 16, nm.lParam, true ).Ptr();

				if (codeDialog.RadioButton( IDC_CHEATS_ADDCODE_USE_RAW ).Unchecked())
				{
					codeDialog.RadioButton( IDC_CHEATS_ADDCODE_USE_RAW   ).Check( true  );
					codeDialog.RadioButton( IDC_CHEATS_ADDCODE_USE_GENIE ).Check( false );
					codeDialog.RadioButton( IDC_CHEATS_ADDCODE_USE_ROCKY ).Check( false );
					UpdateInput();
				}
			}
		}

		void Cheats::AddSearchEntry(Control::ListView list,const uint address) const
		{
			const int index = list.Add( HexString( 16, address, true ), address );

			if (searcher.hex)
			{
				list[index].Text(1) << HexString( 8, searcher.ram[address], true ).Ptr();
				list[index].Text(2) << HexString( 8, Nes::Cheats(emulator).GetRam()[address], true ).Ptr();
			}
			else
			{
				list[index].Text(1) << String::Num<wchar_t>( uint(searcher.ram[address]) ).Ptr();
				list[index].Text(2) << String::Num<wchar_t>( uint(Nes::Cheats(emulator).GetRam()[address]) ).Ptr();
			}
		}

		void Cheats::UpdateHexView(bool changed)
		{
			Mem mem;

			if (changed)
			{
				changed = GetRawCode( mem );

				if (emulator.IsGameOn())
				{
					searcher.a = GetSearchValue( IDC_CHEATS_ADDCODE_SEARCH_A );
					searcher.b = GetSearchValue( IDC_CHEATS_ADDCODE_SEARCH_B );
				}

				searcher.hex = codeDialog.CheckBox( IDC_CHEATS_ADDCODE_USE_HEX ).Checked();
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

			if (emulator.IsGameOn())
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
			const bool raw = codeDialog.RadioButton( IDC_CHEATS_ADDCODE_USE_RAW ).Checked();
			const bool genie = !raw && codeDialog.RadioButton( IDC_CHEATS_ADDCODE_USE_GENIE ).Checked();
			const bool rocky = !raw && !genie;

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

			const uchar values[] =
			{
				GetSearchValue( IDC_CHEATS_ADDCODE_SEARCH_A ),
				GetSearchValue( IDC_CHEATS_ADDCODE_SEARCH_B )
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
						if (searcher.ram[i] == values[0] && ((searcher.ram[i] - ram[i]) & 0xFFU) == values[1])
							AddSearchEntry( list, i );
					}
					break;

				case IDC_CHEATS_ADDCODE_SEARCH_R0R1_B:

					for (uint i=0; i < Nes::Cheats::RAM_SIZE; ++i)
					{
						if (((searcher.ram[i] - ram[i]) & 0xFFU) == values[1])
							AddSearchEntry( list, i );
					}
					break;
			}

			list.Columns().Align();
		}

		bool Cheats::GetRawCode(Mem& mem) const
		{
			HeapString string;

			if (!(codeDialog.Edit( IDC_CHEATS_ADDCODE_ADDRESS ) >> string))
				return false;

			string.Insert( 0, "0x" );

			uint value;

			if (!(string >> value))
				return false;

			mem.address = value;

			if (searcher.hex)
			{
				if (!(codeDialog.Edit( IDC_CHEATS_ADDCODE_VALUE ) >> string))
					return false;

				string.Insert( 0, "0x" );

				if (!(string >> value))
					return false;

				mem.value = value;

				if (codeDialog.Edit( IDC_CHEATS_ADDCODE_COMPARE ) >> string)
				{
					string.Insert( 0, "0x" );

					if (!(string >> value))
						return false;

					mem.compare = value;
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
					return false;

				mem.value = value;

				if (codeDialog.Edit( IDC_CHEATS_ADDCODE_COMPARE ) >> value)
				{
					if (value > 0xFF)
						return false;

					mem.compare = value;
					mem.useCompare = true;
				}
				else
				{
					mem.compare = 0x00;
					mem.useCompare = false;
				}
			}

			return true;
		}

		void Cheats::SetRawCode(const Mem& mem) const
		{
			codeDialog.Edit( IDC_CHEATS_ADDCODE_ADDRESS ) << HexString( 16, mem.address, true ).Ptr();

			if (searcher.hex)
				codeDialog.Edit( IDC_CHEATS_ADDCODE_VALUE ) << HexString( 8, mem.value, true ).Ptr();
			else
				codeDialog.Edit( IDC_CHEATS_ADDCODE_VALUE ) << uint(mem.value);

			if (!mem.useCompare)
			{
				codeDialog.Edit( IDC_CHEATS_ADDCODE_COMPARE ).Clear();
			}
			else if (searcher.hex)
			{
				codeDialog.Edit( IDC_CHEATS_ADDCODE_COMPARE ) << HexString( 8, mem.compare, true ).Ptr();
			}
			else
			{
				codeDialog.Edit( IDC_CHEATS_ADDCODE_COMPARE ) << uint(mem.compare);
			}
		}

		bool Cheats::GetGenieCode(Mem& mem) const
		{
			String::Heap<char> string;
			codeDialog.Edit( IDC_CHEATS_ADDCODE_GENIE ) >> string;
			return NES_SUCCEEDED(Nes::Cheats::GameGenieDecode( string.Ptr(), mem ));
		}

		bool Cheats::GetRockyCode(Mem& mem) const
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
				codeDialog.Edit( id ) << HexString( 8, value, true ).Ptr();
			else
				codeDialog.Edit( id ) << uint(value);
		}

		ibool Cheats::OnCodeCmdReset(Param& param)
		{
			if (param.Button().Clicked())
			{
				codeDialog.Control( IDC_CHEATS_ADDCODE_SEARCH_RESET ).Disable();
				std::memcpy( searcher.ram, Nes::Cheats(emulator).GetRam(), Nes::Cheats::RAM_SIZE );
				UpdateSearchList();
			}

			return true;
		}

		ibool Cheats::OnCodeCmdType(Param& param)
		{
			if (param.Button().Clicked())
			{
				const uint cmd = param.Button().GetId();

				codeDialog.RadioButton( IDC_CHEATS_ADDCODE_USE_RAW   ).Check( cmd == IDC_CHEATS_ADDCODE_USE_RAW   );
				codeDialog.RadioButton( IDC_CHEATS_ADDCODE_USE_GENIE ).Check( cmd == IDC_CHEATS_ADDCODE_USE_GENIE );
				codeDialog.RadioButton( IDC_CHEATS_ADDCODE_USE_ROCKY ).Check( cmd == IDC_CHEATS_ADDCODE_USE_ROCKY );

				UpdateInput();
			}

			return true;
		}

		ibool Cheats::OnCodeSearchType(Param& param)
		{
			if (param.Button().Clicked())
			{
				searcher.filter = param.Button().GetId();
				UpdateSearchList();
			}

			return true;
		}

		ibool Cheats::OnCodeCmdHex(Param& param)
		{
			if (param.Button().Clicked())
				UpdateHexView( true );

			return true;
		}

		ibool Cheats::OnCodeCmdValidate(Param& param)
		{
			if (param.Button().Clicked())
			{
				uint id;
				Mem mem;

				if (codeDialog.RadioButton( IDC_CHEATS_ADDCODE_USE_RAW).Checked())
				{
					id = GetRawCode( mem ) ? IDC_CHEATS_ADDCODE_USE_RAW : 0;
				}
				else if (codeDialog.RadioButton( IDC_CHEATS_ADDCODE_USE_GENIE ).Checked())
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
						if (mem.address >= 0x8000)
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
						if (mem.address >= 0x8000 && mem.useCompare)
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

				codeDialog.Edit( IDC_CHEATS_ADDCODE_RESULT ) << Resource::String(id ? IDS_CHEATS_RESULT_VALID : IDS_CHEATS_RESULT_INVALID);
			}

			return true;
		}

		ibool Cheats::OnCodeCmdSubmit(Param& param)
		{
			if (param.Button().Clicked())
			{
				bool result;
				Mem mem;

				if (codeDialog.RadioButton( IDC_CHEATS_ADDCODE_USE_RAW ).Checked())
				{
					result = GetRawCode( mem );
				}
				else if (codeDialog.RadioButton( IDC_CHEATS_ADDCODE_USE_GENIE ).Checked())
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

			return true;
		}

		ibool Cheats::OnDestroyCodeDialog(Param&)
		{
			searcher.a = GetSearchValue( IDC_CHEATS_ADDCODE_SEARCH_A );
			searcher.b = GetSearchValue( IDC_CHEATS_ADDCODE_SEARCH_B );

			return true;
		}
	}
}
