
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#include <CNFG.h>

#include "tophat.h"
#include <GL/gl.h>
#include <chew.h>

extern th_global thg;

extern GLuint gRDShaderProg;
extern GLuint gRDBlitProg;
extern GLuint gRDShaderProgUX;
extern GLuint gRDBlitProgUX;
extern GLuint gRDBlitProgUT;
extern GLuint gRDBlitProgTex;
extern GLuint gRDLastResizeW;
extern GLuint gRDLastResizeH;
#ifdef _WIN32
#define glActiveTexture glActiveTextureCHEW
#endif

void th_tmap_draw(th_tmap *t, th_rect *cam) {
	th_image *a;
	GET_IMAGE(a, t->a.i);

	int camx = cam->x - (cam->w / 2);
	int camy = cam->y - (cam->h / 2);

	int
		tw = t->w,
		th = t->cells.len / t->w; 

	if (camx > t->pos.x + tw*t->scale * t->a.cs.x)
		return;
	if (camy > t->pos.y + th*t->scale * t->a.cs.y)
		return;

	int sx = fabs((fabs(t->pos.x)-abs(camx))) / (t->scale * t->a.cs.x);
	int sy = fabs((fabs(t->pos.y)-abs(camy))) / (t->scale * t->a.cs.y);
	int sw = cam->w/(t->scale * t->a.cs.x) + 2;
	int sh = cam->h/(t->scale * t->a.cs.y) + 2;

	if (t->pos.x>=camx)
		sx = 0;
	if (t->pos.y>=camy)
		sy = 0;

	if (sw > tw)
		sw = tw - sx;
	if (sh > th)
		sh = th - sy;

	CNFGFlushRender();

	glUseProgram(gRDBlitProg);
	glUniform4f(gRDBlitProgUX,
		1.f/gRDLastResizeW, -1.f/gRDLastResizeH,
		-0.5f+t->pos.x/(float)gRDLastResizeW, 0.5f-t->pos.y/(float)gRDLastResizeH);
	glUniform1i(gRDBlitProgUT, 0);

	glBindTexture(GL_TEXTURE_2D, a->gltexture);

	for (int i=sx; i < sx+sw; i++) for (int j=sy; j < sy+sh; j++) {
		if (t->cells.data[j*t->w+i] == 0) continue;
		int cell = t->cells.data[j * t->w + i];

		const float
			w = t->a.cs.x * thg.scaling * t->scale,
			h = t->a.cs.y * thg.scaling * t->scale,
			x = (t->pos.x - camx) * thg.scaling + w * i,
			y = (t->pos.y - camy) * thg.scaling + h * j;
		const float verts[] = {
			x, y,  x + w, y,      x + w, y + h,
			x, y,  x + w, y + h,  x, y + h	
		};
	
		th_rect r = th_atlas_get_cell(&t->a, th_atlas_nth_coords(&t->a, cell - 1));
		const float tex_verts[] = {
			r.x, r.y,  (r.x + r.w), r.y,          (r.x + r.w), (r.y + r.h),
			r.x, r.y,  (r.x + r.w), (r.y + r.h),  r.x,         (r.y + r.h)
		};

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, verts);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_TRUE, 0, tex_verts);

		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
}

static uu safe_get(uu *src, uu w, uu h, uu x, uu y) {
	if (x > w || y > h)
		return 0;

	return src[w * y + x]; 
}

enum {
	top = 1,
	right = 2,
	bot = 4,
	left = 8
};

void th_tmap_autotile(uu *tgt, uu *src, uu w, uu h, uu *tiles, uu limiter) {
	for (int x=0; x < w; x++) for (int y=0; y < h; y++) {
		if (src[x + y * w] != limiter) continue;
		int sum = 0;
		sum += top * !!safe_get(src, w, h, x, y-1);
		sum += bot * !!safe_get(src, w, h, x, y+1);
		sum += right * !!safe_get(src, w, h, x+1, y);
		sum += left * !!safe_get(src, w, h, x-1, y);
		tgt[x + y * w] = tiles[sum];
	}
}

