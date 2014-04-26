#ifndef __NIGHT_H__
#define __NIGHT_H__

#include "config.h"
#include <glib.h>
#include "portab.h"
#include "graphics.h"
#include "surface.h"
#include "ags.h"
#include "sprite.h"



// �����������Ȥμ���
#define KEYWAIT_NONE    0
#define KEYWAIT_SIMPLE  1
#define KEYWAIT_SPRITE  2
#define KEYWAIT_MESSAGE 3
#define KEYWAIT_SELECT  4
#define KEYWAIT_BACKLOG 5

#define MSGBUFMAX 512

#if 0
// ʸ�����ִ���
typedef struct {
	char *src; // �֤�������ʸ����
	char *dst; // �֤�����ʸ����
} strexchange_t;
#endif

struct _night {
	// scenario
	int Month;
	int Day;
	int DayOfWeek;

	// keyevent
	int waittype;
	int waitskiplv;
	int waitkey;

	
	// graph
	// cginfo_t *cg[CGMAX];

	MyRectangle updaterect;

	int fonttype;
	int fontsize;

	sprite_t *sp[SPRITEMAX];

	int msgplace;
	int msgframe;

	// msg
	char msgbuf[MSGBUFMAX];
	boolean zhiding;
	struct {
		void (* cbmove)(agsevent_t *);
		void (* cbrelease)(agsevent_t *);
	} msg;
	//GSList *strreplace;

	// sel
	int selmode;
	struct {
		void (* cbmove)(agsevent_t *);
		void (* cbrelease)(agsevent_t *);
	} sel;
};

typedef struct _night night_t;

#define night nightprv
extern night_t night;


#endif /* __NIGHT_H__ */
