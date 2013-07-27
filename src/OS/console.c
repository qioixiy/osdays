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
  
  //��ʾ��ʾ��
  cons_putchar(&cons, '>', 1);
  
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
      
      if (i == 2) {//���ON
	cons.cur_c = COL8_FFFFFF;
      }
     
      if (i == 3) {//���OFF
	boxfill8(sheet->buf, sheet->bxsize, COL8_000000, cons.cur_x, cons.cur_y, cons.cur_x+7, cons.cur_y + 15);
	cons.cur_c = -1;
      }
     
      if (256 <= i && i <= 511) {//��������
	if (i == 8 + 256) {//�˸��
	  if (cons.cur_x > 16) {
	    //�ÿհײ������󽫹��ǰ��һλ
	    cons_putchar(&cons, ' ', 0);
	    cons.cur_x -= 8;
	  }
	} else if (i == 10+256) {//�س���enter
	  //�ÿո񽫹�����
	  cons_putchar(&cons, ' ', 0);
     	  cmdline[cons.cur_x/8-2] = 0;
	  cons_newline(&cons);

	  //ִ������
	  cons_runcmd(cmdline, &cons, fat, memtotal);

	  //��ʾ��ʾ��
	  cons_putchar(&cons, '>', 1);
	} else {//һ���ַ�
	  if (cons.cur_x < 240) {
	    //��ʾ�ַ��󽫹����ƶ�һλ
	    cmdline[cons.cur_x/8 - 2] = i - 256;//�����ַ���cmdline
	    cons_putchar(&cons, i - 256, 1);
	  }
	}
      }
      
      //������ʾ���
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
  
  if (s[0] == 0x09) {//�Ʊ��
    for (;;) {
      putfont8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, " ", 1);
      cons->cur_x += 8;
      if (cons->cur_x == 8 + 240) {
	cons->cur_x = 8;
	cons->cur_y = cons_newline(cons);
      }
      if (((cons->cur_x - 8) & 0x1f) == 0) {
	//��32������ֹͣ
	break;
      }
    }
  } else if (s[0] == 0x0a) {//���з�
    cons->cur_x = 8;
    cons->cur_y = cons_newline(cons);
  } else if (s[0] == 0x0d) {//�س���
    ;
  } else {
    putfont8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 1);
    if (move != 0) {
      cons->cur_x += 8;
      if (cons->cur_x == 8 + 240) {//�������Ҷ˺���
	cons->cur_x = 8;
	cons->cur_y = cons_newline(cons);
      }
    }
  }
  return;
}

//���д���
int cons_newline(struct CONSOLE *cons)
{
  int x, y;
  struct SHEET *sheet = cons->sht;

  int cursor_y = cons->cur_y;

  if (cursor_y < 28+112) {
    cursor_y += 16;//����
  } else {
    //����
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
    //�������Ҳ���ǿ���
    if (!cmd_app(cons, fat, cmdline)) {
      cons_putstr0(cons, "Bad command.\n\n");
    }
  }
}

