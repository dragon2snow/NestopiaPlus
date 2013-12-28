////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003-2006 Martin Freij
// Copyright (C) 2001, 2002, 2003, 2004 Andrea Mazzoleni
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

#ifndef NST_VIDEO_FILTER_SCALEX_H
#define NST_VIDEO_FILTER_SCALEX_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Video
		{
			class Renderer::FilterScaleX: public Renderer::Filter
			{
				template<typename T,int PREV,int NEXT>
				NST_FORCE_INLINE T* Blit2xBorder(T* NST_RESTRICT,const u16* NST_RESTRICT,const u32 (&)[PALETTE]) const;

				template<typename T,int PREV,int NEXT>
				NST_FORCE_INLINE T* Blit3xBorder(T* NST_RESTRICT,const u16* NST_RESTRICT,const u32 (&)[PALETTE]) const;

				template<typename T>
				NST_FORCE_INLINE T* Blit3xCenter(T* NST_RESTRICT,const u16* NST_RESTRICT,const u32 (&)[PALETTE]) const;

				template<typename T,int PREV,int NEXT>
				NST_FORCE_INLINE T* Blit2xLine(T*,const u16*,const u32 (&)[PALETTE],long) const;

				template<typename T,int PREV,int NEXT>
				NST_FORCE_INLINE T* Blit3xLine(T*,const u16*,const u32 (&)[PALETTE],long) const;

				template<typename T>
				NST_FORCE_INLINE void Blit2x(const Input&,const Output&) const;

				template<typename T>
				NST_FORCE_INLINE void Blit3x(const Input&,const Output&) const;

				template<typename T>
				NST_FORCE_INLINE void BlitType(const Input&,const Output&) const;

				const RenderState::Filter type;

				void Blit(const Input&,const Output&,uint);

			public:

				FilterScaleX(const RenderState&);

				static bool Check(const RenderState&);
			};
		}
	}
}

#endif
