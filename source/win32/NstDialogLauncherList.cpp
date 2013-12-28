////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
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

#include "NstWindowUser.hpp"
#include "NstWindowDropFiles.hpp"
#include "NstResourceString.hpp"
#include "NstDialogFind.hpp"
#include "NstManagerPaths.hpp"
#include "NstDialogLauncher.hpp"
#include <Shlwapi.h>

namespace Nestopia
{
	namespace Window
	{
		const tchar Launcher::List::Strings::mappers[256][4] =
		{
			_T("-"),_T("1"),_T("2"),_T("3"),_T("4"),_T("5"),_T("6"),_T("7"),_T("8"),_T("9"),
			_T("10"),_T("11"),_T("12"),_T("13"),_T("14"),_T("15"),_T("16"),_T("17"),_T("18"),_T("19"),
			_T("20"),_T("21"),_T("22"),_T("23"),_T("24"),_T("25"),_T("26"),_T("27"),_T("28"),_T("29"),
			_T("30"),_T("31"),_T("32"),_T("33"),_T("34"),_T("35"),_T("36"),_T("37"),_T("38"),_T("39"),
			_T("40"),_T("41"),_T("42"),_T("43"),_T("44"),_T("45"),_T("46"),_T("47"),_T("48"),_T("49"),
			_T("50"),_T("51"),_T("52"),_T("53"),_T("54"),_T("55"),_T("56"),_T("57"),_T("58"),_T("59"),
			_T("60"),_T("61"),_T("62"),_T("63"),_T("64"),_T("65"),_T("66"),_T("67"),_T("68"),_T("69"),
			_T("70"),_T("71"),_T("72"),_T("73"),_T("74"),_T("75"),_T("76"),_T("77"),_T("78"),_T("79"),
			_T("80"),_T("81"),_T("82"),_T("83"),_T("84"),_T("85"),_T("86"),_T("87"),_T("88"),_T("89"),
			_T("90"),_T("91"),_T("92"),_T("93"),_T("94"),_T("95"),_T("96"),_T("97"),_T("98"),_T("99"),
			_T("100"),_T("101"),_T("102"),_T("103"),_T("104"),_T("105"),_T("106"),_T("107"),_T("108"),_T("109"),
			_T("110"),_T("111"),_T("112"),_T("113"),_T("114"),_T("115"),_T("116"),_T("117"),_T("118"),_T("119"),
			_T("120"),_T("121"),_T("122"),_T("123"),_T("124"),_T("125"),_T("126"),_T("127"),_T("128"),_T("129"),
			_T("130"),_T("131"),_T("132"),_T("133"),_T("134"),_T("135"),_T("136"),_T("137"),_T("138"),_T("139"),
			_T("140"),_T("141"),_T("142"),_T("143"),_T("144"),_T("145"),_T("146"),_T("147"),_T("148"),_T("149"),
			_T("150"),_T("151"),_T("152"),_T("153"),_T("154"),_T("155"),_T("156"),_T("157"),_T("158"),_T("159"),
			_T("160"),_T("161"),_T("162"),_T("163"),_T("164"),_T("165"),_T("166"),_T("167"),_T("168"),_T("169"),
			_T("170"),_T("171"),_T("172"),_T("173"),_T("174"),_T("175"),_T("176"),_T("177"),_T("178"),_T("179"),
			_T("180"),_T("181"),_T("182"),_T("183"),_T("184"),_T("185"),_T("186"),_T("187"),_T("188"),_T("189"),
			_T("190"),_T("191"),_T("192"),_T("193"),_T("194"),_T("195"),_T("196"),_T("197"),_T("198"),_T("199"),
			_T("200"),_T("201"),_T("202"),_T("203"),_T("204"),_T("205"),_T("206"),_T("207"),_T("208"),_T("209"),
			_T("210"),_T("211"),_T("212"),_T("213"),_T("214"),_T("215"),_T("216"),_T("217"),_T("218"),_T("219"),
			_T("220"),_T("221"),_T("222"),_T("223"),_T("224"),_T("225"),_T("226"),_T("227"),_T("228"),_T("229"),
			_T("230"),_T("231"),_T("232"),_T("233"),_T("234"),_T("235"),_T("236"),_T("237"),_T("238"),_T("239"),
			_T("240"),_T("241"),_T("242"),_T("243"),_T("244"),_T("245"),_T("246"),_T("247"),_T("248"),_T("249"),
			_T("250"),_T("251"),_T("252"),_T("253"),_T("254"),_T("255")
		};

