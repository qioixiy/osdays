#ifndef _TASK_H
#define _TASK_H
#include "memory.h"

#define MAX_TASKS 1000//���������
#define TASK_GDT0 3//�����GDT�ĵڼ��ſ�ʼ�����TSS

struct TSS32{
  int backlink, esp0, ss0,esp1, ss1, esp2, ss2, cr3;
  int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
  int es, cs, ss, ds, fs, gs;
  int ldtr, iomap;
};

struct TASK {
  int sel, flags;//sel�������GDT�ı��
  struct TSS32 tss;//һ�������е�tss��
};
//TASK����ṹ��
struct TASKCTL {
  int running;//�������е�������
  int now;//��ǰ���е�����
  struct TASK *tasks[MAX_TASKS];
  struct TASK tasks0[MAX_TASKS];
};

extern struct TIMER *mt_timer;
extern int mt_tr;

void mt_init(void);
void mt_taskswitch(void);

struct TASK *task_init(struct MEMMAN *memman);
struct TASK *task_alloc(void);

#endif
