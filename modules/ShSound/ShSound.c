/*
 * ShSound.c ���ڴ�Ϣ module
 *
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
/* $Id: ShSound.c,v 1.13 2003/08/02 13:10:32 chikama Exp $ */
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <glib.h>

#include "portab.h"
#include "xsystem35.h"
#include "nact.h"
#include "dri.h"
#include "ald_manager.h"
#include "wavfile.h"
#include "music_client.h"
#include "pcmlib.h"
#include "shpcmlib.c"

/* for wav*Memory */
static WAVFILE *wfile;

void Init() {
	/*
	  �⥸�塼������
	*/
	int p1 = getCaliValue(); /* ISys3x */
	
	DEBUG_COMMAND("ShSound.Init %d:\n", p1);
}

void wavLoad() {
	/*
	  ����Υ���åȤ�PCM�ե���������
	  
	  slot: ���ɤ��륹��å�(�����ͥ�)�ֹ�
	  no  : ���ɤ���ե������ֹ�
	*/
	int slot = getCaliValue();
	int no   = getCaliValue();
	
	DEBUG_COMMAND("ShSound.wavLoad %d,%d:\n", slot, no);
	
	mus_wav_load(slot, no);
}

void wavUnload() {
	/*
	  ����Υ���åȤ�PCM�ե��������
	  
	  slot: ������륹��å��ֹ�
	*/
	int slot = getCaliValue();
	
	mus_wav_unload(slot);
	
	DEBUG_COMMAND("ShSound.wavUnload %d:\n",slot);
}

void wavUnloadRange() {
	/*
	  ����Υ���å�(ʣ��)��PCM�ե��������
	  
	  slot:  �������ǽ�Υ���å��ֹ�
	  range: �������Ŀ�
	*/
	int slot  = getCaliValue();
	int range = getCaliValue();
	int i;
	
	for (i = slot; i < (slot + range); i++) {
		mus_wav_unload(i);
	}
	
	DEBUG_COMMAND("ShSound.wavUnloadRange %d,%d:\n", slot, range);
}

void wavUnloadAll() {
	/*
	  ���٤ƤΥ���åȤ�PCM�ե��������
	*/
	int i;
	
	for (i = 0; i < 128; i++) {
		mus_wav_unload(i);
	}
	
	DEBUG_COMMAND("ShSound.wavUnloadAll:\n");
}

void wavLoadMemory() {
	/*
	  ������ֹ�� WAV �ե������������ɤ߹���
	  
	  no: �ɤ߹���ե������ֹ�
	*/
	int no = getCaliValue();
	
	wfile = pcmlib_load_rw(no);
	
	DEBUG_COMMAND("ShSound.wavLoadMemory %d:\n", no);
}

void wavSendMemory() {
	/*
	  wavLoadMemory ���ɤ߹�����ǡ��������Υ���åȤ�����
	  
	  slot: PCM�ǡ��������륹��å��ֹ�
	*/
	int slot = getCaliValue();
	
	if (wfile) {
		mus_wav_sendfile(slot, wfile);
		pcmlib_free(wfile);
		wfile = NULL;
	}
	
	DEBUG_COMMAND("ShSound.wavSendMemory %d:\n", slot);
}

void wavFadeVolumeMemory() {
	/*
	  wavLoadMemory ���ɤ߹�����ǡ����Υܥ�塼��Υե�����
	  
	  start: �ե����ɳ��ϻ��� (10msecñ��)
	  range: �ե����ɷ�³���� (10msecñ��)
	*/
	int start = getCaliValue();
	int range = getCaliValue();
	
	if (wfile == NULL) return;
	
	pcmlib_fade_volume_memory(wfile, start, range);
	
	DEBUG_COMMAND("ShSound.wavFadeVolumeMemory %d,%d:\n", start, range);
}

void wavReversePanMemory() {
	/*
	  wavLoadMemory���ɤ߹�����ǡ����κ����Υ����ͥ��ȿž
	*/
	
	if (wfile == NULL) return;
	
	pcmlib_reverse_pan_memory(wfile);
	
	DEBUG_COMMAND("ShSound.wavReversePanMemory:\n");
}

void wavPlay() {
	/*
	  ����Υ���åȤ�PCM�����
	  
	  slot: �������륹��å��ֹ�
	  loop: 0�ʤ飱�����������!0�ʤ�̵�¤˷����֤�
	*/
	int slot = getCaliValue();
	int loop = getCaliValue();
	
	mus_wav_play(slot, loop == 0 ? 1 : -1);
	
	DEBUG_COMMAND("ShSound.wavPlay %d, %d:\n", slot, loop);
}

void wavPlayRing() {
	/*
	  ������ϰϤΥ���åȤ�PCM��ƤФ����˷����֤�
	  
	  start: �ǽ�Υ���å��ֹ�
	  cnt:   �����֤�����åȤθĿ�
	  *cur:  ���ߺ������Ƥ��륹��åȤΥ���ǥå���
	*/
	int start = getCaliValue();
	int cnt   = getCaliValue();
	int *cur  = getCaliVariable();
	
	mus_wav_play(start + (*cur % cnt), 1);
	*cur = (*cur + 1) % cnt;
	
	DEBUG_COMMAND("ShSound.wavPlayRing %d,%d,%d:\n", start, cnt, *cur);
}

void wavStop() {
	/*
	  ����Υ���åȤκ��������
	  
	  slot: ��ߤ��륹��å��ֹ�
	*/
	int slot = getCaliValue();
	
	DEBUG_COMMAND("ShSound.wavStop %d:\n", slot);
	
	mus_wav_stop(slot);
}

void wavStopAll() {
	/*
	  ���ƤΥ���åȤκ��������
	*/
	int i;
	
	for (i = 0; i < 128; i++) {
		mus_wav_stop(i);
	}
	
	DEBUG_COMMAND("ShSound.wavStopAll:\n");
}

void wavPause() {
	/*
	  ����Υ���åȤκ����������
	  
	  slot: �����ߤ��륹��å��ֹ�
	*/
	int slot = getCaliValue();
	
	DEBUG_COMMAND_YET("ShSound.wavPause %d:\n", slot);
}

void wavIsPlay() {
	/*
	  ����Υ���åȤ������椫�ɤ�����Ĵ�٤�
	  
	  slot:    Ĵ�٤륹��å��ֹ�
	  *result: 0�ʤ�����桢!0�ʤ������
	*/ 
	int slot = getCaliValue();
	int *result = getCaliVariable();
	
	*result = mus_wav_get_playposition(slot);
	
	DEBUG_COMMAND("ShSound.wavIsPlay %d,%p:\n", slot, result);
}

void wavIsPlayRange() {
	/*
	  ������ϰϤΥ���åȤ������椫�ɤ�����Ĵ�٤�
	  
	  slot: Ĵ�٤�ǽ�Υ���å�
	  range: Ĵ�٤륹��åȤθĿ�
	  *result: 0:���٤ƤΥ���åȤ������, 1: �ɤ줫�Υ���åȤ�������
	*/
	int slot  = getCaliValue();
	int range = getCaliValue();
	int *result = getCaliVariable();
	int i, ret = 0;
	
	for (i = slot; i < (slot + range); i++) {
		ret += mus_wav_get_playposition(i);
	}
	
	*result = ret;
	
	DEBUG_COMMAND("ShSound.wavIsPlayRange %d,%d,%p:\n", slot, range, result);
}
