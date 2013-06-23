#include <stdio.h>

void io_hlt(void);//暂停系统
void io_cli(void);//关闭中断
void io_out8(int port, int data);
int  io_load_eflags(void);//加载标志位
void io_store_eflags(int eflags);//还原标志位

void init_palette(void);//设定调色板
void init_screen(unsigned char *vram, int xsize, int ysize);//初始化窗口
void init_mouse_cursor8(char *mouse, char bc);//准备鼠标指针【16×16】
void set_palette(int start, int end, unsigned char *rgb);
void boxfill8(unsigned char *vram, int xsize, unsigned char c,int x0, int y0, int x1, int y1);
void putblock8_8(char *vram, int vxsize, int pxsize,
		 int pysize, int px0, int py0, char *buf, int bxsize);
void putfont8(unsigned char *vram, //display a char
	      int xsize, int x, int y, 
	      char color, char *font);
void putfont8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *str);//put string

//about GDT and IDT
struct SEGMENT_DESCRIPTOR {
  short limit_low, base_low;
  char base_mid, access_right;
  char limit_high, base_high;
};

struct GATE_DESCRIPTOR {
  short offset_low, selector;
  char dw_count, access_right;
  short offset_high;
};

void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR * gd, int offset, int selector, int ar);

#define COL8_000000 0
#define COL8_FF0000 1
#define COL8_00FF00 2
#define COL8_FFFF00 3
#define COL8_0000FF 4
#define COL8_FF00FF 5
#define COL8_00FFFF 6
#define COL8_FFFFFF 7
#define COL8_C6C6C6 8
#define COL8_840000 9
#define COL8_008400 10
#define COL8_848400 11
#define COL8_000084 12
#define COL8_840084 13
#define COL8_008484 14
#define COL8_848484 15

struct BOOTINFO {
  char cyls, leds, vmode, reserve;
  short scrnx,scrny;
  char *vram;
};

void init_gdtidt(void)
{
  //计划将0x00270000到0x0027ffff作为GDT使用
  struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)0x00270000;
  //计划将0x0026f800到0x0026ffff作为IDT使用
  struct GATE_DESCRIPTOR *idt = (struct GATE_DESCRIPTOR *)0x0026f800;
  int i = 0;

  //GDT的初始化
  for (i = 0; i < 8192; i++) {
    set_segmdesc(gdt + i, 0, 0, 0);
  }
  set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, 0x4092);
  set_segmdesc(gdt + 2, 0x0007ffff, 0x00280000, 0x409a);
  load_gdtr(0xffff, 0x00270000);
  
  //IDT的初始化
  for (i = 0; i < 256; i++) {
    set_gatedesc(idt + i, 0, 0, 0);
  }
  load_idtr(0x7ff, 0x0026f800);

  return;
}

void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar)
{
  if (limit > 0xfffff) {
    ar |= 0x8000;//G_bit = 1
    limit /= 0x1000;
  }
  sd->limit_low = limit & 0xffff;
  sd->base_low  = base & 0xffff;
  sd->base_mid  = (base >> 16) & 0xff;
  sd->access_right = ar & 0xff;
  sd->limit_high = ((limit >> 16) & 0x0f) | ((ar>>8) & 0xf0);
  sd->base_high = (base>>24) & 0xff;

  return;
}

void set_gatedesc(struct GATE_DESCRIPTOR * gd, int offset, int selector, int ar)
{
  gd->offset_low = offset & 0xffff;
  gd->selector = selector;
  gd->dw_count = (ar>>8) & 0xff;
  gd->access_right = ar & 0xff;
  gd->offset_high = (offset>>16) & 0xffff;
  
  return;
}

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

void init_screen(unsigned char *vram, int xsize, int ysize)
{
  boxfill8(vram, xsize, COL8_008484,  0,         0,          xsize -  1, ysize - 29);
  boxfill8(vram, xsize, COL8_C6C6C6,  0,         ysize - 28, xsize -  1, ysize - 28);
  boxfill8(vram, xsize, COL8_FFFFFF,  0,         ysize - 27, xsize -  1, ysize - 27);
  boxfill8(vram, xsize, COL8_C6C6C6,  0,         ysize - 26, xsize -  1, ysize -  1);

  boxfill8(vram, xsize, COL8_FFFFFF,  3,         ysize - 24, 59,         ysize - 24);
  boxfill8(vram, xsize, COL8_FFFFFF,  2,         ysize - 24,  2,         ysize -  4);
  boxfill8(vram, xsize, COL8_848484,  3,         ysize -  4, 59,         ysize -  4);
  boxfill8(vram, xsize, COL8_848484, 59,         ysize - 23, 59,         ysize -  5);
  boxfill8(vram, xsize, COL8_000000,  2,         ysize -  3, 59,         ysize -  3);
  boxfill8(vram, xsize, COL8_000000, 60,         ysize - 24, 60,         ysize -  3);

  boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 24, xsize -  4, ysize - 24);
  boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 23, xsize - 47, ysize -  4);
  boxfill8(vram, xsize, COL8_FFFFFF, xsize - 47, ysize -  3, xsize -  4, ysize -  3);
  boxfill8(vram, xsize, COL8_FFFFFF, xsize -  3, ysize - 24, xsize -  3, ysize -  3);
}

