/*
 * Gpx.c  Graphics ���Ѵؿ�
 *
 *      OnlyYou -�ꡦ������
 *      system3.9�� ���ܲ����
 *      �簭��
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
/* $Id: Gpx.c,v 1.11 2003/04/22 16:29:52 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <glib.h>

#include "portab.h"
#include "system.h"
#include "xsystem35.h"
#include "nact.h"
#include "ags.h"
#include "cg.h"
#include "image.h"
#include "counter.h"
#include "imput.h"
// #include "alpha_plane.h"
#include "surface.h"
#include "graph.h"
// #include "graph2.h"

#include "effectcopy.h"
#include "ngraph.h"

#define MAX_SURFACE 256




static int pre_freesurfno; /* ľ���˳������� surface no */
static surface_t *suf[MAX_SURFACE];

static int find_null_surface() {
	int i;

	if (suf[pre_freesurfno] == NULL) return pre_freesurfno;

	for (i = 1; i < MAX_SURFACE; i++) {
		if (suf[i] == NULL) return i;
	}
	
	SYSERROR("no free surface\n");
	return 0;
}

static int sf_free_one(int no) {
	surface_t *s;
	if (no == 0) return NG;
	
	s = suf[no];
	if (s == NULL) return NG;
	
	if (s->pixel) g_free(s->pixel);
	if (s->alpha) g_free(s->alpha);
	g_free(s);
	
	suf[no] = NULL;
	pre_freesurfno = no;
	return OK;
}

static int sf_free_all() {
	int i;
	surface_t *s;
	
	for (i = 1; i < MAX_SURFACE; i++) {
		if (suf[i] == NULL) continue;
		s = suf[i];
		if (s->pixel) g_free(s->pixel);
		if (s->alpha) g_free(s->alpha);
		g_free(s);
		suf[i] = NULL;
	}
	
	pre_freesurfno = 1;
	return OK;
}

static surface_t *sf_get(int no) {
	if (no == 0) {
		return nact->ags.dib;
	}
	return suf[no];
}

static int load_cg_main(int no) {
	surface_t *sf = sf_loadcg_no(no);
	int sno;

	if (sf == NULL) {
		WARNING("load fail(cg==NULL,no=%d)\n", no);
		return 0;
	}
	
	sno = find_null_surface();
	
	sf->no = sno;
	suf[sno] = sf;
	
	return sf->no;
}

void Init() {
	/*
	  Gpx.Init(): Gpx �⥸�塼��ν����
	*/
	int p1 = getCaliValue(); /* ISys3x */
	
	suf[0] = NULL;
	gr_init();
	
	// surface0 �� pre_freesurfno �Ȥ����֤��ʤ��褦�ˡ�
	pre_freesurfno = 1;
	
	DEBUG_COMMAND("Gpx.Init %d:\n", p1);
}

void Create() {
	/*
	  Gpx.Create(): ���� surface �κ���(Pixel��Alpha�ޥåפ�ξ��)
	  
	   var   : �������� surface ���ֹ���֤��ѿ�
	           �����˼��Ԥ������� 0 ���֤�
	   width : surface ����
	   height: surface �ι⤵
	   bpp   : surface �ο���(���ꥸ�ʥ�Ǥ�24bpp�Τߥ��ݡ���,
	                          xsystem35 �Ǥ� display �� depth ��Ʊ��)
	*/
	int *var   = getCaliVariable();
	int width  = getCaliValue();
	int height = getCaliValue();
	int bpp    = getCaliValue();
	surface_t *s;
	
	//get_surface0();
	s = sf_create_surface(width, height, sf_get(0)->depth);
	
	if (s == NULL) {
		*var = 0;
	} else {
		int no = find_null_surface();
		*var = s->no = no;
		suf[no] = s;
	}
	
	DEBUG_COMMAND("Gpx.Create %p,%d,%d,%d:\n", var, width, height, bpp);
}

void CreatePixelOnly() {
	/*
	  Gpx.CreatePixelOnly(): ���� surface �κ���(Pixel�Τ�)
	  
	   var   : �������� surface ���ֹ���֤��ѿ�
	   width : surface ����
	   height: surface �ι⤵
	   bpp   : surface �ο���(24bpp only)
	*/
	int *var   = getCaliVariable();
	int width  = getCaliValue();
	int height = getCaliValue();
	int bpp    = getCaliValue();
	surface_t *s;
	
	s = sf_create_pixel(width, height, sf_get(0)->depth);
	
	if (s == NULL) {
		*var = 0;
	} else {
		int no = find_null_surface();
		*var = s->no = no;
		suf[no] = s;
	}
	
	DEBUG_COMMAND("Gpx.CreatePixelOnly %d,%d,%d,%d:\n", *var, width, height, bpp);
}

