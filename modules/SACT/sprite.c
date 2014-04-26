/*
 * sprite.c: ���ץ饤�ȴ��ܳƼ����
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
/* $Id: sprite.c,v 1.5 2003/11/16 15:29:52 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <glib.h>

#include "portab.h"
#include "system.h"
#include "ngraph.h"
#include "ags.h"
#include "nact.h"
#include "sact.h"
#include "sprite.h"
#include "surface.h"
#include "sactcg.h"
#include "sactsound.h"

static gint compare_spriteno_smallfirst(gconstpointer a, gconstpointer b);


#define sp_assert_no(no) G_STMT_START{                               \
  if ((no) >= SPRITEMAX) {                                           \
    WARNING("no is too large (should be %d < %d)\n", no, SPRITEMAX); \
    return NG;                                                       \
  }                                                                  \
}G_STMT_END

#define sp_assert_null(no) G_STMT_START{                             \
  if (sact.sp[no] == NULL) {                                         \
    WARNING("sprite %d is NULL\n", no);                              \
    return NG;                                                       \
  }                                                                  \
}G_STMT_END


// ���ץ饤�Ȥ��ֹ��˹������뤿��˥ꥹ�Ȥ˽��֤��פ�뤿���callbck
static gint compare_spriteno_smallfirst(gconstpointer a, gconstpointer b) {
	sprite_t *sp1 = (sprite_t *)a;
	sprite_t *sp2 = (sprite_t *)b;
	
	if (sp1->no < sp2->no) {
		return -1;
	}
	if (sp1->no > sp2->no) {
		return 1;
	}
	return 0;
}

// �ǥե���Ȥ��ɻ�update
static int sp_draw_wall(sprite_t *sp) {
	int sx, sy, w, h;
	
	sx = sact.updaterect.x;
	sy = sact.updaterect.y;
	w = sact.updaterect.width;
	h = sact.updaterect.height;
	gr_fill(sf0, sx, sy, w, h, 0, 0, 0);
	
	WARNING("do update no=%d, sx=%d, sy=%d, w=%d, h=%d, \n",
		sp->no, sx, sy, w, h);
	
	return OK;
}

/**
 * sprite ��Ϣ�ν����
 * @param  none
 * @return OK:����, NG:����
 */
int sp_init() {
	int i;
	
	// DLL�ѥ�å�����ɽ��
	nact->msgout = smsg_add;

	// mouse/key event handler
	nact->ags.eventcb = spev_callback;

	// main callback
	nact->callback = spev_main;
	
	// ���������ͳ�������ƤΥ��ץ饤�Ȥ򤢤餫����������Ƥ���
	for (i = 0; i < SPRITEMAX; i++) {
		sact.sp[i] = g_new0(sprite_t, 1);
		sact.sp[i]->no   = i;
		sact.sp[i]->type = SPRITE_NONE;
		sact.sp[i]->show = FALSE;
	}
	
	// �ɻ�(���ץ饤���ֹ棰)�ϥǥե���Ȥ�
	sp_set_wall_paper(0);
	
	// �ɻ�� update�ꥹ�Ȥ��ɲ�
	sact.updatelist = g_slist_append(sact.updatelist, sact.sp[0]);
	
	return OK;
}

/**
 * �������ץ饤�Ȥκ���
 * @param no: ���ץ饤���ֹ�
 * @param cg1: 1���ܤ�CG
 * @param cg2: 2���ܤ�CG (�ʤ�����0)
 * @param cg3: 3���ܤ�CG (�ʤ�����0)
 * @param type: ���ץ饤�Ȥμ���
 */
