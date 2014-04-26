/*
 * sprite_msg.c: ��å��������ץ饤�Ȥν���
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
/* $Id: sprite_msg.c,v 1.8 2004/10/31 04:18:02 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "portab.h"
#include "system.h"
#include "counter.h"
#include "ags.h"
#include "nact.h"
#include "imput.h"
#include "sact.h"
#include "sprite.h"
#include "ngraph.h"
#include "drawtext.h"
#include "sactlog.h"
#include "eucsjis.h"

// ��å����������Ԥ��λ���ɽ�����륢�˥᡼�����˴ؤ������
struct markinfo {
	sprite_t *sp;
	cginfo_t *cg;
	int interval;
};

static boolean is_messagesprite(int wNum);
static void replacestr_cb(gpointer data, gpointer userdata);
static char *replacestr(char *msg);
static void update_mark(sprite_t *sp, cginfo_t *cg);
static int  setupmark(int wNum1, int wNum2, struct markinfo *minfo);
static int  get_linelen(BYTE *msg);
static BYTE *get_char(BYTE *msg, char *mbuf, char *rbuf, int bufmax);
static void append_to_log(char *msg);
static void sactlog_newline();
static void set_align(char *msg, sprite_t *sp, int wSize, int wAlign);


// ʸ�����֤������� (ɽ������on-the-fly���Ѵ�����ɽ��)
#define REPLACEBUFSIZE MSGBUFMAX+100
static char repbuf[2][REPLACEBUFSIZE];
static char *replacesrc;
static char *replacedst;

// ������ֹ�Υ��ץ饤�Ȥ���å��������ץ饤�Ȥ��ɤ���������å�
static boolean is_messagesprite(int wNum) {
	// check sprite number is sane
	if (wNum >= (SPRITEMAX -1) || wNum <= 0) return FALSE;
	
	// check sprite is set
	if (sact.sp[wNum] == NULL) return FALSE;
	
	// check sprite is message sprite
	if (sact.sp[wNum]->type != SPRITE_MSG) return FALSE;
	
	return TRUE;
}

// ʸ������֤���������
static void replacestr_cb(gpointer data, gpointer userdata) {
	strexchange_t *ex = (strexchange_t *)data;
	char *start, *next, *out;
	
	if (ex == NULL) return;
	
	start = replacesrc;
	out   = replacedst;
	
	while (TRUE) {
		next = strstr(start, ex->src);
		if (next == NULL) break;
		strncat(out, start, (size_t)(next - start));
		strncat(out, ex->dst, max(0, (REPLACEBUFSIZE - (int)strlen(out))));
		start = next + strlen(ex->src);
	}
	strncat(out, start, max(0, REPLACEBUFSIZE - (int)strlen(out)));
	
	replacedst = replacesrc;
	replacesrc = out;
	replacedst[0] = '\0';
}

// ʸ������֤�����
static char *replacestr(char *msg) {
	if (sact.strreplace == NULL) return msg;
	
	repbuf[0][0] = '\0';
	repbuf[1][0] = '\0';
	strncpy(repbuf[0], msg, REPLACEBUFSIZE);
	replacesrc = repbuf[0];
	replacedst = repbuf[1];
	g_slist_foreach(sact.strreplace, replacestr_cb, NULL);

	return (repbuf[0][0] == '\0') ? repbuf[1] : repbuf[0];
}

// ���˥�ѥ����������
static void update_mark(sprite_t *sp, cginfo_t *cg) {
	boolean show = sp->show;
	cginfo_t *curcg = sp->curcg;

	sp->show = TRUE;
	sp->curcg = cg;
	
	sp_updateme(sp);
	sp_update_clipped();

	sp->show = show;
	sp->curcg = curcg;
}

// ���˥�ѥ�����ν����
static int setupmark(int wNum1, int wNum2, struct markinfo *minfo) {
	sprite_t *sp1, *sp2;
	int i = 0;
	
	if (wNum1 == 0 || wNum2 == 0) return 0;
	
	sp1 = sact.sp[wNum1];
	sp2 = sact.sp[wNum2];
	if (sp1 == NULL || sp2 == NULL) return 0;
	
	if (sp1->cg1) {
		minfo[i].sp = sp1;
		minfo[i].cg = sp1->cg1;
		minfo[i].interval = sp1->u.anime.interval;
		i++;
	}
	if (sp1->cg2) {
		minfo[i].sp = sp1;
		minfo[i].cg = sp1->cg2;
		minfo[i].interval = sp1->u.anime.interval;
		i++;
	}
	if (sp1->cg3) {
		minfo[i].sp = sp1;
		minfo[i].cg = sp1->cg3;
		minfo[i].interval = sp1->u.anime.interval;
		i++;
	}
	if (sp2->cg1) {
		minfo[i].sp = sp2;
		minfo[i].cg = sp2->cg1;
		minfo[i].interval = sp2->u.anime.interval;
		i++;
	}
	if (sp2->cg2) {
		minfo[i].sp = sp2;
		minfo[i].cg = sp2->cg2;
		minfo[i].interval = sp2->u.anime.interval;
		i++;
	}
	if (sp2->cg3) {
		minfo[i].sp = sp2;
		minfo[i].cg = sp2->cg3;
		minfo[i].interval = sp2->u.anime.interval;
		i++;
	}
	return i;
}

/*
  ��å�������Хåե����ɲ�
    nact.c�Υ�å����������ޥ�ɲ��ϥ롼���󤫤�ƤФ��
    
  @param msg: �ɲä���ʸ����
*/
void smsg_add(char *msg) {
	int len;
	
	if (msg[0] == '\0') return;
	
	if (0) {
		char *b = sjis2lang(msg);
		fprintf(stderr, "add msg '%s'\n", b);
		free(b);
	}
	
	len = MSGBUFMAX - (int)strlen(sact.msgbuf);
	if (len < 0) {
		WARNING("buf shortage (%d)\n", len);
		return;
	}
	
	strncat(sact.msgbuf, msg, len);
	sact.msgbuf[MSGBUFMAX -1] = '\0';
}

