
void osd_exit(running_machine &machine)
{	
	write_log("osd_exit called \n");

        if (our_target != NULL)
                render_target_free(our_target);
        our_target = NULL;

	global_free(P1_device);
	global_free(P2_device);
	global_free(retrokbd_device);
	global_free(mouse_device);
}

void osd_init(running_machine* machine)
{
	int gamRot=0;

 	machine->add_notifier(MACHINE_NOTIFY_EXIT, osd_exit);

	our_target = render_target_alloc(machine,NULL, 0);

	initInput(machine);

	write_log("machine screen orientation: %s \n", 
		(machine->gamedrv->flags & ORIENTATION_SWAP_XY) ? "VERTICAL" : "HORIZONTAL"
	);

        orient  = (machine->gamedrv->flags & ORIENTATION_MASK);
	vertical = (machine->gamedrv->flags & ORIENTATION_SWAP_XY);
        
        gamRot = (ROT270 == orient) ? 1 : gamRot;
        gamRot = (ROT180 == orient) ? 2 : gamRot;
        gamRot = (ROT90  == orient) ? 3 : gamRot;
        
	prep_retro_rotation(gamRot);

	write_log("osd init done\n");
}

void osd_update(running_machine *machine, int skip_redraw)
{
	const render_primitive_list *primlist;
	UINT8 *surfptr;

	if(pauseg==-1){
		machine->schedule_exit();
		return;
	}

	if (FirstTimeUpdate == 1) {
		skip_redraw = 0; //force redraw to make sure the video texture is created
	}

	if (!skip_redraw) {
		// get the minimum width/height for the current layout
		int minwidth, minheight;
		render_target_get_minimum_size(our_target, &minwidth, &minheight);
		
		if (FirstTimeUpdate == 1) {

			FirstTimeUpdate++;			
			write_log("game screen w=%i h=%i  rowPixels=%i\n", minwidth, minheight,minwidth );

			rtwi=minwidth;
			rthe=minheight;
			topw=minwidth;			

			int gamRot=0;
		        orient  = (machine->gamedrv->flags & ORIENTATION_MASK);
			vertical = (machine->gamedrv->flags & ORIENTATION_SWAP_XY);
        
		        gamRot = (ROT270 == orient) ? 1 : gamRot;
		        gamRot = (ROT180 == orient) ? 2 : gamRot;
		        gamRot = (ROT90  == orient) ? 3 : gamRot;
        
			prep_retro_rotation(gamRot);

		}

		if (minwidth != rtwi || minheight != rthe ){
			write_log("Res change: old(%d,%d) new(%d,%d) %d\n",rtwi,rthe,minwidth,minheight,topw);
			rtwi=minwidth;
			rthe=minheight;
			topw=minwidth;
		}

		// make that the size of our target
		render_target_set_bounds(our_target,rtwi,rthe, 0);

		// get the list of primitives for the target at the current size
		primlist = render_target_get_primitives(our_target);

		// lock them, and then render them
		osd_lock_acquire(primlist->lock);

		surfptr = (UINT8 *) videoBuffer;
#ifdef M16B
		rgb565_draw_primitives(primlist->head, surfptr,rtwi,rthe,rtwi);
#else
		rgb888_draw_primitives(primlist->head, surfptr, rtwi,rthe,rtwi);
#endif

		osd_lock_release(primlist->lock);

		colib_mainthread();
	} 


}  
 
//============================================================
//  osd_wait_for_debugger
//============================================================
 
void osd_wait_for_debugger(running_device *device, int firststop)
{
	// we don't have a debugger, so we just return here
}
 
//============================================================
//  update_audio_stream
//============================================================
 
void osd_update_audio_stream(running_machine *machine, INT16 *buffer, int samples_this_frame)
{
	audio_batch_cb(buffer, samples_this_frame);
}
  

//============================================================
//  set_mastervolume
//============================================================

void osd_set_mastervolume(int attenuation)
{
	// if we had actual sound output, we would adjust the global
	// volume in response to this function
}


//============================================================
//  customize_input_type_list
//============================================================
void osd_customize_input_type_list(input_type_desc *typelist)
{
	// This function is called on startup, before reading the
	// configuration from disk. Scan the list, and change the
	// default control mappings you want. It is quite possible
	// you won't need to change a thing.
}
 
