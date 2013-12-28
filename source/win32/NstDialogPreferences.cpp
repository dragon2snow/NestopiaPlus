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

#include "NstObjectPod.hpp"
#include "NstSystemRegistry.hpp"
#include "NstWindowUser.hpp"
#include "NstApplicationInstance.hpp"
#include "NstWindowParam.hpp"
#include "NstManagerEmulator.hpp"
#include "NstDialogPreferences.hpp"
#include "NstIoLog.hpp"
#include "NstResourceString.hpp"
#include "../core/api/NstApiCartridge.hpp"
#include <CommDlg.h>

namespace Nestopia
{
	using namespace Window;

	struct Preferences::MenuColorWindow
	{
		COLORREF color;
		Rect rect;
	};

	Preferences::MenuColorWindow Preferences::menuColorWindows[2];

	class Preferences::Association
	{
	public:

		enum
		{
			NUM_EXTENSIONS = 5
		};

		Association();

		void Create(const uint);
		void Delete(const uint);
		void Update(const uint);

	private:

		enum
		{
			EXTENSION,
			NAME,
			DESCRIPTION,
			NUM_KEYTYPES
		};

		enum
		{
			ICON_OFFSET = 2,
			UNCACHED = UCHAR_MAX
		};

		System::Registry registry;
		String::Heap tmpString;

		static uchar cache[NUM_EXTENSIONS];
		static cstring const keyNames[NUM_EXTENSIONS][NUM_KEYTYPES];

	public:

		bool IsEnabled(const uint index) const
		{
			return cache[index];
		}
	};

	uchar Preferences::Association::cache[NUM_EXTENSIONS] =
	{
		UNCACHED, UNCACHED, UNCACHED, UNCACHED, UNCACHED
	};

	cstring const Preferences::Association::keyNames[NUM_EXTENSIONS][NUM_KEYTYPES] =
	{
		{ ".nes", "Nestopia.nes", "Nestopia iNes File"                },
		{ ".unf", "Nestopia.unf", "Nestopia UNIF File"                },
		{ ".fds", "Nestopia.fds", "Nestopia Famicom Disk System File" },
		{ ".nsf", "Nestopia.nsf", "Nestopia NES Sound File"           },
		{ ".nsp", "Nestopia.nsp", "Nestopia Script File"              }
	};

	Preferences::Association::Association()
	{
		if (*cache == UNCACHED)
		{
			for (uint i=0; i < NUM_EXTENSIONS; ++i)
				cache[i] = (registry[keyNames[i][EXTENSION]] >> tmpString) && (keyNames[i][NAME] == tmpString);
		}
	}

	void Preferences::Association::Update(const uint index)
	{
		cache[index] = TRUE;

		// "nestopia.extension\DefaultIcon" <- "drive:\directory\nestopia.exe, icon"

		tmpString = Application::Instance::GetPath();
		registry[keyNames[index][NAME]]["DefaultIcon"] << (tmpString << ',' << (ICON_OFFSET + index));

		// "nestopia.extension\Shell\Open\Command" <- "drive:\directory\nestopia.exe "%1"

		tmpString.ShrinkTo( Application::Instance::GetPath().Size() );
		registry[keyNames[index][NAME]]["Shell\\Open\\Command"] << (tmpString << " \"%1\"");
	}

	void Preferences::Association::Create(const uint index)
	{
		// ".extension" will point to "nestopia.extension"

		registry[keyNames[index][EXTENSION]] << keyNames[index][NAME];
		registry[keyNames[index][NAME]] << keyNames[index][DESCRIPTION];

		Update( index );

		Io::Log() << "Preferences: Creating registry keys: \"HKEY_CLASSES_ROOT\\" 
			      << keyNames[index][EXTENSION] 
			      << "\" and \"HKEY_CLASSES_ROOT\\" 
				  << keyNames[index][NAME] 
				  << "\"..\r\n";
	}