int sp_new(int no, int cg1, int cg2, int cg3, int type) {
	sprite_t *sp;
	
	sp_assert_no(no);

	sp = sact.sp[no];

	if (sp->type != SPRITE_NONE) {
		sp_free(no);
	}
	
	// �����ꥹ�Ȥ���Ͽ
	sact.updatelist = g_slist_insert_sorted(sact.updatelist, sp, compare_spriteno_smallfirst);
	
	sp->type = type;
	sp->no   = no;
	
	// set�����Ǥ�cg�����Ѥ����(draw���ǤϤʤ�)
	if (cg1) sp->cg1 = scg_loadcg_no(cg1, TRUE); else sp->cg1 = NULL;
	if (cg2) sp->cg2 = scg_loadcg_no(cg2, TRUE); else sp->cg2 = NULL;
	if (cg3) sp->cg3 = scg_loadcg_no(cg3, TRUE); else sp->cg3 = NULL;
	
	//�����curcg��cg1
	sp->curcg = sp->cg1;
	
	sp->show = TRUE; // ������֤�ɽ��
	sp->blendrate = 255; // �֥���̵��
	sp->loc.x = 0;   // ���ɽ�����֤�(0,0)
	sp->loc.y = 0;
	sp->cur = sp->loc;
	
	// cg1���礭���򥹥ץ饤�Ȥ��礭���Ȥ���
	if (sp->curcg == NULL) {
		sp->cursize.width = 0;
		sp->cursize.height = 0;
	} else {
		sp->cursize.width = sp->curcg->sf->width;
		sp->cursize.height = sp->curcg->sf->height;
	}
	
	sp->freezed_state = 0; // ���ָ����̵��
	sp->update = DEFAULT_UPDATE;  // default �� update�롼����
	
	// �ƥ��ץ饤�ȥ�������ν����
	switch(type) {
	case SPRITE_SWITCH:
		sp_sw_setup(sp);
		break;
		
	case SPRITE_GETA:
	case SPRITE_GETB:
		sp_get_setup(sp);
		break;
		
	case SPRITE_PUT:
	case SPRITE_SWPUT:
		sp_put_setup(sp);
		break;
		
	case SPRITE_ANIME:
		sp_anime_setup(sp);
		break;
	}
	
	return OK;
}

// ��å��������ץ饤�Ȥκ���
int sp_new_msg(int no, int x, int y, int width, int height) {
	sprite_t *sp;
	
	sp_assert_no(no);
	
	sp = sact.sp[no];
	
	if (sp->type != SPRITE_NONE) {
		sp_free(no);
	}
	// �����ꥹ�Ȥ���Ͽ
	sact.updatelist = g_slist_insert_sorted(sact.updatelist, sp, compare_spriteno_smallfirst);
	
	
	sp->type = SPRITE_MSG;
	sp->no   = no;
	sp->show = TRUE; // ������֤�ɽ��
	sp->blendrate = 255; // �֥���̵��
	sp->freezed_state = 0; // ���ָ���̵��
	sp->loc.x = x - sact.origin.x; // ���ɽ������
	sp->loc.y = y - sact.origin.y;
	sp->u.msg.dspcur.x = 0; // ʸ�����賫�ϰ���
	sp->u.msg.dspcur.y = 0;
	sp->cursize.width = width;  // ���ץ饤�Ȥ��礭��
	sp->cursize.height = height;
	sp->cur = sp->loc;
	sp->u.msg.buf = NULL;
	
	// ʸ�������ѥ����Х�
	sp->u.msg.canvas = sf_create_surface(width, height, sf0->depth);
	
	// ���ץ饤�Ⱥ������ѥ�����Хå�
	sp->update = smsg_update;
	
	return OK;
}

// �ɻ������
int sp_set_wall_paper(int no) {
	sprite_t *sp = sact.sp[0];
	
	if (sp->curcg) {
		scg_free_cgobj(sp->curcg);
	}
	
	if (no) { // �����CG��ɽ��
		sp->curcg = scg_loadcg_no(no, TRUE);
		sp->update = DEFAULT_UPDATE;
		sp->cursize.width  = sp->curcg->sf->width;
		sp->cursize.height = sp->curcg->sf->height;
	} else { // ����
		sp->cursize.width  = sf0->width;
		sp->cursize.height = sf0->height;
		sp->curcg = NULL;
		sp->update = sp_draw_wall;
	}
	
	sp->type = SPRITE_WP;
	sp->show = TRUE;
	sp->blendrate = 255;
	sp->cur.x = 0;
	sp->cur.y = 0;
	
	return OK;
}

// ���Ƥ� sprite ��õ�
int sp_free_all() {
	int i;
	
	for (i = 1; i < SPRITEMAX; i++) {
		sp_free(i);
	}
	return OK;
}

