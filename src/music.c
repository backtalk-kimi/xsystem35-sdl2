/*
 * music.c  Sound module
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
#include "portab.h"

#include <stdio.h>

#include "music.h"
#include "music_private.h"

struct _musprvdat musprv;

int mus_init() {
	muscd_init();
	musmidi_init();
	prv.pcm_valid = muspcm_init() == OK;
	return OK;
}

int mus_exit() {
	if (prv.cd_valid) muscd_exit();
	if (prv.midi_valid) musmidi_exit();
	if (prv.pcm_valid) muspcm_exit();
	return OK;
}

/*
 * cdrom �α��ճ��� 
 *   track: �ȥ�å��ֹ� (���ȥ�å��� 1)
 *   loop : �����֤���� (0�ξ���̵��)
 */
int mus_cdrom_start(int track, int loop) {
	if (!prv.cd_valid) return NG;
	muscd_start(track, loop);
	muscd_cb();
	return OK;
}

/*
 * cdrom �α������
 */
int mus_cdrom_stop() {
	if (!prv.cd_valid) return NG;
	muscd_stop();
	muscd_cb();
	return OK;
}

/*
 * cdrom �α��վ��֤μ���
 *   info: ���ջ���(track/min/sec/frame)�ξ��֤��Ǽ������
 *         ��ߤ��Ƥ������ 999/999/999/999 ���֤�
 */
int mus_cdrom_get_playposition(cd_time *tm) {
	if (!prv.cd_valid) return NG;
	*tm = muscd_getpos();
	return OK;
}

/*
 * cdrom �κ���ȥ�å����μ���
 *   
 */
int mus_cdrom_get_maxtrack() {
	if (!prv.cd_valid) return 0;
	return prv.cd_maxtrk;
}

/*
 * CDROM ��ͭ��/̵�� �ե饰�μ���
 *   return: FALASE -> ̵��
 *           TRUE   -> ͭ��
 */
boolean mus_cdrom_get_state() {
	return prv.cd_valid;
}

/*
 * midi �α��ճ��� 
 *   no  : �ե������ֹ�( no >= 1)
 *   loop: �����֤���� (0�ξ���̵��)
 */
int mus_midi_start(int no, int loop) {
	return musmidi_start(no, loop);
}

/*
 * midi �α������
 */
int mus_midi_stop(void) {
	return musmidi_stop();
}

/*
 * midi �ΰ�����
 */
int mus_midi_pause(void) {
	return musmidi_pause();
}

/*
 * midi �ΰ����߲��
 */
int mus_midi_unpause(void) {
	return musmidi_unpause();
}

/*
 * midi �α��վ��֤μ���
 *  state: ���ջ��֤��ֹ�ξ��֤��Ǽ������
 *         ��ߤ��Ƥ������ 0 ������
 */
int mus_midi_get_playposition(midiplaystate *state) {
	midiplaystate st = musmidi_getpos();
	*state = st;
	return OK;
}

/*
 * midi �α��� flag/variable �ξ��֤����ꤹ��
 *   mode : 0 -> flag mode
 *          1 -> variable mode
 *   index: flag/variable �ֹ�
 *   val  : �񤭹�����
 */
int mus_midi_set_flag(int mode, int index, int val) {
	return musmidi_setflag(mode, index, val);
}

/*
 * midi �α��� flag/variable �ξ��֤��������
 *   mode : 0 -> flag mode
 *          1 -> variable mode
 *   index: flag/variable �ֹ�
 *
 *   return : flag/variable ����
 */
int mus_midi_get_flag(int mode, int index) {
	return musmidi_getflag(mode, index);
}

/*
 * MIDI ��ͭ��/̵�� �ե饰�μ���
 *   return: FALASE -> ̵��
 *           TRUE   -> ͭ��
 */
boolean mus_midi_get_state() {
	return prv.midi_valid;
}

/*
 * WAV �α��ճ��� (command S?)
 *   no  : �ե������ֹ�( no >= 1)
 *   loop: �����֤���� (0�ξ���̵��)
 */
int mus_pcm_start(int no, int loop) {
	if (!prv.pcm_valid) return NG;
	if (muspcm_load_no(0, no) == NG)
		return NG;
	return muspcm_start(0, loop);
}

/*
 * WAV �򺸱� mix ���Ʊ���
 *   noL : ���ѤΥե������ֹ�(noL >= 1)
 *   noR : ���ѤΥե������ֹ�(noR >= 1)
 *   loop: �����֤���(0�ξ���̵�¥롼��)
 */
