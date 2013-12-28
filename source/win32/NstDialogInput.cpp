////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES / Famicom emulator written in C++
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

#include "NstResourceString.hpp"
#include "NstIoLog.hpp"
#include "NstApplicationConfiguration.hpp"
#include "NstManagerEmulator.hpp"
#include "NstSystemKeyboard.hpp"
#include "NstWindowParam.hpp"
#include "NstWindowDynamic.hpp"
#include "NstWindowUser.hpp"
#include "NstDialogInput.hpp"
#include <Shlwapi.h>

namespace Nestopia
{
	NST_COMPILE_ASSERT
	(
		DirectX::DirectInput::AXIS_X        == 0x001 &&
		DirectX::DirectInput::AXIS_Y        == 0x002 &&       
		DirectX::DirectInput::AXIS_Z        == 0x004 &&       
		DirectX::DirectInput::AXIS_RX       == 0x008 &&      
		DirectX::DirectInput::AXIS_RY       == 0x010 &&      
		DirectX::DirectInput::AXIS_RZ       == 0x020 &&      
		DirectX::DirectInput::AXIS_SLIDER_0 == 0x040 &&
		DirectX::DirectInput::AXIS_SLIDER_1 == 0x080 &&
		DirectX::DirectInput::AXIS_POV_0    == 0x100 &&	  
		DirectX::DirectInput::AXIS_POV_1    == 0x200 &&	  
		DirectX::DirectInput::AXIS_POV_2    == 0x400 &&	  
		DirectX::DirectInput::AXIS_POV_3	== 0x800
	);

	NST_COMPILE_ASSERT
	(
		IDC_INPUT_JOYSTICKS              == IDC_INPUT_JOYSTICKS +  0 && 
		IDC_INPUT_JOYSTICKS_ENABLE       == IDC_INPUT_JOYSTICKS +  1 && 
		IDC_INPUT_JOYSTICKS_X            == IDC_INPUT_JOYSTICKS +  2 &&
		IDC_INPUT_JOYSTICKS_Y            == IDC_INPUT_JOYSTICKS +  3 &&    
		IDC_INPUT_JOYSTICKS_Z            == IDC_INPUT_JOYSTICKS +  4 &&    
		IDC_INPUT_JOYSTICKS_RX           == IDC_INPUT_JOYSTICKS +  5 &&   
		IDC_INPUT_JOYSTICKS_RY           == IDC_INPUT_JOYSTICKS +  6 &&   
		IDC_INPUT_JOYSTICKS_RZ           == IDC_INPUT_JOYSTICKS +  7 &&   
		IDC_INPUT_JOYSTICKS_S0           == IDC_INPUT_JOYSTICKS +  8 &&
		IDC_INPUT_JOYSTICKS_S1           == IDC_INPUT_JOYSTICKS +  9 &&
		IDC_INPUT_JOYSTICKS_POV0         == IDC_INPUT_JOYSTICKS + 10 &&
		IDC_INPUT_JOYSTICKS_POV1         == IDC_INPUT_JOYSTICKS + 11 &&
		IDC_INPUT_JOYSTICKS_POV2         == IDC_INPUT_JOYSTICKS + 12 &&
		IDC_INPUT_JOYSTICKS_POV3         == IDC_INPUT_JOYSTICKS + 13 &&
		IDC_INPUT_JOYSTICKS_DEADZONE     == IDC_INPUT_JOYSTICKS + 14 &&
		IDC_INPUT_JOYSTICKS_DEADZONE_NUM == IDC_INPUT_JOYSTICKS + 15 && 
		IDC_INPUT_JOYSTICKS_DEFAULT      == IDC_INPUT_JOYSTICKS + 16
	);

	using namespace Window;

	const Input::Settings::Type Input::Settings::types[OFFSET_COUNT] =
	{
		{ PAD1_KEYS,           _T( "Pad 1"		     ) },
		{ PAD2_KEYS,           _T( "Pad 2"		     ) },
		{ PAD3_KEYS,           _T( "Pad 3"           ) },
		{ PAD4_KEYS,           _T( "Pad 4"		     ) },
		{ POWERPAD_KEYS,       _T( "Power Pad"       ) },
		{ CRAZYCLIMBER_KEYS,   _T( "Crazy Climber"   ) },
		{ MAHJONG_KEYS,        _T( "Mahjong"         ) },
		{ EXCITINGBOXING_KEYS, _T( "Exciting Boxing" ) },
		{ POKKUNMOGURAA_KEYS,  _T( "Pokkun Moguraa"  ) },
		{ PARTYTAP_KEYS,       _T( "Party Tap"       ) },
		{ EMULATION_KEYS,      _T( "Emulation"	     ) },
		{ FILE_KEYS,           _T( "File"			 ) },
		{ MACHINE_KEYS,        _T( "Machine"	     ) },
		{ NSF_KEYS,            _T( "Nsf"		     ) },
		{ VIEW_KEYS,           _T( "View"     		 ) },
		{ HELP_KEYS,           _T( "Help"     		 ) },
		{ NUM_KEYS,			   _T( ""				 ) }
	};