void CreateAMapOnly() {
	/*
	  Gpx.CreateAMapOnly(): ���� surface �κ���(AlphaMap�Τ�)
	  
	   var   : �������� surface ���ֹ���֤��ѿ�
	   width : surface ����
	   height: surface �ι⤵
	*/
	int *var   = getCaliVariable();
	int width  = getCaliValue();
	int height = getCaliValue();
	surface_t *s;
	
	s = sf_create_alpha(width, height);
	
	if (s == NULL) {
		*var = 0;
	} else {
		int no = find_null_surface();
		*var = s->no = no;
		suf[no] = s;
	}
	
	DEBUG_COMMAND("Gpx.CreateAMapOnly %p,%d,%d:\n", var, width, height);
}

void IsSurface() {
	/*
	  Gpx.IsSurface(): ������ֹ�� surface �� surface ���ɤ���
	                   (pixel �� alpha ��ξ���Υǡ��������)��Ĵ�٤�
	  
	   p1  : surface �ֹ�
	   var : ��̤��֤��ѿ���surface �ʤ�� 1, !surface �ʤ�� 0
	*/
	int p1   = getCaliValue();
	int *var = getCaliVariable();
	surface_t *s;
	
	s = sf_get(p1);
	
	if (s == NULL) {
		*var = 0;
	} else {
		*var = (s->has_alpha && s->has_pixel) ? 1 : 0;
	}
	
	DEBUG_COMMAND("Gpx.IsSurface %d,%p:\n", p1, var);
}

void IsPixel() {
	/*
	  Gpx.IsPixel(): ������ֹ�� surface �� pixel�ǡ������ɤ�����Ĵ�٤�
	  
	   p1  : surface �ֹ�
	   var : ��̤��֤��ѿ���pixel �ʤ�� 1, !pixel �ʤ�� 0
	*/
	int p1   = getCaliValue();
	int *var = getCaliVariable();
	surface_t *s;
	
	s = sf_get(p1);
	
	if (s == NULL) {
		*var = 0;
	} else {
		*var = s->has_pixel ? 1 : 0;
	}
	
	DEBUG_COMMAND("Gpx.IsPixel %d,%p:\n", p1, var);
}

void IsAlpha() {
	/*
	  Gpx.IsAlpha(): ������ֹ�� surface �� alpha map���ɤ�����Ĵ�٤�
	  
	   p1  : surface �ֹ�
	   var : ��̤��֤��ѿ���alpha �ʤ�� 1, !alpha �ʤ�� 0
	*/
	int p1   = getCaliValue();
	int *var = getCaliVariable();
	surface_t *s;
	
	s = sf_get(p1);
	
	if (s == NULL) {
		*var = 0;
	} else {
		*var = s->has_alpha ? 1 : 0;
	}
	
	DEBUG_COMMAND("Gpx.IsAlpha %d,%p:\n", p1, var);
}

void GetWidth() {
	/*
	  Gpx.GetWidth(): ������ֹ�� surface �������������
	  
	   p1  : surface �ֹ�
	   var : ��̤��֤��ѿ���
	*/
	int p1   = getCaliValue();
	int *var = getCaliVariable();
	surface_t *s;
	
	s = sf_get(p1);
	
	if (s == NULL) {
		*var = 0;
	} else {
		*var = s->width;
	}
	
	DEBUG_COMMAND("Gpx.GetWidth %d,%d:\n", p1, *var);
}

void GetHeight() {
	/*
	  Gpx.GetWidth(): ������ֹ�� surface �ι⤵���������
	  
	   p1  : surface �ֹ�
	   var : ��̤��֤��ѿ���
	*/
	int p1   = getCaliValue();
	int *var = getCaliVariable();
	surface_t *s;
	
	s = sf_get(p1);
	
	if (s == NULL) {
		*var = 0;
	} else {
		*var = s->height;
	}
	
	DEBUG_COMMAND("Gpx.GetHeight %d,%d:\n", p1, *var);
}

