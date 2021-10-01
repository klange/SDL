/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2012 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org

    Based on the dspaudio driver, Oct 2004 by Hannu Savolainen 
    hannu@opensound.com

    Modified Oct 2021 by K. Lange
    klange@toaruos.org
*/
#include "SDL_config.h"

/* Allow access to a raw mixing buffer */

#include <stdio.h>	/* For perror() */
#include <string.h>	/* For strerror() */
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include "SDL_timer.h"
#include "SDL_audio.h"
#include "../SDL_audiomem.h"
#include "../SDL_audio_c.h"
#include "../SDL_audiodev_c.h"
#include "SDL_toaruaudio.h"

/* The tag name used by Toaru audio */
#define TOARU_DRIVER_NAME         "toaru"

/* Open the audio device for playback, and don't block if busy */
#define OPEN_FLAGS	O_WRONLY

/* Audio driver functions */
static int TOARU_OpenAudio(_THIS, SDL_AudioSpec *spec);
static void TOARU_WaitAudio(_THIS);
static void TOARU_PlayAudio(_THIS);
static Uint8 *TOARU_GetAudioBuf(_THIS);
static void TOARU_CloseAudio(_THIS);

/* Audio driver bootstrap functions */

static int Audio_Available(void)
{
	int fd;
	int available;

	available = 0;
	fd = open("/dev/dsp", O_WRONLY);
	if ( fd >= 0 ) {
		available = 1;
		close(fd);
	}
	return(available);
}

static void Audio_DeleteDevice(SDL_AudioDevice *device)
{
	SDL_free(device->hidden);
	SDL_free(device);
}

static SDL_AudioDevice *Audio_CreateDevice(int devindex)
{
	SDL_AudioDevice *this;

	/* Initialize all variables that we clean on shutdown */
	this = (SDL_AudioDevice *)SDL_malloc(sizeof(SDL_AudioDevice));
	if ( this ) {
		SDL_memset(this, 0, (sizeof *this));
		this->hidden = (struct SDL_PrivateAudioData *)
				SDL_malloc((sizeof *this->hidden));
	}
	if ( (this == NULL) || (this->hidden == NULL) ) {
		SDL_OutOfMemory();
		if ( this ) {
			SDL_free(this);
		}
		return(0);
	}
	SDL_memset(this->hidden, 0, (sizeof *this->hidden));
	audio_fd = -1;

	/* Set the function pointers */
	this->OpenAudio   = TOARU_OpenAudio;
	this->WaitAudio   = TOARU_WaitAudio;
	this->PlayAudio   = TOARU_PlayAudio;
	this->GetAudioBuf = TOARU_GetAudioBuf;
	this->CloseAudio  = TOARU_CloseAudio;

	this->free = Audio_DeleteDevice;

	return this;
}

AudioBootStrap TOARU_audiobootstrap = {
	TOARU_DRIVER_NAME, "ToaruOS /dev/dsp standard audio",
	Audio_Available, Audio_CreateDevice
};

/* This function waits until it is possible to write a full sound buffer */
static void TOARU_WaitAudio(_THIS)
{
	/* Not needed at all since OSS handles waiting automagically */
}

static void TOARU_PlayAudio(_THIS)
{
	if (write(audio_fd, mixbuf, mixlen)==-1)
	{
		perror("Audio write");
		this->enabled = 0;
	}

#ifdef DEBUG_AUDIO
	fprintf(stderr, "Wrote %d bytes of audio data\n", mixlen);
#endif
}

static Uint8 *TOARU_GetAudioBuf(_THIS)
{
	return(mixbuf);
}

static void TOARU_CloseAudio(_THIS)
{
	if ( mixbuf != NULL ) {
		SDL_FreeAudioMem(mixbuf);
		mixbuf = NULL;
	}
	if ( audio_fd >= 0 ) {
		close(audio_fd);
		audio_fd = -1;
	}
}

static int TOARU_OpenAudio(_THIS, SDL_AudioSpec *spec)
{

    if (spec->channels != 2)
        spec->channels = 2;

	/* Open the audio device */
	audio_fd = open("/dev/dsp", OPEN_FLAGS);
	if ( audio_fd < 0 ) {
		SDL_SetError("Couldn't open %s: %s", "/dev/dsp", strerror(errno));
		return(-1);
	}
	mixbuf = NULL;

	spec->format = AUDIO_S16;
	spec->freq = 48000;

	/* Calculate the final parameters for this audio specification */
	SDL_CalculateAudioSpec(spec);

	/* Allocate mixing buffer */
	mixlen = spec->size;
	mixbuf = (Uint8 *)SDL_AllocAudioMem(mixlen);
	if ( mixbuf == NULL ) {
		TOARU_CloseAudio(this);
		return(-1);
	}
	SDL_memset(mixbuf, spec->silence, spec->size);

	/* Get the parent process id (we're the parent of the audio thread) */
	parent = getpid();

	/* We're ready to rock and roll. :-) */
	return(0);
}

