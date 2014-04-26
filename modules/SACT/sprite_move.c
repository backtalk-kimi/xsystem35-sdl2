/*
 * sprite_move.c: ���ץ饤�Ȥΰ�ư�˴ؤ���Ƽ����
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
/* $Id: sprite_move.c,v 1.1 2003/04/22 16:29:52 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <math.h>
#include <glib.h>

#include "portab.h"
#include "system.h"
#include "imput.h"
#include "nact.h"
#include "sact.h"
#include "sprite.h"
#include "counter.h"

/*

  SP_MOVE �ˤ�륹�ץ饤�Ȥΰ�ư�ϡ�SACT.Draw��ȯ�Ԥ�����Ʊ����
  ��ư�򳫻Ϥ������Ƥΰ�ư����λ����ޤ��Ԥġ�

  ��ư�����Ϥ����ޤǤ˺�����ޤ��Ͽ����˺������줿���ץ饤�Ȥ�
  ��ư���ʤ��Τ����
  
*/


static void move_drain(sprite_t *sp);
static int move_cb(sprite_t *sp, agsevent_t *e);

// SP_MOVE���ޥ�ɤθ����
static void move_drain(sprite_t *sp) {
	// �Ť�����update
	sp_updateme(sp);

	// �ǽ���ư���˥��ץ饤�Ȱ��֤򥻥å�
	sp->cur = sp->loc = sp->move.to;

	// �����餷������update
	sp_updateme(sp);
	
	// ��ǡ�movelist ���鳰���Ƥ�餦����ν���
	sact.teventremovelist = g_slist_append(sact.teventremovelist, sp);
	
	sp->move.moving = FALSE;
	sp->move.time = 0; // ��ư���֤ν����
}

// SP_MOVE �� timer event callback
static int move_cb(sprite_t *sp, agsevent_t *e) {
	int t, update = 0;
	int now, newx, newy;

	// ���߻���μ���
	now = sact.movecurtime;
	
	WARNING("no = %d now = %d st = %d, ed = %d\n",
		sp->no, now, sp->move.starttime, sp->move.endtime);
	
	if (now >= sp->move.endtime) {
		// ���֥����С��ʤ顢�ǽ����֤˰�ư����MOVE��λ
		move_drain(sp);
		return 1;
	}
	
	// �в����
	t = now - sp->move.starttime;
	
	newx = sp->loc.x + t * (sp->move.to.x - sp->loc.x) / sp->move.time;
	newy = sp->loc.y + t * (sp->move.to.y - sp->loc.y) / sp->move.time;
	
	// ��ư���Ƥ����鿷�������֤�Ͽ���ƽ񤭴�����ؼ�
	if (newx != sp->cur.x || newy != sp->cur.y) {
		// �Ť�����update
		sp_updateme(sp);
		sp->cur.x = newx;
		sp->cur.y = newy;
		// ����������update
		sp_updateme(sp);
		update++;
	} else {
		usleep(1);
	}
	
	return update;
}

/*
 SP_MOVE���ޥ�ɡ���ư�ν���
 @param data: sprite
 @param userdata: ̤����
*/
void spev_move_setup(gpointer data, gpointer userdata) {
	sprite_t *sp = (sprite_t *)data;
	
	// ��ɽ���Τ�Τϰ�ư���ʤ�(�����Τ���)
	if (!sp->show) return;
	
	// move ���ϻ���ε�Ͽ
	sp->move.starttime = sact.movestarttime;
	sp->move.moving = TRUE;
	
	// MOVE_SPEED �����ꤷ�����ϡ���ư�̤��θ���ư�ư���֤����
	if (sp->move.time == -1) {
		// speed ���� time��
		int dx = sp->move.to.x - sp->loc.x;
		int dy = sp->move.to.y - sp->loc.y;
		int d = (int)sqrt(dx*dx+dy*dy);
		sp->move.time = d * 100 / sp->move.speed;
	}
	
	// move ��λͽ�����
	sp->move.endtime = sp->move.starttime + sp->move.time;
	
	// �����ޥ�����Хå���Ͽ
	spev_add_teventlistener(sp, move_cb);
	
	WARNING("no=%d,from(%d,%d@%d)to(%d,%d@%d),time=%d\n", sp->no,
		sp->cur.x, sp->cur.y, sp->move.starttime,
		sp->move.to.x, sp->move.to.y, sp->move.endtime,
		sp->move.time);

}

/*
  ���ץ饤�Ȥΰ�ư�����ꤷ����ư��ˤ���ޤ��Ԥ�
  @param sp: �оݥ��ץ饤��
  @param dx: ��ư��غ�ɸ
  @param dy: ��ư��ٺ�ɸ
  @param time: ��ư®��
*/
void spev_move_waitend(sprite_t *sp, int dx, int dy, int time) {
	sp->loc = sp->cur;
	sp->move.to.x = dx;
	sp->move.to.y = dy;
	sp->move.speed = time;
	sp->move.time = -1;
	
	sact.movelist = g_slist_append(sact.movelist, sp);
	sact.movestarttime = get_high_counter(SYSTEMCOUNTER_MSEC);
	g_slist_foreach(sact.movelist, spev_move_setup, NULL);
	g_slist_free(sact.movelist);
	sact.movelist = NULL;
	
	while (sp->move.moving) {
		nact->callback();
	}
}

/*
  ���Ƥΰ�ư��Υ��ץ饤�Ȥ���ư��λ����Τ��Ԥ�
*/
void spev_wait4moving_sp() {
	GSList *node;
	
	// ��ư��Υ��ץ饤�Ȥ� sact.updatelist �ˤ���Ϥ�������
	// ���Τʤ��Υ��ץ饤�ȤˤĤ��ơ���ư�椫�ɤ����Υե饰������å�
	for (node = sact.updatelist; node; node = node->next) {
		sprite_t *sp = (sprite_t *)node->data;
		if (sp == NULL) continue;
		if (!sp->show)  continue;
		
		while (sp->move.moving) {
			nact->callback();
		}
	}
}
