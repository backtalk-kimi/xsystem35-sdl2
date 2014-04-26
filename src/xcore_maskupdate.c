/*
 * xcore_maskupdate.c  X11 mask �դ� update
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
/* $Id: xcore_maskupdate.c,v 1.4 2002/05/02 17:21:32 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "portab.h"
#include "xcore_private.h"

static Pixmap maskpix;  /* clipmask �� Bitmap */
static Pixmap clippix;  /* copy �� Pixmap */
static Pixmap savepix;  /* copy �� Pixmap */
static Pixmap doublepix;/* double buffer �� Pixmap */
static GC     maskgc;   /* clipmask Bitmap �� GC */

static void draw_5star(double mul, double rad, int off_x, int off_y);
static void draw_6star(double mul, double rad, int off_x, int off_y);
static void draw_windwheel_90(double rad, int r, int off_x, int off_y);
static void draw_windwheel_180(double rad, int r, int off_x, int off_y);
static void draw_windwheel_360(double rad, int r, int off_x, int off_y);

/*
 * ������������
 *  mul  : ��Ψ (1 �� 100 * 100) �������������
 *  rad  : ��ž����(radian)
 *  off_x: �濴��ɸ�� x ���� offset
 *  off_y: �濴��ɸ�� y ���� offset
 */
static void draw_5star(double mul, double rad, int off_x, int off_y) {
	static XPoint pt[] = {
		{  0, -50},
		{-29,  40},
		{ 47, -15},
		{-47, -15},
		{ 29,  40}
	};
	XPoint p[5];
	int i;
	
	/*
	 *   x'   cos(X) -sin(X)   mul  0       x
	 *     =                 *           * 
	 *   y'   sin(X)  cos(X)   0    mul     y
	 */
	for (i = 0; i < 5; i++) {
		p[i].x = mul * (pt[i].x * cos(rad) - pt[i].y * sin(rad)) + off_x;
		p[i].y = mul * (pt[i].x * sin(rad) + pt[i].y * cos(rad)) + off_y;
	}
	
	XFillPolygon(x11_display, maskpix, maskgc, p, 5, Complex, CoordModeOrigin);
}

/*
 * ϻ����������
 *  mul  : ��Ψ (1 �� 100 * 100) �������������
 *  rad  : ��ž����(radian)
 *  off_x: �濴��ɸ�� x ���� offset
 *  off_y: �濴��ɸ�� y ���� offset
 */
static void draw_6star(double mul, double rad, int off_x, int off_y) {
	static XPoint pt1[] = {
		{  0, -50},
		{-43,  25},
		{ 43,  25}
	};
	static XPoint pt2[] = { 
		{-43, -25},
		{  0,  50},
		{ 43, -25}
	};
	XPoint p[3];
	int i;
	
	for (i = 0; i < 3; i++) {
		p[i].x = mul * (pt1[i].x * cos(rad) - pt1[i].y * sin(rad)) + off_x;
		p[i].y = mul * (pt1[i].x * sin(rad) + pt1[i].y * cos(rad)) + off_y;
	}

	XFillPolygon(x11_display, maskpix, maskgc, p, 3, Convex, CoordModeOrigin);
	
	for (i = 0; i < 3; i++) {
		p[i].x = mul * (pt2[i].x * cos(rad) - pt2[i].y * sin(rad)) + off_x;
		p[i].y = mul * (pt2[i].x * sin(rad) + pt2[i].y * cos(rad)) + off_y;
	}
	
	XFillPolygon(x11_display, maskpix, maskgc, p, 3, Convex, CoordModeOrigin);
}

/*
 * �𷿤��濴�� (�Ƥ��ȡ�)
 *   0.2 radian
 */
#define WHEELDELTA 0.2

/*
 * ���֣�����
 *  rad  : ��ž�� (radian) 0 - pi/2
 *  r    : Ⱦ��
 *  off_x: �濴��ɸ�� x ���� offset
 *  off_y: �濴��ɸ�� y ���� offset
 */
