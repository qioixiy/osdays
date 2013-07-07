#include <stdio.h>
#include "bootpack.h"

static char keytable[0x54] = {
  0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0,   0,
  'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0,   0,   'A', 'S',
  'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
  'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
  '2', '3', '0', '.'
};

void HariMain(void)
{
  //bootinfo struct pointer
  struct BOOTINFO *binfo = (struct BOOTINFO *)0x0ff0;
  char s[50], mcursor[256];
  struct TIMER *timer, *timer2, *timer3;

  int mx, my;
  unsigned int i;
    
  //�����أ�mouse_phase,���״̬��
  struct MOUSE_DEC mdec;

  init_gdtidt();
  init_pic();//��ʼ��PIC
  io_sti();//PIC��ʼ����ɣ����ж�
  
  init_pit();  //�����ȳ�ʼ��pit��Ȼ�����ʹ��settimer

  struct FIFO32 fifo;
  unsigned int fifobuf[128];
  fifo32_init(&fifo, sizeof(fifobuf)/sizeof(fifobuf[0]), fifobuf);

  timer = timer_alloc();
  timer2 = timer_alloc();
  timer3 = timer_alloc();
  timer_init(timer, &fifo, 10);
  timer_init(timer2, &fifo, 3);
  timer_init(timer3, &fifo, 1);
  timer_settime(timer, 1000); 
  timer_settime(timer2, 300);
  timer_settime(timer3, 50);
  
  io_out8(PIC0_IMR, 0xf8); /* PIC1���ж�(11111000) */
  io_out8(PIC1_IMR, 0xef); /* �򿪼����ж�(11101111) */
  
  init_palette();
   
  //keyboard and mouse init
  init_keyboard(&fifo, 256);
  enable_mouse(&fifo, 512, &mdec);

  unsigned int memtotal;//memtotal memory ����
  struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
  memtotal = memtest(0x00400000, 0xbfffffff);//��ȡ�ڴ���ʵ�Ĵ�С
  memman_init(memman);
  memman_free(memman, 0x00001000, 0x0009e000);
  memman_free(memman, 0x00400000, memtotal - 0x00400000);

  //ͼ���������ʼ��
  struct SHTCTL *shtctl;
  shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);

  struct SHEET *sht_back, *sht_mouse, *sht_win;//���������ͼ��
  sht_back = sheet_alloc(shtctl);
  sht_mouse = sheet_alloc(shtctl);
  sht_win = sheet_alloc(shtctl);
  
  unsigned char *buf_back, buf_mouse[256], *buf_win;
  buf_back = (unsigned char *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
  buf_win = (unsigned char *)memman_alloc_4k(memman, 160*52);

  sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);//û��͸��ɫ
  sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);//͸��ɫ��99
  sheet_setbuf(sht_win, buf_win, 160, 52, -1);//͸��ɫ��99
  
  init_screen8(buf_back, binfo->scrnx, binfo->scrny);
  init_mouse_cursor8(buf_mouse, 99);//����ɫ��99
  make_window8(sht_win->buf, 160, 52, "window");//
  make_textbox8(sht_win, 8, 28, 144, 16, COL8_FFFFFF);

  sheet_slide(sht_back, 0, 0);//�ƶ�����ͼ�㣬ͬʱ��ʾ����
  sheet_slide(sht_win, 80, 72);
  //����ʼλ��
  mx = (binfo->scrnx - 16) / 2;
  my = (binfo->scrny - 16) / 2;
  
  sheet_slide(sht_mouse, mx, my);//�ƶ���굽���ģ���ʾ����
  //
  sheet_updown(sht_back, 0);
  sheet_updown(sht_win, 1);
  sheet_updown(sht_mouse, 2);
  
  sprintf(s, "(%3d, %3d)", mx, my);
  putfont8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
  sprintf(s, "memory %dMB free : %dKB",
	  memtotal/ (1024*1024), memman_total(memman) / 1024);
  putfont8_asc(buf_back, binfo->scrnx, 0 , 32, COL8_FFFFFF, s);
  sheet_refresh(sht_back, 0, 0, binfo->scrnx, 48);
 
  for (;;) {
    io_cli();
    if (0 == fifo32_status(&fifo)){
      io_stihlt();
    } else {
      i = fifo32_get(&fifo);
      io_sti();
      if (256 <= i && i <= 511) {//��������
	sprintf(s, "%02X", i-256);
	putfont8_asc_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, s, strlen(s));
	if (i < 0x54 + 256) {
	  if (keytable[i-256] != 0) {
	    s[0] = keytable[i-256];
	    s[1] = 0;
	    putfont8_asc_sht(sht_win, 40, 28, COL8_000000, COL8_C6C6C6, s, strlen(s));
	  }
	}
      } else if (512 <= i && i <= 767) {//�������
	//����3���ֽڶ���ȫ�ˣ���ʾ����
	if (mouse_decode(&mdec, i-512) != 0) {
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
	  
	  putfont8_asc_sht(sht_back, 32, 16, COL8_FFFFFF, COL8_008484, s, strlen(s));
	  
	  //��������µ�λ��
	  mx += mdec.x;
	  my += mdec.y;
	  if (mx < 0) {
	    mx = 0;
	  }
	  if (my < 0) {
	    my = 0;
	  }
	  if (mx > binfo->scrnx - 1) {
	    mx = binfo->scrnx - 1;
	  }
	  if (my > binfo->scrny - 1) {
	    my = binfo->scrny - 1;
	  }
	  sprintf(s, "(%3d, %3d)", mx, my);
	  putfont8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, strlen(s));
	  
	  //���������ʾ������sheet_reflush
	  sheet_slide(sht_mouse, mx, my);
	}
      } else if (10 == i) {//10s timer
	putfont8_asc_sht(sht_back, 0, 64, COL8_FFFFFF, COL8_008484, "10[sec]", 7);
      } else if (3 == i){//3s timer
	putfont8_asc_sht(sht_back, 0, 80, COL8_FFFFFF, COL8_008484, "3[sec]", 6);
      } else if(1 == i) {//I1
	timer_init(timer3, &fifo, 0);//����Ϊ0
	boxfill8(buf_back, binfo->scrnx, COL8_FFFFFF, 8, 96, 15, 111);
	timer_settime(timer3, 50);
	sheet_refresh(sht_back, 8, 96, 16, 112);
      } else if(0 == i){
	timer_init(timer3, &fifo, 1);//����Ϊ1
	boxfill8(buf_back, binfo->scrnx, COL8_008484, 8, 96, 15, 111); 
	timer_settime(timer3, 50);
	sheet_refresh(sht_back, 8, 96, 16, 112);
      }
    }
  }
}
