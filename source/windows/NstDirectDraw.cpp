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
#include "../2xSai/2xSai.h"
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

	if (desc->dwWidth < NES_WIDTH || desc->dwHeight < NES_HEIGHT)
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
hWnd             (NULL),
device           (NULL),
SelectedDevice   (0),
FrontBuffer      (NULL),					
BackBuffer       (NULL),
NesBuffer        (NULL),
clipper          (NULL),
GDIMode          (0),
windowed         (TRUE),
UseVRam          (FALSE),
ScaleFactor      (0),
DontFlip         (FALSE),
FlipFlags        (DDFLIP_WAIT),
PaletteChanged   (TRUE),
ready            (FALSE),
Use2xSaI         (FALSE),
Use2xSaI565      (FALSE),
DDError          (FALSE),
ScreenEffect     (SCREENEFFECT_NONE),
IsNesBuffer2xSaI (FALSE)
{
	SetRect( &NesRect, 0, 0, 256, 224 );
	SetRect( &ScreenRect, 0, 0, 256, 224 );

	PDXMemZero( PixelBuffer, PIXEL_BUFFER_LENGTH );
	PDXMemZero( EffectBuffer, EFFECT_BUFFER_LENGTH );
	PDXMemZero( ConvBuffer, CONV_BUFFER_LENGTH );
	PDXMemZero( DisplayMode );
	PDXMemZero( caps );
	PDXMemZero( guid );
}

////////////////////////////////////////////////////////////////////////////////////////
// destructor
////////////////////////////////////////////////////////////////////////////////////////

