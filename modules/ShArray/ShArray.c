/*
 * ShArray.c  �Ƽ�����黻 module
 *
 *    ������ˤ��ˤ祢�ꥹ
 *    �簭��
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
/* $Id: ShArray.c,v 1.4 2002/08/18 09:35:29 chikama Exp $ */

#include "config.h"

#include <stdio.h>

#include "portab.h"
#include "system.h"
#include "xsystem35.h"
#include "nact.h"

void GetAtArray(void) { /* 0 */
	/*
	  ���󤫤�黻���ʤ����ͤ���Ф�
	  
	  vAry: ����
	  cnt : �Ŀ�
	  type: �黻�μ���
	  vResult: �黻��̤��֤��ѿ�
	*/
	int *vAry    = getCaliVariable();
	int cnt      = getCaliValue();
	int type     = getCaliValue();
	int *vResult = getCaliVariable();
	int i, j;

	DEBUG_COMMAND("ShArray.GetAtArray %p,%d,%d,%p:\n", vAry, cnt, type, vResult);
	
	j = *vAry; vAry++;
	for (i = 1; i < cnt; i++) {
		switch(type) {
		case 1:
			j += *vAry;
			break;
		case 2:
			j *= *vAry;
			break;
		case 3:
			j &= *vAry;
			break;
		case 4:
			j |= *vAry;
			break;
		case 5:
			j ^= *vAry;
			break;
		}
		vAry++;
	}
	
	if (j > 65535) {
		j = 65535;
	}
	
	*vResult = j;
}

void AddAtArray(void) { /* 1 */
	/*
	  ���󣱤����󣲤�­����65535 ��Ķ������ 65535 �ˡ�
	  
	  vAry1: ����
	  vAry2: ����
	  cnt  : �Ŀ�
	*/
	int *vAry1 = getCaliVariable();
	int *vAry2 = getCaliVariable();
	int cnt    = getCaliValue();
	int i;
	
	DEBUG_COMMAND("ShArray.AddAtArray %p,%p,%d:\n", vAry1, vAry2, cnt);
	
	for (i = 0; i < cnt; i++) {
		int result = (*vAry1) + (*vAry2);
		if (result > 65535) {
			*vAry1 = 65535;
		} else {
			*vAry1 = result;
		}
		vAry1++; vAry2++;
	}
}

void SubAtArray(void) { /* 2 */
	/*
	  ���󣱤������󣲤��������ˤʤä��飰�򤫤�
	  
 	  vAry1: ����
	  vAry2: ����
	  cnt  : �Ŀ�
	*/
	int *vAry1 = getCaliVariable();
	int *vAry2 = getCaliVariable();
	int cnt    = getCaliValue();
	int i;
	
	DEBUG_COMMAND("ShArray.SubAtArray %p,%p,%d:\n", vAry1, vAry2, cnt);
	
	for (i = 0; i < cnt; i++) {
		int result = (*vAry1) - (*vAry2);
		if (result < 0) {
			*vAry1 = 0;
		} else {
			*vAry1 = result;
		}
		vAry1++; vAry2++;
	}
}

void MulAtArray(void) { /* 3 */
	/*
	  ���󣱤����󣲤򤫤��ơ����󣱤˳�Ǽ��65535�ޤǡ�

 	  vAry1: ����
	  vAry2: ����
	  cnt  : �Ŀ�
	*/
	int *vAry1 = getCaliVariable();
	int *vAry2 = getCaliVariable();
	int cnt    = getCaliValue();
	int i;
	
	DEBUG_COMMAND("ShArray.MulAtArray %p,%p,%d:\n", vAry1, vAry2, cnt);
	
	for (i = 0; i < cnt; i++) {
		int result = (*vAry1) * (*vAry2);
		if (result > 65535) {
			*vAry1 = 65535;
		} else {
			*vAry1 = result;
		}
		vAry1++; vAry2++;
	}
}

void DivAtArray(void) { /* 4 */
	/*
	  ���󣱤����󣲤ǳ�äơ����󣱤˳�Ǽ��65535�ޤǡ�
	  
 	  vAry1: ����
	  vAry2: ����
	  cnt  : �Ŀ�
	*/
	int *vAry1 = getCaliVariable();
	int *vAry2 = getCaliVariable();
	int cnt    = getCaliValue();
	int i;
	
	DEBUG_COMMAND("ShArray.DivAtArray: %d,%d,%d:\n", vAry1, vAry2, cnt);
	
	for (i = 0; i < cnt; i++) {
		if (*vAry2 == 0) {
			*vAry1 = 0;
		} else {
			int result = (*vAry1) / (*vAry2);
			if (result > 65535) {
				*vAry1 = 65535;
			} else {
				*vAry1 = result;
			}
		}
		vAry1++; vAry2++;
	}
}

