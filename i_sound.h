/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *  Copyright 2023 by
 *  Frenkel Smeijers
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *      System interface, sound.
 *
 *-----------------------------------------------------------------------------*/

#ifndef __I_SOUND__
#define __I_SOUND__

#include "sounds.h"
#include "doomtype.h"

// Init at program start...
void I_InitSound(void);

//
//  SFX I/O
//


// Starts a sound in a particular sound channel.
int16_t I_StartSound(sfxenum_t id, int16_t channel, int32_t vol, int16_t sep);


//
//  MUSIC I/O
//

// Volume.
void I_SetMusicVolume(int32_t volume);


// Called by anything that wishes to start music.
//  plays a song, and when the song is done,
//  starts playing it again in an endless loop.
// Horrible thing to do, considering.
void I_PlaySong(int32_t handle, int32_t looping);

// Stops a song over 3 seconds.
void I_StopSong(int32_t handle);

// CPhipps - put these in config file
extern const int32_t snd_samplerate;


typedef struct
{
    const char* data;
    const char* enddata;
    int32_t vol;

} channel_info_t;

#define MUSIC_BUFFER_SAMPLES 2048
#define MAX_CHANNELS    8


#endif
