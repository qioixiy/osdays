#include <stdio.h>
#include "int.h"
#include "bootpack.h"
#include "graphic.h"
#include "fifo.h"

void init_pic(void)
{
  io_out8(PIC0_IMR, 0xff);//禁止所有中断
  io_out8(PIC1_IMR, 0xff);//禁止所有中断

  io_out8(PIC0_ICW1, 0x11);//边沿触发模式
  io_out8(PIC0_ICW2, 0x20);//irq0-7由INT20-27接收
  io_out8(PIC0_ICW3, 0x01<<2);//PIC1由IRQ2连接
  io_out8(PIC0_ICW4, 0x01);//无缓冲模式

  io_out8(PIC1_ICW1, 0x11);//边沿触发模式
  io_out8(PIC1_ICW2, 0x28);//irq8-15由INT28-2f接收
  io_out8(PIC1_ICW3, 2);//PIC1由IRQ2连接
  io_out8(PIC1_ICW4, 0x01);//无缓冲模式

  io_out8(PIC0_IMR, 0xfb);//11111011，PIC0-2以外的全部禁止
  io_out8(PIC1_IMR, 0xff);//禁止所有中断
  return;
}

//定义一个全局keyboard fifo
struct FIFO8 keyfifo;

//keyboard int
void inthandler21(int *esp)
{
  unsigned char data;
  io_out8(PIC0_OCW2, 0x61);//通知PIC IRQ-01已经受理完了
  data = io_in8(PORT_KEYDAT);

  fifo8_put(&keyfifo, data);
  return;
}

struct FIFO8 mousefifo;
//mouse int
void inthandler2c(int *esp)
{
  unsigned char data;
  io_out8(PIC1_OCW2, 0x64);//通知PIC1 IRQ-12处理已经完成
  io_out8(PIC0_OCW2, 0x62);//通知PIC0 IRQ-02处理已经完成
  
  data = io_in8(PORT_KEYDAT);
  fifo8_put(&mousefifo, data);
  
  return;
}

void inthandler27(int *esp)
{
  io_out8(PIC0_OCW2, 0X67);
  return;
}
