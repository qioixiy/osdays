#include "console.h"
#include "bootpack.h"

void cons_putchar(struct CONSOLE *cons, int chr, char move);
int cons_newline(struct CONSOLE *cons);
void cons_runcmd(char *cmdline, struct CONSOLE *cons, int *fat, unsigned int memtotal);
void cmd_mem(struct CONSOLE *cons, unsigned int memtotal) ;
void cmd_cls(struct CONSOLE *cons);
void cmd_dir(struct CONSOLE *cons);
void cmd_type(struct CONSOLE *cons, int *fat, char *cmdline);
void cmd_hlt(struct CONSOLE *cons, int *fat);
struct FILEINFO *file_search(char *name, struct FILEINFO *finfo, int max);

#define putfonts8_asc_sht putfont8_asc_sht
void console_task(struct SHEET *sheet, unsigned int memtotal)
{
  struct TIMER *timer;
  struct TASK *task = task_now();
  struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
  int i, fifobuf[128], *fat = (int *) memman_alloc_4k(memman, 4 * 2880);
  struct CONSOLE cons;
  char cmdline[30];
  cons.sht = sheet;
  cons.cur_x =  8;
  cons.cur_y = 28;
  cons.cur_c = -1;
  *((int *)0x0fec) = (int)&cons;

  fifo32_init(&task->fifo, 128, fifobuf, task);
  timer = timer_alloc();
  timer_init(timer, &task->fifo, 1);
  timer_settime(timer, 50);
  file_readfat(fat, (unsigned char *) (ADR_DISKIMG + 0x000200));
  
  //显示提示符
  cons_putchar(&cons, '>', 1);
  
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
	  if (cons.cur_c >= 0) {
	    cons.cur_c = COL8_FFFFFF;
	  }
	} else {
	  timer_init(timer, &task->fifo, 1);
	  if (cons.cur_c >= 0) {
	    cons.cur_c = COL8_000000;
	  }
	}
	timer_settime(timer, 50);
      }
      
      if (i == 2) {//光标ON
	cons.cur_c = COL8_FFFFFF;
      }
     
      if (i == 3) {//光标OFF
	boxfill8(sheet->buf, sheet->bxsize, COL8_000000, cons.cur_x, cons.cur_y, cons.cur_x+7, cons.cur_y + 15);
	cons.cur_c = -1;
      }
     
      if (256 <= i && i <= 511) {//键盘数据
	if (i == 8 + 256) {//退格键
	  if (cons.cur_x > 16) {
	    //用空白擦除光标后将光标前移一位
	    cons_putchar(&cons, ' ', 0);
	    cons.cur_x -= 8;
	  }
	} else if (i == 10+256) {//回车键enter
	  //用空格将光标擦除
	  cons_putchar(&cons, ' ', 0);
     	  cmdline[cons.cur_x/8-2] = 0;
	  cons_newline(&cons);

	  //执行命令
	  cons_runcmd(cmdline, &cons, fat, memtotal);

	  //显示提示符
	  cons_putchar(&cons, '>', 1);
	} else {//一般字符
	  if (cons.cur_x < 240) {
	    //显示字符后将光标后移动一位
	    cmdline[cons.cur_x/8 - 2] = i - 256;//保持字符到cmdline
	    cons_putchar(&cons, i - 256, 1);
	  }
	}
      }
      
      //重新显示光标
      if (cons.cur_c >= 0) {
	boxfill8(sheet->buf, sheet->bxsize, cons.cur_c, cons.cur_x, cons.cur_y, cons.cur_x + 7, cons.cur_y+15);
      }
      sheet_refresh(sheet, cons.cur_x, cons.cur_y, cons.cur_x + 8, cons.cur_y+16);
    }
  }
}

void cons_putchar(struct CONSOLE *cons, int chr, char move) 
{
  char s[2];
  s[0] = chr;
  s[1] = 0;
  
  if (s[0] == 0x09) {//制表符
    for (;;) {
      putfont8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, " ", 1);
      cons->cur_x += 8;
      if (cons->cur_x == 8 + 240) {
	cons->cur_x = 8;
	cons->cur_y = cons_newline(cons);
      }
      if (((cons->cur_x - 8) & 0x1f) == 0) {
	//被32整除就停止
	break;
      }
    }
  } else if (s[0] == 0x0a) {//换行符
    cons->cur_x = 8;
    cons->cur_y = cons_newline(cons);
  } else if (s[0] == 0x0d) {//回车符
    ;
  } else {
    putfont8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 1);
    if (move != 0) {
      cons->cur_x += 8;
      if (cons->cur_x == 8 + 240) {//到达最右端后换行
	cons->cur_x = 8;
	cons->cur_y = cons_newline(cons);
      }
    }
  }
  return;
}

//换行处理
int cons_newline(struct CONSOLE *cons)
{
  int x, y;
  struct SHEET *sheet = cons->sht;

  int cursor_y = cons->cur_y;

  if (cursor_y < 28+112) {
    cursor_y += 16;//换行
  } else {
    //滚动
    for (y = 28; y < 28+112; y++) {
      for (x = 8; x < 8+240; x++) {
	sheet->buf[x + y * sheet->bxsize] = sheet->buf[x + (y+16) * sheet->bxsize];
      }
    }
    for (y = 28+112; y < 28+128; y++) {
      for (x = 8; x < 8+240; x++) {
	sheet->buf[x + y*sheet->bxsize] = COL8_000000;
      }
    }
    sheet_refresh(sheet, 8, 28, 8+240, 28+128);
  }
  cons->cur_y = cursor_y;
  cons->cur_x = 8;

  return cursor_y;
}

