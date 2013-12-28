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

#include "NstApplicationInstance.hpp"
#include "NstIoLog.hpp"
#include "NstWindowMenu.hpp"
#include "NstDialogLogfile.hpp"
#include "NstObjectHeap.hpp"
#include "NstManagerEmulator.hpp"
#include "NstManagerPreferences.hpp"
#include "NstManagerLogfile.hpp"
#include "../core/api/NstApiUser.hpp"

namespace Nestopia
{
	using namespace Managers;

	struct Logfile::Callbacks
	{
		static void NST_CALLBACK DoOutput(Nes::User::UserData data,tstring text,uint length)
		{
			NST_ASSERT( data && text );

			Logfile& log = *static_cast<Logfile*>(data);

			if (length && log.preferences[Preferences::SAVE_LOGFILE])
			{
				try
				{
					if (log.file.IsOpen())
					{
						if (log.file.Size() > MAX_SIZE)
							log.file.Truncate( log.msgOffset );
					}
					else
					{
						log.Open();
					}

					log.file.Write( text, length * sizeof(tchar) );
				}
				catch (Io::File::Exception)
				{
					log.Close();
				}
			}
		}

		static void NST_CALLBACK DoCharOutput(Nes::User::UserData data,cstring text,uint length)
		{
			NST_ASSERT( data && text );

			Logfile& log = *static_cast<Logfile*>(data);

			if (length && log.preferences[Preferences::SAVE_LOGFILE])
			{
				const HeapString string( text, length );
				DoOutput( data, string.Ptr(), string.Length() );
			}
		}
	};

	Logfile::Logfile(Emulator& e,Window::Menu& m,const Preferences& p)
	:
	emulator    ( e ),
	preferences ( p ),
	menu        ( m )
	{
		m.Commands().Add( IDM_VIEW_LOGFILE, this, &Logfile::OnCommand );
		emulator.Events().Add( this, &Logfile::OnEmuEvent );

		Io::Log::callbacker.data = this;
		Io::Log::callbacker.code = &Callbacks::DoOutput;
		Nes::User::logCallback.Set( &Callbacks::DoCharOutput, this );
	}

	Logfile::~Logfile()
	{
		emulator.Events().Remove( this );

		Io::Log::callbacker.data = NULL;
		Io::Log::callbacker.code = NULL;
		Nes::User::logCallback.Set( NULL, NULL );
	}

	void Logfile::OnEmuEvent(Emulator::Event event)
	{
		switch (event)
		{
			case Emulator::EVENT_INIT:
			case Emulator::EVENT_NETPLAY_MODE_ON:
			case Emulator::EVENT_NETPLAY_MODE_OFF:

				menu[IDM_VIEW_LOGFILE].Enable( event != Emulator::EVENT_NETPLAY_MODE_ON && file.IsOpen() );
				break;
		}
	}

	void Logfile::OnCommand(uint)
	{
		if (file.IsOpen())
		{
			try
			{
				file.Rewind();

				HeapString string;
				file.ReadText( string );

				if (Window::Logfile().Open( string.Ptr() ))
					file.Truncate( msgOffset );
			}
			catch (Io::File::Exception)
			{
				Close();
			}
		}
	}

	void Logfile::Open()
	{
		file.Open
		(
			Application::Instance::GetExePath(_T("nestopia.log")),
			Io::File::READ|Io::File::WRITE|Io::File::EMPTY
		);

		HeapString text;

		text << "Nestopia log file version "
             << Application::Instance::GetVersion()
             << "\r\n-----------------------------------\r\n\r\n";

		file.WriteText( text.Ptr(), text.Length(), true );
		msgOffset = file.Position();

		menu[IDM_VIEW_LOGFILE].Enable();
	}

	void Logfile::Close()
	{
		Io::Log::callbacker.data = NULL;
		Io::Log::callbacker.code = NULL;
		Nes::User::logCallback.Set( NULL, NULL );

		menu[IDM_VIEW_LOGFILE].Disable();

		file.Close();
	}
}
