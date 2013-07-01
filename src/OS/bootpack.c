#include <stdio.h>
#include "bootpack.h"

unsigned int memtest(unsigned int start, unsigned int end)
{
  char flg486 = 0;
  unsigned int eflg, cr0, i;

  //确认CPU是386还是486以上的
  eflg = io_load_eflags();
  eflg |= EFLAGS_AC_BIT;//AC-bit = 1
  io_store_eflags(eflg);
  eflg = io_load_eflags();

  //如果是386，即使是设定AC=1，AC的值还是会自动回到0
  if ((eflg & EFLAGS_AC_BIT) != 0) {
      flg486 = 1;
  }
  eflg &= ~EFLAGS_AC_BIT;//AC-bit = 0
  io_store_eflags(eflg);

  if (flg486 != 0) {
    cr0 = load_cr0();
    cr0 |= CR0_CACHE_DISABLE;//禁止缓冲
    store_cr0(cr0);
  }

  i = memtest_sub(start, end);

  if (flg486 != 0) {
    cr0 = load_cr0();
    cr0 &= ~CR0_CACHE_DISABLE;//允许缓冲
    store_cr0(cr0);
  }

  return i;
}

//*
unsigned int memtest_sub(unsigned int start, unsigned int end)
{
  unsigned int i, *p, old, pat0 = 0xaa55aa55, pat1 = 0x55aa55aa;

  for (i = start; i <= end; i += 1000) {
    p = (unsigned int *)(i + 0xffc);
    old = *p;//先记住修改前的值
    *p = pat0;//试写
    *p ^= 0xffffffff;//反转
    if(*p != pat1) {//检查反转结果
not_memory:
      *p = old;
      break;
    }
    *p ^= 0xffffffff;//再次反转
    if (*p != pat0){ //检查值是否反转
      goto not_memory;
    }
    *p = old;
  }
  return i;
}//*/

void HariMain(void)
{
  //bootinfo struct pointer
  struct BOOTINFO *binfo = (struct BOOTINFO *)0x0ff0;
  char s[50], mcursor[256];
  char keybuf[32], mousebuf[128];
  int mx, my;
  unsigned char i, j ;
  unsigned int mem_size;
    
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
  init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
  mx = (binfo->scrnx - 16) / 2;
  my = (binfo->scrny - 16) / 2;
  init_mouse_cursor8(mcursor, COL8_008484);  
  putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
  sprintf(s, "(%d, %d)", mx, my);
  putfont8_asc(binfo->vram, binfo->scrnx, 0,0, COL8_FFFFFF, s);

  //mouse init
  enable_mouse(&mdec);

  mem_size = memtest(0x00400000, 0xbfffffff) / (1024 * 1024);
  sprintf(s, "memory %dMB", mem_size);
  putfont8_asc(binfo->vram, binfo->scrnx, 0, 32, COL8_FFFFFF, s);

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

	  boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 32 + 15*8 - 1, 31);
	  putfont8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);

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
	  boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 0, 79, 15);//隐藏坐标
	  putfont8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);//显示坐标
	  putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);//描画鼠标
	}
      }
    }
  }
}
