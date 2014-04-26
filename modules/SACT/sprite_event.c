/*
 * sprite_event.c: SACT��� mouse/key ���٥�ȤΥϥ�ɥ�
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
/* $Id: sprite_event.c,v 1.5 2003/11/09 15:06:13 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <glib.h>

#include "portab.h"
#include "system.h"
#include "counter.h"
#include "menu.h"
#include "imput.h"
#include "nact.h"
#include "key.h"
#include "sact.h"
#include "sprite.h"
#include "sactsound.h"
#include "sactlog.h"

/*
  SACT���SpriteKey�Ԥ����å�����Key�Ԥ��ʤɤ�¸����뤿�ᡢX|SDL ����
  Key/Mouse ���٥�Ȥ����ä��Ȥ��ˡ�nact->ags.eventcb()�ˤ�ä�¾�Υ⥸�塼��
  �˥��٥�Ȥ����ФǤ���褦�ˤ�����

  SACT�ǤϺǽ�� spev_callback �ǥ��٥�Ȥ�������褦�ˤ������ߤΥ����Ԥ���
  ����(sact.waittype)�ˤ�äƤ��줾��Υ��٥�ȥϥ�ɥ��ƤӽФ��Ƥ��롣
  
  ���ߤΤȤ���1) ��å����������Ԥ���2) ñ�㥭���Ԥ���3) ��˥塼�����Ԥ���
  4) ���ץ饤�ȥ��� 5) �Хå��������Ԥ��Σ��Ĥ����롣

  1)��å����������Ԥ�
    �����������������ޤ��Ԥġ�Z�������ץ饤�Ȥξõ�Ǥ��롣

  2)ñ�㥭���Ԥ�
    1)��Ʊ��

  3)��˥塼�����Ԥ�
    ��˥塼�����ץ���ν������ޥ�����ư�������ȡ�����(or�ܥ���)��Υ�줿�Ȥ���
    �����Ĥ��ν�����Ԥ���

  4)���ץ饤���Ԥ�
    SW/GETA/GETB/PUT/SWPUT���ץ饤�Ȥγƽ�����Ԥ���
    ���ץ饤�Ȥ��ֹ�ξ�������Τ����˾�˽Ťͤ�ɽ���������ˤʤäƤ���
    ���ץ饤�Ȥ˥��٥�Ȥ������ʤ��褦�ˤ���
    -> SpriteKey�Ԥ���ľ���˥��ץ饤�Ȥ��ֹ�ˤ�� depth map ���������
       �ޥ����ΰ��֤ˤ��륹�ץ饤�Ȥ��ֹ��Ǽ��Ф���
       Alpha�ޥ����Ĥ���Sprite�ξ�硢�����ȿ�Ǥ���ɽ�����Ƥ�����Τߤ�
       depthmap��ȿ�ǡ�

      drag��Υ��ץ饤�ȤϾ�˺Ǿ�̤ˤ���褦��ɽ�������ޥ�����ư���Υ��٥��
      �Ϻǽ�˽������롣

  5)�Хå���ɽ����Υ����Ԥ�
    ���ꥸ�ʥ��Ʊ���������ΤϤ�
*/


static void cb_focused_swsp(gpointer s, gpointer data);
static void cb_defocused_swsp(gpointer s, gpointer data);
static int  cb_focused(sprite_t *sp);
static int  cb_defocused(sprite_t *sp);
static void cb_waitkey_simple(agsevent_t *e);
static void cb_waitkey_sprite(agsevent_t *e);
static void cb_waitkey_selection(agsevent_t *e);

/*
 �ե����������������ץ饤�Ȥ��������ץ饤�Ȥ���Ͽ����Ƥ�������
 �������ץ饤�Ȥ�ɽ��ON
*/
static void cb_focused_swsp(gpointer s, gpointer data) {
	sprite_t *sp = (sprite_t *)s;
	int *update  = (int *)data;
	boolean oldstate = sp->show;
	
	WARNING("show up spex %d\n", sp->no);

	sp->show = TRUE;
	if (oldstate != sp->show) {
		(*update)++;
		sp_updateme(sp);
	}
}

