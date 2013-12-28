////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003-2006 Martin Freij
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

#pragma comment(lib,"d3d9")
#pragma comment(lib,"d3dx9")

#include "NstIoLog.hpp"
#include "NstDirect2D.hpp"
#include "NstApplicationException.hpp"

namespace Nestopia
{
	using DirectX::Direct2D;

	Direct2D::Mode::Mode(uint w,uint h,uint b)
	: width(w), height(h), bpp(b) {}

	bool Direct2D::Mode::operator == (const Mode& mode) const
	{
		return width == mode.width && height == mode.height && bpp == mode.bpp;
	}

	bool Direct2D::Mode::operator < (const Mode& mode) const
	{
		if ( width  < mode.width  ) return true;
		if ( width  > mode.width  ) return false;
		if ( height < mode.height ) return true;
		if ( height > mode.height ) return false;
		if ( bpp    < mode.bpp    ) return true;
		if ( bpp    > mode.bpp    ) return false;

		return false;
	}

	Direct2D::Base::Base()
	: com(Create()), adapters(EnumerateAdapters(com)) {}

	Direct2D::Base::~Base()
	{
		com.Release();
	}

	inline Direct2D::Base::operator IDirect3D9& () const
	{
		return com;
	}

	IDirect3D9& Direct2D::Base::Create()
	{
		if (IDirect3D9* com = ::Direct3DCreate9( D3D_SDK_VERSION ))
		{
			return *com;
		}
		else if (IDirect3D9* com = ::Direct3DCreate9( D3D9b_SDK_VERSION )) // unofficial, it may work, it may not work
		{
			return *com;
		}
		else
		{
			throw Application::Exception(_T("::Direct3DCreate9() failed! Upgrade to DirectX 9.0c or better!"));
		}
	}