		inline tstring Launcher::List::Strings::GetMapper(uint index) const
		{
			NST_ASSERT( index < 256 );
			return mappers[index];
		}

		tstring Launcher::List::Strings::GetSize(u32 value)
		{
			return value ? sizes(value).Ptr() : _T("-");
		}

		inline void Launcher::List::Strings::Flush()
		{
			sizes.Destroy();
		}

		Launcher::List::List
		(
			Dialog& dialog,
			Menu::CmdHandler& cmdHandler,
			const Managers::Paths& p,
			const Configuration& cfg,
			const Nes::Cartridge::Database& database
		)
		:
		imageDatabase    ( database ),
		useImageDatabase ( NULL ),
		typeFilter       ( 0 ),
		style            ( STYLE ),
		finder           ( dialog ),
		paths            ( cfg ),
		files            ( database ),
		columns          ( cfg ),
		pathManager      ( p )
		{
			static const Menu::CmdHandler::Entry<Launcher::List> commands[] =
			{
				{ IDM_LAUNCHER_EDIT_FIND,         &List::OnCmdEditFind         },
				{ IDM_LAUNCHER_EDIT_INSERT,       &List::OnCmdEditInsert       },
				{ IDM_LAUNCHER_EDIT_REMOVE,       &List::OnCmdEditDelete       },
				{ IDM_LAUNCHER_EDIT_CLEAR,        &List::OnCmdEditClear        },
				{ IDM_LAUNCHER_VIEW_ALIGNCOLUMNS, &List::OnCmdViewAlignColumns },
				{ IDM_LAUNCHER_OPTIONS_COLUMNS,   &List::OnCmdOptionsColumns   }
			};

			cmdHandler.Add( this, commands );

			if (cfg["launcher show gridlines"] != Configuration::NO)
				style |= LVS_EX_GRIDLINES;

			if (cfg["launcher use image database"] != Configuration::NO)
				useImageDatabase = &imageDatabase;
		}

		Launcher::List::~List()
		{
		}

		void Launcher::List::operator = (const Control::ListView& listView)
		{
			typeFilter = 0;

			ctrl = listView;
			ctrl.StyleEx() = style;

			ReloadListColumns();

			ctrl.Reserve( files.Count() );
			ctrl.Columns().Align();

			InvalidateRect( ctrl.GetWindow(), NULL, false );
		}

		void Launcher::List::Close()
		{
			finder.Close();
			UpdateColumnOrder();
			columns.Update( order );
			strings.Flush();
		}

		void Launcher::List::Insert(const Param& param)
		{
			DropFiles dropFiles( param );

			if (dropFiles.Inside( ctrl.GetHandle() ))
			{
				ibool anyInserted = false;

				for (uint i=0, n=dropFiles.Size(); i < n; ++i)
					anyInserted |= files.Insert( imageDatabase, dropFiles[i] );

				if (anyInserted && !Optimize())
					Redraw();
			}
		}

		void Launcher::List::Add(tstring const fileName)
		{
			if (files.Insert( imageDatabase, fileName ) && !Optimize())
				Redraw();
		}

		void Launcher::List::Save(Configuration& cfg,ibool saveFiles)
		{
			paths.Save( cfg );
			columns.Save( cfg );

			if (saveFiles)
				files.Save( imageDatabase );

			cfg["launcher show gridlines"].YesNo() = (style & LVS_EX_GRIDLINES);
			cfg["launcher use image database"].YesNo() = (useImageDatabase != NULL);
		}

