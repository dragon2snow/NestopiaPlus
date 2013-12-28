
/* NES NTSC composite video to RGB emulator/blitter */

/* nes_ntsc 0.1.7 */

#ifndef NES_NTSC_H
#define NES_NTSC_H

/* Picture parameters, ranging from -1.0 to 1.0 where 0.0 is normal. To easily
clear all fields, make it a static object then set whatever fields you want:
	static nes_ntsc_setup_t setup;
	setup.hue = ... */
typedef struct nes_ntsc_setup_t
{
	float hue; /* -1.0 = -180 degrees, +1.0 = +180 degrees */
	float saturation;
	float contrast;
	float brightness;
	float sharpness;
	float hue_warping; /* < 0 expands purple and green, > 0 expands orange and cyan */
	int merge_fields;  /* if 1, merges even and odd fields together to reduce flicker */
	float const* decoder_matrix; /* optional RGB decoder matrix, 6 elements */
	unsigned char* palette_out; /* optional RGB palette out, 3 bytes per color */
} nes_ntsc_setup_t;

/* Initialize and adjust parameters. Can be called multiple times on the same
nes_ntsc_t object. Caller must allocate memory for nes_ntsc_t. */
typedef struct nes_ntsc_t nes_ntsc_t;
void nes_ntsc_init( nes_ntsc_t* ntsc, nes_ntsc_setup_t const* setup );

/* Similar to nes_ntsc_init, with color emphasis support (requires custom blitter). */
typedef struct nes_ntsc_emph_t nes_ntsc_emph_t;
void nes_ntsc_init_emph( nes_ntsc_emph_t* ntsc, nes_ntsc_setup_t const* setup );

/* Blit one or more scanlines of NES 8-bit palette indicies to 16-bit RGB output. For
every 7 output pixels it reads approximately 3 NES pixels. Use constants below for
definite input and output pixel counts. */
void nes_ntsc_blit( nes_ntsc_t const* ntsc, unsigned char const* nes_in, long in_pitch,
		int burst_phase, int out_width, int out_height,
		unsigned short* rgb16_out, long out_pitch );

/* Useful values to use for output width and number of input pixels read */
enum { nes_ntsc_min_out_width = 602 }; /* minimum width that doesn't cut off active area */
enum { nes_ntsc_min_in_width  = 256 };

enum { nes_ntsc_640_out_width = 637 }; /* room for some overscan while fitting within 640 */
enum { nes_ntsc_640_in_width  = 271 };
enum { nes_ntsc_640_overscan_left  = 8 };
enum { nes_ntsc_640_overscan_right = 7 };

enum { nes_ntsc_full_out_width = 665 }; /* room for full overscan*/
enum { nes_ntsc_full_in_width  = 283 };
enum { nes_ntsc_full_overscan_left  = 16 };
enum { nes_ntsc_full_overscan_right = 11 };


/* Interface for user-defined custom blitters.
Can be used with nes_ntsc_t and nes_ntsc_emph_t */

enum { nes_ntsc_burst_count = 3  }; /* burst phase cycles through 0, 1, and 2 */
enum { nes_ntsc_in_chunk    = 3  }; /* number of nes pixels read per chunk */
enum { nes_ntsc_out_chunk   = 7  }; /* number of output pixels generated per chunk */
enum { nes_ntsc_black = 0x0F };     /* palette index for black */

/* Begin outputting row. Declares variables, so must be before first statement in a block. */
#define NES_NTSC_BEGIN_ROW( ntsc, burst, first_pixel ) \
	char const* ktable = (char*) (ntsc)->table + burst * (nes_ntsc_burst_size * sizeof (ntsc_rgb_t));\
	ntsc_rgb_t const* kernel0  = NES_NTSC_ENTRY_( nes_ntsc_black );\
	ntsc_rgb_t const* kernel1  = kernel0;\
	ntsc_rgb_t const* kernel2  = NES_NTSC_ENTRY_( first_pixel );\
	ntsc_rgb_t const* kernelx0 = kernel0;\
	ntsc_rgb_t const* kernelx1 = kernel0;\
	ntsc_rgb_t const* kernelx2 = kernel0

