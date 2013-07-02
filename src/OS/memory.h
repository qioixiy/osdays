#ifndef _MEMORY_H
#define _MEMORY_H
//
#define EFLAGS_AC_BIT 0x00040000
#define CR0_CACHE_DISABLE 0x60000000

//memtest
unsigned int memtest(unsigned int start, unsigned int end);
unsigned int memtest_sub(unsigned int start, unsigned int end);

#define MEMMAN_FREES 4096 //

//可用信息
struct FREEINFO
{
  unsigned int addr, size;
};

//内存管理
struct MEMMAN{
  int frees, maxfrees, lostsize, losts;
  struct FREEINFO free[MEMMAN_FREES];
};//

//内存管理的起始地址
#define MEMMAN_ADDR 0X003C0000
//memory manager
void memman_init(struct MEMMAN *man);
unsigned int memman_total(struct MEMMAN *man);
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size);
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size);

#endif
