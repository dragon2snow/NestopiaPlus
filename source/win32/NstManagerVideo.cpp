////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
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

#include "NstManagerPaths.hpp"
#include "NstResourceString.hpp"
#include "NstWindowParam.hpp"
#include "NstWindowUser.hpp"
#include "NstDialogVideo.hpp"
#include "NstManagerVideo.hpp"
#include "NstIoScreen.hpp"

#ifdef __INTEL_COMPILER
#pragma warning( disable : 279 )
#endif

namespace Nestopia
{
	namespace Managers
	{
		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		struct Video::Callbacks
		{
			static bool NST_CALLBACK ScreenLock(Nes::Video::UserData data,Nes::Video::Output& output)
			{
				NST_ASSERT( data );

				return static_cast<DirectX::Direct2D*>(data)->LockScreen( output.pixels, output.pitch );
			}

			static void NST_CALLBACK ScreenUnlock(Nes::Video::UserData data,Nes::Video::Output&)
			{
				NST_ASSERT( data );

				static_cast<DirectX::Direct2D*>(data)->UnlockScreen();
				static_cast<DirectX::Direct2D*>(data)->RenderScreen( DirectX::Direct2D::RENDER_PICTURE|DirectX::Direct2D::RENDER_FPS|DirectX::Direct2D::RENDER_MSG );
			}
		};

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif

		inline Video::Fps::Fps()
		: frame(0) {}

		inline Video::Nsf::Nsf()
		: songTextOffset(0) {}

		void Video::Nsf::Load(const Nes::Nsf emulator)
		{
			struct Info
			{
				static cstring GoodName(cstring text)
				{
					if
					(
						text && *text &&
						std::strcmp( text, "<?>"   ) &&
						std::strcmp( text, "< ? >" ) &&
						std::strcmp( text, "?"     )
					)
						return text;

					return NULL;
				}

				static cstring GetMode(Nes::Nsf::TuneMode mode)
				{
					return
					(
						mode == Nes::Nsf::TUNE_MODE_PAL  ? " (PAL)" :
						mode == Nes::Nsf::TUNE_MODE_NTSC ? " (NTSC)" :
                                                           " (NTSC/PAL)"
					);
				}
			};

			text.Clear();

			if (cstring name = Info::GoodName( emulator.GetName() ))
				text.Import( name );
			else
				text = "noname";

			text << Info::GetMode( emulator.GetMode() );

			cstring const artist = Info::GoodName( emulator.GetArtist() );
			cstring const maker = Info::GoodName( emulator.GetMaker() );

			if (artist)
				(text << "\r\n").Import( artist );

			if (maker)
				(text << "\r\n").Import( maker );

			if (const uint chips = emulator.GetChips())
			{
				text << (maker || artist ? ", " : "\r\n");

				if ( chips & Nes::Nsf::CHIP_MMC5 ) text << "MMC5 ";
				if ( chips & Nes::Nsf::CHIP_FDS  )  text << "FDS ";
				if ( chips & Nes::Nsf::CHIP_VRC6 )  text << "VRC6 ";
				if ( chips & Nes::Nsf::CHIP_VRC7 )  text << "VRC7 ";
				if ( chips & Nes::Nsf::CHIP_N106 )  text << "N106 ";
				if ( chips & Nes::Nsf::CHIP_S5B  )  text << "Sunsoft5B ";

				text << ((chips & (chips-1)) ? "chips" : "chip");
			}

			text << "\r\nSong: ";
			songTextOffset = text.Length();

			Update( emulator );
		}

		void Video::Nsf::Update(const Nes::Nsf emulator)
		{
			text.ShrinkTo( songTextOffset );
			text << (emulator.GetCurrentSong() + 1) << '/' << emulator.GetNumSongs();
		}

