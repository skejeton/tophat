#include "tophat.h"

void th_transform_quad(th_quad *q, th_transform t) {
	for (uu i=0; i < 4; i++) {
		q->v[i].x *= t.scale.x;
		q->v[i].y *= t.scale.y;
		th_rotate_point(&q->v[i], t.origin, t.rot);
		q->v[i].x += t.pos.x;
		q->v[i].y += t.pos.y;
	}
}

void th_transform_rect(th_quad *q, th_transform t, th_rect r) {
	q->tl.x = r.x;
	q->tl.y = r.y;
	q->tr.x = r.x + r.w;
	q->tr.y = r.y;
	q->br.x = r.x + r.w;
	q->br.y = r.y + r.h;
	q->bl.x = r.x;
	q->bl.y = r.y + r.h;

	th_transform_quad(q, t);
}

void th_transform_vf2(th_vf2 *v, th_transform t) {
	v->x *= t.scale.x;
	v->y *= t.scale.y;
	th_rotate_point(v, t.origin, t.rot);
	v->x += t.pos.x;
	v->y += t.pos.y;
}
