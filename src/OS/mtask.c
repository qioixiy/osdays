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
  //���ﲻ��Ҫtimer_init
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
  
  //��ʼ��GDT TSS
  for (i = 0; i < MAX_TASKS; i++) {
    taskctl->tasks0[i].flags = 0;
    taskctl->tasks0[i].sel = (TASK_GDT0 + i) * 8;
    set_segmdesc(gdt+TASK_GDT0+i, 103, (int)&taskctl->tasks0[i].tss, AR_TSS32);
  }

  task = task_alloc();//����һ���������ǰ
  task->flags = 2;//����״̬�����
  taskctl->running = 1;
  taskctl->now = 0;
  taskctl->tasks[0] = task;
  load_tr(task->sel);
  task_timer = timer_alloc();//����task�л�timer
  timer_settime(task_timer, 2);//���������л�ʱ��
  
  return task;
}

struct TASK *task_alloc(void)
{
  int i;
  struct TASK *task;
  for (i = 0; i < MAX_TASKS; i++) {
    if (taskctl->tasks0[i].flags == 0) {//δʹ�õ�task�ṹ
      taskctl->tasks0[i].flags = 1;//�ѷ���״̬
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

void task_run(struct TASK *task)
{
  task->flags = 2;//��Ϊ����״̬
  taskctl->tasks[taskctl->running] = task;
  taskctl->running++;//������������1
  return;
}

void task_switch(void)
{
  timer_settime(task_timer, 2);//��дtask timer
  if (taskctl->running >= 2) {//����һ����������
    taskctl->now++;
    if (taskctl->now == taskctl->running) {//��������һ�����´ξ�ѡ���һ��task
      taskctl->now = 0;//��ѯ����
    }
    farjmp(0, taskctl->tasks[taskctl->now]->sel);
  }
  return ;
}
