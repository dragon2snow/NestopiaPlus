
/* nes_ntsc 0.2.1. http://www.slack.net/~ant/ */

#include "nes_ntsc.h"

/* Copyright (C) 2006 Shay Green. This module is free software; you
can redistribute it and/or modify it under the terms of the GNU Lesser
General Public License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version. This
module is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details. You should have received a copy of the GNU Lesser General
Public License along with this module; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA */

nes_ntsc_setup_t const nes_ntsc_monochrome = { 0,-1, 0, 0,.2,  0,.2,-.2,-.2,-1, 1, 0, 0, 0 };
nes_ntsc_setup_t const nes_ntsc_composite  = { 0, 0, 0, 0, 0,  0, 0,  0,  0, 0, 1, 0, 0, 0 };
nes_ntsc_setup_t const nes_ntsc_svideo     = { 0, 0, 0, 0,.2,  0,.2, -1, -1, 0, 1, 0, 0, 0 };
nes_ntsc_setup_t const nes_ntsc_rgb        = { 0, 0, 0, 0,.2,  0,.7, -1, -1,-1, 1, 0, 0, 0 };

#define ntsc_prefix     nes
#define alignment_count 3
#define burst_count     3
#define rescale_in      8
#define rescale_out     7
#define artifacts_mid   1.0f
#define fringing_mid    1.0f
#define std_decoder_hue -15

#include "nes_ntsc_impl.h"

pixel_info_t const ntsc_pixels [alignment_count] = {
	{ PIXEL_OFFSET( -4, -9 ), { 1, 1, .6667f, 0 } },
	{ PIXEL_OFFSET( -2, -7 ), {       .3333f, 1, 1, .3333f } },
	{ PIXEL_OFFSET(  0, -5 ), {                  0, .6667f, 1, 1 } },
};

static void correct_errors( ntsc_rgb_t color, ntsc_rgb_t* out )
{
	int n;
	for ( n = burst_count; n; --n )
	{
		int i;
		for ( i = 0; i < rgb_kernel_size / 2; i++ )
		{
			ntsc_rgb_t error = color -
					out [i    ] - out [(i+12)%14+14] - out [(i+10)%14+28] -
					out [i + 7] - out [i + 5    +14] - out [i + 3    +28];
			DISTRIBUTE_ERROR( i+3+28, i+5+14, i+7 );
		}
		out += alignment_count * rgb_kernel_size;
	}
}

static void nes_ntsc_init_( ntsc_rgb_t* table, nes_ntsc_setup_t const* setup, int color_count )
{
	int merge_fields;
	int entry;
	ntsc_impl_t impl;
	float gamma_factor;
	
	if ( !setup )
		setup = &nes_ntsc_composite;
	
	{
		float const gamma = 1.1333f - 1 - (float) setup->gamma * 0.5f;
		gamma_factor = (float) pow( (float) fabs( gamma ), 0.73f );
		if ( gamma < 0 )
			gamma_factor = -gamma_factor;
	}
	
	init_ntsc_impl( &impl, setup );
	
	merge_fields = setup->merge_fields;
	if ( setup->artifacts <= -1 && setup->fringing <= -1 )
		merge_fields = 1;
	
	for ( entry = 0; entry < color_count; entry++ )
	{
		static float const lo_levels [4] = { -0.12f, 0.00f, 0.31f, 0.72f };
		static float const hi_levels [4] = {  0.40f, 0.68f, 1.00f, 1.00f };
		int level = entry >> 4 & 0x03;
		float lo = lo_levels [level];
		float hi = hi_levels [level];
		
		int color = entry & 0x0F;
		if ( color == 0 )
			lo = hi;
		if ( color == 0x0D )
			hi = lo;
		if ( color > 0x0D )
			hi = lo = 0.0f;
		
		{
			#define TO_ANGLE( c ) (PI / 6 * ((c) - 3))
			float angle = TO_ANGLE( color );
			float sat = (hi - lo) * 0.5f;
			float i = (float) sin( angle ) * sat;
			float q = (float) cos( angle ) * sat;
			float y = (hi + lo) * 0.5f;
			int tint = entry >> 6 & 7;
			
			/* apply color emphasis */
			if ( tint && color <= 0x0D )
			{
				float const atten_mul = 0.79399f;
				float const atten_sub = 0.0782838f;
				
				if ( tint == 7 )
				{
					y = y * (atten_mul * 1.13f) - (atten_sub * 1.13f);
				}
				else
				{
					static unsigned char tints [8] = { 0, 6, 10, 8, 2, 4, 0, 0 };
					float angle = TO_ANGLE( tints [tint] );
					float sat = hi * (0.5f - atten_mul * 0.5f) + atten_sub * 0.5f;
					y -= sat * 0.5f;
					if ( tint >= 3 && tint != 4 )
					{
						/* combined tint bits */
						sat *= 0.6f;
						y -= sat;
					}
					i += (float) sin( angle ) * sat;
					q += (float) cos( angle ) * sat;
				}
			}
			
			if ( setup->palette )
			{
				unsigned char const* in = &setup->palette [entry * 3];
				float const to_float = 1.0f / 0xFF;
				float r = to_float * *in++;
				float g = to_float * *in++;
				float b = to_float * *in++;
				q = RGB_TO_YIQ( r, g, b, y, i );
			}
			
			y *= (float) setup->contrast * 0.5f + 1;
			y += (float) setup->brightness * 0.5f;
			
			{
				float r, g, b = YIQ_TO_RGB( y, i, q, default_decoder, float, r, g );
				
				/* fast approximation of n = pow( n, gamma ) */
				r = (r * gamma_factor - gamma_factor) * r + r;
				g = (g * gamma_factor - gamma_factor) * g + g;
				b = (b * gamma_factor - gamma_factor) * b + b;
				
				q = RGB_TO_YIQ( r, g, b, y, i );
			}
			
			i *= rgb_unit;
			q *= rgb_unit;
			y *= rgb_unit;
			y += rgb_offset;
			
			{
				int r, g, b = YIQ_TO_RGB( y, i, q, impl.to_rgb, int, r, g );
				/* blue tends to overflow */
				ntsc_rgb_t rgb = PACK_RGB( r, g, (b < 0x3E0 ? b: 0x3E0) );
				
				if ( setup->palette_out )
				{
					unsigned char* out = &setup->palette_out [entry * 3];
					ntsc_rgb_t clamped = rgb;
					NTSC_CLAMP( clamped );
					*out++ = (unsigned char) (clamped >> 21);
					*out++ = (unsigned char) (clamped >> 11);
					*out++ = (unsigned char) (clamped >>  1);
				}
				
				if ( table )
				{
					gen_kernel( &impl, y, i, q, table );
					if ( merge_fields )
						merge_kernel_fields( table );
					correct_errors( rgb, table );
					table += nes_ntsc_entry_size;
				}
			}
		}
	}
}

