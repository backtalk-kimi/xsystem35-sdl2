/*
 * sact.c: SACT
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
/* $Id: SACT.c,v 1.10 2004/10/31 04:18:02 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <glib.h>

#include "portab.h"
#include "system.h"
#include "imput.h"
#include "xsystem35.h"
#include "gametitle.h"
#include "message.h"
#include "nact.h"
#include "sact.h"
#include "sprite.h"
#include "sactcg.h"
#include "sactstring.h"
#include "sactsound.h"
#include "sactbgm.h"
#include "sactcrypto.h"
#include "sactchart.h"
#include "ngraph.h"
#include "surface.h"
#include "sactamask.h"

/*
  MT���ޥ�ɤ����ꤵ�줿ʸ����ˤ�äơ��С������֤ΰ㤤��ۼ�

  Version 1.0 : �������쥤�䡼
          1.1 : Rance5D
          1.2(����): �ʤߤ�����
          1.2(���): SACT��ȯ���å�, �����롦���쥤��, NightDemon
*/ 

/*
  �ʤߤ�������������

  ��å����������å�(���ɡ�̤�ɴط��ʤ������å�) -> Ctrl
  ��ư��å��������� -> A����
    A�����򲡤����Ȥǡ���ưŪ�˥�å�������������ǹԤ��ޤ���(��ư��å�����
    ���꤬ON�ˤʤ�ȡ���å�����������ɤα����ˤ��������Ԥ���������(>>>>)��
    �ä��ޤ������������ϡ�����դ⤷���ϲ�����λ�ޤǡ�A�����򲡤�³����
    ����������A������Υ�����Ȥ��ˡ������Ԥ��������뤬ɽ�������в�����줿
    ���Ȥˤʤ�ޤ���
  �Хå��� -> �ۥ�����
*/


/*
   wNum ���ϰϤʤɡ������Υ����å��ϳƥ��֥����ƥ��Ǥ��롣


������ǧ����
  SACT.CreateSprite �Ϥ��θƤФ줿�ִ֤�CG�����Ѥ����
    ->SACT.Draw�ޤǤ�CG���ѹ�����Ƥ⡢Create����CG�����
  OutputMessage�ϥ�å�������ɽ������ޤǤ�ɤäƤ��ʤ���
  QuakeScreen�Ͻ�λ�ޤǤ�ɤäƤ��ʤ���(����ȴ������)
  ��ư���ԤϤ��ʤ�
  OutputMessage �����ƻϤ�ƣ�ʸ���Ť������Ԥ���NewLine�ǤϽ񤫤ʤ���
  ~KEY 2: ��ľ�������Ƥ�SP_MOVE����λ����ޤǥ����å����ץ饤������ȿ�����ʤ�
  drag��˥����å����ץ饤�Ȥ�ȿ�����ʤ�
  ���˥᡼����󥹥ץ饤�ȤϤĤͤ�ư��
  �����å����ץ饤�Ȥϡ��ܥ��󤬲������֤ǥ��ץ饤��������ä��褿�Ȥ��⡢
  cg3���Ѳ����롣���ΤȤ��ܥ����Υ���Ƥ� SpriteKeyWait��ȴ���ʤ���
  �ޤ����ܥ���򲡤����ޤޥ��ץ饤�ȳ��˽ФƤ⡢�Ф����cg1���Ѳ�����
  GETA/B��ե������������CG2�˥ܥ��󲡲���CG3��
  SWPUT�ϥܥ��󲡲���ȴ����
*/


// SACT ��Ϣ�ξ���
sact_t sactprv;
extern char *xsys35_sact01;

/**
 * SACT.Init (1.0~)
 *   SACT���Τν����
 */
void Init() {
	int p1 = getCaliValue(); /* ISys3x */
	
	// �����ॿ���ȥ�ˤ��С����������
	if (0 == strcmp(nact->game_title_name, GT_ESUKA)) {
		sact.version = 100;
	} else if (0 == strcmp(nact->game_title_name, GT_RANCE5D)){
		sact.version = 110;
	} else {
		sact.version = 120;
	}
	
	NOTICE("SACT version = %d\n", sact.version);
	
	// �����ɸ����
	sact.origin.x = 0;
	sact.origin.y = 0;
	
	// �ƥ��֥����ƥ�����
	sp_init();
	sstr_init();
	ssel_init();
	stimer_init();
	ssnd_init();
	
	if (nact->files.sact01) {
		smask_init(nact->files.sact01);
	}
	
	// create depth map
	sact.dmap = sf_create_pixel(sf0->width, sf0->height, 16);
	
	// ����¾ System35 �Υǥե����ư����ѹ�
	nact->ags.font->antialiase_on = TRUE;
	sys_setHankakuMode(2); // ����Ⱦ���Ѵ�̵��
	ags_autorepeat(FALSE); // key auto repeat off
	
	if (sact.version >= 120) {
		sact.logging = TRUE;
	} else {
		sact.logging = FALSE;
	}
	
	DEBUG_COMMAND("SACT.Init %d:\n", p1);
}

/**
 * SACT.CreateSprite (1.0~)
 *   ���ץ饤�Ⱥ���
 *   @param wNum: ���ץ饤���ֹ�
 *   @param wNumCG1: �̾�ɽ������ã��ֹ�
 *   @param wNumCG2: �ޥ������������Ťͤ��Ȥ��Σã��ֹ�
 *   @param wNumCG3: ����å������Ȥ��Σã��ֹ�
 *   @param wType: ���ץ饤�ȤΥ�����
 */
void CreateSprite() {
	int wNum    = getCaliValue();
	int wNumCG1 = getCaliValue();
	int wNumCG2 = getCaliValue();
	int wNumCG3 = getCaliValue();
	int wType   = getCaliValue();
	
	sp_new(wNum, wNumCG1, wNumCG2, wNumCG3, wType);
	
	DEBUG_COMMAND_YET("SACT.CreateSprite %d,%d,%d,%d,%d:\n", wNum, wNumCG1, wNumCG2, wNumCG3, wType);
}

/**
 * SACT.CreateTextSprite (1.0~)
 *   ��å�������ɽ�����륹�ץ饤�Ȥκ���
 *   @param wNum: ���ץ饤���ֹ�
 *   @param wX: ɽ�����֤Σغ�ɸ
 *   @param wY: ɽ�����֤Σٺ�ɸ
 *   @param wWidth: ���ץ饤�Ȥ���
 *   @param wHeight: ���ץ饤�Ȥι⤵
 */
void CreateTextSprite() {
	int wNum = getCaliValue();
	int wX   = getCaliValue();
	int wY   = getCaliValue();
	int wWidth  = getCaliValue();
	int wHeight = getCaliValue();
	
	sp_new_msg(wNum, wX, wY, wWidth, wHeight);
	
	DEBUG_COMMAND_YET("SACT.CreateTextSprite %d,%d,%d,%d,%d:\n", wNum, wX, wY, wWidth, wHeight);
}

/**
 * SACT.SetWallPaper (1.0~)
 *   �ɻ�(�����ط�)�Ȥ���ɽ������ãǤ�����
 *   @param wNum: �ɻ�(�ط�)�Ȥ���ɽ������ãǤ��ֹ�
 */
void SetWallPaper() {
	int wNum = getCaliValue();
	
	sp_set_wall_paper(wNum);
	
	DEBUG_COMMAND_YET("SACT.SetWallPaper %d:\n", wNum);
}

/**
 * SACT.Clear (1.0~)
 *   �����ץ饤�Ⱥ��(~SP_CLR)
 */
void Clear() {
	sp_free_all();
	
	DEBUG_COMMAND_YET("SACT.Clear:\n");
}

/**
 * SACT.Delete (1.0~)
 *   ���ץ饤�Ȥκ��
 *   @param wNum: ������륹�ץ饤���ֹ�
 */
void Delete() {
	int wNum = getCaliValue();
	
	sp_free(wNum);
	
	DEBUG_COMMAND_YET("SACT.Delete %d:\n", wNum);
}

/**
 * SACT.SpriteDeleteCount (1.0~)
 *   wNum�֤���wCount�Ĥ��ϰϤΥ��ץ饤�Ȥκ��
 *   @param wNum: ��Ƭ���ץ饤���ֹ�
 *   @param wCount: �ϰ�
 */
void SpriteDeleteCount() {
	int wNum   = getCaliValue();
	int wCount = getCaliValue();
	int i;
	
	for (i = wNum; i < (wNum + wCount); i++) {
		sp_free(i);
	}
	
	DEBUG_COMMAND_YET("SACT.SpriteDeleteCount %d,%d:\n", wNum, wCount);
}

/**
 * SACT.Draw (1.0~)
 *   �������ꤵ��Ƥ��륹�ץ饤�Ⱦ��֤���̤�ȿ��(~SP_UPDATE)
 */
void Draw() {
	sp_update_all(TRUE);

	DEBUG_COMMAND_YET("SACT.Draw:\n");
}

/**
 * SCAT.DrawEffect (1.0~)
 *   ���̻����դ����̹���
 *   @param wType: ���ե����ȥ�����
 *   @param wEffectTime: ���ե����Ȥλ���(1/100��ñ��)
 *   @param wEffectKey: ����ȴ������ (1.1~) (1��ͭ��)
 */
void DrawEffect() {
	int wType       = getCaliValue();
	int wEffectTime = getCaliValue();
	int wEffectkey = 1;
	
	if (sact.version >= 110) {
		wEffectkey = getCaliValue();
	}
	
	sp_eupdate(wType, wEffectTime, wEffectkey);
	
	DEBUG_COMMAND_YET("SACT.DrawEffect %d,%d,%d:\n", wType, wEffectTime, wEffectkey);
}

/**
 * SCAT.DrawEffectAlphaMap (1.1~)
 *   ���ޥ����Ĥ����̹���
 *   @param nIndexAlphaMap: �ޥ������ֹ�
 *   @param wEffectTime: ���ե����Ȥλ���(1/100��ñ��)
 *   @param wEffectKey: ����ȴ������
 */
