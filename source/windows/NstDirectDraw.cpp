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

#include "../paradox/PdxQuickSort.h"
#include "NstDirectX.h"
#include "NstApplication.h"

#pragma comment(lib,"ddraw")

////////////////////////////////////////////////////////////////////////////////////////
// lock the nes surface
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT NES::IO::GFX::Lock()
{
	PDX_ASSERT(device);
	return PDX_CAST(DIRECTDRAW*,device)->Lock(*this);
}

////////////////////////////////////////////////////////////////////////////////////////
// return the current pixel for the given coordinates
////////////////////////////////////////////////////////////////////////////////////////

UINT NES::IO::GFX::GetPixel(const UINT x,const UINT y) const
{
	PDX_ASSERT(device);
	return PDX_CAST(DIRECTDRAW*,device)->GetPixel(x,y);
}

////////////////////////////////////////////////////////////////////////////////////////
// unlock the nes surface
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT NES::IO::GFX::Unlock()
{
	PDX_ASSERT(device);
	return PDX_CAST(DIRECTDRAW*,device)->Unlock();
}

////////////////////////////////////////////////////////////////////////////////////////
// enumerate the devices
////////////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DIRECTDRAW::EnumAdapters(LPGUID guid,LPSTR desc,LPSTR,LPVOID context,HMONITOR)
{
	if (!context)
		return DDENUMRET_OK;

	LPDIRECTDRAW7 device;

	if (FAILED(DirectDrawCreateEx(guid,PDX_CAST_PTR(LPVOID,device),IID_IDirectDraw7,NULL)))
		return DDENUMRET_OK;

	ADAPTERS& adapters = *PDX_CAST(ADAPTERS*,context);
	
	adapters.Grow();

	ADAPTER& adapter = adapters.Back();

	if (guid) memcpy( &adapter.guid, guid, sizeof(adapter.guid) );
	else      memset( &adapter.guid, 0x00, sizeof(adapter.guid) );

	adapter.name = desc;

	if (adapter.name.IsEmpty())
		adapter.name = "unknown";
	
	DISPLAYMODES& modes = adapter.DisplayModes;

	device->EnumDisplayModes(0,NULL,PDX_CAST(LPVOID,&modes),EnumDisplayModes);
	device->Release();

	if (modes.IsEmpty())
	{
		adapters.EraseBack();
		return DDENUMRET_OK;
	}

	PDXQUICKSORT::Sort( modes.Begin(), modes.End() );

	return DDENUMRET_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
// enumerate the display modes
////////////////////////////////////////////////////////////////////////////////////////

HRESULT CALLBACK DIRECTDRAW::EnumDisplayModes(LPDDSURFACEDESC2 desc,LPVOID context)
{
	if (!desc || !context)
		return DDENUMRET_OK;

	if (desc->dwWidth < NES::IO::GFX::WIDTH || desc->dwHeight < NES::IO::GFX::HEIGHT)
		return DDENUMRET_OK;

	if (!(desc->ddpfPixelFormat.dwFlags & DDPF_RGB))
		return DDENUMRET_OK;

	if (desc->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8)
		return DDENUMRET_OK;

	if (desc->ddpfPixelFormat.dwRGBBitCount != 16 && desc->ddpfPixelFormat.dwRGBBitCount != 32)
		return DDENUMRET_OK;

	DISPLAYMODE mode;

	mode.width       = desc->dwWidth;
	mode.height	     = desc->dwHeight;
	mode.RefreshRate = 0;
	mode.bpp         = desc->ddpfPixelFormat.dwRGBBitCount;

	mode.SetPixelFormat
	(
       	desc->ddpfPixelFormat.dwRBitMask,
		desc->ddpfPixelFormat.dwGBitMask,
		desc->ddpfPixelFormat.dwBBitMask
	);

	PDX_CAST(DISPLAYMODES*,context)->InsertBack(mode);

	return DDENUMRET_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
// constructor
////////////////////////////////////////////////////////////////////////////////////////

DIRECTDRAW::DIRECTDRAW()
: 
hWnd           (NULL),
device         (NULL),
SelectedDevice (0),
FrontBuffer    (NULL),					
BackBuffer     (NULL),
NesBuffer      (NULL),
clipper        (NULL),
GDIMode        (0),
windowed       (TRUE),
UseVRam        (FALSE),
ScaleFactor    (0),
DontFlip       (FALSE),
UseVSync       (TRUE),
PaletteChanged (TRUE),
ready          (FALSE),
ScreenEffect   (SCREENEFFECT_NONE),
PixelBuffer    (new NES::IO::GFX::PIXEL[NES::IO::GFX::WIDTH * NES::IO::GFX::HEIGHT]),
RefreshRate    (DEFAULT_REFRESH_RATE)
{
	SetRect( &NesRect, 0, 0, NES::IO::GFX::WIDTH, NES::IO::GFX::HEIGHT );
	SetRect( &ScreenRect, 0, 0, NES::IO::GFX::WIDTH, NES::IO::GFX::HEIGHT );

	PDXMemZero( PixelBuffer, NES::IO::GFX::WIDTH * NES::IO::GFX::HEIGHT );
	PDXMemZero( DisplayMode );
	PDXMemZero( caps );
}

////////////////////////////////////////////////////////////////////////////////////////
// destructor
////////////////////////////////////////////////////////////////////////////////////////

DIRECTDRAW::~DIRECTDRAW()
{
	Destroy();
	delete [] PixelBuffer;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTDRAW::Error(const CHAR* const msg)
{
	ready = FALSE;

	ReleaseBuffers();

	application.LogOutput(PDXSTRING("DIRECTDRAW: ") + msg);
	application.OnError( msg );

	return PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
// set the desired refresh rate and initialize the timer
////////////////////////////////////////////////////////////////////////////////////////

VOID DIRECTDRAW::SetRefreshRate(const UINT rate)
{
	PDX_ASSERT(rate == NES_FPS_NTSC || rate == NES_FPS_PAL);
	RefreshRate = rate;
}

////////////////////////////////////////////////////////////////////////////////////////
// update the parameters
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTDRAW::SetScreenParameters(const SCREENEFFECT effect,const BOOL vram,const RECT& rect,const BOOL vsync)
{
	PDX_ASSERT( rect.bottom - rect.top < NES::IO::GFX::HEIGHT );
	PDX_ASSERT( rect.right - rect.left < NES::IO::GFX::WIDTH  );

	SetRect( &NesRect, rect.left, rect.top, rect.right+1, rect.bottom+1 );
	
	NesBltRect = NesRect;	
	ScreenEffect = effect;
	UseVSync = vsync;
	UseVRam = vram;

	return ready ? CreateNesBuffer() : PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID DIRECTDRAW::UpdateScreenRect(const RECT& rect)
{
	ScreenRect = rect;

	if (!windowed && ScreenEffect == SCREENEFFECT_SCANLINES)
		CreateNesBuffer();
}

////////////////////////////////////////////////////////////////////////////////////////
// main initializer, called once
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTDRAW::Initialize(HWND h,const UINT rate)
{
	PDX_ASSERT(h);

	ready = FALSE;
	hWnd = h;

	SetRefreshRate( rate );

	application.LogOutput("DIRECTDRAW: Initializing");

	adapters.Clear();

	if (FAILED(DirectDrawEnumerateEx(EnumAdapters,PDX_CAST(LPVOID,&adapters),DDENUM_ATTACHEDSECONDARYDEVICES)))
		return Error("DirectDrawEnumerateEx() failed!");

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
// create the device
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTDRAW::Create(GUID* const guid)
{
	PDX_ASSERT( guid );

	if (adapters.IsEmpty())
		return PDX_FAILURE;

	if (device && !memcmp( &adapters[SelectedDevice].guid, guid, sizeof(*guid) ))
		return PDX_OK;

	Destroy();

	if (FAILED(DirectDrawCreateEx(guid,PDX_CAST(LPVOID*,&device),IID_IDirectDraw7,NULL)))
		return PDX_FAILURE;

	PDX_TRY(GetCaps());

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
// create the front and back surfaces
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTDRAW::CreateScreenBuffers()
{
	PDX_ASSERT( device && !FrontBuffer && !BackBuffer );

	if (!device)
		return PDX_FAILURE;

	DIRECTX::Release( FrontBuffer );
	DIRECTX::Release( BackBuffer  );

	DIRECTX::InitStruct( FrontDesc );

	FrontDesc.dwFlags = DDSD_CAPS;
	FrontDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	if (!windowed)
	{
		FrontDesc.dwFlags |= DDSD_BACKBUFFERCOUNT;
		FrontDesc.ddsCaps.dwCaps |= DDSCAPS_FLIP|DDSCAPS_COMPLEX;
		FrontDesc.dwBackBufferCount = 2;

		if (caps.dwCaps & DDCAPS_BLTSTRETCH)
		{
			application.LogOutput("DIRECTDRAW: creating the primary surface in video memory");
			FrontDesc.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
		}
		else
		{
			application.LogOutput("DIRECTDRAW: creating the primary surface in system memory");
			FrontDesc.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
		}
	}
	else
	{
		application.LogOutput("DIRECTDRAW: creating the primary surface");
	}

	if (FAILED(device->CreateSurface(&FrontDesc,&FrontBuffer,NULL)))
	{
		if (!windowed)
		{
			application.LogOutput("DIRECTDRAW: primary surface creation failed, resolving to use just one backbuffer");
			FrontDesc.dwBackBufferCount = 1;

			if (FAILED(device->CreateSurface(&FrontDesc,&FrontBuffer,NULL)))
			{
				PDXSTRING log;
				log = "DIRECTDRAW: primary surface creation failed, trying with ";

				if (FrontDesc.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY)
				{
					log += "system memory";
					FrontDesc.ddsCaps.dwCaps &= ~DDSCAPS_VIDEOMEMORY;
					FrontDesc.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
				}
				else
				{
					log += "video memory";
					FrontDesc.ddsCaps.dwCaps &= ~DDSCAPS_SYSTEMMEMORY;
					FrontDesc.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
				}

				application.LogOutput( log );
			}
		}

		if (windowed || FAILED(device->CreateSurface(&FrontDesc,&FrontBuffer,NULL)))
			return Error("IDirectDraw7::CreateSurface() failed!");
	}

	PDX_TRY(GetSurfaceDesc(FrontBuffer,FrontDesc));

	if (!windowed)
	{
		DDSCAPS2 caps2;
		PDXMemZero( caps2 );
		
		caps2.dwCaps = DDSCAPS_BACKBUFFER;

		if (FAILED(FrontBuffer->GetAttachedSurface(&caps2,&BackBuffer)))
			return Error("IDirectDrawSurface7::GetAttachedBuffer() failed!");

		BackBuffer->AddRef();

		PDX_TRY(GetSurfaceDesc(BackBuffer,BackDesc));

		return ClearSurface(BackBuffer);
	}

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
// create the nes surface
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTDRAW::CreateNesBuffer()
{
	PDX_ASSERT( device );

	if (!device)
		return PDX_FAILURE;

	DIRECTX::Release(NesBuffer);
	DIRECTX::InitStruct(NesDesc);

	NesDesc.dwFlags         = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
	NesDesc.ddsCaps.dwCaps  = DDSCAPS_OFFSCREENPLAIN;
	NesDesc.ddsCaps.dwCaps |= UseVRam ? DDSCAPS_VIDEOMEMORY : DDSCAPS_SYSTEMMEMORY;
	NesDesc.dwWidth         = NES::IO::GFX::WIDTH;
	NesDesc.dwHeight        = NES::IO::GFX::HEIGHT;

	NesBltRect = NesRect;
	ScaleFactor = 0;

	if (!windowed && ScreenEffect == SCREENEFFECT_SCANLINES)
	{
		const UINT width = ScreenRect.right - ScreenRect.left;
		const UINT height = ScreenRect.bottom - ScreenRect.top;

		while ((NesBltRect.right-NesBltRect.left) * 2 <= width && (NesBltRect.bottom-NesBltRect.top) * 2 <= height)
		{
			NesDesc.dwWidth   *= 2;
			NesDesc.dwHeight  *= 2;

			NesBltRect.top    *= 2;			
			NesBltRect.right  *= 2;
			NesBltRect.bottom *= 2;
			NesBltRect.left   *= 2;

			++ScaleFactor;
		}
	}

	{
		PDXSTRING log;

		log  = "DIRECTDRAW: creating the nes render target surface in ";
		log += (UseVRam ? "video" : "system");
		log += " memory with BLT dimensions (";
		log += NesBltRect.left;
		log += ",";
		log += NesBltRect.top;
		log += ") - (";
		log += NesBltRect.right;
		log += ",";
		log += NesBltRect.bottom;
		log += ")";

		application.LogOutput( log );
	}

	if (FAILED(device->CreateSurface(&NesDesc,&NesBuffer,NULL)))
	{
		PDXSTRING log("DIRECTDRAW: nes render target surface creation failed, trying with ");

		if (NesDesc.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY)
		{
			log += "system memory";
			NesDesc.ddsCaps.dwCaps &= ~DDSCAPS_VIDEOMEMORY;
			NesDesc.ddsCaps.dwCaps |=  DDSCAPS_SYSTEMMEMORY;
		}
		else
		{
			log += "video memory";
			NesDesc.ddsCaps.dwCaps &= ~DDSCAPS_SYSTEMMEMORY;
			NesDesc.ddsCaps.dwCaps |=  DDSCAPS_VIDEOMEMORY;
		}

		application.LogOutput( log );

		if (FAILED(device->CreateSurface(&NesDesc,&NesBuffer,NULL)))
			return Error("IDirectDraw7::CreateSurface() failed!");
	}

	if (FAILED(GetSurfaceDesc(NesBuffer,NesDesc)))
		return PDX_FAILURE;

	return ClearSurface(NesBuffer);
}

////////////////////////////////////////////////////////////////////////////////////////
// switch to desktop mode
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTDRAW::SwitchToWindowed(const RECT& rect)
{
	PDX_ASSERT( device && hWnd );

	if (!device || !hWnd)
		return PDX_FAILURE;

	if (ready && windowed)
	{
		UpdateScreenRect( rect );
		return PDX_OK;
	}

	windowed = TRUE;
	ready = FALSE;
	PaletteChanged = TRUE;
	GDIMode = 0;

	ReleaseBuffers();

	if (FAILED(device->RestoreDisplayMode()))
		return Error("IDirectDraw7::RestoreDisplayMode() failed!");

	if (FAILED(device->SetCooperativeLevel(hWnd,DDSCL_NORMAL)))
		return Error("IDirectDraw7::SetCooperativeLevel() failed!");

	PDX_TRY(CreateScreenBuffers());
	PDX_TRY(CreateClipper());

	if (FAILED(FrontBuffer->SetClipper(clipper)))
		return Error("IDirectDrawSurface7::SetClipper() failed!");

	if (FrontDesc.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8)
	{
		// very unlikely, but it might pop up if you have a disgusting desktop setup
		return Error("The current desktop screen bit depth is not supported!");
	}

	DisplayMode.width       = FrontDesc.dwWidth;
	DisplayMode.height      = FrontDesc.dwHeight;
	DisplayMode.bpp         = FrontDesc.ddpfPixelFormat.dwRGBBitCount;
	DisplayMode.RefreshRate	= 0;

	UpdateScreenRect( rect );

	{
		DWORD frequency;

		if (SUCCEEDED(device->GetMonitorFrequency(&frequency)))
			DisplayMode.RefreshRate	= frequency;
	}

	DisplayMode.SetPixelFormat
	(
   		FrontDesc.ddpfPixelFormat.dwRBitMask,
   		FrontDesc.ddpfPixelFormat.dwGBitMask,
   		FrontDesc.ddpfPixelFormat.dwBBitMask
	);

	PDX_TRY(CreateNesBuffer());

	ready = TRUE;

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
// switch to fullscreen mode
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTDRAW::SwitchToFullScreen(const UINT width,const UINT height,const UINT bpp)
{
	PDX_ASSERT( device && hWnd );

	if (!device || !hWnd)
		return PDX_FAILURE;

	if (ready && !windowed && DisplayMode.width == width && DisplayMode.height == height && DisplayMode.bpp == bpp)
		return PDX_OK;

 	windowed = FALSE;
	ready = FALSE;
	PaletteChanged = TRUE;

	ReleaseBuffers();

	BOOL found = FALSE;

	for (UINT i=0; i < adapters[SelectedDevice].DisplayModes.Size(); ++i)
	{
		const DISPLAYMODE& mode = adapters[SelectedDevice].DisplayModes[i];

		if (width == mode.width && height == mode.height && bpp == mode.bpp)
		{
			found = TRUE;
			DisplayMode = mode;
			break;
		}
	}

	if (!found)
		return Error("Found no good display mode!");

	if (FAILED(device->SetCooperativeLevel(hWnd,DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN|DDSCL_ALLOWREBOOT)))
		return Error("IDirectDraw7::SetCooperativeLevel() failed!");

	{
		const UINT rate = RefreshRate <= 60 ? 60 : 0;

		if (FAILED(device->SetDisplayMode(DisplayMode.width,DisplayMode.height,DisplayMode.bpp,rate,0)))
		{
			if (FAILED(device->SetDisplayMode(DisplayMode.width,DisplayMode.height,DisplayMode.bpp,0,0)))
				return Error("IDirectDraw7::SetDisplayMode() failed!");

			DWORD f=0;
			DisplayMode.RefreshRate = (device->GetMonitorFrequency(&f) == DD_OK && f == 60) ? 60 : 0;
		}
		else
		{
			DisplayMode.RefreshRate = rate;
		}
	}

	PDX_TRY(CreateScreenBuffers());
	PDX_TRY(CreateClipper());

	if (GDIMode)
	{
		if (FAILED(device->FlipToGDISurface()))
			return Error("IDirectDraw7::FlipToGDISurface() failed!");

		if (FAILED(FrontBuffer->SetClipper(clipper)))
			return Error("IDirectDrawSurface7::SetClipper() failed!");
	}

	{
		const UINT x = ( DisplayMode.width  - NES::IO::GFX::WIDTH  ) / 2;
		const UINT y = ( DisplayMode.height - NES::IO::GFX::HEIGHT ) / 2;

		SetRect( &ScreenRect, x, y, DisplayMode.width - x, DisplayMode.height - y );
	}

	PDX_TRY(CreateNesBuffer());
	
	ready = TRUE;

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
// enable/disable GDI output in fullscreen mode
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTDRAW::EnableGDI(const BOOL state)
{
	if (windowed || !ready)
		return PDX_OK;

	PDX_ASSERT(device && FrontBuffer && clipper);

	if (!device || !FrontBuffer || !clipper)
		return PDX_FAILURE;

	if (state)
	{
		if (!GDIMode++)
		{
			Repaint();

			if (FAILED(FrontBuffer->SetClipper(clipper)))
				return Error("IDirectDrawSurface7::SetClipper() failed!");

			if (FAILED(device->FlipToGDISurface()))
				return Error("IDirectDraw7::FlipToGDISurface() failed!");
		}
	}
	else
	{
		if (GDIMode && !--GDIMode)
		{
			if (FAILED(FrontBuffer->SetClipper(NULL)))
				return Error("IDirectDrawSurface7::SetClipper() failed!");

			Repaint();
		}
	}

   #ifdef _DEBUG
	application.LogOutput(PDXSTRING("DIRECTDRAW: GDI mode reference count: ") + GDIMode);
   #endif

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
// create the clipper
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTDRAW::CreateClipper()
{
	PDX_ASSERT(device && FrontBuffer);

	DIRECTX::Release(clipper);

	if (FAILED(device->CreateClipper(0,&clipper,NULL)))
		return Error("IDirectDraw7::CreateClipper() failed!");

	if (FAILED(clipper->SetHWnd(0,hWnd)))
		return Error("IDirectDrawClipper::SetHWnd() failed!");

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTDRAW::TryClearScreen()
{
	LPDIRECTDRAWSURFACE7 surface = windowed ? NesBuffer : BackBuffer;
	PDX_ASSERT( surface );

	if (!surface || FAILED(surface->GetBltStatus(DDGBS_CANBLT)))
		return PDX_FAILURE;

	ClearScreen();

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
// clear any surface
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTDRAW::ClearSurface(LPDIRECTDRAWSURFACE7 surface) const
{
	PDX_TRY(ValidateSurface(surface));

	DDBLTFX BltFX;
	PDXMemZero( BltFX );

	BltFX.dwSize = sizeof(BltFX);
	BltFX.dwFillColor = 0;

	if (FAILED(surface->Blt(NULL,NULL,NULL,DDBLT_COLORFILL|DDBLT_WAIT|DDBLT_ASYNC,&BltFX)))
		surface->Blt(NULL,NULL,NULL,DDBLT_COLORFILL|DDBLT_WAIT,&BltFX);

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
// return the capabilities of the device
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTDRAW::GetCaps()
{
	DIRECTX::InitStruct(caps);

	if (FAILED(device->GetCaps(&caps,NULL)))
		return Error("IDirectDraw7::GetCaps() failed!");

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
// destroyer
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTDRAW::Destroy()
{
	ready = FALSE;

	PDXRESULT result = PDX_OK;

	ReleaseBuffers();

	if (device)
	{
		if (FAILED(device->SetCooperativeLevel(hWnd,DDSCL_NORMAL)))
			result = Error("IDirectDraw7::SetCooperativeLevel() failed!");

		if (DIRECTX::Release(device,TRUE))
			result = PDX_FAILURE;
	}

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////
// lock the nes surface
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTDRAW::Lock(NES::IO::GFX& PixelMap)
{
	PDX_ASSERT(ready);

	if (PixelMap.PaletteChanged || PaletteChanged)
	{
		PaletteChanged = FALSE;
		PixelMap.PaletteChanged = FALSE;

		U32* output = *palette;
		const U8* input = PixelMap.palette;

		for (UINT i=0; i < NES::IO::GFX::PALETTE_LENGTH; ++i)
		{
			*output++ = ( *input++ >> DisplayMode.rgbShiftRight[0] ) << DisplayMode.rgbShiftLeft[0];
			*output++ = ( *input++ >> DisplayMode.rgbShiftRight[1] ) << DisplayMode.rgbShiftLeft[1];
			*output++ = ( *input++ >> DisplayMode.rgbShiftRight[2] ) << DisplayMode.rgbShiftLeft[2];
		}
	}

	PixelMap.pixels = PixelBuffer;

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
// unlock the nes surface
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTDRAW::Unlock()
{
	PDX_ASSERT(ready);

	DDSURFACEDESC2 desc;

	PDX_TRY(LockNesBuffer(desc));

	switch (DisplayMode.bpp)
	{
     	case 16: BltNesScreen( PDX_CAST(U16*,desc.lpSurface), desc.lPitch / sizeof(U16) ); break;
       	default: BltNesScreen( PDX_CAST(U32*,desc.lpSurface), desc.lPitch / sizeof(U32) ); break;
	}

	if (FAILED(NesBuffer->Unlock(NULL)))
		return Error("IDirectDrawSurface7::Unlock() failed!");

	return windowed ? PDX_OK : DrawNesBuffer();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTDRAW::UnlockNesBuffer()
{
	PDX_ASSERT(NesBuffer && ready);
	return (!NesBuffer || FAILED(NesBuffer->Unlock(NULL))) ? PDX_FAILURE : PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
// blit to the nes surface 
////////////////////////////////////////////////////////////////////////////////////////

template<class T> 
VOID DIRECTDRAW::BltNesScreen(T* const dst,const LONG pitch)
{
	if (windowed || ScreenEffect != SCREENEFFECT_SCANLINES)
	{
		if (pitch == NES::IO::GFX::WIDTH)
		{
			BltNesScreenAligned( dst );
		}
		else
		{
			BltNesScreenUnaligned( dst, pitch );
		}
	}
	else
	{
		BltNesScreenScanLines( dst, pitch );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// blit to the nes surface 
////////////////////////////////////////////////////////////////////////////////////////

template<class T> 
VOID DIRECTDRAW::BltNesScreenAligned(T* const dst)
{
	for (UINT i=0; i < NES::IO::GFX::WIDTH * NES::IO::GFX::HEIGHT; ++i)
	{
		const U32* const rgb = palette[PixelBuffer[i]];
		dst[i] = T(rgb[0] + rgb[1] + rgb[2]);
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// blit to the nes surface 
////////////////////////////////////////////////////////////////////////////////////////

template<class T>
VOID DIRECTDRAW::BltNesScreenUnaligned(T* dst,const LONG pitch)
{
	const NES::IO::GFX::PIXEL* src = PixelBuffer;

	for (UINT y=0; y < NES::IO::GFX::HEIGHT; ++y)
	{
		for (UINT x=0; x < NES::IO::GFX::WIDTH; ++x)
		{
			const U32* const rgb = palette[src[x]];
			dst[x] = T(rgb[0] + rgb[1] + rgb[2]);
		}
		
		dst += pitch;
		src += NES::IO::GFX::WIDTH;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

template<class T> 
VOID DIRECTDRAW::BltNesScreenScanLines(T* dst,const LONG pitch)
{
	const UINT yScale = NES::IO::GFX::HEIGHT << ScaleFactor;
	const UINT xScale = NES::IO::GFX::WIDTH << ScaleFactor;
	const UINT ScanStep = PDX_MAX(1,ScaleFactor);

	const UINT rMask = DisplayMode.rMask;
	const UINT gMask = DisplayMode.gMask;
	const UINT bMask = DisplayMode.bMask;

	const NES::IO::GFX::PIXEL* const offset = PixelBuffer;

	for (UINT y=0; y < yScale; ++y)
	{
		const UINT yScan = y & ScanStep;

		const NES::IO::GFX::PIXEL* const src = offset + ((y >> ScaleFactor) * NES::IO::GFX::WIDTH);

		for (UINT x=0; x < xScale; ++x)
		{
			const U32* const rgb = palette[src[x >> ScaleFactor]];
			
			dst[x] = T
			(
				((rgb[0] >> yScan) & rMask) + 
				((rgb[1] >> yScan) & gMask) + 
				((rgb[2] >> yScan) & bMask)
			);     
  		}

		dst += pitch;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// lock the nes surface
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTDRAW::LockNesBuffer(DDSURFACEDESC2& desc)
{
	PDX_TRY(ValidateSurface(NesBuffer));

	desc.dwSize = sizeof(desc);

	if (FAILED(NesBuffer->Lock(NULL,&desc,DDLOCK_NOSYSLOCK|DDLOCK_WAIT|DDLOCK_WRITEONLY,NULL)))
		return Error("IDirectDrawSurface7::Lock() failed!");

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
// get the surface description
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTDRAW::GetSurfaceDesc(LPDIRECTDRAWSURFACE7 surface,DDSURFACEDESC2& desc)
{
	PDX_ASSERT(surface);

	DIRECTX::InitStruct(desc);

	if (FAILED(surface->GetSurfaceDesc(&desc)))
		return Error("IDirectDrawSurface7::GetCaps() failed!");

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
// release the prisoners
////////////////////////////////////////////////////////////////////////////////////////

VOID DIRECTDRAW::ReleaseBuffers()
{
	DIRECTX::Release( clipper );
	DIRECTX::Release( NesBuffer, TRUE );
	DIRECTX::Release( BackBuffer );

	if (FrontBuffer)
		FrontBuffer->SetClipper( NULL );

	DIRECTX::Release( FrontBuffer );
}

////////////////////////////////////////////////////////////////////////////////////////
// draw the nes surface to the back buffer
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTDRAW::DrawNesBuffer()
{
	PDX_ASSERT(!windowed && NesBuffer);

	if (!NesBuffer)
		return PDX_FAILURE;

	PDX_TRY(ValidateSurface(BackBuffer));

	if (FAILED(BackBuffer->Blt(&ScreenRect,NesBuffer,&NesBltRect,DDBLT_WAIT|DDBLT_ASYNC,NULL)))
		BackBuffer->Blt(&ScreenRect,NesBuffer,&NesBltRect,DDBLT_WAIT,NULL);

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline PDXRESULT DIRECTDRAW::ValidateSurface(LPDIRECTDRAWSURFACE7 surface) const
{
	PDX_ASSERT( surface );

	if (!surface)
		return PDX_FAILURE;

	if (FAILED(surface->IsLost()))
	{
		Sleep(100);

		PDX_ASSERT( device );

		if (FAILED(device->RestoreAllSurfaces()))
			return PDX_FAILURE;
	}

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID DIRECTDRAW::Repaint()
{
	if (!ready || PDX_FAILED(ValidateSurface(FrontBuffer)))
		return;

	if (windowed)
	{
		if (NesBuffer)
		{
			if (FAILED(FrontBuffer->Blt(&ScreenRect,NesBuffer,&NesBltRect,DDBLT_WAIT|DDBLT_ASYNC,NULL)))
				FrontBuffer->Blt(&ScreenRect,NesBuffer,&NesBltRect,DDBLT_WAIT,NULL);
		}
	}
	else
	{
		DrawNesBuffer();

		if (BackBuffer)
		{
			if (FAILED(FrontBuffer->Blt(NULL,BackBuffer,NULL,DDBLT_WAIT|DDBLT_ASYNC,NULL)))
				FrontBuffer->Blt(NULL,BackBuffer,NULL,DDBLT_WAIT,NULL);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL DIRECTDRAW::DoVSync()
{
	DontFlip = !(caps.dwCaps2 & DDCAPS2_FLIPNOVSYNC);

	if (UseVSync && RefreshRate == DisplayMode.RefreshRate)
	{
		DontFlip = FALSE;

		if (windowed || GDIMode)
		{
			if (!device || FAILED(device->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN,0)))
				return FALSE;
		}

		return TRUE;
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID DIRECTDRAW::Present()
{
	if (!ready || PDX_FAILED(ValidateSurface(FrontBuffer)))
		return;

	if (windowed)
	{
		if (NesBuffer)
		{
			if (FAILED(FrontBuffer->Blt(&ScreenRect,NesBuffer,&NesBltRect,DDBLT_WAIT|DDBLT_ASYNC,NULL)))
				FrontBuffer->Blt(&ScreenRect,NesBuffer,&NesBltRect,DDBLT_WAIT,NULL);
		}
	}
	else if (GDIMode || DontFlip)
	{
		if (BackBuffer)
		{
			if (FAILED(FrontBuffer->Blt(NULL,BackBuffer,NULL,DDBLT_WAIT|DDBLT_ASYNC,NULL)))
				FrontBuffer->Blt(NULL,BackBuffer,NULL,DDBLT_WAIT,NULL);
		}
	}
	else
	{
		FrontBuffer->Flip
		(
	       	NULL,
			UseVSync && DisplayMode.RefreshRate == RefreshRate ? DDFLIP_WAIT : DDFLIP_WAIT|DDFLIP_NOVSYNC
		);
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID DIRECTDRAW::Print(const CHAR* const text,const UINT x,const UINT y,const ULONG color,const UINT length)
{
	LPDIRECTDRAWSURFACE7 surface = windowed ? NesBuffer : BackBuffer;
	PDX_ASSERT( surface && text );

	HDC hdc; 

	if (surface && text && SUCCEEDED(surface->GetDC( &hdc )))
	{
		SetTextColor( hdc, color );
		SetBkMode( hdc, TRANSPARENT );
		TextOut( hdc, x, y, text, length ? length : strlen(text) );
		surface->ReleaseDC( hdc );
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// display mode sorter
////////////////////////////////////////////////////////////////////////////////////////

BOOL DIRECTDRAW::DISPLAYMODE::operator < (const DIRECTDRAW::DISPLAYMODE& mode) const
{
	if (width < mode.width) return TRUE;
	if (width > mode.width) return FALSE;

	if (height < mode.height) return TRUE;
	if (height > mode.height) return FALSE;

	if (bpp < mode.bpp) return TRUE;
	if (bpp > mode.bpp) return FALSE;

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
// display mode match
////////////////////////////////////////////////////////////////////////////////////////

BOOL DIRECTDRAW::DISPLAYMODE::operator == (const DIRECTDRAW::DISPLAYMODE& mode) const
{
	return 
	(
		width == mode.width && 
		height == mode.height &&
		bpp == mode.bpp &&
		RefreshRate == mode.RefreshRate &&
		AnyShiftRight == mode.AnyShiftRight &&
		AnyShiftLeft == mode.AnyShiftLeft
	);
}

////////////////////////////////////////////////////////////////////////////////////////
// setup the RGB bit masks and shifts
////////////////////////////////////////////////////////////////////////////////////////
							  
VOID DIRECTDRAW::DISPLAYMODE::SetPixelFormat(const DWORD r,const DWORD g,const DWORD b)
{
	rMask = r;
	gMask = g;
	bMask = b;

	DWORD rgbMask[3] = {r,g,b};

	for (UINT i=0; i < 3; ++i)
	{
		rgbShiftLeft[i] = 0;
		rgbShiftRight[i] = 0;

		if (rgbMask[i]) 
		{ 
			while ((rgbMask[i] & 0x1) == 0)
			{
				++rgbShiftLeft[i]; 
				rgbMask[i] >>= 1;
			}

			while ((rgbMask[i] & 0x1) != 0)
			{
				++rgbShiftRight[i];
				rgbMask[i] >>= 1;
			}

			rgbShiftRight[i] = 8 - rgbShiftRight[i];
		}
	}
}
