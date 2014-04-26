/*
 * sact.h: SACT
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
/* $Id: sact.h,v 1.3 2003/07/14 16:22:51 chikama Exp $ */

#ifndef __SACT_H__
#define __SACT_H__

#include "config.h"

#include <glib.h>
#include "portab.h"
#include "graphics.h"
#include "surface.h"
#include "sacttimer.h"
#include "variable.h"


// ���ץ饤�Ȥκ����
#define SPRITEMAX 21845

// CG�κ����
#define CGMAX 63336

// ��å������κ���Ĺ��
#define MSGBUFMAX 257*10

// �����κ������ǿ�
#define SEL_ELEMENT_MAX 20


// �����������Ȥμ���
#define KEYWAIT_NONE 0
#define KEYWAIT_SIMPLE 1
#define KEYWAIT_SPRITE 2
#define KEYWAIT_MESSAGE 3
#define KEYWAIT_SELECT 4
#define KEYWAIT_BACKLOG 5

// ʸ�����ִ���
typedef struct {
	char *src; // �֤�������ʸ����
	char *dst; // �֤�����ʸ����
} strexchange_t;

// SACTEFAM ��Ȥä��ޥ���
typedef struct {
	int fd;       // SACTEFAM.KLD �Υե�����ǥ������ץ꥿
	char *mapadr; // mmap ���줿�ǽ�Υ��ɥ쥹
	off_t size;   // mmap �����礭��
	int datanum;  // SACTEFAM.KLD ��Υޥ����ե�����ο�
	int *no;      // ���ʥꥪ¦�Ǥ��ֹ�
	int *offset;  // �ǡ����ؤΥ��ե��å�
} SACTEFAM_t;


// CG_XX �Ǻ��CG�μ���
enum cgtype {
	CG_NOTUSED = 0,
	CG_LINKED  = 1,
	CG_SET     = 2,
	CG_REVERSE = 3,
	CG_STRETCH = 4
};

// cg�˴ؤ������
struct _cginfo {
	enum cgtype type;  // CG�μ���, 0: ̤����, 1:��󥯤���Ƥ���, ...
	int no;            // CG���ֹ�
	surface_t *sf;     // CG����
	int refcnt;        // ���ȥ����󥿡����ˤʤä��鳫�����Ƥ�褤��
};
typedef struct _cginfo cginfo_t;

// ���ץ饤�ȤΥ�����
enum spritetype {
	SPRITE_NORMAL = 0,
	SPRITE_SWITCH = 1,
	SPRITE_GETA   = 2,
	SPRITE_GETB   = 3,
	SPRITE_PUT    = 4,
	SPRITE_ANIME  = 5,
	SPRITE_SWPUT  = 6,
	SPRITE_MSG    =100,
	SPRITE_WP,
	SPRITE_NONE   =-1
};

// (����������)
struct _sprite;

// ���ץ饤�Ȥ˴ؤ���Ƽ����
struct _sprite {
	// ���ץ饤�ȤΥ�����
	enum spritetype type;
	
	// ���ץ饤���ֹ�
	int no;
	
	// ���줾��ξ��֤λ����Ĥ餹��������ֹ�
	int numsound1, numsound2, numsound3;
	
	// ��� sprite ���礭��(cg1���礭��)
	MyDimension cursize;
	
	// ���줾��ξ��֤�ɽ������ CG
	cginfo_t *cg1, *cg2, *cg3;
	
	// update ����Ȥ���ɽ������cg
	cginfo_t *curcg;
	
	// ���ץ饤�Ȥ�ɽ�����뤫
	boolean show;
	boolean show_save; // Zkey hide save��
	
	// ɽ������ݤΥ֥���Ψ 0:���������ʤ�, 255: �̾�ɽ��
	int blendrate; 
	
	// ���ץ饤�Ȥ� Freeze ����Ƥ��뤫(0:No 1-3: �����ֹ�)
	int freezed_state;
	
	// ɽ������ (SetPos)
	MyPoint loc;
	
	// ���ߤΥ��ץ饤�Ȥ�ɽ������
	MyPoint cur;
	
	// event callback
	int (* eventcb)(struct _sprite *sp, agsevent_t *e);  // for key/mouse
	int (* teventcb)(struct _sprite *sp, agsevent_t *e); // for timer
        // sprite������� callback
	void (* remove)(struct _sprite *sp);
	// sprite������褹��Ȥ��� callback
	int  (* update)(struct _sprite *sp);
	
	boolean focused; // forcus�����Ƥ��뤫
	boolean pressed; // ����sprite��ǥޥ�����������Ƥ��뤫
	
	GSList *expsp; // �������ץ饤�ȤΥꥹ��
	