void DrawEffectAlphaMap() {
	int nIndexAlphaMap = getCaliValue();
	int wEffectTime = getCaliValue();
	int wEffectKey  = getCaliValue();
	
	sp_eupdate_amap(nIndexAlphaMap, wEffectTime, wEffectKey);
	
	DEBUG_COMMAND_YET("SACT.DrawEffectAlphaMap %d,%d,%d:\n", nIndexAlphaMap, wEffectTime, wEffectKey);
}

/**
 * SCAT.QuakeScreen (1.0~)
 *   �����ɤ餷
 *   @param wType: 0=�Ĳ�, 1:��ž
 *   @param wParam1: wType=0�ΤȤ�x�����ο���
 *                   wType=1�ΤȤ�����
 *   @param wParam2: wType=0�ΤȤ�y�����ο���
 *                   wType=1�ΤȤ���ž��
 *   @param wCount: ����(1/100��)
 *   @param nfKeyEnable: ����ȴ�� (1��ͭ��) (1.1~) 
 */
void QuakeScreen() {
	int wType   = getCaliValue();
	int wParam1 = getCaliValue();
	int wParam2 = getCaliValue();
	int wCount  = getCaliValue();
	int nfKeyEnable = 1;
	
	if (sact.version >= 110) {
		nfKeyEnable = getCaliValue();
	}
	
	sp_quake_screen(wType, wParam1, wParam2, wCount, nfKeyEnable);
	
	DEBUG_COMMAND_YET("SACT.QuakeScreen %d,%d,%d,%d,%d:\n", wType, wParam1, wParam2, wCount, nfKeyEnable);
}

/**
 * SACT.SetOrigin (1.0~)
 *   ����ɸ�ѹ�
 *   @param wX: �����ˤ���غ�ɸ�ΰ���
 *   @param wY: �����ˤ���ٺ�ɸ�ΰ���
 */
void SetOrigin() {
	int wX = getCaliValue();
	int wY = getCaliValue();
	
	sact.origin.x = wX;
	sact.origin.y = wY;
	
	DEBUG_COMMAND_YET("SACT.SetOrigin %d,%d:\n", wX, wY);
}

/**
 * SACT.SetShow (1.0~)
 *   ���ץ饤�Ȥ�ɽ�����֤��ѹ�
 *   @param wNum: ��Ƭ�Υ��ץ饤���ֹ�
 *   @param wCount: ɽ������Ŀ�
 *   @param wShow: 0:��ɽ��, 1:ɽ��
 */
void SetShow() {
	int wNum   = getCaliValue();
	int wCount = getCaliValue();
	int wShow  = getCaliValue();
	
	sp_set_show(wNum, wCount, wShow);

	DEBUG_COMMAND_YET("SACT.SetShow %d,%d,%d:\n", wNum, wCount, wShow);
}

/**
 * SACT.SetBlendRate (1.1~)
 *   ���ץ饤�Ȥ�ɽ�����֤��ѹ�
 *   @param wNum: ��Ƭ�Υ��ץ饤���ֹ�
 *   @param wCount: ɽ������Ŀ�
 *   @param nBlendRate: �֥���Ψ
 */
void SetBlendRate() {
	int wNum   = getCaliValue();
	int wCount = getCaliValue();
	int nBlendRate  = getCaliValue();
	
	sp_set_blendrate(wNum, wCount, nBlendRate);
	
	DEBUG_COMMAND_YET("SACT.SetBlendRate %d,%d,%d:\n", wNum, wCount, nBlendRate);
}

/**
 * SACT.SetPos (1.0~)
 *   ���ץ饤�Ȥ�ɽ�����֤�����(�ѹ�)
 *   @param wNum: ���ץ饤���ֹ�
 *   @param wX: ɽ���غ�ɸ
 *   @param wY: ɽ���ٺ�ɸ
 */
void SetPos() {
	int wNum = getCaliValue();
	int wX  = getCaliValue();
	int wY  = getCaliValue();
	
	sp_set_pos(wNum, wX, wY);
	
	DEBUG_COMMAND_YET("SACT.SetPos %d,%d,%d:\n", wNum, wX, wY);
}

/**
 * SACT.SetMove (1.0~)
 *   ���ץ饤�Ȥΰ�ư (�����˽������᤹)
 *   @param wNum: ���ץ饤���ֹ�
 *   @param wX: ɽ���غ�ɸ
 *   @param wY: ɽ���ٺ�ɸ
 */
void SetMove() {
	int wNum = getCaliValue();
	int wX   = getCaliValue();
	int wY   = getCaliValue();
	
	sp_set_move(wNum, wX, wY);
	
	DEBUG_COMMAND_YET("SACT.SetMove %d,%d,%d:\n", wNum, wX, wY);
}

/**
 * SACT.SetMoveTime (1.0~)
 *   SetMove�ˤ�륹�ץ饤�Ȱ�ư�λ��֤�����
 *   @param wNum: ���ץ饤���ֹ�
 *   @param wTime: ��ư��λ����ޤǤλ���(1/100��ñ��)
 */
void SetMoveTime() {
	int wNum  = getCaliValue();
	int wTime = getCaliValue();
	
	sp_set_movetime(wNum, wTime);
	
	DEBUG_COMMAND_YET("SACT.SetMoveTime %d,%d:\n", wNum, wTime);
}

/**
 * SACT.SetMoveSpeed (1.0~)
 *   SetMove�ˤ�륹�ץ饤�Ȱ�ư��®�٤�����
 *   @param wNum: ���ץ饤���ֹ�
 *   @param wSpeed: ��ư®��(�ǥե���Ȥ�100%�Ȥ���%����)
 */
void SetMoveSpeed() {
	int wNum   = getCaliValue();
	int wSpeed = getCaliValue();
	
	sp_set_movespeed(wNum, wSpeed);
	
	DEBUG_COMMAND_YET("SACT.SetMoveSpeed %d,%d:\n", wNum, wSpeed);
}

/**
 * SACT.SetMoveSpeedCount (1.0~)
 *   ʣ���Υ��ץ饤�Ȥ��Ф��륹�ץ饤�Ȱ�ư��®�٤�����
 *   @param wNum: ��Ƭ���ץ饤���ֹ�
 *   @param wCount: �ϰ�
 *   @param wSpeed: ��ư®��(�ǥե���Ȥ�100%�Ȥ���%����)
 */
void SetMoveSpeedCount() {
	int wNum   = getCaliValue();
	int wCount = getCaliValue();
	int wSpeed = getCaliValue();
	int i;
	
	for (i = wNum; i < (wNum + wCount); i++) {
		sp_set_movespeed(i, wSpeed);
	}
	
	DEBUG_COMMAND_YET("SACT.SetMoveSpeedCount %d,%d,%d:\n", wNum, wCount, wSpeed);
}

/**
 * SACT.SetSpriteAnimeTimeInterval (1.1~)
 *   ʣ���Υ��ץ饤�Ȥ��Ф��륢�˥᡼����󥹥ץ饤�Ȥδֳ�
 *   @param wNum: ��Ƭ���ץ饤���ֹ�
 *   @param wCount: �ϰ�
 *   @param nTime: �ֳ� 
 */
void SetSpriteAnimeTimeInterval() {
	int wNum   = getCaliValue();
	int wCount = getCaliValue();
	int nTime  = getCaliValue();
	int i;
	
	for (i = wNum; i < (wNum + wCount); i++) {
		sp_set_animeinterval(i, nTime);
	}
	
	DEBUG_COMMAND_YET("SACT.SetSpriteAnimeTimeInterval %d,%d,%d:\n", wNum, wCount, nTime);
}

/**
 * SACT.AddZKeyHideSprite (1.0~)
 *   ���������Ԥ���Z�����������줿�Ȥ���ɽ��OFF�ˤʤ륹�ץ饤�Ȥ���Ͽ
 *   @param wNum: ���ץ饤���ֹ�
 */
void AddZKeyHideSprite() {
	int wNum = getCaliValue();
	
	sp_add_zkey_hidesprite(wNum);
	
	DEBUG_COMMAND_YET("SACT.AddZKeyHideSprite %d:\n", wNum);
}

/**
 * SACT.ClearZKeyHideSprite (1.0~)
 *   AddZKeyHideSprite����Ͽ�������ץ饤���ֹ�����ƥ��ꥢ
 */
void ClearZKeyHideSprite() {
	sp_clear_zkey_hidesprite_all();
	
	DEBUG_COMMAND_YET("SACT.ClearZKeyHideSprite:\n");
}

/**
 * SACT.SpriteFreeze (1.0~)
 *   ���ץ饤�ȥ����å���wIndex�ξ��֤Ǹ��ꤷ��~KEY 2:�ʤɤ�ȿ�����ʤ�
 *   �褦�ˤ���
 *   @param wNum: ���ץ饤���ֹ�
 *   @param wIndex: ���ꤹ������ֹ�(1-3)
 */
void SpriteFreeze() {
	int wNum   = getCaliValue();
	int wIndex = getCaliValue();
	
	sp_freeze_sprite(wNum, wIndex);
	
	DEBUG_COMMAND_YET("SACT.SpriteFreeze %d,%d:\n", wNum, wIndex);
}

/**
 * SACT.SpriteThaw (1.0~)
 *   Freeze�������ץ饤�Ⱦ��֤���
 *   @param wNum: ���ץ饤���ֹ�
 */
void SpriteThaw() {
	int wNum = getCaliValue();
	
	sp_thaw_sprite(wNum);

	DEBUG_COMMAND_YET("SACT.SpriteThaw %d:\n", wNum);
}

/**
 * SACT.SpriteFreezeCount (1.0~)
 *   ʣ���Υ��ץ饤�Ȥ�Freeze����
 *   @param wNum: ��Ƭ���ץ饤���ֹ�
 *   @param wCount: �ϰ�
 *   @param wIndex: ���ꤹ������ֹ�
 */
void SpriteFreezeCount() {
	int wNum   = getCaliValue();
	int wCount = getCaliValue();
	int wIndex = getCaliValue();
	int i;

	for (i = wNum; i < (wNum + wCount); i++) {
		sp_freeze_sprite(i, wIndex);
	}
	
	DEBUG_COMMAND_YET("SACT.SpriteFreezeCount %d,%d,%d:\n", wNum, wCount, wIndex);
}

