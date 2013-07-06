#include "timer.h"
#include "int.h"
#include "naskfunc.h"

#define PIT_CTRL 0X0043
#define PIT_CNT0 0X0040

void init_pit(void)
{
  io_out8(PIT_CTRL, 0x34);
  
  //设置为0x2e9c定时器产生频率大约为100HZ
  io_out8(PIT_CNT0, 0x9c);
  io_out8(PIT_CNT0, 0x2e);
}

void inthandler20(int *esp)
{
  io_out8(PIC0_OCW2, 0X60);//把IRQ-00信号接收完了的信息通知给PIC

  return;
}
