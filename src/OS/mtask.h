#ifndef _TASK_H
#define _TASK_H

struct TSS32{
  int backlink, esp0, ss0,esp1, ss1, esp2, ss2, cr3;
  int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
  int es, cs, ss, ds, fs, gs;
  int ldtr, iomap;
};

extern struct TIMER *mt_timer;
extern int mt_tr;

void mt_init(void);
void mt_taskswitch(void);

#endif
