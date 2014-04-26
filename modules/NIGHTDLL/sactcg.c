/*
 * sactcg.c: CG����
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
/* $Id: sactcg.c,v 1.2 2003/11/16 15:29:52 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "portab.h"
#include "nact.h"
#include "system.h"
#include "ags.h"
#include "surface.h"
#include "ngraph.h"
#include "sactcg.h"

#define CGMAX 65536
static cginfo_t *cgs[CGMAX];

#include "sactcg_stretch.c"
#include "sactcg_blend.c"


#define spcg_assert_no(no) \
  if ((no) > (CGMAX -1)) { \
    WARNING("no is too large (should be %d < %d)\n", (no), CGMAX); \
    return NG; \
  } \

 
/*
  cg���ɤ߹���

    ������ֹ��CG���󥯥ե����뤫���ɤ߹�����ꡢ
    CG_xxx�Ǻ�������CG�򻲾Ȥ���
    
  @param no: �ɤ߹���CG�ֹ�
  @param refinc: ���ȥ����󥿤����䤹���ɤ�����
                 sprite���黲�Ȥ����Ȥ������䤷��CG_xxx�������
                 ���Ȥ����Ȥ������䤵�ʤ���
*/
cginfo_t *scg_loadcg_no(int no, boolean refinc) {
	cginfo_t *i;
	
	if (no >= (CGMAX -1)) {
		WARNING("no is too large (should be %d < %d)\n", (no), CGMAX);
		return NULL;
	}
	
	// ���Ǥ� ���ɤ���Ƥ��뤫��CG_xxx �Ǻ������ߤξ���
	// ���ȥ����󥿤����䤹
	if (cgs[no] != NULL) {
		if (refinc) {
			cgs[no]->refcnt++;
		}
		return cgs[no];
	}
	
	i = g_new(cginfo_t, 1);
	i->type = CG_LINKED;
	i->no   = no;
	i->refcnt = (refinc ? 1 : 0);
	i->sf   = sf_loadcg_no(no -1);
	if (i->sf == NULL) {
		WARNING("load fail (%d)\n", no -1);
		g_free(i);
		return NULL;
	}
	
	cgs[no] = i;
	
	return i;
}

//  ������礭�������ζ���� CG �����
int scg_create(int wNumCG, int wWidth, int wHeight, int wR, int wG, int wB, int wBlendRate) {
	cginfo_t *i;
	
	spcg_assert_no(wNumCG);
	
	i = g_new(cginfo_t, 1);
	i->type = CG_SET;
	i->no   = wNumCG;
	i->refcnt = 1;
	i->sf = sf_create_surface(wWidth, wHeight, sf0->depth);
	gr_fill(i->sf, 0, 0, wWidth, wHeight, wR, wG, wB);
	gr_fill_alpha_map(i->sf, 0, 0, wWidth, wHeight, wBlendRate);
	
	// �⤷���˺���������Τ����ꡢ̤�����ξ��ϳ���
	scg_free(wNumCG);
	
	cgs[wNumCG] = i;
	
	return OK;
}

// �����CG��ȿž������CG�����
int scg_create_reverse(int wNumCG, int wNumSrcCG, int wReverseX, int wReverseY) {
	cginfo_t *i, *srccg;
	surface_t *src;
	
	spcg_assert_no(wNumCG);
	spcg_assert_no(wNumSrcCG);
	
	// ���ˤ���CG�򻲾� (LINKCG�ʤ��ɤ߹���)
	if (NULL == (srccg = scg_loadcg_no(wNumSrcCG, FALSE))) {
		return NG;
	}
	
	i = g_new(cginfo_t, 1);
	i->type = CG_REVERSE;
	i->no   = wNumCG;
	i->refcnt = 0;
	
	src = srccg->sf;
	i->sf = stretch(src, src->width, src->height, (wReverseX << 1) | wReverseY);
	// �⤷���˺���������Τ����ꡢ̤�����ξ��ϳ���
	scg_free(wNumCG);
	
	cgs[wNumCG] = i;
	
	return OK;
}

