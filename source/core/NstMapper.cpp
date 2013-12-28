////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
//
// Copyright (C) 2003-2005 Martin Freij
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

#include <cstdio>
#include <cstring>
#include "NstLog.hpp"
#include "NstMapper.hpp"
#include "board/NstBrdMmc1.hpp"
#include "board/NstBrdMmc2.hpp"
#include "board/NstBrdMmc3.hpp"
#include "board/NstBrdMmc5.hpp"
#include "board/NstBrdVrc4.hpp"
#include "board/NstBrdVrc6.hpp"
#include "board/NstBrdVrc7.hpp"
#include "board/NstBrdN106.hpp"
#include "board/NstBrdN118.hpp"
#include "board/NstBrdFme7.hpp"
#include "board/NstBrdBandai.hpp"
#include "board/NstBrdTaitoTc.hpp"
#include "board/NstBrdTaitoX.hpp"
#include "board/NstBrdColorDreams.hpp"
#include "board/NstBrdBtlTek2A.hpp"
#include "board/NstBrdFfe.hpp"
#include "board/NstBrdMario1Malee2.hpp"
#include "board/NstBrdSuper24In1.hpp"
#include "board/NstBrdNovelDiamond.hpp"
#include "board/NstBrd8237.hpp"
#include "board/NstBrdWs.hpp"
#include "mapper/NstMapper000.hpp"
#include "mapper/NstMapper001.hpp"
#include "mapper/NstMapper002.hpp"
#include "mapper/NstMapper003.hpp"
#include "mapper/NstMapper004.hpp"
#include "mapper/NstMapper005.hpp"
#include "mapper/NstMapper006.hpp"
#include "mapper/NstMapper007.hpp"
#include "mapper/NstMapper008.hpp"
#include "mapper/NstMapper009.hpp"
#include "mapper/NstMapper010.hpp"
#include "mapper/NstMapper011.hpp"
#include "mapper/NstMapper012.hpp"
#include "mapper/NstMapper013.hpp"
#include "mapper/NstMapper015.hpp"
#include "mapper/NstMapper016.hpp"
#include "mapper/NstMapper017.hpp"
#include "mapper/NstMapper018.hpp"
#include "mapper/NstMapper019.hpp"
#include "mapper/NstMapper021.hpp"
#include "mapper/NstMapper022.hpp"
#include "mapper/NstMapper023.hpp"
#include "mapper/NstMapper024.hpp"
#include "mapper/NstMapper025.hpp"
#include "mapper/NstMapper026.hpp"
#include "mapper/NstMapper032.hpp"
#include "mapper/NstMapper033.hpp"
#include "mapper/NstMapper034.hpp"
#include "mapper/NstMapper040.hpp"
#include "mapper/NstMapper041.hpp"
#include "mapper/NstMapper042.hpp"
#include "mapper/NstMapper044.hpp"
#include "mapper/NstMapper045.hpp"
#include "mapper/NstMapper046.hpp"
#include "mapper/NstMapper047.hpp"
#include "mapper/NstMapper048.hpp"
#include "mapper/NstMapper049.hpp"
#include "mapper/NstMapper050.hpp"
#include "mapper/NstMapper051.hpp"
#include "mapper/NstMapper052.hpp"
#include "mapper/NstMapper053.hpp"
#include "mapper/NstMapper056.hpp"
#include "mapper/NstMapper057.hpp"
#include "mapper/NstMapper058.hpp"
#include "mapper/NstMapper060.hpp"
#include "mapper/NstMapper061.hpp"
#include "mapper/NstMapper062.hpp"
#include "mapper/NstMapper064.hpp"
#include "mapper/NstMapper065.hpp"
#include "mapper/NstMapper066.hpp"
#include "mapper/NstMapper067.hpp"
#include "mapper/NstMapper068.hpp"
#include "mapper/NstMapper069.hpp"
#include "mapper/NstMapper070.hpp"
#include "mapper/NstMapper071.hpp"
#include "mapper/NstMapper072.hpp"
#include "mapper/NstMapper073.hpp"
#include "mapper/NstMapper074.hpp"
#include "mapper/NstMapper075.hpp"
#include "mapper/NstMapper076.hpp"
#include "mapper/NstMapper077.hpp"
#include "mapper/NstMapper078.hpp"
#include "mapper/NstMapper079.hpp"
#include "mapper/NstMapper080.hpp"
#include "mapper/NstMapper082.hpp"
#include "mapper/NstMapper083.hpp"
#include "mapper/NstMapper085.hpp"
#include "mapper/NstMapper086.hpp"
#include "mapper/NstMapper087.hpp"
#include "mapper/NstMapper088.hpp"
#include "mapper/NstMapper089.hpp"
#include "mapper/NstMapper090.hpp"
#include "mapper/NstMapper091.hpp"
#include "mapper/NstMapper092.hpp"
#include "mapper/NstMapper093.hpp"
#include "mapper/NstMapper094.hpp"
#include "mapper/NstMapper095.hpp"
#include "mapper/NstMapper096.hpp"
#include "mapper/NstMapper097.hpp"
#include "mapper/NstMapper099.hpp"
#include "mapper/NstMapper100.hpp"
#include "mapper/NstMapper101.hpp"
#include "mapper/NstMapper105.hpp"
#include "mapper/NstMapper107.hpp"
#include "mapper/NstMapper112.hpp"
#include "mapper/NstMapper113.hpp"
#include "mapper/NstMapper114.hpp"
#include "mapper/NstMapper115.hpp"
#include "mapper/NstMapper117.hpp"
#include "mapper/NstMapper118.hpp"
#include "mapper/NstMapper119.hpp"
#include "mapper/NstMapper133.hpp"
#include "mapper/NstMapper134.hpp"
#include "mapper/NstMapper135.hpp"
#include "mapper/NstMapper140.hpp"
#include "mapper/NstMapper142.hpp"
#include "mapper/NstMapper144.hpp"
#include "mapper/NstMapper151.hpp"
#include "mapper/NstMapper152.hpp"
#include "mapper/NstMapper153.hpp"
#include "mapper/NstMapper154.hpp"
#include "mapper/NstMapper155.hpp"
#include "mapper/NstMapper156.hpp"
#include "mapper/NstMapper157.hpp"
#include "mapper/NstMapper158.hpp"
#include "mapper/NstMapper160.hpp"
#include "mapper/NstMapper164.hpp"
#include "mapper/NstMapper165.hpp"
#include "mapper/NstMapper180.hpp"
#include "mapper/NstMapper181.hpp"
#include "mapper/NstMapper182.hpp"
#include "mapper/NstMapper184.hpp"
#include "mapper/NstMapper185.hpp"
#include "mapper/NstMapper187.hpp"
#include "mapper/NstMapper188.hpp"
#include "mapper/NstMapper189.hpp"
#include "mapper/NstMapper193.hpp"
#include "mapper/NstMapper198.hpp"
#include "mapper/NstMapper200.hpp"
#include "mapper/NstMapper201.hpp"
#include "mapper/NstMapper202.hpp"
#include "mapper/NstMapper203.hpp"
#include "mapper/NstMapper204.hpp"
#include "mapper/NstMapper205.hpp"
#include "mapper/NstMapper206.hpp"
#include "mapper/NstMapper207.hpp"
#include "mapper/NstMapper208.hpp"
#include "mapper/NstMapper209.hpp"
#include "mapper/NstMapper210.hpp"
#include "mapper/NstMapper211.hpp"
#include "mapper/NstMapper212.hpp"
#include "mapper/NstMapper213.hpp"
#include "mapper/NstMapper215.hpp"
#include "mapper/NstMapper217.hpp"
#include "mapper/NstMapper222.hpp"
#include "mapper/NstMapper225.hpp"
#include "mapper/NstMapper226.hpp"
#include "mapper/NstMapper227.hpp"
#include "mapper/NstMapper228.hpp"
#include "mapper/NstMapper229.hpp"
#include "mapper/NstMapper230.hpp"
#include "mapper/NstMapper231.hpp"
#include "mapper/NstMapper232.hpp"
#include "mapper/NstMapper233.hpp"
#include "mapper/NstMapper234.hpp"
#include "mapper/NstMapper235.hpp"
#include "mapper/NstMapper240.hpp"
#include "mapper/NstMapper241.hpp"
#include "mapper/NstMapper242.hpp"
#include "mapper/NstMapper243.hpp"
#include "mapper/NstMapper244.hpp"
#include "mapper/NstMapper245.hpp"
#include "mapper/NstMapper246.hpp"
#include "mapper/NstMapper248.hpp"
#include "mapper/NstMapper249.hpp"
#include "mapper/NstMapper250.hpp"
#include "mapper/NstMapper252.hpp"
#include "mapper/NstMapper254.hpp"
#include "mapper/NstMapper255.hpp"