DIRECTDRAW::~DIRECTDRAW()
{
	Destroy();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTDRAW::Error(const CHAR* const msg)
{
	ready = FALSE;

	ReleaseBuffers();

	PDXSTRING string("DIRECTDRAW: ");
	string << msg;

	application.LogOutput( string.String() );
	application.OnError( msg );

	return PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
// update the parameters
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTDRAW::SetScreenParameters(const SCREENEFFECT effect,const BOOL vram,const RECT& rect)
{
	PDX_ASSERT( rect.bottom - rect.top < NES_HEIGHT );
	PDX_ASSERT( rect.right - rect.left < NES_WIDTH  );

	if 
	(
     	NesBuffer && 
		data.effect == effect && 
		data.vram == vram && 
		!memcmp(&data.rect,&rect,sizeof(RECT))
	)
        return PDX_OK;

	{
		data.effect = effect;
		data.vram   = vram;
		data.rect   = rect;
	}

	SetRect( &NesRect, rect.left, rect.top, rect.right+1, rect.bottom+1 );
	
	NesBltRect   = NesRect;	
	ScreenEffect = effect;
	UseVRam      = vram;

	return ready ? CreateNesBuffer() : PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID DIRECTDRAW::UpdateScreenRect(const RECT& rect,const BOOL ReCreate)
{
	if (!windowed || ReCreate)
	{
		if (memcmp( &rect, &ScreenRect, sizeof(RECT) ))
		{
			ScreenRect = rect;
			CreateNesBuffer();

			PDXSTRING log;
			log  = "DIRECTDRAW: target BLT dimensions was changed to (";
			log += ScreenRect.left;
			log += ",";
			log += ScreenRect.top;
			log += ") - (";
			log += ScreenRect.right;
			log += ",";
			log += ScreenRect.bottom;
			log += ")";
			application.LogOutput( log.String() );
		}
	}
	else
	{
		ScreenRect = rect;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// main initializer, called once
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTDRAW::Initialize(HWND h)
{
	PDX_ASSERT(h);

	ready = FALSE;
	hWnd = h;

	application.LogOutput("DIRECTDRAW: Initializing");

	adapters.Clear();

	if (FAILED(DirectDrawEnumerateEx(EnumAdapters,PDX_CAST(LPVOID,&adapters),DDENUM_ATTACHEDSECONDARYDEVICES)))
		return Error("DirectDrawEnumerateEx() failed!");

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
// create the device
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTDRAW::Create(const GUID& g)
{
	if (device && !memcmp( &guid, &g, sizeof(guid) ))
		return PDX_OK;

	Destroy();

	guid = g;

	if (FAILED(DirectDrawCreateEx(&guid,PDX_CAST(LPVOID*,&device),IID_IDirectDraw7,NULL)))
		return PDX_FAILURE;

	{
		PDXSTRING string("DIRECTDRAW: creating device - guid: ");
		string << CONFIGFILE::FromGUID( guid );
		application.LogOutput( string.String() );
	}

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

				application.LogOutput( log.String() );
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
	if (!device)
		return PDX_FAILURE;

	DIRECTX::Release(NesBuffer);
	DIRECTX::InitStruct(NesDesc);

	NesDesc.dwFlags        = DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT|DDSD_PIXELFORMAT;
	NesDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN|DDSCAPS_SYSTEMMEMORY;
	NesDesc.dwWidth        = NES_WIDTH;
	NesDesc.dwHeight       = NES_HEIGHT;
	
	if (UseVRam && (FrontDesc.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY))
	{
		NesDesc.ddsCaps.dwCaps &= ~DDSCAPS_SYSTEMMEMORY;
		NesDesc.ddsCaps.dwCaps |=  DDSCAPS_VIDEOMEMORY;
	}

	NesDesc.ddpfPixelFormat.dwSize        = sizeof(NesDesc.ddpfPixelFormat);
	NesDesc.ddpfPixelFormat.dwFlags       = DDPF_RGB;
	NesDesc.ddpfPixelFormat.dwRGBBitCount = FrontDesc.ddpfPixelFormat.dwRGBBitCount;
	NesDesc.ddpfPixelFormat.dwRBitMask    = FrontDesc.ddpfPixelFormat.dwRBitMask;
	NesDesc.ddpfPixelFormat.dwGBitMask    = FrontDesc.ddpfPixelFormat.dwGBitMask;
	NesDesc.ddpfPixelFormat.dwBBitMask    = FrontDesc.ddpfPixelFormat.dwBBitMask;

	PaletteChanged = TRUE;
	NesBltRect = NesRect;
	ScaleFactor = 0;
	Use2xSaI = FALSE;

	{
		const UINT width = ScreenRect.right - ScreenRect.left;
		const UINT height = ScreenRect.bottom - ScreenRect.top;

		switch (ScreenEffect)
		{
     		case SCREENEFFECT_SCANLINES:
			{
				if ((NesBltRect.right-NesBltRect.left) * 2 <= width && (NesBltRect.bottom-NesBltRect.top) * 2 <= height)
				{
					++ScaleFactor;
					NesDesc.dwWidth   *= 2;
					NesDesc.dwHeight  *= 2;
					NesBltRect.top    *= 2;			
					NesBltRect.right  *= 2;
					NesBltRect.bottom *= 2;
					NesBltRect.left   *= 2;
				}
				break;
			}
  
			case SCREENEFFECT_TV:
			{
				ScaleFactor        = 1;
				NesDesc.dwWidth   *= 2;
				NesDesc.dwHeight  *= 2;
				NesBltRect.top    *= 2;			
				NesBltRect.right  *= 2;
				NesBltRect.bottom *= 2;
				NesBltRect.left   *= 2;
				break;
			}

			case SCREENEFFECT_2XSAI:
			case SCREENEFFECT_SUPER_2XSAI:
			case SCREENEFFECT_SUPER_EAGLE:
			{
				if ((NesBltRect.right-NesBltRect.left) * 2 <= width && (NesBltRect.bottom-NesBltRect.top) * 2 <= height)
				{
					++ScaleFactor;

					NesDesc.dwWidth   *= 2;
					NesDesc.dwHeight  *= 2;

					NesBltRect.top    *= 2;			
					NesBltRect.right  *= 2;
					NesBltRect.bottom *= 2;
					NesBltRect.left   *= 2;

					Use2xSaI = TRUE;
				}
				break;
			}
		}
	}

	{
		PDXSTRING log;

		log  = "DIRECTDRAW: creating the nes render target surface in ";
		log += (UseVRam ? "video memory" : "system memory");
		application.LogOutput( log.String() );

		log  = "DIRECTDRAW: surface dimensions (";
		log += NesDesc.dwWidth;
		log += ",";
		log += NesDesc.dwHeight;
		log += ")";
		application.LogOutput( log.String() );

		log  = "DIRECTDRAW: source BLT dimensions (";
		log += NesBltRect.left;
		log += ",";
		log += NesBltRect.top;
		log += ") - (";
		log += NesBltRect.right;
		log += ",";
		log += NesBltRect.bottom;
		log += ")";
		application.LogOutput( log.String() );

		log  = "DIRECTDRAW: target BLT dimensions (";
		log += ScreenRect.left;
		log += ",";
		log += ScreenRect.top;
		log += ") - (";
		log += ScreenRect.right;
		log += ",";
		log += ScreenRect.bottom;
		log += ")";
		application.LogOutput( log.String() );

		log  = "DIRECTDRAW: scale factor - ";
		log += ScaleFactor;
		application.LogOutput( log.String() );

		log  = "DIRECTDRAW: 2xSaI - ";
		log += (Use2xSaI ? "on" : "off");
		application.LogOutput( log.String() );
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

		application.LogOutput( log.String() );

		if (FAILED(device->CreateSurface(&NesDesc,&NesBuffer,NULL)))
			return Error("IDirectDraw7::CreateSurface() failed!");
	}

	if (FAILED(GetSurfaceDesc(NesBuffer,NesDesc)))
		return PDX_FAILURE;

	Use2xSaI565 = FALSE;
	IsNesBuffer2xSaI = FALSE;

	if (Use2xSaI)
	{
		IsNesBuffer2xSaI = NesDesc.ddpfPixelFormat.dwRGBBitCount == 16 &&
		(
       		NesDesc.ddpfPixelFormat.dwRBitMask == b16(01111100,00000000) &&
     		NesDesc.ddpfPixelFormat.dwGBitMask == b16(00000011,11100000) &&
       		NesDesc.ddpfPixelFormat.dwBBitMask == b16(00000000,00011111)
		);

		if (IsNesBuffer2xSaI)
		{
			Init_2xSaI( 555 );
		}
		else
		{
			Init_2xSaI( 565 );

			Use2xSaI565 = TRUE;

			IsNesBuffer2xSaI = NesDesc.ddpfPixelFormat.dwRGBBitCount == 16 &&
			(
				NesDesc.ddpfPixelFormat.dwRBitMask == b16(11111000,00000000) &&
				NesDesc.ddpfPixelFormat.dwGBitMask == b16(00000111,11100000) &&
				NesDesc.ddpfPixelFormat.dwBBitMask == b16(00000000,00011111)
			);
		}
	}

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

PDXRESULT DIRECTDRAW::SwitchToFullScreen(const UINT width,const UINT height,const UINT bpp,const RECT* const rect)
{
	PDX_ASSERT( device && hWnd );

	if (!device || !hWnd)
		return PDX_FAILURE;

 	windowed = FALSE;
	ready = FALSE;

	if (!FrontBuffer || !BackBuffer || DisplayMode.width != width || DisplayMode.height != height || DisplayMode.bpp != bpp)
	{
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
			{
				PDXSTRING msg;

				msg << "DIRECTDRAW: setting display mode ";
				msg << DisplayMode.width;
				msg << " * ";
				msg << DisplayMode.height;
				msg << " * ";
				msg << DisplayMode.bpp;
				msg << ", 60 hz";

				application.LogOutput( msg.String() );
			}

			if (FAILED(device->SetDisplayMode(DisplayMode.width,DisplayMode.height,DisplayMode.bpp,60,0)))
			{
				application.LogOutput("DIRECTDRAW: SetDisplayMode() failed, trying with default refresh rate");

				if (FAILED(device->SetDisplayMode(DisplayMode.width,DisplayMode.height,DisplayMode.bpp,0,0)))
					return Error("IDirectDraw7::SetDisplayMode() failed!");

				DWORD f=0;
				DisplayMode.RefreshRate = (device->GetMonitorFrequency(&f) == DD_OK && f == 60) ? 60 : 0;
			}
			else
			{
				DisplayMode.RefreshRate = 60;
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
	}

	if (rect)
	{
		ScreenRect = *rect;

		PDX_ASSERT( ScreenRect.right - ScreenRect.left <= DisplayMode.width );
		PDX_ASSERT( ScreenRect.bottom - ScreenRect.top <= DisplayMode.height );
	}
	else
	{
		SetRect( &ScreenRect, 0, 0, DisplayMode.width, DisplayMode.height );
	}

	{
		PDXSTRING log;
		log  = "DIRECTDRAW: initializing target BLT dimensions to (";
		log += ScreenRect.left;
		log += ",";
		log += ScreenRect.top;
		log += ") - (";
		log += ScreenRect.right;
		log += ",";
		log += ScreenRect.bottom;
		log += ")";
		application.LogOutput( log.String() );
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

	if (FAILED(FrontBuffer->IsLost()))
	{
		Sleep(100);
		device->RestoreAllSurfaces();
	}

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
	{
		PDXSTRING string("DIRECTDRAW: GDI mode reference count");
		string << GDIMode;
		application.LogOutput( string.String() );
	}
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

	if (!surface || FAILED(surface->GetBltStatus(DDGBS_CANBLT)))
		return PDX_FAILURE;
	
	ClearScreen();

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
// clear any surface
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTDRAW::ClearSurface(LPDIRECTDRAWSURFACE7 surface)
{
	if (surface)
	{
		DDBLTFX BltFX;
		BltFX.dwSize = sizeof(BltFX);
		BltFX.dwFillColor = 0;

		if (FAILED(surface->Blt(NULL,NULL,NULL,DDBLT_COLORFILL|DDBLT_WAIT|DDBLT_ASYNC,&BltFX)))
			if (FAILED(surface->Blt(NULL,NULL,NULL,DDBLT_COLORFILL|DDBLT_WAIT,&BltFX)))
				DDError = TRUE;
	}

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
		if (!windowed)
			device->RestoreDisplayMode();

		if (FAILED(device->SetCooperativeLevel(hWnd,DDSCL_NORMAL)))
			result = Error("IDirectDraw7::SetCooperativeLevel() failed!");

		if (DIRECTX::Release(device,TRUE))
			result = PDX_FAILURE;
	}

	PDXMemZero( guid );

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////
// lock the nes surface
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTDRAW::Lock(NES::IO::GFX& PixelMap)
{
	PDX_ASSERT( ready );

	if (PixelMap.PaletteChanged || PaletteChanged)
	{
		PaletteChanged = FALSE;
		PixelMap.PaletteChanged = FALSE;
		UpdatePalette( PixelMap.palette );
	}

	PixelMap.pixels = PixelBuffer;

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID DIRECTDRAW::UpdatePalette(const U8* const input)
{
	PDX_ASSERT( input );

	if (Use2xSaI)
	{
		if (Use2xSaI565)
		{
			for (UINT i=0; i < PALETTE_LENGTH; ++i)
			{
				palette[i][0] = (input[(i*3) + 0] >> 3) << 11;
				palette[i][1] = (input[(i*3) + 1] >> 2) <<  5;
				palette[i][2] = (input[(i*3) + 2] >> 3) <<  0;
			}
		}
		else
		{
			for (UINT i=0; i < PALETTE_LENGTH; ++i)
			{
				palette[i][0] = (input[(i*3) + 0] >> 2) << 10;
				palette[i][1] = (input[(i*3) + 1] >> 2) <<  5;
				palette[i][2] = (input[(i*3) + 2] >> 2) <<  0;
			}
		}
	}
	else
	{
		for (UINT i=0; i < PALETTE_LENGTH; ++i)
		{
			palette[i][0] = (input[(i*3) + 0] >> DisplayMode.rgbShiftRight[0]) << DisplayMode.rgbShiftLeft[0];
			palette[i][1] = (input[(i*3) + 1] >> DisplayMode.rgbShiftRight[1]) << DisplayMode.rgbShiftLeft[1];
			palette[i][2] = (input[(i*3) + 2] >> DisplayMode.rgbShiftRight[2]) << DisplayMode.rgbShiftLeft[2];
		}
	}
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
	if (Use2xSaI)
	{
		BltNesScreen2xSaI
		(
     		ScreenEffect == SCREENEFFECT_2XSAI ? _2xSaI :
     		ScreenEffect == SCREENEFFECT_SUPER_2XSAI ? Super2xSaI :
     		SuperEagle,
			dst,
			pitch
		);
	}
	else if (ScreenEffect == SCREENEFFECT_SCANLINES)
	{
		switch (ScaleFactor)
		{
			case 1:  BltNesScreenScanLines1( dst, pitch );      break;
			default: BltNesScreenScanLinesFactor( dst, pitch ); break;
		}
	}
	else if (ScreenEffect == SCREENEFFECT_TV)
	{
		BltNesScreenTV( dst, pitch );
	}
	else
	{
		if (pitch == NES_WIDTH)
		{
			BltNesScreenAligned( dst );
		}
		else
		{
			BltNesScreenUnaligned( dst, pitch );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// blit to the nes surface 
////////////////////////////////////////////////////////////////////////////////////////

template<class T> 
VOID DIRECTDRAW::BltNesScreenAligned(T* const dst)
{
	for (UINT i=0; i < PIXEL_BUFFER_LENGTH; ++i)
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

	for (UINT y=0; y < NES_HEIGHT; ++y)
	{
		for (UINT x=0; x < NES_WIDTH; ++x)
		{
			const U32* const rgb = palette[src[x]];
			dst[x] = T(rgb[0] + rgb[1] + rgb[2]);
		}
		
		dst += pitch;
		src += NES_WIDTH;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

template<class T>
VOID DIRECTDRAW::BltNesScreenTV(T* dst,const LONG pitch)
{
	const U32* rgb;

	const LONG pitch2 = pitch + pitch;

	for (UINT y=0; y < PIXEL_BUFFER_LENGTH; y += NES_WIDTH, dst += pitch2)
	{
		UINT i=0;

		for (UINT x=0; x < NES_WIDTH-2; ++x, i+=2)
		{
			UINT p = y + x;
			const U32* const pixel = palette[PixelBuffer[p]];

			dst[i+0] = T
			(
		     	(pixel[0] & DisplayMode.rMask) + 
				(pixel[1] & DisplayMode.gMask) + 
				(pixel[2] & DisplayMode.bMask)
			);

			rgb = palette[PixelBuffer[++p]];

			dst[i+1] = T
			(
		     	(((pixel[0] + rgb[0]) >> 1) & DisplayMode.rMask) + 
				(((pixel[1] + rgb[1]) >> 1) & DisplayMode.gMask) + 
				(((pixel[2] + rgb[2]) >> 1) & DisplayMode.bMask)
			);

			rgb = palette[PixelBuffer[p+=NES_WIDTH]];

			dst[i+pitch+0] = T
			(
		     	((((pixel[0] + rgb[0]) * 11) >> 5) & DisplayMode.rMask) + 
				((((pixel[1] + rgb[1]) * 11) >> 5) & DisplayMode.gMask) + 
				((((pixel[2] + rgb[2]) * 11) >> 5) & DisplayMode.bMask)
			);

			rgb = palette[PixelBuffer[p+1]];

			dst[i+pitch+1] = T
			(
		     	((((pixel[0] + rgb[0]) * 11) >> 5) & DisplayMode.rMask) + 
				((((pixel[1] + rgb[1]) * 11) >> 5) & DisplayMode.gMask) + 
				((((pixel[2] + rgb[2]) * 11) >> 5) & DisplayMode.bMask)
			);
		}

		rgb = palette[PixelBuffer[y+NES_WIDTH-2]]; 
		dst[i+0] = T(rgb[0] + rgb[1] + rgb[2]);

		rgb = palette[PixelBuffer[y+NES_WIDTH-1]]; 
		dst[i+1] = T(rgb[0] + rgb[1] + rgb[2]);
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

template<class T> 
VOID DIRECTDRAW::BltNesScreenScanLines1(T* dst,const LONG pitch)
{
	const NES::IO::GFX::PIXEL* src = PixelBuffer;
	const NES::IO::GFX::PIXEL* const end = PixelBuffer + PIXEL_BUFFER_LENGTH;

	while (src != end)
	{
		for (UINT x=0,i=0; x < NES_WIDTH; ++x, i+=2)
		{
			const U32* const rgb = palette[src[x]];

			dst[i+1] = dst[i+0] = T
			(
				(rgb[0] & DisplayMode.rMask) + 
				(rgb[1] & DisplayMode.gMask) + 
				(rgb[2] & DisplayMode.bMask)
			);     
		}

		dst += pitch;

		for (UINT x=0,i=0; x < NES_WIDTH; ++x, i+=2)
		{
			const U32* const rgb = palette[src[x]];

			dst[i+1] = dst[i+0] = T
			(
				((rgb[0] >> 2) & DisplayMode.rMask) + 
				((rgb[1] >> 2) & DisplayMode.gMask) + 
				((rgb[2] >> 2) & DisplayMode.bMask)
			);     
		}

		dst += pitch;
		src += NES_WIDTH;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

template<class T> 
VOID DIRECTDRAW::BltNesScreenScanLinesFactor(T* dst,const LONG pitch)
{
	const UINT yScale = NES_HEIGHT << ScaleFactor;
	const UINT xScale = NES_WIDTH << ScaleFactor;
	const UINT ScanStep = PDX_MAX(1,ScaleFactor);

	const NES::IO::GFX::PIXEL* const offset = PixelBuffer;

	for (UINT y=0; y < yScale; ++y)
	{
		const NES::IO::GFX::PIXEL* const src = offset + ((y >> ScaleFactor) * NES_WIDTH);

		const UINT yScan = y & ScanStep;

		if (yScan)
		{
			for (UINT x=0; x < xScale; ++x)
			{
				const U32* const rgb = palette[src[x >> ScaleFactor]];

				dst[x] = T
				(
					((rgb[0] >> yScan) & DisplayMode.rMask) + 
					((rgb[1] >> yScan) & DisplayMode.gMask) + 
					((rgb[2] >> yScan) & DisplayMode.bMask)
				);     
			}
		}
		else
		{
			for (UINT x=0; x < xScale; ++x)
			{
				const U32* const rgb = palette[src[x >> ScaleFactor]];

				dst[x] = T
				(
					(rgb[0] & DisplayMode.rMask) + 
					(rgb[1] & DisplayMode.gMask) + 
					(rgb[2] & DisplayMode.bMask)
				);     
			}
		}

		dst += pitch;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

template<class T>
VOID DIRECTDRAW::BltNesScreen2xSaI(F2XAI F2xSaI,T* dst,const LONG pitch)
{
	PDX_ASSERT( NesDesc.dwWidth == NES_WIDTH_2 && NesDesc.dwHeight == NES_HEIGHT_2 );

	for (UINT i=0; i < EFFECT_BUFFER_LENGTH; ++i)
	{
		const U32* const rgb = palette[PixelBuffer[i]];
		EffectBuffer[i] = U16(rgb[0] + rgb[1] + rgb[2]);
	}

	if (IsNesBuffer2xSaI)
	{
		F2xSaI
		( 
			PDX_CAST(U8*,EffectBuffer), 
			NES_WIDTH * sizeof(U16), 
			NULL, 
			PDX_CAST(U8*,dst), 
			pitch * sizeof(U16), 
			NES_WIDTH, 
			NES_HEIGHT
		);
	}
	else
	{
		F2xSaI
		( 
			PDX_CAST(U8*,EffectBuffer), 
			NES_WIDTH * sizeof(U16), 
			NULL, 
			PDX_CAST(U8*,ConvBuffer), 
			NES_WIDTH_2 * sizeof(U16), 
			NES_WIDTH, 
			NES_HEIGHT
		);

		UINT rShift = DisplayMode.rShiftLeft;
		if (DisplayMode.rShiftRight < 8-5) rShift += (8-DisplayMode.rShiftRight) - 5;

		UINT gShift = DisplayMode.gShiftLeft;
		if (DisplayMode.gShiftRight < 8-6) gShift += (8-DisplayMode.gShiftRight) - 6;

		UINT bShift = DisplayMode.bShiftLeft;
		if (DisplayMode.bShiftRight < 8-5) bShift += (8-DisplayMode.bShiftRight) - 5;

		if (pitch == NES_WIDTH_2)
		{
			for (UINT i=0; i < CONV_BUFFER_LENGTH; ++i)
			{
				const T pixel(ConvBuffer[i]);

				dst[i] =
				(
					(((pixel                         ) >> 11) << rShift) + 
					(((pixel & b16(00000111,11100000)) >>  5) << gShift) + 
					(((pixel & b16(00000000,00011111))      ) << bShift)  
				);
			}
		}
		else
		{
			const U16* src = ConvBuffer;

			for (UINT y=0; y < NES_HEIGHT_2; ++y)
			{
				for (UINT x=0; x < NES_WIDTH_2; ++x)
				{
					const T pixel(src[x]);

					dst[x] =
					(
						(((pixel                         ) >> 11) << rShift) + 
						(((pixel & b16(00000111,11100000)) >>  5) << gShift) + 
						(((pixel & b16(00000000,00011111))      ) << bShift)  
					);
				}

				src += NES_WIDTH_2;
				dst += pitch;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// lock the nes surface
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTDRAW::LockNesBuffer(DDSURFACEDESC2& desc)
{
	if (NesBuffer)
	{
		desc.dwSize = sizeof(desc);
		desc.dwFlags = 0;

		if (FAILED(NesBuffer->Lock(NULL,&desc,DDLOCK_NOSYSLOCK|DDLOCK_WAIT|DDLOCK_WRITEONLY,NULL)))
			DDError = TRUE;
	}

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

	if (NesBuffer)
	{
		if (FAILED(BackBuffer->Blt(&ScreenRect,NesBuffer,&NesBltRect,DDBLT_WAIT|DDBLT_ASYNC,NULL)))
			if (FAILED(BackBuffer->Blt(&ScreenRect,NesBuffer,&NesBltRect,DDBLT_WAIT,NULL)))
				DDError = TRUE;

		return PDX_OK;
	}

	return PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline PDXRESULT DIRECTDRAW::ValidateSurface(LPDIRECTDRAWSURFACE7 surface) const
{
	PDX_ASSERT( surface );

	if (surface)
	{
		if (FAILED(surface->IsLost()))
		{
			Sleep(100);

			PDX_ASSERT( device );

			if (FAILED(device->RestoreAllSurfaces()))
				return PDX_FAILURE;
		}
  
		return PDX_OK;
	}
  
	return PDX_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID DIRECTDRAW::Repaint()
{
	if (ready && FrontBuffer)
	{
		if (windowed)
		{
			if (NesBuffer)
			{
				if (FAILED(FrontBuffer->Blt(&ScreenRect,NesBuffer,&NesBltRect,DDBLT_WAIT|DDBLT_ASYNC,NULL)))
					if (FAILED(FrontBuffer->Blt(&ScreenRect,NesBuffer,&NesBltRect,DDBLT_WAIT,NULL)))
						DDError = TRUE;
			}
		}
		else
		{
			DrawNesBuffer();

			if (BackBuffer)
			{
				if (FAILED(FrontBuffer->Blt(NULL,BackBuffer,NULL,DDBLT_WAIT|DDBLT_ASYNC,NULL)))
					if (FAILED(FrontBuffer->Blt(NULL,BackBuffer,NULL,DDBLT_WAIT,NULL)))
						DDError = TRUE;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL DIRECTDRAW::UpdateRefresh(const BOOL vSync,const UINT RefreshRate)
{
	FlipFlags = DDFLIP_WAIT;
	DontFlip = FALSE;

	if (vSync && RefreshRate == DisplayMode.RefreshRate)
	{
		if (windowed || GDIMode)
		{
			if (FAILED(device->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN,0)))
				return FALSE;
		}

		return TRUE;
	}
	else
	{
		if (caps.dwCaps2 & DDCAPS2_FLIPNOVSYNC)
		{
			FlipFlags = DDFLIP_WAIT | DDFLIP_NOVSYNC;
		}
		else
		{
			DontFlip = TRUE;
		}
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID DIRECTDRAW::Present()
{
	if (ready && FrontBuffer)
	{
		if (windowed)
		{
			if (FAILED(FrontBuffer->Blt(&ScreenRect,NesBuffer,&NesBltRect,DDBLT_WAIT|DDBLT_ASYNC,NULL)))
				if (FAILED(FrontBuffer->Blt(&ScreenRect,NesBuffer,&NesBltRect,DDBLT_WAIT,NULL)))
					DDError = TRUE;
		}
		else if (GDIMode || DontFlip)
		{
			if (FAILED(FrontBuffer->Blt(NULL,BackBuffer,NULL,DDBLT_WAIT|DDBLT_ASYNC,NULL)))
				if (FAILED(FrontBuffer->Blt(NULL,BackBuffer,NULL,DDBLT_WAIT,NULL)))
					DDError = TRUE;
		}
		else
		{
			if (FAILED(FrontBuffer->Flip(NULL,FlipFlags)))
				DDError = TRUE;
		}

		if (DDError)
		{
			DDError = FALSE;

			ValidateSurface( FrontBuffer );
			ValidateSurface( NesBuffer   );

			if (!windowed)
				ValidateSurface( BackBuffer );
		}
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
		rShiftLeft == mode.rShiftLeft &&
		gShiftLeft == mode.gShiftLeft &&
		bShiftLeft == mode.bShiftLeft &&
		rShiftRight == mode.rShiftRight &&
		gShiftRight == mode.gShiftRight &&
		bShiftRight == mode.bShiftRight
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
