#include <stdio.h>
#include "bootpack.h"

void HariMain(void)
{
  //bootinfo struct pointer
  struct BOOTINFO *binfo = (struct BOOTINFO *)0x0ff0;
  char s[50], mcursor[256], keybuf[32];
  int mx, my;
  unsigned char i, j ;
    
  fifo8_init(&keyfifo, sizeof(keybuf), keybuf);
  
  init_gdtidt();
  init_pic();//��ʼ��PIC
  io_sti();//���ж�

  io_out8(PIC0_IMR, 0xf9); /* PIC1���ж�(11111001) */
  io_out8(PIC1_IMR, 0xef); /* �򿪼����ж�(11101111) */

  init_palette();
  init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
  mx = (binfo->scrnx - 16) / 2;
  my = (binfo->scrny - 16) / 2;
  init_mouse_cursor8(mcursor, COL8_008484);  
  putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
  sprintf(s, "(%d, %d)", mx, my);
  putfont8_asc(binfo->vram, binfo->scrnx, 0,0, COL8_FFFFFF, s);

  for (;;) {
    io_cli();
    
    if (0 == fifo8_status(&keyfifo)){
      io_stihlt();
    } else {
      i = fifo8_get(&keyfifo);
      io_sti();
      sprintf(s, "%02X", i);
      boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
      putfont8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
    }
  }
}
