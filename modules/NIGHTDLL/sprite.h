
#ifndef __SPRITE_H__
#define __SPRITE_H__

#include <glib.h>

#include "portab.h"
#include "surface.h"
#include "graphics.h"

#define DEFAULT_UPDATE sp_draw


#define SPRITEMAX 20
#define SPNO_WALL    0  // �ɻ�
#define SPNO_SCENERY 1  // �ط�
#define SPNO_TACHI_L 2  // Ω������
#define SPNO_TACHI_M 3  // Ω�������
#define SPNO_TACHI_R 4  // Ω������
#define SPNO_MSGBG   5  // ʸ������
#define SPNO_MSGFRAME_BG 6 // ��å��������������
#define SPNO_MSGFRAME_FG 7 // ��å�����ʸ�����襭���Х�
#define SPNO_FACE 11        // ��ʪ��CG
#define SPNO_MSG_KEYANIM 12 // ��å������������ϥ��˥᡼�����
#define SPNO_MSG_ICON_MUTE 13 // ��å�����������ɥ�������

#define CGMAX 65536
// 0-9999: reserve for Link CG
// 2051: ��å��������������CG
#define CGNO_MSGFRAME_LCG 2051
// 4017: ��å�����������ɤ�����mute�ˤ���CG
#define CGNO_MSGFRAME_NOVICE_LCG 4017
// 4018: ��å�����������ɤΥ�������˥ޥ������Ťʤä��Ȥ���CG
#define CGNO_MSGFRAME_ICONREV_LCG 4018
// 4034: �������Ϥ�¥�����˥᡼�����CG
#define CGNO_MSGHAK_LCG 4034

// 10000: ��å�����������ɤǤΥ����Ԥ����˥� ����1
#define CGNO_MSGHAK_1 10000
// 10001: ��å�����������ɤǤΥ����Ԥ����˥� ����2
#define CGNO_MSGHAK_2 10001
// 10002: ʸ������CG
#define CGNO_MSGFR_BG 10002


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


enum spritetype {
	SPRITE_NORMAL = 0,
	SPRITE_ANIME  = 5,
	SPRITE_MSG    = 100,
	SPRITE_WP,
	SPRITE_NONE   = -1
};

struct _sprite {
	enum spritetype type;
	
	int no;
	
	MyDimension cursize;
	
	cginfo_t *curcg;
	cginfo_t *cg1, *cg2, *cg3;
	
	boolean show;
	
	int blendrate;
	
	MyPoint loc;
	
	MyPoint cur;

	int (* update)(struct _sprite *sp, MyRectangle *updatearea);
	
	union {
		struct {
			int interval;
			int startttime;
			int npat;
			unsigned int tick;
		} anime;
		
		struct {
			surface_t *canvas;
			MyPoint dspcur;
		} msg;
	} u;
};

typedef struct _sprite sprite_t;


/* in nt_sprite.c */
extern sprite_t *sp_new(int no, int cg1, int cg2, int cg3, int type);
extern sprite_t *sp_msg_new(int no, int x, int y, int width, int height);
extern void sp_free(sprite_t *sp);
extern void sp_set_show(sprite_t *sp, boolean show);
// extern void sp_set_cg(sprite_t *sp, int no);
extern void sp_set_loc(sprite_t *sp, int x, int y);

/* in nt_sprite_update.c */
extern int sp_update_clipped();
extern int sp_update_all(boolean syncscreen);
extern int sp_updateme(sprite_t *sp);
extern int sp_updateme_part(sprite_t *sp, int x, int y, int w, int h);
extern void sp_add_updatelist(sprite_t *sp);
extern void sp_remove_updatelist(sprite_t *sp);
extern int sp_draw_wall(sprite_t *sp, MyRectangle *r);

/* in nt_sprite_draw.c */
extern int sp_draw(sprite_t *sp, MyRectangle *r);
extern int sp_draw2(sprite_t *sp, cginfo_t *cg, MyRectangle *r);
extern void sp_draw_dmap(gpointer data, gpointer userdata);
extern int sp_draw_scg(sprite_t *sp, MyRectangle *r);

/* in nt_sprite_eupdate.c */
extern int sp_eupdate(int type, int time, int cancel);


#endif /* __SPRITE_H__ */
