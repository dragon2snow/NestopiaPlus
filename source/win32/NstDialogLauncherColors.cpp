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

#include "NstObjectPod.hpp"
#include "NstApplicationConfiguration.hpp"
#include "NstDialogLauncher.hpp"
#include <CommDlg.h>

namespace Nestopia
{
	using namespace Window;

	inline Launcher::Colors::Type::Type(int l,int t,int r,int b)
	: rect(l,t,r,b) {}

	struct Launcher::Colors::Handlers
	{
		static const MsgHandler::Entry<Colors> messages[];
		static const MsgHandler::Entry<Colors> commands[];
	};

	const MsgHandler::Entry<Launcher::Colors> Launcher::Colors::Handlers::messages[] =
	{
		{ WM_PAINT, &Colors::OnPaint }
	};

	const MsgHandler::Entry<Launcher::Colors> Launcher::Colors::Handlers::commands[] =
	{
		{ IDC_LAUNCHER_COLORS_BG_CHANGE, &Colors::OnCmdChangeBackground },
		{ IDC_LAUNCHER_COLORS_FG_CHANGE, &Colors::OnCmdChangeForeground },
		{ IDC_LAUNCHER_COLORS_DEFAULT,	 &Colors::OnCmdDefault          },
		{ IDC_LAUNCHER_COLORS_OK,		 &Colors::OnCmdOk               }
	};

	Launcher::Colors::Colors(const Configuration& cfg)
	: 
	background (20,94,74,114),
	foreground (20,33,74,53),
	dialog     (IDD_LAUNCHER_COLORS,this,Handlers::messages,Handlers::commands)
	{
		background.color = cfg["launcher color background"].Default( (uint) DEF_BACKGROUND_COLOR );
		foreground.color = cfg["launcher color foreground"].Default( (uint) DEF_FOREGROUND_COLOR );
	}

	void Launcher::Colors::Save(Configuration& cfg) const
	{
		cfg[ "launcher color foreground" ] = HexString( (u32) foreground.color );
		cfg[ "launcher color background" ] = HexString( (u32) background.color );
	}

	void Launcher::Colors::Paint(const Type& type) const
	{
		if (HDC const hDC = ::GetDC( dialog ))
		{
			HPEN const hPen = ::CreatePen( PS_SOLID, 1, RGB(0x00,0x00,0x00) );
			HPEN const hPenOld = static_cast<HPEN>(::SelectObject( hDC, hPen ));

			HBRUSH const hBrush = ::CreateSolidBrush( type.color );
			HBRUSH const hBrushOld = static_cast<HBRUSH>(::SelectObject( hDC, hBrush ));

			::Rectangle( hDC, type.rect.left, type.rect.top, type.rect.right, type.rect.bottom );

			::SelectObject( hDC, hBrushOld );
			::DeleteObject( hBrush );

			::SelectObject( hDC, hPenOld );
			::DeleteObject( hPen );
			
			::ReleaseDC( dialog, hDC );
		}
	}

	void Launcher::Colors::ChangeColor(COLORREF& color)
	{
		Object::Pod<CHOOSECOLOR> cc;

		cc.lStructSize  = sizeof(cc);
		cc.hwndOwner    = dialog;
		cc.lpCustColors = customColors;
		cc.rgbResult    = color;
		cc.Flags        = CC_FULLOPEN|CC_RGBINIT;

		if (::ChooseColor( &cc )) 
			color = cc.rgbResult;
	}

	ibool Launcher::Colors::OnPaint(Param&)
	{
		Paint( background );
		Paint( foreground );
		return FALSE;
	}

	ibool Launcher::Colors::OnCmdChangeBackground(Param&)
	{
		ChangeColor( background.color );
		Paint( background );
		return TRUE;
	}

	ibool Launcher::Colors::OnCmdChangeForeground(Param&)
	{
		ChangeColor( foreground.color );
		Paint( foreground );
		return TRUE;
	}

	ibool Launcher::Colors::OnCmdDefault(Param&)
	{
		background.color = DEF_BACKGROUND_COLOR;
		foreground.color = DEF_FOREGROUND_COLOR;

		Paint( background );
		Paint( foreground );

		return TRUE;
	}

	ibool Launcher::Colors::OnCmdOk(Param& param)
	{
		if (param.Button().IsClicked())
			dialog.Close();

		return TRUE;
	}
}
