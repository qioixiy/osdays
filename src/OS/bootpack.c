#include <stdio.h>
#include <string.h>
#include "bootpack.h"

static char keytable0[0x80] = {
  0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0,   0,
  'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0,   0,   'A', 'S',
  'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
  'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
  '2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0x5c, 0,  0,   0,   0,   0,   0,   0,   0,   0,   0x5c, 0,  0
};
static char keytable1[0x80] = {
  0,   0,   '!', 0x22, '#', '$', '%', '&', 0x27, '(', ')', '~', '=', '~', 0,   0,
  'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '`', '{', 0,   0,   'A', 'S',
  'D', 'F', 'G', 'H', 'J', 'K', 'L', '+', '*', 0,   0,   '}', 'Z', 'X', 'C', 'V',
  'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
  '2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   '_', 0,   0,   0,   0,   0,   0,   0,   0,   0,   '|', 0,   0
};

void console_task(struct SHEET *sheet)
{
  struct TIMER *timer;
  struct TASK *task = task_now();
  char s[10];

  int i, fifobuf[128], cursor_x = 16, cursor_c = COL8_000000;
  fifo32_init(&task->fifo, 128, fifobuf, task);
  timer = timer_alloc();
  timer_init(timer, &task->fifo, 1);
  timer_settime(timer, 50);

  //��ʾ��ʾ��
  putfont8_asc_sht(sheet, 8, 28, COL8_FFFFFF, COL8_000000, ">", 1);
  for (;;) {
    io_cli();
    if (fifo32_status(&task->fifo) == 0) {
      task_sleep(task);
      io_sti();
    } else {
      i = fifo32_get(&task->fifo);
      
      io_sti();
      
      if (i <= 1){//��궨ʱ��
	if (i != 0) {
	  timer_init(timer, &task->fifo, 0);
	  cursor_c = COL8_FFFFFF;
	} else {
	  timer_init(timer, &task->fifo, 1);
	  cursor_c = COL8_000000;
	}
	timer_settime(timer, 50);
	boxfill8(sheet->buf, sheet->bxsize, cursor_c, cursor_x, 28,cursor_x+7, 43);
	sheet_refresh(sheet, cursor_x, 28, cursor_x+8, 44);
      }
      
      if (256 <= i && i <= 511) {//��������
	if (i == 8 + 256) {//�˸��
	  if (cursor_x > 16) {
	    //�ÿհײ������󽫹��ǰ��һλ
	    putfont8_asc_sht(sheet, cursor_x, 28, COL8_FFFFFF, COL8_000000, " ", 1);
	    cursor_x -= 8;
	  }
	}
	else {//һ���ַ�
	  if (cursor_x < 240) {
	    //��ʾ�ַ��󽫹����ƶ�һλ
	    s[0] = i-256;
	    s[1] = 0;
	    putfont8_asc_sht(sheet, cursor_x, 28, COL8_FFFFFF, COL8_000000, s, 1);
	    cursor_x += 8;
	  }
	}
      }
      
      //������ʾ���
      boxfill8(sheet->buf, sheet->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
      sheet_refresh(sheet,cursor_x, 28, cursor_x + 8, 44);
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
  make_window8(sht_win->buf, 144, 52, "task_a", 1);//
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

  //sht_cons
  struct TASK *task_cons;
  struct SHEET *sht_cons;
  unsigned char *buf_cons;
  sht_cons = sheet_alloc(shtctl);
  buf_cons = (unsigned char *)memman_alloc_4k(memman, 256*165);
  sheet_setbuf(sht_cons, buf_cons, 256, 165, -1);//��͸��ɫ
  make_window8(buf_cons,256, 165, "console", 0);
  make_textbox8(sht_cons, 8, 28, 240, 128, COL8_000000);
  task_cons = task_alloc();//����һ��task struct
  task_cons->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 -8;
  task_cons->tss.eip = (int)&console_task;
  task_cons->tss.es = 1*8;
  task_cons->tss.cs = 2*8;
  task_cons->tss.ss = 1*8;
  task_cons->tss.ds = 1*8;
  task_cons->tss.fs = 1*8;
  task_cons->tss.gs = 1*8;
  *((int *)(task_cons->tss.esp+4)) = (int)sht_cons;
  task_run(task_cons, 2, 2);//level=2,priority=2

  sheet_slide(sht_back, 0, 0);
  sheet_slide(sht_win, 64, 56);
  sheet_slide(sht_cons, 32, 4);
  sheet_slide(sht_mouse, mx, my);//�ƶ���굽���ģ���ʾ����

  sheet_updown(sht_back, 0);
  sheet_updown(sht_cons, 1);
  sheet_updown(sht_win, 2);
  sheet_updown(sht_mouse, 3);

  int cursor_x = 8;//���λ��
  int cursor_c = COL8_FFFFFF;//����ʼ��ɫ
  
  int key_to = 0;
  int key_shift = 0;//shift״̬
  int key_leds = (binfo->leds>>4) & 7;//����ָʾ��״̬
  int keycmd_wait = -1;

  struct FIFO32 keycmd;//���̿������趨FIFO
  int keycmd_buf[32];
  fifo32_init(&keycmd_buf, sizeof keycmd_buf, keycmd_wait, 0);
  //Ϊ�˱���ͼ��̵�ǰ״̬�ĳ�ͻ����һ��ʼ�Ƚ�������
  fifo32_put(&keycmd, KEYCMD_LED);
  fifo32_put(&keycmd, key_leds);
  
  for (;;) {
    if (fifo32_status(&keycmd) > 0 && keycmd_wait < 0) {
      //�����������̿��������͵����ݣ����ͳ�ȥ
      keycmd_wait = fifo32_get(&keycmd);
      wait_KBC_sendready();
      io_out8(PORT_KEYDAT, key_leds);
    }

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
	
	//CAPSLOCK����д������
	if (i == 256 + 0x3a) {
	  key_leds ^= 4;
	  fifo32_put(&keycmd, KEYCMD_LED);
	  fifo32_put(&keycmd, key_leds);
	}
	//Numlock
	if (i == 256 + 0x45) {
	  key_leds ^= 2;
	  fifo32_put(&keycmd, KEYCMD_LED);
	  fifo32_put(&keycmd, key_leds);
	}
	//ScrollLock
	if (i == 256 + 0x46) {
	  key_leds ^= 1;
	  fifo32_put(&keycmd, KEYCMD_LED);
	  fifo32_put(&keycmd, key_leds);
	}
	if (i == 256 + 0xfa) {//���̳ɹ����յ�����
	  keycmd_wait = -1;
	}
	if (i == 256 + 0xfe) {//����û�гɹ����յ�����
	  wait_KBC_sendready();
	  io_out8(PORT_KEYDAT, keycmd_wait);
	}


	
	if (i < 0x54 + 256 ) {//�����̱���Ϊ�ַ�����
	  if (key_shift == 0) {
	    s[0] = keytable0[i - 256];
	  } else if (key_shift == 1 || key_shift == 2) {
	    s[0] = keytable1[i - 256];
	  } else {
	    s[0] = 0;
	  }
	  //��Сд�ַ�ת��
	  if ('A' <= s[0] && s[0] <= 'Z') {
	    if (((key_leds&4) == 0 && key_shift == 0) //��д�ַ�������û�д�,shiftû�а���
		|| ((key_leds&4) != 0 && key_shift != 0)) {
	      s[0] += 0X20;
	    }
	  }
	  
	  if (s[0] != 0) {
	    if (key_to == 0) {//���͸�����a
	      if (cursor_x < 128) {
		s[1] = 0;
		putfont8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, s, strlen(s));
		cursor_x += 8;
	      }
	    } else {//�������д��ڷ����ַ�
	      fifo32_put(&task_cons->fifo, s[0]+256);
	    }
	  }
	}

	if (i == 256+0x0e) {//�˸��
	  if (key_to == 0) { 
	    if (cursor_x > 8) {
	      putfont8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
	      cursor_x -= 8;
	    }
	  } else {
	    fifo32_put(&task_cons->fifo, 8+256);
	  }
	}

	if (i == 256 + 0x0f) {//tab��
	  if (key_to == 0) {
	    key_to = 1;
	    make_wtitle8(buf_win, sht_win->bxsize, "task_a", 0);
	    make_wtitle8(buf_cons, sht_cons->bxsize, "console", 1);
	  } else {
	    key_to = 0;
	    make_wtitle8(buf_win, sht_win->bxsize, "task_a", 1);
	    make_wtitle8(buf_cons, sht_cons->bxsize, "console", 0);
	  }
	  //ˢ��title
	  sheet_refresh(sht_win, 0, 0, sht_win->bxsize, 21);
	  sheet_refresh(sht_cons, 0, 0, sht_cons->bxsize, 21);
	}
	
	if (i == 256 + 0x2a) {//��shift on
	  key_shift |= 1;
	}

	if (i == 256 + 0x36) {//��shift on
	  key_shift |= 2;
	}

	if (i == 256 + 0xaa) {//��shift off
	  key_shift &= ~1;
	}

	if (i == 256 + 0xb6) {//��shift off
	  key_shift &= ~2;
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
