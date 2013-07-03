#include "memory.h"
#include "naskfunc.h"

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

//4kΪ��Ԫ���з���
unsigned int memman_alloc_4k(struct MEMMAN * man, unsigned int size)
{
  unsigned int a;
  size = (size + 0xfff) & 0xfffff000;//��������4K
  a = memman_alloc(man, size);
  return a;
}

//4k��Ԫ�ͷ�
int memman_free_4k(struct MEMMAN * man, unsigned int addr, unsigned int size) 
{
  int i;
  size = (size + 0xfff) & 0xfffff000;//��������4K
  i = memman_free(man, addr, size);
  return i;
}

int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
  int i, j;
  
  //Ϊ�˱��ڹ����ڴ棬 ��free[]����addr��˳������
  //���ԣ��Ⱦ���Ӧ�÷�������
  for (i = 0; i < man->frees; i++) {
    if (man->free[i].addr > addr) {
      break;
    }
  }
  
  //1��free[i-1].addr < addr <free[i].addr
  if (i > 0) {
    //ǰ���п����õ��ڴ�
    if (man->free[i - 1].addr + man->free[i - 1].size == addr) {
      //������ǰ����ڴ������һ��
      man->free[i - 1].size += size;
      if (i < man->frees) {
	//����Ҳ�У�
	if (addr + size == man->free[i].addr) {
	  man->free[i - 1].size += man->free[i].size;
	  //ɾ��free[i]
	  man->frees--;//�ܵļ�ȥ1
	  for (;i < man->frees; i++) {
	    man->free[i] = man->free[i + 1];//���ƽṹ��
	  }
	}
      }
      return 0;//�ɹ�����
    }
  }
  //2�����ܺ�ǰ��Ŀռ�ϲ�
  if (i < man->frees) {
    //���������ϲ���
    if (addr + size == man->free[i].addr){
      //���������ĺϲ�һ��
      man->free[i].addr = addr;
      man->free[i].size += size;
      return 0;//�ɹ�����
    }
  }
  //3�����ܺ�ǰ����ߺ���ϲ������
  if (man->frees < MEMMAN_FREES) {
    //free[i]֮��ģ�����ƶ�һ����Ԫ
    for (j = man->frees; j >i; j--) {
      man->free[j] = man->free[j - 1];
    }
    man->frees++;
    if (man->maxfrees < man->frees) {
      man->maxfrees = man->frees;//�������ֵ
    }
    man->free[i].addr = addr;
    man->free[i].size = size;
    return 0;//�ɹ�����
  }

  //4������������ƶ��ˣ��Ѿ������ֵ��
  man->losts++;
  man->lostsize += size;
  return -1;//����ʧ��
}
