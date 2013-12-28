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
#include "board/NstBrdJaleco.hpp"
#include "board/NstBrdTaitoTc.hpp"
#include "board/NstBrdTaitoX.hpp"
#include "board/NstBrdColorDreams.hpp"
#include "board/NstBrdJy.hpp"
#include "board/NstBrdFfe.hpp"
#include "board/NstBrdMmc3Waixing.hpp"
#include "board/NstBrdNina.hpp"
#include "board/NstBrdS8259.hpp"
#include "board/NstBrdSuper24In1.hpp"
#include "board/NstBrd8157.hpp"
#include "board/NstBrd8237.hpp"
#include "board/NstBrdWs.hpp"
#include "board/NstBrdDreamTech01.hpp"
#include "board/NstBrdCc21.hpp"
#include "board/NstBrdKof97.hpp"
#include "board/NstBrd64in1Nr.hpp"
#include "board/NstBrdStreetHeroes.hpp"
#include "board/NstBrdT262.hpp"
#include "board/NstBrdFk23C.hpp"
#include "board/NstBrd6035052.hpp"
#include "board/NstBrdA65AS.hpp"
#include "board/NstBrdEdu2000.hpp"
#include "board/NstBrdSubor.hpp"
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
#include "mapper/NstMapper014.hpp"
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
#include "mapper/NstMapper027.hpp"
#include "mapper/NstMapper032.hpp"
#include "mapper/NstMapper033.hpp"
#include "mapper/NstMapper034.hpp"
#include "mapper/NstMapper037.hpp"
#include "mapper/NstMapper039.hpp"
#include "mapper/NstMapper040.hpp"
#include "mapper/NstMapper041.hpp"
#include "mapper/NstMapper042.hpp"
#include "mapper/NstMapper043.hpp"
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
#include "mapper/NstMapper054.hpp"
#include "mapper/NstMapper055.hpp"
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
#include "mapper/NstMapper101.hpp"
#include "mapper/NstMapper105.hpp"
#include "mapper/NstMapper107.hpp"
#include "mapper/NstMapper112.hpp"
#include "mapper/NstMapper113.hpp"
#include "mapper/NstMapper114.hpp"
#include "mapper/NstMapper115.hpp"
#include "mapper/NstMapper116.hpp"
#include "mapper/NstMapper117.hpp"
#include "mapper/NstMapper118.hpp"
#include "mapper/NstMapper119.hpp"
#include "mapper/NstMapper123.hpp"
#include "mapper/NstMapper132.hpp"
#include "mapper/NstMapper133.hpp"
#include "mapper/NstMapper137.hpp"
#include "mapper/NstMapper138.hpp"
#include "mapper/NstMapper139.hpp"
#include "mapper/NstMapper140.hpp"
#include "mapper/NstMapper141.hpp"
#include "mapper/NstMapper142.hpp"
#include "mapper/NstMapper143.hpp"
#include "mapper/NstMapper144.hpp"
#include "mapper/NstMapper145.hpp"
#include "mapper/NstMapper146.hpp"
#include "mapper/NstMapper147.hpp"
#include "mapper/NstMapper148.hpp"
#include "mapper/NstMapper149.hpp"
#include "mapper/NstMapper150.hpp"
#include "mapper/NstMapper151.hpp"
#include "mapper/NstMapper152.hpp"
#include "mapper/NstMapper153.hpp"
#include "mapper/NstMapper154.hpp"
#include "mapper/NstMapper155.hpp"
#include "mapper/NstMapper156.hpp"
#include "mapper/NstMapper157.hpp"
#include "mapper/NstMapper159.hpp"
#include "mapper/NstMapper163.hpp"
#include "mapper/NstMapper164.hpp"
#include "mapper/NstMapper165.hpp"
#include "mapper/NstMapper166.hpp"
#include "mapper/NstMapper167.hpp"
#include "mapper/NstMapper169.hpp"
#include "mapper/NstMapper170.hpp"
#include "mapper/NstMapper171.hpp"
#include "mapper/NstMapper172.hpp"
#include "mapper/NstMapper180.hpp"
#include "mapper/NstMapper182.hpp"
#include "mapper/NstMapper183.hpp"
#include "mapper/NstMapper184.hpp"
#include "mapper/NstMapper185.hpp"
#include "mapper/NstMapper186.hpp"
#include "mapper/NstMapper187.hpp"
#include "mapper/NstMapper188.hpp"
#include "mapper/NstMapper189.hpp"
#include "mapper/NstMapper191.hpp"
#include "mapper/NstMapper192.hpp"
#include "mapper/NstMapper193.hpp"
#include "mapper/NstMapper194.hpp"
#include "mapper/NstMapper195.hpp"
#include "mapper/NstMapper196.hpp"
#include "mapper/NstMapper197.hpp"
#include "mapper/NstMapper198.hpp"
#include "mapper/NstMapper199.hpp"
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
#include "mapper/NstMapper214.hpp"
#include "mapper/NstMapper215.hpp"
#include "mapper/NstMapper216.hpp"
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
#include "mapper/NstMapper236.hpp"
#include "mapper/NstMapper240.hpp"
#include "mapper/NstMapper241.hpp"
#include "mapper/NstMapper242.hpp"
#include "mapper/NstMapper243.hpp"
#include "mapper/NstMapper244.hpp"
#include "mapper/NstMapper245.hpp"
#include "mapper/NstMapper246.hpp"
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
				NMT_ZERO,
				NMT_FOURSCREEN
			};

			cstring board;
			uchar prg : 3;
			uchar nmt : 3;
			uchar nmtd : 2;
		};

		#ifdef _MSC_VER
		#pragma pack(pop)
		#endif

		const Mapper::Setup Mapper::setup[256+NUM_EXT_MAPPERS] =
		{
			{ "NROM",                                  Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 000
			{ "SxROM",                                 Setup::PRG_DEFAULT,  Setup::NMT_ZERO,       Setup::NMT_ZERO       }, // 001
			{ "UxROM",                                 Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 002
			{ "CNROM",                                 Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 003
			{ "TxROM / HxROM / DxROM / TENGEN 8000xx", Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 004
			{ "ExROM",                                 Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 005
			{ "FFE F4xxx(128+128)/F3xxx(128+64)",      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 006
			{ "AxROM",                                 Setup::PRG_0123_32K, Setup::NMT_ZERO,       Setup::NMT_ZERO       }, // 007
			{ "FFE F3xxx",                             Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 008
			{ "PxROM",                                 Setup::PRG_0XXX_24K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 009
			{ "FxROM",                                 Setup::PRG_01XX_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 010
			{ "COLOR DREAMS",                          Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 011
			{ "BTL REX DBZ5",                          Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 012
			{ "CPROM",                                 Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 013
			{ "SL1632",                                Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 014
			{ "BMC CONTRA 100-IN-1",                   Setup::PRG_0123_32K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 015
			{ "BANDAI +24C02",                         Setup::PRG_01XX_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 016
			{ "FFE F8xxx/SMxxxx",                      Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 017
			{ "JALECO SS8806",                         Setup::PRG_012X_8K,  Setup::NMT_HORIZONTAL, Setup::NMT_HORIZONTAL }, // 018
			{ "NAMCOT 106/163",                        Setup::PRG_012X_8K,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 019
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 020
			{ "KONAMI VRC4 2A",                        Setup::PRG_01XX_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 021
			{ "KONAMI VRC4 1B",                        Setup::PRG_01XX_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 022
			{ "KONAMI VRC2/VRC2B",                     Setup::PRG_01XX_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 023
			{ "KONAMI VRC6 A0/A1",                     Setup::PRG_01XX_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 024
			{ "KONAMI VRC4 Y",                         Setup::PRG_01XX_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 025
			{ "KONAMI VRC6 A1/A0",                     Setup::PRG_01XX_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 026
			{ "VRC4 +EXT.CHR",                         Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 027
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 028
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 029
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 030
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 031
			{ "IREM G-101",                            Setup::PRG_XXXX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 032
			{ "TAITO TC0190/TC0350",                   Setup::PRG_0XXX_24K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 033
			{ "BNROM / AVE NINA-001",                  Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 034
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 035
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 036
			{ "ZZ",                                    Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 037
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 038
			{ "BMC STUDY & GAME 32-IN-1",              Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 039
			{ "BTL SMB2",                              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 040
			{ "CALTRON 6-IN-1",                        Setup::PRG_0123_32K, Setup::NMT_HORIZONTAL, Setup::NMT_HORIZONTAL }, // 041
			{ "BTL AI SENSHI NICOL / MARIO BABY",      Setup::PRG_XXXX_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 042
			{ "BTL SMB2 (LF-36)",                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 043
			{ "BMC SUPER BIG 7-IN-1",                  Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 044
			{ "BMC SUPER/HERO X-IN-1",                 Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 045
			{ "RUMBLE-STATION 15-IN-1",                Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 046
			{ "QJ",                                    Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 047
			{ "TAITO TC190V",                          Setup::PRG_0XXX_24K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 048
			{ "BMC SUPER HiK 4-IN-1",                  Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 049
			{ "BTL SMB2 (alt)",                        Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 050
			{ "BMC BALL GAMES 11-IN-1",                Setup::PRG_0123_32K, Setup::NMT_HORIZONTAL, Setup::NMT_HORIZONTAL }, // 051
			{ "BMC MARIO PARTY 7-IN-1",                Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 052
			{ "BMC SUPERVISION 16-IN-1",               Setup::PRG_DEFAULT,  Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 053
			{ "BMC NOVELDIAMOND 9999999-IN-1",         Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 054
			{ "BTL MARIO1-MALEE2",                     Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 055
			{ "BTL SMB3",                              Setup::PRG_012X_8K,  Setup::NMT_HORIZONTAL, Setup::NMT_HORIZONTAL }, // 056
			{ "BMC GK-54/GK-L01A/GK-L02A",             Setup::PRG_0101_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 057
			{ "GK-192",                                Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 058
			{ "BMC T3H53",                             Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 059
			{ "BMC VT5201",                            Setup::PRG_DEFAULT,  Setup::NMT_HORIZONTAL, Setup::NMT_HORIZONTAL }, // 060
			{ "BMC 20-IN-1",                           Setup::PRG_01XX_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 061
			{ "BMC SUPER 700-IN-1",                    Setup::PRG_0123_32K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 062
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 063
			{ "TENGEN 800032",                         Setup::PRG_DEFAULT,  Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 064
			{ "IREM H-3001",                           Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 065
			{ "GNROM / MHROM",                         Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 066
			{ "SUNSOFT3",                              Setup::PRG_01XX_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 067
			{ "SUNSOFT4",                              Setup::PRG_01XX_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 068
			{ "FME-7 / SUNSOFT5A / SUNSOFT5B",         Setup::PRG_01XX_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 069
			{ "BANDAI 74161/32",                       Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 070
			{ "CAMERICA BF9093/BF9097",                Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 071
			{ "JALECO",                                Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 072
			{ "KONAMI VRC3",                           Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 073
			{ "WAIXING MMC3 +XRAM.4K +CRAM.2K",        Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 074
			{ "JALECO SS8805 / KONAMI VRC1",           Setup::PRG_012X_8K,  Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 075
			{ "NAMCOT 108 +MB74LS32",                  Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 076
			{ "IREM",                                  Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 077
			{ "IREM 74HC161/32",                       Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 078
			{ "AVE NINA-03/NINA-06",                   Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 079
			{ "TAITO X-005",                           Setup::PRG_012X_8K,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 080
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 081
			{ "TAITO X1-017",                          Setup::PRG_01XX_16K, Setup::NMT_HORIZONTAL, Setup::NMT_HORIZONTAL }, // 082
			{ "BMC DBZ 4-IN-1 / CONY",                 Setup::PRG_DEFAULT,  Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 083
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 084
			{ "KONAMI VRC7",                           Setup::PRG_01XX_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 085
			{ "JALECO JF-14",                          Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 086
			{ "KONAMI 74HC161/32 / JALECO",            Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 087
			{ "NAMCOT 118",                            Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 088
			{ "SUNSOFT2",                              Setup::PRG_01XX_16K, Setup::NMT_ZERO,       Setup::NMT_ZERO       }, // 089
			{ "J.Y.COMPANY +EXT.MIRR.OFF",             Setup::PRG_DEFAULT,  Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 090
			{ "BTL SF3",                               Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 091
			{ "JALECO",                                Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 092
			{ "SUNSOFT1 / SUNSOFT2",                   Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 093
			{ "CAPCOM 74HC161/32",                     Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 094
			{ "NAMCOT 118 +A1.D5.MIRR",                Setup::PRG_01XX_16K, Setup::NMT_ZERO,       Setup::NMT_ZERO       }, // 095
			{ "BANDAI 74HC161/32",                     Setup::PRG_0123_32K, Setup::NMT_ZERO,       Setup::NMT_ZERO       }, // 096
			{ "IREM 74HC161/32",                       Setup::PRG_XX01_16K, Setup::NMT_HORIZONTAL, Setup::NMT_HORIZONTAL }, // 097
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 098
			{ "VS-SYSTEM +D2.CHR/PRG",                 Setup::PRG_0123_32K, Setup::NMT_FOURSCREEN, Setup::NMT_FOURSCREEN }, // 099
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 100
			{ "JALECO 74HC161/32",                     Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 101
			{ "",                                      Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 102
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 103
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 104
			{ "EVENT",                                 Setup::PRG_DEFAULT,  Setup::NMT_ZERO,       Setup::NMT_ZERO       }, // 105
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 106
			{ "MAGICSERIES",                           Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 107
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 108
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 109
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 110
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 111
			{ "ASDER (NTDEC)",                         Setup::PRG_01XX_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 112
			{ "NINA-03/NINA-06 +MIRR",                 Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_HORIZONTAL }, // 113
			{ "BTL THE LION KING",                     Setup::PRG_01XX_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 114
			{ "KASING",                                Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 115
			{ "SOMERI TEAM SL-12",                     Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 116
			{ "FUTUREMEDIA",                           Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 117
			{ "TLSROM / TKSROM / TENGEN 800037",       Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 118
			{ "TQROM",                                 Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 119
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 120
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 121
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 122
			{ "H2288",                                 Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 123
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 124
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 125
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 126
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 127
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 128
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 129
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 130
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 131
			{ "TXC 22211",                             Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 132
			{ "SA72008",                               Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 133
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 134
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 135
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 136
			{ "S8259D",                                Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 137
			{ "S8259C",                                Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 138
			{ "S8259B",                                Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 139
			{ "JALECO JF-xx",                          Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 140
			{ "S8259A",                                Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 141
			{ "KS 202",                                Setup::PRG_012X_8K,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 142
			{ "TCA01",                                 Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 143
			{ "AGCI 50282",                            Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 144
			{ "SA72007",                               Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 145
			{ "SA0161M",                               Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 146
			{ "TCU01",                                 Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 147
			{ "SA0037",                                Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 148
			{ "SA0036",                                Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 149
			{ "S74LS374N",                             Setup::PRG_0123_32K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 150
			{ "KONAMI VS-SYSTEM",                      Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 151
			{ "BANDAI 74161/32 +MIRR",                 Setup::PRG_01XX_16K, Setup::NMT_ZERO,       Setup::NMT_ZERO       }, // 152
			{ "BANDAI +WRAM",                          Setup::PRG_01XX_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 153
			{ "NAMCOT 118 +A0.D6.MIRR",                Setup::PRG_01XX_16K, Setup::NMT_ZERO,       Setup::NMT_ZERO       }, // 154
			{ "MMC1A",                                 Setup::PRG_DEFAULT,  Setup::NMT_ZERO,       Setup::NMT_ZERO       }, // 155
			{ "DAOU 306",                              Setup::PRG_01XX_16K, Setup::NMT_ZERO,       Setup::NMT_ZERO       }, // 156
			{ "BANDAI DATACH JOINT SYSTEM",            Setup::PRG_01XX_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 157
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 158
			{ "BANDAI +24C01",                         Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 159
			{ "",                                      Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 160
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 161
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 162
			{ "NANJING",                               Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 163
			{ "WAIXING / MARS PRODUCTION",             Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 164
			{ "WAIXING SHENGHUO HUIZHANG 2",           Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 165
			{ "SUBOR (Russian)",                       Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 166
			{ "SUBOR (Chinese)",                       Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 167
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 168
			{ "N625092",                               Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 169
			{ "FUJIYA NROM +SECURITY",                 Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 170
			{ "KAISER KS7058",                         Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 171
			{ "IDEA-TEK CNROM +SECURITY",              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 172
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 173
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 174
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 175
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 176
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 177
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 178
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 179
			{ "UNROM M5",                              Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 180
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 181
			{ "HOSENKAN ELECTRONICS",                  Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 182
			{ "BTL SHUI GUAN PIPE",                    Setup::PRG_012X_8K,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 183
			{ "SUNSOFT1",                              Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 184
			{ "CNROM +CHR.DISABLE",                    Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 185
			{ "SBX",                                   Setup::PRG_0101_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 186
			{ "BTL SFZ297/KOF96/S3DB6",                Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 187
			{ "BANDAI KARAOKE STUDIO",                 Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 188
			{ "YOKOSOFT / TXC",                        Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 189
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 190
			{ "WAIXING MMC3 +XRAM.4K +CRAM.2K",        Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 191
			{ "WAIXING MMC3 +XRAM.4K +CRAM.4K",        Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 192
			{ "MEGA SOFT (NTDEC)",                     Setup::PRG_0XXX_24K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 193
			{ "WAIXING MMC3 +XRAM.4K +CRAM.2K (alt)",  Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 194
			{ "WAIXING MMC3 +XRAM.4K +CRAM.4K (alt)",  Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 195
			{ "MMC3 +A0/A2",                           Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 196
			{ "SUPER FIGHTER III",                     Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 197
			{ "WAIXING MMC3 +XRAM.4K",                 Setup::PRG_01XX_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 198
			{ "WAIXING MMC3 +XRAM.4K +CRAM.8K",        Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 199
			{ "BMC 1200/36-IN-1",                      Setup::PRG_0101_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 200
			{ "BMC 21/8-IN-1",                         Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 201
			{ "BMC 150-IN-1",                          Setup::PRG_01XX_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 202
			{ "BMC 35-IN-1",                           Setup::PRG_0101_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 203
			{ "BMC 64-IN-1",                           Setup::PRG_0101_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 204
			{ "BMC 15/3-IN-1",                         Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 205
			{ "DE1ROM",                                Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 206
			{ "TAITO X-005 +MIRR",                     Setup::PRG_012X_8K,  Setup::NMT_HORIZONTAL, Setup::NMT_HORIZONTAL }, // 207
			{ "GOUDER BTL SF4",                        Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 208
			{ "J.Y.COMPANY +EXT.MIRR.CTRL",            Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 209
			{ "NAMCOT",                                Setup::PRG_012X_8K,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 210
			{ "J.Y.COMPANY +EXT.MIRR.ON",              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 211
			{ "BMC SUPER HIK 300-IN-1",                Setup::PRG_XXXX_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 212
			{ "BMC 9999999-IN-1",                      Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 213
			{ "BMC SUPER GUN 20-IN-1",                 Setup::PRG_0101_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 214
			{ "BMC SUPER 308 3-IN-1 / M-E3",           Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 215
			{ "RCM MAGIC JEWELRY 2",                   Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 216
			{ "BMC SPC009",                            Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 217
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 218
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 219
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 220
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 221
			{ "BTL DRAGON NINJA",                      Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 222
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 223
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 224
			{ "BMC 58/64/72-IN-1",                     Setup::PRG_0123_32K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 225
			{ "BMC 76-IN-1 / SUPER 42-IN-1",           Setup::PRG_0123_32K, Setup::NMT_HORIZONTAL, Setup::NMT_HORIZONTAL }, // 226
			{ "BMC 1200-IN-1",                         Setup::PRG_DEFAULT,  Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 227
			{ "ACTION 52",                             Setup::PRG_0123_32K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 228
			{ "BMC 31-IN-1",                           Setup::PRG_0123_32K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 229
			{ "BMC 22-IN-1",                           Setup::PRG_DEFAULT,  Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 230
			{ "BMC 20-IN-1",                           Setup::PRG_0101_16K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 231
			{ "CAMERICA BF9096",                       Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 232
			{ "BMC 20/22-IN-1",                        Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 233
			{ "AVE D-1012",                            Setup::PRG_0123_32K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 234
			{ "BMC GOLDEN GAME 150/260-IN-1",          Setup::PRG_0123_32K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 235
			{ "BMC 800/70-IN-1",                       Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 236
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 237
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 238
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 239
			{ "SUPERTONE / C&E",                       Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 240
			{ "MXMDHTWO / TXC",                        Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 241
			{ "WAIXING ZHAN SHI",                      Setup::PRG_0123_32K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 242
			{ "SACHEN 74LS374N",                       Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 243
			{ "C&E DECATHLON",                         Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 244
			{ "WAIXING MMC3 +EX.PRG",                  Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 245
			{ "C&E PHONE SERM BERM",                   Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 246
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 247
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 248
			{ "WAIXING MMC3 +EX.PRG/CHR",              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 249
			{ "NITRA MMC3",                            Setup::PRG_DEFAULT,  Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 250
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 251
			{ "WAIXING SAN GUO ZHI",                   Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 252
			{ "",                                      Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 253
			{ "BTL PIKACHU Y2K",                       Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // 254
			{ "BMC 110/115-IN-1",                      Setup::PRG_0123_32K, Setup::NMT_VERTICAL,   Setup::NMT_VERTICAL   }, // 255
			{ "BMC SUPER 24-IN-1",                     Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // Ext. 256
			{ "BMC 8157",                              Setup::PRG_0101_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // Ext. 257
			{ "8237",                                  Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // Ext. 258
			{ "WS",                                    Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // Ext. 259
			{ "DREAMTECH-01",                          Setup::PRG_01XX_16K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // Ext. 260
			{ "CC21",                                  Setup::PRG_0123_32K, Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // Ext. 261
			{ "KOF97",                                 Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // Ext. 262
			{ "BMC 64-IN-1 NO REPEAT",                 Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // Ext. 263
			{ "SHERO",                                 Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // Ext. 264
			{ "BMC T-262",                             Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // Ext. 265
			{ "BMC FK23C",                             Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // Ext. 266
			{ "603-5052",                              Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // Ext. 267
			{ "A65AS",                                 Setup::PRG_DEFAULT,  Setup::NMT_DEFAULT,    Setup::NMT_VERTICAL   }, // Ext. 268
			{ "EDU2000",                               Setup::PRG_DEFAULT,  Setup::NMT_ZERO,       Setup::NMT_ZERO       }  // Ext. 269
		};

		Mapper* Mapper::Create(Context& context)
		{
			switch (context.id)
			{
				case   0: return new Mapper0   ( context );
				case   1: return new Mapper1   ( context );
				case   2: return new Mapper2   ( context );
				case   3: return new Mapper3   ( context );
				case   4: return new Mapper4   ( context );
				case   5: return new Mapper5   ( context );
				case   6: return new Mapper6   ( context );
				case   7: return new Mapper7   ( context );
				case   8: return new Mapper8   ( context );
				case   9: return new Mapper9   ( context );
				case  10: return new Mapper10  ( context );
				case  11: return new Mapper11  ( context );
				case  12: return new Mapper12  ( context );
				case  13: return new Mapper13  ( context );
				case  14: return new Mapper14  ( context );
				case  15: return new Mapper15  ( context );
				case  16: return new Mapper16  ( context );
				case  17: return new Mapper17  ( context );
				case  18: return new Mapper18  ( context );
				case  19: return new Mapper19  ( context );
				case  21: return new Mapper21  ( context );
				case  22: return new Mapper22  ( context );
				case  23: return new Mapper23  ( context );
				case  24: return new Mapper24  ( context );
				case  25: return new Mapper25  ( context );
				case  26: return new Mapper26  ( context );
				case  27: return new Mapper27  ( context );
				case  32: return new Mapper32  ( context );
				case  33: return new Mapper33  ( context );
				case  34: return new Mapper34  ( context );
				case  37: return new Mapper37  ( context );
				case  39: return new Mapper39  ( context );
				case  40: return new Mapper40  ( context );
				case  41: return new Mapper41  ( context );
				case  42: return new Mapper42  ( context );
				case  43: return new Mapper43  ( context );
				case  44: return new Mapper44  ( context );
				case  45: return new Mapper45  ( context );
				case  46: return new Mapper46  ( context );
				case  47: return new Mapper47  ( context );
				case  48: return new Mapper48  ( context );
				case  49: return new Mapper49  ( context );
				case  50: return new Mapper50  ( context );
				case  51: return new Mapper51  ( context );
				case  52: return new Mapper52  ( context );
				case  53: return new Mapper53  ( context );
				case  54: return new Mapper54  ( context );
				case  55: return new Mapper55  ( context );
				case  56: return new Mapper56  ( context );
				case  57: return new Mapper57  ( context );
				case  58: return new Mapper58  ( context );
				case  60: return new Mapper60  ( context );
				case  61: return new Mapper61  ( context );
				case  62: return new Mapper62  ( context );
				case  64: return new Mapper64  ( context );
				case  65: return new Mapper65  ( context );
				case  66: return new Mapper66  ( context );
				case  67: return new Mapper67  ( context );
				case  68: return new Mapper68  ( context );
				case  69: return new Mapper69  ( context );
				case  70: return new Mapper70  ( context );
				case  71: return new Mapper71  ( context );
				case  72: return new Mapper72  ( context );
				case  73: return new Mapper73  ( context );
				case  74: return new Mapper74  ( context );
				case  75: return new Mapper75  ( context );
				case  76: return new Mapper76  ( context );
				case  77: return new Mapper77  ( context );
				case  78: return new Mapper78  ( context );
				case  79: return new Mapper79  ( context );
				case  80: return new Mapper80  ( context );
				case  82: return new Mapper82  ( context );
				case  83: return new Mapper83  ( context );
				case  85: return new Mapper85  ( context );
				case  86: return new Mapper86  ( context );
				case  87: return new Mapper87  ( context );
				case  88: return new Mapper88  ( context );
				case  89: return new Mapper89  ( context );
				case  90: return new Mapper90  ( context );
				case  91: return new Mapper91  ( context );
				case  92: return new Mapper92  ( context );
				case  93: return new Mapper93  ( context );
				case  94: return new Mapper94  ( context );
				case  95: return new Mapper95  ( context );
				case  96: return new Mapper96  ( context );
				case  97: return new Mapper97  ( context );
				case  99: return new Mapper99  ( context );
				case 101: return new Mapper101 ( context );
				case 105: return new Mapper105 ( context );
				case 107: return new Mapper107 ( context );
				case 112: return new Mapper112 ( context );
				case 113: return new Mapper113 ( context );
				case 114: return new Mapper114 ( context );
				case 115: return new Mapper115 ( context );
				case 116: return new Mapper116 ( context );
				case 117: return new Mapper117 ( context );
				case 118: return new Mapper118 ( context );
				case 119: return new Mapper119 ( context );
				case 123: return new Mapper123 ( context );
				case 132: return new Mapper132 ( context );
				case 133: return new Mapper133 ( context );
				case 137: return new Mapper137 ( context );
				case 138: return new Mapper138 ( context );
				case 139: return new Mapper139 ( context );
				case 140: return new Mapper140 ( context );
				case 141: return new Mapper141 ( context );
				case 142: return new Mapper142 ( context );
				case 143: return new Mapper143 ( context );
				case 144: return new Mapper144 ( context );
				case 145: return new Mapper145 ( context );
				case 146: return new Mapper146 ( context );
				case 147: return new Mapper147 ( context );
				case 148: return new Mapper148 ( context );
				case 149: return new Mapper149 ( context );
				case 150: return new Mapper150 ( context );
				case 151: return new Mapper151 ( context );
				case 152: return new Mapper152 ( context );
				case 153: return new Mapper153 ( context );
				case 154: return new Mapper154 ( context );
				case 155: return new Mapper155 ( context );
				case 156: return new Mapper156 ( context );
				case 157: return new Mapper157 ( context );
				case 159: return new Mapper159 ( context );
				case 163: return new Mapper163 ( context );
				case 164: return new Mapper164 ( context );
				case 165: return new Mapper165 ( context );
				case 166: return new Mapper166 ( context );
				case 167: return new Mapper167 ( context );
				case 169: return new Mapper169 ( context );
				case 170: return new Mapper170 ( context );
				case 171: return new Mapper171 ( context );
				case 172: return new Mapper172 ( context );
				case 180: return new Mapper180 ( context );
				case 182: return new Mapper182 ( context );
				case 183: return new Mapper183 ( context );
				case 184: return new Mapper184 ( context );
				case 185: return new Mapper185 ( context );
				case 186: return new Mapper186 ( context );
				case 187: return new Mapper187 ( context );
				case 188: return new Mapper188 ( context );
				case 189: return new Mapper189 ( context );
				case 191: return new Mapper191 ( context );
				case 192: return new Mapper192 ( context );
				case 193: return new Mapper193 ( context );
				case 194: return new Mapper194 ( context );
				case 195: return new Mapper195 ( context );
				case 196: return new Mapper196 ( context );
				case 197: return new Mapper197 ( context );
				case 198: return new Mapper198 ( context );
				case 199: return new Mapper199 ( context );
				case 200: return new Mapper200 ( context );
				case 201: return new Mapper201 ( context );
				case 202: return new Mapper202 ( context );
				case 203: return new Mapper203 ( context );
				case 204: return new Mapper204 ( context );
				case 205: return new Mapper205 ( context );
				case 206: return new Mapper206 ( context );
				case 207: return new Mapper207 ( context );
				case 208: return new Mapper208 ( context );
				case 209: return new Mapper209 ( context );
				case 210: return new Mapper210 ( context );
				case 211: return new Mapper211 ( context );
				case 212: return new Mapper212 ( context );
				case 213: return new Mapper213 ( context );
				case 214: return new Mapper214 ( context );
				case 215: return new Mapper215 ( context );
				case 216: return new Mapper216 ( context );
				case 217: return new Mapper217 ( context );
				case 222: return new Mapper222 ( context );
				case 225: return new Mapper225 ( context );
				case 226: return new Mapper226 ( context );
				case 227: return new Mapper227 ( context );
				case 228: return new Mapper228 ( context );
				case 229: return new Mapper229 ( context );
				case 230: return new Mapper230 ( context );
				case 231: return new Mapper231 ( context );
				case 232: return new Mapper232 ( context );
				case 233: return new Mapper233 ( context );
				case 234: return new Mapper234 ( context );
				case 235: return new Mapper235 ( context );
				case 236: return new Mapper236 ( context );
				case 240: return new Mapper240 ( context );
				case 241: return new Mapper241 ( context );
				case 242: return new Mapper242 ( context );
				case 243: return new Mapper243 ( context );
				case 244: return new Mapper244 ( context );
				case 245: return new Mapper245 ( context );
				case 246: return new Mapper246 ( context );
				case 249: return new Mapper249 ( context );
				case 250: return new Mapper250 ( context );
				case 252: return new Mapper252 ( context );
				case 254: return new Mapper254 ( context );
				case 255: return new Mapper255 ( context );

				case EXT_SUPER24IN1:   return new Boards::Super24In1   ( context );
				case EXT_8157:         return new Boards::Unl8157      ( context );
				case EXT_8237:         return new Boards::Unl8237      ( context );
				case EXT_WS:           return new Boards::Ws           ( context );
				case EXT_DREAMTECH01:  return new Boards::DreamTech01  ( context );
				case EXT_CC21:         return new Boards::Cc21         ( context );
				case EXT_KOF97:        return new Boards::Kof97        ( context );
				case EXT_64IN1NR:      return new Boards::Bmc64In1Nr   ( context );
				case EXT_STREETHEROES: return new Boards::StreetHeroes ( context );
				case EXT_T262:         return new Boards::T262         ( context );
				case EXT_FK23C:        return new Boards::Fk23C        ( context );
				case EXT_6035052:      return new Boards::Unl6035052   ( context );
				case EXT_A65AS:        return new Boards::A65AS        ( context );
				case EXT_EDU2000:      return new Boards::Edu2000      ( context );

				default: throw RESULT_ERR_UNSUPPORTED_MAPPER;
			}
		}

		Mapper::Mapper(Context& context,const uint settings)
		:
		cpu       (context.cpu),
		ppu       (context.ppu),
		chr       (context.ppu.GetChrMem()),
		nmt       (context.ppu.GetNmtMem()),
		id        (context.id),
		mirroring (context.mirroring)
		{
			dword size = context.prg.Size();

			switch (settings & PROM_SETTINGS)
			{
				case PROM_MAX_16K:  size = SIZE_16K;  break;
				case PROM_MAX_32K:  size = SIZE_32K;  break;
				case PROM_MAX_64K:  size = SIZE_64K;  break;
				case PROM_MAX_128K: size = SIZE_128K; break;
				case PROM_MAX_256K: size = SIZE_256K; break;
				case PROM_MAX_512K: size = SIZE_512K; break;
			}

			if (context.prg.Size() > size)
			{
				NST_DEBUG_MSG( "PRG-ROM truncated!" );
				context.prg.Set( size );
			}
			else
			{
				context.prg.Mirror( SIZE_16K );
			}

			prg.Source(0).Set( context.prg.Mem(), context.prg.Size(), true, false );

			cstring chrRomDiscarded = NULL;

			size = context.chr.Size();

			if (size)
			{
				switch (settings & CROM_SETTINGS)
				{
					case CROM_NONE:      size = 0;         break;
					case CROM_MAX_8K:    size = SIZE_8K;   break;
					case CROM_MAX_16K:   size = SIZE_16K;  break;
					case CROM_MAX_32K:   size = SIZE_32K;  break;
					case CROM_MAX_64K:   size = SIZE_64K;  break;
					case CROM_MAX_128K:  size = SIZE_128K; break;
					case CROM_MAX_256K:  size = SIZE_256K; break;
					case CROM_MAX_512K:  size = SIZE_512K; break;
				}

				if (context.chr.Size() > size)
				{
					NST_DEBUG_MSG( "CHR-ROM truncated!" );

					if (!size)
						chrRomDiscarded = "warning, CHR-ROM discarded!" NST_LINEBREAK;

					context.chr.Set( size );
				}
				else
				{
					context.chr.Mirror( SIZE_8K );
				}
			}

			switch (settings & CRAM_SETTINGS)
			{
				case CRAM_1K:  size = SIZE_1K;  break;
				case CRAM_2K:  size = SIZE_2K;  break;
				case CRAM_4K:  size = SIZE_4K;  break;
				case CRAM_8K:  size = SIZE_8K;  break;
				case CRAM_16K: size = SIZE_16K; break;
				case CRAM_32K: size = SIZE_32K; break;
				default:       size = 0;        break;
			}

			if (context.chr.Size())
			{
				chr.Source(0).Set( context.chr.Mem(), context.chr.Size(), true, false );
				chr.Source(1).Set( context.chr.Mem(), context.chr.Size(), true, false );
			}
			else if (size < SIZE_8K)
			{
				size = SIZE_8K;
			}

			context.chrRam = size;

			if (context.chrRam)
			{
				chr.Source( context.chr.Size() != 0 ).Set( context.chrRam, true, true );

				if (context.chr.Empty())
					chr.Source(1).Set( chr.Source().Mem(), chr.Source().Size(), true, true );
			}

			if (mirroring == Ppu::NMT_FOURSCREEN)
				nmt.Source().Set( SIZE_4K, true, true );

			nmt.Source(1).Set( chr.Source().Mem(), chr.Source().Size(), true, context.chr.Size() == 0 );

			context.wrkAuto = false;

			switch (settings & WRAM_SETTINGS)
			{
				case WRAM_NONE: size = 0;        break;
				case WRAM_1K:   size = SIZE_1K;  break;
				case WRAM_2K:   size = SIZE_2K;  break;
				case WRAM_4K:   size = SIZE_4K;  break;
				case WRAM_8K:   size = SIZE_8K;  break;
				case WRAM_16K:  size = SIZE_16K; break;
				case WRAM_32K:  size = SIZE_32K; break;
				case WRAM_40K:  size = SIZE_40K; break;
				case WRAM_64K:  size = SIZE_32K; break;

				case WRAM_DEFAULT:

					size = context.wrk.Size() ? NST_MAX(context.wrkBacked,SIZE_8K) : 0;
					break;

				default:

					size = NST_MAX(context.wrkBacked,SIZE_8K);
					context.wrkAuto = context.wrk.Empty();
					break;
			}

			context.wrk.Set( size );

			wrk.Source(0).Set
			(
				context.wrk.Size() ? context.wrk.Mem() : context.prg.Mem(),
				context.wrk.Size() ? context.wrk.Size() : context.prg.Size(),
				true,
				context.wrk.Size() != 0
			);

			wrk.Source(1).Set( prg.Source().Mem(), prg.Source().Size(), true, false );
			prg.Source(1).Set( wrk.Source().Mem(), wrk.Source().Size(), true, context.wrk.Size() != 0 );

			Log log;

			char buffer[16];
			cstring title = buffer;

			if (id <= 255)
				std::sprintf( buffer, "Mapper %u: ", id );
			else
				title = "Mapper: ";

			if (*GetBoard( id ))
				log << title << GetBoard( id ) << NST_LINEBREAK;

			log << title << (prg.Source().Size() / SIZE_1K) << "k PRG-ROM" NST_LINEBREAK
				<< title << (chr.Source().Size() / SIZE_1K) << (context.chr.Size() ? "k CHR-ROM" NST_LINEBREAK : "k CHR-RAM" NST_LINEBREAK);

			if (chrRomDiscarded)
				log << title << chrRomDiscarded;

			if (chr.Source(1).Internal())
				log << title << (chr.Source(1).Size() / SIZE_1K) << "k CHR-RAM" NST_LINEBREAK;

			if (context.wrk.Size())
				log << title << (context.wrk.Size() / SIZE_1K) << (context.wrkAuto ? "k auto W-RAM" NST_LINEBREAK : "k W-RAM" NST_LINEBREAK);

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

			log << title << type << " mirroring" NST_LINEBREAK;
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

			if (wrk.RamSize() >= SIZE_8K)
				cpu.Map( 0x6000U, 0x7FFFU ).Set( this, &Mapper::Peek_Wrk_6, &Mapper::Poke_Wrk_6 );
			else
				cpu.Map( 0x6000U, 0x7FFFU ).Set( this, &Mapper::Peek_Nop, &Mapper::Poke_Nop );

			cpu.Map( 0x8000U, 0x9FFFU ).Set( this, &Mapper::Peek_Prg_8, &Mapper::Poke_Nop );
			cpu.Map( 0xA000U, 0xBFFFU ).Set( this, &Mapper::Peek_Prg_A, &Mapper::Poke_Nop );
			cpu.Map( 0xC000U, 0xDFFFU ).Set( this, &Mapper::Peek_Prg_C, &Mapper::Poke_Nop );
			cpu.Map( 0xE000U, 0xFFFFU ).Set( this, &Mapper::Peek_Prg_E, &Mapper::Poke_Nop );

			cpu.ClearIRQ();

			if (hard)
			{
				NST_ASSERT( id < NST_COUNT(setup) );

				if (chr.Source(0).Internal() || chr.Source(1).Internal())
					chr.Source( !chr.Source(0).Internal() ).Fill( 0x00 );

				{
					static const uint defPrg[8][4] =
					{
						{  0U,  1U, ~1U, ~0U }, // PRG_01XX_16K
						{ ~1U, ~0U,  0U,  1U }, // PRG_XX01_16K
						{  0U,  1U,  0U,  1U }, // PRG_0101_16K
						{ ~1U, ~0U, ~1U, ~0U }, // PRG_XXXX_16K
						{  0U, ~2U, ~1U, ~0U }, // PRG_0XXX_24K
						{ ~3U, ~2U, ~1U, ~0U }, // PRG_XXXX_32K
						{  0U,  1U,  2U,  3U }, // PRG_0123_32K
						{  0U,  1U,  2U, ~0U }  // PRG_012X_8K
					};

					prg.SwapBanks<SIZE_8K,0x0000U>
					(
						defPrg[setup[id].prg][0],
						defPrg[setup[id].prg][1],
						defPrg[setup[id].prg][2],
						defPrg[setup[id].prg][3]
					);
				}

				{
					static const uchar defNmt[] =
					{
						Ppu::NMT_VERTICAL,
						Ppu::NMT_HORIZONTAL,
						Ppu::NMT_ZERO,
						Ppu::NMT_FOURSCREEN
					};

					ppu.SetMirroring
					(
						setup[id].nmt                    ? defNmt[setup[id].nmt-1] :
						mirroring == Ppu::NMT_CONTROLLED ? defNmt[setup[id].nmtd-1] :
                                                           mirroring
					);
				}

				chr.SwapBank<SIZE_8K,0x0000U>(0);
				wrk.SwapBank<SIZE_8K,0x0000U>(0);
			}

			SubReset( hard );
		}

		void Mapper::Map(WrkMapping mapping) const
		{
			switch (mapping)
			{
				case WRK_PEEK:           Map( 0x6000U, 0x7FFFU, &Mapper::Peek_Wrk_6 ); break;
				case WRK_POKE:           Map( 0x6000U, 0x7FFFU, &Mapper::Poke_Wrk_6 ); break;
				case WRK_PEEK_POKE:      Map( 0x6000U, 0x7FFFU, &Mapper::Peek_Wrk_6, &Mapper::Poke_Wrk_6 ); break;
				case WRK_SAFE_POKE:      Map( 0x6000U, 0x7FFFU, &Mapper::Poke_Wrk_Safe_6 ); break;
				case WRK_SAFE_PEEK:      Map( 0x6000U, 0x7FFFU, &Mapper::Peek_Wrk_Safe_6 ); break;
				case WRK_SAFE_PEEK_POKE: Map( 0x6000U, 0x7FFFU, &Mapper::Peek_Wrk_Safe_6, &Mapper::Poke_Wrk_Safe_6 ); break;
			}
		}

		dword Mapper::GetStateName() const
		{
			if (id <= 255)
			{
				return NES_STATE_CHUNK_ID('0' + (id / 100U),'0' + (id % 100U / 10U),'0' + (id % 10U),'\0');
			}
			else
			{
				NST_COMPILE_ASSERT
				(
					EXT_SUPER24IN1   == 256 &&
					EXT_8157         == 257 &&
					EXT_8237         == 258 &&
					EXT_WS           == 259 &&
					EXT_DREAMTECH01  == 260 &&
					EXT_CC21         == 261 &&
					EXT_KOF97        == 262 &&
					EXT_64IN1NR      == 263 &&
					EXT_STREETHEROES == 264 &&
					EXT_T262         == 265 &&
					EXT_FK23C        == 266 &&
					EXT_6035052      == 267 &&
					EXT_A65AS        == 268 &&
					EXT_EDU2000      == 269
				);

				static const dword chunks[] =
				{
					NES_STATE_CHUNK_ID('S','2','4','\0'),
					NES_STATE_CHUNK_ID('8','1','5','\0'),
					NES_STATE_CHUNK_ID('8','2','3','\0'),
					NES_STATE_CHUNK_ID('W','S','4','\0'),
					NES_STATE_CHUNK_ID('D','0','1','\0'),
					NES_STATE_CHUNK_ID('C','2','1','\0'),
					NES_STATE_CHUNK_ID('K','O','F','\0'),
					NES_STATE_CHUNK_ID('6','4','N','\0'),
					NES_STATE_CHUNK_ID('S','H','R','\0'),
					NES_STATE_CHUNK_ID('T','2','6','\0'),
					NES_STATE_CHUNK_ID('F','K','2','\0'),
					NES_STATE_CHUNK_ID('6','0','3','\0'),
					NES_STATE_CHUNK_ID('A','6','5','\0'),
					NES_STATE_CHUNK_ID('E','D','U','\0')
				};

				NST_COMPILE_ASSERT( NUM_EXT_MAPPERS == NST_COUNT(chunks) );

				return chunks[id - 256];
			}
		}

		void Mapper::SaveState(State::Saver& state) const
		{
			prg.SaveState( State::Saver::Subset(state,'P','R','G','\0').Ref(), b00 );
			chr.SaveState( State::Saver::Subset(state,'C','H','R','\0').Ref(), chr.Source(0).Internal() ? b01 : chr.Source(1).Internal() ? b10 : b00 );
			nmt.SaveState( State::Saver::Subset(state,'N','M','T','\0').Ref(), nmt.Source(0).Internal() ? b01 : nmt.Source(1).Internal() ? b10 : b00 );
			wrk.SaveState( State::Saver::Subset(state,'W','R','K','\0').Ref(), wrk.HasRam() ? b01 : b00 );
			BaseSave( state );
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

					default:

						BaseLoad( State::Loader::Subset(state).Ref(), chunk );
						break;
				}

				state.End();
			}
		}

		cstring Mapper::GetBoard(uint i)
		{
			return i < NST_COUNT(setup) ? setup[i].board : "";
		}

		#ifdef NST_PRAGMA_OPTIMIZE
		#pragma optimize("", on)
		#endif

		NES_PEEK(Mapper,Prg_8) { return prg[0][address - 0x8000U]; }
		NES_PEEK(Mapper,Prg_A) { return prg[1][address - 0xA000U]; }
		NES_PEEK(Mapper,Prg_C) { return prg[2][address - 0xC000U]; }
		NES_PEEK(Mapper,Prg_E) { return prg[3][address - 0xE000U]; }

		NES_POKE(Mapper,Prg_8k_0)  { prg.SwapBank<SIZE_8K,0x0000U>( data ); }
		NES_POKE(Mapper,Prg_8k_1)  { prg.SwapBank<SIZE_8K,0x2000U>( data ); }
		NES_POKE(Mapper,Prg_8k_2)  { prg.SwapBank<SIZE_8K,0x4000U>( data ); }
		NES_POKE(Mapper,Prg_8k_3)  { prg.SwapBank<SIZE_8K,0x6000U>( data ); }
		NES_POKE(Mapper,Prg_16k_0) { prg.SwapBank<SIZE_16K,0x0000U>( data ); }
		NES_POKE(Mapper,Prg_16k_1) { prg.SwapBank<SIZE_16K,0x4000U>( data ); }
		NES_POKE(Mapper,Prg_32k)   { prg.SwapBank<SIZE_32K,0x0000U>( data ); }

		NES_POKE(Mapper,Chr_1k_0) { ppu.Update(); chr.SwapBank<SIZE_1K,0x0000U>( data ); }
		NES_POKE(Mapper,Chr_1k_1) { ppu.Update(); chr.SwapBank<SIZE_1K,0x0400U>( data ); }
		NES_POKE(Mapper,Chr_1k_2) { ppu.Update(); chr.SwapBank<SIZE_1K,0x0800U>( data ); }
		NES_POKE(Mapper,Chr_1k_3) { ppu.Update(); chr.SwapBank<SIZE_1K,0x0C00U>( data ); }
		NES_POKE(Mapper,Chr_1k_4) { ppu.Update(); chr.SwapBank<SIZE_1K,0x1000U>( data ); }
		NES_POKE(Mapper,Chr_1k_5) { ppu.Update(); chr.SwapBank<SIZE_1K,0x1400U>( data ); }
		NES_POKE(Mapper,Chr_1k_6) { ppu.Update(); chr.SwapBank<SIZE_1K,0x1800U>( data ); }
		NES_POKE(Mapper,Chr_1k_7) { ppu.Update(); chr.SwapBank<SIZE_1K,0x1C00U>( data ); }
		NES_POKE(Mapper,Chr_2k_0) { ppu.Update(); chr.SwapBank<SIZE_2K,0x0000U>( data ); }
		NES_POKE(Mapper,Chr_2k_1) { ppu.Update(); chr.SwapBank<SIZE_2K,0x0800U>( data ); }
		NES_POKE(Mapper,Chr_2k_2) { ppu.Update(); chr.SwapBank<SIZE_2K,0x1000U>( data ); }
		NES_POKE(Mapper,Chr_2k_3) { ppu.Update(); chr.SwapBank<SIZE_2K,0x1800U>( data ); }
		NES_POKE(Mapper,Chr_4k_0) { ppu.Update(); chr.SwapBank<SIZE_4K,0x0000U>( data ); }
		NES_POKE(Mapper,Chr_4k_1) { ppu.Update(); chr.SwapBank<SIZE_4K,0x1000U>( data ); }
		NES_POKE(Mapper,Chr_8k)   { ppu.Update(); chr.SwapBank<SIZE_8K,0x0000U>( data ); }

		NES_POKE(Mapper,Wrk_6)
		{
			NST_VERIFY( wrk.Writable(0) );
			wrk[0][address - 0x6000U] = data;
		}

		NES_PEEK(Mapper,Wrk_6)
		{
			NST_VERIFY( wrk.Readable(0) );
			return wrk[0][address - 0x6000U];
		}

		NES_POKE(Mapper,Wrk_Safe_6)
		{
			NST_VERIFY( wrk.Writable(0) );

			if (wrk.Writable(0))
				wrk[0][address - 0x6000U] = data;
		}

		NES_PEEK(Mapper,Wrk_Safe_6)
		{
			NST_VERIFY( wrk.Readable(0) );
			return wrk.Readable(0) ? wrk[0][address - 0x6000U] : (address >> 8);
		}

		NES_POKE(Mapper,Nmt_Hv)
		{
			NST_COMPILE_ASSERT( Ppu::NMT_HORIZONTAL == 0 && Ppu::NMT_VERTICAL == 1 );
			NST_VERIFY( mirroring == Ppu::NMT_CONTROLLED || mirroring == Ppu::NMT_VERTICAL || mirroring == Ppu::NMT_HORIZONTAL );

			ppu.SetMirroring( ~data & 0x1 );
		}

		NES_POKE(Mapper,Nmt_Vh)
		{
			NST_COMPILE_ASSERT( Ppu::NMT_HORIZONTAL == 0 && Ppu::NMT_VERTICAL == 1 );
			NST_VERIFY( mirroring == Ppu::NMT_CONTROLLED || mirroring == Ppu::NMT_VERTICAL || mirroring == Ppu::NMT_HORIZONTAL );

			ppu.SetMirroring( data & 0x1 );
		}

		NES_POKE(Mapper,Nmt_Vh01)
		{
			static const uchar lut[4][4] =
			{
				{0,1,0,1},
				{0,0,1,1},
				{0,0,0,0},
				{1,1,1,1}
			};

			ppu.SetMirroring( lut[data & 0x3] );
		}

		NES_POKE(Mapper,Nmt_Hv01)
		{
			static const uchar lut[4][4] =
			{
				{0,0,1,1},
				{0,1,0,1},
				{0,0,0,0},
				{1,1,1,1}
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
