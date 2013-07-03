#include <stdio.h>
#include "bootpack.h"

void HariMain(void)
{
  //bootinfo struct pointer
  struct BOOTINFO *binfo = (struct BOOTINFO *)0x0ff0;
  char s[50], mcursor[256];
  char keybuf[32], mousebuf[128];
  int mx, my;
  unsigned char i;
    
  //鼠标相关，mouse_phase,鼠标状态；
  struct MOUSE_DEC mdec;

  init_gdtidt();
  init_pic();//初始化PIC
  io_sti();//PIC初始化完成，开中断

  fifo8_init(&keyfifo, sizeof(keybuf), keybuf);
  fifo8_init(&mousefifo, sizeof(mousebuf), mousebuf);
  
  io_out8(PIC0_IMR, 0xf9); /* PIC1打开中断(11111001) */
  io_out8(PIC1_IMR, 0xef); /* 打开键盘中断(11101111) */
  
  init_keyboard();
  
  init_palette();
  init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);
  init_mouse_cursor8(mcursor, COL8_008484);  
  
  //mouse init
  enable_mouse(&mdec);

  unsigned int memtotal;
  struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;

  memtotal = memtest(0x00400000, 0xbfffffff);//获取内存真实的大小
  memman_init(memman);
  memman_free(memman, 0x00001000, 0x0009e000);
  memman_free(memman, 0x00400000, memtotal - 0x00400000);

  //图层控制器初始化
  struct SHTCTL *shtctl;
  shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);

  struct SHEET *sht_back, *sht_mouse;//背景和鼠标图层
  sht_back = sheet_alloc(shtctl);
  sht_mouse = sheet_alloc(shtctl);
  
  unsigned char *buf_back, buf_mouse[256];
  buf_back = (unsigned char *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
  
  sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);//没有透明色
  sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);//透明色号99
  
  init_screen8(buf_back, binfo->scrnx, binfo->scrny);
  init_mouse_cursor8(buf_mouse, 99);//背景色号99

  sheet_slide(shtctl, sht_back, 0, 0);//移动背景图层，同时显示出来

  //鼠标初始位置
  mx = (binfo->scrnx - 16) / 2;
  my = (binfo->scrny - 16) / 2;
  
  sheet_slide(shtctl, sht_mouse, mx, my);//移动鼠标到中心，显示出来
  //
  sheet_updown(shtctl, sht_back, 0);
  sheet_updown(shtctl, sht_mouse, 1);
  
  sprintf(s, "(%3d, %3d)", mx, my);
  putfont8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
  sprintf(s, "memory %dMB free : %dKB",
	  memtotal/ (1024*1024), memman_total(memman) / 1024);
  putfont8_asc(buf_back, binfo->scrnx, 0 , 32, COL8_FFFFFF, s);
  sheet_refresh(shtctl);

  for (;;) {
    io_cli();
    
    if (0 == fifo8_status(&keyfifo) + fifo8_status(&mousefifo)){
      io_stihlt();
    } else {
      if (fifo8_status(&keyfifo) != 0) {
	i = fifo8_get(&keyfifo);
	io_sti();

	sprintf(s, "%02X", i);
	boxfill8(buf_back, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
	putfont8_asc(buf_back, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
	sheet_refresh(shtctl);
      }
      if (fifo8_status(&mousefifo) != 0) {
	i = fifo8_get(&mousefifo);
	io_sti();

	//鼠标的3个字节都齐全了，显示出来
	if (mouse_decode(&mdec, i) != 0) {
	  
	  sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
	  if (mdec.btn & 0x01) {
	    s[1] = 'L';
	  } 
	  if (mdec.btn & 0x02) {
	    s[3] = 'R';
	  } 
	  if (mdec.btn & 0x04) {
	    s[2] = 'C';
	  } 
	  
	  boxfill8(buf_back, binfo->scrnx, COL8_008484, 32, 16, 32 + 15*8 - 1, 31);
	  putfont8_asc(buf_back, binfo->scrnx, 32, 16, COL8_FFFFFF, s);

	  //鼠标指针的移动
	  boxfill8(binfo->vram, binfo->scrnx, COL8_008484, mx, my, mx + 15, my + 15);//隐藏鼠标
	  //计算鼠标新的位置
	  mx += mdec.x;
	  my += mdec.y;
	  if (mx < 0) {
	    mx = 0;
	  }
	  if (my < 0) {
	    my = 0;
	  }
	  if (mx > binfo->scrnx - 16) {
	    mx = binfo->scrnx - 16;
	  }
	  if (my > binfo->scrny - 16) {
	    my = binfo->scrny - 16;
	  }
	  sprintf(s, "(%3d, %3d)", mx, my);
	  boxfill8(buf_back, binfo->scrnx, COL8_008484, 0, 0, 79, 15);//隐藏坐标
	  putfont8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s);//显示坐标
	  
	  //滑动鼠标显示，包含sheet_reflush
	  sheet_slide(shtctl, sht_mouse, mx, my);
	}
      }
    }
  }
}