void GetCreatedSurface() { /* not used ? */
	int *var = getCaliVariable();
	
	DEBUG_COMMAND_YET("Gpx.GetCreatedSurface %p:\n", var);
}

void LoadCG() {
	/*
	  Gpx.LoadCG(): ���� surface ��������Ƥ��ξ�� CG �� load
	  
	   var : �������� surface ���ֹ���֤��ѿ�
	   p1  : �ɤ߹��� CG ���ֹ�
	*/
	int *var = getCaliVariable();
	int p1   = getCaliValue();
	
	*var = load_cg_main(p1 -1);
	
	DEBUG_COMMAND("Gpx.LoadCG %p,%d (%d):\n", var, p1, *var);
}

void GetCGPosX() { /* not useed ? */
	int *var = getCaliVariable();
	int p1 = getCaliValue();
	
	DEBUG_COMMAND_YET("Gpx.GetCgPosX %p,%d:\n", var, p1);
}

void GetCGPosY() { /* not useed ? */
	int *var = getCaliVariable();
	int p1 = getCaliValue();
	
	DEBUG_COMMAND_YET("Gpx.GetCgPosY %p,%d:\n", var, p1);
}

void Free() {
	/*
	  Gpx.Free(): ����� surface ��������
	  
	   p1: �������� surface ���ֹ�
	*/
	int p1 = getCaliValue();
	
	DEBUG_COMMAND("Gpx.Free %d:\n", p1);
	
	if (p1 != 0) {
		sf_free_one(p1);
	}
}

void FreeAll() {
	/*
	  Gpx.FreeAll(): ���Ƥ� surface ��������
	*/
	sf_free_all();
	
	DEBUG_COMMAND("Gpx.FreeAll:\n");
}

void Copy() {
	/*
	  Gpx.Copy(): ���� surface �ΰ�Υ��ԡ�
	  
	   ds: ž���� surface �ֹ�
	   dx: ž���� x ��ɸ
	   dy: ž���� y ��ɸ
	   ss: ž���� surface �ֹ�
	   sx: ž���� x ��ɸ
	   sy: ž���� y ��ɸ
	   sw: ž���� ��
	   sh: ž���� �⤵
	*/
	int ds = getCaliValue();
	int dx = getCaliValue();
	int dy = getCaliValue();
	int ss = getCaliValue();
	int sx = getCaliValue();
	int sy = getCaliValue();
	int sw = getCaliValue();
	int sh = getCaliValue();
	surface_t *src, *dst;
	
	DEBUG_COMMAND("Gpx.Copy %d,%d,%d,%d,%d,%d,%d,%d:\n", ds, dx, dy, ss, sx, sy, sw, sh);
	
	ags_sync();
	src = sf_get(ss);
	dst = sf_get(ds);
	gr_copy(dst, dx, dy, src, sx, sy, sw, sh);
}

void CopyBright() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	int p8 = getCaliValue();
	int p9 = getCaliValue();

	DEBUG_COMMAND_YET("Gpx.CopyBright %d,%d,%d,%d,%d,%d,%d,%d,%d:\n", p1, p2, p3, p4, p5, p6, p7, p8, p9);
}

void CopyAMap() {
	/*
	  Gpx.CopyAMap(): ���� surface �� alpha map �ΰ�Υ��ԡ�
	  
	   da: ž���� surface(alpha map) �ֹ�
	   dx: ž���� x ��ɸ
	   dy: ž���� y ��ɸ
	   sa: ž���� surface(alpha map) �ֹ�
	   sx: ž���� x ��ɸ
	   sy: ž���� y ��ɸ
	   sw: ž���� ��
	   sh: ž���� �⤵
	*/
	int da = getCaliValue();
	int dx = getCaliValue();
	int dy = getCaliValue();
	int sa = getCaliValue();
	int sx = getCaliValue();
	int sy = getCaliValue();
	int sw = getCaliValue();
	int sh = getCaliValue();
	surface_t *src, *dst;
	
	DEBUG_COMMAND("Gpx.CopyAMap %d,%d,%d,%d,%d,%d,%d,%d:\n", da, dx, dy, sa, sx, sy, sw, sh);
	
	ags_sync();
	src = sf_get(sa);
	dst = sf_get(da);
	gr_copy_alpha_map(dst, dx, dy, src, sx, sy, sw, sh);
}

