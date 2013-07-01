#include "memory.h"

unsigned int memtest(unsigned int start, unsigned int end)
{
  char flg486 = 0;
  unsigned int eflg, cr0, i;

  //ȷ��CPU��386����486���ϵ�
  eflg = io_load_eflags();
  eflg |= EFLAGS_AC_BIT;//AC-bit = 1
  io_store_eflags(eflg);
  eflg = io_load_eflags();

  //�����386����ʹ���趨AC=1��AC��ֵ���ǻ��Զ��ص�0
  if ((eflg & EFLAGS_AC_BIT) != 0) {
      flg486 = 1;
  }
  eflg &= ~EFLAGS_AC_BIT;//AC-bit = 0
  io_store_eflags(eflg);

  if (flg486 != 0) {
    cr0 = load_cr0();
    cr0 |= CR0_CACHE_DISABLE;//��ֹ����
    store_cr0(cr0);
  }

  i = memtest_sub(start, end);

  if (flg486 != 0) {
    cr0 = load_cr0();
    cr0 &= ~CR0_CACHE_DISABLE;//������
    store_cr0(cr0);
  }

  return i;
}

void memman_init(struct MEMMAN *man)
{
  man->frees = 0;//������Ϣ��Ŀ
  man->maxfrees = 0;//���ڹ۲����״̬��frees�����ֵ
  man->lostsize = 0;//�ͷ�ʧ�ܵ��ڴ��С�ܺ�
  man->losts = 0;//�ͷ�ʧ�ܵĴ���
  return;
}

//�������ռ�Ĵ�С
unsigned int memman_total(struct MEMMAN *man)
{
  unsigned int i, t = 0;
  for (i = 0; i < man->frees; i++) {
    t += man->free[i].size;
  }
  return t;
}

//�����ڴ�
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size)
{
  unsigned int i, a;
  for (i = 0; i < man->frees; i++) {
    if (man->free[i].size >= size) {
      //�ҵ��㹻����ڴ�λ��
      a = man->free[i].addr;
      man->free[i].addr += size;
      man->free[i].size -= size;
      if (man->free[i].size == 0) {
	//���sizeΪ0�ͼ���һ��������Ϣ
	man->frees--;
	for(; i < man->frees; i++) {
	  man->free[i] = man->free[i+1];
	}
      }
      return a;
    }
  }
  return 0;//����ʧ��
}
