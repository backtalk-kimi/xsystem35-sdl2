/*
 *	for new GOGO-no-coda (1999/09)
 *	Copyright (C) 1999 shigeo
 */

#ifndef _CHKCPU_H
#define _CHKCPU_H

#define tFPU	(1<<0)
#define tMMX	(1<<1)
#define t3DN	(1<<2)
#define tSSE	(1<<3)
#define tCMOV	(1<<4)
#define tE3DN	(1<<5)	/* Athlon��(Externd 3D Now!) */
#define tEMMX   (1<<6)  /* EMMX=E3DNow!_INT=SSE_INT  */
#define tINTEL	(1<<8)
#define tAMD	(1<<9)
#define tCYRIX	(1<<10)
#define tIDT	(1<<11)
#define tMULTI	(1<<12)	/* for Multi-threaded encoder. */
		/* Never set on UP or in the binary linked w/o multithread lib. */
#define tUNKNOWN	(1<<15)	/* �٥�������� */
#define tSPC1 (1<<16)	/* ���̤ʥ����å� */
#define tSPC2 (1<<17)	/* ���ӤϷ�ޤäƤʤ� */

#define tFAMILY4	(1<<20)	/* 486 ���λ��٥����Ƚ������Ƥˤʤ�ʤ� */
#define tFAMILY5	(1<<21)	/* 586 (P5, P5-MMX, K6, K6-2, K6-III) */
#define tFAMILY6	(1<<22)	/* 686�ʹ� P-Pro, P-II, P-III, Athlon */

/*
 *	��ܤ��Ƥ����˥åȤ˽��äƾ���ͤ������¤��֤�
 *	chkcpu.asm�Ȥ����������
 */

int haveUNIT(void);

/*
 *	useUNIT�˽��äƴؿ��λ��Ѥ��ѹ�����
 */

void setupUNIT(int useUNIT);

/*
 *	SSE��ͼθ����⡼�ɤˤ���
 */

void setPIII_round(void);

 /*
 *	���Ѵؿ���ɽ��(�ǥХå���)
 */

void SETUP_DSP(char *mes);

#ifdef WIN32
void maskFPU_exception( void );
#endif

#if 0
#define SETUP_DEBUG

#ifdef SETUP_DEBUG
#define SETUP_DSP(x) fprintf(stderr,x)
#else
#define SETUP_DSP(x)
#endif
#endif

#endif
