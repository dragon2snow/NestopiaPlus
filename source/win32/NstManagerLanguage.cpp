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

#include "NstApplicationException.hpp"
#include "NstResourceVersion.hpp"
#include "NstIoLog.hpp"
#include "NstWindowMenu.hpp"
#include "NstWindowUser.hpp"
#include "NstDialogLanguage.hpp"
#include "NstManagerEmulator.hpp"
#include "NstManagerLanguage.hpp"

namespace Nestopia
{
	namespace Managers
	{
		Language::Language(Emulator& e,Window::Menu& m)
		: menu(m), emulator(e)
		{
			m.Commands().Add( IDM_OPTIONS_LANGUAGE, this, &Language::OnCmd );
			emulator.Events().Add( this, &Language::OnEmuEvent );

			const Resource::Version version( Application::Instance::GetLanguage().GetResourcePath().Ptr(), Resource::Version::PRODUCT );

			if (version != "1.33" && version != Application::Instance::GetVersion() && !Window::User::Confirm( IDS_INCOMPATIBLE_RESOURCE ))
				throw Application::Exception();

			Io::Log() << "Language: loaded \""
                      << Application::Instance::GetLanguage().GetResourcePath().File()
                      << "\" version "
                      << version
                      << '.'
                      << Resource::Version( Application::Instance::GetLanguage().GetResourcePath().Ptr(), Resource::Version::FILE )
                      << "\r\n";
		}

		Language::~Language()
		{
			emulator.Events().Remove( this );
		}

		void Language::OnCmd(uint)
		{
			const Path newPath( Window::Language().Open() );

			if (newPath.Length())
			{
				Application::Instance::GetLanguage().UpdateResource( newPath.Ptr() );

				if (Window::User::Confirm( IDS_LANGUAGE_UPDATE ))
					Application::Instance::GetMainWindow().Close();
			}
		}

		void Language::OnEmuEvent(const Emulator::Event event)
		{
			switch (event)
			{
				case Emulator::EVENT_NETPLAY_MODE_ON:
				case Emulator::EVENT_NETPLAY_MODE_OFF:

					menu[IDM_OPTIONS_LANGUAGE].Enable( event == Emulator::EVENT_NETPLAY_MODE_OFF );
					break;
			}
		}
	}
}
