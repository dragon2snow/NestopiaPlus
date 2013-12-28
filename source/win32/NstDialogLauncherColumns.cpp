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

#include "NstApplicationConfiguration.hpp"
#include "NstDialogLauncher.hpp"

namespace Nestopia
{
	using namespace Window;

	tstring const Launcher::List::Columns::strings[NUM_STRING_STYLES][NUM_TYPES] =
	{
		{
			// gui naming

			_T( "File"      ),
			_T( "System"    ),
			_T( "Mapper"    ),
			_T( "pRom"      ),
			_T( "cRom"      ),
			_T( "wRam"      ),
			_T( "Battery"   ),
			_T( "Trainer"   ),
			_T( "Mirroring" ),
			_T( "Name"      ),
			_T( "Maker"     ),
			_T( "Folder"    )
		},
		{
			// cfg naming

			_T( "file"	   ),
			_T( "system"	   ),
			_T( "mapper"	   ),
			_T( "prom"	   ),
			_T( "crom"	   ),
			_T( "wram"	   ),
			_T( "battery"   ),
			_T( "trainer"   ),
			_T( "mirroring" ),
			_T( "name"	   ),
			_T( "maker"	   ),
			_T( "folder"	   )
		}
	};

	cstring Launcher::List::Columns::CfgName(const uint i)
	{
		NST_ASSERT( i < 20 );

		static char name[] = "launcher column xx";

		name[16] = (i < 9 ? '1' + i : '1');
		name[17] = (i < 9 ? '\0' : '0' + i - 9);

		return name;
	}

	struct Launcher::List::Columns::Handlers
	{
		static const MsgHandler::Entry<Columns> messages[];
		static const MsgHandler::Entry<Columns> commands[];
	};

	const MsgHandler::Entry<Launcher::List::Columns> Launcher::List::Columns::Handlers::messages[] =
	{
		{ WM_INITDIALOG, &Columns::OnInitDialog }
	};

	const MsgHandler::Entry<Launcher::List::Columns> Launcher::List::Columns::Handlers::commands[] =
	{
		{ IDC_LAUNCHER_COLUMNSELECT_SELECTED,  &Columns::OnCmdSelected  },
		{ IDC_LAUNCHER_COLUMNSELECT_AVAILABLE, &Columns::OnCmdAvailable },
		{ IDC_LAUNCHER_COLUMNSELECT_ADD,	   &Columns::OnCmdAdd       },
		{ IDC_LAUNCHER_COLUMNSELECT_REMOVE,	   &Columns::OnCmdRemove    },
		{ IDC_LAUNCHER_COLUMNSELECT_DEFAULT,   &Columns::OnCmdDefault   },
		{ IDC_LAUNCHER_COLUMNSELECT_OK,		   &Columns::OnCmdOk        },
		{ IDC_LAUNCHER_COLUMNSELECT_CANCEL,	   &Columns::OnCmdCancel    }
	};

	Launcher::List::Columns::Columns(const Configuration& cfg)
	: 
	available (NUM_TYPES),
	dialog    (IDD_LAUNCHER_COLUMNSELECT,this,Handlers::messages,Handlers::commands)
	{
		for (uint i=0; i < NUM_TYPES; ++i)
			available[i] = Types::Item(i);

		selected.Reserve( NUM_TYPES );

		for (uint i=0; i < NUM_TYPES; ++i)
		{
			const GenericString string( cfg[CfgName(i)] );

			if (string.Empty())
				break;

			for (Types::Iterator it = available.Begin(); it != available.End(); ++it)
			{
				if (string == strings[CFG][*it])
				{
					selected.PushBack( *it );
					available.Erase( it );
					break;
				}
			}
		}

		if (selected.Empty())
			Reset();
	}

	Launcher::List::Columns::~Columns()
	{
	}

	void Launcher::List::Columns::Reset()
	{
		selected.Resize( NUM_DEFAULT_SELECTED_TYPES );
		available.Resize( NUM_DEFAULT_AVAILABLE_TYPES );

		for (uint i=0; i < NUM_DEFAULT_SELECTED_TYPES; ++i)
			selected[i] = Types::Item(i);

		for (uint i=0; i < NUM_DEFAULT_AVAILABLE_TYPES; ++i)
			available[i] = Types::Item(NUM_DEFAULT_SELECTED_TYPES+i);
	}

	void Launcher::List::Columns::Update(const uchar* const order)
	{
		selected.Assign( order, selected.Size() );
	}

	void Launcher::List::Columns::Save(Configuration& cfg) const
	{
		for (uint i=0; i < selected.Size(); ++i)
			cfg[CfgName(i)] = strings[CFG][selected[i]];
	}

	void Launcher::List::Columns::UpdateButtonRemove()
	{
		const Control::ListBox list( dialog.ListBox(IDC_LAUNCHER_COLUMNSELECT_SELECTED) );

		dialog.Control(IDC_LAUNCHER_COLUMNSELECT_REMOVE).Enable
		( 
			list.Size() > 1 && list.Selection().IsValid()
		);
	}

