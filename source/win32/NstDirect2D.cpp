////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003-2005 Martin Freij
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

	ibool Direct2D::Mode::operator == (const Mode& mode) const
	{
		return 
		(
			width == mode.width && 
			height == mode.height && 
			bpp == mode.bpp
		);
	}

	ibool Direct2D::Mode::operator < (const Mode& mode) const
	{
		if ( width  < mode.width  ) return TRUE;
		if ( width  > mode.width  ) return FALSE;
		if ( height < mode.height ) return TRUE;
		if ( height > mode.height ) return FALSE;
		if ( bpp    < mode.bpp    ) return TRUE;
		if ( bpp    > mode.bpp    ) return FALSE;

		return FALSE;
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
			return *com;
		else
			throw Application::Exception("::Direct3DCreate9() failed! Upgrade to DirectX 9.0c or better!");
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
						
						if (display.Width < Adapter::MIN_WIDTH || display.Height < Adapter::MIN_HEIGHT)
							continue;
						
						const Mode entry( display.Width, display.Height, format ? 32 : 16, display.RefreshRate );

						if (Mode* const current = modes.Find( entry ))
						{
							if (display.RefreshRate == Mode::DESIRED_HZ)
								current->rate = Mode::DESIRED_HZ;
						}
						else
						{
							modes.Insert( entry );
						}
					}
				}

				if (modes.Size())
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
					adapter.videoMemScreen  = ((caps.Caps2 & D3DCAPS2_DYNAMICTEXTURES) != 0);
					adapter.modes           = modes;
					adapter.filters         = 0;

					if ((caps.TextureFilterCaps & (D3DPTFILTERCAPS_MINFLINEAR|D3DPTFILTERCAPS_MAGFLINEAR)) == (D3DPTFILTERCAPS_MINFLINEAR|D3DPTFILTERCAPS_MAGFLINEAR))
						adapter.filters |= Adapter::FILTER_BILINEAR;

					Io::Log() << "Direct3D: dynamic textures: " << (adapter.videoMemScreen ? "supported\r\n" : "unsupported\r\n")
							  << "Direct3D: texture bilinear filtering: " << ((adapter.filters & Adapter::FILTER_BILINEAR) ? "supported\r\n" : "unsupported\r\n")
							  << "Direct3D: max texture dimensions: " << caps.MaxTextureWidth << 'x' << caps.MaxTextureHeight
						      << "\r\nDirect3D: found " << modes.Size() << " display modes\r\n";
				}
				else
				{
					Io::Log() << "Direct3D: found no valid display mode, continuing enumeration..\r\n";
				}
			}
		}

		if (adapters.empty())
			throw Application::Exception("Found no valid display adapter!");

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
			arial ? "Arial" : "System",
			&com
		);
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

	void Direct2D::Device::Fonts::Font::Update(const String::Generic& newstring)
	{
		string = newstring;
		length = newstring.Size();

		if (length && com)
			com->PreloadText( string, string.Size() );
	}

	void Direct2D::Device::Fonts::Create(const Device& device)
	{
		nfo.Create( device );

		if (!device.presentation.Windowed)
		{
			fps.Create( device );
			msg.Create( device );
		}
	}

	void Direct2D::Device::Fonts::Destroy(const ibool newDevice)
	{
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
		com->DrawText( NULL, string, length, &rect, flags, color );
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
	: vsync(FALSE), frameRate(Mode::DESIRED_HZ) {}

	Direct2D::Device::Device(HWND const hWnd,const Base& base)
	{
		NST_ASSERT( hWnd );

		presentation.BackBufferWidth            = 0;
		presentation.BackBufferHeight           = 0;
		presentation.BackBufferFormat           = D3DFMT_UNKNOWN;
		presentation.BackBufferCount            = DEFAULT_BACK_BUFFER_COUNT;
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

		fonts.Destroy( TRUE );
		com.Release();

		uint buffers = presentation.BackBufferCount = DEFAULT_BACK_BUFFER_COUNT;
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
				throw Application::Exception("Can't start! Direct3D is busy!");
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
				throw Application::Exception("IDirect3DDevice9::CreateDevice() failed!");
			}
		}

		if (FAILED(com->SetDialogBoxMode( TRUE )))
			throw Application::Exception("IDirect3DDevice9::SetDialogBoxMode() failed!");

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

	DWORD Direct2D::Device::GetDesiredPresentationInterval() const
	{
		if (timing.vsync)
		{
			if (presentation.Windowed)
			{
				D3DDISPLAYMODE mode;

				if (GetDisplayMode( mode ) && timing.frameRate == mode.RefreshRate)
					return D3DPRESENT_INTERVAL_ONE;
			}
			else
			{
				if (timing.frameRate == presentation.FullScreen_RefreshRateInHz)
					return D3DPRESENT_INTERVAL_ONE;
			}
		}

		return D3DPRESENT_INTERVAL_IMMEDIATE;
	}

	HRESULT Direct2D::Device::Reset()
	{
		fonts.OnLost();

		const uint oldInterval = presentation.PresentationInterval;
		presentation.PresentationInterval = GetDesiredPresentationInterval();
		uint buffers = presentation.BackBufferCount = DEFAULT_BACK_BUFFER_COUNT;
  
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
				throw Application::Exception("IDirect3DDevice9::Reset() failed!");
			}
		}

		Prepare();
		fonts.OnReset();

		if (presentation.PresentationInterval != oldInterval)
			Io::Log() << (presentation.PresentationInterval == D3DPRESENT_INTERVAL_IMMEDIATE ? "Direct3D: disabling VSYNC\r\n" : "Direct3D: enabling VSYNC\r\n");

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
		com->SetRenderState( D3DRS_CLIPPING, FALSE        );
		
		com->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );

		if (FAILED(com->SetFVF( VertexBuffer::FVF )))
			throw Application::Exception("IDirect3DDevice9::SetFVF() failed!");
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

		cstring msg;

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
						
						msg = "Video driver error! Contact your adapter manufacturer!";
						break;
				
					default:
						
						msg = "IDirect3DDevice9::TestCooperativeLevel() failed!";
						break;
				}

			case D3DERR_DRIVERINTERNALERROR:
				
				msg = "Video driver error! Contact your adapter manufacturer!";
				break;

			case E_OUTOFMEMORY:

				msg = "Out of memory!";
				break;

			case D3DERR_OUTOFVIDEOMEMORY:				
				
				msg = "Out of video memory!";
				break;

			default:
				
				msg = "Unknown Direct3D error!";
				break;
		}

		throw Application::Exception( msg );
	}

	ibool Direct2D::Device::CanSwitchFullscreen(const Mode& mode) const
	{
		return 
		(
	       	presentation.Windowed || 
			presentation.BackBufferWidth != mode.width ||
			presentation.BackBufferHeight != mode.height ||
			presentation.BackBufferFormat != (mode.bpp == 16 ? D3DFMT_R5G6B5 : D3DFMT_X8R8G8B8)
		);
	}

	void Direct2D::Device::SwitchFullscreen(const Mode& mode)
	{
		presentation.Windowed = FALSE;
		presentation.BackBufferWidth = mode.width;
		presentation.BackBufferHeight = mode.height;
		presentation.BackBufferFormat = (mode.bpp == 16 ? D3DFMT_R5G6B5 : D3DFMT_X8R8G8B8);
		presentation.FullScreen_RefreshRateInHz = mode.rate;
		presentation.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER; // required for SetDialogBoxMode()

		if (FAILED(Reset()))
			throw Application::Exception("Couldn't switch display mode!");

		fonts.Create( *this );
		LogDisplaySwitch();
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
			throw Application::Exception("Couldn't switch display mode!");

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

	ibool Direct2D::Device::CanResetFrameRate(const uint frameRate,uint vsync)
	{
		timing.frameRate = frameRate;
		timing.vsync = vsync;

		return presentation.PresentationInterval != GetDesiredPresentationInterval();
	}

	Direct2D::VertexBuffer::Vertex::Vertex()
	: z(0.f), rhw(1.f) {}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("at", on)
    #endif

	void Direct2D::VertexBuffer::Update(const Rect& picture,const Rect& clip,const float scale)
	{
		NST_ASSERT( picture.Width() > 0 && picture.Height() > 0 && clip.Width() > 0 && clip.Height() > 0 );

		rect = picture;

		vertices[0].x = (float) ( picture.left - 0.5f );
		vertices[0].y = (float) ( picture.top - 0.5f  );
		vertices[0].u = (float) ( clip.left / scale   );
		vertices[0].v = (float) ( clip.top / scale    );
		vertices[1].x = (float) ( picture.left - 0.5f );
		vertices[1].y = (float) ( picture.bottom      );
		vertices[1].u = (float) ( clip.left / scale   );
		vertices[1].v = (float) ( clip.bottom / scale );
		vertices[2].x = (float) ( picture.right       );
		vertices[2].y = (float) ( picture.top - 0.5f  );
		vertices[2].u = (float) ( clip.right / scale  );
		vertices[2].v = (float) ( clip.top / scale    );
		vertices[3].x = (float) ( picture.right       );
		vertices[3].y = (float) ( picture.bottom      );
		vertices[3].u = (float) ( clip.right / scale  );
		vertices[3].v = (float) ( clip.bottom / scale );
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
				D3DUSAGE_WRITEONLY|D3DUSAGE_DONOTCLIP,
				FVF,
				D3DPOOL_DEFAULT,
				&com,
				NULL
			);

			if (SUCCEEDED(hResult))
			{
				if (FAILED(device.SetStreamSource( 0, com, 0, sizeof(Vertex) )))
					throw Application::Exception("IDirect3DDevice9::SetStreamSource() failed!");

				dirty = TRUE;
			}
			else if (hResult == D3DERR_DEVICELOST) 
			{
				return D3DERR_DEVICELOST;
			}
			else
			{
				throw Application::Exception("IDirect3DDevice9::CreateVertexBuffer() failed!");
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
				throw Application::Exception("IDirect3DVertexBuffer9::Lock() failed!");
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
					throw Application::Exception("IDirect3DDevice9::GetLevelDesc() failed!");

				format = desc.Format;
				lockFlags = (desc.Usage & D3DUSAGE_DYNAMIC) ? (D3DLOCK_DISCARD|D3DLOCK_NOSYSLOCK) : D3DLOCK_NOSYSLOCK;

				if (desc.Height < size || desc.Width < size)
					throw Application::Exception("Maximum texture dimension too small!");

				const uint bpp = GetBitsPerPixel();

				if (!bpp)
					throw Application::Exception("Unsupported bit-per-pixel format!");

				if (FAILED(device.SetTexture( 0, com )))
					throw Application::Exception("IDirect3DDevice9::SetTexture() failed!");

				Io::Log() << "Direct3D: creating " 
					      << desc.Width 
						  << 'x' 
						  << desc.Height 
						  << 'x' 
						  << bpp
						  << " NES screen texture in " 
						  << (desc.Pool == D3DPOOL_DEFAULT ? "video memory\r\n" : "system memory\r\n");
			}
			else if (hResult == D3DERR_DEVICELOST) 
			{	
				return D3DERR_DEVICELOST;
			}
			else
			{
				throw Application::Exception("IDirect3DDevice9::CreateTexture() failed!");
			}
		}

		const DWORD type = (filter == Adapter::FILTER_NONE ? D3DTEXF_POINT : D3DTEXF_LINEAR);

		device.SetSamplerState( 0, D3DSAMP_MINFILTER, type );
		device.SetSamplerState( 0, D3DSAMP_MAGFILTER, type );

		return D3D_OK;
	}

	ibool Direct2D::Texture::SaveToFile(cstring const file,const D3DXIMAGE_FILEFORMAT type) const
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

	void Direct2D::SelectAdapter(const uint index)
	{
		if (device.GetOrdinal() != base.GetAdapter(index).ordinal)
		{
			InvalidateObjects();
			device.Create( base, base.GetAdapter(index) );
			lastResult = NST_E_INVALID_RECT;
		}
	}

	ibool Direct2D::SwitchFullscreen(const Mode& mode)
	{
		if (device.CanSwitchFullscreen( mode ))
		{
			InvalidateObjects();
			device.SwitchFullscreen( mode );
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

	void Direct2D::UpdateFrameRate(const uint frameRate,const ibool vsync)
	{
		if (device.CanResetFrameRate( frameRate, vsync ))
		{
			InvalidateObjects();

			if (SUCCEEDED(lastResult))
			{
				lastResult = device.Reset();
				ValidateObjects();
			}
		}
	}

	Direct2D::ScreenShotResult Direct2D::SaveScreenShot(cstring const file,const uint ext) const
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