// �����sprite ��õ�
int sp_free(int no) {
	sprite_t *sp;
	
	sp_assert_no(no);
	
	sp = sact.sp[no];

	// ��ư���Ϥ��Ƥ��ʤ����ϥꥹ�Ȥ�����
	if (!sp->move.moving) {
		sact.movelist = g_slist_remove(sact.movelist, sp);
	}
	
	// CG���֥������Ȥκ��
	if (sp->cg1) scg_free_cgobj(sp->cg1);
	if (sp->cg2) scg_free_cgobj(sp->cg2);
	if (sp->cg3) scg_free_cgobj(sp->cg3);
	
	// remove���ν���������м¹�
	if (sp->remove) {
		sp->remove(sp);
	}
	
	// �������ץ饤�Ȥκ��
	//   �����Ǿä�����ޤ�������
	g_slist_free(sp->expsp);
	sp->expsp = NULL;
	
	if (sp->type == SPRITE_MSG) {
		g_slist_free(sp->u.msg.buf);
		sf_free(sp->u.msg.canvas);
	}
	sact.updatelist = g_slist_remove(sact.updatelist, sp);
	
	// SACT.Numeral_XXX �ϻĤ��Ƥ���
	{
		sprite_t back;
		memcpy(&(back.numeral), &(sp->numeral), sizeof(sp->numeral));
		memset(sp, 0, sizeof(sprite_t));
		sp->type = SPRITE_NONE;
		sp->no = no;
		sp->show = FALSE;
		memcpy(&(sp->numeral), &(back.numeral), sizeof(sp->numeral));
	}
	return OK;
}

// ɽ�����֤��ѹ�
int sp_set_show(int wNum, int wCount, int sShow) {
	int i;
	boolean oldstate;
	sprite_t *sp;
	
	sp_assert_no(wNum);
	
	for (i = wNum; i < (wNum + wCount); i++) {
		if (i >= (SPRITEMAX -1)) break;
		sp = sact.sp[i];
		oldstate = sp->show;
		
		sp->show = (sShow == 1 ? TRUE : FALSE);
	}
	return OK;
}

// ɽ�����֤�����
int sp_set_pos(int wNum, int wX, int wY) {
	sprite_t *sp;
	
	sp_assert_no(wNum);
	
	sp = sact.sp[wNum];
	sp->loc.x = wX - sact.origin.x;
	sp->loc.y = wY - sact.origin.y;
	sp->cur.x = sp->loc.x;
	sp->cur.y = sp->loc.y;
	return OK;
	
}

// ���ץ饤�Ȥΰ�ư
int sp_set_move(int wNum, int wX, int wY) {
	sprite_t *sp;
	
	sp_assert_no(wNum);
	
	sp = sact.sp[wNum];
	sp->move.to.x = wX - sact.origin.x;
	sp->move.to.y = wY - sact.origin.y;
	
	if (sp->move.time == 0) {
		sp->move.time = -1;
		sp->move.speed = 100;
	}
	
	sp->cur = sp->loc;
	
	// move���륹�ץ饤�ȥꥹ�Ȥ���Ͽ
	// �ºݤ� move �򳫻Ϥ���Τ� ~SP_DRAW(sp_update_all)���ƤФ줿�Ȥ�
	sact.movelist = g_slist_append(sact.movelist, sp);
	
	return OK;
}

// ���ץ饤�Ȱ�ư���֤�����
int sp_set_movetime(int wNum, int wTime) {
	sp_assert_no(wNum);
	
	sact.sp[wNum]->move.time = wTime * 10;
	return OK;
}

// ���ץ饤�Ȱ�ư®�٤�����
int sp_set_movespeed(int wNum, int wTime) {
	sp_assert_no(wNum);
	
	if (wTime == 0) wTime = 1;
	
	sact.sp[wNum]->move.speed = wTime ;
	sact.sp[wNum]->move.time = -1;
	
	return OK;
}

// Z�����򲡤����Ȥ��˱������ץ饤�Ȥ���Ͽ
int sp_add_zkey_hidesprite(int wNum) {
	sprite_t *sp;
	
	sp_assert_no(wNum);
	sp = sact.sp[wNum];

	// ��Ͽ�����Ǥޤ��������Ƥ��ʤ����ץ饤�Ȥϱ����ʤ�
	//   �����륯�쥤��Ǥޤ����Τ����ä��Τ����
	// if (sp->type == SPRITE_NONE) return NG;
	
	sact.sp_zhide = g_slist_append(sact.sp_zhide, sp);
	return OK;
}

// �����Ͽ�������ץ饤�Ȥκ��
int sp_clear_zkey_hidesprite_all() {
	g_slist_free(sact.sp_zhide);
	sact.sp_zhide = NULL;
	return OK;
}

// ���ץ饤�Ⱦ��֤θǲ�
int sp_freeze_sprite(int wNum, int wIndex) {
	sprite_t *sp;
	void *oldstate;
	
	sp_assert_no(wNum);
	
	sp = sact.sp[wNum];
	sp->freezed_state = wIndex;

	oldstate = (void *)sp->curcg;
	switch(wIndex) {
	case 1:
		sp->curcg = sp->cg1; break;
	case 2:
		sp->curcg = sp->cg2; break;
	case 3:
		sp->curcg = sp->cg3; break;
	}
	return OK;
}

