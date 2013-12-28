
/* nes_ntsc 0.1.7. http://www.slack.net/~ant/ */

/* compilable in C or C++; just change the file extension */

#include "nes_ntsc.h"

#include <string.h>
#include <math.h>

/* Based on algorithm by NewRisingSun */
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

enum { alignment_count = 3 };
enum { rgb_kernel_size = 14 };
enum { composite_border = 6 };
enum { center_offset = 4 };

enum { composite_size = composite_border + 8 + composite_border };
enum { rgb_pad = (center_offset + composite_border + 7) / 8 * 8 - center_offset - composite_border };
enum { rgb_size = (rgb_pad + composite_size + 7) / 8 * 8 };
enum { rescaled_size = rgb_size / 8 * 7 };
enum { ntsc_kernel_size = composite_size * 2 };

enum { rgb_unit = 0x1000 }; /* (int) casts on rgb_unit work around compiler bug */
#define MAKE_KRGB( r, g, b ) \
	( ((r + 8) >> 4 << 21) + ((g + 8) >> 4 << 11) + ((b + 8) >> 4 << 1) )

typedef struct ntsc_to_rgb_t
{
	float composite [composite_size];
	float to_rgb [6];
	float decoder_matrix [6];
	float sin_hue;
	float cos_hue;
	float brightness;
	float contrast;
	float sharpness;
	float hue_warping;
	short rgb [rgb_size] [3];
	short rescaled [rescaled_size + 1] [3]; /* extra space for sharpen */
	float kernel [ntsc_kernel_size];
} ntsc_to_rgb_t;

#define PI 3.14159265358979323846f

static void rotate_matrix( float const* in, float s, float c, float* out )
{
	int n = 3;
	while ( n-- )
	{
		float i = *in++;
		float q = *in++;
		*out++ = i * c - q * s;
		*out++ = i * s + q * c;
	}
}

static void ntsc_to_rgb_init( ntsc_to_rgb_t* ntsc, nes_ntsc_setup_t const* setup )
{
	float const gaussian_factor = 1.3f; /* 1 = normal, > 1 reduces echoes of bright objects */
	int i;
	
	/* ranges need to be scaled a bit to avoid pixels overflowing at extremes */
	ntsc->brightness = setup->brightness * (0.4f * (int) rgb_unit);
	ntsc->contrast = setup->contrast * (0.4f * (int) rgb_unit) + (float) rgb_unit;
	ntsc->sharpness = 1 + (setup->sharpness < 0 ? setup->sharpness * 0.5f : setup->sharpness);
	ntsc->hue_warping = setup->hue_warping;
	
	for ( i = 0; i < composite_size; i++ )
		ntsc->composite [i] = 0;
	
	/* Generate gaussian kernel, padded with zero */
	for ( i = 0; i < ntsc_kernel_size; i++ )
		ntsc->kernel [i] = 0;
	for ( i = -composite_border; i <= composite_border; i++ )
		ntsc->kernel [ntsc_kernel_size / 2 + i] = exp( i * i * (-0.03125f * gaussian_factor) );
	
	/* normalize kernel totals of every fourth sample (at all four phases) to 0.5, otherwise
	i/q low-pass will favor one of the four alignments and cause repeating spots */
	for ( i = 0; i < 4; i++ )
	{
		float sum = 0;
		float scale;
		int x;
		for ( x = i; x < ntsc_kernel_size; x += 4 )
			sum += ntsc->kernel [x];
		scale = 0.5f / sum;
		for ( x = i; x < ntsc_kernel_size; x += 4 )
			ntsc->kernel [x] *= scale;
	}
	
	/* adjust decoder matrix */
	{
		static float const default_decoder [6] =
			{ 0.956f, 0.621f, -0.272f, -0.647f, -1.105f, 1.702f };
		float hue = setup->hue * PI;
		float sat = setup->saturation;
		float const* decoder = setup->decoder_matrix;
		if ( !decoder )
		{
			decoder = default_decoder;
			hue -= PI / 180 * 15;
		}
		sat = (sat < 0 ? sat : sat * 0.41f) + 1;
		ntsc->sin_hue = sin( hue );
		ntsc->cos_hue = cos( hue );
		rotate_matrix( decoder, ntsc->sin_hue * sat, ntsc->cos_hue * sat, ntsc->decoder_matrix );
	}
	
	memset( ntsc->rgb, 0, sizeof ntsc->rgb );
}