	// move command �ѥѥ�᡼��
	struct {
		MyPoint to;     // ��ư��
		int time;       // ��ư��λ����
		int speed;      // ��ư®��
		int starttime;  // ��ư���ϻ���
		int endtime;    // ��ư��λͽ�����
		boolean moving; // ��ư�椫�ɤ���
	} move;
	
	// SACT.Numeral�ѥѥ�᡼��
	struct {
		int cg[10];
		MyPoint pos;
		int span;
	} numeral;
	
	// ���ץ饤�Ȥμ�����ξ���
	union {
		// �����å����ץ饤��
		struct {
			
		} sw;
		
		// ���åȥ��ץ饤��
		struct {
			boolean dragging;  // �ɥ�å���
			MyPoint dragstart; // �ɥ�å����ϰ���
		} get;

		// �ץåȥ��ץ饤��
		struct {
			
		} put;

		// ���˥᡼����󥹥ץ饤��
		struct {
			int interval;      // �����ޤδֳ�(10msec)
			int starttime;     // ���ϻ���
			int npat;          // ���˥ᥳ�޿�(1/2/3)
			unsigned int tick; // ������
		} anime;
		
		// ��å��������ץ饤��
		struct {
			GSList    *buf;       // ɽ������ʸ���Υꥹ��
			surface_t *canvas;    // ʸ�������褹��surface
			MyPoint    dspcur;    // ���ߤ�ɽ������
		} msg;
	} u;
};
typedef struct _sprite sprite_t;

// SACT���Τξ���
struct _sact {
	// SACT�ΥС������
	int version;
	
	// ���ץ饤������
	sprite_t *sp[SPRITEMAX];
	
	GSList *sp_zhide;  // Z�����Ǿä����ץ饤�ȤΥꥹ��
	GSList *sp_quake;  // Quake���ɤ餹���ץ饤�ȤΥꥹ��
	
	GSList *updatelist; // �����褹�륹�ץ饤�ȤΥꥹ��
	
	cginfo_t *cg[CGMAX]; // cg�ޤ���CG_xx�Ǻ�ä� CG
	
	// ��ɸ�Ϥθ���
	MyPoint origin;
	
	// ʸ���� push/pop/replce ��
	GSList *strstack;
	GSList *strreplace;
	char   *strreplacesrc;
	char   *strreplacedst; 
	
	// ��å��������ץ饤���ѥ�å������Хåե�
	char msgbuf[MSGBUFMAX];
	char msgbuf2[MSGBUFMAX];
	
	// ���򥦥����
	struct {
		char *elem[SEL_ELEMENT_MAX]; // �����ʸ����
		int spno; // �طʥ��ץ饤���ֹ�
		int font_size; // �����ʸ��������
		int font_type; // �����ե����
		int frame_dot; // �ȥ��ץ饤�Ȥγ�¦����Υԥ������
		int linespace; // �����ιԴ�
		int movecursor; // �������
		int align; // �Ԥ���
		void (* cbmove)(agsevent_t *);
		void (* cbrelease)(agsevent_t *);
		surface_t   *charcanvas;
	} sel;
	
	// event listener
	GSList *eventlisteners;
	GSList *teventlisteners;
	GSList *teventremovelist;
	
	// MOVE���륹�ץ饤�ȤΥꥹ��
	GSList *movelist;
	int movestarttime; // ���Ƥ˰�ư�򳫻Ϥ��뤿��γ��ϻ���
	int movecurtime;
	
	MyRectangle updaterect; // ������ɬ�פ�sprite���ΰ����
	
	// sact timer
	stimer_t timer[65536];
	
	// DnD�˴ؤ�����
	sprite_t *draggedsp;  // drag��Υ��ץ饤��
	boolean dropped;      // ���ץ饤�Ȥ��ɥ�åפ��줿���ɤ���
	
	// keywait�μ���
	int waittype;
	int waitkey;
	int sp_result_sw;
	int sp_result_get;
	int sp_result_put;
	
	// wait skip level
	//  0 �̾省���Ԥ�
	//  1 ���ɤΤߥ����å�
	//  2 ̤�ɤ⥹���å�
	int waitskiplv;
	
	// �ϰϳ��򥯥�å������Ȥ��β�
	int numsoundob;
	
	// depth map
	surface_t *dmap;
	
	// SACTEFAM.KLD
	SACTEFAM_t am;
	
	boolean zhiding;  // Zkey�ˤ�뱣����
	int     zofftime;
	boolean zdooff;
	
	// �Хå���
	boolean logging;
	GList  *log;
};
typedef struct _sact sact_t;

// shortcut
#define sact sactprv
extern sact_t sact;

#endif /* __SACT_H__ */
