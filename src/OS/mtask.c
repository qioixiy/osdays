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
  
  //初始化GDT TSS
  for (i = 0; i < MAX_TASKS; i++) {
    taskctl->tasks0[i].flags = 0;
    taskctl->tasks0[i].sel = (TASK_GDT0 + i) * 8;
    set_segmdesc(gdt+TASK_GDT0+i, 103, (int)&taskctl->tasks0[i].tss, AR_TSS32);
  }

  for (i = 0; i < MAX_TASKLEVELS; i++){//初始化各层的状态为0
    taskctl->level[i].running = 0;
    taskctl->level[i].now = 0;
  }

  task = task_alloc();//分配一个任务给当前
  task->flags = 2;//运行状态，活动中
  task->priority = 2;//0.02s
  task->level = 0;//最高层
  task_add(task);
  task_switchsub();//level 设置

  load_tr(task->sel);
  task_timer = timer_alloc();//分配task切换timer
  timer_settime(task_timer, task->priority);//设置任务切换时间,并且应用优先级
  
  //添加闲置任务
  struct TASK *idle;
  idle = task_alloc();
  idle->tss.esp = memman_alloc_4k(memman, 64*1024) + 64*1026;
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

void task_run(struct TASK *task, int level, int priority)
{
  if (level < 0) {//异常处理
    level = task->level;//不改变level
  }

  if (priority > 0) {//异常处理
    task->priority = priority;
  }

  if (task->flags == 2 && task->level != level) {//改变活动中的level
    task_remove(task);//移除后其他flags为1，后面处理
  }
  if (task->flags != 2) {
    //唤醒
    task->level = level;
    task_add(task);//添加到level中运行
  }
  
  taskctl->lv_change = 1;//下次任务切换时检查level
  
  return;
}

void task_switch(void)
{
  struct TASKLEVEL *tl = &taskctl->level[taskctl->now_lv];
  struct TASK *new_task, *now_task = tl->tasks[tl->now];

  tl->now++;
  if (tl->now == tl->running) {//如果是最后一个，下次就选择第一个task
    tl->now = 0;//轮询运行
  }
  if (taskctl->lv_change != 0) {
    task_switchsub();
    tl = &taskctl->level[taskctl->now_lv];
  }

  new_task = tl->tasks[tl->now];
  timer_settime(task_timer, new_task->priority);//重写task timer
 
  if (new_task != now_task) {
    farjmp(0, new_task->sel);
  }
  return ;
}


void task_sleep(struct TASK *task)
{
  struct TASK *now_task;
  if (task->flags == 2) {//如果是运行状态。。。
    now_task = task_now();
    task_remove(task);
    if (task == now_task) {//如果睡眠时当前任务，需要进行任务切换
      task_switchsub();
      now_task = task_now();//获取切换后的任务
      farjmp(0, now_task->sel);
    }
  }
  return;
}

//取得现在活动的task
struct TASK *task_now(void)
{
  struct TASKLEVEL *tl = &taskctl->level[taskctl->now_lv];
  return tl->tasks[tl->now];
}

//向TASKLEVEL添加一个任务
void task_add(struct TASK *task)
{
  struct TASKLEVEL *tl = &taskctl->level[task->level];
  tl->tasks[tl->running] = task;
  tl->running++;
  task->flags = 2;//标记为活动中
  return;
}

void task_remove(struct TASK *task)
{
  int i;
  struct TASKLEVEL *tl = &taskctl->level[task->level];

  //寻找task所在的位置
  for (i = 0; i < tl->running; i++) {
    if (task == tl->tasks[i]) {
      break;
    }
  }

  tl->running--;
  if (i < tl->now) {
    tl->now--;
  }
  if (tl->now >= tl->running) {//异常情况
    tl->now = 0;
  }
  task->flags = 1;

  //移动
  for ( ;i < tl->running; i++) {
    tl->tasks[i] = tl->tasks[i+1];
  }
}

static void task_switchsub(void) 
{
  int i;
  //寻找最上层的leve
  for (i = 0; i < MAX_TASKLEVELS; i++) {
    if (taskctl->level[i].running > 0) {
      break;//找到在运行的level
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