/*
 �ե����������������ץ饤�Ȥ��������ץ饤�Ȥ���Ͽ����Ƥ�������
 �������ץ饤�Ȥ�ɽ��OFF
*/
static void cb_defocused_swsp(gpointer s, gpointer data) {
	sprite_t *sp = (sprite_t *)s;
	int *update  = (int *)data;
	boolean oldstate = sp->show;
	
	WARNING("hide spex %d\n", sp->no);
	
	sp->show = FALSE;
	if (oldstate != sp->show) {
		(*update)++;
		sp_updateme(sp);
	}
}

// zkey hide off
static void cb_focused_zkey(gpointer s, gpointer data) {
	sprite_t *sp = (sprite_t *)s;
	int *update  = (int *)data;
	boolean oldstate = sp->show;
	
	sp->show = sp->show_save;
	if (oldstate != sp->show) {
		(*update)++;
		sp_updateme(sp);
	}
}

// zkey hide on
static void cb_defocused_zkey(gpointer s, gpointer data) {
	sprite_t *sp = (sprite_t *)s;
	int *update  = (int *)data;
	boolean oldstate = sp->show;
	
	sp->show = FALSE;
	if (oldstate != sp->show) {
		(*update)++;
		sp_updateme(sp);
		sp->show_save = oldstate;
	}
}

/*
  �ե����������������ץ饤�Ȥν���
    cg2�������curcg��cg2������

    drag��Υ��ץ饤�Ȥ�������
      -> PUT/SWPUT���ץ饤�ȤΤ�ȿ��
    drag��Υ��ץ饤�Ȥ��ʤ����
      -> GETA/GETB/SWPUT���ץ饤�ȤΤ�ȿ��
*/
static int cb_focused(sprite_t *sp) {
	int update = 0;

	if (sact.draggedsp) {
		if (sp->type != SPRITE_PUT &&
		    sp->type != SPRITE_SWPUT) return 0;
	} else {
		if (sp->type == SPRITE_PUT) return 0;
	}
	
	if (!sp->focused) {
		if (sp->cg2) {
			if (sp->curcg != sp->cg2) {
				sp_updateme(sp);
			}
			sp->curcg = sp->cg2;
			update++;
		}
		sp->focused = TRUE;
		WARNING("get forcused %d, type %d\n", sp->no, sp->type);
		if (sp->numsound1) {
			ssnd_play(sp->numsound1);
		}
	}
	
	return update;
}

/*
  �ե��������򼺤ä����ץ饤�Ȥν���
    curcg �� cg1 �˥��å�
*/
static int cb_defocused(sprite_t *sp) {
	int update = 0;
	
	if (sp->focused) {
		if (sp->curcg != sp->cg1) {
			sp_updateme(sp);
		}
		sp->curcg = sp->cg1;
		update++;
		sp->focused = FALSE;
		WARNING("lost forcused %d\n", sp->no);
	}
	
	return update;
}