/*
  ����
    ���Ԥϥ�å�����������ˤĤä��ߡ����ϻ��˲���������Ф��Ʋ���

  @param wNum: ���Ԥ��륹�ץ饤���ֹ�
  @param size: ������
*/
void smsg_newline(int wNum, int size) {
	BYTE buf[3];
	
	if (!is_messagesprite(wNum)) return;

	buf[0] = '\n';
	buf[1] = size;
	buf[2] = '\0';
	smsg_add(buf);
}

/*
  (��ӤĤ�)��å������ν���

  @param wSpriteNumber: ��å�������ɽ�������å��������ץ饤���ֹ�
  @param wSize: �ե���Ȥ��礭��
  @param wColorR: ��å������ο�(Red)
  @param wColorG: ��å������ο�(Green)
  @param wColorB: ��å������ο�(Blue)
  @param wFont: ��å������Υե����(0:�����å�, 1:��ī)
  @param wSpeed: ��å�������ɽ��®�� (msec)
  @param wLineSpace: �Դ֥��ڡ���
  @param wAlign: �Ԥ���
  @param wRSize: ��ӥե���ȥ�����
  @param wRFont: ��ӥե����
  @param wRLineSpace: ��Ӥ���ʸ��ʸ���ֳ�
  @param vLength: ???
*/
void smsg_out(int wNum, int wSize, int wColorR, int wColorG, int wColorB, int wFont, int wSpeed, int wLineSpace, int wAlign, int wRSize, int wRFont, int wRLineSpace, int *wLength) {
	char *msg;
	sprite_t *sp;
	int len = 0; // ��������ʸ����?
	boolean needupdate = FALSE;
	MyRectangle uparea = {0,0,0,0};
	
	// wRSize == 0 -> ���̵��(SACT.MessageOutput����θƽ�)

	if (sact.msgbuf[0] == '\0') return;
	
	if (!is_messagesprite(wNum)) return;
	
	// MessageSkip���ʸ������®�٤�����
	if (sact.waitskiplv > 1) wSpeed = 0;
	
	// shortcut
	sp = sact.sp[wNum];
	
	// update����Y��ɸ (X��ɸ��0����)
	uparea.y = sp->u.msg.dspcur.y;

	// ʸ�����ִ�
	msg = replacestr(sact.msgbuf);
	
	// ʸ�����饤����Ȥ�Ĵ��
	set_align(msg, sp, wSize, wAlign);
	
	while (*msg) {
		char mbuf[20], rbuf[20];
		int cw, delta, wcnt;
		
		wcnt = get_high_counter(SYSTEMCOUNTER_MSEC);
		
		mbuf[0] = rbuf[0] = '\0';
		msg = get_char(msg, mbuf, rbuf, sizeof(mbuf) -1); 
		
		if (mbuf[0] == '\n') {
			sp->u.msg.dspcur.x = 0;
			sp->u.msg.dspcur.y += (mbuf[1] + wLineSpace + wRSize + wRLineSpace);
			set_align(msg, sp, wSize, wAlign);
			
			sactlog_newline();
			
			continue;
		}
		
		if (rbuf[0] != '\0') {
			int mlen = strlen(mbuf) * wSize  /2;
			int rlen = strlen(rbuf) * wRSize /2;
			int adjx = max(0, (mlen - rlen) /2);
			dt_setfont(wRFont, wRSize);
			dt_drawtext_col(sp->u.msg.canvas,
					sp->u.msg.dspcur.x + adjx,
					sp->u.msg.dspcur.y,
					rbuf,
					wColorR, wColorG, wColorB);
		}
		dt_setfont(wFont, wSize);

		if (0) {
			char *b = sjis2lang(mbuf);
			fprintf(stderr, "msg '%s'\n", b);
			free(b);
		}
		
		cw = dt_drawtext_col(sp->u.msg.canvas,
				     sp->u.msg.dspcur.x,
				     sp->u.msg.dspcur.y + wRSize + wRLineSpace,
				     mbuf,
				     wColorR, wColorG, wColorB);
		
		needupdate = TRUE;
		
		append_to_log(mbuf);
		
		if (wSpeed > 0) {
			sp_updateme_part(sp,
					 sp->u.msg.dspcur.x,
					 sp->u.msg.dspcur.y,
					 cw,
					 wSize + wRSize + wRLineSpace);
			sp_update_clipped();
			needupdate = FALSE;
			
			// keywait
			delta = get_high_counter(SYSTEMCOUNTER_MSEC) - wcnt;
			if (delta < wSpeed) {
				if (sys_keywait(wSpeed - delta, FALSE)) {
					
					wSpeed = 0;
				}
			}
		}
		
		sp->u.msg.dspcur.x += cw;
	}
	
	sactlog_newline();
	
	// �Хåե�������ʸ���Υ��ꥢ
	sact.msgbuf[0] = '\0';
	
	// Wait�ʤ��ν��ϤϺǸ��update
	if (needupdate) {
		uparea.width  = sp->cursize.width;
		uparea.height = min(sp->cursize.height, uparea.y - sp->u.msg.dspcur.y + wLineSpace + wLineSpace + wRSize);
		sp_updateme_part(sp, uparea.x, uparea.y, uparea.width, uparea.height);
	}
	
	// ????
	if (wLength) {
		*wLength = len;
	}
}