		void Launcher::List::SetColors(const uint bg,const uint fg,const Updater redraw) const
		{
			ctrl.SetBkColor( bg );
			ctrl.SetTextBkColor( bg );
			ctrl.SetTextColor( fg );

			if (redraw)
				ctrl.Redraw();
		}

		void Launcher::List::Redraw()
		{
			Application::Instance::Waiter wait;
			Generic::LockDraw lock( ctrl.GetHandle() );

			ctrl.Clear();

			if (const uint count = files.Count())
			{
				uint size = 0;

				for (uint i=0; i < count; ++i)
					size += (files[i].GetType() & typeFilter) != 0;

				if (size)
				{
					ctrl.Reserve( size );

					for (uint i=0; i < count; ++i)
					{
						if (files[i].GetType() & typeFilter)
							ctrl.Add( GenericString(), &files[i] );
					}

					Sort();
					ctrl.Columns().Align();
				}
			}
		}

		ibool Launcher::List::Optimize()
		{
			if (files.ShouldDefrag())
			{
				files.Defrag();
				Redraw();
				return true;
			}

			return false;
		}

		ibool Launcher::List::CanRefresh() const
		{
			return
			(
				(paths.GetSettings().folders.size()) &&
				(paths.GetSettings().include.Word() & Paths::Settings::Include::TYPES)
			);
		}

		void Launcher::List::Refresh()
		{
			if (CanRefresh())
			{
				{
					Application::Instance::Waiter wait;
					ctrl.Clear();
				}

				files.Refresh( paths.GetSettings(), imageDatabase );
				ctrl.Reserve( files.Count() );
				Redraw();
			}
		}

