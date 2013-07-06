#ifndef _FIFO_H
#define _FIFO_H

#define FLAGS_OVERRUN 0

struct FIFO32 {
  unsigned int *buf;//fifo buffer pointer
  int p;//��һ������д���λ��
  int q;//��һ�����ݶ�ȡ��λ��
  int size;//�������ܵĳ���
  int free;//û�����ݵĳ���
  int flags;//
};

void fifo32_init(struct FIFO32 *fifo, int size, unsigned int *buf);
unsigned int fifo32_put(struct FIFO32 *fifo, unsigned int data);
unsigned int fifo32_get(struct FIFO32 *fifo);
int fifo32_status(struct FIFO32 *fifo);

#endif
