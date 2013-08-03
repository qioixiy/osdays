#include "console.h"
#include "bootpack.h"

void cons_putchar(struct CONSOLE *cons, int chr, char move);
int cons_newline(struct CONSOLE *cons);
void cons_runcmd(char *cmdline, struct CONSOLE *cons, int *fat, unsigned int memtotal);
void cmd_mem(struct CONSOLE *cons, unsigned int memtotal) ;
void cmd_cls(struct CONSOLE *cons);
void cmd_dir(struct CONSOLE *cons);
void cmd_type(struct CONSOLE *cons, int *fat, char *cmdline);
int cmd_app(struct CONSOLE *cons, int *fat, char *cmdline);

void cmd_hlt(struct CONSOLE *cons, int *fat);
struct FILEINFO *file_search(char *name, struct FILEINFO *finfo, int max);

static void hrb_api_linewin(struct SHEET *sht, int x0, int y0, int x1, int y1, int col);

#define putfonts8_asc_sht putfont8_asc_sht
void console_task(struct SHEET *sheet, unsigned int memtotal)
{
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
  cons.timer = timer_alloc();
  timer_init(cons.timer, &task->fifo, 1);
  timer_settime(cons.timer, 50);
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
	  timer_init(cons.timer, &task->fifo, 0);
	  if (cons.cur_c >= 0) {
	    cons.cur_c = COL8_FFFFFF;
	  }
	} else {
	  timer_init(cons.timer, &task->fifo, 1);
	  if (cons.cur_c >= 0) {
	    cons.cur_c = COL8_000000;
	  }
	}
	timer_settime(cons.timer, 50);
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
  } else if (cmdline[0] != 0) {
    //不是命令，也不是空行
    if (!cmd_app(cons, fat, cmdline)) {
      cons_putstr0(cons, "Bad command.\n\n");
    }
  }
}