/**
 * SACT.SpriteThawCount (1.0~)
 *    ʣ����Freeze���ץ饤�Ⱦ��֤β��
 *    @param wNum: ��Ƭ���ץ饤���ֹ�
 *    @param wCount: �ϰ�
 */
void SpriteThawCount() {
	int wNum   = getCaliValue();
	int wCount = getCaliValue();
	int i;
	
	for (i = wNum; i < (wNum + wCount); i++) {
		sp_thaw_sprite(i);
	}
	
	DEBUG_COMMAND_YET("SACT.SpriteThawCount %d,%d:\n", wNum, wCount);
}

/**
 * SACT.QuakeSpriteAdd (1.0~)
 *   QuakeSprite���ɤ餹���ץ饤�Ȥ��ɲ�
 *   @param wNum: ���ץ饤���ֹ�
 */
void QuakeSpriteAdd() {
	int wNum = getCaliValue();
	
	sp_add_quakesprite(wNum);
	
	DEBUG_COMMAND_YET("SACT.QuakeSpriteAdd %d:\n", wNum);
}

/**
 * SACT.QuakeSpriteReset (1.0~)
 *   QuakeSpriteAdd����������Ʋ��
 */
void QuakeSpriteReset() {
	sp_clear_quakesprite_all();
	DEBUG_COMMAND_YET("SACT.QuakeSpriteReset:\n");
}

/**
 * SACT.QuakeSprite (1.0~)
 *   QuakeSpriteAdd�����ꤷ�����ץ饤�Ȥ��ɤ餹
 *   @param wType: 0:�Ĳ�(���ƤΥ��ץ饤�Ȥ�Ʊ���褦���ɤ餹)
 *                 1:�Ĳ�(���ƤΥ��ץ饤�Ȥ�Х�Х���ɤ餹)
 *   @param wAmplitudeX: �������ο���
 *   @param wAmplitudeY: �������ο���
 *   @param wCount: ����(1/100��)
 *   @param nfKeyEnable: (1.1~): ��������󥻥뤢��(=1)
 */
void QuakeSprite() {
	int wType       = getCaliValue();
	int wAmplitudeX = getCaliValue();
	int wAmplitudeY = getCaliValue();
	int wCount      = getCaliValue();
	int nfKeyEnable = 0;

	if (sact.version >= 110) {
		nfKeyEnable = getCaliValue();
	}
	
	sp_quake_sprite(wType, wAmplitudeX, wAmplitudeY, wCount, nfKeyEnable);
	
	DEBUG_COMMAND_YET("SACT.QuakeSprite %d,%d,%d,%d:\n", wType, wAmplitudeX, wAmplitudeY, wCount);
}

/**
 * SACT.QuerySpriteIsExist (1.0~)
 *  ����Υ��ץ饤�Ȥ���Ͽ����Ƥ��뤫�ɤ��������
 *  @param wNum: ���ץ饤���ֹ�
 *  @param var: ��Ͽ���֤��֤��ѿ� 0: ̤��Ͽ, 1:����Ͽ
 */
void QuerySpriteIsExist() {
	int wNum = getCaliValue();
	int *var = getCaliVariable();

	sp_query_isexist(wNum, var);
	
	DEBUG_COMMAND_YET("SACT.QuerySpriteIsExist %d,%p:\n", wNum, var);
}

/**
 * SACT.QuerySpriteInfo (1.0~)
 *   ���ץ饤�Ȥξ�������
 *   @param wNum: ���ץ饤���ֹ�
 *   @param vType: ���ץ饤�ȥ����ס��ƥ����ȥ��ץ饤�Ȥξ���100
 *   @param vCG1: �ãǣ�(�ʤ����ϣ�)
 *   @param vCG2: �ãǣ�(�ʤ����ϣ�)
 *   @param vCG3: �ãǣ�(�ʤ����ϣ�)
 */
void QuerySpriteInfo() {
	int wNum   = getCaliValue();
	int *vType = getCaliVariable();
	int *vCG1  = getCaliVariable();
	int *vCG2  = getCaliVariable();
	int *vCG3  = getCaliVariable();
	
	sp_query_info(wNum, vType, vCG1, vCG2, vCG3);
	
	DEBUG_COMMAND_YET("SACT.QuerySpriteInfo %d,%p,%p,%p,%p:\n", wNum, vType, vCG1, vCG2, vCG3);
}

/**
 * SACT.QuerySpriteShow (1.0~)
 *   ���ץ饤�Ȥ�ɽ������(SP_SHOW����)�����
 *   @param wNum: ���ץ饤���ֹ�
 *   @param vShow: 0:��ɽ��, 1:ɽ��
 */
void QuerySpriteShow() {
	int wNum = getCaliValue();
	int *vShow = getCaliVariable();

	sp_query_show(wNum, vShow);
	
	DEBUG_COMMAND_YET("SACT.QuerySpriteShow %d,%p:\n", wNum, vShow);
}

/**
 * SACT.QuerySpritePos (1.0~)
 *   ���ץ饤�Ȥ�ɽ�����֤μ���
 *   @param wNum: ���ץ饤���ֹ�
 *   @param vX: �غ�ɸ
 *   @param vY: �ٺ�ɸ
 */
void QuerySpritePos() {
	int wNum = getCaliValue();
	int *vX  = getCaliVariable();
	int *vY  = getCaliVariable();
	
	sp_query_pos(wNum, vX, vY);
	
	DEBUG_COMMAND_YET("SACT.QuerySpritePos %d,%p,%p:\n", wNum, vX, vY);
}

/**
 * SACT.QuerySpriteSize (1.0~)
 *   ���ץ饤�Ȥ��礭���μ���
 *   @param wNum: ���ץ饤���ֹ�
 *   @param vWidth: ���ץ饤�Ȥ���
 *   @param vHeight: ���ץ饤�Ȥι⤵
 */
void QuerySpriteSize() {
	int wNum     = getCaliValue();
	int *vWidth  = getCaliVariable();
	int *vHeight = getCaliVariable();
	
	sp_query_size(wNum, vWidth, vHeight);
	
	DEBUG_COMMAND_YET("SACT.QuerySpriteSize %d,%p,%p:\n", wNum, vWidth, vHeight);
}

/**
 * SACT.QueryTextPos (1.2~)
 *   ��å��������ץ饤�Ȥ�ʸ������
 *   @param nMesSpID:
 *   @param pwX:
 *   @param pwY:
 */
void QueryTextPos() {
	int wNum = getCaliValue();
	int *vX  = getCaliVariable();
	int *vY  = getCaliVariable();
	
	sp_query_textpos(wNum, vX, vY);
	
	DEBUG_COMMAND_YET("SACT.QueryTextPos %d,%p,%p:\n", wNum, vX, vY);
}

/**
 * SCAT.CG_Clear (1.0~)
 *   CG_Create�Ǻ�������CG�����ƺ��
 */
void CG_Clear() {
	scg_freeall();
	
	DEBUG_COMMAND_YET("SACT.CG_Clear:\n");
}

/**
 * SACT.CG_Reset (1.0~)
 *   CG_Create�Ǻ�������CG����
 *   @param wNumCG: �������CG�ֹ�
 */
void CG_Reset() {
	int wNumCG = getCaliValue();
	
	scg_free(wNumCG);
	
	DEBUG_COMMAND_YET("SACT.CG_Reset %d:\n", wNumCG);
}

/**
 * SACT.CG_QueryType (1.0~)
 *   CG�ξ���(CG�Υ�����)�����
 *   @param wNumCG: CG�ֹ�
 *   @param vType: CG�μ���, 0: ̤����, 1:��󥯤���Ƥ���, 2: CG_SET�Ǻ���
 *                 3: CG_REVERSE�Ǻ���, 4: CG_STRETCH�Ǻ���
 */
void CG_QueryType() {
	int wNumCG = getCaliValue();
	int *vType = getCaliVariable();
	
	scg_querytype(wNumCG, vType);
	
	DEBUG_COMMAND_YET("SACT.CG_QueryType %d,%p:\n", wNumCG, vType);
}

/**
 * SACT.CG_QuerySize (1.0~)
 *   CG���礭�������
 *   @param wNumCG: CG�ֹ�
 *   @param vWidth: ��
 *   @param vHeight: �⤵
 */
void CG_QuerySize() {
	int wNumCG   = getCaliValue();
	int *vWidth  = getCaliVariable();
	int *vHeight = getCaliVariable();
	
	scg_querysize(wNumCG, vWidth, vHeight);
	
	DEBUG_COMMAND_YET("SACT.CG_QuerySize %d,%p,%p:\n", wNumCG, vWidth, vHeight);
}

/**
 * SACT.CG_QueryBpp (1.0~)
 *   CG��bpp�����
 *   @param wNumCG: CG�ֹ�
 *   @param vBpp: CG��bpp
 */
void CG_QueryBpp() {
	int wNumCG = getCaliValue();
	int *vBpp  = getCaliVariable();
	
	scg_querybpp(wNumCG, vBpp);
	
	DEBUG_COMMAND_YET("SACT.CG_QueryBpp %d,%p:\n", wNumCG, vBpp);
}

/**
 * SACT.CG_ExistAlphaMap (1.0~)
 *   CG�Φ��ޥå�(�ޥ���)������У����ʤ���У�
 *   @param wNumCG: CG�ֹ�
 *   @param vMask: 0/1
 */
void CG_ExistAlphaMap() {
	int wNumCG = getCaliValue();
	int *vMask = getCaliVariable();
	
	scg_existalphamap(wNumCG, vMask);
	
	DEBUG_COMMAND_YET("SACT.CG_ExistAlphaMap %d,%p:\n", wNumCG, vMask);
}

/**
 * SACT.CG_Create (1.0~)
 *   ���ꥵ�����������֥���Ψ�λͳѤ�ɽ������
 *   @param wNumCG: CG�ֹ�
 *   @param wWidth: ��
 *   @param wHeight: �⤵
 *   @param wR: RGB�ͤ���(0-255)
 *   @param wG: RGB�ͤ���(0-255)
 *   @param wB: RGB�ͤ���(0-255)
 *   @param wBlendRate: �֥���Ψ(0-255)
 */