// �����CG�����/�̾�����CG�����
int scg_create_stretch(int wNumCG, int wWidth, int wHeight, int wNumSrcCG) {
	cginfo_t *i, *srccg;
	surface_t *src;
	
	spcg_assert_no(wNumCG);
	spcg_assert_no(wNumSrcCG);

	// ���ˤ���CG�򻲾� (LINKCG�ʤ��ɤ߹���)
	if (NULL == (srccg = scg_loadcg_no(wNumSrcCG, FALSE))) {
		return NG;
	}
	
	i = g_new(cginfo_t, 1);
	i->type = CG_STRETCH;
	i->no   = wNumCG;
	i->refcnt = 0;
	
	src = srccg->sf;
	i->sf = stretch(src, wWidth, wHeight, 0);
	
	// �⤷���˺���������Τ����ꡢ̤�����ξ��ϳ���
	scg_free(wNumCG);
	
	cgs[wNumCG] = i;
	
	return OK;
}

// �١���CG�ξ�˥֥���CG��Ťͤ� CG �����
int scg_create_blend(int wNumDstCG, int wNumBaseCG, int wX, int wY, int wNumBlendCG, int wAlphaMapMode) {
	cginfo_t *i, *basecg, *blendcg;
	
	spcg_assert_no(wNumDstCG);
	spcg_assert_no(wNumBaseCG);
	spcg_assert_no(wNumBlendCG);
	
	// ���ˤ���CG�򻲾� (LINKCG�ʤ��ɤ߹���)
	basecg  = scg_loadcg_no(wNumBaseCG, FALSE);
	blendcg = scg_loadcg_no(wNumBlendCG, FALSE);
	if (basecg == NULL || blendcg == NULL) return NG;
	
	i = g_new(cginfo_t, 1);
	i->type = CG_SET;
	i->no = wNumDstCG;
	i->refcnt = 0;
	
	i->sf = blend(basecg->sf, wX , wY, blendcg->sf, wAlphaMapMode);
	
	// �⤷���˺���������Τ����ꡢ̤�����ξ��ϳ���
	scg_free(wNumDstCG);
	
	cgs[wNumDstCG] = i;
	
	return OK;
}

// �����ʸ�����CG�����
int scg_create_text(int wNumCG, int wSize, int wR, int wG, int wB, char *cText) {
	cginfo_t *i;
	agsurface_t *glyph;
	FONT *font;
	
	if (0) {
		char *b = sjis2euc(cText);
		WARNING("str = '%s'\n", b);
		free(b);
	}
	
	spcg_assert_no(wNumCG);
	
	// ����˽ФƤ����Τ��ʡ�
	if (strlen(cText) == 0) return OK;
	
	font = nact->ags.font;
	font->sel_font(FONT_GOTHIC, wSize);
	
	glyph = font->get_glyph(cText);
	i = g_new(cginfo_t, 1);
	i->type = CG_SET;
	i->no = wNumCG;
	i->refcnt = 0;
	
	i->sf = sf_create_surface(glyph->width, wSize, nact->ags.dib->depth);
	gr_fill(i->sf, 0, 0, glyph->width, wSize, wR, wG, wB);
	gr_draw_amap(i->sf, 0, 0, glyph->pixel, glyph->width, wSize, glyph->bytes_per_line);
	
	// �⤷���˺���������Τ����ꡢ̤�����ξ��ϳ���
	scg_free(wNumCG);
	
	cgs[wNumCG] = i;
	
	return OK;
}

