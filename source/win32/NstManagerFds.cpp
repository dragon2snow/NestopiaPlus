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

#include "NstResourceString.hpp"
#include "NstObjectHeap.hpp"
#include "NstIoScreen.hpp"
#include "NstSystemKeyboard.hpp"
#include "NstWindowMenu.hpp"
#include "NstManagerEmulator.hpp"
#include "NstManagerFds.hpp"
#include "NstDialogFds.hpp"

namespace Nestopia
{
	namespace Managers
	{
		NST_COMPILE_ASSERT
		(
			IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_B == IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A +  1 &&
			IDM_MACHINE_EXT_FDS_INSERT_DISK_2_SIDE_A == IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A +  2 &&
			IDM_MACHINE_EXT_FDS_INSERT_DISK_2_SIDE_B == IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A +  3 &&
			IDM_MACHINE_EXT_FDS_INSERT_DISK_3_SIDE_A == IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A +  4 &&
			IDM_MACHINE_EXT_FDS_INSERT_DISK_3_SIDE_B == IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A +  5 &&
			IDM_MACHINE_EXT_FDS_INSERT_DISK_4_SIDE_A == IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A +  6 &&
			IDM_MACHINE_EXT_FDS_INSERT_DISK_4_SIDE_B == IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A +  7 &&
			IDM_MACHINE_EXT_FDS_INSERT_DISK_5_SIDE_A == IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A +  8 &&
			IDM_MACHINE_EXT_FDS_INSERT_DISK_5_SIDE_B == IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A +  9 &&
			IDM_MACHINE_EXT_FDS_INSERT_DISK_6_SIDE_A == IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A + 10 &&
			IDM_MACHINE_EXT_FDS_INSERT_DISK_6_SIDE_B == IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A + 11 &&
			IDM_MACHINE_EXT_FDS_INSERT_DISK_7_SIDE_A == IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A + 12 &&
			IDM_MACHINE_EXT_FDS_INSERT_DISK_7_SIDE_B == IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A + 13 &&
			IDM_MACHINE_EXT_FDS_INSERT_DISK_8_SIDE_A == IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A + 14 &&
			IDM_MACHINE_EXT_FDS_INSERT_DISK_8_SIDE_B == IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A + 15
		);

		struct Fds::Callbacks
		{
			static void NST_CALLBACK OnDiskChange(Nes::Fds::UserData user,Nes::Fds::Event event,uint disk,uint side)
			{
				static_cast<Fds*>(user)->OnDiskChange( event, disk, side );
			}

			static void NST_CALLBACK OnDiskAccessNumLock(Nes::Fds::UserData,Nes::Fds::Motor motor)
			{
				System::Keyboard::ToggleIndicator( System::Keyboard::NUM_LOCK, motor );
			}

			static void NST_CALLBACK OnDiskAccessScrollLock(Nes::Fds::UserData,Nes::Fds::Motor motor)
			{
				System::Keyboard::ToggleIndicator( System::Keyboard::SCROLL_LOCK, motor );
			}

			static void NST_CALLBACK OnDiskAccessCapsLock(Nes::Fds::UserData,Nes::Fds::Motor motor)
			{
				System::Keyboard::ToggleIndicator( System::Keyboard::CAPS_LOCK, motor );
			}

			static void NST_CALLBACK OnDiskAccessScreen(Nes::Fds::UserData,Nes::Fds::Motor motor)
			{
				Io::Screen(15000) << (motor ? Resource::String(motor == Nes::Fds::MOTOR_READ ? IDS_SCREEN_FDS_READING : IDS_SCREEN_FDS_WRITING).Ptr() : _T(""));
			}
		};