void CG_Create() {
	int wNumCG     = getCaliValue();
	int wWidth     = getCaliValue();
	int wHeight    = getCaliValue();
	int wR         = getCaliValue();
	int wG         = getCaliValue();
	int wB         = getCaliValue();
	int wBlendRate = getCaliValue();
	
	scg_create(wNumCG, wWidth, wHeight, wR, wG, wB, wBlendRate);
	
	DEBUG_COMMAND_YET("SACT.CG_Create %d,%d,%d,%d,%d,%d,%d:\n", wNumCG, wWidth, wHeight, wR, wG, wB, wBlendRate);
}

/**
 * SACT,CG_CreateReverse (1.0~)
 *   ���ˤʤ�CG��ȿž����CG���������
 *   @param wNumCG: CG�ֹ�
 *   @param wNumSrcCG: ���ԡ��θ��ˤʤ�CG���ֹ�
 *   @param wReverseX: X������ȿž�����å�(0:ȿž���ʤ���1:ȿž����)
 *   @param wReverseY: Y������ȿž�����å�(0:ȿž���ʤ���1:ȿž����)
 */
void CG_CreateReverse() {
	int wNumCG = getCaliValue();
	int wNumSrcCG = getCaliValue();
	int wReverseX = getCaliValue();
	int wReverseY = getCaliValue();

	scg_create_reverse(wNumCG, wNumSrcCG, wReverseX, wReverseY);
	
	DEBUG_COMMAND_YET("SACT.CG_CreateReverse %d,%d,%d,%d:\n", wNumCG, wNumSrcCG, wReverseX, wReverseY);
}

/**
 * SACT.CG_CreateStretch (1.0~)
 *   ���ˤʤ�CG�����⤷���Ͻ̾�����CG���������
 *   @param wNumCG: CG�ֹ�
 *   @param wWidth: ��������CG����
 *   @param wHeight: ��������CG�ι⤵
 *   @param wNumSrcCG: ���ˤʤ�CG���ֹ�
 */
void CG_CreateStretch() {
	int wNumCG    = getCaliValue();
	int wWidth    = getCaliValue();
	int wHeight   = getCaliValue();
	int wNumSrcCG = getCaliValue();

	scg_create_stretch(wNumCG, wWidth, wHeight, wNumSrcCG);
	
	DEBUG_COMMAND_YET("SACT.CG_CreateStretch %d,%d,%d,%d:\n", wNumCG, wWidth, wHeight, wNumSrcCG);
}

/**
 * SACT.CG_CreateBlend (1.0~)
 *   �����CG�򤫤��ͤ��碌��CG�����
 *   @param wNumDstCG: CG�ֹ�(������)
 *   @param wNumBaseCG: �Ť͹�碌�Τ�ȤȤʤ�CG
 *   @param wX: �Ť͹�碌����֤Σغ�ɸ
 *   @param wY: �Ť͹�碌����֤Σٺ�ɸ
 *   @param wNumBlendCG: ��˽Ť͹�碌��CG
 *   @param wAlphaMapMode: ���ޥåפκ����⡼��
 */
void CG_CreateBlend() {
	int wNumDstCG  = getCaliValue();
	int wNumBaseCG = getCaliValue();
	int wX = getCaliValue();
	int wY = getCaliValue();
	int wNumBlendCG   = getCaliValue();
	int wAlphaMapMode = getCaliValue();
	
	DEBUG_COMMAND_YET("SACT.CG_CreateBlend %d,%d,%d,%d,%d,%d:\n", wNumDstCG, wNumBaseCG, wX, wY, wNumBlendCG, wAlphaMapMode);
	scg_create_blend(wNumDstCG, wNumBaseCG, wX, wY, wNumBlendCG, wAlphaMapMode);
	
}

/**
 * SACT.CG_CreateText (1.0~)
 *   ʸ���󤫤�CG�����
 *   @param wNumCG: ��������CG�ֹ�
 *   @param wSize: ʸ���ι⤵(pixel)
 *   @param wR: ʸ����R��(0-255)
 *   @param wG: ʸ����G��(0-255)
 *   @param wB: ʸ����B��(0-255)
 *   @param wText: ���褹��ʸ�����ѿ����ֹ�
 */
void CG_CreateText() {
	int wNumCG = getCaliValue();
	int wSize  = getCaliValue();
	int wR     = getCaliValue();
	int wG     = getCaliValue();
	int wB     = getCaliValue();
	int wText  = getCaliValue();
	
	scg_create_text(wNumCG, wSize, wR, wG, wB, wText);
	
	DEBUG_COMMAND_YET("SACT.CG_CreateText %d,%d,%d,%d,%d,%d:\n", wNumCG, wSize, wR, wG, wB, wText);
}

/**
 * SACT.CG_CreateTextNum (1.0~)
 *   ���ͤ��饷���ƥ�ƥ����Ȥ�CG�����
 *   @param wNumCG: ��������CG�ֹ�
 *   @param wSize: ʸ���ι⤵(pixel)
 *   @param wR: ʸ����R��(0-255)
 *   @param wG: ʸ����G��(0-255)
 *   @param wB: ʸ����B��(0-255)
 *   @param wFigs: ���
 *   @param wZeroPadding: ����������ʤ���ʬ�������뤫�ɤ����Υե饰
 *                        0:������ᤷ�ʤ� 1:������᤹��
 *   @param wValue: ���褹����
 */
void CG_CreateTextNum() {
	int wNumCG       = getCaliValue();
	int wSize        = getCaliValue();
	int wR           = getCaliValue();
	int wG           = getCaliValue();
	int wB           = getCaliValue();
	int wFigs        = getCaliValue();
	int wZeroPadding = getCaliValue();
	int wValue       = getCaliValue();
	
	scg_create_textnum(wNumCG, wSize, wR, wG, wB, wFigs, wZeroPadding, wValue);
	
	DEBUG_COMMAND_YET("SACT.CG_CreateTextNum %d,%d,%d,%d,%d,%d,%d,%d:\n", wNumCG, wSize, wR, wG, wB, wFigs, wZeroPadding, wValue);
}

/**
 * SACT.CG_Copy (1.0~)
 *   CG��ʣ��
 *   @param wNumDst: ʣ�����CG�ֹ�
 *   @param wNumSrc: ʣ�̸���CG�ֹ�
 */
void CG_Copy() {
	int wNumDst = getCaliValue();
	int wNumSrc = getCaliValue();
	
	scg_copy(wNumDst, wNumSrc);

	DEBUG_COMMAND_YET("SACT.CG_Copy %d,%d:\n", wNumDst, wNumSrc);
}

/**
 * SACT.CG_Cut (1.0~)
 *   ����CG�ΰ������ڤ�̤���CG�����
 *   @param wNumDstCG: CG�ֹ�(������)
 *   @param wNumSrcCG: CG�ֹ�(���åȸ�)
 *   @param wX: ���åȳ��ϣغ�ɸ
 *   @param wY: ���åȳ��ϣٺ�ɸ
 *   @param wWidth: ���å���
 *   @param wHeight: ���åȹ⤵
 */
void CG_Cut() {
	int wNumDstCG = getCaliValue();
	int wNumSrcCG = getCaliValue();
	int wX = getCaliValue();
	int wY = getCaliValue();
	int wWidth  = getCaliValue();
	int wHeight = getCaliValue();
	
	scg_cut(wNumDstCG, wNumSrcCG, wX, wY, wWidth, wHeight);
	
	DEBUG_COMMAND_YET("SACT.CG_Cut %d,%d,%d,%d,%d,%d:\n", wNumDstCG, wNumSrcCG, wX, wY, wWidth, wHeight);
}

/**
 * SACT.CG_PartCopy (1.0~)
 *   ����CG�ΰ������ڤ�̤���CG�������CG�Υ��������ΤϤ�ȤΤޤޤǡ�
 *   �ޥ����ǡ����Τߤ�������Ƹ��ݤ���Υ��������Ѳ�������
 *   @param wNumDstCG: CG�ֹ�(������)
 *   @param wNumSrcCG: CG�ֹ�(��)
 *   @param wX: ���ϣغ�ɸ
 *   @param wY: ���ϣٺ�ɸ
 *   @param wWidth: ���å���
 *   @param wHeight: ���åȹ⤵
 */
void CG_PartCopy() {
	int wNumDstCG = getCaliValue();
	int wNumSrcCG = getCaliValue();
	int wX = getCaliValue();
	int wY = getCaliValue();
	int wWidth  = getCaliValue();
	int wHeight = getCaliValue();
	
	scg_partcopy(wNumDstCG, wNumSrcCG, wX, wY, wWidth, wHeight);
	
	DEBUG_COMMAND_YET("SACT.PartCopy %d,%d,%d,%d,%d,%d:\n", wNumDstCG, wNumSrcCG, wX, wY, wWidth, wHeight);
}

/**
 * SACT.WiatKeySimple (1.0~)
 *   �̾省�������Ԥ�
 *   @param vKey: ���Ϥ��줿����
 */
void WaitKeySimple() {
	int *vKey = getCaliVariable();

	DEBUG_COMMAND_YET("SACT.WaitKeySimple %d:\n", vKey);

	// �Ȥꤢ����������
	sp_update_all(TRUE);
	
	sact.waittype = KEYWAIT_SIMPLE;
	sact.waitkey = -1;
	
	while(sact.waitkey == -1) {
		sys_keywait(25, TRUE);
	}
	
	sact.waittype = KEYWAIT_NONE;
	
	*vKey = sact.waitkey;
	
}

/**
 * SACT.WaitKeyMessgae (1.0~)
 *   ��å��������������Ԥ�
 *   @param wMessageMark1: ���ץ饤���ֹ�1(���˥᡼����󥹥ץ饤��)
 *   @param wMessageMark2: ���ץ饤���ֹ�2(���˥᡼����󥹥ץ饤��)
 *   @param wMessageLength: (1.2~)
 */
void WaitKeyMessage() {
	int wMessageMark1 = getCaliValue();
	int wMessageMark2 = getCaliValue();
	int wMessageLength = 0;
	
	if (sact.version >= 120) {
		wMessageLength = getCaliValue();
	}
	
	smsg_keywait(wMessageMark1, wMessageMark2, wMessageLength);
	
	DEBUG_COMMAND_YET("SACT.WaitKeyMessage %d,%d,%d:\n", wMessageMark1, wMessageMark2, wMessageLength);
}