	void Preferences::Association::Delete(const uint index)
	{
		cache[index] = FALSE;

		// remove ".extension" (if default) and "nestopia.extension"

		registry[keyNames[index][EXTENSION]].Delete( keyNames[index][NAME] );
		registry[keyNames[index][NAME]].Delete();

		Io::Log() << "Preferences: Deleting registry keys: \"HKEY_CLASSES_ROOT\\" 
			      << keyNames[index][EXTENSION] 
		       	  << "\" and \"HKEY_CLASSES_ROOT\\" 
				  << keyNames[index][NAME] 
				  << "\"..\r\n";
	}

	struct Preferences::Handlers
	{
		static const MsgHandler::Entry<Preferences> messages[];
		static const MsgHandler::Entry<Preferences> commands[];
	};

	const MsgHandler::Entry<Preferences> Preferences::Handlers::messages[] =
	{
		{ WM_INITDIALOG, &Preferences::OnInitDialog },
		{ WM_PAINT,      &Preferences::OnPaint      }
	};

	const MsgHandler::Entry<Preferences> Preferences::Handlers::commands[] =
	{
		{ IDC_PREFERENCES_MENUCOLOR_DESKTOP_CHANGE,     &Preferences::OnCmdMenuColorChange  },
		{ IDC_PREFERENCES_MENUCOLOR_FULLSCREEN_CHANGE,  &Preferences::OnCmdMenuColorChange  },
		{ IDC_PREFERENCES_MENUCOLOR_DESKTOP_DEFAULT,    &Preferences::OnCmdMenuColorDefault },
		{ IDC_PREFERENCES_MENUCOLOR_FULLSCREEN_DEFAULT, &Preferences::OnCmdMenuColorDefault },
		{ IDC_PREFERENCES_DEFAULT,                      &Preferences::OnCmdDefault          },
		{ IDC_PREFERENCES_CANCEL,                       &Preferences::OnCmdCancel           },
		{ IDC_PREFERENCES_OK,                           &Preferences::OnCmdOk               }
	};