// ����ʸ�����CG�����
int scg_create_textnum(int wNumCG, int wSize, int wR, int wG, int wB, int wFigs, int wZeroPadding, int wValue) {
	cginfo_t *i;
	agsurface_t *glyph;
	FONT *font;
	char s[256], ss[256];
	
	spcg_assert_no(wNumCG);
	
	if (wZeroPadding) {
		char *sss = "%%0%dd";
		sprintf(ss, sss, wFigs);
	} else {
		char *sss = "%%%dd";
		sprintf(ss, sss, wFigs);
	}
	sprintf(s, ss, wValue);
	
	font = nact->ags.font;
	font->sel_font(FONT_GOTHIC, wSize);
	glyph = font->get_glyph(s);
	
	i = g_new(cginfo_t, 1);
	i->type = CG_SET;
	i->no = wNumCG;
	i->refcnt = 0;
	i->sf = sf_create_surface(glyph->width, wSize, nact->ags.dib->depth);
	gr_fill(i->sf, 0, 0, glyph->width, wSize, wR, wG, wB);
	gr_draw_amap(i->sf, 0, 0, glyph->pixel, glyph->width, wSize, glyph->bytes_per_line);
	
	// �⤷���˺���������Τ����ꡢ̤�����ξ��ϳ���
	scg_free(wNumCG);
	
	cgs[wNumCG] = i;
	
	return OK;
}

// CG��ʣ��
int scg_copy(int wNumDstCG, int wNumSrcCG) {
	cginfo_t *i, *srccg;
	
	spcg_assert_no(wNumDstCG);
	spcg_assert_no(wNumSrcCG);
	
	// ���ˤ���CG�򻲾� (LINKCG�ʤ��ɤ߹���)
	if (NULL == (srccg = scg_loadcg_no(wNumSrcCG, FALSE))) {
		return NG;
	}
	
	i = g_new(cginfo_t, 1);
	i->type = CG_SET;
	i->no = wNumDstCG;
	i->refcnt = 0;
	i->sf = sf_dup(srccg->sf);
	
	// �⤷���˺���������Τ����ꡢ̤�����ξ��ϳ���
	scg_free(wNumDstCG);
	
	cgs[wNumDstCG] = i;
	
	return OK;
}

// CG�ΰ������ڤ�̤���CG�����
int scg_cut(int wNumDstCG, int wNumSrcCG, int wX, int wY, int wWidth, int wHeight) {
	cginfo_t *i, *srccg;
	surface_t *dst, *src;
	
	spcg_assert_no(wNumDstCG);
	spcg_assert_no(wNumSrcCG);
	
	// ���ˤ���CG�򻲾� (LINKCG�ʤ��ɤ߹���)
	if (NULL == (srccg = scg_loadcg_no(wNumSrcCG, FALSE))) {
		return NG;
	}
	
	i = g_new(cginfo_t, 1);
	i->type = CG_SET;
	i->no = wNumDstCG;
	i->refcnt = 0;
	
	src = srccg->sf;
	if (src->has_alpha) {
		dst = sf_create_surface(wWidth, wHeight, src->depth);
	} else {
		dst = sf_create_pixel(wWidth, wHeight, src->depth);
	}
	if (src->has_pixel) {
		gr_copy(dst, 0, 0, src, wX, wY, wWidth, wHeight);
	}
	if (src->has_alpha) {
		gr_copy_alpha_map(dst, 0, 0, src, wX, wY, wWidth, wHeight);
	}
	
	i->sf = dst;
	
	// �⤷���˺���������Τ����ꡢ̤�����ξ��ϳ���
	scg_free(wNumDstCG);
	
	cgs[wNumDstCG] = i;
	
	return OK;
}

// ����CG�ΰ������ڤ�̤���CG�����
int scg_partcopy(int wNumDstCG, int wNumSrcCG, int wX, int wY, int wWidth, int wHeight) {
	cginfo_t *i, *srccg;
	surface_t *dst, *src;
	
	spcg_assert_no(wNumDstCG);
	spcg_assert_no(wNumSrcCG);
	
	// ���ˤ���CG�򻲾� (LINKCG�ʤ��ɤ߹���)
	if (NULL == (srccg = scg_loadcg_no(wNumSrcCG, FALSE))) {
		return NG;
	}
	
	i = g_new(cginfo_t, 1);
	i->type = CG_SET;
	i->no = wNumDstCG;
	i->refcnt = 0;

	src = srccg->sf;
	if (src->has_alpha) {
		dst = sf_create_surface(src->width, src->height, src->depth);
		gr_fill_alpha_map(dst, 0, 0, src->width, src->height, 255);
	} else {
		dst = sf_create_pixel(src->width, src->height, src->depth);
	}
	
	if (src->has_pixel) {
		gr_copy(dst, wX, wY, src, wX, wY, wWidth, wHeight);
	}
	if (src->has_alpha) {
		gr_copy_alpha_map(dst, wX, wY, src, wX, wY, wWidth, wHeight);
	}
	
	i->sf = dst;
	
	// �⤷���˺���������Τ����ꡢ̤�����ξ��ϳ���
	scg_free(wNumDstCG);
	
	cgs[wNumDstCG] = i;
	
	return OK;
}

