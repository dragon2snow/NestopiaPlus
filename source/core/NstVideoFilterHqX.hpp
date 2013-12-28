////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2006 Martin Freij
// Copyright (C) 2003 MaxSt ( maxst@hiend3d.com )
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

#ifndef NST_VIDEO_FILTER_HQX_H
#define NST_VIDEO_FILTER_HQX_H

#ifdef NST_PRAGMA_ONCE_SUPPORT
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Video
		{
			class Renderer::FilterHqX : public Renderer::Filter
			{
				void Transform(const u8 (&)[PALETTE][3],u32 (&)[PALETTE]) const;

				template<u32 R,u32 G,u32 B> static dword Interpolate1(dword,dword);
				template<u32 R,u32 G,u32 B> static dword Interpolate2(dword,dword,dword);
				template<u32 R,u32 G,u32 B> static dword Interpolate3(dword,dword);
				template<u32 R,u32 G,u32 B> static dword Interpolate4(dword,dword,dword);
				template<u32 R,u32 G,u32 B> static dword Interpolate5(dword,dword);
				template<u32 R,u32 G,u32 B> static dword Interpolate6(dword,dword,dword);
				template<u32 R,u32 G,u32 B> static dword Interpolate7(dword,dword,dword);
				template<u32 R,u32 G,u32 B> static dword Interpolate8(dword,dword);
				template<u32 R,u32 G,u32 B> static dword Interpolate9(dword,dword,dword);
				template<u32 R,u32 G,u32 B> static dword Interpolate10(dword,dword,dword);

				inline dword Diff(uint,uint) const;

				template<typename T,u32 R,u32 G,u32 B>
				void Blit2xRgb(const Input&,const Output&) const;

				template<typename T,u32 R,u32 G,u32 B>
				void Blit3xRgb(const Input&,const Output&) const;

				template<typename T,u32 R,u32 G,u32 B>
				void Blit4xRgb(const Input&,const Output&) const;

				template<typename T>
				NST_FORCE_INLINE void Blit2x(const Input&,const Output&) const;

				template<typename T>
				NST_FORCE_INLINE void Blit3x(const Input&,const Output&) const;

				template<typename T>
				NST_FORCE_INLINE void Blit4x(const Input&,const Output&) const;

				template<typename T>
				NST_FORCE_INLINE void BlitType(const Input&,const Output&) const;

				template<typename T>
				struct Buffer;

				struct Lut
				{
					Lut(bool,const uint (&)[3],u32* = NULL);
					~Lut();

					enum
					{
						YUV_OFFSET = (0x440UL << 21) + (0x207UL << 11) + 0x407UL,
						YUV_MASK   = (0x380UL << 21) + (0x1F0UL << 11) + 0x3F0UL
					};

					u32 yuv[0x10000UL];
					const u32* const NST_RESTRICT rgb;
				};

				const Lut lut;
				const RenderState::Filter type;

				void Blit(const Input&,const Output&,uint);

			public:

				FilterHqX(const RenderState&);

				static bool Check(const RenderState&);
			};
		}
	}
}

#endif
