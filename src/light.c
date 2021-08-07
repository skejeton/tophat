#include <string.h>
#include <stdint.h>
#include <math.h>

#include "../lib/rawdraw/CNFG.h"
#include "light.h"

#define INTERP(x0, y0, x1, y1, x) (y0 + (x - x0) * ((y1 - y0) / (x1 - x0)))

extern float scaling;

void th_lightmask_clear(th_lightmask *d) {
	for (int i=0; i < d->w * d->h; i++)
		d->dots[i] = d->color;
}

void th_lightmask_draw(th_lightmask *d) {
	unsigned tex = CNFGTexImage(d->dots, d->w, d->h);
	CNFGBlitTex(tex, 0, 0,
	            d->w * scaling,
							d->h * scaling);
	CNFGDeleteTex(tex);
}

void _th_lightmask_stamp_point(th_lightmask *d, int x, int y, uint32_t color) {
	if (x < 0 || y < 0 || x >= d->w || y >= d->h)
		return;

	if ((color & 0xff) < (d->dots[y * d->w + x] & 0xff))
		d->dots[y * d->w + x] = color; 
}

void th_spotlight_stamp(th_spotlight *l, th_lightmask *d) {
	int tile_r = l->radius / d->rect_size;
	
	// this is kinda naive way to rasterize a circle, since it calls sqrt every cycle. TODO use bresenham circle
	for (int y=-tile_r; y < tile_r; y++) for (int x=-tile_r; x < tile_r; x++) {
		double dist = sqrt(x * x + y * y);

		if (dist >= tile_r) {
			continue;
		}

		uint32_t color = 0;

		if (dist > tile_r/3) { // TODO this being tweakable
			color = ((dist - tile_r/3) / (tile_r - tile_r/3)) * d->color;
		}

		if (l->tint) {
			float a = dist / tile_r;
      
			for (int i=0; i < 32; i+=8) {
				color += (int)((1 - a) * ((l->tint >> i) & 0xff)) << i;
			}
		}

		//color += (int)(0xff000000 * INTERP(0, 1000, tile_r, 0, dist) / 1000) << 24;

		_th_lightmask_stamp_point(d, l->x / d->rect_size + x, l->y / d->rect_size + y, color);
	}
}