/*
  ��å������ΰ�Υ��ꥢ
  @param wNum: ���ꥢ���륹�ץ饤���ֹ�
 */
void smsg_clear(int wNum) {
	sprite_t *sp;
	surface_t *sf;
	
	if (!is_messagesprite(wNum)) return;
	
	// ɽ�����֤ν����
	sp = sact.sp[wNum];
	sp->u.msg.dspcur.x = 0;
	sp->u.msg.dspcur.y = 0;
	
	sact.msgbuf[0]  = '\0';
	sact.msgbuf2[0] = '\0';
	
	// �����Х��Υ��ꥢ
	sf = sp->u.msg.canvas;
	memset(sf->pixel, 0, sf->bytes_per_line * sf->height);
	memset(sf->alpha, 0, sf->width * sf->height);
	
	sp_updateme(sp);
	
	if (sact.logging) {
		sact.log = g_list_append(sact.log, g_strdup("\n"));
	}
}

/*
  �������ʸ���󤬤��뤫�����å�
  @return: �ʤ�(0) , ����(1)
 */
int smsg_is_empty() {
	return (sact.msgbuf[0] != '\0');
}

/*
  ��å��������������Ԥ�
   @param wNum1: ���ץ饤���ֹ�1(���˥᡼����󥹥ץ饤��)
   @param wNum2: ���ץ饤���ֹ�2(���˥᡼����󥹥ץ饤��)
   @param msglen: ?
 */
int smsg_keywait(int wNum1, int wNum2, int msglen) {
	struct markinfo minfo[6];
	int i = 0, j, maxstep;
	
	if (sact.waitskiplv > 0) return 0;
	
	// ���˥�ѥ�����ν����
	maxstep = setupmark(wNum1, wNum2, minfo);
	
	sact.waittype = KEYWAIT_MESSAGE;
	sact.waitkey = -1;
	
	while (sact.waitkey == -1) {
		int st = get_high_counter(SYSTEMCOUNTER_MSEC);
		int interval = 25;
		
		// ���˥�ѥ����󤬤����硢���ι���
		// Z�����Ǳ�����Ƥ���Ȥ�ɽ�����ʤ�
		// (���˥�ѥ����󤬱����оݤǤʤ����Ϥɤ����褦...)
		if (maxstep &&
		    !sact.zhiding &&
		    sact.waittype != KEYWAIT_BACKLOG) {
			j = i % maxstep;
			interval = minfo[j].interval;
			update_mark(minfo[j].sp, minfo[j].cg);
			i++;
		} 
		sys_keywait(interval - (get_high_counter(SYSTEMCOUNTER_MSEC) - st), FALSE);
	}
	
	sact.waittype = KEYWAIT_NONE;
	
	return sact.waitkey;
}

