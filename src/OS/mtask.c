#include "mtask.h"
#include "timer.h"
#include "naskfunc.h"
#include "memory.h"
#include "dsctbl.h"

struct TASKCTL *taskctl;
struct TIMER *task_timer;

static void task_switchsub(void);
	
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

  for (i = 0; i < MAX_TASKLEVELS; i++){//��ʼ�������״̬Ϊ0
    taskctl->level[i].running = 0;
    taskctl->level[i].now = 0;
  }

  task = task_alloc();//����һ���������ǰ
  task->flags = 2;//����״̬�����
  task->priority = 2;//0.02s
  task->level = 0;//��߲�
  task_add(task);
  task_switchsub();//level ����

  load_tr(task->sel);
  task_timer = timer_alloc();//����task�л�timer
  timer_settime(task_timer, task->priority);//���������л�ʱ��,����Ӧ�����ȼ�
  
  //�����������
  struct TASK *idle;
  idle = task_alloc();
  idle->tss.esp = memman_alloc_4k(memman, 64*1024) + 64*1024;
  idle->tss.eip = (int)&task_idle;
  idle->tss.es = 1*8;
  idle->tss.cs = 2*8;
  idle->tss.ss = 1*8;
  idle->tss.ds = 1*8;
  idle->tss.fs = 1*8;
  idle->tss.gs = 1*8;
  task_run(idle, MAX_TASKLEVELS-1, 1);

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
      task->tss.ss0 = 0;
      return task;
    }
  }
  return 0;
}

void task_run(struct TASK *task, int level, int priority)
{
  if (level < 0) {//�쳣����
    level = task->level;//���ı�level
  }

  if (priority > 0) {//�쳣����
    task->priority = priority;
  }

  if (task->flags == 2 && task->level != level) {//�ı��е�level
    task_remove(task);//�Ƴ�������flagsΪ1�����洦��
  }
  if (task->flags != 2) {
    //����
    task->level = level;
    task_add(task);//��ӵ�level������
  }
  
  taskctl->lv_change = 1;//�´������л�ʱ���level
  
  return;
}

void task_switch(void)
{
  struct TASKLEVEL *tl = &taskctl->level[taskctl->now_lv];
  struct TASK *new_task, *now_task = tl->tasks[tl->now];

  tl->now++;
  if (tl->now == tl->running) {//��������һ�����´ξ�ѡ���һ��task
    tl->now = 0;//��ѯ����
  }
  if (taskctl->lv_change != 0) {
    task_switchsub();
    tl = &taskctl->level[taskctl->now_lv];
  }

  new_task = tl->tasks[tl->now];
  timer_settime(task_timer, new_task->priority);//��дtask timer
 
  if (new_task != now_task) {
    farjmp(0, new_task->sel);
  }
  return ;
}


void task_sleep(struct TASK *task)
{
  struct TASK *now_task;
  if (task->flags == 2) {//���������״̬������
    now_task = task_now();
    task_remove(task);
    if (task == now_task) {//���˯��ʱ��ǰ������Ҫ���������л�
      task_switchsub();
      now_task = task_now();//��ȡ�л��������
      farjmp(0, now_task->sel);
    }
  }
  return;
}

//ȡ�����ڻ��task
struct TASK *task_now(void)
{
  struct TASKLEVEL *tl = &taskctl->level[taskctl->now_lv];
  return tl->tasks[tl->now];
}

//��TASKLEVEL���һ������
void task_add(struct TASK *task)
{
  struct TASKLEVEL *tl = &taskctl->level[task->level];
  tl->tasks[tl->running] = task;
  tl->running++;
  task->flags = 2;//���Ϊ���
  return;
}

void task_remove(struct TASK *task)
{
  int i;
  struct TASKLEVEL *tl = &taskctl->level[task->level];

  //Ѱ��task���ڵ�λ��
  for (i = 0; i < tl->running; i++) {
    if (task == tl->tasks[i]) {
      break;
    }
  }

  tl->running--;
  if (i < tl->now) {
    tl->now--;
  }
  if (tl->now >= tl->running) {//�쳣���
    tl->now = 0;
  }
  task->flags = 1;

  //�ƶ�
  for ( ;i < tl->running; i++) {
    tl->tasks[i] = tl->tasks[i+1];
  }
}

static void task_switchsub(void) 
{
  int i;
  //Ѱ�����ϲ��leve
  for (i = 0; i < MAX_TASKLEVELS; i++) {
    if (taskctl->level[i].running > 0) {
      break;//�ҵ������е�level
    }
  }

  taskctl->now_lv = i;
  taskctl->lv_change = 0;
  return;
}

void task_idle(void)
{
  for(;;) {
    io_hlt();
  }
}