/**
 * SACT.WaitKeySprite (1.0~)
 *   ���ץ饤�Ƚ����Ԥ�
 *   @param vOK: 0�ʤ�б�����å� 
 *   @param vRND: �������Ϸ��
 *   @param vRsv1: ͽ��
 *   @param vRsv2: ͽ��
 */
void WaitKeySprite() {
	int *vOK = getCaliVariable();
	int *vRND = getCaliVariable();
	int *vRsv1 = getCaliVariable();
	int *vRsv2 = getCaliVariable();
	
	DEBUG_COMMAND("SACT.WaitKeySprite %p,%p,%p,%p:\n", vOK, vRND, vRsv1, vRsv2);
	
	sp_keywait(vOK, vRND, vRsv1, vRsv2, NULL, -1);
	
	DEBUG_COMMAND_YET("SACT.WaitKeySprite %d,%d,%d,%d:\n", *vOK, *vRND, *vRsv1, *vRsv2);
}

/**
 * SACT.PeekKey (1.2~)
 *   ?????
 *   @param nKeyCode:
 *   @param vResult:
 */
void PeekKey() {
	int nKeyCode = getCaliValue();
	int *vResult = getCaliVariable();
	
	WARNING("NOT IMPLEMENTED\n");
	DEBUG_COMMAND_YET("SACT.PeekKey %d,%p:\n", nKeyCode, vResult);
}

/**
 * SACT.WaitKeySKipKeyUp (1.0~)
 *   ʸ�����ꥭ����������äѤʤ��λ���Υ�����ޤ��Ԥ�
 */
void WaitMsgSkipKeyUp() {
	WARNING("NOT IMPLEMENTED\n");
	DEBUG_COMMAND_YET("SACT.WaitMsgSkipKeyUp:\n");
}

/**
 * SACT.WaiKeySimpleTimeOut (1.0~)
 *   �����ॢ���ȤĤ�������������
 *   @param vRND: ���Ϥ��줿����������
 *   @param vD03: �����ॢ���Ȥ������=1, ���ʤ����=0
 *   @param wTime: �����ॢ���Ȼ��� (1/100sec)
 */
void WaitKeySimpleTimeOut() {
	int *vRND = getCaliVariable();
	int *vD03 = getCaliVariable();
	int wTime = getCaliValue();

	sact.waittype = KEYWAIT_SIMPLE;
	sact.waitkey = -1;
	
	sys_keywait(wTime * 10, TRUE);
	if (sact.waitkey == -1) {
		*vD03 = 1;
		*vRND = 0;
	} else {
		*vD03 = 0;
		*vRND = sact.waitkey;
	}
	
	sact.waittype = KEYWAIT_NONE;
	
	DEBUG_COMMAND_YET("SACT.WaitKeySimpleTimeOut %p,%p,%d:\n", vRND, vD03, wTime);
}

/**
 * SACT.WaiKeySpriteTimeOut (1.0~)
 *   �����ॢ���ȤĤ����ץ饤�ȥ����������� 
 *   @param vOK: 0 �λ�������å�
 *   @param vRND: �����å����ץ饤�Ȥ��ֹ�
 *   @param vD01: ���åȥ��ץ饤�Ȥ��ֹ�
 *   @param vD02: �ץåȥ��ץ饤�Ȥ��ֹ�
 *   @param vD03: �����ॢ���Ȥ������=1, ���ʤ����=0
 *   @param wTime: �����ॢ���Ȼ��� (1/100sec)
 */
void WaitKeySpriteTimeOut() {
	int *vOK = getCaliVariable();
	int *vRND = getCaliVariable();
	int *vD01 = getCaliVariable();
	int *vD02 = getCaliVariable();
	int *vD03 = getCaliVariable();
	int wTime = getCaliValue();
	
	sp_keywait(vOK, vRND, vD01, vD02, vD03, wTime);
	
	DEBUG_COMMAND_YET("SACT.WaitKeySpriteTimeOut %p,%p,%p,%p,%p,%d:\n", vOK, vRND, vD01, vD02, vD03, wTime);
}

/**
 * SACT.QueryMessageSkip (1.0~)
 *   ??????
 *   @param vSkip:
 */
void QueryMessageSkip() {
	int *vSkip = getCaliVariable();

	*vSkip = get_skipMode() ? 1 : 0;
	
	DEBUG_COMMAND_YET("SACT.QueryMessageSkip %p:\n", vSkip);
}

/**
 * SACT.RegistReplaceString (1.0~)
 *   ��å��������ʸ������֤�����
 *   @param sstr: �Ѵ���ʸ�����ֹ�
 *   @param dstr: �Ѵ���ʸ�����ֹ�
 */
void RegistReplaceString() {
	int sstr = getCaliValue();
	int dstr = getCaliValue();
	
	sstr_regist_replace(sstr, dstr);
	
	DEBUG_COMMAND_YET("SACT.RegistReplaceString %d,%d:\n", sstr, dstr);
}

/**
 * SACT.MessageOutput (1.0~)
 *   @param wMessageSpriteNumber: ��å�������ɽ�������å��������ץ饤���ֹ�
 *                                (~MES)
 *   @param wMessageSize: �ե���Ȥ��礭�� (~MES_SIZE|~MES_SET)
 *   @param wMessageColorR: ��å������ο�(Red) (~MES_SET|~MES_COLOR)
 *   @param wMessageColorG: ��å������ο�(Green) (~MES_SET|~MES_COLOR)
 *   @param wMessageColorB: ��å������ο�(Blue) (~MES_SET|~MES_COLOR)
 *   @param wMessageFont: ��å������Υե����(0:�����å�, 1:��ī)
 *                         (~MES_FONT)
 *   @param wMessageSpeed: ��å�������ɽ��®��(0:��������̵��, 1:®��,
 *                          2:�椯�餤, 3: �٤�) (~MES_SPEED) (msec)
 *   @param wMessageLineSpace: �Դ֥��ڡ��� (~MES_SPC_Y)
 *   @param wMessageAlign: �Ԥ��� (1.1~)
 *   @param vMessageLength: ???     (1.2~)
 */
void MessageOutput() {
	int wMessageSpriteNumber = getCaliValue();
	int wMessageSize   = getCaliValue();
	int wMessageColorR = getCaliValue();
	int wMessageColorG = getCaliValue();
	int wMessageColorB = getCaliValue();
	int wMessageFont   = getCaliValue();
	int wMessageSpeed  = getCaliValue();
	int wMessageLineSpace = getCaliValue();
	int wMessageAlign   = 0;
	int *vMessageLength  = NULL;
	
	if (sact.version >= 110) {
		wMessageAlign  = getCaliValue();
	}
	if (sact.version >= 120) {
		vMessageLength  = getCaliVariable();
	}
	
	smsg_out(wMessageSpriteNumber, wMessageSize, wMessageColorR, wMessageColorG, wMessageColorB, wMessageFont, wMessageSpeed, wMessageLineSpace, wMessageAlign, 0, 0, 0, vMessageLength);
	
	DEBUG_COMMAND_YET("SACT.MessageOutput %d,%d,%d,%d,%d,%d,%d,%d,%d,%p:\n", wMessageSpriteNumber, wMessageSize, wMessageColorR, wMessageColorG, wMessageColorB, wMessageFont, wMessageSpeed, wMessageLineSpace, wMessageAlign, vMessageLength);
}

/**
 * SACT.MessageOutputEx (1.1~)
 *   ��ӤĤ���å���������
 *   @param wMessageSpriteNumber: ��å�������ɽ�������å��������ץ饤���ֹ�
 *                                (~MES)
 *   @param wMessageSize: �ե���Ȥ��礭�� (~MES_SIZE|~MES_SET)
 *   @param wMessageColorR: ��å������ο�(Red) (~MES_SET|~MES_COLOR)
 *   @param wMessageColorG: ��å������ο�(Green) (~MES_SET|~MES_COLOR)
 *   @param wMessageColorB: ��å������ο�(Blue) (~MES_SET|~MES_COLOR)
 *   @param wMessageFont: ��å������Υե����(0:�����å�, 1:��ī)
 *                         (~MES_FONT)
 *   @param wMessageSpeed: ��å�������ɽ��®��(0:��������̵��, 1:®��,
 *                          2:�椯�餤, 3: �٤�) (~MES_SPEED)
 *   @param wMessageLineSpace: �Դ֥��ڡ��� (~MES_SPC_Y)
 *   @param wMessageAlign: �Ԥ���
 *   @param wRubySize:     ���ʸ�����礭��
 *   @param wRubyFont:     ���ʸ���Υե���Ȥμ���((0:�����å�, 1:��ī)
 *   @param wRubyLineSpace: ���ʸ���ȥ�å������ιԴ֥��ڡ���
 *   @param vLength: ???    (1.2~)
 */
void MessageOutputEx() {
	int wMessageSpriteNumber = getCaliValue();
	int wMessageSize   = getCaliValue();
	int wMessageColorR = getCaliValue();
	int wMessageColorG = getCaliValue();
	int wMessageColorB = getCaliValue();
	int wMessageFont   = getCaliValue();
	int wMessageSpeed  = getCaliValue();
	int wMessageLineSpace = getCaliValue();
	int wMessageAlign  = getCaliValue();
	int wRubySize      = getCaliValue();
	int wRubyFont      = getCaliValue();
	int wRubyLineSpace = getCaliValue();
	int *vLength = NULL;
	
	if (sact.version >= 120) {
		vLength = getCaliVariable();
	}
	
	smsg_out(wMessageSpriteNumber, wMessageSize, wMessageColorR, wMessageColorG, wMessageColorB, wMessageFont, wMessageSpeed, wMessageLineSpace, wMessageAlign, wRubySize, wRubyFont, wRubyLineSpace, vLength);
	
	DEBUG_COMMAND_YET("SACT.MessageOutputEx %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%p:\n", wMessageSpriteNumber, wMessageSize, wMessageColorR, wMessageColorG, wMessageColorB, wMessageFont, wMessageSpeed, wMessageLineSpace,wMessageAlign, wRubySize, wRubyFont, wRubyLineSpace, vLength);
}

