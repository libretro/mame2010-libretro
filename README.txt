MAME.0139-LIBRETRO

Port of MAME 0.139 for libretro.
 
Source base is mame0.139 official source: 

http://www.mamedev.org/downloader.php?file=releases/mame0139s.zip

OSD code is highly inspired by source code of others mame ports :
 
- librerto-mame0.78 port : https://github.com/libretro/mame2003-libretro
- ps3 mame0.125 port     : http://www.volny.cz/molej/ps3/mame_ps3.htm
- mame4droid             : http://code.google.com/p/imame4all/source/browse/


Build :

     for now you must build in 2 pass: 
     one for the native buildtools and second for the target emulator build.
  
     build for android:

     make -f Makefile.libretro "VRENDER=soft" "NATIVE=1" buildtools
     make -f Makefile.libretro "VRENDER=soft" "platform=android" emulator -j4
     
     build for pc linux/win:

     make -f Makefile.libretro "VRENDER=soft" -j4 buildtools
     make -f Makefile.libretro "VRENDER=soft" -j4
  
     (NB: for 64 bits build export PTR64=1 at least on win64)

Usage : 

 rompath , inipath and others must be set in mame.ini
 and the initial inipath is hardcoded:

 for PC        to	   ./
 for ANDROID   to 	   /mnt/sdcard/

 Once mame.ini is in the good place , launch retroarch , select game 
 
 controls are: 

	RETRO_DEVICE_ID_JOYPAD_START		[KEY_START]
	RETRO_DEVICE_ID_JOYPAD_SELECT		[KEY_COIN]
	RETRO_DEVICE_ID_JOYPAD_A		[KEY_BUTTON_1]
	RETRO_DEVICE_ID_JOYPAD_B		[KEY_BUTTON_2]
	RETRO_DEVICE_ID_JOYPAD_X		[KEY_BUTTON_3]
	RETRO_DEVICE_ID_JOYPAD_Y		[KEY_BUTTON_4]
	RETRO_DEVICE_ID_JOYPAD_L 		[KEY_BUTTON_5]
	RETRO_DEVICE_ID_JOYPAD_R		[KEY_BUTTON_6]
	RETRO_DEVICE_ID_JOYPAD_R2		[KEY_F11]
	RETRO_DEVICE_ID_JOYPAD_L2		[KEY_TAB]
	RETRO_DEVICE_ID_JOYPAD_R3		[KEY_F3]
	RETRO_DEVICE_ID_JOYPAD_L3		[KEY_F2]
	RETRO_DEVICE_ID_JOYPAD_UP		[KEY_JOYSTICK_U]
	RETRO_DEVICE_ID_JOYPAD_DOWN		[KEY_JOYSTICK_D]
	RETRO_DEVICE_ID_JOYPAD_LEFT		[KEY_JOYSTICK_L]
	RETRO_DEVICE_ID_JOYPAD_RIGHT		[KEY_JOYSTICK_R]

        tips: L2 to tab and select newgame from mameui
              R2 to show framerate
              L3 to access Service Mode