	const Direct2D::Adapters Direct2D::Base::EnumerateAdapters(IDirect3D9& d3d)
	{
		NST_COMPILE_ASSERT( D3DADAPTER_DEFAULT == 0 );

		Io::Log() << "Direct3D: initializing..\r\n"; 

		Adapters adapters;

		for (uint ordinal=0, numAdapters=d3d.GetAdapterCount(); ordinal < numAdapters; ++ordinal)
		{
			D3DADAPTER_IDENTIFIER9 identifier;

			if (SUCCEEDED(d3d.GetAdapterIdentifier( ordinal, 0, &identifier )))
			{
				if (!adapters.empty() && adapters.back().guid == identifier.DeviceIdentifier)
					continue;

				Io::Log() << "Direct3D: enumerating device - name: " 
					      << (*identifier.Description ? identifier.Description : "unknown")
     				      << ", GUID: "
					      << System::Guid( identifier.DeviceIdentifier ).GetString()
					      << "\r\n";

				Adapter::Modes modes;

				for (uint format=0; format < 2; ++format)
				{
					const D3DFORMAT type = (format ? D3DFMT_X8R8G8B8 : D3DFMT_R5G6B5);

					for (uint mode=0, numModes=d3d.GetAdapterModeCount( ordinal, type ); mode < numModes; ++mode)
					{
						D3DDISPLAYMODE display;

						if (FAILED(d3d.EnumAdapterModes( ordinal, type, mode, &display )))
							continue;
						
						if (display.Width < Mode::MIN_WIDTH || display.Height < Mode::MIN_HEIGHT || display.RefreshRate > Mode::MAX_RATE)
							continue;
						
						modes.insert(Mode( display.Width, display.Height, format ? 32 : 16 )).first->rates.insert( (uchar) display.RefreshRate );
					}
				}

				if (modes.empty())
				{
					Io::Log() << "Direct3D: found no valid display mode, continuing enumeration..\r\n";
				}
				else
				{
					D3DCAPS9 caps;

					if (FAILED(d3d.GetDeviceCaps( ordinal, D3DDEVTYPE_HAL, &caps )))
					{
						if (FAILED(d3d.GetDeviceCaps( ordinal, D3DDEVTYPE_REF, &caps )))
						{
							Io::Log() << "Direct3D: warning, bogus device, continuing enumeration..\r\n";
							continue;
						}
						else
						{
							Io::Log() << "Direct3D: performance warning, this is a REF device only!\r\n";
						}
					}

					adapters.push_back( Adapter() );
					Adapter& adapter = adapters.back();

					adapter.guid            = identifier.DeviceIdentifier;
					adapter.name            = (*identifier.Description ? identifier.Description : "Unknown");
					adapter.ordinal         = ordinal;
					adapter.deviceType      = (caps.DeviceType != D3DDEVTYPE_REF ? Adapter::DEVICE_HAL : Adapter::DEVICE_HEL);
					adapter.maxScreenSize   = NST_MIN(caps.MaxTextureWidth,caps.MaxTextureHeight);
					adapter.videoMemScreen  = caps.Caps2 & D3DCAPS2_DYNAMICTEXTURES;
					adapter.intervalTwo     = caps.PresentationIntervals & D3DPRESENT_INTERVAL_TWO;
					adapter.intervalThree   = caps.PresentationIntervals & D3DPRESENT_INTERVAL_THREE;
					adapter.intervalFour    = caps.PresentationIntervals & D3DPRESENT_INTERVAL_FOUR;
					adapter.filters         = 0;
					adapter.modes           = modes;

					if ((caps.TextureFilterCaps & (D3DPTFILTERCAPS_MINFLINEAR|D3DPTFILTERCAPS_MAGFLINEAR)) == (D3DPTFILTERCAPS_MINFLINEAR|D3DPTFILTERCAPS_MAGFLINEAR))
						adapter.filters |= Adapter::FILTER_BILINEAR;

					Io::Log log;

					log << "Direct3D: dynamic textures: " << (adapter.videoMemScreen ? "supported\r\n" : "unsupported\r\n")
					    << "Direct3D: texture bilinear filtering: " << ((adapter.filters & Adapter::FILTER_BILINEAR) ? "supported\r\n" : "unsupported\r\n")
					    << "Direct3D: max texture dimensions: " << caps.MaxTextureWidth << 'x' << caps.MaxTextureHeight
						<< "\r\nDirect3D: vsync on every second refresh: " << (adapter.intervalTwo ? "supported\r\n" : "unsupported\r\n")
						<< "Direct3D: vsync on every third refresh: " << (adapter.intervalThree ? "supported\r\n" : "unsupported\r\n")
					    << "Direct3D: found " << modes.size() << " display modes\r\n"
				     	<< "Direct3D: supported monitor frequencies: ";
					
					Mode::Rates rates;

					for (Adapter::Modes::const_iterator it(modes.begin()), end(modes.end()); it != end; ++it)
						rates.insert( it->rates.begin(), it->rates.end() );

					for (Mode::Rates::const_iterator it(rates.begin()), end(rates.end());; )
					{
						log << uint(*it);

						if (++it != end)
						{
							log << "hz, ";
						}
						else
						{
							log << "hz\r\n";
							break;
						}
					}
				}
			}
		}

		if (adapters.empty())
			throw Application::Exception(_T("Found no valid display adapter!"));

		return adapters;
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("t", on)
    #endif

	uint Direct2D::Base::FormatToBpp(const D3DFORMAT format)
	{
		switch (format)
		{
			case D3DFMT_X8R8G8B8:
			case D3DFMT_X8B8G8R8:
			case D3DFMT_A8R8G8B8:
			case D3DFMT_A8B8G8R8:
			case D3DFMT_A2R10G10B10:
			case D3DFMT_A2B10G10R10:
				return 32;

			case D3DFMT_R5G6B5:
			case D3DFMT_X1R5G5B5:
			case D3DFMT_X4R4G4B4:
			case D3DFMT_A1R5G5B5:
			case D3DFMT_A4R4G4B4:
			case D3DFMT_A8R3G3B2:
				return 16;
		}
		
		return 0;
	}

	void Direct2D::Base::FormatToMask(const D3DFORMAT format,ulong& r,ulong& g,ulong& b)
	{
		switch (format)
		{
			case D3DFMT_X8R8G8B8: 
			case D3DFMT_A8R8G8B8:    r = 0x00FF0000UL; g = 0x0000FF00UL; b = 0x000000FFUL; break;
			case D3DFMT_X8B8G8R8: 
			case D3DFMT_A8B8G8R8:    r = 0x000000FFUL; g = 0x0000FF00UL; b = 0x00FF0000UL; break;
			case D3DFMT_A2R10G10B10: r = 0x3FF00000UL; g = 0x000FFC00UL; b = 0x000003FFUL; break;
			case D3DFMT_A2B10G10R10: r = 0x000003FFUL; g = 0x000FFC00UL; b = 0x3FF00000UL; break;
			case D3DFMT_R5G6B5:	     r = 0xF800UL;     g = 0x07E0UL;     b = 0x001FUL;     break;
			case D3DFMT_X1R5G5B5: 
			case D3DFMT_A1R5G5B5:    r = 0x7C00UL;     g = 0x03E0UL;     b = 0x001FUL;     break;
			case D3DFMT_X4R4G4B4: 
			case D3DFMT_A4R4G4B4:    r = 0x0F00UL;     g = 0x00F0UL;     b = 0x000FUL;     break;
			case D3DFMT_A8R3G3B2:    r = 0x00E0UL;     g = 0x001CUL;     b = 0x0003UL;     break;
			default:				 r = 0;			   g = 0;			 b = 0;			   break;
		}
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("", on)
    #endif

	Direct2D::Device::Fonts::Fonts()
	: width(0) {}

	void Direct2D::Device::Fonts::Font::Create(const Device& device)
	{
		uint height;
		bool arial;

		if (device.presentation.Windowed)
		{
			if (com)
				return;

			height = 30;
			arial = true;
		}
		else
		{
			arial = (device.presentation.BackBufferWidth > 320 && device.presentation.BackBufferHeight > 240);
			height = arial ? device.presentation.BackBufferHeight / 16 : 12;

			if (com)
			{
				Object::Pod<D3DXFONT_DESC> desc;
				com->GetDesc( &desc );

				if (desc.Height == int(height) && bool(desc.Width > 320 && desc.Height > 240) == arial)
					return;

				com.Release();
			}
		}

		::D3DXCreateFont
		(
			device.com,
			height,
			0,
			FW_NORMAL,
			1,
			FALSE,
			DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS,
			DEFAULT_QUALITY,
			DEFAULT_PITCH|FF_DONTCARE,
			arial ? _T("Arial") : _T("System"),
			&com
		);
	}

	uint Direct2D::Device::Fonts::Font::GetWidth() const
	{
		TEXTMETRIC metric;

		if (com && SUCCEEDED(com->GetTextMetrics( &metric )))
			return metric.tmAveCharWidth;
		else
			return 0;
	}

	void Direct2D::Device::Fonts::Font::Destroy()
	{
		length = 0;
		com.Release();
	}

	void Direct2D::Device::Fonts::Font::OnReset() const
	{
		if (com)
			com->OnResetDevice();
	}

	void Direct2D::Device::Fonts::Font::OnLost() const
	{
		if (com)
			com->OnLostDevice();
	}

	void Direct2D::Device::Fonts::Font::Update(const GenericString& newstring)
	{
		string = newstring;
		length = newstring.Length();

		if (length && com)
			com->PreloadText( string.Ptr(), string.Length() );
	}

	void Direct2D::Device::Fonts::Create(const Device& device)
	{
		nfo.Create( device );

		if (!device.presentation.Windowed)
		{
			fps.Create( device );
			msg.Create( device );
		}

		width = nfo.GetWidth();
	}

	void Direct2D::Device::Fonts::Destroy(const ibool newDevice)
	{
		width = 0;

		fps.Destroy();
		msg.Destroy();

		if (newDevice)
			nfo.Destroy();
		else
			nfo.OnReset();
	}

	void Direct2D::Device::Fonts::OnReset() const
	{
		fps.OnReset();
		msg.OnReset();
		nfo.OnReset();
	}

	void Direct2D::Device::Fonts::OnLost() const
	{
		fps.OnLost();
		msg.OnLost();
		nfo.OnLost();
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("t", on)
    #endif

	inline ibool Direct2D::Device::Fonts::Font::CanDraw() const
	{
		return length && com;
	}

	inline void Direct2D::Device::Fonts::Font::Draw(const D3DCOLOR color,const DWORD flags,Rect rect) const
	{
		com->DrawText( NULL, string.Ptr(), length, &rect, flags, color );
	}

	NST_FORCE_INLINE void Direct2D::Device::Fonts::Render(const D3DPRESENT_PARAMETERS& presentation,const uint state) const
	{
		const uint width = presentation.BackBufferWidth;
		const uint height = presentation.BackBufferHeight;

		if (!presentation.Windowed)
		{
			if ((state & RENDER_FPS) && fps.CanDraw())
			{
				for (uint i=0; i < 2; ++i)
				{
					fps.Draw
					( 
						i ? D3DCOLOR_ARGB(0xFF,0xA5,0xB5,0x40) : D3DCOLOR_ARGB(0xFF,0x2A,0x35,0x10),
						DT_SINGLELINE|TA_BOTTOM|TA_RIGHT|DT_NOCLIP, 
						Rect(width-31,height-31,width-i-3,height-i-3) 
					);
				}
			}

			if ((state & RENDER_MSG) && msg.CanDraw())
			{
				for (uint i=0; i < 2; ++i)
				{
					msg.Draw
					( 
						i ? D3DCOLOR_ARGB(0xFF,0xFF,0x20,0x20) : D3DCOLOR_ARGB(0xFF,0x20,0x20,0xA0),
						DT_SINGLELINE|TA_BOTTOM|TA_LEFT|DT_NOCLIP, 
						Rect(4-i,height-31,width,height-i-3) 
					);
				}
			}
		}

		if ((state & RENDER_NFO) && nfo.CanDraw())
		{
			for (uint i=0; i < 2; ++i)
			{
				nfo.Draw
				( 
     				i ? D3DCOLOR_ARGB(0xFF,0x20,0xFF,0x20) : D3DCOLOR_ARGB(0xFF,0x20,0x60,0x20),
					TA_TOP|TA_LEFT|DT_NOCLIP, 
					Rect(16-i,16-i,width,height) 
				);
			}
		}
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("", on)
    #endif

	Direct2D::Device::Timing::Timing()
	: 
	autoHz          (false),
	vsync           (false), 
	tripleBuffering (false),
	frameRate       (Mode::DEFAULT_RATE) 
	{
	}

	Direct2D::Device::Device(HWND const hWnd,const Base& base)
	{
		NST_ASSERT( hWnd );

		presentation.BackBufferWidth            = 0;
		presentation.BackBufferHeight           = 0;
		presentation.BackBufferFormat           = D3DFMT_UNKNOWN;
		presentation.BackBufferCount            = timing.tripleBuffering ? 2 : 1;
		presentation.MultiSampleType            = D3DMULTISAMPLE_NONE;
		presentation.MultiSampleQuality         = 0;
		presentation.SwapEffect                 = D3DSWAPEFFECT_DISCARD;
		presentation.hDeviceWindow              = hWnd;
		presentation.Windowed                   = TRUE;
		presentation.EnableAutoDepthStencil     = FALSE;
		presentation.AutoDepthStencilFormat     = D3DFMT_UNKNOWN;
		presentation.Flags                      = 0;
		presentation.FullScreen_RefreshRateInHz = 0;
		presentation.PresentationInterval       = D3DPRESENT_INTERVAL_IMMEDIATE;
		
		Create( base, base.GetAdapter(0) );
	}

	inline Direct2D::Device::operator IDirect3DDevice9& () const
	{
		return *com;
	}

	void Direct2D::Device::Create(IDirect3D9& d3d,const Adapter& adapter)
	{
		ordinal = adapter.ordinal;
		intervalTwo = adapter.intervalTwo;
		intervalThree = adapter.intervalThree;
		intervalFour = adapter.intervalFour;

		fonts.Destroy( TRUE );
		com.Release();

		uint buffers = presentation.BackBufferCount = timing.tripleBuffering ? 2 : 1;
		DWORD flags = D3DCREATE_PUREDEVICE|D3DCREATE_HARDWARE_VERTEXPROCESSING;

		for (;;)
		{
			const HRESULT hResult = d3d.CreateDevice
			(
				adapter.ordinal,
				adapter.deviceType == Adapter::DEVICE_HAL ? D3DDEVTYPE_HAL : D3DDEVTYPE_REF,
				presentation.hDeviceWindow,
				flags,
				&presentation,
				&com
			);

			if (SUCCEEDED(hResult))
			{
				break;
			}
			else if (hResult == D3DERR_DEVICELOST)
			{
				throw Application::Exception(_T("Can't start! Direct3D is busy!"));
			}
			else if (buffers != presentation.BackBufferCount)
			{
				buffers = presentation.BackBufferCount;
				Io::Log() << "Direct3D: Warning! IDirect3D9::CreateDevice() failed, retrying with one back-buffer only..\r\n";
			}
			else if (flags == (D3DCREATE_PUREDEVICE|D3DCREATE_HARDWARE_VERTEXPROCESSING))
			{
				flags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
				Io::Log() << "Direct3D: Warning! IDirect3D9::CreateDevice() failed, retrying without a pure device..\r\n";
			}
			else if (flags == D3DCREATE_HARDWARE_VERTEXPROCESSING)
			{
				flags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
				Io::Log() << "Direct3D: Warning! IDirect3D9::CreateDevice() failed, retrying with software vertex processing mode..\r\n";
			}
			else
			{
				throw Application::Exception(_T("IDirect3D9::CreateDevice() failed!"));
			}
		}

		Prepare();
		fonts.Create( *this );		

		Io::Log() << "Direct3D: creating " 
			      << (adapter.deviceType == Adapter::DEVICE_HAL ? "HAL device #" : "REF device #")
				  << adapter.ordinal 
				  << "\r\n";

		LogDisplaySwitch();
	}

	ibool Direct2D::Device::GetDisplayMode(D3DDISPLAYMODE& displayMode) const
	{
		ibool result = FALSE;

		IDirect3D9* base;

		if (SUCCEEDED(com->GetDirect3D( &base )))
		{
			result = SUCCEEDED(base->GetAdapterDisplayMode( ordinal, &displayMode ));
			base->Release();
		}

		return result;
	}

	uint Direct2D::Device::GetDesiredPresentationRate(const Mode& mode) const
	{
		if (presentation.Windowed)
		{
			return 0;
		}
		else if (timing.autoHz)
		{
			int match = INT_MAX;
			Mode::Rates::const_iterator close(mode.rates.begin());

			for (Mode::Rates::const_iterator it(mode.rates.end()), begin(mode.rates.begin());; )
			{
				--it;

				for (uint i=5; --i; )
				{
					int diff = int(timing.frameRate * i) - int(*it);

					if (diff == 0)
						return *it;

					if (diff < 0)
						diff = int(*it) - int(timing.frameRate * i);

					if (match > diff)
					{
						match = diff;
						close = it;
					}
				}

				if (it == begin)
					break;
			}

			return *close;
		}
		else for (Mode::Rates::const_iterator it(mode.rates.begin()), end(mode.rates.end()); it != end; ++it)
		{
			if (*it == Mode::DEFAULT_RATE)
				return Mode::DEFAULT_RATE;
		}

		return 0;
	}

	DWORD Direct2D::Device::GetDesiredPresentationInterval(const uint rate) const
	{
		if (!timing.vsync || rate % timing.frameRate)
		{
			return D3DPRESENT_INTERVAL_IMMEDIATE;
		}
		else if (presentation.Windowed)
		{
			return D3DPRESENT_INTERVAL_ONE;
		}
		else if (timing.frameRate * 4U == rate && intervalFour)
		{
			return D3DPRESENT_INTERVAL_FOUR;
		}
		else if (timing.frameRate * 3U == rate && intervalThree)
		{
			return D3DPRESENT_INTERVAL_THREE;
		}
		else if (timing.frameRate * 2U == rate && intervalTwo)
		{
			return D3DPRESENT_INTERVAL_TWO;
		}
		else
		{
			return D3DPRESENT_INTERVAL_ONE;
		}
	}

	DWORD Direct2D::Device::GetDesiredPresentationInterval() const
	{
		uint rate;

		if (presentation.Windowed)
		{
			D3DDISPLAYMODE mode;
			rate = GetDisplayMode( mode ) ? mode.RefreshRate : 0;
		}
		else
		{
			rate = presentation.FullScreen_RefreshRateInHz;
		}

		return GetDesiredPresentationInterval( rate );
	}

	HRESULT Direct2D::Device::Reset()
	{
		fonts.OnLost();

		const uint oldInterval = presentation.PresentationInterval;
		presentation.PresentationInterval = GetDesiredPresentationInterval();
		uint buffers = presentation.BackBufferCount = timing.tripleBuffering ? 2 : 1;
  
		if (!(presentation.Flags & D3DPRESENTFLAG_LOCKABLE_BACKBUFFER))
			com->SetDialogBoxMode( FALSE );

		for (;;)
		{
			const HRESULT hResult = com->Reset( &presentation );

			if (SUCCEEDED(hResult))
			{
				break;
			}
			else if (hResult == D3DERR_DEVICELOST)
			{			
				return D3DERR_DEVICELOST;
			}
			else if (buffers != presentation.BackBufferCount)
			{
				buffers = presentation.BackBufferCount;
				Io::Log() << "Direct3D: Warning! IDirect3DDevice9::Reset() failed, retrying with one back-buffer only..\r\n";
			}
			else
			{
				throw Application::Exception(_T("IDirect3DDevice9::Reset() failed!"));
			}
		}

		if ((presentation.Flags & D3DPRESENTFLAG_LOCKABLE_BACKBUFFER) && FAILED(com->SetDialogBoxMode( TRUE )))
			throw Application::Exception(_T("IDirect3DDevice9::SetDialogBoxMode() failed!"));

		Prepare();
		fonts.OnReset();

		if (presentation.PresentationInterval != oldInterval)
		{
			cstring msg;

			switch (presentation.PresentationInterval)
			{
				case D3DPRESENT_INTERVAL_IMMEDIATE:	msg = "Direct3D: disabling VSYNC\r\n";                  break;
				case D3DPRESENT_INTERVAL_TWO:		msg = "Direct3D: enabling VSYNC on second refresh\r\n"; break;
				case D3DPRESENT_INTERVAL_THREE:		msg = "Direct3D: enabling VSYNC on third refresh\r\n";  break;
				default:							msg = "Direct3D: enabling VSYNC\r\n";                   break;
			}

			Io::Log() << msg;
		}

		return D3D_OK;
	}

	void Direct2D::Device::LogDisplaySwitch() const
	{
		Io::Log log;
		log << "Direct3D: entering ";

		D3DDISPLAYMODE mode;

		if (GetDisplayMode( mode ))
		{
			log << mode.Width 
				<< 'x' 
				<< mode.Height 
				<< 'x' 
				<< Base::FormatToBpp(mode.Format)
				<< ' '
				<< mode.RefreshRate
				<< "hz ";
		}

		log << (presentation.Windowed ? "window mode\r\n" : "full-screen mode\r\n");
	}

	void Direct2D::Device::Prepare() const
	{
		com->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
		com->SetRenderState( D3DRS_LIGHTING, FALSE        );
		
		com->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );

		if (FAILED(com->SetFVF( VertexBuffer::FVF )))
			throw Application::Exception(_T("IDirect3DDevice9::SetFVF() failed!"));
	}

	uint Direct2D::Device::GetMaxMessageLength() const
	{																				  
		return fonts.GetWidth() ? (presentation.BackBufferWidth - fonts.GetWidth() * 7) / fonts.GetWidth() : 64;
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("t", on)
    #endif

	HRESULT Direct2D::Device::RenderScreen(const uint state) const
	{
		HRESULT hResult = com->BeginScene();

		if (SUCCEEDED(hResult))
		{
			if (state & RENDER_PICTURE)
				hResult = com->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );

			fonts.Render( presentation, state );
			com->EndScene();
		}

		return hResult;
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("", on)
    #endif

	HRESULT Direct2D::Device::Repair(const HRESULT lastError)
	{
		NST_ASSERT( FAILED(lastError) );

		tstring msg;

		switch (lastError)
		{
			case D3DERR_DEVICELOST:
			case D3DERR_DEVICENOTRESET:
				
				switch (com->TestCooperativeLevel())
				{
					case D3DERR_DEVICELOST:						
						
						return D3DERR_DEVICELOST;
				
					case D3DERR_DEVICENOTRESET:						
					
						return Reset();					
				
					case D3DERR_DRIVERINTERNALERROR:
						
						msg = _T("Video driver error! Contact your adapter manufacturer!");
						break;
				
					default:
						
						msg = _T("IDirect3DDevice9::TestCooperativeLevel() failed!");
						break;
				}

			case D3DERR_DRIVERINTERNALERROR:
				
				msg = _T("Video driver error! Contact your adapter manufacturer!");
				break;

			case E_OUTOFMEMORY:

				msg = _T("Out of memory!");
				break;

			case D3DERR_OUTOFVIDEOMEMORY:				
				
				msg = _T("Out of video memory!");
				break;

			default:
				
				msg = _T("Unknown Direct3D error!");
				break;
		}

		throw Application::Exception( msg );
	}

	ibool Direct2D::Device::CanToggleDialogBoxMode(bool enable) const
	{
		return !presentation.Windowed && bool(presentation.Flags & D3DPRESENTFLAG_LOCKABLE_BACKBUFFER) != enable;
	}

	ibool Direct2D::Device::CanSwitchFullscreen(const Mode& mode) const
	{
		return 
		(
	       	presentation.Windowed || 
			presentation.BackBufferWidth != mode.width ||
			presentation.BackBufferHeight != mode.height ||
			presentation.BackBufferFormat != (mode.bpp == 16 ? D3DFMT_R5G6B5 : D3DFMT_X8R8G8B8) ||
			presentation.FullScreen_RefreshRateInHz != GetDesiredPresentationRate( mode )
		);
	}

	void Direct2D::Device::SwitchFullscreen(const Mode& mode)
	{
		presentation.Windowed = FALSE;
		presentation.BackBufferWidth = mode.width;
		presentation.BackBufferHeight = mode.height;
		presentation.BackBufferFormat = (mode.bpp == 16 ? D3DFMT_R5G6B5 : D3DFMT_X8R8G8B8);
		presentation.FullScreen_RefreshRateInHz = GetDesiredPresentationRate( mode );
		presentation.Flags = 0;

		if (FAILED(Reset()))
			throw Application::Exception(_T("Couldn't switch display mode!"));

		fonts.Create( *this );
		LogDisplaySwitch();
	}

	HRESULT Direct2D::Device::ToggleDialogBoxMode(ibool enable)
	{
		NST_ASSERT( !presentation.Windowed );

		presentation.Flags = (enable ? D3DPRESENTFLAG_LOCKABLE_BACKBUFFER : 0);
		return Reset();
	}

	void Direct2D::Device::SwitchWindowed()
	{
		fonts.Destroy( FALSE );

		presentation.Windowed = TRUE;
		presentation.BackBufferWidth = 0;
		presentation.BackBufferHeight = 0;
		presentation.BackBufferFormat = D3DFMT_UNKNOWN;
		presentation.FullScreen_RefreshRateInHz = 0;
		presentation.Flags = 0;

		if (FAILED(Reset()))
			throw Application::Exception(_T("Couldn't switch display mode!"));

		LogDisplaySwitch();
	}

	HRESULT Direct2D::Device::ResetWindowClient(const Point& client,HRESULT hResult)
	{
		NST_ASSERT( presentation.Windowed && client.x > 0 && client.y > 0 );

		presentation.BackBufferWidth = uint(client.x);
		presentation.BackBufferHeight = uint(client.y);

		if (SUCCEEDED(hResult) || hResult == NST_E_INVALID_RECT)
			hResult = Reset();

		return hResult;
	}

	ibool Direct2D::Device::ResetFrameRate(uint frameRate,bool vsync,bool tripleBuffering,const Base& base)
	{
		timing.frameRate = (u8) frameRate;
		timing.vsync = vsync;

		if (timing.tripleBuffering != tripleBuffering)
		{
			timing.tripleBuffering = tripleBuffering;
			return TRUE;
		}
		else if (presentation.Windowed)
		{
			return presentation.PresentationInterval != GetDesiredPresentationInterval();
		}
		else
		{
			const Mode mode
			( 
		     	presentation.BackBufferWidth, 
				presentation.BackBufferHeight, 
				presentation.BackBufferFormat == D3DFMT_X8R8G8B8 ? 32 : 16
			);

			frameRate = GetDesiredPresentationRate( *base.GetAdapter(ordinal).modes.find(mode) );

			if (presentation.FullScreen_RefreshRateInHz != frameRate)
			{
				presentation.FullScreen_RefreshRateInHz = frameRate;
				return TRUE;
			}
			else
			{
				return presentation.PresentationInterval != GetDesiredPresentationInterval( frameRate );
			}
		}
	}

	Direct2D::VertexBuffer::Vertex::Vertex()
	: z(0.f), rhw(1.f) {}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("t", on)
    #endif

	void Direct2D::VertexBuffer::Update(const Rect& picture,const Rect& clip,const float scale)
	{
		NST_ASSERT( picture.Width() > 0 && picture.Height() > 0 && clip.Width() > 0 && clip.Height() > 0 );

		rect = picture;

		vertices[0].x = picture.left - 0.5f;  
		vertices[0].y = picture.top - 0.5f;   
		vertices[0].u = clip.left / scale;    
		vertices[0].v = clip.top / scale;     
		vertices[1].x = picture.left - 0.5f;  
		vertices[1].y = picture.bottom - 0.5f;
		vertices[1].u = clip.left / scale;    
		vertices[1].v = clip.bottom / scale;  
		vertices[2].x = picture.right - 0.5f; 
		vertices[2].y = picture.top - 0.5f;   
		vertices[2].u = clip.right / scale;   
		vertices[2].v = clip.top / scale;     
		vertices[3].x = picture.right - 0.5f; 
		vertices[3].y = picture.bottom - 0.5f;
		vertices[3].u = clip.right / scale;   
		vertices[3].v = clip.bottom / scale;  
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("", on)
    #endif

	inline void Direct2D::VertexBuffer::Invalidate()
	{
		com.Release();
	}

	HRESULT Direct2D::VertexBuffer::Validate(IDirect3DDevice9& device,ibool dirty)
	{
		if (com == NULL)
		{
			const HRESULT hResult = device.CreateVertexBuffer
			(
				sizeof(Vertex) * NUM_VERTICES,
				D3DUSAGE_WRITEONLY,
				FVF,
				D3DPOOL_DEFAULT,
				&com,
				NULL
			);

			if (SUCCEEDED(hResult))
			{
				if (FAILED(device.SetStreamSource( 0, com, 0, sizeof(Vertex) )))
					throw Application::Exception(_T("IDirect3DDevice9::SetStreamSource() failed!"));

				dirty = TRUE;
			}
			else if (hResult == D3DERR_DEVICELOST) 
			{
				return D3DERR_DEVICELOST;
			}
			else
			{
				throw Application::Exception(_T("IDirect3DDevice9::CreateVertexBuffer() failed!"));
			}
		}

		if (dirty)
		{
			void* ram;		
			const HRESULT hResult = com->Lock( 0, 0, &ram, D3DLOCK_NOSYSLOCK );

			if (SUCCEEDED(hResult))
			{
				std::memcpy( ram, vertices, sizeof(vertices) );
				com->Unlock();
			}
			else if (hResult == D3DERR_DEVICELOST)
			{
				return D3DERR_DEVICELOST;
			}
			else
			{
				throw Application::Exception(_T("IDirect3DVertexBuffer9::Lock() failed!"));
			}
		}

		return D3D_OK;
	}

	Direct2D::Texture::Texture(const D3DFORMAT backBufferFormat)
	: 
	size      (256), 
	width     (256), 
	height    (256), 
	useVidMem (TRUE),
	lockFlags (D3DLOCK_NOSYSLOCK),
	format    (backBufferFormat), 
	filter    (Adapter::FILTER_NONE) 
	{}

	Direct2D::Texture::~Texture()
	{
		Invalidate();
	}

	inline uint Direct2D::Texture::Size() const
	{
		return size;
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("t", on)
    #endif

	void Direct2D::Texture::Update(const Point& screen,const Adapter::Filter f,const bool wantVidMem)
	{
		NST_ASSERT( screen.x > 0 && screen.y > 0 );

		width = (uint) screen.x;
		height = (uint) screen.y;
		filter = f;

		uint pow = NST_MAX(width,height);

		--pow;		
		pow |= pow >> 1;
		pow |= pow >> 2;
		pow |= pow >> 4;
		pow |= pow >> 8;
		pow |= pow >> 16;
		++pow;

		if (size != pow || bool(useVidMem) != wantVidMem)
		{
			size = pow;
			useVidMem = wantVidMem;
			Invalidate();
		}
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("", on)
    #endif

	void Direct2D::Texture::Invalidate()
	{
		if (com)
		{
			IDirect3DDevice9* device;

			if (SUCCEEDED(com->GetDevice( &device )))
			{
				device->SetTexture( 0, NULL );
				device->Release();
			}

			com.Release();
		}
	}

	HRESULT Direct2D::Texture::Validate(IDirect3DDevice9& device,const Adapter& adapter,const D3DFORMAT backBufferFormat)
	{
		if (com == NULL)
		{
			const HRESULT hResult = ::D3DXCreateTexture
			( 
				&device,
				size, 
				size, 
				1, 
				(useVidMem && adapter.videoMemScreen) ? D3DUSAGE_DYNAMIC : 0, 
				backBufferFormat, 
				(useVidMem && adapter.videoMemScreen) ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED, 
				&com
			);

			if (SUCCEEDED(hResult))
			{
				Object::Pod<D3DSURFACE_DESC> desc;

				if (FAILED(com->GetLevelDesc( 0, &desc )))
					throw Application::Exception(_T("IDirect3DDevice9::GetLevelDesc() failed!"));

				format = desc.Format;
				lockFlags = (desc.Usage & D3DUSAGE_DYNAMIC) ? (D3DLOCK_DISCARD|D3DLOCK_NOSYSLOCK) : D3DLOCK_NOSYSLOCK;

				if (desc.Height < size || desc.Width < size)
					throw Application::Exception(_T("Maximum texture dimension too small!"));

				const uint bpp = GetBitsPerPixel();

				if (!bpp)
					throw Application::Exception(_T("Unsupported bit-per-pixel format!"));

				if (FAILED(device.SetTexture( 0, com )))
					throw Application::Exception(_T("IDirect3DDevice9::SetTexture() failed!"));
			}
			else if (hResult == D3DERR_DEVICELOST) 
			{	
				return D3DERR_DEVICELOST;
			}
			else
			{
				throw Application::Exception(_T("IDirect3DDevice9::CreateTexture() failed!"));
			}
		}

		const DWORD type = (filter == Adapter::FILTER_NONE ? D3DTEXF_POINT : D3DTEXF_LINEAR);

		device.SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
		device.SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
		device.SetSamplerState( 0, D3DSAMP_MINFILTER, type );
		device.SetSamplerState( 0, D3DSAMP_MAGFILTER, type );
										
		return D3D_OK;
	}

	ibool Direct2D::Texture::SaveToFile(tstring const file,const D3DXIMAGE_FILEFORMAT type) const
	{
		NST_ASSERT( file && *file && width && height );

		IDirect3DSurface9* surface;

		if (com && SUCCEEDED(com->GetSurfaceLevel( 0, &surface )))
		{
			const RECT rect = {0,0,width,height};
			const ibool result = SUCCEEDED(::D3DXSaveSurfaceToFile( file, type, surface, NULL, &rect ));
			surface->Release();
			return result;
		}

		return FALSE;
	}

	Direct2D::Direct2D(HWND hWnd)
	:
	device     ( hWnd, base ),
	texture    ( device.GetPresentation().BackBufferFormat ),
	lastResult ( D3D_OK )
	{
		ValidateObjects();
	}

	void Direct2D::InvalidateObjects()
	{
		vertexBuffer.Invalidate();
		texture.Invalidate();
	}

	void Direct2D::ValidateObjects()
	{
		if (SUCCEEDED(lastResult))
		{
			lastResult = vertexBuffer.Validate( device );

			if (SUCCEEDED(lastResult))
				lastResult = texture.Validate( device, GetAdapter(), device.GetPresentation().BackBufferFormat );
		}
	}

	void Direct2D::SelectAdapter(const Adapters::const_iterator adapter)
	{
		if (device.GetOrdinal() != adapter->ordinal)
		{
			InvalidateObjects();
			device.Create( base, *adapter );
			lastResult = NST_E_INVALID_RECT;
		}
	}

	ibool Direct2D::SwitchFullscreen(const Adapter::Modes::const_iterator mode)
	{
		if (device.CanSwitchFullscreen( *mode ))
		{
			InvalidateObjects();
			device.SwitchFullscreen( *mode );
			lastResult = D3D_OK;
			ValidateObjects();
			return TRUE;
		}

		return FALSE;
	}

	ibool Direct2D::SwitchWindowed()
	{
		if (!device.GetPresentation().Windowed)
		{
			InvalidateObjects();
			device.SwitchWindowed();
			lastResult = D3D_OK;
			ValidateObjects();
			return TRUE;
		}

		return FALSE;
	}

	void Direct2D::EnableDialogBoxMode(const ibool enable)
	{
		if (device.CanToggleDialogBoxMode( enable ))
		{
			InvalidateObjects();
			lastResult = device.ToggleDialogBoxMode( enable );
			ValidateObjects();
		}
	}

	ibool Direct2D::Reset()
	{
		if (FAILED(lastResult) && lastResult != NST_E_INVALID_RECT)
		{
			InvalidateObjects();
			lastResult = device.Repair( lastResult );
			ValidateObjects();
		}

		return SUCCEEDED(lastResult);
	}

	void Direct2D::UpdateWindowView()
	{
		const Point picture( Rect::Picture( device.GetPresentation().hDeviceWindow ).Size() );

		if (picture.x > 0 && picture.y > 0)
		{
			const Point client( Rect::Client( device.GetPresentation().hDeviceWindow ).Size() );
			NST_ASSERT( client.x >= picture.x && client.y >= picture.y );

			if 
			(
				(uint(client.x) != device.GetPresentation().BackBufferWidth) ||
				(uint(client.y) != device.GetPresentation().BackBufferHeight)
			)
			{
				InvalidateObjects();
				lastResult = device.ResetWindowClient( client, lastResult );
			}

			ValidateObjects();
		}
		else
		{
			lastResult = NST_E_INVALID_RECT;
		}
	}

	void Direct2D::UpdateWindowView(const Point& screen,const Rect& clipping,const Adapter::Filter filter,const ibool useVidMem)
	{
		const Point picture( Rect::Picture( device.GetPresentation().hDeviceWindow ).Size() );

		if (picture.x > 0 && picture.y > 0)
		{
			texture.Update( screen, filter, useVidMem );
			vertexBuffer.Update( picture, clipping, (float) texture.Size() );

			const Point client( Rect::Client( device.GetPresentation().hDeviceWindow ).Size() );
			NST_ASSERT( client.x >= picture.x && client.y >= picture.y );

			if 
			(
				(uint(client.x) != device.GetPresentation().BackBufferWidth) ||
				(uint(client.y) != device.GetPresentation().BackBufferHeight)
			)
			{
				InvalidateObjects();
				lastResult = device.ResetWindowClient( client, lastResult );
			}

			ValidateObjects();
		}
		else
		{
			lastResult = NST_E_INVALID_RECT;
		}
	}

	void Direct2D::UpdateFullscreenView(const Rect& picture,const Point& screen,const Rect& clipping,const Adapter::Filter filter,const ibool useVidMem)
	{
		NST_ASSERT( picture.Width() && picture.Height() );

		texture.Update( screen, filter, useVidMem );
		vertexBuffer.Update( picture, clipping, (float) texture.Size() );
		ValidateObjects();
	}

	void Direct2D::UpdateFrameRate(const uint frameRate,const ibool vsync,const ibool tripleBuffering)
	{
		if (device.ResetFrameRate( frameRate, vsync, tripleBuffering, base ))
		{
			InvalidateObjects();

			if (SUCCEEDED(lastResult))
			{
				lastResult = device.Reset();
				ValidateObjects();
			}
		}
	}

	Direct2D::ScreenShotResult Direct2D::SaveScreenShot(tstring const file,const uint ext) const
	{
		NST_ASSERT( file && *file );

		if (SUCCEEDED(lastResult))
		{
			D3DXIMAGE_FILEFORMAT format;

			switch (ext)
			{
				case MAKEFOURCC('p','n','g','\0'): format = D3DXIFF_PNG; break;
				case MAKEFOURCC('j','p','g','\0'): format = D3DXIFF_JPG; break;
				case MAKEFOURCC('b','m','p','\0'): format = D3DXIFF_BMP; break;
				default: return SCREENSHOT_UNSUPPORTED;
			}

			if (texture.SaveToFile( file, format ))
				return SCREENSHOT_OK;
		}

		return SCREENSHOT_ERROR;
	}
}