	Preferences::Preferences(Managers::Emulator& e,const Configuration& cfg)
	:
	emulator ( e ),
	dialog   ( IDD_PREFERENCES, this, Handlers::messages, Handlers::commands )
	{
		NST_COMPILE_ASSERT
		(
			START_IN_FULLSCREEN      == IDC_PREFERENCES_STARTUP_FULLSCREEN    - IDC_PREFERENCES_STARTUP_FULLSCREEN &&
			SUPPRESS_WARNINGS        == IDC_PREFERENCES_DISABLE_ROM_WARNINGS  - IDC_PREFERENCES_STARTUP_FULLSCREEN &&
			FIRST_UNLOAD_ON_EXIT     == IDC_PREFERENCES_CLOSE_POWER_OFF       - IDC_PREFERENCES_STARTUP_FULLSCREEN &&
			CONFIRM_EXIT             == IDC_PREFERENCES_CONFIRM_EXIT          - IDC_PREFERENCES_STARTUP_FULLSCREEN &&
			RUN_IN_BACKGROUND        == IDC_PREFERENCES_RUN_IN_BACKGROUND     - IDC_PREFERENCES_STARTUP_FULLSCREEN &&
			AUTOSTART_EMULATION      == IDC_PREFERENCES_BEGIN_EMULATION       - IDC_PREFERENCES_STARTUP_FULLSCREEN &&
			SAVE_LOGFILE             == IDC_PREFERENCES_SAVE_LOGFILE          - IDC_PREFERENCES_STARTUP_FULLSCREEN &&
			AUTOCORRECT_IMAGES       == IDC_PREFERENCES_USE_ROM_DATABASE      - IDC_PREFERENCES_STARTUP_FULLSCREEN &&
			ALLOW_MULTIPLE_INSTANCES == IDC_PREFERENCES_MULTIPLE_INSTANCES    - IDC_PREFERENCES_STARTUP_FULLSCREEN &&
			SAVE_LAUNCHER            == IDC_PREFERENCES_SAVE_LAUNCHER         - IDC_PREFERENCES_STARTUP_FULLSCREEN &&
			CONFIRM_RESET            == IDC_PREFERENCES_CONFIRM_RESET         - IDC_PREFERENCES_STARTUP_FULLSCREEN &&
			SAVE_CHEATS              == IDC_PREFERENCES_SAVE_CHEATCODES       - IDC_PREFERENCES_STARTUP_FULLSCREEN &&
			SAVE_NETPLAY_GAMELIST    == IDC_PREFERENCES_SAVE_NETPLAY_GAMELIST - IDC_PREFERENCES_STARTUP_FULLSCREEN &&
			SAVE_WINDOWPOS           == IDC_PREFERENCES_SAVE_WINDOWPOS        - IDC_PREFERENCES_STARTUP_FULLSCREEN
		);

		NST_COMPILE_ASSERT
		(
			IDC_PREFERENCES_ASSOCIATE_UNF - IDC_PREFERENCES_ASSOCIATE_NES == 1 &&
			IDC_PREFERENCES_ASSOCIATE_FDS - IDC_PREFERENCES_ASSOCIATE_NES == 2 &&
			IDC_PREFERENCES_ASSOCIATE_NSF - IDC_PREFERENCES_ASSOCIATE_NES == 3 &&
			IDC_PREFERENCES_ASSOCIATE_NSP - IDC_PREFERENCES_ASSOCIATE_NES == 4
		);

		settings[ AUTOSTART_EMULATION      ] = ( cfg[ "preferences autostart emulation"      ] != Configuration::NO  );
		settings[ RUN_IN_BACKGROUND        ] = ( cfg[ "preferences run in background"        ] == Configuration::YES ); 
		settings[ START_IN_FULLSCREEN      ] = ( cfg[ "preferences start in fullscreen"      ] == Configuration::YES ); 
		settings[ SUPPRESS_WARNINGS        ] = ( cfg[ "preferences suppress warnings"        ] == Configuration::YES ); 
		settings[ FIRST_UNLOAD_ON_EXIT     ] = ( cfg[ "preferences power off on exit"        ] == Configuration::YES ); 
		settings[ CONFIRM_EXIT             ] = ( cfg[ "preferences confirm exit"             ] != Configuration::NO  ); 
		settings[ CONFIRM_RESET            ] = ( cfg[ "preferences confirm machine reset"    ] == Configuration::YES ); 
		settings[ AUTOCORRECT_IMAGES       ] = ( cfg[ "preferences autocorrect images"       ] != Configuration::NO  );
		settings[ ALLOW_MULTIPLE_INSTANCES ] = ( cfg[ "preferences allow multiple instances" ] == Configuration::YES ); 
		settings[ SAVE_LOGFILE             ] = ( cfg[ "preferences save logfile"             ] == Configuration::YES ); 
		settings[ SAVE_SETTINGS            ] = ( cfg[ "preferences save settings"            ] != Configuration::NO  ); 
		settings[ SAVE_LAUNCHER            ] = ( cfg[ "preferences save launcher files"      ] != Configuration::NO  ); 
		settings[ SAVE_CHEATS              ] = ( cfg[ "preferences save cheats"              ] != Configuration::NO  ); 
		settings[ SAVE_NETPLAY_GAMELIST    ] = ( cfg[ "preferences save netplay list"        ] != Configuration::NO  ); 
		settings[ SAVE_WINDOWPOS           ] = ( cfg[ "preferences save window"              ] == Configuration::YES ); 

		settings.menuLookDesktop.enabled    = ( cfg[ "preferences default desktop menu color"    ] == Configuration::NO );
		settings.menuLookFullscreen.enabled = ( cfg[ "preferences default fullscreen menu color" ] == Configuration::NO );

		settings.menuLookDesktop.color    = cfg[ "preferences desktop menu color"    ].Default( (uint) DEFAULT_DESKTOP_MENU_COLOR );
		settings.menuLookFullscreen.color = cfg[ "preferences fullscreen menu color" ].Default( (uint) DEFAULT_FULLSCREEN_MENU_COLOR );

		const String::Heap& priority = cfg[ "preferences priority" ];

		if (priority == "high")
		{
			settings.priority = PRIORITY_HIGH;
		}
		else if (priority == "above normal")
		{
			settings.priority = PRIORITY_ABOVE_NORMAL;
		}
		else
		{
			settings.priority = PRIORITY_NORMAL;
		}

		Association association;

		for (uint i=0; i < Association::NUM_EXTENSIONS; ++i)
		{
			if (association.IsEnabled( i ))
				association.Update( i );
		}

		Nes::Cartridge(emulator).GetDatabase().Enable( settings[AUTOCORRECT_IMAGES] );
	}