		Fds::Fds(Emulator& e,const Configuration& cfg,Window::Menu& m,const Paths& paths)
		:
		emulator ( e ),
		master   ( false ),
		menu     ( m ),
		dialog   ( new Window::Fds(e,cfg,paths) )
		{
			static const Window::Menu::CmdHandler::Entry<Fds> commands[] =
			{
				{ IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A, &Fds::OnCmdInsertDisk },
				{ IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_B, &Fds::OnCmdInsertDisk },
				{ IDM_MACHINE_EXT_FDS_INSERT_DISK_2_SIDE_A, &Fds::OnCmdInsertDisk },
				{ IDM_MACHINE_EXT_FDS_INSERT_DISK_2_SIDE_B, &Fds::OnCmdInsertDisk },
				{ IDM_MACHINE_EXT_FDS_INSERT_DISK_3_SIDE_A, &Fds::OnCmdInsertDisk },
				{ IDM_MACHINE_EXT_FDS_INSERT_DISK_3_SIDE_B, &Fds::OnCmdInsertDisk },
				{ IDM_MACHINE_EXT_FDS_INSERT_DISK_4_SIDE_A, &Fds::OnCmdInsertDisk },
				{ IDM_MACHINE_EXT_FDS_INSERT_DISK_4_SIDE_B, &Fds::OnCmdInsertDisk },
				{ IDM_MACHINE_EXT_FDS_INSERT_DISK_5_SIDE_A, &Fds::OnCmdInsertDisk },
				{ IDM_MACHINE_EXT_FDS_INSERT_DISK_5_SIDE_B, &Fds::OnCmdInsertDisk },
				{ IDM_MACHINE_EXT_FDS_INSERT_DISK_6_SIDE_A, &Fds::OnCmdInsertDisk },
				{ IDM_MACHINE_EXT_FDS_INSERT_DISK_6_SIDE_B, &Fds::OnCmdInsertDisk },
				{ IDM_MACHINE_EXT_FDS_INSERT_DISK_7_SIDE_A, &Fds::OnCmdInsertDisk },
				{ IDM_MACHINE_EXT_FDS_INSERT_DISK_7_SIDE_B, &Fds::OnCmdInsertDisk },
				{ IDM_MACHINE_EXT_FDS_INSERT_DISK_8_SIDE_A, &Fds::OnCmdInsertDisk },
				{ IDM_MACHINE_EXT_FDS_INSERT_DISK_8_SIDE_B, &Fds::OnCmdInsertDisk },
				{ IDM_MACHINE_EXT_FDS_CHANGE_SIDE,          &Fds::OnCmdChangeSide },
				{ IDM_MACHINE_EXT_FDS_EJECT_DISK,           &Fds::OnCmdEjectDisk  },
				{ IDM_MACHINE_EXT_FDS_OPTIONS,              &Fds::OnCmdOptions    }
			};

			static const Window::Menu::PopupHandler::Entry<Fds> popups[] =
			{
				{ Window::Menu::PopupHandler::Pos<IDM_POS_MACHINE,IDM_POS_MACHINE_EXT,IDM_POS_MACHINE_EXT_FDS,IDM_POS_MACHINE_EXT_FDS_INSERTDISK>::ID, &Fds::OnMenuInsert }
			};

			Nes::Fds::diskChangeCallback.Set( Callbacks::OnDiskChange, this );

			m.Commands().Add( this, commands );
			m.PopupRouter().Add( this, popups );
			emulator.Events().Add( this, &Fds::OnEmuEvent );

			UpdateSettings();
		}

		Fds::~Fds()
		{
			Nes::Fds::diskChangeCallback.Set( NULL, NULL );
			Nes::Fds::diskAccessLampCallback.Set( NULL, NULL );

			emulator.Events().Remove( this );
		}

		void Fds::Save(Configuration& cfg) const
		{
			dialog->Save( cfg );
		}

		void Fds::UpdateSettings() const
		{
			Nes::Fds::diskAccessLampCallback.Set
			(
				dialog->GetLed() == Window::Fds::LED_SCREEN      ? Callbacks::OnDiskAccessScreen :
				dialog->GetLed() == Window::Fds::LED_NUM_LOCK    ? Callbacks::OnDiskAccessNumLock :
				dialog->GetLed() == Window::Fds::LED_CAPS_LOCK   ? Callbacks::OnDiskAccessCapsLock :
				dialog->GetLed() == Window::Fds::LED_SCROLL_LOCK ? Callbacks::OnDiskAccessScrollLock : NULL,
				NULL
			);
		}

