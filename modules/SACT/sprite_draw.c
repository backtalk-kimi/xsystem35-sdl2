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
/* $Id: sprite_draw.c,v 1.4 2004/10/31 04:18:02 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <glib.h>

#include "portab.h"
#include "system.h"
#include "ags.h"
#include "graphics.h"
#include "sact.h"
#include "surface.h"
#include "ngraph.h"
#include "sprite.h"

static void fill_dmap(int dx ,int dy, int w, int h, WORD val);
static void fill_dmap_mask(surface_t *src, int sx, int sy, int dx ,int dy, int w, int h, WORD val);



// ����� depthmap ������
static void fill_dmap(int dx ,int dy, int w, int h, WORD val) {
	BYTE *dp, *dp_;
	int x, y;
	
	dp = dp_ = (GETOFFSET_PIXEL(sact.dmap, dx, dy));
	
	for (x = 0; x < w; x++) {
		*((WORD *)dp + x) = val;
	}
	dp += sact.dmap->bytes_per_line;
	
	for (y = 1; y < h; y++) {
		memcpy(dp, dp_, w * 2);
		dp += sact.dmap->bytes_per_line;
	}
}

// alphamap�ˤ������äơ�alpha�ͤ�0����礭���Ȥ��������depth�Ȥ���
static void fill_dmap_mask(surface_t *src, int sx, int sy, int dx ,int dy, int w, int h, WORD val) {
	BYTE *sp, *dp;
	int x, y;
	
	dp = GETOFFSET_PIXEL(sact.dmap, dx, dy);
	sp = GETOFFSET_ALPHA(src, sx, sy);
	
	for (y = 0; y < h; y++) {
		BYTE *yls = (BYTE *)(sp + y * src->width);
		WORD *yld = (WORD *)(dp + y * sact.dmap->bytes_per_line);
		for (x = 0; x < w; x++) {
			if (*yls > 0) *yld = val;
			yls++; yld++;
		}
	}
}

/*
 ����� sprite (�θ��ߤ�CG)�� surface0 �˽�
 @param sp: ���褹�륹�ץ饤��
*/
int sp_draw(sprite_t *sp) {
	if (sp == NULL) return NG;
	
	return sp_draw2(sp, sp->curcg);
}

/*
  ����� sprite�λ����CG�� surface0 �˽�
  (���Υ��󥿡��ե������Ϥ⤦����?)

  @param sp: ���褹�륹�ץ饤��
  @param cg: ���褹��CG
*/
int sp_draw2(sprite_t *sp, cginfo_t *cg) {
	surface_t update;
	int sx, sy, w, h, dx, dy;
	
	if (cg == NULL) return NG;
	if (cg->sf == NULL) return NG;

	// �����ΰ�γ���
	update.width  = sact.updaterect.width;
	update.height = sact.updaterect.height;
	sx = 0;
	sy = 0;
	dx = sp->cur.x - sact.updaterect.x;
	dy = sp->cur.y - sact.updaterect.y;
	w = cg->sf->width;
	h = cg->sf->height;
	
	if (!gr_clip(cg->sf, &sx, &sy, &w, &h, &update, &dx, &dy)) {
		return NG;
	}
		
	dx += sact.updaterect.x;
	dy += sact.updaterect.y;
	
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
	
	WARNING("do update no=%d, sx=%d, sy=%d, w=%d, h=%d, dx=%d, dy=%d\n", sp->no, sx, sy, w, h, dx, dy);
	
	return OK;
}

/*
  ���ץ饤�ȥ����Ԥ��Ѥ�depthmap �򹹿�
*/
void sp_draw_dmap(gpointer data, gpointer userdata) {
	sprite_t *sp = (sprite_t *)data;
	cginfo_t *cg;
	surface_t update;
	int sx, sy, w, h, dx, dy;
	
	// ��ɽ�����֤λ���̵��
	if (!sp->show) return;
	
	// �ɥ�å���Υ��ץ饤�Ȥ�̵��
	if (sp == sact.draggedsp) return;
	
	cg = sp->curcg;
	if (cg == NULL) return;
	if (cg->sf == NULL) return;
	
	// depth map ����ΰ�����
	update.width  = sf0->width;
	update.height = sf0->height;
	sx = 0;
	sy = 0;
	dx = sp->cur.x;
	dy = sp->cur.y;
	w = cg->sf->width;
	h = cg->sf->height;
	
	if (!gr_clip(cg->sf, &sx, &sy, &w, &h, &update, &dx, &dy)) {
		return;
	}
	
	if (cg->sf->has_alpha) {
		fill_dmap_mask(cg->sf, sx, sy, dx, dy, w, h, sp->no);
	} else {
		fill_dmap(dx, dy, w, h, sp->no);
	}
	
	return;
}
