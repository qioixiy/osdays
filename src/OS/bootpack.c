#include <stdio.h>
#include <string.h>
#include "bootpack.h"

static char keytable[0x54] = {
  0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0,   0,
  'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0,   0,   'A', 'S',
  'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
  'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
  '2', '3', '0', '.'
};

void task_b_main(struct SHEET *sht_back)
{
  struct FIFO32 fifo;
  struct TIMER *timer_1s;
  int i, fifobuf[128];
  int count = 0;
  int count0 = 0;
  char s[12];
 
  fifo32_init(&fifo, 128, fifobuf, 0);

  timer_1s = timer_alloc();
  timer_init(timer_1s, &fifo, 100);
  timer_settime(timer_1s, 100);

  for (;;) {
    count++;

    io_cli();
    if (fifo32_status(&fifo) == 0) {
      io_sti();
    } else {
      i = fifo32_get(&fifo);
      io_sti();
      
      if (100 == i){
	sprintf(s, "%11d", count - count0);
	putfont8_asc_sht(sht_back, 24, 28, COL8_000000, COL8_C6C6C6, s, 11);
	count0 = count;
	timer_settime(timer_1s, 100);
      }
    }
  }
}

void HariMain(void)
{
  //bootinfo struct pointer
  struct BOOTINFO *binfo = (struct BOOTINFO *)0x0ff0;
  char s[50];
  struct TIMER *timer;

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
  fifo32_init(&fifo, sizeof(fifobuf)/sizeof(fifobuf[0]), fifobuf, 0);

  timer = timer_alloc();
  timer_init(timer, &fifo, 1);
  timer_settime(timer, 50); 
  
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
  sheet_setbuf(sht_win, buf_win, 144, 52, -1);//͸��ɫ��99
  
  init_screen8(buf_back, binfo->scrnx, binfo->scrny);
  init_mouse_cursor8(buf_mouse, 99);//����ɫ��99
  make_window8(sht_win->buf, 144, 52, "window", 1);//
  make_textbox8(sht_win, 8, 28, 128, 16, COL8_FFFFFF);

  //����ʼλ��
  mx = (binfo->scrnx - 16) / 2;
  my = (binfo->scrny - 16) / 2;
  
  sprintf(s, "(%3d, %3d)", mx, my);
  putfont8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
  sprintf(s, "memory %dMB free : %dKB",
	  memtotal/ (1024*1024), memman_total(memman) / 1024);
  putfont8_asc(buf_back, binfo->scrnx, 0 , 32, COL8_FFFFFF, s);
  sheet_refresh(sht_back, 0, 0, binfo->scrnx, 48);
  
  //task_a
  struct TASK *task_a;
  task_a = task_init(memman);
  fifo.task = task_a;
  task_run(task_a, 1, 0);

  //task_b[3]
  struct SHEET *sht_win_b[3];
  unsigned char *buf_win_b;
  struct TASK *task_b[3];
  for (i = 0; i < 3; i++) {
    sht_win_b[i] = sheet_alloc(shtctl);
    buf_win_b = (unsigned char *)memman_alloc_4k(memman, 144*52);
    sheet_setbuf(sht_win_b[i], buf_win_b, 144, 52, -1);//��͸����ɫ
    sprintf(s, "task_b%d", i);
    make_window8(buf_win_b, 144, 52, s, 0);

    task_b[i] = task_alloc();//����һ��task struct
    task_b[i]->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 -8;
    task_b[i]->tss.eip = (int)&task_b_main;
    task_b[i]->tss.es = 1*8;
    task_b[i]->tss.cs = 2*8;
    task_b[i]->tss.ss = 1*8;
    task_b[i]->tss.ds = 1*8;
    task_b[i]->tss.fs = 1*8;
    task_b[i]->tss.gs = 1*8;
    *((int *)(task_b[i]->tss.esp+4)) = (int)sht_win_b[i];
    //task_run(task_b[i], 2, i+1);
  }
  sheet_slide(sht_back, 0, 0);
  sheet_slide(sht_win_b[0], 168, 56);
  sheet_slide(sht_win_b[1], 8, 116);
  sheet_slide(sht_win_b[2], 168, 116);
  sheet_slide(sht_win, 8, 56);
  sheet_slide(sht_mouse, mx, my);//�ƶ���굽���ģ���ʾ����

  sheet_updown(sht_back, 0);
  sheet_updown(sht_win_b[0], 1);
  sheet_updown(sht_win_b[1], 2);
  sheet_updown(sht_win_b[2], 3);
  sheet_updown(sht_win, 4);
  sheet_updown(sht_mouse, 5);

  int cursor_x = 8;
  int cursor_c = COL8_FFFFFF;
  
  for (;;) {
    io_cli();
    if (0 == fifo32_status(&fifo)){
      task_sleep(task_a);
      io_sti();
    } else {
      i = fifo32_get(&fifo);
      io_sti();
      if (256 <= i && i <= 511) {//��������
	sprintf(s, "%02X", i-256);
	putfont8_asc_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, s, strlen(s));
	if (i < 0x54 + 256) {//һ������
	  if (keytable[i-256] != 0 && cursor_x < 128) {
	    s[0] = keytable[i-256];
	    s[1] = 0;
	    putfont8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, s, strlen(s));
	    cursor_x += 8;
	  }
	}
	if (i == 256+0x0e && cursor_x > 8) {//�˸��
	  putfont8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
	  cursor_x -= 8;
	}
	//�������ʾ
	boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28,cursor_x+7, 43);
	sheet_refresh(sht_win, cursor_x, 28, cursor_x+8, 44);
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
	  
	  if ((mdec.btn & 0x01) != 0) {//��������
	    sheet_slide(sht_win, mx-80, my-8);
	  }
	}
      } else if (10 == i) {//10s timer
	putfont8_asc_sht(sht_back, 0, 64, COL8_FFFFFF, COL8_008484, "10[sec]", 7);
      } else if (3 == i){//3s timer
	putfont8_asc_sht(sht_back, 0, 80, COL8_FFFFFF, COL8_008484, "3[sec]", 6);
      } else if (i <= 1) {
	if(1 == i) {//I1
	  timer_init(timer, &fifo, 0);//����Ϊ0
	  cursor_c = COL8_000000;
	} else if(0 == i){
	  timer_init(timer, &fifo, 1);//����Ϊ1
	  cursor_c = COL8_FFFFFF;
	}
	timer_settime(timer, 50);
	boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x+7, 43); 
	sheet_refresh(sht_win, cursor_x, 28, cursor_x+8, 44);
      } else {
	
      }
    }
  }
}