void nes_ntsc_init( nes_ntsc_t* ntsc, nes_ntsc_setup_t const* setup )
{
	nes_ntsc_init_( (ntsc ? ntsc->table : 0), setup, nes_ntsc_palette_size );
}

void nes_ntsc_init_emph( nes_ntsc_emph_t* ntsc, nes_ntsc_setup_t const* setup )
{
	nes_ntsc_init_( (ntsc ? ntsc->table : 0), setup, nes_ntsc_emph_palette_size );
}

#ifndef NES_NTSC_OUT_DEPTH
	#define NES_NTSC_OUT_DEPTH 16
#endif

#if NES_NTSC_OUT_DEPTH > 16
	typedef ntsc_uint32_t nes_ntsc_out_t;
#else
	typedef ntsc_uint16_t nes_ntsc_out_t;
#endif

#ifndef NES_NTSC_NO_BLITTERS

void nes_ntsc_blit( nes_ntsc_t const* ntsc, unsigned char const* nes_in,
		long in_row_width, int burst_phase, int in_width, int in_height,
		void* rgb_out, long out_pitch )
{
	int chunk_count = (in_width - 1) / nes_ntsc_in_chunk;
	for ( ; in_height; --in_height )
	{
		unsigned char const* line_in = nes_in;
		NES_NTSC_BEGIN_ROW( ntsc, burst_phase,
				nes_ntsc_black, nes_ntsc_black, *line_in++ & 0x3F );
		nes_ntsc_out_t* restrict line_out = (nes_ntsc_out_t*) rgb_out;
		int n;
		
		for ( n = chunk_count; n; --n )
		{
			/* order of input and output pixels must not be altered */
			NES_NTSC_COLOR_IN( 0, line_in [0] & 0x3F );
			NES_NTSC_RGB_OUT( 0, line_out [0], NES_NTSC_OUT_DEPTH );
			NES_NTSC_RGB_OUT( 1, line_out [1], NES_NTSC_OUT_DEPTH );
			
			NES_NTSC_COLOR_IN( 1, line_in [1] & 0x3F );
			NES_NTSC_RGB_OUT( 2, line_out [2], NES_NTSC_OUT_DEPTH );
			NES_NTSC_RGB_OUT( 3, line_out [3], NES_NTSC_OUT_DEPTH );
			
			NES_NTSC_COLOR_IN( 2, line_in [2] & 0x3F );
			NES_NTSC_RGB_OUT( 4, line_out [4], NES_NTSC_OUT_DEPTH );
			NES_NTSC_RGB_OUT( 5, line_out [5], NES_NTSC_OUT_DEPTH );
			NES_NTSC_RGB_OUT( 6, line_out [6], NES_NTSC_OUT_DEPTH );
			
			line_in  += 3;
			line_out += 7;
		}
		
		/* finish final pixels */
		NES_NTSC_COLOR_IN( 0, nes_ntsc_black );
		NES_NTSC_RGB_OUT( 0, line_out [0], NES_NTSC_OUT_DEPTH );
		NES_NTSC_RGB_OUT( 1, line_out [1], NES_NTSC_OUT_DEPTH );
		
		NES_NTSC_COLOR_IN( 1, nes_ntsc_black );
		NES_NTSC_RGB_OUT( 2, line_out [2], NES_NTSC_OUT_DEPTH );
		NES_NTSC_RGB_OUT( 3, line_out [3], NES_NTSC_OUT_DEPTH );
		
		NES_NTSC_COLOR_IN( 2, nes_ntsc_black );
		NES_NTSC_RGB_OUT( 4, line_out [4], NES_NTSC_OUT_DEPTH );
		NES_NTSC_RGB_OUT( 5, line_out [5], NES_NTSC_OUT_DEPTH );
		NES_NTSC_RGB_OUT( 6, line_out [6], NES_NTSC_OUT_DEPTH );
		
		burst_phase = (burst_phase + 1) % nes_ntsc_burst_count;
		nes_in += in_row_width;
		rgb_out = (char*) rgb_out + out_pitch;
	}
}

