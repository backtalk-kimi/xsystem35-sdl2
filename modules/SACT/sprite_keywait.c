/*
 * sprite_keywait.c: ���ץ饤�ȥ����Ԥ�
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
/* $Id: sprite_keywait.c,v 1.2 2003/05/09 05:14:34 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <glib.h>

#include "portab.h"
#include "system.h"
#include "counter.h"
#include "ags.h"
#include "nact.h"
#include "imput.h"
#include "ngraph.h"
#include "surface.h"
#include "sact.h"
#include "sprite.h"
#include "sactsound.h"


static void hidesprite(sprite_t *sp);
static boolean waitcond(int endtime);



// ���露��ä�
static void hidesprite(sprite_t *sp) {
	int i;
	
	for (i = 255; i >= 0; i-=32) {
		sp->blendrate = i;
		sp_updateme(sp);
		sp_update_clipped();
		sys_keywait(10, FALSE);
	}
	
	sp_updateme(sp);
	sp->show = FALSE;
	sp_update_clipped();
}

/*
  �����Ԥ���λ�ξ������å�
   @param endtime: ��λ����
   @return: ��λ�ʤ� TRUE����³�ʤ� FALSE
   
   ���ץ饤�Ȥ��ɥ�åפ��줿���γƼ������ޤ�
*/
static boolean waitcond(int endtime) {
	int curtime = get_high_counter(SYSTEMCOUNTER_MSEC);
	if (curtime >= endtime) return TRUE;
	
	if (sact.dropped) {
		sact.draggedsp->u.get.dragging = FALSE;
		if (sact.waitkey != -1) {
			// drop�������ץ饤�Ȥ򤸤露��ä� 
			hidesprite(sact.draggedsp);
			sact.sp_result_get = sact.draggedsp->no;
			sact.sp_result_put = sact.waitkey;
			sp_free(sact.draggedsp->no);
			sact.draggedsp = NULL;
			sact.dropped = FALSE;
			return TRUE;
		} else {
			// PUT/SWPUT���ץ饤�Ȱʳ��ΤȤ����drop�������
			sprite_t *sp = sact.draggedsp;
			if (sact.draggedsp->type == SPRITE_GETA) {
				// ��Ȥξ��ˤ�ä����᤹
				spev_move_waitend(sp, sp->loc.x, sp->loc.y, 150); 
			} else {
				// ���ξ���α�ޤ�
				sp->loc = sp->cur;
			}
			
			// drop��������С��Ĥ餹
			if (sact.draggedsp->numsound3) {
				ssnd_play(sact.draggedsp->numsound3);
			}
			sact.draggedsp = NULL;
			sact.dropped = FALSE;
		}
	}
	
	// ����¾�������Ϥ�����н�λ
	return sact.waitkey == -1 ? FALSE : TRUE;
}

/*
  ���ץ饤�ȥ����Ԥ�
  @param vOK: 0�ʤ�б�����å� 
  @param vRND: �����å����ץ饤�Ȥ��ֹ�
  @param vD01: ���åȥ��ץ饤�Ȥ��ֹ�
  @param vD02: �ץåȥ��ץ饤�Ȥ��ֹ�
  @param vD03: �����ॢ���Ȥ������=1, ���ʤ����=0
  @param wTime: �����ॢ���Ȼ��� (1/100sec)
*/
int sp_keywait(int *vOK, int *vRND, int *vD01, int *vD02, int *vD03, int timeout) {
	int curtime, endtime;
	
	// �Ȥꤢ����������
	sp_update_all(TRUE);
	
	// depthmap �����
	g_slist_foreach(sact.updatelist, sp_draw_dmap, NULL);
	
	sact.waittype = KEYWAIT_SPRITE;
	sact.waitkey = -1;
	sact.sp_result_sw  = 0;
	sact.sp_result_get = 0;
	sact.sp_result_put = 0;
	sact.draggedsp = NULL;
	
	{
		// �Ȥꤢ���������ߤΥޥ������֤����äơ�switch sprite ��
		// ���֤򹹿����Ƥ���
		agsevent_t agse;
		MyPoint p;
		sys_getMouseInfo(&p, FALSE);
		agse.type = AGSEVENT_MOUSE_MOTION;
		agse.d1 = p.x;
		agse.d2 = p.y;
		agse.d3 = 0;
		nact->ags.eventcb(&agse);
	}
	
	// ��λ���֤η׻�
	curtime = get_high_counter(SYSTEMCOUNTER_MSEC);
	endtime = timeout < 0 ? G_MAXINT: (curtime + timeout * 10);
	
	// ���ץ饤�ȥ����Ԥ��ᥤ��
	while (!waitcond(endtime)) {
		sys_keywait(25, TRUE);
	}
	
	if (sact.waitkey == 0) {
		// ������å�����󥻥�
		*vOK = 0;
		if (vD03) *vD03 = 0;
	} else if (sact.waitkey == -1) {
		// timeout
		*vOK = 1;
		if (vD03) *vD03 = 1;
	} else {
		*vOK = 1;
		if (vD03) *vD03 = 0;
	}
	
	*vRND = sact.sp_result_sw;
	*vD01 = sact.sp_result_get;
	*vD02 = sact.sp_result_put;
	
	sact.waittype = KEYWAIT_NONE;
	
	return OK;
}


