static INT32 pad1_get_state(void *device_internal, void *item_internal)
{
	UINT8 *itemdata = (UINT8 *)item_internal;
	return *itemdata ;
}

static INT32 pad2_get_state(void *device_internal, void *item_internal)
{
	UINT8 *itemdata = (UINT8 *)item_internal;
	return *itemdata ;

}

static INT32 retrokbd_get_state(void *device_internal, void *item_internal)
{
	UINT8 *itemdata = (UINT8 *)item_internal;
	return *itemdata ;

}

static INT32 generic_axis_get_state(void *device_internal, void *item_internal)
{
	INT32 *axisdata = (INT32 *) item_internal;
	return *axisdata;
}


static INT32 generic_button_get_state(void *device_internal, void *item_internal)
{
	INT32 *itemdata = (INT32 *) item_internal;
	return *itemdata >> 7;
}

#define input_device_item_add_mouse(a,b,c,d,e) input_device_item_add(a,b,c,d,e)

#define input_device_item_add_p1(a,b,c,d,e) input_device_item_add(a,b,c,d,e)

#define input_device_item_add_p2(a,b,c,d,e) input_device_item_add(a,b,c,d,e)

#define input_device_item_add_kbd(a,b,c,d,e) input_device_item_add(a,b,c,d,e)

