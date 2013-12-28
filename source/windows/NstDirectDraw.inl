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

inline NES::IO::GFX* DIRECTDRAW::GetFormat()
{ 
	return &format; 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline UINT DIRECTDRAW::GetPixel(const UINT x,const UINT y) const
{
	PDX_ASSERT(((y * NES_WIDTH) + x) < (NES_WIDTH * NES_HEIGHT));		
	return PixelData->buffer[(y * NES_WIDTH) + x];
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline VOID DIRECTDRAW::ClearScreen()
{ 
	PDX_ASSERT(ready && bool(ready) == CheckReady());
	ClearSurface( windowed ? NesBuffer : BackBuffer ); 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline BOOL DIRECTDRAW::IsGDI()      const { return GDIMode;  }
inline BOOL DIRECTDRAW::IsReady()    const { return ready;    }
inline BOOL DIRECTDRAW::IsWindowed() const { return windowed; }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline LPDIRECTDRAW7 DIRECTDRAW::GetDevice()
{ 
	return device; 
}

inline LPDIRECTDRAWSURFACE7 DIRECTDRAW::GetNesBuffer() const
{ 
	return NesBuffer; 
}

inline const DDSURFACEDESC2& DIRECTDRAW::GetNesDesc() const
{ 
	return NesDesc; 
}

inline const DDCAPS& DIRECTDRAW::GetHalCaps() const
{
	return HalCaps;
}

inline const DDCAPS& DIRECTDRAW::GetHelCaps() const
{
	return HelCaps;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline UINT DIRECTDRAW::GetScaleFactor() const
{ 
	return ScaleFactor; 
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline const RECT& DIRECTDRAW::GetScreenRect() const { return ScreenRect; }
inline const RECT& DIRECTDRAW::GetNesRect()    const { return NesRect;    }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline UINT DIRECTDRAW::GetDisplayWidth()  const { return DisplayMode.width;  }
inline UINT DIRECTDRAW::GetDisplayHeight() const { return DisplayMode.height; }
inline UINT DIRECTDRAW::GetDisplayBPP()    const { return DisplayMode.bpp;    }

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////

inline BOOL DIRECTDRAW::CanRenderWindowed() const
{ 
	return 
	(
    	(HalCaps.dwCaps2 & DDCAPS2_CANRENDERWINDOWED) ||
		(HelCaps.dwCaps2 & DDCAPS2_CANRENDERWINDOWED)
	); 
}

inline BOOL DIRECTDRAW::Is8BitModeSupported() const
{
	return 
	(
     	((HalCaps.dwPalCaps & DDPCAPS_8BIT) || (HelCaps.dwPalCaps & DDPCAPS_8BIT)) &&
		((HalCaps.dwPalCaps & DDPCAPS_ALLOW256) || (HelCaps.dwPalCaps & DDPCAPS_ALLOW256))
	);
}

inline BOOL DIRECTDRAW::CanFlipNoVSync() const
{
	return 
	(
     	(HalCaps.dwCaps2 & DDCAPS2_FLIPNOVSYNC) || 
		(HelCaps.dwCaps2 & DDCAPS2_FLIPNOVSYNC)
	);
}


