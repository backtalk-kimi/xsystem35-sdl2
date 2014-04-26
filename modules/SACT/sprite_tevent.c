/*
 * sprite_tevent.c: ���˥᡼����󥹥ץ饤�Ȥȥ��ץ饤�Ȥΰ�ư
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
/* $Id: sprite_tevent.c,v 1.2 2003/04/25 17:23:55 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <glib.h>

#include "portab.h"
#include "system.h"
#include "menu.h"
#include "imput.h"
#include "nact.h"
#include "sact.h"
#include "sprite.h"
#include "counter.h"

/*

 �����ޥ��٥�Ȥˤ�륹�ץ饤�Ȥΰ�ư�ȥ��˥᡼����󥹥ץ饤�Ȥι���

 �ºݤˤϡ�nact->callback()�����ϤǸƤӽФ��Ǥ��뤳�Ȥ��ۤȤ�ɤǡ�
 �����ޥ��٥�Ȥ��ߤ������ץ饤�������Υ�����Хå��Ƿв���֤�׻�����
 ���֤򹹿����Ƥ��롣

 ¾�ˤ� system35 �Υᥤ��롼�� nact_main() ����ƤФ�뤳�Ȥ䡢
 X|SDL�Υ����Ԥ������˸ƤФ�롣
 
 ���ץ饤�Ȥΰ�ư�� SACT.Draw ���ƤФ�Ƥ����ư�򳫻Ϥ�����ư��λ�ޤ�
 �ԤäƤ���Τǡ�nact_main����ƤФ��Ȥ��ϥ��˥᡼����󥹥ץ饤��
 �ι�������������
 
*/

static void tevent_callback(agsevent_t *e);


/*
 �����ޥ��٥�� callback �ᥤ��
*/
static void tevent_callback(agsevent_t *e) {
	GSList *node;
	int update = 0;
	
	// SP_MOVE ��Ʊ����ư�Τ���Υ����󥿤��ɤ߹���
	sact.movecurtime = get_high_counter(SYSTEMCOUNTER_MSEC);
	
	for (node = sact.teventlisteners; node; node = node->next) {
		sprite_t *sp = (sprite_t *)node->data;
		if (sp == NULL) continue;
		if (sp->teventcb == NULL) continue;
		
		// ��ɽ���Ǥϥ��٥�Ȥ�ȿ�����ʤ�
		if (!sp->show) continue;

		// ���ץ饤����Υ����ޥ��٥�ȥϥ�ɥ�θƤӽФ�
		update += sp->teventcb(sp, e);
	}
	
	// �ѹ�������в��̤򹹿�
	if (update) {
		sp_update_clipped();
	}
	
	// timer event litener �κ�� (��Υ롼����Ǻ���Ǥ��ʤ��Τ�)
	for (node = sact.teventremovelist; node; node = node->next) {
		sprite_t *sp = (sprite_t *)node->data;
		if (sp == NULL) continue;
		sact.teventlisteners = g_slist_remove(sact.teventlisteners, sp);
	}
	g_slist_free(sact.teventremovelist);
	sact.teventremovelist = NULL;
}

/*
  �����ޥ��٥�� callback ����Ͽ
  @param sp: ��Ͽ���륹�ץ饤��
  @param cb: �ƤӽФ����callback
*/
void spev_add_teventlistener(sprite_t *sp, int (*cb)(sprite_t *, agsevent_t *)) {
	sp->teventcb = cb;
	sact.teventlisteners = g_slist_append(sact.teventlisteners, sp);
}

/*
  �����Ͽ���� callback �κ��
  @param sp: ������륹�ץ饤��
*/
void spev_remove_teventlistener(sprite_t *sp) {
	sact.teventlisteners = g_slist_remove(sact.teventlisteners, sp);
}

/*
  system35�Υᥤ��롼�פ���ǸƤФ�륳����Хå�
*/
void spev_main() {
	agsevent_t e;
	
	e.type = AGSEVENT_TIMER;
	tevent_callback(&e);

	// �ǥե���ȤΥ�����Хå��Τ�����������ɬ�פʤ�Τ���
	// ������(VA���ޥ��callback�Ϥʤ�)
	if (nact->popupmenu_opened) {
		menu_gtkmainiteration();
		if (nact->is_quit) sys_exit(0);
        }
}