static void draw_windwheel_90(double rad, int r, int off_x, int off_y) {
	XPoint p[3];
	
	p[0].x = 0 + off_x;
	p[0].y = 0 + off_y;

	p[1].x =   r * sin(rad) + off_x;
	p[1].y =  -r * cos(rad) + off_y;

	p[2].x =   r * sin(rad + WHEELDELTA) + off_x;
	p[2].y =  -r * cos(rad + WHEELDELTA) + off_y;
	
	XFillPolygon(x11_display, maskpix, maskgc, p, 3, Convex, CoordModeOrigin);

	p[1].x =   r * cos(rad) + off_x;
	p[1].y =   r * sin(rad) + off_y;

	p[2].x =   r * cos(rad + WHEELDELTA) + off_x;
	p[2].y =   r * sin(rad + WHEELDELTA) + off_y;

	XFillPolygon(x11_display, maskpix, maskgc, p, 3, Convex, CoordModeOrigin);
	
	p[1].x = -r * sin(rad) + off_x;
	p[1].y =  r * cos(rad) + off_y;

	p[2].x = -r * sin(rad + WHEELDELTA) + off_x;
	p[2].y =  r * cos(rad + WHEELDELTA) + off_y;

	XFillPolygon(x11_display, maskpix, maskgc, p, 3, Convex, CoordModeOrigin);
	p[1].x = -r * cos(rad) + off_x;
	p[1].y = -r * sin(rad) + off_y;

	p[2].x = -r * cos(rad + WHEELDELTA) + off_x;
	p[2].y = -r * sin(rad + WHEELDELTA) + off_y;

	XFillPolygon(x11_display, maskpix, maskgc, p, 3, Convex, CoordModeOrigin);
}

/*
 * ���֣�������
 *  rad  : ��ž�� (radian) 0 - pi
 *  r    : Ⱦ��
 *  off_x: �濴��ɸ�� x ���� offset
 *  off_y: �濴��ɸ�� y ���� offset
 */
static void draw_windwheel_180(double rad, int r, int off_x, int off_y) {
	XPoint p[3];
	
	p[0].x = 0 + off_x;
	p[0].y = 0 + off_y;

	
	p[1].x = -r * cos(rad) + off_x;
	p[1].y = -r * sin(rad) + off_y;

	p[2].x = -r * cos(rad + WHEELDELTA) + off_x;
	p[2].y = -r * sin(rad + WHEELDELTA) + off_y;
	
	XFillPolygon(x11_display, maskpix, maskgc, p, 3, Convex, CoordModeOrigin);

	p[1].x =  r * cos(rad) + off_x;
	p[1].y =  r * sin(rad) + off_y;

	p[2].x =  r * cos(rad + WHEELDELTA) + off_x;
	p[2].y =  r * sin(rad + WHEELDELTA) + off_y;

	XFillPolygon(x11_display, maskpix, maskgc, p, 3, Convex, CoordModeOrigin);
}

/*
 * ���֣�������
 *  rad  : ��ž�� (radian) 0 - 2pi
 *  r    : Ⱦ��
 *  off_x: �濴��ɸ�� x ���� offset
 *  off_y: �濴��ɸ�� y ���� offset
 */
static void draw_windwheel_360(double rad, int r, int off_x, int off_y) {
	XPoint p[3];
	
	p[0].x = 0 + off_x;
	p[0].y = 0 + off_y;

	p[1].x =  -r * cos(rad) + off_x;
	p[1].y =  -r * sin(rad) + off_y;

	p[2].x =  -r * cos(rad + WHEELDELTA) + off_x;
	p[2].y =  -r * sin(rad + WHEELDELTA) + off_y;
	
	XFillPolygon(x11_display, maskpix, maskgc, p, 3, Convex, CoordModeOrigin);

}

/*
 * �ޥ����դ��ΰ蹹��
 *   sx: ���ԡ��� x ��ɸ
 *   sy: ���ԡ��� y ��ɸ
 *   w : ���ԡ��� width
 *   h : ���ԡ��� height
 *   dx: ���ԡ��� x ��ɸ
 *   dy: ���ԡ��� y ��ɸ
 *   func: �ޥ�������
 *      44: ������(��->��)
 *      45: ������(��->��)
 *      46: ϻ����(��->��)
 *      47: ϻ����(��->��)
 *      50: ���֣�����
 *      51: ���֣�������
 *      52: ���֣�������
 *   step: �ƤӽФ��ֹ� 0 - 256
 */
