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

/* Dummy SDL video driver implementation; this is just enough to make an
 *  SDL-based application THINK it's got a working video driver, for
 *  applications that call SDL_Init(SDL_INIT_VIDEO) when they don't need it,
 *  and also for use as a collection of stubs when porting SDL to a new
 *  platform for which you haven't yet written a valid video driver.
 *
 * This is also a great way to determine bottlenecks: if you think that SDL
 *  is a performance problem for a given platform, enable this driver, and
 *  then see if your application runs faster without video overhead.
 *
 * Initial work by Ryan C. Gordon (icculus@icculus.org). A good portion
 *  of this was cut-and-pasted from Stephane Peter's work in the AAlib
 *  SDL video driver.  Renamed to "TOARU" by Kevin Lange.
 */

#include "SDL_video.h"
#include "SDL_mouse.h"
#include "../SDL_sysvideo.h"
#include "../SDL_pixels_c.h"
#include "../../events/SDL_events_c.h"

#include "SDL_toaruvideo.h"
#include "SDL_toaruevents_c.h"
#include "SDL_toarumouse_c.h"

#include <toaru/yutani.h>
#include <toaru/graphics.h>
#include <toaru/decorations.h>

#define TOARUVID_DRIVER_NAME "toaru"

/* Initialization/Query functions */
static int TOARU_VideoInit(_THIS, SDL_PixelFormat *vformat);
static SDL_Rect **TOARU_ListModes(_THIS, SDL_PixelFormat *format, Uint32 flags);
static SDL_Surface *TOARU_SetVideoMode(_THIS, SDL_Surface *current, int width, int height, int bpp, Uint32 flags);
static int TOARU_SetColors(_THIS, int firstcolor, int ncolors, SDL_Color *colors);
static void TOARU_VideoQuit(_THIS);

/* Hardware surface functions */
static int TOARU_AllocHWSurface(_THIS, SDL_Surface *surface);
static int TOARU_LockHWSurface(_THIS, SDL_Surface *surface);
static void TOARU_UnlockHWSurface(_THIS, SDL_Surface *surface);
static void TOARU_FreeHWSurface(_THIS, SDL_Surface *surface);

static void TOARU_SetCaption(_THIS, const char *title, const char *icon);

/* etc. */
static void TOARU_UpdateRects(_THIS, int numrects, SDL_Rect *rects);

/* TOARU driver bootstrap functions */

static int TOARU_Available(void)
{
	return 1;
}

static void TOARU_DeleteDevice(SDL_VideoDevice *device)
{
	SDL_free(device->hidden);
	SDL_free(device);
}

static SDL_VideoDevice *TOARU_CreateDevice(int devindex)
{
	SDL_VideoDevice *device;

	/* Initialize all variables that we clean on shutdown */
	device = (SDL_VideoDevice *)SDL_malloc(sizeof(SDL_VideoDevice));
	if ( device ) {
		SDL_memset(device, 0, (sizeof *device));
		device->hidden = (struct SDL_PrivateVideoData *)
				SDL_malloc((sizeof *device->hidden));
	}
	if ( (device == NULL) || (device->hidden == NULL) ) {
		SDL_OutOfMemory();
		if ( device ) {
			SDL_free(device);
		}
		return(0);
	}
	SDL_memset(device->hidden, 0, (sizeof *device->hidden));

	/* Set the function pointers */
	device->VideoInit = TOARU_VideoInit;
	device->ListModes = TOARU_ListModes;
	device->SetVideoMode = TOARU_SetVideoMode;
	device->CreateYUVOverlay = NULL;
	device->SetColors = TOARU_SetColors;
	device->UpdateRects = TOARU_UpdateRects;
	device->VideoQuit = TOARU_VideoQuit;
	device->AllocHWSurface = TOARU_AllocHWSurface;
	device->CheckHWBlit = NULL;
	device->FillHWRect = NULL;
	device->SetHWColorKey = NULL;
	device->SetHWAlpha = NULL;
	device->LockHWSurface = TOARU_LockHWSurface;
	device->UnlockHWSurface = TOARU_UnlockHWSurface;
	device->FlipHWSurface = NULL;
	device->FreeHWSurface = TOARU_FreeHWSurface;
	device->SetCaption = NULL;
	device->SetIcon = NULL;
	device->IconifyWindow = NULL;
	device->GrabInput = NULL;
	device->GetWMInfo = NULL;
	device->FreeWMCursor = TOARU_FreeWMCursor;
	device->CreateWMCursor = TOARU_CreateWMCursor;
	device->ShowWMCursor = TOARU_ShowWMCursor;
	device->MoveWMCursor = TOARU_MoveWMCursor;
	device->InitOSKeymap = TOARU_InitOSKeymap;
	device->PumpEvents = TOARU_PumpEvents;
	device->SetCaption = TOARU_SetCaption;

	device->free = TOARU_DeleteDevice;

	return device;
}

VideoBootStrap TOARU_bootstrap = {
	TOARUVID_DRIVER_NAME, "SDL ToaruOS video driver",
	TOARU_Available, TOARU_CreateDevice
};


int TOARU_VideoInit(_THIS, SDL_PixelFormat *vformat)
{
	fprintf(stderr, "Congratulations, you are using the とあるOS SDL Video Driver!\n");

	vformat->BitsPerPixel = 32;
	vformat->BytesPerPixel = 4;

	this->hidden->yctx = yutani_init();
	init_decorations();
	this->hidden->triggered_resize = 0;

	/* We're done! */
	return(0);
}

SDL_Rect **TOARU_ListModes(_THIS, SDL_PixelFormat *format, Uint32 flags)
{
   	 return (SDL_Rect **) -1;
}

