#include "memory.h"
#include "sheet.h"

struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize)
{
  struct SHTCTL *ctl;
  int i;
   
  ctl = (struct SHTCTL *)memman_alloc_4k(memman, sizeof (struct SHTCTL));
  if (ctl == 0) {
    goto err;
  }
  
  ctl->map = (unsigned char *)memman_alloc_4k(memman, xsize * ysize);
  if (!ctl->map) {
    goto err;
  }
  
  ctl->vram = vram;
  ctl->xsize = xsize;
  ctl->ysize = ysize;
  ctl->top = -1;//一个sheet都没有
  for (i = 0; i < MAX_SHEETS; i++) {
    ctl->sheets0[i].flags = 0;//标记为未使用状态
    ctl->sheets0[i].ctl = ctl;//将每个sheet的controler关联到自己
  }
 err:
 return ctl;
}

#define SHEET_USE 1
struct SHEET *sheet_alloc(struct SHTCTL *ctl)
{
  struct SHEET *sht;
  int i;
  for(i = 0; i < MAX_SHEETS; i++) {
    if (ctl->sheets0[i].flags == 0) {//未使用的图层
      sht = &ctl->sheets0[i];
      sht->flags = SHEET_USE;//标记为使用中
      sht->height = -1;//分配先隐藏
      return sht;
    }
  }
  return 0;
}

void sheet_free(struct SHEET *sht)
{
  if (sht->height >= 0) {
    sheet_updown(sht, -1);//如果处与显示先设定为隐藏，再删除
  }
  
  sht->flags = 0;//未使用标记
  return;
}

//setting a sheet
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv)
{
  sht->buf = buf;
  sht->bxsize = xsize;
  sht->bysize = ysize;
  sht->col_inv = col_inv;
  return ;
}

void sheet_updown(struct SHEET *sht, int height)
{
  struct SHTCTL *ctl = sht->ctl;
  int h, old = sht->height;//储存设置前的高度

  //如果指定的高度过低或者是过高要进行修正
  if (height > ctl->top + 1) {//顶层属性
    height = ctl->top + 1;
  }
  if (height < -1) {//隐藏属性
    height = -1;
  }
  sht->height = height;//设定高度

  //对图层排序
  if (old > height) {//比以前图层要低
    if (height >= 0) {//而且是可显示的图层
      //把中间的图层向上提
      for (h = old; h > height; h--) {
	ctl->sheets[h] = ctl->sheets[h-1];
	ctl->sheets[h]->height = h;
      }
      ctl->sheets[height] = sht;
      sheet_refreshmap(ctl,sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height + 1);
      sheet_refreshsub(ctl,sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height + 1, old);//按照新的图层信息重绘
    } else {//需要隐藏的图层
      if (ctl->top > old) {
	//需要把它之上的图层向下降
	for (h = old; h < ctl->top; h++) {
	  ctl->sheets[h] = ctl->sheets[h + 1];
	  ctl->sheets[h]->height = h;
	}
      }
      ctl->top--;//由于显示的图层减少一个，top -= 1
      sheet_refreshmap(ctl,sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, 0);
      sheet_refreshsub(ctl,sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, 0, old - 1);//按照新的图层信息重绘
    }
  } else if (old < height ) {//比以前要高
    if (old >= 0) {//以前是显示的
      //向下移动图层
      for (h = old; h < height; h++) {
	ctl->sheets[h] = ctl->sheets[h + 1];
	ctl->sheets[h]->height = h;
      }
      ctl->sheets[height] = sht;
    } else {//由隐藏变成显示状态
      //将height开始的图层提高一级
      for (h = ctl->top; h >= height; h--) {
	ctl->sheets[h+1] = ctl->sheets[h];
	ctl->sheets[h+1]->height = h + 1;
      }
      ctl->sheets[height] = sht;
      ctl->top++;//图层高度加1
    }
    sheet_refreshmap(ctl,sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height);
    sheet_refreshsub(ctl,sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height, height);//按照新的图层信息重绘
  } else {//没有改变
    return;
  }
}

//重绘
void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1)
{
  if (sht->height >= 0) {//如果正在显示，才刷新
    sheet_refreshsub(sht->ctl, sht->vx0 + bx0, sht->vy0 + by0, sht->vx0 + bx1, sht->vy0 + by1, sht->height, sht->height);
  }
  return;
}

