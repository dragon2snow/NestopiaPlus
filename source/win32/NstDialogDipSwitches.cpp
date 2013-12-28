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

#include "NstWindowParam.hpp"
#include "NstDialogDipSwitches.hpp"
#include "NstManagerEmulator.hpp"
#include "../core/api/NstApiDipSwitches.hpp"

namespace Nestopia
{
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

	DipSwitches::DipSwitches(Managers::Emulator& e)
	: dialog(IDD_DIPSWITCHES,this,Handlers::messages,Handlers::commands), emulator(e) {}

	uint DipSwitches::NumDips() const
	{
		return Nes::DipSwitches(emulator).NumDips();
	}

	void DipSwitches::Open()
	{
		if (NumDips())
			dialog.Open();
	}

    #ifdef IDC_DIPSWITCHES_9
    #error Must update dipswitch dialog fields!
    #endif

	ibool DipSwitches::OnInitDialog(Param&)
	{
		NST_COMPILE_ASSERT
		(
			IDC_DIPSWITCHES_2 - IDC_DIPSWITCHES_1 == 1 &&
			IDC_DIPSWITCHES_3 - IDC_DIPSWITCHES_1 == 2 &&
			IDC_DIPSWITCHES_4 - IDC_DIPSWITCHES_1 == 3 &&
			IDC_DIPSWITCHES_5 - IDC_DIPSWITCHES_1 == 4 &&
			IDC_DIPSWITCHES_6 - IDC_DIPSWITCHES_1 == 5 &&
			IDC_DIPSWITCHES_7 - IDC_DIPSWITCHES_1 == 6 &&
			IDC_DIPSWITCHES_8 - IDC_DIPSWITCHES_1 == 7
		);

		NST_COMPILE_ASSERT
		(
			IDC_DIPSWITCHES_2_TEXT - IDC_DIPSWITCHES_1_TEXT == 1 &&
			IDC_DIPSWITCHES_3_TEXT - IDC_DIPSWITCHES_1_TEXT == 2 &&
			IDC_DIPSWITCHES_4_TEXT - IDC_DIPSWITCHES_1_TEXT == 3 &&
			IDC_DIPSWITCHES_5_TEXT - IDC_DIPSWITCHES_1_TEXT == 4 &&
			IDC_DIPSWITCHES_6_TEXT - IDC_DIPSWITCHES_1_TEXT == 5 &&
			IDC_DIPSWITCHES_7_TEXT - IDC_DIPSWITCHES_1_TEXT == 6 &&
			IDC_DIPSWITCHES_8_TEXT - IDC_DIPSWITCHES_1_TEXT == 7
		);

		const Nes::DipSwitches dips(emulator);
		NST_ASSERT( dips.NumDips() <= MAX_DIPS );

		for (uint i=0; i < MAX_DIPS; ++i)
		{
			const Control::ComboBox valueField( dialog.ComboBox(IDC_DIPSWITCHES_1 + i) );
			const Control::Generic textField( dialog.Control(IDC_DIPSWITCHES_1_TEXT + i) );

			if (dips.NumDips() > i)
			{
				textField.Text() << dips.GetDipName(i);

				for (uint j=0, n=dips.NumValues(i); j < n; ++j)
					valueField.Add( dips.GetValueName(i,j) );

				valueField[dips.GetValue(i)].Select();
			}
			else
			{
				textField.GetWindow().Destroy();
				valueField.GetWindow().Destroy();
			}
		}

		return TRUE;
	}

	ibool DipSwitches::OnCmdOk(Param& param)
	{
		if (param.Button().IsClicked())
		{
			Nes::DipSwitches dips(emulator);

			for (uint i=0, n=dips.NumDips(); i < n; ++i)
				dips.SetValue( i, dialog.ComboBox( IDC_DIPSWITCHES_1 + i ).Selection().GetIndex() );

			dialog.Close();
		}

		return TRUE;
	}

	ibool DipSwitches::OnCmdCancel(Param& param)
	{
		if (param.Button().IsClicked())
			dialog.Close();

		return TRUE;
	}
}
