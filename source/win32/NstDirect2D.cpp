////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2007 Martin Freij
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

#include "language/resource.h"
#include "NstIoLog.hpp"
#include "NstApplicationException.hpp"
#include "NstDirect2D.hpp"
#include "NstIoScreen.hpp"

#if NST_MSVC
#pragma comment(lib,"d3d9")
#pragma comment(lib,"d3dx9")
#endif

namespace Nestopia
{
	namespace DirectX
	{
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
			IDirect3D9* com;

			if (NULL != (com = ::Direct3DCreate9( D3D_SDK_VERSION )))
			{
				return *com;
			}
			else if (NULL != (com = ::Direct3DCreate9( D3D9b_SDK_VERSION ))) // unofficial, it may work, it may not work
			{
				return *com;
			}
			else
			{
				throw Application::Exception( IDS_ERR_D3D_FAILED );
			}
		}

		const Direct2D::Adapters Direct2D::Base::EnumerateAdapters(IDirect3D9& d3d)
		{
			NST_COMPILE_ASSERT( D3DADAPTER_DEFAULT == 0 );

			Io::Log() << "Direct3D: initializing..\r\n";

			Adapters adapters;

			for (uint ordinal=0, numAdapters=d3d.GetAdapterCount(); ordinal < NST_MIN(numAdapters,255); ++ordinal)
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

							// C++ standard vagueness, sometimes set::iterator == set::const_iterator
							const_cast<Mode::Rates&>(modes.insert(Mode( display.Width, display.Height, format ? 32 : 16 )).first->rates).insert( display.RefreshRate );
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
						adapter.modern          = (caps.PixelShaderVersion >= D3DPS_VERSION(2,0));
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
				case D3DFMT_A8R8G8B8:    r = 0x00FF0000; g = 0x0000FF00; b = 0x000000FF; break;
				case D3DFMT_X8B8G8R8:
				case D3DFMT_A8B8G8R8:    r = 0x000000FF; g = 0x0000FF00; b = 0x00FF0000; break;
				case D3DFMT_A2R10G10B10: r = 0x3FF00000; g = 0x000FFC00; b = 0x000003FF; break;
				case D3DFMT_A2B10G10R10: r = 0x000003FF; g = 0x000FFC00; b = 0x3FF00000; break;
				case D3DFMT_R5G6B5:      r = 0xF800;     g = 0x07E0;     b = 0x001F;     break;
				case D3DFMT_X1R5G5B5:
				case D3DFMT_A1R5G5B5:    r = 0x7C00;     g = 0x03E0;     b = 0x001F;     break;
				case D3DFMT_X4R4G4B4:
				case D3DFMT_A4R4G4B4:    r = 0x0F00;     g = 0x00F0;     b = 0x000F;     break;
				case D3DFMT_A8R3G3B2:    r = 0x00E0;     g = 0x001C;     b = 0x0003;     break;
				default:                 r = 0;          g = 0;          b = 0;          break;
			}
		}

		Direct2D::Device::Fonts::Fonts()
		: width(0) {}

		void Direct2D::Device::Fonts::Font::Create(const Device& device)
		{
			tstring fontName = _T("System");
			uint fontHeight = 12;

			D3DDISPLAYMODE mode;
			device.GetDisplayMode( mode );

			if (mode.Width > 320 && mode.Height > 240)
			{
				fontHeight = mode.Height / (device.presentation.Windowed ? 32 : 16);

				switch (PRIMARYLANGID(::GetUserDefaultLangID()))
				{
					case LANG_JAPANESE: fontName = _T("MS Gothic"); break;
					case LANG_CHINESE:  fontName = _T("MS Hei");    break;
					case LANG_KOREAN:   fontName = _T("GulimChe");  break;
					default:            fontName = _T("Arial");     break;
				}
			}

			if (com != NULL)
			{
				Object::Pod<D3DXFONT_DESC> desc;
				com->GetDesc( &desc );

				if (desc.Height == int(fontHeight) && bool(desc.Width > 320 && desc.Height > 240) == bool(mode.Width > 320 && mode.Height > 240))
					return;

				com.Release();
			}

			::D3DXCreateFont
			(
				*device.com,
				fontHeight,
				0,
				FW_NORMAL,
				1,
				false,
				DEFAULT_CHARSET,
				OUT_DEFAULT_PRECIS,
				DEFAULT_QUALITY,
				DEFAULT_PITCH|FF_DONTCARE,
				fontName,
				&com
			);
		}

		uint Direct2D::Device::Fonts::Font::Width() const
		{
			TEXTMETRIC metric;

			if (com != NULL && SUCCEEDED(com->GetTextMetrics( &metric )))
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
			if (com != NULL)
				com->OnResetDevice();
		}

		void Direct2D::Device::Fonts::Font::OnLost() const
		{
			if (com != NULL)
				com->OnLostDevice();
		}

		void Direct2D::Device::Fonts::Font::Update(const GenericString& newstring)
		{
			string = newstring;
			length = newstring.Length();

			if (length && com != NULL)
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

			width = nfo.Width();
		}

		void Direct2D::Device::Fonts::Destroy(const bool newDevice)
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

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		inline bool Direct2D::Device::Fonts::Font::CanDraw() const
		{
			return length && com != NULL;
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

		#ifdef NST_MSVC_OPTIMIZE
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
			presentation.BackBufferCount            = 1;
			presentation.MultiSampleType            = D3DMULTISAMPLE_NONE;
			presentation.MultiSampleQuality         = 0;
			presentation.SwapEffect                 = D3DSWAPEFFECT_DISCARD;
			presentation.hDeviceWindow              = hWnd;
			presentation.Windowed                   = true;
			presentation.EnableAutoDepthStencil     = false;
			presentation.AutoDepthStencilFormat     = D3DFMT_UNKNOWN;
			presentation.Flags                      = 0;
			presentation.FullScreen_RefreshRateInHz = 0;
			presentation.PresentationInterval       = D3DPRESENT_INTERVAL_IMMEDIATE;

			Create( base, base.GetAdapter(0) );
		}

		inline Direct2D::Device::operator IDirect3DDevice9& () const
		{
			return **com;
		}

		void Direct2D::Device::Create(IDirect3D9& d3d,const Adapter& adapter)
		{
			ordinal = adapter.ordinal;
			intervalTwo = adapter.intervalTwo;
			intervalThree = adapter.intervalThree;
			intervalFour = adapter.intervalFour;

			fonts.Destroy( true );
			com.Release();

			NST_VERIFY( !!Point::Client(presentation.hDeviceWindow) );

			uint buffers = presentation.BackBufferCount = (timing.tripleBuffering ? 2 : 1);
			presentation.SwapEffect = GetSwapEffect();
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
					if (HDC const hdc = ::GetDC( NULL ))
					{
						const int bits = ::GetDeviceCaps( hdc, BITSPIXEL );
						::ReleaseDC( NULL, hdc );

						if (bits && bits != 16 && bits != 32)
							throw Application::Exception( IDS_ERR_BAD_BPP );
					}

					throw Application::Exception( IDS_ERR_D3D_DEVICE_FAILED );
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

		bool Direct2D::Device::GetDisplayMode(D3DDISPLAYMODE& displayMode) const
		{
			IDirect3D9* base;

			if (SUCCEEDED(com->GetDirect3D( &base )))
			{
				const HRESULT hResult = base->GetAdapterDisplayMode( ordinal, &displayMode );
				base->Release();

				if (SUCCEEDED(hResult))
					return true;
			}

			displayMode.Width = presentation.BackBufferWidth;
			displayMode.Height = presentation.BackBufferHeight;

			return false;
		}

		D3DSWAPEFFECT Direct2D::Device::GetSwapEffect() const
		{
			if
			(
				presentation.BackBufferCount > 1 ||
				presentation.Flags == D3DPRESENTFLAG_LOCKABLE_BACKBUFFER
			)
				return D3DSWAPEFFECT_DISCARD;
			else
				return D3DSWAPEFFECT_COPY;
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
			else if (!presentation.Windowed)
			{
				if (timing.frameRate * 4 == rate && intervalFour)
				{
					return D3DPRESENT_INTERVAL_FOUR;
				}
				else if (timing.frameRate * 3 == rate && intervalThree)
				{
					return D3DPRESENT_INTERVAL_THREE;
				}
				else if (timing.frameRate * 2 == rate && intervalTwo)
				{
					return D3DPRESENT_INTERVAL_TWO;
				}
			}

			return timing.frameRate == rate ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
		}

		uint Direct2D::Device::GetRefreshRate() const
		{
			if (presentation.Windowed)
			{
				D3DDISPLAYMODE mode;
				return GetDisplayMode( mode ) ? mode.RefreshRate : 0;
			}
			else
			{
				return presentation.FullScreen_RefreshRateInHz;
			}
		}

		DWORD Direct2D::Device::GetDesiredPresentationInterval() const
		{
			return GetDesiredPresentationInterval( GetRefreshRate() );
		}

		HRESULT Direct2D::Device::Reset()
		{
			fonts.OnLost();

			const uint oldInterval = presentation.PresentationInterval;
			presentation.PresentationInterval = GetDesiredPresentationInterval();
			uint buffers = presentation.BackBufferCount = (timing.tripleBuffering ? 2 : 1);
			presentation.SwapEffect = GetSwapEffect();

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
				else throw Application::Exception
				(
					IDS_FAILED,
					hResult == D3DERR_INVALIDCALL         ? _T( "IDirect3DDevice9::Reset() (code: D3DERR_INVALIDCALL)"         ) :
					hResult == D3DERR_OUTOFVIDEOMEMORY    ? _T( "IDirect3DDevice9::Reset() (code: D3DERR_OUTOFVIDEOMEMORY)"    ) :
					hResult == D3DERR_DRIVERINTERNALERROR ? _T( "IDirect3DDevice9::Reset() (code: D3DERR_DRIVERINTERNALERROR)" ) :
					hResult == E_OUTOFMEMORY              ? _T( "IDirect3DDevice9::Reset() (code: E_OUTOFMEMORY)"              ) :
															_T( "IDirect3DDevice9::Reset()"                                    )
				);
			}

			NST_ASSERT( !presentation.Windowed || !presentation.Flags );

			if (presentation.Flags == D3DPRESENTFLAG_LOCKABLE_BACKBUFFER && FAILED(com->SetDialogBoxMode( true )))
				throw Application::Exception( IDS_FAILED, _T("IDirect3DDevice9::SetDialogBoxMode()") );

			Prepare();
			fonts.OnReset();

			if (presentation.PresentationInterval != oldInterval)
			{
				Io::Log() <<
				(
					presentation.PresentationInterval == D3DPRESENT_INTERVAL_IMMEDIATE ? "Direct3D: disabling VSYNC\r\n" :
					presentation.PresentationInterval == D3DPRESENT_INTERVAL_TWO       ? "Direct3D: enabling VSYNC on second refresh\r\n" :
					presentation.PresentationInterval == D3DPRESENT_INTERVAL_THREE     ? "Direct3D: enabling VSYNC on third refresh\r\n" :
                                                                                         "Direct3D: enabling VSYNC\r\n"
				);
			}

			if (!presentation.Windowed && ::GetMenu( presentation.hDeviceWindow ))
				::DrawMenuBar( presentation.hDeviceWindow );

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
			com->SetRenderState( D3DRS_ZWRITEENABLE, false        );
			com->SetRenderState( D3DRS_COLORVERTEX,  false        );
			com->SetRenderState( D3DRS_CULLMODE,     D3DCULL_NONE );
			com->SetRenderState( D3DRS_LIGHTING,     false        );

			com->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );

			if (FAILED(com->SetFVF( VertexBuffer::FVF )))
				throw Application::Exception( IDS_FAILED, _T("IDirect3DDevice9::SetFVF()") );
		}

		uint Direct2D::Device::GetMaxMessageLength() const
		{
			return fonts.Width() ? (presentation.BackBufferWidth - fonts.Width() * 7) / fonts.Width() : 64;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		NST_FORCE_INLINE HRESULT Direct2D::Device::RenderScreen(const uint state,const uint numIndexedStrips,const uint numVertices) const
		{
			HRESULT hResult = com->BeginScene();

			if (SUCCEEDED(hResult))
			{
				if (state & RENDER_PICTURE)
				{
					if (numIndexedStrips)
						hResult = com->DrawIndexedPrimitive( D3DPT_TRIANGLESTRIP, 0, 0, numVertices, 0, numIndexedStrips );
					else
						hResult = com->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
				}

				fonts.Render( presentation, state );
				com->EndScene();
			}

			return hResult;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		HRESULT Direct2D::Device::Repair(const HRESULT lastError)
		{
			NST_ASSERT( FAILED(lastError) );

			uint id = 0;
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

							id = IDS_FAILED;
							msg = _T("IDirect3DDevice9::TestCooperativeLevel()");
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

			throw Application::Exception( id, msg );
		}

		bool Direct2D::Device::CanToggleDialogBoxMode(bool enable) const
		{
			return !presentation.Windowed && bool(presentation.Flags) != enable;
		}

		bool Direct2D::Device::CanSwitchFullscreen(const Mode& mode) const
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
			presentation.Windowed = false;
			presentation.BackBufferWidth = mode.width;
			presentation.BackBufferHeight = mode.height;
			presentation.BackBufferFormat = (mode.bpp == 16 ? D3DFMT_R5G6B5 : D3DFMT_X8R8G8B8);
			presentation.FullScreen_RefreshRateInHz = GetDesiredPresentationRate( mode );

			if (FAILED(Reset()))
				throw Application::Exception(_T("Couldn't switch display mode!"));

			fonts.Create( *this );
			LogDisplaySwitch();
		}

		HRESULT Direct2D::Device::ToggleDialogBoxMode()
		{
			NST_ASSERT( !presentation.Windowed );

			if (presentation.Flags == D3DPRESENTFLAG_LOCKABLE_BACKBUFFER)
			{
				presentation.Flags = 0;
				com->SetDialogBoxMode( false );
			}
			else
			{
				presentation.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
			}

			return Reset();
		}

		void Direct2D::Device::SwitchWindowed()
		{
			fonts.Destroy( false );

			presentation.Windowed = true;
			presentation.BackBufferWidth = 0;
			presentation.BackBufferHeight = 0;
			presentation.BackBufferFormat = D3DFMT_UNKNOWN;
			presentation.FullScreen_RefreshRateInHz = 0;

			if (presentation.Flags == D3DPRESENTFLAG_LOCKABLE_BACKBUFFER)
			{
				presentation.Flags = 0;
				com->SetDialogBoxMode( false );
			}

			if (FAILED(Reset()))
				throw Application::Exception(_T("Couldn't switch display mode!"));

			fonts.Create( *this );
			LogDisplaySwitch();
		}

		HRESULT Direct2D::Device::ResetWindowClient(const Point& client,HRESULT hResult)
		{
			NST_ASSERT( presentation.Windowed && client.x > 0 && client.y > 0 );

			presentation.BackBufferWidth = client.x;
			presentation.BackBufferHeight = client.y;

			if (SUCCEEDED(hResult) || hResult == INVALID_RECT)
				hResult = Reset();

			return hResult;
		}

		bool Direct2D::Device::ResetFrameRate(uint frameRate,bool vsync,bool tripleBuffering,const Base& base)
		{
			timing.frameRate = frameRate;
			timing.vsync = vsync;

			bool update = false;

			if (timing.tripleBuffering != tripleBuffering)
			{
				timing.tripleBuffering = tripleBuffering;
				update = true;
			}

			if (!presentation.Windowed)
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
					update = true;
				}
			}

			return update || presentation.PresentationInterval != GetDesiredPresentationInterval();
		}

		Direct2D::VertexBuffer::Vertex::Vertex()
		: z(0.f), rhw(1.f) {}

		inline uint Direct2D::VertexBuffer::NumVertices() const
		{
			return vertices.size();
		}

		void Direct2D::VertexBuffer::Update
		(
			const Rect& picture,
			const float clip[4],
			const float scale,
			const uint patches,
			const int screenCurvature
		)
		{
			NST_ASSERT( picture.Width() > 0 && picture.Height() > 0 && clip[2]-clip[0] > 0 && clip[3]-clip[1] > 0 );

			if (vertices.size() != (patches+1) * (patches+1))
			{
				vertices.resize( (patches+1) * (patches+1) );
				Invalidate();
			}

			rect = picture;

			if (patches > 1)
			{
				Vertex* NST_RESTRICT v = &vertices.front();

				const float z = 1.f - screenCurvature / 40.f;
				const D3DXVECTOR2 p0( rect.left - 0.5f, rect.top - 0.5f );
				const D3DXVECTOR2 p1( rect.right - 0.5f, rect.bottom - 0.5f );
				const D3DXVECTOR2 t( rect.Width() * z, rect.Height() * z );

				for (uint y=0; y <= patches; ++y)
				{
					D3DXVECTOR2 vy;

					float weight = float(y) / patches;
					::D3DXVec2Hermite( &vy, &p0, &t, &p1, &t, weight );

					vy.x = (clip[1] + (clip[3]-clip[1]) * weight) / scale;

					for (uint x=0; x <= patches; ++x, ++v)
					{
						D3DXVECTOR2 vx;

						weight = float(x) / patches;
						::D3DXVec2Hermite( &vx, &p0, &t, &p1, &t, weight );

						v->x = vx.x;
						v->y = vy.y;
						v->u = (clip[0] + (clip[2]-clip[0]) * weight) / scale;
						v->v = vy.x;
					}
				}
			}
			else
			{
				vertices[0].x = picture.left - 0.5f;
				vertices[0].y = picture.top - 0.5f;
				vertices[0].u = clip[0] / scale;
				vertices[0].v = clip[1] / scale;
				vertices[1].x = picture.left - 0.5f;
				vertices[1].y = picture.bottom - 0.5f;
				vertices[1].u = clip[0] / scale;
				vertices[1].v = clip[3] / scale;
				vertices[2].x = picture.right - 0.5f;
				vertices[2].y = picture.top - 0.5f;
				vertices[2].u = clip[2] / scale;
				vertices[2].v = clip[1] / scale;
				vertices[3].x = picture.right - 0.5f;
				vertices[3].y = picture.bottom - 0.5f;
				vertices[3].u = clip[2] / scale;
				vertices[3].v = clip[3] / scale;
			}
		}

		Direct2D::VertexBuffer::VertexBuffer()
		: vertices(4) {}

		Direct2D::VertexBuffer::~VertexBuffer()
		{
			Invalidate();
		}

		void Direct2D::VertexBuffer::Invalidate()
		{
			if (com != NULL)
			{
				IDirect3DDevice9* device;

				if (SUCCEEDED(com->GetDevice( &device )))
				{
					device->SetStreamSource( 0, NULL, 0, 0 );
					device->Release();
				}

				com.Release();
			}
		}

		HRESULT Direct2D::VertexBuffer::Validate(IDirect3DDevice9& device,bool dirty)
		{
			if (com == NULL)
			{
				const HRESULT hResult = device.CreateVertexBuffer
				(
					sizeof(Vertex) * vertices.size(),
					D3DUSAGE_WRITEONLY,
					FVF,
					D3DPOOL_DEFAULT,
					&com,
					NULL
				);

				if (SUCCEEDED(hResult))
				{
					if (FAILED(device.SetStreamSource( 0, *com, 0, sizeof(Vertex) )))
						throw Application::Exception( IDS_FAILED, _T("IDirect3DDevice9::SetStreamSource()") );

					dirty = true;
				}
				else if (hResult == D3DERR_DEVICELOST)
				{
					return D3DERR_DEVICELOST;
				}
				else
				{
					throw Application::Exception( IDS_FAILED, _T("IDirect3DDevice9::CreateVertexBuffer()") );
				}
			}

			if (dirty)
			{
				void* ram;
				const HRESULT hResult = com->Lock( 0, 0, &ram, D3DLOCK_NOSYSLOCK );

				if (SUCCEEDED(hResult))
				{
					std::memcpy( ram, &vertices.front(), vertices.size() * sizeof(Vertex) );
					com->Unlock();
				}
				else if (hResult == D3DERR_DEVICELOST)
				{
					return D3DERR_DEVICELOST;
				}
				else
				{
					throw Application::Exception( IDS_FAILED, _T("IDirect3DVertexBuffer9::Lock()") );
				}
			}

			return D3D_OK;
		}

		inline Direct2D::IndexBuffer::IndexBuffer()
		: strips(0), patches(1) {}

		Direct2D::IndexBuffer::~IndexBuffer()
		{
			Invalidate();
		}

		inline uint Direct2D::IndexBuffer::NumStrips() const
		{
			return strips;
		}

		void Direct2D::IndexBuffer::Update(uint p)
		{
			if (patches != p)
			{
				patches = p;
				strips = p > 1 ? ((p * 2 + 1) * p) - 1: 0;

				Invalidate();
			}
		}

		void Direct2D::IndexBuffer::Invalidate()
		{
			if (com != NULL)
			{
				IDirect3DDevice9* device;

				if (SUCCEEDED(com->GetDevice( &device )))
				{
					device->SetIndices( NULL );
					device->Release();
				}

				com.Release();
			}
		}

		HRESULT Direct2D::IndexBuffer::Validate(IDirect3DDevice9& device,bool dirty)
		{
			if (strips)
			{
				if (com == NULL)
				{
					const HRESULT hResult = device.CreateIndexBuffer
					(
						(((patches * 2 + 1) * patches) + 1) * sizeof(WORD),
						D3DUSAGE_WRITEONLY,
						D3DFMT_INDEX16,
						D3DPOOL_DEFAULT,
						&com,
						NULL
					);

					if (SUCCEEDED(hResult))
					{
						if (FAILED(device.SetIndices( *com )))
							throw Application::Exception( IDS_FAILED, _T("IDirect3DDevice9::SetIndices()") );

						dirty = true;
					}
					else if (hResult == D3DERR_DEVICELOST)
					{
						return D3DERR_DEVICELOST;
					}
					else
					{
						throw Application::Exception( IDS_FAILED, _T("IDirect3DDevice9::CreateIndexBuffer()") );
					}
				}

				if (dirty)
				{
					void* ptr;
					const HRESULT hResult = com->Lock( 0, 0, &ptr, D3DLOCK_NOSYSLOCK );

					if (SUCCEEDED(hResult))
					{
						WORD* NST_RESTRICT data = static_cast<WORD*>(ptr);

						for (uint p=0, i=0, n=patches+1;;)
						{
							uint j = i + n;

							do
							{
								*data++ = i;
								*data++ = i++ + n;
							}
							while (i < j);

							i += n - 1;

							do
							{
								*data++ = i-- + n;
								*data++ = i;
							}
							while (i > j);

							i += n;
							p += 2;

							if (p == patches)
							{
								*data++ = i;
								break;
							}
						}

						com->Unlock();
					}
					else if (hResult == D3DERR_DEVICELOST)
					{
						return D3DERR_DEVICELOST;
					}
					else
					{
						throw Application::Exception( IDS_FAILED, _T("IDirect3DIndexBuffer9::Lock()") );
					}
				}
			}
			else
			{
				Invalidate();
			}

			return D3D_OK;
		}

		Direct2D::Texture::Texture(const D3DFORMAT backBufferFormat)
		:
		width     (256),
		height    (256),
		size      (256),
		useVidMem (true),
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

		void Direct2D::Texture::Update(const Point& screen,const Adapter::Filter f,const bool wantVidMem)
		{
			NST_ASSERT( screen.x > 0 && screen.y > 0 );

			width = screen.x;
			height = screen.y;
			filter = f;

			uint pow = NST_MAX(width,height);

			--pow;
			pow |= pow >> 1;
			pow |= pow >> 2;
			pow |= pow >> 4;
			pow |= pow >> 8;
			pow |= pow >> 16;
			++pow;

			if (size != pow || useVidMem != wantVidMem)
			{
				size = pow;
				useVidMem = wantVidMem;
				Invalidate();
			}
		}

		void Direct2D::Texture::Invalidate()
		{
			if (com != NULL)
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
						throw Application::Exception( IDS_FAILED, _T("IDirect3DDevice9::GetLevelDesc()") );

					format = desc.Format;
					lockFlags = (desc.Usage & D3DUSAGE_DYNAMIC) ? (D3DLOCK_DISCARD|D3DLOCK_NOSYSLOCK) : D3DLOCK_NOSYSLOCK;

					if (desc.Height < size || desc.Width < size)
						throw Application::Exception(_T("Maximum texture dimension too small!"));

					if (!GetBitsPerPixel())
						throw Application::Exception(_T("Unsupported bits-per-pixel format!"));

					if (FAILED(device.SetTexture( 0, *com )))
						throw Application::Exception( IDS_FAILED, _T("IDirect3DDevice9::SetTexture()") );
				}
				else if (hResult == D3DERR_DEVICELOST)
				{
					return D3DERR_DEVICELOST;
				}
				else
				{
					throw Application::Exception( IDS_FAILED, _T("IDirect3DDevice9::CreateTexture()") );
				}
			}

			const DWORD type = (filter == Adapter::FILTER_NONE ? D3DTEXF_POINT : D3DTEXF_LINEAR);

			device.SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
			device.SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
			device.SetSamplerState( 0, D3DSAMP_MINFILTER, type );
			device.SetSamplerState( 0, D3DSAMP_MAGFILTER, type );

			return D3D_OK;
		}

		bool Direct2D::Texture::SaveToFile(tstring const file,const D3DXIMAGE_FILEFORMAT type) const
		{
			NST_ASSERT( file && *file && width && height );

			if (com == NULL)
				return false;

			ComInterface<IDirect3DSurface9> surface;
			const RECT rect = {0,0,width,height};

			uint bitFormat = 0;

			switch (format)
			{
				case D3DFMT_A8R8G8B8:
				case D3DFMT_X8R8G8B8:

					bitFormat = 32;
					break;

				case D3DFMT_R5G6B5:
				case D3DFMT_X1R5G5B5:
				case D3DFMT_A1R5G5B5:

					bitFormat = 16;
					break;
			}

			if (bitFormat == 32 || (bitFormat == 16 && type == D3DXIFF_BMP))
			{
				{
					ComInterface<IDirect3DDevice9> device;

					if (FAILED(com->GetDevice( &device )))
						return false;

					if (FAILED(device->CreateOffscreenPlainSurface( width, height, D3DFMT_R8G8B8, D3DPOOL_SCRATCH, &surface, NULL )))
						return false;
				}

				D3DLOCKED_RECT dstLock, srcLock;

				if (FAILED(surface->LockRect( &dstLock, NULL, D3DLOCK_NOSYSLOCK )))
					return false;

				if (FAILED(com->LockRect( 0, &srcLock, &rect, D3DLOCK_READONLY|D3DLOCK_NOSYSLOCK )))
				{
					surface->UnlockRect();
					return false;
				}

				if (bitFormat == 16)
				{
					const uint g = (format == D3DFMT_R5G6B5 ?  2 :  3);
					const uint b = (format == D3DFMT_R5G6B5 ? 11 : 10);

					for (uint y=0; y < height; ++y)
					{
						const WORD* NST_RESTRICT src = static_cast<WORD*>(srcLock.pBits);
						BYTE* NST_RESTRICT dst = static_cast<BYTE*>(dstLock.pBits);

						for (const BYTE* const end=dst+width*3; dst != end; dst += 3, ++src)
						{
							const uint p = *src;

							dst[0] = p << 3 & 0xFF;
							dst[1] = p >> 5 << g & 0xFF;
							dst[2] = p >> b << 3 & 0xFF;
						}

						srcLock.pBits = static_cast<BYTE*>(srcLock.pBits) + srcLock.Pitch;
						dstLock.pBits = static_cast<BYTE*>(dstLock.pBits) + dstLock.Pitch;
					}
				}
				else
				{
					for (uint y=0; y < height; ++y)
					{
						const BYTE* NST_RESTRICT src = static_cast<BYTE*>(srcLock.pBits);
						BYTE* NST_RESTRICT dst = static_cast<BYTE*>(dstLock.pBits);

						for (const BYTE* const end=dst+width*3; dst != end; dst += 3, src += 4)
							std::memcpy( dst, src, 3 );

						srcLock.pBits = static_cast<BYTE*>(srcLock.pBits) + srcLock.Pitch;
						dstLock.pBits = static_cast<BYTE*>(dstLock.pBits) + dstLock.Pitch;
					}
				}

				com->UnlockRect( 0 );
				surface->UnlockRect();
			}
			else if (FAILED(com->GetSurfaceLevel( 0, &surface )))
			{
				return false;
			}

			return SUCCEEDED(::D3DXSaveSurfaceToFile( file, type, *surface, NULL, &rect ));
		}

		Direct2D::Direct2D(HWND hWnd)
		:
		device     ( hWnd, base ),
		texture    ( device.GetPresentation().BackBufferFormat ),
		lastResult ( D3D_OK )
		{
			ValidateObjects();
		}

		Direct2D::~Direct2D()
		{
		}

		void Direct2D::InvalidateObjects()
		{
			indexBuffer.Invalidate();
			vertexBuffer.Invalidate();
			texture.Invalidate();
		}

		void Direct2D::ValidateObjects()
		{
			if (SUCCEEDED(lastResult))
			{
				lastResult = vertexBuffer.Validate( device );

				if (SUCCEEDED(lastResult))
				{
					lastResult = indexBuffer.Validate( device );

					if (SUCCEEDED(lastResult))
						lastResult = texture.Validate( device, GetAdapter(), device.GetPresentation().BackBufferFormat );
				}
			}
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		void Direct2D::RenderScreen(uint state)
		{
			if (SUCCEEDED(lastResult))
				lastResult = device.RenderScreen( state, indexBuffer.NumStrips(), vertexBuffer.NumVertices() );
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		void Direct2D::SelectAdapter(const Adapters::const_iterator adapter)
		{
			if (device.GetOrdinal() != adapter->ordinal)
			{
				InvalidateObjects();
				device.Create( base, *adapter );
				lastResult = INVALID_RECT;
			}
		}

		bool Direct2D::CanSwitchFullscreen(const Adapter::Modes::const_iterator mode) const
		{
			return device.CanSwitchFullscreen( *mode );
		}

		bool Direct2D::SwitchFullscreen(const Adapter::Modes::const_iterator mode)
		{
			if (CanSwitchFullscreen( mode ))
			{
				InvalidateObjects();
				device.SwitchFullscreen( *mode );
				lastResult = D3D_OK;
				ValidateObjects();
				return true;
			}

			return false;
		}

		bool Direct2D::SwitchWindowed()
		{
			if (!device.GetPresentation().Windowed)
			{
				InvalidateObjects();
				device.SwitchWindowed();
				lastResult = D3D_OK;
				ValidateObjects();
				return true;
			}

			return false;
		}

		void Direct2D::EnableDialogBoxMode(const bool enable)
		{
			if (device.CanToggleDialogBoxMode( enable ))
			{
				InvalidateObjects();
				lastResult = device.ToggleDialogBoxMode();
				ValidateObjects();
			}
		}

		bool Direct2D::Reset()
		{
			if (FAILED(lastResult) && lastResult != INVALID_RECT)
			{
				InvalidateObjects();
				lastResult = device.Repair( lastResult );
				ValidateObjects();
			}

			return SUCCEEDED(lastResult);
		}

		void Direct2D::UpdateWindowView()
		{
			const Point::Picture picture( device.GetPresentation().hDeviceWindow );

			if (picture.x > 0 && picture.y > 0)
			{
				const Point::Client client( device.GetPresentation().hDeviceWindow );
				NST_ASSERT( client.x >= picture.x && client.y >= picture.y );

				if
				(
					client.x != device.GetPresentation().BackBufferWidth ||
					client.y != device.GetPresentation().BackBufferHeight ||
					lastResult == INVALID_RECT
				)
				{
					InvalidateObjects();
					lastResult = device.ResetWindowClient( client, lastResult );
				}

				ValidateObjects();
			}
			else
			{
				lastResult = INVALID_RECT;
			}
		}

		void Direct2D::UpdateWindowView
		(
			const Point& screen,
			const float clipping[4],
			const int screenCurvature,
			const Adapter::Filter filter,
			const bool useVidMem
		)
		{
			const Point::Picture picture( device.GetPresentation().hDeviceWindow );

			if (picture.x > 0 && picture.y > 0)
			{
				texture.Update( screen, filter, useVidMem );
				vertexBuffer.Update( picture, clipping, texture.Size(), screenCurvature ? TSL_PATCHES : 1, screenCurvature );
				indexBuffer.Update( screenCurvature ? TSL_PATCHES : 1 );

				const Point::Client client( device.GetPresentation().hDeviceWindow );
				NST_ASSERT( client.x >= picture.x && client.y >= picture.y );

				if
				(
					client.x != device.GetPresentation().BackBufferWidth ||
					client.y != device.GetPresentation().BackBufferHeight ||
					lastResult == INVALID_RECT
				)
				{
					InvalidateObjects();
					lastResult = device.ResetWindowClient( client, lastResult );
				}

				ValidateObjects();
			}
			else
			{
				lastResult = INVALID_RECT;
			}
		}

		void Direct2D::UpdateFullscreenView
		(
			const Rect& picture,
			const Point& screen,
			const float clipping[4],
			const int screenCurvature,
			const Adapter::Filter filter,
			const bool useVidMem
		)
		{
			NST_ASSERT( picture.Width() && picture.Height() );

			texture.Update( screen, filter, useVidMem );
			vertexBuffer.Update( picture, clipping, texture.Size(), screenCurvature ? TSL_PATCHES : 1, screenCurvature );
			indexBuffer.Update( screenCurvature ? TSL_PATCHES : 1 );
			ValidateObjects();
		}

		void Direct2D::UpdateFrameRate(const uint frameRate,const bool vsync,const bool tripleBuffering)
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
}