		Video::Video
		(
			Window::Custom& w,
			Window::Menu& m,
			Emulator& e,
			const Paths& p,
			const Configuration& cfg
		)
		:
		emulator               ( e ),
		window                 ( w ),
		menu                   ( m ),
		statusBar              ( w, STATUSBAR_WIDTH ),
		direct2d               ( w ),
		dialog                 ( new Window::Video( e, direct2d.GetAdapters(), p, cfg ) ),
		sizingMoving           ( false ),
		paths                  ( p ),
		childWindowSwitchCount ( Application::Instance::NumChildWindows() + 1 )
		{
			static const Window::MsgHandler::Entry<Video> messages[] =
			{
				{ WM_PAINT,          &Video::OnPaint         },
				{ WM_NCPAINT,        &Video::OnNcPaint       },
				{ WM_ERASEBKGND,     &Video::OnEraseBkGnd    },
				{ WM_DISPLAYCHANGE,  &Video::OnDisplayChange }
			};

			static const Window::MsgHandler::HookEntry<Video> hooks[] =
			{
				{ WM_ENTERSIZEMOVE, &Video::OnEnterSizeMove },
				{ WM_EXITSIZEMOVE,  &Video::OnExitSizeMove  }
			};

			static const Window::Menu::CmdHandler::Entry<Video> commands[] =
			{
				{ IDM_OPTIONS_VIDEO,                    &Video::OnCmdOptionsVideo            },
				{ IDM_MACHINE_OPTIONS_UNLIMITEDSPRITES, &Video::OnCmdMachineUnlimitedSprites },
				{ IDM_FILE_SAVE_SCREENSHOT,             &Video::OnCmdFileScreenShot          },
				{ IDM_VIEW_WINDOWSIZE_1X,               &Video::OnCmdViewScreenSize          },
				{ IDM_VIEW_WINDOWSIZE_2X,               &Video::OnCmdViewScreenSize          },
				{ IDM_VIEW_WINDOWSIZE_3X,               &Video::OnCmdViewScreenSize          },
				{ IDM_VIEW_WINDOWSIZE_4X,               &Video::OnCmdViewScreenSize          },
				{ IDM_VIEW_WINDOWSIZE_5X,               &Video::OnCmdViewScreenSize          },
				{ IDM_VIEW_WINDOWSIZE_6X,               &Video::OnCmdViewScreenSize          },
				{ IDM_VIEW_WINDOWSIZE_7X,               &Video::OnCmdViewScreenSize          },
				{ IDM_VIEW_WINDOWSIZE_8X,               &Video::OnCmdViewScreenSize          },
				{ IDM_VIEW_WINDOWSIZE_9X,               &Video::OnCmdViewScreenSize          },
				{ IDM_VIEW_WINDOWSIZE_MAX,              &Video::OnCmdViewScreenSize          },
				{ IDM_VIEW_WINDOWSIZE_TVASPECT,         &Video::OnCmdViewTvAspect            },
				{ IDM_VIEW_STATUSBAR,                   &Video::OnCmdViewStatusBar           },
				{ IDM_VIEW_FPS,                         &Video::OnCmdViewFps                 }
			};

			static const Window::Menu::PopupHandler::Entry<Video> popups[] =
			{
				{ Window::Menu::PopupHandler::Pos<IDM_POS_VIEW,IDM_POS_VIEW_SCREENSIZE>::ID, &Video::OnMenuScreenSizes },
				{ Window::Menu::PopupHandler::Pos<IDM_POS_MACHINE,IDM_POS_MACHINE_OPTIONS>::ID, &Video::OnMenuUnlimSprites }
			};

			m.Commands().Add( this, commands );
			m.PopupRouter().Add( this, popups );
			w.Messages().Add( this, messages, hooks );
			emulator.Events().Add( this, &Video::OnEmuEvent );
			Instance::Events::Add( this, &Video::OnAppEvent );

			Io::Screen::SetCallback( this, &Video::OnScreenText );

			Nes::Video::Output::lockCallback.Set( &Callbacks::ScreenLock, &direct2d );
			Nes::Video::Output::unlockCallback.Set( &Callbacks::ScreenUnlock, &direct2d );

			direct2d.EnableAutoFrequency( dialog->UseAutoFrequency() );
			direct2d.SelectAdapter( dialog->GetAdapter() );

			if (cfg["view show status bar"] != Configuration::NO)
			{
				menu[IDM_VIEW_STATUSBAR].Check();
				statusBar.Enable();
			}
			else
			{
				menu[IDM_VIEW_FPS].Disable();
			}

			if (cfg["view show fps"] == Configuration::YES)
				menu[IDM_VIEW_FPS].Check();

			if (cfg["machine no sprite limit"] == Configuration::YES)
				Nes::Video(emulator).EnableUnlimSprites( true );

			{
				const GenericString type( cfg["view size window"] );
				ResetScreenRect( (type.Length() == 1 && type[0] >= '2' && type[0] <= '9') ? type[0] - '1' : 0 );
			}

			menu[IDM_VIEW_WINDOWSIZE_TVASPECT].Check( dialog->TvAspect() );

			UpdateMenuScreenSizes( GetDisplayMode() );
		}