void cmd_mem(struct CONSOLE *cons, unsigned int memtotal) 
{
  //mem����
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

  for (x = 0; x < 224; x++) {//�����ɵ�����ļ���
    if (finfo[x].name[0] == 0x00) {//�������ļ�
      break;
    }
    if (finfo[x].name[0] != 0xe5) {//0xe5��ʾ�ļ��Ѿ���ɾ��
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
    //�ҵ��ļ��������
    p = (char *)memman_alloc_4k(memman, finfo->size);//�����ļ�buffer
    file_loadfile(finfo->clustno, finfo->size, p, fat, (char *)(ADR_DISKIMG + 0x003e00));
    cons_putstr1(cons, p, finfo->size);
    memman_free_4k(memman, (int)p, finfo->size);
  } else {//û���ҵ��ļ�
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
    set_segmdesc(gdt + 1003, finfo->size - 1, (int)p, AR_CODE32_ER);//���ö�����
    farcall(0, 1003*8);
    memman_free_4k(memman, (int)p, finfo->size);
  } else {
    //û���ҵ��ļ�
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
  
  //׼���ļ���
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

//������Ӧ�ó�������
int cmd_app(struct CONSOLE *cons, int *fat, char *cmdline)
{
  struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
  struct FILEINFO *finfo;
  struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT;

  char name[18], *p, *q;
  struct TASK *task = task_now();
  int i;
  
  //���������������ļ���
  for (i = 0; i < 13; i++) {
    if (cmdline[i] <= ' ') {
      break;
    }
    name[i] = cmdline[i];
  }
  name[i] = 0;//

  //Ѱ���ļ�
  finfo = file_search(name, (struct FILEINFO *)(ADR_DISKIMG + 0x002600), 224);
  if (finfo == 0 && name[i-1] != '.') {
    //�Ҳ����ļ����Ϻ�׺���²���
    name[i] = '.';
    name[i+1] = 'H';
    name[i+2] = 'R';
    name[i+3] = 'B';
    name[i+4] = 0;
    finfo = file_search(name, (struct FILEINFO *)(ADR_DISKIMG + 0x002600), 224);
  }

  if (finfo != 0) {
    int segsiz, datsiz, esp, dathrb;

    //�ҵ��ļ�
    p = (char *)memman_alloc_4k(memman, finfo->size);

    *((int *)0xfe8) = (int)p;//����CS��ַ,����ε���ʼλ��
    file_loadfile(finfo->clustno, finfo->size, p, fat, (char *)(ADR_DISKIMG + 0x003e00));
    if (finfo->size >= 36 &&
	strncmp(p+4, "Hari", 4) == 0 &&
	*p == 0x00) {
      segsiz = *((int *)(p + 0x0000));
      esp = *((int *)(p + 0x000c));
      datsiz = *((int *)(p + 0x0010));
      dathrb = *((int *)(p + 0x0014));
      q = (char *)memman_alloc_4k(memman, segsiz);//�������ݶ��ڴ�
      *((int *)0xfe8) = (int)q;
      
      set_segmdesc(gdt + 1003, finfo->size - 1, (int)p, AR_CODE32_ER+0x60);//���ö����ԣ������,����0x60��ʾ��Ӧ�ó�������
      set_segmdesc(gdt + 1004, segsiz - 1,     (int)q, AR_DATA32_RW+0x60);//���öο�д���ԣ����ݶ�
      
      for (i = 0; i < datsiz; i++) {//����hrb���ݶ���Ϣ����ȷ���ڴ�λ��
	q[esp+i] = p[dathrb + i];
      }
      start_app(0x1b, 1003*8, esp, 1004*8, &(task->tss.esp0));
      memman_free_4k(memman, (int)q, segsiz);//�ͷ����ݶοռ�
    } else {
      cons_putstr0(cons, ".hrb file format error!\n");
    }

    memman_free_4k(memman, (int)p, finfo->size);
    cons_newline(cons);
    return 1;
  }
  //�ҵ��ļ�
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
  int ds_base = *((int *)0xfe8);//ȡ��DS��ַ
  struct TASK *task = task_now();
  struct CONSOLE *cons = (struct CONSOLE *)*((int *)0xfec);
  struct SHTCTL *shtctl = (struct SHTCTL *)*((int *)0xfe4);
  struct SHEET *sht;

  int *reg = &eax + 1;//ǿ�и�дͨ��PUSHAD�����ֵ
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
    sheet_setbuf(sht, (char *)ebx+ds_base, esi, edi, eax);
    make_window8((char *)ebx + ds_base, esi, edi, (char *)ecx + ds_base, 0);
    sheet_slide(sht, 100+x++, 50+y++);
    sheet_updown(sht, 3);//
    reg[7] = (int)sht;//����ֵ
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
    ecx &= 0xfffffff0;//��16�ֽ�Ϊ��λ
    memman_free((struct MEMMAN *)(ebx+ds_base), eax, ecx);
  } else if (edx == 9) {
    ecx = (ecx+0x0f)&0xfffffff0;//��16�ֽ�Ϊ��λȡ��
    reg[7] = memman_alloc((struct MEMMAN *)(ebx+ds_base), ecx);
  } else if (edx == 10) {
    ecx = (ecx+0x0f)&0xfffffff0;
    memman_free((struct MEMMAN *)(ebx+ds_base), eax, ecx);
  } else if (edx == 11) {
    sht = (struct SHEET *)(ebx & 0xfffffffe);
    sht->buf[sht->bxsize*edi + esi] = eax;//ɫ��
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
  }
  return 0;
}

//ջ�쳣����
int inthandler0c(int *esp)
{
  struct CONSOLE *cons = (struct CONSOLE *)*((int *)0x0fec);
  struct TASK *task = task_now();
  char s[30];

  cons_putstr0(cons, "\nINT 0C:\n Stack Exception.\n");
  sprintf(s, "EIP = %08X\n", esp[11]);
  cons_putstr0(cons, s);

  return (int)&(task->tss.esp0);//��Ӧ�ó���ǿ�ƽ���
}

//һ���쳣����
int inthandler0d(int *esp)
{
  struct CONSOLE *cons = (struct CONSOLE *)*((int *)0x0fec);
  struct TASK *task = task_now();

  cons_putstr0(cons, "\nINT 0D:\n General Protected Exception.\n");
  return (int)&(task->tss.esp0);//��Ӧ�ó���ǿ�ƽ���
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