void Blend() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	int p8 = getCaliValue();
	int p9 = getCaliValue();
	
	DEBUG_COMMAND_YET("Gpx.Blend %d,%d,%d,%d,%d,%d,%d,%d,%d:\n", p1, p2, p3, p4, p5, p6, p7, p8, p9);
}

void BlendSrcBright() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	int p8 = getCaliValue();
	int p9 = getCaliValue();
	int p10 = getCaliValue();
	
	DEBUG_COMMAND_YET("Gpx.BlendSrcBright %d,%d,%d,%d,%d,%d,%d,%d,%d,%d:\n", p1, p2, p3, p4, p5, p6, p7, p8, p9, p10);
}

void BlendAddSatur() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	int p8 = getCaliValue();
	
	DEBUG_COMMAND_YET("Gpx.BlendAddStatur %d,%d,%d,%d,%d,%d,%d,%d:\n", p1, p2, p3, p4, p5, p6, p7, p8);
}

void BlendAMap() {
	/*
	  Gpx.BlendAMap(): ž������ alpha map �򻲾Ȥ��� �����ΰ�� alpha blend
	  
	   ds: ž���� surface �ֹ�
	   dx: ž���� x ��ɸ
	   dy: ž���� y ��ɸ
	   ss: ž���� surface �ֹ�
	   sx: ž���� x ��ɸ
	   sy: ž���� y ��ɸ
	   sw: ž���� ��
	   sh: ž���� �⤵
	*/
	int ds = getCaliValue();
	int dx = getCaliValue();
	int dy = getCaliValue();
	int ss = getCaliValue();
	int sx = getCaliValue();
	int sy = getCaliValue();
	int sw = getCaliValue();
	int sh = getCaliValue();
	surface_t *src, *dst;
	
	DEBUG_COMMAND("Gpx.BlendAMap %d,%d,%d,%d,%d,%d,%d,%d:\n", ds, dx, dy, ss, sx, sy, sw, sh);
	
	ags_sync();
	src = sf_get(ss);
	dst = sf_get(ds);
	gr_blend_alpha_map(dst, dx, dy, src, sx, sy, sw, sh);
}

void BlendAMapSrcOnly() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	int p8 = getCaliValue();
	
	DEBUG_COMMAND_YET("Gpx.BlendAMapSrcOnly %d,%d,%d,%d,%d,%d,%d,%d:\n", p1, p2, p3, p4, p5, p6, p7, p8);
}

void BlendAMapColor() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	int p8 = getCaliValue();
	int p9 = getCaliValue();
	int p10 = getCaliValue();
	int p11 = getCaliValue();
	
	DEBUG_COMMAND_YET("Gpx.BlendAMapColor %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d:\n", p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11);
}

void BlendAMapColorAlpha() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	int p8 = getCaliValue();
	int p9 = getCaliValue();
	int p10 = getCaliValue();
	int p11 = getCaliValue();
	int p12 = getCaliValue();
	
	DEBUG_COMMAND_YET("Gpx.BlendAMapColorAlpha %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d:\n", p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12);
}

void BlendAMapAlpha() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	int p8 = getCaliValue();
	int p9 = getCaliValue();
	
	DEBUG_COMMAND_YET("Gpx.BlendAMapAlpha %d,%d,%d,%d,%d,%d,%d,%d,%d:\n", p1, p2, p3, p4, p5, p6, p7, p8, p9);
}

void BlendAMapBright() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	int p8 = getCaliValue();
	int p9 = getCaliValue();
	
	DEBUG_COMMAND_YET("Gpx.BlendAMapBright %d,%d,%d,%d,%d,%d,%d,%d,%d:\n", p1, p2, p3, p4, p5, p6, p7, p8, p9);
}

void BlendAMapAlphaSrcBright() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	int p8 = getCaliValue();
	int p9 = getCaliValue();
	int p10 = getCaliValue();
	
	DEBUG_COMMAND_YET("Gpx.BlendAMapBright %d,%d,%d,%d,%d,%d,%d,%d,%d,%d:\n", p1, p2, p3, p4, p5, p6, p7, p8, p9, p10);
}

void BlendUseAMapColor() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	int p8 = getCaliValue();
	int p9 = getCaliValue();
	int p10 = getCaliValue();
	int p11 = getCaliValue();
	int p12 = getCaliValue();
	
	DEBUG_COMMAND_YET("Gpx.BlendUseAMapColor %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d:\n", p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12);
}

