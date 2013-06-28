#include <stdio.h>
#include "int.h"
#include "bootpack.h"
#include "graphic.h"

void init_pic(void)
{
  io_out8(PIC0_IMR, 0xff);//��ֹ�����ж�
  io_out8(PIC1_IMR, 0xff);//��ֹ�����ж�

  io_out8(PIC0_ICW1, 0x11);//���ش���ģʽ
  io_out8(PIC0_ICW2, 0x20);//irq0-7��INT20-27����
  io_out8(PIC0_ICW3, 0x01<<2);//PIC1��IRQ2����
  io_out8(PIC0_ICW4, 0x01);//�޻���ģʽ

  io_out8(PIC1_ICW1, 0x11);//���ش���ģʽ
  io_out8(PIC1_ICW2, 0x28);//irq8-15��INT28-2f����
  io_out8(PIC1_ICW3, 2);//PIC1��IRQ2����
  io_out8(PIC1_ICW4, 0x01);//�޻���ģʽ

  io_out8(PIC0_IMR, 0xfb);//11111011��PIC0-2�����ȫ����ֹ
  io_out8(PIC1_IMR, 0xff);//��ֹ�����ж�
  return;
}

//����һ��ȫ��keyboard buffer
struct KEYBUF keybuf;

//keyboard int
void inthandler21(int *esp)
{
#define PORT_KEYDAT		0x0060

  struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
  unsigned char data, s[4];
  io_out8(PIC0_OCW2, 0x61);//֪ͨPIC IRQ-01�Ѿ���������
  data = io_in8(PORT_KEYDAT);

  if (keybuf.next < sizeof(keybuf.data)) {
    keybuf.data[keybuf.next] = data;
    keybuf.next++;
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
