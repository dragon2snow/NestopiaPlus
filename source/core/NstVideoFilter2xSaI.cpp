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

// --------------------------------------------------------------------
// The following (piece of) code, (part of) the 2xSaI engine,
// copyright (c) 2001 by Derek Liauw Kie Fa.
// Non-Commercial use of the engine is allowed and is encouraged,
// provided that appropriate credit be given and that this copyright
// notice will not be removed under any circumstance.
// You may freely modify this code, but I request
// that any improvements to the engine be submitted to me, so
// that I can implement these improvements in newer versions of
// the engine.
// If you need more information, have any comments or suggestions,
// you can e-mail me. My e-mail: DerekL666@yahoo.com
// --------------------------------------------------------------------

#include "NstCore.hpp"

#ifndef NST_NO_2XSAI

#include "api/NstApiVideo.hpp"
#include "NstVideoRenderer.hpp"
#include "NstVideoFilter2xSaI.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Video
		{
			#ifdef NST_PRAGMA_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			Renderer::Filter2xSaI::Filter2xSaI(const RenderState& state)
			:
			Filter (state),
			lsb0   (~((1UL << format.left[0]) | (1UL << format.left[1]) | (1UL << format.left[2]))),
			lsb1   (~((3UL << format.left[0]) | (3UL << format.left[1]) | (3UL << format.left[2]))),
			type   (state.filter)
			{
			}

			bool Renderer::Filter2xSaI::Check(const RenderState& state)
			{
				return
				(
					(state.bits.count == 16 || state.bits.count == 32) &&
					(state.width == WIDTH*2 && state.height == HEIGHT*2) &&
					(state.scanlines == 0)
				);
			}

			#ifdef NST_PRAGMA_OPTIMIZE
			#pragma optimize("", on)
			#endif

			inline dword Renderer::Filter2xSaI::Blend(dword a,dword b) const
			{
				return (a != b) ? ((a & lsb0) >> 1) + ((b & lsb0) >> 1) + (a & b & ~lsb0) : a;
			}

			inline dword Renderer::Filter2xSaI::Blend(dword a,dword b,dword c,dword d) const
			{
				return
				(
					(((a & lsb1) >> 2) + ((b & lsb1) >> 2) + ((c & lsb1) >> 2) + ((d & lsb1) >> 2)) +
					((((a & ~lsb1) + (b & ~lsb1) + (c & ~lsb1) + (d & ~lsb1)) >> 2) & ~lsb1)
				);
			}

			template<typename T>
			NST_FORCE_INLINE void Renderer::Filter2xSaI::Blit2xSaI(const Input& input,const Output& output) const
			{
				const u16* NST_RESTRICT src = input.screen;
				const long pitch = output.pitch;

				T* NST_RESTRICT dst[2] =
				{
					static_cast<T*>(output.pixels),
					reinterpret_cast<T*>(reinterpret_cast<u8*>(output.pixels) + pitch)
				};

				dword a,b,c,d,e=0,f=0,g,h,i=0,j=0,k,l,m,n,o;

				for (uint y=0; y < HEIGHT; ++y)
				{
					for (uint x=0; x < WIDTH; ++x, ++src, dst[0] += 2, dst[1] += 2)
					{
						if (y)
						{
							i = x > 0 ?       input.palette[src[ -WIDTH-1 ]] : 0;
							e =               input.palette[src[ -WIDTH   ]];
							f = x < WIDTH-1 ? input.palette[src[ -WIDTH+1 ]] : 0;
							j = x < WIDTH-2 ? input.palette[src[ -WIDTH+2 ]] : 0;
						}

						g = x > 0 ?       input.palette[src[ -1 ]] : 0;
						a =               input.palette[src[  0 ]];
						b = x < WIDTH-1 ? input.palette[src[  1 ]] : 0;
						k = x < WIDTH-2 ? input.palette[src[  2 ]] : 0;

						if (y < HEIGHT-1)
						{
							h = x > 0 ?       input.palette[src[ WIDTH-1 ]] : 0;
							c =               input.palette[src[ WIDTH   ]];
							d = x < WIDTH-1 ? input.palette[src[ WIDTH+1 ]] : 0;
							l = x < WIDTH-2 ? input.palette[src[ WIDTH+2 ]] : 0;

							if (y < HEIGHT-2)
							{
								m = x > 0 ?       input.palette[src[ WIDTH*2-1 ]] : 0;
								n =               input.palette[src[ WIDTH*2   ]];
								o = x < WIDTH-1 ? input.palette[src[ WIDTH*2+1 ]] : 0;
							}
							else
							{
								m = n = o = 0;
							}
						}
						else
						{
							h = c = d = l = m = n = o = 0;
						}

						dword q[3];

						if (a == d && b != c)
						{
							if ((a == e && b == l) || (a == c && a == f && b != e && b == j))
							{
								q[0] = a;
							}
							else
							{
								q[0] = Blend( a, b );
							}

							if ((a == g && c == o) || (a == b && a == h && g != c && c == m))
							{
								q[1] = a;
							}
							else
							{
								q[1] = Blend( a, c );
							}

							q[2] = a;
						}
						else if (b == c && a != d)
						{
							if ((b == f && a == h) || (b == e && b == d && a != f && a == i))
							{
								q[0] = b;
							}
							else
							{
								q[0] = Blend( a, b );
							}

							if ((c == h && a == f) || (c == g && c == d && a != h && a == i))
							{
								q[1] = c;
							}
							else
							{
								q[1] = Blend( a, c );
							}

							q[2] = b;
						}
						else if (a == d && b == c)
						{
							if (a == b)
							{
								q[0] = a;
								q[1] = a;
								q[2] = a;
							}
							else
							{
								q[1] = Blend( a, c );
								q[0] = Blend( a, b );

								const int result =
								(
									(a == g && a == e ? -1 : b == g && b == e ? +1 : 0) +
									(b == k && b == f ? -1 : a == k && a == f ? +1 : 0) +
									(b == h && b == n ? -1 : a == h && a == n ? +1 : 0) +
									(a == l && a == o ? -1 : b == l && b == o ? +1 : 0)
								);

								if (result > 0)
								{
									q[2] = a;
								}
								else if (result < 0)
								{
									q[2] = b;
								}
								else
								{
									q[2] = Blend( a, b, c, d );
								}
							}
						}
						else
						{
							q[2] = Blend( a, b, c, d );

							if (a == c && a == f && b != e && b == j)
							{
								q[0] = a;
							}
							else if (b == e && b == d && a != f && a == i)
							{
								q[0] = b;
							}
							else
							{
								q[0] = Blend( a, b );
							}

							if (a == b && a == h && g != c && c == m)
							{
								q[1] = a;
							}
							else if (c == g && c == d && a != h && a == i)
							{
								q[1] = c;
							}
							else
							{
								q[1] = Blend( a, c );
							}
						}

						dst[0][0] = a;
						dst[0][1] = q[0];
						dst[1][0] = q[1];
						dst[1][1] = q[2];
					}

					dst[0] = reinterpret_cast<T*>(reinterpret_cast<u8*>(dst[1]) + (pitch - long(sizeof(T) * WIDTH*2)));
					dst[1] = reinterpret_cast<T*>(reinterpret_cast<u8*>(dst[0]) + pitch);
				}
			}

			template<typename T>
			NST_FORCE_INLINE void Renderer::Filter2xSaI::BlitSuper2xSaI(const Input& input,const Output& output) const
			{
				const u16* NST_RESTRICT src = input.screen;
				const long pitch = output.pitch;

				T* NST_RESTRICT dst[2] =
				{
					static_cast<T*>(output.pixels),
					reinterpret_cast<T*>(reinterpret_cast<u8*>(output.pixels) + pitch)
				};

				dword a,b,c,d,e,f,g,h,i,j,k=0,l=0,m=0,n=0,o,p;

				for (uint y=0; y < HEIGHT; ++y)
				{
					for (uint x=0; x < WIDTH; ++x, ++src, dst[0] += 2, dst[1] += 2)
					{
						if (y)
						{
							k = x > 0 ?       input.palette[src[ -WIDTH-1 ]] : 0;
							l =               input.palette[src[ -WIDTH   ]];
							m = x < WIDTH-1 ? input.palette[src[ -WIDTH+1 ]] : 0;
							n = x < WIDTH-2 ? input.palette[src[ -WIDTH+2 ]] : 0;
						}

						d = x > 0 ?       input.palette[src[ -1 ]] : 0;
						e =               input.palette[src[  0 ]];
						f = x < WIDTH-1 ? input.palette[src[  1 ]] : 0;
						p = x < WIDTH-2 ? input.palette[src[  2 ]] : 0;

						if (y < HEIGHT-1)
						{
							a = x > 0 ?       input.palette[src[ WIDTH-1 ]] : 0;
							b =               input.palette[src[ WIDTH   ]];
							c = x < WIDTH-1 ? input.palette[src[ WIDTH+1 ]] : 0;
							o = x < WIDTH-2 ? input.palette[src[ WIDTH+2 ]] : 0;

							if (y < HEIGHT-2)
							{
								g = x > 0 ?       input.palette[src[ WIDTH*2-1 ]] : 0;
								h =               input.palette[src[ WIDTH*2   ]];
								i = x < WIDTH-1 ? input.palette[src[ WIDTH*2+1 ]] : 0;
								j = x < WIDTH-2 ? input.palette[src[ WIDTH*2+2 ]] : 0;
							}
							else
							{
								g = h = i = j = 0;
							}
						}
						else
						{
							a = b = c = o = g = h = i = j = 0;
						}

						dword q[4];

						if (b == f && e != c)
						{
							q[3] = q[1] = b;
						}
						else if (e == c && b != f)
						{
							q[3] = q[1] = e;
						}
						else if (e == c && b == f && e != f)
						{
							const int result =
							(
								(f == a && f == h ? -1 : e == a && e == h ? +1 : 0) +
								(f == d && f == l ? -1 : e == d && e == l ? +1 : 0) +
								(f == i && f == o ? -1 : e == i && e == o ? +1 : 0) +
								(f == m && f == p ? -1 : e == m && e == p ? +1 : 0)
							);

							if (result > 0)
							{
								q[3] = q[1] = f;
							}
							else if (result < 0)
							{
								q[3] = q[1] = e;
							}
							else
							{
								q[3] = q[1] = Blend( e, f );
							}
						}
						else
						{
							if (f == c && c == h && b != i && c != g)
							{
								q[3] = Blend( c, c, c, b );
							}
							else if (e == b && b == i && h != c && b != j)
							{
								q[3] = Blend( b, b, b, c );
							}
							else
							{
								q[3] = Blend( b, c );
							}

							if (f == c && f == l && e != m && f != k)
							{
								q[1] = Blend( f, f, f, e );
							}
							else if (e == b && e == m && l != f && e != n)
							{
								q[1] = Blend( f, e, e, e );
							}
							else
							{
								q[1] = Blend( e, f );
							}
						}

						if (e == c && b != f && d == e && e != i)
						{
							q[2] = Blend( b, e );
						}
						else if (e == a && f == e && d != b && e != g)
						{
							q[2] = Blend( b, e );
						}
						else
						{
							q[2] = b;
						}

						if (b == f && e != c && a == b && b != m)
						{
							q[0] = Blend( b, e );
						}
						else if (d == b && c == b && a != e && b != k)
						{
							q[0] = Blend( b, e );
						}
						else
						{
							q[0] = e;
						}

						dst[0][0] = q[0];
						dst[0][1] = q[1];
						dst[1][0] = q[2];
						dst[1][1] = q[3];
					}

					dst[0] = reinterpret_cast<T*>(reinterpret_cast<u8*>(dst[1]) + (pitch - long(sizeof(T) * WIDTH*2)));
					dst[1] = reinterpret_cast<T*>(reinterpret_cast<u8*>(dst[0]) + pitch);
				}
			}

			template<typename T>
			NST_FORCE_INLINE void Renderer::Filter2xSaI::BlitSuperEagle(const Input& input,const Output& output) const
			{
				const u16* NST_RESTRICT src = input.screen;
				const long pitch = output.pitch;

				T* NST_RESTRICT dst[2] =
				{
					static_cast<T*>(output.pixels),
					reinterpret_cast<T*>(reinterpret_cast<u8*>(output.pixels) + pitch)
				};

				dword a,b,c,d,e,f,g,h,i=0,j=0,k,l;

				for (uint y=0; y < HEIGHT; ++y)
				{
					for (uint x=0; x < WIDTH; ++x, ++src, dst[0] += 2, dst[1] += 2)
					{
						if (y)
						{
							i = input.palette[src[ -WIDTH   ]];
							j = input.palette[src[ -WIDTH+1 ]];
						}

						d = x > 0 ?       input.palette[src[ -1 ]] : 0;
						e =               input.palette[src[  0 ]];
						f = x < WIDTH-1 ? input.palette[src[  1 ]] : 0;
						l = x < WIDTH-2 ? input.palette[src[  2 ]] : 0;

						if (y < HEIGHT-1)
						{
							a = x > 0 ?       input.palette[src[ WIDTH-1 ]] : 0;
							b =               input.palette[src[ WIDTH   ]];
							c = x < WIDTH-1 ? input.palette[src[ WIDTH+1 ]] : 0;
							k = x < WIDTH-2 ? input.palette[src[ WIDTH+2 ]] : 0;

							if (y < HEIGHT-2)
							{
								g =               input.palette[src[ WIDTH*2   ]];
								h = x < WIDTH-1 ? input.palette[src[ WIDTH*2+1 ]] : 0;
							}
							else
							{
								g = h = 0;
							}
						}
						else
						{
							a = b = c = k = g = h = 0;
						}

						dword q[4];

						if (b == f && e != c)
						{
							q[1] = q[2] = b;

							if ((a == b && f == l) || (b == g && f == j))
							{
								q[0] = Blend( b, e    );
								q[0] = Blend( b, q[0] );
								q[3] = Blend( b, c    );
								q[3] = Blend( b, q[3] );
							}
							else
							{
								q[0] = Blend( e, f );
								q[3] = Blend( b, c );
							}
						}
						else if (e == c && b != f)
						{
							q[3] = q[0] = e;

							if ((i == e && c == h) || (d == e && c == k))
							{
								q[1] = Blend( e, f    );
								q[1] = Blend( e, q[1] );
								q[2] = Blend( e, b    );
								q[2] = Blend( e, q[2] );
							}
							else
							{
								q[1] = Blend( e, f );
								q[2] = Blend( b, c );
							}
						}
						else if (e == c && b == f && e != f)
						{
							const int result =
							(
								(f == a && f == g ? -1 : e == a && e == g ? +1 : 0) +
								(f == d && f == i ? -1 : e == d && e == i ? +1 : 0) +
								(f == h && f == k ? -1 : e == h && e == k ? +1 : 0) +
								(f == j && f == l ? -1 : e == j && e == l ? +1 : 0)
							);

							if (result > 0)
							{
								q[1] = q[2] = b;
								q[0] = q[3] = Blend( e, f) ;
							}
							else if (result < 0)
							{
								q[3] = q[0] = e;
								q[1] = q[2] = Blend( e, f );
							}
							else
							{
								q[3] = q[0] = e;
								q[1] = q[2] = b;
							}
						}
						else
						{
							if (b == e || c == f)
							{
								q[0] = e;
								q[2] = b;
								q[1] = f;
								q[3] = c;

							}
							else
							{
								q[1] = q[0] = Blend( e, f );
								q[0] = Blend( e, q[0] );
								q[1] = Blend( f, q[1] );

								q[2] = q[3] = Blend( b, c );
								q[2] = Blend( b, q[2] );
								q[3] = Blend( c, q[3] );
							}
						}

						dst[0][0] = q[0];
						dst[0][1] = q[1];
						dst[1][0] = q[2];
						dst[1][1] = q[3];
					}

					dst[0] = reinterpret_cast<T*>(reinterpret_cast<u8*>(dst[1]) + (pitch - long(sizeof(T) * WIDTH*2)));
					dst[1] = reinterpret_cast<T*>(reinterpret_cast<u8*>(dst[0]) + pitch);
				}
			}

			template<typename T>
			NST_FORCE_INLINE void Renderer::Filter2xSaI::BlitType(const Input& input,const Output& output) const
			{
				switch (type)
				{
					case RenderState::FILTER_2XSAI:

						Blit2xSaI<T>( input, output );
						break;

					case RenderState::FILTER_SUPER_2XSAI:

						BlitSuper2xSaI<T>( input, output );
						break;

					case RenderState::FILTER_SUPER_EAGLE:

						BlitSuperEagle<T>( input, output );
						break;

					NST_UNREACHABLE
				}
			}

			void Renderer::Filter2xSaI::Blit(const Input& input,const Output& output,uint)
			{
				switch (bpp)
				{
					case 32: BlitType< u32 >( input, output ); break;
					case 16: BlitType< u16 >( input, output ); break;

					NST_UNREACHABLE
				}
			}
		}
	}
}

#endif