/*
  WaitKeySimple��callback
*/
static void cb_waitkey_simple(agsevent_t *e) {
	int cur, update = 0;
	
	switch (e->type) {
	case AGSEVENT_KEY_PRESS:
		if (e->d3 == KEY_Z) {
			cur = get_high_counter(SYSTEMCOUNTER_MSEC);
			if (!sact.zhiding) {
				g_slist_foreach(sact.sp_zhide, cb_defocused_zkey, &update);
				sact.zhiding = TRUE;
				sact.zdooff = TRUE;
				sact.zofftime = cur;
			} else {
				sact.zdooff = FALSE;
			}
		}
		break;
		
	case AGSEVENT_BUTTON_RELEASE:
		// back log view mode �˰ܹ�
		if (e->d3 == AGSEVENT_WHEEL_UP ||
		    e->d3 == AGSEVENT_WHEEL_DN) {
			// MessageKey �Ԥ��ΤȤ��Τ�
			if (sact.waittype != KEYWAIT_MESSAGE) break;
			sblog_start();
			sact.waittype = KEYWAIT_BACKLOG;
			break;
		}
		if (sact.zhiding) {
			g_slist_foreach(sact.sp_zhide, cb_focused_zkey, &update);
			sact.zhiding = FALSE;
		}
		// fall through
		
	case AGSEVENT_KEY_RELEASE:
		switch(e->d3) {
		case KEY_Z:
			cur = get_high_counter(SYSTEMCOUNTER_MSEC);
			if (500 < (cur - sact.zofftime) || !sact.zdooff) {
				g_slist_foreach(sact.sp_zhide, cb_focused_zkey, &update);
				sact.zhiding = FALSE;
			}
			break;
		case KEY_PAGEUP:
		case KEY_PAGEDOWN:
			// MessageKey �Ԥ��ΤȤ��Τ�
			if (sact.waittype != KEYWAIT_MESSAGE) break;
			sblog_start();
			sact.waittype = KEYWAIT_BACKLOG;
			break;
		default:
			sact.waitkey = e->d3;
			break;
		}
	}
	
	if (update) {
		sp_update_clipped();
	}
}

/*
  WaitKeySprite��callback
*/
static void cb_waitkey_sprite(agsevent_t *e) {
	GSList *node;
	sprite_t *focused_sp = NULL;   // focus �����Ƥ��� sprite
	sprite_t *defocused_sp = NULL; // focus �򼺤ä� sprite
	int update = 0;
	
	// �������٥�Ȥ�̵��
	switch(e->type) {
	case AGSEVENT_KEY_RELEASE:
	case AGSEVENT_KEY_PRESS:
		return;
	}
	
	if (sact.draggedsp) {
		// ��� drag���sprite�˥��٥�Ȥ�����
		update = sact.draggedsp->eventcb(sact.draggedsp, e);
	} else {
		// ������å�����󥻥�
		// drag��Ǥʤ����Τߡ�����󥻥������դ���
		if (e->type == AGSEVENT_BUTTON_RELEASE &&
		    e->d3   == AGSEVENT_BUTTON_RIGHT) {
			sact.waitkey = 0;
			return;
		}
	}
	
	// forcus�����Ƥ��� sprite �� focus�򼺤ä� sprite ��õ��
	for (node = sact.eventlisteners; node; node = node->next) {
		sprite_t *sp = (sprite_t *)node->data;
		
		if (sp == NULL) continue;
		if (!sp->show) continue;
		
		// freeze���֤Ǥ�CG���Ѳ����ʤ�
		if (sp->freezed_state != 0) continue;
		
		// dragg��� sprite ��̵�뤹��
		if (sp == sact.draggedsp) continue;
		
		if (focused_sp == NULL && sp_is_insprite(sp, e->d1, e->d2)) {
			/*
			  focus�����Ƥ��� sprite
			*/
			update += cb_focused(sp);
			focused_sp = sp;
		} else {
			/* 
			   ���ߤΥ���������֤ˤϤ��޽������Ƥ����ֹ��sprite
			   ��¸�ߤ��ʤ��Τǡ�defocus �ν�����Ԥ���
			   ����Υޥ�����ư���٥�Ȥˤ��focus�򼺤ä��Τ�
			   ����С�cb_decocused(sp)�� 1 ���֤�
			*/ 
			int ret = cb_defocused(sp);
			if (ret > 0) defocused_sp = sp;
			update += ret;
		}
	}
	
	// focus������ sprite �� BUTTON���٥�ȤΤߤ�����
	if (focused_sp && e->type != AGSEVENT_MOUSE_MOTION) {
		update += focused_sp->eventcb(focused_sp, e);
	}
	
	// �ϰϳ�(focus�����Ƥ���sprite���ʤ����)�򥯥�å������Ȥ��β�
	if (!focused_sp &&
	    e->type != AGSEVENT_MOUSE_MOTION &&
	    sact.numsoundob) {
		ssnd_play(sact.numsoundob);
	}
	
	// drag��Ǥʤ����ϡ��������ץ饤�Ȥ�ɽ�����õ��Ԥ�
	if (sact.draggedsp == NULL && e->type == AGSEVENT_MOUSE_MOTION) {
		// focus �򼺤ä� sprite �� ���� sprite �ξõ�
		if (defocused_sp) {
			sprite_t *sp = defocused_sp;
			if (sp->expsp) {
				g_slist_foreach(sp->expsp, cb_defocused_swsp, &update);
			}
		}
		
		// focus ������ sprite �� ���� sprite ��ɽ��
		if (focused_sp) {
			sprite_t *sp = focused_sp;
			if (sp->expsp) {
				g_slist_foreach(sp->expsp, cb_focused_swsp, &update);
			}
		}
	}
	
	// ɽ�����֤��ѹ�������Ф����ΰ�򹹿�
	if (update) {
		sp_update_clipped();
	}
}

