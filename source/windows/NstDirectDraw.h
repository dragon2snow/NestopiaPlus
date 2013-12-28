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
		NES_WIDTH                = NES::IO::GFX::WIDTH,
		NES_HEIGHT               = NES::IO::GFX::HEIGHT,
		NES_WIDTH_2              = NES_WIDTH * 2,
		NES_HEIGHT_2             = NES_HEIGHT * 2,
		PALETTE_LENGTH           = NES::IO::GFX::PALETTE_LENGTH,
		PIXEL_BUFFER_LENGTH      = NES_WIDTH * NES_HEIGHT,
		REAL_PIXEL_BUFFER_LENGTH = NES_WIDTH * (NES_HEIGHT+1),
		EFFECT_BUFFER_LENGTH     = NES_WIDTH * NES_HEIGHT,
		CONV_BUFFER_LENGTH       = NES_WIDTH_2 * NES_HEIGHT_2
	};

	BOOL Present();
	PDX_NO_INLINE VOID Repaint();

	NES::IO::GFX* GetFormat();

	PDXRESULT Lock(NES::IO::GFX&);
	PDXRESULT Unlock() throw(const CHAR*);

	BOOL DoVSync();

	PDX_NO_INLINE VOID EnableGDI(const BOOL) throw(const CHAR*);
	PDX_NO_INLINE VOID UpdateScreenRect(const RECT&,const BOOL=FALSE);

	BOOL TryClearScreen();
	BOOL UpdateRefresh(const BOOL,const UINT);

	BOOL IsGDI()      const;
	BOOL IsReady()    const;
	BOOL IsWindowed() const;

	UINT GetPixel(const UINT,const UINT) const;

	VOID Print(const CHAR* const,const UINT,const UINT,const ULONG,const UINT=0);

	VOID ClearScreen();
	VOID ClearNesScreen();

	LPDIRECTDRAW7 GetDevice();
	
	UINT GetScaleFactor() const;
	
	const RECT& GetNesRect() const;
	const RECT& GetScreenRect() const;
	
	UINT GetDisplayWidth()  const;
	UINT GetDisplayHeight() const;
	UINT GetDisplayBPP()    const;

	BOOL CanRenderWindowed()   const;
	BOOL Is8BitModeSupported() const;
	BOOL CanFlipNoVSync()      const;

	const DDSURFACEDESC2& GetNesDesc() const;

protected:

	DIRECTDRAW();				 
	~DIRECTDRAW();

	PDX_NO_INLINE VOID Initialize(HWND) throw(const CHAR*);
	PDX_NO_INLINE VOID Destroy();
	PDX_NO_INLINE VOID SwitchToFullScreen(const UINT,const UINT,const UINT,const RECT&) throw(const CHAR*);
	PDX_NO_INLINE VOID SwitchToWindowed(const RECT&,const BOOL=FALSE) throw(const CHAR*);
	PDX_NO_INLINE BOOL Create(const GUID&,const BOOL=FALSE) throw(const CHAR*);
	PDX_NO_INLINE VOID CreateClipper() throw(const CHAR*);

	const DDCAPS& GetHalCaps() const;
	const DDCAPS& GetHelCaps() const;

	DWORD GetMonitorFrequency() const;

	VOID DrawNesBuffer();

	enum SCREENEFFECT
	{
		SCREENEFFECT_NONE,
		SCREENEFFECT_SCANLINES,
		SCREENEFFECT_TV,
		SCREENEFFECT_2XSAI,
		SCREENEFFECT_SUPER_2XSAI,
		SCREENEFFECT_SUPER_EAGLE
	};

	VOID ReleaseBuffers();
	BOOL LockNesBuffer(DDSURFACEDESC2&) throw(const CHAR*);

	PDX_NO_INLINE VOID SetScreenParameters(const SCREENEFFECT,const BOOL,const RECT&,const RECT* const=NULL);

	LPDIRECTDRAWSURFACE7 GetNesBuffer() const;

	HWND hWnd;

private:

	static HRESULT CALLBACK EnumDisplayModes(LPDDSURFACEDESC2,LPVOID);
	static BOOL WINAPI EnumAdapters(LPGUID,LPSTR,LPSTR,LPVOID,HMONITOR);

	PDX_NO_INLINE BOOL Validate(const HRESULT=DD_OK);
	PDX_NO_INLINE VOID UpdatePalette(const U8* const);
	PDX_NO_INLINE VOID CreateScreenBuffers() throw(const CHAR*);
	PDX_NO_INLINE VOID CreateNesBuffer() throw(const CHAR*);

	VOID Wait();
	VOID UpdateNesBuffer();
	VOID ClearSurface(LPDIRECTDRAWSURFACE7,const DWORD=0);
	VOID DrawWindowText();
	
	typedef VOID (*F2XAI)(U8*,U32,U8*,U8*,U32,INT,INT);

	template<class T> VOID BltNesScreenAligned(T* const);
	template<class T> PDX_NO_INLINE VOID BltNesScreen(T* const,const LONG);
	PDX_NO_INLINE VOID BltNesScreen(U8*,const LONG);
	template<class T> PDX_NO_INLINE VOID BltNesScreenUnaligned(T*,const LONG);
	template<class T> PDX_NO_INLINE VOID BltNesScreenScanLines1(T*,const LONG);
	template<class T> PDX_NO_INLINE VOID BltNesScreenScanLinesFactor(T*,const LONG);
	template<class T> PDX_NO_INLINE VOID BltNesScreen2xSaI(F2XAI,T*,const LONG);
	template<class T> PDX_NO_INLINE VOID BltNesScreenTV(T*,const LONG);

	DDCAPS HelCaps;
	DDCAPS HalCaps;

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
	UINT  SelectedDevice;
	UINT  GDIMode;
	BOOL  DontFlip;
	DWORD FlipFlags;
	UINT  ScaleFactor;
	BOOL  PaletteChanged;
	BOOL  UseVRam;
	BOOL  Use2xSaI;
	BOOL  Use2xSaI565;
	BOOL  IsNesBuffer2xSaI;
	UINT  FrameLatency;
	
	DDSURFACEDESC2 FrontDesc;
	DDSURFACEDESC2 BackDesc;
	DDSURFACEDESC2 NesDesc;

protected:

	struct DISPLAYMODE
	{
		DISPLAYMODE()
		: 
		width       (0),
		height      (0),
		bpp         (0),
		RefreshRate (0),
		rMask       (0),
		gMask       (0),
		bMask       (0),
		rShiftLeft  (0),
		gShiftLeft  (0),
		bShiftLeft  (0),
		rShiftRight (0),
		gShiftRight (0),
		bShiftRight (0)
		{}

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
		ADAPTER()
		{
			PDXMemZero(guid);
		}

		GUID guid;
		PDXSTRING name;
		DISPLAYMODES DisplayModes;
	};

	typedef PDXARRAY<ADAPTER> ADAPTERS;

	ADAPTERS adapters;

private:

	enum {MAX_FRAME_LATENCY = 3};

	BOOL CheckReady() const;

	DISPLAYMODE DisplayMode;

	GUID guid;

	NES::IO::GFX format;

	struct PIXELDATA
	{
		PIXELDATA();

		PDX_NO_INLINE VOID Reset();

		U32 palette[NES::IO::GFX::PALETTE_LENGTH][3];

		NES::IO::GFX::PIXEL buffer[PIXEL_BUFFER_LENGTH];

		U16 effect[EFFECT_BUFFER_LENGTH];
		U16 conv[CONV_BUFFER_LENGTH];
	};

	PIXELDATA* const PixelData;
};

#include "NstDirectDraw.inl"

#endif