		Video::~Video()
		{
			emulator.Events().Remove( this );
			Instance::Events::Remove( this );

			Io::Screen::UnsetCallback();

			window.Messages().Remove( this );
			window.StopTimer( this, &Video::OnTimerText );
		}

		void Video::Save(Configuration& cfg,const Rect& client) const
		{
			cfg[ "view show status bar" ].YesNo() = menu[IDM_VIEW_STATUSBAR].Checked();
			cfg[ "view show fps" ].YesNo() = menu[IDM_VIEW_FPS].Checked();
			cfg[ "machine no sprite limit" ].YesNo() = Nes::Video(emulator).AreUnlimSpritesEnabled();

			{
				const uint scale = Point(dialog->GetNesRect()).ScaleToFit( client, Point::SCALE_NEAREST );

				cfg["view size window"] = (scale <= 8 ? scale + 1 : 1);
			}

			dialog->Save( cfg );
		}

		void Video::LoadPalette(const Path& path)
		{
			dialog->LoadGamePalette( path );
		}

		void Video::SavePalette(Path& path) const
		{
			dialog->SavePalette( path );
		}

		Video::Point Video::GetDisplayMode() const
		{
			return Fullscreen() ? direct2d.GetFullscreenDisplayMode() : Point( ::GetSystemMetrics( SM_CXSCREEN ), ::GetSystemMetrics( SM_CYSCREEN ) );
		}

		uint Video::GetMaxMessageLength() const
		{
			return Fullscreen() ? direct2d.GetMaxMessageLength() : statusBar.Enabled() ? statusBar.GetMaxMessageLength() : 0;
		}

		void Video::OnCmdOptionsVideo(uint)
		{
			const Rect oldRect( dialog->GetNesRect() );

			dialog->Open();

			direct2d.EnableAutoFrequency( dialog->UseAutoFrequency() );
			direct2d.SelectAdapter( dialog->GetAdapter() );

			if (Windowed() || !SwitchFullscreen( dialog->GetMode() ))
			{
				if (oldRect != dialog->GetNesRect())
				{
					UpdateMenuScreenSizes( GetDisplayMode() );

					if (Windowed() && window.Restored())
						ResetScreenRect( CalculateWindowScale() );
				}

				window.Redraw();
			}
		}

		void Video::OnCmdViewScreenSize(uint id)
		{
			ResetScreenRect( id == IDM_VIEW_WINDOWSIZE_MAX ? Window::Video::SCREEN_STRETCHED : id - IDM_VIEW_WINDOWSIZE_1X );

			Application::Instance::GetMainWindow().Post( Application::Instance::WM_NST_COMMAND_RESUME );
		}

		void Video::OnCmdViewTvAspect(uint)
		{
			menu[IDM_VIEW_WINDOWSIZE_TVASPECT].Check( dialog->ToggleTvAspect() );

			if (Windowed())
				window.PostCommand( IDM_VIEW_WINDOWSIZE_1X + CalculateWindowScale() );
			else
				window.Redraw();

			Application::Instance::GetMainWindow().Post( Application::Instance::WM_NST_COMMAND_RESUME );
		}

