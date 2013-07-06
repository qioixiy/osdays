#include "timer.h"
#include "int.h"
#include "naskfunc.h"
#include "fifo.h"

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
  int e, i, j;
  timer->timeout = timeout + timerctl.count;
  timer->flags = TIMER_FLAGS_USING;
  e = io_load_eflags();
  io_cli();
  //搜索注册位置
  for (i = 0; i < MAX_TIMER; i++) {
    if (timerctl.timers[i]->timeout >= timer->timeout) {
      break;
    }
  }
  
  //i之后的向后移动
  for (j = timerctl.using; j > i; j--) {
    timerctl.timers[j] = timerctl.timers[j - 1];
  }
  
  timerctl.using++;
  //插入到空位
  timerctl.timers[i] = timer;
  timerctl.next = timerctl.timers[0]->timeout;
  
  io_store_eflags(e);
  return;
}

void inthandler20(int *esp)
{
  int i, j;
  io_out8(PIC0_OCW2, 0X60);//把IRQ-00信号接收完了的信息通知给PIC
  timerctl.count++;
  if (timerctl.count < timerctl.next) {
    return ;//还不到下一时刻
  }
  
  for (i = 0; i < timerctl.using; i++) {
    //timer都是USING 状态不需要确认
    if (timerctl.timers[i]->timeout > timerctl.count) {
      break;
    }
    //超时
    timerctl.timers[i]->flags = TIMER_FLAGS_ALLOC;
    fifo32_put(timerctl.timers[i]->fifo, timerctl.timers[i]->data);
  }
  
  //有i个定时器超时
  timerctl.using -= i;
  for (j = 0; j < timerctl.using; j++) {
    timerctl.timers[j] = timerctl.timers[i+j];
  }
  if (timerctl.using > 0) {
    timerctl.next = timerctl.timers[0]->timeout;
  } else {
    timerctl.next = 0xffffffff;
  }
  return;
}

//时间溢出调整
void timer_adjust(void)
{
  int t0 = timerctl.count;
  int i = 0;
  io_cli();
  
  timerctl.count -= t0;
  for (i = 0; i < MAX_TIMER; i++) {
    if (timerctl.timers0[i].flags == TIMER_FLAGS_USING) {
      timerctl.timers0[i].timeout -= t0;
    }
  }
  io_sti();
}