	void Launcher::List::Columns::UpdateButtonAdd()
	{
		dialog.Control(IDC_LAUNCHER_COLUMNSELECT_ADD).Enable
		( 
			dialog.ListBox(IDC_LAUNCHER_COLUMNSELECT_AVAILABLE).Selection().IsValid()
		);
	}

	ibool Launcher::List::Columns::OnInitDialog(Param&)
	{
		Control::ListBox list( dialog.ListBox(IDC_LAUNCHER_COLUMNSELECT_SELECTED) );
		list.Reserve( selected.Size() );

		for (uint i=0; i != selected.Size(); ++i)
			list.Add( strings[GUI][selected[i]] );

		list[0].Select();
		list = dialog.ListBox(IDC_LAUNCHER_COLUMNSELECT_AVAILABLE);
		list.Reserve( available.Size() );

		for (uint i=0; i < available.Size(); ++i)
			list.Add( strings[GUI][available[i]] );

		list[0].Select();
		return TRUE;
	}

	ibool Launcher::List::Columns::OnCmdSelected(Param& param)
	{
		if (param.ListBox().SelectionChanged())
			UpdateButtonRemove();

		return TRUE;
	}

	ibool Launcher::List::Columns::OnCmdAvailable(Param& param)
	{
		if (param.ListBox().SelectionChanged())
			UpdateButtonAdd();

		return TRUE;
	}

	ibool Launcher::List::Columns::OnCmdAdd(Param& param)
	{
		if (param.Button().IsClicked())
			Add( IDC_LAUNCHER_COLUMNSELECT_SELECTED, IDC_LAUNCHER_COLUMNSELECT_AVAILABLE );

		return TRUE;
	}

	ibool Launcher::List::Columns::OnCmdRemove(Param& param)
	{
		if (param.Button().IsClicked())
			Add( IDC_LAUNCHER_COLUMNSELECT_AVAILABLE, IDC_LAUNCHER_COLUMNSELECT_SELECTED );

		return TRUE;
	}

	ibool Launcher::List::Columns::OnCmdDefault(Param& param)
	{
		if (param.Button().IsClicked())
		{
			Control::ListBox list( dialog.ListBox(IDC_LAUNCHER_COLUMNSELECT_SELECTED) );

			list.Clear();
			list.Add( strings[GUI], NUM_DEFAULT_SELECTED_TYPES );
			list[0].Select();

			list = dialog.ListBox(IDC_LAUNCHER_COLUMNSELECT_AVAILABLE);
			list.Clear();
			list.Add( strings[GUI] + NUM_DEFAULT_SELECTED_TYPES, NUM_DEFAULT_AVAILABLE_TYPES );
			list[0].Select();

			dialog.Control( IDC_LAUNCHER_COLUMNSELECT_REMOVE ).Enable();
			dialog.Control( IDC_LAUNCHER_COLUMNSELECT_ADD    ).Enable();
		}

		return TRUE;
	}

	ibool Launcher::List::Columns::OnCmdOk(Param& param)
	{
		if (param.Button().IsClicked())
		{
			HeapString text;

			for (uint i=0; i < 2; ++i)
			{
				Control::ListBox list = dialog.ListBox
				( 
					i ? IDC_LAUNCHER_COLUMNSELECT_SELECTED : 
			         	IDC_LAUNCHER_COLUMNSELECT_AVAILABLE 
				);

				Types& types = (i ? selected : available);
				types.Resize( list.Size() );

				for (uint j=0; j < types.Size(); ++j)
				{
					list[j].Text() >> text;

					for (uint k=0; k < NUM_TYPES; ++k)
					{
						if (text == strings[GUI][k])
						{
							types[j] = (Types::Item) k;
							break;
						}
					}
				}
			}

			dialog.Close();
		}

		return TRUE;
	}

	ibool Launcher::List::Columns::OnCmdCancel(Param& param)
	{
		if (param.Button().IsClicked())
			dialog.Close();

		return TRUE;
	}

	void Launcher::List::Columns::Add(const uint iDst,const uint iSrc)
	{
		const Control::ListBox cSrc( dialog.ListBox(iSrc) );
		const int sSrc = cSrc.Selection().GetIndex();

		if (sSrc >= 0 && (iDst == IDC_LAUNCHER_COLUMNSELECT_SELECTED || cSrc.Size() > 1))
		{
			HeapString text;
			cSrc[sSrc].Text() >> text;

			const Control::ListBox cDst( dialog.ListBox(iDst) );
			const int sDst = cDst.Selection().GetIndex();

			if (sDst >= 0)
				cDst.Insert( sDst + 1, text.Ptr() ).Select();
			else
				cDst.Add( text.Ptr() );

			cSrc[sSrc].Remove();

			if (cSrc.Size() > (uint) sSrc)
				cSrc[sSrc].Select();

			UpdateButtonRemove();
			UpdateButtonAdd();
		}
	}
}