void cmd_mem(struct CONSOLE *cons, unsigned int memtotal) 
{
  //mem命令
  struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
  char s[60];

  sprintf(s, "total %dMB\nfree %dKB\n\n", memman_total(memman)/1024, memtotal/(1024*1024));
  cons_putstr0(cons, s);

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

  int x, y;
  char s[30];

  for (x = 0; x < 224; x++) {//能容纳的最大文件数
    if (finfo[x].name[0] == 0x00) {//不存在文件
      break;
    }
    if (finfo[x].name[0] != 0xe5) {//0xe5表示文件已经被删除
      if ((finfo[x].type & 0x18) == 0) {
	sprintf(s, "filename.ext %7d\n", finfo[x].size);
	for (y = 0; y < 8; y++) {
	  s[y] = finfo[x].name[y];
	}
	s[9] = finfo[x].ext[0];
	s[10] = finfo[x].ext[1];
	s[11] = finfo[x].ext[2];
	cons_putstr0(cons, s);
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

  if (finfo != 0) {
    //找到文件的情况下
    p = (char *)memman_alloc_4k(memman, finfo->size);//分配文件buffer
    file_loadfile(finfo->clustno, finfo->size, p, fat, (char *)(ADR_DISKIMG + 0x003e00));
    cons_putstr1(cons, p, finfo->size);
    memman_free_4k(memman, (int)p, finfo->size);
  } else {//没有找到文件
    cons_putstr0(cons, "File not found.\n");
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
    farcall(0, 1003*8);
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

//尝试以应用程序运行
int cmd_app(struct CONSOLE *cons, int *fat, char *cmdline)
{
  struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
  struct FILEINFO *finfo;
  struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT;

  char name[18], *p, *q;
  struct TASK *task = task_now();
  int i;
  
  struct SHTCTL *shtctl;
  struct SHEET  *sht;

  //根据命令行生成文件名
  for (i = 0; i < 13; i++) {
    if (cmdline[i] <= ' ') {
      break;
    }
    name[i] = cmdline[i];
  }
  name[i] = 0;//

  //寻找文件
  finfo = file_search(name, (struct FILEINFO *)(ADR_DISKIMG + 0x002600), 224);
  if (finfo == 0 && name[i-1] != '.') {
    //找不到文件加上后缀重新查找
    name[i] = '.';
    name[i+1] = 'H';
    name[i+2] = 'R';
    name[i+3] = 'B';
    name[i+4] = 0;
    finfo = file_search(name, (struct FILEINFO *)(ADR_DISKIMG + 0x002600), 224);
  }

  if (finfo != 0) {
    int segsiz, datsiz, esp, dathrb;

    //找到文件
    p = (char *)memman_alloc_4k(memman, finfo->size);

    *((int *)0xfe8) = (int)p;//保存CS基址,代码段的起始位置
    file_loadfile(finfo->clustno, finfo->size, p, fat, (char *)(ADR_DISKIMG + 0x003e00));
    if (finfo->size >= 36 &&
	strncmp(p+4, "Hari", 4) == 0 &&
	*p == 0x00) {
      segsiz = *((int *)(p + 0x0000));
      esp = *((int *)(p + 0x000c));
      datsiz = *((int *)(p + 0x0010));
      dathrb = *((int *)(p + 0x0014));
      q = (char *)memman_alloc_4k(memman, segsiz);//申请数据段内存
      *((int *)0xfe8) = (int)q;
      
      set_segmdesc(gdt + 1003, finfo->size - 1, (int)p, AR_CODE32_ER+0x60);//设置段属性，代码段,加上0x60表示是应用程序所有
      set_segmdesc(gdt + 1004, segsiz - 1,     (int)q, AR_DATA32_RW+0x60);//设置段可写属性，数据段
      
      for (i = 0; i < datsiz; i++) {//复制hrb数据段信息到正确的内存位置
	q[esp+i] = p[dathrb + i];
      }
      start_app(0x1b, 1003*8, esp, 1004*8, &(task->tss.esp0));

      shtctl = (struct SHTCTL *)*((int *)0x0fe4);
      for (i = 0; i < MAX_SHEETS; i++) {
	sht = &(shtctl->sheets0[i]);
	if ((sht->flags & 0x11) == 0x11 && 
	    sht->task == task) {
	  //找到应用程序遗留的sheet
	  sheet_free(sht);
	}
      }

      memman_free_4k(memman, (int)q, segsiz);//释放数据段空间
    } else {
      cons_putstr0(cons, ".hrb file format error!\n");
    }

    memman_free_4k(memman, (int)p, finfo->size);
    cons_newline(cons);
    return 1;
  }
  //找到文件
  return 0;
}

void cons_putstr0(struct CONSOLE *cons, char *s)
{
  for (; *s != 0; s++) {
    cons_putchar(cons, *s, 1);
  }
  return ;
}

void cons_putstr1(struct CONSOLE *cons, char *s, int l)
{
  int i;
  for (i = 0; i < l; i++) {
    cons_putchar(cons, s[i], 1);
  }
  return ;
}

int hrb_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax)
{
  int ds_base = *((int *)0xfe8);//取得DS基址
  struct TASK *task = task_now();
  struct CONSOLE *cons = (struct CONSOLE *)*((int *)0xfec);
  struct SHTCTL *shtctl = (struct SHTCTL *)*((int *)0xfe4);
  struct SHEET *sht;
  int i;
  int *reg = &eax + 1;//强行改写通过PUSHAD保存的值
  /*reg[0]:EDI,....reg[7]:EAX*/
  static int x = 0, y = 0;

  if (edx == 1) {
    cons_putchar(cons, eax&0xff, 1);
  } else if (edx == 2) {
    cons_putstr0(cons, (char *)ebx + ds_base);
    //sprintf(s, "%08X", ebx);
    //cons_putstr0(cons, s);
  } else if (edx == 3) {
    cons_putstr1(cons, (char *)ebx + ds_base, ecx);
  } else if (edx == 4) {
    return (int)&(task->tss.esp0);
  } else if (edx == 5) {
    sht = sheet_alloc(shtctl);
    sht->task = task;//将当前task与sheet相关联
    sht->flags |= 0x10;//打开窗口自动关闭的功能
    sheet_setbuf(sht, (char *)ebx+ds_base, esi, edi, eax);
    make_window8((char *)ebx + ds_base, esi, edi, (char *)ecx + ds_base, 0);
    sheet_slide(sht, 100+x++, 50+y++);
    sheet_updown(sht, 3);//在task_a之上
    reg[7] = (int)sht;//返回值
  } else if (edx == 6){
    sht = (struct SHEET *)(ebx & 0xfffffffe);
    putfont8_asc(sht->buf, sht->bxsize, esi, edi, eax, (char *)ebp+ds_base);
    if ((ebx & 1) == 0) {
      sheet_refresh(sht, esi, edi, esi+ecx*8, edi+16);
    }
  } else if (edx == 7) {
    sht = (struct SHEET *)(ebx & 0xfffffffe);
    boxfill8(sht->buf, sht->bxsize, ebp, eax, ecx, esi, edi);
    if ((ebx & 1) == 0) {
      sheet_refresh(sht, eax, ecx, esi+1, edi+1);
    }
  } else if (edx == 8) {
    memman_init((struct MEMMAN *)(ebx+ds_base));
    ecx &= 0xfffffff0;//以16字节为单位
    memman_free((struct MEMMAN *)(ebx+ds_base), eax, ecx);
  } else if (edx == 9) {
    ecx = (ecx+0x0f)&0xfffffff0;//以16字节为单位取整
    reg[7] = memman_alloc((struct MEMMAN *)(ebx+ds_base), ecx);
  } else if (edx == 10) {
    ecx = (ecx+0x0f)&0xfffffff0;
    memman_free((struct MEMMAN *)(ebx+ds_base), eax, ecx);
  } else if (edx == 11) {
    sht = (struct SHEET *)(ebx & 0xfffffffe);
    sht->buf[sht->bxsize*edi + esi] = eax;//色号
    if ((ebx & 1) == 0) {
      sheet_refresh(sht, esi, edi, esi+1, edi+1);
    }
  } else if (edx == 12) {
    sht = (struct SHEET *)ebx;
    sheet_refresh(sht, eax, ecx, esi, edi);
  } else if (edx == 13) {
    sht = (struct SHEET *)(ebx & 0xfffffffe);
    hrb_api_linewin(sht, eax, ecx, esi, edi, ebp);
    if ((ebx & 1) == 0) {
      sheet_refresh(sht, esi, edi, esi+1, edi+1);
    }
  } else if (edx == 14) {
    sheet_free((struct SHEET *)ebx);
  } else if (edx == 15) {
    for(;;) {
      io_cli();
      if (fifo32_status(&task->fifo) == 0) {
	if (eax != 0 ) {
	  task_sleep(task);/*fifo 为空，休眠并等待*/
	} else {
	  io_sti();
	  reg[7] = -1;
	  return 0;
	}
      }
      i = fifo32_get(&task->fifo);
      io_sti();
      if (i <= 1) {/*光标定时器*/
	//应用程序运行时不需要显示光标，因此总是将下次显示用的值置为1
	timer_init(cons->timer, &task->fifo, 1);//下次置为1
	timer_settime(cons->timer, 50);
      }
      if (i == 2) {//光标ON
	cons->cur_c = COL8_FFFFFF;
      }
      if (i == 3) {
	cons->cur_c = -1;//光标OFF
      }
      if (256 <= i && i <= 511) {//键盘数据，通过task A得到
	reg[7] = i - 256;
	return 0;
      }
    }
  }
  return 0;
}

//栈异常处理
int inthandler0c(int *esp)
{
  struct CONSOLE *cons = (struct CONSOLE *)*((int *)0x0fec);
  struct TASK *task = task_now();
  char s[30];

  cons_putstr0(cons, "\nINT 0C:\n Stack Exception.\n");
  sprintf(s, "EIP = %08X\n", esp[11]);
  cons_putstr0(cons, s);

  return (int)&(task->tss.esp0);//让应用程序强制结束
}

//一般异常处理
int inthandler0d(int *esp)
{
  struct CONSOLE *cons = (struct CONSOLE *)*((int *)0x0fec);
  struct TASK *task = task_now();

  cons_putstr0(cons, "\nINT 0D:\n General Protected Exception.\n");
  return (int)&(task->tss.esp0);//让应用程序强制结束
}

static void hrb_api_linewin(struct SHEET *sht, int x0, int y0, int x1, int y1, int col)
{
  int i, x, y, len, dx, dy;
  
  dx = x1 - x0;
  dy = y1 - y0;
  x = x0<<10;
  y = y0<<10;
  
  if (dx < 0) {
    dx = -dx;
  }
  if (dy < 0) {
    dy = -dy;
  }

  if (dx >= dy) {
    len = dx+1;
    if (x0 > x1) {
      dx = -1024;
    } else {
      dx = 1024;
    }

    if (y0 <= y1) {
      dy = ((y1 - y0+1)<<10)/len;
    } else {
      dy = ((y1 - y0-1)<<10)/len;
    }
  } else {
    len = dy+1;
    if (y0 > y1) {
      dy = -1024;
    } else {
      dy = 1024;
    }

    if (x0 <= x1) {
      dx = ((x1 - x0+1)<<10)/len;
    } else {
      dx = ((x1 - x0-1)<<10)/len;
    }
  }

  for (i = 0; i < len; i++) {
    sht->buf[(y>>10) * sht->bxsize + (x>>10)] = col;
    x += dx;
    y += dy;
  }
}