SDL_Surface *TOARU_SetVideoMode(_THIS, SDL_Surface *current,
				int width, int height, int bpp, Uint32 flags)
{
	if ( this->hidden->window) {
		fprintf(stderr, "Resize request to %d x %d.\n", width, height);

		if (!this->hidden->triggered_resize) {
			yutani_window_t * w = (yutani_window_t *) this->hidden->window;
			this->hidden->triggered_resize = 2;
			fprintf(stderr, "Requesting window resize.\n");
			yutani_window_resize(this->hidden->yctx, w, width + this->hidden->x_w, height + this->hidden->x_h);

			yutani_msg_t * m = yutani_wait_for(this->hidden->yctx, YUTANI_MSG_RESIZE_OFFER);
			struct yutani_msg_window_resize * wr = (void*)m->data;
			width = wr->width - this->hidden->x_w;
			height = wr->height - this->hidden->x_h;
			free(m);
		} else {
			this->hidden->triggered_resize = 0;
		}

		yutani_window_resize_accept(this->hidden->yctx, this->hidden->window, width + this->hidden->x_w, height + this->hidden->x_h);

		fprintf(stderr, "Reinitializing graphics context...\n");
		reinit_graphics_yutani(this->hidden->ctx, this->hidden->window);

		if (this->hidden->bordered) {
			this->hidden->buffer = realloc(this->hidden->buffer, sizeof(uint32_t) * width * height);
			this->hidden->redraw_borders = 1;
		}
		yutani_window_resize_done(this->hidden->yctx, this->hidden->window);
	} else {

		if (flags & SDL_NOFRAME) {
			fprintf(stderr, "Initializing without borders.\n");
			this->hidden->bordered = 0;
			this->hidden->x_h = 0;
			this->hidden->x_w = 0;
			this->hidden->o_h = 0;
			this->hidden->o_w = 0;
		} else {
			fprintf(stderr, "Initializing with borders.\n");
			this->hidden->bordered = 1;
			this->hidden->x_h = decor_height();
			this->hidden->x_w = decor_width();
			this->hidden->o_h = decor_top_height;
			this->hidden->o_w = decor_left_width;
			this->hidden->redraw_borders = 1;
		}

		fprintf(stderr, "Initializing window %dx%d (%d bpp)\n", width, height, bpp);

		yutani_window_t * win = yutani_window_create(this->hidden->yctx, width + this->hidden->x_w, height + this->hidden->x_h);

		gfx_context_t * ctx = init_graphics_yutani_double_buffer(win);
		this->hidden->window = (void *)win;
		this->hidden->ctx    = (void *)ctx;

		if (this->hidden->bordered) {
			this->hidden->buffer = malloc(sizeof(uint32_t) * width * height);
		} else {
			this->hidden->buffer = ((gfx_context_t *)this->hidden->ctx)->backbuffer;
		}


		fprintf(stderr, "Window output initialized...\n");
	}

	/* Set up the new mode framebuffer */
	current->flags = flags;
	this->hidden->w = current->w = width;
	this->hidden->h = current->h = height;
	current->pitch = current->w * (4);
	current->pixels = this->hidden->buffer;

	/* We're done */
	return(current);
}

/* We don't actually allow hardware surfaces other than the main one */
static int TOARU_AllocHWSurface(_THIS, SDL_Surface *surface)
{
	return(-1);
}
static void TOARU_FreeHWSurface(_THIS, SDL_Surface *surface)
{
	return;
}

/* We need to wait for vertical retrace on page flipped displays */
static int TOARU_LockHWSurface(_THIS, SDL_Surface *surface)
{
	return(0);
}

static void TOARU_UnlockHWSurface(_THIS, SDL_Surface *surface)
{
	return;
}

static void TOARU_UpdateRects(_THIS, int numrects, SDL_Rect *rects)
{
	if (!this->hidden->window)
		return;

	int y = 0;
	int x = 0;
	if (this->hidden->bordered) {
		gfx_context_t * ctx = (gfx_context_t *)this->hidden->ctx;
		if (this->hidden->redraw_borders) {
			if (this->hidden->title) {
				render_decorations(this->hidden->window, this->hidden->ctx, this->hidden->title);
			} else {
				render_decorations(this->hidden->window, this->hidden->ctx, "[SDL App]");
			}
			this->hidden->redraw_borders = 0;
		}
		flip(this->hidden->ctx);
		for (y = 0; y < this->hidden->h; ++y) {
			for (x = 0; x < this->hidden->w; ++x) {
				GFX(ctx, x + this->hidden->o_w, y + this->hidden->o_h) = ((uint32_t *)this->hidden->buffer)[y * this->hidden->w + x] | 0xFF000000;
			}
		}
	} else {
		gfx_context_t * ctx = (gfx_context_t *)this->hidden->ctx;
		for (y = 0; y < ctx->height; ++y) {
			for (x = 0; x < ctx->width; ++x) {
				GFX(ctx, x, y) = GFX(ctx,x,y) | 0xFF000000;
			}
		}
		flip(this->hidden->ctx);
	}
	yutani_flip(this->hidden->yctx, this->hidden->window);

}

int TOARU_SetColors(_THIS, int firstcolor, int ncolors, SDL_Color *colors)
{
	/* We don't support palettes... */
	return(1);
}

/* Note:  If we are terminated, this could be called in the middle of
   another SDL video routine -- notably UpdateRects.
*/
void TOARU_VideoQuit(_THIS)
{
	/* XXX close windows */
}

static void TOARU_SetCaption(_THIS, const char *title, const char *icon) {
	if (this->hidden->title) {
		free(this->hidden->title);
	}
	this->hidden->title = strdup(title);
	this->hidden->redraw_borders = 1;
}