/* Begin NES pixel */
#define NES_NTSC_COLOR_IN( in_index, color_in ) \
	{ kernelx##in_index = kernel##in_index; kernel##in_index = NES_NTSC_ENTRY_( color_in ); }

/* Generate output pixel in raw, 24-bit, 16-bit, or 15-bit RGB format */

/* RRRRRRRR GGGGGGGG BBBBBBBB */
#define NES_NTSC_RGB24_OUT( out_index, rgb_out ) \
	NES_NTSC_RGB_OUT_( (out_index), rgb_out, NES_NTSC_RGB24_ )

/* RRRRRGGG GGGBBBBB */
#define NES_NTSC_RGB16_OUT( out_index, rgb_out ) \
	NES_NTSC_RGB_OUT_( (out_index), rgb_out, NES_NTSC_RGB16_ )

/* 0RRRRRGG GGGBBBBB */
#define NES_NTSC_RGB15_OUT( out_index, rgb_out ) \
	NES_NTSC_RGB_OUT_( (out_index), rgb_out, NES_NTSC_RGB15_ )

/* Bits can be 15, 16, 24, or 32 (treated as 24). It should be a compile-time constant
otherwise it'll generate a lot of code. */
#define NES_NTSC_RGB_OUT( out_index, rgb_out, bits ) \
	NES_NTSC_RGB_OUT_( (out_index), rgb_out, (bits == 32 || bits == 24 ? NES_NTSC_RGB24_ : \
	(bits == 16 ? NES_NTSC_RGB16_ : (bits == 15 ? NES_NTSC_RGB15_ : 0))) )

/* xxxRRRRR RRRxxGGG GGGGGxxB BBBBBBBx (x = junk bits, not necessarily zero) */
#define NES_NTSC_RAW_OUT( out_index, rgb_out ) \
	NES_NTSC_RGB_OUT_( (out_index), rgb_out, raw )

/* private */

enum { nes_ntsc_entry_size = 128 };
enum { nes_ntsc_color_count = 64 };
enum { nes_ntsc_emph_count = 8 };
typedef unsigned long ntsc_rgb_t;
struct nes_ntsc_t {
	ntsc_rgb_t table [nes_ntsc_color_count * nes_ntsc_entry_size];
};
struct nes_ntsc_emph_t {
	ntsc_rgb_t table [nes_ntsc_color_count * nes_ntsc_emph_count * nes_ntsc_entry_size];
};
enum { nes_ntsc_burst_size = nes_ntsc_entry_size / nes_ntsc_burst_count };

enum { ntsc_rgb_builder = (1L << 21) | (1 << 11) | (1 << 1) };
enum { nes_ntsc_clamp_mask = ntsc_rgb_builder * 3 / 2 };
enum { nes_ntsc_clamp_add  = ntsc_rgb_builder * 0x101 };

#define NES_NTSC_ENTRY_( n ) \
	(ntsc_rgb_t*) (ktable + (n) * (nes_ntsc_entry_size * sizeof (ntsc_rgb_t)))

#define NES_NTSC_RGB24_ ((raw >> 5 & 0xFF0000) | (raw >> 3 & 0xFF00) | (raw >> 1 & 0xFF))
#define NES_NTSC_RGB16_ ((raw >> 13  & 0xF800) | (raw >> 8 & 0x07E0) | (raw >> 4 & 0x001F))
#define NES_NTSC_RGB15_ ((raw >> 14  & 0x7C00) | (raw >> 9 & 0x03E0) | (raw >> 4 & 0x001F))

#define NES_NTSC_CLAMP_( io ){\
	ntsc_rgb_t sub = io >> 9 & nes_ntsc_clamp_mask;\
	ntsc_rgb_t clamp = nes_ntsc_clamp_add - sub;\
	io |= clamp;\
	clamp -= sub;\
	io &= clamp;\
}

#define NES_NTSC_RGB_OUT_( x, out, to_rgb ) { \
	ntsc_rgb_t raw =\
		kernel0  [x       ] + kernel1  [(x+12)%7+14] + kernel2  [(x+10)%7+28] +\
		kernelx0 [(x+7)%14] + kernelx1 [(x+ 5)%7+21] + kernelx2 [(x+ 3)%7+35];\
	NES_NTSC_CLAMP_( raw );\
	out = to_rgb;\
}

#endif

