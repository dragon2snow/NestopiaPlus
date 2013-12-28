nes_ntsc 0.1.7: NES NTSC Composite Video Emulator
-------------------------------------------------
This is an efficient Nintendo NES NTSC composite video emulator for displaying
an authentic image in an emulator. It uses a highly optimized algorithm to
perform the same signal processing as an NTSC decoder in a TV, giving very
similar pixel artifacts. The usual TV picture controls can be adjusted: hue
(tint), saturation, contrast, brightness, and sharpness.

The output is scaled to the proper horizontal width, leaving it up the emulator
to simply double the height. An optional even/odd field merging feature is
included to help when the host display's refresh rate doesn't match the
emulator's frame rate. Custom blitters can be written using a special
interface, allowing output in other pixel formats and efficient scanline
doubling as a part of blitting.

Blitting a 256x240 NES source image to a 602x240 pixel 16-bit RGB memory buffer
at 60 frames per second uses 8% CPU on a 2.0 GHz Athlon 3500+ and 40% CPU on a
10-year-old 400 MHz G3 PowerMac.

Feedback about the interface and usability would be helpful.

Author  : Shay Green <gblargg@gmail.com>
Website : http://www.slack.net/~ant/
Forum   : http://groups.google.com/group/blargg-sound-libs
License : GNU Lesser General Public License (LGPL)
Language: C or C++


Getting Started
---------------
Build a program consisting of demo.c, nes_ntsc.c, and the SDL multimedia
library (see http://libsdl.org/). Running the program with "nes.raw" in the
same directory should show a NES image as it would look on a TV, with mouse
control of two picture parameters.

A simple benchmark program is included to allow measuring the frame rate and
how much CPU time the blitter would use running at 60 frames per second. This
is useful for getting an idea of whether this library will be usable in your
emulator, and for improving the performance of a custom blitter (see
notes.txt).

If you're using C++, to avoid linking issues rename nes_ntsc.c to nes_ntsc.cpp
(this causes compilation in C++ rather than C).

See notes.txt for more information, and respective header (.h) files for
reference. Post to the discussion forum for assistance.


Files
-----
notes.txt               General notes about the library
changes.txt             Changes made since previous releases
LGPL.txt                GNU Lesser General Public License

demo.c                  Loads raw NES image and displays on screen using SDL
nes.raw                 Raw NES image for demo
benchmark.c             Measures frame rate and CPU usage of blitter

nes_ntsc.h              Library header and source
nes_ntsc.c

tests/                  Test ROMs to verify burst phase operation