void BlendScreen() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	int p8 = getCaliValue();
	
	DEBUG_COMMAND_YET("Gpx.BlendScreen %d,%d,%d,%d,%d,%d,%d,%d:\n", p1, p2, p3, p4, p5, p6, p7, p8);
}

void BlendMultiply() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	int p8 = getCaliValue();
	
	DEBUG_COMMAND_YET("Gpx.BlendMultiply %d,%d,%d,%d,%d,%d,%d,%d:\n", p1, p2, p3, p4, p5, p6, p7, p8);
}

void BlendScreenAlpha() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	int p8 = getCaliValue();
	int p9 = getCaliValue();
	
	DEBUG_COMMAND_YET("Gpx.BlendScreenAlpha %d,%d,%d,%d,%d,%d,%d,%d,%d:\n", p1, p2, p3, p4, p5, p6, p7, p8, p9);
}

void Fill() {
	/*
	  Gpx.Fill(): �����ΰ���ɤ�Ĥ֤�
	  
	   ds: �ɤ�Ĥ֤� surface �ֹ�
	   dx: �ΰ� x ��ɸ
	   dy: �ΰ� y ��ɸ
	   dw: �ΰ� ��
	   dh: �ΰ� �⤵
	   r : �ɤ�Ĥ֤��� ��
	   g : �ɤ�Ĥ֤��� ��
	   b : �ɤ�Ĥ֤��� ��
	*/
	int ds = getCaliValue();
	int dx = getCaliValue();
	int dy = getCaliValue();
	int dw = getCaliValue();
	int dh = getCaliValue();
	int r  = getCaliValue();
	int g  = getCaliValue();
	int b  = getCaliValue();
	surface_t *dst;
	
	DEBUG_COMMAND("Gpx.Fill %d,%d,%d,%d,%d,%d,%d,%d:\n", ds, dx, dy, dw, dh, r, g, b);
	
	ags_sync();
	dst = sf_get(ds);
	gr_fill(dst, dx, dy, dw, dh, r, g, b);
}

void FillAlphaColor() { /* not used ? */
	int ds = getCaliValue();
	int dx = getCaliValue();
	int dy = getCaliValue();
	int dw = getCaliValue();
	int dh = getCaliValue();
	int r = getCaliValue();
	int g = getCaliValue();
	int b = getCaliValue();
	int lv = getCaliValue();
	surface_t *dst;
	
	DEBUG_COMMAND_YET("Gpx.FillAlphaColor %d,%d,%d,%d,%d,%d,%d,%d,%d:\n", ds, dx, dy, dw, dh, r, g, b, lv);

	ags_sync();
	dst = sf_get(ds);
	gr_fill_alpha_color(dst, dx, dy, dw, dh, r, g, b, lv);
}

void FillAMap() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	
	DEBUG_COMMAND_YET("Gpx.FillAMap %d,%d,%d,%d,%d,%d:\n", p1, p2, p3, p4, p5, p6);
}

void FillAMapOverBorder() {
	/*
	  Gpx.FillAMapOverBorder(): ����ΰ���� ���Ͱʾ�� alpha �ͤ����
	                            ��Τ����� alpha �ͤ��֤�������
	  
	   ds: �ɤ�Ĥ֤� surface �ֹ�
	   dx: �ΰ� x ��ɸ
	   dy: �ΰ� y ��ɸ
	   dw: �ΰ� ��
	   dh: �ΰ� �⤵
	   s : ����
	   d : ���ͤ�Ķ��������������
	*/
	int ds = getCaliValue();
	int dx = getCaliValue();
	int dy = getCaliValue();
	int dw = getCaliValue();
	int dh = getCaliValue();
	int s = getCaliValue();
	int d = getCaliValue();
	surface_t *dst;
	
	DEBUG_COMMAND("Gpx.BlendAMapOverBorder %d,%d,%d,%d,%d,%d,%d:\n", ds, dx, dy, dw, dh, s, d);

	dst = sf_get(ds);
	gr_fill_alpha_overborder(dst, dx, dy, dw, dh, s, d);
}

void FillAMapUnderBorder() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	
	DEBUG_COMMAND_YET("Gpx.BlendAMapUnderBorder %d,%d,%d,%d,%d,%d,%d:\n", p1, p2, p3, p4, p5, p6, p7);
}

