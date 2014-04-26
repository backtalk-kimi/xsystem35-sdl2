/*
 * sactamask.c: SACTEFAM.KLD Ÿ��
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
/* $Id: sactamask.c,v 1.1 2003/04/22 16:29:52 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <glib.h>

#include "portab.h"
#include "system.h"
#include "LittleEndian.h"
#include "imput.h"
#include "sact.h"
#include "surface.h"
#include "ngraph.h"
#include "sprite.h"
#include "counter.h"

static surface_t *smask_get(int no);
static surface_t *smask_mul(surface_t *sf, int val);



struct ecopyparam {
	int sttime;
	int curtime;
	int edtime;
	int curstep;
	int oldstep;
};
typedef struct ecopyparam ecopyparam_t;
static ecopyparam_t ecp;

// SACTEFAM.KLD ���ɤ߹���
int smask_init(char *path) {
	struct stat sbuf;
	int i, fd;
	char *adr;
	SACTEFAM_t *am;

	if (0 > (fd = open(path, O_RDONLY))) {
		WARNING("open: %s\n", strerror(errno));
		return NG;
	}
	
	if (0 > fstat(fd, &sbuf)) {
		WARNING("fstat: %s\n", strerror(errno));
		close(fd);
		return NG;
	}

	if (MAP_FAILED == (adr = mmap(0, sbuf.st_size, PROT_READ, MAP_SHARED, fd, 0))) {
		WARNING("mmap: %s\n", strerror(errno));
		close(fd);
		return NG;
	}
	
	am = &sact.am;
	am->mapadr = adr;
	am->size = sbuf.st_size;
	am->fd = fd;
	
	am->datanum = LittleEndian_getDW(adr, 0);
	am->no = g_new(int, am->datanum);
	am->offset = g_new(int, am->datanum);
	
	for (i = 0; i < am->datanum; i++) {
		am->no[i] = LittleEndian_getDW(adr, 16 + i * 16);
		am->offset[i] = LittleEndian_getDW(adr, 16 + i * 16 + 8);
	}
	
	return OK;
}

// �����ֹ�� alphamask �ե�������ߤ���
static surface_t *smask_get(int no) {
	int i;
	SACTEFAM_t *am = &sact.am;
	
	for (i = 0; i < am->datanum; i++) {
		if (am->no[i] == no) break;
	}

	if (i == am->datanum) return NULL;
	
	return sf_getcg(am->mapadr + am->offset[i]);
}

// �١����ˤʤ�ޥ����� alpha �ͤ���礷�Ƽ��Ф�
static surface_t *smask_mul(surface_t *sf, int val) {
	surface_t *out = sf_create_alpha(sf->width, sf->height);
	BYTE *src = sf->alpha;
	BYTE *dst = out->alpha;
	int pix = sf->width * sf->height;

	while(pix--) {
		int i = (*src - val) * 16;
		if (i < 0)        *dst = 255; // �����ͤ����礭���Τϥ��ԡ�
		else if (i > 255) *dst = 0;   // �����ͤ��⾮�����Τ�̵��
		else              *dst = 255-i; // ����ʳ����ͤ�16��
		src++; dst++;
	}
	
	return out;
}

/**
 * �ޥ����Ĥ����̹���
 */
int sp_eupdate_amap(int index, int time, int cancel) {
	surface_t *mask, *mask2;
	surface_t *sfsrc, *sfdst;
	int key;
	
	mask = smask_get(index);
	if (mask == NULL) {
		sp_update_all(TRUE);
		return OK;
	}
	
	// ���ߤ� sf0 �򥻡���
	sfsrc = sf_dup(sf0);
	sp_update_all(FALSE);
	sfdst = sf_dup(sf0);
	sf_copyall(sf0, sfsrc);
	
	ecp.sttime = ecp.curtime = get_high_counter(SYSTEMCOUNTER_MSEC);
	ecp.edtime = ecp.curtime + time*10;
	ecp.oldstep = 0;
	
	while ((ecp.curtime = get_high_counter(SYSTEMCOUNTER_MSEC)) < ecp.edtime) {
		int curstep = 255 * (ecp.curtime - ecp.sttime)/ (ecp.edtime - ecp.sttime);
		// ���ˤʤ�ޥ�����alpha�ͤ�16�ܤ����ߤ����Ȥ���������Ф�
		mask2 = smask_mul(mask, curstep);
		
		gre_BlendUseAMap(sf0, 0, 0, sfsrc, 0, 0, sfdst, 0, 0, sfsrc->width, sfsrc->height, mask2, 0, 0, 255);
		ags_updateFull();
		
		key = sys_keywait(10, cancel);
		if (cancel && key) break;
		
		// ����ޥ�������
		sf_free(mask2);
	}
	
	sf_copyall(sf0, sfdst);
	ags_updateFull();
	sf_free(sfsrc);
	sf_free(sfdst);
	
	sf_free(mask);
	return OK;
}