void MinAtArray(void) { /* 5 */
	/*
	  ���� vAry1 ����Ȥ����� vAry2 �ǲ��¤����ꤹ��

 	  vAry1: ����
	  vAry2: ����
	  cnt  : �Ŀ�
	*/
	int *vAry1 = getCaliVariable();
	int *vAry2 = getCaliVariable();
	int cnt    = getCaliValue();
	int i;
	
	DEBUG_COMMAND("ShArray.MinAtArray: %d,%d,%d:\n", vAry1, vAry2, cnt);
	
	for (i = 0; i < cnt; i++) {
		if (*vAry1 < *vAry2) {
			*vAry1 = *vAry2;
		}
		vAry1++; vAry2++;
	}
}

void MaxAtArray(void) { /* 6 */
	/*
	  ���� vAry1 ����Ȥ����� vAry2 �Ǿ�¤����ꤹ��
	  
 	  vAry1: ����
	  vAry2: ����
	  cnt  : �Ŀ�
	*/
	int *vAry1 = getCaliVariable();
	int *vAry2 = getCaliVariable();
	int cnt    = getCaliValue();
	int i;
	
	DEBUG_COMMAND("ShArray.MaxAtArray: %d,%d,%d:\n", vAry1, vAry2, cnt);
	
	for (i = 0; i < cnt; i++) {
		if (*vAry1 > *vAry2) {
			*vAry1 = *vAry2;
		}
		vAry1++; vAry2++;
	}
}

void AndNumArray(void) { /* 7 */
	/*
	  ����Υǡ����� val �� AND ��Ȥ�

	  vAry: ����
	  cnt : �Ŀ�
	  val : AND��Ȥ���
	*/
	int *vAry = getCaliVariable();
	int cnt   = getCaliValue();
	int val   = getCaliValue();
	int i;
	
	DEBUG_COMMAND("ShArray.AndNumArray: %p,%d,%d:\n", vAry, cnt, val);
	
	for (i = 0; i < cnt; i++) {
		(*vAry) &= val;
		vAry++;
	}
}

void OrNumArray(void) { /* 8 */
	/*
	  ����Υǡ����� val �� OR ��Ȥ�

	  vAry: ����
	  cnt : �Ŀ�
	  val : OR��Ȥ���
	*/
	int *vAry = getCaliVariable();
	int cnt   = getCaliValue();
	int val   = getCaliValue();
	int i;
	
	DEBUG_COMMAND_YET("ShArray.OrNumArray: %p,%d,%d:\n", vAry, cnt, val);
	
	for (i = 0; i < cnt; i++) {
		(*vAry) |= val;
		vAry++;
	}
}

void XorNumArray(void) { /* 9 */
	/*
	  ����Υǡ����� val �� XOR ��Ȥ�

	  vAry: ����
	  cnt : �Ŀ�
	  val : XOR��Ȥ���
	*/
	int *vAry = getCaliVariable();
	int cnt   = getCaliValue();
	int val   = getCaliValue();
	int i;
	
	DEBUG_COMMAND("ShArray.XorNumArray %p,%d,%d:\n", vAry, cnt, val);
	
	for (i = 0; i < cnt; i++) {
		(*vAry) ^= val;
		vAry++;
	}
}

void SetEquArray(void) { /* 10 */
	/*
	  ���� val ����������� ���� vResults �� 1 �򡢤����Ǥʤ����
	  0 ������
	  
	  vAry: ����
	  cnt : �Ŀ�
	  val : ��Ӥ�����
	  vResults  : ��̤��Ǽ��������
	*/
	int *vAry = getCaliVariable();
	int cnt   = getCaliValue();
	int val   = getCaliValue();
	int *vResults = getCaliVariable();
	int i;
	
	DEBUG_COMMAND("ShArray.SetEquArray %p,%d,%d,%p:\n", vAry, cnt, val, vResults);
	
	for (i = 0; i < cnt; i++) {
		*vResults = (*vAry == val) ? 1 : 0;
		vResults++; vAry++;
	}
}

void SetNotArray(void) { /* 11 */
	/*
	  ���� val ���������ʤ�������� vResults �� 1 �򡢤����Ǥʤ����
	  0 ������
	  
	  vAry: ����
	  cnt : �Ŀ�
	  val : ��Ӥ�����
	  vResults  : ��̤��Ǽ��������
	*/
	int *vAry = getCaliVariable();
	int cnt   = getCaliValue();
	int val   = getCaliValue();
	int *vResults = getCaliVariable();
	int i;
	
	DEBUG_COMMAND("ShArray.SetNotArray %p,%d,%d,%p:\n", vAry, cnt, val, vResults);

	for (i = 0; i < cnt; i++) {
		*vResults = (*vAry != val) ? 1 : 0;
		vResults++; vAry++;
	}
}

void SetLowArray(void) { /* 12 */
	/*
	  ����ǡ����� val ���⾮������� vResult �� 1 �򥻥å�
	  
	  vAry: ����
	  cnt : �Ŀ�
	  val : ����
	  vResult: ��̤��֤��ѿ�
	*/
	int *vAry = getCaliVariable();
	int cnt   = getCaliValue();
	int val   = getCaliValue();
	int *vResults = getCaliVariable();
	int i;
	
	DEBUG_COMMAND("ShArray.SetLowArray %p,%d,%d,%p:\n", vAry, cnt, val, vResults);
	
	for (i = 0; i < cnt; i++) {
		(*vResults) = ((*vAry < val) ? 1 : 0);
		vResults++; vAry++;
	}
}

