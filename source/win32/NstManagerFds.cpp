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

#include "NstResourceString.hpp"
#include "NstObjectHeap.hpp"
#include "NstIoScreen.hpp"
#include "NstSystemKeyboard.hpp"
#include "NstWindowMenu.hpp"
#include "NstDialogFds.hpp"
#include "NstManagerEmulator.hpp"
#include "NstManagerFds.hpp"

namespace Nestopia
{
	NST_COMPILE_ASSERT
	(
		( IDM_MACHINE_FDS_INSERT_DISK_1_SIDE_B - IDM_MACHINE_FDS_INSERT_DISK_1_SIDE_A ) == 1  && 
		( IDM_MACHINE_FDS_INSERT_DISK_2_SIDE_A - IDM_MACHINE_FDS_INSERT_DISK_1_SIDE_A ) == 2  &&  
		( IDM_MACHINE_FDS_INSERT_DISK_2_SIDE_B - IDM_MACHINE_FDS_INSERT_DISK_1_SIDE_A ) == 3  && 
		( IDM_MACHINE_FDS_INSERT_DISK_3_SIDE_A - IDM_MACHINE_FDS_INSERT_DISK_1_SIDE_A ) == 4  && 
		( IDM_MACHINE_FDS_INSERT_DISK_3_SIDE_B - IDM_MACHINE_FDS_INSERT_DISK_1_SIDE_A ) == 5  && 
		( IDM_MACHINE_FDS_INSERT_DISK_4_SIDE_A - IDM_MACHINE_FDS_INSERT_DISK_1_SIDE_A ) == 6  && 
		( IDM_MACHINE_FDS_INSERT_DISK_4_SIDE_B - IDM_MACHINE_FDS_INSERT_DISK_1_SIDE_A ) == 7  && 
		( IDM_MACHINE_FDS_INSERT_DISK_5_SIDE_A - IDM_MACHINE_FDS_INSERT_DISK_1_SIDE_A ) == 8  && 
		( IDM_MACHINE_FDS_INSERT_DISK_5_SIDE_B - IDM_MACHINE_FDS_INSERT_DISK_1_SIDE_A ) == 9  &&
		( IDM_MACHINE_FDS_INSERT_DISK_6_SIDE_A - IDM_MACHINE_FDS_INSERT_DISK_1_SIDE_A ) == 10 &&
		( IDM_MACHINE_FDS_INSERT_DISK_6_SIDE_B - IDM_MACHINE_FDS_INSERT_DISK_1_SIDE_A ) == 11 &&
		( IDM_MACHINE_FDS_INSERT_DISK_7_SIDE_A - IDM_MACHINE_FDS_INSERT_DISK_1_SIDE_A ) == 12 &&
		( IDM_MACHINE_FDS_INSERT_DISK_7_SIDE_B - IDM_MACHINE_FDS_INSERT_DISK_1_SIDE_A ) == 13 &&
		( IDM_MACHINE_FDS_INSERT_DISK_8_SIDE_A - IDM_MACHINE_FDS_INSERT_DISK_1_SIDE_A ) == 14 &&
		( IDM_MACHINE_FDS_INSERT_DISK_8_SIDE_B - IDM_MACHINE_FDS_INSERT_DISK_1_SIDE_A ) == 15
	);