		void Video::OnCmdViewStatusBar(uint)
		{
			NST_VERIFY( Windowed() );

			if (Windowed())
			{
				const ibool enable = menu[IDM_VIEW_STATUSBAR].ToggleCheck();
				menu[IDM_VIEW_FPS].Enable( enable );

				Point size;

				if (enable)
				{
					statusBar.Enable( true, false );
					size.y = statusBar.Height();
				}
				else
				{
					size.y = statusBar.Height();
					statusBar.Disable();
				}

				if (window.Restored())
				{
					if (enable)
						window.Size() += size;
					else
						window.Size() -= size;
				}
				else if (!enable)
				{
					window.Redraw();
				}

				if (enable)
					statusBar.Show();

				if (emulator.Running() && emulator.Is(Nes::Machine::GAME) && menu[IDM_VIEW_FPS].Checked())
					ToggleFps( enable );
			}
		}

		void Video::OnCmdViewFps(uint)
		{
			NST_VERIFY( Fullscreen() || statusBar.Enabled() );

			if (Fullscreen() || statusBar.Enabled())
			{
				const ibool enable = menu[IDM_VIEW_FPS].ToggleCheck();

				if (emulator.Running() && emulator.Is(Nes::Machine::GAME))
					ToggleFps( enable );
			}
		}

		void Video::OnCmdMachineUnlimitedSprites(uint)
		{
			const ibool enable = !Nes::Video(emulator).AreUnlimSpritesEnabled();
			Nes::Video(emulator).EnableUnlimSprites( enable );
			Io::Screen() << Resource::String(enable ? IDS_SCREEN_NOSPRITELIMIT_ON : IDS_SCREEN_NOSPRITELIMIT_OFF );
		}

		void Video::OnCmdFileScreenShot(uint)
		{
			if (emulator.Is(Nes::Machine::GAME,Nes::Machine::ON))
			{
				const Path path( paths.GetScreenShotPath() );
				const DirectX::Direct2D::ScreenShotResult result = direct2d.SaveScreenShot( path.Ptr(), path.Extension().Id() );

				if (result == DirectX::Direct2D::SCREENSHOT_OK)
				{
					const uint length = GetMaxMessageLength();

					if (length > 22)
						Io::Screen() << Resource::String(IDS_SCREEN_SCREENSHOT_SAVED_TO).Invoke( Path::Compact( path, length - 20 ) );
				}
				else
				{
					Window::User::Fail
					(
						result == DirectX::Direct2D::SCREENSHOT_UNSUPPORTED ? IDS_SCREENSHOT_UNSUPPORTED_FORMAT :
                                                                              IDS_SCREENSHOT_SAVE_FAILED
					);
				}
			}
		}

		uint Video::CalculateWindowScale() const
		{
			return Point(dialog->GetNesRect().Size()).ScaleToFit( window.PictureCoordinates(), Point::SCALE_NEAREST );
		}

		uint Video::CalculateFullscreenScale() const
		{
			const Point screen( GetDisplayMode() );
			return Point(dialog->GetNesRect()).ScaleToFit( screen + (screen / SCALE_TOLERANCE), Point::SCALE_BELOW, dialog->GetFullscreenScale() );
		}

		void Video::OnMenuScreenSizes(Window::Menu::PopupHandler::Param& param)
		{
			uint scale;

			if (Fullscreen())
			{
				if (dialog->GetFullscreenScale() == Window::Video::SCREEN_STRETCHED)
					scale = Window::Video::SCREEN_STRETCHED;
				else
					scale = CalculateFullscreenScale();
			}
			else
			{
				if (window.Maximized())
				{
					scale = Window::Video::SCREEN_STRETCHED;
				}
				else if (WindowMatched())
				{
					scale = CalculateWindowScale();
				}
				else
				{
					scale = 0xBEDBABE;
				}
			}

			ibool check;

			if (scale == Window::Video::SCREEN_STRETCHED)
			{
				scale = IDM_VIEW_WINDOWSIZE_MAX;
				check = true;
			}
			else if (scale < IDM_VIEW_WINDOWSIZE_MAX-IDM_VIEW_WINDOWSIZE_1X)
			{
				scale += IDM_VIEW_WINDOWSIZE_1X;
				check = true;
			}
			else
			{
				scale = IDM_VIEW_WINDOWSIZE_1X;
				check = false;
			}

			param.menu[scale].Check( IDM_VIEW_WINDOWSIZE_1X, IDM_VIEW_WINDOWSIZE_MAX, check );
		}

