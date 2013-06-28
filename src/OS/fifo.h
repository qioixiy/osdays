#ifndef _FIFO_H
#define _FIFO_H

#define FLAGS_OVERRUN 0

struct FIFO8 {
  unsigned char *buf;//fifo buffer pointer
  int p;//��һ������д���λ��
  int q;//��һ�����ݶ�ȡ��λ��
  int size;//�������ܵĳ���
  int free;//û�����ݵĳ���
  int flags;//
};

void fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf);
int fifo8_put(struct FIFO8 *fifo, unsigned char data);
int fifo8_get(struct FIFO8 *fifo);
int fifo8_status(struct FIFO8 *fifo);

#endif
