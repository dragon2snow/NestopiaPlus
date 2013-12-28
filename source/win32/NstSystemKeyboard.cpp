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

#include <algorithm>
#include "NstString.hpp"
#include "NstSystemKeyboard.hpp"
#include <Windows.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

namespace Nestopia
{
	using System::Keyboard;

	const Keyboard::Key Keyboard::keys[] =
	{
		{ "",             NONE,             NONE             },
		{ "'",            NONE,             DIK_APOSTROPHE   },
		{ "'\"",          VK_OEM_7,         NONE             },
		{ ",",            VK_OEM_COMMA,     DIK_COMMA        },
		{ "-",            VK_OEM_MINUS,     DIK_MINUS        },
		{ ".",            VK_OEM_PERIOD,    DIK_PERIOD       },
		{ "/",            NONE,             DIK_SLASH        },
		{ "/?",           VK_OEM_2,         NONE             },
		{ "0",            '0',              DIK_0            },		
		{ "1",            '1',              DIK_1            },		 
		{ "2",            '2',              DIK_2            },
		{ "3",            '3',              DIK_3            },
		{ "4",            '4',              DIK_4            },
		{ "5",            '5',              DIK_5            },
		{ "6",            '6',              DIK_6            },
		{ "7",            '7',              DIK_7            },
		{ "8",            '8',              DIK_8            },
		{ "9",            '9',              DIK_9            },
		{ ":",            NONE,             DIK_COLON        },
		{ ";",            NONE,             DIK_SEMICOLON    },
		{ ";:",           VK_OEM_1,         NONE             },
		{ "=",            NONE,             DIK_EQUALS       },
		{ "[",            NONE,             DIK_LBRACKET     },
		{ "[{",           VK_OEM_4,         NONE             },
		{ "\\",           NONE,             DIK_BACKSLASH    },
		{ "\\|",          VK_OEM_5,         NONE             },
		{ "]",            NONE,             DIK_RBRACKET     },
		{ "]}",           VK_OEM_6,         NONE             },
		{ "_",            NONE,             DIK_UNDERLINE    },
		{ "`~",           VK_OEM_3,         NONE             },
		{ "A",            'A',              DIK_A            },
		{ "ABNT C1",      NONE,             DIK_ABNT_C1      },
		{ "ABNT C2",      NONE,             DIK_ABNT_C2      },
		{ "ACCEPT",       VK_ACCEPT,        NONE             },
		{ "ALT",          VK_LMENU,         DIK_LMENU        },
		{ "ALT GR",       VK_RMENU,         DIK_RMENU        },
		{ "APPS",         VK_APPS,          DIK_APPS         },
		{ "AT",           NONE,             DIK_AT           },
		{ "ATTN",         VK_ATTN,          NONE             },
		{ "AX",           VK_OEM_AX,        DIK_AX           },
		{ "B",            'B',              DIK_B            },
		{ "BACK",         VK_BACK,          DIK_BACK         },
		{ "C",            'C',              DIK_C            },
		{ "CALCULATOR",   NONE,             DIK_CALCULATOR   },
		{ "CAPS-LOCK",    VK_CAPITAL,       DIK_CAPITAL      },
		{ "CLEAR",        VK_CLEAR,         NONE             },
		{ "CONTROL",      VK_CONTROL,       NONE             },
		{ "CONVERT",      VK_CONVERT,       DIK_CONVERT      },
		{ "CRSEL",        VK_CRSEL,         NONE             },
		{ "D",            'D',              DIK_D            },
		{ "DELETE",       VK_DELETE,        DIK_DELETE       },
		{ "DOWN",         VK_DOWN,          DIK_DOWN         },
		{ "E",            'E',              DIK_E            },
		{ "END",          VK_END,           DIK_END          },
		{ "EREOF",        VK_EREOF,         NONE             },
		{ "ESC",          VK_ESCAPE,        DIK_ESCAPE       },
		{ "EXECUTE",      VK_EXECUTE,       NONE             },
		{ "EXSEL",        VK_EXSEL,         NONE             },
		{ "F",            'F',              DIK_F            },
		{ "F1",           VK_F1,            DIK_F1           },
		{ "F10",          VK_F10,           DIK_F10          },
		{ "F11",          VK_F11,           DIK_F11          },
		{ "F12",          VK_F12,           DIK_F12          },
		{ "F13",          VK_F13,           DIK_F13          },
		{ "F14",          VK_F14,           DIK_F14          },
		{ "F15",          VK_F15,           DIK_F15          },
		{ "F16",          VK_F16,           NONE             },
		{ "F17",          VK_F17,           NONE             },
		{ "F18",          VK_F18,           NONE             },
		{ "F19",          VK_F19,           NONE             },
		{ "F2",           VK_F2,            DIK_F2           },
		{ "F20",          VK_F20,           NONE             },
		{ "F21",          VK_F21,           NONE             },
		{ "F22",          VK_F22,           NONE             },
		{ "F23",          VK_F23,           NONE             },
		{ "F24",          VK_F24,           NONE             },
		{ "F3",           VK_F3,            DIK_F3           },
		{ "F4",           VK_F4,            DIK_F4           },
		{ "F5",           VK_F5,            DIK_F5           },
		{ "F6",           VK_F6,            DIK_F6           },
		{ "F7",           VK_F7,            DIK_F7           },
		{ "F8",           VK_F8,            DIK_F8           },
		{ "F9",           VK_F9,            DIK_F9           },
		{ "FINAL",        VK_FINAL,         NONE             },
		{ "G",            'G',              DIK_G            },
		{ "H",            'H',              DIK_H            },
		{ "HELP",         VK_HELP,          NONE             },
		{ "HOME",         VK_HOME,          DIK_HOME         },
		{ "I",            'I',              DIK_I            },
		{ "ICO 00",       VK_ICO_00,        NONE             },
		{ "ICO HELP",     VK_ICO_HELP,      NONE             },
		{ "INSERT",       VK_INSERT,        DIK_INSERT       },
		{ "J",            'J',              DIK_J            },
		{ "JUNJA",        VK_JUNJA,         NONE             },
		{ "K",            'K',              DIK_K            },
		{ "KANA",         VK_KANA,          DIK_KANA         },
		{ "KANJI",        VK_KANJI,         DIK_KANJI        },
		{ "L",            'L',              DIK_L            },
		{ "LCTRL",        VK_LCONTROL,      DIK_LCONTROL     },
		{ "LEFT",         VK_LEFT,          DIK_LEFT         },
		{ "LSHIFT",       VK_LSHIFT,        DIK_LSHIFT       },
		{ "LWIN",         VK_LWIN,          DIK_LWIN         },
		{ "M",            'M',              DIK_M            },
		{ "MAIL",         NONE,             DIK_MAIL         },
		{ "MEDIASELECT",  NONE,             DIK_MEDIASELECT  },
		{ "MEDIASTOP",    NONE,             DIK_MEDIASTOP    },
		{ "MODECHANGE",   VK_MODECHANGE,    NONE             },
		{ "MUTE",         NONE,             DIK_MUTE         },
		{ "MYCOMPUTER",   NONE,             DIK_MYCOMPUTER   },
		{ "N",            'N',              DIK_N            },
		{ "NEXTTRACK",    NONE,             DIK_NEXTTRACK    },
		{ "NOCONVERT",    NONE,             DIK_NOCONVERT    },
		{ "NONAME",       VK_NONAME,        NONE             },
		{ "NONCONVERT",   VK_NONCONVERT,    NONE             },
		{ "NUMLOCK",      VK_NUMLOCK,       DIK_NUMLOCK      },
		{ "NUMPAD *",     VK_MULTIPLY,      DIK_NUMPADSTAR   },
		{ "NUMPAD +",     VK_ADD,           DIK_NUMPADPLUS   },
		{ "NUMPAD ,",     NONE,             DIK_NUMPADCOMMA  },
		{ "NUMPAD -",     VK_SUBTRACT,      DIK_NUMPADMINUS  },
		{ "NUMPAD .",     VK_DECIMAL,       DIK_NUMPADPERIOD },
		{ "NUMPAD /",     VK_DIVIDE,        DIK_NUMPADSLASH  },
		{ "NUMPAD 0",     VK_NUMPAD0,       DIK_NUMPAD0      },
		{ "NUMPAD 1",     VK_NUMPAD1,       DIK_NUMPAD1      },
		{ "NUMPAD 2",     VK_NUMPAD2,       DIK_NUMPAD2      },
		{ "NUMPAD 3",     VK_NUMPAD3,       DIK_NUMPAD3      },
		{ "NUMPAD 4",     VK_NUMPAD4,       DIK_NUMPAD4      },
		{ "NUMPAD 5",     VK_NUMPAD5,       DIK_NUMPAD5      },
		{ "NUMPAD 6",     VK_NUMPAD6,       DIK_NUMPAD6      },
		{ "NUMPAD 7",     VK_NUMPAD7,       DIK_NUMPAD7      },
		{ "NUMPAD 8",     VK_NUMPAD8,       DIK_NUMPAD8      },
		{ "NUMPAD 9",     VK_NUMPAD9,       DIK_NUMPAD9      },
		{ "NUMPAD =",     VK_OEM_NEC_EQUAL, DIK_NUMPADEQUALS },
		{ "NUMPAD ENTER", NONE,             DIK_NUMPADENTER  },
		{ "O",            'O',              DIK_O            },
		{ "OEM 102",      VK_OEM_102,       DIK_OEM_102      },
		{ "OEM ATTN",     VK_OEM_ATTN,      NONE             },
		{ "OEM AUTO",     VK_OEM_AUTO,      NONE             },
		{ "OEM BACKTAB",  VK_OEM_BACKTAB,   NONE             },
		{ "OEM CLEAR",    VK_OEM_CLEAR,     NONE             },
		{ "OEM COPY",     VK_OEM_COPY,      NONE             },
		{ "OEM CUSEL",    VK_OEM_CUSEL,     NONE             },
		{ "OEM ENLW",     VK_OEM_ENLW,      NONE             },
		{ "OEM FINISH",   VK_OEM_FINISH,    NONE             },
		{ "OEM JUMP",     VK_OEM_JUMP,      NONE             },
		{ "OEM PA1",      VK_OEM_PA1,       NONE             },
		{ "OEM PA2",      VK_OEM_PA2,       NONE             },
		{ "OEM PA3",      VK_OEM_PA3,       NONE             },
		{ "OEM RESET",    VK_OEM_RESET,     NONE             },
		{ "OEM WSCTRL",   VK_OEM_WSCTRL,    NONE             },
		{ "P",            'P',              DIK_P            },
		{ "PA1",          VK_PA1,           NONE             },
		{ "PAGE-DOWN",    VK_NEXT,          DIK_NEXT         },
		{ "PAGE-UP",      VK_PRIOR,         DIK_PRIOR        },
		{ "PAUSE",        VK_PAUSE,         DIK_PAUSE        },
		{ "PLAY",         VK_PLAY,          NONE             },
		{ "PLAYPAUSE",    NONE,             DIK_PLAYPAUSE    },
		{ "POWER",        NONE,             DIK_POWER        },
		{ "PREVTRACK",    NONE,             DIK_PREVTRACK    },
		{ "PRINT",        VK_PRINT,         NONE             },
		{ "PRTSCN",       VK_SNAPSHOT,      DIK_SYSRQ        },
		{ "Q",            'Q',              DIK_Q            },
		{ "R",            'R',              DIK_R            },
		{ "RCTRL",        VK_RCONTROL,      DIK_RCONTROL     },
		{ "RETURN",       VK_RETURN,        DIK_RETURN       },
		{ "RIGHT",        VK_RIGHT,         DIK_RIGHT        },
		{ "RSHIFT",       VK_RSHIFT,        DIK_RSHIFT       },
		{ "RWIN",         VK_RWIN,          DIK_RWIN         },
		{ "S",            'S',              DIK_S            },
		{ "SCROLL",       VK_SCROLL,        DIK_SCROLL       },
		{ "SELECT",       VK_SELECT,        NONE             },
		{ "SEPARATOR",    VK_SEPARATOR,     NONE             },
		{ "SHIFT",        VK_SHIFT,         NONE             },
		{ "SLEEP",        VK_SLEEP,         DIK_SLEEP        },
		{ "SPACE",        VK_SPACE,         DIK_SPACE        },
		{ "STOP",         NONE,             DIK_STOP         },
		{ "T",            'T',              DIK_T            },
		{ "TAB",          VK_TAB,           DIK_TAB          },
		{ "U",            'U',              DIK_U            },
		{ "UNLABELED",    NONE,             DIK_UNLABELED    },
		{ "UP",           VK_UP,            DIK_UP           },
		{ "V",            'V',              DIK_V            },
		{ "VOLUMEDOWN",   NONE,             DIK_VOLUMEDOWN   },
		{ "VOLUMEUP",     NONE,             DIK_VOLUMEUP     },
		{ "W",            'W',              DIK_W            },
		{ "WAKE",         NONE,             DIK_WAKE         },
		{ "WEBBACK",      NONE,             DIK_WEBBACK      },
		{ "WEBFAVORITES", NONE,             DIK_WEBFAVORITES },
		{ "WEBFORWARD",   NONE,             DIK_WEBFORWARD   },
		{ "WEBHOME",      NONE,             DIK_WEBHOME      },
		{ "WEBREFRESH",   NONE,             DIK_WEBREFRESH   },
		{ "WEBSEARCH",    NONE,             DIK_WEBSEARCH    },
		{ "WEBSTOP",      NONE,             DIK_WEBSTOP      },
		{ "X",            'X',              DIK_X            },
		{ "Y",            'Y',              DIK_Y            },
		{ "YEN",          NONE,             DIK_YEN          },
		{ "Z",            'Z',              DIK_Z            },
		{ "ZOOM",         VK_ZOOM,          NONE             },
		{ "§",            NONE,             DIK_GRAVE        }
	};

