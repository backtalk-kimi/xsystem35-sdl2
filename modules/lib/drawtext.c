/*
 * drawtext.c  DLL�Ѥ� surface ���ʸ��������
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
/* $Id: drawtext.c,v 1.2 2003/07/21 23:06:47 chikama Exp $ */

#include "config.h"

#include <stdio.h>

#include "portab.h"
#include "system.h"
#include "nact.h"
#include "ags.h"
#include "font.h"
#include "surface.h"
#include "ngraph.h"

static int ftype;  // �ե���Ȥμ���
static int fsize;  // �ե���Ȥ��礭��

/**
 * ��������ʸ���Υե���Ȥμ�����礭��������
 * 
 * @param type: �ե���ȼ��� (FONT_MINCHO, FONT_GOTHIC)
 * @param size: �ե���ȥ�����
 */
int dt_setfont(int type, int size) {
	ftype = type;
	fsize = size;
	return OK;
}

/**
 * surface �˥�Υ���ʸ�������衣����������ꥢ������Ƥ���256��Ĵ
 * �񤭽Ф�surface��8�ӥåȤǤ���ɬ�פ���
 *
 * @param sf: ���褹�� surface
 * @param x: ������֣غ�ɸ
 * @param y: ������֣ٺ�ɸ
 * @param buf: ����ʸ���� (SJIS)
 * @return: �ºݤ����褷����
*/
int dt_drawtext(surface_t *sf, int x, int y, char *buf) {
	agsurface_t *glyph;
	int sx, sy, sw, sh;
	FONT *font = nact->ags.font;
	
	font->sel_font(ftype, fsize);
	
	glyph = font->get_glyph(buf);
	if (glyph == NULL) return 0;
	
	sx = x;	sy = y;
	sw = glyph->width;
	sh = glyph->height;
	if (!gr_clip_xywh(sf, &sx, &sy, &sw, &sh)) return 0;
	
	gr_copy(sf, sx, sy, glyph, 0, 0, sw, sh);
	
	return sw;
}

/**
 * surface �˥��顼ʸ�������衣alphamap ��256��Ĵ�Υ���������ꥢ�����줿
 * ʸ����������pixelmap �ˤϡ����������������
 * 
 * @param sf: ���褹�� surface
 * @param x: ������֣غ�ɸ
 * @param y: ������֣ٺ�ɸ
 * @param buf: ����ʸ����(SJIS)
 * @param r: ʸ������
 * @param g: ʸ������
 * @param b: ʸ������
 * @return: �ºݤ����褷����
 */ 
int dt_drawtext_col(surface_t *sf, int x, int y, char *buf, int r, int g, int b) {
	agsurface_t *glyph;
	int sx, sy, sw, sh;
	FONT *font = nact->ags.font;
	
	font->sel_font(ftype, fsize);
	
	glyph = font->get_glyph(buf);
	if (glyph == NULL) return 0;

	sx = x;	sy = y;
	sw = glyph->width;
	sh = glyph->height;
	if (!gr_clip_xywh(sf, &sx, &sy, &sw, &sh)) return 0;
	
	// alpha map ��ʸ�����Τ�Τ�����
	gr_draw_amap(sf, sx, sy, glyph->pixel, sw, sh, glyph->bytes_per_line);
	
	// pixel map �ˤϿ��������������
	gr_fill(sf, sx, sy, sw, sh, r, g, b);
	
	return sw;
}