/* Convert NTSC composite signal to RGB, where composite signal contains only four
non-zero samples beginning at offset */
static void ntsc_to_rgb( ntsc_to_rgb_t const* ntsc, int offset, short* out )
{
	float const* comp = &ntsc->composite [offset];
	float const f0 = comp [0];
	float const f1 = comp [1];
	float const f2 = comp [2];
	float const f3 = comp [3];
	float const* kernel = &ntsc->kernel [ntsc_kernel_size / 2 - offset];
	int x = 0;
	while ( x < composite_size )
	{
		#define PIXEL( o, get_y ) \
		{\
			float i = kernel [o    ] * f0 + kernel [o - 2] * f2;\
			float q = kernel [o - 1] * f1 + kernel [o - 3] * f3;\
			float y = get_y;\
			float r = y + i * ntsc->to_rgb [0] + q * ntsc->to_rgb [1];\
			float g = y + i * ntsc->to_rgb [2] + q * ntsc->to_rgb [3];\
			float b = y + i * ntsc->to_rgb [4] + q * ntsc->to_rgb [5];\
			out [o * 3    ] = (short) (int) r;\
			out [o * 3 + 1] = (short) (int) g;\
			out [o * 3 + 2] = (short) (int) b;\
		}
		/* float->int->short instead of direct float->short, since latter might
		cause compiler to invoke library function or something, rather than truncating */ 
		
		PIXEL( 0, i - ntsc->composite [x + 0] )
		PIXEL( 1, q - ntsc->composite [x + 1] )
		PIXEL( 2, ntsc->composite [x + 2] - i )
		PIXEL( 3, ntsc->composite [x + 3] - q )
		kernel += 4;
		out += 4 * 3;
		x += 4;
		
		#undef PIXEL
	}
}

/* 7 output pixels for every 8 input pixels, linear interpolation */
static void rescale( short const* in, int count, short* out )
{
	do
	{
		int const accuracy = 16;
		int const unit = 1 << accuracy;
		int const step = unit / 8;
		int left = unit - step;
		int right = step;
		int n = 7;
		while ( n-- )
		{
			int r = (in [0] * left + in [3] * right) >> accuracy;
			int g = (in [1] * left + in [4] * right) >> accuracy;
			int b = (in [2] * left + in [5] * right) >> accuracy;
			out [0] = (short) r;
			out [1] = (short) g;
			out [2] = (short) b;
			left  -= step;
			right += step;
			in += 3;
			out += 3;
		}
		in += 3;
	}
	while ( (count -= 7) > 0 );
}

/* sharpen image using (level-1)/2, level, (level-1)/2 convolution kernel */
static void sharpen( short const* in, float level, int count, short* out )
{
	/* to do: sharpen luma only? */
	int const accuracy = 16;
	int const middle = (int) (level * (1 << accuracy));
	int const side   = (middle - (1 << accuracy)) >> 1;
	
	*out++ = *in++;
	*out++ = *in++;
	*out++ = *in++;
	
	for ( count = (count - 2) * 3; count--; in++ )
		*out++ = (short) ((in [0] * middle - in [-3] * side - in [3] * side) >> accuracy);
	
	*out++ = *in++;
	*out++ = *in++;
	*out++ = *in++;
}

/* Generate pixel and capture into table */
static ntsc_rgb_t* gen_pixel( ntsc_to_rgb_t* ntsc, int ntsc_pos, int rescaled_pos,
		ntsc_rgb_t* out )
{
	ntsc_to_rgb( ntsc, composite_border + ntsc_pos, ntsc->rgb [rgb_pad] );
	rescale( ntsc->rgb [0], rescaled_size, ntsc->rescaled [1] );
	sharpen( ntsc->rescaled [1], ntsc->sharpness, rescaled_size, ntsc->rescaled [0] );
	
	{
		short const* in = ntsc->rescaled [rescaled_pos];
		int n = rgb_kernel_size;
		while ( n-- )
		{
			*out++ = MAKE_KRGB( in [0], in [1], in [2] );
			in += 3;
		}
	}
	return out;
}