// ��Ǹǲ��������֤β��
int sp_thaw_sprite(int wNum) {
	sp_assert_no(wNum);
	
	sact.sp[wNum]->freezed_state = 0;
	return OK;
}

// SP_QUAKE���ɤ餹���ץ饤�Ȥ���Ͽ
int sp_add_quakesprite(int wNum) {
	sp_assert_no(wNum);
	
	sact.sp_quake = g_slist_append(sact.sp_quake, sact.sp[wNum]);
	return OK;
}

// �����Ͽ�������ץ饤�Ȥκ��
int sp_clear_quakesprite_all() {
	g_slist_free(sact.sp_quake);
	sact.sp_quake = NULL;
	return OK;
}

// ���˥᡼����󥹥ץ饤�Ȥδֳ֤�����
int sp_set_animeinterval(int wNum, int wTime) {
	sp_assert_no(wNum);

	if (sact.sp[wNum]->type != SPRITE_ANIME) return NG;
	
	sact.sp[wNum]->u.anime.interval = wTime * 10;
	
	return OK;
}

// ���ץ饤�ȤΥ֥���Ψ������
int sp_set_blendrate(int wNum, int wCount, int rate) {
	int i;
	sprite_t *sp;
	
	sp_assert_no(wNum);
	
	for (i = wNum; i < (wNum + wCount); i++) {
		if (i >= (SPRITEMAX -1)) break;
		sp = sact.sp[i];
		sp->blendrate = rate;
	}
	
	return OK;
}

// ���ץ饤�Ȥ� create ����Ƥ��뤫�ɤ����μ���
int sp_query_isexist(int wNum, int *ret) {
	if (wNum >= SPRITEMAX) goto errexit;
	if (sact.sp[wNum]->type == SPRITE_NONE) goto errexit;
	
	*ret = 1;
	return OK;
	
 errexit:
	*ret = 0;
	return NG;
}

// ���ץ饤�ȤΥ����פȲ��֤�CG�����åȤ���Ƥ��뤫�μ���
int sp_query_info(int wNum, int *vtype, int *vcg1, int *vcg2, int *vcg3) {
	sprite_t *sp;
	
	if (wNum >= SPRITEMAX) goto errexit;

	sp = sact.sp[wNum];
	if (sp->type == SPRITE_NONE) goto errexit;
	
	*vtype = sp->type;
	*vcg1 = sp->cg1 ? sp->cg1->no : 0;
	*vcg2 = sp->cg2 ? sp->cg2->no : 0;
	*vcg3 = sp->cg3 ? sp->cg3->no : 0;
	
	return OK;
	
 errexit:
	*vtype = 0;
	*vcg1 = 0;
	*vcg2 = 0;
	*vcg3 = 0;
	return NG;
}

// ���ץ饤�Ȥ�ɽ�����֤μ���
int sp_query_show(int wNum, int *vShow) {
	if (wNum >= SPRITEMAX) goto errexit;
	if (sact.sp[wNum]->type == SPRITE_NONE) goto errexit;

	*vShow = sact.sp[wNum]->show ? 1: 0;
	return OK;
	
 errexit:
	*vShow = 0;
	return NG;
}

// ���ץ饤�Ȥ�ɽ�����֤μ���
int sp_query_pos(int wNum, int *vx, int *vy) {
	if (wNum >= SPRITEMAX) goto errexit;
	if (sact.sp[wNum]->type == SPRITE_NONE) goto errexit;

	*vx = sact.sp[wNum]->loc.x;
	*vy = sact.sp[wNum]->loc.y;
	return OK;

 errexit:
	*vx = 0;
	*vy = 0;
	return NG;
}

// ���ץ饤�Ȥ��礭���μ���
int sp_query_size(int wNum, int *vw, int *vh) {
	sprite_t *sp;
	
	if (wNum >= SPRITEMAX) goto errexit;

	sp = sact.sp[wNum];

	if (sp->type == SPRITE_NONE) goto errexit;

	*vw = sp->cursize.width;
	*vh = sp->cursize.height;
	
	return OK;
	
 errexit:
	*vw = 0;
	*vh = 0;
	return NG;
}

