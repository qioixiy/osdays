#ifndef _SHEET_H
#define _SHEET_H
#include "memory.h"

struct SHEET{
  unsigned char *buf;
  int bxsize, bysize;//图层的大小
  int vx0, vy0;//图层在画面上的相对位置
  int col_inv;//透明色色号
  int height;//图层高度
  int flags;//图层有关的设定信息
  struct TASK *task;//图层相关的任务

  struct SHTCTL *ctl;//sheet control member with the sheet
};

#define MAX_SHEETS 256//最多的图层数

struct SHTCTL {
  unsigned char *vram;//VRAM地址
  unsigned char *map;//
  int xsize, ysize;//大小
  int top;//最上层的高度
  struct SHEET *sheets[MAX_SHEETS];//表示图层的顺序结构
  struct SHEET sheets0[MAX_SHEETS];//实际图层结构数组
};

struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize);
struct SHEET *sheet_alloc(struct SHTCTL *ctl);
void sheet_free(struct SHEET *sht);
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv);
void sheet_updown(struct SHEET *sht, int height);
void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1);
void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0, int h1);
void sheet_refreshmap(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0);
void sheet_slide(struct SHEET *sht, int vx0, int vy0);
//*/
#endif