		void Launcher::List::OnGetDisplayInfo(LPARAM lParam)
		{
			LVITEM& item = reinterpret_cast<NMLVDISPINFO*>(lParam)->item;

			if (item.mask & LVIF_TEXT)
			{
				const Files::Entry& entry = *reinterpret_cast<const Files::Entry*>( item.lParam );

				switch (columns.GetType( item.iSubItem ))
				{
					case Columns::TYPE_FILE:

						item.pszText = const_cast<tchar*>( entry.GetFile(files.GetStrings()) );
						break;

					case Columns::TYPE_SYSTEM:
					{
						NST_COMPILE_ASSERT
						(
							Files::Entry::SYSTEM_UNKNOWN  == 0 &&
							Files::Entry::SYSTEM_PC10     == 1 &&
							Files::Entry::SYSTEM_VS       == 2 &&
							Files::Entry::SYSTEM_PAL      == 3 &&
							Files::Entry::SYSTEM_NTSC     == 4 &&
							Files::Entry::SYSTEM_NTSC_PAL == 5
						);

						static const tchar lut[][9] =
						{
							_T( "-"        ),
							_T( "pc10"     ),
							_T( "vs"       ),
							_T( "pal"      ),
							_T( "ntsc"     ),
							_T( "ntsc/pal" )
						};

						item.pszText = const_cast<tchar*>( lut[entry.GetSystem( useImageDatabase )] );
						break;
					}

					case Columns::TYPE_BATTERY:

						item.pszText = const_cast<tchar*>
						(
							(entry.GetType() & (Files::Entry::NES|Files::Entry::UNF)) ?
							entry.GetBattery( useImageDatabase ) ? _T("yes") : _T("no") : _T("-")
						);
						break;

					case Columns::TYPE_TRAINER:

						item.pszText = const_cast<tchar*>
						(
							(entry.GetType() & Files::Entry::NES) ?
							entry.GetTrainer( useImageDatabase ) ? _T("yes") : _T("no") : _T("-")
						);
						break;

					case Columns::TYPE_MIRRORING:
					{
						NST_COMPILE_ASSERT
						(
							Files::Entry::MIRROR_NONE       == 0 &&
							Files::Entry::MIRROR_HORIZONTAL == 1 &&
							Files::Entry::MIRROR_VERTICAL   == 2 &&
							Files::Entry::MIRROR_ZERO       == 3 &&
							Files::Entry::MIRROR_ONE        == 4 &&
							Files::Entry::MIRROR_FOURSCREEN == 5 &&
							Files::Entry::MIRROR_CONTROLLED == 6
						);

						static const tchar lut[][11] =
						{
							_T( "-"          ),
							_T( "horizontal" ),
							_T( "vertical"   ),
							_T( "zero"       ),
							_T( "one"        ),
							_T( "fourscreen" ),
							_T( "controlled" )
						};

						item.pszText = const_cast<tchar*>( lut[entry.GetMirroring( useImageDatabase )] );
						break;
					}

					case Columns::TYPE_CONDITION:
					{
						NST_COMPILE_ASSERT
						(
							Files::Entry::CONDITION_UNKNOWN == 0 &&
							Files::Entry::CONDITION_BAD     == 1 &&
							Files::Entry::CONDITION_OK      == 2
						);

						static const tchar lut[][4] =
						{
							_T( "-"   ),
							_T( "bad" ),
							_T( "ok"  )
						};

						item.pszText = const_cast<tchar*>( lut[entry.GetCondition( useImageDatabase )] );
						break;
					}

					case Columns::TYPE_NAME:

						item.pszText = const_cast<tchar*>( entry.GetName( files.GetStrings(), useImageDatabase ) );
						break;

					case Columns::TYPE_MAKER:

						item.pszText = const_cast<tchar*>( entry.GetMaker( files.GetStrings() ) );
						break;

					case Columns::TYPE_FOLDER:

						item.pszText = const_cast<tchar*>( entry.GetPath( files.GetStrings() ) );
						break;

					case Columns::TYPE_PROM:

						item.pszText = const_cast<tchar*>( strings.GetSize(entry.GetPRom(useImageDatabase)) );
						break;

					case Columns::TYPE_CROM:

						if (const uint cRom = entry.GetCRom( useImageDatabase ))
							item.pszText = const_cast<tchar*>( strings.GetSize( cRom ) );
						else
							item.pszText = const_cast<tchar*>( _T("-") );
						break;

					case Columns::TYPE_MAPPER:

						item.pszText = const_cast<tchar*>( strings.GetMapper(entry.GetMapper( useImageDatabase )) );
						break;

					case Columns::TYPE_WRAM:

						if (const uint wRam = entry.GetWRam( useImageDatabase ))
							item.pszText = const_cast<tchar*>( strings.GetSize( wRam ) );
						else
							item.pszText = const_cast<tchar*>( _T("-") );
						break;
				}
			}
		}

		void Launcher::List::ReloadListColumns() const
		{
			ctrl.Columns().Clear();

			for (uint i=0; i < columns.Count(); ++i)
				ctrl.Columns().Insert( i, Resource::String(columns.GetStringId(i)).Ptr() );
		}

		void Launcher::List::UpdateColumnOrder()
		{
			int array[Columns::NUM_TYPES];
			ctrl.Columns().GetOrder( array, columns.Count() );

			for (uint i=0; i < columns.Count(); ++i)
				order[i] = columns.GetType( array[i] );
		}

		void Launcher::List::UpdateSortColumnOrder(const uint firstSortColumn)
		{
			int array[Columns::NUM_TYPES];
			ctrl.Columns().GetOrder( array, columns.Count() );

			order[0] = columns.GetType( firstSortColumn );

			for (uint i=0, j=1; i < columns.Count(); ++i)
			{
				if (uint(array[i]) != firstSortColumn)
					order[j++] = columns.GetType( array[i] );
			}
		}

		void Launcher::List::Sort(const uint firstSortColumn)
		{
			if (ctrl.Size() > 1)
			{
				UpdateSortColumnOrder( firstSortColumn );
				ctrl.Sort( this, &List::Sorter );
			}
		}