//重绘sub
void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0, int h1)
{
  int h, bx, by, vx, vy, bx0, by0, bx1, by1;
  unsigned char *buf;
  unsigned char sid;
  unsigned char *vram = ctl->vram;//显存位置
  unsigned char *map = ctl->map;
  struct SHEET *sht;

  //如果刷新的范围超出了画面则修正下
  if (vx0 < 0) vx0 = 0;  
  if (vy0 < 0) vy0 = 0;
  if (vx1 > ctl->xsize) vx1 = ctl->xsize;
  if (vy1 > ctl->ysize) vy1 = ctl->ysize;

  for (h = h0; h <= h1; h++) {
    sht = ctl->sheets[h];
    sid = sht - ctl->sheets0;   
    buf = sht->buf;
    
    //使用vx0～vy1，对bx0～by1进行倒推导
    bx0 = vx0-sht->vx0;
    by0 = vy0-sht->vy0;
    bx1 = vx1-sht->vx0;
    by1 = vy1-sht->vy0;
    if (bx0 < 0) bx0 = 0;
    if (by0 < 0) by0 = 0;
    if (bx1 > sht->bxsize) bx1 = sht->bxsize; 
    if (by1 > sht->bysize) by1 = sht->bysize;

    for (by = by0; by < by1; by++) {
      vy = sht->vy0 + by;
      for (bx = bx0; bx < bx1; bx++) {
	vx = sht->vx0 + bx;

	if (map[vy*ctl->xsize + vx] == sid) {
	  vram[vy *ctl->xsize + vx] = buf[by*sht->bxsize + bx];
	}
      }
    }
  }
  return;
}
//重绘相关的map
void sheet_refreshmap(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0)
{
  int h, bx, by, vx, vy, bx0, by0, bx1, by1;
  unsigned char *buf;
  unsigned char sid;
  unsigned char *map = ctl->map;

  struct SHEET *sht;

  //如果刷新的范围超出了画面则修正下
  if (vx0 < 0) vx0 = 0;  
  if (vy0 < 0) vy0 = 0;
  if (vx1 > ctl->xsize) vx1 = ctl->xsize;
  if (vy1 > ctl->ysize) vy1 = ctl->ysize;

  for (h = h0; h <= ctl->top; h++) {
    sht = ctl->sheets[h];
    sid = sht-ctl->sheets0;//将进行减法计算的地址作为图层号码使用 
    buf = sht->buf;
    
    //使用vx0～vy1，对bx0～by1进行倒推导
    bx0 = vx0-sht->vx0;
    by0 = vy0-sht->vy0;
    bx1 = vx1-sht->vx0;
    by1 = vy1-sht->vy0;
    if (bx0 < 0) bx0 = 0;
    if (by0 < 0) by0 = 0;
    if (bx1 > sht->bxsize) bx1 = sht->bxsize; 
    if (by1 > sht->bysize) by1 = sht->bysize;

    for (by = by0; by < by1; by++) {
      vy = sht->vy0 + by;
      for (bx = bx0; bx < bx1; bx++) {
	vx = sht->vx0 + bx;
	if (buf[by*sht->bxsize + bx] != sht->col_inv) {
	  map[vy * ctl->xsize + vx] = sid;
	}
      }
    }
  }
  return;
}

//滑动图层
void sheet_slide(struct SHEET *sht, int vx0, int vy0)
{
  struct SHTCTL *ctl = sht->ctl;
  int old_vx0 = sht->vx0, old_vy0 = sht->vy0;
  sht->vx0 = vx0;
  sht->vy0 = vy0;
  if (sht->height >= 0) {//如果图层在显示，则刷新
    sheet_refreshmap(ctl, old_vx0, old_vy0, old_vx0+sht->bxsize, old_vy0+sht->bysize, 0);
    sheet_refreshmap(ctl, vx0, vy0, vx0+sht->bxsize, vy0+sht->bysize, sht->height);
    sheet_refreshsub(ctl, old_vx0, old_vy0, old_vx0+sht->bxsize, old_vy0+sht->bysize, 0, sht->height-1);
    sheet_refreshsub(ctl, vx0, vy0, vx0+sht->bxsize, vy0+sht->bysize, sht->height, sht->height);
  }
  return;
}

