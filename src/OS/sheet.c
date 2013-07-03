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
  ctl->vram = vram;
  ctl->xsize = xsize;
  ctl->ysize = ysize;
  ctl->top = -1;//һ��sheet��û��
  for (i = 0; i < MAX_SHEETS; i++) {
    ctl->sheets0[i].flags = 0;//���Ϊδʹ��״̬
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
    if (ctl->sheets0[i].flags == 0) {//δʹ�õ�ͼ��
      sht = &ctl->sheets0[i];
      sht->flags = SHEET_USE;//���Ϊʹ����
      sht->height = -1;//����������
      return sht;
    }
  }
  return 0;
}

void sheet_free(struct SHTCTL *ctl, struct SHEET *sht)
{
  if (sht->height >= 0) {
    sheet_updown(ctl, sht, -1);//���������ʾ���趨Ϊ���أ���ɾ��
  }
  
  sht->flags = 0;//δʹ�ñ��
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

void sheet_updown(struct SHTCTL *ctl, struct SHEET *sht, int height)
{
  int h, old = sht->height;//��������ǰ�ĸ߶�

  //���ָ���ĸ߶ȹ��ͻ����ǹ���Ҫ��������
  if (height > ctl->top + 1) {//��������
    height = ctl->top + 1;
  }
  if (height < -1) {//��������
    height = -1;
  }
  sht->height = height;//�趨�߶�

  //��ͼ������
  if (old > height) {//����ǰͼ��Ҫ��
    if (height >= 0) {//�����ǿ���ʾ��ͼ��
      //���м��ͼ��������
      for (h = old; h > height; h--) {
	ctl->sheets[h] = ctl->sheets[h-1];
	ctl->sheets[h]->height = h;
      }
      ctl->sheets[height] = sht;
    } else {//��Ҫ���ص�ͼ��
      if (ctl->top > old) {
	//��Ҫ����֮�ϵ�ͼ�����½�
	for (h = old; h < ctl->top; h++) {
	  ctl->sheets[h] = ctl->sheets[h + 1];
	  ctl->sheets[h]->height = h;
	}
      }
      ctl->top--;//������ʾ��ͼ�����һ����top -= 1
    }
    sheet_refresh(ctl);//�����µ�ͼ����Ϣ�ػ�
  } else if (old < height ) {//����ǰҪ��
    if (old >= 0) {//��ǰ����ʾ��
      //�����ƶ�ͼ��
      for (h = old; h < height; h++) {
	ctl->sheets[h] = ctl->sheets[h + 1];
	ctl->sheets[h]->height = h;
      }
      ctl->sheets[height] = sht;
    } else {//�����ر����ʾ״̬
      //��height��ʼ��ͼ�����һ��
      for (h = ctl->top; h >= height; h--) {
	ctl->sheets[h+1] = ctl->sheets[h];
	ctl->sheets[h+1]->height = h + 1;
      }
      ctl->sheets[height] = sht;
      ctl->top++;//ͼ��߶ȼ�1
    }
    sheet_refresh(ctl);//�ػ�
  } else {//û�иı�
    return;
  }
}

//�ػ�
void sheet_refresh(struct SHTCTL *ctl)
{
  int h, bx, by, vx, vy;
  unsigned char *buf;
  unsigned char c;
  unsigned char *vram = ctl->vram;//�Դ�λ��

  struct SHEET *sht;
  for (h = 0; h <= ctl->top; h++) {
    sht = ctl->sheets[h];
    buf = sht->buf;
    for (by = 0; by < sht->bysize; by++) {
      vy = sht->vy0 + by;
      for (bx = 0; bx < sht->bxsize; bx++) {
	vx = sht->vx0 + bx;
	c = buf[by * sht->bxsize + bx];
	if (c != sht->col_inv) {
	  vram[vy * ctl->xsize + vx] = c;
	}
      }
    }
  }
  return;
}

//����ͼ��
void sheet_slide(struct SHTCTL *ctl, struct SHEET *sht, int vx0, int vy0)
{
  sht->vx0 = vx0;
  sht->vy0 = vy0;
  if (sht->height >= 0) {//���ͼ������ʾ����ˢ��
    sheet_refresh(ctl);
  }
  return;
}