void SaturDP_DPxSA() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	int p8 = getCaliValue();
	
	DEBUG_COMMAND_YET("Gpx.SaturDP_DPxSA %d,%d,%d,%d,%d,%d,%d,%d:\n", p1, p2, p3, p4, p5, p6, p7, p8);
}

void ScreenDA_DAxSA() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	int p8 = getCaliValue();
	
	DEBUG_COMMAND_YET("Gpx.ScreenDA_DAxSA %d,%d,%d,%d,%d,%d,%d,%d:\n", p1, p2, p3, p4, p5, p6, p7, p8);
}

void AddDA_DAxSA() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	int p8 = getCaliValue();
	
	DEBUG_COMMAND_YET("Gpx.AddDA_DAxSA %d,%d,%d,%d,%d,%d,%d,%d:\n", p1, p2, p3, p4, p5, p6, p7, p8);
}

void SpriteCopyAMap() {
	/*
	  Gpx.SpriteCopyAMap(): ����� alpha �Ͱʳ����ΰ�� alpha map ���ԡ�
	  
	   da: ž���� surface(alpha map) �ֹ�
	   dx: ž���� x ��ɸ
	   dy: ž���� y ��ɸ
	   ss: ž���� surface(alpha map) �ֹ�
	   sx: ž���� x ��ɸ
	   sy: ž���� y ��ɸ
	   sw: ž���� ��
	   sh: ž���� �⤵
	   cl: ž�����ʤ� alpha ��
	*/
	int da = getCaliValue();
	int dx = getCaliValue();
	int dy = getCaliValue();
	int sa = getCaliValue();
	int sx = getCaliValue();
	int sy = getCaliValue();
	int sw = getCaliValue();
	int sh = getCaliValue();
	int cl = getCaliValue();
	surface_t *src, *dst;
	
	DEBUG_COMMAND("Gpx.SpriteCopyAMap %d,%d,%d,%d,%d,%d,%d,%d,%d:\n", da, dx, dy, sa, sx, sy, sw, sh, cl);
	
	ags_sync();
	src = sf_get(sa);
	dst = sf_get(da);
	gr_copy_alpha_map_sprite(dst, dx, dy, src, sx, sy, sw, sh, cl);
	
}

void BrightDestOnly() {
	/*
	  Gpx.BrightDestOnly(): �����ΰ�����뤵������
	  
	   ds: ���뤵�����ꤹ�� surface �ֹ�
	   dx: �ΰ� x ��ɸ
	   dy: �ΰ� y ��ɸ
	   dw: �ΰ� ��
	   dh: �ΰ� �⤵
	   r : �������뤵 (255: ���뤵 100%, pixel�ͤ��Τ��)
	*/
	int ds = getCaliValue();
	int dx = getCaliValue();
	int dy = getCaliValue();
	int dw = getCaliValue();
	int dh = getCaliValue();
	int r  = getCaliValue();
	surface_t *dst;
	
	DEBUG_COMMAND("Gpx.BrightDestOnly %d,%d,%d,%d,%d,%d:\n", ds, dx, dy, dw, dh, r);
	
	ags_sync();
	dst = sf_get(ds);
	
	gr_bright_dst_only(dst, dx, dy, dw, dh, r);
}

void CopyTextureWrap() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	int p8 = getCaliValue();
	int p9 = getCaliValue();
	int p10 = getCaliValue();
	int p11 = getCaliValue();
	int p12 = getCaliValue();
	
	DEBUG_COMMAND_YET("Gpx.CopyTextureWrap %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d:\n", p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12);
}

void CopyTextureWrapAlpha() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	int p8 = getCaliValue();
	int p9 = getCaliValue();
	int p10 = getCaliValue();
	int p11 = getCaliValue();
	int p12 = getCaliValue();
	int p13 = getCaliValue();
	
	DEBUG_COMMAND_YET("Gpx.CopyTextureWrapAlpha %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d:\n", p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13);
}

