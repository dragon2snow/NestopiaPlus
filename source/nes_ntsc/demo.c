
/* Uses nes_ntsc to display a raw NES image on screen with adjustment of
hue and sharpness using mouse. Writes "nes.pal" RGB color file on exit.

Spacebar    Toggles field merging
S           Toggles between standard and Sony decoder matrix
*/

#include "nes_ntsc.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "SDL.h"

/* NES pixel buffer size */
enum { nes_width  = nes_ntsc_min_in_width };
enum { nes_height = 240 };

/* Output size */
enum { width  = nes_ntsc_min_out_width };
enum { height = nes_height * 2 };

/* Shell */
void fatal_error( const char* str );
void sdl_init( int width, int height, int depth );
void sdl_lock_pixels();
void sdl_display();
int sdl_run();
static unsigned char* sdl_pixels;
static long sdl_pitch;
static float mouse_x = 0.5f, mouse_y = 0.5f; /* 0.0 to 1.0 */
static int key_pressed;

/* Globals */
static unsigned char* nes_pixels;
static nes_ntsc_setup_t setup; /* making this static conveniently clears all fields to 0 */
static nes_ntsc_t* ntsc;
static int phase;

static void init()
{
	FILE* file;
	
	/* allocate memory for nes_ntsc and initialize */
	ntsc = (nes_ntsc_t*) malloc( sizeof (nes_ntsc_t) );
	if ( !ntsc )
		fatal_error( "Out of memory" );
	setup.merge_fields = 1;
	nes_ntsc_init( ntsc, &setup );
	
	/* read raw nes image */
	file = fopen( "nes.raw", "rb" );
	nes_pixels = (unsigned char*) malloc( (long) nes_height * nes_width );
	if ( !nes_pixels )
		fatal_error( "Out of memory" );
	if ( !file )
		fatal_error( "Couldn't open image file" );
	fread( nes_pixels, nes_width, nes_height, file );
	fclose( file );
}

static void display()
{
	int i;
	sdl_lock_pixels();
	
	/* force phase to 0 if merge_fields is on */
	phase ^= 1;
	if ( setup.merge_fields )
		phase = 0;
	
	/* blit nes image on every other scanline of output by halving the height
	and doubling the output pitch */
	nes_ntsc_blit( ntsc, nes_pixels, nes_width, phase, width, height / 2,
			(unsigned short*) sdl_pixels, sdl_pitch * 2 );
	
	/* fill in blank scanlines with duplicates */
	for ( i = 0; i < height; i += 2 )
	{
		unsigned char* line = sdl_pixels + i * sdl_pitch;
		memcpy( line + sdl_pitch, line, width * 2 );
	}
	
	sdl_display();
}

static void write_palette()
{
	FILE* out = fopen( "nes.pal", "wb" );
	if ( out )
	{
		unsigned char palette [64 * 3];
		setup.palette_out = palette;
		nes_ntsc_init( 0, &setup );
		fwrite( palette, sizeof palette, 1, out );
	}
}

int main( int argc, char** argv )
{
	init();
	sdl_init( width, height, 16 ); /* 16-bit RGB pixel buffer */
	
	/* keep displaying frames until mouse is clicked */
	while ( sdl_run() )
	{
		display();
		
		/* mouse controls saturation and sharpness */
		if ( mouse_x >= 0 )
		{
			/* available parameters: hue, saturation, contrast, brightness,
			sharpness, hue_warping */
			setup.saturation = mouse_x * 2 - 1;
			setup.sharpness  = mouse_y * 2 - 1;
			
			nes_ntsc_init( ntsc, &setup );
			mouse_x = -1; /* only call nes_ntsc_init when mouse moves */
		}
		
		/* space toggles field merging */
		if ( key_pressed == SDLK_SPACE )
		{
			setup.merge_fields = !setup.merge_fields;
			nes_ntsc_init( ntsc, &setup );
		}
		
		/* S toggles between standard and Sony decoder matrix */
		if ( key_pressed == SDLK_s )
		{
			if ( !setup.decoder_matrix )
			{
				/* Sony CXA2095S US */
				static float matrix [6] = { 1.539, -0.622, -0.571, -0.185, 0.000, 2.000 };
				setup.decoder_matrix = matrix;
				setup.hue = 33 / 180.0;
			}
			else
			{
				setup.decoder_matrix = 0;
				setup.hue = 0;
			}
			nes_ntsc_init( ntsc, &setup );
		}
	}
	
	free( ntsc );
	
	write_palette();
	
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

