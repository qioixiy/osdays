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

#define putfonts8_asc_sht putfont8_asc_sht
void console_task(struct SHEET *sheet)
{
  struct TIMER *timer;
  struct TASK *task = task_now();
  char s[10];

  int i, fifobuf[128], cursor_x = 16, cursor_y = 28, cursor_c = -1;//初始为不闪烁
  fifo32_init(&task->fifo, 128, fifobuf, task);
  timer = timer_alloc();
  timer_init(timer, &task->fifo, 1);
  timer_settime(timer, 50);

  //显示提示符
  putfont8_asc_sht(sheet, 8, 28, COL8_FFFFFF, COL8_000000, ">", 1);

  int x, y;
  for (;;) {
    io_cli();
    if (fifo32_status(&task->fifo) == 0) {
      task_sleep(task);
      io_sti();
    } else {
      i = fifo32_get(&task->fifo);
      
      io_sti();
      
      if (i <= 1){//光标定时器
	if (i != 0) {
	  timer_init(timer, &task->fifo, 0);
	  if (cursor_c >= 0) {
	    cursor_c = COL8_FFFFFF;
	  }
	} else {
	  timer_init(timer, &task->fifo, 1);
	  if (cursor_c >= 0) {
	    cursor_c = COL8_000000;
	  }
	}
	timer_settime(timer, 50);
      }
      
      if (i == 2) {//光标ON
	cursor_c = COL8_FFFFFF;
      }
     
      if (i == 3) {//光标OFF
	boxfill8(sheet->buf, sheet->bxsize, COL8_000000, cursor_x, 28, cursor_x+7, 43);
	cursor_c = -1;
      }
     
      if (256 <= i && i <= 511) {//键盘数据
	if (i == 8 + 256) {//退格键
	  if (cursor_x > 16) {
	    //用空白擦除光标后将光标前移一位
	    putfont8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
	    cursor_x -= 8;
	  }
	} else if (i == 10+256) {//回车键enter
	  //用空格将光标擦除
	  putfont8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
	  //
	  if (cursor_y < 28+112) {
	    cursor_y += 16;//换行
	  } else {
	    //向下滚动一行
	    for (y = 28; y < 28+112; y++) {
	      for (x = 8; x < 8+240; x++) {
		sheet->buf[x+y*sheet->bxsize] = sheet->buf[x+(y+16) * sheet->bxsize];
	      }
	    }
	    for (y = 28+112; y < 28+128; y++) {
	      for (x = 8; x < 8+240; x++) {
		sheet->buf[x+y*sheet->bxsize] = COL8_000000;
	      }
	    }
	    sheet_refresh(sheet, 8, 28, 8+240, 28+128);
	  }
	  //显示提示符
	  putfont8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, ">", 1);
	  cursor_x = 16;
	} else {//一般字符
	  if (cursor_x < 240) {
	    //显示字符后将光标后移动一位
	    s[0] = i-256;
	    s[1] = 0;
	    putfont8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, s, 1);
	    cursor_x += 8;
	  }
	}
      }
      
      //重新显示光标
      if (cursor_c >= 0) {
	boxfill8(sheet->buf, sheet->bxsize, cursor_c, cursor_x, cursor_y, cursor_x + 7, cursor_y+15);
      }
      sheet_refresh(sheet,cursor_x, cursor_y, cursor_x + 8, cursor_y+16);
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
    
  //鼠标相关，mouse_phase,鼠标状态；
  struct MOUSE_DEC mdec;

  init_gdtidt();
  init_pic();//初始化PIC
  io_sti();//PIC初始化完成，开中断
  
  init_pit();  //必须先初始化pit，然后才能使用settimer

  struct FIFO32 fifo;
  unsigned int fifobuf[128];
  fifo32_init(&fifo, sizeof(fifobuf)/sizeof(fifobuf[0]), fifobuf, 0);

  timer = timer_alloc();
  timer_init(timer, &fifo, 1);
  timer_settime(timer, 50); 
  
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
  sheet_setbuf(sht_win, buf_win, 144, 52, -1);//透明色号99
  
  init_screen8(buf_back, binfo->scrnx, binfo->scrny);
  init_mouse_cursor8(buf_mouse, 99);//背景色号99
  make_window8(sht_win->buf, 144, 52, "task_a", 1);//
  make_textbox8(sht_win, 8, 28, 128, 16, COL8_FFFFFF);

  //鼠标初始位置
  mx = (binfo->scrnx - 16) / 2;
  my = (binfo->scrny - 16) / 2;
  
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
  sheet_setbuf(sht_cons, buf_cons, 256, 165, -1);//无透明色
  make_window8(buf_cons,256, 165, "console", 0);
  make_textbox8(sht_cons, 8, 28, 240, 128, COL8_000000);
  task_cons = task_alloc();//分配一个task struct
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
  sheet_slide(sht_mouse, mx, my);//移动鼠标到中心，显示出来

  sheet_updown(sht_back, 0);
  sheet_updown(sht_cons, 1);
  sheet_updown(sht_win, 2);
  sheet_updown(sht_mouse, 3);

  int cursor_x = 8;//光标位置
  int cursor_c = COL8_FFFFFF;//光标初始颜色
  
  int key_to = 0;
  int key_shift = 0;//shift状态
  int key_leds = (binfo->leds>>4) & 7;//键盘指示灯状态
  int keycmd_wait = -1;

  struct FIFO32 keycmd;//键盘控制器设定FIFO
  int keycmd_buf[32];
  fifo32_init(&keycmd_buf, sizeof keycmd_buf, keycmd_wait, 0);
  //为了避免和键盘当前状态的冲突，在一开始先进行设置
  fifo32_put(&keycmd, KEYCMD_LED);
  fifo32_put(&keycmd, key_leds);
  
  for (;;) {
    if (fifo32_status(&keycmd) > 0 && keycmd_wait < 0) {
      //如果存在向键盘控制器发送的数据，则发送出去
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
      if (256 <= i && i <= 511) {//键盘数据
	
	//CAPSLOCK，大写锁定键
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
	if (i == 256 + 0xfa) {//键盘成功接收到数据
	  keycmd_wait = -1;
	}
	if (i == 256 + 0xfe) {//键盘没有成功接收到数据
	  wait_KBC_sendready();
	  io_out8(PORT_KEYDAT, keycmd_wait);
	}
	
	if (i < 0x54 + 256 ) {//按键盘编码为字符编码
	  if (key_shift == 0) {
	    s[0] = keytable0[i - 256];
	  } else if (key_shift == 1 || key_shift == 2) {
	    s[0] = keytable1[i - 256];
	  } else {
	    s[0] = 0;
	  }
	  //大小写字符转换
	  if ('A' <= s[0] && s[0] <= 'Z') {
	    if (((key_leds&4) == 0 && key_shift == 0) //大写字符锁定键没有打开,shift没有按下
		|| ((key_leds&4) != 0 && key_shift != 0)) {
	      s[0] += 0X20;
	    }
	  }
	  
	  if (s[0] != 0) {
	    if (key_to == 0) {//发送给任务a
	      if (cursor_x < 128) {
		s[1] = 0;
		putfont8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, s, strlen(s));
		cursor_x += 8;
	      }
	    } else {//向命令行窗口发送字符
	      fifo32_put(&task_cons->fifo, s[0]+256);
	    }
	  }
	}

	if (i == 256+0x0e) {//退格键
	  if (key_to == 0) { 
	    if (cursor_x > 8) {
	      putfont8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
	      cursor_x -= 8;
	    }
	  } else {
	    fifo32_put(&task_cons->fifo, 8+256);
	  }
	}

	if (i == 256 + 0x0f) {//tab键
	  if (key_to == 0) {
	    key_to = 1;
	    make_wtitle8(buf_win, sht_win->bxsize, "task_a", 0);
	    make_wtitle8(buf_cons, sht_cons->bxsize, "console", 1);
	    cursor_c = -1;//不显示光标
	    boxfill8(sht_win->buf, sht_win->bxsize,COL8_FFFFFF, cursor_x, 28, cursor_x+7, 43);
	    fifo32_put(&task_cons->fifo, 2);//命令行窗口光标On，2
	  } else {
	    key_to = 0;
	    make_wtitle8(buf_win, sht_win->bxsize, "task_a", 1);
	    make_wtitle8(buf_cons, sht_cons->bxsize, "console", 0);
	    cursor_c = COL8_000000;//显示光标
	    fifo32_put(&task_cons->fifo, 3);//命令行窗口光标On，3
	  }
	  //刷新title
	  sheet_refresh(sht_win, 0, 0, sht_win->bxsize, 21);
	  sheet_refresh(sht_cons, 0, 0, sht_cons->bxsize, 21);
	}
	//重新显示光标
	if (cursor_c >= 0) {
	  boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x+7, 43);
	}
	sheet_refresh(sht_win, cursor_x, 28, cursor_x+8, 44);
	if (i == 256 + 0x2a) {//左shift on
	  key_shift |= 1;
	}
	if (i == 256 + 0x36) {//右shift on
	  key_shift |= 2;
	}
	if (i == 256 + 0xaa) {//左shift off
	  key_shift &= ~1;
	}
	if (i == 256 + 0xb6) {//右shift off
	  key_shift &= ~2;
	}
	if (i == 256+0x1c) {//回车键
	  if (key_to != 0) {
	    fifo32_put(&task_cons->fifo, 10+256);
	  }
	}
	
      } else if (512 <= i && i <= 767) {//鼠标数据
	//鼠标的3个字节都齐全了，显示出来
	if (mouse_decode(&mdec, i-512) != 0) {	  
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
	  timer_init(timer, &fifo, 0);//设置为0
	  if (cursor_c >= 0){
	    cursor_c = COL8_000000;
	  }
	} else if(0 == i){
	  timer_init(timer, &fifo, 1);//设置为1
	  if (cursor_c >= 0) {
	    cursor_c = COL8_FFFFFF;
	  }
	}
	timer_settime(timer, 50);
	if (cursor_c >= 0) {
	  boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x+7, 43); 
	  sheet_refresh(sht_win, cursor_x, 28, cursor_x+8, 44);
	}
      } else {
	
      }
    }
  }
}
