/*
 * sprite_sel.c: ��������
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
/* $Id: sprite_sel.c,v 1.1 2003/04/22 16:29:52 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <glib.h>

#include "portab.h"
#include "nact.h"
#include "ags.h"
#include "imput.h"
#include "key.h"
#include "sact.h"
#include "sprite.h"
#include "ngraph.h"
#include "drawtext.h"

// ���򤵤줿����(1~) ����󥻥�ξ���0��������֤� -1
static int selected_item;
static int selected_item_cur;

// ���������褹��surface
static surface_t *selcanvas;

// ���Υ�������ξ���
static boolean oldstate; // sprite���椫����
static int oldindex; // �����ܤ����Ǥ�(0~)


static boolean sp_is_insprite2(sprite_t *sp, int x, int y, int margin);
static void cb_select_move(agsevent_t *e);
static void cb_select_release(agsevent_t *e);
static int update_selwindow(sprite_t *sp);
static void setup_selwindow();
static void remove_selwindow();
static int sel_main();




/**
 *  ��¦�˥ޡ������ޤ� sprite���ΰ�����å�
 *  �ޡ��������sprite�����Ȥ�Ƚ�Ǥ��ʤ�
 */
static boolean sp_is_insprite2(sprite_t *sp, int x, int y, int margin) {
	MyRectangle r;
	cginfo_t *curcg = sp->curcg;
	
	r.x = sp->cur.x + margin;
	r.y = sp->cur.y + margin;
	r.width = curcg->sf->width   - 2 * margin;
	r.height = curcg->sf->height - 2 * margin;
	return ags_regionContains(&r, x, y);
}

// �ޥ�������ư�����Ȥ��� callback
static void cb_select_move(agsevent_t *e) {
	int x = e->d1, y = e->d2;
	sprite_t *sp = sact.sp[sact.sel.spno];
	boolean newstate;
	int newindex;
	
	// sprite�⤫��
	newstate = sp_is_insprite2(sp, x, y, sact.sel.frame_dot);
	newindex = (y - (sp->cur.y + sact.sel.frame_dot)) / (sact.sel.font_size + sact.sel.linespace);
	
	if (newstate == oldstate) {
		if ((newstate == FALSE) || (newindex == oldindex)) {
			// ���Ⱦ��֤�Ʊ���Ǥ��ġ����������֤�sprite�γ�����
			// index���Ѥ��ʤ����Ϥʤˤ⤷�ʤ���
			return;
		}
	}
	
	if (newstate) {
		// sprite������
		// fprintf(stderr, "in region %d\n", newindex);
		//update_selwindow(newindex + 1);
		selected_item_cur = newindex + 1;
	} else {
		// sprite�γ���
		//update_selwindow(0);
		selected_item_cur = 0;
	}
	
	oldstate = newstate;
	oldindex = newindex;
	
	// ������
	sp_updateme(sp);
	sp_update_clipped();
}

// �ܥ��󤬥�꡼�����줿�Ȥ��� callback
static void cb_select_release(agsevent_t *e) {
	int x = e->d1, y = e->d2;
	sprite_t *sp = sact.sp[sact.sel.spno];
	boolean st;
	int iy;
	
	switch (e->d3) {
	case AGSEVENT_BUTTON_LEFT:
		st = sp_is_insprite2(sp, x, y, sact.sel.frame_dot);
		iy = (y - (sp->cur.y + sact.sel.frame_dot)) / (sact.sel.font_size + sact.sel.linespace);

		// �������뤬 sprite �γ��ξ���̵��
		if (st == FALSE) {
			return;
		}
		
		// �������Ǥ����ξ���̵��
		if (sact.sel.elem[iy + 1] == NULL) return;
		
		selected_item = iy + 1;
		break;
		
	case AGSEVENT_BUTTON_RIGHT:
		// ����󥻥�
		selected_item = 0;
		break;
	}
}

// ���򥦥���ɤ򹹿�����Ȥ��� callback
static int update_selwindow(sprite_t *sp) {
	int selno = selected_item_cur;
	int x0, y0;

	x0 = sp->cur.x;
	y0 = sp->cur.y;
	// �ط� CG
	sp_draw(sp);
	
	// ���򤵤�Ƥ�������
	if (selno && sact.sel.elem[selno] != NULL) {
		int w = selcanvas->width - 2 * sact.sel.frame_dot;
		int h = sact.sel.font_size + sact.sel.linespace;
		int x = x0 + sact.sel.frame_dot;
		int y = y0 + sact.sel.frame_dot + (selno -1) * h;
		gr_fill(sf0, x, y, w, h, 0, 0, 0);
		gr_drawrect(sf0, x, y, w, h, 255, 255, 255);
	}
	
	// �����ʸ����
	gr_expandcolor_blend(sf0, x0, y0, 
			     sact.sel.charcanvas, 0, 0,
			     selcanvas->width, selcanvas->height, 255, 255, 255);
	
	return OK;
}

