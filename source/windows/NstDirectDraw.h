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

#ifndef NST_DIRECTDRAW_H
#define NST_DIRECTDRAW_H

#undef DIRECTDRAW_VERSION
#define DIRECTDRAW_VERSION 0x0700

#include <ddraw.h>
#include "../paradox/PdxArray.h"
#include "../NstNes.h"

////////////////////////////////////////////////////////////////////////////////////////
// Direct Draw wrapper
////////////////////////////////////////////////////////////////////////////////////////

class DIRECTDRAW
{
public:

	VOID Present();
	VOID Repaint();
	BOOL DoVSync();

	PDXRESULT Lock(NES::IO::GFX&);
	PDXRESULT Unlock();
	PDXRESULT EnableGDI(const BOOL);
	PDXRESULT TryClearScreen();

	inline UINT IsGDI() const
	{ return GDIMode; }
	
	inline UINT GetPixel(const UINT x,const UINT y) const
	{
		PDX_ASSERT(((y * NES::IO::GFX::WIDTH) + x) < (NES::IO::GFX::WIDTH * NES::IO::GFX::HEIGHT));		
		return PixelBuffer[(y * NES::IO::GFX::WIDTH) + x];
	}

	VOID Print
	(
      	const CHAR* const,
		const UINT,
		const UINT,
		const ULONG,
		const UINT=0
	);

	inline VOID ClearScreen()
	{ ClearSurface(windowed ? NesBuffer : BackBuffer); }

	inline VOID ClearNesScreen()
	{ ClearSurface( NesBuffer ); }

	inline BOOL IsReady() const
	{ return ready; }

	inline BOOL IsWindowed() const
	{ return windowed; }

	VOID UpdateScreenRect(const RECT& rect)
	{
		PDX_ASSERT(windowed);
		ScreenRect = rect; 
	}

	inline LPDIRECTDRAW7 GetDevice()
	{ return device; }

	inline const RECT& GetScreenRect() const
	{ return ScreenRect; }

	inline UINT GetDisplayWidth() const
	{ return DisplayMode.width; }

	inline UINT GetDisplayHeight() const
	{ return DisplayMode.height; }

	inline BOOL CanRenderWindowed() const
	{ return caps.dwCaps2 & DDCAPS2_CANRENDERWINDOWED; }

protected:

	DIRECTDRAW();
	~DIRECTDRAW();

	VOID SetRefreshRate(const UINT);

	PDXRESULT Initialize(HWND,const UINT=DEFAULT_REFRESH_RATE);
	PDXRESULT Destroy();
	PDXRESULT SwitchToFullScreen(const UINT,const UINT,const UINT);
	PDXRESULT SwitchToWindowed(const RECT&);
	PDXRESULT Create(GUID* const);
	PDXRESULT DrawNesBuffer();
	PDXRESULT ValidateSurface(LPDIRECTDRAWSURFACE7) const;

	PDXRESULT SwitchToWindowed()
	{ return SwitchToWindowed( ScreenRect ); }

	enum SCREENMODE
	{
		SCREENMODE_NORMAL,
		SCREENMODE_MATCHED,
		SCREENMODE_STRETCHED
	};

	enum SCREENEFFECT
	{
		SCREENEFFECT_NONE,
		SCREENEFFECT_SCANLINES
	};

	VOID ReleaseBuffers();

	PDXRESULT LockNesBuffer(DDSURFACEDESC2&);
	PDXRESULT UnlockNesBuffer();

	PDX_NO_INLINE PDXRESULT SetScreenParameters
	(
    	const SCREENMODE,
		const SCREENEFFECT,
		const BOOL,
		const RECT&,
		const BOOL
	);

	enum {DEFAULT_REFRESH_RATE=NES_FPS_NTSC};

	LPDIRECTDRAWSURFACE7 GetNesBuffer() const
	{ return NesBuffer; }

	HWND hWnd;

private:

	static PDXRESULT Error(const CHAR* const);

	static HRESULT CALLBACK EnumDisplayModes(LPDDSURFACEDESC2,LPVOID);
	static BOOL WINAPI EnumAdapters(LPGUID,LPSTR,LPSTR,LPVOID,HMONITOR);

	PDX_NO_INLINE PDXRESULT GetCaps();
	PDX_NO_INLINE PDXRESULT CreateScreenBuffers();
	PDX_NO_INLINE PDXRESULT CreateNesBuffer();
	PDX_NO_INLINE PDXRESULT CreateClipper();
	PDX_NO_INLINE PDXRESULT UpdateRectangles();
	
	PDXRESULT ClearSurface(LPDIRECTDRAWSURFACE7) const;
	
	static PDX_NO_INLINE PDXRESULT GetSurfaceDesc(LPDIRECTDRAWSURFACE7,DDSURFACEDESC2&);

	template<class T> PDX_NO_INLINE VOID BltNesScreen(T* const,const LONG);
	template<class T> PDX_NO_INLINE VOID BltNesScreenUnaligned(T*,const LONG);
	template<class T> PDX_NO_INLINE VOID BltNesScreenScanLines(T*,const LONG);

	template<class T> VOID BltNesScreenAligned(T* const);

	VOID DrawWindowText();

	LPDIRECTDRAW7 device;

	LPDIRECTDRAWSURFACE7 FrontBuffer;
	LPDIRECTDRAWSURFACE7 BackBuffer;
	LPDIRECTDRAWSURFACE7 NesBuffer;

private:

	LPDIRECTDRAWCLIPPER clipper;	
	
	RECT NesRect;
	RECT NesBltRect;
	RECT ScreenRect;

	BOOL windowed;
	BOOL ready;
	UINT RefreshRate;
	UINT SelectedDevice;
	UINT GDIMode;
	BOOL DontFlip;
	UINT ScaleFactor;
	BOOL PaletteChanged;
	BOOL UseVRam;
	BOOL UseVSync;

	SCREENMODE ScreenMode;
	SCREENEFFECT ScreenEffect;

	DDCAPS caps;

	NES::IO::GFX::PIXEL* const PixelBuffer;

	U32 palette[NES::IO::GFX::PALETTE_LENGTH][3];

	DDSURFACEDESC2 FrontDesc;
	DDSURFACEDESC2 BackDesc;
	DDSURFACEDESC2 NesDesc;

protected:

	inline const DDSURFACEDESC2& GetNesDesc() const
	{ return NesDesc; }

	struct DISPLAYMODE
	{
		BOOL operator < (const DISPLAYMODE&) const;
		BOOL operator == (const DISPLAYMODE&) const;
		
		VOID SetPixelFormat(const DWORD,const DWORD,const DWORD);

		UINT width;
		UINT height;
		UINT bpp;
		UINT RefreshRate;
		UINT rMask;
		UINT gMask;
		UINT bMask;

		union
		{
			struct  
			{
				ULONG AnyShiftLeft;
			};

			struct  
			{
				U8 rgbShiftLeft[3];
			};

			struct 
			{
				U8 rShiftLeft;
				U8 gShiftLeft;
				U8 bShiftLeft;
			};
		};

		union
		{
			struct  
			{
				ULONG AnyShiftRight;
			};

			struct  
			{
				U8 rgbShiftRight[3];
			};

			struct 
			{
				U8 rShiftRight;
				U8 gShiftRight;
				U8 bShiftRight;
			};
		};
	};

	typedef PDXARRAY<DISPLAYMODE> DISPLAYMODES;

	struct ADAPTER
	{
		GUID guid;
		PDXSTRING name;
		DISPLAYMODES DisplayModes;
	};

	typedef PDXARRAY<ADAPTER> ADAPTERS;

	ADAPTERS adapters;

private:

	DISPLAYMODE DisplayMode;
};

#endif