	const uchar Keyboard::table[NUM_SETS][NUM_KEYS] =
	{
		{	
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			41,
			176,
			0,
			0,
			45,
			163,
			0,
			0,
			171,
			46,
			34,
			153,
			44,
			95,
			0,
			93,
			83,
			96,
			0,
			55,
			47,
			113,
			33,
			106,
			173,
			152,
			151,
			53,
			87,
			99,
			179,
			164,
			51,
			169,
			158,
			56,
			159,
			91,
			50,
			86,
			8,
			9,
			10,
			11,
			12,
			13,
			14,
			15,
			16,
			17,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			30,
			40,
			42,
			49,
			52,
			58,
			84,
			85,
			88,
			92,
			94,
			97,
			102,
			109,
			133,
			149,
			160,
			161,
			167,
			175,
			177,
			180,
			183,
			192,
			193,
			195,
			101,
			166,
			36,
			0,
			172,
			121,
			122,
			123,
			124,
			125,
			126,
			127,
			128,
			129,
			130,
			115,
			116,
			170,
			118,
			119,
			120,
			59,
			70,
			76,
			77,
			78,
			79,
			80,
			81,
			82,
			60,
			61,
			62,
			63,
			64,
			65,
			66,
			67,
			68,
			69,
			71,
			72,
			73,
			74,
			75,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			114,
			168,
			131,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			100,
			165,
			98,
			162,
			34,
			35,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			20,
			116,
			3,
			4,
			5,
			7,
			29,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			23,
			25,
			27,
			2,
			0,
			0,
			39,
			134,
			90,
			89,
			0,
			0,
			0,
			0,
			147,
			143,
			144,
			145,
			146,
			148,
			140,
			135,
			142,
			139,
			136,
			141,
			137,
			38,
			48,
			57,
			54,
			154,
			196,
			112,
			150,
			138,
			0
		},
		{
			0,
			55,
			9,
			10,
			11,
			12,
			13,
			14,
			15,
			16,
			17,
			8,
			4,
			21,
			41,
			176,
			160,
			183,
			52,
			161,
			175,
			193,
			177,
			88,
			133,
			149,
			22,
			26,
			163,
			98,
			30,
			167,
			49,
			58,
			84,
			85,
			92,
			94,
			97,
			19,
			1,
			197,
			100,
			24,
			195,
			192,
			42,
			180,
			40,
			109,
			102,
			3,
			5,
			6,
			165,
			115,
			34,
			173,
			44,
			59,
			70,
			76,
			77,
			78,
			79,
			80,
			81,
			82,
			60,
			114,
			168,
			128,
			129,
			130,
			118,
			125,
			126,
			127,
			116,
			122,
			123,
			124,
			121,
			119,
			0,
			0,
			134,
			61,
			62,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			63,
			64,
			65,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			95,
			0,
			0,
			31,
			0,
			0,
			0,
			0,
			0,
			47,
			0,
			111,
			0,
			194,
			32,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			131,
			0,
			0,
			157,
			37,
			18,
			28,
			96,
			174,
			39,
			178,
			0,
			110,
			0,
			0,
			132,
			162,
			0,
			0,
			107,
			43,
			155,
			0,
			105,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			181,
			0,
			182,
			0,
			188,
			117,
			0,
			120,
			0,
			159,
			35,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			153,
			0,
			87,
			179,
			152,
			0,
			99,
			0,
			164,
			0,
			53,
			51,
			151,
			91,
			50,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			101,
			166,
			36,
			156,
			172,
			0,
			0,
			0,
			184,
			0,
			190,
			186,
			189,
			191,
			187,
			185,
			108,
			103,
			104,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0
		}
	};

