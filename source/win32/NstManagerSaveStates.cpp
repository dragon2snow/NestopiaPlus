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
#include "NstDialogAutoSaver.hpp"
#include "NstManagerEmulator.hpp"
#include "NstManagerPaths.hpp"
#include "NstWindowMenu.hpp"
#include "NstWindowMain.hpp"
#include "NstManagerSaveStates.hpp"

namespace Nestopia
{
	using namespace Managers;

	SaveStates::SaveStates(Emulator& e,const Configuration&,Window::Menu& m,const Paths& p,const Window::Main& w)
	: 
	emulator        ( e ),
	menu            ( m ),
	window          ( w ),
	paths           ( p ),
	autoSaveEnabled ( FALSE ),
	autoSaver       ( new Window::AutoSaver(paths) )
	{
		emulator.Events().Add( this, &SaveStates::OnEmuEvent );

		static const Window::Menu::CmdHandler::Entry<SaveStates> commands[] =
		{
			{ IDM_FILE_LOAD_NST,		   	       &SaveStates::OnCmdStateLoad        },
			{ IDM_FILE_SAVE_NST,			       &SaveStates::OnCmdStateSave        },
			{ IDM_FILE_QUICK_LOAD_STATE_SLOT_LAST, &SaveStates::OnCmdLastSlotLoad     },
			{ IDM_FILE_QUICK_LOAD_STATE_SLOT_1,	   &SaveStates::OnCmdSlotLoad         },
			{ IDM_FILE_QUICK_LOAD_STATE_SLOT_2,	   &SaveStates::OnCmdSlotLoad         },
			{ IDM_FILE_QUICK_LOAD_STATE_SLOT_3,	   &SaveStates::OnCmdSlotLoad         },
			{ IDM_FILE_QUICK_LOAD_STATE_SLOT_4,	   &SaveStates::OnCmdSlotLoad         },
			{ IDM_FILE_QUICK_LOAD_STATE_SLOT_5,	   &SaveStates::OnCmdSlotLoad         },
			{ IDM_FILE_QUICK_LOAD_STATE_SLOT_6,	   &SaveStates::OnCmdSlotLoad         },
			{ IDM_FILE_QUICK_LOAD_STATE_SLOT_7,	   &SaveStates::OnCmdSlotLoad         },
			{ IDM_FILE_QUICK_LOAD_STATE_SLOT_8,	   &SaveStates::OnCmdSlotLoad         },
			{ IDM_FILE_QUICK_LOAD_STATE_SLOT_9,	   &SaveStates::OnCmdSlotLoad         },
			{ IDM_FILE_QUICK_SAVE_STATE_SLOT_NEXT, &SaveStates::OnCmdNextSlotSave     },
			{ IDM_FILE_QUICK_SAVE_STATE_SLOT_1,	   &SaveStates::OnCmdSlotSave         },
			{ IDM_FILE_QUICK_SAVE_STATE_SLOT_2,	   &SaveStates::OnCmdSlotSave         },
			{ IDM_FILE_QUICK_SAVE_STATE_SLOT_3,	   &SaveStates::OnCmdSlotSave         },
			{ IDM_FILE_QUICK_SAVE_STATE_SLOT_4,	   &SaveStates::OnCmdSlotSave         },
			{ IDM_FILE_QUICK_SAVE_STATE_SLOT_5,	   &SaveStates::OnCmdSlotSave         },
			{ IDM_FILE_QUICK_SAVE_STATE_SLOT_6,	   &SaveStates::OnCmdSlotSave         },
			{ IDM_FILE_QUICK_SAVE_STATE_SLOT_7,	   &SaveStates::OnCmdSlotSave         },
			{ IDM_FILE_QUICK_SAVE_STATE_SLOT_8,	   &SaveStates::OnCmdSlotSave         },
			{ IDM_FILE_QUICK_SAVE_STATE_SLOT_9,	   &SaveStates::OnCmdSlotSave         },
			{ IDM_OPTIONS_AUTOSAVER,               &SaveStates::OnCmdAutoSaverOptions },
			{ IDM_OPTIONS_AUTOSAVER_START,         &SaveStates::OnCmdAutoSaverStart   }
		};

		menu.Commands().Add( this, commands );
	}

	SaveStates::~SaveStates()
	{
		ToggleAutoSaver( FALSE );
		emulator.Events().Remove( this );
	}