int mus_pcm_mix(int noL, int noR, int loop) {
	if (!prv.pcm_valid) return NG;
	if (muspcm_load_mixlr(0, noL, noR) == NG)
		return NG;
	return muspcm_start(0, loop);
}

/*
 * WAV �α������ (command S?)
 *   msec: �ߤޤ�ޤǤλ���(msec), 0�ξ��Ϥ����˻ߤޤ�
 */
int mus_pcm_stop(int msec) {
	if (!prv.pcm_valid) return NG;

	muspcm_fadeout(0, msec);
	return OK;
}

/*
 * WAV �ե����������˺ܤ���
 *   no  : �ե������ֹ�( no >= 1)
 */
int mus_pcm_load(int no) {
	if (!prv.pcm_valid) return NG;

	return muspcm_load_no(0, no);
}

/*
 * WAV �α��վ��֤μ���
 *   pos: ���ջ��֤��Ǽ������(msec)
 *        ��ߤ��Ƥ������ 0 ������
 *        loop���Ƥ�����Ϲ�׻���
 */
int mus_pcm_get_playposition(int *pos) {
	if (!prv.pcm_valid) return NG;

	*pos = muspcm_getpos(0);
	return OK;
}

/* pcm (Scommand) related function */
/*
 * ����Υե����ޥåȤǺ�����ǽ���ɤ���Ĵ�٤�
 *   bit : 8 or 16 bit
 *   rate: frequency
 *   ch  : Mono or Stereo
 *   able: ��ǽ���ɤ����ξ��֤���������
 */
int mus_pcm_check_ability(int bit, int rate, int ch, boolean *able) {
	if (!prv.pcm_valid) {
		*able = FALSE;
		return NG;
	}
	*able = TRUE;
	return OK;
}

/*
 * PCM ��ͭ��/̵�� �ե饰�μ���
 *   return: FALASE -> ̵��
 *           TRUE   -> ͭ��
 */
boolean mus_pcm_get_state() {
	return prv.pcm_valid;
}

/*
 * �ե����ɳ���
 *   device: �ե����ɤ���ǥХ���(MIX_MAXTER/MIX_PCM/....)
 *   time  : �ǽ��ܥ�塼��ޤǤ�ã�������(msec)
 *   volume: �ǽ��ܥ�塼��
 *   stop:   �ե����ɽ�λ���˱��դ򥹥ȥåפ��뤫�ɤ�����
 *           0: ���ʤ�
 *           1: ����
 */ 
int mus_mixer_fadeout_start(int device, int time, int volume, int stop) {
	printf("%s(%d, %d, %d, %d) not implemented\n", __func__, device, time, volume, stop);
	return NG;
}

/*
 * ����ΥǥХ��������ߥե������椫�ɤ�����Ĵ�٤�
 *   device: ����ǥХ���
 *
 *   return: TRUE  -> �ե�������
 *           FALSE -> �ե�������Ǥʤ�
 */
boolean mus_mixer_fadeout_get_state(int device) {
	printf("%s(%d) not implemented\n", __func__, device);
	return FALSE;
}

/*
 * ����ΥǥХ����Υե����ɤ�����ǻߤ��
 *   device: ����ǥХ���
 */
int mus_mixer_fadeout_stop(int device) {
	printf("%s not implemented\n", __func__);
	return NG;
}

/*
 * ����ΥǥХ����Υߥ�������٥���������
 *   device: ����ǥХ���
 *
 *   return: �ߥ�������٥�(0 - 100) (������������ꤵ�줿��)
 */
int mus_mixer_get_level(int device) {
	printf("%s not implemented\n", __func__);
	return 0;
}

/*
 * ����Υ����ͥ�� wave file �����
 *   ch : channel (0-127)
 *   num: �ե������ֹ� (1-65535)
 */
int mus_wav_load(int ch, int num) {
	if (!prv.pcm_valid) return NG;

	if (ch < 0 || ch > 128) return NG;
	return muspcm_load_no(ch + 1, num);
}

/*
 * ����Υ����ͥ뤫�� wave file ���˴�
 *   ch : channel
 */
int mus_wav_unload(int ch) {
	if (!prv.pcm_valid) return NG;

	if (ch < 0 || ch > 128) return NG;
	return muspcm_unload(ch + 1);
}

/*
 * WAV �α��ճ��� (wavXXXX)
 *   ch  : ������������ͥ� (0-127)
           (���餫���� mus_wav_load��load���Ƥ���)
 *   loop: �����֤����       (0�ξ���̵��, ����ʳ��ϣ���Τ�)
 */
