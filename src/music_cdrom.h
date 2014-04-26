/*
 * music_cdrom.h  music server CDROM part
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
/* $Id: music_cdrom.h,v 1.1 2002/08/18 09:35:29 chikama Exp $ */

#ifndef __MUSIC_CDROM_H__
#define __MUSIC_CDROM_H__

#include "portab.h"
#include "cdrom.h"

struct _cdobj {
	int st; // ���֥������Ȥ����ܾ���

	int track; // ���եȥ�å�
	int loop;  // ���꤫������ (0�ξ���̵��)
	int cnt;   // �ºݤ˷����֤�����
	
	boolean in_play; // �����桩
        
	cd_time time; // ���հ���
	
	cdromdevice_t *dev;
};
typedef struct _cdobj cdobj_t;

extern int muscd_init();
extern int muscd_exit();
extern int muscd_start(int trk, int loop);
extern int muscd_stop();
extern cd_time muscd_getpos();
extern int muscd_cb();

#endif /* __MUSIC_CDROM_H__ */
