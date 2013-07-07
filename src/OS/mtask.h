#ifndef _TASK_H
#define _TASK_H
#include "memory.h"

#define MAX_TASKS 1000//最大任务数
#define TASK_GDT0 3//定义从GDT的第几号开始分配给TSS

struct TSS32{
  int backlink, esp0, ss0,esp1, ss1, esp2, ss2, cr3;
  int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
  int es, cs, ss, ds, fs, gs;
  int ldtr, iomap;
};

struct TASK {
  int sel, flags;//sel用来存放GDT的编号
  int priority;//task优先级
  struct TSS32 tss;//一个任务中的tss段
};
//TASK管理结构体
struct TASKCTL {
  int running;//正在运行的任务数
  int now;//当前运行的任务
  struct TASK *tasks[MAX_TASKS];
  struct TASK tasks0[MAX_TASKS];
};

extern struct TIMER *task_timer;

struct TASK *task_init(struct MEMMAN *memman);
struct TASK *task_alloc(void);
void task_run(struct TASK *task, int priority);
void task_sleep(struct TASK *task);
void task_switch(void);

#endif