	const Input::Settings::Mapping Input::Settings::map[NUM_KEYS] =
	{
		{ PAD1_KEYS + PAD_KEY_LEFT,       DIK_LEFT,   _T( "Left"        ), "input pad1 left"       },
		{ PAD1_KEYS + PAD_KEY_UP,         DIK_UP,     _T( "Up"          ), "input pad1 up"         },
		{ PAD1_KEYS + PAD_KEY_RIGHT,      DIK_RIGHT,  _T( "Right"       ), "input pad1 right"      },
		{ PAD1_KEYS + PAD_KEY_DOWN,       DIK_DOWN,   _T( "Down"        ), "input pad1 down"       },
		{ PAD1_KEYS + PAD_KEY_SELECT,     DIK_RSHIFT, _T( "Select"      ), "input pad1 select"     },
		{ PAD1_KEYS + PAD_KEY_START,      DIK_RETURN, _T( "Start"       ), "input pad1 start"      },
		{ PAD1_KEYS + PAD_KEY_B,          DIK_COMMA,  _T( "B"           ), "input pad1 b"          },
		{ PAD1_KEYS + PAD_KEY_A,          DIK_PERIOD, _T( "A"           ), "input pad1 a"          },
		{ PAD1_KEYS + PAD_KEY_AUTOFIRE_B, DIK_K,	  _T( "Auto-Fire B" ), "input pad1 autofire b" },
		{ PAD1_KEYS + PAD_KEY_AUTOFIRE_A, DIK_L,	  _T( "Auto-Fire A" ), "input pad1 autofire a" },
		{ PAD1_KEYS + PAD_KEY_MIC,        NO_KEY,	  _T( "Mic"         ), "input pad1 mic"        },

		{ PAD2_KEYS + PAD_KEY_LEFT,       DIK_C,      _T( "Left"        ), "input pad2 left"       },
		{ PAD2_KEYS + PAD_KEY_UP,         DIK_F,      _T( "Up"          ), "input pad2 up"         },
		{ PAD2_KEYS + PAD_KEY_RIGHT,      DIK_B,      _T( "Right"       ), "input pad2 right"      },
		{ PAD2_KEYS + PAD_KEY_DOWN,       DIK_V,      _T( "Down"        ), "input pad2 down"       },
		{ PAD2_KEYS + PAD_KEY_SELECT,     DIK_A,      _T( "Select"      ), "input pad2 select"     },
		{ PAD2_KEYS + PAD_KEY_START,      DIK_S,      _T( "Start"       ), "input pad2 start"      },
		{ PAD2_KEYS + PAD_KEY_B,          DIK_Z,      _T( "B"           ), "input pad2 b"          },
		{ PAD2_KEYS + PAD_KEY_A,          DIK_X,      _T( "A"           ), "input pad2 a"          },
		{ PAD2_KEYS + PAD_KEY_AUTOFIRE_B, DIK_Q,      _T( "Auto-Fire B" ), "input pad2 autofire b" },
		{ PAD2_KEYS + PAD_KEY_AUTOFIRE_A, DIK_W,      _T( "Auto-Fire A" ), "input pad2 autofire a" },
		{ PAD2_KEYS + PAD_KEY_MIC,        DIK_M,	  _T( "Mic"         ), "input pad2 mic"        },

		{ PAD3_KEYS + PAD_KEY_LEFT,       NO_KEY,     _T( "Left"        ), "input pad3 left"       },
		{ PAD3_KEYS + PAD_KEY_UP,         NO_KEY,     _T( "Up"          ), "input pad3 up"         },
		{ PAD3_KEYS + PAD_KEY_RIGHT,      NO_KEY,     _T( "Right"       ), "input pad3 right"      },
		{ PAD3_KEYS + PAD_KEY_DOWN,       NO_KEY,     _T( "Down"        ), "input pad3 down"       },
		{ PAD3_KEYS + PAD_KEY_SELECT,     NO_KEY,     _T( "Select"      ), "input pad3 select"     },
		{ PAD3_KEYS + PAD_KEY_START,      NO_KEY,     _T( "Start"       ), "input pad3 start"      },
		{ PAD3_KEYS + PAD_KEY_B,          NO_KEY,     _T( "B"           ), "input pad3 b"          },
		{ PAD3_KEYS + PAD_KEY_A,          NO_KEY,     _T( "A"           ), "input pad3 a"          },
		{ PAD3_KEYS + PAD_KEY_AUTOFIRE_B, NO_KEY,     _T( "Auto-Fire B" ), "input pad3 autofire b" },
		{ PAD3_KEYS + PAD_KEY_AUTOFIRE_A, NO_KEY,     _T( "Auto-Fire A" ), "input pad3 autofire a" },
		{ PAD3_KEYS + PAD_KEY_MIC,        NO_KEY,	  _T( "Mic"         ), "input pad3 mic"        },

		{ PAD4_KEYS + PAD_KEY_LEFT,       NO_KEY,     _T( "Left"        ), "input pad4 left"       },
		{ PAD4_KEYS + PAD_KEY_UP,         NO_KEY,     _T( "Up"          ), "input pad4 up"         },
		{ PAD4_KEYS + PAD_KEY_RIGHT,      NO_KEY,     _T( "Right"       ), "input pad4 right"      },
		{ PAD4_KEYS + PAD_KEY_DOWN,       NO_KEY,     _T( "Down"        ), "input pad4 down"       },
		{ PAD4_KEYS + PAD_KEY_SELECT,     NO_KEY,     _T( "Select"      ), "input pad4 select"     },
		{ PAD4_KEYS + PAD_KEY_START,      NO_KEY,     _T( "Start"       ), "input pad4 start"      },
		{ PAD4_KEYS + PAD_KEY_B,          NO_KEY,     _T( "B"           ), "input pad4 b"          },
		{ PAD4_KEYS + PAD_KEY_A,          NO_KEY,     _T( "A"           ), "input pad4 a"          },
		{ PAD4_KEYS + PAD_KEY_AUTOFIRE_B, NO_KEY,     _T( "Auto-Fire B" ), "input pad4 autofire b" },
		{ PAD4_KEYS + PAD_KEY_AUTOFIRE_A, NO_KEY,     _T( "Auto-Fire A" ), "input pad4 autofire a" },
		{ PAD4_KEYS + PAD_KEY_MIC,        NO_KEY,	  _T( "Mic"         ), "input pad4 mic"        },

		{ POWERPAD_KEYS + POWERPAD_KEY_SIDE_A_1,  DIK_Q, _T( "Side A 1"  ), "input powerpad side a 1"  },
		{ POWERPAD_KEYS + POWERPAD_KEY_SIDE_A_2,  DIK_W, _T( "Side A 2"  ), "input powerpad side a 2"  },
		{ POWERPAD_KEYS + POWERPAD_KEY_SIDE_A_3,  DIK_E, _T( "Side A 3"  ), "input powerpad side a 3"  },
		{ POWERPAD_KEYS + POWERPAD_KEY_SIDE_A_4,  DIK_R, _T( "Side A 4"  ), "input powerpad side a 4"  },
		{ POWERPAD_KEYS + POWERPAD_KEY_SIDE_A_5,  DIK_A, _T( "Side A 5"  ), "input powerpad side a 5"  },
		{ POWERPAD_KEYS + POWERPAD_KEY_SIDE_A_6,  DIK_S, _T( "Side A 6"  ), "input powerpad side a 6"  },
		{ POWERPAD_KEYS + POWERPAD_KEY_SIDE_A_7,  DIK_D, _T( "Side A 7"  ), "input powerpad side a 7"  },
		{ POWERPAD_KEYS + POWERPAD_KEY_SIDE_A_8,  DIK_F, _T( "Side A 8"  ), "input powerpad side a 8"  },
		{ POWERPAD_KEYS + POWERPAD_KEY_SIDE_A_9,  DIK_Z, _T( "Side A 9"  ), "input powerpad side a 9"  },
		{ POWERPAD_KEYS + POWERPAD_KEY_SIDE_A_10, DIK_X, _T( "Side A 10" ), "input powerpad side a 10" },
		{ POWERPAD_KEYS + POWERPAD_KEY_SIDE_A_11, DIK_C, _T( "Side A 11" ), "input powerpad side a 11" },
		{ POWERPAD_KEYS + POWERPAD_KEY_SIDE_A_12, DIK_V, _T( "Side A 12" ), "input powerpad side a 12" },
		{ POWERPAD_KEYS + POWERPAD_KEY_SIDE_B_3,  DIK_Y, _T( "Side B 3"  ), "input powerpad side b 3"  },
		{ POWERPAD_KEYS + POWERPAD_KEY_SIDE_B_2,  DIK_U, _T( "Side B 2"  ), "input powerpad side b 2"  },
		{ POWERPAD_KEYS + POWERPAD_KEY_SIDE_B_8,  DIK_G, _T( "Side B 8"  ), "input powerpad side b 8"  },
		{ POWERPAD_KEYS + POWERPAD_KEY_SIDE_B_7,  DIK_H, _T( "Side B 7"  ), "input powerpad side b 7"  },
		{ POWERPAD_KEYS + POWERPAD_KEY_SIDE_B_6,  DIK_J, _T( "Side B 6"  ), "input powerpad side b 6"  },
		{ POWERPAD_KEYS + POWERPAD_KEY_SIDE_B_5,  DIK_K, _T( "Side B 5"  ), "input powerpad side b 5"  },
		{ POWERPAD_KEYS + POWERPAD_KEY_SIDE_B_11, DIK_N, _T( "Side B 11" ), "input powerpad side b 11" },
		{ POWERPAD_KEYS + POWERPAD_KEY_SIDE_B_10, DIK_M, _T( "Side B 10" ), "input powerpad side b 10" },

		{ CRAZYCLIMBER_KEYS + CRAZYCLIMBER_KEY_LEFT_UP,     DIK_W, _T( "Left Up"     ), "input crazyclimber left up"     },
		{ CRAZYCLIMBER_KEYS + CRAZYCLIMBER_KEY_LEFT_RIGHT,  DIK_D, _T( "Left Right"  ), "input crazyclimber left right"  },
		{ CRAZYCLIMBER_KEYS + CRAZYCLIMBER_KEY_LEFT_DOWN,   DIK_S, _T( "Left Down"   ), "input crazyclimber left down"   },
		{ CRAZYCLIMBER_KEYS + CRAZYCLIMBER_KEY_LEFT_LEFT,   DIK_A, _T( "Left Left"   ), "input crazyclimber left left"   },
		{ CRAZYCLIMBER_KEYS + CRAZYCLIMBER_KEY_RIGHT_UP,    DIK_Y, _T( "Right Up"    ), "input crazyclimber right up"    },
		{ CRAZYCLIMBER_KEYS + CRAZYCLIMBER_KEY_RIGHT_RIGHT, DIK_J, _T( "Right Right" ), "input crazyclimber right right" },
		{ CRAZYCLIMBER_KEYS + CRAZYCLIMBER_KEY_RIGHT_DOWN,  DIK_H, _T( "Right Down"  ), "input crazyclimber right down"  },
		{ CRAZYCLIMBER_KEYS + CRAZYCLIMBER_KEY_RIGHT_LEFT,  DIK_G, _T( "Right Left"  ), "input crazyclimber right left"  },

		{ MAHJONG_KEYS + MAHJONG_KEY_A,      DIK_Q, _T( "A"      ), "input mahjong a"      },
		{ MAHJONG_KEYS + MAHJONG_KEY_B,      DIK_W, _T( "B"      ), "input mahjong b"      },
		{ MAHJONG_KEYS + MAHJONG_KEY_C,      DIK_E, _T( "C"      ), "input mahjong c"      },
		{ MAHJONG_KEYS + MAHJONG_KEY_D,      DIK_R, _T( "D"      ), "input mahjong d"      },
		{ MAHJONG_KEYS + MAHJONG_KEY_E,      DIK_T, _T( "E"      ), "input mahjong e"      },
		{ MAHJONG_KEYS + MAHJONG_KEY_F,      DIK_A, _T( "F"      ), "input mahjong f"      },
		{ MAHJONG_KEYS + MAHJONG_KEY_G,      DIK_S, _T( "G"      ), "input mahjong g"      },
		{ MAHJONG_KEYS + MAHJONG_KEY_H,      DIK_D, _T( "H"      ), "input mahjong h"      },
		{ MAHJONG_KEYS + MAHJONG_KEY_I,      DIK_F, _T( "I"      ), "input mahjong i"      },
		{ MAHJONG_KEYS + MAHJONG_KEY_J,      DIK_G, _T( "J"      ), "input mahjong j"      },
		{ MAHJONG_KEYS + MAHJONG_KEY_K,      DIK_H, _T( "K"      ), "input mahjong k"      },
		{ MAHJONG_KEYS + MAHJONG_KEY_L,      DIK_J, _T( "L"      ), "input mahjong l"      },
		{ MAHJONG_KEYS + MAHJONG_KEY_M,      DIK_K, _T( "M"      ), "input mahjong m"      },
		{ MAHJONG_KEYS + MAHJONG_KEY_N,      DIK_L, _T( "N"      ), "input mahjong n"      },
		{ MAHJONG_KEYS + MAHJONG_KEY_START,  DIK_Z, _T( "Start"  ), "input mahjong start"  },
		{ MAHJONG_KEYS + MAHJONG_KEY_SELECT, DIK_X, _T( "Select" ), "input mahjong select" },
		{ MAHJONG_KEYS + MAHJONG_KEY_KAN,    DIK_C, _T( "Kan"    ), "input mahjong kan"    },
		{ MAHJONG_KEYS + MAHJONG_KEY_PON,    DIK_V, _T( "Pon"    ), "input mahjong pon"    },
		{ MAHJONG_KEYS + MAHJONG_KEY_CHII,   DIK_B, _T( "Chii"   ), "input mahjong chii"   },
		{ MAHJONG_KEYS + MAHJONG_KEY_REACH,  DIK_N, _T( "Reach"  ), "input mahjong reach"  },
		{ MAHJONG_KEYS + MAHJONG_KEY_RON,    DIK_M, _T( "Ron"    ), "input mahjong ron"	   },

		{ EXCITINGBOXING_KEYS + EXCITINGBOXING_KEY_LEFT_HOOK,  DIK_K,      _T( "Left Hook"  ), "input excitingboxing left hook"  },
		{ EXCITINGBOXING_KEYS + EXCITINGBOXING_KEY_RIGHT_HOOK, DIK_L,      _T( "Right Hook" ), "input excitingboxing right hook" },
		{ EXCITINGBOXING_KEYS + EXCITINGBOXING_KEY_LEFT_JABB,  DIK_COMMA,  _T( "Left Jabb"  ), "input excitingboxing left jabb"  },
		{ EXCITINGBOXING_KEYS + EXCITINGBOXING_KEY_RIGHT_JABB, DIK_PERIOD, _T( "Right Jabb" ), "input excitingboxing right jabb" },
		{ EXCITINGBOXING_KEYS + EXCITINGBOXING_KEY_STRAIGHT,   DIK_UP,     _T( "Straight"   ), "input excitingboxing straight"   },
		{ EXCITINGBOXING_KEYS + EXCITINGBOXING_KEY_BODY,       DIK_DOWN,   _T( "Body"       ), "input excitingboxing body"	     },
		{ EXCITINGBOXING_KEYS + EXCITINGBOXING_KEY_LEFT_MOVE,  DIK_LEFT,   _T( "Left Move"  ), "input excitingboxing left move"  },
		{ EXCITINGBOXING_KEYS + EXCITINGBOXING_KEY_RIGHT_MOVE, DIK_RIGHT,  _T( "Right Move" ), "input excitingboxing right move" },

		{ POKKUNMOGURAA_KEYS + POKKUNMOGURAA_KEY_ROW_1_1, DIK_T,     _T( "Row 1 button 1" ), "input pokkunmoguraa row 1 button 1" },
		{ POKKUNMOGURAA_KEYS + POKKUNMOGURAA_KEY_ROW_1_2, DIK_Y,     _T( "Row 1 button 2" ), "input pokkunmoguraa row 1 button 2" },
		{ POKKUNMOGURAA_KEYS + POKKUNMOGURAA_KEY_ROW_1_3, DIK_U,     _T( "Row 1 button 3" ), "input pokkunmoguraa row 1 button 3" },
		{ POKKUNMOGURAA_KEYS + POKKUNMOGURAA_KEY_ROW_1_4, DIK_I,     _T( "Row 1 button 4" ), "input pokkunmoguraa row 1 button 4" },
		{ POKKUNMOGURAA_KEYS + POKKUNMOGURAA_KEY_ROW_2_1, DIK_G,     _T( "Row 2 button 1" ), "input pokkunmoguraa row 2 button 1" },
		{ POKKUNMOGURAA_KEYS + POKKUNMOGURAA_KEY_ROW_2_2, DIK_H,     _T( "Row 2 button 2" ), "input pokkunmoguraa row 2 button 2" },
		{ POKKUNMOGURAA_KEYS + POKKUNMOGURAA_KEY_ROW_2_3, DIK_J,     _T( "Row 2 button 3" ), "input pokkunmoguraa row 2 button 3" },
		{ POKKUNMOGURAA_KEYS + POKKUNMOGURAA_KEY_ROW_2_4, DIK_K,     _T( "Row 2 button 4" ), "input pokkunmoguraa row 2 button 4" },
		{ POKKUNMOGURAA_KEYS + POKKUNMOGURAA_KEY_ROW_3_1, DIK_B,     _T( "Row 3 button 1" ), "input pokkunmoguraa row 3 button 1" },
		{ POKKUNMOGURAA_KEYS + POKKUNMOGURAA_KEY_ROW_3_2, DIK_N,     _T( "Row 3 button 2" ), "input pokkunmoguraa row 3 button 2" },
		{ POKKUNMOGURAA_KEYS + POKKUNMOGURAA_KEY_ROW_3_3, DIK_M,     _T( "Row 3 button 3" ), "input pokkunmoguraa row 3 button 3" },
		{ POKKUNMOGURAA_KEYS + POKKUNMOGURAA_KEY_ROW_3_4, DIK_COMMA, _T( "Row 3 button 4" ), "input pokkunmoguraa row 3 button 4" },

		{ PARTYTAP_KEYS + PARTYTAP_UNIT_1, DIK_Q, _T( "Unit 1" ), "input partytap 1" },
		{ PARTYTAP_KEYS + PARTYTAP_UNIT_2, DIK_W, _T( "Unit 2" ), "input partytap 2" },
		{ PARTYTAP_KEYS + PARTYTAP_UNIT_3, DIK_E, _T( "Unit 3" ), "input partytap 3" },
		{ PARTYTAP_KEYS + PARTYTAP_UNIT_4, DIK_R, _T( "Unit 4" ), "input partytap 4" },
		{ PARTYTAP_KEYS + PARTYTAP_UNIT_5, DIK_T, _T( "Unit 5" ), "input partytap 5" },
		{ PARTYTAP_KEYS + PARTYTAP_UNIT_6, DIK_Y, _T( "Unit 6" ), "input partytap 6" },

		{ EMULATION_KEYS + EMULATION_KEY_ALT_SPEED,       DIK_TAB,  _T( "Alternative Speed" ), "input emulation alternative speed" },
		{ EMULATION_KEYS + EMULATION_KEY_REWIND,          DIK_BACK, _T( "Rewind"            ), "input emulation rewind"            },
		{ EMULATION_KEYS + EMULATION_KEY_INSERT_COIN_1,   DIK_F2,   _T( "Insert Coin 1"     ), "input emulation insert coin 1"     },
		{ EMULATION_KEYS + EMULATION_KEY_INSERT_COIN_2,   DIK_F3,   _T( "Insert Coin 2"     ), "input emulation insert coin 2"     },

		{ FILE_KEYS + FILE_KEY_OPEN,                  CTRL | 'O',         _T( "Open"				  ), "input file open"				    },
		{ FILE_KEYS + FILE_KEY_SAVE_STATE,            VK_F5,              _T( "Save State"			  ), "input file save state"			},
		{ FILE_KEYS + FILE_KEY_SAVE_SCRIPT,           CTRL | SHIFT | 'S', _T( "Save Script"		      ), "input file save script"		    },
		{ FILE_KEYS + FILE_KEY_LOAD_STATE,            VK_F7,              _T( "Load State"			  ), "input file load state"			},
		{ FILE_KEYS + FILE_KEY_LOAD_SCRIPT,           CTRL | SHIFT | 'W', _T( "Load Script"		      ), "input file load script"		    },
		{ FILE_KEYS + FILE_KEY_QUICK_LOAD_STATE_1,    SHIFT | '1',        _T( "Quick Load State 1"	  ), "input file quick load state 1"	},
		{ FILE_KEYS + FILE_KEY_QUICK_LOAD_STATE_2,    SHIFT | '2',        _T( "Quick Load State 2"	  ), "input file quick load state 2"	},
		{ FILE_KEYS + FILE_KEY_QUICK_LOAD_STATE_3,    SHIFT | '3',        _T( "Quick Load State 3"	  ), "input file quick load state 3"	},
		{ FILE_KEYS + FILE_KEY_QUICK_LOAD_STATE_4,    SHIFT | '4',        _T( "Quick Load State 4"	  ), "input file quick load state 4"	},
		{ FILE_KEYS + FILE_KEY_QUICK_LOAD_STATE_5,    SHIFT | '5',        _T( "Quick Load State 5"	  ), "input file quick load state 5"	},
		{ FILE_KEYS + FILE_KEY_QUICK_LOAD_STATE_6,    SHIFT | '6',        _T( "Quick Load State 6"	  ), "input file quick load state 6"	},
		{ FILE_KEYS + FILE_KEY_QUICK_LOAD_STATE_7,    SHIFT | '7',        _T( "Quick Load State 7"	  ), "input file quick load state 7"	},
		{ FILE_KEYS + FILE_KEY_QUICK_LOAD_STATE_8,    SHIFT | '8',        _T( "Quick Load State 8"	  ), "input file quick load state 8"	},
		{ FILE_KEYS + FILE_KEY_QUICK_LOAD_STATE_9,    SHIFT | '9',        _T( "Quick Load State 9"	  ), "input file quick load state 9"	},
		{ FILE_KEYS + FILE_KEY_QUICK_LOAD_LAST_STATE, SHIFT | '0',        _T( "Quick Load Last State" ), "input file quick load last state" },
		{ FILE_KEYS + FILE_KEY_QUICK_SAVE_STATE_1,    '1',                _T( "Quick Save State 1"	  ), "input file quick save state 1"	},
		{ FILE_KEYS + FILE_KEY_QUICK_SAVE_STATE_2,    '2',                _T( "Quick Save State 2"	  ), "input file quick save state 2"	},
		{ FILE_KEYS + FILE_KEY_QUICK_SAVE_STATE_3,    '3',                _T( "Quick Save State 3"	  ), "input file quick save state 3"	},
		{ FILE_KEYS + FILE_KEY_QUICK_SAVE_STATE_4,    '4',                _T( "Quick Save State 4"	  ), "input file quick save state 4"	},
		{ FILE_KEYS + FILE_KEY_QUICK_SAVE_STATE_5,    '5',                _T( "Quick Save State 5"	  ), "input file quick save state 5"	},
		{ FILE_KEYS + FILE_KEY_QUICK_SAVE_STATE_6,    '6',                _T( "Quick Save State 6"	  ), "input file quick save state 6"	},
		{ FILE_KEYS + FILE_KEY_QUICK_SAVE_STATE_7,    '7',                _T( "Quick Save State 7"	  ), "input file quick save state 7"	},
		{ FILE_KEYS + FILE_KEY_QUICK_SAVE_STATE_8,    '8',                _T( "Quick Save State 8"	  ), "input file quick save state 8"	},
		{ FILE_KEYS + FILE_KEY_QUICK_SAVE_STATE_9,    '9',                _T( "Quick Save State 9"	  ), "input file quick save state 9"	},
		{ FILE_KEYS + FILE_KEY_QUICK_SAVE_NEXT_STATE, '0',                _T( "Quick Save Next State" ), "input file quick save next state" },
		{ FILE_KEYS + FILE_KEY_SAVE_SCREENSHOT,       ALT | 'E',          _T( "Save Screenshot"	      ), "input file save screenshot"	    },
		{ FILE_KEYS + FILE_KEY_LAUNCHER,              ALT | 'L',          _T( "Launcher"			  ), "input file launcher"			    },
		{ FILE_KEYS + FILE_KEY_EXIT,                  ALT | 'X',          _T( "Exit"				  ), "input file exit"				    },

		{ MACHINE_KEYS + MACHINE_KEY_POWER,             SHIFT | 'D', _T( "Power"                   ), "input machine power"                   },
		{ MACHINE_KEYS + MACHINE_KEY_RESET_SOFT,        SHIFT | 'R', _T( "Soft Reset"              ), "input machine soft reset"              },		    
		{ MACHINE_KEYS + MACHINE_KEY_RESET_HARD,        SHIFT | 'T', _T( "Hard Reset"              ), "input machine hard reset"              },
		{ MACHINE_KEYS + MACHINE_KEY_PAUSE,             SHIFT | 'P', _T( "Pause"                   ), "input machine pause"                   },
		{ MACHINE_KEYS + MACHINE_KEY_UNLIMITED_SPRITES, SHIFT | 'U', _T( "Unlimited Sprite Toggle" ), "input machine unlimited sprite toggle" },
		{ MACHINE_KEYS + MACHINE_KEY_CHANGE_DISK_SIDE,  SHIFT | 'B', _T( "Change Disk Side"        ), "input machine change disk side"        },

		{ NSF_KEYS + NSF_KEY_PLAY,  SHIFT | VK_UP,    _T( "Play Song"     ), "input nsf play song"     },
		{ NSF_KEYS + NSF_KEY_STOP,  SHIFT | VK_DOWN,  _T( "Stop Song"     ), "input nsf stop song"     },
		{ NSF_KEYS + NSF_KEY_NEXT,  SHIFT | VK_RIGHT, _T( "Next Song"     ), "input nsf next song"     },
		{ NSF_KEYS + NSF_KEY_PREV,  SHIFT | VK_LEFT,  _T( "Previous Song" ), "input nsf previous song" },

		{ VIEW_KEYS + VIEW_KEY_SCREENSIZE_1X,  ALT | '1',        _T( "Screen Size 1x"       ), "input view screen size 1x"      },
		{ VIEW_KEYS + VIEW_KEY_SCREENSIZE_2X,  ALT | '2',        _T( "Screen Size 2x"       ), "input view screen size 2x"      },
		{ VIEW_KEYS + VIEW_KEY_SCREENSIZE_3X,  ALT | '3',        _T( "Screen Size 3x"       ), "input view screen size 3x"      },
		{ VIEW_KEYS + VIEW_KEY_SCREENSIZE_4X,  ALT | '4',        _T( "Screen Size 4x"       ), "input view screen size 4x"      },
		{ VIEW_KEYS + VIEW_KEY_SCREENSIZE_5X,  ALT | '5',        _T( "Screen Size 5x"       ), "input view screen size 5x"      },
		{ VIEW_KEYS + VIEW_KEY_SCREENSIZE_6X,  ALT | '6',        _T( "Screen Size 6x"       ), "input view screen size 6x"      },
		{ VIEW_KEYS + VIEW_KEY_SCREENSIZE_7X,  ALT | '7',        _T( "Screen Size 7x"       ), "input view screen size 7x"      },
		{ VIEW_KEYS + VIEW_KEY_SCREENSIZE_8X,  ALT | '8',        _T( "Screen Size 8x"       ), "input view screen size 8x"      },
		{ VIEW_KEYS + VIEW_KEY_SCREENSIZE_9X,  ALT | '9',        _T( "Screen Size 9x"       ), "input view screen size 9x"      },
		{ VIEW_KEYS + VIEW_KEY_SCREENSIZE_MAX, ALT | 'S',        _T( "Screen Size Max"      ), "input view screen size max"     },
		{ VIEW_KEYS + VIEW_KEY_SHOW_MENU,      VK_ESCAPE,        _T( "Toggle Menu"          ), "input view toggle menu"         },
		{ VIEW_KEYS + VIEW_KEY_SHOW_STATUSBAR, CTRL | 'B',       _T( "Toggle Status Bar"    ), "input view toggle statusbar"    },
		{ VIEW_KEYS + VIEW_KEY_SHOW_ONTOP,     CTRL | 'T',       _T( "Toggle Window On-Top" ), "input view toggle window ontop" },
		{ VIEW_KEYS + VIEW_KEY_SHOW_FPS,       CTRL | 'F',       _T( "Toggle FPS"           ), "input view toggle fps"          },
		{ VIEW_KEYS + VIEW_KEY_FULLSCREEN,     ALT  | VK_RETURN, _T( "Fullscreen"           ), "input view fullscreen"          },

		{ HELP_KEYS + HELP_KEY_HELP, VK_F1 , _T( "Help" ), "input help help" }
	};

