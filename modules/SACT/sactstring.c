/*
 * sactstring.c: SACT��ʸ��������Ϣ
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
/* $Id: sactstring.c,v 1.3 2003/08/02 13:10:32 chikama Exp $ */

#include "config.h"
#include <stdio.h>
#include <glib.h>
#include <string.h>
#include "portab.h"
#include "system.h"
#include "variable.h"
#include "sact.h"

#define DEFSTACKSIZE 100
static char **stack;   // stack����
static int idx;        // stack pointer
static int idxmax;     // stack pointer�κ���

/**
 * ʸ�����ѿ������å��ν����
 */
int sstr_init() {
	stack = g_new(char *, DEFSTACKSIZE);
	idx = 0;
	idxmax = DEFSTACKSIZE;
	return OK;
}

/**
 * ʸ�����ѿ������å���ʸ������Ѥ�
 * @param strno: ���ʥꥪ��Ǥ�ʸ�����ѿ��ֹ�
 */
int sstr_push(int strno) {
	if (idx >= idxmax) {
		stack = g_renew(char *, stack, idx*2);
		idxmax = idx*2;
	}
	
	stack[idx++] = g_strdup(v_str(strno -1));
	
	return OK;
}

/**
 * ʸ�����ѿ������å�����ʸ�������Ф�
 * @param strno: �����å������ᤷ��ʸ������Ǽ����ʸ�����ѿ��ֹ�
 */
int sstr_pop(int strno) {
	if (idx == 0) return NG;
	
	v_strcpy(strno -1, stack[--idx]);
	g_free(stack[idx]);
	
	return OK;
}

/**
 * ʸ������֤�����
 * @param sstrno: �Ѵ���ʸ�����ѿ��ֹ�
 * @param dstrno: �Ѵ���ʸ�����ѿ��ֹ�
 */
int sstr_regist_replace(int sstrno, int dstrno) {
	strexchange_t *ex;
	
	if (sstrno == dstrno) return NG;
	
	ex = g_new(strexchange_t, 1);
	ex->src = strdup(v_str(sstrno -1));
	ex->dst = strdup(v_str(dstrno -1));
	sact.strreplace = g_slist_append(sact.strreplace, ex);
	return OK;
}

/**
 * ���� -> ʸ����
 */
int sstr_num2str(int strno, int fig, int nzeropad, int num) {
	char s[256], ss[256];
	
	if (nzeropad) {
		char *sss = "%%0%dd";
		sprintf(ss, sss, fig);
	} else {
		char *sss = "%%%dd";
		sprintf(ss, sss, fig);
	}
	
	sprintf(s, ss, num);
	v_strcpy(strno -1, s);
	
	return OK;
}