	const uchar Keyboard::converter[NUM_SETS][NUM_KEYS] =
	{
		{
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			14,
			15,
			0,
			0,
			0,
			28,
			0,
			0,
			0,
			0,
			56,
			197,
			58,
			112,
			0,
			0,
			0,
			148,
			0,
			1,
			121,
			0,
			0,
			0,
			57,
			201,
			209,
			207,
			199,
			203,
			200,
			205,
			208,
			0,
			0,
			0,
			183,
			210,
			211,
			0,
			11,
			2,
			3,
			4,
			5,
			6,
			7,
			8,
			9,
			10,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			30,
			48,
			46,
			32,
			18,
			33,
			34,
			35,
			23,
			36,
			37,
			38,
			50,
			49,
			24,
			25,
			16,
			19,
			31,
			20,
			22,
			47,
			17,
			45,
			21,
			44,
			219,
			220,
			221,
			0,
			223,
			82,
			79,
			80,
			81,
			75,
			76,
			77,
			71,
			72,
			73,
			55,
			78,
			0,
			74,
			83,
			181,
			59,
			60,
			61,
			62,
			63,
			64,
			65,
			66,
			67,
			68,
			87,
			88,
			100,
			101,
			102,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			69,
			70,
			141,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			42,
			54,
			29,
			157,
			56,
			184,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			78,
			51,
			12,
			52,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			150,
			86,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0
		},
		{
			0,
			27,
			49,
			50,
			51,
			52,
			53,
			54,
			55,
			56,
			57,
			48,
			189,
			0,
			8,
			9,
			81,
			87,
			69,
			82,
			84,
			89,
			85,
			73,
			79,
			80,
			0,
			0,
			13,
			162,
			65,
			83,
			68,
			70,
			71,
			72,
			74,
			75,
			76,
			0,
			0,
			0,
			160,
			0,
			90,
			88,
			67,
			86,
			66,
			78,
			77,
			188,
			190,
			0,
			161,
			106,
			18,
			32,
			20,
			112,
			113,
			114,
			115,
			116,
			117,
			118,
			119,
			120,
			121,
			144,
			145,
			103,
			104,
			105,
			109,
			100,
			101,
			102,
			187,
			97,
			98,
			99,
			96,
			110,
			0,
			0,
			226,
			122,
			123,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			124,
			125,
			126,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			21,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			28,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			146,
			0,
			0,
			0,
			0,
			0,
			0,
			25,
			0,
			225,
			0,
			0,
			0,
			0,
			0,
			0,
			163,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			111,
			0,
			44,
			165,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			19,
			0,
			36,
			38,
			33,
			0,
			37,
			0,
			39,
			0,
			35,
			40,
			34,
			45,
			46,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			91,
			92,
			93,
			0,
			95,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0
		}
	};