namespace Nes
{
	namespace Core
	{	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("s", on)
        #endif
	
        #ifdef _MSC_VER
        #pragma pack(push,1)
        #endif

		struct Mapper::Setup
		{
			enum
			{
				PRG_DEFAULT,
				PRG_01XX_16K = PRG_DEFAULT,
				PRG_XX01_16K,
				PRG_0101_16K,
				PRG_XXXX_16K,
				PRG_0XXX_24K,
				PRG_XXXX_32K,
				PRG_0123_32K,
				PRG_012X_8K
			};

			enum
			{
				NMT_DEFAULT,
				NMT_VERTICAL,
				NMT_HORIZONTAL,
				NMT_ZERO
			};

			cstring board;
			uchar prg : 3;
			uchar nmt : 2;
			uchar nmtd : 2;
		};

        #ifdef _MSC_VER
        #pragma pack(pop)
        #endif

		const Mapper::Setup Mapper::setup[256+NUM_EXT_MAPPERS] =
		{
			{ "NROM",				              Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 000
			{ "MMC1",				              Setup::PRG_DEFAULT,  Setup::NMT_ZERO,       Setup::NMT_ZERO       }, // 001
			{ "UNROM",			                  Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 002
			{ "CNROM",			                  Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 003
			{ "MMC3 / MMC6 / MIMIC-1",	          Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 004
			{ "MMC5",				              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 005
			{ "FFE F4xxx",		                  Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 006
			{ "AOROM / AMROM",	                  Setup::PRG_0123_32K, Setup::NMT_ZERO,       Setup::NMT_ZERO       }, // 007
			{ "FFE F3xxx",		                  Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 008
			{ "MMC2",				              Setup::PRG_0XXX_24K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 009
			{ "MMC4",				              Setup::PRG_01XX_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 010
			{ "COLOR DREAMS",                     Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 011
			{ "DBZ5 (MMC3)",                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 012
			{ "CPROM",			                  Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 013
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 014
			{ "BMC CONTRA 100-IN-1",	          Setup::PRG_0123_32K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 015
			{ "BANDAI",			                  Setup::PRG_01XX_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 016
			{ "FFE F8xxx",		                  Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 017
			{ "JALECO SS8806",	                  Setup::PRG_012X_8K,  Setup::NMT_HORIZONTAL, Setup::NMT_HORIZONTAL }, // 018
			{ "NAMCO 106",		                  Setup::PRG_012X_8K,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 019
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 020
			{ "KONAMI VRC4 2A",	                  Setup::PRG_01XX_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 021
			{ "KONAMI VRC2 / VRC4 A",	          Setup::PRG_01XX_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 022
			{ "KONAMI VRC2 / VRC4 B",		      Setup::PRG_01XX_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 023
			{ "KONAMI VRC6 A0/A1",                Setup::PRG_01XX_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 024
			{ "KONAMI VRC4 Y",	                  Setup::PRG_01XX_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 025
			{ "KONAMI VRC6 A1/A0",                Setup::PRG_01XX_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 026
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 027
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 028
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 029
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 030
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 031
			{ "IREM G-101",		                  Setup::PRG_XXXX_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 032
			{ "TAITO TC0190/TC0350",              Setup::PRG_0XXX_24K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 033
			{ "NINA-1 / BNROM",                   Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 034
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 035
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 036
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 037
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 038
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 039
			{ "BTL-SMB2",   	                  Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 040
			{ "CALTRON 6-IN-1",	                  Setup::PRG_0123_32K, Setup::NMT_HORIZONTAL, Setup::NMT_HORIZONTAL }, // 041
			{ "BTL-MARIOBABY",                    Setup::PRG_XXXX_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 042
			{ "",                                 Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 043
			{ "BMC SUPER BIG 7-IN-1", 	          Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 044
			{ "BMC SUPER (1000000/13/7)-IN-1",    Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 045
			{ "GAME-STATION / RUMBLE-STATION",    Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 046
			{ "NES-QJ",        	                  Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 047
			{ "TAITO TC190V",		              Setup::PRG_0XXX_24K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 048
			{ "SUPER HiK 4-IN-1 (MMC3)",          Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 049
			{ "SMB2j REV.A", 	                  Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 050
			{ "BMC BALL GAMES 11-IN-1",           Setup::PRG_0123_32K, Setup::NMT_HORIZONTAL, Setup::NMT_HORIZONTAL }, // 051
			{ "MARIO PARTY 7-IN-1 (MMC3)",        Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 052
			{ "SUPERVISION 16-IN-1",              Setup::PRG_DEFAULT,  Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 053
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 054
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 055
			{ "BTL-SMB3",			              Setup::PRG_012X_8K,  Setup::NMT_HORIZONTAL, Setup::NMT_HORIZONTAL }, // 056
			{ "GAME STAR GK-54",	              Setup::PRG_0101_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 057
			{ "STUDY & GAME 32-IN-1",             Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 058
			{ "BMC T3H53",		                  Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 059
			{ "BMC RESET-TRIGGERED 4-IN-1",       Setup::PRG_DEFAULT,  Setup::NMT_HORIZONTAL, Setup::NMT_HORIZONTAL }, // 060
			{ "BMC 20-IN-1",		              Setup::PRG_01XX_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 061
			{ "BMC 700-IN-1",		              Setup::PRG_0123_32K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 062
			{ "HELLO KITTY 255-IN-1",             Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 063
			{ "TENGEN RAMBO-1",	                  Setup::PRG_DEFAULT,  Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 064
			{ "IREM H-3001",		              Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 065
			{ "GNROM",                            Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 066
			{ "SUNSOFT #3",		                  Setup::PRG_01XX_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 067
			{ "SUNSOFT #4",		                  Setup::PRG_01XX_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 068
			{ "SUNSOFT FME-07",	                  Setup::PRG_01XX_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 069
			{ "BANDAI 74161/32",                  Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 070
			{ "CAMERICA", 	                      Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 071
			{ "JALECO",			                  Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 072
			{ "KONAMI VRC3",		              Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 073
			{ "MMC3 TAIWAN",                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 074
			{ "JALECO SS8805 / KONAMI VRC1",      Setup::PRG_012X_8K,  Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 075
			{ "NAMCO 109",		                  Setup::PRG_01XX_16K, Setup::NMT_HORIZONTAL, Setup::NMT_HORIZONTAL }, // 076
			{ "IREM",				              Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 077
			{ "IREM 74HC161/32",                  Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 078
			{ "NINA-06 / NINA-03",	              Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 079
			{ "TAITO X-005",			          Setup::PRG_012X_8K,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 080
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 081
			{ "TAITO",			                  Setup::PRG_01XX_16K, Setup::NMT_HORIZONTAL, Setup::NMT_HORIZONTAL }, // 082
			{ "BTL-DRAGON BALL PARTY",            Setup::PRG_DEFAULT,  Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 083
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 084
			{ "KONAMI VRC7",		              Setup::PRG_01XX_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 085
			{ "JALECO",			                  Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 086
			{ "KONAMI 74HC161/32",                Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 087
			{ "NAMCO 118",		                  Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 088
			{ "SUNSOFT",			              Setup::PRG_01XX_16K, Setup::NMT_ZERO,       Setup::NMT_ZERO       }, // 089
			{ "BTL-TEK2A",   		              Setup::PRG_DEFAULT,  Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 090
			{ "PC-HK-SF3",					      Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 091
			{ "JALECO (early)",	                  Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 092
			{ "SUNSOFT 74HC161/32",		          Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 093
			{ "CAPCOM 74HC161/32",                Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 094
			{ "DRAGON BUSTER (MMC3)",             Setup::PRG_01XX_16K, Setup::NMT_ZERO,       Setup::NMT_ZERO       }, // 095
			{ "BANDAI 74HC161/32",			      Setup::PRG_0123_32K, Setup::NMT_ZERO,       Setup::NMT_ZERO       }, // 096
			{ "IREM 74HC161/32",		          Setup::PRG_XX01_16K, Setup::NMT_HORIZONTAL, Setup::NMT_HORIZONTAL }, // 097
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 098
			{ "VS SYSTEM 8KB CHR SWITCH",         Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 099			 
			{ "MMC3 HACK",	                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 100			 
			{ "JALECO 74HC161/32",		          Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 101			 
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 102	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 103	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 104	
			{ "NES-EVENT",		                  Setup::PRG_DEFAULT,  Setup::NMT_ZERO,       Setup::NMT_ZERO       }, // 105			 
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 106	
			{ "MAGIC DRAGON",		              Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 107			 
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 108	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 109	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 110	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 111	
			{ "ASDER",			                  Setup::PRG_01XX_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 112			 
			{ "MB-91",			                  Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 113			 
			{ "THE LION KING",	                  Setup::PRG_01XX_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 114			 
			{ "YUU YUU HAKUSHO FINAL",            Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 115			 
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 116	
			{ "PC-FUTURE",		                  Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 117			 
			{ "TLSROM / TKSROM (MMC3)",	          Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 118			 
			{ "TQROM (MMC3)",                     Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 119			 
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 120	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 121	
			{ "",                                 Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 122	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 123	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 124	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 125	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 126	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 127	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 128	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 129	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 130	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 131	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 132	
			{ "SACHEN JOVIAL RACE",	              Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 133			 
			{ "SACHEN-74LS374N",                  Setup::PRG_0123_32K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 134	
			{ "SA-72008",			      	      Setup::PRG_0123_32K, Setup::NMT_ZERO,       Setup::NMT_ZERO       }, // 135	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 136	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 137	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 138	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 139	
			{ "JALECO",			                  Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 140			 
			{ "SACHEN",			                  Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 141	
			{ "BTL KS 202 (SMB2J)",               Setup::PRG_012X_8K,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 142	
			{ "SACHEN",			                  Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 143	
			{ "AGCI 50282",		                  Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 144			 
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 145	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 146	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 147	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 148	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 149	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 150	
			{ "KONAMI VS-SYSTEM",                 Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 151			 
			{ "BANDAI 74161/32 +MIRR",            Setup::PRG_01XX_16K, Setup::NMT_ZERO,       Setup::NMT_ZERO       }, // 152			 
			{ "BANDAI +WRAM",                     Setup::PRG_01XX_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 153			 
			{ "NAMCO 118 +MIRR",	              Setup::PRG_01XX_16K, Setup::NMT_ZERO,       Setup::NMT_ZERO       }, // 154			 
			{ "MMC1 WRAM-ENABLE",   	          Setup::PRG_DEFAULT,  Setup::NMT_ZERO,       Setup::NMT_ZERO       }, // 155			 
			{ "TBA", 			                  Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 156			 
			{ "BANDAI +BARCODE",			      Setup::PRG_01XX_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 157			 
			{ "TENGEN RAMBO1 (alt)",              Setup::PRG_DEFAULT,  Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 158			 
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 159	
			{ "PC-ALADDIN",		                  Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 160			 
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 161	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 162	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 163	
			{ "FINAL FANTASY V",	              Setup::PRG_XXXX_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 164	
			{ "FIRE EMBLEM",		              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 165			 
			{ "",		                          Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 166	
			{ "",		                          Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 167	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 168	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 169	
			{ "",	       			              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 170	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 171	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 172	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 173	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 174	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 175	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 176	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 177	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 178	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 179	
			{ "NICHIBUTSU",	                      Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 180			 
			{ "HACKER INT.TYPE2",	              Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 181			 
			{ "SUPER DONKEY KONG",                Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 182			 
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 183	
			{ "SUNSOFT 74HC161/32",	              Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 184			 
			{ "CROM WRITE-ENABLE",    		      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 185			 
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 186	
			{ "STREET FIGHTER ZERO 2 97",	      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 187			 
			{ "BANDAI KARAOKE STUDIO",            Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 188			 
			{ "SF2 YOKO VERSION",                 Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 189			 
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 190	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 191	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 192	
			{ "MEGA SOFT (NTDEC)",                Setup::PRG_0XXX_24K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 193			 
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 194	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 195	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 196	
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 197	
			{ "DESTINY OF AN EMPEROR",            Setup::PRG_01XX_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 198			 
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 199	
			{ "BMC (1200-IN-1)",		          Setup::PRG_0101_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 200	
			{ "BMC (21-IN-1)",			          Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 201	
			{ "BMC (150-IN-1)",			          Setup::PRG_01XX_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 202	
			{ "BMC (35-IN-1)",			          Setup::PRG_0101_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 203	
			{ "BMC (64-IN-1)",			          Setup::PRG_0101_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 204	
			{ "BMC (15/3-IN-1)",		          Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 205	
			{ "DEIROM",			                  Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 206	
			{ "TAITO X-005 +MIRR",	              Setup::PRG_012X_8K,  Setup::NMT_HORIZONTAL, Setup::NMT_HORIZONTAL }, // 207	
			{ "STREET FIGHTER 4", 	              Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 208	
			{ "BTL-TEK2A EXT.MIRR",               Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 209	
			{ "NAMCO",      		              Setup::PRG_012X_8K,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 210	
			{ "2-IN-1: DKC4 & THE JUNGLE BOOK 2", Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 211
			{ "SUPER HIK 300-IN-1",	              Setup::PRG_XXXX_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 212
			{ "9999999-IN-1",		              Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 213
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 214
			{ "M-E3",				              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 215
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 216
			{ "BMC SPC009",   		              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 217
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 218
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 219
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 220
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 221
			{ "BTL DRAGON NINJA",	              Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 222
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 223
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 224
			{ "BMC (58-IN-1 & 110-IN-1/52)",      Setup::PRG_0123_32K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 225
			{ "BMC (76-IN-1)",                    Setup::PRG_0123_32K, Setup::NMT_HORIZONTAL, Setup::NMT_HORIZONTAL }, // 226
			{ "BMC (1200-IN-1)",                  Setup::PRG_DEFAULT,  Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 227
			{ "ACTION 52",		                  Setup::PRG_0123_32K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 228
			{ "BMC (31-IN-1)",                    Setup::PRG_0123_32K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 229
			{ "BMC (22-IN-1)",                    Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 230
			{ "20-IN-1",			              Setup::PRG_0101_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 231
			{ "BIC-48",			                  Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 232
			{ "BMC (42-IN-1)",                    Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 233
			{ "MAXI-15",			              Setup::PRG_0123_32K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 234
			{ "BMC GOLDEN GAME (150-IN-1)",       Setup::PRG_0123_32K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 235
			{ "",                                 Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 236
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 237
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 238
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 239
			{ "GEN KE LE ZHUAN",	              Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 240
			{ "EDU",				              Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 241
			{ "WAI XING ZHAN SHI",                Setup::PRG_0123_32K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 242
			{ "SACHEN 74LS374N",			      Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 243
			{ "HACKER INT.",		              Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 244
			{ "YONG ZHE DOU E LONG",              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 245
			{ "PHONE SERM BERM",	              Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 246
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 247
			{ "BAO QING TIAN",	                  Setup::PRG_01XX_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 248
			{ "WAIXING",			              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 249
			{ "TIME DIVER AVENGER (MMC3)",        Setup::PRG_DEFAULT,  Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 250
			{ "SUPER 8-IN-1 FIGHTING",            Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 251
			{ "", 				                  Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 252
			{ "",					              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 253
			{ "PIKACHU Y2K",		              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 254
			{ "BMC (110-IN-1)",                   Setup::PRG_0123_32K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 255
			{ "MARIO1MALEE2",                     Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // Ext. 256
			{ "SUPER 24-IN-1",					  Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // Ext. 257
			{ "BMC NOVELDIAMOND (9999999-IN-1)",  Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // Ext. 258
			{ "UNL 8237",                         Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // Ext. 259
			{ "BMC WS",                           Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }  // Ext. 260
		};

		Mapper* Mapper::Create(Context& context)
		{
			Mapper* mapper = NULL;
	
			switch (context.id)
			{
				case   0: mapper = new Mapper0   ( context ); break;
				case   1: mapper = new Mapper1   ( context ); break;
				case   2: mapper = new Mapper2   ( context ); break;
				case   3: mapper = new Mapper3   ( context ); break;
				case   4: mapper = new Mapper4   ( context ); break;
				case   5: mapper = new Mapper5   ( context ); break;
				case   6: mapper = new Mapper6   ( context ); break;
				case   7: mapper = new Mapper7   ( context ); break;
				case   8: mapper = new Mapper8   ( context ); break;
				case   9: mapper = new Mapper9   ( context ); break;
				case  10: mapper = new Mapper10  ( context ); break;
				case  11: mapper = new Mapper11  ( context ); break;
				case  12: mapper = new Mapper12  ( context ); break;
				case  13: mapper = new Mapper13  ( context ); break;
				case  15: mapper = new Mapper15  ( context ); break;
				case  16: mapper = new Mapper16  ( context ); break;
				case  17: mapper = new Mapper17  ( context ); break;
				case  18: mapper = new Mapper18  ( context ); break;
				case  19: mapper = new Mapper19  ( context ); break;
				case  21: mapper = new Mapper21  ( context ); break;
				case  22: mapper = new Mapper22  ( context ); break;
				case  23: mapper = new Mapper23  ( context ); break;
				case  24: mapper = new Mapper24  ( context ); break;
				case  25: mapper = new Mapper25  ( context ); break;
				case  26: mapper = new Mapper26  ( context ); break;
				case  32: mapper = new Mapper32  ( context ); break;
				case  33: mapper = new Mapper33  ( context ); break;
				case  34: mapper = new Mapper34  ( context ); break;
				case  40: mapper = new Mapper40  ( context ); break;
				case  41: mapper = new Mapper41  ( context ); break;
				case  42: mapper = new Mapper42  ( context ); break;
				case  44: mapper = new Mapper44  ( context ); break;
				case  45: mapper = new Mapper45  ( context ); break;
				case  46: mapper = new Mapper46  ( context ); break;
				case  47: mapper = new Mapper47  ( context ); break;
				case  48: mapper = new Mapper48  ( context ); break;
				case  49: mapper = new Mapper49  ( context ); break;
				case  50: mapper = new Mapper50  ( context ); break;
				case  51: mapper = new Mapper51  ( context ); break;
				case  52: mapper = new Mapper52  ( context ); break;
				case  53: mapper = new Mapper53  ( context ); break;
				case  56: mapper = new Mapper56  ( context ); break;
				case  57: mapper = new Mapper57  ( context ); break;
				case  58: mapper = new Mapper58  ( context ); break;
				case  60: mapper = new Mapper60  ( context ); break;
				case  61: mapper = new Mapper61  ( context ); break;
				case  62: mapper = new Mapper62  ( context ); break;
				case  64: mapper = new Mapper64  ( context ); break;
				case  65: mapper = new Mapper65  ( context ); break;
				case  66: mapper = new Mapper66  ( context ); break;
				case  67: mapper = new Mapper67  ( context ); break;
				case  68: mapper = new Mapper68  ( context ); break;
				case  69: mapper = new Mapper69  ( context ); break;
				case  70: mapper = new Mapper70  ( context ); break;
				case  71: mapper = new Mapper71  ( context ); break;
				case  72: mapper = new Mapper72  ( context ); break;
				case  73: mapper = new Mapper73  ( context ); break;
				case  74: mapper = new Mapper74  ( context ); break;
				case  75: mapper = new Mapper75  ( context ); break;
				case  76: mapper = new Mapper76  ( context ); break;
				case  77: mapper = new Mapper77  ( context ); break;
				case  78: mapper = new Mapper78  ( context ); break;
				case  79: mapper = new Mapper79  ( context ); break;
				case  80: mapper = new Mapper80  ( context ); break;
				case  82: mapper = new Mapper82  ( context ); break;
				case  83: mapper = new Mapper83  ( context ); break;
				case  85: mapper = new Mapper85  ( context ); break;
				case  86: mapper = new Mapper86  ( context ); break;
				case  87: mapper = new Mapper87  ( context ); break;
				case  88: mapper = new Mapper88  ( context ); break;
				case  89: mapper = new Mapper89  ( context ); break;
				case  90: mapper = new Mapper90  ( context ); break;
				case  91: mapper = new Mapper91  ( context ); break;
				case  92: mapper = new Mapper92  ( context ); break;
				case  93: mapper = new Mapper93  ( context ); break;
				case  94: mapper = new Mapper94  ( context ); break;
				case  95: mapper = new Mapper95  ( context ); break;
				case  96: mapper = new Mapper96  ( context ); break;
				case  97: mapper = new Mapper97  ( context ); break;
				case  99: mapper = new Mapper99  ( context ); break;
				case 100: mapper = new Mapper100 ( context ); break;
				case 101: mapper = new Mapper101 ( context ); break;
				case 105: mapper = new Mapper105 ( context ); break;
				case 107: mapper = new Mapper107 ( context ); break;
				case 112: mapper = new Mapper112 ( context ); break;
				case 113: mapper = new Mapper113 ( context ); break;
				case 114: mapper = new Mapper114 ( context ); break;
				case 115: mapper = new Mapper115 ( context ); break;
				case 117: mapper = new Mapper117 ( context ); break;
				case 118: mapper = new Mapper118 ( context ); break;
				case 119: mapper = new Mapper119 ( context ); break;
				case 133: mapper = new Mapper133 ( context ); break;
				case 134: mapper = new Mapper134 ( context ); break;
				case 135: mapper = new Mapper135 ( context ); break;
				case 140: mapper = new Mapper140 ( context ); break;
				case 142: mapper = new Mapper142 ( context ); break;
				case 144: mapper = new Mapper144 ( context ); break;
				case 151: mapper = new Mapper151 ( context ); break;
				case 152: mapper = new Mapper152 ( context ); break;
				case 153: mapper = new Mapper153 ( context ); break;
				case 154: mapper = new Mapper154 ( context ); break;
				case 155: mapper = new Mapper155 ( context ); break;
				case 156: mapper = new Mapper156 ( context ); break;
				case 157: mapper = new Mapper157 ( context ); break;
				case 158: mapper = new Mapper158 ( context ); break;
				case 160: mapper = new Mapper160 ( context ); break;
				case 164: mapper = new Mapper164 ( context ); break;
				case 165: mapper = new Mapper165 ( context ); break;
				case 180: mapper = new Mapper180 ( context ); break;
				case 181: mapper = new Mapper181 ( context ); break;
				case 182: mapper = new Mapper182 ( context ); break;
				case 184: mapper = new Mapper184 ( context ); break;
				case 185: mapper = new Mapper185 ( context ); break;
				case 187: mapper = new Mapper187 ( context ); break;
				case 188: mapper = new Mapper188 ( context ); break;
				case 189: mapper = new Mapper189 ( context ); break;
				case 193: mapper = new Mapper193 ( context ); break;
				case 198: mapper = new Mapper198 ( context ); break;
				case 200: mapper = new Mapper200 ( context ); break;
				case 201: mapper = new Mapper201 ( context ); break;
				case 202: mapper = new Mapper202 ( context ); break;
				case 203: mapper = new Mapper203 ( context ); break;
				case 204: mapper = new Mapper204 ( context ); break;
				case 205: mapper = new Mapper205 ( context ); break;
				case 206: mapper = new Mapper206 ( context ); break;
				case 207: mapper = new Mapper207 ( context ); break;
				case 208: mapper = new Mapper208 ( context ); break;
				case 209: mapper = new Mapper209 ( context ); break;
				case 210: mapper = new Mapper210 ( context ); break;
				case 211: mapper = new Mapper211 ( context ); break;
				case 212: mapper = new Mapper212 ( context ); break;
				case 213: mapper = new Mapper213 ( context ); break;
				case 215: mapper = new Mapper215 ( context ); break;
				case 217: mapper = new Mapper217 ( context ); break;
				case 222: mapper = new Mapper222 ( context ); break;
				case 225: mapper = new Mapper225 ( context ); break;
				case 226: mapper = new Mapper226 ( context ); break;
				case 227: mapper = new Mapper227 ( context ); break;
				case 228: mapper = new Mapper228 ( context ); break;
				case 229: mapper = new Mapper229 ( context ); break;
				case 230: mapper = new Mapper230 ( context ); break;
				case 231: mapper = new Mapper231 ( context ); break;
				case 232: mapper = new Mapper232 ( context ); break;
				case 233: mapper = new Mapper233 ( context ); break;
				case 234: mapper = new Mapper234 ( context ); break;
				case 235: mapper = new Mapper235 ( context ); break;
				case 240: mapper = new Mapper240 ( context ); break;
				case 241: mapper = new Mapper241 ( context ); break;
				case 242: mapper = new Mapper242 ( context ); break;
				case 243: mapper = new Mapper243 ( context ); break;
				case 244: mapper = new Mapper244 ( context ); break;
				case 245: mapper = new Mapper245 ( context ); break;
				case 246: mapper = new Mapper246 ( context ); break;
				case 248: mapper = new Mapper248 ( context ); break;
				case 249: mapper = new Mapper249 ( context ); break;
				case 250: mapper = new Mapper250 ( context ); break;
				case 252: mapper = new Mapper252 ( context ); break;
				case 254: mapper = new Mapper254 ( context ); break;
				case 255: mapper = new Mapper255 ( context ); break;
			
				case EXT_SUPER24IN1:   mapper = new Boards::Super24In1   ( context ); break;
				case EXT_MARIO1MALEE2: mapper = new Boards::Mario1Malee2 ( context ); break;
				case EXT_NOVELDIAMOND: mapper = new Boards::NovelDiamond ( context ); break;
				case EXT_8237:         mapper = new Boards::Unl8237      ( context ); break;
				case EXT_WS:           mapper = new Boards::Ws           ( context ); break;
			
				default: throw RESULT_ERR_UNSUPPORTED_MAPPER;
			}
	
			NST_ASSERT( mapper );
	
			return mapper;
		}
	
		void Mapper::Destroy(Mapper*& mapper)
		{
			delete mapper;
			mapper = NULL;
		}

		Mapper::Mapper(Context& context,const uint settings)
		:
		prg       (context.pRom.Mem(),context.pRom.Size(),true,false),
		cpu       (context.cpu),
		ppu       (context.ppu),
		chr       (context.ppu.GetChrMem()),
		nmt       (context.ppu.GetNmtMem()),
		mirroring (context.mirroring),
		id        (context.id)
		{
			const bool cRomDiscarded = (settings & CROM_NONE) && context.cRom.Size();
								
			if (cRomDiscarded)
				context.cRom.Destroy();
			
			dword cRamSize = (settings & CRAM_SIZES) << 2;
	
			if (context.cRom.Size())
			{
				chr.Source(0).Set( context.cRom.Mem(), context.cRom.Size(), true, false );
				chr.Source(1).Set( context.cRom.Mem(), context.cRom.Size(), true, false );
			}
			else if (cRamSize < NES_8K)
			{
				cRamSize = NES_8K;
			}
	
			if (cRamSize)
			{
				chr.Source( context.cRom.Size() != 0 ).Set( cRamSize, true, true );
	
				if (context.cRom.Empty())
					chr.Source(1).Set( chr.Source().Mem(), chr.Source().Size(), true, true );
			}

			if (mirroring == Ppu::NMT_FOURSCREEN)
				nmt.Source().Set( NES_4K, true, true );
			
			nmt.Source(1).Set( chr.Source().Mem(), chr.Source().Size(), true, context.cRom.Size() == 0 );

			NST_ASSERT( context.wRam.Size() % NES_8K == 0 );

			context.wRamAuto = false;

			switch (settings & WRAM_SETTINGS)
			{
				case WRAM_AUTO:
			
					context.wRamAuto = context.wRam.Empty();
			
					if (context.wRam.Empty() || (!context.battery && context.wRam.Size() != NES_8K))
					{
						context.wRam.Set( NES_8K );
			
						if (context.wRamAuto)
							std::memset( context.wRam.Mem(), 0x00, NES_8K );
					}					
					break;
			
				case WRAM_NONE:
			
					context.wRam.Destroy();
					break;
			
				default:
				{
					const dword size = (settings & WRAM_SIZES) * NES_8K;
					const dword curr = context.wRam.Size(); 
			
					if (curr < size)
					{
						// WRAM size not big enough. Enlarge it.
						context.wRam.Set( size );
						std::memset( context.wRam.Mem(curr), 0x00, size - curr );
					}
					else if (curr > size && ((settings & WRAM_RESTRICT) || !context.battery))
					{
						// WRAM size bigger than it need to be. Shrink it.
						context.wRam.Set( size );
					}
					break;
				}
			}

			wrk.Source(0).Set
			( 
		     	context.wRam.Size() ? context.wRam.Mem() : context.pRom.Mem(), 
				context.wRam.Size() ? context.wRam.Size() : context.pRom.Size(),
				true,
				context.wRam.Size() != 0
			);

			wrk.Source(1).Set( prg.Source().Mem(), prg.Source().Size(), true, false );
			prg.Source(1).Set( wrk.Source().Mem(), wrk.Source().Size(), true, context.wRam.Size() != 0 );

			Log log;

			char buffer[16];
			cstring title = buffer;

			if (id <= 255)
				std::sprintf( buffer, "Mapper %u: ", id );
			else
				title = "Mapper: ";
	
			if (*GetBoard( id ))
				log << title << GetBoard( id ) << NST_LINEBREAK;
			
			log << title << (prg.Source().Size() / NES_1K) << "k PRG-ROM" NST_LINEBREAK
				<< title << (chr.Source().Size() / NES_1K) << (context.cRom.Size() ? "k CHR-ROM" NST_LINEBREAK : "k CHR-RAM" NST_LINEBREAK);
	
			if (cRomDiscarded)
				log << title << "warning, CHR-ROM discarded!" NST_LINEBREAK;
	
			if (chr.Source(1).Internal())
				log << title << (chr.Source(1).Size() / NES_1K) << "k CHR-RAM" NST_LINEBREAK; 
	
			if (context.wRam.Size())
				log << title << (context.wRam.Size() / NES_1K) << ((settings & WRAM_SIZES) ? "k WRAM" NST_LINEBREAK : "k auto WRAM" NST_LINEBREAK);
	
			cstring type;
	
			switch (mirroring)
			{
				case Ppu::NMT_HORIZONTAL: type = "horizontal";        break;
				case Ppu::NMT_VERTICAL:   type = "vertical";          break;
				case Ppu::NMT_FOURSCREEN: type = "four-screen";       break;
				case Ppu::NMT_ZERO:       type = "$2000";             break;
				case Ppu::NMT_ONE:        type = "$2400";             break;
				case Ppu::NMT_CONTROLLED: type = "mapper controlled"; break;
				default:                  type = "unknown";           break;
			}
	
			log << title << type << " screen mirroring" NST_LINEBREAK;
		}
	
		Mapper::~Mapper()
		{
			for (uint i=0; i < Chr::NUM_SOURCES; ++i)
				chr.Source(i).Remove();
	
			for (uint i=0; i < Nmt::NUM_SOURCES; ++i)
				nmt.Source(i).Remove();
		}
	
		void Mapper::Reset(const bool hard)
		{
			cpu.Map( 0x4018U, 0x5FFFU ).Set( this, &Mapper::Peek_Nop, &Mapper::Poke_Nop );

			cpu.Map( 0x6000U, 0x7FFFU ).Set
			( 
		     	this, 
				wrk.HasRam() ? &Mapper::Peek_Wrk_6 : &Mapper::Peek_Nop, 
				wrk.HasRam() ? &Mapper::Poke_Wrk_6 : &Mapper::Poke_Nop
			);

			cpu.Map( 0x8000U, 0x9FFFU ).Set( this, &Mapper::Peek_Prg_8, &Mapper::Poke_Nop );
			cpu.Map( 0xA000U, 0xBFFFU ).Set( this, &Mapper::Peek_Prg_A, &Mapper::Poke_Nop );
			cpu.Map( 0xC000U, 0xDFFFU ).Set( this, &Mapper::Peek_Prg_C, &Mapper::Poke_Nop );
			cpu.Map( 0xE000U, 0xFFFFU ).Set( this, &Mapper::Peek_Prg_E, &Mapper::Poke_Nop );
	
			cpu.ClearIRQ();

			if (hard)
			{
				NST_ASSERT( id < NST_COUNT(setup) );

				if (chr.Source(0).Internal() || chr.Source(1).Internal())
					chr.Source( !chr.Source(0).Internal() ).Clear();

				{
					static const uint defPrg[8][4] =
					{
						{  0U,  1U, ~1U, ~0U },	// PRG_01XX_16K
						{ ~1U, ~0U,  0U,  1U },	// PRG_XX01_16K
						{  0U,  1U,  0U,  1U },	// PRG_0101_16K
						{ ~1U, ~0U, ~1U, ~0U },	// PRG_XXXX_16K
						{  0U, ~2U, ~1U, ~0U },	// PRG_0XXX_24K
						{ ~3U, ~2U, ~1U, ~0U },	// PRG_XXXX_32K
						{  0U,  1U,  2U,  3U },	// PRG_0123_32K
						{  0U,  1U,  2U, ~0U }	// PRG_012X_8K
					};										 

					prg.SwapBanks<NES_8K,0x0000U>
					( 
						defPrg[setup[id].prg][0],
						defPrg[setup[id].prg][1],
						defPrg[setup[id].prg][2],
						defPrg[setup[id].prg][3]
					);
				}

				{
					static const uchar defNmt[3] =
					{
						Ppu::NMT_VERTICAL,
						Ppu::NMT_HORIZONTAL,
						Ppu::NMT_ZERO
					};

					ppu.SetMirroring
					( 
				     	setup[id].nmt                    ? defNmt[setup[id].nmt-1] : 
				    	mirroring == Ppu::NMT_CONTROLLED ? defNmt[setup[id].nmtd-1] : 
					                                       mirroring 
					);
				}

				chr.SwapBank<NES_8K,0x0000U>(0);
				wrk.SwapBank<NES_8K,0x0000U>(0);
			}

			SubReset( hard );
		}
	
		void Mapper::Map(WRamMapping mapping) const
		{
			switch (mapping)
			{
				case WRK_PEEK:          Map( 0x6000U, 0x7FFFU, &Mapper::Peek_Wrk_6 ); break;
				case WRK_POKE:          Map( 0x6000U, 0x7FFFU, &Mapper::Poke_Wrk_6 ); break;
				case WRK_PEEK_POKE:     Map( 0x6000U, 0x7FFFU, &Mapper::Peek_Wrk_6, &Mapper::Poke_Wrk_6 ); break;
				case WRK_POKE_BUS:      Map( 0x6000U, 0x7FFFU, &Mapper::Poke_Wrk_Bus_6 ); break;
				case WRK_PEEK_BUS:      Map( 0x6000U, 0x7FFFU, &Mapper::Peek_Wrk_Bus_6 ); break;
				case WRK_PEEK_POKE_BUS: Map( 0x6000U, 0x7FFFU, &Mapper::Peek_Wrk_Bus_6, &Mapper::Poke_Wrk_Bus_6 ); break;
			}
		}

		dword Mapper::GetStateName() const
		{
			NST_COMPILE_ASSERT( NUM_EXT_MAPPERS == 5 );

			if (id <= 255)
			{
				return NES_STATE_CHUNK_ID('0' + (id / 100),'0' + (id % 100 / 10),'0' + (id % 10),'\0');
			}
			else switch (id)
			{
				case EXT_MARIO1MALEE2: return NES_STATE_CHUNK_ID('M','M','2','\0');
				case EXT_SUPER24IN1:   return NES_STATE_CHUNK_ID('S','2','4','\0');
				case EXT_NOVELDIAMOND: return NES_STATE_CHUNK_ID('N','O','V','\0');
				case EXT_8237:         return NES_STATE_CHUNK_ID('8','2','3','\0');
				case EXT_WS:           return NES_STATE_CHUNK_ID('W','S','4','\0');
			}
			
			return NES_STATE_CHUNK_ID('X','X','X','\0');
		}
	
		void Mapper::SaveState(State::Saver& state) const
		{
			prg.SaveState( State::Saver::Subset(state,'P','R','G','\0').Ref(), b00 );
			chr.SaveState( State::Saver::Subset(state,'C','H','R','\0').Ref(), chr.Source(0).Internal() ? b01 : chr.Source(1).Internal() ? b10 : b00 );
			nmt.SaveState( State::Saver::Subset(state,'N','M','T','\0').Ref(), nmt.Source(0).Internal() ? b01 : nmt.Source(1).Internal() ? b10 : b00 );
			wrk.SaveState( State::Saver::Subset(state,'W','R','K','\0').Ref(), wrk.HasRam() ? b01 : b00 );

			switch (id)
			{
     			case MMC1_00:
       			case MMC1_01:
				case MMC1_02:

					static_cast<const Boards::Mmc1*>(this)->SaveState( State::Saver::Subset(state,'M','M','1','\0').Ref() );
					break;

				case MMC2_00:
				case MMC2_01:

					static_cast<const Boards::Mmc2*>(this)->SaveState( State::Saver::Subset(state,'M','M','2','\0').Ref() );
					break;

				case MMC3_00:
				case MMC3_01:
				case MMC3_02:
				case MMC3_03:
				case MMC3_04:
				case MMC3_05:
				case MMC3_06:
				case MMC3_07:
				case MMC3_08:
				case MMC3_09:
				case MMC3_10:
				case MMC3_11:
				case MMC3_12:
				case MMC3_13:
				case MMC3_14:
				case MMC3_15:
				case MMC3_16:
				case MMC3_17:
				case MMC3_18:
				case MMC3_19:
				case MMC3_20:
				case MMC3_21:
				case MMC3_22:
				case MMC3_23:
				case MMC3_24:

					static_cast<const Boards::Mmc3*>(this)->SaveState( State::Saver::Subset(state,'M','M','3','\0').Ref() );
					break;

				case MMC5_00:

					static_cast<const Boards::Mmc5*>(this)->SaveState( State::Saver::Subset(state,'M','M','5','\0').Ref() );
					break;

				case N106_00:
				case N106_01:

					static_cast<const Boards::N106*>(this)->SaveState( State::Saver::Subset(state,'N','M','6','\0').Ref() );
					break;

				case N118_00:
				case N118_01:

					static_cast<const Boards::N118*>(this)->SaveState( State::Saver::Subset(state,'N','M','8','\0').Ref() );
					break;

				case VRC6_00:
				case VRC6_01:

					static_cast<const Boards::Vrc6*>(this)->SaveState( State::Saver::Subset(state,'V','R','6','\0').Ref() );
					break;

				case VRC7_00:

					static_cast<const Boards::Vrc7*>(this)->SaveState( State::Saver::Subset(state,'V','R','7','\0').Ref() );
					break;

				case BANDAI_00:
				case BANDAI_01:
				case BANDAI_02:

					static_cast<const Boards::Bandai*>(this)->SaveState( State::Saver::Subset(state,'B','A','N','\0').Ref() );
					break;

				case FME07_00:

					static_cast<const Boards::Fme07*>(this)->SaveState( State::Saver::Subset(state,'F','M','7','\0').Ref() );
					break;

				case TAITOTC_00:
				case TAITOTC_01:

					static_cast<const Boards::TaitoTc*>(this)->SaveState( State::Saver::Subset(state,'T','T','C','\0').Ref() );
					break;

				case FFE_00:
				case FFE_01:
				case FFE_02:

					static_cast<const Boards::Ffe*>(this)->SaveState( State::Saver::Subset(state,'F','F','E','\0').Ref() );
					break;

				case BTLTEK2A_00:
				case BTLTEK2A_01:

					static_cast<const Boards::BtlTek2A*>(this)->SaveState( State::Saver::Subset(state,'B','T','K','\0').Ref() );
					break;
			}

			SubSave( State::Saver::Subset(state,GetStateName()).Ref() );
		}
	
		void Mapper::LoadState(State::Loader& state)
		{
			const dword name = GetStateName();
	
			while (const dword chunk = state.Begin())
			{
				if (chunk == name)
				{
					SubLoad( State::Loader::Subset(state).Ref() );
				}
				else switch (chunk)
				{
					case NES_STATE_CHUNK_ID('P','R','G','\0'):
	
						prg.LoadState( State::Loader::Subset(state).Ref(), b00 );
						break;
	
					case NES_STATE_CHUNK_ID('C','H','R','\0'):
	
						chr.LoadState( State::Loader::Subset(state).Ref(), chr.Source(0).Internal() ? b01 : chr.Source(1).Internal() ? b10 : b00 );
						break;
	
					case NES_STATE_CHUNK_ID('N','M','T','\0'):
	
						nmt.LoadState( State::Loader::Subset(state).Ref(), nmt.Source(0).Internal() ? b01 : nmt.Source(1).Internal() ? b10 : b00 );
						break;

					case NES_STATE_CHUNK_ID('W','R','K','\0'):

						wrk.LoadState( State::Loader::Subset(state).Ref(), wrk.HasRam() ? b01 : b00 );
						break;

					case NES_STATE_CHUNK_ID('M','M','1','\0'):

						switch (id)
						{
							case MMC1_00:
							case MMC1_01:
							case MMC1_02:

								static_cast<Boards::Mmc1*>(this)->LoadState( State::Loader::Subset(state).Ref() );
								break;
						}
						break;

					case NES_STATE_CHUNK_ID('M','M','2','\0'):

						switch (id)
						{
							case MMC2_00:
							case MMC2_01:

								static_cast<Boards::Mmc2*>(this)->LoadState( State::Loader::Subset(state).Ref() );
								break;
						}
						break;

					case NES_STATE_CHUNK_ID('M','M','3','\0'):

						switch (id)
						{
							case MMC3_00:
							case MMC3_01:
							case MMC3_02:
							case MMC3_03:
							case MMC3_04:
							case MMC3_05:
							case MMC3_06:
							case MMC3_07:
							case MMC3_08:
							case MMC3_09:
							case MMC3_10:
							case MMC3_11:
							case MMC3_12:
							case MMC3_13:
							case MMC3_14:
							case MMC3_15:
							case MMC3_16:
							case MMC3_17:
							case MMC3_18:
							case MMC3_19:
							case MMC3_20:
							case MMC3_21:
							case MMC3_22:
							case MMC3_23:
							case MMC3_24:

								static_cast<Boards::Mmc3*>(this)->LoadState( State::Loader::Subset(state).Ref() );
								break;
						}
						break;

					case NES_STATE_CHUNK_ID('M','M','5','\0'):

						if (id == MMC5_00)
							static_cast<Boards::Mmc5*>(this)->LoadState( State::Loader::Subset(state).Ref() );

						break;

					case NES_STATE_CHUNK_ID('N','M','6','\0'):

						switch (id)
						{
							case N106_00:
							case N106_01:

								static_cast<Boards::N106*>(this)->LoadState( State::Loader::Subset(state).Ref() );
								break;
						}
						break;

					case NES_STATE_CHUNK_ID('N','M','8','\0'):

						switch (id)
						{
							case N118_00:
							case N118_01:

								static_cast<Boards::N118*>(this)->LoadState( State::Loader::Subset(state).Ref() );
								break;
						}
						break;

					case NES_STATE_CHUNK_ID('V','R','6','\0'):

						switch (id)
						{
							case VRC6_00:
							case VRC6_01:
						
								static_cast<Boards::Vrc6*>(this)->LoadState( State::Loader::Subset(state).Ref() );
								break;
						}
						break;

					case NES_STATE_CHUNK_ID('V','R','7','\0'):

						if (id == VRC7_00)
							static_cast<Boards::Vrc7*>(this)->LoadState( State::Loader::Subset(state).Ref() );
						
						break;

					case NES_STATE_CHUNK_ID('F','M','7','\0'):

						if (id == FME07_00)
							static_cast<Boards::Fme07*>(this)->LoadState( State::Loader::Subset(state).Ref() );
						
						break;

					case NES_STATE_CHUNK_ID('B','A','N','\0'):

						switch (id)
						{
     						case BANDAI_00:
     						case BANDAI_01:
							case BANDAI_02:

								static_cast<Boards::Bandai*>(this)->LoadState( State::Loader::Subset(state).Ref() );
								break;
						}
						break;

					case NES_STATE_CHUNK_ID('T','T','C','\0'):

						switch (id)
						{
     						case TAITOTC_00:
     						case TAITOTC_01:

								static_cast<Boards::TaitoTc*>(this)->LoadState( State::Loader::Subset(state).Ref() );
								break;
						}
						break;

					case NES_STATE_CHUNK_ID('F','F','E','\0'):

						switch (id)
						{
     						case FFE_00:
     						case FFE_01:
							case FFE_02:

								static_cast<Boards::Ffe*>(this)->LoadState( State::Loader::Subset(state).Ref() );
								break;
						}
						break;

					case NES_STATE_CHUNK_ID('B','T','K','\0'):

						switch (id)
						{
     						case BTLTEK2A_00:
							case BTLTEK2A_01:

								static_cast<Boards::BtlTek2A*>(this)->LoadState( State::Loader::Subset(state).Ref() );
								break;
						}
						break;
				}
	
				state.End();
			}
		}

		cstring Mapper::GetBoard(const uint i)
		{ 
			return i < NST_COUNT(setup) ? setup[i].board : ""; 
		}
	
        #ifdef NST_PRAGMA_OPTIMIZE
        #pragma optimize("", on)
        #endif
	
		NES_POKE(Mapper,Wrk_6) 
		{ 
			NST_VERIFY( wrk.IsWritable(0) );
			wrk[0][address - 0x6000U] = data; 
		}

		NES_PEEK(Mapper,Wrk_6) 
		{ 
			NST_VERIFY( wrk.IsReadable(0) );
			return wrk[0][address - 0x6000U]; 
		}

		NES_POKE(Mapper,Wrk_Bus_6) 
		{ 
			NST_VERIFY( wrk.IsWritable(0) );

			if (wrk.IsWritable(0))
				wrk[0][address - 0x6000U] = data; 
		}

		NES_PEEK(Mapper,Wrk_Bus_6) 
		{ 
			NST_VERIFY( wrk.IsReadable(0) );
			return wrk.IsReadable(0) ? wrk[0][address - 0x6000U] : (address >> 8); 
		}

		NES_PEEK(Mapper,Prg_8) { return prg[0][address - 0x8000U]; }
		NES_PEEK(Mapper,Prg_A) { return prg[1][address - 0xA000U]; }
		NES_PEEK(Mapper,Prg_C) { return prg[2][address - 0xC000U]; }
		NES_PEEK(Mapper,Prg_E) { return prg[3][address - 0xE000U]; }
		
		NES_POKE(Mapper,Prg_8k_0) { prg.SwapBank<NES_8K,0x0000U>( data ); }
		NES_POKE(Mapper,Prg_8k_1) { prg.SwapBank<NES_8K,0x2000U>( data ); }
		NES_POKE(Mapper,Prg_8k_2) { prg.SwapBank<NES_8K,0x4000U>( data ); }
		NES_POKE(Mapper,Prg_8k_3) { prg.SwapBank<NES_8K,0x6000U>( data ); }
		NES_POKE(Mapper,Prg_16k)  { prg.SwapBank<NES_16K,0x0000U>( data ); }
		NES_POKE(Mapper,Prg_32k)  { prg.SwapBank<NES_32K,0x0000U>( data ); }

		NES_POKE(Mapper,Chr_1k_0) { ppu.Update(); chr.SwapBank<NES_1K,0x0000U>( data ); }
		NES_POKE(Mapper,Chr_1k_1) { ppu.Update(); chr.SwapBank<NES_1K,0x0400U>( data ); }
		NES_POKE(Mapper,Chr_1k_2) { ppu.Update(); chr.SwapBank<NES_1K,0x0800U>( data ); }
		NES_POKE(Mapper,Chr_1k_3) { ppu.Update(); chr.SwapBank<NES_1K,0x0C00U>( data ); }
		NES_POKE(Mapper,Chr_1k_4) { ppu.Update(); chr.SwapBank<NES_1K,0x1000U>( data ); }
		NES_POKE(Mapper,Chr_1k_5) { ppu.Update(); chr.SwapBank<NES_1K,0x1400U>( data ); }
		NES_POKE(Mapper,Chr_1k_6) { ppu.Update(); chr.SwapBank<NES_1K,0x1800U>( data ); }
		NES_POKE(Mapper,Chr_1k_7) { ppu.Update(); chr.SwapBank<NES_1K,0x1C00U>( data ); }
		NES_POKE(Mapper,Chr_2k_0) { ppu.Update(); chr.SwapBank<NES_2K,0x0000U>( data ); }
		NES_POKE(Mapper,Chr_2k_1) { ppu.Update(); chr.SwapBank<NES_2K,0x0800U>( data ); }
		NES_POKE(Mapper,Chr_2k_2) { ppu.Update(); chr.SwapBank<NES_2K,0x1000U>( data ); }
		NES_POKE(Mapper,Chr_2k_3) { ppu.Update(); chr.SwapBank<NES_2K,0x1800U>( data ); }
		NES_POKE(Mapper,Chr_4k_0) { ppu.Update(); chr.SwapBank<NES_4K,0x0000U>( data ); }
		NES_POKE(Mapper,Chr_4k_1) { ppu.Update(); chr.SwapBank<NES_4K,0x1000U>( data ); }
		NES_POKE(Mapper,Chr_8k)   { ppu.Update(); chr.SwapBank<NES_8K,0x0000U>( data ); }

		NES_POKE(Mapper,Nmt_Hv)  
		{
			NST_COMPILE_ASSERT( Ppu::NMT_HORIZONTAL == 0 && Ppu::NMT_VERTICAL == 1 );
			NST_VERIFY( mirroring != Ppu::NMT_FOURSCREEN );

			ppu.SetMirroring( (data & 0x1) ^ 0x1 );
		}

		NES_POKE(Mapper,Nmt_Vh)  
		{
			NST_COMPILE_ASSERT( Ppu::NMT_HORIZONTAL == 0 && Ppu::NMT_VERTICAL == 1 );
			NST_VERIFY( mirroring != Ppu::NMT_FOURSCREEN );

			ppu.SetMirroring( data & 0x1 );
		}

		NES_POKE(Mapper,Nmt_Vh01) 
		{
			static const uchar lut[4] =
			{
				Ppu::NMT_VERTICAL,
				Ppu::NMT_HORIZONTAL,
				Ppu::NMT_ZERO,
				Ppu::NMT_ONE
			};
	
			ppu.SetMirroring( lut[data & 0x3] );
		}

		NES_POKE(Mapper,Nop)
		{
		}
	
		NES_PEEK(Mapper,Nop)
		{
			return address >> 8;
		}
	}
}
