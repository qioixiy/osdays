#ifndef _TASK_H
#define _TASK_H
#include "memory.h"
#include "fifo.h"

#define MAX_TASKS 1000//���������
#define TASK_GDT0 3//�����GDT�ĵڼ��ſ�ʼ�����TSS

//task level priority
#define MAX_TASKS_LV 100
#define MAX_TASKLEVELS 10

struct TSS32{
  int backlink, esp0, ss0,esp1, ss1, esp2, ss2, cr3;
  int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
  int es, cs, ss, ds, fs, gs;
  int ldtr, iomap;
};

struct TASK {
  int sel, flags;//sel�������GDT�ı��
  int priority;//task���ȼ�
  int level;
  struct FIFO32 fifo;//task FIFI32
  struct TSS32 tss;//һ�������е�tss��
};

struct TASKLEVEL {
  int running;//�����������������
  int now;//�������е�����
  struct TASK *tasks[MAX_TASKS_LV];
};

//TASK����ṹ��
struct TASKCTL {
  int now_lv;//���ڻ�е�level
  char lv_change;//�´������л�ʱ�Ƿ���Ҫ�ı�level
  struct TASKLEVEL level[MAX_TASKLEVELS];
  struct TASK tasks0[MAX_TASKS];
};

extern struct TIMER *task_timer;

struct TASK *task_init(struct MEMMAN *memman);
struct TASK *task_alloc(void);
void task_run(struct TASK *task, int level, int priority);
void task_sleep(struct TASK *task);
void task_switch(void);

struct TASK *task_now(void);
void task_add(struct TASK *task);
void task_remove(struct TASK *task);

//����task
void task_idle(void);
#endif
