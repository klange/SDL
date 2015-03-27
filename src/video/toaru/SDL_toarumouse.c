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

#include "SDL_mouse.h"
#include "../../events/SDL_events_c.h"

#include "SDL_toarumouse_c.h"


/* The implementation dependent data for the window manager cursor */
struct WMcursor {
	int unused;
};

/* There isn't any implementation dependent data */
void TOARU_FreeWMCursor(_THIS, WMcursor *cursor)
{
	return;
}

/* There isn't any implementation dependent data */
WMcursor *TOARU_CreateWMCursor(_THIS,
		Uint8 *data, Uint8 *mask, int w, int h, int hot_x, int hot_y)
{
	return((WMcursor *)0x01);
}

static void TOARU_MoveCursor(_THIS, SDL_Cursor *cursor, int x, int y)
{
	/* do nothing */
}

void TOARU_MoveWMCursor(_THIS, int x, int y)
{
	/* do nothing */
}

int TOARU_ShowWMCursor(_THIS, WMcursor *wmcursor)
{
	return (1);
}
