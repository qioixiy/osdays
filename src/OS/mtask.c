#include "mtask.h"
#include "timer.h"
#include "naskfunc.h"
#include "memory.h"
#include "dsctbl.h"

struct TIMER *mt_timer;
int mt_tr;

void mt_init(void)
{
  mt_timer = timer_alloc();
  //这里不需要timer_init
  timer_settime(mt_timer, 2);
  mt_tr = 3*8;
  return;
}

void mt_taskswitch(void) 
{
  if (mt_tr == 3*8) {
    mt_tr = 4*8; 
  } else {
    mt_tr = 3*8;
  }
  timer_settime(mt_timer, 2);
  farjmp(0, mt_tr);
  return;
}

struct TASKCTL *taskctl;
struct TIMER *task_timer;

struct TASK *task_init(struct MEMMAN *memman)
{
  int i;
  struct TASK *task;
  struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT;
  taskctl = (struct TASKCTL *)memman_alloc_4k(memman, sizeof(struct TASKCTL));
  
  //初始化GDT TSS
  for (i = 0; i < MAX_TASKS; i++) {
    taskctl->tasks0[i].flags = 0;
    taskctl->tasks0[i].sel = (TASK_GDT0 + i) * 8;
    set_segmdesc(gdt+TASK_GDT0+i, 103, (int)&taskctl->tasks0[i].tss, AR_TSS32);
  }

  task = task_alloc();//分配一个任务给当前
  task->flags = 2;//运行状态，活动中
  taskctl->running = 1;
  taskctl->now = 0;
  taskctl->tasks[0] = task;
  load_tr(task->sel);
  task_timer = timer_alloc();//分配task切换timer
  timer_settime(task_timer, 2);//设置任务切换时间
  
  return task;
}

struct TASK *task_alloc(void)
{
  int i;
  struct TASK *task;
  for (i = 0; i < MAX_TASKS; i++) {
    if (taskctl->tasks0[i].flags == 0) {//未使用的task结构
      taskctl->tasks0[i].flags = 1;//已分配状态
      task = &taskctl->tasks0[i];
      task->tss.eflags = 0x00000202;//IF=1
      task->tss.eax = 0;
      task->tss.ecx = 0;
      task->tss.edx = 0;      
      task->tss.ebx = 0;
      task->tss.ebp = 0;
      task->tss.esi = 0;
      task->tss.edi = 0;
      task->tss.es = 0;
      task->tss.ds = 0;
      task->tss.fs = 0;
      task->tss.gs = 0;
      task->tss.ldtr = 0;      
      task->tss.iomap = 0x40000000;
      return task;
    }
  }
  return 0;
}

