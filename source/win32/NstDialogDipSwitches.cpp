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

#include "NstWindowParam.hpp"
#include "NstDialogDipSwitches.hpp"

namespace Nestopia
{
	NST_COMPILE_ASSERT
	(
		IDC_DIPSWITCHES_2 == IDC_DIPSWITCHES_1 + 1 &&
		IDC_DIPSWITCHES_3 == IDC_DIPSWITCHES_1 + 2 &&
		IDC_DIPSWITCHES_4 == IDC_DIPSWITCHES_1 + 3 &&
		IDC_DIPSWITCHES_5 == IDC_DIPSWITCHES_1 + 4 &&
		IDC_DIPSWITCHES_6 == IDC_DIPSWITCHES_1 + 5 &&
		IDC_DIPSWITCHES_7 == IDC_DIPSWITCHES_1 + 6 &&
		IDC_DIPSWITCHES_8 == IDC_DIPSWITCHES_1 + 7 &&

		IDC_DIPSWITCHES_2_TEXT == IDC_DIPSWITCHES_1_TEXT + 1 &&
		IDC_DIPSWITCHES_3_TEXT == IDC_DIPSWITCHES_1_TEXT + 2 &&
		IDC_DIPSWITCHES_4_TEXT == IDC_DIPSWITCHES_1_TEXT + 3 &&
		IDC_DIPSWITCHES_5_TEXT == IDC_DIPSWITCHES_1_TEXT + 4 &&
		IDC_DIPSWITCHES_6_TEXT == IDC_DIPSWITCHES_1_TEXT + 5 &&
		IDC_DIPSWITCHES_7_TEXT == IDC_DIPSWITCHES_1_TEXT + 6 &&
		IDC_DIPSWITCHES_8_TEXT == IDC_DIPSWITCHES_1_TEXT + 7
	);

	using namespace Window;

	struct DipSwitches::Handlers
	{
		static const MsgHandler::Entry<DipSwitches> messages[];
		static const MsgHandler::Entry<DipSwitches> commands[];
	};

	const MsgHandler::Entry<DipSwitches> DipSwitches::Handlers::messages[] =
	{
		{ WM_INITDIALOG, &DipSwitches::OnInitDialog }
	};

	const MsgHandler::Entry<DipSwitches> DipSwitches::Handlers::commands[] =
	{
		{ IDC_DIPSWITCHES_OK,     &DipSwitches::OnCmdOk     },
		{ IDC_DIPSWITCHES_CANCEL, &DipSwitches::OnCmdCancel }
	};

	DipSwitches::DipSwitches(Managers::Emulator& emulator)
	:
	dialog      (IDD_DIPSWITCHES,this,Handlers::messages,Handlers::commands),
	dipSwitches (Nes::DipSwitches(emulator))
	{
	}

	#ifdef IDC_DIPSWITCHES_9
	#error Must update dipswitch dialog fields!
	#endif

	ibool DipSwitches::OnInitDialog(Param&)
	{
		NST_ASSERT( dipSwitches.NumDips() <= MAX_DIPS );

		Point erase;
		HeapString valueName;

		for (uint i=0; i < MAX_DIPS; ++i)
		{
			const Control::ComboBox valueField( dialog.ComboBox(IDC_DIPSWITCHES_1 + i) );
			const Control::Generic textField( dialog.Control(IDC_DIPSWITCHES_1_TEXT + i) );

			if (i < dipSwitches.NumDips())
			{
				textField.Text() << (HeapString() << dipSwitches.GetDipName(i) << ':').Ptr();

				for (uint j=0, n=dipSwitches.NumValues(i); j < n; ++j)
				{
					valueName = dipSwitches.GetValueName(i,j);
					valueField.Add( valueName.Ptr() );
				}

				valueField[dipSwitches.GetValue(i)].Select();
			}
			else
			{
				if (i == dipSwitches.NumDips())
					erase.y = valueField.GetWindow().GetRectangle().top;

				if (i == MAX_DIPS-1)
					erase.y = valueField.GetWindow().GetRectangle().bottom - erase.y;

				textField.GetWindow().Destroy();
				valueField.GetWindow().Destroy();
			}
		}

		Point p;

		for (uint i=IDC_DIPSWITCHES_1_TEXT, n=IDC_DIPSWITCHES_1_TEXT+dipSwitches.NumDips(); i < n; ++i)
		{
			const Control::Generic textField( dialog.Control(i) );
			p = Point(textField.GetMaxTextSize().x,textField.GetWindow().GetSize().y);

			if (erase.x < p.x)
				erase.x = p.x;

			textField.GetWindow().Resize( p );
		}

		p = dialog.ComboBox(IDC_DIPSWITCHES_1).GetWindow().GetRectangle().Position();
		::ScreenToClient( dialog, &p );
		erase.x = p.x - (erase.x + 12);

		for (uint i=IDC_DIPSWITCHES_1, n=IDC_DIPSWITCHES_1+dipSwitches.NumDips(); i < n; ++i)
		{
			p = dialog.Control(i).GetWindow().GetPosition();
			p.x -= erase.x;
			::ScreenToClient( dialog, &p );
			dialog.Control(i).GetWindow().Move( p );
		}

		p = dialog.Control(IDC_DIPSWITCHES_OK).GetWindow().GetPosition() - erase;
		::ScreenToClient( dialog, &p );
		dialog.Control(IDC_DIPSWITCHES_OK).GetWindow().Move( p );

		p = dialog.Control(IDC_DIPSWITCHES_CANCEL).GetWindow().GetPosition() - erase;
		::ScreenToClient( dialog, &p );
		dialog.Control(IDC_DIPSWITCHES_CANCEL).GetWindow().Move( p );

		dialog.Control(IDC_DIPSWITCHES_GROUP).GetWindow().Resize( dialog.Control(IDC_DIPSWITCHES_GROUP).GetWindow().GetSize() - erase );
		dialog.Resize( dialog.GetSize() - erase );

		return true;
	}

	ibool DipSwitches::OnCmdOk(Param& param)
	{
		if (param.Button().IsClicked())
		{
			for (uint i=0, n=dipSwitches.NumDips(); i < n; ++i)
				dipSwitches.SetValue( i, dialog.ComboBox( IDC_DIPSWITCHES_1 + i ).Selection().GetIndex() );

			dialog.Close();
		}

		return true;
	}

	ibool DipSwitches::OnCmdCancel(Param& param)
	{
		if (param.Button().IsClicked())
			dialog.Close();

		return true;
	}
}