	NST_COMPILE_ASSERT
	(
		VK_OEM_7         == 222 &&
		VK_OEM_COMMA     == 188 &&
		VK_OEM_MINUS     == 189 &&
		VK_OEM_PERIOD    == 190 &&
		VK_OEM_2         == 191 &&
		'0'              == 48  &&
		'1'              == 49  &&
		'2'              == 50  &&
		'3'              == 51  &&
		'4'              == 52  &&
		'5'              == 53  &&
		'6'              == 54  &&
		'7'              == 55  &&
		'8'              == 56  &&
		'9'              == 57  &&
		VK_OEM_1         == 186 &&
		VK_OEM_4         == 219 &&
		VK_OEM_5         == 220 &&
		VK_OEM_6         == 221 &&
		VK_OEM_3         == 192 &&
		'A'              == 65  &&
		VK_LMENU         == 164 &&
		VK_RMENU         == 165 &&
		VK_APPS          == 93  &&
		VK_ATTN          == 246 &&
		VK_OEM_AX        == 225 &&
		'B'              == 66  &&
		VK_BACK          == 8   &&
		'C'              == 67  &&
		VK_CAPITAL       == 20  &&
		VK_CLEAR         == 12  &&
		VK_CONTROL       == 17  &&
		VK_CONVERT       == 28  &&
		VK_CRSEL         == 247 &&
		'D'              == 68  &&
		VK_DELETE        == 46  &&
		VK_DOWN          == 40  &&
		'E'              == 69  &&
		VK_END           == 35  &&
		VK_EREOF         == 249 &&
		VK_ESCAPE        == 27  &&
		VK_EXECUTE       == 43  &&
		VK_EXSEL         == 248 &&
		'F'              == 70  &&
		VK_F1            == 112 &&
		VK_F10           == 121 &&
		VK_F11           == 122 &&
		VK_F12           == 123 &&
		VK_F13           == 124 &&
		VK_F14           == 125 &&
		VK_F15           == 126 &&
		VK_F16           == 127 &&
		VK_F17           == 128 &&
		VK_F18           == 129 &&
		VK_F19           == 130 &&
		VK_F2            == 113 &&
		VK_F20           == 131 &&
		VK_F21           == 132 &&
		VK_F22           == 133 &&
		VK_F23           == 134 &&
		VK_F24           == 135 &&
		VK_F3            == 114 &&
		VK_F4            == 115 &&
		VK_F5            == 116 &&
		VK_F6            == 117 &&
		VK_F7            == 118 &&
		VK_F8            == 119 &&
		VK_F9            == 120 &&
		VK_FINAL         == 24  &&
		'G'              == 71  &&
		'H'              == 72  &&
		VK_HELP          == 47  &&
		VK_HOME          == 36  &&
		'I'              == 73  &&
		VK_ICO_00        == 228 &&
		VK_ICO_HELP      == 227 &&
		VK_INSERT        == 45  &&
		'J'              == 74  &&
		VK_JUNJA         == 23  &&
		'K'              == 75  &&
		VK_KANA          == 21  &&
		VK_KANJI         == 25  &&
		'L'              == 76  &&
		VK_LCONTROL      == 162 &&
		VK_LEFT          == 37  &&
		VK_LSHIFT        == 160 &&
		VK_LWIN          == 91  &&
		'M'              == 77  &&
		VK_MODECHANGE    == 31  &&
		'N'              == 78  &&
		VK_NONAME        == 252 &&
		VK_NONCONVERT    == 29  &&
		VK_NUMLOCK       == 144 &&
		VK_MULTIPLY      == 106 &&
		VK_ADD           == 107 &&
		VK_SUBTRACT      == 109 &&
		VK_DECIMAL       == 110 &&
		VK_DIVIDE        == 111 &&
		VK_NUMPAD0       == 96  &&
		VK_NUMPAD1       == 97  &&
		VK_NUMPAD2       == 98  &&
		VK_NUMPAD3       == 99  &&
		VK_NUMPAD4       == 100 &&
		VK_NUMPAD5       == 101 &&
		VK_NUMPAD6       == 102 &&
		VK_NUMPAD7       == 103 &&
		VK_NUMPAD8       == 104 &&
		VK_NUMPAD9       == 105 &&
		VK_OEM_NEC_EQUAL == 146 &&
		'O'              == 79  &&
		VK_OEM_102       == 226 &&
		VK_OEM_ATTN      == 240 &&
		VK_OEM_AUTO      == 243 &&
		VK_OEM_BACKTAB   == 245 &&
		VK_OEM_CLEAR     == 254 &&
		VK_OEM_COPY      == 242 &&
		VK_OEM_CUSEL     == 239 &&
		VK_OEM_ENLW      == 244 &&
		VK_OEM_FINISH    == 241 &&
		VK_OEM_JUMP      == 234 &&
		VK_OEM_PA1       == 235 &&
		VK_OEM_PA2       == 236 &&
		VK_OEM_PA3       == 237 &&
		VK_OEM_RESET     == 233 &&
		VK_OEM_WSCTRL    == 238 &&
		'P'              == 80  &&
		VK_PA1           == 253 &&
		VK_NEXT          == 34  &&
		VK_PRIOR         == 33  &&
		VK_PAUSE         == 19  &&
		VK_PLAY          == 250 &&
		VK_PRINT         == 42  &&
		VK_SNAPSHOT      == 44  &&
		'Q'              == 81  &&
		'R'              == 82  &&
		VK_RCONTROL      == 163 &&
		VK_RETURN        == 13  &&
		VK_RIGHT         == 39  &&
		VK_RSHIFT        == 161 &&
		VK_RWIN          == 92  &&
		'S'              == 83  &&
		VK_SCROLL        == 145 &&
		VK_SELECT        == 41  &&
		VK_SEPARATOR     == 108 &&
		VK_SHIFT         == 16  &&
		VK_SLEEP         == 95  &&
		VK_SPACE         == 32  &&
		'T'              == 84  &&
		VK_TAB           == 9   &&
		'U'              == 85  &&
		VK_UP            == 38  &&
		'V'              == 86  &&
		'W'              == 87  &&
		'X'              == 88  &&
		'Y'              == 89  &&
		'Z'              == 90  &&
		VK_ZOOM          == 251
	);

