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

/* Being a null driver, there's no event stream. We just define stubs for
   most of the API. */

#include "SDL.h"
#include "../../events/SDL_sysevents.h"
#include "../../events/SDL_events_c.h"

#include "SDL_toaruvideo.h"
#include "SDL_toaruevents_c.h"

#include <toaru/window.h>
#include <toaru/kbd.h>

static SDLKey keymap[256];
static SDLMod modstate = KMOD_NONE;

int mouse_state = 0;

void TOARU_PumpEvents(_THIS) {
	wins_packet_t * event = get_window_events_async();
	while (event) {
		switch (event->command_type & WE_GROUP_MASK) {
			case WE_WINDOW_EVT:
				{
					w_window_t * evt = (w_window_t *)((uintptr_t)event + sizeof(wins_packet_t));
					window_t * window = NULL;
					switch (event->command_type) {
						case WE_RESIZED:
							fprintf(stderr, "[SDL] Received RESIZE message from server.\n");
							if (this->hidden->triggered_resize != 2) {
								fprintf(stderr, "--- From a natural resize. Informing SDL.\n");
								this->hidden->triggered_resize = 1;
								SDL_PrivateResize(evt->width - this->hidden->x_w, evt->height - this->hidden->x_h);
							} else {
								this->hidden->triggered_resize = 0;
							}
							break;
						case WE_FOCUSCHG:
							window = wins_get_window(evt->wid);
							if (window) {
								window->focused = evt->left;
							}
							this->hidden->redraw_borders = 1;
							SDL_PrivateAppActive(0, evt->left);
							break;
					}
					break;
				}
			case WE_KEY_EVT:
				{
					w_keyboard_t * kbd = (w_keyboard_t *)((uintptr_t)event + sizeof(wins_packet_t));
					if (kbd->event.keycode) {
						SDL_keysym keysym;
						int action = kbd->event.action == KEY_ACTION_DOWN ? SDL_PRESSED : SDL_RELEASED;
						keysym.scancode = kbd->event.keycode;
						keysym.unicode  = 0;
						keysym.mod      = KMOD_NONE;
						switch (kbd->event.keycode) {
							case '\n':
								keysym.sym      = SDLK_RETURN;
								SDL_PrivateKeyboard(action, &keysym);
								break;
							case ' ':
								keysym.sym      = SDLK_SPACE;
								SDL_PrivateKeyboard(action, &keysym);
								break;
							case KEY_ARROW_UP:
								keysym.sym      = SDLK_UP;
								SDL_PrivateKeyboard(action, &keysym);
								break;
							case KEY_ARROW_LEFT:
								keysym.sym      = SDLK_LEFT;
								SDL_PrivateKeyboard(action, &keysym);
								break;
							case KEY_ARROW_RIGHT:
								keysym.sym      = SDLK_RIGHT;
								SDL_PrivateKeyboard(action, &keysym);
								break;
							case KEY_ARROW_DOWN:
								keysym.sym      = SDLK_DOWN;
								SDL_PrivateKeyboard(action, &keysym);
								break;
							case KEY_ESCAPE:
								keysym.sym      = SDLK_ESCAPE;
								SDL_PrivateKeyboard(action, &keysym);
								break;
							case KEY_BACKSPACE:
								keysym.sym      = SDLK_BACKSPACE;
								SDL_PrivateKeyboard(action, &keysym);
								break;
							case '\t':
								keysym.sym      = SDLK_TAB;
								SDL_PrivateKeyboard(action, &keysym);
								break;
							case '0': case '1': case '2': case '3': case '4':
							case '5': case '6': case '7': case '8': case '9':
							case 'a': case 'b': case 'c': case 'd': case 'e':
							case 'f': case 'g': case 'h': case 'i': case 'j':
							case 'k': case 'l': case 'm': case 'n': case 'o':
							case 'p': case 'q': case 'r': case 's': case 't':
							case 'u': case 'v': case 'w': case 'x': case 'y':
							case 'z':
							case ':': case ';': case '<': case '=': case '>':
							case '?': case '@': case '[': case ']': case '\\':
							case '^': case '_': case '`': case '.': case '/':
							case '*': case '-': case '+': case '#': case '"':
							case '!': case '&': case '$': case '(': case '\'':
								keysym.sym      = (kbd->event.keycode - 'a' + SDLK_a);
								SDL_PrivateKeyboard(action, &keysym);
								break;
							default:
								fprintf(stderr, "[SDL] Key press: %d ('%c')\n", kbd->event.keycode, kbd->key);
								break;
						}
					}
					break;
				}
			case WE_MOUSE_EVT:
				{
					w_mouse_t * mouse = (w_mouse_t *)((uintptr_t)event + sizeof(wins_packet_t));
					int i;
					signed short x = mouse->new_x - this->hidden->o_w;
					signed short y = mouse->new_y - this->hidden->o_h;
					for (i = 0; i < 3; ++i) {
						int was = mouse_state & (1 << i);
						int is  = mouse->buttons & (1 << i);
						if (is && (was != is)) {
							SDL_PrivateMouseButton(SDL_PRESSED, i + 1, x, y);
						} else if ((was) && (was != is)) {
							SDL_PrivateMouseButton(SDL_RELEASED, i + 1, x, y);
						} else if (was != is) {
							SDL_PrivateMouseButton(SDL_RELEASED, i + 1, x, y);
						}
						mouse_state = mouse->buttons;
					}
					SDL_PrivateMouseMotion(0, 0, x, y);
					break;
				}
		}
		free(event);
		event = get_window_events_async();
	}
}

void TOARU_InitOSKeymap(_THIS)
{
	/* do nothing. */
}

/* end of SDL_nullevents.c ... */

