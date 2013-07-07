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

struct TSS32 tss_a, tss_b;

void task_b_main(struct SHEET *sht_back)
{
  struct FIFO32 fifo;
  struct TIMER *timer_ts, *timer_put;
  int i, fifobuf[128];
  int count = 0;
  char s[12];
 
  fifo32_init(&fifo, 128, fifobuf);
  timer_ts = timer_alloc();
  timer_put = timer_alloc();
  timer_init(timer_ts, &fifo, 2);
  timer_init(timer_put, &fifo, 1);
  timer_settime(timer_ts, 2);
  timer_settime(timer_put, 1);

  for (;;) {
    count++;

    io_cli();
    if (fifo32_status(&fifo) == 0) {
      io_sti();
    } else {
      i = fifo32_get(&fifo);
      io_sti();
      
      if (2 == i) {//任务超时
	farjmp(0, 3*8);
	timer_settime(timer_ts, 2);
      } else if (1 == i) {
	sprintf(s, "%11d", count);
	putfont8_asc_sht(sht_back, 0, 144, COL8_FFFFFF, COL8_008484, s, 10);
	timer_settime(timer_put, 1);
      } else {
	
      }
    }
  }
}

void HariMain(void)
{
  //bootinfo struct pointer
  struct BOOTINFO *binfo = (struct BOOTINFO *)0x0ff0;
  char s[50];
  struct TIMER *timer, *timer2, *timer3;
  struct TIMER *timer_ts;

  int mx, my;
  unsigned int i;
    
  //鼠标相关，mouse_phase,鼠标状态；
  struct MOUSE_DEC mdec;

  init_gdtidt();
  init_pic();//初始化PIC
  io_sti();//PIC初始化完成，开中断
  
  init_pit();  //必须先初始化pit，然后才能使用settimer

  struct FIFO32 fifo;
  unsigned int fifobuf[128];
  fifo32_init(&fifo, sizeof(fifobuf)/sizeof(fifobuf[0]), fifobuf);

  timer = timer_alloc();
  timer2 = timer_alloc();
  timer3 = timer_alloc();
  timer_ts = timer_alloc();
  timer_init(timer, &fifo, 10);
  timer_init(timer2, &fifo, 3);
  timer_init(timer3, &fifo, 1);
  timer_init(timer_ts, &fifo, 2);//任务切换用
  timer_settime(timer, 1000); 
  timer_settime(timer2, 300);
  timer_settime(timer3, 50);
  timer_settime(timer_ts, 2);
  
  io_out8(PIC0_IMR, 0xf8); /* PIC1打开中断(11111000) */
  io_out8(PIC1_IMR, 0xef); /* 打开键盘中断(11101111) */
  
  init_palette();
   
  //keyboard and mouse init
  init_keyboard(&fifo, 256);
  enable_mouse(&fifo, 512, &mdec);

  unsigned int memtotal;//memtotal memory 总数
  struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
  memtotal = memtest(0x00400000, 0xbfffffff);//获取内存真实的大小
  memman_init(memman);
  memman_free(memman, 0x00001000, 0x0009e000);
  memman_free(memman, 0x00400000, memtotal - 0x00400000);

  //图层控制器初始化
  struct SHTCTL *shtctl;
  shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);

  struct SHEET *sht_back, *sht_mouse, *sht_win;//背景和鼠标图层
  sht_back = sheet_alloc(shtctl);
  sht_mouse = sheet_alloc(shtctl);
  sht_win = sheet_alloc(shtctl);
  
  unsigned char *buf_back, buf_mouse[256], *buf_win;
  buf_back = (unsigned char *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
  buf_win = (unsigned char *)memman_alloc_4k(memman, 160*52);

  sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);//没有透明色
  sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);//透明色号99
  sheet_setbuf(sht_win, buf_win, 160, 52, -1);//透明色号99
  
  init_screen8(buf_back, binfo->scrnx, binfo->scrny);
  init_mouse_cursor8(buf_mouse, 99);//背景色号99
  make_window8(sht_win->buf, 160, 52, "window");//
  make_textbox8(sht_win, 8, 28, 144, 16, COL8_FFFFFF);

  sheet_slide(sht_back, 0, 0);//移动背景图层，同时显示出来
  sheet_slide(sht_win, 80, 72);
  //鼠标初始位置
  mx = (binfo->scrnx - 16) / 2;
  my = (binfo->scrny - 16) / 2;
  
  sheet_slide(sht_mouse, mx, my);//移动鼠标到中心，显示出来
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

  //esp_b
  int task_b_esp;
  task_b_esp = memman_alloc_4k(memman, 64*1024) + 64*1024;
  *((int *)(task_b_esp + 4)) = (int)sht_back;
  //task
  tss_a.ldtr = 0;
  tss_a.iomap = 0x40000000;
  tss_b.ldtr = 0;
  tss_b.iomap = 0x40000000;
  tss_b.eip = (int)&task_b_main;
  tss_b.eflags = 0x00000202;//IF=1
  tss_b.eax = 0;
  tss_b.ecx = 0;
  tss_b.edx = 0;
  tss_b.ebx = 0;
  tss_b.esp = task_b_esp;
  tss_b.ebp = 0;
  tss_b.esi = 0;
  tss_b.edi = 0;
  tss_b.es = 1*8;
  tss_b.cs = 2*8;
  tss_b.ss = 1*8;
  tss_b.ds = 1*8;
  tss_b.fs = 1*8;
  tss_b.gs = 1*8;

  struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT;
  set_segmdesc(gdt+3, 103,(int)&tss_a, AR_TSS32);//段长限制为103字节
  set_segmdesc(gdt+4, 103,(int)&tss_b, AR_TSS32);

  load_tr(3 * 8);
  
  int cursor_x = 8;
  int cursor_c = COL8_FFFFFF;
  for (;;) {
    io_cli();
    if (0 == fifo32_status(&fifo)){
      io_stihlt();
    } else {
      i = fifo32_get(&fifo);
      io_sti();
      if (2 == i) {
	farjmp(0, 4*8);
	timer_settime(timer_ts, 2);
      } else if (256 <= i && i <= 511) {//键盘数据
	sprintf(s, "%02X", i-256);
	putfont8_asc_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, s, strlen(s));
	if (i < 0x54 + 256) {//一般数据
	  if (keytable[i-256] != 0 && cursor_x < 144) {
	    s[0] = keytable[i-256];
	    s[1] = 0;
	    putfont8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, s, strlen(s));
	    cursor_x += 8;
	  }
	}
	if (i == 256+0x0e && cursor_x > 8) {//退格键
	  putfont8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
	  cursor_x -= 8;
	}
	//光标再显示
	boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28,cursor_x+7, 43);
	sheet_refresh(sht_win, cursor_x, 28, cursor_x+8, 44);
      } else if (512 <= i && i <= 767) {//鼠标数据
	//鼠标的3个字节都齐全了，显示出来
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
	  
	  //计算鼠标新的位置
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
	  
	  //滑动鼠标显示，包含sheet_reflush
	  sheet_slide(sht_mouse, mx, my);
	  
	  if ((mdec.btn & 0x01) != 0) {//鼠标左键按
	    sheet_slide(sht_win, mx-80, my-8);
	  }
	}
      } else if (10 == i) {//10s timer
	putfont8_asc_sht(sht_back, 0, 64, COL8_FFFFFF, COL8_008484, "10[sec]", 7);
      } else if (3 == i){//3s timer
	putfont8_asc_sht(sht_back, 0, 80, COL8_FFFFFF, COL8_008484, "3[sec]", 6);
      } else if (i <= 1) {
	if(1 == i) {//I1
	  timer_init(timer3, &fifo, 0);//设置为0
	  cursor_c = COL8_000000;
	} else if(0 == i){
	  timer_init(timer3, &fifo, 1);//设置为1
	  cursor_c = COL8_FFFFFF;
	}
	timer_settime(timer3, 50);
	boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x+7, 43); 
	sheet_refresh(sht_win, cursor_x, 28, cursor_x+8, 44);
      }
    }
  }
}