void nes_ntsc_blit_emph( nes_ntsc_emph_t const* ntsc, unsigned short const* nes_in,
		long in_row_width, int burst_phase, int in_width, int height,
		void* rgb_out, long out_pitch )
{
	int chunk_count = (in_width - 1) / nes_ntsc_in_chunk;
	for ( ; height; --height )
	{
		unsigned short const* line_in = nes_in;
		NES_NTSC_BEGIN_ROW( ntsc, burst_phase,
				nes_ntsc_black, nes_ntsc_black, *line_in++ & 0x1FF );
		nes_ntsc_out_t* restrict line_out = (nes_ntsc_out_t*) rgb_out;
		int n;
		
		for ( n = chunk_count; n; --n )
		{
			NES_NTSC_COLOR_IN( 0, line_in [0] & 0x1FF );
			NES_NTSC_RGB_OUT( 0, line_out [0], NES_NTSC_OUT_DEPTH );
			NES_NTSC_RGB_OUT( 1, line_out [1], NES_NTSC_OUT_DEPTH );
			
			NES_NTSC_COLOR_IN( 1, line_in [1] & 0x1FF );
			NES_NTSC_RGB_OUT( 2, line_out [2], NES_NTSC_OUT_DEPTH );
			NES_NTSC_RGB_OUT( 3, line_out [3], NES_NTSC_OUT_DEPTH );
			
			NES_NTSC_COLOR_IN( 2, line_in [2] & 0x1FF );
			NES_NTSC_RGB_OUT( 4, line_out [4], NES_NTSC_OUT_DEPTH );
			NES_NTSC_RGB_OUT( 5, line_out [5], NES_NTSC_OUT_DEPTH );
			NES_NTSC_RGB_OUT( 6, line_out [6], NES_NTSC_OUT_DEPTH );
			
			line_in  += 3;
			line_out += 7;
		}
		
		NES_NTSC_COLOR_IN( 0, nes_ntsc_black );
		NES_NTSC_RGB_OUT( 0, line_out [0], NES_NTSC_OUT_DEPTH );
		NES_NTSC_RGB_OUT( 1, line_out [1], NES_NTSC_OUT_DEPTH );
		
		NES_NTSC_COLOR_IN( 1, nes_ntsc_black );
		NES_NTSC_RGB_OUT( 2, line_out [2], NES_NTSC_OUT_DEPTH );
		NES_NTSC_RGB_OUT( 3, line_out [3], NES_NTSC_OUT_DEPTH );
		
		NES_NTSC_COLOR_IN( 2, nes_ntsc_black );
		NES_NTSC_RGB_OUT( 4, line_out [4], NES_NTSC_OUT_DEPTH );
		NES_NTSC_RGB_OUT( 5, line_out [5], NES_NTSC_OUT_DEPTH );
		NES_NTSC_RGB_OUT( 6, line_out [6], NES_NTSC_OUT_DEPTH );
		
		burst_phase = (burst_phase + 1) % nes_ntsc_burst_count;
		nes_in += in_row_width;
		rgb_out = (char*) rgb_out + out_pitch;
	}
}

#endif

