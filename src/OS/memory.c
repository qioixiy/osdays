#include "memory.h"

unsigned int memtest(unsigned int start, unsigned int end)
{
  char flg486 = 0;
  unsigned int eflg, cr0, i;

  //确认CPU是386还是486以上的
  eflg = io_load_eflags();
  eflg |= EFLAGS_AC_BIT;//AC-bit = 1
  io_store_eflags(eflg);
  eflg = io_load_eflags();

  //如果是386，即使是设定AC=1，AC的值还是会自动回到0
  if ((eflg & EFLAGS_AC_BIT) != 0) {
      flg486 = 1;
  }
  eflg &= ~EFLAGS_AC_BIT;//AC-bit = 0
  io_store_eflags(eflg);

  if (flg486 != 0) {
    cr0 = load_cr0();
    cr0 |= CR0_CACHE_DISABLE;//禁止缓冲
    store_cr0(cr0);
  }

  i = memtest_sub(start, end);

  if (flg486 != 0) {
    cr0 = load_cr0();
    cr0 &= ~CR0_CACHE_DISABLE;//允许缓冲
    store_cr0(cr0);
  }

  return i;
}

void memman_init(struct MEMMAN *man)
{
  man->frees = 0;//可用信息数目
  man->maxfrees = 0;//用于观察可用状态：frees的最大值
  man->lostsize = 0;//释放失败的内存大小总和
  man->losts = 0;//释放失败的次数
  return;
}

//报告空余空间的大小
unsigned int memman_total(struct MEMMAN *man)
{
  unsigned int i, t = 0;
  for (i = 0; i < man->frees; i++) {
    t += man->free[i].size;
  }
  return t;
}

//分配内存
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size)
{
  unsigned int i, a;
  for (i = 0; i < man->frees; i++) {
    if (man->free[i].size >= size) {
      //找到足够大的内存位置
      a = man->free[i].addr;
      man->free[i].addr += size;
      man->free[i].size -= size;
      if (man->free[i].size == 0) {
	//如果size为0就减掉一条可用信息
	man->frees--;
	for(; i < man->frees; i++) {
	  man->free[i] = man->free[i+1];
	}
      }
      return a;
    }
  }
  return 0;//分配失败
}
