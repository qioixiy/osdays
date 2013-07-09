#ifndef _TASK_H
#define _TASK_H
#include "memory.h"
#include "fifo.h"

#define MAX_TASKS 1000//最大任务数
#define TASK_GDT0 3//定义从GDT的第几号开始分配给TSS

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
  int sel, flags;//sel用来存放GDT的编号
  int priority;//task优先级
  int level;
  struct FIFO32 fifo;//task FIFI32
  struct TSS32 tss;//一个任务中的tss段
};

struct TASKLEVEL {
  int running;//正在运行任务的数量
  int now;//正在运行的任务
  struct TASK *tasks[MAX_TASKS_LV];
};

//TASK管理结构体
struct TASKCTL {
  int now_lv;//现在活动中的level
  char lv_change;//下次任务切换时是否需要改变level
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

//闲置task
void task_idle(void);
#endif
