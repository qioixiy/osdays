#include "fifo.h"

//初始化FIFO buf
void fifo32_init(struct FIFO32 *fifo, int size, unsigned int *buf)
{
  fifo->size = size;
  fifo->buf = buf;
  fifo->free = size;
  fifo->flags = 0;
  fifo->p = 0;
  fifo->q = 0;

  return;
}

//向fifo中保存数据
unsigned int fifo32_put(struct FIFO32 *fifo, unsigned int data)
{
  if (fifo->free == 0) {
    //没有空间，溢出
    fifo->flags = FLAGS_OVERRUN;
    return -1;
  }

  fifo->buf[fifo->p] = data;
  fifo->p++;
  fifo->free--;
  if (fifo->p == fifo->size) {
    fifo->p = 0;
  }
 
  return 0;
}

unsigned int fifo32_get(struct FIFO32 *fifo)
{
  int data;

  if (fifo->free == fifo->size) {
    //没有数据
    return -1;
  }
  
  data = fifo->buf[fifo->q];
  fifo->free++;
  fifo->q++;
  if (fifo->q == fifo->size) {
    fifo->q = 0;
  }
  return data;
}

//计算保存有多少数据
int fifo32_status(struct FIFO32 *fifo)
{
  return fifo->size - fifo->free;
}
