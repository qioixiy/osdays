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
  for(i = 0; i<MAX_TIMER; i++) {
    timerctl.timer[i].flags = TIMER_FLAGS_NOUSE;//未使用
  }
  return;
}

struct TIMER *timer_alloc(void)
{
  int i;
  for(i = 0; i < MAX_TIMER; i++){
    if (timerctl.timer[i].flags == TIMER_FLAGS_NOUSE) {
      timerctl.timer[i].flags = TIMER_FLAGS_ALLOC;
      return &timerctl.timer[i];
    }
  }
  return 0;
}

void timer_free(struct TIMER *timer)
{
  timer->flags = TIMER_FLAGS_NOUSE;
}

void timer_init(struct TIMER *timer,struct FIFO8 *fifo, unsigned char data)
{
  timer->fifo = fifo;
  timer->data = data;
  return ;
}

void timer_settime(struct TIMER *timer, unsigned int timeout)
{
  timer->timeout = timeout;
  timer->flags = TIMER_FLAGS_USING;
  return;
}

void inthandler20(int *esp)
{
  int i;
  io_out8(PIC0_OCW2, 0X60);//把IRQ-00信号接收完了的信息通知给PIC
  timerctl.count++;
  
  for(i = 0; i < MAX_TIMER; i++) {
    if (timerctl.timer[i].flags == TIMER_FLAGS_USING ){//如果设定了超时
      timerctl.timer[i].timeout--;
      if (timerctl.timer[i].timeout == 0) {//超时已经到
	timerctl.timer[i].flags = TIMER_FLAGS_ALLOC;
	fifo8_put(timerctl.timer[i].fifo, timerctl.timer[i].data);
      }
    }
  }
  return;
}
