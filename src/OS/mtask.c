#include "mtask.h"
#include "timer.h"
#include "naskfunc.h"
#include "memory.h"
#include "dsctbl.h"

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
  task->priority = 2;//0.02s
  taskctl->running = 1;
  taskctl->now = 0;
  taskctl->tasks[0] = task;
  load_tr(task->sel);
  task_timer = timer_alloc();//����task�л�timer
  timer_settime(task_timer, task->priority);//���������л�ʱ��,����Ӧ�����ȼ�
  
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

void task_run(struct TASK *task, int priority)
{
  if (priority > 0) {
    task->priority = priority;
  }

  if (task->flags != 2) {//�����������״̬��...,����ֻ�Ǹı����ȼ�
    task->flags = 2;//��Ϊ����״̬
    taskctl->tasks[taskctl->running] = task;
    taskctl->running++;//������������1
  }
  return;
}

void task_switch(void)
{
  struct TASK *task;
  taskctl->now++;
  if (taskctl->now == taskctl->running) {//��������һ�����´ξ�ѡ���һ��task
    taskctl->now = 0;//��ѯ����
  }
  task = taskctl->tasks[taskctl->now];
  timer_settime(task_timer, task->priority);//��дtask timer
 
  if (taskctl->running >= 2) {//����һ����������
    farjmp(0, task->sel);
  }
  return ;
}


void task_sleep(struct TASK *task)
{
  int i;
  char ts = 0;
  if (task->flags == 2) {//���ָ��������������״̬
    if (task == taskctl->tasks[taskctl->now]) {//���˯�ߵ��ǵ�ǰ��task,��Ҫ���������л�
      ts = 1;
    }
    //Ѱ��task��λ��
    for (i = 0; i < MAX_TASKS; i++) {
      if (task == taskctl->tasks[i]) {
	break;
      }
    }
    taskctl->running--;
    if (i < taskctl->now) {
      taskctl->now--;
    }
    //�ƶ���Ա
    for (; i < taskctl->running; i++) {
      taskctl->tasks[i] = taskctl->tasks[i+1];
    }
    task->flags = 1;//��Ϊ������״̬
    if (ts != 0) {
      //�����л�
      if (taskctl->now >= taskctl->running) {//�쳣������
	taskctl->now = 0;
      }
      farjmp(0, taskctl->tasks[taskctl->now]->sel);
    }
  }
  return;
}
