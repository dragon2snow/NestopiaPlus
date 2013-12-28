
/* Uses snes_ntsc to display a raw SNES image on screen with adjustment of
sharpness and resolution using mouse.

Space   Toggles field merging
C       Composite video quality
S       S-video quality
R       RGB video quality
M       Monochrome video quality
D       Toggles between standard and Sony decoder matrix
*/

#include "snes_ntsc.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "SDL.h"

/* SNES pixel buffer size */
enum { snes_width  = 256 };
enum { snes_height = 223 };

/* Output size */
enum { out_width  = SNES_NTSC_OUT_WIDTH( snes_width ) };
enum { out_height = snes_height * 2 };

/* Shell */
void fatal_error( const char* str );
void sdl_init( int width, int height, int depth );
void sdl_lock_pixels();
void sdl_display();
int sdl_run();
static unsigned char* sdl_pixels;
static long sdl_pitch;
static float mouse_x = 0.5f, mouse_y = 0.5f; /* 0.0 to 1.0 */
static int mouse_moved;
static int key_pressed;

/* Globals */
static unsigned short* snes_pixels;
static snes_ntsc_setup_t setup;
static snes_ntsc_t* ntsc;
static int burst_phase = 0;

static void init()
{
	/* read raw image */
	FILE* file = fopen( "snes.raw", "rb" );
	if ( !file )
		fatal_error( "Couldn't open image file" );
	snes_pixels = (unsigned short*) malloc( snes_height * (snes_width * sizeof *snes_pixels) );
	if ( !snes_pixels )
		fatal_error( "Out of memory" );
	fread( snes_pixels, snes_width * 2, snes_height, file );
	fclose( file );
	
	/* convert from little-endian to native byte order */
	{
		long i;
		for ( i = 0; i < (long) snes_height * snes_width; i++ )
		{
			unsigned short* p = &snes_pixels [i];
			*p = ((unsigned char*) p) [1] * 0x100 + ((unsigned char*) p) [0];
		}
	}
	
	/* allocate memory for snes_ntsc and initialize */
	ntsc = (snes_ntsc_t*) malloc( sizeof (snes_ntsc_t) );
	if ( !ntsc )
		fatal_error( "Out of memory" );
	snes_ntsc_init( ntsc, &setup );
}

static void display()
{
	sdl_lock_pixels();
	
	/* force phase to 0 if merge_fields is on */
	burst_phase ^= 1;
	if ( setup.merge_fields )
		burst_phase = 0;
	
	/* blit image to every other row of output by doubling output pitch */
	snes_ntsc_blit( ntsc, snes_pixels, snes_width, burst_phase,
			snes_width, snes_height, sdl_pixels, sdl_pitch * 2 );
	
	/* interpolate and darken between scanlines */
	{
		int y;
		for ( y = 1; y < out_height - 1; y += 2 )
		{
			unsigned char* io = sdl_pixels + y * sdl_pitch;
			int n;
			for ( n = out_width; n; --n )
			{
				unsigned prev = *(unsigned short*) (io - sdl_pitch);
				unsigned next = *(unsigned short*) (io + sdl_pitch);
				/* mix 16-bit rgb without losing low bits */
				unsigned mixed = prev + next + ((prev ^ next) & 0x0821);
				/* darken by 12% */
				*(unsigned short*) io = (mixed >> 1) - (mixed >> 4 & 0x18E3);
				io += 2;
			}
		}
	}
	
	sdl_display();
}

int main( int argc, char** argv )
{
	int merge_fields = 1;
	int sony_decoder = 0;
	
	setup = snes_ntsc_composite;
	init();
	sdl_init( out_width, out_height, 16 ); /* 16-bit RGB output buffer */
	
	/* keep displaying frames until mouse is clicked */
	while ( sdl_run() )
	{
		display();
		
		switch ( key_pressed )
		{
			case SDLK_SPACE: merge_fields = !merge_fields; break;
			
			case SDLK_c    : setup = snes_ntsc_composite; break;
			
			case SDLK_s    : setup = snes_ntsc_svideo; break;
			
			case SDLK_r    : setup = snes_ntsc_rgb; break;
			
			case SDLK_m    : setup = snes_ntsc_monochrome; break;
			
			case SDLK_d    : sony_decoder = !sony_decoder; break;
		}
		
		if ( key_pressed || mouse_moved )
		{
			/* convert mouse range to -1 to +1 */
			float x = mouse_x * 2 - 1;
			float y = mouse_y * 2 - 1;
			
			/* parameters: hue, saturation, contrast, brightness, sharpness,
			gamma, bleed, resolution, artifacts, fringing, hue_warping */
			setup.sharpness   = x;
			setup.hue_warping = y;
			
			setup.merge_fields = merge_fields;
			
			setup.decoder_matrix = 0;
			setup.hue = 0;
			if ( sony_decoder )
			{
				/* Sony CXA2095S US */
				static float matrix [6] = { 1.539, -0.622, -0.571, -0.185, 0.000, 2.000 };
				setup.decoder_matrix = matrix;
				setup.hue += 33 / 180.0;
			}
			
			snes_ntsc_init( ntsc, &setup );
		}
	}
	
	free( ntsc );
	
	return 0;
}

/* Shell */

static SDL_Rect rect;
static SDL_Surface* screen;
static SDL_Surface* surface;
static unsigned long next_time;

void fatal_error( const char* str )
{
	fprintf( stderr, "Error: %s\n", str );
	exit( EXIT_FAILURE );
}

void sdl_init( int width, int height, int depth )
{
	rect.w = width;
	rect.h = height;
	
	if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
		fatal_error( "SDL initialization failed" );
	atexit( SDL_Quit );
	
	screen = SDL_SetVideoMode( width, height, 0, 0 );
	surface = SDL_CreateRGBSurface( SDL_SWSURFACE, width, height, depth, 0, 0, 0, 0 );
	if ( !screen || !surface )
		fatal_error( "SDL initialization failed" );
}

int sdl_run()
{
	SDL_Event e;
	
	/* limit to 60 calls per second */
	unsigned long start = SDL_GetTicks();
	if ( start < next_time && next_time - start > 10 )
		SDL_Delay( next_time - start );
	while ( SDL_GetTicks() < next_time ) { }
	next_time = start + 1000 / 60;
	
	mouse_moved = 0;
	key_pressed = 0;
	
	while ( SDL_PollEvent( &e ) )
	{
		if ( e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_QUIT )
			return 0;
		
		if ( e.type == SDL_KEYDOWN )
		{
			if ( e.key.keysym.sym == SDLK_ESCAPE || e.key.keysym.sym == SDLK_q )
				return 0;
			key_pressed = e.key.keysym.sym;
		}
		
		if ( e.type == SDL_MOUSEMOTION )
		{
			int x, y;
			SDL_GetMouseState( &x, &y );
			mouse_moved = 1;
			mouse_x = x / (float) (SDL_GetVideoSurface()->w - 1);
			mouse_y = 1 - y / (float) (SDL_GetVideoSurface()->h - 1);
		}
	}
	return 1;
}

void sdl_lock_pixels()
{
	if ( SDL_LockSurface( surface ) < 0 )
		fatal_error( "Couldn't lock surface" );
	sdl_pitch = surface->pitch;
	sdl_pixels = (unsigned char*) surface->pixels;
}

void sdl_display()
{
	SDL_UnlockSurface( surface );
	if ( SDL_BlitSurface( surface, &rect, screen, &rect ) < 0 || SDL_Flip( screen ) < 0 )
		fatal_error( "SDL blit failed" );
}