void SetHighArray(void) { /* 13 */
	/*
	  ����ǡ����� val �����礭����� vResult �� 1 �򥻥å�
	  
	  vAry: ����
	  cnt : �Ŀ�
	  val : ����
	  vResults: ��̤��֤��ѿ�
	*/
	int *vAry = getCaliVariable();
	int cnt   = getCaliValue();
	int val   = getCaliValue();
	int *vResults = getCaliVariable();
	int i;
	
	DEBUG_COMMAND("ShArray.SetHighArray %p,%d,%d,%p:\n", vAry, cnt, val, vResults);
	
	for (i = 0; i < cnt; i++) {
		(*vResults) = ((*vAry > val) ? 1 : 0);
		vResults++; vAry++;
	}
}

void SetRangeArray(void) { /* 14 */
	/* 
	   ����ǡ����������ϰ�(min��max)�ˤ��뤫�����å�
	   
	   vAry: ����
	   cnt : �Ŀ�
	   min : �Ǿ���
	   max : ������
	   vResults: ��̤��֤��ѿ�
	     min < vAry < max �λ� vResults = 1;
             ����ʳ�              vResults = 0;
	 */
	int *vAry = getCaliVariable();
	int cnt   = getCaliValue();
	int min   = getCaliValue();
	int max   = getCaliValue();
	int *vResults = getCaliVariable();
	int i;
	
	DEBUG_COMMAND("ShArray.SetRangeArray %p,%d,%d,%d,%p:\n", vAry, cnt, min, max, vResults);
	
	for (i = 0; i < cnt; i++) {
		*vResults = ((*vAry > min) && (*vAry < max)) ? 1 : 0;
		vResults++; vAry++;
	}
}

void SetAndEquArray(void) { /* 15 */
	/*
	  ���� vAry �� mask �Ȥ� AND ��Ȥä� val �����������
	  ���� vResults �ˣ����������ʤ���� 0 ���

	   vAry: ����
	   mask: ����ˤ�����ޥ���
	   cnt : �Ŀ�
	   val : ��Ӥ�����
	   vResults  : ��̤�������������

	*/
	int *vAry = getCaliVariable();
	int mask   = getCaliValue();
	int cnt    = getCaliValue();
	int val    = getCaliValue();
	int *vResults = getCaliVariable();
	int i;
	
	DEBUG_COMMAND("ShArray.SetAndEquArray: %p,%d,%d,%d,%p:\n", vAry, mask, cnt, val, vResults);
	
	for (i = 0; i < cnt; i++) {
		*vResults = ((*vAry & mask) == val) ? 1 : 0;
		vResults++; vAry++;
	}
}

void AndEquArray(void) { /* 16 */
	/*
	  ���� vAry ��� val ��Ʊ���ʤ�С�vResult �� 1 �� AND ��
	  �㤦�ʤ�� 0 �򤫤���
	   
	   vAry: ����
	   cnt : �Ŀ�
	   val : ��Ӥ�����
	   vResults  : ��̤�������������
	*/
	int *vAry = getCaliVariable();
	int cnt   = getCaliValue();
	int val   = getCaliValue();
	int *vResults = getCaliVariable();
	int i;
	
	DEBUG_COMMAND("ShArray.AndEquArray %p,%d,%d,%p:\n", vAry, cnt, val, vResults);
	
	for (i = 0; i < cnt; i++) {
		*vResults &= ((*vAry == val) ? 1 : 0);
		vResults++; vAry++;
	}
}

void AndNotArray(void) { /* 17 */
	/*
	  ���� vAry ��� val ���������ʤ��ʤ�С�vResult �� 1 �� AND ��
	  �㤦���� 0 �򤫤���
	  
	   vAry: ����
	   cnt : �Ŀ�
	   val : ��Ӥ�����
	   vResults  : ��̤�������������
	*/
	int *vAry = getCaliVariable();
	int cnt   = getCaliValue();
	int val   = getCaliValue();
	int *vResults = getCaliVariable();
	int i;
	
	DEBUG_COMMAND("ShArray.AndNotArray %p,%d,%d,%p:\n", vAry, cnt, val, vResults);
	
	for (i = 0; i < cnt; i++) {
		*vResults &= ((*vAry != val) ? 1 : 0);
		vResults++; vAry++;
	}
}

void AndLowArray(void) { /* 18 */
	/*
	  ���� vAry �� min ���⾮�����ʤ�С�vResult �� 1 �� AND ��
	  �����Ǥʤ��ʤ�� 0 �򤫤���
	  
	   vAry: ����
	   cnt : �Ŀ�
	   min : �Ǿ���
	   vResults  : ��̤�������������
	*/
	int *vAry = getCaliVariable();
	int cnt   = getCaliValue();
	int min   = getCaliValue();
	int *vResults = getCaliVariable();
	int i;
	
	DEBUG_COMMAND("ShArray.AndLowArray: %d,%d,%d,%d:\n", vAry, cnt, min, vResults);
	
	for (i = 0; i < cnt; i++) {
		*vResults &= ((*vAry < min) ? 1 : 0);
		vResults++; vAry++;
	}
}

