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

BOOL GRAPHICMANAGER::HasGDIPlus()
{
	HMODULE hDLL = LoadLibrary("GdiPlus.dll");

	if (hDLL)
	{
		FreeLibrary( hDLL );
		return TRUE;
	}

	return FALSE;
}

const BOOL GRAPHICMANAGER::GdiPlusAvailable = GRAPHICMANAGER::HasGDIPlus();

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

GRAPHICMANAGER::GRAPHICMANAGER()
: 
MANAGER           (IDD_GRAPHICS), 
hDlg              (NULL),
SelectedAdapter   (0),
SelectedMode      (0),
SelectedBpp       (IDC_GRAPHICS_16_BIT),
SelectedOffScreen (IDC_GRAPHICS_SRAM),
SelectedEffect    (0),
SelectedPalette   (IDC_GRAPHICS_PALETTE_INTERNAL),
SelectedFactor	  (SCREEN_FACTOR_4X),
Support8Bpp       (FALSE),
Support16Bpp      (FALSE),
Support32Bpp      (FALSE),
palette           (new U8[PALETTE_LENGTH])
{
	SetRect( &rcNtsc, 0, 8, 255, 231 );
	SetRect( &rcPal,  0, 0, 255, 239 );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

GRAPHICMANAGER::~GRAPHICMANAGER()
{
	delete [] palette;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GRAPHICMANAGER::Create(CONFIGFILE* const ConfigFile)
{
	SelectedAdapter = 0;

	DIRECTDRAW::Initialize( MANAGER::hWnd );

	if (ConfigFile)
	{
		CONFIGFILE& file = *ConfigFile;

		CreateDevice( CONFIGFILE::ToGUID( file["video device"].String() ) );

		{
			const PDXSTRING& bpp = file["video fullscreen bpp"];

			SelectedBpp =
			(
				bpp == "8"  ? IDC_GRAPHICS_8_BIT  :
     			bpp == "32" ? IDC_GRAPHICS_32_BIT :
     			IDC_GRAPHICS_16_BIT
			);
		}

		EnableBpp();

		SelectedOffScreen = (file[ "video offscreen buffer in vram" ] == "yes" ? IDC_GRAPHICS_VRAM : IDC_GRAPHICS_SRAM);	

		{
			const UINT width = file[ "video fullscreen width" ].ToUlong();
			const UINT height = file[ "video fullscreen height" ].ToUlong();

			const UINT bpp = 
			(
		     	SelectedBpp == IDC_GRAPHICS_32_BIT ? 32 : 
     			SelectedBpp == IDC_GRAPHICS_16_BIT ? 16 :
				8
			);

			if (!InitModes(width,height,bpp))
				ResetBpp();
		}

		{
			const PDXSTRING& filter = file["video filter"];

			SelectedEffect =
			(
	     		filter == "scanlines"   ? 1 :
	     		filter == "tv"          ? 2 :
	    		filter == "2xsai"       ? 3 :
	 			filter == "super 2xsai" ? 4 :
	 			filter == "super eagle" ? 5 :
	 			0
			);

			if (SelectedBpp == IDC_GRAPHICS_8_BIT)
				SelectedEffect = 0;
		}

		{
			const PDXSTRING& factor = file["video screen"];

			SelectedFactor =
			(
				factor == "1x"        ? SCREEN_FACTOR_1X :
	     		factor == "2x"        ? SCREEN_FACTOR_2X :
    	   		factor == "3x"        ? SCREEN_FACTOR_3X :
	  			factor == "stretched" ? SCREEN_STRETCHED :
    			SCREEN_FACTOR_4X
			);
		}

		{
			const PDXSTRING& palette = file["video palette"];

			SelectedPalette =
			(
				palette == "emulated" ? IDC_GRAPHICS_PALETTE_EMULATED :
	 			palette == "custom"   ? IDC_GRAPHICS_PALETTE_CUSTOM   :
	   			IDC_GRAPHICS_PALETTE_INTERNAL
			);
		}

		context.InfiniteSprites = (file[ "video infinite sprites" ] == "yes" ? TRUE : FALSE);

		const PDXSTRING* string;

		string = &file[ "video ntsc left"   ]; rcNtsc.left   = string->Length() ? string->ToUlong() : 0;
		string = &file[ "video ntsc top"    ]; rcNtsc.top    = string->Length() ? string->ToUlong() : 8;
		string = &file[ "video ntsc right"  ]; rcNtsc.right  = string->Length() ? string->ToUlong() : 255;
		string = &file[ "video ntsc bottom" ]; rcNtsc.bottom = string->Length() ? string->ToUlong() : 231;
		string = &file[ "video pal left"    ]; rcPal.left    = string->Length() ? string->ToUlong() : 0;
		string = &file[ "video pal top"     ]; rcPal.top     = string->Length() ? string->ToUlong() : 0;
		string = &file[ "video pal right"   ]; rcPal.right   = string->Length() ? string->ToUlong() : 255;
		string = &file[ "video pal bottom"  ]; rcPal.bottom  = string->Length() ? string->ToUlong() : 239;

		rcNtsc.left   = PDX_CLAMP( rcNtsc.left,   0,             255 );
		rcNtsc.top    = PDX_CLAMP( rcNtsc.top,    0,             239 );
		rcNtsc.right  = PDX_CLAMP( rcNtsc.right,  rcNtsc.left+1, 255 );
		rcNtsc.bottom = PDX_CLAMP( rcNtsc.bottom, rcNtsc.top+1,  239 );
		rcPal.left    = PDX_CLAMP( rcPal.left,    0,             255 );
		rcPal.top     = PDX_CLAMP( rcPal.top,     0,             239 );
		rcPal.right   = PDX_CLAMP( rcPal.right,   rcPal.left+1,  255 );
		rcPal.bottom  = PDX_CLAMP( rcPal.bottom,  rcPal.top+1,   239 );

		string = &file[ "video color brightness" ]; context.brightness = string->Length() ? string->ToUlong() : 128;
		string = &file[ "video color saturation" ]; context.saturation = string->Length() ? string->ToUlong() : 128;
		string = &file[ "video color hue"        ]; context.hue        = string->Length() ? string->ToUlong() : 128;

		context.brightness = PDX_MIN(context.brightness,255);
		context.saturation = PDX_MIN(context.saturation,255);
		context.hue        = PDX_MIN(context.hue,255);

		PaletteFile = file["video palette file"];
	}
	else
	{
		Reset();
	}

	UpdateDirectDraw();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GRAPHICMANAGER::CreateDevice(GUID guid)
{
	for (UINT i=0; i < adapters.Size(); ++i)
	{
		if (PDXMemCompare(guid,adapters[i].guid))
		{
			if (DIRECTDRAW::Create(guid))
			{
				SelectedAdapter = i;
				return;
			}
			break;
		}
	}

	PDXMemZero( guid );

	for (UINT i=0; i < adapters.Size(); ++i)
	{
		if (PDXMemCompare(guid,adapters[i].guid))
		{
			if (DIRECTDRAW::Create(guid))
			{
				SelectedAdapter = i;
				return;
			}
			break;
		}
	}

	if (adapters.Size() && DIRECTDRAW::Create(adapters.Front().guid))
	{
		SelectedAdapter = 0;
		return;
	}

	throw ("No valid graphic device found!");
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GRAPHICMANAGER::Destroy(CONFIGFILE* const ConfigFile)
{
	if (ConfigFile)
	{
		CONFIGFILE& file = *ConfigFile;
	
		BOOL WroteMode = FALSE;

		if (adapters.Size() > SelectedAdapter)
		{
			const ADAPTER& adapter = adapters[SelectedAdapter];

			file[ "video device" ] = CONFIGFILE::FromGUID( adapter.guid );

			if (SelectedMode < ModeIndices.Size() && ModeIndices[SelectedMode] < adapter.DisplayModes.Size())
			{
				file[ "video fullscreen width"  ] = adapters[SelectedAdapter].DisplayModes[ModeIndices[SelectedMode]].width;
				file[ "video fullscreen height" ] = adapters[SelectedAdapter].DisplayModes[ModeIndices[SelectedMode]].height;
				file[ "video fullscreen bpp"    ] = adapters[SelectedAdapter].DisplayModes[ModeIndices[SelectedMode]].bpp;
				WroteMode = TRUE;
			}
		}
		else
		{
			file[ "video device" ];
		}
		
		if (!WroteMode)
		{
			file[ "video fullscreen width"  ];
			file[ "video fullscreen height" ];
			file[ "video fullscreen bpp"    ];
		}
	
		switch (SelectedOffScreen)
		{
        	case IDC_GRAPHICS_VRAM: file[ "video offscreen buffer in vram" ] = "yes"; break;
			default:                file[ "video offscreen buffer in vram" ] = "no";  break;
		}
	
		switch (SelectedEffect)
		{
			case 1:  file[ "video filter" ] = "scanlines";   break;
			case 2:  file[ "video filter" ] = "tv";          break;
			case 3:  file[ "video filter" ] = "2xsai";       break;
			case 4:  file[ "video filter" ] = "super 2xsai"; break;
			case 5:  file[ "video filter" ] = "super eagle"; break;
			default: file[ "video filter" ] = "none";        break;
		}

		switch (SelectedFactor)
		{
			case SCREEN_FACTOR_1X: file[ "video screen" ] = "1x";        break;
			case SCREEN_FACTOR_2X: file[ "video screen" ] = "2x";        break;
			case SCREEN_FACTOR_3X: file[ "video screen" ] = "3x";        break;
			case SCREEN_FACTOR_4X: file[ "video screen" ] = "4x";        break;
			default:               file[ "video screen" ] = "stretched"; break;
		}

		switch (SelectedPalette)
		{
         	case IDC_GRAPHICS_PALETTE_EMULATED: file[ "video palette" ] = "emulated"; break;
         	case IDC_GRAPHICS_PALETTE_CUSTOM:   file[ "video palette" ] = "custom";   break;
         	default:                            file[ "video palette" ] = "internal"; break;
		}
		
		file[ "video palette file"     ] = PaletteFile;	
		file[ "video infinite sprites" ] = (context.InfiniteSprites ? "yes" : "no");	
		file[ "video ntsc left"        ] = rcNtsc.left;
		file[ "video ntsc top"         ] = rcNtsc.top;
		file[ "video ntsc right"       ] = rcNtsc.right;
		file[ "video ntsc bottom"      ] = rcNtsc.bottom;	
		file[ "video pal left"         ] = rcPal.left;
		file[ "video pal top"          ] = rcPal.top;
		file[ "video pal right"        ] = rcPal.right;
		file[ "video pal bottom"       ] = rcPal.bottom;	
		file[ "video color brightness" ] = context.brightness;
		file[ "video color saturation" ] = context.saturation;
		file[ "video color hue"        ] = context.hue;
	}

	DIRECTDRAW::Destroy();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL GRAPHICMANAGER::InitModes(const UINT width,const UINT height,const UINT bpp)
{
	if (SelectedAdapter >= adapters.Size())
		return FALSE;

	ModeIndices.Clear();

	BOOL found = FALSE;

	const ADAPTER& adapter = adapters[SelectedAdapter];

	for (UINT i=0; i < adapter.DisplayModes.Size(); ++i)
	{
		const DISPLAYMODE& mode = adapter.DisplayModes[i];

		if (mode.bpp == bpp)
		{
			if (mode.width == width && mode.height == height)
			{
				found = TRUE;
				SelectedMode = ModeIndices.Size();
			}

			ModeIndices.InsertBack(i);
		}
	}

	return found;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL GRAPHICMANAGER::OnFocus(const BOOL active)
{
	LPDIRECTDRAW7 device = GetDevice();

	if (!device)
		return TRUE;
	
	HRESULT hResult;

	if (FAILED(hResult=device->TestCooperativeLevel()))
	{
		switch (hResult)
		{
	 		case DDERR_EXCLUSIVEMODEALREADYSET:
	   		case DDERR_NOEXCLUSIVEMODE:
	   			return FALSE;

	 		case DDERR_WRONGMODE:
			{
				if (active && IsWindowed())
				{
					DIRECTDRAW::Create( adapters[SelectedAdapter].guid, TRUE );
					DIRECTDRAW::SwitchToWindowed( DIRECTDRAW::GetScreenRect(), TRUE );
					return TRUE;
				}
			}
		}
	}

	if (active)
		device->RestoreAllSurfaces();
	
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GRAPHICMANAGER::LoadPalette(const PDXSTRING& name)
{
	{
		const PDXSTRING tmp(name);
		PaletteFile = name;

		if (!ImportPalette())
		{
			PaletteFile = name;
			return;
		}
	}

	SelectedPalette = IDC_GRAPHICS_PALETTE_CUSTOM;

	nes.GetGraphicContext( context );
	context.palette = palette;
	nes.SetGraphicContext( context );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GRAPHICMANAGER::Reset()
{
	GUID guid;
	PDXMemZero( guid );
	CreateDevice( guid );

	ResetBpp();

	SelectedOffScreen = IDC_GRAPHICS_SRAM;
	SelectedPalette = IDC_GRAPHICS_PALETTE_INTERNAL;
	SelectedEffect = 0;
	SelectedFactor = SCREEN_FACTOR_4X;

	SetRect( &rcNtsc, 0, 8, 255, 231 );
	SetRect( &rcPal,  0, 0, 255, 239 );

	PaletteFile.Clear();

	nes.GetGraphicContext( context );
	
	context.palette         = NULL;
	context.InfiniteSprites = FALSE;
	context.brightness      = 128;
	context.saturation      = 128;
	context.hue             = 128;
	context.PaletteMode     = NES::IO::GFX::PALETTE_EMULATED;
	
	nes.SetGraphicContext( context );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GRAPHICMANAGER::ResetBpp()
{
	PDX_ASSERT(SelectedAdapter < adapters.Size());

	if (SelectedAdapter >= adapters.Size())
		throw ("Internal error in GRAPHICMANAGER::ResetBPP()!");

	EnableBpp();

	SelectedMode = 0;
	ModeIndices.Clear();

	UINT DesiredBpp;

	if (Support16Bpp)
	{
		DesiredBpp = 16;
		SelectedBpp = IDC_GRAPHICS_16_BIT;
	}
	else if (Support8Bpp)
	{
		DesiredBpp = 8;
		SelectedBpp = IDC_GRAPHICS_8_BIT;
	}
	else
	{
		DesiredBpp = 32;
		SelectedBpp = IDC_GRAPHICS_32_BIT;
	}

	const ADAPTER& adapter = adapters[SelectedAdapter];

	for (UINT i=0; i < adapter.DisplayModes.Size(); ++i)
	{
		if (adapter.DisplayModes[i].bpp == DesiredBpp)
			ModeIndices.InsertBack(i);
	}

	if (ModeIndices.IsEmpty())
		throw ("Couldn't find any valid display mode!");
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GRAPHICMANAGER::EnableBpp()
{
	PDX_ASSERT(SelectedAdapter < adapters.Size());

	Support8Bpp = FALSE;
	Support16Bpp = FALSE;
	Support32Bpp = FALSE;

	if (SelectedAdapter >= adapters.Size())
		throw ("Internal error in GRAPHICMANAGER::EnableBPP()!");

	const DISPLAYMODES& modes = adapters[SelectedAdapter].DisplayModes;

	for (UINT i=0; i < modes.Size(); ++i)
	{
		switch (modes[i].bpp)
		{
     		case 8:  Support8Bpp  = TRUE; continue;
			case 16: Support16Bpp = TRUE; continue;
			case 32: Support32Bpp = TRUE; continue;
		}
	}

	if (!Is8BitModeSupported())
		Support8Bpp = FALSE;

	if (!Support8Bpp && !Support16Bpp && !Support32Bpp)
		throw ("Couldn't find a valid display mode!");
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GRAPHICMANAGER::BeginDialogMode()
{
	if (!IsWindowed())
	{
		CheckModeBounds();
		const DISPLAYMODE& mode = adapters[SelectedAdapter].DisplayModes[ModeIndices[SelectedMode]];

		if (mode.width < 640 || mode.height < 480)
		{
			RECT rect;
			SetScreenSize( SelectedFactor, rect, 640, 480 );
			DIRECTDRAW::SwitchToFullScreen( 640, 480, 16, rect );
		}

		DIRECTDRAW::EnableGDI( TRUE );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GRAPHICMANAGER::EndDialogMode()
{
	if (!IsWindowed())
	{
		CheckModeBounds();
		const DISPLAYMODE& mode = adapters[SelectedAdapter].DisplayModes[ModeIndices[SelectedMode]];

		RECT rect;
		SetScreenSize( SelectedFactor, rect );
		DIRECTDRAW::SwitchToFullScreen( mode.width, mode.height, mode.bpp, rect );
		DIRECTDRAW::EnableGDI( FALSE );

		application.UpdateWindowSizes( mode.width, mode.height );
	}
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
						UpdateEffects();
					}	     	     
					return TRUE;

				case IDC_GRAPHICS_MODE:

					if (HIWORD(wParam) == CBN_SELCHANGE)
						SelectedMode = ComboBox_GetCurSel(GetDlgItem(hDlg,IDC_GRAPHICS_MODE));
					
					return TRUE;

				case IDC_GRAPHICS_8_BIT:
				case IDC_GRAPHICS_16_BIT:
				case IDC_GRAPHICS_32_BIT:

					if (SelectedBpp != LOWORD(wParam))
					{
						SelectedBpp = LOWORD(wParam);
						UpdateMode();
						UpdateBpp();
						UpdateEffects();
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

					context.InfiniteSprites = (IsDlgButtonChecked(hDlg,IDC_GRAPHICS_INFINITE_SPRITES) == BST_CHECKED);
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

			CreateDevice(adapters[SelectedAdapter].guid);
			UpdateDirectDraw();
			hDlg = NULL;

			return TRUE;
	}

	return FALSE;
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
		case 2: effect = DIRECTDRAW::SCREENEFFECT_TV;          break;
		case 3: effect = DIRECTDRAW::SCREENEFFECT_2XSAI;       break;
		case 4: effect = DIRECTDRAW::SCREENEFFECT_SUPER_2XSAI; break;
		case 5: effect = DIRECTDRAW::SCREENEFFECT_SUPER_EAGLE; break;
	}

	PDX_ASSERT(SelectedEffect == 0 || SelectedBpp != IDC_GRAPHICS_8_BIT);

	RECT sRect;
	RECT* pRect = NULL;

	if (!IsWindowed() && !hDlg)
	{
		SetScreenSize( SelectedFactor, sRect );
		pRect = &sRect;
	}

	SetScreenParameters
	(
	    effect,
		SelectedOffScreen == IDC_GRAPHICS_VRAM ? TRUE : FALSE,
		nes.IsPAL() ? rcPal : rcNtsc,
		pRect
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

	nes.SetGraphicContext( context );
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
	SendMessage( GetDlgItem( hDlg, IDC_GRAPHICS_8_BIT  ), BM_SETCHECK, BST_UNCHECKED, 0 );
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

VOID GRAPHICMANAGER::UpdateMode()
{
	HWND item = GetDlgItem( hDlg, IDC_GRAPHICS_MODE );

	ModeIndices.Clear();
	ComboBox_ResetContent (item );

	PDXSTRING string;

	const DISPLAYMODES& modes = adapters[SelectedAdapter].DisplayModes;

	const UINT bpp = 
	(
     	SelectedBpp == IDC_GRAPHICS_32_BIT ? 32 : 
	    SelectedBpp == IDC_GRAPHICS_16_BIT ? 16 :
       	8
	);

	for (UINT i=0; i < modes.Size(); ++i)
	{
		if (modes[i].bpp == bpp)
		{
			string  = modes[i].width;
			string += "x";
			string += modes[i].height;

			ModeIndices.InsertBack( i );
			ComboBox_AddString( item, string. Begin() );
		}
	}

	if (ModeIndices.Size() <= SelectedMode)
		SelectedMode = 0;

	PDX_ASSERT(ModeIndices.Size());

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
	SetDlgItemText( hDlg, IDC_GRAPHICS_COLORS_BRIGHTNESS_VAL, PDXSTRING( context.brightness ).String() );
	SetDlgItemText( hDlg, IDC_GRAPHICS_COLORS_SATURATION_VAL, PDXSTRING( context.saturation ).String() );
	SetDlgItemText( hDlg, IDC_GRAPHICS_COLORS_HUE_VAL,        PDXSTRING( context.hue        ).String() );

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
     		case IDC_GRAPHICS_NTSC_LEFT:   rcNtsc.left   = value = PDX_MIN( value, 255 ); break;
			case IDC_GRAPHICS_NTSC_TOP:	   rcNtsc.top    = value = PDX_MIN( value, 239 ); break;
     		case IDC_GRAPHICS_NTSC_RIGHT:  rcNtsc.right  = value = PDX_MAX( rcNtsc.left, PDX_MIN( value, 255 ) ); break;
       		case IDC_GRAPHICS_NTSC_BOTTOM: rcNtsc.bottom = value = PDX_MAX( rcNtsc.top,  PDX_MIN( value, 239 ) ); break;
     		case IDC_GRAPHICS_PAL_LEFT:	   rcPal.left    = value = PDX_MIN( value, 255 ); break;
     		case IDC_GRAPHICS_PAL_TOP:	   rcPal.top     = value = PDX_MIN( value, 239 ); break;
     		case IDC_GRAPHICS_PAL_RIGHT:   rcPal.right   = value = PDX_MAX( rcPal.left, PDX_MIN( value, 255 ) ); break;
     		case IDC_GRAPHICS_PAL_BOTTOM:  rcPal.bottom  = value = PDX_MAX( rcPal.top,  PDX_MIN( value, 239 ) ); break;
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

	EnableWindow( GetDlgItem( hDlg, IDC_GRAPHICS_8_BIT ),  Support8Bpp  );
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

	SetDlgItemInt( hDlg, IDC_GRAPHICS_NTSC_LEFT,   rcNtsc.left,   FALSE );
	SetDlgItemInt( hDlg, IDC_GRAPHICS_NTSC_TOP,    rcNtsc.top,    FALSE );
	SetDlgItemInt( hDlg, IDC_GRAPHICS_NTSC_RIGHT,  rcNtsc.right,  FALSE );
	SetDlgItemInt( hDlg, IDC_GRAPHICS_NTSC_BOTTOM, rcNtsc.bottom, FALSE );

	SetDlgItemInt( hDlg, IDC_GRAPHICS_PAL_LEFT,   rcPal.left,   FALSE );
	SetDlgItemInt( hDlg, IDC_GRAPHICS_PAL_TOP,    rcPal.top,    FALSE );
	SetDlgItemInt( hDlg, IDC_GRAPHICS_PAL_RIGHT,  rcPal.right,  FALSE );
	SetDlgItemInt( hDlg, IDC_GRAPHICS_PAL_BOTTOM, rcPal.bottom, FALSE );

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
	ofn.nMaxFile     = NST_MAX_PATH-1;
	ofn.Flags        = OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;

	if (GetOpenFileName(&ofn) && name && strlen(name))
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
	ComboBox_AddString( hItem, "None" );

	CheckModeBounds();

	if (adapters[SelectedAdapter].DisplayModes[ModeIndices[SelectedMode]].bpp > 8)
	{
		ComboBox_AddString( hItem, "Scanlines"    );
		ComboBox_AddString( hItem, "TV-Mode"      );
		ComboBox_AddString( hItem, "2xSaI"        );
		ComboBox_AddString( hItem, "Super 2xSaI"  );
		ComboBox_AddString( hItem, "Super Eagle"  );
	}
	else
	{
		SelectedEffect = 0;
	}

	ComboBox_SetCurSel( hItem, SelectedEffect );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GRAPHICMANAGER::CheckModeBounds() const
{
	const BOOL InBound = 
	(
		SelectedAdapter < adapters.Size() &&
		SelectedMode < ModeIndices.Size() &&
		ModeIndices[SelectedMode] < adapters[SelectedAdapter].DisplayModes.Size()
	);

	if (!InBound)
		throw ("Internal error in GRAPHICMANAGER::BeginDialogMode()!");
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GRAPHICMANAGER::SwitchToWindowed(const RECT& rect)
{
	DIRECTDRAW::SwitchToWindowed( rect );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GRAPHICMANAGER::SwitchToFullScreen()
{
	RECT rect;
	SetScreenSize( SelectedFactor, rect );

	CheckModeBounds();
	const DISPLAYMODE& mode = adapters[SelectedAdapter].DisplayModes[ModeIndices[SelectedMode]];

	DIRECTDRAW::SwitchToFullScreen( mode.width, mode.height, mode.bpp, rect );	
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GRAPHICMANAGER::SetScreenSize(const SCREENTYPE type)
{
	RECT rect;
	SelectedFactor = type;
	SetScreenSize( type, rect );
	UpdateScreenRect( rect );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GRAPHICMANAGER::SetScreenSize(const SCREENTYPE type,RECT& rect) const
{
	CheckModeBounds();

	SetScreenSize
	( 
     	type, 
		rect, 
		adapters[SelectedAdapter].DisplayModes[ModeIndices[SelectedMode]].width,
		adapters[SelectedAdapter].DisplayModes[ModeIndices[SelectedMode]].height
	);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID GRAPHICMANAGER::SetScreenSize(const SCREENTYPE type,RECT& rect,const UINT width,const UINT height) const
{
	CheckModeBounds();

	if (type == SCREEN_STRETCHED)
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
		const RECT& NesRect = (nes.IsPAL() ? rcPal : rcNtsc);

		UINT x = (NesRect.right+1) - NesRect.left;
		UINT y = (NesRect.bottom+1) - NesRect.top;

		const UINT factor = UINT(type);

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
	mbstowcs( wString, filename.String(), filename.Length() + 1 );

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