	Preferences::~Preferences()
	{
	}

	void Preferences::Save(Configuration& cfg) const
	{
		cfg[ "preferences autostart emulation"      ].YesNo() = settings[ AUTOSTART_EMULATION      ];
		cfg[ "preferences run in background"        ].YesNo() = settings[ RUN_IN_BACKGROUND        ];
		cfg[ "preferences start in fullscreen"      ].YesNo() = settings[ START_IN_FULLSCREEN      ];
		cfg[ "preferences suppress warnings"        ].YesNo() = settings[ SUPPRESS_WARNINGS        ];
		cfg[ "preferences power off on exit"        ].YesNo() = settings[ FIRST_UNLOAD_ON_EXIT     ];
		cfg[ "preferences confirm exit"             ].YesNo() = settings[ CONFIRM_EXIT             ]; 
		cfg[ "preferences confirm machine reset"    ].YesNo() = settings[ CONFIRM_RESET            ];
		cfg[ "preferences autocorrect images"       ].YesNo() = settings[ AUTOCORRECT_IMAGES       ];
		cfg[ "preferences allow multiple instances" ].YesNo() = settings[ ALLOW_MULTIPLE_INSTANCES ];
		cfg[ "preferences save logfile"             ].YesNo() = settings[ SAVE_LOGFILE             ]; 
		cfg[ "preferences save settings"            ].YesNo() = settings[ SAVE_SETTINGS            ]; 
		cfg[ "preferences save launcher files"      ].YesNo() = settings[ SAVE_LAUNCHER            ];
		cfg[ "preferences save cheats"              ].YesNo() = settings[ SAVE_CHEATS              ];
		cfg[ "preferences save netplay list"        ].YesNo() = settings[ SAVE_NETPLAY_GAMELIST    ];
		cfg[ "preferences save window"              ].YesNo() = settings[ SAVE_WINDOWPOS           ];

		cfg[ "preferences default desktop menu color"    ].YesNo() = !settings.menuLookDesktop.enabled;
		cfg[ "preferences default fullscreen menu color" ].YesNo() = !settings.menuLookFullscreen.enabled;

		cfg[ "preferences desktop menu color"    ] = String::Hex( (u32) settings.menuLookDesktop.color );
		cfg[ "preferences fullscreen menu color" ] = String::Hex( (u32) settings.menuLookFullscreen.color );

		cstring const priority = 
		(
	       	settings.priority == PRIORITY_HIGH         ? "high" : 
       		settings.priority == PRIORITY_ABOVE_NORMAL ? "above normal" : 
		                                                 "normal"
		);

		cfg[ "preferences priority" ] = priority;
	}

	ibool Preferences::OnInitDialog(Param&)
	{
		for (uint i=0; i < 2; ++i)
		{
			MenuColorWindow& type = menuColorWindows[i];

			type.color = i ? settings.menuLookFullscreen.color : settings.menuLookDesktop.color;

			Window::Generic ctrl( dialog.Control(i ? IDC_PREFERENCES_MENUCOLOR_FULLSCREEN_CHANGE : IDC_PREFERENCES_MENUCOLOR_DESKTOP_CHANGE).GetWindow() );
			
			type.rect = ctrl.GetClientRect();
			::MapWindowPoints( ctrl, dialog, reinterpret_cast<POINT*>(&type.rect), 2 );
			type.rect.Position() -= Point( int(type.rect.Width()) + 8, 0 );
		}

		dialog.Control( IDC_PREFERENCES_MENUCOLOR_DESKTOP_CHANGE ).Enable( settings.menuLookDesktop.enabled );
		dialog.Control( IDC_PREFERENCES_MENUCOLOR_FULLSCREEN_CHANGE ).Enable( settings.menuLookFullscreen.enabled );

		dialog.CheckBox( IDC_PREFERENCES_MENUCOLOR_DESKTOP_DEFAULT ).Check( !settings.menuLookDesktop.enabled );
		dialog.CheckBox( IDC_PREFERENCES_MENUCOLOR_FULLSCREEN_DEFAULT ).Check( !settings.menuLookFullscreen.enabled );

		for (uint i=0; i < NUM_SETTINGS; ++i)
		{
			if (i != SAVE_SETTINGS)
				dialog.CheckBox( IDC_PREFERENCES_STARTUP_FULLSCREEN + i ).Check( settings[i] );
		}

		Association association;

		for (uint i=0; i < Association::NUM_EXTENSIONS; ++i)
			dialog.CheckBox( IDC_PREFERENCES_ASSOCIATE_NES + i ).Check( association.IsEnabled(i) ); 

		Control::ComboBox priorities( dialog.ComboBox( IDC_PREFERENCES_PRIORITY ) ); 

		priorities.Add( Resource::String(IDS_PRIORITY_NORMAL) );
		priorities.Add( Resource::String(IDS_PRIORITY_ABOVE_NORMAL) );
		priorities.Add( Resource::String(IDS_PRIORITY_HIGH) );
		
		priorities[settings.priority].Select();

		return TRUE;
	}