void CopyStretch() {
	/*
	  Gpx.CopyStretch(): ���硦�̾�
	  
	   ds: ž���� surface �ֹ�
	   dx: ž���� x ��ɸ
	   dy: ž���� y ��ɸ
	   dw: ž���� ��
	   dh: ž���� �⤵
	   ss: ž���� surface �ֹ�
	   sx: ž���� x ��ɸ
	   sy: ž���� y ��ɸ
	   sw: ž���� ��
	   sh: ž���� �⤵
	*/
	int ds = getCaliValue();
	int dx = getCaliValue();
	int dy = getCaliValue();
	int dw = getCaliValue();
	int dh = getCaliValue();
	int ss = getCaliValue();
	int sx = getCaliValue();
	int sy = getCaliValue();
	int sw = getCaliValue();
	int sh = getCaliValue();
	surface_t *src, *dst;

	DEBUG_COMMAND_YET("Gpx.CopyStretch %d,%d,%d,%d,%d,%d,%d,%d,%d,%d:\n", ds, dx, dy, dw, dh, ss, sx, sy, sw, sh);
	
	src = sf_get(ss);
	dst = sf_get(ds);
	gr_copy_stretch(dst, dx, dy, dw, dh, src, sx, sy, sw, sh);
}

void CopyStretchBlend() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	int p8 = getCaliValue();
	int p9 = getCaliValue();
	int p10 = getCaliValue();
	int p11 = getCaliValue();
	
	DEBUG_COMMAND_YET("Gpx.CopyStretchBlend %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d:\n", p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11);
}

void CopyStretchBlendAMap() {
	/*
	  Gpx.CopyStretchBlendAMap(): ���硦�̾����ʤ��� alpha blend
	  
	   ds: ž���� surface �ֹ�
	   dx: ž���� x ��ɸ
	   dy: ž���� y ��ɸ
	   dw: ž���� ��
	   dh: ž���� �⤵
	   ss: ž���� surface �ֹ�
	   sx: ž���� x ��ɸ
	   sy: ž���� y ��ɸ
	   sw: ž���� ��
	   sh: ž���� �⤵
	*/
	int ds = getCaliValue();
	int dx = getCaliValue();
	int dy = getCaliValue();
	int dw = getCaliValue();
	int dh = getCaliValue();
	int ss = getCaliValue();
	int sx = getCaliValue();
	int sy = getCaliValue();
	int sw = getCaliValue();
	int sh = getCaliValue();
	surface_t *src, *dst;
	
	DEBUG_COMMAND("Gpx.CopyStretchBlendAMap %d,%d,%d,%d,%d,%d,%d,%d,%d,%d:\n", ds, dx, dy, dw, dh, ss, sx, sy, sw, sh);

	ags_sync();
	src = sf_get(ss);
	dst = sf_get(ds);
	gr_copy_stretch_blend_alpha_map(dst, dx, dy, dw, dh, src, sx, sy, sw, sh);
}

void StretchBlendScreen2x2() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	int p8 = getCaliValue();
	
	DEBUG_COMMAND_YET("Gpx.StretchBlendScreen2x2 %d,%d,%d,%d,%d,%d,%d,%d:\n", p1, p2, p3, p4, p5, p6, p7, p8);
}


void StretchBlendScreen2x2WDS() {
	/*
	  Gpx.StretchBlenfScreen2x2WDS(): ����� surface ��Ĳ����ܤ˳��礷�Ĥ�
                                         alpha blend (˰�²û�)����
	  
	   ds: ž���� surface �ֹ�
	   dx: ž���� x ��ɸ
	   dy: ž���� y ��ɸ
	   ss1: ž���� surface �ֹ� (���縵 surface) ss1������������
	   sx1: ž���� x ��ɸ
	   sy1: ž���� y ��ɸ
	   ss2: ž���� surface �ֹ� (�Ť���� surface)
	   sx2: ž���� x ��ɸ
	   sy2: ž���� y ��ɸ
	   sw: ž���� �� (sw*2, sh*2)���礭���˳���
	   sh: ž���� �⤵
	*/
	int ds  = getCaliValue();
	int dx  = getCaliValue();
	int dy  = getCaliValue();
	int ss1 = getCaliValue();
	int sx1 = getCaliValue();
	int sy1 = getCaliValue();
	int ss2 = getCaliValue();
	int sx2 = getCaliValue();
	int sy2 = getCaliValue();
	int sw  = getCaliValue();
	int sh  = getCaliValue();
	surface_t *src1, *src2, *dst;
	
	DEBUG_COMMAND("Gpx.StretchBlendScreen2x2WDS %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d:\n", ds, dx, dy, ss1, sx1, sy1, ss2, sx2, sy2, sw, sh);
	
	ags_sync();
	src1 = sf_get(ss1);
	src2 = sf_get(ss2);
	dst  = sf_get(ds);
	gr_blend_alpha_wds_stretch2x2(src1, sx1, sy1, src2, sx2, sy2, sw, sh, dst, dx, dy);
}

