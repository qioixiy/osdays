#include "fifo.h"

//��ʼ��FIFO buf
void fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf)
{
  fifo->size = size;
  fifo->buf = buf;
  fifo->free = size;
  fifo->flags = 0;
  fifo->p = 0;
  fifo->q = 0;

  return;
}

//��fifo�б�������
int fifo8_put(struct FIFO8 *fifo, unsigned char data)
{
  if (fifo->free == 0) {
    //û�пռ䣬���
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

int fifo8_get(struct FIFO8 *fifo)
{
  int data;

  if (fifo->free == fifo->size) {
    //û������
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

//���㱣���ж�������
int fifo8_status(struct FIFO8 *fifo)
{
  return fifo->size - fifo->free;
}