	ibool Preferences::OnPaint(Param&)	
	{
		if (HDC const hDC = ::GetDC( dialog ))
		{
			HPEN const hPen = ::CreatePen( PS_SOLID, 1, RGB(0x00,0x00,0x00) );
			HPEN const hPenOld = static_cast<HPEN>(::SelectObject( hDC, hPen ));

			for (uint i=0; i < 2; ++i)
			{
				const MenuColorWindow& type = menuColorWindows[i];

				HBRUSH const hBrush = ::CreateSolidBrush( type.color );
				HBRUSH const hBrushOld = static_cast<HBRUSH>(::SelectObject( hDC, hBrush ));

				::Rectangle( hDC, type.rect.left, type.rect.top, type.rect.right, type.rect.bottom );

				::SelectObject( hDC, hBrushOld );
				::DeleteObject( hBrush );
			}

			::SelectObject( hDC, hPenOld );
			::DeleteObject( hPen );
			
			::ReleaseDC( dialog, hDC );
		}

		return FALSE;
	}
  
	ibool Preferences::OnCmdMenuColorChange(Param& param)
	{
		if (param.Button().IsClicked())
		{
			static COLORREF customColors[16] = {0};

			Object::Pod<CHOOSECOLOR> cc;

			MenuColorWindow& type = menuColorWindows[param.Button().GetId() == IDC_PREFERENCES_MENUCOLOR_FULLSCREEN_CHANGE];

			cc.lStructSize  = sizeof(cc);
			cc.hwndOwner    = dialog;
			cc.lpCustColors = customColors;
			cc.rgbResult    = type.color;
			cc.Flags        = CC_FULLOPEN|CC_RGBINIT;

			if (::ChooseColor( &cc )) 
				type.color = cc.rgbResult;
		}

		return TRUE;
	}

	ibool Preferences::OnCmdMenuColorDefault(Param& param)
	{
		if (param.Button().IsClicked())
		{
			uint id;

			if (param.Button().GetId() == IDC_PREFERENCES_MENUCOLOR_FULLSCREEN_DEFAULT)
				id = IDC_PREFERENCES_MENUCOLOR_FULLSCREEN_CHANGE;
			else
				id = IDC_PREFERENCES_MENUCOLOR_DESKTOP_CHANGE;

			dialog.Control( id ).Enable( dialog.CheckBox(param.Button().GetId()).IsUnchecked() );
		}

		return TRUE;
	}