	NST_COMPILE_ASSERT
	(
		( IDS_FDS_DISK_1_SIDE_B - IDS_FDS_DISK_1_SIDE_A ) == 1  &&
		( IDS_FDS_DISK_2_SIDE_A - IDS_FDS_DISK_1_SIDE_A ) == 2  &&
		( IDS_FDS_DISK_2_SIDE_B - IDS_FDS_DISK_1_SIDE_A ) == 3  &&
		( IDS_FDS_DISK_3_SIDE_A - IDS_FDS_DISK_1_SIDE_A ) == 4  &&
		( IDS_FDS_DISK_3_SIDE_B - IDS_FDS_DISK_1_SIDE_A ) == 5  &&
		( IDS_FDS_DISK_4_SIDE_A - IDS_FDS_DISK_1_SIDE_A ) == 6  &&
		( IDS_FDS_DISK_4_SIDE_B - IDS_FDS_DISK_1_SIDE_A ) == 7  &&
		( IDS_FDS_DISK_5_SIDE_A - IDS_FDS_DISK_1_SIDE_A ) == 8  &&
		( IDS_FDS_DISK_5_SIDE_B - IDS_FDS_DISK_1_SIDE_A ) == 9  &&
		( IDS_FDS_DISK_6_SIDE_A - IDS_FDS_DISK_1_SIDE_A ) == 10 &&
		( IDS_FDS_DISK_6_SIDE_B - IDS_FDS_DISK_1_SIDE_A ) == 11 &&
		( IDS_FDS_DISK_7_SIDE_A - IDS_FDS_DISK_1_SIDE_A ) == 12 &&
		( IDS_FDS_DISK_7_SIDE_B - IDS_FDS_DISK_1_SIDE_A ) == 13 &&
		( IDS_FDS_DISK_8_SIDE_A - IDS_FDS_DISK_1_SIDE_A ) == 14 &&
		( IDS_FDS_DISK_8_SIDE_B - IDS_FDS_DISK_1_SIDE_A ) == 15
	);

	NST_COMPILE_ASSERT
	(
		( IDS_SCREEN_DISK_1_SIDE_B_INSERTED - IDS_SCREEN_DISK_1_SIDE_A_INSERTED ) == 1  &&
		( IDS_SCREEN_DISK_2_SIDE_A_INSERTED - IDS_SCREEN_DISK_1_SIDE_A_INSERTED ) == 2  &&
		( IDS_SCREEN_DISK_2_SIDE_B_INSERTED - IDS_SCREEN_DISK_1_SIDE_A_INSERTED ) == 3  &&
		( IDS_SCREEN_DISK_3_SIDE_A_INSERTED - IDS_SCREEN_DISK_1_SIDE_A_INSERTED ) == 4  &&
		( IDS_SCREEN_DISK_3_SIDE_B_INSERTED - IDS_SCREEN_DISK_1_SIDE_A_INSERTED ) == 5  &&
		( IDS_SCREEN_DISK_4_SIDE_A_INSERTED - IDS_SCREEN_DISK_1_SIDE_A_INSERTED ) == 6  &&
		( IDS_SCREEN_DISK_4_SIDE_B_INSERTED - IDS_SCREEN_DISK_1_SIDE_A_INSERTED ) == 7  &&
		( IDS_SCREEN_DISK_5_SIDE_A_INSERTED - IDS_SCREEN_DISK_1_SIDE_A_INSERTED ) == 8  &&
		( IDS_SCREEN_DISK_5_SIDE_B_INSERTED - IDS_SCREEN_DISK_1_SIDE_A_INSERTED ) == 9  &&
		( IDS_SCREEN_DISK_6_SIDE_A_INSERTED - IDS_SCREEN_DISK_1_SIDE_A_INSERTED ) == 10 &&
		( IDS_SCREEN_DISK_6_SIDE_B_INSERTED - IDS_SCREEN_DISK_1_SIDE_A_INSERTED ) == 11 &&
		( IDS_SCREEN_DISK_7_SIDE_A_INSERTED - IDS_SCREEN_DISK_1_SIDE_A_INSERTED ) == 12 &&
		( IDS_SCREEN_DISK_7_SIDE_B_INSERTED - IDS_SCREEN_DISK_1_SIDE_A_INSERTED ) == 13 &&
		( IDS_SCREEN_DISK_8_SIDE_A_INSERTED - IDS_SCREEN_DISK_1_SIDE_A_INSERTED ) == 14 &&
		( IDS_SCREEN_DISK_8_SIDE_B_INSERTED - IDS_SCREEN_DISK_1_SIDE_A_INSERTED ) == 15
	);

