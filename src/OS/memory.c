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

int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
  int i, j;
  
  //为了便于归纳内存， 将free[]按照addr的顺序排列
  //所以，先决定应该放在哪里
  for (i = 0; i < man->frees; i++) {
    if (man->free[i].addr > addr) {
      break;
    }
  }
  
  //1、free[i-1].addr < addr <free[i].addr
  if (i > 0) {
    //前面有可以用的内存
    if (man->free[i - 1].addr + man->free[i - 1].size == addr) {
      //可以与前面的内存归纳在一起
      man->free[i - 1].size += size;
      if (i < man->frees) {
	//后面也有？
	if (addr + size == man->free[i].addr) {
	  man->free[i - 1].size += man->free[i].size;
	  //删除free[i]
	  man->frees--;//总的减去1
	  for (;i < man->frees; i++) {
	    man->free[i] = man->free[i + 1];//复制结构体
	  }
	}
      }
      return 0;//成功返回
    }
  }
  //2、不能和前面的空间合并
  if (i < man->frees) {
    //可以与后面合并？
    if (addr + size == man->free[i].addr){
      //可以与后面的合并一起
      man->free[i].addr = addr;
      man->free[i].size += size;
      return 0;//成功返回
    }
  }
  //3、不能和前面或者后面合并的情况
  if (man->frees < MEMMAN_FREES) {
    //free[i]之后的，向后移动一个单元
    for (j = man->frees; j >i; j--) {
      man->free[j] = man->free[j - 1];
    }
    man->frees++;
    if (man->maxfrees < man->frees) {
      man->maxfrees = man->frees;//更新最大值
    }
    man->free[i].addr = addr;
    man->free[i].size = size;
    return 0;//成功返回
  }

  //4、不能向后面移动了，已经是最大值。
  man->losts++;
  man->lostsize += size;
  return -1;//返回失败
}