	struct Input::Handlers
	{
		static const MsgHandler::Entry<Input> messages[];
		static const MsgHandler::Entry<Input> commands[];
	};

	const MsgHandler::Entry<Input> Input::Handlers::messages[] =
	{
		{ WM_INITDIALOG, &Input::OnInitDialog },
		{ WM_HSCROLL,    &Input::OnHScroll    },
		{ WM_DESTROY,	 &Input::OnDestroy    }
	};

	const MsgHandler::Entry<Input> Input::Handlers::commands[] =
	{
		{ IDC_INPUT_MAP,               &Input::OnCmdDblClk           },
		{ IDC_INPUT_DEVICES,           &Input::OnCmdDevice           },
		{ IDC_INPUT_JOYSTICKS,         &Input::OnCmdJoysticks        },
		{ IDC_INPUT_SET,               &Input::OnCmdSet              },
		{ IDC_INPUT_SETALL,            &Input::OnCmdSetAll           },
		{ IDC_INPUT_CLEAR,             &Input::OnCmdClear            },
		{ IDC_INPUT_CLEARALL,          &Input::OnCmdClearAll         },
		{ IDC_INPUT_JOYSTICKS_ENABLE,  &Input::OnCmdJoystickEnable   },
		{ IDC_INPUT_JOYSTICKS_X,	   &Input::OnCmdJoystickAxis     },
		{ IDC_INPUT_JOYSTICKS_Y,	   &Input::OnCmdJoystickAxis     },
		{ IDC_INPUT_JOYSTICKS_Z,	   &Input::OnCmdJoystickAxis     },
		{ IDC_INPUT_JOYSTICKS_RX,	   &Input::OnCmdJoystickAxis     },
		{ IDC_INPUT_JOYSTICKS_RY,	   &Input::OnCmdJoystickAxis     },
		{ IDC_INPUT_JOYSTICKS_RZ,	   &Input::OnCmdJoystickAxis     },
		{ IDC_INPUT_JOYSTICKS_S0,	   &Input::OnCmdJoystickAxis     },
		{ IDC_INPUT_JOYSTICKS_S1,	   &Input::OnCmdJoystickAxis     },
		{ IDC_INPUT_JOYSTICKS_POV0,	   &Input::OnCmdJoystickAxis     },
		{ IDC_INPUT_JOYSTICKS_POV1,	   &Input::OnCmdJoystickAxis     },
		{ IDC_INPUT_JOYSTICKS_POV2,	   &Input::OnCmdJoystickAxis     },
		{ IDC_INPUT_JOYSTICKS_POV3,	   &Input::OnCmdJoystickAxis     },
		{ IDC_INPUT_JOYSTICKS_DEFAULT, &Input::OnCmdJoysticksDefault },
		{ IDC_INPUT_DEFAULT,           &Input::OnCmdDefault          },
		{ IDC_INPUT_DEFAULT_CATEGORY,  &Input::OnCmdDefaultCategory  },
		{ IDC_INPUT_AUTOFIRE_DEFAULT,  &Input::OnCmdAutoFireDefault  },
		{ IDC_INPUT_OK,                &Input::OnCmdOk               }
	};