/**
 * SACT.MessageNewLine (1.0~)
 *   ����(�ҥ��ޥ������) (~MES_NEW_LINE)
 *   @param wMessageSpriteNumber: ��å��������ץ饤���ֹ�
 *   @param wMessageSize: �ե���Ȥ��礭�� (~MES_SIZE|~MES_SET)
 */
void MessageNewLine() {
	int wMessageSpriteNumber = getCaliValue();
	int wMessageSize = getCaliValue();
	
	smsg_newline(wMessageSpriteNumber, wMessageSize);
	
	DEBUG_COMMAND_YET("SACT.MessageNewLine %d,%d:\n", wMessageSpriteNumber, wMessageSize);
}

/**
 * SACT.MessageClear (1.0~)
 *   ��å������ΰ�ξõ�(A���ޥ������)
 *   @param wMessageSpriteNumber: ��å��������ץ饤���ֹ�
 */ 
void MessageClear() {
	int wMessageSpriteNumber = getCaliValue();
	
	smsg_clear(wMessageSpriteNumber);
	
	DEBUG_COMMAND_YET("SACT.MessageClear %d:\n", wMessageSpriteNumber);
}

/**
 * SACT.MessageIsEmpty  (1.0~)
 *   ��å��������ĤäƤ����� wResult �� 0 ���֤�?
 *   @param wResult: ��̤��֤��ѿ�
 */
void MessageIsEmpty() {
	int *wResult = getCaliVariable();

	*wResult = smsg_is_empty();
	
	DEBUG_COMMAND_YET("SACT.MessageIsEmpty %p:\n", wResult);
}

/**
 * SACT.MessagePeek  (1.2+~) (�ʤߤ������ˤϤʤ�)
 *   ��å������Хåե������Ƥ��������
 *   @param vCount: ���������Կ�
 *   @param nTopStringNum: �Хåե����������ʸ�����ѿ��κǽ�
 */
void MessagePeek() {
	int *vCount = getCaliVariable();
	int nTopStringNum = getCaliValue();

	WARNING("NOT IMPLEMENTED\n");
	
	DEBUG_COMMAND_YET("SACT.MessagePeek %p,%d:\n", vCount, nTopStringNum);
}

/**
 * SACT.Log_Stop (1.2~)
 *   ���μ����
 */
void Log_Stop() {
	sact.logging = FALSE;
	DEBUG_COMMAND_YET("SACT.Log_Stop:\n");
}

/**
 * SACT.Log_Start (1.2~)
 *   ���μ賫��
 */
void Log_Start() {
	sact.logging = TRUE;
	DEBUG_COMMAND_YET("SACT.Log_Start:\n");
}

/**
 * SACT.MenuClear (1.0~)
 *   SACT��������������򥯥ꥢ
 */
void MenuClear() {
	ssel_clear();
	
	DEBUG_COMMAND_YET("SACT.MenuClear:\n");
}

/**
 * SACT.MenuAdd (1.0~)
 *   ��Ͽʸ�����SACT��������������ɲ�
 *   @param nString: ��Ͽ����ʸ�����ѿ��ֹ�
 *   @param wI: ��Ͽ������� (1-)
 */
void MenuAdd() {
	int nString = getCaliValue();
	int wI = getCaliValue();

	ssel_add(nString, wI);
	
	DEBUG_COMMAND_YET("SACT.MenuAdd %d,%d:\n", nString, wI);
}

/**
 * SACT.MenuOpen (1.0~)
 *   SACT��������롼��
 *   @param wMenuResult: ������(�ֹ�) ����󥻥뤷����0
 *   @param wNum: ��,�طʤȤ��륹�ץ饤���ֹ� (~SP_SEL)
 *   @param wChoiceSize: �����ʸ�������� (~SEL_SIZE)
 *   @param wMenuOutSpc: �ȥ��ץ饤�Ȥγ�¦����Υԥ������(~SP_SETSELSPC)
 *   @param wChoiceLineSpace: �����ιԴ�(1�˸���?)
 *   @param wChoiceAutoMoveCursor: �����ץ���˼�ưŪ�˰�ư�����������ֹ�
 *   @param nAlign: �Ԥ��� (0:��, 1:���, 2: ��) (1.1~)
 */
void MenuOpen() {
	int *wMenuResult = getCaliVariable();
	int wNum         = getCaliValue();
	int wChoiceSize  = getCaliValue();
	int wMenuOutSpc  = getCaliValue();
	int wChoiceLineSpace = getCaliValue();
	int wChoiceAutoMoveCursor = getCaliValue();
	int nAlign = 0;
	
	if (sact.version >= 110) {
		nAlign = getCaliValue();
	}
	
	*wMenuResult = ssel_select(wNum, wChoiceSize, wMenuOutSpc, wChoiceLineSpace, wChoiceAutoMoveCursor, nAlign);
	
	DEBUG_COMMAND_YET("SACT.MenuOpen %p,%d,%d,%d,%d,%d,%d:\n", wMenuResult, wNum, wChoiceSize, wMenuOutSpc, wChoiceLineSpace, wChoiceAutoMoveCursor, nAlign);
}

/**
 * SACT.PushString (1.0~)
 *   SACT������ʸ�����ѿ���ץå���
 *   @param nString: ʸ�����ѿ��ֹ�
 */
void PushString() {
	int nString = getCaliValue();
	
	sstr_push(nString);
	
	DEBUG_COMMAND_YET("SACT.PushString %d:\n", nString);
}

/**
 * SACT.PopString (1.0~)
 *   SACT�����˥ץå��夷��ʸ�����ѿ���ݥå�
 *   @param nString: ʸ�����ѿ��ֹ�
 */
void PopString() {
	int nString = getCaliValue();

	sstr_pop(nString);
	
	DEBUG_COMMAND_YET("SACT.PopString %d:\n", nString);
}

/**
 * SACT.Numeral_XXXX
 *   ���ץ饤����ˡ�����ο���(0��9)���Ф����б�����CG�ֹ��
 *   ɽ�����֡��ֳ֤ʤɤ��Ǽ�����Ф���Ԥ�
 */

/**
 * SACT.Numeral_SetCG (1.0~)
 *   ����ο��ͤ��Ф���CG�ֹ������
 *   @param nNum: ���ץ饤���ֹ�
 *   @param nIndex: ����(0-9)
 *   @param nCG: �������б�����CG�ֹ�
 */
void Numeral_SetCG() {
	int nNum = getCaliValue();
	int nIndex = getCaliValue();
	int nCG = getCaliValue();
	
	sp_num_setcg(nNum, nIndex, nCG);
	
	DEBUG_COMMAND_YET("SACT.Numeral_SetCG %d,%d,%d:\n", nNum, nIndex, nCG);
}

/**
 * SACT.Numeral_GetCG (1.0~)
 *   ����ο��ͤ��Ф���CG�ֹ������
 *   @param nNum: ���ץ饤���ֹ�
 *   @param nIndex: ����(0-9)
 *   @param vCG: ���ꤵ��Ƥ���CG�ֹ���֤��ѿ�
 */
void Numeral_GetCG() {
	int nNum = getCaliValue();
	int nIndex = getCaliValue();
	int *vCG = getCaliVariable();
	
	sp_num_getcg(nNum, nIndex, vCG);
	
	DEBUG_COMMAND_YET("SACT.Numeral_GetCG %d,%d,%p:\n", nNum, nIndex, vCG);
}

/**
 * SACT.Numeral_SetPos (1.0~)
 *   Numeral��ɽ�����֤�����
 *   @param nNum: ���ץ饤���ֹ�
 *   @param nX: ɽ���غ�ɸ
 *   @param ny: ɽ���ٺ�ɸ
 */
void Numeral_SetPos() {
	int nNum = getCaliValue();
	int nX = getCaliValue();
	int nY = getCaliValue();
	
	sp_num_setpos(nNum, nX, nY);
	
	DEBUG_COMMAND_YET("SACT.Numeral_SetPos %d,%d,%d:\n", nNum, nX, nY);
}

/**
 * SACT.Numeral_GetPos (1.0~)
 *   Numeral_SetPos�����ꤷ����ɸ�μ��Ф�
 *   @param nNum: ���ץ饤���ֹ�
 *   @param vX: �غ�ɸ���Ǽ�����ѿ�
 *   @param vY: �ٺ�ɸ���Ǽ�����ѿ�
 */
void Numeral_GetPos() {
	int nNum = getCaliValue();
	int *vX = getCaliVariable();
	int *vY = getCaliVariable();
	
	sp_num_getpos(nNum, vX, vY);
	
	DEBUG_COMMAND_YET("SACT.Numeral_GetPos %d,%p,%p:\n", nNum, vX, vY);
}

/**
 * SACT.Numeral_SetSpan (1.0~)
 *   Numeral�δֳ�(Span)������
 *   @param nNum: ���ץ饤���ֹ�
 *   @param nSpan: �ֳ�
 */
void Numeral_SetSpan() {
	int nNum = getCaliValue();
	int nSpan = getCaliValue();
	
	sp_num_setspan(nNum, nSpan);
	
	DEBUG_COMMAND_YET("SACT.Numeral_SetSpan %d,%d:\n", nNum, nSpan);
}

/**
 * SACT.Numeral_GetSpan (1.0~)
 *   Numeral_SetSpan�����ꤷ���ͤμ��Ф�
 *   @param nNum: ���ץ饤���ֹ�
 *   @param vSpan: �ͤ��Ǽ�����ѿ�
 */
void Numeral_GetSpan() {
	int nNum = getCaliValue();
	int *vSpan = getCaliVariable();

	sp_num_getspan(nNum, vSpan);
	
	DEBUG_COMMAND_YET("SACT.Numeral_GetSpan %d,%p:\n", nNum, vSpan);
}

/**
 * SACT.ExpSp_Clear (1.0~)
 *   �������ץ饤�����ꥯ�ꥢ
 */
void ExpSp_Clear() {
	DEBUG_COMMAND_YET("SACT.ExpSp_Clear:\n");

	sp_exp_clear();
}

/**
 * SACT.ExpSp_Add (1.0~)
 *   �������ץ饤�������ɲ�
 *   @param wNumSP1: �����å����ץ饤��
 *   @param wNumSP2: �������ץ饤��
 */
