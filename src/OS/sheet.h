#ifndef _SHEET_H
#define _SHEET_H

struct SHEET{
  unsigned char *buf;
  int bxsize, bysize;//ͼ��Ĵ�С
  int vx0, vy0;//ͼ���ڻ����ϵ����λ��
  int col_inv;//͸��ɫɫ��
  int height;//ͼ��߶�
  int flags;//ͼ���йص��趨��Ϣ
};

#define MAX_SHEETS 256//����ͼ����

struct SHTCTL {
  unsigned char *vram;//VRAM��ַ
  int xsize, ysize;//��С
  int top;//���ϲ�ĸ߶�
  struct SHEET *sheets[MAX_SHEETS];//��ʾͼ���˳��ṹ
  struct SHEET sheets0[MAX_SHEETS];//ʵ��ͼ��ṹ����
};

struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize);
struct SHEET *sheet_alloc(struct SHTCTL *ctl);
void sheet_free(struct SHTCTL *ctl, struct SHEET *sht);
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv);
void sheet_updown(struct SHTCTL *ctl, struct SHEET *sht, int height);
void sheet_refresh(struct SHTCTL *ctl);
void sheet_slide(struct SHTCTL *ctl, struct SHEET *sht, int vx0, int vy0);
//*/
#endif