void BlendScreenWDS() {
	/*
	  Gpx.BlendScreenWDS(): ˰�²û� alpha blend
	  
	   ds : ž���� surface �ֹ�
	   dx : ž���� x ��ɸ
	   dy : ž���� y ��ɸ
	   ss1: ž���� surface �ֹ� (�Ť͸��� surface)
	   sx1: ž���� x ��ɸ
	   sy1: ž���� y ��ɸ
	   ss2: ž���� surface �ֹ� (�Ť���� surface)
	   sx2: ž���� x ��ɸ
	   sy2: ž���� y ��ɸ
	   sw : ž���� ��
	   sh : ž���� �⤵
	*/
	int ds  = getCaliValue();
	int dx  = getCaliValue();
	int dy  = getCaliValue();
	int ss1 = getCaliValue();
	int sx1 = getCaliValue();
	int sy1 = getCaliValue();
	int ss2 = getCaliValue();
	int sx2 = getCaliValue();
	int sy2 = getCaliValue();
	int sw  = getCaliValue();
	int sh  = getCaliValue();
	surface_t *src1, *src2, *dst;
	
	DEBUG_COMMAND("Gpx.BlendScreenWDS %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d:\n", ds, dx, dy, ss1, sx1, sy1, ss2, sx2, sy2, sw, sh);
	
	ags_sync();
	src1 = sf_get(ss1);
	src2 = sf_get(ss2);
	dst  = sf_get(ds);
	gr_blend_alpha_wds(src1, sx1, sy1, src2, sx2, sy2, sw, sh, dst, dx, dy);
}

void EffectCopy() {
	/*
	  Gpx.EffectCopy(): ���̤Ĥ��ΰ襳�ԡ�
	  
	   no: �����ֹ�
	   dx: ž���� x ��ɸ
	   dy: ž���� y ��ɸ
	   ss1: ž���� surface �ֹ� (�Ť͸��� surface)
	   sx1: ž���� x ��ɸ
	   sy1: ž���� y ��ɸ
	   ss2: ž���� surface �ֹ� (�Ť���� surface)
	   sx2: ž���� x ��ɸ
	   sy2: ž���� y ��ɸ
	   sw: ž���� ��
	   sh: ž���� �⤵
	   time: �¹Ի���(ms) (0�ξ��Ϥ��줾�Τ������Υǥե������)
	   var: �������Ϥ����ä����ʤ�����(?????)


	  effect no:
	    1: CE31 (cross fade)
	    2: CE29 (fade out)
	    3: CE27 (fade in)
	    4: CE30 (white out)
	    5: CE28 (white in)
	    7: CE11 (���������)
	    11: CE53 (�����ܤ���)
	    12: CE35 (��->�������ե�����)
	    13: CE36 (��->�奯���ե�����)
	*/
	int no   = getCaliValue();
	int dx   = getCaliValue();
	int dy   = getCaliValue();
	int ss1  = getCaliValue();
	int sx1  = getCaliValue();
	int sy1  = getCaliValue();
	int ss2  = getCaliValue();
	int sx2  = getCaliValue();
	int sy2  = getCaliValue();
	int sw   = getCaliValue();
	int sh   = getCaliValue();
	int time = getCaliValue();
	int *var = getCaliVariable();
	surface_t *dib, *dst, *src;
	
	switch(no) {
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 7:
	case 11:
	case 12:
	case 13:
		DEBUG_COMMAND("Gpx.EffectCopy %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%p:\n", no, dx, dy, ss1, sx1, sy1, ss2, sx2, sy2, sw, sh, time, var);
		break;
	default:
		DEBUG_COMMAND_YET("Gpx.EffectCopy %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%p:\n", no, dx, dy, ss1, sx1, sy1, ss2, sx2, sy2, sw, sh, time, var);
	}
	ags_sync();
	dib = sf_get(0);
	dst = sf_get(ss1);
	src = sf_get(ss2);
	gpx_effect(no, dib, dx, dy, dst, sx1, sy1, src, sx2, sy2, sw, sh, time, var);
	
}

void SetClickCancelFlag() { /* not used ? */
	int p1 = getCaliValue();
	
	DEBUG_COMMAND_YET("Gpx.SetClikCancelFlag %d:\n", p1);
}
