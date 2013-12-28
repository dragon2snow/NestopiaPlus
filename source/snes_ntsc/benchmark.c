
/* Measures performance of blitter, useful for improving a custom blitter.
NOTE: This assumes that the process is getting 100% CPU time; you might need to
arrange for this or else the performance will be reported lower than it really is. */

#include "snes_ntsc.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

enum { time_high_res = 0 }; /* change to 1 to time high-res */

enum { in_width   = 256 * 2 };
enum { in_height  = 223 };

enum { out_width  = SNES_NTSC_OUT_WIDTH( in_width ) };
enum { out_height = in_height };

struct data_t
{
	snes_ntsc_t ntsc;
	unsigned short in  [ in_height] [ in_width];
	unsigned short out [out_height] [out_width];
};

static int time_blitter( void );

int main()
{
	struct data_t* data = (struct data_t*) malloc( sizeof *data );
	if ( data )
	{
		clock_t start;
		
		/* fill with random pixel data */
		int y;
		for ( y = 0; y < in_height; y++ )
		{
			int x;
			for ( x = 0; x < in_width; x++ )
				data->in [y] [x] = (rand() >> 4 & 0x1F) * 64;
		}
		
		/* time initialization */
		start = clock();
		for ( y = 10; y--; )
			snes_ntsc_init( &data->ntsc, 0 );
		printf( "Init time: %.2f seconds\n",
				(double) (clock() - start) / (CLOCKS_PER_SEC * 10) );
		
		/* measure frame rate */
		while ( time_blitter() )
		{
			if ( time_high_res )
				snes_ntsc_blit_hires( &data->ntsc, data->in [0], in_width, 0,
						in_width, in_height, data->out [0], sizeof data->out [0] );
			else
				snes_ntsc_blit( &data->ntsc, data->in [0], in_width, 0,
						in_width / 2, in_height, data->out [0], sizeof data->out [0] );
		}
		
		free( data );
	}
	
	return 0;
}

static int time_blitter( void )
{
	int const duration = 4; /* seconds */
	static clock_t end_time;
	static int count;
	if ( !count )
	{
		clock_t time = clock();
		while ( clock() == time ) { }
		if ( clock() - time > CLOCKS_PER_SEC )
		{
			/* clock increments less-often than once every second */
			printf( "Insufficient time resolution\n" );
			return 0;
		}
		end_time = clock() + CLOCKS_PER_SEC * duration;
	}
	else if ( clock() >= end_time )
	{
		int rate = count / duration;
		printf( "Performance: %d frames per second, which would use %d%% CPU at 60 FPS\n",
				rate, 60 * 100 / rate );
		return 0;
	}
	count++;
	
	return 1;
}