		void Fds::OnDiskChange(const Nes::Fds::Event event,const uint disk,const uint side) const
		{
			const ibool inserted = (event == Nes::Fds::DISK_INSERT);

			menu[IDM_MACHINE_EXT_FDS_CHANGE_SIDE].Enable( inserted && master && Nes::Fds(emulator).CanChangeDiskSide() );
			menu[IDM_MACHINE_EXT_FDS_EJECT_DISK].Enable( inserted && master );

			Io::Screen() << Resource::String( inserted ? side ? IDS_SCREEN_DISK_SIDE_B_INSERTED : IDS_SCREEN_DISK_SIDE_A_INSERTED : IDS_SCREEN_DISK_EJECTED ).Invoke( tchar('1' + disk) );
		}

		void Fds::OnMenuInsert(Window::Menu::PopupHandler::Param& param)
		{
			const Nes::Fds fds(emulator);
			const int disk = fds.GetCurrentDisk();

			param.menu[IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A + (disk != Nes::Fds::NO_DISK ? disk*2+fds.GetCurrentDiskSide() : 0)].Check
			(
				IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A,
				IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A + fds.GetNumSides(),
				disk != Nes::Fds::NO_DISK
			);
		}

		void Fds::OnCmdInsertDisk(uint disk)
		{
			disk -= IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A;

			Nes::Fds(emulator).InsertDisk( disk / 2, disk % 2 );
			Application::Instance::GetMainWindow().Post( Application::Instance::WM_NST_COMMAND_RESUME );
		}

		void Fds::OnCmdChangeSide(uint)
		{
			Nes::Fds(emulator).ChangeSide();
			Application::Instance::GetMainWindow().Post( Application::Instance::WM_NST_COMMAND_RESUME );
		}

		void Fds::OnCmdEjectDisk(uint)
		{
			Nes::Fds(emulator).EjectDisk();
			Application::Instance::GetMainWindow().Post( Application::Instance::WM_NST_COMMAND_RESUME );
		}

		void Fds::OnCmdOptions(uint)
		{
			dialog->Open();
			UpdateSettings();
		}

		void Fds::OnEmuEvent(Emulator::Event event)
		{
			switch (event)
			{
				case Emulator::EVENT_LOAD:
				case Emulator::EVENT_NETPLAY_LOAD:

					if (emulator.Is(Nes::Machine::DISK))
					{
						master = (event == Emulator::EVENT_LOAD || emulator.GetPlayer() == 1);

						const Window::Menu::Item subMenu( menu[IDM_POS_MACHINE][IDM_POS_MACHINE_EXT][IDM_POS_MACHINE_EXT_FDS][IDM_POS_MACHINE_EXT_FDS_INSERTDISK] );

						subMenu.Enable();
						subMenu.Clear();

						Nes::Fds fds( emulator );

						for (uint i=0, n=fds.GetNumSides(); i < n; ++i)
						{
							menu.Insert
							(
								subMenu[i],
								IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A + i,
								Resource::String( (i % 2) ? IDS_FDS_DISK_SIDE_B : IDS_FDS_DISK_SIDE_A ).Invoke( tchar('1' + (i / 2)) )
							);

							if (!master)
								menu[IDM_MACHINE_EXT_FDS_INSERT_DISK_1_SIDE_A + i].Disable();
						}

						fds.InsertDisk( 0, 0 );
					}
					break;

				case Emulator::EVENT_INIT:
				case Emulator::EVENT_UNLOAD:
				case Emulator::EVENT_NETPLAY_UNLOAD:

					menu[IDM_MACHINE_EXT_FDS_CHANGE_SIDE].Disable();
					menu[IDM_MACHINE_EXT_FDS_EJECT_DISK].Disable();
					menu[IDM_POS_MACHINE][IDM_POS_MACHINE_EXT][IDM_POS_MACHINE_EXT_FDS][IDM_POS_MACHINE_EXT_FDS_INSERTDISK].Clear();
					menu[IDM_POS_MACHINE][IDM_POS_MACHINE_EXT][IDM_POS_MACHINE_EXT_FDS][IDM_POS_MACHINE_EXT_FDS_INSERTDISK].Disable();
					break;

				case Emulator::EVENT_NETPLAY_MODE_ON:
				case Emulator::EVENT_NETPLAY_MODE_OFF:

					menu[IDM_MACHINE_EXT_FDS_OPTIONS].Enable( event == Emulator::EVENT_NETPLAY_MODE_OFF );
					break;

				case Emulator::EVENT_QUERY_FDS_BIOS:

					dialog->QueryBiosFile();
					break;
			}
		}
	}
}
