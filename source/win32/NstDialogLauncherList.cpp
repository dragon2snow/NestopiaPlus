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

#include "NstApplicationInstance.hpp"
#include "NstWindowUser.hpp"
#include "NstDialogFind.hpp"
#include "NstManagerPaths.hpp"
#include "NstDialogLauncher.hpp"

namespace Nestopia
{
	using namespace Window;

	const char Launcher::List::Strings::mappers[256][4] =
	{
		"-","1","2","3","4","5","6","7","8","9",
		"10","11","12","13","14","15","16","17","18","19",
		"20","21","22","23","24","25","26","27","28","29",
		"30","31","32","33","34","35","36","37","38","39",
		"40","41","42","43","44","45","46","47","48","49",
		"50","51","52","53","54","55","56","57","58","59",
		"60","61","62","63","64","65","66","67","68","69",
		"70","71","72","73","74","75","76","77","78","79",
		"80","81","82","83","84","85","86","87","88","89",
		"90","91","92","93","94","95","96","97","98","99",
		"100","101","102","103","104","105","106","107","108","109",
		"110","111","112","113","114","115","116","117","118","119",
		"120","121","122","123","124","125","126","127","128","129",
		"130","131","132","133","134","135","136","137","138","139",
		"140","141","142","143","144","145","146","147","148","149",
		"150","151","152","153","154","155","156","157","158","159",
		"160","161","162","163","164","165","166","167","168","169",
		"170","171","172","173","174","175","176","177","178","179",
		"180","181","182","183","184","185","186","187","188","189",
		"190","191","192","193","194","195","196","197","198","199",
		"200","201","202","203","204","205","206","207","208","209",
		"210","211","212","213","214","215","216","217","218","219",
		"220","221","222","223","224","225","226","227","228","229",
		"230","231","232","233","234","235","236","237","238","239",
		"240","241","242","243","244","245","246","247","248","249",
		"250","251","252","253","254","255"
	};

	inline cstring Launcher::List::Strings::GetMapper(uint index) const
	{
		NST_ASSERT( index < 256 );
		return mappers[index];
	}

