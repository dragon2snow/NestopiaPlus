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

#include "NstWindowMenu.hpp"
#include "NstDialogImageInfo.hpp"
#include "NstManagerEmulator.hpp"
#include "NstManagerImageInfo.hpp"

namespace Nestopia
{
	using namespace Managers;

	ImageInfo::ImageInfo(Emulator& e,Window::Menu& m)
	: emulator(e), menu(m)
	{
		m.Commands().Add( IDM_VIEW_IMAGE_INFO, this, &ImageInfo::OnCmd );
		emulator.Events().Add( this, &ImageInfo::OnEmuEvent );
	}

	ImageInfo::~ImageInfo()
	{
		emulator.Events().Remove( this );
	}

	void ImageInfo::OnCmd(uint)
	{
		Window::ImageInfo dialog( emulator );
	}

	void ImageInfo::OnEmuEvent(Emulator::Event event)
	{
		switch (event)
		{
    		case Emulator::EVENT_LOAD:
			case Emulator::EVENT_UNLOAD:

	     		menu[IDM_VIEW_IMAGE_INFO].Enable( event == Emulator::EVENT_LOAD );
				break;
		}
	}
}
