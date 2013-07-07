#include "timer.h"
#include "int.h"
#include "naskfunc.h"
#include "fifo.h"
#include "mtask.h"

#define PIT_CTRL 0X0043
#define PIT_CNT0 0X0040

#define TIMER_FLAGS_NOUSE 0//未使用
#define TIMER_FLAGS_ALLOC 1//已分配状态
#define TIMER_FLAGS_USING 2//定时器运行中

struct TIMERCTL timerctl;

void init_pit(void)
{
  int i = 0;

  io_out8(PIT_CTRL, 0x34);  
  //设置为0x2e9c定时器产生频率大约为100HZ
  io_out8(PIT_CNT0, 0x9c);
  io_out8(PIT_CNT0, 0x2e);

  timerctl.count = 0;
  timerctl.next = 0xffffffff;
  for(i = 0; i<MAX_TIMER; i++) {
    timerctl.timers0[i].flags = TIMER_FLAGS_NOUSE;//未使用
  }
  
  struct TIMER *t;
  t = timer_alloc();
  t->timeout = 0xffffffff;
  t->flags = TIMER_FLAGS_USING;
  t->next = 0;
  timerctl.t0 = t;//因为现在只有此哨兵timer
  timerctl.next = 0xffffffff;//下一个超时时刻就是0xffffffff
  
  return;
}

struct TIMER *timer_alloc(void)
{
  int i;
  for(i = 0; i < MAX_TIMER; i++){
    if (timerctl.timers0[i].flags == TIMER_FLAGS_NOUSE) {
      timerctl.timers0[i].flags = TIMER_FLAGS_ALLOC;
      return &timerctl.timers0[i];
    }
  }
  return 0;
}

void timer_free(struct TIMER *timer)
{
  timer->flags = TIMER_FLAGS_NOUSE;
}

void timer_init(struct TIMER *timer,struct FIFO32 *fifo, unsigned int data)
{
  timer->fifo = fifo;
  timer->data = data;
  return ;
}

void timer_settime(struct TIMER *timer, unsigned int timeout)
{
  //由于加入哨兵timer，插入timer只能在最前面或者中间
  int e;
  struct TIMER *s, *t;
  
  timer->timeout = timeout + timerctl.count;
  timer->flags = TIMER_FLAGS_USING;
  e = io_load_eflags();
  io_cli();

  t = timerctl.t0;
  if (timer->timeout <= t->timeout) {//如果是最近的定时器，放在最前面
    timerctl.t0 = timer;
    timer->next = t;
    timerctl.next = timer->timeout;
    io_store_eflags(e);
    return;
  }
  
  //搜寻插入的位置
  for(;;) {
    s = t;
    t = t->next;
    if (t == 0) {
      break;//最后面插入
    }
    if (timer->timeout <= t->timeout) {
      //在t之前插入, s,t之间
      s->next = timer;
      timer->next = t;
      io_store_eflags(e);
      return;
    }
  }
}

void inthandler20(int *esp)
{
  char ts = 0;
  struct TIMER *timer;
  io_out8(PIC0_OCW2, 0X60);//把IRQ-00信号接收完了的信息通知给PIC
  timerctl.count++;
  if (timerctl.count < timerctl.next) {
    return ;//还不到下一时刻
  }

  timer = timerctl.t0;//首先把最前面的地址给timer
  for (;;) {
    //timer都是USING 状态不需要确认
    if (timer->timeout > timerctl.count) {
      break;
    }
    //超时
    timer->flags = TIMER_FLAGS_ALLOC;
    if (timer != task_timer) {
      fifo32_put(timer->fifo, timer->data);
    } else {
      ts = 1;//mt_timer超时
    }
    timer = timer->next;//指向下一个定时器
  }
  
  timerctl.t0 = timer;
  //timer next 设定
  timerctl.next = timerctl.t0->timeout;

  if (ts != 0) {
    task_switch();
  }
  return;
}

//时间溢出调整
void timer_adjust(void)
{
  int t0 = timerctl.count;
  int i = 0;
  io_cli();//禁止中断
  
  timerctl.count -= t0;
  for (i = 0; i < MAX_TIMER; i++) {
    if (timerctl.timers0[i].flags == TIMER_FLAGS_USING) {
      if (timerctl.timers0[i].timeout != 0xffffffff)
      timerctl.timers0[i].timeout -= t0;
    }
  }
  io_sti();
}