void AndHighArray(void) { /* 19 */
	/*
	  ���� vAry �� min �����礭���ʤ�С�vResult �� 1 �� AND ��
	  �����Ǥʤ��ʤ�� 0 �򤫤���
	  
	   vAry: ����
	   cnt : �Ŀ�
	   max : �Ǿ���
	   vResults  : ��̤�������������
	*/
	int *vAry     = getCaliVariable();
	int cnt       = getCaliValue();
	int max       = getCaliValue();
	int *vResults = getCaliVariable();
	int i;
	
	DEBUG_COMMAND("ShArray.AndHighArray: %p,%d,%d,%p:\n", vAry, cnt, max, vResults);
	
	for (i = 0; i < cnt; i++) {
		*vResults &= ((*vAry > max) ? 1 : 0);
		vResults++; vAry++;
	}
}

void AndRangeArray(void) { /* 20 */
	/*
	  vAry �� min ���� max �ˤ����硢vResults �� 1 �� AND ��
	  �����Ǥʤ���� 0 �򤫤���
	  
	  vAry: ����
	  cnt:  �Ŀ�
	  min:  �Ǿ���
	  max:  ������
	  vResults:   ��̤��֤�����
	*/
	int *vAry = getCaliVariable();
	int cnt   = getCaliValue();
	int min   = getCaliValue();
	int max   = getCaliValue();
	int *vResults = getCaliVariable();
	int i;
	
	DEBUG_COMMAND("ShArray.AndRangeArray %d,%d,%d,%d,%d:\n", vAry, cnt, min, max, vResults);
	
	for (i = 0; i < cnt; i++) {
		*vResults &= (((*vAry > min) && (*vAry < max)) ? 1 : 0);
		vResults++; vAry++;
	}
}

void AndAndEquArray(void) { /* 21 */
	/*
	  ���� vAry �� mask �Ȥ� AND ��Ȥä� val �����������
	  ���� vResults �ȣ��� AND���������ʤ���� 0 ���

	   vAry: ����
	   mask: ����ˤ�����ޥ���
	   cnt : �Ŀ�
	   val : ��Ӥ�����
	   vResults  : ��̤�������������

	*/
	int *vAry = getCaliVariable();
	int mask  = getCaliValue();
	int cnt   = getCaliValue();
	int val   = getCaliValue();
	int *vResults = getCaliVariable();
	int i;
	
	DEBUG_COMMAND("ShArray.AndAndEquArray: %d,%d,%d,%d,%d:\n", vAry, mask, cnt, val, vResults);
	
	for (i = 0; i < cnt; i++) {
		(*vResults) &= (((*vAry & mask) == val) ? 1 : 0);
		vResults++; vAry++;
	}
}

void OrEquArray(void) { /* 22 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();

	DEBUG_COMMAND_YET("ShArray.OrEquArray: %d,%d,%d,%d:\n", p1, p2, p3, p4);
}

void OrNotArray(void) { /* 23 */
	/*
	  ������ͤ� val ���������ʤ���� vResult �� 1 ��񤭹���
	  
	  vAry: ����
	  cnt : �Ŀ�
	  val : ��Ӥ�����
	  vResults: ��̤�񤭹����ѿ�
	*/
	int *vAry = getCaliVariable();
	int cnt   = getCaliValue();
	int val   = getCaliValue();
	int *vResults = getCaliVariable();
	int i;
	
	DEBUG_COMMAND("ShArray.OrNotArray %p,%d,%d,%p:\n", vAry, cnt, val, vResults);
	
	for (i = 0; i < cnt; i++) {
		// if (*vAry != val) *vResults = 1;
		(*vResults) |= ((*vAry != val) ? 1 : 0);
		vResults++; vAry++;
	}
}

void OrLowArray(void) { /* 24 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();

	DEBUG_COMMAND_YET("ShArray.OrLowArray: %d,%d,%d,%d:\n", p1, p2, p3, p4);
}

void OrHighArray(void) { /* 25 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();

	DEBUG_COMMAND_YET("ShArray.OrHighArray: %d,%d,%d,%d:\n", p1, p2, p3, p4);
}

void OrRangeArray(void) { /* 26 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();

	DEBUG_COMMAND_YET("ShArray.OrRangeArray: %d,%d,%d,%d,%d:\n", p1, p2, p3, p4,p5);
}

void OrAndEquArray(void) { /* 27 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();

	DEBUG_COMMAND_YET("ShArray.OrAndEquArray: %d,%d,%d,%d,%d:\n", p1, p2, p3, p4,p5);
}

void EnumEquArray(void) { /* 28 */
	/*
	  ����Υǡ������ val ��Ʊ���ǡ�������� vResult ���֤�
	  
	  vAry: ����
	  cnt : �Ŀ�
	  val : ��Ӥ�����
	  vResult: ���פ���Ŀ����֤��ѿ�
	*/
	int *vAry = getCaliVariable();
	int cnt   = getCaliValue();
	int val   = getCaliValue();
	int *vResult = getCaliVariable();
	int i;
	
	DEBUG_COMMAND("ShArray.EnumEquArray %p,%d,%d,%p:\n", vAry, cnt, val, vResult);
	
	*vResult = 0;
	
	for (i = 0; i < cnt; i++) {
		if (*vAry == val) (*vResult)++;
		vAry++;
	}
}