/*
  ����� Window Open ���� callback
*/
static void cb_waitkey_selection(agsevent_t *e) {
	switch (e->type) {
	case AGSEVENT_BUTTON_RELEASE:
		sact.sel.cbrelease(e);
		break;
		
	case AGSEVENT_MOUSE_MOTION:
		sact.sel.cbmove(e);
		break;
	}
}

/*
  �Хå������Ȼ�
*/
static void cb_waitkey_backlog(agsevent_t *e) {
	switch (e->type) {
	case AGSEVENT_KEY_RELEASE:
		switch (e->d3) {
		case KEY_ESC:
			sblog_end();
			sact.waittype = KEYWAIT_MESSAGE;
			break;
		case KEY_PAGEUP:
			sblog_pageup();
			break;
		case KEY_PAGEDOWN:
			sblog_pagedown();
			break;
		case KEY_UP:
			sblog_pagenext();
			break;
		case KEY_DOWN:
			sblog_pagepre();
			break;
		}
		break;
		
	case AGSEVENT_BUTTON_RELEASE:
		switch(e->d3) {
		case AGSEVENT_WHEEL_UP:
			sblog_pagenext();
			break;
		case AGSEVENT_WHEEL_DN:
			sblog_pagepre();
			break;
		case AGSEVENT_BUTTON_RIGHT:
			sblog_end();
			sact.waittype = KEYWAIT_MESSAGE;
			break;
		}
		break;
	}
}

/*
  X|SDL �Υ��٥�ȥǥ����ѥå��㤫�餯��ǽ�ξ��
*/
void spev_callback(agsevent_t *e) {
	// menu open���̵��
	if (nact->popupmenu_opened) {
		return;
	}
	
	if (sact.waittype != KEYWAIT_BACKLOG) {
		if (e->type == AGSEVENT_KEY_PRESS && e->d3 == KEY_CTRL) {
			sact.waitskiplv = 2;
			sact.waitkey = e->d3;
			return;
		}
		
		if (e->type == AGSEVENT_KEY_RELEASE && e->d3 == KEY_CTRL) {
			sact.waitskiplv = 0;
			sact.waitkey = e->d3;
			return;
		}
	}
	
	switch (sact.waittype) {
	case KEYWAIT_MESSAGE:
	case KEYWAIT_SIMPLE:
		cb_waitkey_simple(e);
		break;

	case KEYWAIT_SPRITE:
		cb_waitkey_sprite(e);
		break;
	
	case KEYWAIT_SELECT:
		cb_waitkey_selection(e);
		break;

	case KEYWAIT_BACKLOG:
		cb_waitkey_backlog(e);
		break;
		
	default:
		return;
	}
}

/*
  �ƥ��ץ饤����Υ��٥�� callback ����Ͽ
*/
void spev_add_eventlistener(sprite_t *sp, int (*cb)(sprite_t *, agsevent_t *)) {
	sp->eventcb = cb;
	sact.eventlisteners = g_slist_append(sact.eventlisteners, sp);
}

/*
  �����Ͽ���� callback �κ��
*/
void spev_remove_eventlistener(sprite_t *sp) {
	sact.eventlisteners = g_slist_remove(sact.eventlisteners, sp);
}

