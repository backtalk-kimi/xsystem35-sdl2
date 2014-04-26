/*
 * sprite_get.c: ���å�A/B���ץ饤����ͭ�ν���
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
/* $Id: sprite_get.c,v 1.1 2003/04/22 16:29:52 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <glib.h>

#include "portab.h"
#include "ags.h"
#include "sact.h"
#include "sprite.h"
#include "sactsound.h"

static void cb_defocused_swsp(gpointer s, gpointer data);
static int eventCB_GET(sprite_t *sp, agsevent_t *e);
static void cb_remove(sprite_t *sp);

/*
  �������ץ饤�Ȥξõ�
    drag���Ϥޤä����������ץ饤�ȤϾõ��
*/
static void cb_defocused_swsp(gpointer s, gpointer data) {
	sprite_t *sp = (sprite_t *)s;
	int *update = (int *)data;
	boolean oldstate = sp->show;
	
	sp->show = FALSE;
	if (oldstate != sp->show) {
		(*update)++;
		sp_updateme(sp);
	}
}


//���åȥ��ץ饤�ȤΥ��٥�Ƚ���
static int eventCB_GET(sprite_t *sp, agsevent_t *e) {
	int update = 0;
	
	switch(e->type) {
	case AGSEVENT_BUTTON_PRESS:
		if (e->d3 != AGSEVENT_BUTTON_LEFT) break;
		
		// drag���ϻ��Υޥ����ΰ��ֵ�Ͽ
		sp->u.get.dragging = TRUE;
		sp->u.get.dragstart.x = e->d1;
		sp->u.get.dragstart.y = e->d2;
		
		if (sp->cg3) {
			sp->curcg = sp->cg3;
			update++;
			sp_updateme(sp);
		}
		
		// ���ץ饤�Ȥ�ɽ����������˻��ä����
		sact.draggedsp = sp;
		sact.dropped = FALSE;
		
		// �������ץ饤�Ȥ�������ϡ��������ɽ���ˤ���
		if (sp->expsp) {
			g_slist_foreach(sp->expsp, cb_defocused_swsp, &update);
		}
		
		// SpriteSound������С�������Ĥ餹
		if (sp->numsound2) {
			ssnd_play(sp->numsound2);
		}
		
		break;
		
	case AGSEVENT_BUTTON_RELEASE:
		// �ɤΥܥ���Ǥ�ɥ�å����|��λ
		
		if (!sp->u.get.dragging) break;
		
		sact.dropped = TRUE;
		break;

	case AGSEVENT_MOUSE_MOTION:
	{
		int newx, newy;
		
		// MOUSE MOTION �� dragg��ˤ����ƤФ�ʤ�����
		// ���������
		// if (!sp->u.get.dragging) break;
		
		// �ޥ����θ��߰��֤ˤ�꿷��������׻�
		newx = sp->loc.x + (e->d1 - sp->u.get.dragstart.x);
		newy = sp->loc.y + (e->d2 - sp->u.get.dragstart.y);
		if (newx != sp->cur.x || newy != sp->cur.y) {
			sp_updateme(sp);
			sp->cur.x = newx;
			sp->cur.y = newy;
			update++;
			sp_updateme(sp);
		}
		break;
	}}
	
	return update;
}

// ���ץ饤�Ⱥ�����ν���
static void cb_remove(sprite_t *sp) {
	spev_remove_eventlistener(sp);
}

/*
  sp_new �λ��˥��ץ饤�Ȥμ�����ν����
  @param sp: ��������륹�ץ饤��
*/
int sp_get_setup(sprite_t *sp) {
	spev_add_eventlistener(sp, eventCB_GET);
	sp->remove = cb_remove;
	
	return OK;
}