// ���Ƥ�CG�γ���
int scg_freeall() {
	int i;
	
	for (i = 1; i < CGMAX; i++) {
		scg_free(i);
	}
	return OK;
}

/**
 * ������ֹ�� CG �򥪥֥������ȥꥹ�Ȥ���ä������֥������Ȥ��ɤ�����⻲��
 * ����Ƥ��ʤ�(���ȿ���0��)���Τߡ����֥������Ȥ���
 */
int scg_free(int no) {
	cginfo_t *cg;
	
	spcg_assert_no(no);
	
	if (NULL == (cg = cgs[no])) return NG;
	
	// ���ȿ���0�λ��Τߥ��֥������Ȥ���
	if (cg->refcnt == 0) {
		scg_free_cgobj(cg);
	}
	
	// �ֹ�Ǿä����Ȥ��ϥ��֥������Ȥ���������ʤ��Ƥ�
	// ���֥������ȥꥹ�Ȥ�����
	cgs[no] = NULL;
	
	return OK;
}

/**
 * CG ���֥������Ȥγ���
 */
int scg_free_cgobj(cginfo_t *cg) {
	if (cg == NULL) return NG;
	
	(cg->refcnt)--;
	// ¾�Ǥޤ����Ȥ��Ƥ���г������ʤ�
	if (cg->refcnt > 0) {
		return NG;
	}
	
	// CG���Τγ���
	if (cg->sf) {
		sf_free(cg->sf);
	}
	
	// ������륪�֥������Ȥ򻲾Ȥ��Ƥ��륪�֥������ȥꥹ�Ȥ���
	if (cg == cgs[cg->no]) {
		cgs[cg->no] = NULL;
	}
	
	g_free(cg);
	
	return OK;
}

// CG�μ�������
int scg_querytype(int wNumCG, int *ret) {
	if (wNumCG >= (CGMAX -1)) goto errexit;
	if (cgs[wNumCG] == NULL) goto errexit;

	*ret = cgs[wNumCG]->type;
	
	return OK;

 errexit:
	*ret = CG_NOTUSED;
	return NG;
}

// CG���礭�������
int scg_querysize(int wNumCG, int *w, int *h) {
	if (wNumCG >= (CGMAX -1)) goto errexit;
	if (cgs[wNumCG] == NULL) goto errexit;
	if (cgs[wNumCG]->sf == NULL) goto errexit;

	*w = cgs[wNumCG]->sf->width;
	*h = cgs[wNumCG]->sf->height;
	
	return OK;

 errexit:
	*w = *h = 0;
	return NG;
}

// CG��BPP�����
int scg_querybpp(int wNumCG, int *ret) {
	if (wNumCG >= (CGMAX -1)) goto errexit;
	if (cgs[wNumCG] == NULL) goto errexit;
	if (cgs[wNumCG]->sf == NULL) goto errexit;
	
	*ret = cgs[wNumCG]->sf->depth;

	return OK;

 errexit:
	*ret = 0;
	return NG;
}

// CG�� alphamap ��¸�ߤ��뤫�����
int scg_existalphamap(int wNumCG, int *ret) {
	if (wNumCG >= (CGMAX -1)) goto errexit;
	if (cgs[wNumCG] == NULL) goto errexit;
	if (cgs[wNumCG]->sf == NULL) goto errexit;
	
	*ret = cgs[wNumCG]->sf->has_alpha ? 1 : 0;
	
 errexit:
	*ret = 0;
	return NG;
}