		void Video::OnMenuUnlimSprites(Window::Menu::PopupHandler::Param& param)
		{
			param.menu[IDM_MACHINE_OPTIONS_UNLIMITEDSPRITES].Check( Nes::Video(emulator).AreUnlimSpritesEnabled() );
		}

		ibool Video::WindowMatched() const
		{
			const Point output( window.PictureCoordinates() );
			const Point input( dialog->GetNesRect().Size() );

			return
			(
				output.x && (output.x % input.x) == 0 &&
				output.y && (output.y % input.y) == 0
			);
		}

		void Video::ResetScreenRect(const uint scale)
		{
			if (Fullscreen())
			{
				if (dialog->GetFullscreenScale() != scale)
				{
					dialog->SetFullscreenScale( scale );
					window.Redraw();
				}
			}
			else if (scale == Window::Video::SCREEN_STRETCHED)
			{
				window.Maximize();
			}
			else
			{
				if (!window.Restored())
					window.Restore();

				Point size( dialog->GetNesRect() );
				size.ScaleToFit( GetDisplayMode(), Point::SCALE_BELOW, scale );

				window.Size() = size + window.NonClientCoordinates();

				if (window.PictureCoordinates() != size)
					window.Size() = size + window.NonClientCoordinates();
			}
		}

		void Video::UpdateMenuScreenSizes(Point screen) const
		{
			for (uint i=IDM_VIEW_WINDOWSIZE_1X; i < IDM_VIEW_WINDOWSIZE_MAX; ++i)
				menu[i].Remove();

			const Point original( dialog->GetNesRect() );
			Point nes( original );

			if (Fullscreen())
				screen += screen / SCALE_TOLERANCE;

			for (uint i=0; i < (IDM_VIEW_WINDOWSIZE_MAX-IDM_VIEW_WINDOWSIZE_1X); ++i)
			{
				menu.Insert( menu[IDM_VIEW_WINDOWSIZE_MAX], IDM_VIEW_WINDOWSIZE_1X + i, Resource::String(IDS_MENU_X).Invoke( tchar('1'+i) ) );
				nes = original * (i+2);

				if (nes.x > screen.x || nes.y > screen.y)
					break;
			}
		}