	Input::Settings::Settings()
	: autoFireSpeed(AUTOFIRE_DEFAULT_SPEED) {}

	inline uint Input::Settings::Mapping::Code() const
	{
		return key & 0xFF;
	}

	inline uint Input::Settings::Mapping::Alt() const
	{
		return (key & ALT) ? VK_MENU : 0;
	}

	inline uint Input::Settings::Mapping::Shift() const
	{
		return (key & SHIFT) ? VK_SHIFT : 0;
	}

	inline uint Input::Settings::Mapping::Ctrl() const
	{
		return (key & CTRL) ? VK_CONTROL : 0;
	}

	inline const Input::Settings::Mapping& Input::Settings::GetMapping(uint type,uint index)
	{
		NST_ASSERT( index < NumTypeKeys(type) );
		return map[types[type].offset + index];
	}

	inline const Input::Settings::Type& Input::Settings::GetType(uint type)
	{
		NST_ASSERT( type < NUM_TYPES );
		return types[type];
	}

	inline void Input::Settings::Unmap(uint index)
	{
		NST_ASSERT( index < NUM_KEYS );
		keys[index].Unmap();
	}

	ibool Input::Settings::Map(const uint index,const Key& key)
	{
		NST_ASSERT( index < NUM_KEYS );

		if (key.IsAssigned())
		{
			if (index >= COMMAND_KEYS)
			{
				for (uint i=0; i < COMMAND_KEYS; ++i)
				{
					if (key == keys[i])
						return (i == index);
				}
			}

			for (uint i=COMMAND_KEYS; i < NUM_KEYS; ++i)
			{
				if (key == keys[i])
					return (i == index);
			}
		}

		keys[index] = key;

		return TRUE;
	}