void ExpSp_Add() {
	int wNumSP1 = getCaliValue();
	int wNumSP2 = getCaliValue();
	
	sp_exp_add(wNumSP1, wNumSP2);
	
	DEBUG_COMMAND_YET("SACT.ExpSp_Add %d,%d:\n", wNumSP1, wNumSP2);
}

/**
 * SACT.ExpSp_Del (1.0~)
 *   �������ץ饤�Ⱥ��
 *   @param wNum: ���ץ饤���ֹ�
 */
void ExpSp_Del() {
	int wNum = getCaliValue();
	
	sp_exp_del(wNum);
	
	DEBUG_COMMAND_YET("SACT.ExpSp_Del %d:\n", wNum);
}

/**
 * SACT.TimerSet (1.0~)
 *   �����ID�Υ����ޡ���wCount�ͤǥꥻ�å�
 *   @param wTimerID: �����ޡ�ID
 *   @param wCount: �ꥻ�åȤ�����
 */
void TimerSet() {
	int wTimerID = getCaliValue();
	int wCount = getCaliValue();
	
	stimer_reset(wTimerID, wCount);
	
	DEBUG_COMMAND("SACT.TimerSet %d,%d:\n", wTimerID, wCount);
}

/**
 * SACT.TimerGet (1.0~)
 *   �����ID�Υ����ޡ���RND�˼���
 *   @param wTimerID: �����ޡ�ID
 *   @param vRND: ���������ѿ�
 */
void TimerGet() {
	int wTimerID = getCaliValue();
	int *vRND = getCaliVariable();

	*vRND = stimer_get(wTimerID);
	
	DEBUG_COMMAND("SACT.TimerGet %d,%p:\n", wTimerID, vRND);
}

/**
 * SACT.TimerWait (1.0~)
 *   ����ID�Υ����ޡ������ꥫ����Ȥˤʤ�ޤ��Ԥ�
 *   @param wTimerID: �����ޡ�ID
 *   @param wCount: ���ꥫ�����
 */
void TimerWait() {
	int wTimerID = getCaliValue();
	int wCount = getCaliValue();

	while(wCount > stimer_get(wTimerID)) {
		sys_keywait(10, FALSE);
	}
	
	DEBUG_COMMAND("SACT.TimerWait %d,%d:\n", wTimerID, wCount);
}

/**
 * SACT.Wait (1.1~)
 *   ������֡����٤Ƥ�ư������
 *   @param nCount: ����(1/100��ñ��)
 */
void Wait() {
	int wCount = getCaliValue();
	
	sys_keywait(wCount*10, FALSE);
	
	DEBUG_COMMAND_YET("SACT.Wait %d:\n", wCount);
}

/**
 * SACT.SoundPlay (1.0~)
 *   �������ľ�ܺ���  (~SOUND_PLAY)
 *   @param wNum: ���������ֹ�
 */
void SoundPlay() {
	int wNum = getCaliValue();
	
	ssnd_play(wNum);
	
	DEBUG_COMMAND_YET("SACT.SoundPlay %d:\n", wNum);
}

/**
 * SACT.SoundStop (1.0~)
 *   ������ɺ������ (~SOUND_STOP)
 *   @param wNum: ��ߤ����ֹ�
 *   @param wFadeTime: ��ߤ���ޤǤλ��� (1/100sec)
 */
void SoundStop() {
	int wNum = getCaliValue();
	int wFadeTime = getCaliValue();
	
	ssnd_stop(wNum, wFadeTime);
	
	DEBUG_COMMAND_YET("SACT.SoundStop %d,%d:\n", wNum, wFadeTime);
}

/**
 * SACT.SoundStopAll (1.1~)
 *   ������ɺ������ (~SOUND_STOP)
 *   @param wNum: ��ߤ����ֹ�
 *   @param wFadeTime: ��ߤ���ޤǤλ��� (1/100sec)
 */
void SoundStopAll() {
	int wFadeTime = getCaliValue();
	
	ssnd_stopall(wFadeTime);
	
	DEBUG_COMMAND_YET("SACT.SoundStopAll %d:\n", wFadeTime);
}

/**
 * SACT.SoundWait (1.0~)
 *   �إå��ǻ��ꤵ�줿����or������λ�ޤ��Ԥ�  (~SOUND_WAIT)
 *   @param wNum: �����ֹ�
 */
void SoundWait() {
	int wNum = getCaliValue();
	
	ssnd_wait(wNum);
	
	DEBUG_COMMAND_YET("SACT.SoundWait %d:\n", wNum);
}

/**
 * SACT.SoundWaitKey (1.0~)
 *   ���ꤵ�줿������ɤ�������λ���뤫���������������ޤ��Ԥ�
 *   @param wNum: �����ֹ�
 *   @param vKey: ����󥻥륭��
 */
void SoundWaitKey() {
	int wNum = getCaliValue();
	int *vKey = getCaliVariable();
	
	ssnd_waitkey(wNum, vKey);
	
	DEBUG_COMMAND_YET("SACT.SoundWaitKey %d,%p:\n", wNum, vKey);
}

/**
 * SACT.SoundPrepare (1.0~)
 *   �����ν����򤹤�(~SOUND_PREPARE)
 *   @param wNum: ���������ֹ�
 */
void SoundPrepare() {
	int wNum = getCaliValue();
	
	ssnd_prepare(wNum);
	
	DEBUG_COMMAND_YET("SACT.SoundPrepare %d:\n", wNum);
}

/**
 * SACT.SoundPrepareLR (1.0~)
 *   �����ν����򤹤�(����ȿž) (~SOUND_PREPARE_LR)
 *   @param wNum: ���������ֹ�
 */
void SoundPrepareLR() {
	int wNum = getCaliValue();

	ssnd_prepareLRrev(wNum);
	
	DEBUG_COMMAND_YET("SACT.SoundPrepareLR %d:\n", wNum);
}

/**
 * SACT.SoundPlayLR (1.0~)
 *   ����ȿž���ƺ��� (~SOUND_PLAY_LR)
 *   @param wNum: ���������ֹ�
 */
void SoundPlayLR() {
	int wNum = getCaliValue();
	
	ssnd_playLRrev(wNum);
	
	DEBUG_COMMAND_YET("SACT.SoundPlayLR %d:\n", wNum);
}

/**
 * SACT.SpriteSound (1.0~)
 * �������(���ץ饤�Ȼ���) (~SP_SOUND)
 *   @param wNumSP: ���ꤹ�륹�ץ饤���ֹ�
 *   @param nCount: ���ꤹ��Ŀ�
 *   @param wNumWave1: Sound1
 *   @param wNumWave2: Sound2
 *   @param wNumWave3: Sound3
 */
void SpriteSound() {
	int wNumSP = getCaliValue();
	int nCount = getCaliValue();
	int wNumWave1 = getCaliValue();
	int wNumWave2 = getCaliValue();
	int wNumWave3 = getCaliValue();
	int i;
	
	for (i = wNumSP; i < (wNumSP + nCount); i++) {
		sp_sound_set(i, wNumWave1, wNumWave2, wNumWave3);
	}
	
	DEBUG_COMMAND_YET("SACT.SpriteSound %d,%d,%d,%d,%d:\n", wNumSP, nCount, wNumWave1, wNumWave2, wNumWave3);
}

/**
 * SACT.SpriteSoundWait (1.0~)
 *   SpriteSound�����ꤷ�����٤Ƥβ��κ�����λ�ޤ��Ԥ� (~SP_SOUND_WAIT)
 */
void SpriteSoundWait() {
	DEBUG_COMMAND_YET("SACT.SpriteSoundWait:\n");

	sp_sound_wait();
}

/**
 * SACT.SpriteSoundOB (1.0~)
 *   �ϰϳ��򥯥�å������Ȥ��β�  (~SPRITE_SOUND_OB)
 *   @param wNumWave: ���������ֹ桢���ǥ��ꥢ
 */
void SpriteSoundOB() {
	int wNumWave = getCaliValue();
	
	sp_sound_ob(wNumWave);
	
	DEBUG_COMMAND_YET("SACT.SpriteSoundOB %d:\n", wNumWave);
}

/**
 * SACT.MusicCheck (1.0~)
 *   ���ڥǡ��������뤫�ɤ��� (~MUSIC_CHECK)
 *   @param wNum: �ֹ�
 *   @param vRND: 0:�ʤ���1:����
 */
void MusicCheck() {
	int wNum = getCaliValue();
	int *vRND = getCaliVariable();
	
	*vRND = smus_check(wNum);
	
	DEBUG_COMMAND_YET("SACT.MusicCheck %d,%p:\n", wNum, vRND);
}

/**
 * SACT.MusicGetLength (1.0~)
 *   ���ڥǡ�����Ĺ����1/100��ñ�̤Ǽ��� (~MUSIC_GET_LENGTH)
 *   @param wNum: �����ֹ�
 *   @param vRND: ��������Ĺ�����Ǽ�����ѿ�
 */
void MusicGetLength() {
	int wNum = getCaliValue();
	int *vRND = getCaliVariable();
	
	*vRND = smus_getlength(wNum);
	
	DEBUG_COMMAND_YET("SACT.MusicGetLength %d,%d:\n", wNum, *vRND);
}

/**
 * SACT.MusicGetPos (1.0~)
 *   ���ڥǡ����κ������֤�1/100��ñ�̤Ǽ��� (~MUSIC_GET_POS)
 *   @param wNum: �����ֹ�
 *   @param vRND: �����������֤��Ǽ�����ѿ�
 */
void MusicGetPos() {
	int wNum = getCaliValue();
	int *vRND = getCaliVariable();
	
	*vRND = smus_getpos(wNum);
	
	DEBUG_COMMAND_YET("SACT.MusicGetPos %d,%d:\n", wNum, *vRND);
}

/**
 * SACT.MusicPlay (1.0~)
 *   ���� (~MUSIC_PLAY)
 *   @param wNum: �����ֹ�
 *   @param wFadeTime: �ե����ɥ������(1/100��)
 *   @param wVolume: ����(0-100)
 */
