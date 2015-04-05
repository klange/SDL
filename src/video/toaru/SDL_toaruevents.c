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
#include <toaru/decorations.h>
#include <toaru/kbd.h>

static SDLKey keymap[256];
static SDLMod modstate = KMOD_NONE;

int mouse_state = 0;

static SDLMod make_mod(unsigned int m) {
	SDLMod ret = KMOD_NONE;

	if (m & KEY_MOD_LEFT_CTRL)   ret |= KMOD_LCTRL;
	if (m & KEY_MOD_LEFT_SHIFT)  ret |= KMOD_LSHIFT;
	if (m & KEY_MOD_LEFT_ALT)    ret |= KMOD_LALT;
	if (m & KEY_MOD_LEFT_SUPER)  ret |= KMOD_LMETA;
	if (m & KEY_MOD_RIGHT_CTRL)  ret |= KMOD_RCTRL;
	if (m & KEY_MOD_RIGHT_SHIFT) ret |= KMOD_RSHIFT;
	if (m & KEY_MOD_RIGHT_ALT)   ret |= KMOD_RALT;
	if (m & KEY_MOD_RIGHT_SUPER) ret |= KMOD_RMETA;

	return ret;
}

void TOARU_PumpEvents(_THIS) {
	yutani_msg_t * m;
	do {
		m = yutani_poll_async(this->hidden->yctx);

		if (!m) break;

		switch (m->type) {
			case YUTANI_MSG_KEY_EVENT:
				{
					struct yutani_msg_key_event * ke = (void*)m->data;
					if (ke->event.keycode) {
						SDL_keysym keysym;
						int action = ke->event.action == KEY_ACTION_DOWN ? SDL_PRESSED : SDL_RELEASED;
						keysym.scancode = ke->event.keycode;
						keysym.unicode  = 0;
						keysym.mod      = make_mod(ke->event.modifiers); /* TODO */
						switch (ke->event.keycode) {
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
							case KEY_LEFT_CTRL:
								keysym.sym      = SDLK_LCTRL;
								SDL_PrivateKeyboard(action, &keysym);
								break;
							case KEY_LEFT_SHIFT:
								keysym.sym      = SDLK_LSHIFT;
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
								keysym.sym      = (ke->event.keycode - 'a' + SDLK_a);
								SDL_PrivateKeyboard(action, &keysym);
								break;
							default:
								fprintf(stderr, "[SDL] Key press: %d ('%c')\n", ke->event.keycode, ke->event.key);
								break;
						}
					}
				}
				break;
			case YUTANI_MSG_WINDOW_FOCUS_CHANGE:
				{
					struct yutani_msg_window_focus_change * fc = (void*)m->data;
					yutani_window_t * w = hashmap_get(this->hidden->yctx->windows, (void*)fc->wid);
					if (w == this->hidden->window) {
						w->focused = fc->focused;
						this->hidden->redraw_borders = 1;
						SDL_PrivateAppActive(0, fc->focused);
					}
				}
				break;
			case YUTANI_MSG_RESIZE_OFFER:
				{
					struct yutani_msg_window_resize * wr = (void*)m->data;
					yutani_window_t * w = hashmap_get(this->hidden->yctx->windows, (void*)wr->wid);
					if (w == this->hidden->window) {
						if (this->hidden->triggered_resize != 2) {
							this->hidden->triggered_resize = 1;
							SDL_PrivateResize(wr->width - this->hidden->x_w, wr->height - this->hidden->x_h);
						}
					}
				}
				break;
			case YUTANI_MSG_WINDOW_MOUSE_EVENT:
				{
					struct yutani_msg_window_mouse_event * me = (void*)m->data;
					if (this->hidden->bordered && decor_handle_event(this->hidden->yctx, m) == DECOR_CLOSE) {
						SDL_PrivateQuit();
						break;
					}

					int i;
					signed int x = me->new_x - this->hidden->o_w;
					signed int y = me->new_y - this->hidden->o_h;

#define CONVERT_MOUSE(i) (i == 0 ? 1 : (i == 1 ? 3 : (i == 2 ? 2 : 0)))

					for (i = 0; i < 3; ++i) {
						int was = mouse_state & (1 << i);
						int is  = me->buttons & (1 << i);
						if (is && (was != is)) {
							SDL_PrivateMouseButton(SDL_PRESSED, CONVERT_MOUSE(i), x, y);
						} else if ((was) && (was != is)) {
							SDL_PrivateMouseButton(SDL_RELEASED, CONVERT_MOUSE(i), x, y);
						} else if (was != is) {
							SDL_PrivateMouseButton(SDL_RELEASED, CONVERT_MOUSE(i), x, y);
						}
					}
					mouse_state = me->buttons;

					SDL_PrivateMouseMotion(0, 0, x, y);
				}
				break;
			case YUTANI_MSG_SESSION_END:
				fprintf(stderr, "[sdl-toaru] Need to implement: %u\n", (unsigned int)m->type);
				break;
			case YUTANI_MSG_WINDOW_MOVE:
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