	void SaveStates::OnEmuEvent(Emulator::Event event)
	{
		switch (event)
		{
			case Emulator::EVENT_POWER_ON:
			case Emulator::EVENT_POWER_OFF:
			
				if (emulator.Is( Nes::Machine::GAME ))
				{
					const ibool on = (event == Emulator::EVENT_POWER_ON);

					menu[ IDM_FILE_SAVE_NST ].Enable( on );
					menu[ IDM_POS_FILE ][ IDM_POS_FILE_QUICKSAVESTATE ].Enable( on );
					menu[ IDM_OPTIONS_AUTOSAVER_START ].Enable( on );

					for (uint i=IDM_FILE_QUICK_SAVE_STATE_SLOT_NEXT; i <= IDM_FILE_QUICK_SAVE_STATE_SLOT_9; ++i)
						menu[i].Enable( on );

					ToggleAutoSaver( FALSE );
				}
				break;
		
			case Emulator::EVENT_LOAD:
			
				lastSaveSlot = SLOT_NONE;

				if (emulator.Is( Nes::Machine::GAME ))
				{
					menu[ IDM_FILE_LOAD_NST ].Enable();
					menu[ IDM_POS_FILE ][ IDM_POS_FILE_QUICKLOADSTATE ].Enable();

					if (paths.SaveSlotImportingEnabled())
						ImportSlots();
				}
				break;

			case Emulator::EVENT_UNLOAD:

				menu[ IDM_FILE_LOAD_NST ].Disable();
				menu[ IDM_POS_FILE ][ IDM_POS_FILE_QUICKLOADSTATE ].Disable();
				menu[ IDM_FILE_QUICK_SAVE_STATE_SLOT_1 + lastSaveSlot ].Uncheck();
				menu[ IDM_FILE_QUICK_LOAD_STATE_SLOT_LAST ].Disable();

				for (uint i=0; i < NUM_SLOTS; ++i)
				{
					menu[ IDM_FILE_QUICK_LOAD_STATE_SLOT_1 + i ].Disable();
					slots[i].Destroy();
				}
				break;

			case Emulator::EVENT_NETPLAY_MODE_ON:
			case Emulator::EVENT_NETPLAY_MODE_OFF:
			
				menu[IDM_POS_OPTIONS][IDM_POS_OPTIONS_AUTOSAVER].Enable( event == Emulator::EVENT_NETPLAY_MODE_OFF );
				break;
		}
	}

	void SaveStates::SaveToSlot(const uint index,const ibool notify)
	{	
		if (emulator.SaveState( slots[index], paths.UseStateCompression(), Emulator::STICKY ))
		{
			menu[ IDM_FILE_QUICK_SAVE_STATE_SLOT_1 + index ].Check( IDM_FILE_QUICK_SAVE_STATE_SLOT_1, IDM_FILE_QUICK_SAVE_STATE_SLOT_9 );
			menu[ IDM_FILE_QUICK_LOAD_STATE_SLOT_1 + index ].Enable();
			menu[ IDM_FILE_QUICK_LOAD_STATE_SLOT_LAST ].Enable();

			lastSaveSlot = index;
			
			if (paths.SaveSlotExportingEnabled())
				ExportSlot( index );

			if (notify)
			{
				Io::Screen() << Resource::String(IDS_SCREEN_SAVE_STATE_TO) 
				         	 << ' '
							 << Resource::String(IDS_SCREEN_SLOT_1 + index);
			}
		}
	}

	void SaveStates::LoadFromSlot(const uint index,const ibool notify)
	{
		if (emulator.LoadState( slots[index], Emulator::STICKY ))
		{
			if (notify)
			{
				Io::Screen() << Resource::String(IDS_SCREEN_LOAD_STATE_FROM) 
					         << ' ' 
							 << Resource::String(IDS_SCREEN_SLOT_1 + index);
			}

			emulator.Resume();
		}
	}

	void SaveStates::Load(Collection::Buffer& data,const GenericString name) const
	{
		if (emulator.LoadState( data ))
		{
			const uint length = window.GetMaxMessageLength(); 

			if (name.Length() && length > 20)
			{
				Io::Screen() << Resource::String( IDS_SCREEN_LOAD_STATE_FROM )
					         << " \""
							 << Path::Compact( name, length - 18 )
							 << '\"';
			}

			emulator.Resume();
		}
	}

	void SaveStates::ExportSlot(const uint index)
	{
		Path path( emulator.GetImagePath().Target().File() );
		NST_ASSERT( slots[index].Size() && path.Length() );
		
		path.Extension() = _T("ns1");
		path.Back() = (tchar) ('1' + index);
		
		paths.Save( slots[index].Ptr(), slots[index].Size(), Paths::File::SLOTS, path );
	}

	void SaveStates::ImportSlots()
	{
		uint last = UINT_MAX;

		Path path( emulator.GetImagePath().Target().File() );
		NST_ASSERT( path.Length() );

		path.Extension() = _T("ns1");

		Paths::File file;

		for (uint i=0; i < NUM_SLOTS; ++i)
		{
			path.Back() = '1' + i;

			if (paths.Load( file, Paths::File::SLOTS, path, Paths::QUIETLY ))
			{
				last = i;
				slots[i].Import( file.data );
				menu[ IDM_FILE_QUICK_LOAD_STATE_SLOT_1 + i ].Enable();
			}
		}

		if (last != UINT_MAX)
		{
			lastSaveSlot = last;

			menu[ IDM_FILE_QUICK_SAVE_STATE_SLOT_1 + last ].Check( IDM_FILE_QUICK_SAVE_STATE_SLOT_1, IDM_FILE_QUICK_SAVE_STATE_SLOT_9 );
			menu[ IDM_FILE_QUICK_LOAD_STATE_SLOT_LAST ].Enable();
		}
	}

