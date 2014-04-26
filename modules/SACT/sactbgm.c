/*
 * sactbgm.c: SACT Music ��Ϣ
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
/* $Id: sactbgm.c,v 1.4 2003/08/30 21:29:16 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <glib.h>

#include "portab.h"
#include "system.h"
#include "ald_manager.h"
#include "music_client.h"

// ������ֹ�β��ڤ�¸�ߤ��뤫�����å�
int smus_check(int no) {
	dridata *dfile = ald_getdata(DRIFILE_BGM, no -1);
	int st = 0;
	
	if (dfile == NULL) {
		st = 0;
	} else {
		st = 1;
		ald_freedata(dfile);
	}
	
	return st;
}

// ������ֹ�β��ڤ�Ĺ�������
int smus_getlength(int no) {
	return mus_bgm_getlength(no);
}

// ������ֹ�β��ڤκ������֤����
int smus_getpos(int no) {
	return mus_bgm_getpos(no);
}

// ������ֹ�β��ڤκ�������
int smus_play(int no, int time, int vol) {
	mus_bgm_play(no, time, vol);
	return OK;
}

// ������ֹ�β��ڤκ������
int smus_stop(int no, int fadetime) {
	mus_bgm_stop(no, fadetime);
	return OK;
}

// ������ֹ�β��ڤΥܥ�塼��ե�����
int smus_fade(int no, int time, int vol) {
	mus_bgm_fade(no, time, vol);
	return OK;
}

// ������ֹ�β��ڤ���λ����Τ��Ԥ�
int smus_wait(int no, int timeout) {
	mus_bgm_wait(no, timeout);
	return OK;
}

// ������ֹ�β��ڤ�����ΰ��֤ޤǺ��������Τ��Ԥ�
int smus_waitpos(int no, int index) {
	mus_bgm_waitpos(no, index);
	return OK;
}

// ���Ƥβ��ڤκ��������
int smus_stopall(int time) {
	mus_bgm_stopall(time);
	return OK;
}

