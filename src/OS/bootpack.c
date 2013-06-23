#include <stdio.h>
#include "graphic.h"
#include "dsctbl.h"

struct BOOTINFO {
  char cyls, leds, vmode, reserve;
  short scrnx,scrny;
  char *vram;
};

extern char hankaku[4096];
void HariMain(void)
{
  //bootinfo struct pointer
  struct BOOTINFO *binfo = (struct BOOTINFO *)0x0ff0;
  char s[50], mcursor[256];
  int mx, my;

  init_palette();
  init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
  mx = (binfo->scrnx - 16) / 2;
  my = (binfo->scrny - 16) / 2;
  init_mouse_cursor8(mcursor, COL8_008484);  
  putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
  sprintf(s, "(%d, %d)", mx, my);
  putfont8_asc(binfo->vram, binfo->scrnx, 0,0, COL8_FFFFFF, s);
  
  for (;;) {
    io_hlt();
  }
}