	void Input::Settings::Clear()
	{
		for (uint i=0; i < NUM_KEYS; ++i)
			keys[i].Unmap();
	}

	void Input::Settings::Clear(const uint type)
	{
		NST_ASSERT( type < NUM_TYPES );

		for (uint i=types[type].offset, n=types[type+1].offset; i < n; ++i)
			keys[i].Unmap();
	}

	void Input::Settings::Reset(const DirectX::DirectInput& directInput)
	{
		for (uint i=0; i < NUM_TYPES; ++i)
			Reset( directInput, i );
	}

	void Input::Settings::Reset(const DirectX::DirectInput& directInput,const uint type)
	{
		NST_ASSERT( type < NUM_TYPES );

		const Mapping* it = map + types[type].offset;
		const Mapping* const end = map + types[type + 1].offset;

		NST_ASSERT( it < end );

		Key key;

		if (type < TYPE_FILE)
		{
			do
			{
				directInput.MapKeyboard( key, it->key );
				Map( it->index, key );
			}
			while (++it != end);
		}
		else
		{
			do
			{
				key.MapVirtKey( it->Code(), it->Alt(), it->Ctrl(), it->Shift() );
				Map( it->index, key );
			}
			while (++it != end);
		}
	}

	Input::Input(DirectX::DirectInput& di,Managers::Emulator& emulator,const Configuration& cfg)
	: 
	nes         ( emulator ),
	directInput ( di ),
	dialog      ( IDD_INPUT, this, Handlers::messages, Handlers::commands )
	{
		settings.Clear();
		settings.autoFireSpeed = cfg["input autofire speed"].Default( (uint) Settings::AUTOFIRE_DEFAULT_SPEED );
		settings.allowSimulAxes = (cfg["input allow simultaneous axes"] == Configuration::YES);
		
		if (settings.autoFireSpeed > Settings::AUTOFIRE_MAX_SPEED) 
			settings.autoFireSpeed = Settings::AUTOFIRE_DEFAULT_SPEED;

		System::Guid joyGuids[DirectX::DirectInput::MAX_JOYSTICKS];
		uint maxGuids = 0;

		{
			String::Stack<16,char> deviceIndex( "input device xx" );
			NST_COMPILE_ASSERT( DirectX::DirectInput::MAX_JOYSTICKS <= 99 );

			for (uint i=0; i < DirectX::DirectInput::MAX_JOYSTICKS; ++i)
			{
				deviceIndex(13) = i;
				const GenericString string( cfg[deviceIndex] );

				if (string.Length())
				{
					joyGuids[i] = string;
					maxGuids = i + 1;
				}
			}
		}

		{
			String::Stack<32,char> joyIndex("input joy xx");

			for (uint i=0, n=directInput.NumJoysticks(); i < maxGuids; ++i)
			{
				joyIndex(10) = i;
				const uint joyOffset = joyIndex.Length();

				for (uint j=0; j < n; ++j)
				{
					if (joyGuids[i] == directInput.GetJoystickGuid(j))
					{
						joyIndex(joyOffset) = " enabled";
						directInput.EnableJoystick( j, cfg[joyIndex] != Configuration::NO );

						joyIndex(joyOffset) = " deadzone";
						uint deadzone = cfg[joyIndex].Default( (uint) DirectX::DirectInput::DEFAULT_DEADZONE );

						if (deadzone > DirectX::DirectInput::DEADZONE_MAX) 
							deadzone = DirectX::DirectInput::DEFAULT_DEADZONE;

						directInput.SetAxisDeadZone( j, deadzone );

						joyIndex(joyOffset) = " scan ";
						const uint joyOffset2 = joyIndex.Length();

						uint axes = 0;

						static cstring const names[] =
						{
							"x","y","z","rx","ry","rz","z0","z1","p0","p1","p2","p3"
						};

						for (uint k=0; k < NST_COUNT(names); ++k)
						{
							joyIndex(joyOffset2) = names[k];

							if (cfg[joyIndex] == Configuration::YES)
							{
								axes |= (1U << k);
							}
							else if (cfg[joyIndex] != Configuration::NO)
							{
								axes |= (1U << k) & DirectX::DirectInput::DEFAULT_AXES;
							}
						}

						directInput.SetScannerAxes( j, axes );
					}
				}
			}
		}
		
		Settings::Key key;
		HeapString keyName;

		for (uint i=0; i < Settings::NUM_TYPES; ++i)
		{
			for (uint j=0, n=Settings::NumTypeKeys(i); j < n; ++j)
			{
				const Settings::Mapping& mapping = Settings::GetMapping( i, j );
				keyName = cfg[mapping.cfgName];

				while (tstring const quote=::StrStrI( keyName.Ptr(), _T("quotation") ))
					keyName( quote - keyName.Ptr(), 9 ) = _T('\"');

				if (keyName.Length())
				{
					if (i < Settings::TYPE_COMMAND)
						directInput.MapKey( key, keyName.Ptr(), joyGuids, maxGuids );
					else
						key.MapVirtKey( keyName );
				}
				else
				{
					if (i < Settings::TYPE_COMMAND)
						directInput.MapKeyboard( key, mapping.key );
					else
						key.MapVirtKey( mapping.Code(), mapping.Alt(), mapping.Ctrl(), mapping.Shift() );
				}

				if (!settings.Map( mapping.index, key ))
					Io::Log() << "DirectInput: warning, key assigned to \"" << mapping.cfgName << "\" is already in use!\r\n";
			}
		}
	}