void Xcore_maskupdate(int sx, int sy, int w, int h, int dx, int dy, int func, int step) {
	GC gc;

	dx -= view_x;
	dy -= view_y;

	if (step == 0) {
		/* �ǽ�ˤ�����Ƚ��� */
		XSync(x11_display, False);
		/* ���ԡ����� DIB ���� pixmap �˻��ä���� */
		clippix = x11_clip_from_DIB(sx, sy, w, h);

		/* clip mask �� Pixmap ������ */
		maskpix = XCreatePixmap(x11_display, x11_window, w, h, 1);
		maskgc = XCreateGC(x11_display, maskpix, None, NULL);
		
		/* clipmask �� Pixmap �ν���� */
		XSetForeground(x11_display, maskgc, 0);
		XFillRectangle(x11_display, maskpix, maskgc, 0, 0, w, h);
		
		/* �����򺹤���Ȥ����ɤ�Ĥ֤��Υ롼������� */
		XSetFillRule(x11_display, maskgc, WindingRule);	

		/* ������� Window ���ΰ����¸ */
		savepix = XCreatePixmap(x11_display, x11_window, w, h, WIN_DEPTH);
		XCopyArea(x11_display, x11_window, savepix, x11_gc_pix,
			  winoffset_x + dx, winoffset_y + dy, w, h, 0, 0);
		
		/* ���֥�Хåե��Ѥ� Pixmap ����� */
		doublepix = XCreatePixmap(x11_display, x11_window, w, h, WIN_DEPTH);
		XCopyArea(x11_display, x11_window, doublepix, x11_gc_pix,
			  winoffset_x + dx, winoffset_y + dy, w, h, 0, 0);
		
		XSync(x11_display, False);
		return;
	} else if (step == 256) {
		/* �Ǹ�˸���� */
#if 0
		XCopyArea(x11_display, clippix, x11_window, x11_gc_pix,
			  0, 0, w, h, winoffset_x + dx, winoffset_y + dy);
#endif
#if 0
		XCopyArea(x11_display, clippix, x11_pixmap, x11_gc_pix,
			  0, 0, w, h, winoffset_x + dx, winoffset_y + dy);
		ags_updateFull();
#endif
		ags_copyArea(sx, sy, w, h, dx + view_x, dy + view_y);
		ags_updateArea(dx + view_x, dy + view_y, w, h);
		XFreePixmap(x11_display, maskpix);
		
		XFreePixmap(x11_display, clippix);
		XFreePixmap(x11_display, savepix);
		XFreePixmap(x11_display, doublepix);
		XFreeGC(x11_display, maskgc);
		
		return;
	}
	
	/* clipmask �Ѥ� GC ������ (����������ʤ��Ƥ��ɤ����⡩) */
	gc = XCreateGC(x11_display, clippix, None, NULL);
	XSetClipMask(x11_display, gc, maskpix);
	XSetClipOrigin(x11_display, gc, 0, 0);
	
	switch(func) {
	case 44:	
	case 46:
		/* �⤫�鳰�� */
		XSetForeground(x11_display, maskgc, 0);
		XFillRectangle(x11_display, maskpix, maskgc, 0, 0, w, h);
		XSetForeground(x11_display, maskgc, 1);
		break;
	case 45:
	case 47:
		/* ��������� */
		XSetForeground(x11_display, maskgc, 1);
		XFillRectangle(x11_display, maskpix, maskgc, 0, 0, w, h);
		XSetForeground(x11_display, maskgc, 0);
		break;
	default:
		XSetForeground(x11_display, maskgc, 1);
	}

	/* mask ������ */
	switch(func) {
	case 44:
		draw_5star(1.5 * (max(w,h) * step) / (256 * 50.0),
			   1.0 * M_PI * step / 256, w/2, h/2);
		break;
		
	case 45:
		draw_5star(1.5 * (max(w,h) * (256-step)) / (256 * 50.0),
			   1.0 * M_PI * step / 256, w/2, h/2);
		break;
		
	case 46:
		draw_6star((max(w,h) * step) / (256 * 50.0),
			   1.0 * M_PI * step / 256, w/2, h/2);
		break;
		
	case 47:
		draw_6star((max(w,h) * (256-step)) / (256 * 50.0),
			   1.0 * M_PI * step / 256, w/2, h/2);
		break;
		
	case 50:
		draw_windwheel_90(0.5 * M_PI * step / 256, sqrt(w*w+h*h)/2, 
			       w/2, h/2);
		break;
	case 51:
		draw_windwheel_180(1.0 * M_PI * step / 256, sqrt(w*w+h*h)/2, 
				   w/2, h/2);
		break;
	case 52:
		draw_windwheel_360(2.0 * M_PI * step / 256, sqrt(w*w+h*h)/2, 
				   w/2, h/2);
		break;
	}
	
	/* double buffer �� ��¸���� window �����Ƥ򥳥ԡ� */
	XCopyArea(x11_display, savepix, doublepix, x11_gc_win,
		  0, 0, w, h, 0, 0);
	
	/* ���ԡ��������Ƥ� mask�դ��� double buffer �إ��ԡ� */
	XCopyArea(x11_display, clippix, doublepix, gc,
		  0, 0, w, h, 0, 0);
	
	/* double buffer ���� window �إ��ԡ� */
	XCopyArea(x11_display, doublepix, x11_window, x11_gc_win,
		  0, 0, w, h, winoffset_x + dx, winoffset_y + dy);
	
	/* Ʊ�� */
	XSync(x11_display, False);
	
	/* clipmask �� gc ���˴� */
	XFreeGC(x11_display, gc);
}