	NST_COMPILE_ASSERT
	(
		( IDS_SCREEN_DISK_2_EJECTED - IDS_SCREEN_DISK_1_EJECTED ) == 1 &&
		( IDS_SCREEN_DISK_3_EJECTED - IDS_SCREEN_DISK_1_EJECTED ) == 2 &&
		( IDS_SCREEN_DISK_4_EJECTED - IDS_SCREEN_DISK_1_EJECTED ) == 3 &&
		( IDS_SCREEN_DISK_5_EJECTED - IDS_SCREEN_DISK_1_EJECTED ) == 4 &&
		( IDS_SCREEN_DISK_6_EJECTED - IDS_SCREEN_DISK_1_EJECTED ) == 5 &&
		( IDS_SCREEN_DISK_7_EJECTED - IDS_SCREEN_DISK_1_EJECTED ) == 6 &&
		( IDS_SCREEN_DISK_8_EJECTED - IDS_SCREEN_DISK_1_EJECTED ) == 7
	);

	using namespace Managers;

	struct Fds::Callbacks
	{
		static void NST_CALLBACK OnDiskChange(Nes::Fds::UserData user,Nes::Fds::Event event,uint disk,uint side)
		{
			static_cast<Fds*>(user)->OnDiskChange( event, disk, side );
		}

		static void NST_CALLBACK OnDiskAccessNumLock(Nes::Fds::UserData,bool on)
		{
			System::Keyboard::ToggleIndicator( System::Keyboard::NUM_LOCK, on );
		}

		static void NST_CALLBACK OnDiskAccessScrollLock(Nes::Fds::UserData,bool on)
		{
			System::Keyboard::ToggleIndicator( System::Keyboard::SCROLL_LOCK, on );
		}

		static void NST_CALLBACK OnDiskAccessCapsLock(Nes::Fds::UserData,bool on)
		{
			System::Keyboard::ToggleIndicator( System::Keyboard::CAPS_LOCK, on );
		}

		static void NST_CALLBACK OnDiskAccessScreen(Nes::Fds::UserData,bool on)
		{
			Io::Screen() << (on ? Resource::String(IDS_SCREEN_FDS_LED) : "");
		}
	};

