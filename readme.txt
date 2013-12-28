-----------------------------------------------------------------------------
Nestopia 1.05 - NES/Famicom emulator
-----------------------------------------------------------------------------

Nestopia is Copyright 2003 by Martin Freij
under the terms and conditions of the 
GNU General Public License. 
http://www.gnu.org

Homepage: http://nestopia.znes.org/ (currently down)
Project:  http://sourceforge.net/projects/nestopia/
Mail:     martin-freij at home.se

-----------------------------------------------------------------------------
Licence
-----------------------------------------------------------------------------

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your option)
any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
for more details. You should have received a copy of the GNU General
Public License along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

-----------------------------------------------------------------------------
About the Emulator
-----------------------------------------------------------------------------

Written in C++, compiled with Intel Compiler and made to be as accurate as 
possible. All The different components are synchronized "as needed" on a 
clock cycle level, basically meaning that the PPU and pAPU update themselves 
only if an instruction is going to change the way the PPU render pixels or 
the pAPU generates sound or else they simply just wait until the end of the 
frame and then carry on. They all use the "master clock" as a cycle counter 
base which eliminates the need for decimals alltogether. With this method 
many games that rely on perfect timing work flawlessly on Nestopia (Mach Rider, 
Slalom, Marble Maddness etc). However, in comparision to other emulators which 
are either scanline or tile based this is slower but if you're running it on.. 
say a P4 or at least a very fast P3 there shouldn't be any notable speed loss. 

-----------------------------------------------------------------------------
About the Source Code
-----------------------------------------------------------------------------

Altough I've had portability in mind and done my best to make it more or less 
platform independent there may still be some things in the core which may 
prevent it from working on anything other than IA32. Also, since the code is 
written in C++ and relies heavily on templates with all its bells and whistles 
compilers which doesn't conform close enough to the current ANSI standard may 
fail. If you're interested in porting Nestopia to another platform feel free 
to contact me and I'll try to help out.

-----------------------------------------------------------------------------
System Requirements
-----------------------------------------------------------------------------

Minimum:
--------
	
Processor: Pentium MMX or comparable AMD
Ram:       32MB
Video:     DirectDraw 7 compatible graphic card
OS:        Windows 98/Me/NT/2000/XP or greater
Software:  DirectX 8.1 or greater

Recommended:
------------

Processor: Pentium 4 or comparable AMD
Ram:       128MB
Video:     Direct3D compatible graphic card
Audio:     DirectSound 8.1 compatible sound card
OS:        Windows XP or greater
Software:  DirectX 9 or greater

Note: If you have GdiPlus from Microsoft installed (comes with XP) you'll be 
      able to save screenshots in png, jpg, bmp and tif format.

-----------------------------------------------------------------------------
Supported File Types
-----------------------------------------------------------------------------

NES  - NES Rom Image. Since this fileformat is somewhat lacking in cartridge 
       info plus many file headers floating around on the net are broken for 
       some reason or another (think dudes with disks) Nestopia uses a rom 
       database internally and will try to identify and repair them temporarly 
       in memory using some CRC32 magic.

UNIF - NES Rom Image. If Nestopia can't recognize the specified board name you 
       have the option to select the corresponding mapper the cartridge should 
       use. IPS patching is not supported for this format.

NSP  - Nestopia Game Configuration File. It's text based and easy to edit by 
       hand. The available commands are:
        
        -ROM or -IMAGE <file>
        -SAV or -SAVE  <file>
        -STATE <file>
        -MOVIE <file>
        -STATESLOT1..STATESLOT9 <file>
        -IPS <file>
        -PALETTE <file>
        -MODE <ntsc/pal>
        -PORT1..PORT5 <unconnected,pad1,pad2,pad3,pad4,zapper,paddle,powerpad,
                       keyboard>
        -GENIE or -CHEAT <code> <comment> (last is optional)

       Note, the brackets <> should not be included.

       Example - c:\game.nsp:
       
        -IMAGE mammamia.nes // C style comments are supported
        -MODE ntsc
        -PORT1 pad1
        -PORT2 unconnected
        -CHEAT SXIOPO infinite plumbers

       This will load up the mammamia.nes file and force it into NTSC mode using
       one standard controller plugged into the first port and enable one game-
       genie code. These files can both be directly or indirectly loaded. You can 
       also at any time have your current game configuration saved into this 
       format.

NSF  - Nes Sound File. Scroll down for info on supported sound chips.

FDS  - Famicom Disk System. Bios rom file required which can be both in iNES 
       or raw format. Nestopia tries to locate it by searching for "disksys.rom".
       If it can't find it you can manually select the proper file in the FDS 
       options in the menu. There's also an option available which lets you 
       protect the disk files from being written back to. 

SAV  - Battery-ram data separated from the rom files. If the cartridge has it 
       Nestopia automatically searches for a valid save file with the same name 
       as the rom image file to use during load. Just like the FDS files these 
       can also be write protected.

IPS  - Patch file. Patching of NES and FDS files can be applied both manually 
       and automatically. If you select "Auto Apply" in the path configuration 
       dialog Nestopia will search for an IPS file with the same name as the 
       image file during load and patch it on-the-fly. Keep in mind that the 
       only files which will be permanently changed (if you've told Nestopia to 
       do so) are those of the FDS format since they are.. well, disks, and
       gets written to all the time.