void init_palette(void)
{
  static unsigned char table_rgb[16*3]= {
    0x00, 0x00, 0x00,//0:黑色
    0xff, 0x00, 0x00,//1:亮红色
    0x00, 0xff, 0x00,//2:亮绿色
    0xff, 0xff, 0x00,//3:亮黄色
    0x00, 0x00, 0xff,//4:亮蓝色
    0xff, 0x00, 0xff,//5:亮紫色
    0x00, 0xff, 0xff,//6:浅亮色
    0xff, 0xff, 0xff,//7:白色
    0xc6, 0xc6, 0xc6,//8:亮灰色
    0x84, 0x00, 0x00,//9:暗红色
    0x00, 0x84, 0x00,//10:暗绿色
    0x84, 0x84, 0x00,//11:暗黄色
    0x00, 0x00, 0x84,//12:暗青色
    0x84, 0x00, 0x84,//13:暗紫色
    0x00, 0x84, 0x84,//14:浅暗色
    0x84, 0x84, 0x84,//15:暗灰色
  };
  set_palette(0, 15, table_rgb);
  return;
}

void init_mouse_cursor8(char *mouse, char bc)//准备鼠标指针【16×16】
{
  static char cursor[16][16] = {
    "**************..",
    "*OOOOOOOOOOO*...",
    "*OOOOOOOOOO*....",
    "*OOOOOOOOO*.....",
    "*OOOOOOOO*......",
    "*OOOOOOO*.......",
    "*OOOOOOO*.......",
    "*OOOOOOOO*......",
    "*OOOO**OOO*.....",
    "*OOO*..*OOO*....",
    "*OO*....*OOO*...",
    "*O*......*OOO*..",
    "**........*OOO*.",
    "*..........*OOO*",
    "............*OO*",
    ".............***",
  };

  int x, y;
  for (y = 0; y < 16; y++) {
    for (x = 0; x < 16; x++) {
      if (cursor[y][x] == '*') {
	mouse[y * 16 + x] = COL8_000000;
      } else if (cursor[y][x] == 'O') {
	mouse[y *16 + x] = COL8_FFFFFF;
      } else if (cursor[y][x] == '.') {
	mouse[y *16 + x] = bc;
      }
    }
  }
  return;
}

void set_palette(int start, int end, unsigned char *rgb)
{
  int i, eflags;
  eflags = io_load_eflags();//记录中断许可标志的值
  io_cli();//将中断许可标志置为0， 禁止中断
  io_out8(0x03c8, start);
  
  for ( i = start; i <= end; i ++ ) {
    io_out8(0x03c9,rgb[0] / 4);
    io_out8(0x03c9,rgb[1] / 4);
    io_out8(0x03c9,rgb[2] / 4);
    rgb += 3;
  }
  io_store_eflags(eflags);//复原中断许可标志
}

void boxfill8(unsigned char *vram, int xsize, unsigned char c,int x0, int y0, int x1, int y1)
{
  int x, y;
  for (y = y0; y <= y1; y++){
    for (x= x0; x <=x1; x++){
      vram[y * xsize + x] = c;
    }
  }
}

void putfont8(unsigned char *vram, int xsize, int x, int y, char color, char *font)
{
  int i;
  char *p, d; //data
  
  for (i = 0; i < 16; i++){
    p = vram + (y + i) * xsize + x;
    d = font[i];

    if ((d & 0x80) != 0) {p[0] = color; }
    if ((d & 0x40) != 0) {p[1] = color; }
    if ((d & 0x20) != 0) {p[2] = color; }
    if ((d & 0x10) != 0) {p[3] = color; }
    if ((d & 0x08) != 0) {p[4] = color; }
    if ((d & 0x04) != 0) {p[5] = color; }
    if ((d & 0x02) != 0) {p[6] = color; }
    if ((d & 0x01) != 0) {p[7] = color; }
  }
  return;
}

void putfont8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *str)
{
  extern char hankaku[4096];
  for ( ; *str != 0x00; str++) {
    putfont8(vram, xsize, x, y, c, hankaku + *str * 16);
    x += 8;
  }
  return;
}

void putblock8_8(char *vram, int vxsize, int pxsize,
		 int pysize, int px0, int py0, char *buf, int bxsize)
{
  int x, y;
  for (y = 0; y < pysize; y++) {
    for (x = 0; x < pxsize; x++) {
      vram[(py0 + y) * vxsize + (px0 + x)] = buf[y * bxsize + x];
    }
  }
}