/*
  ���ץ饤�Ⱥ�����Υ�����Хå�
  @param sp: �����褹�륹�ץ饤��
 */
int smsg_update(sprite_t *sp) {
	int sx, sy, w, h, dx, dy;
	surface_t update;
	
	// canvas �� clean �ΤȤ��Ϥʤˤ⤷�ʤ�
	//  -> �������ץ饤�ȤΤ褦�ˡ�SetShow���줿�Ȥ����б��Ǥ��ʤ�������� 
	//if (sact.msgbufempty) return OK;
	
	update.width  = sact.updaterect.width;
	update.height = sact.updaterect.height;
	
	dx = sp->cur.x - sact.updaterect.x;
	dy = sp->cur.y - sact.updaterect.y;
	
	w = sp->cursize.width;
	h = sp->cursize.height;
	
	sx = 0; sy = 0;
	
	if (!gr_clip(sp->u.msg.canvas, &sx, &sy, &w, &h, &update, &dx, &dy)) {
		return NG;
	}
	
	dx += sact.updaterect.x;
	dy += sact.updaterect.y;
	
	gre_BlendUseAMap(sf0, dx, dy, sf0, dx, dy, sp->u.msg.canvas, sx, sy, w, h, sp->u.msg.canvas, sx, sy, sp->blendrate);
	
	WARNING("do update no=%d, sx=%d, sy=%d, w=%d, h=%d, dx=%d, dy=%d\n",
		sp->no, sx, sy, w, h, dx, dy);
	
	return OK;
}

// ���Ԥ�ޤ�ʸ����Хåե��椫�顢���ԤޤǤ�ʸ�����Ĺ������Ф�
static int get_linelen(BYTE *msg) {
	int c = 0;
	
	while (*msg) {
		if (*msg == '\n') break;
		if (0 == strncmp("|RB|", msg, 4)) {
			msg += 4;
			while (*msg != '|') {
				msg++; c++;
			}
			msg++;
			while (*msg != '|') {
				msg++;
			}
			msg++;
		} else {
			msg++; c++;
		}
	}
	return c;
}

// ���Ԥ��Ӥ�ޤ�ʸ����Хåե����餽������Ф�
//   ���Ԥξ��        : �������⤤�ä����
//   ��ӤĤ�ʸ���ξ��: ��å��������Τ��б�������ʸ����
//   ����ʳ�          : ����|Ⱦ��ʸ����ʸ��
static BYTE *get_char(BYTE *msg, char *mbuf, char *rbuf, int bufmax) {
	int c1, i;

	//  ����
	if (msg[0] == '\n') {
		mbuf[0] = '\n';
		mbuf[1] = msg[1];
		mbuf[2] = '\0';
		return msg +2;
	}
	
	// ��ӤĤ�ʸ��
	if (0 == strncmp("|RB|", msg, 4)) {
		msg += 4;
		for (i = 0; *msg != '|' && i < bufmax; i++) {
			mbuf[i] = *msg++;
		}
		msg++; mbuf[i] = '\0';
		for (i = 0; *msg != '|' && i < bufmax; i++) {
			rbuf[i] = *msg++;
		}
		msg++; rbuf[i] = '\0';
	} else {
		c1 = *msg++;
		
		*mbuf++ = c1;
		
		if ((c1 >= 0x81 && c1 < 0xa0) || (c1 >= 0xe0 && c1 <= 0xee)) {
			*mbuf++ = *msg++;
		}
		*mbuf = '\0';
	}
	return msg;
}

// �Хå����ѥХåե����ɲ�
static void append_to_log(char *msg) {
	if (sact.logging) {
		int len = MSGBUFMAX - (int)strlen(sact.msgbuf2);
		strncat(sact.msgbuf2, msg, len);
		sact.msgbuf2[MSGBUFMAX -1] = '\0';
	}
}

// �Хå�������¸����ꥹ�Ȥ��ɲ�
static void sactlog_newline() {
	if (sact.logging) {
		if (sact.msgbuf2[0] == '\0') return;
		sact.log = g_list_append(sact.log, g_strdup(sact.msgbuf2));
		sact.msgbuf2[0] = '\0';
	}
}

// �񤭽Ф����֤�x=0�λ��Τߥ��饤����Ȥ�Ĵ����Ԥ�
static void set_align(char *msg, sprite_t *sp, int wSize, int wAlign) {
	if (sp->u.msg.dspcur.x == 0) {
		int mlen = get_linelen(msg) * wSize/2;
		int adjx = 0;
		
		switch (wAlign) {
		case 1: // ���󥿥��
			adjx = (sp->cursize.width - mlen) / 2;
			break;
		case 2: // ����
			adjx = (sp->cursize.width - mlen);
			break;
		}
		sp->u.msg.dspcur.x = max(0, adjx);
	}
}