		ibool Video::OnDisplayChange(Window::Param& param)
		{
			UpdateMenuScreenSizes( Point(LOWORD(param.lParam),HIWORD(param.lParam)) );
			return true;
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		ibool Video::MustClearFrameScreen() const
		{
			return Fullscreen() && dialog->GetFullscreenScale() != Window::Video::SCREEN_STRETCHED;
		}

		ibool Video::OnPaint(Window::Param&)
		{
			if (emulator.Is(Nes::Machine::ON))
			{
				if (!sizingMoving)
					UpdateScreen();

				if (emulator.Is(Nes::Machine::GAME))
				{
					if (MustClearFrameScreen())
						ClearScreen();

					if (!sizingMoving && emulator.Running())
					{
						window.Redraw( false );
						return true;
					}

					Nes::Video( emulator ).Blit( nesOutput );
				}
				else
				{
					ClearScreen();
					direct2d.RenderScreen( DirectX::Direct2D::RENDER_MSG|DirectX::Direct2D::RENDER_NFO );
				}
			}
			else
			{
				ClearScreen();
				direct2d.RenderScreen( DirectX::Direct2D::RENDER_MSG );
			}

			window.Redraw( false );
			PresentScreen();

			return true;
		}

		void Video::UpdateDialogBoxMode()
		{
			direct2d.EnableDialogBoxMode
			(
				!emulator.Is(Nes::Machine::ON) ||
				emulator.Is(Nes::Machine::SOUND) ||
				menu.Visible()
			);
		}

		ibool Video::OnNcPaint(Window::Param&)
		{
			UpdateDialogBoxMode();
			return false;
		}

		ibool Video::OnEraseBkGnd(Window::Param& param)
		{
			param.lResult = true;
			return true;
		}

		void Video::OnEnterSizeMove(Window::Param&)
		{
			sizingMoving = Windowed();
		}

		void Video::OnExitSizeMove(Window::Param&)
		{
			sizingMoving = false;
			window.Redraw();
		}

		uint Video::OnTimerText()
		{
			if (Fullscreen())
			{
				direct2d.ClearMsg();
				window.Redraw();
			}
			else if (statusBar.Enabled())
			{
				statusBar.Text(Window::StatusBar::FIRST_FIELD).Clear();
			}

			return false;
		}

		uint Video::OnTimerFps()
		{
			if (emulator.Is(Nes::Machine::ON,Nes::Machine::GAME))
			{
				static HeapString string( Resource::String(IDS_TEXT_FPS) << ": " );

				{
					uint current = emulator.GetFrame();

					uint delta = current - fps.frame;
					fps.frame = current;

					current = delta / (Fps::UPDATE_INTERVAL / 1000);
					delta = delta % (Fps::UPDATE_INTERVAL / 1000) ? '5' : '0';

					string(5) = NST_MIN(current,999);
					string << '.' << tchar(delta);
				}

				if (Fullscreen())
				{
					direct2d.DrawFps( string(5) );
					return true;
				}
				else if (statusBar.Enabled())
				{
					statusBar.Text(Window::StatusBar::SECOND_FIELD) << string.Ptr();
					return true;
				}
			}

			return false;
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif

		void Video::RepairScreen()
		{
			if (direct2d.Reset())
			{
				if (emulator.Is(Nes::Machine::ON))
					UpdateScreen();

				direct2d.ClearScreen();
			}
		}

		ibool Video::SwitchFullscreen(Mode mode)
		{
			const ibool prevFullscreen = Fullscreen();
			const ibool toggleMenu = menu.Visible() && direct2d.CanSwitchFullscreen( mode );

			if (toggleMenu)
				menu.Hide();

			const ibool switched = direct2d.SwitchFullscreen( mode );

			if (switched && prevFullscreen && dialog->GetFullscreenScale() != Window::Video::SCREEN_STRETCHED)
				dialog->SetFullscreenScale( Window::Video::SCREEN_MATCHED );

			if (toggleMenu)
				menu.Show();

			window.Redraw();

			return switched;
		}

		void Video::SwitchScreen()
		{
			menu[IDM_VIEW_STATUSBAR].Enable( Fullscreen() );

			if (Windowed())
			{
				if (statusBar.Enabled())
					statusBar.Disable();
				else
					menu[IDM_VIEW_FPS].Enable();

				SwitchFullscreen( dialog->GetMode() );
			}
			else
			{
				direct2d.SwitchWindowed();

				if (menu[IDM_VIEW_STATUSBAR].Checked())
					statusBar.Enable();
				else
					menu[IDM_VIEW_FPS].Disable();
			}
		}

		void Video::OnAppEvent(Instance::Event event,const void* param)
		{
			if (Fullscreen())
			{
				switch (event)
				{
					case Instance::EVENT_WINDOW_CREATE:

						if (Instance::NumChildWindows() == childWindowSwitchCount)
						{
							const Instance::Events::WindowCreateParam& info =
							(
								*static_cast<const Instance::Events::WindowCreateParam*>(param)
							);

							const Point mode( GetDisplayMode() );

							if
							(
								(mode.x < MIN_DIALOG_WIDTH || mode.y < MIN_DIALOG_HEIGHT) &&
								(mode.x < int(info.x) || mode.y < int(info.y))
							)
								SwitchFullscreen( dialog->GetDialogMode() );
						}

						menu.Show();
						break;

					case Instance::EVENT_WINDOW_DESTROY:

						if (Instance::NumChildWindows() == childWindowSwitchCount)
						{
							SwitchFullscreen( dialog->GetMode() );
							UpdateDialogBoxMode();
						}
						break;
				}
			}
		}

		const Video::Rect& Video::GetInputRect() const
		{
			return dialog->GetInputRect();
		}

		void Video::UpdateScreen()
		{
			NST_ASSERT( emulator.Is(Nes::Machine::ON) );

			if (emulator.Is(Nes::Machine::GAME))
			{
				Rect picture;

				if (Windowed())
				{
					picture = window.PictureCoordinates();
				}
				else
				{
					const Point screen( GetDisplayMode() );

					if (dialog->GetFullscreenScale() == Window::Video::SCREEN_STRETCHED)
					{
						picture = screen;
					}
					else // scale up and center to screen
					{
						Point nesPoint( dialog->GetNesRect() );
						nesPoint.ScaleToFit( screen + (screen / SCALE_TOLERANCE), Point::SCALE_BELOW, dialog->GetFullscreenScale() );
						picture = nesPoint;
						picture.Center() = screen.Center();

						if (nesPoint.x > screen.x + (screen.x / SCALE_TOLERANCE))
						{
							picture.left = 0;
							picture.right = screen.x;
						}

						if (nesPoint.y > screen.y + (screen.y / SCALE_TOLERANCE))
						{
							picture.top = 0;
							picture.bottom = screen.y;
						}
					}
				}

				Nes::Video::RenderState renderState;

				renderState.bits.count = direct2d.GetBitsPerPixel();
				NST_ASSERT( renderState.bits.count == 16 || renderState.bits.count == 32 );

				direct2d.GetBitMask( renderState.bits.mask.r, renderState.bits.mask.g, renderState.bits.mask.b );
				NST_ASSERT( renderState.bits.mask.r && renderState.bits.mask.g && renderState.bits.mask.b );

				float nesScreen[4];
				dialog->GetRenderState( renderState, nesScreen, picture.Size() );
				NST_ASSERT( direct2d.GetAdapter().maxScreenSize >= NST_MAX(renderState.width,renderState.height) );

				Nes::Video(emulator).SetRenderState( renderState );

				if (Windowed())
				{
					direct2d.UpdateWindowView
					(
						Point(renderState.width,renderState.height),
						nesScreen,
						dialog->GetScreenCurvature(),
						dialog->GetDirect2dFilter(),
						dialog->PutTextureInVideoMemory()
					);
				}
				else
				{
					direct2d.UpdateFullscreenView
					(
						picture,
						Point(renderState.width,renderState.height),
						nesScreen,
						dialog->GetScreenCurvature(),
						dialog->GetDirect2dFilter(),
						dialog->PutTextureInVideoMemory()
					);
				}

				UpdateFieldMergingState();
			}
			else if (Windowed())
			{
				direct2d.UpdateWindowView();
			}
		}

		void Video::UpdateFieldMergingState() const
		{
			Nes::Video(emulator).EnableFieldMerging
			(
				dialog->EnableFieldMerging() ||
				(dialog->UseAutoFieldMerging() && !direct2d.SmoothFrameRate())
			);
		}

		void Video::ToggleFps(const ibool enable)
		{
			if (enable)
			{
				NST_ASSERT( Fullscreen() || statusBar.Enabled() );

				fps.frame = emulator.GetFrame();

				if (statusBar.Enabled())
					statusBar.Text(Window::StatusBar::SECOND_FIELD) << (Resource::String(IDS_TEXT_FPS) << ": ").Ptr();
				else
					direct2d.DrawFps( _T("0.0") );

				window.StartTimer( this, &Video::OnTimerFps, Fps::UPDATE_INTERVAL );
			}
			else
			{
				fps.frame = 0;

				if (window.StopTimer( this, &Video::OnTimerFps ))
				{
					if (statusBar.Enabled())
						statusBar.Text(Window::StatusBar::SECOND_FIELD).Clear();
					else
						direct2d.ClearFps();
				}
			}
		}

		void Video::StartEmulation()
		{
			if
			(
				emulator.Is(Nes::Machine::GAME) &&
				menu[IDM_VIEW_FPS].Checked() &&
				(Fullscreen() || statusBar.Enabled())
			)
				ToggleFps( true );
		}

		void Video::StopEmulation()
		{
			ToggleFps( false );
		}

		void Video::OnEmuEvent(Emulator::Event event)
		{
			switch (event)
			{
				case Emulator::EVENT_BASE_SPEED:

					direct2d.UpdateFrameRate
					(
						emulator.GetBaseSpeed(),
						emulator.SyncFrameRate(),
						emulator.UseTripleBuffering()
					);

					UpdateFieldMergingState();
					break;

				case Emulator::EVENT_NSF_NEXT:
				case Emulator::EVENT_NSF_PREV:

					nsf.Update( Nes::Nsf(emulator) );
					direct2d.DrawNfo( nsf.text );
					window.Redraw();
					break;

				case Emulator::EVENT_POWER_ON:
				case Emulator::EVENT_NETPLAY_POWER_ON:

					UpdateScreen();

				case Emulator::EVENT_POWER_OFF:

					if (event != Emulator::EVENT_NETPLAY_POWER_ON)
						menu[IDM_FILE_SAVE_SCREENSHOT].Enable( event == Emulator::EVENT_POWER_ON && emulator.Is(Nes::Machine::GAME) );

				case Emulator::EVENT_NETPLAY_POWER_OFF:

					window.Redraw();
					break;

				case Emulator::EVENT_MODE_NTSC:
				case Emulator::EVENT_MODE_PAL:

					if (Windowed() && WindowMatched())
						window.PostCommand( IDM_VIEW_WINDOWSIZE_1X + CalculateWindowScale() );
					else
						window.Redraw();

					break;

				case Emulator::EVENT_LOAD:

					if (emulator.Is(Nes::Machine::GAME))
					{
						dialog->UpdateAutoModes();
					}
					else if (emulator.Is(Nes::Machine::SOUND))
					{
						nsf.Load( Nes::Nsf(emulator) );
						direct2d.DrawNfo( nsf.text );
					}
					break;

				case Emulator::EVENT_NETPLAY_LOAD:

					if (emulator.Is(Nes::Machine::GAME))
						dialog->UpdateAutoModes();

					menu[IDM_MACHINE_OPTIONS_UNLIMITEDSPRITES].Disable();
					break;

				case Emulator::EVENT_NETPLAY_UNLOAD:

					menu[IDM_MACHINE_OPTIONS_UNLIMITEDSPRITES].Enable();

				case Emulator::EVENT_UNLOAD:

					dialog->UnloadGamePalette();
					break;

				case Emulator::EVENT_INIT:

					menu[IDM_FILE_SAVE_SCREENSHOT].Disable();
					break;

				case Emulator::EVENT_NETPLAY_MODE_ON:
				case Emulator::EVENT_NETPLAY_MODE_OFF:

					menu[IDM_OPTIONS_VIDEO].Enable( event == Emulator::EVENT_NETPLAY_MODE_OFF );
					break;
			}
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("t", on)
		#endif

		void Video::OnScreenText(const GenericString& text,const uint time)
		{
			if (Fullscreen())
			{
				direct2d.DrawMsg( text );
				window.Redraw();
			}
			else if (statusBar.Enabled())
			{
				statusBar.Text(Window::StatusBar::FIRST_FIELD) << text.Ptr();
			}
			else
			{
				return;
			}

			window.StartTimer( this, &Video::OnTimerText, time ? time : SCREEN_TEXT_DURATION );
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif
	}
}
