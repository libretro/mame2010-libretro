/**********************************************************************************************
 *
 *   DMA-driven DAC driver
 *   by Aaron Giles
 *
 **********************************************************************************************/

#pragma once

#ifndef __DMADAC_H__
#define __DMADAC_H__

#include "devlegcy.h"

DECLARE_LEGACY_SOUND_DEVICE(DMADAC, dmadac);

void dmadac_transfer(dmadac_sound_device **devlist, uint8_t num_channels, offs_t channel_spacing, offs_t frame_spacing, offs_t total_frames, int16_t *data);
void dmadac_enable(dmadac_sound_device **devlist, uint8_t num_channels, uint8_t enable);
void dmadac_set_frequency(dmadac_sound_device **devlist, uint8_t num_channels, double frequency);
void dmadac_set_volume(dmadac_sound_device **devlist, uint8_t num_channels, uint16_t volume);

#endif /* __DMADAC_H__ */
