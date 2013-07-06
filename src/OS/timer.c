#include "timer.h"
#include "int.h"
#include "naskfunc.h"
#include "fifo.h"

#define PIT_CTRL 0X0043
#define PIT_CNT0 0X0040

void init_pit(void)
{
  io_out8(PIT_CTRL, 0x34);
  
  //设置为0x2e9c定时器产生频率大约为100HZ
  io_out8(PIT_CNT0, 0x9c);
  io_out8(PIT_CNT0, 0x2e);

  timerctl.count = 0;
  timerctl.timeout = 0;
}

struct TIMERCTL timerctl;
void inthandler20(int *esp)
{
  io_out8(PIC0_OCW2, 0X60);//把IRQ-00信号接收完了的信息通知给PIC
  timerctl.count++;
  
  if (timerctl.timeout > 0){//如果设定了超时
    timerctl.timeout--;
    if (timerctl.timeout == 0) {//超时已经到
      fifo8_put(timerctl.fifo, timerctl.data);
    }
  }
  return;
}

void settimer(unsigned int timeout, struct FIFO8 *fifo, unsigned char data)
{
  int eflags;
  eflags = io_load_eflags();
  io_cli();

  timerctl.timeout = timeout;
  timerctl.fifo = fifo;
  timerctl.data = data;
  io_store_eflags(eflags);
  return;
}