	void Input::Save(Configuration& cfg) const
	{
		cfg["input autofire speed"] = settings.autoFireSpeed;
		cfg["input allow simultaneous axes"].YesNo() = settings.allowSimulAxes;

		{
			String::Stack<16,char> deviceIndex( "input device xx" );
			String::Stack<32,char> joyIndex( "input joy xx" );

			for (uint i=0, n=directInput.NumJoysticks(); i < n; ++i)
			{
				deviceIndex(13) = i;
				cfg[deviceIndex].Quote() = directInput.GetJoystickGuid(i).GetString();

				joyIndex(10) = i;
				const uint joyOffset = joyIndex.Length();

				joyIndex(joyOffset) = " enabled";
				cfg[joyIndex].YesNo() = directInput.IsJoystickEnabled(i);

				joyIndex(joyOffset) = " deadzone";
				cfg[joyIndex] = directInput.GetAxisDeadZone(i);

				joyIndex(joyOffset) = " scan ";
				const uint joyOffset2 = joyIndex.Length();

				static cstring const names[] =
				{
					"x","y","z","rx","ry","rz","z0","z1","p0","p1","p2","p3"
				};

				for (uint j=0, axes=directInput.GetScannerAxes(i); j < NST_COUNT(names); ++j)
				{
					joyIndex(joyOffset2) = names[j];
					cfg[joyIndex].YesNo() = (axes & (1U << j));
				}
			}
		}

		HeapString keyName;

		for (uint i=0; i < Settings::NUM_TYPES; ++i)
		{
			for (uint j=0, n=Settings::NumTypeKeys(i); j < n; ++j)
			{
				const Settings::Mapping& mapping = Settings::GetMapping( i, j );
				keyName = directInput.GetKeyName( settings.GetKey(mapping.index) );

				while (tstring const quote=::StrChr( keyName.Ptr(), _T('\"') ))
					keyName( quote - keyName.Ptr() ) = _T("quotation");

				cfg[mapping.cfgName].Quote() = keyName;
			}
		}
	}

	ibool Input::OnInitDialog(Param&)
	{
		{
			const Control::ListBox listBox( dialog.ListBox(IDC_INPUT_DEVICES) );

			listBox.Reserve( Settings::NUM_TYPES );

			for (uint i=0; i < Settings::NUM_TYPES; ++i)
				listBox.Add( Settings::GetType(i).name );

			listBox[0].Select();
		}

		if (const uint numJoysticks = directInput.NumJoysticks())
		{
			const Control::ComboBox comboBox( dialog.ComboBox(IDC_INPUT_JOYSTICKS) );

			for (uint i=0; i < numJoysticks; ++i)
				comboBox.Add( directInput.GetJoystickName(i).Ptr() );

			comboBox[0].Select();

			dialog.Slider(IDC_INPUT_JOYSTICKS_DEADZONE).SetRange( 0, DirectX::DirectInput::DEADZONE_MAX );

			UpdateJoysticks( 0 );
		}
		else for (uint i=IDC_INPUT_JOYSTICKS; i <= IDC_INPUT_JOYSTICKS_DEFAULT; ++i)
		{
			dialog.Control( i ).Disable();
		}
		
		dialog.Slider( IDC_INPUT_AUTOFIRE_SLIDER ).SetRange( 0, Settings::AUTOFIRE_MAX_SPEED );
		dialog.Slider( IDC_INPUT_AUTOFIRE_SLIDER ).Position() = settings.autoFireSpeed;

		dialog.CheckBox( IDC_INPUT_ALLOW_SIMUL_AXES ).Check( settings.allowSimulAxes );

		UpdateKeyNames( 0 );
		UpdateKeyMap( 0 );

		return TRUE;
	}

