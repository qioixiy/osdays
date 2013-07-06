#include <stdio.h>
#include "int.h"
#include "bootpack.h"
#include "graphic.h"
#include "fifo.h"

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

//����һ��ȫ��keyboard fifo
struct FIFO32 *keyfifo;
unsigned int keydata0;
//keyboard int
void inthandler21(int *esp)
{
  unsigned int data;
  io_out8(PIC0_OCW2, 0x61);//֪ͨPIC IRQ-01�Ѿ���������
  data = io_in8(PORT_KEYDAT);

  fifo32_put(keyfifo, data+keydata0);
  return;
}

struct FIFO32 *mousefifo;
unsigned int mousedata0;
//mouse int
void inthandler2c(int *esp)
{
  unsigned int data;
  io_out8(PIC1_OCW2, 0x64);//֪ͨPIC1 IRQ-12�����Ѿ����
  io_out8(PIC0_OCW2, 0x62);//֪ͨPIC0 IRQ-02�����Ѿ����
  
  data = io_in8(PORT_KEYDAT);
  fifo32_put(mousefifo, data+mousedata0);
  
  return;
}

void inthandler27(int *esp)
{
  io_out8(PIC0_OCW2, 0X67);
  return;
}
