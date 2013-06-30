#include <stdio.h>
#include "bootpack.h"

void wait_KBC_sendready(void)
{
  //等待键盘控制电路准备完毕
  for (;;) {
    if (( io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
      break;
    }
  }
  return;
}

//鼠标设备描述结构
struct MOUSE_DEC {
  unsigned char buf[3];//鼠标的3字节buffer
  unsigned char phase;//鼠标接收的状态
};
void enable_mouse(struct MOUSE_DEC *mdec);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char data);

void enable_mouse(struct MOUSE_DEC *mdec)
{
  //激活鼠标
  wait_KBC_sendready();
  io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
  wait_KBC_sendready();
  io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);

  mdec->phase = 0;//正常情况下键盘控制器会返回0xfa
  return;
}

int mouse_decode(struct MOUSE_DEC *mdec, unsigned char data)
{
  int res = 0;

  if (0 == mdec->phase ) {
    //等待鼠标的0xfa状态
    if (data == 0xfa) {
      mdec->phase = 1;
    }
    res = 0;
  }else if(1 == mdec->phase){
    //等待鼠标的第一个字节
    mdec->buf[0] = data;
    mdec->phase = 2;
    res = 0;
  } else if (2 == mdec->phase) {
    //等待鼠标的第2个字节
    mdec->buf[1] = data;
    mdec->phase = 3;
    res = 0;
  } else if (mdec->phase == 3) {
    //等待鼠标的第3个字节
    mdec->buf[2] = data;
    mdec->phase = 1;
    res = 1;
  }
  
  return res;
}

void init_keyboard(void)
{
  //初始化键盘控制电路
  wait_KBC_sendready();
  io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
  wait_KBC_sendready();
  io_out8(PORT_KEYDAT, KBC_MODE);
  
  return;
}

void HariMain(void)
{
  //bootinfo struct pointer
  struct BOOTINFO *binfo = (struct BOOTINFO *)0x0ff0;
  char s[50], mcursor[256];
  char keybuf[32], mousebuf[128];
  int mx, my;
  unsigned char i, j ;
    
  //鼠标相关，mouse_phase,鼠标状态；
  struct MOUSE_DEC mdec;

  fifo8_init(&keyfifo, sizeof(keybuf), keybuf);
  fifo8_init(&mousefifo, sizeof(mousebuf), mousebuf);
  
  init_gdtidt();
  init_pic();//初始化PIC
  io_sti();//开中断

  io_out8(PIC0_IMR, 0xf9); /* PIC1打开中断(11111001) */
  io_out8(PIC1_IMR, 0xef); /* 打开键盘中断(11101111) */
  
  init_keyboard();
  
  init_palette();
  init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
  mx = (binfo->scrnx - 16) / 2;
  my = (binfo->scrny - 16) / 2;
  init_mouse_cursor8(mcursor, COL8_008484);  
  putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
  sprintf(s, "(%d, %d)", mx, my);
  putfont8_asc(binfo->vram, binfo->scrnx, 0,0, COL8_FFFFFF, s);

  //mouse init
  enable_mouse(&mdec);

  for (;;) {
    io_cli();
    
    if (0 == fifo8_status(&keyfifo) + fifo8_status(&mousefifo)){
      io_stihlt();
    } else {
      if (fifo8_status(&keyfifo) != 0) {
	i = fifo8_get(&keyfifo);
	io_sti();

	sprintf(s, "%02X", i);
	boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
	putfont8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
      }
      if (fifo8_status(&mousefifo) != 0) {
	i = fifo8_get(&mousefifo);
	io_sti();

	if (mouse_decode(&mdec, i) != 0) {
	  //鼠标的3个字节都齐全了，显示出来
	  sprintf(s, "%02X,%02X,%02X", mdec.buf[0], mdec.buf[1], mdec.buf[2]);
	  boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 32 + 8 * 8 - 1, 31);
	  putfont8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
	}
      }
    }
  }
}
