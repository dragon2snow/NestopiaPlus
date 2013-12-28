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

	enum
	{
		NES_WIDTH            = NES::IO::GFX::WIDTH,
		NES_HEIGHT           = NES::IO::GFX::HEIGHT,
		NES_WIDTH_2          = NES_WIDTH * 2,
		NES_HEIGHT_2         = NES_HEIGHT * 2,
		PALETTE_LENGTH       = NES::IO::GFX::PALETTE_LENGTH,
		PIXEL_BUFFER_LENGTH  = NES_WIDTH * NES_HEIGHT,
		EFFECT_BUFFER_LENGTH = NES_WIDTH * NES_HEIGHT,
		CONV_BUFFER_LENGTH   = NES_WIDTH_2 * NES_HEIGHT_2
	};

	VOID Present();
	VOID Repaint();
	BOOL DoVSync();

	PDXRESULT Lock(NES::IO::GFX&);
	PDXRESULT Unlock();
	PDXRESULT EnableGDI(const BOOL);
	PDXRESULT TryClearScreen();

	VOID UpdateScreenRect(const RECT&,const BOOL=FALSE);

	BOOL IsGDI()      const;
	BOOL IsReady()    const;
	BOOL IsWindowed() const;

	UINT GetPixel(const UINT,const UINT) const;

	VOID Print(const CHAR* const,const UINT,const UINT,const ULONG,const UINT=0);

	VOID ClearScreen();
	VOID ClearNesScreen();

	LPDIRECTDRAW7 GetDevice();
	
	UINT GetScaleFactor() const;
	
	const RECT& GetScreenRect() const;
	const RECT& GetNesRect()    const;
	
	UINT GetDisplayWidth()  const;
	UINT GetDisplayHeight() const;

	BOOL CanRenderWindowed() const;

	const DDSURFACEDESC2& GetNesDesc() const;

protected:

	DIRECTDRAW();
	~DIRECTDRAW();

	PDX_NO_INLINE VOID SetRefreshRate(const UINT);

	PDX_NO_INLINE PDXRESULT Initialize(HWND,const UINT=DEFAULT_REFRESH_RATE);
	PDX_NO_INLINE PDXRESULT Destroy();
	PDX_NO_INLINE PDXRESULT SwitchToFullScreen(const UINT,const UINT,const UINT,const RECT* const=NULL);
	PDX_NO_INLINE PDXRESULT SwitchToWindowed(const RECT&);
	PDX_NO_INLINE PDXRESULT Create(GUID* const);
	PDX_NO_INLINE PDXRESULT ValidateSurface(LPDIRECTDRAWSURFACE7) const;

	PDXRESULT DrawNesBuffer();

	enum SCREENEFFECT
	{
		SCREENEFFECT_NONE,
		SCREENEFFECT_SCANLINES,
		SCREENEFFECT_2XSAI,
		SCREENEFFECT_SUPER_2XSAI,
		SCREENEFFECT_SUPER_EAGLE
	};

	VOID ReleaseBuffers();

	PDXRESULT LockNesBuffer(DDSURFACEDESC2&);
	PDXRESULT UnlockNesBuffer();

	PDX_NO_INLINE PDXRESULT SetScreenParameters
	(
		const SCREENEFFECT,
		const BOOL,
		const RECT&,
		const BOOL
	);

	enum {DEFAULT_REFRESH_RATE=NES_FPS_NTSC};

	LPDIRECTDRAWSURFACE7 GetNesBuffer() const;

	HWND hWnd;

private:

	PDXRESULT Error(const CHAR* const);

	static HRESULT CALLBACK EnumDisplayModes(LPDDSURFACEDESC2,LPVOID);
	static BOOL WINAPI EnumAdapters(LPGUID,LPSTR,LPSTR,LPVOID,HMONITOR);

	VOID UpdatePalette(const U8* const);

	PDX_NO_INLINE PDXRESULT GetCaps();
	PDX_NO_INLINE PDXRESULT CreateScreenBuffers();
	PDX_NO_INLINE PDXRESULT CreateNesBuffer();
	PDX_NO_INLINE PDXRESULT CreateClipper();
	
	BOOL CanBltFast() const;
	DWORD GetFlipFlags() const;

	PDXRESULT ClearSurface(LPDIRECTDRAWSURFACE7);
	
	PDX_NO_INLINE PDXRESULT GetSurfaceDesc(LPDIRECTDRAWSURFACE7,DDSURFACEDESC2&);

	typedef VOID (*F2XAI)(U8*,U32,U8*,U8*,U32,INT,INT);

	template<class T> PDX_NO_INLINE VOID BltNesScreen(T* const,const LONG);
	template<class T> VOID BltNesScreenAligned(T* const);
	template<class T> PDX_NO_INLINE VOID BltNesScreenUnaligned(T*,const LONG);
	template<class T> PDX_NO_INLINE VOID BltNesScreenScanLines(T*,const LONG);
	template<class T> PDX_NO_INLINE VOID BltNesScreen2xSaI(F2XAI,T*,const LONG);

	VOID DrawWindowText();

	DDCAPS caps;

	LPDIRECTDRAW7 device;
	LPDIRECTDRAWSURFACE7 FrontBuffer;
	LPDIRECTDRAWSURFACE7 BackBuffer;
	LPDIRECTDRAWSURFACE7 NesBuffer;
	LPDIRECTDRAWCLIPPER clipper;	

	SCREENEFFECT ScreenEffect;

	RECT NesRect;
	RECT NesBltRect;
	RECT ScreenRect;

	BOOL  windowed;
	BOOL  ready;
	UINT  RefreshRate;
	UINT  SelectedDevice;
	UINT  GDIMode;
	BOOL  DontFlip;
	UINT  ScaleFactor;
	BOOL  PaletteChanged;
	BOOL  UseVRam;
	BOOL  UseVSync;
	BOOL  Use2xSaI;
	BOOL  Use2xSaI565;
	BOOL  IsNesBuffer2xSaI;
	BOOL  ShouldBltFast;
	DWORD FlipFlags;
	BOOL  BltFailed;
	
	DDSURFACEDESC2 FrontDesc;
	DDSURFACEDESC2 BackDesc;
	DDSURFACEDESC2 NesDesc;

protected:

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
				UINT rgbShiftLeft[3];
			};

			struct 
			{
				UINT rShiftLeft;
				UINT gShiftLeft;
				UINT bShiftLeft;
			};
		};

		union
		{
			struct  
			{
				UINT rgbShiftRight[3];
			};

			struct 
			{
				UINT rShiftRight;
				UINT gShiftRight;
				UINT bShiftRight;
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

	U32 palette[NES::IO::GFX::PALETTE_LENGTH][3];

	NES::IO::GFX::PIXEL PixelBuffer[PIXEL_BUFFER_LENGTH];

	U16 EffectBuffer[EFFECT_BUFFER_LENGTH];
	U16 ConvBuffer[CONV_BUFFER_LENGTH];
};

#include "NstDirectDraw.inl"

#endif

