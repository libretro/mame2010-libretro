/***************************************************************************

    devimage.c

    Legacy image device helpers.

****************************************************************************

    Copyright Miodrag Milanovic
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/


#include "emu.h"
#include "emuopts.h"
#include "devlegcy.h"
#include "hashfile.h"
#include "zippath.h"

#include "retromain.h"


//**************************************************************************
//  LEGACY IMAGE DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  legacy_image_device_config_base - constructor
//-------------------------------------------------

legacy_image_device_config_base::legacy_image_device_config_base(const machine_config &mconfig, device_type type, const char *tag, const device_config *owner, UINT32 clock, device_get_config_func get_config)
	: legacy_device_config_base(mconfig, type, tag, owner, clock, get_config),
	  device_config_image_interface(mconfig, *this),
	  m_create_option_guide(NULL),
	  m_formatlist(NULL)
{
}

//-------------------------------------------------
//  device_config_complete - update configuration
//  based on completed device setup
//-------------------------------------------------

void legacy_image_device_config_base::device_config_complete()
{
	const device_config_image_interface *image = NULL;
	int count = 0;
	int index = -1;
    image_device_format **formatptr;
    image_device_format *format;
    formatptr = &m_formatlist;
    int cnt = 0;

	m_type = static_cast<iodevice_t>(get_legacy_config_int(DEVINFO_INT_IMAGE_TYPE));
	m_readable = get_legacy_config_int(DEVINFO_INT_IMAGE_READABLE)!=0;
	m_writeable = get_legacy_config_int(DEVINFO_INT_IMAGE_WRITEABLE)!=0;
	m_creatable = get_legacy_config_int(DEVINFO_INT_IMAGE_CREATABLE)!=0;
	m_must_be_loaded = get_legacy_config_int(DEVINFO_INT_IMAGE_MUST_BE_LOADED)!=0;
	m_reset_on_load = get_legacy_config_int(DEVINFO_INT_IMAGE_RESET_ON_LOAD)!=0;
	m_has_partial_hash = get_legacy_config_int(DEVINFO_FCT_IMAGE_PARTIAL_HASH)!=0;

	m_interface_name = get_legacy_config_string(DEVINFO_STR_IMAGE_INTERFACE);

	m_file_extensions = get_legacy_config_string(DEVINFO_STR_IMAGE_FILE_EXTENSIONS);

	m_create_option_guide = reinterpret_cast<const option_guide *>(get_legacy_config_ptr(DEVINFO_PTR_IMAGE_CREATE_OPTGUIDE));

    int format_count = get_legacy_config_int(DEVINFO_INT_IMAGE_CREATE_OPTCOUNT);

	for (int i = 0; i < format_count; i++)
	{
		// only add if creatable
		if (get_legacy_config_string(DEVINFO_PTR_IMAGE_CREATE_OPTSPEC + i)) {
			// allocate a new format
			format = global_alloc_clear(image_device_format);

			// populate it
			format->m_index       = cnt;
			format->m_name        = get_legacy_config_string(DEVINFO_STR_IMAGE_CREATE_OPTNAME + i);
			format->m_description = get_legacy_config_string(DEVINFO_STR_IMAGE_CREATE_OPTDESC + i);
			format->m_extensions  = get_legacy_config_string(DEVINFO_STR_IMAGE_CREATE_OPTEXTS + i);
			format->m_optspec     = get_legacy_config_string(DEVINFO_PTR_IMAGE_CREATE_OPTSPEC + i);

			// and append it to the list
			*formatptr = format;
			formatptr = &format->m_next;
			cnt++;
		}
	}

	for (bool gotone = device_config::m_machine_config.m_devicelist.first(image); gotone; gotone = image->next(image))
	{
		if (this == image)
			index = count;
		if (image->image_type_direct() == m_type)
			count++;
	}
	if (count > 1) {
		m_instance_name.printf("%s%d", device_typename(m_type), index + 1);
		m_brief_instance_name.printf("%s%d", device_brieftypename(m_type), index + 1);
	}
	else
	{
		m_instance_name = device_typename(m_type);
		m_brief_instance_name = device_brieftypename(m_type);
	}
	// Override in case of hardcoded values
	if (strlen(get_legacy_config_string(DEVINFO_STR_IMAGE_INSTANCE_NAME))>0) {
		m_instance_name = get_legacy_config_string(DEVINFO_STR_IMAGE_INSTANCE_NAME);
	}
	if (strlen(get_legacy_config_string(DEVINFO_STR_IMAGE_BRIEF_INSTANCE_NAME))>0) {
		m_brief_instance_name = get_legacy_config_string(DEVINFO_STR_IMAGE_BRIEF_INSTANCE_NAME);
	}
}

//-------------------------------------------------
//  uses_file_extension - update configuration
//  based on completed device setup
//-------------------------------------------------

bool legacy_image_device_config_base::uses_file_extension(const char *file_extension) const
{
    bool result = FALSE;

	if (file_extension[0] == '.')
        file_extension++;

	/* find the extensions */
	astring extensions(m_file_extensions);
	char *ext = strtok((char*)extensions.cstr(),",");
	while (ext != NULL)
	{
		if (!mame_stricmp(ext, file_extension))
        {
            result = TRUE;
            break;
        }
		ext = strtok (NULL, ",");
	}
    return result;
}

