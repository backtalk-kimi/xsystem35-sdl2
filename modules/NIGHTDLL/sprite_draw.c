/*
 * sprite_draw.c: ���ץ饤�Ⱥ�����Ƽ�
 *
 * Copyright (C) 1997-1998 Masaki Chikama (Wren) <chikama@kasumi.ipl.mech.nagoya-u.ac.jp>
 *               1998-                           <masaki-c@is.aist-nara.ac.jp>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
*/
/* $Id: sprite_draw.c,v 1.1 2003/11/09 15:06:12 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <glib.h>

#include "portab.h"
#include "system.h"
#include "ags.h"
#include "graphics.h"
#include "surface.h"
#include "ngraph.h"
#include "sprite.h"

/*
 ����� sprite (�θ��ߤ�CG)�� surface0 �˽�
 @param sp: ���褹�륹�ץ饤��
 @param r : �����褹���ΰ�
*/
int sp_draw(sprite_t *sp, MyRectangle *r) {
	if (sp == NULL) return NG;
	
	return sp_draw2(sp, sp->curcg, r);
}

/*
  ����� sprite�λ����CG�� surface0 �˽�
  (���Υ��󥿡��ե������Ϥ⤦����?)

  @param sp: ���褹�륹�ץ饤��
  @param cg: ���褹��CG
 @param r : �����褹���ΰ�
*/
int sp_draw2(sprite_t *sp, cginfo_t *cg, MyRectangle *r) {
	surface_t update;
	int sx, sy, w, h, dx, dy;
	
	if (cg == NULL) return NG;
	if (cg->sf == NULL) return NG;

	// �����ΰ�γ���
	update.width  = r->width;
	update.height = r->height;
	sx = 0;
	sy = 0;
	dx = sp->cur.x - r->x;
	dy = sp->cur.y - r->y;
	w = cg->sf->width;
	h = cg->sf->height;
	
	if (!gr_clip(cg->sf, &sx, &sy, &w, &h, &update, &dx, &dy)) {
		return NG;
	}
		
	dx += r->x;
	dy += r->y;
	
	if (cg->sf->has_alpha) {
		// alpha map ��������
		gre_BlendUseAMap(sf0, dx, dy,
				 sf0, dx, dy,
				 cg->sf, sx, sy, w, h,
				 cg->sf, sx, sy,
				 sp->blendrate);
	} else {
		if (sp->blendrate == 255) {
			// alpha�ͻ��̵꤬�����
			gr_copy(sf0, dx, dy, cg->sf, sx, sy, w, h);
		} else if (sp->blendrate > 0) {
			// alpha�ͻ��꤬������
			gre_Blend(sf0, dx, dy,
				  sf0, dx, dy,
				  cg->sf, sx, sy, w, h,
				  sp->blendrate);
		}
	}
	
	WARNING("do update no=%d, sx=%d, sy=%d, w=%d, h=%d, dx=%d, dy=%d\n",
		sp->no, sx, sy, w, h, dx, dy);
	
	return OK;
}

// BlendScreen�ˤ������
int sp_draw_scg(sprite_t *sp, MyRectangle *r) {
	surface_t update;
	cginfo_t *cg;
	int sx, sy, w, h, dx, dy;
	
	if (sp == NULL) return NG;
	
	cg = sp->curcg;
	
	if (cg == NULL) return NG;
	if (cg->sf == NULL) return NG;
	
	// �����ΰ�γ���
	update.width  = r->width;
	update.height = r->height;
	sx = 0;
	sy = 0;
	dx = sp->cur.x - r->x;
	dy = sp->cur.y - r->y;
	w = cg->sf->width;
	h = cg->sf->height;
	
	if (!gr_clip(cg->sf, &sx, &sy, &w, &h, &update, &dx, &dy)) {
		return NG;
	}
		
	dx += r->x;
	dy += r->y;
	
	gre_BlendScreen(sf0, dx, dy,
			sf0, dx, dy,
			cg->sf, sx, sy, w, h);
	
	WARNING("do update no=%d, sx=%d, sy=%d, w=%d, h=%d, dx=%d, dy=%d\n",
		sp->no, sx, sy, w, h, dx, dy);
	
}
