# mame2010

Port of MAME 0.139 for libretro, originally sourced from https://github.com/mamedev/mame/releases/download/mame0139/mame0139s.zip

The OSD code is highly inspired by other MAME ports :
 
- mame2003 : https://github.com/libretro/mame2003-libretro
- ps3 mame0.125 port     : http://www.volny.cz/molej/ps3/mame_ps3.htm
- mame4droid             : http://code.google.com/p/imame4all/source/browse/

## Directories

mame2010 requires the folllowing directories exist, and will create them if they are missing:

* libretro system folder/mame2010/ - `cheat.dat` cheats file
* libretro system folder/mame2010/artwork
* libretro system folder/mame2010/crosshairs
* libretro system folder/mame2010/fonts
* libretro system folder/mame2010/samples

* libretro saves folder/mame2010/cfg
* libretro saves folder/mame2010/comment
* libretro saves folder/mame2010/ctrlr
* libretro saves folder/mame2010/image
* libretro saves folder/mame2010/ini
* libretro saves folder/mame2010/input
* libretro saves folder/mame2010/memcard
* libretro saves folder/mame2010/nvram


## Controls: 

	RETRO_DEVICE_ID_JOYPAD_START		[KEY_START]
	RETRO_DEVICE_ID_JOYPAD_SELECT		[KEY_COIN]
	RETRO_DEVICE_ID_JOYPAD_A		[KEY_BUTTON_1]
	RETRO_DEVICE_ID_JOYPAD_B		[KEY_BUTTON_2]
	RETRO_DEVICE_ID_JOYPAD_X		[KEY_BUTTON_3]
	RETRO_DEVICE_ID_JOYPAD_Y		[KEY_BUTTON_4]
	RETRO_DEVICE_ID_JOYPAD_L 		[KEY_BUTTON_5]
	RETRO_DEVICE_ID_JOYPAD_R		[KEY_BUTTON_6]
	RETRO_DEVICE_ID_JOYPAD_L2		[KEY_TAB]
	RETRO_DEVICE_ID_JOYPAD_R3		[KEY_F3]
	RETRO_DEVICE_ID_JOYPAD_L3		[KEY_F2]
	RETRO_DEVICE_ID_JOYPAD_UP		[KEY_JOYSTICK_U]
	RETRO_DEVICE_ID_JOYPAD_DOWN		[KEY_JOYSTICK_D]
	RETRO_DEVICE_ID_JOYPAD_LEFT		[KEY_JOYSTICK_L]
	RETRO_DEVICE_ID_JOYPAD_RIGHT		[KEY_JOYSTICK_R]

        tips: L2 for the MAME UI
              L3 for Service Mode