	NST_COMPILE_ASSERT
	(
		DIK_APOSTROPHE   == 40  &&
		DIK_COMMA        == 51  &&
		DIK_MINUS        == 12  &&
		DIK_PERIOD       == 52  &&
		DIK_SLASH        == 53  &&
		DIK_0            == 11  &&
		DIK_1            == 2   &&
		DIK_2            == 3   &&
		DIK_3            == 4   &&
		DIK_4            == 5   &&
		DIK_5            == 6   &&
		DIK_6            == 7   &&
		DIK_7            == 8   &&
		DIK_8            == 9   &&
		DIK_9            == 10  &&
		DIK_COLON        == 146 &&
		DIK_SEMICOLON    == 39  &&
		DIK_EQUALS       == 13  &&
		DIK_LBRACKET     == 26  &&
		DIK_BACKSLASH    == 43  &&
		DIK_RBRACKET     == 27  &&
		DIK_UNDERLINE    == 147 &&
		DIK_A            == 30  &&
		DIK_ABNT_C1      == 115 &&
		DIK_ABNT_C2      == 126 &&
		DIK_LMENU        == 56  &&
		DIK_RMENU        == 184 &&
		DIK_APPS         == 221 &&
		DIK_AT           == 145 &&
		DIK_AX           == 150 &&
		DIK_B            == 48  &&
		DIK_BACK         == 14  &&
		DIK_C            == 46  &&
		DIK_CALCULATOR   == 161 &&
		DIK_CAPITAL      == 58  &&
		DIK_CONVERT      == 121 &&
		DIK_D            == 32  &&
		DIK_DELETE       == 211 &&
		DIK_DOWN         == 208 &&
		DIK_E            == 18  &&
		DIK_END          == 207 &&
		DIK_ESCAPE       == 1   &&
		DIK_F            == 33  &&
		DIK_F1           == 59  &&
		DIK_F10          == 68  &&
		DIK_F11          == 87  &&
		DIK_F12          == 88  &&
		DIK_F13          == 100 &&
		DIK_F14          == 101 &&
		DIK_F15          == 102 &&
		DIK_F2           == 60  &&
		DIK_F3           == 61  &&
		DIK_F4           == 62  &&
		DIK_F5           == 63  &&
		DIK_F6           == 64  &&
		DIK_F7           == 65  &&
		DIK_F8           == 66  &&
		DIK_F9           == 67  &&
		DIK_G            == 34  &&
		DIK_H            == 35  &&
		DIK_HOME         == 199 &&
		DIK_I            == 23  &&
		DIK_INSERT       == 210 &&
		DIK_J            == 36  &&
		DIK_K            == 37  &&
		DIK_KANA         == 112 &&
		DIK_KANJI        == 148 &&
		DIK_L            == 38  &&
		DIK_LCONTROL     == 29  &&
		DIK_LEFT         == 203 &&
		DIK_LSHIFT       == 42  &&
		DIK_LWIN         == 219 &&
		DIK_M            == 50  &&
		DIK_MAIL         == 236 &&
		DIK_MEDIASELECT  == 237 &&
		DIK_MEDIASTOP    == 164 &&
		DIK_MUTE         == 160 &&
		DIK_MYCOMPUTER   == 235 &&
		DIK_N            == 49  &&
		DIK_NEXTTRACK    == 153 &&
		DIK_NOCONVERT    == 123 &&
		DIK_NUMLOCK      == 69  &&
		DIK_NUMPADSTAR   == 55  &&
		DIK_NUMPADPLUS   == 78  &&
		DIK_NUMPADCOMMA  == 179 &&
		DIK_NUMPADMINUS  == 74  &&
		DIK_NUMPADPERIOD == 83  &&
		DIK_NUMPADSLASH  == 181 &&
		DIK_NUMPAD0      == 82  &&
		DIK_NUMPAD1      == 79  &&
		DIK_NUMPAD2      == 80  &&
		DIK_NUMPAD3      == 81  &&
		DIK_NUMPAD4      == 75  &&
		DIK_NUMPAD5      == 76  &&
		DIK_NUMPAD6      == 77  &&
		DIK_NUMPAD7      == 71  &&
		DIK_NUMPAD8      == 72  &&
		DIK_NUMPAD9      == 73  &&
		DIK_NUMPADEQUALS == 141 &&
		DIK_NUMPADENTER  == 156 &&
		DIK_O            == 24  &&
		DIK_OEM_102      == 86  &&
		DIK_P            == 25  &&
		DIK_NEXT         == 209 &&
		DIK_PRIOR        == 201 &&
		DIK_PAUSE        == 197 &&
		DIK_PLAYPAUSE    == 162 &&
		DIK_POWER        == 222 &&
		DIK_PREVTRACK    == 144 &&
		DIK_SYSRQ        == 183 &&
		DIK_Q            == 16  &&
		DIK_R            == 19  &&
		DIK_RCONTROL     == 157 &&
		DIK_RETURN       == 28  &&
		DIK_RIGHT        == 205 &&
		DIK_RSHIFT       == 54  &&
		DIK_RWIN         == 220 &&
		DIK_S            == 31  &&
		DIK_SCROLL       == 70  &&
		DIK_SLEEP        == 223 &&
		DIK_SPACE        == 57  &&
		DIK_STOP         == 149 &&
		DIK_T            == 20  &&
		DIK_TAB          == 15  &&
		DIK_U            == 22  &&
		DIK_UNLABELED    == 151 &&
		DIK_UP           == 200 &&
		DIK_V            == 47  &&
		DIK_VOLUMEDOWN   == 174 &&
		DIK_VOLUMEUP     == 176 &&
		DIK_W            == 17  &&
		DIK_WAKE         == 227 &&
		DIK_WEBBACK      == 234 &&
		DIK_WEBFAVORITES == 230 &&
		DIK_WEBFORWARD   == 233 &&
		DIK_WEBHOME      == 178 &&
		DIK_WEBREFRESH   == 231 &&
		DIK_WEBSEARCH    == 229 &&
		DIK_WEBSTOP      == 232 &&
		DIK_X            == 45  &&
		DIK_Y            == 21  &&
		DIK_YEN          == 125 &&
		DIK_Z            == 44  &&
		DIK_GRAVE        == 41
	);

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("t", on)
    #endif

