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

#pragma once

#ifndef NST_GRAPHICMANAGER_H
#define NST_GRAPHICMANAGER_H

#include "NstManager.h"
#include "NstDirectDraw.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

class GRAPHICMANAGER : public DIRECTDRAW, public MANAGER
{
public:

	GRAPHICMANAGER();
	~GRAPHICMANAGER();

	VOID Create  (CONFIGFILE* const);
	VOID Destroy (CONFIGFILE* const);

	enum SCREENTYPE
	{
		SCREEN_FACTOR_1X,
		SCREEN_FACTOR_2X,
		SCREEN_FACTOR_3X,
		SCREEN_FACTOR_4X,
		SCREEN_STRETCHED
	};

	VOID SwitchToWindowed(const RECT&);
	BOOL TestCooperativeLevel();

	PDX_NO_INLINE VOID SwitchToFullScreen();
	PDX_NO_INLINE VOID BeginDialogMode();
	PDX_NO_INLINE VOID EndDialogMode();
	PDX_NO_INLINE VOID LoadPalette(const PDXSTRING&);
	PDX_NO_INLINE VOID SetScreenSize(const SCREENTYPE);
	PDX_NO_INLINE VOID UpdateDirectDraw();

	VOID EnableGDI(const BOOL);

	VOID DisplayMsg (const PDXSTRING* const);
	VOID DisplayNsf (const PDXSTRING&,const PDXSTRING&,const PDXSTRING&,const PDXSTRING&);
	VOID DisplayFPS (DOUBLE);

	PDX_NO_INLINE PDXRESULT SaveScreenShot();

	BOOL CanExportBitmaps() const;
	BOOL IsCustomPalette()  const;

	const PDXSTRING& GetPaletteFile() const;

private:

	PDX_NO_INLINE VOID CreateDevice(GUID);	
	PDX_NO_INLINE VOID ResetBpp();
	PDX_NO_INLINE VOID EnableBpp();
	PDX_NO_INLINE VOID SetScreenSize(const SCREENTYPE,RECT&) const;
	PDX_NO_INLINE VOID SetScreenSize(const SCREENTYPE,RECT&,const UINT,const UINT) const;

	PDXRESULT ExportBitmap(const PDXSTRING&);

	BOOL DialogProc(HWND,UINT,WPARAM,LPARAM);

	PDX_NO_INLINE VOID Reset();
	PDX_NO_INLINE VOID UpdateDialog();
	PDX_NO_INLINE VOID UpdateDevice();
	PDX_NO_INLINE VOID UpdateMode();
	PDX_NO_INLINE VOID UpdateBpp();
	PDX_NO_INLINE VOID UpdateNesScreen(const WPARAM);
	PDX_NO_INLINE VOID UpdatePalette();
	PDX_NO_INLINE BOOL ImportPalette();
	PDX_NO_INLINE VOID ResetColors();
	PDX_NO_INLINE BOOL InitModes(const UINT,const UINT,const UINT);

	VOID UpdateOffScreen();
	VOID UpdateColors();
	VOID UpdateEffects();
	VOID BrowsePalette();
	VOID ClearPalette();
	
	PDX_NO_INLINE VOID CheckModeBounds() const;

	HWND hDlg;

	UINT SelectedAdapter;
	UINT SelectedMode;
	UINT SelectedBpp;
	UINT SelectedOffScreen;
	UINT SelectedEffect;
	UINT SelectedPalette;	
	SCREENTYPE SelectedFactor;

	BOOL Support8Bpp;
	BOOL Support16Bpp;
	BOOL Support32Bpp;

	RECT rcNtsc;
	RECT rcPal;

	PDXSTRING PaletteFile;
	
	enum {PALETTE_LENGTH = 64 * 3};

	NES::IO::GFX::CONTEXT context;
	
	PDXARRAY<UINT> ModeIndices;

	U8* const palette;

	static BOOL HasGDIPlus();

	static const BOOL GdiPlusAvailable;
};

#include "NstGraphicManager.inl"

#endif
