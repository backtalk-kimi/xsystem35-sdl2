/*
 * cdrom.Irix.c  CD-ROM��������
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
/* $Id: cdrom.Irix.c,v 1.10 2006/04/21 16:40:48 chikama Exp $ */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <cdaudio.h>

#include "portab.h"
#include "cdrom.h"
#include "music_server.h"

static int  cdrom_init(char *);
static int  cdrom_exit();
static int  cdrom_start(int);
static int  cdrom_stop();
static int  cdrom_getPlayingInfo(cd_time *);

#define cdrom cdrom_irix
cdromdevice_t cdrom = {
	cdrom_init,
	cdrom_exit,
	cdrom_start,
	cdrom_stop,
	cdrom_getPlayingInfo,
	NULL,
	NULL
};

static CDPLAYER* cd_fd;
static boolean   enabled = FALSE;
static int       lastindex;             /* �ǽ��ȥ�å� */

/* CD-ROM ���ܼ����ɤ߽Ф��Ƥ��� */
static int get_cd_entry() {
	CDSTATUS st;

	/* �ǽ��ȥ�å��ֹ������ */
	if (CDgetstatus(cd_fd, &st) < 0) {
		perror("CDgetstatus");
		return NG;
	}
	lastindex = st.last;
	if (lastindex <= 1) { /* ���ȥ�å��ʾ�ʤ��ȥ��� */
		fprintf(stderr, "No CD-AUDIO in CD-ROM\n");
		return NG; 
	}
	
	prv.cd_maxtrk = lastindex;
	
	return OK;
}

/* �ǥХ����ν���� */
int cdrom_init(char *dev_cd) {
	if (dev_cd == NULL) return NG;

	if ((cd_fd = CDopen(dev_cd, "r")) == NULL) {
		perror("CDROM_DEVICE OPEN");
		enabled = FALSE;
		return NG;
	}
	if (OK == get_cd_entry()) {
		enabled = TRUE;
		return OK;
	}
	enabled = FALSE;
	return NG;
}

/* �ǥХ����θ���� */
int cdrom_exit() {
	if (enabled) {
		cdrom_stop();
		CDclose(cd_fd);
	}
	return OK;
}

/* �ȥ�å��ֹ� trk �α��� trk = 1~ */
int cdrom_start(int trk) {
	if (!enabled) return NG;

	/* �ʿ�����¿��������Բ�*/
	if (trk > lastindex) {
		return NG;
	}
	if (CDplaytrack(cd_fd, trk, 1) < 0) {
		perror("CDplaytrack");
		return NG;
	}
	return OK;
}

/* ������� */
int cdrom_stop() {
	if (enabled) {
		if (CDstop(cd_fd) < 0) {
			perror("CDstop");
			return NG;
		}
		return OK;
	}
	return NG;
}

/* ���߱�����Υȥ�å�����μ��� */
int cdrom_getPlayingInfo (cd_time *info) {
	CDSTATUS st;
	
	if (!enabled)
		goto errexit;
	
	if (CDgetstatus(cd_fd, &st) < 0) {
		perror("CDgetstatus");
		goto errexit;
	}
	if (st.state != CD_PLAYING) {
		goto errexit;
	}
	info->t = st.track;
	info->m = st.min;
	info->s = st.sec;
	info->f = st.frame;
	return OK;
 errexit:
	info->t = info->m = info->s = info->f = 999;
	return NG;
}