	ibool Input::OnCmdDblClk(Param& param)
	{
		if (HIWORD(param.wParam) == LBN_DBLCLK)
		{
			if (ScanKeys() == SCAN_NEXT)
				SelectNextMapKey();

			return TRUE;
		}

		return FALSE;
	}

	ibool Input::OnHScroll(Param& param)
	{
		if (param.Slider().IsControl( IDC_INPUT_JOYSTICKS_DEADZONE ))
		{
			const uint deadZone = param.Slider().Scroll();

			if (directInput.SetAxisDeadZone( dialog.ComboBox(IDC_INPUT_JOYSTICKS).Selection().GetIndex(), deadZone ))
				dialog.Edit( IDC_INPUT_JOYSTICKS_DEADZONE_NUM ) << deadZone;
		}

		return TRUE;
	}

	void Input::UpdateKeyNames(const uint type) const
	{
		NST_ASSERT( type < Settings::NUM_TYPES );

		const Control::ListBox listBox( dialog.ListBox(IDC_INPUT_KEYS) );

		listBox.Clear();
		listBox.Reserve( Settings::NumTypeKeys(type) );

		for (uint i=0, n=Settings::NumTypeKeys(type); i < n; ++i)
			listBox.Add( Settings::GetMapping(type,i).dlgName );
	}

	void Input::UpdateKeyMap(const uint type) const
	{
		const Control::ListBox listBox( dialog.ListBox(IDC_INPUT_MAP) );

		listBox.Clear();
		listBox.Reserve( Settings::NumTypeKeys(type) );

		for (uint i=0, n=Settings::NumTypeKeys(type); i < n; ++i)
			listBox.Add( directInput.GetKeyName( settings.GetKey(Settings::GetMapping(type,i).index) ).Ptr() );

		listBox[0].Select();
	}

	void Input::ResetJoysticks()
	{
		if (uint numJoysticks = directInput.NumJoysticks())
		{
			do 
			{					
				directInput.EnableJoystick( --numJoysticks, TRUE );
				directInput.SetAxisDeadZone( numJoysticks, DirectX::DirectInput::DEFAULT_DEADZONE );
				directInput.SetScannerAxes( numJoysticks, DirectX::DirectInput::DEFAULT_AXES );
			} 
			while (numJoysticks);

			UpdateJoysticks( dialog.ComboBox(IDC_INPUT_JOYSTICKS).Selection().GetIndex() );
		}
	}

	void Input::UpdateJoysticks(const uint type) const
	{
		NST_ASSERT( directInput.NumJoysticks() );

		const ibool enabled = directInput.IsJoystickEnabled( type );
		dialog.CheckBox(IDC_INPUT_JOYSTICKS_ENABLE).Check( enabled );

		{
			const uint deadZone = directInput.GetAxisDeadZone( type );

			dialog.Slider( IDC_INPUT_JOYSTICKS_DEADZONE ).Position() = deadZone;
			dialog.Slider( IDC_INPUT_JOYSTICKS_DEADZONE ).Enable( enabled );

			dialog.Edit( IDC_INPUT_JOYSTICKS_DEADZONE_NUM ) << deadZone;
			dialog.Edit( IDC_INPUT_JOYSTICKS_DEADZONE_NUM ).Enable( enabled );
		}

		{
			const uint axes[] = 
			{
				directInput.GetAvailableAxes( type ),
				directInput.GetScannerAxes( type )
			};

			for (uint i=0; i <= (IDC_INPUT_JOYSTICKS_POV3-IDC_INPUT_JOYSTICKS_X); ++i)
			{
				const Control::CheckBox box( dialog.CheckBox(IDC_INPUT_JOYSTICKS_X + i) );
				box.Enable( enabled && (axes[0] & (1U << i)) );
				box.Check( axes[1] & (1U << i) );
			}
		}
	}

	ibool Input::OnCmdDevice(Param& param)
	{
		if (param.ListBox().SelectionChanged())
		{
			const uint type = dialog.ListBox(IDC_INPUT_DEVICES).Selection().GetIndex();
			
			UpdateKeyNames( type );
			UpdateKeyMap( type );
		}

		return TRUE;
	}
 
	ibool Input::OnCmdJoysticks(Param& param)
	{
		if (param.ComboBox().SelectionChanged())
			UpdateJoysticks( dialog.ComboBox(IDC_INPUT_JOYSTICKS).Selection().GetIndex() );

		return TRUE;
	}

	ibool Input::OnCmdJoystickEnable(Param& param)
	{
		if (param.Button().IsClicked())
		{
			const uint type = dialog.ComboBox(IDC_INPUT_JOYSTICKS).Selection().GetIndex();

			directInput.EnableJoystick( type, dialog.CheckBox(param.Button().GetId()).IsChecked() );
			UpdateJoysticks( type  );
		}

		return TRUE;
	}

	ibool Input::OnCmdJoystickAxis(Param& param)
	{
		if (param.Button().IsClicked())
		{
			directInput.SetScannerAxes
			( 
		       	dialog.ComboBox(IDC_INPUT_JOYSTICKS).Selection().GetIndex(), 
				1U << (param.Button().GetId() - IDC_INPUT_JOYSTICKS_X),
				dialog.CheckBox(param.Button().GetId()).IsChecked() 
			);
		}

		return TRUE;
	}

	ibool Input::OnCmdSet(Param& param) 
	{
		if (param.Button().IsClicked() && ScanKeys() == SCAN_NEXT)
			SelectNextMapKey();

		return TRUE;
	}

	ibool Input::OnCmdSetAll(Param& param) 
	{
		if (param.Button().IsClicked())
		{
			const Control::ListBox listBox( dialog.ListBox(IDC_INPUT_MAP) );
			listBox[0].Select();

			for (uint count=listBox.Size(); count; --count)
			{
				if (ScanKeys() == SCAN_ABORT)
					break;

				SelectNextMapKey();
			}
		}

		return TRUE;
	}

	ibool Input::OnCmdClear(Param& param) 
	{
		if (param.Button().IsClicked())
		{
			const Control::ListBox keyBox( dialog.ListBox(IDC_INPUT_MAP) );
			const uint index = Settings::GetMapping( dialog.ListBox(IDC_INPUT_DEVICES).Selection().GetIndex(), keyBox.Selection().GetIndex() ).index;

			settings.Unmap( index );

			keyBox.Selection().Text() << directInput.GetKeyName( settings.GetKey(index) ).Ptr();
			SelectNextMapKey();
		}

		return TRUE;
	}

	ibool Input::OnCmdClearAll(Param& param) 
	{
		if (param.Button().IsClicked())
		{
			const uint type = dialog.ListBox(IDC_INPUT_DEVICES).Selection().GetIndex();

			settings.Clear( type );			
			
			UpdateKeyMap( type );
		}

		return TRUE;
	}

	ibool Input::OnCmdDefaultCategory(Param& param)
	{
		if (param.Button().IsClicked())
		{
			const uint type = dialog.ListBox(IDC_INPUT_DEVICES).Selection().GetIndex();

			settings.Clear( type );
			settings.Reset( directInput, type );

			UpdateKeyMap( type );
		}

		return TRUE;
	}

	ibool Input::OnCmdAutoFireDefault(Param& param)
	{
		if (param.Button().IsClicked())
			dialog.Slider( IDC_INPUT_AUTOFIRE_SLIDER ).Position() = Settings::AUTOFIRE_DEFAULT_SPEED;

		return TRUE;
	}

	ibool Input::OnCmdJoysticksDefault(Param& param)
	{
		if (param.Button().IsClicked())
			ResetJoysticks();

		return TRUE;
	}