ZIP  - If Nestopia finds more than one valid file in the archive you may choose 
       which one you want to open. ZIP files can't be written back to though, 
       instead you'll be prompted if you want to save any changed data outside 
       the archive.

NST  - Nestopia Save State File. Aside from loading and saving via files there 
       are also nine slots accessable by the numerical keys. Remember to hold in 
       SHIFT if you want to load a state. Push '0' to save into the next slot in 
       incrementing order and Shift + '0' to load from the last saved slot. You
       can also assign the afformentioned commands to any other keys.

NSV  - Nestopia Movie File. Available commands are located in the menu. One note,
       if you don't rewind the tape the next time you record anything (assuming 
       you're in the same game) it will continue off from the last saved 
       position.

-----------------------------------------------------------------------------
Fully or Partially Supported Mappers
-----------------------------------------------------------------------------

1,2,3,4,5,6,7,8,9,10,11,13,15,16,17,18,19,21,22,23,24,25,26,32,33,
34,40,41,42,43,44,45,46,47,48,49,50,51,52,57,58,60,61,62,64,65,66,
67,68,69,70,71,72,73,74,75,76,77,78,79,80,82,83,85,86,87,88,89,90,
91,92,93,94,95,96,97,99,100,101,105,107,112,113,114,115,117,118,119,
122,133,134,135,140,144,151,152,153,154,155,156,157,160,180,181,182,
183,184,185,187,188,189,198,222,225,226,227,228,229,230,231,232,233,
234,235,236,237,240,241,242,243,244,245,246,248,249,250,255 = a total of 137

-----------------------------------------------------------------------------
Supported Sound Chips
-----------------------------------------------------------------------------

Famicom Disk System
Konami VRC6
Namcot 106
Nintendo MMC5
Sunsoft FME-07

-----------------------------------------------------------------------------
Supported Controllers
-----------------------------------------------------------------------------

Standard Controller (up to four players)
Zapper
Power Pad
Arkanoid Paddle
Family Basic Keyboard

-----------------------------------------------------------------------------
Exclusively Supported VS-Unisystem Games (only .nes)
-----------------------------------------------------------------------------

Battle City
Castlevania
Clu Clu Land
Dr. Mario
Excitebike
Freedom Force
Golf
Goonies
Gradius
Hogan's Alley
Ice Climber
Lady Golf
Mach Rider
Ninja Jajamaru Kun
Pinball
Platoon
RBI Baseball
Sky Kid
Slalom
Soccer
Star Luster
Super Mario Bros
Tengen Tetris
TKO Boxing
Top Gun

-----------------------------------------------------------------------------
Performance issue
-----------------------------------------------------------------------------

As stated earlier, Nestopia can be slow on low-end computers so here are some
suggestions on what you can do to speed things up.

- Enable auto frame-skip and uncheck the vsync button.

- Keep the menu hidden when you're in fullscreen mode. Having it visible 
  forces Nestopia to co-operate with GDI which adds extra overhead.

- Set the display mode to 320*240 8 bit.

- Disable graphic filtering.

- Off-screen surfaces in video memory is usually faster than having them in 
  system memory but it may produce undesired image filtering when a surface 
  blit require the image to be scaled. There's no way to control this under 
  DirectDraw 7 so the default setting for Nestopia is system memory, but you 
  can try video memory and see if it helps.

- Enable "high priority" in the preferences dialog.

- Don't enable the support for more than eight simultaneous sprites visible at 
  the screen. It probably won't make any difference though but I thought I'd 
  mention it since it may require the PPU to do some extra work.

- Raise the sound volume to max. Using a different volume forces DirectSound to 
  perform a few more calculations for each sample it process.

- Try a lower sample rate and bit length, 22.05 kHz and 8 bit may be a good 
  choice here. If you're desperate, turn off sound completely.

-----------------------------------------------------------------------------
Credits and Thanks
-----------------------------------------------------------------------------

Brad Taylor               - PPU and pAPU technical documents
Chris Covell              - demos, test roms and general info
Derek Liauw Kie Fa.       - 2xSaI engine
Digitoxin                 - Nestoy rom database which is the one used 
                            internally by Nestopia
FireBug                   - mapper documents
Gilles Vollant            - ZIP file loading routines 
Goroh                     - technical documents on various hardware 
                            components
Groggy                    - testing and ideas
Jean-loup Gailly          - ZLib compression library
Jeremy Chadwick           - NES technical document
Ki                        - pAPU technical info
Kevin Horton              - NTSC palette emulation info, sound/mapper 
                            technical  documents and some 6502 tidbits
Loopy                     - various info
Marat Fayzullin           - NES technical document
Mark Adler                - ZLib compression library
Mark Knibbs               - various info 
Matthew Conte             - pAPU technical info
Matrixz'                  - for his neat palette which is the default one 
                            Nestopia uses internally
Memblers                  - nesdev.parodius.com, simply the best resource 
                            for NES development
Tennessee Carmel-Veilleux - UNIF format inventor
Quietust                  - various info

Also a big thanks to the authors of NNNesterJ and FCEUltra for a good 
reference on the more exotic mappers, you know, those that made you just 
wanna stand up and start breakdancing because they were way too funky. 
Phew, and lastly, credits to the authors of the VS.NES part in MAME for 
all the dipswitch settings and explanations, thanks to them you can now 
control the appearence of that retarded bear in Ice Climber!

<eof>