/* Generate pixel at all burst phases and column alignments */
static void gen_kernel( ntsc_to_rgb_t* ntsc, float y, float ci, float cq, ntsc_rgb_t* out )
{
	static float const burst_phases [nes_ntsc_burst_count] [2] = {
		/* 0 deg, -120 deg, -240 deg */
		{0.0f, 1.0f}, {-0.866025f, -0.5f}, {0.866025f, -0.5f}
	};
	int burst;
	
	/* generate for each scanline burst phase */
	for ( burst = 0; burst < nes_ntsc_burst_count; burst++ )
	{
		/* adjust i, q, and decoder matrix for current burst phase */
		float sin_b = burst_phases [burst] [0];
		float cos_b = burst_phases [burst] [1];
		float fi = ci * cos_b - cq * sin_b;
		float fq = ci * sin_b + cq * cos_b;
		rotate_matrix( ntsc->decoder_matrix, sin_b, cos_b, ntsc->to_rgb );
		
		/* generate at the three different column alignments */
		ntsc->composite [composite_border + 0] = fi + y;
		ntsc->composite [composite_border + 1] = fq + y;
		ntsc->composite [composite_border + 2] = (fi - y) * (2 / 3.0f);
		out = gen_pixel( ntsc, 0, 5, out );
		
		ntsc->composite [composite_border + 0] = 0;
		ntsc->composite [composite_border + 1] = 0;
		ntsc->composite [composite_border + 2] = (fi - y) * (1 / 3.0f);
		ntsc->composite [composite_border + 3] = fq - y;
		ntsc->composite [composite_border + 4] = fi + y;
		ntsc->composite [composite_border + 5] = (fq + y) * (1 / 3.0f);
		out = gen_pixel( ntsc, 2, 7, out );
		
		ntsc->composite [composite_border + 2] = 0;
		ntsc->composite [composite_border + 3] = 0;
		ntsc->composite [composite_border + 4] = 0;
		ntsc->composite [composite_border + 5] = (fq + y) * (2 / 3.0f);
		ntsc->composite [composite_border + 6] = fi - y;
		ntsc->composite [composite_border + 7] = fq - y;
		out = gen_pixel( ntsc, 4, 9, out );
		
		/* keep composite clear for next time */
		ntsc->composite [composite_border + 5] = 0;
		ntsc->composite [composite_border + 6] = 0;
		ntsc->composite [composite_border + 7] = 0;
	}
}

/* correct kernel colors and merge burst phases */
static void adjust_kernel( ntsc_rgb_t color, int merge_fields, ntsc_rgb_t* out )
{
	ntsc_rgb_t const bias = 0x200 * ntsc_rgb_builder;
	
	if ( merge_fields )
	{
		ntsc_rgb_t const mask = ntsc_rgb_builder;
		int i;
		for ( i = 0; i < nes_ntsc_burst_size; i++ )
		{
			ntsc_rgb_t* p = &out [i];
			ntsc_rgb_t p0 = p [nes_ntsc_burst_size * 0] + bias;
			ntsc_rgb_t p1 = p [nes_ntsc_burst_size * 1] + bias;
			ntsc_rgb_t p2 = p [nes_ntsc_burst_size * 2] + bias;
			/* merge fields without losing precision */
			p [nes_ntsc_burst_size * 0] = ((p0 + p1 - ((p0 ^ p1) & mask)) >> 1) - bias;
			p [nes_ntsc_burst_size * 1] = ((p1 + p2 - ((p1 ^ p2) & mask)) >> 1) - bias;
			p [nes_ntsc_burst_size * 2] = ((p2 + p0 - ((p2 ^ p0) & mask)) >> 1) - bias;
		}
	}
	
	/* correct roundoff errors that would cause speckles in solid areas */
	color += bias;
	{
		int burst;
		for ( burst = 0; burst < nes_ntsc_burst_count; burst++ )
		{
			int i;
			for ( i = 0; i < rgb_kernel_size / 2; i++ )
			{
				/* pixel of first kernel should be difference between final color and
				sum of pixels from other kernels */
				out [i] = color - out [(i+12)%14+14] - out [(i+10)%14+28] -
					out [i + 7] - out [ i+ 5    +14] - out [ i+ 3    +28];
			}
			out += alignment_count * rgb_kernel_size;
		}
	}
}

static void nes_ntsc_init_( ntsc_rgb_t* table, nes_ntsc_setup_t const* setup, int color_count )
{
	/* init pixel renderer */
	int entry;
	ntsc_to_rgb_t ntsc;
	ntsc_to_rgb_init( &ntsc, setup );
	
	/* Generate pixel for every possible color */
	for ( entry = 0; entry < color_count; entry++ )
	{
		/* determine y/i/q levels for color */
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
			float sat = (hi - lo) * (0.5f * (int) rgb_unit);
			float i = sin( angle ) * sat;
			float q = cos( angle ) * sat;
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
					sat *= (float) rgb_unit;
					i += sin( angle ) * sat;
					q += cos( angle ) * sat;
				}
			}
			
			/* warp hue */
			if ( ntsc.hue_warping )
			{
				/* rotate to angle decoder would use */
				float wi = q * ntsc.sin_hue + i * ntsc.cos_hue;
				float wq = q * ntsc.cos_hue - i * ntsc.sin_hue;
				
				float factor = wi * wq;
				if ( factor <= 0 && wq != 0 )
				{
					factor *= ntsc.hue_warping / (wi * wi + wq * wq);
					wi -= wi * factor;
					wq += wq * factor;
					
					/* rotate back to pre-decoder angle */
					i = wi * ntsc.cos_hue - wq * ntsc.sin_hue;
					q = wi * ntsc.sin_hue + wq * ntsc.cos_hue;
				}
			}
			
			y = y * ntsc.contrast + ntsc.brightness;
			
			{
				/* determine color kernel should produce */
				float r = y + i * ntsc.decoder_matrix [0] + q * ntsc.decoder_matrix [1];
				float g = y + i * ntsc.decoder_matrix [2] + q * ntsc.decoder_matrix [3];
				float b = y + i * ntsc.decoder_matrix [4] + q * ntsc.decoder_matrix [5];
				ntsc_rgb_t rgb = MAKE_KRGB( (int) r, (int) g, (int) b );
				
				/* optionally output to palette */
				unsigned char* out = setup->palette_out;
				if ( out )
				{
					ntsc_rgb_t clamped = rgb + 0x200 * ntsc_rgb_builder;
					NES_NTSC_CLAMP_( clamped );
					out += entry * 3;
					out [0] = clamped >> 21 & 0xFF;
					out [1] = clamped >> 11 & 0xFF;
					out [2] = clamped >>  1 & 0xFF;
				}
				
				/* generate and adjust kernel */
				if ( table )
				{
					gen_kernel( &ntsc, y, i, q, table );
					adjust_kernel( rgb, setup->merge_fields, table );
					table += nes_ntsc_entry_size;
				}
			}
		}
	}
}