	ibool Input::OnCmdDefault(Param& param) 
	{
		if (param.Button().IsClicked())
		{
			settings.Clear();
			settings.Reset( directInput );

			dialog.CheckBox( IDC_INPUT_ALLOW_SIMUL_AXES ).Uncheck();
			dialog.Slider( IDC_INPUT_AUTOFIRE_SLIDER ).Position() = Settings::AUTOFIRE_DEFAULT_SPEED;

			ResetJoysticks();

			UpdateKeyMap( dialog.ListBox(IDC_INPUT_DEVICES).Selection().GetIndex() );
		}

		return TRUE;
	}

	ibool Input::OnCmdOk(Param& param) 
	{
		if (param.Button().IsClicked())
			dialog.Close();

		return TRUE;
	}

	ibool Input::OnDestroy(Param&) 
	{
		settings.autoFireSpeed = dialog.Slider( IDC_INPUT_AUTOFIRE_SLIDER ).Position();
		settings.allowSimulAxes = dialog.CheckBox( IDC_INPUT_ALLOW_SIMUL_AXES ).IsChecked();

		return TRUE;
	}

	INT_PTR Input::ScanKeys()
	{
		class KeyPressWindow : Dynamic
		{
		public:

			enum Result
			{
				RESULT_OK,
				RESULT_ABORT,
				RESULT_DUPLICATE,
				RESULT_INVALID
			};

		private:

			struct Timer
			{
				enum 
				{
					RATE = 50,
					CLOCK = 1000,
					START = 5000,
					SEC   = 1000
				};

				int remaining;
				int clock;

				Timer()
				: remaining(START), clock(START) {}
			};

			Input& base;
			Timer timer;
			Result result;

			void Close(Result r)
			{
				result = r;
				base.dialog.Enable( TRUE );
				Destroy();
			}

			ibool OnTimer()
			{
				timer.remaining -= Timer::RATE;

				if (timer.remaining <= 0 || *this != ::GetForegroundWindow())
				{
					Close( RESULT_ABORT );
					return FALSE;
				}

				if (timer.remaining <= timer.clock)
				{
					uint msgId;

					if (base.dialog.ListBox(IDC_INPUT_DEVICES).Selection().GetIndex() >= Settings::TYPE_COMMAND)
						msgId = IDS_DIALOG_INPUT_PRESS_ANY_KEY_MENU;
					else
						msgId = IDS_DIALOG_INPUT_PRESS_ANY_KEY_EMU;

					base.dialog.Control(IDC_INPUT_KEYPRESS_TEXT).Text() <<
					( 
						HeapString() << Resource::String(msgId) 
						             << " ("
						             << (tchar) ('0' + (timer.clock / Timer::SEC))
						             << ')'
					).Ptr();

					timer.clock -= Timer::CLOCK;
				}

				if (base.dialog.ListBox(IDC_INPUT_DEVICES).Selection().GetIndex() < Settings::TYPE_COMMAND)
				{
					Settings::Key key;

					switch (base.directInput.ScanKey( key ))
					{
						case DirectX::DirectInput::SCAN_GOOD_KEY: 
					
							for (uint i=10; i && base.directInput.IsAnyPressed(); --i)
								::Sleep( 100 );
					
							if (base.MapSelectedKey( key ))
							{
								Close( RESULT_OK );
								return FALSE;
							}
							else
							{
								Close( RESULT_DUPLICATE );
								return FALSE;
							}
							break;
					
						case DirectX::DirectInput::SCAN_INVALID_KEY: 
					
							Close( RESULT_INVALID );
							return FALSE;
					}
				}

				return TRUE;
			}

			ibool OnCreate(Param& param)
			{
				base.dialog.Enable( FALSE );

				if (base.dialog.ListBox(IDC_INPUT_DEVICES).Selection().GetIndex() < Settings::TYPE_COMMAND)
					base.directInput.BeginScanMode( param.hWnd );

				StartTimer( this, &KeyPressWindow::OnTimer, Timer::RATE );
				::SetWindowPos( param.hWnd, HWND_TOP, 0, 0, 0, 0, SWP_HIDEWINDOW );

				return FALSE;
			}

			ibool OnDestroy(Param&)
			{
				StopTimer( this, &KeyPressWindow::OnTimer );

				if (base.dialog.ListBox(IDC_INPUT_DEVICES).Selection().GetIndex() < Settings::TYPE_COMMAND)
					base.directInput.EndScanMode();

				base.dialog.Control(IDC_INPUT_KEYPRESS_TEXT).Text().Clear();

				return FALSE;
			}

			ibool OnKeyDown(Param& param)
			{
				if (param.wParam != VK_SHIFT && param.wParam != VK_CONTROL && param.wParam != VK_MENU)
				{
					const uint vKeys[3] =
					{
						(::GetAsyncKeyState( VK_SHIFT   ) & 0x8000) ? VK_SHIFT   : 0,
						(::GetAsyncKeyState( VK_CONTROL ) & 0x8000) ? VK_CONTROL : 0,
						(::GetAsyncKeyState( VK_MENU    ) & 0x8000) ? VK_MENU    : 0
					};

					Settings::Key key;

					if (!key.MapVirtKey( param.wParam, vKeys[0], vKeys[1], vKeys[2] ))
					{
			       		Close( RESULT_INVALID );
					}
					else if (!base.MapSelectedKey( key ))
					{
			     		Close( RESULT_DUPLICATE );
					}
					else
					{
			     		Close( RESULT_OK );
					}
				}

				return FALSE;
			}

		public:

			KeyPressWindow(Input& b)
			: base(b), result(RESULT_ABORT)
			{
				static const MsgHandler::Entry<KeyPressWindow> messages[] =
				{
					{ WM_CREATE,     &KeyPressWindow::OnCreate  },
					{ WM_DESTROY,    &KeyPressWindow::OnDestroy },
					{ WM_KEYDOWN,    &KeyPressWindow::OnKeyDown },
					{ WM_SYSKEYDOWN, &KeyPressWindow::OnKeyDown }
				};

				Messages().Set( this, messages );

				Context context;

				context.windowName = context.className = _T("Poll Key");
				context.winStyle = WS_POPUP;
				context.hParent = base.dialog;

				Create( context );
			}

			Result Poll() const
			{
				for (MSG msg; *this && ::GetMessage( &msg, NULL, 0, 0 ) > 0; )
				{
					::TranslateMessage( &msg );
					::DispatchMessage( &msg );
				}

				return result;
			}
		};

		switch (KeyPressWindow(*this).Poll())
		{
     		case KeyPressWindow::RESULT_INVALID:

				User::Warn( IDS_DIALOG_INPUT_PRESS_ANY_KEY_INVALID, IDS_TITLE_ERROR );
				return SCAN_ABORT;			

			case KeyPressWindow::RESULT_DUPLICATE:
			
				User::Warn( IDS_DIALOG_INPUT_DUPLICATE_KEYS, IDS_TITLE_ERROR );
				return SCAN_ABORT;			

			case KeyPressWindow::RESULT_ABORT:
				return SCAN_ABORT;			
		}

		return SCAN_NEXT;
	}

	void Input::SelectNextMapKey()
	{
		const Control::ListBox keyBox( dialog.ListBox(IDC_INPUT_MAP) );

		const uint index = keyBox.Selection().GetIndex() + 1;
		keyBox[index < keyBox.Size() ? index : 0].Select();
	}

	ibool Input::MapSelectedKey(const Settings::Key& key)
	{
		const uint index = Settings::GetMapping( dialog.ListBox(IDC_INPUT_DEVICES).Selection().GetIndex(), dialog.ListBox(IDC_INPUT_MAP).Selection().GetIndex() ).index;

		if (settings.Map( index, key ))
		{
			dialog.ListBox(IDC_INPUT_MAP).Selection().Text() << directInput.GetKeyName( settings.GetKey(index) ).Ptr();
			return TRUE;
		}

		return FALSE;
	}
}