//-------------------------------------------------
//  ~legacy_device_config_base - destructor
//-------------------------------------------------

legacy_image_device_config_base::~legacy_image_device_config_base()
{
    image_device_format **formatptr = &m_formatlist;

	/* free all entries */
	while (*formatptr != NULL)
	{
		image_device_format *entry = *formatptr;
		*formatptr = entry->m_next;
		global_free(entry);
	}
}

device_image_partialhash_func legacy_image_device_config_base::get_partial_hash() const
{
	return reinterpret_cast<device_image_partialhash_func>(get_legacy_config_fct(DEVINFO_FCT_IMAGE_PARTIAL_HASH));
}


//**************************************************************************
//  LIVE LEGACY IMAGE DEVICE
//**************************************************************************

//-------------------------------------------------
//  legacy_image_device_base - constructor
//-------------------------------------------------

legacy_image_device_base::legacy_image_device_base(running_machine &machine, const device_config &config)
	: legacy_device_base(machine, config),
	  device_image_interface(machine, config, *this),
	  m_is_loading(FALSE)
{
}


/****************************************************************************
    IMAGE LOADING
****************************************************************************/

/*-------------------------------------------------
    is_loaded - quick check to determine whether an
    image is loaded
-------------------------------------------------*/

bool legacy_image_device_base::is_loaded()
{
    return (m_file != NULL);
}

/*-------------------------------------------------
    load_image_by_path - loads an image with a
    specific path
-------------------------------------------------*/

image_error_t legacy_image_device_base::load_image_by_path(UINT32 open_flags, const char *path)
{
    file_error filerr = FILERR_NOT_FOUND;
    image_error_t err = IMAGE_ERROR_FILENOTFOUND;
    astring revised_path;

    /* attempt to read the file */
    filerr = zippath_fopen(path, open_flags, &m_file, &revised_path);

    /* did the open succeed? */
    switch(filerr)
    {
        case FILERR_NONE:
            /* success! */
            m_writeable = (open_flags & OPEN_FLAG_WRITE) ? 1 : 0;
            m_created = (open_flags & OPEN_FLAG_CREATE) ? 1 : 0;
            err = IMAGE_ERROR_SUCCESS;
            break;

        case FILERR_NOT_FOUND:
        case FILERR_ACCESS_DENIED:
            /* file not found (or otherwise cannot open); continue */
            err = IMAGE_ERROR_FILENOTFOUND;
            break;

        case FILERR_OUT_OF_MEMORY:
            /* out of memory */
            err = IMAGE_ERROR_OUTOFMEMORY;
            break;

        case FILERR_ALREADY_OPEN:
            /* this shouldn't happen */
            err = IMAGE_ERROR_ALREADYOPEN;
            break;

        case FILERR_FAILURE:
        case FILERR_TOO_MANY_FILES:
        case FILERR_INVALID_DATA:
        default:
            /* other errors */
            err = IMAGE_ERROR_INTERNAL;
            break;
    }

    /* if successful, set the file name */
    if (filerr == FILERR_NONE)
        set_image_filename(revised_path);

    return err;
}

/*-------------------------------------------------
    determine_open_plan - determines which open
    flags to use, and in what order
-------------------------------------------------*/

void legacy_image_device_base::determine_open_plan(int is_create, UINT32 *open_plan)
{
    int i = 0;

    /* emit flags */
    if (!is_create && m_image_config.is_readable() && m_image_config.is_writeable())
        open_plan[i++] = OPEN_FLAG_READ | OPEN_FLAG_WRITE;
    if (!is_create && !m_image_config.is_readable() && m_image_config.is_writeable())
        open_plan[i++] = OPEN_FLAG_WRITE;
    if (!is_create && m_image_config.is_readable())
        open_plan[i++] = OPEN_FLAG_READ;
    if (m_image_config.is_writeable() && m_image_config.is_creatable())
        open_plan[i++] = OPEN_FLAG_READ | OPEN_FLAG_WRITE | OPEN_FLAG_CREATE;
    open_plan[i] = 0;
}