// ���򥦥���ɤν���
static void setup_selwindow() {
	sprite_t *sp = sact.sp[sact.sel.spno];
	int i;
	
	//���򥦥���ɺ�� surface������
	selcanvas = sf_dup(sp->cg1->sf);
	
	// �����ʸ���� canvas
	sact.sel.charcanvas = sf_create_pixel(selcanvas->width, selcanvas->height, 8);
	
	dt_setfont(sact.sel.font_type, sact.sel.font_size);
	
	// ����Υ��ץ饤�Ȥ�ʸ��(�������)
	for (i = 1; i < SEL_ELEMENT_MAX; i++) {
		int x, y;
		if (sact.sel.elem[i] == NULL) continue;
		// ʸ���ξ��׻�
		x = 0; // �Ԥ�����̵��
		y = (i - 1) * (sact.sel.font_size + sact.sel.linespace);
		dt_drawtext(sact.sel.charcanvas,
			    x + sact.sel.frame_dot, y + sact.sel.frame_dot,
			    sact.sel.elem[i]);
	}
	
	// �ǥե���Ȥ����򤵤������褬�����硢�����إ���������ư
	if (sact.sel.movecursor) {
		ags_setCursorLocation(sp->cur.x + sact.sel.frame_dot + 2,
				      sp->cur.y + sact.sel.frame_dot + 2 + (sact.sel.font_size + sact.sel.linespace)*(sact.sel.movecursor -1), TRUE);
		selected_item = (sact.sel.movecursor -1);
		oldstate = TRUE;
		oldindex = selected_item -1;
	}

	// ����¾�����
	selected_item_cur = 0;

	// ���ץ饤�Ⱥ����� callback ����Ͽ
	sp->update = update_selwindow;
}

// ���򥦥���ɤκ��
static void remove_selwindow() {
	sprite_t *sp = sact.sp[sact.sel.spno];

	// ���ץ饤�Ⱥ����� callback �򸵤ˤ�ɤ�
	sp->update = sp_draw;

	// ���ץ饤�Ȥ�����褷��(�����餯�ä�)
	sp_updateme(sp);
	sp_update_clipped();
	
	// ����� surface �κ��
	sf_free(selcanvas);
	sf_free(sact.sel.charcanvas);
}


// ����ᥤ��롼��
static int sel_main() {
	sact.waittype = KEYWAIT_SELECT;
	sact.waitkey = -1;

	selected_item = -1;
	
	while(selected_item == -1) {
		sys_keywait(25, TRUE);
	}
	
	sact.waittype = KEYWAIT_NONE;
	
	return selected_item;
}

/*
 ������Ϣ�ν����
*/
void ssel_init() {
	// callback������
	sact.sel.cbmove = cb_select_move;
	sact.sel.cbrelease = cb_select_release;

	// �ǥե���ȥե����
	sact.sel.font_type = FONT_GOTHIC;
}


/*
  ��������������򥯥ꥢ
*/
void ssel_clear() {
	int i;
	
	for (i = 0; i < SEL_ELEMENT_MAX; i++) {
		g_free(sact.sel.elem[i]);
		sact.sel.elem[i] = NULL;
	}
}

/*
  ��Ͽʸ�������������������ɲ�
  @param nString: ��Ͽʸ�����ѿ�
  @param wI     : ��Ͽ����
*/
void ssel_add(int nString, int wI) {
	if ((wI >= SEL_ELEMENT_MAX -1) || (wI <= 0)) {
		//error
		return;
	}
	if (sact.sel.elem[wI] != NULL) {
		g_free(sact.sel.elem[wI]);
	}
	
	sact.sel.elem[wI] = g_strdup(v_str(nString -1));
}

/*
  ���򥦥���ɤ򳫤�������

  @param wNum: ��,�طʤȤ��륹�ץ饤���ֹ�
  @param wChoiceSize: �����ʸ��������
  @param wMenuOutSpc: �ȥ��ץ饤�Ȥγ�¦����Υԥ������
  @param wChoiceLineSpace: �����ιԴ�
  @param wChoiceAutoMoveCursor: �����ץ���˼�ưŪ�˰�ư�����������ֹ�
  @param nAlign: �Ԥ��� (0:��, 1:���, 2: ��)
*/
int ssel_select(int wNum, int wChoiceSize, int wMenuOutSpc, int wChoiceLineSpace, int wChoiceAutoMoveCursor, int nAlign) {
	int ret = 0;
	boolean saveflag;
	
	// check sprite number is sane
	if (wNum >= (SPRITEMAX-1) || wNum <= 0) return ret;
	
	// check sprite is set
	if (sact.sp[wNum] == NULL) return ret;
	
	// must be normal sprite
	if (sact.sp[wNum]->type != SPRITE_NORMAL) return ret;
	
	sact.sel.spno = wNum;
	sact.sel.font_size = wChoiceSize;
	sact.sel.frame_dot = wMenuOutSpc;
	sact.sel.linespace = wChoiceLineSpace;
	sact.sel.movecursor = wChoiceAutoMoveCursor;
	sact.sel.align      = nAlign;

	// �Ť� sprite ��ɽ���ե饰����¸
	saveflag = sact.sp[wNum]->show;
	sact.sp[wNum]->show = TRUE;
	setup_selwindow();
	
	ret = sel_main();

	// ɽ���ե饰�򸵤��᤹
	sact.sp[wNum]->show = saveflag;
	
	remove_selwindow();
	
	return ret;
}