	cstring Launcher::List::Strings::GetSize(u32 value)
	{					   
		return value ? (cstring) sizes(value) : "-";
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
	finder           ( dialog ),
	paths            ( cfg ),
	pathManager      ( p ),
	files            ( database ),
	columns          ( cfg ),
	style            ( STYLE )
	{
		static const Menu::CmdHandler::Entry<Launcher::List> commands[] =
		{
			{ IDM_LAUNCHER_EDIT_FIND,		  &List::OnCmdEditFind         },
			{ IDM_LAUNCHER_EDIT_INSERT,		  &List::OnCmdEditInsert       },
			{ IDM_LAUNCHER_EDIT_REMOVE,		  &List::OnCmdEditDelete       },
			{ IDM_LAUNCHER_EDIT_CLEAR,		  &List::OnCmdEditClear        },
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

		InvalidateRect( ctrl.GetWindow(), NULL, FALSE );
	}

	void Launcher::List::Close()
	{
		UpdateColumnOrder();
		columns.Update( order );
		strings.Flush();
	}

	void Launcher::List::Insert(const Param::DropFilesParam dropFiles)
	{
		if (dropFiles.IsInside( ctrl.GetHandle() ))
		{
			ibool anyInserted = FALSE;

			for (uint i=0, n=dropFiles.Size(); i < n; ++i)
				anyInserted |= files.Insert( imageDatabase, dropFiles[i] );

			if (anyInserted && !Optimize())
				Redraw();
		}
	}

	void Launcher::List::Add(cstring const fileName)
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

	void Launcher::List::SetColors(const uint bg,const uint fg,const Updater repaint) const
	{
		ctrl.SetBkColor( bg );
		ctrl.SetTextBkColor( bg );
		ctrl.SetTextColor( fg );

		if (repaint)
			ctrl.Repaint();
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
						ctrl.Add( String::Generic(), &files[i] );
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
			return TRUE;
		}

		return FALSE;
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
			
					item.pszText = const_cast<char*>( entry.GetFile(files.GetStrings()) ); 
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
			
					static const char lut[][9] =
					{
						"-",
						"pc10",
						"vs",
						"pal",
						"ntsc",
						"ntsc/pal"
					};
			
					item.pszText = const_cast<char*>( lut[entry.GetSystem( useImageDatabase )] );
					break;
				}
			
				case Columns::TYPE_BATTERY:	 
			
					item.pszText = const_cast<char*>
					( 
						(entry.GetType() & (Files::Entry::NES|Files::Entry::UNF)) ?
						entry.GetBattery( useImageDatabase ) ? "yes" : "no" : "-"
					); 
					break;
			
				case Columns::TYPE_TRAINER:	 
			
					item.pszText = const_cast<char*>
					( 
						(entry.GetType() & Files::Entry::NES) ?
						entry.GetTrainer( useImageDatabase ) ? "yes" : "no" : "-"
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
			
					static const char lut[][11] =
					{
						"-",
						"horizontal",
						"vertical",
						"zero",
						"one",
						"fourscreen",
						"controlled"
					};
			
					item.pszText = const_cast<char*>( lut[entry.GetMirroring( useImageDatabase )] );
					break;
				}
			
				case Columns::TYPE_NAME:		 
			
					item.pszText = const_cast<char*>( entry.GetName( files.GetStrings(), useImageDatabase ) ); 
					break;
			
				case Columns::TYPE_MAKER:
			
					item.pszText = const_cast<char*>( entry.GetMaker( files.GetStrings(), useImageDatabase ) ); 
					break;
			
				case Columns::TYPE_FOLDER:
			
					item.pszText = const_cast<char*>( entry.GetPath( files.GetStrings() ) ); 
					break;
			
				case Columns::TYPE_PROM:
			
					item.pszText = const_cast<char*>( strings.GetSize(entry.GetPRom(useImageDatabase)) ); 
					break;	
			
				case Columns::TYPE_CROM:		 
			
					if (const uint cRom = entry.GetCRom( useImageDatabase ))
						item.pszText = const_cast<char*>( strings.GetSize( cRom ) ); 
					else
						item.pszText = const_cast<char*>( "-" ); 
					break;	
			
				case Columns::TYPE_MAPPER:	
			
					item.pszText = const_cast<char*>( strings.GetMapper(entry.GetMapper( useImageDatabase )) );
					break;	
			
				case Columns::TYPE_WRAM:	
			
					if (const uint wRam = entry.GetWRam( useImageDatabase ))
						item.pszText = const_cast<char*>( strings.GetSize( wRam ) ); 
					else
						item.pszText = const_cast<char*>( "-" ); 
					break;
			}
		}
	}

	void Launcher::List::ReloadListColumns() const
	{
		ctrl.Columns().Clear();

		for (uint i=0; i < columns.Count(); ++i)
			ctrl.Columns().Insert( i, columns.GetString(i) );
	}

	void Launcher::List::UpdateColumnOrder()
	{
		int array[Columns::NUM_TYPES];
		ctrl.Columns().GetOrder( array, columns.Count() );

		for (uint i=0; i < columns.Count(); ++i)
			order[i] = (uchar) columns.GetType( array[i] );
	}

	void Launcher::List::UpdateSortColumnOrder(const uint firstSortColumn)
	{
		int array[Columns::NUM_TYPES];
		ctrl.Columns().GetOrder( array, columns.Count() );

		order[0] = (uchar) columns.GetType( firstSortColumn );

		for (uint i=0, j=1; i < columns.Count(); ++i)
		{
			if (uint(array[i]) != firstSortColumn)
				order[j++] = (uchar) columns.GetType( array[i] );
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

		for (uint i=0; i < columns.Count(); ++i)
		{
			switch (order[i])
			{
				case Columns::TYPE_FILE:
			
					if (const int ret = String::Compare( a.GetFile(files.GetStrings()), b.GetFile(files.GetStrings()) ))
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
			
				case Columns::TYPE_NAME:
				{
					cstring const names[] =
					{
						a.GetName( files.GetStrings(), useImageDatabase ), 
						b.GetName( files.GetStrings(), useImageDatabase )
					};
			
					if (names[0][0] != '-' && names[1][0] == '-') return -1;
					if (names[0][0] == '-' && names[1][0] != '-') return +1;
			
					if (const int ret = String::Compare( names[0], names[1] ))
						return ret;
			
					continue;
				}
			
				case Columns::TYPE_MAKER:
				{
					cstring const names[] =
					{
						a.GetMaker( files.GetStrings(), useImageDatabase ), 
						b.GetMaker( files.GetStrings(), useImageDatabase )
					};
			
					if (names[0][0] != '-' && names[1][0] == '-') return -1;
					if (names[0][0] == '-' && names[1][0] != '-') return +1;
			
					if (const int ret = String::Compare( names[0], names[1] ))
						return ret;
			
					continue;
				}
			
				case Columns::TYPE_FOLDER:
			
					if (const int ret = String::Compare( a.GetPath(files.GetStrings()), b.GetPath(files.GetStrings()) ))
						return ret;
			
					continue;
			}
		}

		return 0;
	}

	void Launcher::List::OnFind(const String::Generic string,uint flags)
	{
		const uint count = ctrl.Size();

		if (count > 1 && string.Size())
		{
			const uint column = ctrl.Columns().GetIndex(0);
			const int selection = ctrl.Selection().GetIndex();
			const uint wrap = selection > 0 ? selection : 0;
			uint index = wrap;
			ibool found;

			String::Smart<256> item;

			do
			{
				if (flags & Finder::DOWN)
				{
					if (++index == count)
						index = 0;
				}
				else
				{
					if (--index == ~0U)
						index = count - 1;
				}

				ctrl[index].Text( column ) >> item;

				if (flags & Finder::WHOLEWORD)
				{
					if (flags & Finder::MATCHCASE)
						found = (std::strcmp( item, string ) == 0);
					else
						found = (item == string);
				}
				else
				{
					if (flags & Finder::MATCHCASE)
						found = (std::strstr( item, string ) != NULL);
					else
						found = item.Has( string );
				}
			}
			while (!found && index != wrap);

			if (found)
			{
				if (selection >= 0)
					ctrl[selection].Select( FALSE );

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

		Add( pathManager.BrowseLoad( FILE_TYPES ) );
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