void EnumEquArray2(void) { /* 29 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();

	DEBUG_COMMAND_YET("ShArray.EnumEquArray2: %d,%d,%d,%d,%d,%d:\n", p1, p2, p3, p4, p5, p6);
}

void EnumEquNotArray2(void) { /* 30 */
	/*
	  ���󣱤� val1 �������������󣲤�val2 ���������ʤ���Το���
	  vResult ���֤���

	  vAry1: ����
	  vAry2: ����
	  cnt:   �Ŀ�
	  val1: ���󣱤���Ӥ�����
	  val2: ���󣲤���Ӥ�����
	  vResult: ���˰��פ�������֤��ѿ�
	*/
	int *vAry1 = getCaliVariable();
	int *vAry2 = getCaliVariable();
	int cnt    = getCaliValue();
	int val1   = getCaliValue();
	int val2   = getCaliValue();
	int *vResult = getCaliVariable();
	int i;
	
	DEBUG_COMMAND("ShArray.EnumEquNotArray2 %p,%p,%d,%d,%d,%p:\n", vAry1, vAry2, cnt, val1, val2, vResult);
	
	*vResult = 0;
	
	for (i = 0; i < cnt; i++) {
		if ((*vAry1 == val1) && (*vAry2 != val2)) {
			(*vResult)++;
		}
		vAry1++; vAry2++;
	}
}

void EnumNotArray(void) { /* 31 */
	/*
	  ����Τʤ��� val ���������ʤ���ΤθĿ����֤�
	  
	  vAry: ����
	  cnt: �Ŀ�
	  val: ��Ӥ�����
	  vResult: �������ʤ���Το�
	*/
	int *vAry  = getCaliVariable();
	int cnt    = getCaliValue();
	int val    = getCaliValue();
	int *vResult = getCaliVariable();
	int i;

	DEBUG_COMMAND("ShArray.EnumNotArray %p, %d, %d, %p:\n", vAry, cnt, val, vResult);
	
	*vResult = 0;
	
	for (i = 0; i < cnt; i++) {
		if (*vAry != val) {
			(*vResult)++;
		}
		vAry++;
	}
}

void EnumNotArray2(void) { /* 32 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();

	DEBUG_COMMAND_YET("ShArray.EnumNotArray2: %d,%d,%d,%d,%d,%d:\n", p1, p2, p3, p4, p5, p6);
}

void EnumLowArray(void) { /* 33 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	
	DEBUG_COMMAND_YET("ShArray.EnumLowArray: %d,%d,%d,%d:\n", p1, p2, p3, p4);
}

void EnumHighArray(void) { /* 34 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	
	DEBUG_COMMAND_YET("ShArray.EnumHighArray: %d,%d,%d,%d:\n", p1, p2, p3, p4);
}

void EnumRangeArray(void) { /* 35 */
	/*
	  ������ͤΤ����� min �� max �δ֤����Το��� vResult ���֤�
	  
	  vAry: ����
	  cnt : �Ŀ�
	  min : �Ǿ���
	  max : ������
	  vResult: ���פ��������֤��ѿ�
	*/
	int *vAry  = getCaliVariable();
	int cnt    = getCaliValue();
	int min    = getCaliValue();
	int max    = getCaliValue();
	int *vResult = getCaliVariable();
	int i;
	
	DEBUG_COMMAND("ShArray.EnumRangeArray %d,%d,%d,%d,%d:\n", vAry, cnt, min, max, vResult);
	
	*vResult = 0;
	
	for (i = 0; i < cnt; i++) {
		if ((*vAry > min) && (*vAry < max)) {
			(*vResult)++;
		}
		vAry++;
	}

}

void GrepEquArray(void) { /* 36 */
	/*
	  ������ͤ� val ����������� vLastMatch �˰��פ��� index ���֤�
	  vResult �� 1 ���֤�
	  
	  vAry: ����
	  cnt : �Ŀ�
	  val : ��Ӥ�����
	  vMatch: ���פ�������ǥå���
	  vResult: ��ĤǤ� val ��Ʊ���ͤ������ 1
	*/
	int *vAry  = getCaliVariable();
	int cnt    = getCaliValue();
	int val    = getCaliValue();
	int *vMatch  = getCaliVariable();
	int *vResult = getCaliVariable();
	int i;

	DEBUG_COMMAND("ShArray.GrepEquArray  %p,%d,%d,%p,%p:\n", vAry, cnt, val, vMatch, vResult);
	
	*vResult = 0;
	
	for (i = 0; i < cnt; i++) {
		if (*vAry == val) {
			*vMatch  = i;
			*vResult = 1;
			return;
		}
		vAry++;
	}
}