		int Launcher::List::Sorter(const void* obj1,const void* obj2)
		{
			const Files::Entry& a = *static_cast<const Files::Entry*>( obj1 );
			const Files::Entry& b = *static_cast<const Files::Entry*>( obj2 );

			for (uint i=0, n=columns.Count(); i < n; ++i)
			{
				switch (order[i])
				{
					case Columns::TYPE_FILE:

						if (const int ret = ::StrCmp( a.GetFile(files.GetStrings()), b.GetFile(files.GetStrings()) ))
							return ret;

						continue;

					case Columns::TYPE_SYSTEM:
					{
						const uint system[] =
						{
							a.GetSystem( useImageDatabase ),
							b.GetSystem( useImageDatabase )
						};

						if (system[0] == system[1])
							continue;

						return system[0] < system[1] ? +1 : -1;
					}

					case Columns::TYPE_MAPPER:
					{
						const uint mapper[] =
						{
							a.GetMapper( useImageDatabase ),
							b.GetMapper( useImageDatabase )
						};

						if (mapper[0] == mapper[1])
							continue;

						return mapper[0] > mapper[1] ? +1 : -1;
					}

					case Columns::TYPE_PROM:
					{
						const uint pRom[] =
						{
							a.GetPRom( useImageDatabase ),
							b.GetPRom( useImageDatabase )
						};

						if (pRom[0] == pRom[1])
							continue;

						return pRom[0] > pRom[1] ? +1 : -1;
					}

					case Columns::TYPE_CROM:
					{
						const uint cRom[] =
						{
							a.GetCRom( useImageDatabase ),
							b.GetCRom( useImageDatabase )
						};

						if (cRom[0] == cRom[1])
							continue;

						return cRom[0] > cRom[1] ? +1 : -1;
					}

					case Columns::TYPE_WRAM:
					{
						const uint wRam[] =
						{
							a.GetWRam( useImageDatabase ),
							b.GetWRam( useImageDatabase )
						};

						if (wRam[0] == wRam[1])
							continue;

						return wRam[0] > wRam[1] ? +1 : -1;
					}

					case Columns::TYPE_BATTERY:
					{
						const uint battery[] =
						{
							a.GetBattery( useImageDatabase ) + (bool) (a.GetType() & (List::Files::Entry::NES|List::Files::Entry::UNF)),
							b.GetBattery( useImageDatabase ) + (bool) (b.GetType() & (List::Files::Entry::NES|List::Files::Entry::UNF))
						};

						if (battery[0] == battery[1])
							continue;

						return battery[0] < battery[1] ? +1 : -1;
					}

					case Columns::TYPE_TRAINER:
					{
						const uint trainer[] =
						{
							a.GetTrainer( useImageDatabase ) + (bool) (a.GetType() & List::Files::Entry::NES),
							b.GetTrainer( useImageDatabase ) + (bool) (b.GetType() & List::Files::Entry::NES)
						};

						if (trainer[0] == trainer[1])
							continue;

						return trainer[0] < trainer[1] ? +1 : -1;
					}

					case Columns::TYPE_MIRRORING:
					{
						const uint mirroring[] =
						{
							a.GetMirroring( useImageDatabase ),
							b.GetMirroring( useImageDatabase )
						};

						if (mirroring[0] == mirroring[1])
							continue;

						return mirroring[0] > mirroring[1] ? +1 : -1;
					}

					case Columns::TYPE_CONDITION:
					{
						const uint condition[] =
						{
							a.GetCondition( useImageDatabase ),
							b.GetCondition( useImageDatabase )
						};

						if (condition[0] == condition[1])
							continue;

						return condition[0] < condition[1] ? +1 : -1;
					}

					case Columns::TYPE_NAME:
					{
						tstring const names[] =
						{
							a.GetName( files.GetStrings(), useImageDatabase ),
							b.GetName( files.GetStrings(), useImageDatabase )
						};

						if (names[0][0] != '-' && names[1][0] == '-') return -1;
						if (names[0][0] == '-' && names[1][0] != '-') return +1;

						if (const int ret = ::StrCmp( names[0], names[1] ))
							return ret;

						continue;
					}

					case Columns::TYPE_MAKER:
					{
						tstring const names[] =
						{
							a.GetMaker( files.GetStrings() ),
							b.GetMaker( files.GetStrings() )
						};

						if (names[0][0] != '-' && names[1][0] == '-') return -1;
						if (names[0][0] == '-' && names[1][0] != '-') return +1;

						if (const int ret = ::StrCmp( names[0], names[1] ))
							return ret;

						continue;
					}

					case Columns::TYPE_FOLDER:

						if (const int ret = ::StrCmp( a.GetPath(files.GetStrings()), b.GetPath(files.GetStrings()) ))
							return ret;

						continue;
				}
			}

			return 0;
		}

