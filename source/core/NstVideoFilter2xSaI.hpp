////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
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

#ifndef NST_VIDEO_FILTER_2XSAI_H
#define NST_VIDEO_FILTER_2XSAI_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Video
		{
			class Renderer::Filter2xSaI: public Renderer::Filter
			{
				inline dword Blend(dword,dword) const;
				inline dword Blend(dword,dword,dword,dword) const;

				template<typename T>
				NST_FORCE_INLINE void Blit2xSaI(const Input&,const Output&) const;

				template<typename T>
				NST_FORCE_INLINE void BlitSuper2xSaI(const Input&,const Output&) const;

				template<typename T>
				NST_FORCE_INLINE void BlitSuperEagle(const Input&,const Output&) const;

				template<typename T>
				NST_FORCE_INLINE void BlitType(const Input&,const Output&) const;

				const dword lsb0;
				const dword lsb1;
				const RenderState::Filter type;

			public:

				Filter2xSaI(const RenderState&);

				void Blit(const Input&,const Output&);
				static bool Check(const RenderState&);
			};
		}
	}
}

#endif