void GrepNotArray(void) { /* 37 */
	/*
	  ������ͤ� val ���������ʤ���� vLastMatch �ˤ��� index ���֤�
	  vResult �� 1 ���֤�
	  
	  vAry: ����
	  cnt : �Ŀ�
	  val : ��Ӥ�����
	  vMatch: ���פ���index
	  vResult: ��ĤǤ� val ��Ʊ���ͤ������ 1
	*/
	int *vAry  = getCaliVariable();
	int cnt    = getCaliValue();
	int val    = getCaliValue();
	int *vMatch  = getCaliVariable();
	int *vResult = getCaliVariable();
	int i;
	
	DEBUG_COMMAND("ShArray.GrepNotArray %p,%d,%d,%p,%p:\n", vAry, cnt, val, vMatch, vResult);
	
	*vResult = 0;
	
	for (i = 0; i < cnt; i++) {
		if (*vAry != val) {
			*vMatch  = i;
			*vResult = 1;
			return;
		}
		vAry++;
	}
}

void GrepEquArray2(void) { /* 38 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();

	DEBUG_COMMAND_YET("ShArray.GrepEquArray2: %d,%d,%d,%d,%d,%d,%d:\n", p1, p2, p3, p4, p5, p6, p7);
}

void GrepNotArray2(void) { /* 39 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();

	DEBUG_COMMAND_YET("ShArray.GrepNotArray2: %d,%d,%d,%d,%d,%d,%d:\n", p1, p2, p3, p4, p5, p6, p7);
}

void GrepEquNotArray2(void) { /* 40 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();

	DEBUG_COMMAND_YET("ShArray.GrepEquNotArray2: %d,%d,%d,%d,%d,%d,%d:\n", p1, p2, p3, p4, p5, p6, p7);
}

void GrepLowArray(void) { /* 41 */
	/*
	  ������ͤ� min ���⾮������� vMatch �˰��פ��� index 
	  ���֤� vResult �� 1 ���֤�
	  
	  vAry: ����
	  cnt : �Ŀ�
	  min : �Ǿ���
	  vMatch: ���פ�������ǥå���
	  vResult: ��ĤǤ� val ��Ʊ���ͤ������ 1
	*/
	int *vAry = getCaliVariable();
	int cnt   = getCaliValue();
	int min   = getCaliValue();
	int *vMatch  = getCaliVariable();
	int *vResult = getCaliVariable();
	int i;
	
	DEBUG_COMMAND("ShArray.GrepLowArray: %p,%d,%d,%p,%p:\n", vAry, cnt, min, vMatch, vResult);
	
	*vResult = 0;
	
	for (i = 0; i < cnt; i++) {
		if (*vAry < min) {
			*vMatch = i;
			*vResult = 1;
			return;
		}
		vAry++;
	}
}

void GrepHighArray(void) { /* 42 */
	/*
	  ������ͤ� min �����礭����� vMatch �˰��פ��� index 
	  ���֤� vResult �� 1 ���֤�
	  
	  vAry: ����
	  cnt : �Ŀ�
	  max : ������
	  vMatch: ���פ�������ǥå���
	  vResult: ��ĤǤ� val ��Ʊ���ͤ������ 1
	*/
	int *vAry = getCaliVariable();
	int cnt   = getCaliValue();
	int max   = getCaliValue();
	int *vMatch  = getCaliVariable();
	int *vResult = getCaliVariable();
	int i;
	
	DEBUG_COMMAND("ShArray.GrepHighArray: %p,%d,%d,%p,%p:\n", vAry, cnt, max, vMatch, vResult);
	
	*vResult = 0;
	
	for (i = 0; i < cnt; i++) {
		if (*vAry > max) {
			*vMatch = i;
			*vResult = 1;
			return;
		}
		vAry++;
	}
}

void GrepRangeArray(void) { /* 43 */
	/*
	  ������ͤ� max �� min �δ֤ˤ���� vMatch �˰��פ��� index 
	  ���֤� vResult �� 1 ���֤�
	  
	  vAry: ����
	  cnt : �Ŀ�
	  min : �Ǿ���
	  max : ������
	  vMatch: ���פ�������ǥå���
	  vResult: ��ĤǤ� val ��Ʊ���ͤ������ 1
	*/
	int *vAry  = getCaliVariable();
	int cnt    = getCaliValue();
	int min    = getCaliValue();
	int max    = getCaliValue();
	int *vMatch = getCaliVariable();
	int *vResult    = getCaliVariable();
	int i;
	
	DEBUG_COMMAND("ShArray.GrepRangeArray %p,%d,%d,%d,%p,%p:\n", vAry, cnt, max, min, vMatch, vResult);
	
	*vResult = 0;
	
	for (i = 0; i < cnt; i++) {
		if ((*vAry > min) && (*vAry < max)) {
			*vMatch = i;
			*vResult = 1;
			return;
		}
		vAry++;
	}
}