// �ƥ����ȥ��ץ饤�Ȥθ��ߤ�ʸ��ɽ�����֤μ���
int sp_query_textpos(int wNum, int *vx, int *vy) {
	if (wNum >= SPRITEMAX) goto errexit;
	if (sact.sp[wNum]->type != SPRITE_MSG) goto errexit;
	
	*vx = sact.sp[wNum]->u.msg.dspcur.x;
	*vy = sact.sp[wNum]->u.msg.dspcur.y;
	return OK;
	
 errexit:
	*vx = 0;
	*vy = 0;
	return NG;
}

// NumeralXXX��CG�Υ��å�
int sp_num_setcg(int nNum, int nIndex, int nCG) {
	sp_assert_no(nNum);

	sact.sp[nNum]->numeral.cg[nIndex] = nCG;

	return OK;
}

// NumeralXXX��CG�μ���
int sp_num_getcg(int nNum, int nIndex, int *vCG) {
	sp_assert_no(nNum);
	
	*vCG = sact.sp[nNum]->numeral.cg[nIndex];
	
	return OK;
}

// NumeralXXX�ΰ��֤Υ��å�
int sp_num_setpos(int nNum, int nX, int nY) {
	sp_assert_no(nNum);

	sact.sp[nNum]->numeral.pos.x = nX;
	sact.sp[nNum]->numeral.pos.y = nY;

	return OK;
}

// NumeralXXX�ΰ��֤μ���
int sp_num_getpos(int nNum, int *vX, int *vY) {
	sp_assert_no(nNum);
	
	*vX = sact.sp[nNum]->numeral.pos.x;
	*vY = sact.sp[nNum]->numeral.pos.y;
	
	return OK;
}

// NumeralXXX�Υ��ѥ�Υ��å�
int sp_num_setspan(int nNum, int nSpan) {
	sp_assert_no(nNum);
	
	sact.sp[nNum]->numeral.span = nSpan;
	
	return OK;
}

// NumeralXXX�Υ��ѥ�μ���
int sp_num_getspan(int nNum, int *vSpan) {
	sp_assert_no(nNum);

	*vSpan = sact.sp[nNum]->numeral.span;

	return OK;
}

// ���٤Ƥ��������ץ饤�Ȥκ��
int sp_exp_clear() {
	GSList *node;
	
	for (node = sact.updatelist; node; node = node->next) {
		sprite_t *sp = (sprite_t *)node->data;
		if (sp == NULL) continue;
		sp_exp_del(sp->no);
	}
	
	return OK;
}

// �������ץ饤�Ȥ���Ͽ
int sp_exp_add(int nNumSP1, int nNumSP2) {
	sprite_t *swsp, *expsp;
	sp_assert_no(nNumSP1);
	sp_assert_no(nNumSP2);

	swsp  = sact.sp[nNumSP1];
	expsp = sact.sp[nNumSP2];
	
	swsp->expsp = g_slist_append(swsp->expsp, expsp);
	
	return OK;
}

// �������ץ饤�Ȥκ��
int sp_exp_del(int nNum) {
	sprite_t *sp;
	
	sp_assert_no(nNum);
	
	sp  = sact.sp[nNum];
	
	g_slist_free(sp->expsp);
	sp->expsp = NULL;

	return OK;
}

// ���ץ饤�ȥ�����ɤΥ��å�
int sp_sound_set(int wNumSP, int wNumWave1, int wNumWave2, int wNumWave3) {
	sprite_t *sp;
	
	sp_assert_no(wNumSP);

	sp  = sact.sp[wNumSP];
	sp->numsound1 = wNumWave1;
	sp->numsound2 = wNumWave2;
	sp->numsound3 = wNumWave3;
	
	return OK;
}

// ���٤ƤΥ��ץ饤�ȥ�����ɤν�λ���Ԥ�
int sp_sound_wait() {
	WARNING("NOT IMPLEMENTED\n");
	return OK;
}

// �ϰϳ��򥯥�å������Ȥ��Υ�����ɤ�����
int sp_sound_ob(int wNumWave) {
	sact.numsoundob = wNumWave;
	return OK;
}

/**
 * ����κ�ɸ�����ߤΥ��ץ饤�Ȥΰ��֤��ϰϤ����äƤ��뤫��
 * @param sp: Ĵ�٤��оݤΥ��ץ饤��
 * @param x,y: ��ɸ
 * @return: TRUE:���äƤ���, FALSE: ���äƤ��ʤ�
 */
boolean sp_is_insprite(sprite_t *sp, int x, int y) {
	BYTE *dp;
	
	if (x < 0 || y < 0 || x >= sf0->width || y >= sf0->height) return FALSE;
	
	dp = GETOFFSET_PIXEL(sact.dmap, x, y);
	return (*(WORD *)dp == sp->no);
}