/*-------------------------------------------------
    load_software - software image loading
-------------------------------------------------*/
bool legacy_image_device_base::load_software(char *swlist, char *swname, rom_entry *start)
{
	const rom_entry *region;
	astring regiontag;
	bool retVal = FALSE;
	for (region = start; region != NULL; region = rom_next_region(region))
	{
		/* loop until we hit the end of this region */
		const rom_entry *romp = region + 1;
		while (!ROMENTRY_ISREGIONEND(romp))
		{
			/* handle files */
			if (ROMENTRY_ISFILE(romp))
			{
				UINT32 crc = 0;
				UINT8 crcbytes[4];
				file_error filerr;

				bool has_crc = hash_data_extract_binary_checksum(ROM_GETHASHDATA(romp), HASH_CRC, crcbytes);
				if (has_crc)
					crc = (crcbytes[0] << 24) | (crcbytes[1] << 16) | (crcbytes[2] << 8) | crcbytes[3];

				astring fname(swlist, PATH_SEPARATOR, swname, PATH_SEPARATOR, ROM_GETNAME(romp));
				if (has_crc)
					filerr = mame_fopen_crc(libretro_content_directory, fname, crc, OPEN_FLAG_READ, &m_mame_file);
				else
					filerr = mame_fopen(libretro_content_directory, fname, OPEN_FLAG_READ, &m_mame_file);

				if (filerr == FILERR_NONE)
				{
					m_file = mame_core_file(m_mame_file);
					retVal = TRUE;
				}
				break; // load first item for start
			}
			romp++;	/* something else; skip */
		}
	}
	return retVal;
}

/*-------------------------------------------------
    load_internal - core image loading
-------------------------------------------------*/

bool legacy_image_device_base::load_internal(const char *path, bool is_create, int create_format, option_resolution *create_args)
{
    image_error_t err;
    UINT32 open_plan[4];
    int i;
	bool softload = FALSE;

    /* first unload the image */
    unload();

    /* clear any possible error messages */
    clear_error();

    /* we are now loading */
    m_is_loading = TRUE;

    /* record the filename */
    err = set_image_filename(path);
    if (err)
        goto done;

	/* Check if there's a software list defined for this device and use that if we're not creating an image */
	softload = load_software_part( this, path, &m_software_info_ptr, &m_software_part_ptr, &m_full_software_name );
	if (is_create || (!softload  && m_software_info_ptr==NULL))
	{
		/* determine open plan */
		determine_open_plan(is_create, open_plan);

		/* attempt to open the file in various ways */
		for (i = 0; !m_file && open_plan[i]; i++)
		{
			/* open the file */
			err = load_image_by_path(open_plan[i], path);
			if (err && (err != IMAGE_ERROR_FILENOTFOUND))
				goto done;
		}
	}

	/* Copy some image information when we have been loaded through a software list */
	if ( m_software_info_ptr )
	{
		m_longname = m_software_info_ptr->longname;
		m_manufacturer = m_software_info_ptr->publisher;
		m_year = m_software_info_ptr->year;
		//m_playable = m_software_info_ptr->supported;
	}

	/* did we fail to find the file? */
	if (!is_loaded() && !softload)
	{
		err = IMAGE_ERROR_FILENOTFOUND;
		goto done;
	}

	/* call device load or create */
	m_create_format = create_format;
	m_create_args = create_args;

	if (m_init_phase==FALSE) {
		err = (image_error_t)finish_load();
		if (err)
			goto done;
	}
    /* success! */

done:
    if (m_err) {
		if (!m_init_phase)
		{
			if (machine->phase() == MACHINE_PHASE_RUNNING)
				popmessage("Error: Unable to %s image '%s': %s\n", is_create ? "create" : "load", path, error());
			else
				mame_printf_error("Error: Unable to %s image '%s': %s", is_create ? "create" : "load", path, error());
		}
		clear();
	}
	else {
		/* do we need to reset the CPU? only schedule it if load/create is successful */
		if ((attotime_compare(timer_get_time(device().machine), attotime_zero) > 0) && m_image_config.is_reset_on_load())
			device().machine->schedule_hard_reset();
		else
		{
			if (!m_init_phase)
			{
				if (machine->phase() == MACHINE_PHASE_RUNNING)
					popmessage("Image '%s' was successfully %s.", path, is_create ? "created" : "loaded");
				else
					mame_printf_info("Image '%s' was successfully %s.\n", path, is_create ? "created" : "loaded");
			}
		}
	}
	return err ? IMAGE_INIT_FAIL : IMAGE_INIT_PASS;
}



/*-------------------------------------------------
    load - load an image into MESS
-------------------------------------------------*/