		void Launcher::List::OnFind(GenericString string,const uint flags)
		{
			const uint count = ctrl.Size();

			if (count > 1 && string.Length())
			{
				const uint column = ctrl.Columns().GetIndex(0);
				const int selection = ctrl.Selection().GetIndex();
				const uint wrap = selection > 0 ? selection : 0;
				uint index = wrap;
				ibool found;

				HeapString item;

				do
				{
					if (flags & Finder::DOWN)
					{
						if (++index == count)
							index = 0;
					}
					else
					{
						if (uint(--index) == ~0U)
							index = count - 1;
					}

					ctrl[index].Text( column ) >> item;

					if (flags & Finder::WHOLEWORD)
					{
						found = item.Length() == string.Length() && ::StrIsIntlEqual( (flags & Finder::MATCHCASE), item.Ptr(), string.Ptr(), string.Length() );
					}
					else if (flags & Finder::MATCHCASE)
					{
						found = (::StrStr( item.Ptr(), string.Ptr() ) != NULL);
					}
					else
					{
						found = (::StrStrI( item.Ptr(), string.Ptr() ) != NULL);
					}
				}
				while (!found && index != wrap);

				if (found)
				{
					if (selection >= 0)
						ctrl[selection].Select( false );

					ctrl[index].Select();
					ctrl[index].Show();
				}
				else
				{
					User::Inform( IDS_TEXT_SEARCH_NOT_FOUND, IDS_TEXT_FIND );
				}
			}
		}

		void Launcher::List::OnCmdEditFind(uint)
		{
			finder.Open( this, &List::OnFind );
		}

		void Launcher::List::OnCmdEditInsert(uint)
		{
			enum
			{
				FILE_TYPES =
				(
					Managers::Paths::File::IMAGE |
					Managers::Paths::File::SCRIPT |
					Managers::Paths::File::IPS |
					Managers::Paths::File::ARCHIVE
				)
			};

			Add( pathManager.BrowseLoad( FILE_TYPES, GenericString(), Managers::Paths::DONT_CHECK_FILE ).Ptr() );
		}

		void Launcher::List::OnCmdEditDelete(uint)
		{
			Application::Instance::Waiter wait;

			int last = -1;

			for (int index = ctrl.Selection().GetIndex(); index != -1; index = ctrl.Selection().GetIndex())
			{
				last = index;
				void* const entry = ctrl[index].Data();
				ctrl[index].Delete();
				files.Disable( static_cast<Files::Entry*>(entry) );
			}

			if (ctrl.Size())
			{
				if (last != -1)
					ctrl[last].Select();
			}
			else if (typeFilter == Files::Entry::ALL)
			{
				files.Clear();
			}
		}

		void Launcher::List::OnCmdEditClear(uint)
		{
			Application::Instance::Waiter wait;
			ctrl.Clear();
			files.Clear();
		}

		void Launcher::List::OnCmdViewAlignColumns(uint)
		{
			Application::Instance::Waiter wait;
			ctrl.Columns().Align();
		}

		void Launcher::List::OnCmdOptionsColumns(uint)
		{
			UpdateColumnOrder();

			columns.Update( order );
			columns.Open();

			Application::Instance::Waiter wait;
			Generic::LockDraw lock( ctrl.GetHandle() );

			ReloadListColumns();
			Sort();
			ctrl.Columns().Align();
		}
	}
}
