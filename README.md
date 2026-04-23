# mame2010

Port of MAME 0.139 for libretro, originally sourced from https://github.com/mamedev/mame/releases/download/mame0139/mame0139s.zip

The OSD code is highly inspired by other MAME ports :
 
- mame2003 : https://github.com/libretro/mame2003-libretro
- ps3 mame0.125 port     : http://www.volny.cz/molej/ps3/mame_ps3.htm
- mame4droid             : http://code.google.com/p/imame4all/source/browse/

## Directories and Files

mame2010 requires that the following directories exist, and will create them if they are missing.

mame2010 supports cheat that `cheat.zip` file must be in this subfolders: libretro saves folder/mame2010/cheat.zip

Use `Cheat enable` switch it's under the Retroarch `Core Options` menu.
Cheat will be activated and visible on the MAME config menu after restart the core.

libretro system subfolders:

- libretro system folder/mame2010/ - `cheat.zip` file DOES NOT work in here!
- libretro system folder/mame2010/cheat/ unziped `.xml` cheat file(s) doesn't work in here!
- libretro system folder/mame2010/ini
- libretro system folder/mame2010/artwork
- libretro system folder/mame2010/crosshairs
- libretro system folder/mame2010/fonts
- libretro system folder/mame2010/samples

libretro saves subfolders:
- libretro saves folder/mame2010/ - `cheat.zip` file ONLY works in here.
- libretro saves folder/mame2010/cheat/ - unziped `.xml` cheat file(s) can be added in here (optional).
- libretro saves folder/mame2010/cfg
- libretro saves folder/mame2010/comment
- libretro saves folder/mame2010/ctrlr
- libretro saves folder/mame2010/ini/ - `mame.ini` file can be found in here.
- libretro saves folder/mame2010/input
- libretro saves folder/mame2010/memcard
- libretro saves folder/mame2010/nvram


## Default Player 1 and 2 Controls: 

	RETRO_DEVICE_ID_JOYPAD_START        MAME: KEY_START
	RETRO_DEVICE_ID_JOYPAD_SELECT       MAME: KEY_COIN
	RETRO_DEVICE_ID_JOYPAD_A            MAME: KEY_BUTTON_1
	RETRO_DEVICE_ID_JOYPAD_B            MAME: KEY_BUTTON_2
	RETRO_DEVICE_ID_JOYPAD_X            MAME: KEY_BUTTON_3
	RETRO_DEVICE_ID_JOYPAD_Y            MAME: KEY_BUTTON_4
	RETRO_DEVICE_ID_JOYPAD_L            MAME: KEY_BUTTON_5
	RETRO_DEVICE_ID_JOYPAD_R            MAME: KEY_BUTTON_6
	RETRO_DEVICE_ID_JOYPAD_L2           MAME: KEY_BUTTON_7
	RETRO_DEVICE_ID_JOYPAD_UP           MAME: KEY_JOYSTICK_U
	RETRO_DEVICE_ID_JOYPAD_DOWN         MAME: KEY_JOYSTICK_D
	RETRO_DEVICE_ID_JOYPAD_LEFT         MAME: KEY_JOYSTICK_L
	RETRO_DEVICE_ID_JOYPAD_RIGHT        MAME: KEY_JOYSTICK_R
	RETRO_DEVICE_ID_JOYPAD_R2           Turbo Button

## Default Player 3 and 4 Controls: 

	RETRO_DEVICE_ID_JOYPAD_START        MAME: KEY_START
	RETRO_DEVICE_ID_JOYPAD_SELECT       MAME: KEY_COIN
	RETRO_DEVICE_ID_JOYPAD_A            MAME: KEY_BUTTON_1
	RETRO_DEVICE_ID_JOYPAD_B            MAME: KEY_BUTTON_2
	RETRO_DEVICE_ID_JOYPAD_X            MAME: KEY_BUTTON_3
	RETRO_DEVICE_ID_JOYPAD_UP           MAME: KEY_JOYSTICK_U
	RETRO_DEVICE_ID_JOYPAD_DOWN         MAME: KEY_JOYSTICK_D
	RETRO_DEVICE_ID_JOYPAD_LEFT         MAME: KEY_JOYSTICK_L
	RETRO_DEVICE_ID_JOYPAD_RIGHT        MAME: KEY_JOYSTICK_R
	RETRO_DEVICE_ID_JOYPAD_R2           Turbo Button
    
## Native MAME UI Controls:

_Note: these controls are only operational for Player 1_

	RETRO_DEVICE_ID_JOYPAD_L3           Test/Service Mode
    RETRO_DEVICE_ID_JOYPAD_R3           Enter MAME UI
	RETRO_DEVICE_ID_JOYPAD_A            MAME: IPT_UI_SELECT (Make selections in the MAME GUI)
      