void GrepLowOrderArray(void) { /* 44 */
	/*
	  ���� vAry ����� min�����礭���� max ���⾮�������
	  �Τ������Ǥ⾮������Τ� index �� vLastMatch ���֤���
	  ��������v1[index] �� 0 �Ǥ���ɬ�פ����롣
	  
	  vAry: ����
	  cnt : �Ŀ�
	  min : �Ǿ���
	  max : ������
	  v1  : ��̤��֤�����(0 �ξ�ꤷ����Ӥ������Ǿ��ξ��� 1 ���
	  vLastMatch: �Ǿ��ͤ򼨤������index
	  vResult: �Ǿ��ͤ����Ĥ���� 1, ���Ĥ���ʤ���� 0
	*/
	int *vAry  = getCaliVariable();
	int cnt    = getCaliValue();
	int min    = getCaliValue();
	int max    = getCaliValue();
	int *v1    = getCaliVariable();
	int *vLastMatch = getCaliVariable();
	int *vResult    = getCaliVariable();
	int i, j, k = 0;
	
	DEBUG_COMMAND("ShArray.GrepLowOrderArray %p,%d,%d,%d,%p,%p,%p:\n", vAry, cnt, min, max, v1, vLastMatch, vResult);
	
	*vResult = 0;
	for (i = 0; i < cnt; i++) {
		if((*(vAry + i) == min) && (*(v1 + i) == 0)) {
			*vResult    = 1;
			*vLastMatch = i;
			*(v1 + i)   = 1;
			return;
		}
	}
	
	j = 65536;
	for (i = 0; i < cnt; i++) {
		if ((*(vAry + i) > min) && (*(vAry + i) < max) &&
		    (*(v1 + i) == 0) && (*(vAry + i) < j)) {
			j = *(vAry + i);
			k = i;
		}
	}
	if (j < 65536) {
		*vResult    = 1;
		*vLastMatch = k;
		*(v1 + k)   = 1;
	}

}

void GrepHighOrderArray(void) { /* 45 */
	/*
	  ���� vAry ����� min�����礭���� max ���⾮�������
	  �Τ������Ǥ��礭����Τ� index �� vLastMatch ���֤���
	  ��������v1[index] �� 0 �Ǥ���ɬ�פ����롣
	  
	  vAry: ����
	  cnt : �Ŀ�
	  min : �Ǿ���
	  max : ������
	  v1  : ��̤��֤�����(0 �ξ�ꤷ����Ӥ������Ǿ��ξ��� 1 ���
	  vLastMatch: �����ͤ򼨤������index
	  vResult: �����ͤ����Ĥ���� 1, ���Ĥ���ʤ���� 0
	*/
	int *vAry  = getCaliVariable();
	int cnt    = getCaliValue();
	int min    = getCaliValue();
	int max    = getCaliValue();
	int *v1    = getCaliVariable();
	int *vLastMatch = getCaliVariable();
	int *vResult    = getCaliVariable();
	int i, j, k = 0;
	
	DEBUG_COMMAND("ShArray.GrepHighOrderArray %p,%d,%d,%d,%p,%p,%p:\n", vAry, cnt, min, max, v1, vLastMatch, vResult);
	
	*vResult = 0;
	for (i = 0; i < cnt; i++) {
		if ((*(vAry + i) == max) && (*(v1 + i) == 0)) {
			*vResult    = 1;
			*vLastMatch = i;
			*(v1 + i)   = 1;
			return;
		}
	}
	
	j = -1;
	for (i = 0; i < cnt; i++) {
		if ((*(vAry + i) >= min) && (*(vAry + i) < max) &&
		    (*(v1 + i) == 0) && (*(vAry + i) > j)) {
			j = *(vAry + i);
			k = i;
		}
	}
	if (j >= 0) {
		*vResult    = 1;
		*vLastMatch = k;
		*(v1 + k)   = 1;
	}
}

void ChangeEquArray(void) { /* 46 */
	int *vAry = getCaliVariable();
	int cnt = getCaliValue();
	int src = getCaliValue();
	int dst = getCaliValue();
	int i;
	
	DEBUG_COMMAND("ShArray.ChangeEquArray: %d,%d,%d,%d:\n", vAry, cnt, src, dst);
	
	for (i = 0; i < cnt; i++) {
		if (*vAry == src) {
			*vAry = dst;
		}
		vAry++;
	}
}

void ChangeNotArray(void) { /* 47 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	
	DEBUG_COMMAND_YET("ShArray.ChangeNotArray: %d,%d,%d,%d:\n", p1, p2, p3, p4);
}

void ChangeLowArray(void) { /* 48 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	
	DEBUG_COMMAND_YET("ShArray.ChangeLowArray: %d,%d,%d,%d:\n", p1, p2, p3, p4);
}

void ChangeHighArray(void) { /* 49 */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	
	DEBUG_COMMAND_YET("ShArray.ChangeHighArray: %d,%d,%d,%d:\n", p1, p2, p3, p4);
}

void ChangeRangeArray(void) { /* 50 */
	/*
	  ������ǡ����� min ���� max �δ֤ˤ���Ȥ��� val ���֤�����
	  
	  vAry: ����
	  cnt : �Ŀ�
	  min : �Ǿ���
	  max : ������
	  val : �֤���������
	*/
	int *vAry = getCaliVariable();
	int cnt   = getCaliValue();
	int min   = getCaliValue();
	int max   = getCaliValue();
	int val   = getCaliValue();
	int i;
	
	DEBUG_COMMAND("ShArray.ChangeRangeArray %p,%d,%d,%d,%d:\n", vAry, cnt, min, max, val);
	
	for (i = 0; i < cnt; i++) {
		if ((*vAry > min) && (*vAry < max)) {
			*vAry = val;
		}
		vAry++;
	}
}