	ibool Preferences::OnCmdDefault(Param&)
	{
		dialog.CheckBox( IDC_PREFERENCES_BEGIN_EMULATION       ).Check( TRUE  );
		dialog.CheckBox( IDC_PREFERENCES_RUN_IN_BACKGROUND     ).Check( FALSE );
		dialog.CheckBox( IDC_PREFERENCES_STARTUP_FULLSCREEN    ).Check( FALSE );
		dialog.CheckBox( IDC_PREFERENCES_DISABLE_ROM_WARNINGS  ).Check( FALSE );
		dialog.CheckBox( IDC_PREFERENCES_CLOSE_POWER_OFF       ).Check( FALSE );
		dialog.CheckBox( IDC_PREFERENCES_CONFIRM_EXIT          ).Check( TRUE  );
		dialog.CheckBox( IDC_PREFERENCES_CONFIRM_RESET         ).Check( FALSE );
		dialog.CheckBox( IDC_PREFERENCES_USE_ROM_DATABASE      ).Check( TRUE  );
		dialog.CheckBox( IDC_PREFERENCES_MULTIPLE_INSTANCES    ).Check( FALSE );
		dialog.CheckBox( IDC_PREFERENCES_SAVE_LOGFILE          ).Check( FALSE );
		dialog.CheckBox( IDC_PREFERENCES_SAVE_LAUNCHER         ).Check( TRUE  );
		dialog.CheckBox( IDC_PREFERENCES_SAVE_CHEATCODES       ).Check( TRUE  );
		dialog.CheckBox( IDC_PREFERENCES_SAVE_NETPLAY_GAMELIST ).Check( TRUE  );
		dialog.CheckBox( IDC_PREFERENCES_SAVE_WINDOWPOS        ).Check( FALSE );

		dialog.ComboBox( IDC_PREFERENCES_PRIORITY )[ PRIORITY_NORMAL ].Select();

		dialog.CheckBox( IDC_PREFERENCES_MENUCOLOR_DESKTOP_DEFAULT ).Check( TRUE );
		dialog.CheckBox( IDC_PREFERENCES_MENUCOLOR_FULLSCREEN_DEFAULT ).Check( TRUE );

		dialog.Control( IDC_PREFERENCES_MENUCOLOR_DESKTOP_CHANGE ).Enable( FALSE );
		dialog.Control( IDC_PREFERENCES_MENUCOLOR_FULLSCREEN_CHANGE ).Enable( FALSE );

		menuColorWindows[0].color = DEFAULT_DESKTOP_MENU_COLOR;
		menuColorWindows[1].color = DEFAULT_FULLSCREEN_MENU_COLOR;

		dialog.Redraw();

		return TRUE;
	}

	ibool Preferences::OnCmdCancel(Param&)
	{
		dialog.Close();
		return TRUE;
	}

	ibool Preferences::OnCmdOk(Param& param)
	{
		if (param.Button().IsClicked())
		{
			for (uint i=0; i < NUM_SETTINGS; ++i)
			{
				if (i != SAVE_SETTINGS)
					settings[i] = dialog.CheckBox( IDC_PREFERENCES_STARTUP_FULLSCREEN + i ).IsChecked();
			}

			settings.priority = (Priority) dialog.ComboBox( IDC_PREFERENCES_PRIORITY ).Selection().GetIndex();
			
			settings.menuLookDesktop.color = menuColorWindows[0].color;
			settings.menuLookFullscreen.color = menuColorWindows[1].color;

			settings.menuLookDesktop.enabled = dialog.CheckBox( IDC_PREFERENCES_MENUCOLOR_DESKTOP_DEFAULT ).IsUnchecked();
			settings.menuLookFullscreen.enabled = dialog.CheckBox( IDC_PREFERENCES_MENUCOLOR_FULLSCREEN_DEFAULT ).IsUnchecked();

			Nes::Cartridge(emulator).GetDatabase().Enable( settings[AUTOCORRECT_IMAGES] );

			ibool altered = FALSE;
			Association association;

			for (uint i=0; i < Association::NUM_EXTENSIONS; ++i)
			{
				const bool checked = dialog.CheckBox( IDC_PREFERENCES_ASSOCIATE_NES + i ).IsChecked();

				if (checked != association.IsEnabled( i ))
				{
					altered = TRUE;

					if (checked) association.Create( i );
					else	 	 association.Delete( i );
				}
			}

			if (altered)
			{
				System::Registry::UpdateAssociations();

				User::Inform
				( 
					IDS_DIALOGS_PREFERENCES_REGISTRYUPDATED, 
					IDS_DIALOGS_PREFERENCES_REGISTRYUPDATED_TITLE
				);
			}

			dialog.Close();
		}

		return TRUE;
	}
}