void MusicPlay() {
	int wNum = getCaliValue();
	int wFadeTime = getCaliValue();
	int wVolume = getCaliValue();
	
	smus_play(wNum, wFadeTime, wVolume);
	
	DEBUG_COMMAND_YET("SACT.MusicPlay %d,%d,%d:\n", wNum, wFadeTime, wVolume);
}

/**
 * SACT.MusicStop (1.0~)
 *   ������� (~MUSIC_STOP)
 *   @param wNum: �����ֹ�
 *   @param wFadeTime: ��λ����ޤǤλ���(1/100��)
 */
void MusicStop() {
	int wNum = getCaliValue();
	int wFadeTime = getCaliValue();
	
	smus_stop(wNum, wFadeTime);
	
	DEBUG_COMMAND_YET("SACT.MusicStop %d,%d:\n", wNum, wFadeTime);
}

/**
 * SACT.MusicStopAll (1.2~)
 *   ���٤Ƥβ��ڤ����
 *   @param wFadeTime: ��λ����ޤǤλ���(1/100��)
 */
void MusicStopAll() {
	int wFadeTime = getCaliValue();
	
	smus_stopall(wFadeTime);
	
	DEBUG_COMMAND_YET("SACT.MusicStopAll %d:\n", wFadeTime);
}

/**
 * SACT.MusicFade (1.0~)
 *   ����Υܥ�塼��ޤǥե����� (~MUSIC_FADE)
 *   @param wNum: �����ֹ�
 *   @param wFadeTime: �ե����ɻ���(1/100��)
 *   @param wVolume: ���� (1-100)
 */
void MusicFade() {
	int wNum = getCaliValue();
	int wFadeTime = getCaliValue();
	int wVolume = getCaliValue();
	
	smus_fade(wNum, wFadeTime, wVolume);
	
	DEBUG_COMMAND_YET("SACT.MusicFade %d,%d,%d:\n", wNum, wFadeTime, wVolume);
}

/**
 * SACT.MusicWait (1.0~)
 *   ��������λ����ޤ��Ԥ� (~MUSIC_WAIT)
 *   @param wNum: �����ֹ�
 *   @param nTimeOut: (1.1~)
 */
void MusicWait() {
	int wNum = getCaliValue();
	int nTimeOut = 0;
	
	if (sact.version >= 110) {
		nTimeOut = getCaliValue();
	}
	
	smus_wait(wNum, nTimeOut);
	
	DEBUG_COMMAND_YET("SACT.MusicWait %d,%d:\n", wNum, nTimeOut);
}

/**
 * SACT.MusicWatiPos (1.0~)
 *   ����β��ڤ�wIndex�֤Υޡ����ǻ��ꤵ�줿�������֤����ޤ��Ԥ� 
 *   (~MUSIC_WAIT_POS)
 *   @param wNum: �����ֹ�
 *   @param wIndex: ���֥ޡ����ֹ�
 */
void MusicWaitPos() {
	int wNum = getCaliValue();
	int wIndex = getCaliValue();
	
	smus_waitpos(wNum, wIndex);
	
	DEBUG_COMMAND_YET("SACT.MusicWaitPos %d,%d:\n", wNum, wIndex);
}

/**
 * SACT.SoundGetLinkNum (1.0~)
 *   ��������ͥ�Υ���ֹ����� (~SOUND_GET_LINK_NUM)
 *   @param wNum: �����ͥ��ֹ�
 *   @param vRND: ����ֹ�(̤���ѡᣰ)
 */
void SoundGetLinkNum() {
	int wNum = getCaliValue();
	int *vRND = getCaliVariable();

	*vRND = ssnd_getlinknum(wNum);
	
	DEBUG_COMMAND_YET("SACT.SoundGetLinkNum %d,%p:\n", wNum, vRND);
}

/**
 * SACT.ChartPos (1.0~)
 *   ������ѥ��㡼�Ⱥ���
 *   @param pos : ��̽����ѿ�
 *   @param pos1: �Ǿ���
 *   @param pos2: ������
 *   @param val1: ʬΨ�Ǿ���
 *   @param val2: ʬΨ������
 *   @param val : ʬΨ
 *
 *     pos = ((pos2-pos1) / (val2-val1)) * (val-val1) + pos1
 */
void ChartPos() {
	int *pos = getCaliVariable();
	int pos1 = getCaliValue();
	int pos2 = getCaliValue();
	int val1 = getCaliValue();
	int val2 = getCaliValue();
	int val  = getCaliValue();
	
	schart_pos(pos, pos1, pos2, val1, val2, val);
	
	DEBUG_COMMAND_YET("SACT.ChartPos %p,%d,%d,%d,%d,%d:\n", pos, pos1, pos2, val1, val2, val);
}

/**
 * SACT.NumToStr (1.0~)
 *   ���� -> ʸ�����Ѵ�
 *   @param strno: �Ѵ��Ѥ�ʸ�����ѿ��ֹ�
 *   @param fig:   ������
 *   @param zeropad: 0: ������ᤷ�ʤ�, 1: ������᤹��
 *   @param num: �Ѵ��������
 */
void NumToStr() {
	int strno   = getCaliValue();
	int fig     = getCaliValue();
	int zeropad = getCaliValue();
	int num     = getCaliValue();
	
	sstr_num2str(strno, fig, zeropad, num);
	
	DEBUG_COMMAND_YET("SACT.NumToStr %d,%d,%d,%d:\n", strno, fig, zeropad, num);
}

/**
 * SACT.Maze_Create (1.0~)
 */
void Maze_Create() {
	int p1 = getCaliValue();
	int p2 = getCaliValue();

	WARNING("NOT IMPLEMENTED\n");
	
	DEBUG_COMMAND_YET("SACT.Maze_Create %d,%d:\n", p1,p2);
}

/**
 * SACT.Maze_Get (1.0~)
 */
void Maze_Get() {
	int *p1 = getCaliVariable();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	
	WARNING("NOT IMPLEMENTED\n");
	
	DEBUG_COMMAND_YET("SACT.Maze_Get %p,%d,%d:\n", p1,p2,p3);
}

/**
 * SACT.EncryptWORD (1.0~)
 */
void EncryptWORD() {
	int *array = getCaliVariable();
	int num = getCaliValue();
	int key = getCaliValue();

	scryp_encrypt_word(array, num, key);
	
	DEBUG_COMMAND_YET("SACT.EncryptWORD %p,%d,%d:\n", array, num, key);
}

/**
 * SACT.DecryptWORD (1.0~)
 */
void DecryptWORD() {
	int *array = getCaliVariable();
	int num = getCaliValue();
	int key = getCaliValue();

	scryp_encrypt_word(array, num, key);
	
	DEBUG_COMMAND_YET("SACT.DecryptWORD %p,%d,%d:\n", array, num, key);
}

/**
 * SACT.EncryptString (1.0~)
 */
void EncryptString() {
	int p1 = getCaliValue();
	int p2 = getCaliValue();

	scryp_encrypt_str(p1, p2);
	
	DEBUG_COMMAND_YET("SACT.EncryptString %d,%d:\n", p1,p2);
}

/**
 * SACT.DecryptString (1.0~)
 */
void DecryptString() {
	int p1 = getCaliValue();
	int p2 = getCaliValue();

	scryp_decrypt_str(p1, p2);
	
	DEBUG_COMMAND_YET("SACT.DecryptString %d,%d:\n", p1,p2);
}

/**
 * SACT.XMenuClear (1.0~)
 *   ��ĥ��˥塼�����
 */
void XMenuClear() {
	spxm_clear();
	
	DEBUG_COMMAND_YET("SACT.XMenuClear:\n");
}

/**
 * SACT.XMenuRegister (1.0~)
 *   ���ߥХåե��ˤ���ʸ������ĥ��˥塼�Υ����ƥ�Ȥ�����Ͽ
 *   @param nRegiNum: ��ĥ��˥塼����������ǥå����ֹ�
 *   @param nMenuID: ���򤵤줿�Ȥ����֤��ֹ�(ID)
 */
void XMenuRegister() {
	int nRegiNum = getCaliValue();
	int nMenuID  = getCaliValue();
	
	spxm_register(nRegiNum, nMenuID);
	
	DEBUG_COMMAND_YET("SACT.XMenuRegister %d,%d:\n", nRegiNum, nMenuID);
}

/**
 * SACT.XMenuGetNum (1.0~)
 *   XMenuRegister����Ͽ���줿ID���֤�
 *   @param nRegiNum: ��������ǥå����ֹ�
 *   @param vMenuID: ��Ͽ����Ƥ���ID���Ǽ�����ѿ�
 */
void XMenuGetNum() {
	int nRegiNum = getCaliValue();
	int *vMenuID = getCaliVariable();
	
	*vMenuID = spxm_getnum(nRegiNum);
	
	DEBUG_COMMAND_YET("SACT.XMenuGetNum %d,%p:\n", nRegiNum, vMenuID);
}

/**
 * SACT.XMenuGetText (1.0~)
 *   XMenuRegister����Ͽ���������ƥ������ʸ�����ѿ��˥��ԡ�����
 *   @param nRegiNum: ��������ǥå����ֹ�
 *   @param strno: ���ԡ���ʸ�����ѿ��ֹ�
 */
void XMenuGetText() {
	int nRegiNum = getCaliValue();
	int strno    = getCaliValue();
	
	spxm_gettext(nRegiNum, strno);
	
	DEBUG_COMMAND_YET("SACT.XMenuGetText %d,%d:\n", nRegiNum, strno);
}

/**
 * SACT.XMenuTitleRegister (1.0~)
 *   ���ߥХåե��ˤ���ʸ������ĥ��˥塼�Υ����ȥ�Ȥ�����Ͽ
 */
void XMenuTitleRegister() {
	spxm_titlereg();
	
	DEBUG_COMMAND_YET("SACT.XMenuTitleRegister:\n");
}

/**
 * SACT.XMenuTitleGet (1.0~)
 *   ��ĥ��˥塼�Υ����ȥ������ʸ�����ѿ��˥��ԡ�
 *   @param strno: ���ԡ���ʸ�����ѿ��ֹ�
 */
void XMenuTitleGet() {
	int strno = getCaliValue();
	
	spxm_titleget(strno);
	
	DEBUG_COMMAND_YET("SACT.XMenuTitleGet %d:\n", strno);
}