	Fds::Fds(Emulator& e,const Configuration& cfg,Window::Menu& m,const Paths& paths)
	:
	emulator ( e ),
	menu     ( m ),
	master   ( FALSE ),
	dialog   ( new Window::Fds(e,cfg,paths) ) 
	{
		static const Window::Menu::CmdHandler::Entry<Fds> commands[] =
		{
			{ IDM_MACHINE_FDS_INSERT_DISK_1_SIDE_A,	&Fds::OnCmdInsertDisk },
			{ IDM_MACHINE_FDS_INSERT_DISK_1_SIDE_B,	&Fds::OnCmdInsertDisk },
			{ IDM_MACHINE_FDS_INSERT_DISK_2_SIDE_A,	&Fds::OnCmdInsertDisk },
			{ IDM_MACHINE_FDS_INSERT_DISK_2_SIDE_B,	&Fds::OnCmdInsertDisk },
			{ IDM_MACHINE_FDS_INSERT_DISK_3_SIDE_A,	&Fds::OnCmdInsertDisk },
			{ IDM_MACHINE_FDS_INSERT_DISK_3_SIDE_B,	&Fds::OnCmdInsertDisk },
			{ IDM_MACHINE_FDS_INSERT_DISK_4_SIDE_A,	&Fds::OnCmdInsertDisk },
			{ IDM_MACHINE_FDS_INSERT_DISK_4_SIDE_B,	&Fds::OnCmdInsertDisk },
			{ IDM_MACHINE_FDS_INSERT_DISK_5_SIDE_A,	&Fds::OnCmdInsertDisk },
			{ IDM_MACHINE_FDS_INSERT_DISK_5_SIDE_B,	&Fds::OnCmdInsertDisk },
			{ IDM_MACHINE_FDS_INSERT_DISK_6_SIDE_A,	&Fds::OnCmdInsertDisk },
			{ IDM_MACHINE_FDS_INSERT_DISK_6_SIDE_B,	&Fds::OnCmdInsertDisk },
			{ IDM_MACHINE_FDS_INSERT_DISK_7_SIDE_A,	&Fds::OnCmdInsertDisk },
			{ IDM_MACHINE_FDS_INSERT_DISK_7_SIDE_B,	&Fds::OnCmdInsertDisk },
			{ IDM_MACHINE_FDS_INSERT_DISK_8_SIDE_A,	&Fds::OnCmdInsertDisk },
			{ IDM_MACHINE_FDS_INSERT_DISK_8_SIDE_B,	&Fds::OnCmdInsertDisk },
			{ IDM_MACHINE_FDS_CHANGE_SIDE,          &Fds::OnCmdChangeSide },
			{ IDM_MACHINE_FDS_EJECT_DISK,			&Fds::OnCmdEjectDisk  },
			{ IDM_MACHINE_FDS_OPTIONS,				&Fds::OnCmdOptions    }
		};

		Nes::Fds::diskChangeCallback.Set( Callbacks::OnDiskChange, this );

		m.Commands().Add( this, commands );
		emulator.Events().Add( this, &Fds::OnEmuEvent );

		menu[IDM_POS_MACHINE][IDM_POS_MACHINE_FDS][IDM_POS_MACHINE_FDS_INSERTDISK].Clear();

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

	void Fds::OnDiskChange(const Nes::Fds::Event event,uint disk,const uint side) const
	{
		const ibool inserted = (event == Nes::Fds::DISK_INSERT);

		disk = disk * 2 + side;

		menu[IDM_MACHINE_FDS_INSERT_DISK_1_SIDE_A + disk].Check( inserted );
		menu[IDM_MACHINE_FDS_CHANGE_SIDE].Enable( inserted && master );
		menu[IDM_MACHINE_FDS_EJECT_DISK].Enable( inserted && master );

		Io::Screen() << Resource::String( inserted ? (IDS_SCREEN_DISK_1_SIDE_A_INSERTED + disk) : (IDS_SCREEN_DISK_1_EJECTED + disk / 2) );
	}

	void Fds::OnCmdInsertDisk(uint disk)
	{
		disk -= IDM_MACHINE_FDS_INSERT_DISK_1_SIDE_A;

		Nes::Fds(emulator).InsertDisk( disk / 2, disk % 2 );
	}

	void Fds::OnCmdChangeSide(uint)
	{
		Nes::Fds(emulator).ChangeSide();
	}

	void Fds::OnCmdEjectDisk(uint) 
	{
		Nes::Fds(emulator).EjectDisk();
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
		
					const Window::Menu::Item subMenu( menu[IDM_POS_MACHINE][IDM_POS_MACHINE_FDS][IDM_POS_MACHINE_FDS_INSERTDISK] );
		
					subMenu.Enable();
					subMenu.Clear();
		
					Nes::Fds fds( emulator );
		
					const uint totalSides = fds.GetNumSides();
		
					for (uint i=0; i < totalSides; ++i)
					{
						menu.Insert
						( 
							subMenu[i], 
							IDM_MACHINE_FDS_INSERT_DISK_1_SIDE_A + i, 
							Resource::String(IDS_FDS_DISK_1_SIDE_A + i) 
						);

						if (!master)
							menu[IDM_MACHINE_FDS_INSERT_DISK_1_SIDE_A + i].Disable();
					}
		
					fds.InsertDisk( 0, 0 );
				}
				break;
		
			case Emulator::EVENT_UNLOAD:
			case Emulator::EVENT_NETPLAY_UNLOAD:
		
				menu[IDM_POS_MACHINE][IDM_POS_MACHINE_FDS][IDM_POS_MACHINE_FDS_INSERTDISK].Clear();
				menu[IDM_POS_MACHINE][IDM_POS_MACHINE_FDS][IDM_POS_MACHINE_FDS_INSERTDISK].Disable();
				break;

			case Emulator::EVENT_NETPLAY_MODE_ON:
			case Emulator::EVENT_NETPLAY_MODE_OFF:

				menu[IDM_MACHINE_FDS_OPTIONS].Enable( event == Emulator::EVENT_NETPLAY_MODE_OFF );
				break;
		}
	}
}