static void initInput(running_machine* machine)
{
	int i,button;
	char defname[20];

   if (mouse_enable)
   {
     
      mouse_device=input_device_add(machine,DEVICE_CLASS_MOUSE,"Mice1",NULL);
      // add the axes
      input_device_item_add_mouse(mouse_device , "X", &mouseLX, ITEM_ID_XAXIS, generic_axis_get_state);
      input_device_item_add_mouse(mouse_device , "Y", &mouseLY, ITEM_ID_YAXIS, generic_axis_get_state);

      for (button = 0; button < 4; button++)
      {
         input_item_id itemid = (input_item_id) (ITEM_ID_BUTTON1+button);
         sprintf(defname, "B%d", button + 1);

         input_device_item_add_mouse(mouse_device, defname, &mouseBUT[button], itemid, generic_button_get_state);
      }
   }

   
        P1_device=input_device_add(machine,DEVICE_CLASS_KEYBOARD,"Pad1",NULL);	

	P2_device= input_device_add(machine,DEVICE_CLASS_KEYBOARD,"Pad2",NULL);
 
	if (P1_device == NULL)
		fatalerror("P1 Error creating keyboard device\n");

	if (P2_device == NULL)
			fatalerror("P2 Error creating keyboard device\n");

	// our faux keyboard only has a couple of keys (corresponding to the
	// common defaults)

   fprintf(stderr, "SOURCE FILE: %s\n", machine->gamedrv->source_file);
   fprintf(stderr, "PARENT: %s\n", machine->gamedrv->parent);
   fprintf(stderr, "NAME: %s\n", machine->gamedrv->name);
   fprintf(stderr, "DESCRIPTION: %s\n", machine->gamedrv->description);
   fprintf(stderr, "YEAR: %s\n", machine->gamedrv->year);
   fprintf(stderr, "MANUFACTURER: %s\n", machine->gamedrv->manufacturer);

	P1_state[KEY_TAB]        = 0;//RETRO_DEVICE_ID_JOYPAD_R2
	P1_state[KEY_ENTER]      = 0;//RETRO_DEVICE_ID_JOYPAD_L2
	P1_state[KEY_START]      = 0;//RETRO_DEVICE_ID_JOYPAD_START
	P1_state[KEY_COIN]       = 0;//RETRO_DEVICE_ID_JOYPAD_SELECT
   
	P1_state[KEY_BUTTON_1]   = 0;//RETRO_DEVICE_ID_JOYPAD_A
	P1_state[KEY_BUTTON_2]   = 0;//RETRO_DEVICE_ID_JOYPAD_B
  	P1_state[KEY_BUTTON_3]   = 0;//RETRO_DEVICE_ID_JOYPAD_X
 	P1_state[KEY_BUTTON_4]   = 0;//RETRO_DEVICE_ID_JOYPAD_Y
  	P1_state[KEY_BUTTON_5]   = 0;//RETRO_DEVICE_ID_JOYPAD_L
  	P1_state[KEY_BUTTON_6]   = 0;//RETRO_DEVICE_ID_JOYPAD_R
	P1_state[KEY_JOYSTICK_U] = 0;//RETRO_DEVICE_ID_JOYPAD_UP
	P1_state[KEY_JOYSTICK_D] = 0;//RETRO_DEVICE_ID_JOYPAD_DOWN
	P1_state[KEY_JOYSTICK_L] = 0;//RETRO_DEVICE_ID_JOYPAD_LEFT
	P1_state[KEY_JOYSTICK_R] = 0;//RETRO_DEVICE_ID_JOYPAD_RIGHT
   
	P2_state[KEY_START]      = 0;//RETRO_DEVICE_ID_JOYPAD_START
	P2_state[KEY_COIN]       = 0;//RETRO_DEVICE_ID_JOYPAD_SELECT
	P2_state[KEY_BUTTON_1]   = 0;//RETRO_DEVICE_ID_JOYPAD_A
	P2_state[KEY_BUTTON_2]   = 0;//RETRO_DEVICE_ID_JOYPAD_B
  	P2_state[KEY_BUTTON_3]   = 0;//RETRO_DEVICE_ID_JOYPAD_X
 	P2_state[KEY_BUTTON_4]   = 0;//RETRO_DEVICE_ID_JOYPAD_Y
  	P2_state[KEY_BUTTON_5]   = 0;//RETRO_DEVICE_ID_JOYPAD_L
  	P2_state[KEY_BUTTON_6]   = 0;//RETRO_DEVICE_ID_JOYPAD_R
	P2_state[KEY_JOYSTICK_U] = 0;//RETRO_DEVICE_ID_JOYPAD_UP
	P2_state[KEY_JOYSTICK_D] = 0;//RETRO_DEVICE_ID_JOYPAD_DOWN
	P2_state[KEY_JOYSTICK_L] = 0;//RETRO_DEVICE_ID_JOYPAD_LEFT
	P2_state[KEY_JOYSTICK_R] = 0;//RETRO_DEVICE_ID_JOYPAD_RIGHT

	input_device_item_add_p2(P2_device, "Tab", &P2_state[KEY_TAB], ITEM_ID_TAB, pad2_get_state);	
	input_device_item_add_p2(P2_device, "Enter", &P2_state[KEY_ENTER], ITEM_ID_ENTER, pad2_get_state);	
	input_device_item_add_p2(P2_device, "P2 Start", &P2_state[KEY_START], ITEM_ID_2, pad2_get_state);
	input_device_item_add_p2(P2_device, "COIN2", &P2_state[KEY_COIN], ITEM_ID_6, pad2_get_state);
	input_device_item_add_p2(P2_device, "P2 JoyU", &P2_state[KEY_JOYSTICK_U], ITEM_ID_R, pad2_get_state);
	input_device_item_add_p2(P2_device, "P2 JoyD", &P2_state[KEY_JOYSTICK_D], ITEM_ID_F, pad2_get_state);
	input_device_item_add_p2(P2_device, "P2 JoyL", &P2_state[KEY_JOYSTICK_L], ITEM_ID_D, pad2_get_state);
	input_device_item_add_p2(P2_device, "P2 JoyR", &P2_state[KEY_JOYSTICK_R], ITEM_ID_G, pad2_get_state);

	input_device_item_add_p1(P1_device, "Tab", &P1_state[KEY_TAB], ITEM_ID_TAB, pad1_get_state);	
	input_device_item_add_p1(P1_device, "Enter", &P1_state[KEY_ENTER], ITEM_ID_ENTER, pad1_get_state);	
	input_device_item_add_p1(P1_device, "P1 Start", &P1_state[KEY_START], ITEM_ID_1, pad1_get_state);	
	input_device_item_add_p1(P1_device, "COIN1", &P1_state[KEY_COIN], ITEM_ID_5, pad1_get_state);	
	input_device_item_add_p1(P1_device, "P1 JoyU", &P1_state[KEY_JOYSTICK_U], ITEM_ID_UP, pad1_get_state);
	input_device_item_add_p1(P1_device, "P1 JoyD", &P1_state[KEY_JOYSTICK_D], ITEM_ID_DOWN, pad1_get_state);
	input_device_item_add_p1(P1_device, "P1 JoyL", &P1_state[KEY_JOYSTICK_L], ITEM_ID_LEFT, pad1_get_state);
	input_device_item_add_p1(P1_device, "P1 JoyR", &P1_state[KEY_JOYSTICK_R], ITEM_ID_RIGHT, pad1_get_state);

   if ((core_stricmp(machine->gamedrv->name, "tekken") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "tekken") == 0) ||
         (core_stricmp(machine->gamedrv->name, "tekken2") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "tekken2") == 0)
         )
   {
      /* Tekken 1/2 */
      input_device_item_add_p1(P1_device, "RetroPad P1 Y", &P1_state[KEY_BUTTON_4], ITEM_ID_LCONTROL, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 X", &P1_state[KEY_BUTTON_3], ITEM_ID_LALT, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 B", &P1_state[KEY_BUTTON_2], ITEM_ID_SPACE, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 A", &P1_state[KEY_BUTTON_1], ITEM_ID_LSHIFT, pad1_get_state);

      input_device_item_add_p1(P2_device, "RetroPad P2 Y", &P2_state[KEY_BUTTON_4], ITEM_ID_A, pad2_get_state);
      input_device_item_add_p1(P2_device, "RetroPad P2 X", &P2_state[KEY_BUTTON_3], ITEM_ID_S, pad2_get_state);
      input_device_item_add_p1(P2_device, "RetroPad P2 B", &P2_state[KEY_BUTTON_2], ITEM_ID_Q, pad2_get_state);
      input_device_item_add_p1(P2_device, "RetroPad P2 A", &P2_state[KEY_BUTTON_1], ITEM_ID_W, pad2_get_state);
   }
   else
   if ((core_stricmp(machine->gamedrv->name, "souledge") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "souledge") == 0) ||
         (core_stricmp(machine->gamedrv->name, "soulclbr") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "soulclbr") == 0)

         )
   {
      /* Soul Edge/Soul Calibur */
      input_device_item_add_p1(P1_device, "RetroPad P1 Y", &P1_state[KEY_BUTTON_4], ITEM_ID_LCONTROL, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 X", &P1_state[KEY_BUTTON_3], ITEM_ID_LALT, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 A", &P1_state[KEY_BUTTON_1], ITEM_ID_SPACE, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 B", &P1_state[KEY_BUTTON_2], ITEM_ID_LSHIFT, pad1_get_state);

      input_device_item_add_p2(P2_device, "RetroPad P2 Y", &P2_state[KEY_BUTTON_4], ITEM_ID_A, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 X", &P2_state[KEY_BUTTON_3], ITEM_ID_S, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 A", &P2_state[KEY_BUTTON_1], ITEM_ID_Q, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 B", &P2_state[KEY_BUTTON_2], ITEM_ID_W, pad2_get_state);
   }
   else
   if (
         (core_stricmp(machine->gamedrv->name, "doapp") == 0)
         )
   {
      /* Dead or Alive++ */
      input_device_item_add_p1(P1_device, "RetroPad P1 B", &P1_state[KEY_BUTTON_2], ITEM_ID_LCONTROL, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 Y", &P1_state[KEY_BUTTON_4], ITEM_ID_LALT, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 X", &P1_state[KEY_BUTTON_3], ITEM_ID_SPACE, pad1_get_state);

      input_device_item_add_p2(P2_device, "RetroPad P2 B", &P2_state[KEY_BUTTON_2], ITEM_ID_A, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 Y", &P2_state[KEY_BUTTON_4], ITEM_ID_S, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 X", &P2_state[KEY_BUTTON_3], ITEM_ID_Q, pad2_get_state);
   }
   else
   if (
         (core_stricmp(machine->gamedrv->name, "vf") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "vf") == 0)
         )
   {
      /* Virtua Fighter */
      input_device_item_add_p1(P1_device, "RetroPad P1 Y", &P1_state[KEY_BUTTON_4], ITEM_ID_LCONTROL, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 X", &P1_state[KEY_BUTTON_3], ITEM_ID_LALT, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 B", &P1_state[KEY_BUTTON_2], ITEM_ID_SPACE, pad1_get_state);

      input_device_item_add_p2(P2_device, "RetroPad P2 Y", &P2_state[KEY_BUTTON_4], ITEM_ID_A, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 X", &P2_state[KEY_BUTTON_3], ITEM_ID_S, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 B", &P2_state[KEY_BUTTON_2], ITEM_ID_Q, pad2_get_state);
   }
  else
   if (
         (core_stricmp(machine->gamedrv->name, "ehrgeiz") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "ehrgeiz") == 0)
         )
   {
      /* Ehrgeiz */
      input_device_item_add_p1(P1_device, "RetroPad P1 Y", &P1_state[KEY_BUTTON_4], ITEM_ID_LCONTROL, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 B", &P1_state[KEY_BUTTON_2], ITEM_ID_LALT, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 A", &P1_state[KEY_BUTTON_1], ITEM_ID_SPACE, pad1_get_state);

      input_device_item_add_p2(P2_device, "RetroPad P2 Y", &P2_state[KEY_BUTTON_4], ITEM_ID_A, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 B", &P2_state[KEY_BUTTON_2], ITEM_ID_S, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 A", &P2_state[KEY_BUTTON_1], ITEM_ID_Q, pad2_get_state);
   }
   else
   if (
         (core_stricmp(machine->gamedrv->name, "ts2") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "ts2") == 0)
         )
   {
      /* Toshinden 2 */
      input_device_item_add_p1(P1_device, "RetroPad P1 L", &P1_state[KEY_BUTTON_5], ITEM_ID_LCONTROL, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 Y", &P1_state[KEY_BUTTON_4], ITEM_ID_LALT, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 X", &P1_state[KEY_BUTTON_3], ITEM_ID_SPACE, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 R", &P1_state[KEY_BUTTON_6], ITEM_ID_LSHIFT, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 B", &P1_state[KEY_BUTTON_2], ITEM_ID_Z, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 A", &P1_state[KEY_BUTTON_1], ITEM_ID_X, pad1_get_state);

      input_device_item_add_p2(P2_device, "RetroPad P2 L", &P2_state[KEY_BUTTON_5], ITEM_ID_A, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 Y", &P2_state[KEY_BUTTON_4], ITEM_ID_S, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 X", &P2_state[KEY_BUTTON_3], ITEM_ID_Q, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 R", &P2_state[KEY_BUTTON_6], ITEM_ID_W, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 B", &P2_state[KEY_BUTTON_2], ITEM_ID_I, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 A", &P2_state[KEY_BUTTON_1], ITEM_ID_K, pad2_get_state);
   }
   else
   if (
         (core_stricmp(machine->gamedrv->name, "vsav") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "vsav") == 0) ||
         (core_stricmp(machine->gamedrv->name, "vsav2") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "vsav2") == 0) ||
         (core_stricmp(machine->gamedrv->name, "mshvsf") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "mshvsf") == 0) ||
         (core_stricmp(machine->gamedrv->name, "msh") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "msh") == 0) ||
         (core_stricmp(machine->gamedrv->name, "mvsc") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "mvsc") == 0) ||
         (core_stricmp(machine->gamedrv->name, "xmcota") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "xmcota") == 0) ||
         (core_stricmp(machine->gamedrv->name, "sf2") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "sf2") == 0) ||
         (core_stricmp(machine->gamedrv->name, "ssf2") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "ssf2") == 0) ||
         (core_stricmp(machine->gamedrv->name, "sfa") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "sfa") == 0) ||
         (core_stricmp(machine->gamedrv->name, "sfa2") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "sfa2") == 0) ||
         (core_stricmp(machine->gamedrv->name, "sfiii") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "sfiii") == 0) ||
         (core_stricmp(machine->gamedrv->name, "sfiii2") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "sfiii2") == 0) ||
         (core_stricmp(machine->gamedrv->name, "sfiii3") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "sfiii3") == 0) ||
         (core_stricmp(machine->gamedrv->name, "xmvsf") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "xmvsf") == 0) ||
         (core_stricmp(machine->gamedrv->name, "sftm") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "sftm") == 0) ||
         (core_stricmp(machine->gamedrv->name, "dstlk") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "dstlk") == 0) ||
         (core_stricmp(machine->gamedrv->name, "nwarr") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "nwarr") == 0) ||
         (core_stricmp(machine->gamedrv->name, "sfex") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "sfex") == 0) ||
         (core_stricmp(machine->gamedrv->name, "sfexp") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "sfexp") == 0) ||
         (core_stricmp(machine->gamedrv->name, "sfex2") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "sfex2") == 0) ||
         (core_stricmp(machine->gamedrv->name, "sfex2p") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "sfex2p") == 0) ||
         (core_stricmp(machine->gamedrv->name, "rvschool") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "rvschool") == 0) ||
         (core_stricmp(machine->gamedrv->name, "starglad") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "starglad") == 0) ||
         (core_stricmp(machine->gamedrv->name, "sfa3") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "sfa3") == 0)
         )
   {
      /* Capcom CPS2 6-button fighting games */
      input_device_item_add_p1(P1_device, "RetroPad P1 Y", &P1_state[KEY_BUTTON_4], ITEM_ID_LCONTROL, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 X", &P1_state[KEY_BUTTON_3], ITEM_ID_LALT, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 L", &P1_state[KEY_BUTTON_5], ITEM_ID_SPACE, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 B", &P1_state[KEY_BUTTON_2], ITEM_ID_LSHIFT, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 A", &P1_state[KEY_BUTTON_1], ITEM_ID_Z, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 R", &P1_state[KEY_BUTTON_6], ITEM_ID_X, pad1_get_state);

      input_device_item_add_p2(P2_device, "RetroPad P2 Y", &P2_state[KEY_BUTTON_4], ITEM_ID_A, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 X", &P2_state[KEY_BUTTON_3], ITEM_ID_S, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 L", &P2_state[KEY_BUTTON_5], ITEM_ID_Q, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 B", &P2_state[KEY_BUTTON_2], ITEM_ID_W, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 A", &P2_state[KEY_BUTTON_1], ITEM_ID_I, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 R", &P2_state[KEY_BUTTON_6], ITEM_ID_K, pad2_get_state);
   }
   else
   if (
         (core_stricmp(machine->gamedrv->parent, "aof") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "aof2") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "aof3") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "fatfury1") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "fatfury2") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "fatfury3") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "garou") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "neogeo") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "mslug") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "mslug2") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "mslugx") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "mslug3") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "mslug4") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "mslug5") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "kof94") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "kof95") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "kof96") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "kof97") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "kof98") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "kof99") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "kof2000") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "kof2001") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "kof2002") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "kof2003") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "lresort") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "lastblad") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "lastbld2") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "sengoku3") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "samsho") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "samsho2") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "samsho3") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "samsho4") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "samsho5") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "matrim") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "viewpoin") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "wakuwak7") == 0)
      )
   {
      /* Neo Geo */
      input_device_item_add_p1(P1_device, "RetroPad P1 B", &P1_state[KEY_BUTTON_2], ITEM_ID_LCONTROL, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 A", &P1_state[KEY_BUTTON_1], ITEM_ID_LALT, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 Y", &P1_state[KEY_BUTTON_4], ITEM_ID_SPACE, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 X", &P1_state[KEY_BUTTON_3], ITEM_ID_LSHIFT, pad1_get_state);

      input_device_item_add_p2(P2_device, "RetroPad P2 B", &P2_state[KEY_BUTTON_2], ITEM_ID_A, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 A", &P2_state[KEY_BUTTON_1], ITEM_ID_S, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 Y", &P2_state[KEY_BUTTON_4], ITEM_ID_Q, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 X", &P2_state[KEY_BUTTON_3], ITEM_ID_W, pad2_get_state);
   }
   else
   if (
         (core_stricmp(machine->gamedrv->name, "kinst") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "kinst") == 0) ||
         (core_stricmp(machine->gamedrv->name, "kinst2") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "kinst2") == 0)
         )
   {
      /* Killer Instinct 1/2 */
      input_device_item_add_p1(P1_device, "RetroPad P1 L", &P1_state[KEY_BUTTON_5], ITEM_ID_LCONTROL, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 Y", &P1_state[KEY_BUTTON_4], ITEM_ID_LALT, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 X", &P1_state[KEY_BUTTON_3], ITEM_ID_SPACE, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 R", &P1_state[KEY_BUTTON_6], ITEM_ID_LSHIFT, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 B", &P1_state[KEY_BUTTON_2], ITEM_ID_Z, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 A", &P1_state[KEY_BUTTON_1], ITEM_ID_X, pad1_get_state);

      input_device_item_add_p2(P2_device, "RetroPad P2 L", &P2_state[KEY_BUTTON_5], ITEM_ID_A, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 Y", &P2_state[KEY_BUTTON_4], ITEM_ID_S, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 X", &P2_state[KEY_BUTTON_3], ITEM_ID_Q, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 R", &P2_state[KEY_BUTTON_6], ITEM_ID_W, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 B", &P2_state[KEY_BUTTON_2], ITEM_ID_I, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 A", &P2_state[KEY_BUTTON_1], ITEM_ID_K, pad2_get_state);
   }
   else
   if ((core_stricmp(machine->gamedrv->name, "tektagt") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "tektagt") == 0) ||
         (core_stricmp(machine->gamedrv->name, "tekken3") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "tekken3") == 0)
         )
   {
      /* Tekken 3/Tekken Tag Tournament */
      input_device_item_add_p1(P1_device, "RetroPad P1 Y", &P1_state[KEY_BUTTON_4], ITEM_ID_LCONTROL, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 X", &P1_state[KEY_BUTTON_3], ITEM_ID_LALT, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 R", &P1_state[KEY_BUTTON_6], ITEM_ID_SPACE, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 B", &P1_state[KEY_BUTTON_2], ITEM_ID_LSHIFT, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 A", &P1_state[KEY_BUTTON_1], ITEM_ID_Z, pad1_get_state);

      input_device_item_add_p2(P2_device, "RetroPad P2 Y", &P2_state[KEY_BUTTON_4], ITEM_ID_A, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 X", &P2_state[KEY_BUTTON_3], ITEM_ID_S, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 R", &P2_state[KEY_BUTTON_6], ITEM_ID_Q, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 B", &P2_state[KEY_BUTTON_2], ITEM_ID_W, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 A", &P2_state[KEY_BUTTON_1], ITEM_ID_I, pad2_get_state);
   }
   else
   if (
         (core_stricmp(machine->gamedrv->name, "mk") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "mk") == 0) ||
         (core_stricmp(machine->gamedrv->name, "mk2") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "mk2") == 0) ||
         (core_stricmp(machine->gamedrv->name, "mk3") == 0) ||
         (core_stricmp(machine->gamedrv->name, "umk3") == 0) ||
         (core_stricmp(machine->gamedrv->parent, "umk3") == 0)
      )
   {
      /* Mortal Kombat 1/2/3/Ultimate */
      input_device_item_add_p1(P1_device, "RetroPad P1 Y", &P1_state[KEY_BUTTON_4], ITEM_ID_LCONTROL, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 L", &P1_state[KEY_BUTTON_5], ITEM_ID_LALT, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 X", &P1_state[KEY_BUTTON_3], ITEM_ID_SPACE, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 B", &P1_state[KEY_BUTTON_2], ITEM_ID_LSHIFT, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 A", &P1_state[KEY_BUTTON_1], ITEM_ID_Z, pad1_get_state);
      input_device_item_add_p1(P1_device, "RetroPad P1 R", &P1_state[KEY_BUTTON_6], ITEM_ID_X, pad1_get_state);

      input_device_item_add_p2(P2_device, "RetroPad P2 Y", &P2_state[KEY_BUTTON_4], ITEM_ID_A, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 L", &P2_state[KEY_BUTTON_5], ITEM_ID_S, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 X", &P2_state[KEY_BUTTON_3], ITEM_ID_Q, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 B", &P2_state[KEY_BUTTON_2], ITEM_ID_W, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 A", &P2_state[KEY_BUTTON_1], ITEM_ID_I, pad2_get_state);
      input_device_item_add_p2(P2_device, "RetroPad P2 R", &P2_state[KEY_BUTTON_6], ITEM_ID_K, pad2_get_state);
   }
   else
   {
      /* Default config */
      input_device_item_add_p1(P1_device, "P1 B1", &P1_state[KEY_BUTTON_1], ITEM_ID_LCONTROL, pad1_get_state);
      input_device_item_add_p1(P1_device, "P1 B2", &P1_state[KEY_BUTTON_2], ITEM_ID_LALT, pad1_get_state);
      input_device_item_add_p1(P1_device, "P1 B3", &P1_state[KEY_BUTTON_3], ITEM_ID_SPACE, pad1_get_state);
      input_device_item_add_p1(P1_device, "P1 B4", &P1_state[KEY_BUTTON_4], ITEM_ID_LSHIFT, pad1_get_state);
      input_device_item_add_p1(P1_device, "P1 B5", &P1_state[KEY_BUTTON_5], ITEM_ID_Z, pad1_get_state);
      input_device_item_add_p1(P1_device, "P1 B6", &P1_state[KEY_BUTTON_6], ITEM_ID_X, pad1_get_state);

      input_device_item_add_p2(P2_device, "P2 B1", &P2_state[KEY_BUTTON_1], ITEM_ID_A, pad2_get_state);
      input_device_item_add_p2(P2_device, "P2 B2", &P2_state[KEY_BUTTON_2], ITEM_ID_S, pad2_get_state);
      input_device_item_add_p2(P2_device, "P2 B3", &P2_state[KEY_BUTTON_3], ITEM_ID_Q, pad2_get_state);
      input_device_item_add_p2(P2_device, "P2 B4", &P2_state[KEY_BUTTON_4], ITEM_ID_W, pad2_get_state);
      input_device_item_add_p2(P2_device, "P2 B5", &P2_state[KEY_BUTTON_5], ITEM_ID_I, pad2_get_state);
      input_device_item_add_p2(P2_device, "P2 B6", &P2_state[KEY_BUTTON_6], ITEM_ID_K, pad2_get_state);
   }

	retrokbd_device = input_device_add(machine,DEVICE_CLASS_KEYBOARD,"Retrokdb",NULL);

	if (retrokbd_device == NULL)
			fatalerror("KBD Error creating keyboard device\n");

	#define RETROK_TILDE 178

   for(i = 0; i < RETROK_LAST; i++)
      retrokbd_state[i]=0;

   input_device_item_add_kbd(retrokbd_device, "A", &retrokbd_state[RETROK_a],ITEM_ID_A,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "B", &retrokbd_state[RETROK_b],ITEM_ID_B,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "C", &retrokbd_state[RETROK_c],ITEM_ID_C,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "D", &retrokbd_state[RETROK_d],ITEM_ID_D,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "E", &retrokbd_state[RETROK_e],ITEM_ID_E,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "F", &retrokbd_state[RETROK_f],ITEM_ID_F,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "G", &retrokbd_state[RETROK_g],ITEM_ID_G,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "H", &retrokbd_state[RETROK_h],ITEM_ID_H,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "I", &retrokbd_state[RETROK_i],ITEM_ID_I,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "J", &retrokbd_state[RETROK_j],ITEM_ID_J,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "K", &retrokbd_state[RETROK_k],ITEM_ID_K,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "L", &retrokbd_state[RETROK_l],ITEM_ID_L,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "M", &retrokbd_state[RETROK_m],ITEM_ID_M,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "N", &retrokbd_state[RETROK_n],ITEM_ID_N,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "O", &retrokbd_state[RETROK_o],ITEM_ID_O,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "P", &retrokbd_state[RETROK_p],ITEM_ID_P,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "Q", &retrokbd_state[RETROK_q],ITEM_ID_Q,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "R", &retrokbd_state[RETROK_r],ITEM_ID_R,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "S", &retrokbd_state[RETROK_s],ITEM_ID_S,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "T", &retrokbd_state[RETROK_t],ITEM_ID_T,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "U", &retrokbd_state[RETROK_u],ITEM_ID_U,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "V", &retrokbd_state[RETROK_v],ITEM_ID_V,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "W", &retrokbd_state[RETROK_w],ITEM_ID_W,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "X", &retrokbd_state[RETROK_x],ITEM_ID_X,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "Y", &retrokbd_state[RETROK_y],ITEM_ID_Y,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "Z", &retrokbd_state[RETROK_z],ITEM_ID_Z,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "0", &retrokbd_state[RETROK_0],ITEM_ID_0,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "1", &retrokbd_state[RETROK_1],ITEM_ID_1,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "2", &retrokbd_state[RETROK_2],ITEM_ID_2,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "3", &retrokbd_state[RETROK_QUOTEDBL/*RETROK_3*/],ITEM_ID_3,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "4", &retrokbd_state[RETROK_4],ITEM_ID_4,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "5", &retrokbd_state[RETROK_5],ITEM_ID_5,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "6", &retrokbd_state[RETROK_6],ITEM_ID_6,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "7", &retrokbd_state[RETROK_7],ITEM_ID_7,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "8", &retrokbd_state[RETROK_8],ITEM_ID_8,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "9", &retrokbd_state[RETROK_9],ITEM_ID_9,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "F1", &retrokbd_state[RETROK_F1],ITEM_ID_F1,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "F2", &retrokbd_state[RETROK_F2],ITEM_ID_F2,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "F3", &retrokbd_state[RETROK_F3],ITEM_ID_F3,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "F4", &retrokbd_state[RETROK_F4],ITEM_ID_F4,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "F5", &retrokbd_state[RETROK_F5],ITEM_ID_F5,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "F6", &retrokbd_state[RETROK_F6],ITEM_ID_F6,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "F7", &retrokbd_state[RETROK_F7],ITEM_ID_F7,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "F8", &retrokbd_state[RETROK_F8],ITEM_ID_F8,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "F9", &retrokbd_state[RETROK_F9],ITEM_ID_F9,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "F10", &retrokbd_state[RETROK_F10],ITEM_ID_F10,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "F11", &retrokbd_state[RETROK_F11],ITEM_ID_F11,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "F12", &retrokbd_state[RETROK_F12],ITEM_ID_F12,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "F13", &retrokbd_state[RETROK_F13],ITEM_ID_F13,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "F14", &retrokbd_state[RETROK_F14],ITEM_ID_F14,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "F15", &retrokbd_state[RETROK_F15],ITEM_ID_F15,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "Esc", &retrokbd_state[RETROK_ESCAPE],ITEM_ID_ESC,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "TILDE", &retrokbd_state[RETROK_TILDE],ITEM_ID_TILDE,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "MINUS", &retrokbd_state[RETROK_MINUS],ITEM_ID_MINUS,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "EQUALS", &retrokbd_state[RETROK_EQUALS],ITEM_ID_EQUALS,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "BKCSPACE", &retrokbd_state[RETROK_BACKSPACE],ITEM_ID_BACKSPACE,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "TAB", &retrokbd_state[RETROK_TAB],ITEM_ID_TAB,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "(", &retrokbd_state[RETROK_LEFTPAREN],ITEM_ID_OPENBRACE,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, ")", &retrokbd_state[RETROK_RIGHTPAREN],ITEM_ID_CLOSEBRACE,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "ENTER", &retrokbd_state[RETROK_RETURN],ITEM_ID_ENTER,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "·", &retrokbd_state[RETROK_COLON],ITEM_ID_COLON,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "\'", &retrokbd_state[RETROK_QUOTE],ITEM_ID_QUOTE,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "BCKSLASH", &retrokbd_state[RETROK_BACKSLASH],ITEM_ID_BACKSLASH,retrokbd_get_state);
   //input_device_item_add_kbd(retrokbd_device, "BCKSLASH2", &retrokbd_state[RETROK_],ITEM_ID_BACKSLASH2,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, ",", &retrokbd_state[RETROK_COMMA],ITEM_ID_COMMA,retrokbd_get_state);
   //input_device_item_add_kbd(retrokbd_device, "STOP", &retrokbd_state[RETROK_],ITEM_ID_STOP,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "/", &retrokbd_state[RETROK_SLASH],ITEM_ID_SLASH,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "SPACE", &retrokbd_state[RETROK_SPACE],ITEM_ID_SPACE,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "INS", &retrokbd_state[RETROK_INSERT],ITEM_ID_INSERT,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "DEL", &retrokbd_state[RETROK_DELETE],ITEM_ID_DEL,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "HOME", &retrokbd_state[RETROK_HOME],ITEM_ID_HOME,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "END", &retrokbd_state[RETROK_END],ITEM_ID_END,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "PGUP", &retrokbd_state[RETROK_PAGEUP],ITEM_ID_PGUP,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "PGDW", &retrokbd_state[RETROK_PAGEDOWN],ITEM_ID_PGDN,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "LEFT", &retrokbd_state[RETROK_LEFT],ITEM_ID_LEFT,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "RIGHT", &retrokbd_state[RETROK_RIGHT],ITEM_ID_RIGHT,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "UP", &retrokbd_state[RETROK_UP],ITEM_ID_UP,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "DOWN", &retrokbd_state[RETROK_DOWN],ITEM_ID_DOWN,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "KO", &retrokbd_state[RETROK_KP0],ITEM_ID_0_PAD,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "K1", &retrokbd_state[RETROK_KP1],ITEM_ID_1_PAD,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "K2", &retrokbd_state[RETROK_KP2],ITEM_ID_2_PAD,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "K3", &retrokbd_state[RETROK_KP3],ITEM_ID_3_PAD,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "K4", &retrokbd_state[RETROK_KP4],ITEM_ID_4_PAD,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "K5", &retrokbd_state[RETROK_KP5],ITEM_ID_5_PAD,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "K6", &retrokbd_state[RETROK_KP6],ITEM_ID_6_PAD,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "K7", &retrokbd_state[RETROK_KP7],ITEM_ID_7_PAD,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "K8", &retrokbd_state[RETROK_KP8],ITEM_ID_8_PAD,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "K9", &retrokbd_state[RETROK_KP9],ITEM_ID_9_PAD,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "K/", &retrokbd_state[RETROK_KP_DIVIDE],ITEM_ID_SLASH_PAD,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "K*", &retrokbd_state[RETROK_KP_MULTIPLY],ITEM_ID_ASTERISK,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "K-", &retrokbd_state[RETROK_KP_MINUS],ITEM_ID_MINUS_PAD,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "K+", &retrokbd_state[RETROK_KP_PLUS],ITEM_ID_PLUS_PAD,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "KDEL", &retrokbd_state[RETROK_KP_PERIOD],ITEM_ID_DEL_PAD,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "KRTRN", &retrokbd_state[RETROK_KP_ENTER],ITEM_ID_ENTER_PAD,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "PRINT", &retrokbd_state[RETROK_PRINT],ITEM_ID_PRTSCR,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "PAUSE", &retrokbd_state[RETROK_PAUSE],ITEM_ID_PAUSE,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "LSHFT", &retrokbd_state[RETROK_LSHIFT],ITEM_ID_LSHIFT,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "RSHFT", &retrokbd_state[RETROK_RSHIFT],ITEM_ID_RSHIFT,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "LCTRL", &retrokbd_state[RETROK_LCTRL],ITEM_ID_LCONTROL,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "RCTRL", &retrokbd_state[RETROK_RCTRL],ITEM_ID_RCONTROL,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "LALT", &retrokbd_state[RETROK_LALT],ITEM_ID_LALT,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "RALT", &retrokbd_state[RETROK_RALT],ITEM_ID_RALT,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "SCRLOCK", &retrokbd_state[RETROK_SCROLLOCK],ITEM_ID_SCRLOCK,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "NUMLOCK", &retrokbd_state[RETROK_NUMLOCK],ITEM_ID_NUMLOCK,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "CPSLOCK", &retrokbd_state[RETROK_CAPSLOCK],ITEM_ID_CAPSLOCK,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "LMETA", &retrokbd_state[RETROK_LMETA],ITEM_ID_LWIN,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "RMETA", &retrokbd_state[RETROK_RMETA],ITEM_ID_RWIN,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "MENU", &retrokbd_state[RETROK_MENU],ITEM_ID_MENU,retrokbd_get_state);
   input_device_item_add_kbd(retrokbd_device, "BREAK", &retrokbd_state[RETROK_BREAK],ITEM_ID_CANCEL,retrokbd_get_state);
}

