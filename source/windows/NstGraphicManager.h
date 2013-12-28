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

#include "NstDirectX.h"
#include "NstManager.h"
#include "resource/resource.h"

class PDXFILE;

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

class GRAPHICMANAGER : public DIRECTDRAW, public MANAGER
{
public:

	GRAPHICMANAGER(const INT,const UINT);

	enum SCREENTYPE
	{
		SCREEN_FACTOR_1X,
		SCREEN_FACTOR_2X,
		SCREEN_FACTOR_3X,
		SCREEN_FACTOR_4X,
		SCREEN_STRETCHED
	};

	PDXRESULT SwitchToFullScreen(const SCREENTYPE);
	PDXRESULT SwitchToWindowed(const RECT&);
	PDXRESULT BeginDialogMode();
	PDXRESULT EndDialogMode();
	PDXRESULT SaveScreenShot();
	PDXRESULT LoadPalette(const PDXSTRING&);
	PDXRESULT SetScreenSize(const SCREENTYPE);

	inline UINT GetSelectedDisplayWidth()  const { return adapters[SelectedAdapter].DisplayModes[ModeIndices[SelectedMode]].width;  }
	inline UINT GetSelectedDisplayHeight() const { return adapters[SelectedAdapter].DisplayModes[ModeIndices[SelectedMode]].height; }

	inline BOOL CanExportBitmaps() const
	{ return GdiPlusAvailable; }

	inline BOOL AutoFrameSkip() const
	{ return SelectedTiming == IDC_GRAPHICS_TIMING_FRAMESKIP; }

	VOID EnablePAL(const BOOL);

	inline NES::IO::GFX* GetFormat()
	{ return &format; }

	inline const RECT& GetRectNTSC() const { return ntsc; }
	inline const RECT& GetRectPAL()  const { return pal;  }

	inline BOOL IsCustomPalette() const
	{ return SelectedPalette == IDC_GRAPHICS_PALETTE_CUSTOM; }

	inline const PDXSTRING& GetPaletteFile() const
	{ return PaletteFile; }

private:

	VOID SetScreenSize(const SCREENTYPE,RECT&);

	PDXRESULT Create  (PDXFILE* const);
	PDXRESULT Destroy (PDXFILE* const);

	PDXRESULT ExportBitmap(const PDXSTRING&);

	BOOL DialogProc(HWND,UINT,WPARAM,LPARAM);

	VOID Reset();
	VOID ResetBpp();
	VOID EnableBpp();
	VOID UpdateDialog();
	VOID UpdateDevice();
	VOID UpdateMode();
	VOID UpdateBpp();
	VOID UpdateOffScreen();
	VOID UpdateNesScreen(const WPARAM);
	VOID UpdateEmulation();
	VOID UpdateColors();
	VOID UpdateEffects();
	VOID UpdateTiming();
	VOID UpdateDirectDraw();
	VOID UpdatePalette();
	VOID BrowsePalette();
	BOOL ImportPalette();
	VOID ClearPalette();
	VOID ResetColors();

	HWND hDlg;

	UINT SelectedAdapter;
	UINT SelectedMode;
	UINT SelectedBpp;
	UINT SelectedOffScreen;
	UINT SelectedTiming;
	UINT SelectedEffect;
	UINT SelectedPalette;

	INT PreviousMode;

	BOOL Support16Bpp;
	BOOL Support32Bpp;
	BOOL SupportSRam;

	RECT ntsc;
	RECT pal;
	RECT SaveRect;

	PDXSTRING PaletteFile;
	
	enum {PALETTE_LENGTH = 64 * 3};

	U8 palette[PALETTE_LENGTH];

	NES::IO::GFX format;
	NES::IO::GFX::CONTEXT context;
	
	PDXARRAY<UINT> ModeIndices;

	BOOL GdiPlusAvailable;
	BOOL ShouldBeFullscreen;

   #pragma pack(push,1)

	struct HEADER
	{
		enum BPP
		{
			BPP_16,
			BPP_32
		};

		enum OFFSCREEN
		{
			OFFSCREEN_SRAM,
			OFFSCREEN_VRAM
		};

		enum EFFECT
		{
			EFFECT_NONE,
			EFFECT_SCANLINES,
			EFFECT_2XSAI,
			EFFECT_SUPER_2XSAI,
			EFFECT_SUPER_EAGLE
		};

		enum PALETTE
		{
			PALETTE_EMULATED,
			PALETTE_INTERNAL,
			PALETTE_CUSTOM
		};

		enum TIMING
		{
			TIMING_VSYNC,
			TIMING_FRAMESKIP
		};

		GUID guid;
		U16  width;
		U16  height;
		RECT ntsc;
		RECT pal;
		U8   bpp : 1;
		U8   effect : 3;
		U8   offscreen : 1;
		U8   timing : 1;
		U8   InfiniteSprites : 1;
		U8   palette : 2;
		U8   brightness;
		U8   saturation;
		U8   hue;
	};

   #pragma pack(pop)
};

#endif