int mus_wav_play(int ch, int loop) {
	if (!prv.pcm_valid) return NG;

	if (ch < 0 || ch > 128) return NG;
	return muspcm_start(ch + 1, loop);
}

/*
 * ����Υ����ͥ��WAV�α������ (wavXXX)
 *   ch: channel
 */
int mus_wav_stop(int ch) {
	if (!prv.pcm_valid) return NG;

	if (ch < 0 || ch > 128) return NG;
	return muspcm_stop(ch + 1);
}

/*
 * ����Υ����ͥ�α��վ��֤μ���
 *   ch: channel (0-127)
 *   
 *   return: ���ջ���(msec) 65535ms ��˰��
 */
int mus_wav_get_playposition(int ch) {
	return muspcm_getpos(ch + 1);
}

/*
 * ����Υ����ͥ��WAV�Υե�����
 *   ch: channel(0-127)
 *   time  : �ǽ��ܥ�塼��ޤǤ�ã�������(msec)
 *   volume: �ǽ��ܥ�塼��
 *   stop  : �ե����ɽ�λ���˱��դ򥹥ȥåפ��뤫�ɤ�����
 *             0: ���ʤ�
 *             1: ����
 */
int mus_wav_fadeout_start(int ch, int time, int volume, int stop) {
	printf("%s not implemented\n", __func__);
	return NG;
}

/*
 * ����Υ����ͥ�Υե����ɤ�����ǻߤ��
 *   ch: channel (0-127)
 */
int mus_wav_fadeout_stop(int ch) {
	printf("%s not implemented\n", __func__);
	return NG;
}

/*
 * ����Υ����ͥ뤬���ߥե������椫�ɤ�����Ĵ�٤�
 *   ch: channel
 *
 *   return: TRUE  -> �ե�������
 *           FALSE -> �ե�������Ǥʤ�
 */
boolean mus_wav_fadeout_get_state(int ch) {
	printf("%s not implemented\n", __func__);
	return FALSE;
}

/*
 * ����Υ����ͥ�κ�������λ����ޤ��Ԥ�
 *   ch: channel (0-127)
 */
int mus_wav_waitend(int ch) {
	if (!prv.pcm_valid) return NG;

	if (ch < 0 || ch > 128) return NG;
	return muspcm_waitend(ch + 1);
}

/*
 * ����Υ����ͥ�ǻ����Ԥ�
 *     �������Ƥ��ʤ��ʤ餹������롣���ޥ�ɤ�ȯ�Ԥ��줿�ִ֤˱������
 *     ����С����դ����äƤ������ַв᤹��ޤ��Ԥġ�
 *   ch  : channel (0-127)
 *   time: �Ԥ�����(msec)
 */
int mus_wav_waittime(int ch, int time) {
	printf("%s not implemented\n", __func__);
	return NG;
}

/*
 * ����Υ����ͥ��WAV�ǡ����α��ջ��֤μ���
 *   ch: channel
 *   
 *   return: ����(msec) 65535ms ��˰��
 */
int mus_wav_wavtime(int ch) {
	return muspcm_getwavelen(ch + 1);
}

/*
 * ����Υ����ͥ�� wave file ��LRȿž���ƥ���
 *   ch : channel (0-127)
 *   num: �ե������ֹ� (1-65535)
 */
int mus_wav_load_lrsw(int ch, int num) {
	printf("%s not implemented\n", __func__);
	return NG;
}

int mus_bgm_play(int no, int time, int vol) {
	printf("%s not implemented\n", __func__);
	return NG;
}

int mus_bgm_stop(int no, int time) {
	printf("%s not implemented\n", __func__);
	return NG;
}

int mus_bgm_stopall(int time) {
	printf("%s not implemented\n", __func__);
	return NG;
}

int mus_bgm_fade(int no, int time, int vol) {
	printf("%s not implemented\n", __func__);
	return NG;
}

int mus_bgm_getpos(int no) {
	printf("%s not implemented\n", __func__);
	return 0;
}

int mus_bgm_getlength(int no) {
	printf("%s not implemented\n", __func__);
	return 0;
}

int mus_bgm_wait(int no, int timeout) {
	printf("%s not implemented\n", __func__);
	return NG;
}

int mus_bgm_waitpos(int no, int index) {
	printf("%s not implemented\n", __func__);
	return NG;
}

int mus_vol_set_valance(int *vols, int num) {
	printf("%s not implemented\n", __func__);
	return NG;
}