void retro_poll_mame_input()
{
	input_poll_cb();

   if (mouse_enable)
   {
      static int mbL=0,mbR=0;
      int mouse_l;
      int mouse_r;
      int16_t mouse_x;
      int16_t mouse_y;

      mouse_x = input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_X);
      mouse_y = input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_Y);
      mouse_l = input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_LEFT);
      mouse_r = input_state_cb(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_RIGHT);
      mouseLX=mouse_x*INPUT_RELATIVE_PER_PIXEL;;
      mouseLY=mouse_y*INPUT_RELATIVE_PER_PIXEL;;

      if(mbL==0 && mouse_l)
      {
         mbL=1;		
         mouseBUT[0]=0x80;
      }
      else if(mbL==1 && !mouse_l)
      {	
         mouseBUT[0]=0;
         mbL=0;
      }

      if(mbR==0 && mouse_r)
      {
         mbR=1;
         mouseBUT[1]=1;
      }
      else if(mbR==1 && !mouse_r)
      {
         mouseBUT[1]=0;
         mbR=0;
      }
   }

	P1_state[KEY_TAB] = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R2);
	P1_state[KEY_ENTER] = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L2);

	P1_state[KEY_START] = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START);
	P1_state[KEY_COIN] = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT);
	P1_state[KEY_BUTTON_1] = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0,  RETRO_DEVICE_ID_JOYPAD_A);
	P1_state[KEY_BUTTON_2] = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0,  RETRO_DEVICE_ID_JOYPAD_B);
	P1_state[KEY_BUTTON_3] = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0,  RETRO_DEVICE_ID_JOYPAD_X);
	P1_state[KEY_BUTTON_4] = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0,  RETRO_DEVICE_ID_JOYPAD_Y);
	P1_state[KEY_BUTTON_5] = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L);
	P1_state[KEY_BUTTON_6] = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R);

	P1_state[KEY_JOYSTICK_U] = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP);
	P1_state[KEY_JOYSTICK_D] = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN);
	P1_state[KEY_JOYSTICK_L] = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT);
	P1_state[KEY_JOYSTICK_R] = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT);

	P2_state[KEY_TAB] = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R2);
	P2_state[KEY_ENTER] = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L2);
	P2_state[KEY_START] = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START);
	P2_state[KEY_COIN] = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT);
	P2_state[KEY_BUTTON_1] = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0,  RETRO_DEVICE_ID_JOYPAD_A);
	P2_state[KEY_BUTTON_2] = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0,  RETRO_DEVICE_ID_JOYPAD_B);
	P2_state[KEY_BUTTON_3] = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0,  RETRO_DEVICE_ID_JOYPAD_X);
	P2_state[KEY_BUTTON_4] = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0,  RETRO_DEVICE_ID_JOYPAD_Y);
	P2_state[KEY_BUTTON_5] = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0,  RETRO_DEVICE_ID_JOYPAD_L);
	P2_state[KEY_BUTTON_6] = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0,  RETRO_DEVICE_ID_JOYPAD_R);

	P2_state[KEY_JOYSTICK_U] = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP);
	P2_state[KEY_JOYSTICK_D] = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN);
	P2_state[KEY_JOYSTICK_L] = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT);
	P2_state[KEY_JOYSTICK_R] = input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT);
}
