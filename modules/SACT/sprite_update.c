/*
 * sprite_update.c: sprite���̾ﹹ��������
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
/* $Id: sprite_update.c,v 1.1 2003/04/22 16:29:52 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <glib.h>

#include "portab.h"
#include "system.h"
#include "counter.h"
#include "ags.h"
#include "graphics.h"
#include "sact.h"
#include "surface.h"
#include "ngraph.h"
#include "sprite.h"

// ���ץ饤�Ⱥ�����֤δ֤��ѹ��Τ��ä����ץ饤�Ȥ��ΰ����
static GSList *updatearea;

static void intersection(MyRectangle *r1, MyRectangle *r2, MyRectangle *rst);
static void disjunction(gpointer region, gpointer data);
static void get_updatearea();
static void do_update_each(gpointer data, gpointer userdata);

// �ΰ裱���ΰ裲���Ѥ�׻�
static void intersection(MyRectangle *r1, MyRectangle *r2, MyRectangle *rst) {
        int x1 = max(r1->x, r2->x);
        int x2 = min(r1->x + r1->width, r2->x + r2->width);
        int y1 = max(r1->y, r2->y);
        int y2 = min(r1->y + r1->height, r2->y + r2->height);
	
        rst->x = x1;
	rst->y = y1;
	rst->width  = x2 - x1;
	rst->height = y2 - y1;
}

// �ΰ裱���ΰ裲�򤹤٤ƴޤ����ΰ��׻�
static void disjunction(gpointer region, gpointer data) {
	MyRectangle *r1 = (MyRectangle *)region;
	MyRectangle *r2 = (MyRectangle *)data;
	int x1, x2, y1, y2;
	
	//WARNING("r1x=%d,r1y=%d,r1w=%d,r1h=%d\n", r1->x, r1->y, r1->width, r1->height);
	//WARNING("r2x=%d,r2y=%d,r2w=%d,r2h=%d\n", r2->x, r2->y, r2->width, r2->height);
	
	if (r2->width == 0) {
		r2->x = r1->x;
		r2->y = r1->y;
		r2->width = r1->width;
		r2->height = r1->height;
		return;
	}
	
	x1 = min(r1->x, r2->x);
	x2 = max(r1->x + r1->width,  r2->x + r2->width);
	y1 = min(r1->y, r2->y);
	y2 = max(r1->y + r1->height, r2->y + r2->height);
	
	r2->x = x1;
	r2->y = y1;
	r2->width  = x2 - x1;
	r2->height = y2 - y1;
	
	//WARNING("res:r2x=%d,r2y=%d,r2w=%d,r2h=%d\n", r2->x, r2->y, r2->width, r2->height);
}

// ������ɬ�פʥ��ץ饤�Ȥ��ΰ���¤�Ȥäƥ���åԥ󥰤���
static void get_updatearea() {
	MyRectangle clip = {0, 0, 0, 0};
	MyRectangle rsf0 = {0, 0, sf0->width, sf0->height};
	
	g_slist_foreach(updatearea, disjunction, &clip);
	
	g_slist_free(updatearea);
	updatearea = NULL;
	
	// surface0�Ȥ��ΰ���Ѥ�Ȥ�
	intersection(&rsf0, &clip, &sact.updaterect);
	
	WARNING("clipped area x=%d y=%d w=%d h=%d\n",
		sact.updaterect.x, sact.updaterect.y,
		sact.updaterect.width, sact.updaterect.height);
	
	return;
}

// updatelist ����Ͽ���Ƥ��뤹�٤ƤΥ��ץ饤�Ȥ򹹿�
static void do_update_each(gpointer data, gpointer userdata) {
	sprite_t *sp = (sprite_t *)data;
	
	// ��ɽ���ξ��Ϥʤˤ⤷�ʤ�
	if (!sp->show) return;
	
	if (sp == sact.draggedsp) return; // drag��Υ��ץ饤�ȤϺǸ��ɽ��
		
	// ���ץ饤�����update�롼����θƤӽФ�
	if (sp->update) {
		sp->update(sp);
	}
}

/*
  �������Τι���
  @param syncscreen: surface0 �����褷����Τ� Screen ��ȿ�Ǥ����뤫�ɤ���
 */