void nes_ntsc_init( nes_ntsc_t* emu, nes_ntsc_setup_t const* setup )
{
	nes_ntsc_init_( (emu ? emu->table : 0), setup, nes_ntsc_color_count );
}

void nes_ntsc_init_emph( nes_ntsc_emph_t* emu, nes_ntsc_setup_t const* setup )
{
	nes_ntsc_init_( (emu ? emu->table : 0), setup, nes_ntsc_color_count * nes_ntsc_emph_count );
}

/* Use this as a starting point for writing your own blitter. To allow easy upgrades
to new versions of this library, put your blitter in a separate source file rather
than modifying this one directly. */

void nes_ntsc_blit( nes_ntsc_t const* ntsc, unsigned char const* nes_in, long in_pitch,
		int burst_phase, int width, int height, unsigned short* rgb_out, long out_pitch )
{
	/* determine how many chunks to blit, less one for the final chunk */
	int chunk_count = width / nes_ntsc_out_chunk - 1;
	while ( height-- )
	{
		/* begin row and read first nes pixel */
		unsigned char const* line_in = nes_in;
		NES_NTSC_BEGIN_ROW( ntsc, burst_phase, *line_in++ & 0x3F );
		unsigned short* line_out = rgb_out;
		int n;
		
		/* blit main chunks, each using 3 nes pixels to generate 7 output pixels */
		for ( n = chunk_count; n; --n )
		{
			/* order of NES_NTSC_COLOR_IN and NES_NTSC_RGB16_OUT must not be altered */
			
			/* you can use NES_NTSC_RGB24_OUT and NES_NTSC_RGB15_OUT for other RGB
			output formats, or NES_NTSC_RAW_OUT for the internal format */
			NES_NTSC_COLOR_IN( 0, line_in [0] & 0x3F );
			NES_NTSC_RGB16_OUT( 0, line_out [0] );
			NES_NTSC_RGB16_OUT( 1, line_out [1] );
			
			NES_NTSC_COLOR_IN( 1, line_in [1] & 0x3F );
			NES_NTSC_RGB16_OUT( 2, line_out [2] );
			NES_NTSC_RGB16_OUT( 3, line_out [3] );
			
			NES_NTSC_COLOR_IN( 2, line_in [2] & 0x3F );
			NES_NTSC_RGB16_OUT( 4, line_out [4] );
			NES_NTSC_RGB16_OUT( 5, line_out [5] );
			NES_NTSC_RGB16_OUT( 6, line_out [6] );
			
			line_in  += 3;
			line_out += 7;
		}
		
		/* you can eliminate the need for the final chunk below by padding your nes
		input with three extra black pixels at the end of each row */
		
		/* finish final pixels without starting any new ones */
		NES_NTSC_COLOR_IN( 0, nes_ntsc_black );
		NES_NTSC_RGB16_OUT( 0, line_out [0] );
		NES_NTSC_RGB16_OUT( 1, line_out [1] );
		
		NES_NTSC_COLOR_IN( 1, nes_ntsc_black );
		NES_NTSC_RGB16_OUT( 2, line_out [2] );
		NES_NTSC_RGB16_OUT( 3, line_out [3] );
		
		NES_NTSC_COLOR_IN( 2, nes_ntsc_black );
		NES_NTSC_RGB16_OUT( 4, line_out [4] );
		NES_NTSC_RGB16_OUT( 5, line_out [5] );
		NES_NTSC_RGB16_OUT( 6, line_out [6] );
		
		/* advance burst phase and line pointers */
		burst_phase = (burst_phase + 1) % nes_ntsc_burst_count;
		nes_in += in_pitch;
		rgb_out = (unsigned short*) ((char*) rgb_out + out_pitch);
	}
}