	inline ibool Keyboard::Key::operator < (const Key& key) const
	{
		return String::Compare( name, key.name ) < 0;
	}

	const Keyboard::Key* Keyboard::Locate(cstring const name)
	{
		if (name && *name)
		{
			const Key entry = {name,0,0};
			const Key* const key = std::lower_bound( keys, keys + NST_COUNT(keys), entry );

			if (key != keys + NST_COUNT(keys) && String::Compare( key->name, name ) == 0) 
				return key;
		}

		return NULL;
	}

	uint Keyboard::VikKey(cstring const name)
	{
		if (const Key* const key = Locate( name ))
			return key->vik;

		return NONE;
	}

	uint Keyboard::DikKey(cstring const name)
	{
		if (const Key* const key = Locate( name ))
			return key->dik;

		return NONE;
	}

	bool Keyboard::ToggleIndicator(const Indicator indicator,const bool on)
	{
		uint key = indicator;

		switch (indicator)
		{
     		case CAPS_LOCK:   key = VK_CAPITAL; break;
			case NUM_LOCK:    key = VK_NUMLOCK; break;
			case SCROLL_LOCK: key = VK_SCROLL;  break;
		}

		if (on != (::GetKeyState( key ) & 0x1))
		{
			INPUT input[2];
			std::memset( input, 0, sizeof(input) );

			input[1].type = input[0].type = INPUT_KEYBOARD;
			input[1].ki.wVk = input[0].ki.wVk = (WORD) key;

			input[1].ki.dwFlags = KEYEVENTF_EXTENDEDKEY;
			::SendInput( 2, input, sizeof(INPUT) );

			input[1].ki.dwFlags = KEYEVENTF_EXTENDEDKEY|KEYEVENTF_KEYUP;
			::SendInput( 2, input, sizeof(INPUT) );
			
			return true;
		}

		return false;
	}

    #ifdef NST_PRAGMA_OPTIMIZE
    #pragma optimize("", on)
    #endif
}