int sp_update_all(boolean syncscreen) {

	// ���ץ饤�Ȱ�ư��������ϰ�ư����
	if (sact.movelist) {
		// ��ư���ϻ��֤��碌��
		sact.movestarttime = get_high_counter(SYSTEMCOUNTER_MSEC);
		g_slist_foreach(sact.movelist, spev_move_setup, NULL);
		g_slist_free(sact.movelist);
		sact.movelist = NULL;
	}

	// �������Τ򹹿��ΰ��
	sact.updaterect.x = 0;
	sact.updaterect.y = 0;
	sact.updaterect.width  = sf0->width;
	sact.updaterect.height = sf0->height;
	
	// updatelist����Ͽ���Ƥ��륹�ץ饤�Ȥ������
	// updatelist�ϥ��ץ饤�Ȥ��ֹ����¤�Ǥ���
	g_slist_foreach(sact.updatelist, do_update_each, NULL);

	// ���Υ롼���󤬸ƤФ��Ȥ��ϥ��ץ饤�Ȥϥɥ�å���ǤϤʤ�
	
	// screen��Ʊ����ɬ�פʤȤ��ϲ������Τ�Window��ž��
	if (syncscreen) {
		ags_updateFull();
	}
	
	// ��ư��Τ��٤ƤΥ��ץ饤�Ȥ���ư��λ����ޤ��Ԥ�
	// �������ʤ���ư����Ʊ�����ʤ� 
	spev_wait4moving_sp();
	
	return OK;
}

/*
  ���̤ΰ����򹹿�
   updateme(_part)����Ͽ����������ɬ�פ�sprite���¤��ΰ��update
*/
int sp_update_clipped() {
	// �����ΰ�γ���
	get_updatearea();
	
	// ���ޤ��Ϲ⤵�� 0 �λ��Ϥʤˤ⤷�ʤ�
	if (sact.updaterect.width == 0 || sact.updaterect.height == 0) {
		return OK;
	}

	// �����ΰ�����äƤ��륹�ץ饤�Ȥκ�����
	g_slist_foreach(sact.updatelist, do_update_each, NULL);
	
	// drag��Υ��ץ饤�Ȥ�Ǹ������
	if (sact.draggedsp) {
		sact.draggedsp->update(sact.draggedsp);
	}
	
	// �����ΰ�� Window ��ž��
	ags_updateArea(sact.updaterect.x, sact.updaterect.y, sact.updaterect.width, sact.updaterect.height);
	
	return OK;
}

/*
  sprite���Τι�������Ͽ
  @param sp: �������륹�ץ饤��
*/
int sp_updateme(sprite_t *sp) {
	MyRectangle *r;
	
	if (sp == NULL) return NG;
	if (sp->cursize.width == 0 || sp->cursize.height == 0) return NG;
	
	r = g_new(MyRectangle, 1);
	r->x = sp->cur.x;
	r->y = sp->cur.y;
	r->width = sp->cursize.width;
	r->height = sp->cursize.height;
	
	updatearea = g_slist_append(updatearea, r);
	
	WARNING("x = %d, y = %d, spno = %d w=%d,h=%d\n",
		r->x, r->y, sp->no, r->width, r->height);
	
	return OK;
}

/*
  sprite�ΰ�����������Ͽ
  @param sp: �������륹�ץ饤��
  @param x: �����ΰ�غ�ɸ
  @param y: �����ΰ�ٺ�ɸ
  @param w: �����ΰ���
  @param h: �����ΰ�⤵
*/
int sp_updateme_part(sprite_t *sp, int x, int y, int w, int h) {
	MyRectangle *r;
	
	if (sp == NULL) return NG;
	if (w == 0 || h == 0) return NG;
	
	r = g_new(MyRectangle, 1);
	r->x = sp->cur.x + x;
	r->y = sp->cur.y + y;
	r->width = w;
	r->height = h;
	
	updatearea = g_slist_append(updatearea, r);
	
	WARNING("x = %d, y = %d, spno = %d w=%d,h=%d\n",
		r->x, r->y, sp->no, r->width, r->height);
	
	return OK;
}

