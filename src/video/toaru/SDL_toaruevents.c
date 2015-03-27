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
*/
#include "SDL_config.h"

#include "SDL.h"
#include "../../events/SDL_sysevents.h"
#include "../../events/SDL_events_c.h"

#include "SDL_toaruvideo.h"
#include "SDL_toaruevents_c.h"

#include <toaru/yutani.h>
#include <toaru/kbd.h>

static SDLKey keymap[256];
static SDLMod modstate = KMOD_NONE;

int mouse_state = 0;

void TOARU_PumpEvents(_THIS) {
	yutani_msg_t * m;
	do {
		m = yutani_poll_async(this->hidden->yctx);

		if (!m) break;

		switch (m->type) {
			case YUTANI_MSG_KEY_EVENT:
				{
				}
				break;
			case YUTANI_MSG_WINDOW_FOCUS_CHANGE:
			case YUTANI_MSG_RESIZE_OFFER:
			case YUTANI_MSG_WINDOW_MOUSE_EVENT:
			case YUTANI_MSG_SESSION_END:
				fprintf(stderr, "[sdl-toaru] Need to implement: %u\n", (unsigned int)m->type);
				break;
			default:
				fprintf(stderr, "[sdl-toaru] Unhandled message from Yutani server: type=0x%x\n", (unsigned int)m->type);
				break;
		}

		free(m);
	} while (m);


}

void TOARU_InitOSKeymap(_THIS)
{
	/* do nothing. */
}

/* end of SDL_nullevents.c ... */