bool legacy_image_device_base::load(const char *path)
{
    return load_internal(path, FALSE, 0, NULL);
}


/*-------------------------------------------------
    image_finish_load - special call - only use
    from core
-------------------------------------------------*/

bool legacy_image_device_base::finish_load()
{
    bool err = IMAGE_INIT_PASS;

    if (m_is_loading)
    {
		image_checkhash();

        if (has_been_created() && m_config.get_legacy_config_fct(DEVINFO_FCT_IMAGE_CREATE)!=NULL)
        {
            err = call_create(m_create_format, m_create_args);
            if (err)
            {
                if (!m_err)
                    m_err = IMAGE_ERROR_UNSPECIFIED;
            }
        }
        else
        {
            /* using device load */
            err = call_load();
            if (err)
            {
                if (!m_err)
                    m_err = IMAGE_ERROR_UNSPECIFIED;
            }
        }
    }
    m_is_loading = FALSE;
    m_create_format = 0;
    m_create_args = NULL;
	m_init_phase = FALSE;
    return err;
}

/*-------------------------------------------------
    create - create a image
-------------------------------------------------*/

bool legacy_image_device_base::create(const char *path, const image_device_format *create_format, option_resolution *create_args)
{
    int format_index = (create_format != NULL) ? create_format->m_index : 0;
    return load_internal(path, TRUE, format_index, create_args);
}


/*-------------------------------------------------
    clear - clear all internal data pertaining
    to an image
-------------------------------------------------*/

void legacy_image_device_base::clear()
{
	if (m_mame_file)
    {
		mame_fclose(m_mame_file);
		m_mame_file = NULL;
		m_file = NULL;
	} else {
		if (m_file)
		{
			core_fclose(m_file);
			m_file = NULL;
		}
	}

    m_name.reset();
    m_writeable = FALSE;
    m_created = FALSE;

    m_longname.reset();
    m_manufacturer.reset();
    m_year.reset();
    m_playable.reset();
    m_extrainfo.reset();
    m_basename_noext.reset();
	m_filetype.reset();

	m_full_software_name = NULL;
	m_software_info_ptr = NULL;
	m_software_part_ptr = NULL;
}

/*-------------------------------------------------
    unload - main call to unload an image
-------------------------------------------------*/

void legacy_image_device_base::unload()
{
	if (is_loaded()) {
		call_unload();
	}
    clear();
	clear_error();
}

int legacy_image_device_base::call_load()
{
	device_image_load_func func = reinterpret_cast<device_image_load_func>(m_config.get_legacy_config_fct(DEVINFO_FCT_IMAGE_LOAD));
	if (func) {
		return (*func)(*this);
	} else {
		return FALSE;
	}
}

bool legacy_image_device_base::call_softlist_load(char *swlist, char *swname, rom_entry *start_entry)
{
	device_image_softlist_load_func func = reinterpret_cast<device_image_softlist_load_func>(m_config.get_legacy_config_fct(DEVINFO_FCT_IMAGE_SOFTLIST_LOAD));
	if (func) {
		return (*func)(*this,swlist,swname,start_entry);
	} else {
		return FALSE;
	}
}

int legacy_image_device_base::call_create(int format_type, option_resolution *format_options)
{
	device_image_create_func func = reinterpret_cast<device_image_create_func>(m_config.get_legacy_config_fct(DEVINFO_FCT_IMAGE_CREATE));
	if (func) {
		return (*func)(*this,format_type,format_options);
	} else {
		return FALSE;
	}
}

void legacy_image_device_base::call_unload()
{
	device_image_unload_func func = reinterpret_cast<device_image_unload_func>(m_config.get_legacy_config_fct(DEVINFO_FCT_IMAGE_UNLOAD));
	if (func) (*func)(*this);
}

void legacy_image_device_base::call_display()
{
	device_image_display_func func = reinterpret_cast<device_image_display_func>(m_config.get_legacy_config_fct(DEVINFO_FCT_IMAGE_DISPLAY));
	if (func) (*func)(*this);
}

device_image_partialhash_func legacy_image_device_base::get_partial_hash()
{
	return reinterpret_cast<device_image_partialhash_func>(m_config.get_legacy_config_fct(DEVINFO_FCT_IMAGE_PARTIAL_HASH));
}

void legacy_image_device_base::call_get_devices()
{
	device_image_get_devices_func func = reinterpret_cast<device_image_get_devices_func>(m_config.get_legacy_config_fct(DEVINFO_FCT_IMAGE_GET_DEVICES));
	if (func) (*func)(*this);
}

void *legacy_image_device_base::get_device_specific_call()
{
	return (void*) m_config.get_legacy_config_fct(DEVINFO_FCT_DEVICE_SPECIFIC);
}