void cons_runcmd(char *cmdline, struct CONSOLE *cons, int *fat, unsigned int memtotal)
{
  if (!strcmp(cmdline, "mem")) {
    cmd_mem(cons, memtotal);
  } else if (!strcmp(cmdline, "cls")) {
    cmd_cls(cons);
  } else if (!strcmp(cmdline, "dir")) {
    cmd_dir(cons);
  } else if (!strncmp(cmdline, "type ", 5)) {
    cmd_type(cons, fat, cmdline);
  } else if (!strcmp(cmdline, "hlt")) {
    cmd_hlt(cons, fat);
  } else if (cmdline[0] != 0) {
    //不是命令，也不是空行
    putfont8_asc_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, "Bad command.", 12);
    cons_newline(cons);
    cons_newline(cons);
  }
}

void cmd_mem(struct CONSOLE *cons, unsigned int memtotal) 
{
  //mem命令
  struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
  struct SHEET *sheet = cons->sht;
  char s[30];

  int cursor_y = cons->cur_y;

  sprintf(s, "total %dMB", memtotal/(1024*1024));
  putfont8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, s, 30);
  cursor_y = cons_newline(cons);
  sprintf(s, "free %dKB", memman_total(memman)/1024);
  putfont8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, s, 30);
  cursor_y = cons_newline(cons);
  cursor_y = cons_newline(cons);

  return;
}

void cmd_cls(struct CONSOLE *cons)
{
  int x, y;
  struct SHEET *sheet = cons->sht;
  
  for (y = 28; y < 28+128; y++) {
    for (x = 8; x < 8+240; x++) {
      sheet->buf[x + y*sheet->bxsize] = COL8_000000;
    }
  }
  sheet_refresh(sheet, 8, 28, 8+240, 28+128);
  cons->cur_y = 28;  

  return ;
}

void cmd_dir(struct CONSOLE *cons)
{
  struct FILEINFO *finfo = (struct FILEINFO *)(ADR_DISKIMG + 0x002600);
  struct SHEET *sheet = cons->sht;

  int x, y;
  char s[30];

  for (x = 0; x < 224; x++) {//能容纳的最大文件数
    if (finfo[x].name[0] == 0x00) {//不存在文件
      break;
    }
    if (finfo[x].name[0] != 0xe5) {//0xe5表示文件已经被删除
      if ((finfo[x].type & 0x18) == 0) {
	sprintf(s, "filename.ext %7d", finfo[x].size);
	for (y = 0; y < 8; y++) {
	  s[y] = finfo[x].name[y];
	}
	s[9] = finfo[x].ext[0];
	s[10] = finfo[x].ext[1];
	s[11] = finfo[x].ext[2];
	putfont8_asc_sht(sheet, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 30);
	cons_newline(cons);
      }
    }
  }
  cons_newline(cons);

  return;
}

void cmd_type(struct CONSOLE *cons, int *fat, char *cmdline)
{
  //type 
  struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
  struct FILEINFO *finfo = file_search(cmdline+5, (struct FILEINFO *)(ADR_DISKIMG + 0x002600), 224);
  
  char *p;
  int i;

  if (finfo != 0) {
    //找到文件的情况下
    p = (char *)memman_alloc_4k(memman, finfo->size);//分配文件buffer
    file_loadfile(finfo->clustno, finfo->size, p, fat, (char *)(ADR_DISKIMG + 0x003e00));
        
    for (i = 0; i < finfo->size; i++) {
      cons_putchar(cons, p[i], 1);
    }
    memman_free_4k(memman, (int)p, finfo->size);
  } else {//没有找到文件
    putfont8_asc_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, "File not found.", 15);
    cons_newline(cons);
  }
  cons_newline(cons);

  return;
}

void cmd_hlt(struct CONSOLE *cons, int *fat)
{
  struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
  struct FILEINFO *finfo = file_search("HLT.HRB", (struct FILEINFO *)(ADR_DISKIMG + 0x002600), 224);
  struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT;

  char *p;
  if (finfo != 0) {
    p = (char *)memman_alloc_4k(memman, finfo->size);
    file_loadfile(finfo->clustno, finfo->size, p, fat, (char *)(ADR_DISKIMG + 0x003e00));
    set_segmdesc(gdt + 1003, finfo->size - 1, (int)p, AR_CODE32_ER);//设置段属性
    farjmp(0, 1003*8);
    memman_free_4k(memman, (int)p, finfo->size);
  } else {
    //没有找到文件
    putfont8_asc_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, "File not found.", 15);
    cons_newline(cons);
  }
  cons_newline(cons);

  return;
}

struct FILEINFO *file_search(char *name, struct FILEINFO *finfo, int max)
{
  int i, j;
  char s[12];
  
  //准备文件名
  for (j = 0; j < 11; j++) {
    s[j] = ' ';
  }
  j = 0;
  
  for (i = 0; name[i] != 0; i++) {
    if (j >= 11) { return 0;}
    
    if (name[i] == '.' &&
	j <= 8) {
      j = 8;
    } else {
      s[j] = name[i];
      if ('a' <= s[j] &&
	  s[j] <= 'z') {
	s[j] -= 0x20;
      }
      j++;
    }
  }
  //find file name
  for (i = 0; i < max; ) {
    if (finfo[i].name[0] == 0x00) {
      break;
    }
    if ((finfo[i].type & 0x18) == 0) {
      for (j = 0; j < 11; j++) {
	if (finfo[i].name[j] != s[j]) {
	  goto next;
	}
      }
      //find the file name
      return finfo+i;
    }
  next:
    i++;
  }
  
  return 0;
}
