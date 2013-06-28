#include <stdio.h>
#include "int.h"
#include "bootpack.h"
#include "graphic.h"

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

//定义一个全局keyboard buffer
struct KEYBUF keybuf;

//keyboard int
void inthandler21(int *esp)
{
#define PORT_KEYDAT		0x0060

  struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
  unsigned char data;
  io_out8(PIC0_OCW2, 0x61);//通知PIC IRQ-01已经受理完了
  data = io_in8(PORT_KEYDAT);

  if (keybuf.len < 32){
    keybuf.data[keybuf.next_w] = data;
    keybuf.len++;
    keybuf.next_w++;
    if (keybuf.next_w == 32) {
      keybuf.next_w = 0;
    }
  }

  return;
}

void inthandler2c(int *esp)
{
  struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
  
  boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 0, 0, 32*8-1, 15);
  putfont8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, "INT 2C (IRQ-12):PS/2 PS/2 mouse");
  for (;;) {
    io_hlt();
  }
}

void inthandler27(int *esp)
{
  io_out8(PIC0_OCW2, 0X67);
  return;
}
