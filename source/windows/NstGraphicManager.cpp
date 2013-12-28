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

#include "NstGraphicManager.h"
#include "NstApplication.h"
#include "../paradox/PdxFile.h"
#include <WindowsX.h>
#include <CommCtrl.h>
#include <GdiPlus.h>

#define NST_DEFAULT_SCREENSHOT_FILE_FORMAT ".png"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

GRAPHICMANAGER::GRAPHICMANAGER(const INT id,const UINT chunk)
: MANAGER( id, chunk ), ShouldBeFullscreen(FALSE), PreviousMode(-1), GdiPlusAvailable(FALSE)
{
	SetRect( &SaveRect, 0, 0, 0, 0 );

	format.device = PDX_CAST(VOID*,PDX_STATIC_CAST(DIRECTDRAW*,this));

	HMODULE hDLL = LoadLibrary("GdiPlus.dll");

	if (hDLL)
	{
		GdiPlusAvailable = TRUE;
		FreeLibrary( hDLL );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT GRAPHICMANAGER::Create(PDXFILE* const file)
{
	PDX_TRY(DIRECTDRAW::Initialize( MANAGER::hWnd ));

	if (adapters.IsEmpty() || adapters.Front().DisplayModes.IsEmpty())
		return application.OnError("Found no valid graphic adapter!?");
	
	if (file && file->Read<U8>() == 0xBA)
	{
		HEADER header;
		file->Read(header);

		BOOL found = FALSE;

		for (UINT i=0; i < adapters.Size(); ++i)
		{
			if (!memcmp(&header.guid,&adapters[i].guid,sizeof(header.guid)))
			{
				SelectedAdapter = i;
				found = PDX_SUCCEEDED(DIRECTDRAW::Create(&header.guid));
				break;
			}
		}

		if (!found)
		{
			SelectedAdapter = 0;

			if (PDX_FAILED(DIRECTDRAW::Create(&adapters[0].guid)))
				return application.OnError("DirectDrawCreateEx() failed!");
		}

		UINT bpp;

		switch (header.bpp)
		{
     		case HEADER::BPP_16: bpp = 16; SelectedBpp = IDC_GRAPHICS_16_BIT; break;
			case HEADER::BPP_32: bpp = 32; SelectedBpp = IDC_GRAPHICS_32_BIT; break;
		}

		EnableBpp();

		switch (header.offscreen)
		{
    		case HEADER::OFFSCREEN_SRAM: SelectedOffScreen = IDC_GRAPHICS_SRAM; break;
			case HEADER::OFFSCREEN_VRAM: SelectedOffScreen = IDC_GRAPHICS_VRAM; break;
		}

		switch (header.timing)
		{
    		case HEADER::TIMING_VSYNC:     SelectedTiming = IDC_GRAPHICS_TIMING_VSYNC;     break;
			case HEADER::TIMING_FRAMESKIP: SelectedTiming = IDC_GRAPHICS_TIMING_FRAMESKIP; break;
		}

		found = FALSE;
		ModeIndices.Clear();

		const ADAPTER& adapter = adapters[SelectedAdapter];

		for (UINT i=0; i < adapter.DisplayModes.Size(); ++i)
		{
			const DISPLAYMODE& mode = adapter.DisplayModes[i];

			if (mode.bpp == bpp)
			{
				if (mode.width == header.width && mode.height == header.height)
				{
					found = TRUE;
					SelectedMode = ModeIndices.Size();
				}

				ModeIndices.InsertBack(i);
			}
		}

		if (ModeIndices.IsEmpty())
			ModeIndices.InsertBack(0);

		if (!found)
		{
			SelectedMode = 0;
			ResetBpp();
		}

		switch (header.effect)
		{
     		case HEADER::EFFECT_NONE:        SelectedEffect = 0; break;
     		case HEADER::EFFECT_SCANLINES:   SelectedEffect = 1; break;
			case HEADER::EFFECT_2XSAI:       SelectedEffect = 2; break;
			case HEADER::EFFECT_SUPER_2XSAI: SelectedEffect = 3; break;
			case HEADER::EFFECT_SUPER_EAGLE: SelectedEffect = 4; break;
		}

		switch (header.palette)
		{
     		case HEADER::PALETTE_EMULATED: SelectedPalette = IDC_GRAPHICS_PALETTE_EMULATED; break;
     		case HEADER::PALETTE_INTERNAL: SelectedPalette = IDC_GRAPHICS_PALETTE_INTERNAL; break;
			case HEADER::PALETTE_CUSTOM:   SelectedPalette = IDC_GRAPHICS_PALETTE_CUSTOM;   break;
		}

		context.InfiniteSprites = header.InfiniteSprites;

		ntsc = header.ntsc;
		pal  = header.pal;

		context.brightness = header.brightness;
		context.saturation = header.saturation;
		context.hue        = header.hue;

		file->Text().Read(PaletteFile);

		UpdateDirectDraw();
	}
	else
	{
		PDX_TRY(DIRECTDRAW::Create(&adapters[0].guid));
		Reset();
	}

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT GRAPHICMANAGER::Destroy(PDXFILE* const file)
{
	if (file)
	{
		const U8 good = adapters.Size() > SelectedAdapter ? 0xBA : 0x00;

		file->Write(good);

		if (good)
		{
			HEADER header;

			switch (SelectedBpp)
			{
     			case IDC_GRAPHICS_16_BIT: header.bpp = HEADER::BPP_16; break;
				case IDC_GRAPHICS_32_BIT: header.bpp = HEADER::BPP_32; break;
			}

			switch (SelectedOffScreen)
			{
     			case IDC_GRAPHICS_SRAM: header.offscreen = HEADER::OFFSCREEN_SRAM; break;
				case IDC_GRAPHICS_VRAM: header.offscreen = HEADER::OFFSCREEN_VRAM; break;
			}

			switch (SelectedTiming)
			{
     			case IDC_GRAPHICS_TIMING_VSYNC:     header.timing = HEADER::TIMING_VSYNC;     break;
				case IDC_GRAPHICS_TIMING_FRAMESKIP: header.timing = HEADER::TIMING_FRAMESKIP; break;
			}

			switch (SelectedEffect)
			{
     			case 0: header.effect = HEADER::EFFECT_NONE;        break;
				case 1: header.effect = HEADER::EFFECT_SCANLINES;   break;
				case 2: header.effect = HEADER::EFFECT_2XSAI;       break;
				case 3: header.effect = HEADER::EFFECT_SUPER_2XSAI; break;
				case 4: header.effect = HEADER::EFFECT_SUPER_EAGLE; break;
			}

			switch (SelectedPalette)
			{
     			case IDC_GRAPHICS_PALETTE_EMULATED: header.palette = HEADER::PALETTE_EMULATED; break;
     			case IDC_GRAPHICS_PALETTE_INTERNAL: header.palette = HEADER::PALETTE_INTERNAL; break;
				case IDC_GRAPHICS_PALETTE_CUSTOM:   header.palette = HEADER::PALETTE_CUSTOM;   break;
			}

			header.width           = adapters[SelectedAdapter].DisplayModes[ModeIndices[SelectedMode]].width;
			header.height          = adapters[SelectedAdapter].DisplayModes[ModeIndices[SelectedMode]].height;
			header.InfiniteSprites = context.InfiniteSprites ? 1 : 0;
			header.brightness      = context.brightness;
			header.saturation      = context.saturation;
			header.hue             = context.hue;
			header.ntsc            = ntsc;
			header.pal             = pal;

			memcpy( &header.guid, &adapters[SelectedAdapter].guid, sizeof(header.guid) );

			file->Write(header);
			file->Text().Write(PaletteFile);
		}
	}
	
	return DIRECTDRAW::Destroy();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT GRAPHICMANAGER::LoadPalette(const PDXSTRING& name)
{
	{
		const PDXSTRING tmp(name);
		PaletteFile = name;

		if (!ImportPalette())
		{
			PaletteFile = name;
			return PDX_FAILURE;
		}
	}

	SelectedPalette = IDC_GRAPHICS_PALETTE_CUSTOM;

	nes->GetGraphicContext( context );
	context.palette = palette;
	nes->SetGraphicContext( context );

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GRAPHICMANAGER::Reset()
{
	SelectedAdapter = 0;
	SelectedMode = 0;

	if (ModeIndices.IsEmpty())
		ModeIndices.InsertBack(0);
	
	ResetBpp();

	SelectedOffScreen = IDC_GRAPHICS_SRAM;
	SelectedPalette = IDC_GRAPHICS_PALETTE_INTERNAL;
	SelectedTiming  = IDC_GRAPHICS_TIMING_VSYNC;
	SelectedEffect = 0;

	SetRect( &ntsc, 0, 8, 255, 231 );
	SetRect( &pal,  0, 0, 255, 239 );

	SetScreenParameters
	(
		DIRECTDRAW::SCREENEFFECT_NONE,
		SelectedOffScreen == IDC_GRAPHICS_VRAM ? TRUE : FALSE,
		nes->IsPAL() ? pal : ntsc,
		TRUE
	);

	SetRefreshRate( nes->IsPAL() ? NES_FPS_PAL : NES_FPS_NTSC );

	PaletteFile.Clear();

	nes->GetGraphicContext( context );
	
	context.palette         = NULL;
	context.InfiniteSprites = FALSE;
	context.brightness      = 128;
	context.saturation      = 128;
	context.hue             = 128;
	context.PaletteMode     = NES::IO::GFX::PALETTE_EMULATED;
	
	nes->SetGraphicContext( context );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GRAPHICMANAGER::ResetBpp()
{
	EnableBpp();

	switch (adapters[SelectedAdapter].DisplayModes[0].bpp)
	{
     	case 16: SelectedBpp = IDC_GRAPHICS_16_BIT; break;
     	case 32: SelectedBpp = IDC_GRAPHICS_32_BIT; break;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GRAPHICMANAGER::EnableBpp()
{
	const DISPLAYMODES& modes = adapters[SelectedAdapter].DisplayModes;

	Support16Bpp = FALSE;

	for (UINT i=0; i < modes.Size(); ++i)
	{
		if (modes[i].bpp == 16)
		{
			Support16Bpp = TRUE;
			break;
		}
	}

	Support32Bpp = FALSE;

	for (UINT i=0; i < modes.Size(); ++i)
	{
		if (modes[i].bpp == 32)
		{
			Support32Bpp = TRUE;
			break;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT GRAPHICMANAGER::BeginDialogMode()
{
	application.ResetTimer();

	if (!IsWindowed())
	{
		SaveRect = GetScreenRect();

		if (CanRenderWindowed())
		{
			const DISPLAYMODE& mode = adapters[SelectedAdapter].DisplayModes[ModeIndices[SelectedMode]];

			if (mode.width < 640 || mode.height < 480)
				PDX_TRY(DIRECTDRAW::SwitchToFullScreen(640,480,16));
		}
		else
		{
			RECT rect;
			SetRect( &rect, 0, 0, 256, 224 );
			PDX_TRY(DIRECTDRAW::SwitchToWindowed(rect));
			ShouldBeFullscreen = TRUE;
		}

		return DIRECTDRAW::EnableGDI( TRUE );
	}

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT GRAPHICMANAGER::EndDialogMode()
{
	application.ResetTimer();

	if (!IsWindowed() || ShouldBeFullscreen)
	{
		ShouldBeFullscreen = FALSE;
		
		if (PreviousMode != SelectedMode)
		{
			PreviousMode = SelectedMode;
			SetScreenSize( SCREEN_FACTOR_4X, SaveRect );
		}

		const DISPLAYMODE& mode = adapters[SelectedAdapter].DisplayModes[ModeIndices[SelectedMode]];

		PDX_TRY(DIRECTDRAW::SwitchToFullScreen( mode.width, mode.height, mode.bpp, &SaveRect ));
		PDX_TRY(DIRECTDRAW::EnableGDI( FALSE ));

		application.UpdateWindowSizes( mode.width, mode.height );

		format.PaletteChanged = TRUE;
	}

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL GRAPHICMANAGER::DialogProc(HWND h,UINT uMsg,WPARAM wParam,LPARAM)
{
	switch (uMsg) 
	{
    	case WM_INITDIALOG:

			hDlg = h;
			PreviousMode = SelectedMode;
			UpdateDialog();
     		return TRUE;

		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
				case IDC_GRAPHICS_DEVICE:
     				
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						SelectedAdapter = ComboBox_GetCurSel(GetDlgItem(hDlg,IDC_GRAPHICS_DEVICE));
						UpdateDevice();
						ResetBpp();
						UpdateMode();
						UpdateBpp();
					}	     	     
					return TRUE;

				case IDC_GRAPHICS_MODE:

					if (HIWORD(wParam) == CBN_SELCHANGE)
						SelectedMode = ComboBox_GetCurSel(GetDlgItem(hDlg,IDC_GRAPHICS_MODE));
					
					return TRUE;

				case IDC_GRAPHICS_16_BIT:
				case IDC_GRAPHICS_32_BIT:

					if (SelectedBpp != LOWORD(wParam))
					{
						SelectedBpp = LOWORD(wParam);
						UpdateMode();
						UpdateBpp();
					}
					return TRUE;

				case IDC_GRAPHICS_SRAM:
				case IDC_GRAPHICS_VRAM:

					if (SelectedOffScreen != LOWORD(wParam))
					{
						SelectedOffScreen = LOWORD(wParam);
						UpdateOffScreen();
					}
					return TRUE;
  
				case IDC_GRAPHICS_EFFECTS:

					if (HIWORD(wParam) == CBN_SELCHANGE)
						SelectedEffect = ComboBox_GetCurSel(GetDlgItem(hDlg,IDC_GRAPHICS_EFFECTS));

					return TRUE;

				case IDC_GRAPHICS_NTSC_LEFT:
				case IDC_GRAPHICS_NTSC_TOP:
				case IDC_GRAPHICS_NTSC_RIGHT:
				case IDC_GRAPHICS_NTSC_BOTTOM:
				case IDC_GRAPHICS_PAL_LEFT:
				case IDC_GRAPHICS_PAL_TOP:
				case IDC_GRAPHICS_PAL_RIGHT:
				case IDC_GRAPHICS_PAL_BOTTOM:
				
					UpdateNesScreen(wParam);
					return TRUE;

				case IDC_GRAPHICS_INFINITE_SPRITES:

					context.InfiniteSprites = IsDlgButtonChecked(hDlg,IDC_GRAPHICS_INFINITE_SPRITES) == BST_CHECKED ? TRUE : FALSE;
					return TRUE;

				case IDC_GRAPHICS_PALETTE_EMULATED:
				case IDC_GRAPHICS_PALETTE_INTERNAL:
				case IDC_GRAPHICS_PALETTE_CUSTOM:

					if (SelectedPalette != LOWORD(wParam))
					{
						SelectedPalette = LOWORD(wParam);
						UpdatePalette();				
					}
					return TRUE;

				case IDC_GRAPHICS_TIMING_VSYNC:
				case IDC_GRAPHICS_TIMING_FRAMESKIP:

					if (SelectedTiming != LOWORD(wParam))
					{
						SelectedTiming = LOWORD(wParam);
						UpdateTiming();
					}
					return TRUE;

				case IDC_GRAPHICS_COLORS_RESET:

					context.brightness = 128;
					context.saturation = 128;

					if (IsWindowEnabled(GetDlgItem(hDlg,IDC_GRAPHICS_COLORS_HUE)))
						context.hue = 128;

					ResetColors();
					return TRUE;

				case IDC_GRAPHICS_PALETTE_BROWSE:

					BrowsePalette();
					return TRUE;

				case IDC_GRAPHICS_PALETTE_CLEAR:

					ClearPalette();
					return TRUE;

				case IDC_GRAPHICS_DEFAULT:

					Reset();
					UpdateDialog();
					return TRUE;

				case IDC_GRAPHICS_OK:

					EndDialog(hDlg,0);
					return TRUE;
			}
			return FALSE;

		case WM_HSCROLL:

			UpdateColors();
			return TRUE;

		case WM_CLOSE:

			EndDialog(hDlg,0);
			return TRUE;

		case WM_DESTROY:

			hDlg = NULL;

			if (PDX_SUCCEEDED(DIRECTDRAW::Create(&adapters[SelectedAdapter].guid)))
				UpdateDirectDraw();

			return TRUE;
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GRAPHICMANAGER::EnablePAL(const BOOL UsePAL)
{
	UpdateDirectDraw();
	SetRefreshRate(UsePAL ? NES_FPS_PAL : NES_FPS_NTSC);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GRAPHICMANAGER::UpdateDirectDraw()
{
	DIRECTDRAW::SCREENEFFECT effect = DIRECTDRAW::SCREENEFFECT_NONE;

	switch (SelectedEffect)
	{
		case 1: effect = DIRECTDRAW::SCREENEFFECT_SCANLINES;   break;
		case 2: effect = DIRECTDRAW::SCREENEFFECT_2XSAI;       break;
		case 3: effect = DIRECTDRAW::SCREENEFFECT_SUPER_2XSAI; break;
		case 4: effect = DIRECTDRAW::SCREENEFFECT_SUPER_EAGLE; break;
	}

	SetScreenParameters
	(
	    effect,
		SelectedOffScreen == IDC_GRAPHICS_VRAM ? TRUE : FALSE,
		nes->IsPAL() ? pal : ntsc,
		SelectedTiming == IDC_GRAPHICS_TIMING_VSYNC ? TRUE : FALSE
	);

	context.palette = NULL;

	switch (SelectedPalette)
	{
    	case IDC_GRAPHICS_PALETTE_EMULATED: 
			
			context.PaletteMode = NES::IO::GFX::PALETTE_EMULATED; 
			break;

		case IDC_GRAPHICS_PALETTE_CUSTOM:   
		
			if (ImportPalette())
			{
				context.palette = palette;
				context.PaletteMode = NES::IO::GFX::PALETTE_CUSTOM;
				break;
			}
		
		case IDC_GRAPHICS_PALETTE_INTERNAL:  
			
			context.PaletteMode = NES::IO::GFX::PALETTE_INTERNAL; 
			break;
	}

	nes->SetGraphicContext( context );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GRAPHICMANAGER::UpdateDevice()
{
	HWND item = GetDlgItem( hDlg, IDC_GRAPHICS_DEVICE );

	ComboBox_ResetContent( item );

	for (UINT i=0; i < adapters.Size(); ++i)
		ComboBox_AddString( item, adapters[i].name.Begin() );

	ComboBox_SetCurSel( item, SelectedAdapter );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GRAPHICMANAGER::UpdateBpp()
{
	SendMessage( GetDlgItem( hDlg, IDC_GRAPHICS_16_BIT ), BM_SETCHECK, BST_UNCHECKED, 0 );
	SendMessage( GetDlgItem( hDlg, IDC_GRAPHICS_32_BIT ), BM_SETCHECK, BST_UNCHECKED, 0 );
	SendMessage( GetDlgItem( hDlg, SelectedBpp         ), BM_SETCHECK, BST_CHECKED,   0 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GRAPHICMANAGER::UpdateOffScreen()
{
	SendMessage( GetDlgItem( hDlg, IDC_GRAPHICS_SRAM ), BM_SETCHECK, BST_UNCHECKED, 0 );
	SendMessage( GetDlgItem( hDlg, IDC_GRAPHICS_VRAM ), BM_SETCHECK, BST_UNCHECKED, 0 );
	SendMessage( GetDlgItem( hDlg, SelectedOffScreen ), BM_SETCHECK, BST_CHECKED,   0 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GRAPHICMANAGER::UpdateTiming()
{
	SendMessage( GetDlgItem( hDlg, IDC_GRAPHICS_TIMING_VSYNC     ), BM_SETCHECK, BST_UNCHECKED, 0 );
	SendMessage( GetDlgItem( hDlg, IDC_GRAPHICS_TIMING_FRAMESKIP ), BM_SETCHECK, BST_UNCHECKED, 0 );
	SendMessage( GetDlgItem( hDlg, SelectedTiming                ), BM_SETCHECK, BST_CHECKED,   0 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GRAPHICMANAGER::UpdateMode()
{
	HWND item = GetDlgItem( hDlg, IDC_GRAPHICS_MODE );

	ModeIndices.Clear();
	ComboBox_ResetContent (item );

	PDXSTRING string;

	const DISPLAYMODES& modes = adapters[SelectedAdapter].DisplayModes;

	const UINT bpp = (SelectedBpp == IDC_GRAPHICS_16_BIT) ? 16 : 32;

	for (UINT i=0; i < modes.Size(); ++i)
	{
		if (modes[i].bpp == bpp)
		{
			string.Set( modes[i].width );
			string.Append( "x" );
			string.Append( modes[i].height );

			ModeIndices.InsertBack( i );
			ComboBox_AddString( item, string. Begin() );
		}
	}

	if (ModeIndices.Size() <= SelectedMode)
		SelectedMode = 0;

	ComboBox_SetCurSel( item, SelectedMode );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GRAPHICMANAGER::UpdatePalette()
{
	SendMessage( GetDlgItem( hDlg, IDC_GRAPHICS_PALETTE_EMULATED ), BM_SETCHECK, BST_UNCHECKED, 0 );
	SendMessage( GetDlgItem( hDlg, IDC_GRAPHICS_PALETTE_INTERNAL ), BM_SETCHECK, BST_UNCHECKED, 0 );
	SendMessage( GetDlgItem( hDlg, IDC_GRAPHICS_PALETTE_CUSTOM   ), BM_SETCHECK, BST_UNCHECKED, 0 );
	SendMessage( GetDlgItem( hDlg, SelectedPalette               ), BM_SETCHECK, BST_CHECKED,   0 );

	SetDlgItemText( hDlg, IDC_GRAPHICS_PALETTE_PATH, PaletteFile.Begin() );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GRAPHICMANAGER::ResetColors()
{
	SetDlgItemText( hDlg, IDC_GRAPHICS_COLORS_BRIGHTNESS_VAL, PDXSTRING( context.brightness ) );
	SetDlgItemText( hDlg, IDC_GRAPHICS_COLORS_SATURATION_VAL, PDXSTRING( context.saturation ) );
	SetDlgItemText( hDlg, IDC_GRAPHICS_COLORS_HUE_VAL,        PDXSTRING( context.hue        ) );

	SendMessage( GetDlgItem( hDlg, IDC_GRAPHICS_COLORS_BRIGHTNESS ), TBM_SETPOS, WPARAM(TRUE), context.brightness );
	SendMessage( GetDlgItem( hDlg, IDC_GRAPHICS_COLORS_SATURATION ), TBM_SETPOS, WPARAM(TRUE), context.saturation );
	SendMessage( GetDlgItem( hDlg, IDC_GRAPHICS_COLORS_HUE        ), TBM_SETPOS, WPARAM(TRUE), context.hue        );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GRAPHICMANAGER::UpdateColors()
{
	static const INT types[3] = 
	{
		IDC_GRAPHICS_COLORS_BRIGHTNESS,
		IDC_GRAPHICS_COLORS_SATURATION,
		IDC_GRAPHICS_COLORS_HUE
	};

	for (UINT i=0; i < 3; ++i)
	{
		const UINT value = (UINT) SendMessage
		(
			GetDlgItem(hDlg,types[i]),
			TBM_GETPOS,
			WPARAM(0),
			LPARAM(0)
		);

		switch (i)
		{
   			case 0: context.brightness = value; break;
			case 1: context.saturation = value; break;
			case 2: context.hue        = value; break;
		}
	}

	ResetColors();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GRAPHICMANAGER::ClearPalette()
{
	PaletteFile.Clear();
	SelectedPalette = IDC_GRAPHICS_PALETTE_EMULATED;
	UpdatePalette();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GRAPHICMANAGER::UpdateEmulation()
{
	CheckDlgButton( hDlg, IDC_GRAPHICS_INFINITE_SPRITES, context.InfiniteSprites );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GRAPHICMANAGER::UpdateNesScreen(const WPARAM wParam)
{
	if (HIWORD(wParam) == EN_KILLFOCUS)
	{
		UINT value = GetDlgItemInt( hDlg, LOWORD(wParam), NULL, FALSE );
		
		switch (LOWORD(wParam))
		{
     		case IDC_GRAPHICS_NTSC_LEFT:   ntsc.left   = value = PDX_MIN( value, 255 ); break;
			case IDC_GRAPHICS_NTSC_TOP:	   ntsc.top    = value = PDX_MIN( value, 239 ); break;
     		case IDC_GRAPHICS_NTSC_RIGHT:  ntsc.right  = value = PDX_MAX( ntsc.left, PDX_MIN( value, 255 ) ); break;
       		case IDC_GRAPHICS_NTSC_BOTTOM: ntsc.bottom = value = PDX_MAX( ntsc.top,  PDX_MIN( value, 239 ) ); break;
     		case IDC_GRAPHICS_PAL_LEFT:	   pal.left    = value = PDX_MIN( value, 255 ); break;
     		case IDC_GRAPHICS_PAL_TOP:	   pal.top     = value = PDX_MIN( value, 239 ); break;
     		case IDC_GRAPHICS_PAL_RIGHT:   pal.right   = value = PDX_MAX( pal.left, PDX_MIN( value, 255 ) ); break;
     		case IDC_GRAPHICS_PAL_BOTTOM:  pal.bottom  = value = PDX_MAX( pal.top,  PDX_MIN( value, 239 ) ); break;
		}

		SetDlgItemInt( hDlg, LOWORD(wParam), value, FALSE );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GRAPHICMANAGER::UpdateDialog()
{
	UpdateDevice();

	EnableWindow( GetDlgItem( hDlg, IDC_GRAPHICS_16_BIT ), Support16Bpp );
	EnableWindow( GetDlgItem( hDlg, IDC_GRAPHICS_32_BIT ), Support32Bpp );

	SendMessage( GetDlgItem( hDlg, IDC_GRAPHICS_NTSC_LEFT   ), EM_LIMITTEXT, 3, 0 );
	SendMessage( GetDlgItem( hDlg, IDC_GRAPHICS_NTSC_TOP    ), EM_LIMITTEXT, 3, 0 );
	SendMessage( GetDlgItem( hDlg, IDC_GRAPHICS_NTSC_RIGHT  ), EM_LIMITTEXT, 3, 0 );
	SendMessage( GetDlgItem( hDlg, IDC_GRAPHICS_NTSC_BOTTOM ), EM_LIMITTEXT, 3, 0 );

	SendMessage( GetDlgItem( hDlg, IDC_GRAPHICS_PAL_LEFT   ), EM_LIMITTEXT, 3, 0 );
	SendMessage( GetDlgItem( hDlg, IDC_GRAPHICS_PAL_TOP    ), EM_LIMITTEXT, 3, 0 );
	SendMessage( GetDlgItem( hDlg, IDC_GRAPHICS_PAL_RIGHT  ), EM_LIMITTEXT, 3, 0 );
	SendMessage( GetDlgItem( hDlg, IDC_GRAPHICS_PAL_BOTTOM ), EM_LIMITTEXT, 3, 0 );

	SetDlgItemInt( hDlg, IDC_GRAPHICS_NTSC_LEFT,   ntsc.left,   FALSE );
	SetDlgItemInt( hDlg, IDC_GRAPHICS_NTSC_TOP,    ntsc.top,    FALSE );
	SetDlgItemInt( hDlg, IDC_GRAPHICS_NTSC_RIGHT,  ntsc.right,  FALSE );
	SetDlgItemInt( hDlg, IDC_GRAPHICS_NTSC_BOTTOM, ntsc.bottom, FALSE );

	SetDlgItemInt( hDlg, IDC_GRAPHICS_PAL_LEFT,   pal.left,   FALSE );
	SetDlgItemInt( hDlg, IDC_GRAPHICS_PAL_TOP,    pal.top,    FALSE );
	SetDlgItemInt( hDlg, IDC_GRAPHICS_PAL_RIGHT,  pal.right,  FALSE );
	SetDlgItemInt( hDlg, IDC_GRAPHICS_PAL_BOTTOM, pal.bottom, FALSE );

	SendMessage( GetDlgItem(hDlg,IDC_GRAPHICS_COLORS_BRIGHTNESS), TBM_SETRANGE, WPARAM(FALSE), LPARAM(MAKELONG(0,255)) );
	SendMessage( GetDlgItem(hDlg,IDC_GRAPHICS_COLORS_SATURATION), TBM_SETRANGE, WPARAM(FALSE), LPARAM(MAKELONG(0,255)) );
	SendMessage( GetDlgItem(hDlg,IDC_GRAPHICS_COLORS_HUE),        TBM_SETRANGE, WPARAM(FALSE), LPARAM(MAKELONG(0,255)) );

	ResetColors();
	UpdateBpp();
	UpdateMode();
	UpdateEmulation();
	UpdateEffects();
	UpdatePalette();
	UpdateOffScreen();
	UpdateTiming();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GRAPHICMANAGER::BrowsePalette()
{
	CHAR name[NST_MAX_PATH];
	name[0] = '\0';

	OPENFILENAME ofn;
	PDXMemZero( ofn );

	ofn.lStructSize  = sizeof(ofn);
	ofn.hwndOwner    = MANAGER::hWnd;
	ofn.lpstrFilter  = "Palette Files (.pal)\0*.pal\0All Files (*.*)\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFile    = name;
	ofn.lpstrTitle   = "Import Palette";
	ofn.nMaxFile     = NST_MAX_PATH;
	ofn.Flags        = OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;

	if (GetOpenFileName(&ofn))
		PaletteFile = name;

	SetDlgItemText( hDlg, IDC_GRAPHICS_PALETTE_PATH, PaletteFile.Begin() );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL GRAPHICMANAGER::ImportPalette()
{
	if (PaletteFile.Size())
	{
		PDXFILE file( PaletteFile, PDXFILE::INPUT);

		if (file.IsOpen() && file.Size() >= sizeof(U8) * PALETTE_LENGTH)
		{
			memcpy( palette, file.Begin(), sizeof(U8) * PALETTE_LENGTH );
			return TRUE;
		}
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GRAPHICMANAGER::UpdateEffects()
{
	HWND hItem = GetDlgItem( hDlg, IDC_GRAPHICS_EFFECTS );

	ComboBox_ResetContent( hItem );
	ComboBox_AddString( hItem, "none" );
	ComboBox_AddString( hItem, "scanlines" );
	ComboBox_AddString( hItem, "2xSaI" );
	ComboBox_AddString( hItem, "Super 2xSaI" );
	ComboBox_AddString( hItem, "Super Eagle" );
	ComboBox_SetCurSel( hItem, SelectedEffect );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT GRAPHICMANAGER::SwitchToWindowed(const RECT& rect)
{
	PDX_TRY(DIRECTDRAW::SwitchToWindowed( rect ));
	format.PaletteChanged = TRUE;
	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT GRAPHICMANAGER::SwitchToFullScreen(const SCREENTYPE ScreenType)
{
	RECT rect;
	SetScreenSize( ScreenType, rect );
	
	const DISPLAYMODE& mode = adapters[SelectedAdapter].DisplayModes[ModeIndices[SelectedMode]];
	PDX_TRY(DIRECTDRAW::SwitchToFullScreen( mode.width, mode.height, mode.bpp, &rect ));
	
	format.PaletteChanged = TRUE;	
	
	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT GRAPHICMANAGER::SetScreenSize(const SCREENTYPE ScreenType)
{
	RECT rect;
	SetScreenSize( ScreenType, rect );
	UpdateScreenRect( rect );
	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GRAPHICMANAGER::SetScreenSize(const SCREENTYPE ScreenType,RECT& rect)
{
	const UINT width  = adapters[SelectedAdapter].DisplayModes[ModeIndices[SelectedMode]].width;
	const UINT height = adapters[SelectedAdapter].DisplayModes[ModeIndices[SelectedMode]].height;

	if (ScreenType == SCREEN_STRETCHED)
	{
		SetRect
		( 
	     	&rect, 
			0, 
			0, 
			width, 
			height
		);
	}
	else
	{
		UINT x = GetNesRect().right - GetNesRect().left;
		UINT y = GetNesRect().bottom - GetNesRect().top;

		const UINT factor = UINT(ScreenType);

		for (UINT i=0; i < factor; ++i)
		{
			if (x * 2 > width || y * 2 > height) 
				break;

			x *= 2;
			y *= 2;
		}

		x = ( width  - x ) / 2;
		y = ( height - y ) / 2;

		SetRect
		(
			&rect, 
			x, 
			y, 
			width - x, 
			height - y 
		);
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

typedef Gdiplus::Status (WINGDIPAPI* STARTUP) 
(
     ULONG_PTR*,
	 const Gdiplus::GdiplusStartupInput*,
	 Gdiplus::GdiplusStartupOutput*
);

typedef Gdiplus::GpStatus (WINGDIPAPI* FROMDIRECTDRAW)
(
     IDirectDrawSurface7*,
	 Gdiplus::GpBitmap**
);

typedef Gdiplus::GpStatus (WINGDIPAPI* DISPOSEIMAGE)
(
     Gdiplus::GpImage*
);

typedef VOID (WINGDIPAPI* SHUTDOWN)
(
     ULONG_PTR
);

typedef Gdiplus::GpStatus (WINGDIPAPI* SAVEIMAGE)
(
     Gdiplus::GpImage*,
	 GDIPCONST WCHAR*,
	 GDIPCONST CLSID*,
	 GDIPCONST Gdiplus::EncoderParameters*
);

typedef Gdiplus::Status (WINGDIPAPI* GETIMAGEENCODERSIZE)
(
     UINT*,
	 UINT*
);

typedef Gdiplus::Status (WINGDIPAPI* GETIMAGEENCODERS)
(
     UINT,
	 UINT,
	 Gdiplus::ImageCodecInfo*
);

#define NST_GETFUNCTION(x,y) PDX_CAST(x,GetProcAddress(hDLL,y))

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

static PDXRESULT GetBitmapEncoder(const PDXSTRING& format,CLSID& clsid,HMODULE hDLL)
{
	const WCHAR* string;

	// gif is not included here - I hate gif

	     if (format == "png" ) string = L"image/png"; 
	else if (format == "jpg" ) string = L"image/jpeg";
	else if (format == "jpeg") string = L"image/jpeg";
	else if (format == "bmp" ) string = L"image/bmp"; 
	else if (format == "tif" ) string = L"image/tiff";
	else if (format == "tiff") string = L"image/tiff";
	else return PDX_FAILURE;

	UINT num = 0;
	UINT size = 0;

	GETIMAGEENCODERSIZE GetImageEncoderSize = NST_GETFUNCTION
	(
     	GETIMAGEENCODERSIZE,
		"GdipGetImageEncodersSize"
	);

	if (!GetImageEncoderSize) 
		return PDX_FAILURE;

	GetImageEncoderSize( &num, &size );
	if (!size) return PDX_FAILURE;

	Gdiplus::ImageCodecInfo* const CodecInfo = PDX_CAST(Gdiplus::ImageCodecInfo*,new CHAR[size]);

	GETIMAGEENCODERS GetImageEncoders = NST_GETFUNCTION
	(
     	GETIMAGEENCODERS,
		"GdipGetImageEncoders"
	);
	
	if (GetImageEncoders) 
	{
		GetImageEncoders(num,size,CodecInfo);

		for (UINT j=0; j < num; ++j)
       	{
       		if (wcscmp(CodecInfo[j].MimeType,string) == 0)
       		{
       			clsid = CodecInfo[j].Clsid;
       			delete [] CodecInfo;
       			return PDX_OK;
     		}    
     	}
	}

	delete [] CodecInfo;

	return PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT GRAPHICMANAGER::ExportBitmap(const PDXSTRING& filename)
{
	PDXRESULT result = PDX_FAILURE;	
	
	SHUTDOWN shutdown = NULL;
	DISPOSEIMAGE DisposeImage = NULL;	
	Gdiplus::GpBitmap* bitmap = NULL;

	HMODULE hDLL = LoadLibrary("GdiPlus.dll");
	
	if (!hDLL) 
		goto hell;

	ULONG token;
	Gdiplus::GdiplusStartupInput input;

	STARTUP StartUp = NST_GETFUNCTION
	(
       	STARTUP,
		"GdiplusStartup"
	);

	if (!StartUp || StartUp(&token,&input,NULL) != Gdiplus::Ok)
		goto hell;

	shutdown = NST_GETFUNCTION
	(
     	SHUTDOWN,
		"GdiplusShutdown"
	);

	FROMDIRECTDRAW FromDirectDraw = NST_GETFUNCTION
	(
     	FROMDIRECTDRAW,
		"GdipCreateBitmapFromDirectDrawSurface"
	);

	if (!FromDirectDraw || FromDirectDraw(GetNesBuffer(),&bitmap) != Gdiplus::Ok || !bitmap)
		goto hell;

	DisposeImage = NST_GETFUNCTION
	(
     	DISPOSEIMAGE,
		"GdipDisposeImage"
	);

	CLSID encoder;

	if (PDX_FAILED(GetBitmapEncoder(filename.GetFileExtension(),encoder,hDLL)))
		goto hell;

	WCHAR* const wString = new WCHAR[filename.Length() + 1];
	mbstowcs( wString, filename, filename.Length() + 1 );

	SAVEIMAGE SaveImage = NST_GETFUNCTION
	(
       	SAVEIMAGE,
		"GdipSaveImageToFile"
	);

	if (SaveImage && SaveImage(PDX_STATIC_CAST(Gdiplus::GpImage*,bitmap),wString,&encoder,NULL) == Gdiplus::Ok)
		result = PDX_OK;

	delete [] wString;

hell:

	if (bitmap && DisposeImage)
		DisposeImage(PDX_STATIC_CAST(Gdiplus::GpImage*,bitmap));

	if (shutdown)
		shutdown(token);

	if (hDLL)
		FreeLibrary(hDLL);

	if (PDX_FAILED(result))
		application.OnWarning("Couldn't export any bitmaps, are you sure you have GdiPlus properly installed?");

	return result;
}

#undef NST_GETFUNCTION

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT GRAPHICMANAGER::SaveScreenShot()
{
	PDXSTRING filename;
	filename.Buffer().Resize( NST_MAX_PATH );
	filename.Buffer().Front() = '\0';

	{
		OPENFILENAME ofn;
		PDXMemZero( ofn );

		ofn.lStructSize  = sizeof(ofn);
		ofn.hwndOwner    = MANAGER::hWnd;
		ofn.lpstrFilter  = "Bitmap Files (*.png, *.jpg, *.bmp, *.tif)\0*.png;*.jpg;*.bmp;*.tif\0All Files (*.*)\0*.*\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrFile    = filename.Begin();
		ofn.lpstrTitle   = "Save Screenshot";
		ofn.nMaxFile     = NST_MAX_PATH;
		ofn.Flags        = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;

		if (!GetSaveFileName( &ofn ))
			return PDX_FAILURE;
	}

	filename.Validate();

	if (filename.Size() && filename.GetFileExtension().IsEmpty())
		filename.Append( NST_DEFAULT_SCREENSHOT_FILE_FORMAT );

	return ExportBitmap( filename );
}