void CopyArrayToRect(void) { /* 51 */
	/*
	  vSrc �� sw * sh ���ΰ�� vDst �� dx,dy �ΰ��֤˥��ԡ� 
	  
	  vSrc: ���ԡ�������
	  sw  : ���ԡ��� width
	  sh  : ���ԡ��� height
	  sx  : ���ԡ��� x
	  sy  : ���ԡ��� y
	  vDst: ���ԡ�������
	  dw  : ���ԡ��� width
	  dh  : ���ԡ��� height
	*/
	int *vSrc = getCaliVariable();
	int sw    = getCaliValue();
	int sh    = getCaliValue();
	int sx    = getCaliValue();
	int sy    = getCaliValue();
	int *vDst = getCaliVariable();
	int dw    = getCaliValue();
	int dh    = getCaliValue();
	int x, y;

	DEBUG_COMMAND("ShArray.CopyArrayToRect %p,%d,%d,%d,%d,%p,%d,%d:\n", vSrc, sw, sh, sx, sy, vDst, dw, dh);
	
	vSrc += (sy * sw + sx);
	for (y = 0; y < dh; y++) {
		for (x = 0; x < dw; x++) {
			*(vDst + x) = *(vSrc + x);
		}
		vSrc += sw; vDst += dw; 
	}
}

void CopyRectToArray(void) { /* 52 */
	/*
	  vSrc �� sw * sh ���ΰ�� vDst �� dx,dy �ΰ��֤˥��ԡ� 
	  
	  vSrc: ���ԡ�������
	  sw  : ���ԡ��� width
	  sh  : ���ԡ��� height
	  vDst: ���ԡ�������
	  dw  : ���ԡ��� width
	  dh  : ���ԡ��� height
	  dx  : ���ԡ��� x
	  dy  : ���ԡ��� y
	*/
	int *vSrc = getCaliVariable();
	int sw    = getCaliValue();
	int sh    = getCaliValue();
	int *vDst = getCaliVariable();
	int dw    = getCaliValue();
	int dh    = getCaliValue();
	int dx    = getCaliValue();
	int dy    = getCaliValue();
	int x, y;
	
	DEBUG_COMMAND("ShArray.CopyRectToArray %p,%d,%d,%p,%d,%d,%d,%d:\n", vSrc, sw, sh, vDst, dw, dh, dx, dy);
	
	vDst += (dy * dw + dx);
	for (y = 0; y < sh; y++) {
		for (x = 0; x < sw; x++) {
			*(vDst + x) = *(vSrc + x);
		}
		vSrc += sw; vDst += dw;
	}
}

void ChangeSecretArray(void) { /* 53 */
	/*
	  �ɤ�ʬ����ʤ������ǡ����򥳡��ɲ����Ƥ���褦��
	  
	  vAry: ����
	  cnt : �Ŀ�
	  type: ��ǽ�ֹ�
	  vResult: ��̤��֤��ѿ�
	*/
	int *vAry    = getCaliVariable();
	int cnt      = getCaliValue();
	int type     = getCaliValue();
	int *vResult = getCaliVariable();
	static WORD key[4] = { 0x7A7A, 0xADAD, 0xBCBC, 0xCECE }; /* key */
	
	DEBUG_COMMAND("ShArray.ChangeSecretArray %p,%d,%d,%p:\n", vAry, cnt, type, vResult);
	
	*vResult = 0;
	
	switch(type) {
	case 0:
		/*
		  cnt -1 ���ܤΥ����� vAry �˼��Ф�
		*/
		if (cnt > 0 && cnt < 5) {
			*vAry = key[cnt -1];
			*vResult = 1;
		}
		break;
	case 1:
		/*
		  vAry �� cnt -1 ���ܤΥ����˥��åȤ���
		*/
		if (cnt > 0 && cnt < 5) {
			key[cnt -1] = *vAry;
			*vResult = 1;
		}
		break;
	case 2:
		{
			/*
			  ���󥳡��ɤ��Σ�
			*/
			int i, j = 0;
			WORD ax = key[3] ^ 0x5a5a;
			for (i = 0; i < cnt; i++) {
				(*vAry) ^= ax; ax = (key[i&3] ^ *vAry);
				j ^= ax;
				if (i & 2) {
					ax = !ax ^ (i*3);
				}
				if (i & 4) {
					ax = (ax >> 4) | (ax << 12);
				}
				vAry++;
			}
			*vResult = j;
		}
		break;
	case 3:
		{
			/*
			  ���󥳡��ɤ��Σ�
			*/
			int i, j = 0, k;
			WORD ax = key[3] ^ 0x5a5a;
			for (i = 0; i < cnt; i++) {
				k = *vAry; 
				*vAry ^= ax; ax = (key[i&3] ^ k);
				j ^= ax;
				if (i & 2) {
					ax = !ax ^ (i*3);
				}
				if (i & 4) {
					ax = (ax >> 4) | (ax << 12);
				}
				vAry++;
				
			}
			*vResult = j;
		}
		break;
	}
}