	void SaveStates::OnCmdStateLoad(uint)
	{
		Paths::File file;

		if (paths.Load( file, Paths::File::STATE|Paths::File::SLOTS|Paths::File::ARCHIVE ) && emulator.LoadState( file.data ))
		{
			const uint length = window.GetMaxMessageLength(); 

			if (length > 20)
			{
				Io::Screen() << Resource::String( IDS_SCREEN_LOAD_STATE_FROM )
					         << " \""
							 << Path::Compact( file.name, length - 18 )
							 << '\"';
			}
			
			emulator.Resume();
		}
	}

	void SaveStates::OnCmdStateSave(uint)
	{
		const Path path( paths.BrowseSave( Paths::File::STATE, Paths::SUGGEST ) );

		if (path.Length())
		{
			Collection::Buffer buffer;

			if (emulator.SaveState( buffer, paths.UseStateCompression() ) && paths.Save( buffer.Ptr(), buffer.Size(), Paths::File::STATE, path ))
			{
				const uint length = window.GetMaxMessageLength(); 

				if (length > 20)
				{
					Io::Screen() << Resource::String( IDS_SCREEN_SAVE_STATE_TO ) 
						         << " \""
								 << Path::Compact( path, length - 18 )
								 << '\"';
				}
			}
		}
	}

	void SaveStates::OnCmdSlotSave(uint id)
	{
		SaveToSlot( id - IDM_FILE_QUICK_SAVE_STATE_SLOT_1 );
		Application::Instance::Post( Application::Instance::WM_NST_COMMAND_RESUME );
	}

	void SaveStates::OnCmdNextSlotSave(uint)
	{
		SaveToSlot( lastSaveSlot + 1 <= SLOT_9 ? lastSaveSlot + 1 : SLOT_1 );
		Application::Instance::Post( Application::Instance::WM_NST_COMMAND_RESUME );
	}

	void SaveStates::OnCmdSlotLoad(uint id)
	{
		LoadFromSlot( id - IDM_FILE_QUICK_LOAD_STATE_SLOT_1 );
		Application::Instance::Post( Application::Instance::WM_NST_COMMAND_RESUME );
	}

	void SaveStates::OnCmdLastSlotLoad(uint)
	{
		LoadFromSlot( lastSaveSlot );
		Application::Instance::Post( Application::Instance::WM_NST_COMMAND_RESUME );
	}

	void SaveStates::OnCmdAutoSaverOptions(uint)
	{
		autoSaver->Open();
		ToggleAutoSaver( FALSE );
	}

	void SaveStates::OnCmdAutoSaverStart(uint)
	{
		ToggleAutoSaver( autoSaveEnabled ^ TRUE );
		Application::Instance::Post( Application::Instance::WM_NST_COMMAND_RESUME );
	}

	uint SaveStates::OnTimerAutoSave()
	{
		if (autoSaveEnabled)
		{
			if (autoSaver->GetStateFile().Length())
			{
				Collection::Buffer buffer;

				if 
				(
			       	emulator.SaveState( buffer, paths.UseStateCompression(), Emulator::STICKY ) &&
					paths.Save( buffer.Ptr(), buffer.Size(), Paths::File::STATE, autoSaver->GetStateFile(), Paths::STICKY ) &&
					autoSaver->ShouldNotify()
				)
				{
					const uint length = window.GetMaxMessageLength();
				
					if (length > 20)
					{
						Io::Screen() << Resource::String( IDS_SCREEN_SAVE_STATE_TO )
							         << " \""
									 << Path::Compact( autoSaver->GetStateFile(), length - 18 )
									 << '\"';
					}
				}
			}
			else
			{
				SaveToSlot( Window::AutoSaver::DEFAULT_SAVE_SLOT, autoSaver->ShouldNotify() );
			}
		}

		return autoSaveEnabled != FALSE;
	}

	void SaveStates::ToggleAutoSaver(const ibool enable)
	{
		menu[ IDM_OPTIONS_AUTOSAVER_START ].Text() << Resource::String(enable ? IDS_TEXT_STOP : IDS_TEXT_START);
		
		autoSaveEnabled = enable;

		if (enable)
			window.Get().StartTimer( this, &SaveStates::OnTimerAutoSave, autoSaver->GetInterval() );
		else
			window.Get().StopTimer( this, &SaveStates::OnTimerAutoSave );
	}
}
