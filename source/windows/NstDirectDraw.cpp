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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include "../paradox/PdxQuickSort.h"
#include "../2xSai/2xSai.h"
#include "NstException.h"
#include "NstUtilities.h"
#include "NstLogFileManager.h"
#include "NstDirectDraw.h"
#include "NstDirectX.h"

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
// constructor
////////////////////////////////////////////////////////////////////////////////////////

DIRECTDRAW::PIXELDATA::PIXELDATA()
{ 
	PDXMemZero
	( 
    	(U32*)palette, 
		NES::IO::GFX::PALETTE_LENGTH * 3 
	);

	Reset(); 
}

////////////////////////////////////////////////////////////////////////////////////////
// reset
////////////////////////////////////////////////////////////////////////////////////////

VOID DIRECTDRAW::PIXELDATA::Reset()
{
	for (UINT i=0; i < REAL_PIXEL_BUFFER_LENGTH; ++i)
		buffer[i] = 0x0D;

	memset( effect, 0x00, sizeof(U16) * EFFECT_BUFFER_LENGTH );
	memset( conv, 0x00, sizeof(U16) * CONV_BUFFER_LENGTH );
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
ScreenEffect     (SCREENEFFECT_NONE),
IsNesBuffer2xSaI (FALSE),
FrameLatency     (0),
PixelData        (NULL)
{
	PixelData = new PIXELDATA;

	DisplayMode.width = GetSystemMetrics( SM_CXSCREEN );
	DisplayMode.height = GetSystemMetrics( SM_CYSCREEN );

	::SetRect( &rcNes,    0, 8, 256, 232 );
	::SetRect( &rcScreen, 0, 0, 256, 224 );
	rcNesBlt = rcNes;

	DIRECTX::InitStruct( HalCaps   );
	DIRECTX::InitStruct( HelCaps   );
	DIRECTX::InitStruct( FrontDesc );
	DIRECTX::InitStruct( BackDesc  );
	DIRECTX::InitStruct( NesDesc   );

	PDXMemZero( guid );

	format.device = PDX_CAST(VOID*,this);
}

////////////////////////////////////////////////////////////////////////////////////////
// destructor
////////////////////////////////////////////////////////////////////////////////////////

DIRECTDRAW::~DIRECTDRAW()
{
	Destroy();
	delete PixelData;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL DIRECTDRAW::CheckReady() const
{
	return device && FrontBuffer && (windowed || BackBuffer) && NesBuffer && clipper;
}

////////////////////////////////////////////////////////////////////////////////////////
// enumerate the devices
////////////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DIRECTDRAW::EnumAdapters(LPGUID guid,LPSTR desc,LPSTR name,LPVOID context,HMONITOR)
{
	LPDIRECTDRAW7 device = NULL;
	
	if (!context)
		return DDENUMRET_CANCEL;

	LOGFILE::Output
	(
		"DIRECTDRAW: enumerating device, guid: ",
		(guid ? UTILITIES::FromGUID(*guid).String() : "default"),
		", name: ",
		(desc ? desc : "unknown") 
	);

	if (FAILED(::DirectDrawCreateEx(guid,PDX_CAST_PTR(LPVOID,device),IID_IDirectDraw7,NULL)) || !device)
	{
		LOGFILE::Output("DIRECTDRAW: DirectDrawCreateEx() failed on this device, continuing enumeration..");
		return DDENUMRET_OK;
	}

	ADAPTERS& adapters = *PDX_CAST(ADAPTERS*,context);

	adapters.Grow();

	ADAPTER& adapter = adapters.Back();

	if (guid) PDXMemCopy( adapter.guid, *guid );
	else      PDXMemZero( adapter.guid        );

	adapter.name = desc;

	if (adapter.name.IsEmpty())
		adapter.name = "unknown";

	DISPLAYMODES& modes = adapter.DisplayModes;

	device->EnumDisplayModes(0,NULL,PDX_CAST(LPVOID,&modes),EnumDisplayModes);
	device->Release();

	if (modes.IsEmpty())
	{
		adapters.EraseBack();
		LOGFILE::Output("DIRECTDRAW: found no valid display mode for this device, continuing enumeration..");
		return DDENUMRET_OK;
	}

	PDXQUICKSORT::Sort( modes.Begin(), modes.End() );

	LOGFILE::Output
	(
		"DIRECTDRAW: found ",
		modes.Size(),
		" valid display mode(s) for this device"
	);
	
	return DDENUMRET_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
// enumerate the display modes
////////////////////////////////////////////////////////////////////////////////////////

HRESULT CALLBACK DIRECTDRAW::EnumDisplayModes(LPDDSURFACEDESC2 desc,LPVOID context)
{
	if (!context || !desc)
		return DDENUMRET_OK;

	if (desc->dwWidth < NES_WIDTH || desc->dwHeight < NES_HEIGHT)
		return DDENUMRET_OK;

	if (!(desc->ddpfPixelFormat.dwFlags & DDPF_RGB))
		return DDENUMRET_OK;

	const BOOL normal =
	(
		(
    		(desc->ddpfPixelFormat.dwRGBBitCount == 16 || desc->ddpfPixelFormat.dwRGBBitCount == 32) &&
			!(desc->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8)
		) 
		||
		(
	     	desc->ddpfPixelFormat.dwRGBBitCount == 8 && 
			(desc->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8)
		)
	);

	if (!normal) 
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
// update the parameters
////////////////////////////////////////////////////////////////////////////////////////

VOID DIRECTDRAW::SetScreenParameters(const SCREENEFFECT effect,const BOOL vram,const RECT& nRect,const RECT* const sRect)
{
	PDX_ASSERT( nRect.bottom - nRect.top < NES_HEIGHT );
	PDX_ASSERT( nRect.right - nRect.left < NES_WIDTH  );

	UseVRam = vram;
	ScreenEffect = effect;

	::SetRect( &rcNes, nRect.left, nRect.top, nRect.right+1, nRect.bottom+1 );
	rcNesBlt = rcNes;

	if (sRect)
		rcScreen = *sRect;

	if (device && FrontBuffer)
		CreateNesBuffer();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID DIRECTDRAW::UpdateScreenRect(const RECT& rect)
{
	rcScreen = rect;

	if (device && FrontBuffer)
		CreateNesBuffer();
}

////////////////////////////////////////////////////////////////////////////////////////
// main initializer, called once
////////////////////////////////////////////////////////////////////////////////////////

VOID DIRECTDRAW::Initialize(HWND h)
{
	PDX_ASSERT(adapters.IsEmpty() && h);

	ready = FALSE;
	hWnd = h;

	if (!hWnd || adapters.Size())
		throw ("Internal error in DIRECTDRAW::Initialize()!");

	if (FAILED(::DirectDrawEnumerateEx(EnumAdapters,PDX_CAST(LPVOID,&adapters),DDENUM_ATTACHEDSECONDARYDEVICES)))
		throw ("DirectDrawEnumerateEx() failed!");

	if (adapters.IsEmpty())
		throw (EXCEPTION(IDS_VIDEO_NO_DEVICE));

	PDX_ASSERT(adapters.Front().DisplayModes.Size());
}

////////////////////////////////////////////////////////////////////////////////////////
// create the device
////////////////////////////////////////////////////////////////////////////////////////

BOOL DIRECTDRAW::Create(const GUID& g,const BOOL force)
{
	if (!force && device && PDXMemCompare(guid,g))
		return TRUE;

	ReleaseBuffers();
	DIRECTX::Release(device,TRUE);

	guid = g;

	if (FAILED(::DirectDrawCreateEx(&guid,PDX_CAST(LPVOID*,&device),IID_IDirectDraw7,NULL)) || !device)
		return FALSE;

	LOGFILE::Output
	(
     	"DIRECTDRAW: creating device, guid: ",
		UTILITIES::FromGUID(guid).String()
	);

	DIRECTX::InitStruct( HalCaps );
	DIRECTX::InitStruct( HelCaps );

	if (FAILED(device->GetCaps(&HalCaps,&HelCaps)))
		throw ("IDirectDraw7::GetCaps() failed!");

	if (!CanRenderWindowed())
	{
		LOGFILE::Output
		(
			"DIRECTDRAW: error, this device is not capable of rendering in window mode!"
		);
		return FALSE;
	}

	if (!CanFlipNoVSync())
	{
		LOGFILE::Output
		(
			"DIRECTDRAW: warning, this device is not capable of flipping surfaces without waiting for VBlank! "
			"performance may go down!"
		);
	}

	if (!Is8BitModeSupported)
	{
		LOGFILE::Output
		(
			"DIRECTDRAW: warning, this device doesn't support palettes with 256 entries for 8bit modes!"
		);
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
// create the front and back surfaces
////////////////////////////////////////////////////////////////////////////////////////

VOID DIRECTDRAW::CreateScreenBuffers()
{
	PDX_ASSERT( device && !FrontBuffer && !BackBuffer && !NesBuffer && !clipper);

	DIRECTX::InitStruct(FrontDesc);

	FrontDesc.dwFlags = DDSD_CAPS;
	FrontDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	if (!windowed)
	{
		FrontDesc.dwFlags |= DDSD_BACKBUFFERCOUNT;
		FrontDesc.ddsCaps.dwCaps |= DDSCAPS_FLIP|DDSCAPS_COMPLEX;
		FrontDesc.dwBackBufferCount = 2;

		if ((HalCaps.dwCaps & DDCAPS_BLTSTRETCH) || (HelCaps.dwCaps & DDCAPS_BLTSTRETCH))
		{
			LOGFILE::Output("DIRECTDRAW: creating the primary surface in video memory");
			FrontDesc.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
		}
		else
		{
			LOGFILE::Output("DIRECTDRAW: creating the primary surface in system memory");
			FrontDesc.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
		}
	}
	else
	{
		LOGFILE::Output("DIRECTDRAW: creating the primary surface");
	}
	
	if (FAILED(device->CreateSurface(&FrontDesc,&FrontBuffer,NULL)) || !FrontBuffer)
	{
		if (!windowed)
		{
			LOGFILE::Output("DIRECTDRAW: primary surface creation failed, resolving to use only one backbuffer..");

			FrontDesc.dwBackBufferCount = 1;

			if (FAILED(device->CreateSurface(&FrontDesc,&FrontBuffer,NULL)) || !FrontBuffer)
			{
				PDXSTRING log( "DIRECTDRAW: primary surface creation failed, trying with " );

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

				LOGFILE::Output( log );
			}
		}

		if (windowed || FAILED(device->CreateSurface(&FrontDesc,&FrontBuffer,NULL)) || !FrontBuffer)
			throw ("IDirectDraw7::CreateSurface() failed!");
	}

	if (FAILED(FrontBuffer->GetSurfaceDesc(&FrontDesc)))
		throw ("IDirectDrawSurface7::GetSurfaceDesc() failed!");

	const BOOL InvalidBitDepth =
	(
   		(FrontDesc.ddpfPixelFormat.dwRGBBitCount !=  8) && 
     	(FrontDesc.ddpfPixelFormat.dwRGBBitCount != 16) && 
       	(FrontDesc.ddpfPixelFormat.dwRGBBitCount != 32)
	);

	if (InvalidBitDepth)
		throw (EXCEPTION(IDS_VIDEO_INVALID_DESKTOP_MODE));

	if (!windowed)
	{
		DDSCAPS2 caps2;
		PDXMemZero( caps2 );

		caps2.dwCaps = DDSCAPS_BACKBUFFER;

		if (FAILED(FrontBuffer->GetAttachedSurface(&caps2,&BackBuffer)) || !BackBuffer)
			throw ("IDirectDrawSurface7::GetAttachedBuffer() failed!");

		BackBuffer->AddRef();
		BackDesc.dwSize = sizeof(BackDesc);

		if (FAILED(BackBuffer->GetSurfaceDesc(&BackDesc)))
			throw ("IDirectDrawSurface7::GetSurfaceDesc() failed!");

		ClearSurface( BackBuffer );
	}

	if (FrontDesc.ddpfPixelFormat.dwRGBBitCount == 8)
	{
		LPDIRECTDRAWPALETTE palette = NULL;

		{
			PALETTEENTRY entries[256];

			HDC hDC = GetDC(NULL);

			if (hDC)
			{
     			::GetSystemPaletteEntries( hDC, 0, 10, entries + 0 );
     			::GetSystemPaletteEntries( hDC, 246, 256, entries + 246 );
     			ReleaseDC(NULL,hDC);
			}
			else
			{
				PDXMemZero( entries + 0, 10 );
				PDXMemZero( entries + 246, 10 );
				LOGFILE::Output("DIRECTDRAW: warning, GetDC() failed, colors in fullscreen may not be accurate!");
			}

			for (UINT i=0; i < 256; ++i)
				entries[i].peFlags = PC_NOCOLLAPSE|PC_RESERVED;				 

			if (FAILED(device->CreatePalette(DDPCAPS_8BIT|DDPCAPS_ALLOW256,entries,&palette,NULL)) || !palette)
				throw ("IDirectDraw7::CreatePalette() failed!");
		}

		if (FAILED(FrontBuffer->SetPalette(palette)))
			throw ("IDirectDrawSurface7::SetPalette() failed!");

		palette->Release();
	}

	CreateClipper();
}

////////////////////////////////////////////////////////////////////////////////////////
// create a clipper
////////////////////////////////////////////////////////////////////////////////////////

VOID DIRECTDRAW::CreateClipper()
{
	PDX_ASSERT(hWnd && device && FrontBuffer && !clipper);

	if (FAILED(device->CreateClipper(0,&clipper,NULL)) || !clipper)
		throw ("IDirectDraw7::CreateClipper() failed!");

	if (FAILED(clipper->SetHWnd(0,hWnd)))
	{
		DIRECTX::Release(clipper,TRUE);
		throw ("IDirectDrawClipper::SetHWnd() failed!");
	}

	if (windowed || GDIMode)
	{
		if (FAILED(FrontBuffer->SetClipper(clipper)))
		{
			DIRECTX::Release(clipper,TRUE);
			throw ("IDirectDrawSurface7::SetClipper() failed!");
		}
	}

	if (!windowed && GDIMode)
	{
		if (FAILED(device->FlipToGDISurface()))
			throw ("IDirectDraw7::FlipToGDISurface() failed!");
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// create the nes surface
////////////////////////////////////////////////////////////////////////////////////////

VOID DIRECTDRAW::CreateNesBuffer()
{
	PDX_ASSERT(device && FrontBuffer);

	ready = FALSE;

	DDSURFACEDESC2 desc;
	DIRECTX::InitStruct(desc);

	desc.dwFlags        = DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT|DDSD_PIXELFORMAT;
	desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN|DDSCAPS_SYSTEMMEMORY;
	desc.dwWidth        = NES_WIDTH;
	desc.dwHeight       = NES_HEIGHT;
	
	if (UseVRam && (FrontDesc.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY))
	{
		desc.ddsCaps.dwCaps &= ~DDSCAPS_SYSTEMMEMORY;
		desc.ddsCaps.dwCaps |=  DDSCAPS_VIDEOMEMORY;
	}

	desc.ddpfPixelFormat.dwSize        = sizeof(desc.ddpfPixelFormat);
	desc.ddpfPixelFormat.dwFlags       = FrontDesc.ddpfPixelFormat.dwFlags;
	desc.ddpfPixelFormat.dwRGBBitCount = FrontDesc.ddpfPixelFormat.dwRGBBitCount;
	desc.ddpfPixelFormat.dwRBitMask    = FrontDesc.ddpfPixelFormat.dwRBitMask;
	desc.ddpfPixelFormat.dwGBitMask    = FrontDesc.ddpfPixelFormat.dwGBitMask;
	desc.ddpfPixelFormat.dwBBitMask    = FrontDesc.ddpfPixelFormat.dwBBitMask;

	PaletteChanged = TRUE;
	format.PaletteChanged = TRUE;
	ScaleFactor = 0;
	rcNesBlt = rcNes;
	Use2xSaI = FALSE;

	if (FrontDesc.ddpfPixelFormat.dwRGBBitCount != 8)
	{
		PDX_ASSERT
		( 
		    windowed ||
			(
     			(rcScreen.right - rcScreen.left) <= DisplayMode.width && 
      			(rcScreen.right - rcScreen.left) >= 1 &&
       			(rcScreen.bottom - rcScreen.top) <= DisplayMode.height &&
     			(rcScreen.bottom - rcScreen.top) >= 1
			)
		);

		const INT width = rcScreen.right - rcScreen.left;
		const INT height = rcScreen.bottom - rcScreen.top;

		switch (ScreenEffect)
		{
     		case SCREENEFFECT_SCANLINES:
			{
				if ((rcNes.right-rcNes.left) * 2 <= width && (rcNes.bottom-rcNes.top) * 2 <= height)
				{
					++ScaleFactor;
					desc.dwWidth      *= 2;
					desc.dwHeight     *= 2;
					rcNesBlt.top    *= 2;			
					rcNesBlt.right  *= 2;
					rcNesBlt.bottom *= 2;
					rcNesBlt.left   *= 2;
				}
				break;
			}
  
			case SCREENEFFECT_TV:
			{
				ScaleFactor        = 1;
				desc.dwWidth      *= 2;
				desc.dwHeight     *= 2;
				rcNesBlt.top    *= 2;			
				rcNesBlt.right  *= 2;
				rcNesBlt.bottom *= 2;
				rcNesBlt.left   *= 2;
				break;
			}

			case SCREENEFFECT_2XSAI:
			case SCREENEFFECT_SUPER_2XSAI:
			case SCREENEFFECT_SUPER_EAGLE:
			{
				if ((rcNes.right-rcNes.left) * 2 <= width && (rcNes.bottom-rcNes.top) * 2 <= height)
				{
					++ScaleFactor;

					desc.dwWidth   *= 2;
					desc.dwHeight  *= 2;

					rcNesBlt.top    *= 2;			
					rcNesBlt.right  *= 2;
					rcNesBlt.bottom *= 2;
					rcNesBlt.left   *= 2;

					Use2xSaI = TRUE;
				}
				break;
			}
		}
	}

	const BOOL NotTheSame = 
	(
	    !NesBuffer ||
		desc.ddsCaps.dwCaps != NesDesc.ddsCaps.dwCaps ||
		desc.dwWidth != NesDesc.dwWidth ||
		desc.dwHeight != NesDesc.dwHeight ||
		desc.ddpfPixelFormat.dwFlags != NesDesc.ddpfPixelFormat.dwFlags ||
      	desc.ddpfPixelFormat.dwRGBBitCount != NesDesc.ddpfPixelFormat.dwRGBBitCount ||
       	desc.ddpfPixelFormat.dwRBitMask != NesDesc.ddpfPixelFormat.dwRBitMask ||
     	desc.ddpfPixelFormat.dwGBitMask != NesDesc.ddpfPixelFormat.dwGBitMask ||
     	desc.ddpfPixelFormat.dwBBitMask != NesDesc.ddpfPixelFormat.dwBBitMask
	);

	if (NotTheSame)
	{
		DIRECTX::Release(NesBuffer,TRUE);

		{
			PDXSTRING log;

			log  = "DIRECTDRAW: creating the nes render target surface in ";
			log += (UseVRam ? "video memory" : "system memory");
			LOGFILE::Output( log );

			log  = "DIRECTDRAW: surface dimensions (";
			log << desc.dwWidth << "," << desc.dwHeight << ")";
			LOGFILE::Output( log );

			log  = "DIRECTDRAW: source blit dimensions (";
			log << rcNesBlt.left << "," << rcNesBlt.top << ") - (" << rcNesBlt.right << "," << rcNesBlt.bottom << ")";
			LOGFILE::Output( log );

			log  = "DIRECTDRAW: target blit dimensions (";
			log << rcScreen.left << "," << rcScreen.top << ") - (" << rcScreen.right << "," << rcScreen.bottom << ")";
			LOGFILE::Output( log );

			log = "DIRECTDRAW: filter - ";

			switch (ScreenEffect)
			{
     			case SCREENEFFECT_SCANLINES:   log += "Scanlines";   break;
     			case SCREENEFFECT_TV:		   log += "TV-Mode";     break;
    			case SCREENEFFECT_2XSAI:	   log += "2xSaI";       break;
      			case SCREENEFFECT_SUPER_2XSAI: log += "Super 2xSaI"; break;
      			case SCREENEFFECT_SUPER_EAGLE: log += "Super Eagle"; break;
				default:					   log += "none";        break;
			}

			LOGFILE::Output( log );

			log = "DIRECTDRAW: scale factor - ";
			log << ScaleFactor;
			LOGFILE::Output( log );
		}

		if (FAILED(device->CreateSurface(&desc,&NesBuffer,NULL)) || !NesBuffer)
		{
			PDXSTRING log("DIRECTDRAW: nes render target surface creation failed, trying with ");

			if (desc.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY)
			{
				log += "system memory";
				desc.ddsCaps.dwCaps &= ~DDSCAPS_VIDEOMEMORY;
				desc.ddsCaps.dwCaps |=  DDSCAPS_SYSTEMMEMORY;
			}
			else
			{
				log += "video memory";
				desc.ddsCaps.dwCaps &= ~DDSCAPS_SYSTEMMEMORY;
				desc.ddsCaps.dwCaps |=  DDSCAPS_VIDEOMEMORY;
			}

			LOGFILE::Output( log );

			if (FAILED(device->CreateSurface(&desc,&NesBuffer,NULL)) || !NesBuffer)
				throw ("IDirectDraw7::CreateSurface() failed!");
		}

		NesDesc.dwSize = sizeof(NesDesc);

		if (FAILED(NesBuffer->GetSurfaceDesc(&NesDesc)))
			throw ("IDirectDrawSurface7::GetSurfaceDesc() failed!");

		NesDesc.ddsCaps.dwCaps = desc.ddsCaps.dwCaps;
	}

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

	ready = CheckReady();

	if (NotTheSame)
		UpdateNesBuffer();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID DIRECTDRAW::UpdateNesBuffer()
{
	if (format.palette && PDX_SUCCEEDED(Lock(format)))
		Unlock();
	else
		ClearSurface(NesBuffer);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

DWORD DIRECTDRAW::GetMonitorFrequency() const
{
	PDX_ASSERT(device);

	{
		// #1 try

		DWORD frequency = 0;

		if (SUCCEEDED(device->GetMonitorFrequency(&frequency)) && frequency > 1 && frequency < 512)
			return frequency;
	}

	{
		// #2 try

		HDC hDC = GetDC( NULL );

		if (hDC)
		{
			const INT frequency = ::GetDeviceCaps( hDC, VREFRESH );
			::ReleaseDC( NULL, hDC );

			if (frequency > 1 && frequency < 512)
				return frequency;
		}
	}

	{
		// #3 try

		DDSURFACEDESC2 desc;
		DIRECTX::InitStruct(desc);

		if (SUCCEEDED(device->GetDisplayMode(&desc)) && desc.dwRefreshRate > 1 && desc.dwRefreshRate < 512)
			return desc.dwRefreshRate;
	}

	{
		// #4 try

		DEVMODE devmode;

		PDXMemZero(devmode);
		devmode.dmSize = sizeof(devmode);

		if (::EnumDisplaySettings(NULL,ENUM_CURRENT_SETTINGS,&devmode) && devmode.dmDisplayFrequency > 1 && devmode.dmDisplayFrequency < 512)
			return devmode.dmDisplayFrequency;

		PDXMemZero(devmode);
		devmode.dmSize = sizeof(devmode);

		if (::EnumDisplaySettings(NULL,ENUM_REGISTRY_SETTINGS,&devmode) && devmode.dmDisplayFrequency > 1 && devmode.dmDisplayFrequency < 512)
			return devmode.dmDisplayFrequency;
	}

	// ugh, screw this

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// switch to desktop mode
////////////////////////////////////////////////////////////////////////////////////////

VOID DIRECTDRAW::SwitchToWindowed(const RECT& rect,const BOOL force)
{
	PDX_ASSERT( device && hWnd );

	if (!device || !hWnd)
	{
		ready = FALSE;
		throw ("Internal error in DIRECTDRAW::SwitchToWindowed()!");
	}

	if (ready && windowed && !force)
	{
		UpdateScreenRect( rect );
		return;
	}

	PaletteChanged = TRUE;
	format.PaletteChanged = TRUE;

	ReleaseBuffers();

	if (!windowed)
	{
		windowed = TRUE;
		device->RestoreDisplayMode();
	}

	GDIMode = 0;

    device->SetCooperativeLevel( hWnd, DDSCL_NORMAL );

	CreateScreenBuffers();

	DisplayMode.width       = FrontDesc.dwWidth;
	DisplayMode.height      = FrontDesc.dwHeight;
	DisplayMode.bpp         = FrontDesc.ddpfPixelFormat.dwRGBBitCount;
	DisplayMode.RefreshRate = GetMonitorFrequency();

	if (DisplayMode.RefreshRate)
		LOGFILE::Output("DIRECTDRAW: refresh rate is at ",DisplayMode.RefreshRate," hz");
	else
		LOGFILE::Output("DIRECTDRAW: refresh rate couldn't be detected!");

	UpdateScreenRect( rect );

	DisplayMode.SetPixelFormat
	(
   		FrontDesc.ddpfPixelFormat.dwRBitMask,
   		FrontDesc.ddpfPixelFormat.dwGBitMask,
   		FrontDesc.ddpfPixelFormat.dwBBitMask
	);

	CreateNesBuffer();
	ready = CheckReady();
}

////////////////////////////////////////////////////////////////////////////////////////
// switch to fullscreen mode
////////////////////////////////////////////////////////////////////////////////////////

VOID DIRECTDRAW::SwitchToFullScreen(const UINT width,const UINT height,const UINT bpp,const RECT& rect)
{
	PDX_ASSERT( device && hWnd );

	ready = FALSE;

	if (!device || !hWnd)
		throw ("DIRECTDRAW: internal error");

	if (windowed || !FrontBuffer || DisplayMode.width != width || DisplayMode.height != height || DisplayMode.bpp != bpp)
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
			throw (EXCEPTION(IDS_VIDEO_NO_DISPLAYMODE));

		device->SetCooperativeLevel(hWnd,DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN|DDSCL_ALLOWREBOOT);

		LOGFILE::Output
		( 
   		   	PDXSTRING("DIRECTDRAW: setting display mode ") <<
			DisplayMode.width  << " * " <<
			DisplayMode.height << " * " <<
			DisplayMode.bpp << ", 60 hz"
		);

		if (FAILED(device->SetDisplayMode(DisplayMode.width,DisplayMode.height,DisplayMode.bpp,60,0)))
		{
			LOGFILE::Output("DIRECTDRAW: SetDisplayMode() failed, trying with default refresh rate");

			if (FAILED(device->SetDisplayMode(DisplayMode.width,DisplayMode.height,DisplayMode.bpp,0,0)))
				throw ("IDirectDraw7::SetDisplayMode() failed!");

			DisplayMode.RefreshRate = GetMonitorFrequency();

			if (DisplayMode.RefreshRate)
				LOGFILE::Output("DIRECTDRAW: refresh rate was found to be ", DisplayMode.RefreshRate, " hz");
			else
				LOGFILE::Output("DIRECTDRAW: warning, unknown refresh rate, performance may degrade");
		}
		else
		{
			DisplayMode.RefreshRate = 60;
		}

		if (DisplayMode.bpp == 8)
			LOGFILE::Output("DIRECTDRAW: warning, emulation of color de-emphasis is disabled for 8 bit modes");

		windowed = FALSE;
		CreateScreenBuffers();
	}

	rcScreen = rect;

	PDX_ASSERT
	( 
		(rcScreen.right - rcScreen.left) <= DisplayMode.width && 
		(rcScreen.right - rcScreen.left) >= 1 &&
		(rcScreen.bottom - rcScreen.top) <= DisplayMode.height &&
		(rcScreen.bottom - rcScreen.top) >= 1
	);

	CreateNesBuffer();
	ready = CheckReady();
}

////////////////////////////////////////////////////////////////////////////////////////
// enable/disable GDI output in fullscreen mode
////////////////////////////////////////////////////////////////////////////////////////

INT DIRECTDRAW::EnableGDI(const BOOL state)
{
	if (windowed || !ready)
		return 0;

	PDX_ASSERT(device && FrontBuffer);

	if (state)
	{
		if (!GDIMode++)
		{
			if (FrontBuffer->IsLost() == DDERR_SURFACELOST)
			{
				device->RestoreAllSurfaces();
				UpdateNesBuffer();
			}

			FrontBuffer->SetClipper(clipper);
			device->FlipToGDISurface();
			return +1;
		}
	}
	else
	{
		if (GDIMode && !--GDIMode)
		{
			if (FrontBuffer->IsLost() == DDERR_SURFACELOST)
			{
				device->RestoreAllSurfaces();
				UpdateNesBuffer();
			}

			FrontBuffer->SetClipper(NULL);
			return -1;
		}
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL DIRECTDRAW::ClearScreen(const BOOL wait,const BOOL ClearFrontBuffer)
{
	PDX_ASSERT(ready && bool(ready) == CheckReady());

	LPDIRECTDRAWSURFACE7 surface = (ClearFrontBuffer ? FrontBuffer : windowed ? NesBuffer : BackBuffer);

	DDBLTFX BltFX;
	BltFX.dwSize = sizeof(BltFX);
	BltFX.dwFillColor = 0;

	const DWORD flags = 
	(
     	wait ? (DDBLT_COLORFILL|DDBLT_ASYNC|DDBLT_WAIT) : (DDBLT_COLORFILL|DDBLT_ASYNC|DDBLT_DONOTWAIT)
	);

	HRESULT hResult;

	if (FAILED(hResult=surface->Blt( NULL, NULL, NULL, flags, &BltFX )))
	{
		if (hResult == DDERR_WASSTILLDRAWING || !Validate( hResult ))
			return FALSE;

		if (FAILED(hResult=surface->Blt( NULL, NULL, NULL, flags & ~DDBLT_ASYNC, &BltFX )))
			return FALSE;
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
// clear any surface
////////////////////////////////////////////////////////////////////////////////////////

VOID DIRECTDRAW::ClearSurface(LPDIRECTDRAWSURFACE7 surface,const DWORD color)
{
	DDBLTFX BltFX;
	BltFX.dwSize = sizeof(BltFX);
	BltFX.dwFillColor = color;

	HRESULT hResult;

	if (FAILED(surface->Blt(NULL,NULL,NULL,DDBLT_COLORFILL|DDBLT_WAIT|DDBLT_ASYNC,&BltFX)))
		if (FAILED(hResult=surface->Blt(NULL,NULL,NULL,DDBLT_COLORFILL|DDBLT_WAIT,&BltFX)) && Validate(hResult))
			surface->Blt(NULL,NULL,NULL,DDBLT_COLORFILL|DDBLT_WAIT,&BltFX);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID DIRECTDRAW::ClearNesScreen()
{ 
	PDX_ASSERT(ready && bool(ready) == CheckReady());

	ClearSurface(NesBuffer); 
	PixelData->Reset();
}

////////////////////////////////////////////////////////////////////////////////////////
// destroyer
////////////////////////////////////////////////////////////////////////////////////////

VOID DIRECTDRAW::Destroy()
{
	ReleaseBuffers();

	if (device)
	{
		if (!windowed)
			device->RestoreDisplayMode();

		if (hWnd)
			device->SetCooperativeLevel(hWnd,DDSCL_NORMAL);
	}

	DIRECTX::Release(device,TRUE);
	PDXMemZero( guid );
}

////////////////////////////////////////////////////////////////////////////////////////
// lock the nes surface
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTDRAW::Lock(NES::IO::GFX& PixelMap)
{
	PDX_ASSERT( ready && bool(ready) == CheckReady() );

	if (PixelMap.PaletteChanged || PaletteChanged)
	{
		PaletteChanged = FALSE;
		PixelMap.PaletteChanged = FALSE;
		UpdatePalette( PixelMap.palette );
	}

	PixelMap.pixels = PixelData->buffer;

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
// unlock the nes surface
////////////////////////////////////////////////////////////////////////////////////////

PDXRESULT DIRECTDRAW::Unlock()
{
	DDSURFACEDESC2 desc;	
	
	if (LockNesBuffer(desc))
	{
		switch (NesDesc.ddpfPixelFormat.dwRGBBitCount)
		{
	       	case  8: BltNesScreen( PDX_CAST(U8*,desc.lpSurface),  desc.lPitch / sizeof(U8)  ); break;
       		case 16: BltNesScreen( PDX_CAST(U16*,desc.lpSurface), desc.lPitch / sizeof(U16) ); break;
			default: BltNesScreen( PDX_CAST(U32*,desc.lpSurface), desc.lPitch / sizeof(U32) ); break;
		}

		NesBuffer->Unlock(NULL);

		if (!windowed && (rcScreen.right-rcScreen.left) > 0 && (rcScreen.bottom-rcScreen.top) > 0)
			DrawNesBuffer();
	}

	return PDX_OK;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID DIRECTDRAW::UpdatePalette(const U8* const PDX_RESTRICT input)
{
	PDX_ASSERT( input );

	if (NesDesc.ddpfPixelFormat.dwRGBBitCount == 8)
	{
		LPDIRECTDRAWPALETTE palette = NULL;
			
		if (FAILED(FrontBuffer->GetPalette(&palette)) || !palette)
			throw ("IDirectDrawSurface7::GetPalette() failed!");

		{
			PALETTEENTRY entries[236];

			for (UINT i=0; i < 236; ++i)
			{
				entries[i].peRed   = input[(i*3) + 0];
				entries[i].peGreen = input[(i*3) + 1];
				entries[i].peBlue  = input[(i*3) + 2];
				entries[i].peFlags = PC_NOCOLLAPSE|PC_RESERVED;
			}

			if (FAILED(palette->SetEntries(0,10,236,entries)))
				throw ("IDirectDrawPalette::SetEntries() failed!");
		}

		palette->Release();
	}
	else
	{
		U32 (*const PDX_RESTRICT palette)[3] = PixelData->palette;

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
}

////////////////////////////////////////////////////////////////////////////////////////
// blit to the nes surface 
////////////////////////////////////////////////////////////////////////////////////////

VOID DIRECTDRAW::BltNesScreen(U8* PDX_RESTRICT dst,const LONG pitch)
{
	const NES::IO::GFX::PIXEL* PDX_RESTRICT src = PixelData->buffer;

	if (pitch == NES_WIDTH)
	{
		for (UINT i=0; i < NES_WIDTH * NES_HEIGHT; ++i)
			dst[i] = U8(src[i] + 10);
	}
	else
	{
		for (UINT y=0; y < NES_HEIGHT; ++y)
		{
			for (UINT x=0; x < NES_WIDTH; ++x)
				dst[x] = U8(src[x] + 10);

			dst += pitch;
			src += NES_WIDTH;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// blit to the nes surface 
////////////////////////////////////////////////////////////////////////////////////////

template<class T> 
VOID DIRECTDRAW::BltNesScreen(T* PDX_RESTRICT const dst,const LONG pitch)
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
VOID DIRECTDRAW::BltNesScreenAligned(T* PDX_RESTRICT const dst)
{
	const NES::IO::GFX::PIXEL* const PDX_RESTRICT src = PixelData->buffer;
	const U32 (*const PDX_RESTRICT palette)[3] = PixelData->palette;

	for (UINT i=0; i < PIXEL_BUFFER_LENGTH; ++i)
	{
		dst[i] = T
		(
	     	palette[src[i]][0] + 
			palette[src[i]][1] + 
			palette[src[i]][2]
		);
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// blit to the nes surface 
////////////////////////////////////////////////////////////////////////////////////////

template<class T>
VOID DIRECTDRAW::BltNesScreenUnaligned(T* PDX_RESTRICT dst,const LONG pitch)
{
	const NES::IO::GFX::PIXEL* PDX_RESTRICT src = PixelData->buffer;
	const U32 (*const PDX_RESTRICT palette)[3] = PixelData->palette;

	for (UINT y=0; y < NES_HEIGHT; ++y)
	{
		for (UINT x=0; x < NES_WIDTH; ++x)
		{
			dst[x] = T
			(
		     	palette[src[x]][0] + 
				palette[src[x]][1] + 
				palette[src[x]][2]
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
VOID DIRECTDRAW::BltNesScreenTV(T* PDX_RESTRICT dst,const LONG pitch)
{
	const NES::IO::GFX::PIXEL* const PDX_RESTRICT src = PixelData->buffer;
	const U32 (*const palette)[3] = PixelData->palette;

	const U32* pixel;
	const U32* rgb;

	for (UINT y=0; y < PIXEL_BUFFER_LENGTH; y += NES_WIDTH, dst += pitch)
	{
		for (UINT x=0,i=0; ; ++x, i+=2)
		{
			const UINT p = y + x;
			pixel = palette[src[p]];

			dst[i+0] = T
			(
		     	pixel[0] + 
				pixel[1] + 
				pixel[2]
			);

			rgb = palette[src[p+1]];

			dst[i+1] = T
			(
		     	(((pixel[0] + rgb[0]) >> 1) & DisplayMode.rMask) + 
				(((pixel[1] + rgb[1]) >> 1) & DisplayMode.gMask) + 
				(((pixel[2] + rgb[2]) >> 1) & DisplayMode.bMask)
			);

			rgb = palette[src[p+(NES_WIDTH+1)]];

			dst[i+pitch+0] = T
			(
		     	((((pixel[0] + rgb[0]) * 11) >> 5) & DisplayMode.rMask) + 
				((((pixel[1] + rgb[1]) * 11) >> 5) & DisplayMode.gMask) + 
				((((pixel[2] + rgb[2]) * 11) >> 5) & DisplayMode.bMask)
			);

			if (i == NES_WIDTH_2-4)
				break;

			rgb = palette[src[p+(NES_WIDTH+1+1)]];

			dst[i+pitch+1] = T
			(
		     	((((pixel[0] + rgb[0]) * 11) >> 5) & DisplayMode.rMask) + 
				((((pixel[1] + rgb[1]) * 11) >> 5) & DisplayMode.gMask) + 
				((((pixel[2] + rgb[2]) * 11) >> 5) & DisplayMode.bMask)
			);
		}

		const U32* const data = palette[src[y+(NES_WIDTH-1)]];

		T value
		(
       		data[0] + 
       		data[1] + 
    		data[2] 
		);

		dst[NES_WIDTH_2-2] = value;
		dst[NES_WIDTH_2-1] = value;

		dst += pitch;
  		dst[NES_WIDTH_2-3] = T
		(
		 	((((pixel[0] + rgb[0]) * 11) >> 5) & DisplayMode.rMask) + 
			((((pixel[1] + rgb[1]) * 11) >> 5) & DisplayMode.gMask) + 
			((((pixel[2] + rgb[2]) * 11) >> 5) & DisplayMode.bMask)
		);
  
		value =	T
		(
      		((((data[0] + rgb[0]) * 11) >> 5) & DisplayMode.rMask) + 
       		((((data[1] + rgb[1]) * 11) >> 5) & DisplayMode.gMask) + 
     		((((data[2] + rgb[2]) * 11) >> 5) & DisplayMode.bMask) 
		);

		dst[NES_WIDTH_2-2] = value;
  		dst[NES_WIDTH_2-1] = value;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

template<class T> 
VOID DIRECTDRAW::BltNesScreenScanLines1(T* PDX_RESTRICT dst,const LONG pitch)
{
	const NES::IO::GFX::PIXEL* PDX_RESTRICT src = PixelData->buffer;
	const NES::IO::GFX::PIXEL* const end = PixelData->buffer + PIXEL_BUFFER_LENGTH;
	const U32 (*const PDX_RESTRICT palette)[3] = PixelData->palette;

	do
	{
		for (UINT x=0,i=0; x < NES_WIDTH; ++x, i+=2)
		{
			const T value
			(
    			(palette[src[x]][0] & DisplayMode.rMask) + 
     			(palette[src[x]][1] & DisplayMode.gMask) + 
    			(palette[src[x]][2] & DisplayMode.bMask)
			);

			dst[i+0] = value;
			dst[i+1] = value;
		}

		dst += pitch;

		for (UINT x=0,i=0; x < NES_WIDTH; ++x, i+=2)
		{
			const T value
			(
     			((palette[src[x]][0] >> 2) & DisplayMode.rMask) + 
     			((palette[src[x]][1] >> 2) & DisplayMode.gMask) + 
     			((palette[src[x]][2] >> 2) & DisplayMode.bMask)
			);

			dst[i+0] = value;
			dst[i+1] = value;
		}

		dst += pitch;
		src += NES_WIDTH;
	}
	while (src != end);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

template<class T> 
VOID DIRECTDRAW::BltNesScreenScanLinesFactor(T* PDX_RESTRICT dst,const LONG pitch)
{
	const UINT yScale = NES_HEIGHT << ScaleFactor;
	const UINT xScale = NES_WIDTH << ScaleFactor;
	const UINT ScanStep = PDX_MAX(1,ScaleFactor);

	const NES::IO::GFX::PIXEL* const offset = PixelData->buffer;
	const U32 (*const palette)[3] = PixelData->palette;

	for (UINT y=0; y < yScale; ++y)
	{
		const NES::IO::GFX::PIXEL* PDX_RESTRICT const src = offset + ((y >> ScaleFactor) * NES_WIDTH);

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
VOID DIRECTDRAW::BltNesScreen2xSaI(F2XAI F2xSaI,T* PDX_RESTRICT dst,const LONG pitch)
{
	PDX_ASSERT( NesDesc.dwWidth == NES_WIDTH_2 && NesDesc.dwHeight == NES_HEIGHT_2 );

	const NES::IO::GFX::PIXEL* const PDX_RESTRICT src = PixelData->buffer;
	U16* const PDX_RESTRICT effect = PixelData->effect;
	const U32 (*const PDX_RESTRICT palette)[3] = PixelData->palette;

	for (UINT i=0; i < EFFECT_BUFFER_LENGTH; ++i)
	{
		effect[i] = U16
		(
	     	palette[src[i]][0] + 
			palette[src[i]][1] + 
			palette[src[i]][2]
		);
	}

	if (IsNesBuffer2xSaI)
	{
		F2xSaI
		( 
			PDX_CAST(U8*,effect), 
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
			PDX_CAST(U8*,effect), 
			NES_WIDTH * sizeof(U16), 
			NULL, 
			PDX_CAST(U8*,PixelData->conv), 
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
			const U16* const PDX_RESTRICT src = PixelData->conv;

			for (UINT i=0; i < CONV_BUFFER_LENGTH; ++i)
			{
				const T pixel(src[i]);

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
			const U16* PDX_RESTRICT src = PixelData->conv;

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
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL DIRECTDRAW::Validate(const HRESULT hResult)
{
	PDX_ASSERT(device);

	if (hResult == DDERR_SURFACELOST)
	{
		if (SUCCEEDED(device->RestoreAllSurfaces()))
			return TRUE;
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
// lock the nes surface
////////////////////////////////////////////////////////////////////////////////////////

BOOL DIRECTDRAW::LockNesBuffer(DDSURFACEDESC2& desc)
{
	PDX_ASSERT( ready && bool(ready) == CheckReady() );

	desc.dwSize = sizeof(desc);
	desc.dwFlags = 0;

	HRESULT hResult;

	if (FAILED(hResult=NesBuffer->Lock(NULL,&desc,DDLOCK_NOSYSLOCK|DDLOCK_WAIT|DDLOCK_WRITEONLY,NULL)))
	{
		if (!Validate(hResult))
			return FALSE;

		if (FAILED(NesBuffer->Lock(NULL,&desc,DDLOCK_NOSYSLOCK|DDLOCK_WAIT|DDLOCK_WRITEONLY,NULL)))
			throw ("IDirectDrawSurface7::Lock() failed!");
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
// release the prisoners
////////////////////////////////////////////////////////////////////////////////////////

VOID DIRECTDRAW::ReleaseBuffers()
{
	ready = FALSE;
	DIRECTX::Release(clipper);
	DIRECTX::Release(NesBuffer,TRUE);
	DIRECTX::Release(BackBuffer);
	DIRECTX::Release(FrontBuffer,TRUE);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID DIRECTDRAW::DrawNesBuffer()
{
	PDX_ASSERT(ready && BackBuffer && NesBuffer && !windowed);

	HRESULT hResult;

	if (FAILED(BackBuffer->Blt(&rcScreen,NesBuffer,&rcNesBlt,DDBLT_WAIT|DDBLT_ASYNC,NULL)))
		if (FAILED(hResult=BackBuffer->Blt(&rcScreen,NesBuffer,&rcNesBlt,DDBLT_WAIT,NULL)) && Validate(hResult))
			BackBuffer->Blt(&rcScreen,NesBuffer,&rcNesBlt,DDBLT_WAIT,NULL);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID DIRECTDRAW::RedrawNesScreen()
{
	PDX_ASSERT( bool(ready) == CheckReady() );

	if (ready)
	{
		if (windowed)
		{
			UpdateNesBuffer();
		}
		else
		{
			DrawNesBuffer();

			HRESULT hResult;

			if (FAILED(hResult=BackBuffer->Blt(&rcScreen,NesBuffer,&rcNesBlt,DDBLT_WAIT,NULL)) && Validate(hResult))
				BackBuffer->Blt(&rcScreen,NesBuffer,&rcNesBlt,DDBLT_WAIT,NULL);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

VOID DIRECTDRAW::Repaint()
{
	PDX_ASSERT( bool(ready) == CheckReady() );

	if (ready)
	{
		HRESULT hResult;

		if (windowed)
		{
			if ((rcScreen.right - rcScreen.left) > 0 && (rcScreen.bottom - rcScreen.top) > 0)
			{
				if (FAILED(hResult=FrontBuffer->Blt(&rcScreen,NesBuffer,&rcNesBlt,DDBLT_WAIT,NULL)) && Validate(hResult))
					FrontBuffer->Blt(&rcScreen,NesBuffer,&rcNesBlt,DDBLT_WAIT,NULL);
			}
		}
		else
		{
			if (FAILED(hResult=FrontBuffer->Blt(NULL,BackBuffer,NULL,DDBLT_WAIT,NULL) && Validate(hResult)))
				FrontBuffer->Blt(NULL,BackBuffer,NULL,DDBLT_WAIT,NULL);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

BOOL DIRECTDRAW::Present()
{
	PDX_ASSERT( ready && bool(ready) == CheckReady() );

	if ((rcScreen.right-rcScreen.left) <= 0 || (rcScreen.bottom-rcScreen.top) <= 0)
		return TRUE;

	DWORD flags = DDBLT_WAIT|DDBLT_ASYNC;

	if (FrameLatency++ == MAX_FRAME_LATENCY)
	{
		FrameLatency = 0;
		flags = DDBLT_WAIT;
	}

	HRESULT hResult;

	if (windowed)
	{
		if (FAILED(FrontBuffer->Blt(&rcScreen,NesBuffer,&rcNesBlt,flags,NULL)))
			if (FAILED(hResult=FrontBuffer->Blt(&rcScreen,NesBuffer,&rcNesBlt,DDBLT_WAIT,NULL)))
				if (!Validate(hResult) || FAILED(FrontBuffer->Blt(&rcScreen,NesBuffer,&rcNesBlt,DDBLT_WAIT,NULL)))
					return FALSE;
	}
	else
	{
		if (GDIMode || DontFlip)
		{
			if (FAILED(FrontBuffer->Blt(NULL,BackBuffer,NULL,flags,NULL)))
				if (FAILED(hResult=FrontBuffer->Blt(NULL,BackBuffer,NULL,DDBLT_WAIT,NULL)))
					if (!Validate(hResult) || FAILED(FrontBuffer->Blt(NULL,BackBuffer,NULL,DDBLT_WAIT,NULL)))
       					return FALSE;
		}
		else
		{
			if (FAILED(hResult=FrontBuffer->Flip(NULL,FlipFlags)))
				if (!Validate(hResult) || FAILED(FrontBuffer->Flip(NULL,DDFLIP_WAIT)))
					return FALSE;
		}
	}

	return TRUE;
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
			if (!device || FAILED(device->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN,0)))
				return FALSE;
		}
  
		return TRUE;
	}

	if (CanFlipNoVSync())
	{
		FlipFlags = DDFLIP_WAIT|DDFLIP_NOVSYNC;
	}
	else
	{
		DontFlip = TRUE;
	}

	return FALSE;
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
